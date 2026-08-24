/*
TRANSPORT_ENDPOINT_SET_WINSOCK.C
*/

/* ---------- headers */

#include "cseries.h"
#include "errors.h"
#include "cseries_windows.h"
#include "transport.h"
#include "transport_endpoint_winsock.h"

/* ---------- constants */

enum
{
	TRANSPORT_XNADDR_TIMEOUT= 10000
};

/* ---------- macros */

/* ---------- prototypes */

static long get_next_available_set_array_index(struct endpoint_set *set);
static int poll_ep_array_compare_proc(const void *a, const void *b);

/* ---------- globals */

boolean transport_initialized;
boolean global_client_active;
long global_key_depth;

extern XNKEY global_key;
extern XNKID global_key_id;
extern byte global_nonce[8];
extern XNADDR global_address;
extern boolean server_transport_globals;

/* ---------- public code */

void net_startup_debug(
	void)
{

	return;
}

void transport_push_key(
	XNKEY *key,
	XNKID *key_id)
{
	long error;

	global_key= *key;
	global_key_id= *key_id;

	if (global_key_depth==0)
	{
		error= XNetRegisterKey(&global_key_id, &global_key);
		match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 92, 0 == error);
	}

	global_key_depth++;

	return;
}

void transport_pop_key(
	void)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 102, global_key_depth > 0);

	if (--global_key_depth==0)
	{
		XNetUnregisterKey(&global_key_id);
	}

	return;
}

void transport_get_nonce(
	byte *dst,
	long bytes)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 151, dst != NULL);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 152, bytes == sizeof(global_nonce));

	csmemcpy(dst, global_nonce, sizeof(global_nonce));

	return;
}

boolean transport_nonce_is_equal(
	byte *src,
	byte *dst)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 163, src != NULL);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 164, dst != NULL);

	return (boolean)(csmemcmp(src, dst, sizeof(global_nonce))==0);
}

boolean transport_is_nonce(
	byte *src,
	long bytes)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 175, src != NULL);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 176, bytes == sizeof(global_nonce));

	return transport_nonce_is_equal(src, global_nonce);
}

void transport_client_stop(
	void)
{
	if (global_client_active)
	{
		transport_pop_key();
		global_client_active= FALSE;
	}

	return;
}

XNADDR transport_get_xnaddr(
	void)
{
	return global_address;
}

XNKID transport_get_key_id(
	void)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 224, global_key_depth > 0);

	return global_key_id;
}

XNKEY transport_get_key(
	void)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 231, global_key_depth > 0);

	return global_key;
}

long transport_initialize(
	void)
{
	if (!transport_initialized)
	{
		WSADATA wsa_data= {0};
		XNetStartupParams xnsp= {0};
		DWORD link_status;
		FILE *bypass_security;
		short result;
		DWORD xnaddr_status;
		unsigned long timeout;

		xnsp.cfgSizeOfStruct= sizeof(xnsp);
		xnsp.cfgFlags= 0;
		xnsp.cfgPrivatePoolSizeInPages= 24;
		xnsp.cfgEnetReceiveQueueLength= 8;
		xnsp.cfgIpFragMaxSimultaneous= 4;
		xnsp.cfgIpFragMaxPacketDiv256= 8;
		xnsp.cfgSockMaxSockets= 128;
		xnsp.cfgSockDefaultRecvBufsizeInK= 0;
		xnsp.cfgSockDefaultSendBufsizeInK= 0;
		xnsp.cfgKeyRegMax= 1;
		xnsp.cfgSecRegMax= 32;

		link_status= XNetGetEthernetLinkStatus();
		error(_error_log, "xbox ethernet link is %s%s%s%s%s",
			(link_status&XNET_ETHERNET_LINK_ACTIVE) ? "connected" : "not connected",
			(link_status&XNET_ETHERNET_LINK_100MBPS) ? " at 100 Mbps" : "",
			(link_status&XNET_ETHERNET_LINK_10MBPS) ? " at 10 Mbps" : "",
			(link_status&XNET_ETHERNET_LINK_FULL_DUPLEX) ? " in full-duplex mode" : "",
			(link_status&XNET_ETHERNET_LINK_HALF_DUPLEX) ? " in half-duplex mode" : "");

		xnsp.cfgSizeOfStruct= sizeof(xnsp);
		xnsp.cfgFlags= 0;
		if ((bypass_security= fopen("d:\\bypass_security.txt", "r"))!=NULL)
		{
			error(_error_silent, "XNET_STARTUP_BYPASS_SECURITY [ON]");
			xnsp.cfgFlags|= XNET_STARTUP_BYPASS_SECURITY;
			fclose(bypass_security);
		}

		if (XNetStartup(&xnsp)!=0)
		{
			return _transport_error_not_initialized;
		}

		result= (short)WSAStartup(MAKEWORD(2,0), &wsa_data);
		if (result!=0)
		{
			XNetCleanup();
			winsock_error_to_string(result);

			return _transport_error_not_initialized;
		}

		timeout= system_milliseconds()+TRANSPORT_XNADDR_TIMEOUT;
		do
		{
			xnaddr_status= XNetGetTitleXnAddr(&global_address);
			if (system_milliseconds()>timeout)
			{
				WSACleanup();
				XNetCleanup();

				return _transport_error_not_initialized;
			}
		}
		while (xnaddr_status==XNET_GET_XNADDR_PENDING);

		if (xnaddr_status==XNET_GET_XNADDR_NONE)
		{
			WSACleanup();
			XNetCleanup();

			return _transport_error_not_initialized;
		}

		XNetRandom(global_nonce, sizeof(global_nonce));
		transport_initialized= TRUE;

		return _transport_error_none;
	}

	return _transport_error_none;
}

short transport_dispose(
	void)
{
	short result= _transport_error_none;

	if (transport_initialized)
	{
		WSACleanup();
		XNetCleanup();
		transport_initialized= FALSE;
	}
	else
	{
		result= _transport_error_not_initialized;
	}

	return result;
}

boolean transport_network_available(
	void)
{
	boolean available= (boolean)XNetGetEthernetLinkStatus();

	available&= XNET_ETHERNET_LINK_ACTIVE;

	return available;
}

struct endpoint_set *create_endpoint_set(
	short max_endpoints)
{
	struct endpoint_set *set;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 406, transport_initialized);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 407, max_endpoints > 0);

	set= match_malloc("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 409, sizeof(struct endpoint_set));
	if (set)
	{
		if (max_endpoints<=FD_SETSIZE)
		{
			set->dirty= FALSE;
			FD_ZERO(&set->fds);
			set->ep_array= debug_malloc(max_endpoints*sizeof(transport_endpoint_ref), TRUE,
				"c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 418);
			if (set->ep_array)
			{
				set->max_endpoints= max_endpoints;
				set->last_index= NONE;
				set->next_index= 0;
			}
			else
			{
				match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 426, set);
				set= NULL;
			}
		}
		else
		{
			match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 432, set);
			set= NULL;
		}
	}

	return set;
}

short delete_endpoint_set(
	struct endpoint_set *set)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 443, set && set->ep_array);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 444, transport_initialized);

	match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 446, set->ep_array);
	match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 447, set);

	return _transport_error_none;
}

long poll_endpoint_set(
	struct endpoint_set *set,
	word timeout)
{
	fd_set read_fds;
	struct timeval timeout_value;
	long i= 0, result;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 477, set);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 478, transport_initialized);

	timeout_value.tv_usec= timeout*1000;
	timeout_value.tv_sec= 0;

	if (set->dirty)
	{
		qsort(set->ep_array, set->last_index+1, sizeof(transport_endpoint_ref), poll_ep_array_compare_proc);

		while (set->ep_array[set->last_index]==NULL)
		{
			set->last_index--;
		}

		FD_ZERO(&set->fds);
		for (; i<=set->last_index; i++)
		{
			FD_SET(set->ep_array[i]->socket, &set->fds);
			set->ep_array[i]->flags&= (byte)~FLAG(_endpoint_readable_bit);
		}

		set->dirty= FALSE;
	}
	else
	{
		for (; i<=set->last_index; i++)
		{
			set->ep_array[i]->flags&= (byte)~FLAG(_endpoint_readable_bit);
		}
	}

	// the descriptor set sits at the head of the endpoint set
	csmemcpy(&read_fds, set, sizeof(read_fds));

	result= select(set->last_index+1, &read_fds, NULL, NULL, &timeout_value);
	if (result>0)
	{
		for (i= 0; i<=set->last_index; i++)
		{
			if (set->ep_array[i]->socket==INVALID_SOCKET)
			{
				return _transport_error_bad_endpoint;
			}
			if (FD_ISSET(set->ep_array[i]->socket, &read_fds))
			{
				set->ep_array[i]->flags|= (byte)FLAG(_endpoint_readable_bit);
			}
		}
	}
	else if (result<0)
	{
		winsock_error_to_string(WSAGetLastError());

		return _transport_error_poll_error;
	}
	else if (result==SOCKET_ERROR)
	{
		winsock_error_to_string(WSAGetLastError());

		return _transport_error_poll_error;
	}
	else
	{
		return _transport_result_poll_timeout;
	}

	return _transport_error_none;
}

long add_endpoint_to_set(
	transport_endpoint_ref ep,
	struct endpoint_set *set)
{
	long index;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 559, ep && set);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 560, transport_initialized);

	index= get_next_available_set_array_index(set);
	if (index>=0)
	{
		set->ep_array[index]= ep;
		// xxx listening endpoints take the same path as everybody else
		if (ep->flags&FLAG(_endpoint_listening_bit))
		{
			FD_SET(set->ep_array[index]->socket, &set->fds);
		}
		else
		{
			FD_SET(set->ep_array[index]->socket, &set->fds);
		}
		set->last_index++;
		ep->flags|= (byte)FLAG(_endpoint_in_set_bit);

		return _transport_error_none;
	}

	return _transport_error_endpoint_set_full;
}

long remove_endpoint_from_set(
	transport_endpoint_ref ep,
	struct endpoint_set *set)
{
	long i;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 597, ep && set);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 598, transport_initialized);

	for (i= 0; i<=set->last_index; i++)
	{
		if (set->ep_array[i]==ep)
		{
			FD_CLR(ep->socket, &set->fds);
			ep->flags&= (byte)~FLAG(_endpoint_in_set_bit);
			set->ep_array[i]= NULL;
			set->dirty= TRUE;

			return _transport_error_none;
		}
	}

	return _transport_error_endpoint_not_in_set;
}

void rewind_endpoint_set(
	struct endpoint_set *set)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 621, set);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 622, transport_initialized);

	set->next_index= 0;

	return;
}

transport_endpoint_ref get_next_endpoint_from_set(
	struct endpoint_set *set)
{
	transport_endpoint_ref result= NULL;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 634, set);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 635, transport_initialized);

	if (set->next_index<=set->last_index)
	{
		result= set->ep_array[set->next_index++];
	}

	return result;
}

long count_endpoints_in_set(
	struct endpoint_set *set)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 649, set);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 650, transport_initialized);

	return set->last_index+1;
}

short transport_server_initialize(
	void)
{
	XNKEY key;
	XNKID key_id;

	transport_client_stop();

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 121, 0 == global_key_depth);

	server_transport_globals= TRUE;
	XNetCreateKey(&key_id, &key);
	transport_push_key(&key, &key_id);

	return _transport_error_none;
}

short transport_server_terminate(
	void)
{
	transport_client_stop();
	transport_pop_key();

	csmemset(&server_transport_globals, 0, sizeof(server_transport_globals));

	return _transport_error_none;
}

void transport_client_start(
	XNADDR *xnaddr,
	XNKEY *key,
	XNKID *key_id,
	word port,
	transport_address *address)
{
	IN_ADDR ina;

	transport_client_stop();
	transport_push_key(key, key_id);
	XNetXnAddrToInAddr(xnaddr, key_id, &ina);

	address->address.ipv4_address= ((ina.S_un.S_addr&0xff)<<24) | ((ina.S_un.S_addr&0xff00)<<8) |
		((ina.S_un.S_addr>>8)&0xff00) | ((ina.S_un.S_addr>>24)&0xff);
	address->address_length= IPV4_ADDRESS_LENGTH;
	address->port= port;
	address->address_type= 0;

	global_client_active= TRUE;

	return;
}

/* ---------- private code */

static long get_next_available_set_array_index(
	struct endpoint_set *set)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 57, set);

	if (set->last_index>set->max_endpoints-1)
	{
		return NONE;
	}

	return set->last_index+1;
}

static int poll_ep_array_compare_proc(
	const void *a,
	const void *b)
{
	transport_endpoint_ref ep_a= *(transport_endpoint_ref *)a;
	transport_endpoint_ref ep_b= *(transport_endpoint_ref *)b;

	if (ep_a==NULL && ep_b!=NULL)
	{
		return 1;
	}
	if (ep_a!=NULL && ep_b==NULL)
	{
		return -1;
	}

	return 0;
}
