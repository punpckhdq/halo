/*
TRANSPORT_ENDPOINT_WINSOCK.H

header included in hcex build.
*/

#ifndef __TRANSPORT_ENDPOINT_WINSOCK_H
#define __TRANSPORT_ENDPOINT_WINSOCK_H
#pragma once

#include "transport.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

enum
{
	_endpoint_connected_bit, // 0x01
	_endpoint_listening_bit, // 0x02
	_endpoint_readable_bit, // 0x04, set by poll_endpoint_set from FD_ISSET
	_endpoint_in_set_bit, // 0x08, set by add_endpoint_to_set, cleared by remove_endpoint_from_set
	_endpoint_non_blocking_bit, // 0x10, endpoint_blocking() returns the complement of this
	_endpoint_unknown5_bit, // 0x20, xxx connect_endpoint sets this beside _endpoint_connected_bit and nobody reads it
	NUMBER_OF_ENDPOINT_FLAGS
};

struct transport_endpoint
{
	SOCKET socket; // 0x00, INVALID_SOCKET until bound or connected
	byte flags; // 0x04
	char type; // 0x05, _transport_type_udp or _transport_type_tcp; get_endpoint_type sign-extends it
	word error; // 0x06, the last transport error this endpoint took
}; // 0x08

struct endpoint_set
{
	fd_set fds; // 0x000, driven with the winsock FD_* macros
	transport_endpoint_ref *ep_array; // 0x104
	long max_endpoints; // 0x108
	long last_index; // 0x10c, NONE when empty
	long next_index; // 0x110, the rewind/get_next cursor
	long dirty; // 0x114, poll_endpoint_set re-sorts and rebuilds fds
}; // 0x118

/* ---------- prototypes/TRANSPORT_ENDPOINT_WINSOCK.C */

char *winsock_error_to_string(long error);
transport_endpoint_ref create_transport_endpoint(long type);
void delete_transport_endpoint(transport_endpoint_ref ep);
short get_endpoint_address(transport_endpoint_ref ep, transport_address *address);
long get_endpoint_type(transport_endpoint_ref ep);
short set_endpoint_blocking(transport_endpoint_ref ep, long blocking);
short bind_endpoint(transport_endpoint_ref ep, transport_address *address);
short connect_endpoint(transport_endpoint_ref ep, transport_address *address);
void disconnect_endpoint(transport_endpoint_ref ep);
short connect_endpoint_async(transport_endpoint_ref ep, transport_address *address, transport_connect_process_ref *process_ref_ptr);
short listen_endpoint(transport_endpoint_ref ep);
transport_endpoint_ref accept_endpoint(transport_endpoint_ref listening_endpoint);
short reject_endpoint(transport_endpoint_ref listening_endpoint);
long read_endpoint(transport_endpoint_ref ep, void *buffer, long length);
long read_from_endpoint(transport_endpoint_ref ep, void *buffer, long length, transport_address *src_addr);
long write_to_endpoint(transport_endpoint_ref ep, void *buffer, long length, transport_address *dest_addr);
boolean endpoint_readable(transport_endpoint_ref ep, word timeout);
boolean endpoint_writeable(transport_endpoint_ref ep, word timeout);
boolean endpoint_connected(transport_endpoint_ref ep);
boolean endpoint_listening(transport_endpoint_ref ep);
boolean endpoint_blocking(transport_endpoint_ref ep);
short get_endpoint_error(transport_endpoint_ref ep);
long endpoint_equivalent(transport_endpoint_ref a, transport_endpoint_ref b);

/* ---------- globals */

/* ---------- public code */

#endif // __TRANSPORT_ENDPOINT_WINSOCK_H
