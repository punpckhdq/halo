/*
NETWORK_CONNECTION.H

header included in hcex build.
*/

#ifndef __NETWORK_CONNECTION_H
#define __NETWORK_CONNECTION_H
#pragma once

/* ---------- constants */

enum
{
	DATAGRAM_MAXIMUM_SIZE = 400,
};

enum
{
	_connection_create_server_bit = 0,
	_connection_create_clientside_client_bit,
	_connection_create_serverside_client_bit,
	NUMBER_OF_CONNECTION_CREATE_FLAGS, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

typedef void (*network_connection_rejection_procedure)(struct transport_endpoint *endpoint); /* fake name */

/* ---------- prototypes/NETWORK_CONNECTION.C */

void network_connection_initialize(void);
struct network_connection *network_connection_new(unsigned long flags, word well_known_port);
void network_connection_delete(struct network_connection *connection);
void network_server_allow_client_connections(struct network_connection *server_connection, boolean allow_client_connections);
boolean network_connection_connected(struct network_connection *connection);
boolean network_connection_write(struct network_connection *connection, void *message, word buffer_size, struct transport_address const *dest_address, boolean reliable);
boolean network_connection_read(struct network_connection *connection, void *message, word *buffer_size, struct transport_address *source_address);
boolean network_server_close_client_connection(struct network_connection *server_connection, struct network_connection *client_connection);
boolean network_connection_idle(struct network_connection *connection, unsigned long timeout, struct network_connection **new_client_connection);
void network_connection_get_address(struct network_connection *connection, struct transport_address *reliable_address, struct transport_address *unreliable_address);
boolean network_connection_connect(struct network_connection *connection, struct transport_address const *remote_address, struct transport_connect_process **process_reference);
void network_connection_set_connection_rejection_procedure(struct network_connection *connection, network_connection_rejection_procedure connection_rejection_procedure);
boolean network_connection_server_accept_client_connection(struct network_connection *server_connection, struct network_connection *client_connection);
boolean network_connection_active(struct network_connection *connection);
boolean network_connection_going_stale(struct network_connection *connection);
void network_connection_keep_alive(struct network_connection *connection);
boolean network_connection_disconnect(struct network_connection *connection);

/* ---------- globals */

extern boolean global_connection_dont_timeout;

/* ---------- public code */

#endif // __NETWORK_CONNECTION_H
