/*
NETWORK_CONNECTION.H

header included in hcex build.
*/

#ifndef __NETWORK_CONNECTION_H
#define __NETWORK_CONNECTION_H
#pragma once

/* ---------- headers */

#include "transport.h"
#include "message_header.h"

/* ---------- constants */

enum
{
	NETWORK_SERVER_PORT= 5150,
	NETWORK_CLIENT_PORT= 5151,
};

enum
{
	_connection_create_server_bit= 0,
	_connection_create_clientside_client_bit,
	_connection_create_serverside_client_bit, // a client connection the server accepted
	NUMBER_OF_CONNECTION_CREATE_FLAGS
};

// timeouts passed to network_connection_idle(), in milliseconds
enum
{
	_connection_dont_timeout= 0,
	_connection_timeout_fast= 5000,
	_connection_timeout= 15000
};

/* ---------- structures */

struct network_connection;

typedef void (*connection_rejection_procedure)(transport_endpoint_ref endpoint);

/* ---------- prototypes/NETWORK_CONNECTION.C */

void network_connection_initialize(void);
struct network_connection *network_connection_new(unsigned long flags, word well_known_port);
void network_connection_delete(struct network_connection *connection);
boolean network_connection_idle(struct network_connection *connection, long timeout, struct network_connection **new_client_connection);
boolean network_connection_connected(struct network_connection *connection);
boolean network_connection_active(struct network_connection *connection);
boolean network_connection_going_stale(struct network_connection *connection);
void network_connection_keep_alive(struct network_connection *connection);
void network_connection_get_address(struct network_connection *connection, transport_address *remote_address, transport_address *local_address);
boolean network_connection_connect(struct network_connection *connection, transport_address *address, transport_connect_process_ref *connect_process);
boolean network_connection_disconnect(struct network_connection *connection);
void network_connection_set_connection_rejection_procedure(struct network_connection *connection, connection_rejection_procedure procedure);
boolean network_connection_server_accept_client_connection(struct network_connection *server_connection, struct network_connection *new_client_connection);
boolean network_connection_write(struct network_connection *connection, message_header *message, unsigned short message_size, transport_address *dest_address, boolean reliable);
boolean network_connection_read(struct network_connection *connection, message_header *message, word *buffer_size, transport_address *source_address);
void network_server_allow_client_connections(struct network_connection *server_connection, boolean allow);
boolean network_server_close_client_connection(struct network_connection *server_connection, struct network_connection *client_connection);

/*
transport

- the endpoint and endpoint-set calls below still want a header of their own; the types and
error codes they share with the rest of the transport layer now live in transport.h

- poll_endpoint_set, add_endpoint_to_set and remove_endpoint_from_set are declared short here
and defined long in transport_endpoint_set_winsock.c. That disagreement is the original's:
this unit's call sites read the results as 16-bit, and widening any of the three to match the
definition costs matched bytes (693, 716 and 1000 respectively). Leave them alone.
*/

struct endpoint_set;

struct endpoint_set *create_endpoint_set(short max_endpoints);
short delete_endpoint_set(struct endpoint_set *set);
short poll_endpoint_set(struct endpoint_set *set, word timeout);
short add_endpoint_to_set(transport_endpoint_ref ep, struct endpoint_set *set);
short remove_endpoint_from_set(transport_endpoint_ref ep, struct endpoint_set *set);
void rewind_endpoint_set(struct endpoint_set *set);
transport_endpoint_ref get_next_endpoint_from_set(struct endpoint_set *set);
long count_endpoints_in_set(struct endpoint_set *set);

transport_endpoint_ref create_transport_endpoint(long type);
void delete_transport_endpoint(transport_endpoint_ref ep);
short get_endpoint_address(transport_endpoint_ref ep, transport_address *address);
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
boolean endpoint_connected(transport_endpoint_ref ep);

/* ---------- globals */

/* ---------- public code */

#endif // __NETWORK_CONNECTION_H
