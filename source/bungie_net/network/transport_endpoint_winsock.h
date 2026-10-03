/*
TRANSPORT_ENDPOINT_WINSOCK.H

header included in hcex build.
*/

#ifndef __TRANSPORT_ENDPOINT_WINSOCK_H
#define __TRANSPORT_ENDPOINT_WINSOCK_H
#pragma once

/* ---------- headers */

#include "transport.h"

/* ---------- constants */

enum
{
	MICROSECONDS_PER_MILLISECOND = 1000, /* fake name */
};

enum
{
	_endpoint_connected_bit = 0, /* fake name */
	_endpoint_listening_bit, /* fake name */
	_endpoint_readable_bit, /* fake name */
	_endpoint_in_set_bit, /* fake name */
	_endpoint_nonblocking_bit, /* fake name */
	_endpoint_client_bit, /* fake name */
	NUMBER_OF_ENDPOINT_FLAGS,
};

/* ---------- macros */

/* ---------- structures */

struct transport_endpoint
{
	SOCKET socket;
	char flags;
	char type;
	short error;
};

struct transport_endpoint_set
{
	fd_set sockets; /* fake name */
	struct transport_endpoint **ep_array;
	long max_endpoints;
	long last_index; /* fake name */
	long current_index; /* fake name */
	long needs_compaction; /* fake name */
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

// uninitialized globals
extern XNKEY global_key;
extern XNKID global_key_id;
extern unsigned __int64 global_nonce;
extern XNADDR global_address;
extern boolean server_transport_globals;

/* ---------- public code */

#endif // __TRANSPORT_ENDPOINT_WINSOCK_H
