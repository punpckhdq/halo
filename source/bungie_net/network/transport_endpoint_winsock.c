/*
TRANSPORT_ENDPOINT_WINSOCK.C
*/

/* ---------- headers */

// winsockx.h only gives us the win32 flavour of the WSA_* codes (WSA_INVALID_HANDLE==6,
// WSA_IO_PENDING==997, ...), which is what winsock_error_to_string() names, when WIN32 is defined
#define WIN32

#include "cseries.h"
#include "errors.h"
#include "thread.h"
#include "transport.h"
#include "transport_endpoint_winsock.h"

/* ---------- constants */

enum
{
	MAXIMUM_CONNECT_THREADS= 64, // bounds the connect-thread array
	SOCKET_BUFFER_MINIMUM_SIZE= 0x4000, // create_socket's SO_SNDBUF/SO_RCVBUF floor
	CONNECT_TIMEOUT= 10000, // milliseconds connect_endpoint waits for a pending connect
	CONNECT_MUTEX_TIMEOUT= 1000, // milliseconds
	LISTEN_BACKLOG= 32 // listen_endpoint's backlog
};

/* ---------- macros */

#define SWAP_LONG(x) ((((x)&0xff)<<24) | (((x)&0xff00)<<8) | (((x)>>8)&0xff00) | (((x)>>24)&0xff))
#define SWAP_WORD(x) ((word)((((x)>>8)&0xff) | (((x)&0xff)<<8)))

/* ---------- structures */

struct connect_thread
{
	struct thread_reference *thread;
	boolean terminated;
};

struct transport_endpoint_globals
{
	char *last_error_string; // 0x000
	long unused_4; // 0x004
	struct connect_thread threads[MAXIMUM_CONNECT_THREADS]; // 0x008
	long last_error; // 0x208, the code winsock_error_to_string() reported last
};

struct transport_connect_process
{
	transport_endpoint_ref ep; // 0x00
	transport_address address; // 0x04
	struct thread_reference *thread; // 0x1c
	struct mutex_reference *mutex; // 0x20
	boolean cancelled; // 0x24
}; // 0x28

/* ---------- prototypes */

static boolean add_connect_thread(struct thread_reference *thread);
static void mark_connection_thread_as_terminated(struct thread_reference *thread);
static void connection_thread_list_maintenance(void);
static SOCKET create_socket(long af, long type, long protocol);
static unsigned long __stdcall connect_async_thread_proc(void *parameter);

/* ---------- globals */

static struct transport_endpoint_globals transport_endpoint_globals;

/* ---------- private code */

static boolean add_connect_thread(
	struct thread_reference *thread)
{
	long index;

	index= 0;
	while (transport_endpoint_globals.threads[index].thread!=NULL && index<MAXIMUM_CONNECT_THREADS)
	{
		index++;
	}

	if (index<MAXIMUM_CONNECT_THREADS)
	{
		transport_endpoint_globals.threads[index].thread= thread;
		transport_endpoint_globals.threads[index].terminated= FALSE;
	}
	else
	{
		index= NONE;
	}

	return index!=NONE;
}

static void mark_connection_thread_as_terminated(
	struct thread_reference *thread)
{
	long index;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 79, thread);

	for (index= 0; index<MAXIMUM_CONNECT_THREADS; index++)
	{
		if (transport_endpoint_globals.threads[index].thread==thread)
		{
			transport_endpoint_globals.threads[index].terminated= TRUE;

			return;
		}
	}
}

static void connection_thread_list_maintenance(
	void)
{
	long index;

	for (index= 0; index<MAXIMUM_CONNECT_THREADS; index++)
	{
		if (transport_endpoint_globals.threads[index].thread!=NULL && transport_endpoint_globals.threads[index].terminated)
		{
			dispose_thread(transport_endpoint_globals.threads[index].thread);
			transport_endpoint_globals.threads[index].thread= NULL;
			transport_endpoint_globals.threads[index].terminated= FALSE;
		}
	}
}

/* ---------- public code */

transport_endpoint_ref create_transport_endpoint(
	long type)
{
	transport_endpoint_ref result= NULL;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 206, transport_initialized);

	connection_thread_list_maintenance();

	if (type==_transport_type_udp || type==_transport_type_tcp)
	{
		result= match_malloc("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 212, sizeof(struct transport_endpoint));
		if (result)
		{
			result->socket= INVALID_SOCKET;
			result->flags= 0;
			result->type= (char)type;
			result->error= _transport_error_none;
		}
	}

	return result;
}

void delete_transport_endpoint(
	transport_endpoint_ref ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 228, ep);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 229, transport_initialized);

	disconnect_endpoint(ep);
	match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 232, ep);

	connection_thread_list_maintenance();
}

short get_endpoint_address(
	transport_endpoint_ref ep,
	transport_address *address)
{
	short result= _transport_error_none;
	struct sockaddr_in sockaddress;
	int sockaddress_size= sizeof(sockaddress);

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 247, ep && address);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 248, transport_initialized);

	if (ep->socket!=INVALID_SOCKET)
	{
		if (getpeername(ep->socket, (struct sockaddr *)&sockaddress, &sockaddress_size)==0)
		{
			if (sockaddress.sin_family==AF_INET)
			{
				address->address.ipv4_address= SWAP_LONG(sockaddress.sin_addr.S_un.S_addr);
				address->address_length= IPV4_ADDRESS_LENGTH;
				address->port= SWAP_WORD(sockaddress.sin_port);
			}
			else
			{
				winsock_error_to_string(WSAGetLastError());
				result= _transport_error_address_unknown;
			}
		}
		else if (getsockname(ep->socket, (struct sockaddr *)&sockaddress, &sockaddress_size)==0 &&
			sockaddress.sin_family==AF_INET)
		{
			address->address.ipv4_address= SWAP_LONG(sockaddress.sin_addr.S_un.S_addr);
			address->address_length= IPV4_ADDRESS_LENGTH;
			address->port= SWAP_WORD(sockaddress.sin_port);
		}
		else
		{
			winsock_error_to_string(WSAGetLastError());
			result= _transport_error_address_unknown;
		}
	}
	else
	{
		result= _transport_error_address_unknown;
	}

	ep->error= (word)result;

	return result;
}

long get_endpoint_type(
	transport_endpoint_ref ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 300, ep);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 301, transport_initialized);

	return ep->type;
}

short set_endpoint_blocking(
	transport_endpoint_ref ep,
	long blocking)
{
	short result= _transport_error_none;
	unsigned long value;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 313, ep);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 314, transport_initialized);

	if (!endpoint_blocking(ep))
	{
		if (blocking)
		{
			value= 0;
			result= (short)ioctlsocket(ep->socket, FIONBIO, &value);
			if (result==0)
			{
				ep->flags&= (byte)~FLAG(_endpoint_non_blocking_bit);
			}
			else
			{
				winsock_error_to_string(WSAGetLastError());
				result= _transport_error_options_failed;
			}
		}
	}
	else if (!blocking)
	{
		value= 1;
		result= (short)ioctlsocket(ep->socket, FIONBIO, &value);
		if (result==0)
		{
			ep->flags|= (byte)FLAG(_endpoint_non_blocking_bit);
		}
		else
		{
			winsock_error_to_string(WSAGetLastError());
			result= _transport_error_options_failed;
		}
	}

	ep->error= (word)result;

	return result;
}

short bind_endpoint(
	transport_endpoint_ref ep,
	transport_address *address)
{
	short result= _transport_error_none;
	struct sockaddr_in sockaddress;
	unsigned long ipv4_address;
	word port;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 364, ep && address);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 365, transport_initialized);

	if (ep->socket==INVALID_SOCKET)
	{
		if (ep->type==_transport_type_tcp)
		{
			ep->socket= create_socket(AF_INET, SOCK_STREAM, 0);
			if (ep->socket==INVALID_SOCKET)
			{
				result= _transport_error_unknown;
			}
		}
		else if (ep->type==_transport_type_udp)
		{
			ep->socket= create_socket(AF_INET, SOCK_DGRAM, 0);
			if (ep->socket==INVALID_SOCKET)
			{
				result= _transport_error_unknown;
			}
		}
		else
		{
			result= _transport_error_bad_endpoint;
		}

		if (ep->socket==INVALID_SOCKET || result!=_transport_error_none)
		{
			result= _transport_error_unknown;
			ep->error= (word)result;

			return result;
		}
	}

	ipv4_address= address->address.ipv4_address;
	sockaddress.sin_addr.S_un.S_addr= SWAP_LONG(ipv4_address);
	port= address->port;
	sockaddress.sin_family= AF_INET;
	sockaddress.sin_port= SWAP_WORD(port);

	if (bind(ep->socket, (struct sockaddr *)&sockaddress, sizeof(sockaddress))!=0)
	{
		winsock_error_to_string(WSAGetLastError());
		result= _transport_error_bind_endpoint;
	}

	ep->error= (word)result;

	return result;
}

short connect_endpoint(
	transport_endpoint_ref ep,
	transport_address *address)
{
	short result= _transport_error_none;
	struct sockaddr_in sockaddress;
	fd_set write_fds;
	struct timeval timeout_value;
	boolean blocking;
	long error_code;
	long type;
	unsigned long timeout;
	unsigned long ipv4_address;
	word port;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 437, ep && address);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 438, transport_initialized);

	if (ep->type==_transport_type_udp)
	{
		type= SOCK_DGRAM;
	}
	else if (ep->type==_transport_type_tcp)
	{
		type= SOCK_STREAM;
	}
	else
	{
		result= _transport_error_bad_endpoint;
		ep->error= (word)result;

		return result;
	}

	if (ep->socket==INVALID_SOCKET)
	{
		ep->socket= create_socket(AF_INET, type, 0);
	}

	ipv4_address= address->address.ipv4_address;
	sockaddress.sin_addr.S_un.S_addr= SWAP_LONG(ipv4_address);
	port= address->port;
	sockaddress.sin_family= AF_INET;
	sockaddress.sin_port= SWAP_WORD(port);

	blocking= (boolean)!((ep->flags>>_endpoint_non_blocking_bit)&1);
	set_endpoint_blocking(ep, FALSE);

	if (connect(ep->socket, (struct sockaddr *)&sockaddress, sizeof(sockaddress))!=0)
	{
		error_code= WSAGetLastError();
		if (error_code==WSAEWOULDBLOCK)
		{
			timeout= system_milliseconds()+CONNECT_TIMEOUT;
			timeout_value.tv_sec= 1;
			timeout_value.tv_usec= 0;

			do
			{
				write_fds.fd_count= 1;
				write_fds.fd_array[0]= ep->socket;

				if (select(1, NULL, &write_fds, NULL, &timeout_value)==1)
				{
					error_code= 0;
				}
				else
				{
					error_code= WSAGetLastError();
				}

				if (system_milliseconds()>timeout)
				{
					closesocket(ep->socket);
					error_code= WSAEINPROGRESS;

					break;
				}
			}
			while (error_code==WSAEINPROGRESS);
		}

		if (error_code!=0)
		{
			winsock_error_to_string(error_code);
			result= _transport_error_connect_failed;
			ep->error= (word)result;

			return result;
		}
	}

	set_endpoint_blocking(ep, blocking);
	ep->flags&= (byte)~FLAG(_endpoint_non_blocking_bit);
	ep->flags|= (byte)(FLAG(_endpoint_connected_bit)|FLAG(_endpoint_unknown5_bit));
	ep->error= (word)result;

	return result;
}

void disconnect_endpoint(
	transport_endpoint_ref ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 545, ep);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 546, transport_initialized);

	if (ep->socket!=INVALID_SOCKET)
	{
		if (closesocket(ep->socket)!=0)
		{
			winsock_error_to_string(WSAGetLastError());
		}

		ep->socket= INVALID_SOCKET;
	}

	ep->flags&= (byte)~FLAG(_endpoint_connected_bit);
}

/* ---------- private code */

static unsigned long __stdcall connect_async_thread_proc(
	void *parameter)
{
	transport_connect_process_ref input= (transport_connect_process_ref)parameter;
	short result;
	struct thread_reference *thread;
	struct mutex_reference *mutex= NULL;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 569, input);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 570, input->ep);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 571, input->thread);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 572, transport_initialized);

	result= connect_endpoint(input->ep, &input->address);

	if (take_mutex(input->mutex, CONNECT_MUTEX_TIMEOUT))
	{
		if (input->cancelled)
		{
			disconnect_endpoint(input->ep);
		}

		thread= input->thread;
		mutex= input->mutex;
	}
	else
	{
		result= _transport_error_unknown;
	}

	input->ep->error= (word)result;

	if (mutex)
	{
		match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 594, input);
		release_mutex(mutex);
		dispose_mutex(mutex);
	}

	if (thread)
	{
		mark_connection_thread_as_terminated(thread);
	}

	return result;
}

/* ---------- public code */

short connect_endpoint_async(
	transport_endpoint_ref ep,
	transport_address *address,
	transport_connect_process_ref *process_ref_ptr)
{
	long result;
	transport_connect_process_ref process;

	connection_thread_list_maintenance();

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 616, ep && address && process_ref_ptr);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 617, transport_initialized);

	process= debug_malloc(sizeof(struct transport_connect_process), TRUE,
		"c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 619);
	if (process)
	{
		process->address= *address;
		process->ep= ep;
		process->cancelled= FALSE;

		if (create_mutex(&process->mutex))
		{
			if (create_thread((byte)FLAG(_thread_flag_below_normal_priority_bit),
				connect_async_thread_proc, process, &process->thread))
			{
				if (add_connect_thread(process->thread))
				{
					*process_ref_ptr= process;
					result= _transport_result_connect_in_progress;
				}
				else
				{
					dispose_thread(process->thread);
					dispose_mutex(process->mutex);
					process->thread= NULL;
					result= _transport_error_unknown;
				}
			}
			else
			{
				match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 642, process);
				result= _transport_error_connect_failed;
			}
		}
		else
		{
			match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 642, process);
			result= _transport_error_connect_failed;
		}
	}
	else
	{
		result= _transport_error_out_of_memory;
	}

	ep->error= (word)result;

	return (short)result;
}

void cancel_connect_process(
	transport_connect_process_ref input)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 664, input && input->ep && input->thread);

	connection_thread_list_maintenance();

	if (take_mutex(input->mutex, CONNECT_MUTEX_TIMEOUT))
	{
		disconnect_endpoint(input->ep);
		input->ep->error= _transport_error_none;
		input->cancelled= TRUE;
		release_mutex(input->mutex);
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 677, !"unable to get mutex in cancel_connect_process()!");
	}
}

short listen_endpoint(
	transport_endpoint_ref ep)
{
	short result= _transport_error_none;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 688, ep);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 689, transport_initialized);

	if (ep->socket!=INVALID_SOCKET)
	{
		if (listen(ep->socket, LISTEN_BACKLOG)==0)
		{
			ep->flags|= (byte)FLAG(_endpoint_listening_bit);
		}
		else
		{
			winsock_error_to_string(WSAGetLastError());
			result= _transport_error_listen_failed;
		}
	}
	else
	{
		result= _transport_error_bad_endpoint;
	}

	ep->error= (word)result;

	return result;
}

transport_endpoint_ref accept_endpoint(
	transport_endpoint_ref listening_endpoint)
{
	transport_endpoint_ref result= NULL;
	SOCKET new_socket;
	struct sockaddr_in sockaddress;
	int sockaddress_size= sizeof(sockaddress);

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 721,
		listening_endpoint && (listening_endpoint->socket >= 0));
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 722, transport_initialized);

	new_socket= accept(listening_endpoint->socket, (struct sockaddr *)&sockaddress, &sockaddress_size);
	if (new_socket!=INVALID_SOCKET)
	{
		result= create_transport_endpoint(listening_endpoint->type);
		if (result)
		{
			result->socket= new_socket;
			result->flags|= (byte)FLAG(_endpoint_connected_bit);
		}
		else
		{
			listening_endpoint->error= _transport_error_out_of_memory;
		}
	}
	else
	{
		winsock_error_to_string(WSAGetLastError());
		listening_endpoint->error= _transport_error_unknown;
	}

	return result;
}

short reject_endpoint(
	transport_endpoint_ref listening_endpoint)
{
	transport_endpoint_ref ep= accept_endpoint(listening_endpoint);

	if (ep)
	{
		delete_transport_endpoint(ep);
	}

	return _transport_error_none;
}

long read_endpoint(
	transport_endpoint_ref ep,
	void *buffer,
	long length)
{
	long result;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 802, ep && buffer && (length > 0));
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 803, transport_initialized);

	result= recv(ep->socket, buffer, length, 0);
	if (result==SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSAEWOULDBLOCK:
			result= _transport_result_operation_would_block;
			break;

		case WSAENETRESET:
		case WSAECONNABORTED:
		case WSAECONNRESET:
		case WSAENOTCONN:
		case WSAESHUTDOWN:
		case WSAETIMEDOUT:
			ep->flags&= (byte)~(FLAG(_endpoint_connected_bit)|FLAG(_endpoint_readable_bit));
			result= _transport_error_connection_lost;
			break;

		default:
			result= _transport_error_endpoint_io;
			ep->flags&= (byte)~FLAG(_endpoint_readable_bit);
			break;
		}

		ep->error= (word)result;
	}
	else if (result==0)
	{
		result= _transport_error_connection_lost;
	}

	return result;
}

long write_endpoint(
	transport_endpoint_ref ep,
	void *buffer,
	long length)
{
	long result;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 848, ep && buffer && (length > 0));
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 849, transport_initialized);

	result= send(ep->socket, buffer, length, 0);
	if (result==SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSAEWOULDBLOCK:
			result= _transport_result_operation_would_block;
			break;

		case WSAENETRESET:
		case WSAECONNABORTED:
		case WSAECONNRESET:
		case WSAENOTCONN:
		case WSAESHUTDOWN:
		case WSAETIMEDOUT:
			ep->flags&= (byte)~FLAG(_endpoint_connected_bit);
			result= _transport_error_connection_lost;
			break;

		default:
			result= _transport_error_endpoint_io;
			break;
		}

		ep->error= (word)result;
	}

	return result;
}

long read_from_endpoint(
	transport_endpoint_ref ep,
	void *buffer,
	long length,
	transport_address *src_addr)
{
	long result;
	struct sockaddr_in sockaddress;
	int sockaddress_size= sizeof(sockaddress);

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 887, ep && buffer && src_addr && (length > 0));
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 888, transport_initialized);

	if (ep->socket==INVALID_SOCKET)
	{
		match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 892, ep->type == _transport_type_udp);

		ep->socket= create_socket(AF_INET, SOCK_DGRAM, 0);
		if (ep->socket!=INVALID_SOCKET)
		{
			transport_address address= {0};
			short err;

			address.address_length= IPV4_ADDRESS_LENGTH;
			err= bind_endpoint(ep, &address);
			match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 900, err == _transport_error_none);
		}
	}

	if (ep->socket!=INVALID_SOCKET)
	{
		match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 907, !endpoint_connected(ep));

		result= recvfrom(ep->socket, buffer, length, 0, (struct sockaddr *)&sockaddress,
			&sockaddress_size);
	}
	else
	{
		ep->error= _transport_error_unknown;
		result= SOCKET_ERROR;
	}

	if (result==SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSAEWOULDBLOCK:
			result= _transport_result_operation_would_block;
			break;

		case WSAENETRESET:
		case WSAECONNABORTED:
		case WSAECONNRESET:
		case WSAENOTCONN:
		case WSAESHUTDOWN:
		case WSAETIMEDOUT:
			ep->flags&= (byte)~(FLAG(_endpoint_connected_bit)|FLAG(_endpoint_readable_bit));
			result= _transport_error_connection_lost;
			break;

		default:
			result= _transport_error_endpoint_io;
			ep->flags&= (byte)~FLAG(_endpoint_readable_bit);
			break;
		}
	}
	else if (result>=0)
	{
		src_addr->address.ipv4_address= SWAP_LONG(sockaddress.sin_addr.S_un.S_addr);
		src_addr->address_length= IPV4_ADDRESS_LENGTH;
		src_addr->port= SWAP_WORD(sockaddress.sin_port);
	}

	return result;
}

long write_to_endpoint(
	transport_endpoint_ref ep,
	void *buffer,
	long length,
	transport_address *dest_addr)
{
	long result;
	struct sockaddr_in sockaddress;
	unsigned long ipv4_address;
	word port;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 957, ep && buffer && (length > 0) && dest_addr);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 958, transport_initialized);

	ipv4_address= dest_addr->address.ipv4_address;
	sockaddress.sin_addr.S_un.S_addr= SWAP_LONG(ipv4_address);
	port= dest_addr->port;
	sockaddress.sin_family= AF_INET;
	sockaddress.sin_port= SWAP_WORD(port);

	if (ep->socket==INVALID_SOCKET)
	{
		match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 966, ep->type == _transport_type_udp);

		ep->socket= create_socket(AF_INET, SOCK_DGRAM, 0);
	}

	if (ep->socket!=INVALID_SOCKET)
	{
		result= sendto(ep->socket, buffer, length, 0, (struct sockaddr *)&sockaddress,
			sizeof(sockaddress));
	}
	else
	{
		ep->error= _transport_error_unknown;
		result= SOCKET_ERROR;
	}

	if (result==SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSAEWOULDBLOCK:
			result= _transport_result_operation_would_block;
			break;

		case WSAENETRESET:
		case WSAECONNABORTED:
		case WSAECONNRESET:
		case WSAENOTCONN:
		case WSAESHUTDOWN:
		case WSAETIMEDOUT:
			ep->flags&= (byte)~FLAG(_endpoint_connected_bit);
			result= _transport_error_connection_lost;
			break;

		default:
			result= _transport_error_endpoint_io;
			break;
		}
	}

	return result;
}

// an endpoint in a set answers out of the flag poll_endpoint_set left behind
boolean endpoint_readable(
	transport_endpoint_ref ep,
	word timeout)
{
	boolean result= FALSE;
	fd_set read_fds;
	struct timeval timeout_value;
	unsigned long flags;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1011, ep);

	if (ep->socket!=INVALID_SOCKET)
	{
		flags= ep->flags;
		if (flags&FLAG(_endpoint_in_set_bit))
		{
			return (boolean)((flags>>_endpoint_readable_bit)&1);
		}

		read_fds.fd_array[0]= ep->socket;
		timeout_value.tv_sec= 0;
		timeout_value.tv_usec= timeout*1000;
		read_fds.fd_count= 1;

		if (select(1, &read_fds, NULL, NULL, &timeout_value)>0 &&
			FD_ISSET(ep->socket, &read_fds))
		{
			return TRUE;
		}

		return FALSE;
	}

	return result;
}

boolean endpoint_writeable(
	transport_endpoint_ref ep,
	word timeout)
{
	fd_set write_fds;
	struct timeval timeout_value;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1047, ep && (ep->socket != INVALID_SOCKET));

	timeout_value.tv_usec= timeout*1000;
	timeout_value.tv_sec= 0;
	write_fds.fd_array[0]= ep->socket;
	write_fds.fd_count= 1;

	if (select(1, NULL, &write_fds, NULL, &timeout_value)>0 &&
		FD_ISSET(ep->socket, &write_fds))
	{
		return TRUE;
	}

	return FALSE;
}

boolean endpoint_connected(
	transport_endpoint_ref ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1062, ep);

	return (boolean)(ep->flags&FLAG(_endpoint_connected_bit));
}

boolean endpoint_listening(
	transport_endpoint_ref ep)
{
	long listening;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1070, ep);

	listening= ep->flags&FLAG(_endpoint_listening_bit);

	return (boolean)(listening>>_endpoint_listening_bit);
}

boolean endpoint_blocking(
	transport_endpoint_ref ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1078, ep);

	return (boolean)(~(ep->flags>>_endpoint_non_blocking_bit)&1);
}

short get_endpoint_error(
	transport_endpoint_ref ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1086, ep);

	return (short)ep->error;
}

long endpoint_equivalent(
	transport_endpoint_ref a,
	transport_endpoint_ref b)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1095, a);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1096, b);

	if (a->socket!=INVALID_SOCKET && a->socket==b->socket)
	{
		return TRUE;
	}

	return FALSE;
}

char *winsock_error_to_string(
	long error_code)
{
	char *string;

	switch (error_code)
	{
	case WSA_INVALID_HANDLE: string= "WSA_INVALID_HANDLE"; break;
	case WSA_NOT_ENOUGH_MEMORY: string= "WSA_NOT_ENOUGH_MEMORY"; break;
	case (long)WSA_INVALID_EVENT: string= "WSA_INVALID_EVENT"; break;
	case WSA_MAXIMUM_WAIT_EVENTS: string= "WSA_MAXIMUM_WAIT_EVENTS"; break;
	case (long)WSA_WAIT_FAILED: string= "WSA_WAIT_FAILED"; break;
	case WSA_INVALID_PARAMETER: string= "WSA_INVALID_PARAMETER"; break;
	case WSA_WAIT_TIMEOUT: string= "WSA_WAIT_TIMEOUT"; break;
	case WSA_WAIT_IO_COMPLETION: string= "WSA_WAIT_IO_COMPLETION"; break;
	case WSA_OPERATION_ABORTED: string= "WSA_OPERATION_ABORTED"; break;
	case WSA_IO_INCOMPLETE: string= "WSA_IO_INCOMPLETE"; break;
	case WSA_IO_PENDING: string= "WSA_IO_PENDING"; break;
	case WSAEINTR: string= "WSAEINTR"; break;
	case WSAEBADF: string= "WSAEBADF"; break;
	case WSAEACCES: string= "WSAEACCES"; break;
	case WSAEFAULT: string= "WSAEFAULT"; break;
	case WSAEINVAL: string= "WSAEINVAL"; break;
	case WSAEMFILE: string= "WSAEMFILE"; break;
	case WSAEWOULDBLOCK: string= "WSAEWOULDBLOCK"; break;
	case WSAEINPROGRESS: string= "WSAEINPROGRESS"; break;
	case WSAEALREADY: string= "WSAEALREADY"; break;
	case WSAENOTSOCK: string= "WSAENOTSOCK"; break;
	case WSAEDESTADDRREQ: string= "WSAEDESTADDRREQ"; break;
	case WSAEMSGSIZE: string= "WSAEMSGSIZE"; break;
	case WSAEPROTOTYPE: string= "WSAEPROTOTYPE"; break;
	case WSAENOPROTOOPT: string= "WSAENOPROTOOPT"; break;
	case WSAEPROTONOSUPPORT: string= "WSAEPROTONOSUPPORT"; break;
	case WSAESOCKTNOSUPPORT: string= "WSAESOCKTNOSUPPORT"; break;
	case WSAEOPNOTSUPP: string= "WSAEOPNOTSUPP"; break;
	case WSAEPFNOSUPPORT: string= "WSAEPFNOSUPPORT"; break;
	case WSAEAFNOSUPPORT: string= "WSAEAFNOSUPPORT"; break;
	case WSAEADDRINUSE: string= "WSAEADDRINUSE"; break;
	case WSAEADDRNOTAVAIL: string= "WSAEADDRNOTAVAIL"; break;
	case WSAENETDOWN: string= "WSAENETDOWN"; break;
	case WSAENETUNREACH: string= "WSAENETUNREACH"; break;
	case WSAENETRESET: string= "WSAENETRESET"; break;
	case WSAECONNABORTED: string= "WSAECONNABORTED"; break;
	case WSAECONNRESET: string= "WSAECONNRESET"; break;
	case WSAENOBUFS: string= "WSAENOBUFS"; break;
	case WSAEISCONN: string= "WSAEISCONN"; break;
	case WSAENOTCONN: string= "WSAENOTCONN"; break;
	case WSAESHUTDOWN: string= "WSAESHUTDOWN"; break;
	case WSAETOOMANYREFS: string= "WSAETOOMANYREFS"; break;
	case WSAETIMEDOUT: string= "WSAETIMEDOUT"; break;
	case WSAECONNREFUSED: string= "WSAECONNREFUSED"; break;
	case WSAELOOP: string= "WSAELOOP"; break;
	case WSAENAMETOOLONG: string= "WSAENAMETOOLONG"; break;
	case WSAEHOSTDOWN: string= "WSAEHOSTDOWN"; break;
	case WSAEHOSTUNREACH: string= "WSAEHOSTUNREACH"; break;
	case WSAENOTEMPTY: string= "WSAENOTEMPTY"; break;
	case WSAEPROCLIM: string= "WSAEPROCLIM"; break;
	case WSAEUSERS: string= "WSAEUSERS"; break;
	case WSAEDQUOT: string= "WSAEDQUOT"; break;
	case WSAESTALE: string= "WSAESTALE"; break;
	case WSAEREMOTE: string= "WSAEREMOTE"; break;
	case WSASYSNOTREADY: string= "WSASYSNOTREADY"; break;
	case WSAVERNOTSUPPORTED: string= "WSAVERNOTSUPPORTED"; break;
	case WSANOTINITIALISED: string= "WSANOTINITIALISED"; break;
	case WSAEDISCON: string= "WSAEDISCON"; break;
	case WSAENOMORE: string= "WSAENOMORE"; break;
	case WSAECANCELLED: string= "WSAECANCELLED"; break;
	case WSAEINVALIDPROCTABLE: string= "WSAEINVALIDPROCTABLE"; break;
	case WSAEINVALIDPROVIDER: string= "WSAEINVALIDPROVIDER"; break;
	case WSAEPROVIDERFAILEDINIT: string= "WSAEPROVIDERFAILEDINIT"; break;
	case WSASYSCALLFAILURE: string= "WSASYSCALLFAILURE"; break;
	case WSASERVICE_NOT_FOUND: string= "WSASERVICE_NOT_FOUND"; break;
	case WSATYPE_NOT_FOUND: string= "WSATYPE_NOT_FOUND"; break;
	case WSA_E_NO_MORE: string= "WSA_E_NO_MORE"; break;
	case WSA_E_CANCELLED: string= "WSA_E_CANCELLED"; break;
	case WSAEREFUSED: string= "WSAEREFUSED"; break;
	case WSAHOST_NOT_FOUND: string= "WSAHOST_NOT_FOUND"; break;
	case WSATRY_AGAIN: string= "WSATRY_AGAIN"; break;
	case WSANO_RECOVERY: string= "WSANO_RECOVERY"; break;
	case WSANO_DATA: string= "WSANO_DATA"; break;
	case WSA_QOS_RECEIVERS: string= "WSA_QOS_RECEIVERS"; break;
	case WSA_QOS_SENDERS: string= "WSA_QOS_SENDERS"; break;
	case WSA_QOS_NO_SENDERS: string= "WSA_QOS_NO_SENDERS"; break;
	case WSA_QOS_NO_RECEIVERS: string= "WSA_QOS_NO_RECEIVERS"; break;
	case WSA_QOS_REQUEST_CONFIRMED: string= "WSA_QOS_REQUEST_CONFIRMED"; break;
	case WSA_QOS_ADMISSION_FAILURE: string= "WSA_QOS_ADMISSION_FAILURE"; break;
	case WSA_QOS_POLICY_FAILURE: string= "WSA_QOS_POLICY_FAILURE"; break;
	case WSA_QOS_BAD_STYLE: string= "WSA_QOS_BAD_STYLE"; break;
	case WSA_QOS_BAD_OBJECT: string= "WSA_QOS_BAD_OBJECT"; break;
	case WSA_QOS_TRAFFIC_CTRL_ERROR: string= "WSA_QOS_TRAFFIC_CTRL_ERROR"; break;
	case WSA_QOS_GENERIC_ERROR: string= "WSA_QOS_GENERIC_ERROR"; break;
	default: string= "<unknown error>"; break;
	}

	transport_endpoint_globals.last_error_string= string;

	if (error_code!=transport_endpoint_globals.last_error)
	{
		error(_error_log, "winsock error #%d: %s", error_code, transport_endpoint_globals.last_error_string);
		transport_endpoint_globals.last_error= error_code;
	}

	return transport_endpoint_globals.last_error_string;
}

/* ---------- private code */

static SOCKET create_socket(
	long af,
	long type,
	long protocol)
{
	SOCKET result;
	long option;
	int option_size;

	result= socket(af, type, protocol);
	if (result!=INVALID_SOCKET)
	{
		if (type==SOCK_DGRAM)
		{
			option= -1;
			if (setsockopt(result, SOL_SOCKET, SO_BROADCAST, (char *)&option, sizeof(option))!=0)
			{
				winsock_error_to_string(WSAGetLastError());
			}
		}

		option= 1;
		if (setsockopt(result, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option))!=0)
		{
			winsock_error_to_string(WSAGetLastError());
		}

		option_size= sizeof(option);
		if (getsockopt(result, SOL_SOCKET, SO_SNDBUF, (char *)&option, &option_size)!=0)
		{
			winsock_error_to_string(WSAGetLastError());
		}
		else if (option<SOCKET_BUFFER_MINIMUM_SIZE)
		{
			option= SOCKET_BUFFER_MINIMUM_SIZE;
			if (setsockopt(result, SOL_SOCKET, SO_SNDBUF, (char *)&option, sizeof(option))!=0)
			{
				winsock_error_to_string(WSAGetLastError());
			}
		}

		option_size= sizeof(option);
		if (getsockopt(result, SOL_SOCKET, SO_RCVBUF, (char *)&option, &option_size)!=0)
		{
			winsock_error_to_string(WSAGetLastError());
		}
		else if (option<SOCKET_BUFFER_MINIMUM_SIZE)
		{
			option= SOCKET_BUFFER_MINIMUM_SIZE;
			if (setsockopt(result, SOL_SOCKET, SO_RCVBUF, (char *)&option, sizeof(option))!=0)
			{
				winsock_error_to_string(WSAGetLastError());
			}
		}
	}
	else
	{
		winsock_error_to_string(WSAGetLastError());
	}

	return result;
}
