/*
TRANSPORT_ENDPOINT_SET_WINSOCK.C
*/

/* ---------- headers */

#include "cseries.h"
#include "transport_endpoint_winsock.h"
#include "byte_swapping.h"
#include "network_game_globals.h"
#include "units.h"
#include "network_game_manager.h"
#include "network_messages.h"
#include "main.h"
#include "network_client_manager.h"
#include "network_server_manager.h"
#include "bungie_net/common/memory_manager.h"
#include <assert.h>

/* ---------- constants */

enum
{
	WINSOCK_VERSION_MAJOR = 2, /* fake name */
	WINSOCK_VERSION_MINOR = 0, /* fake name */
	TITLE_ADDRESS_TIMEOUT = 10*MILLISECONDS_PER_SECOND, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static long get_next_available_set_array_index(struct transport_endpoint_set const *set);
static int __cdecl poll_ep_array_compare_proc(void const *a, void const *b);

/* ---------- globals */

boolean transport_initialized = FALSE;
static boolean global_client_active = FALSE;
static long global_key_depth = 0;

/* ---------- public code */

void net_startup_debug(
	void)
{
	return;
}

void transport_push_key(
	XNKEY const *key,
	XNKID const *key_id)
{
	global_key = *key;
	global_key_id = *key_id;

	if (!global_key_depth)
	{
		long error = XNetRegisterKey(&global_key_id, &global_key);

		match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 92, 0 == error);
	}

	global_key_depth++;

	return;
}

void transport_pop_key(
	void)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 102, global_key_depth > 0);

	if (!--global_key_depth)
	{
		XNetUnregisterKey(&global_key_id);
	}

	return;
}

short transport_server_initialize(
	void)
{
	XNKEY key;
	XNKID key_id;

	transport_client_stop();
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 121, 0 == global_key_depth);
	server_transport_globals = TRUE;
	XNetCreateKey(&key_id, &key);
	transport_push_key(&key, &key_id);

	return _transport_error_none;
}

short transport_server_terminate(
	void)
{
	transport_client_stop();
	transport_pop_key();
	memset(&server_transport_globals, 0, sizeof(server_transport_globals));

	return _transport_error_none;
}

void transport_get_nonce(
	void *dst,
	long bytes)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 151, dst != NULL);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 152, bytes == sizeof(global_nonce));

	memcpy(dst, &global_nonce, sizeof(global_nonce));

	return;
}

boolean transport_nonce_is_equal(
	void const *src,
	void const *dst)
{
	boolean result;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 163, src != NULL);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 164, dst != NULL);

	if (!memcmp(src, dst, sizeof(global_nonce)))
	{
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}

	return result;
}

boolean transport_is_nonce(
	void const *src,
	long bytes)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 175, src != NULL);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 176, bytes == sizeof(global_nonce));

	return transport_nonce_is_equal(src, &global_nonce);
}

void transport_client_stop(
	void)
{
	if (global_client_active)
	{
		transport_pop_key();
		global_client_active = FALSE;
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

void transport_client_start(
	XNADDR const *address,
	XNKEY const *key,
	XNKID const *key_id,
	word port,
	struct transport_address *result)
{
	struct in_addr ip;

	transport_client_stop();
	transport_push_key(key, key_id);
	XNetXnAddrToInAddr(address, key_id, &ip);
	result->address.ipv4_address = SWAP4(ip.s_addr);
	result->address_length = IPV4_ADDRESS_LENGTH;
	result->port = port;
	result->address_type = 0;
	global_client_active = TRUE;

	return;
}

long transport_initialize(
	void)
{
	long result = _transport_error_none;

	if (!transport_initialized)
	{
		unsigned long link_status;
		FILE *file;
		WSADATA winsock_data = {0};
		XNetStartupParams startup = {0};

		startup.cfgSizeOfStruct = sizeof(XNetStartupParams);
		startup.cfgFlags = 0;
		startup.cfgPrivatePoolSizeInPages = 24;
		startup.cfgEnetReceiveQueueLength = 8;
		startup.cfgIpFragMaxSimultaneous = 4;
		startup.cfgIpFragMaxPacketDiv256 = 8;
		startup.cfgSockMaxSockets = 128;
		startup.cfgSockDefaultRecvBufsizeInK = 0;
		startup.cfgSockDefaultSendBufsizeInK = 0;
		startup.cfgKeyRegMax = 1;
		startup.cfgSecRegMax = 32;

		link_status = XNetGetEthernetLinkStatus();
		error(
			_error_log,
			"xbox ethernet link is %s%s%s%s%s",
			link_status & XNET_ETHERNET_LINK_ACTIVE ? "connected" : "not connected",
			link_status & XNET_ETHERNET_LINK_100MBPS ? " at 100 Mbps" : "",
			link_status & XNET_ETHERNET_LINK_10MBPS ? " at 10 Mbps" : "",
			link_status & XNET_ETHERNET_LINK_FULL_DUPLEX ? " in full-duplex mode" : "",
			link_status & XNET_ETHERNET_LINK_HALF_DUPLEX ? " in half-duplex mode" : "");

		startup.cfgSizeOfStruct = sizeof(XNetStartupParams);
		startup.cfgFlags = 0;
		file = fopen("d:\\bypass_security.txt", "r");

		if (file)
		{
			error(_error_silent, "XNET_STARTUP_BYPASS_SECURITY [ON]");
			startup.cfgFlags |= XNET_STARTUP_BYPASS_SECURITY;
			fclose(file);
		}

		if (XNetStartup(&startup))
		{
			result = _transport_error_not_initialized;
		}
		else
		{
			short startup_error = WSAStartup(MAKEWORD(WINSOCK_VERSION_MAJOR, WINSOCK_VERSION_MINOR), &winsock_data);

			if (startup_error)
			{
				XNetCleanup();
				winsock_error_to_string(startup_error);

				return _transport_error_not_initialized;
			}
			else
			{
				unsigned long address_status;
				unsigned long deadline = system_milliseconds() + TITLE_ADDRESS_TIMEOUT;

				do
				{
					address_status = XNetGetTitleXnAddr(&global_address);

					if (system_milliseconds() > deadline)
					{
						address_status = XNET_GET_XNADDR_NONE;
						break;
					}
				}
				while (address_status == XNET_GET_XNADDR_PENDING);

				if (address_status == XNET_GET_XNADDR_NONE)
				{
					WSACleanup();
					XNetCleanup();
					result = _transport_error_not_initialized;
				}
				else
				{
					XNetRandom((byte *)&global_nonce, sizeof(global_nonce));
					result = _transport_error_none;
					transport_initialized = TRUE;
				}
			}
		}
	}

	return result;
}

short transport_dispose(
	void)
{
	short result = _transport_error_none;

	if (transport_initialized)
	{
		WSACleanup();
		XNetCleanup();
		transport_initialized = FALSE;
	}
	else
	{
		result = _transport_error_not_initialized;
	}

	return result;
}

boolean transport_network_available(
	void)
{
	boolean available = XNetGetEthernetLinkStatus();

	available &= XNET_ETHERNET_LINK_ACTIVE;

	return available;
}

struct transport_endpoint_set *create_endpoint_set(
	short max_endpoints)
{
	struct transport_endpoint_set *set;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 406, transport_initialized);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 407, max_endpoints > 0);

	set = (struct transport_endpoint_set *)match_malloc("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 409, sizeof(struct transport_endpoint_set));

	if (set)
	{
		if (max_endpoints <= FD_SETSIZE)
		{
			set->needs_compaction = FALSE;
			FD_ZERO(&set->sockets);
			set->ep_array = (struct transport_endpoint **)match_calloc("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 418, max_endpoints, sizeof(struct transport_endpoint *));

			if (set->ep_array)
			{
				set->max_endpoints = max_endpoints;
				set->last_index = NONE;
				set->current_index = 0;
			}
			else
			{
				match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 426, set);
				set = NULL;
			}
		}
		else
		{
			match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 432, set);
			set = NULL;
		}
	}

	return set;
}

short delete_endpoint_set(
	struct transport_endpoint_set *set)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 443, set && set->ep_array);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 444, transport_initialized);

	match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 446, set->ep_array);
	match_free("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 447, set);

	return _transport_error_none;
}

short poll_endpoint_set(
	struct transport_endpoint_set *set,
	word timeout)
{
	fd_set readable;
	struct timeval wait;
	long count;
	boolean poll_failed;
	long result = _transport_error_none;
	long i = 0;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 477, set);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 478, transport_initialized);

	wait.tv_usec = timeout * MICROSECONDS_PER_MILLISECOND;
	wait.tv_sec = 0;

	if (set->needs_compaction)
	{
		qsort(set->ep_array, set->last_index + 1, sizeof(struct transport_endpoint *), poll_ep_array_compare_proc);

		while (!set->ep_array[set->last_index])
		{
			set->last_index--;
		}

		FD_ZERO(&set->sockets);

		for (; i <= set->last_index; i++)
		{
			FD_SET(set->ep_array[i]->socket, &set->sockets);
			SET_FLAG(set->ep_array[i]->flags, _endpoint_readable_bit, FALSE);
		}

		set->needs_compaction = FALSE;
	}
	else
	{
		for (; i <= set->last_index; i++)
		{
			SET_FLAG(set->ep_array[i]->flags, _endpoint_readable_bit, FALSE);
		}
	}

	memcpy(&readable, &set->sockets, sizeof(fd_set));
	count = select(set->last_index + 1, &readable, NULL, NULL, &wait);

	if (count > 0)
	{
		for (i = 0; i <= set->last_index; i++)
		{
			if (set->ep_array[i]->socket == INVALID_SOCKET)
			{
				result = _transport_error_bad_endpoint;
				break;
			}

			if (FD_ISSET(set->ep_array[i]->socket, &readable))
			{
				SET_FLAG(set->ep_array[i]->flags, _endpoint_readable_bit, TRUE);
			}
		}
	}
	else
	{
		poll_failed = count < 0 || count == SOCKET_ERROR;

		if (poll_failed)
		{
			winsock_error_to_string(WSAGetLastError());
			result = _transport_error_poll_error;
		}
		else
		{
			result = _transport_result_poll_timeout;
		}
	}

	return result;
}

short add_endpoint_to_set(
	struct transport_endpoint *ep,
	struct transport_endpoint_set *set)
{
	long result;
	long index;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 559, ep && set);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 560, transport_initialized);

	index = get_next_available_set_array_index(set);

	if (index >= 0)
	{
		set->ep_array[index] = ep;

		if (TEST_FLAG(ep->flags, _endpoint_listening_bit))
		{
			FD_SET(set->ep_array[index]->socket, &set->sockets);
		}
		else
		{
			FD_SET(set->ep_array[index]->socket, &set->sockets);
		}

		set->last_index++;
		SET_FLAG(ep->flags, _endpoint_in_set_bit, TRUE);
		result = _transport_error_none;
	}
	else
	{
		result = _transport_error_endpoint_set_full;
	}

	return result;
}

short remove_endpoint_from_set(
	struct transport_endpoint *ep,
	struct transport_endpoint_set *set)
{
	long result = _transport_error_endpoint_not_in_set;
	long i = 0;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 597, ep && set);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 598, transport_initialized);

	for (; i <= set->last_index; i++)
	{
		if (set->ep_array[i] == ep)
		{
			FD_CLR(ep->socket, &set->sockets);
			SET_FLAG(ep->flags, _endpoint_in_set_bit, FALSE);
			set->ep_array[i] = NULL;
			set->needs_compaction = TRUE;
			result = _transport_error_none;
			break;
		}
	}

	return result;
}

void rewind_endpoint_set(
	struct transport_endpoint_set *set)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 621, set);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 622, transport_initialized);

	set->current_index = 0;

	return;
}

struct transport_endpoint *get_next_endpoint_from_set(
	struct transport_endpoint_set *set)
{
	struct transport_endpoint *ep = NULL;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 634, set);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 635, transport_initialized);

	if (set->current_index <= set->last_index)
	{
		ep = set->ep_array[set->current_index++];
	}

	return ep;
}

long count_endpoints_in_set(
	struct transport_endpoint_set const *set)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 649, set);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 650, transport_initialized);

	return set->last_index + 1;
}

/* ---------- private code */

static long get_next_available_set_array_index(
	struct transport_endpoint_set const *set)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\network\\transport_endpoint_set_winsock.c", 57, set);

	return set->last_index > set->max_endpoints - 1 ? NONE : set->last_index + 1;
}

static int __cdecl poll_ep_array_compare_proc(
	void const *a,
	void const *b)
{
	int result;

	a = *(struct transport_endpoint const *const *)a;
	b = *(struct transport_endpoint const *const *)b;

	if (!a && b)
	{
		result = 1;
	}
	else if (a && !b)
	{
		result = -1;
	}
	else
	{
		result = 0;
	}

	return result;
}
