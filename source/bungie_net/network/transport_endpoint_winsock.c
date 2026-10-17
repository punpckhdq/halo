/*
TRANSPORT_ENDPOINT_WINSOCK.C
*/

/* ---------- headers */

#include "cseries.h"
#include "transport_endpoint_winsock.h"
#include "byte_swapping.h"
#include "bungie_net/common/thread.h"
#include "transport.h"

/* ---------- constants */

enum
{
	MAXIMUM_CONNECT_THREADS = 64, /* fake name */
	MINIMUM_SOCKET_BUFFER_SIZE = 0x4000, /* fake name */
	CONNECT_TIMEOUT = 10*MILLISECONDS_PER_SECOND, /* fake name */
	CONNECT_SELECT_TIMEOUT_SECONDS = 1, /* fake name */
	CONNECT_MUTEX_TIMEOUT = MILLISECONDS_PER_SECOND, /* fake name */
	LISTEN_BACKLOG = 32, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct transport_connect_process /* fake name */
{
	struct transport_endpoint *ep;
	struct transport_address address;
	struct thread *thread;
	struct mutex *mutex;
	boolean cancelled; /* fake name */
};

struct connect_thread_entry /* fake name */
{
	struct thread *thread;
	boolean dispose; /* fake name */
};

struct _transport_endpoint_globals /* fake name */
{
	char const *error_string; /* fake name */
	long unknown; /* fake name */
	struct connect_thread_entry threads[MAXIMUM_CONNECT_THREADS]; /* fake name */
	long last_error; /* fake name */
};

/* ---------- prototypes */

static boolean add_connect_thread(struct thread *thread);
static void mark_connection_thread_as_terminated(struct thread *thread);
static void connection_thread_list_maintenance(void);
static SOCKET create_socket(long family, long type, long protocol);
static DWORD WINAPI connect_async_thread_proc(void *input_data);

/* ---------- globals */

static struct _transport_endpoint_globals transport_endpoint_globals; /* fake name */

/* ---------- public code */

struct transport_endpoint *create_transport_endpoint(
	long type)
{
	struct transport_endpoint *ep = NULL;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 206, transport_initialized);
	connection_thread_list_maintenance();

	if (type == _transport_type_udp || type == _transport_type_tcp)
	{
		ep = (struct transport_endpoint *)match_malloc("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 212, sizeof(struct transport_endpoint));

		if (ep)
		{
			ep->error = _transport_error_none;
			ep->type = type;
			ep->socket = INVALID_SOCKET;
			ep->flags = 0;
		}
	}

	return ep;
}

void delete_transport_endpoint(
	struct transport_endpoint *ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 228, ep);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 229, transport_initialized);

	disconnect_endpoint(ep);
	match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 232, ep);
	connection_thread_list_maintenance();

	return;
}

short get_endpoint_address(
	struct transport_endpoint *ep,
	struct transport_address *address)
{
	struct sockaddr_in socket_address;
	short result = _transport_error_none;
	long address_size = sizeof(struct sockaddr_in);

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 247, ep && address);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 248, transport_initialized);

	if (ep->socket != INVALID_SOCKET)
	{
		if (!getpeername(ep->socket, (struct sockaddr *)&socket_address, &address_size))
		{
			if (socket_address.sin_family == AF_INET)
			{
				address->address.ipv4_address = SWAP4(socket_address.sin_addr.s_addr);
				address->address_length = IPV4_ADDRESS_LENGTH;
				address->port = SWAP2(socket_address.sin_port);
			}
			else
			{
				winsock_error_to_string(WSAGetLastError());
				result = _transport_error_address_unknown;
			}
		}
		else
		{
			if (!getsockname(ep->socket, (struct sockaddr *)&socket_address, &address_size) && socket_address.sin_family == AF_INET)
			{
				address->address.ipv4_address = SWAP4(socket_address.sin_addr.s_addr);
				address->address_length = IPV4_ADDRESS_LENGTH;
				address->port = SWAP2(socket_address.sin_port);
			}
			else
			{
				winsock_error_to_string(WSAGetLastError());
				result = _transport_error_address_unknown;
			}
		}
	}
	else
	{
		result = _transport_error_address_unknown;
	}

	ep->error = result;

	return result;
}

long get_endpoint_type(
	struct transport_endpoint const *ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 300, ep);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 301, transport_initialized);

	return ep->type;
}

short set_endpoint_blocking(
	struct transport_endpoint *ep,
	long blocking)
{
	boolean current_blocking;
	short result = _transport_error_none;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 313, ep);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 314, transport_initialized);

	current_blocking = endpoint_blocking(ep);

	if (!current_blocking)
	{
		if (blocking)
		{
			unsigned long nonblocking = FALSE;

			result = ioctlsocket(ep->socket, FIONBIO, &nonblocking);

			if (!result)
			{
				SET_FLAG(ep->flags, _endpoint_nonblocking_bit, FALSE);
			}
			else
			{
				winsock_error_to_string(WSAGetLastError());
				result = _transport_error_options_failed;
			}
		}
	}
	else if (!blocking)
	{
		unsigned long nonblocking = TRUE;

		result = ioctlsocket(ep->socket, FIONBIO, &nonblocking);

		if (!result)
		{
			SET_FLAG(ep->flags, _endpoint_nonblocking_bit, TRUE);
		}
		else
		{
			winsock_error_to_string(WSAGetLastError());
			result = _transport_error_options_failed;
		}
	}

	ep->error = result;

	return result;
}

short bind_endpoint(
	struct transport_endpoint *ep,
	struct transport_address const *address)
{
	struct sockaddr_in socket_address;
	short result = _transport_error_none;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 364, ep && address);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 365, transport_initialized);

	if (ep->socket == INVALID_SOCKET)
	{
		long socket_type = 0;

		if (ep->type == _transport_type_tcp)
		{
			socket_type = SOCK_STREAM;
		}
		else if (ep->type == _transport_type_udp)
		{
			socket_type = SOCK_DGRAM;
		}
		else
		{
			result = _transport_error_bad_endpoint;
		}

		if (socket_type)
		{
			ep->socket = create_socket(AF_INET, socket_type, 0);

			if (ep->socket == INVALID_SOCKET)
			{
				result = _transport_error_unknown;
			}
		}
	}

	if (ep->socket != INVALID_SOCKET && result == _transport_error_none)
	{
		socket_address.sin_addr.s_addr = SWAP4(address->address.ipv4_address);
		socket_address.sin_family = AF_INET;
		socket_address.sin_port = SWAP2(address->port);

		if (bind(ep->socket, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_in)))
		{
			winsock_error_to_string(WSAGetLastError());
			result = _transport_error_bind_endpoint;
		}
	}
	else
	{
		result = _transport_error_unknown;
	}

	ep->error = result;

	return result;
}

short connect_endpoint(
	struct transport_endpoint *ep,
	struct transport_address const *address)
{
	long socket_type;
	struct sockaddr_in socket_address;
	boolean blocking;
	short result = _transport_error_none;
	long winsock_error = 0;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 437, ep && address);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 438, transport_initialized);

	if (ep->type == _transport_type_udp)
	{
		socket_type = SOCK_DGRAM;
	}
	else if (ep->type == _transport_type_tcp)
	{
		socket_type = SOCK_STREAM;
	}
	else
	{
		result = _transport_error_bad_endpoint;
	}

	if (result == _transport_error_none)
	{
		if (ep->socket == INVALID_SOCKET)
		{
			ep->socket = create_socket(AF_INET, socket_type, 0);
		}

		socket_address.sin_addr.s_addr = SWAP4(address->address.ipv4_address);
		socket_address.sin_family = AF_INET;
		socket_address.sin_port = SWAP2(address->port);
		blocking = endpoint_blocking(ep);
		set_endpoint_blocking(ep, FALSE);

		if (connect(ep->socket, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_in)))
		{
			winsock_error = WSAGetLastError();

			if (winsock_error == WSAEWOULDBLOCK)
			{
				struct timeval timeout;
				fd_set writeable;
				unsigned long deadline = system_milliseconds() + CONNECT_TIMEOUT;

				timeout.tv_sec = CONNECT_SELECT_TIMEOUT_SECONDS;
				timeout.tv_usec = 0;

				do
				{
					writeable.fd_array[0] = ep->socket;
					writeable.fd_count = 1;
					winsock_error = select(1, NULL, &writeable, NULL, &timeout) == 1 ? 0 : WSAGetLastError();

					if (system_milliseconds() > deadline)
					{
						winsock_error = WSAEINPROGRESS;
						closesocket(ep->socket);
						break;
					}
				}
				while (winsock_error == WSAEINPROGRESS);
			}
		}

		if (winsock_error)
		{
			winsock_error_to_string(winsock_error);
			result = _transport_error_connect_failed;
		}
		else
		{
			set_endpoint_blocking(ep, blocking);
			SET_FLAG(ep->flags, _endpoint_nonblocking_bit, FALSE);
			SET_FLAG(ep->flags, _endpoint_connected_bit, TRUE);
			SET_FLAG(ep->flags, _endpoint_client_bit, TRUE);
			result = _transport_error_none;
		}
	}

	ep->error = result;

	return result;
}

void disconnect_endpoint(
	struct transport_endpoint *ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 545, ep);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 546, transport_initialized);

	if (ep->socket != INVALID_SOCKET)
	{
		if (closesocket(ep->socket))
		{
			winsock_error_to_string(WSAGetLastError());
		}

		ep->socket = INVALID_SOCKET;
	}

	SET_FLAG(ep->flags, _endpoint_connected_bit, FALSE);

	return;
}

short connect_endpoint_async(
	struct transport_endpoint *ep,
	struct transport_address const *address,
	struct transport_connect_process **process_ref_ptr)
{
	struct transport_connect_process *input;
	short result = _transport_error_none;

	connection_thread_list_maintenance();
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 616, ep && address && process_ref_ptr);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 617, transport_initialized);

	input = (struct transport_connect_process *)match_calloc("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 619, 1, sizeof(struct transport_connect_process));

	if (input)
	{
		input->address = *address;
		input->ep = ep;
		input->cancelled = FALSE;

		if (create_mutex(&input->mutex) && create_thread(_thread_attribute_flag_priority_high, connect_async_thread_proc, input, &input->thread))
		{
			if (add_connect_thread(input->thread))
			{
				result = _transport_result_connect_in_progress;
				*process_ref_ptr = input;
			}
			else
			{
				dispose_thread(input->thread);
				dispose_mutex(input->mutex);
				input->thread = NULL;
				result = _transport_error_unknown;
			}
		}
		else
		{
			match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 642, input);
			result = _transport_error_connect_failed;
		}
	}
	else
	{
		result = _transport_error_out_of_memory;
	}

	ep->error = result;

	return result;
}

void cancel_connect_process(
	struct transport_connect_process *input)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 664, input && input->ep && input->thread);
	connection_thread_list_maintenance();

	if (take_mutex(input->mutex, CONNECT_MUTEX_TIMEOUT))
	{
		disconnect_endpoint(input->ep);
		input->ep->error = _transport_error_none;
		input->cancelled = TRUE;
		release_mutex(input->mutex);
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 677, !"unable to get mutex in cancel_connect_process()!");
	}

	return;
}

short listen_endpoint(
	struct transport_endpoint *ep)
{
	short result = _transport_error_none;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 688, ep);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 689, transport_initialized);

	if (ep->socket != INVALID_SOCKET)
	{
		if (!listen(ep->socket, LISTEN_BACKLOG))
		{
			SET_FLAG(ep->flags, _endpoint_listening_bit, TRUE);
		}
		else
		{
			winsock_error_to_string(WSAGetLastError());
			result = _transport_error_listen_failed;
		}
	}
	else
	{
		result = _transport_error_bad_endpoint;
	}

	ep->error = result;

	return result;
}

struct transport_endpoint *accept_endpoint(
	struct transport_endpoint *listening_endpoint)
{
	SOCKET accepted;
	struct sockaddr_in address;
	struct transport_endpoint *ep = NULL;
	long address_size = sizeof(struct sockaddr_in);

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 721, listening_endpoint && (listening_endpoint->socket >= 0));
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 722, transport_initialized);

	accepted = accept(listening_endpoint->socket, (struct sockaddr *)&address, &address_size);

	if (accepted != INVALID_SOCKET)
	{
		ep = create_transport_endpoint(listening_endpoint->type);

		if (ep)
		{
			ep->socket = accepted;
			SET_FLAG(ep->flags, _endpoint_connected_bit, TRUE);
		}
		else
		{
			listening_endpoint->error = _transport_error_out_of_memory;
		}
	}
	else
	{
		winsock_error_to_string(WSAGetLastError());
		listening_endpoint->error = _transport_error_unknown;
	}

	return ep;
}

short reject_endpoint(
	struct transport_endpoint *ep)
{
	struct transport_endpoint *accepted = accept_endpoint(ep);

	if (accepted)
	{
		delete_transport_endpoint(accepted);
	}

	return _transport_error_none;
}

long read_endpoint(
	struct transport_endpoint *ep,
	void *buffer,
	long length)
{
	long result;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 802, ep && buffer && (length > 0));
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 803, transport_initialized);

	result = recv(ep->socket, buffer, length, 0);

	if (result == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSAEWOULDBLOCK:
			result = _transport_result_operation_would_block;
			break;
		case WSAENETRESET:
		case WSAECONNABORTED:
		case WSAECONNRESET:
		case WSAENOTCONN:
		case WSAESHUTDOWN:
		case WSAETIMEDOUT:
			SET_FLAG(ep->flags, _endpoint_connected_bit, FALSE);
			SET_FLAG(ep->flags, _endpoint_readable_bit, FALSE);
			result = _transport_error_connection_lost;
			break;
		default:
			result = _transport_error_endpoint_io;
			SET_FLAG(ep->flags, _endpoint_readable_bit, FALSE);
			break;
		}

		ep->error = result;
	}
	else if (!result)
	{
		result = _transport_error_connection_lost;
	}

	return result;
}

long write_endpoint(
	struct transport_endpoint *ep,
	void const *buffer,
	long length)
{
	long result;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 848, ep && buffer && (length > 0));
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 849, transport_initialized);

	result = send(ep->socket, buffer, length, 0);

	if (result == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSAEWOULDBLOCK:
			result = _transport_result_operation_would_block;
			break;
		case WSAENETRESET:
		case WSAECONNABORTED:
		case WSAECONNRESET:
		case WSAENOTCONN:
		case WSAESHUTDOWN:
		case WSAETIMEDOUT:
			SET_FLAG(ep->flags, _endpoint_connected_bit, FALSE);
			result = _transport_error_connection_lost;
			break;
		default:
			result = _transport_error_endpoint_io;
			break;
		}

		ep->error = result;
	}

	return result;
}

long read_from_endpoint(
	struct transport_endpoint *ep,
	void *buffer,
	long length,
	struct transport_address *src_addr)
{
	struct sockaddr_in address;
	long result = SOCKET_ERROR;
	long address_size = sizeof(struct sockaddr_in);

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 887, ep && buffer && src_addr && (length > 0));
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 888, transport_initialized);

	if (ep->socket == INVALID_SOCKET)
	{
		match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 892, ep->type == _transport_type_udp);
		ep->socket = create_socket(AF_INET, SOCK_DGRAM, 0);

		if (ep->socket != INVALID_SOCKET)
		{
			short err;
			struct transport_address bind_address = {0};

			bind_address.address_length = IPV4_ADDRESS_LENGTH;
			err = bind_endpoint(ep, &bind_address);
			match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 900, err == _transport_error_none);
		}
	}

	if (ep->socket != INVALID_SOCKET)
	{
		match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 907, !endpoint_connected(ep));
		result = recvfrom(ep->socket, buffer, length, 0, (struct sockaddr *)&address, &address_size);
	}
	else
	{
		ep->error = _transport_error_unknown;
	}

	if (result == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSAEWOULDBLOCK:
			result = _transport_result_operation_would_block;
			break;
		case WSAENETRESET:
		case WSAECONNABORTED:
		case WSAECONNRESET:
		case WSAENOTCONN:
		case WSAESHUTDOWN:
		case WSAETIMEDOUT:
			SET_FLAG(ep->flags, _endpoint_connected_bit, FALSE);
			SET_FLAG(ep->flags, _endpoint_readable_bit, FALSE);
			result = _transport_error_connection_lost;
			break;
		default:
			result = _transport_error_endpoint_io;
			SET_FLAG(ep->flags, _endpoint_readable_bit, FALSE);
			break;
		}
	}
	else if (result >= 0)
	{
		src_addr->address.ipv4_address = SWAP4(address.sin_addr.s_addr);
		src_addr->address_length = IPV4_ADDRESS_LENGTH;
		src_addr->port = SWAP2(address.sin_port);
	}

	return result;
}

long write_to_endpoint(
	struct transport_endpoint *ep,
	void const *buffer,
	long length,
	struct transport_address const *dest_addr)
{
	struct sockaddr_in address;
	long result = SOCKET_ERROR;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 957, ep && buffer && (length > 0) && dest_addr);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 958, transport_initialized);

	address.sin_addr.s_addr = SWAP4(dest_addr->address.ipv4_address);
	address.sin_family = AF_INET;
	address.sin_port = SWAP2(dest_addr->port);

	if (ep->socket == INVALID_SOCKET)
	{
		match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 966, ep->type == _transport_type_udp);
		ep->socket = create_socket(AF_INET, SOCK_DGRAM, 0);
	}

	if (ep->socket != INVALID_SOCKET)
	{
		result = sendto(ep->socket, buffer, length, 0, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
	}
	else
	{
		ep->error = _transport_error_unknown;
	}

	if (result == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSAEWOULDBLOCK:
			result = _transport_result_operation_would_block;
			break;
		case WSAENETRESET:
		case WSAECONNABORTED:
		case WSAECONNRESET:
		case WSAENOTCONN:
		case WSAESHUTDOWN:
		case WSAETIMEDOUT:
			SET_FLAG(ep->flags, _endpoint_connected_bit, FALSE);
			result = _transport_error_connection_lost;
			break;
		default:
			result = _transport_error_endpoint_io;
			break;
		}
	}

	return result;
}

boolean endpoint_readable(
	struct transport_endpoint *ep,
	word timeout)
{
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1011, ep);

	if (ep->socket != INVALID_SOCKET)
	{
		if (TEST_FLAG(ep->flags, _endpoint_in_set_bit))
		{
			result = TEST_FLAG(ep->flags, _endpoint_readable_bit);
		}
		else
		{
			fd_set readable;
			struct timeval wait;

			readable.fd_array[0] = ep->socket;
			wait.tv_sec = 0;
			wait.tv_usec = timeout * MICROSECONDS_PER_MILLISECOND;
			readable.fd_count = 1;
			result = select(1, &readable, NULL, NULL, &wait) > 0 && FD_ISSET(ep->socket, &readable);
		}
	}

	return result;
}

boolean endpoint_writeable(
	struct transport_endpoint *ep,
	word timeout)
{
	boolean result;
	fd_set writeable;
	struct timeval wait;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1047, ep && (ep->socket != INVALID_SOCKET));

	wait.tv_usec = timeout * MICROSECONDS_PER_MILLISECOND;
	wait.tv_sec = 0;
	writeable.fd_array[0] = ep->socket;
	writeable.fd_count = 1;

	if (select(1, NULL, &writeable, NULL, &wait) > 0 && FD_ISSET(ep->socket, &writeable))
	{
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}

	return result;
}

boolean endpoint_connected(
	struct transport_endpoint const *ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1062, ep);

	return TEST_FLAG(ep->flags, _endpoint_connected_bit);
}

long endpoint_listening(
	struct transport_endpoint const *ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1070, ep);

	return TEST_FLAG(ep->flags, _endpoint_listening_bit) ? TRUE : FALSE;
}

long endpoint_blocking(
	struct transport_endpoint const *ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1078, ep);

	return !TEST_FLAG(ep->flags, _endpoint_nonblocking_bit);
}

short get_endpoint_error(
	struct transport_endpoint const *ep)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1086, ep);

	return ep->error;
}

long endpoint_equivalent(
	struct transport_endpoint const *a,
	struct transport_endpoint const *b)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1095, a);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 1096, b);

	return a->socket != INVALID_SOCKET && a->socket == b->socket;
}

char const *winsock_error_to_string(
	long error_number)
{
	char const *result;

	switch (error_number)
	{
	case ERROR_INVALID_HANDLE:
		result = "WSA_INVALID_HANDLE";
		break;
	case ERROR_NOT_ENOUGH_MEMORY:
		result = "WSA_NOT_ENOUGH_MEMORY";
		break;
	case WSA_INVALID_EVENT:
		result = "WSA_INVALID_EVENT";
		break;
	case WSA_MAXIMUM_WAIT_EVENTS:
		result = "WSA_MAXIMUM_WAIT_EVENTS";
		break;
	case WSA_WAIT_FAILED:
		result = "WSA_WAIT_FAILED";
		break;
	case ERROR_INVALID_PARAMETER:
		result = "WSA_INVALID_PARAMETER";
		break;
	case WAIT_IO_COMPLETION:
		result = "WSA_WAIT_IO_COMPLETION";
		break;
	case WSA_WAIT_TIMEOUT:
		result = "WSA_WAIT_TIMEOUT";
		break;
	case ERROR_OPERATION_ABORTED:
		result = "WSA_OPERATION_ABORTED";
		break;
	case ERROR_IO_INCOMPLETE:
		result = "WSA_IO_INCOMPLETE";
		break;
	case ERROR_IO_PENDING:
		result = "WSA_IO_PENDING";
		break;
	case WSAEINTR:
		result = "WSAEINTR";
		break;
	case WSAEBADF:
		result = "WSAEBADF";
		break;
	case WSAEACCES:
		result = "WSAEACCES";
		break;
	case WSAEFAULT:
		result = "WSAEFAULT";
		break;
	case WSAEINVAL:
		result = "WSAEINVAL";
		break;
	case WSAEMFILE:
		result = "WSAEMFILE";
		break;
	case WSAEWOULDBLOCK:
		result = "WSAEWOULDBLOCK";
		break;
	case WSAEINPROGRESS:
		result = "WSAEINPROGRESS";
		break;
	case WSAEALREADY:
		result = "WSAEALREADY";
		break;
	case WSAENOTSOCK:
		result = "WSAENOTSOCK";
		break;
	case WSAEDESTADDRREQ:
		result = "WSAEDESTADDRREQ";
		break;
	case WSAEMSGSIZE:
		result = "WSAEMSGSIZE";
		break;
	case WSAEPROTOTYPE:
		result = "WSAEPROTOTYPE";
		break;
	case WSAENOPROTOOPT:
		result = "WSAENOPROTOOPT";
		break;
	case WSAEPROTONOSUPPORT:
		result = "WSAEPROTONOSUPPORT";
		break;
	case WSAESOCKTNOSUPPORT:
		result = "WSAESOCKTNOSUPPORT";
		break;
	case WSAEOPNOTSUPP:
		result = "WSAEOPNOTSUPP";
		break;
	case WSAEPFNOSUPPORT:
		result = "WSAEPFNOSUPPORT";
		break;
	case WSAEAFNOSUPPORT:
		result = "WSAEAFNOSUPPORT";
		break;
	case WSAEADDRINUSE:
		result = "WSAEADDRINUSE";
		break;
	case WSAEADDRNOTAVAIL:
		result = "WSAEADDRNOTAVAIL";
		break;
	case WSAENETDOWN:
		result = "WSAENETDOWN";
		break;
	case WSAENETUNREACH:
		result = "WSAENETUNREACH";
		break;
	case WSAENETRESET:
		result = "WSAENETRESET";
		break;
	case WSAECONNABORTED:
		result = "WSAECONNABORTED";
		break;
	case WSAECONNRESET:
		result = "WSAECONNRESET";
		break;
	case WSAENOBUFS:
		result = "WSAENOBUFS";
		break;
	case WSAEISCONN:
		result = "WSAEISCONN";
		break;
	case WSAENOTCONN:
		result = "WSAENOTCONN";
		break;
	case WSAESHUTDOWN:
		result = "WSAESHUTDOWN";
		break;
	case WSAETOOMANYREFS:
		result = "WSAETOOMANYREFS";
		break;
	case WSAETIMEDOUT:
		result = "WSAETIMEDOUT";
		break;
	case WSAECONNREFUSED:
		result = "WSAECONNREFUSED";
		break;
	case WSAELOOP:
		result = "WSAELOOP";
		break;
	case WSAENAMETOOLONG:
		result = "WSAENAMETOOLONG";
		break;
	case WSAEHOSTDOWN:
		result = "WSAEHOSTDOWN";
		break;
	case WSAEHOSTUNREACH:
		result = "WSAEHOSTUNREACH";
		break;
	case WSAENOTEMPTY:
		result = "WSAENOTEMPTY";
		break;
	case WSAEPROCLIM:
		result = "WSAEPROCLIM";
		break;
	case WSAEUSERS:
		result = "WSAEUSERS";
		break;
	case WSAEDQUOT:
		result = "WSAEDQUOT";
		break;
	case WSAESTALE:
		result = "WSAESTALE";
		break;
	case WSAEREMOTE:
		result = "WSAEREMOTE";
		break;
	case WSASYSNOTREADY:
		result = "WSASYSNOTREADY";
		break;
	case WSAVERNOTSUPPORTED:
		result = "WSAVERNOTSUPPORTED";
		break;
	case WSANOTINITIALISED:
		result = "WSANOTINITIALISED";
		break;
	case WSAEDISCON:
		result = "WSAEDISCON";
		break;
	case WSAENOMORE:
		result = "WSAENOMORE";
		break;
	case WSAECANCELLED:
		result = "WSAECANCELLED";
		break;
	case WSAEINVALIDPROCTABLE:
		result = "WSAEINVALIDPROCTABLE";
		break;
	case WSAEINVALIDPROVIDER:
		result = "WSAEINVALIDPROVIDER";
		break;
	case WSAEPROVIDERFAILEDINIT:
		result = "WSAEPROVIDERFAILEDINIT";
		break;
	case WSASYSCALLFAILURE:
		result = "WSASYSCALLFAILURE";
		break;
	case WSASERVICE_NOT_FOUND:
		result = "WSASERVICE_NOT_FOUND";
		break;
	case WSATYPE_NOT_FOUND:
		result = "WSATYPE_NOT_FOUND";
		break;
	case WSA_E_NO_MORE:
		result = "WSA_E_NO_MORE";
		break;
	case WSA_E_CANCELLED:
		result = "WSA_E_CANCELLED";
		break;
	case WSAEREFUSED:
		result = "WSAEREFUSED";
		break;
	case WSAHOST_NOT_FOUND:
		result = "WSAHOST_NOT_FOUND";
		break;
	case WSATRY_AGAIN:
		result = "WSATRY_AGAIN";
		break;
	case WSANO_RECOVERY:
		result = "WSANO_RECOVERY";
		break;
	case WSANO_DATA:
		result = "WSANO_DATA";
		break;
	case WSA_QOS_RECEIVERS:
		result = "WSA_QOS_RECEIVERS";
		break;
	case WSA_QOS_SENDERS:
		result = "WSA_QOS_SENDERS";
		break;
	case WSA_QOS_NO_SENDERS:
		result = "WSA_QOS_NO_SENDERS";
		break;
	case WSA_QOS_NO_RECEIVERS:
		result = "WSA_QOS_NO_RECEIVERS";
		break;
	case WSA_QOS_REQUEST_CONFIRMED:
		result = "WSA_QOS_REQUEST_CONFIRMED";
		break;
	case WSA_QOS_ADMISSION_FAILURE:
		result = "WSA_QOS_ADMISSION_FAILURE";
		break;
	case WSA_QOS_POLICY_FAILURE:
		result = "WSA_QOS_POLICY_FAILURE";
		break;
	case WSA_QOS_BAD_STYLE:
		result = "WSA_QOS_BAD_STYLE";
		break;
	case WSA_QOS_BAD_OBJECT:
		result = "WSA_QOS_BAD_OBJECT";
		break;
	case WSA_QOS_TRAFFIC_CTRL_ERROR:
		result = "WSA_QOS_TRAFFIC_CTRL_ERROR";
		break;
	case WSA_QOS_GENERIC_ERROR:
		result = "WSA_QOS_GENERIC_ERROR";
		break;
	default:
		result = "<unknown error>";
		break;
	}

	transport_endpoint_globals.error_string = result;

	if (error_number != transport_endpoint_globals.last_error)
	{
		error(_error_log, "winsock error #%d: %s", error_number, result);
		transport_endpoint_globals.last_error = error_number;
	}

	return transport_endpoint_globals.error_string;
}

/* ---------- private code */

static boolean add_connect_thread(
	struct thread *thread)
{
	long i = 0;
	boolean occupied = transport_endpoint_globals.threads[i].thread != NULL;

	while (occupied && i < MAXIMUM_CONNECT_THREADS)
	{
		occupied = transport_endpoint_globals.threads[++i].thread != NULL;
	}

	if (i < MAXIMUM_CONNECT_THREADS)
	{
		transport_endpoint_globals.threads[i].thread = thread;
		transport_endpoint_globals.threads[i].dispose = FALSE;
	}
	else
	{
		i = NONE;
	}

	return i != NONE;
}

static void mark_connection_thread_as_terminated(
	struct thread *thread)
{
	long i;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 79, thread);

	for (i = 0; i < MAXIMUM_CONNECT_THREADS; i++)
	{
		if (transport_endpoint_globals.threads[i].thread == thread)
		{
			transport_endpoint_globals.threads[i].dispose = TRUE;
			break;
		}
	}

	return;
}

static void connection_thread_list_maintenance(
	void)
{
	long i;

	for (i = 0; i < MAXIMUM_CONNECT_THREADS; i++)
	{
		if (transport_endpoint_globals.threads[i].thread && transport_endpoint_globals.threads[i].dispose)
		{
			dispose_thread(transport_endpoint_globals.threads[i].thread);
			transport_endpoint_globals.threads[i].thread = NULL;
			transport_endpoint_globals.threads[i].dispose = FALSE;
		}
	}

	return;
}

static SOCKET create_socket(
	long family,
	long type,
	long protocol)
{
	long option;
	long option_size;
	SOCKET result = socket(family, type, protocol);

	if (result != INVALID_SOCKET)
	{
		if (type == SOCK_DGRAM)
		{
			option = -1;

			if (setsockopt(result, SOL_SOCKET, SO_BROADCAST, (char const *)&option, sizeof(long)))
			{
				winsock_error_to_string(WSAGetLastError());
			}
		}

		option = 1;

		if (setsockopt(result, SOL_SOCKET, SO_REUSEADDR, (char const *)&option, sizeof(long)))
		{
			winsock_error_to_string(WSAGetLastError());
		}

		option_size = sizeof(long);

		if (getsockopt(result, SOL_SOCKET, SO_SNDBUF, (char *)&option, &option_size) ||
			(option < MINIMUM_SOCKET_BUFFER_SIZE && (option = MINIMUM_SOCKET_BUFFER_SIZE, setsockopt(result, SOL_SOCKET, SO_SNDBUF, (char const *)&option, sizeof(long)))))
		{
			winsock_error_to_string(WSAGetLastError());
		}

		option_size = sizeof(long);

		if (getsockopt(result, SOL_SOCKET, SO_RCVBUF, (char *)&option, &option_size) ||
			(option < MINIMUM_SOCKET_BUFFER_SIZE && (option = MINIMUM_SOCKET_BUFFER_SIZE, setsockopt(result, SOL_SOCKET, SO_RCVBUF, (char const *)&option, sizeof(long)))))
		{
			winsock_error_to_string(WSAGetLastError());
		}
	}
	else
	{
		winsock_error_to_string(WSAGetLastError());
	}

	return result;
}

static DWORD WINAPI connect_async_thread_proc(
	void *input_data)
{
	struct thread *thread;
	short result;
	struct transport_connect_process *input = (struct transport_connect_process *)input_data;
	struct mutex *mutex = NULL;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 569, input);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 570, input->ep);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 571, input->thread);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_winsock.c", 572, transport_initialized);

	result = connect_endpoint(input->ep, &input->address);

	if (take_mutex(input->mutex, CONNECT_MUTEX_TIMEOUT))
	{
		if (input->cancelled)
		{
			disconnect_endpoint(input->ep);
		}

		mutex = input->mutex;
		thread = input->thread;
	}
	else
	{
		result = _transport_error_unknown;
	}

	input->ep->error = result;

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
