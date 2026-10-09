/*
NETWORK_CONNECTION.C
*/

/* ---------- headers */

#include "cseries.h"
#include "network_connection.h"
#include "network_game_globals.h"
#include "network_game_manager.h"
#include "network_messages.h"
#include "network_server_manager.h"
#include "units.h"
#include "bungie_net/common/64bit_math.h"
#include "bungie_net/common/message_encryption.h"
#include "bungie_net/common/public_key_crypt.h"
#include "bungie_net/common/message_header.h"
#include "bungie_net/network/transport.h"

/* ---------- constants */

enum
{
	_connection_closed_bit = 4, /* fake name */
	_connection_going_stale_bit, /* fake name */
};

enum
{
	RELIABLE_MESSAGE_MAXIMUM_SIZE = 2048, /* fake name */
	MAXIMUM_RESERVED_NETWORK_PORT = 1023,
	MAXIMUM_SERVER_CLIENT_CONNECTIONS = 4, /* fake name */
	MAXIMUM_SERVER_ENDPOINTS = MAXIMUM_SERVER_CLIENT_CONNECTIONS + 1, /* fake name */
	SERVER_UNRELIABLE_INCOMING_QUEUE_SIZE = 16 * DATAGRAM_MAXIMUM_SIZE, /* fake name */
	CLIENT_RELIABLE_INCOMING_QUEUE_SIZE = 16 * RELIABLE_MESSAGE_MAXIMUM_SIZE, /* fake name */
	CLIENT_UNRELIABLE_INCOMING_QUEUE_SIZE = 4 * DATAGRAM_MAXIMUM_SIZE, /* fake name */
	CONNECTION_GOING_STALE_MILLISECONDS = 5 * MILLISECONDS_PER_SECOND, /* fake name */
	CONNECTION_BLOCKED_WARNING_MILLISECONDS = MILLISECONDS_PER_SECOND, /* fake name */
	UDP_HEADER_OVERHEAD = 28, /* fake name */
	TCP_HEADER_OVERHEAD = 40, /* fake name */
	TRAFFIC_LOG_NAME_LENGTH = 256, /* fake name */
};

enum
{
	_network_connection_traffic_event_open = 0, /* fake name */
	_network_connection_traffic_event_close, /* fake name */
	_network_connection_traffic_event_datagram_sent, /* fake name */
	_network_connection_traffic_event_datagram_received, /* fake name */
	_network_connection_traffic_event_stream_bytes_sent, /* fake name */
	_network_connection_traffic_event_stream_bytes_received, /* fake name */
	_network_connection_traffic_event_stream_message_sent, /* fake name */
	_network_connection_traffic_event_stream_message_received, /* fake name */
	NUMBER_OF_NETWORK_CONNECTION_TRAFFIC_EVENTS, /* fake name */
};

/* ---------- macros */

#define TRAFFIC_LOG_SECONDS(connection) ((system_milliseconds() - (connection)->traffic_log_start_time) * (1.0 / MILLISECONDS_PER_SECOND)) /* fake name */

/* ---------- structures */

struct network_connection
{
	struct transport_endpoint *reliable_endpoint;
	struct transport_endpoint *unreliable_endpoint;
	unsigned long last_keep_alive_time;
	network_connection_rejection_procedure connection_rejection_procedure;
	struct circular_queue *reliable_incoming_queue;
	struct circular_queue *unreliable_incoming_queue;
	FILE *traffic_log; /* fake name */
	unsigned long traffic_log_start_time; /* fake name */
	long datagrams_sent; /* fake name */
	long datagrams_received; /* fake name */
	long stream_messages_sent; /* fake name */
	long stream_messages_received; /* fake name */
	unsigned long flags;
	word well_known_port;
};

struct network_server_connection /* fake name */
{
	struct network_connection connection;
	struct transport_endpoint_set *endpoint_set;
	struct network_connection *client_list[MAXIMUM_SERVER_CLIENT_CONNECTIONS];
	boolean allow_client_connections;
};

/* ---------- prototypes */

static struct network_connection *network_connection_create_client_from_endpoint(struct transport_endpoint *reliable_endpoint);
static boolean network_client_reliable_connection_read(struct network_connection *connection, void *message, word *buffer_size, struct transport_address *source_address);
static boolean network_client_unreliable_connection_read(struct network_connection *connection, void *message, word *buffer_size, struct transport_address *source_address);
static boolean network_connection_idle_server_reliable_endpoint(struct network_server_connection *connection, struct network_connection **new_client_connection);
static boolean network_connection_idle_client_reliable_endpoint(struct network_connection *connection);
static void network_connection_log_traffic_event(struct network_connection *connection, long event, long amount);

/* ---------- globals */

boolean global_connection_dont_timeout;

/* ---------- public code */

void network_connection_initialize(
	void)
{
	return;
}

struct network_connection *network_connection_new(
	unsigned long flags,
	word well_known_port)
{
	long reliable_queue_size;
	long unreliable_queue_size;
	struct network_connection *connection = NULL;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_connection.c",
		157,
		(flags&FLAG(_connection_create_server_bit))|| (flags&FLAG(_connection_create_clientside_client_bit)));

	if (TEST_FLAG(flags, _connection_create_server_bit))
	{
		struct network_server_connection *server;

		match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 163, well_known_port > MAXIMUM_RESERVED_NETWORK_PORT);

		server = match_calloc(
			"c:\\halo\\SOURCE\\networking\\network_connection.c",
			165,
			1,
			sizeof(struct network_server_connection));

		if (server)
		{
			server->allow_client_connections = TRUE;
			server->endpoint_set = create_endpoint_set(MAXIMUM_SERVER_ENDPOINTS);

			if (server->endpoint_set)
			{
				reliable_queue_size = 0;
				unreliable_queue_size = SERVER_UNRELIABLE_INCOMING_QUEUE_SIZE;
				connection = &server->connection;
			}
			else
			{
				network_connection_delete(&server->connection);
			}
		}
	}
	else if (TEST_FLAG(flags, _connection_create_clientside_client_bit))
	{
		connection = match_calloc(
			"c:\\halo\\SOURCE\\networking\\network_connection.c",
			182,
			1,
			sizeof(struct network_connection));

		if (connection)
		{
			reliable_queue_size = CLIENT_RELIABLE_INCOMING_QUEUE_SIZE;
			unreliable_queue_size = CLIENT_UNRELIABLE_INCOMING_QUEUE_SIZE;
		}
	}

	if (connection)
	{
		boolean success = TRUE;

		connection->last_keep_alive_time = system_milliseconds();
		connection->flags = flags;
		connection->reliable_endpoint = create_transport_endpoint(_transport_type_tcp);

		if (!connection->reliable_endpoint)
		{
			success = FALSE;
		}

		if (success && TEST_FLAG(flags, _connection_create_server_bit))
		{
			struct transport_address address = {0};

			address.address_length = IPV4_ADDRESS_LENGTH;
			address.port = well_known_port;

			if (bind_endpoint(connection->reliable_endpoint, &address) != _transport_error_none ||
				set_endpoint_blocking(connection->reliable_endpoint, FALSE) != _transport_error_none ||
				listen_endpoint(connection->reliable_endpoint) != _transport_error_none ||
				add_endpoint_to_set(
					connection->reliable_endpoint,
					((struct network_server_connection *)connection)->endpoint_set) != _transport_error_none)
			{
				success = FALSE;
			}
		}

		if (success)
		{
			connection->unreliable_endpoint = create_transport_endpoint(_transport_type_udp);

			if (!connection->unreliable_endpoint)
			{
				success = FALSE;
			}
		}

		if (success)
		{
			struct transport_address address;

			address.address_length = IPV4_ADDRESS_LENGTH;
			address.address.ipv4_address = 0;
			address.port = well_known_port;
			connection->well_known_port = well_known_port;

			if (bind_endpoint(connection->unreliable_endpoint, &address) != _transport_error_none ||
				set_endpoint_blocking(connection->unreliable_endpoint, FALSE) != _transport_error_none)
			{
				success = FALSE;
			}
		}

		if (success && reliable_queue_size)
		{
			connection->reliable_incoming_queue = circular_queue_new("incoming-reliable", reliable_queue_size);

			if (!connection->reliable_incoming_queue)
			{
				success = FALSE;
			}
		}

		if (success && unreliable_queue_size)
		{
			connection->unreliable_incoming_queue = circular_queue_new("incoming-unreliable", unreliable_queue_size);

			if (!connection->unreliable_incoming_queue)
			{
				success = FALSE;
			}
		}

		if (!success)
		{
			network_connection_delete(connection);
			connection = NULL;
		}
		else
		{
			network_connection_log_traffic_event(connection, _network_connection_traffic_event_open, 1);
		}
	}

	return connection;
}

void network_connection_delete(
	struct network_connection *connection)
{
	if (connection)
	{
		network_connection_log_traffic_event(connection, _network_connection_traffic_event_close, 1);

		if (connection->reliable_endpoint)
		{
			delete_transport_endpoint(connection->reliable_endpoint);
		}

		if (connection->unreliable_endpoint)
		{
			delete_transport_endpoint(connection->unreliable_endpoint);
		}

		if (connection->reliable_incoming_queue)
		{
			circular_queue_delete(connection->reliable_incoming_queue);
		}

		if (connection->unreliable_incoming_queue)
		{
			circular_queue_delete(connection->unreliable_incoming_queue);
		}

		if (TEST_FLAG(connection->flags, _connection_create_server_bit))
		{
			struct network_server_connection *server = (struct network_server_connection *)connection;

			if (server->client_list)
			{
				short client_index;

				for (client_index = 0; client_index < NUMBEROF(server->client_list); client_index++)
				{
					if (server->client_list[client_index])
					{
						if (server->endpoint_set)
						{
							remove_endpoint_from_set(
								server->client_list[client_index]->reliable_endpoint,
								server->endpoint_set);
						}

						network_connection_delete(server->client_list[client_index]);
					}
				}
			}

			if (server->endpoint_set)
			{
				delete_endpoint_set(server->endpoint_set);
			}
		}

		match_free("c:\\halo\\SOURCE\\networking\\network_connection.c", 325, connection);
	}

	return;
}

void network_server_allow_client_connections(
	struct network_connection *server_connection,
	boolean allow_client_connections)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 337, server_connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 338, server_connection->flags&FLAG(_connection_create_server_bit));

	((struct network_server_connection *)server_connection)->allow_client_connections = allow_client_connections;

	return;
}

boolean network_connection_connected(
	struct network_connection *connection)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 348, connection);

	return
		(TEST_FLAG(connection->flags, _connection_create_clientside_client_bit) || TEST_FLAG(connection->flags, _connection_create_serverside_client_bit)) 
		&& connection->reliable_endpoint 
		&& endpoint_connected(connection->reliable_endpoint);
}

boolean network_connection_write(
	struct network_connection *connection,
	void *message,
	word buffer_size,
	struct transport_address const *dest_address,
	boolean reliable)
{
	boolean success;
	long result = 0;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 368, message);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 369, buffer_size);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 370, connection);
	match_vassert(
		"c:\\halo\\SOURCE\\networking\\network_connection.c",
		372,
		GET_MESSAGE_SIZE(*(message_header *)message) == buffer_size,
		"bad message or buffer_size parameter");

	byte_swap_message_header(message, _byte_order_network);

	if (TEST_FLAG(connection->flags, _connection_create_server_bit))
	{
		match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 379, !reliable);
		match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 380, dest_address);

		if (buffer_size > DATAGRAM_MAXIMUM_SIZE)
		{
			error(_error_silent, "buffer size was %d max is %d", buffer_size, DATAGRAM_MAXIMUM_SIZE);
			match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 386, buffer_size <= DATAGRAM_MAXIMUM_SIZE);
		}

		result = write_to_endpoint(connection->unreliable_endpoint, message, buffer_size, dest_address);
		network_connection_log_traffic_event(connection, _network_connection_traffic_event_datagram_sent, buffer_size);
	}
	else if (reliable)
	{
		long bytes_written;

		match_vassert(
			"c:\\halo\\SOURCE\\networking\\network_connection.c",
			398,
			buffer_size <= RELIABLE_MESSAGE_MAXIMUM_SIZE,
			"message size exceeds maximum allowed size");
		match_assert(
			"c:\\halo\\SOURCE\\networking\\network_connection.c",
			400,
			(connection->flags&FLAG(_connection_create_clientside_client_bit)) || (connection->flags&FLAG(_connection_create_serverside_client_bit)));

		do
		{
			bytes_written = write_endpoint(connection->reliable_endpoint, message, buffer_size);

			if (bytes_written > 0)
			{
				result = TRUE;
				network_connection_log_traffic_event(
					connection,
					_network_connection_traffic_event_stream_bytes_sent,
					bytes_written);
				network_connection_log_traffic_event(connection, _network_connection_traffic_event_stream_message_sent, 1);
				break;
			}
			else if (bytes_written != _transport_result_operation_would_block)
			{
				error(
					_error_silent,
					"client call to write_endpoint() returned error '%s'",
					transport_error_to_string((short)bytes_written));
			}
		}
		while (bytes_written == _transport_result_operation_would_block);
	}
	else if (connection->unreliable_endpoint)
	{
		match_vassert(
			"c:\\halo\\SOURCE\\networking\\network_connection.c",
			432,
			buffer_size <= DATAGRAM_MAXIMUM_SIZE,
			"message size exceeds maximum allowed size");

		if (!dest_address)
		{
			if (endpoint_connected(connection->unreliable_endpoint))
			{
				write_endpoint(connection->unreliable_endpoint, message, buffer_size);
				network_connection_log_traffic_event(
					connection,
					_network_connection_traffic_event_datagram_sent,
					buffer_size);
			}
		}
		else
		{
			write_to_endpoint(connection->unreliable_endpoint, message, buffer_size, dest_address);
			network_connection_log_traffic_event(connection, _network_connection_traffic_event_datagram_sent, buffer_size);
		}
	}

	if (!reliable)
	{
		success = TRUE;
	}
	else
	{
		success = result > 0;
	}

	return success;
}

boolean network_connection_read(
	struct network_connection *connection,
	void *message,
	word *buffer_size,
	struct transport_address *source_address)
{
	boolean success;

	if (TEST_FLAG(connection->flags, _connection_create_server_bit))
	{
		success = network_client_unreliable_connection_read(connection, message, buffer_size, source_address);
	}
	else
	{
		match_assert(
			"c:\\halo\\SOURCE\\networking\\network_connection.c",
			480,
			connection->flags&FLAG(_connection_create_clientside_client_bit) || connection->flags&FLAG(_connection_create_serverside_client_bit));

		success = network_client_reliable_connection_read(connection, message, buffer_size, source_address);

		if (!success && TEST_FLAG(connection->flags, _connection_create_clientside_client_bit))
		{
			success = network_client_unreliable_connection_read(connection, message, buffer_size, source_address);
		}
	}

	return success;
}

boolean network_server_close_client_connection(
	struct network_connection *server_connection,
	struct network_connection *client_connection)
{
	long client_index;
	struct network_server_connection *server = (struct network_server_connection *)server_connection;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 503, server_connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 504, client_connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 505, server_connection->flags & FLAG(_connection_create_server_bit));
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 506, server->endpoint_set);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 507, server->client_list);

	for (client_index = 0; client_index < MAXIMUM_SERVER_ENDPOINTS; client_index++)
	{
		if (server->client_list[client_index] && server->client_list[client_index] == client_connection)
		{
			if (client_connection->reliable_endpoint &&
				remove_endpoint_from_set(
					server->client_list[client_index]->reliable_endpoint,
					server->endpoint_set) != _transport_error_none)
			{
				error(
					_error_silent,
					"failed to remove a client endpoint from the server's endpoint set (maybe it was already removed)");
			}

			network_connection_delete(server->client_list[client_index]);
			server->client_list[client_index] = NULL;
			success = TRUE;
			break;
		}
	}

	return success;
}

boolean network_connection_idle(
	struct network_connection *connection,
	unsigned long timeout,
	struct network_connection **new_client_connection)
{
	struct transport_address address;
	char buffer[DATAGRAM_MAXIMUM_SIZE + sizeof(unsigned long)];
	unsigned long time = system_milliseconds();
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 541, connection);

	SET_FLAG(connection->flags, _connection_going_stale_bit, FALSE);

	if (timeout)
	{
		if (time > connection->last_keep_alive_time + CONNECTION_GOING_STALE_MILLISECONDS)
		{
			SET_FLAG(connection->flags, _connection_going_stale_bit, TRUE);
		}

		if (time > connection->last_keep_alive_time + timeout)
		{
			if (global_connection_dont_timeout)
			{
				error(_error_silent, "dont timeout is active so not timing out of a connection");
				connection->last_keep_alive_time = time;
			}
			else
			{
				error(_error_silent, "timeout in network_connection_idle");
				success = FALSE;
			}
		}
	}
	else
	{
		connection->last_keep_alive_time = time;
	}

	if (success && TEST_FLAG(connection->flags, _connection_create_server_bit))
	{
		success = network_connection_idle_server_reliable_endpoint(
			(struct network_server_connection *)connection,
			new_client_connection);

		if (!success)
		{
			error(_error_silent, "network_connection_idle_server_reliable_endpoint failed");
		}
	}
	else if (success &&
		(TEST_FLAG(connection->flags, _connection_create_clientside_client_bit) ||
		TEST_FLAG(connection->flags, _connection_create_serverside_client_bit)))
	{
		success = network_connection_idle_client_reliable_endpoint(connection);

		if (!success)
		{
			error(_error_silent, "network_connection_idle_client_reliable_endpoint failed");
		}
	}

	if (success && connection->unreliable_endpoint)
	{
		long free_space = circular_queue_free_space(connection->unreliable_incoming_queue);

		while (success && free_space >= DATAGRAM_MAXIMUM_SIZE + sizeof(unsigned long))
		{
			long bytes_read;
			unsigned long ipv4_address;

			if (endpoint_connected(connection->unreliable_endpoint))
			{
				bytes_read = read_endpoint(connection->unreliable_endpoint, buffer, DATAGRAM_MAXIMUM_SIZE);

				if (bytes_read > 0)
				{
					if (get_endpoint_address(connection->unreliable_endpoint, &address) != _transport_error_none)
					{
						memset(&address, 0, sizeof(address));
						address.address_length = IPV4_ADDRESS_LENGTH;
					}

					network_connection_log_traffic_event(
						connection,
						_network_connection_traffic_event_datagram_received,
						bytes_read);
				}
			}
			else
			{
				bytes_read = read_from_endpoint(connection->unreliable_endpoint, buffer, DATAGRAM_MAXIMUM_SIZE, &address);

				if (bytes_read > 0)
				{
					network_connection_log_traffic_event(
						connection,
						_network_connection_traffic_event_datagram_received,
						bytes_read);
				}
			}

			match_vassert(
				"c:\\halo\\SOURCE\\networking\\network_connection.c",
				621,
				bytes_read <= DATAGRAM_MAXIMUM_SIZE,
				"endpoint read buffer overflowed");

			if (bytes_read <= 0)
			{
				break;
			}

			ipv4_address = address.address.ipv4_address;

			if (ipv4_address)
			{
				memcpy(buffer + bytes_read, &ipv4_address, sizeof(ipv4_address));
				success = circular_queue_queue_data(
					connection->unreliable_incoming_queue,
					buffer,
					bytes_read + sizeof(ipv4_address));
				match_vassert(
					"c:\\halo\\SOURCE\\networking\\network_connection.c",
					633,
					success,
					"circular_queue_queue_data() failed though it should have had enough room");
			}
			else
			{
				error(_error_silent, "datagram received from unknown address");
			}

			free_space = circular_queue_free_space(connection->unreliable_incoming_queue);
		}
	}

	return success;
}

void network_connection_get_address(
	struct network_connection *connection,
	struct transport_address *reliable_address,
	struct transport_address *unreliable_address)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 658, connection);

	if (reliable_address)
	{
		if (connection->reliable_endpoint)
		{
			if (get_endpoint_address(connection->reliable_endpoint, reliable_address) != _transport_error_none)
			{
				memset(reliable_address, 0, sizeof(*reliable_address));
				reliable_address->address_length = IPV4_ADDRESS_LENGTH;
			}
		}
		else
		{
			memset(reliable_address, 0, sizeof(*reliable_address));
			reliable_address->address_length = IPV4_ADDRESS_LENGTH;
		}
	}

	if (unreliable_address)
	{
		if (connection->unreliable_endpoint)
		{
			if (get_endpoint_address(connection->unreliable_endpoint, unreliable_address) != _transport_error_none)
			{
				memset(unreliable_address, 0, sizeof(*unreliable_address));
				unreliable_address->address_length = IPV4_ADDRESS_LENGTH;
			}
		}
		else
		{
			memset(unreliable_address, 0, sizeof(*unreliable_address));
			unreliable_address->address_length = IPV4_ADDRESS_LENGTH;
		}
	}

	return;
}

boolean network_connection_connect(
	struct network_connection *connection,
	struct transport_address const *remote_address,
	struct transport_connect_process **process_reference)
{
	short error_code;
	boolean success;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 704, connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 705, remote_address);

	if (!connection->reliable_endpoint && !connection->unreliable_endpoint)
	{
		success = FALSE;
	}
	else
	{
		success = TRUE;

		if (connection->unreliable_endpoint)
		{
			error_code = connect_endpoint(connection->unreliable_endpoint, remote_address);

			if (error_code != _transport_error_none)
			{
				success = FALSE;
			}
		}

		if (success && !connection->reliable_endpoint)
		{
			// only an unreliable endpoint, nothing left to connect
		}
		else if (success && process_reference &&
			(error_code = connect_endpoint_async(connection->reliable_endpoint, remote_address, process_reference)) != _transport_error_none &&
			error_code != _transport_result_connect_in_progress)
		{
			error(
				_error_silent,
				"connect_endpoint_async() returned error '%s'",
				transport_error_to_string(error_code));
			success = FALSE;
		}
		else if (!success)
		{
			error(
				_error_silent,
				"connect_endpoint() on unreliable endpoint returned error '%s'",
				transport_error_to_string(error_code));
			success = FALSE;
		}
		else if (process_reference)
		{
			success = TRUE;
		}
		else
		{
			error_code = connect_endpoint(connection->reliable_endpoint, remote_address);

			if (error_code != _transport_error_none)
			{
				error(
					_error_silent,
					"connect_endpoint() on reliable endpoint returned error '%s'",
					transport_error_to_string(error_code));
				success = FALSE;
			}
		}
	}

	return success;
}

void network_connection_set_connection_rejection_procedure(
	struct network_connection *connection,
	network_connection_rejection_procedure connection_rejection_procedure)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 792, connection);

	connection->connection_rejection_procedure = connection_rejection_procedure;

	return;
}

boolean network_connection_server_accept_client_connection(
	struct network_connection *server_connection,
	struct network_connection *client_connection)
{
	boolean success;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 804, server_connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 805, server_connection->flags&FLAG(_connection_create_server_bit));
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 806, client_connection);

	success = add_endpoint_to_set(
		client_connection->reliable_endpoint,
		((struct network_server_connection *)server_connection)->endpoint_set) == _transport_error_none;

	return success;
}

boolean network_connection_active(
	struct network_connection *connection)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 816, connection);

	return !TEST_FLAG(connection->flags, _connection_closed_bit);
}

boolean network_connection_going_stale(
	struct network_connection *connection)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 824, connection);

	return TEST_FLAG(connection->flags, _connection_going_stale_bit);
}

void network_connection_keep_alive(
	struct network_connection *connection)
{
	connection->last_keep_alive_time = system_milliseconds();

	return;
}

boolean network_connection_disconnect(
	struct network_connection *connection)
{
	struct transport_address address;
	boolean success = TRUE;

	if (network_connection_connected(connection))
	{
		if (TEST_FLAG(connection->flags, _connection_create_clientside_client_bit) ||
			TEST_FLAG(connection->flags, _connection_create_serverside_client_bit))
		{
			network_connection_idle_client_reliable_endpoint(connection);
		}

		disconnect_endpoint(connection->reliable_endpoint);
	}

	if (connection->unreliable_endpoint && connection->well_known_port)
	{
		address.address_length = IPV4_ADDRESS_LENGTH;
		address.address.ipv4_address = 0;
		address.port = connection->well_known_port;
		delete_transport_endpoint(connection->unreliable_endpoint);
		connection->unreliable_endpoint = create_transport_endpoint(_transport_type_udp);
		success =
			connection->unreliable_endpoint &&
			bind_endpoint(connection->unreliable_endpoint, &address) == _transport_error_none &&
			set_endpoint_blocking(connection->unreliable_endpoint, FALSE) == _transport_error_none;
	}

	return success;
}

/* ---------- private code */

static struct network_connection *network_connection_create_client_from_endpoint(
	struct transport_endpoint *reliable_endpoint)
{
	struct network_connection *connection;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 837, reliable_endpoint);

	connection = match_calloc(
		"c:\\halo\\SOURCE\\networking\\network_connection.c",
		839,
		1,
		sizeof(struct network_connection));

	if (connection)
	{
		connection->flags = FLAG(_connection_create_serverside_client_bit);
		connection->reliable_endpoint = reliable_endpoint;
		connection->reliable_incoming_queue = circular_queue_new("incoming-reliable", CLIENT_RELIABLE_INCOMING_QUEUE_SIZE);

		if (!connection->reliable_incoming_queue)
		{
			network_connection_delete(connection);
			connection = NULL;
		}
		else
		{
			network_connection_log_traffic_event(connection, _network_connection_traffic_event_open, 1);
		}
	}

	return connection;
}

static boolean network_client_reliable_connection_read(
	struct network_connection *connection,
	void *message,
	word *buffer_size,
	struct transport_address *source_address)
{
	message_header header;
	boolean success = FALSE;
	boolean reset_queue = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 881, connection && connection->reliable_incoming_queue);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 882, message);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 883, buffer_size);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 884, *buffer_size>sizeof(message_header));

	if (circular_queue_dequeue_data(connection->reliable_incoming_queue, &header, sizeof(header), FALSE))
	{
		word message_size;

		byte_swap_message_header(&header, _byte_order_host);
		message_size = GET_MESSAGE_SIZE(header);

		if (message_size > RELIABLE_MESSAGE_MAXIMUM_SIZE)
		{
			error(_error_silent, "got an unusually large message (#d bytes); resetting reliable incoming queue", message_size);
			reset_queue = TRUE;
		}
		else if (message_size > *buffer_size)
		{
			error(
				_error_silent,
				"packet in queue is #%d bytes, but we can only handle #%d bytes!; resetting reliable incoming queue",
				message_size,
				*buffer_size);
			reset_queue = TRUE;
		}
		else if (message_size <= circular_queue_size(connection->reliable_incoming_queue) &&
			circular_queue_dequeue_data(connection->reliable_incoming_queue, message, message_size, TRUE))
		{
			*(message_header *)message = header;
			match_vassert(
				"c:\\halo\\SOURCE\\networking\\network_connection.c",
				916,
				!TEST_FLAG(header, _message_flag_encrypted_bit),
				"encryption should not be active");

			if (source_address &&
				get_endpoint_address(connection->reliable_endpoint, source_address) != _transport_error_none)
			{
				memset(source_address, 0, sizeof(*source_address));
				source_address->address_length = IPV4_ADDRESS_LENGTH;
			}

			*buffer_size = message_size;
			success = TRUE;
			connection->stream_messages_received++;
		}

		if (reset_queue)
		{
			circular_queue_reset(connection->reliable_incoming_queue);
		}
	}

	return success;
}

static boolean network_client_unreliable_connection_read(
	struct network_connection *connection,
	void *message,
	word *buffer_size,
	struct transport_address *source_address)
{
	message_header header;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 954, connection && connection->unreliable_incoming_queue && !(connection->flags&FLAG(_connection_create_serverside_client_bit)));
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 955, message);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 956, buffer_size);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 957, *buffer_size>sizeof(message_header));

	if (circular_queue_dequeue_data(connection->unreliable_incoming_queue, &header, sizeof(header), FALSE))
	{
		word message_size;
		unsigned long ipv4_address;

		byte_swap_message_header(&header, _byte_order_host);
		message_size = GET_MESSAGE_SIZE(header);

		if (message_size > DATAGRAM_MAXIMUM_SIZE)
		{
			error(_error_silent, "got an unusually large datagram (#d bytes); resetting unreliable incoming queue", message_size);
		}
		else if (message_size > *buffer_size)
		{
			error(
				_error_silent,
				"packet in queue is #%d bytes, but we can only handle #%d bytes!; resetting unreliable incoming queue",
				message_size,
				*buffer_size);
		}
		else if (message_size + sizeof(ipv4_address) <= (unsigned long)circular_queue_size(connection->unreliable_incoming_queue) &&
			circular_queue_dequeue_data(connection->unreliable_incoming_queue, message, message_size, TRUE) &&
			circular_queue_dequeue_data(connection->unreliable_incoming_queue, &ipv4_address, sizeof(ipv4_address), TRUE))
		{
			*(message_header *)message = header;
			match_vassert(
				"c:\\halo\\SOURCE\\networking\\network_connection.c",
				991,
				!TEST_FLAG(header, _message_flag_encrypted_bit),
				"encryption should not be active");

			if (source_address)
			{
				source_address->address.ipv4_address = ipv4_address;
				source_address->port = 0;
				source_address->address_length = IPV4_ADDRESS_LENGTH;
			}

			*buffer_size = message_size;
			success = TRUE;
		}
		else
		{
			error(
				_error_silent,
				"partial datagram in queue (#%d of #%d bytes); resetting queue",
				circular_queue_size(connection->unreliable_incoming_queue),
				message_size);
		}

		if (!success)
		{
			circular_queue_reset(connection->unreliable_incoming_queue);
		}
	}

	return success;
}

static boolean network_connection_idle_server_reliable_endpoint(
	struct network_server_connection *connection,
	struct network_connection **new_client_connection)
{
	struct transport_endpoint *endpoint;
	short error_code;
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1029, connection != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1030, connection->connection.reliable_endpoint);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1031, connection->endpoint_set);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1032, new_client_connection);

	*new_client_connection = NULL;
	error_code = poll_endpoint_set(connection->endpoint_set, 0);

	if (error_code == _transport_error_none)
	{
		rewind_endpoint_set(connection->endpoint_set);

		while (success &&
			(endpoint = get_next_endpoint_from_set(connection->endpoint_set)) != NULL &&
			error_code == _transport_error_none)
		{
			if (endpoint_readable(endpoint, 0))
			{
				if (endpoint == connection->connection.reliable_endpoint)
				{
					if (connection->allow_client_connections &&
						count_endpoints_in_set(connection->endpoint_set) < MAXIMUM_SERVER_ENDPOINTS)
					{
						struct network_connection *client_connection;
						struct transport_endpoint *client_endpoint = accept_endpoint(endpoint);

						if (client_endpoint &&
							set_endpoint_blocking(client_endpoint, FALSE) == _transport_error_none &&
							(client_connection = network_connection_create_client_from_endpoint(client_endpoint)) != NULL)
						{
							long client_index;

							for (client_index = 0; client_index < MAXIMUM_SERVER_CLIENT_CONNECTIONS; client_index++)
							{
								if (!connection->client_list[client_index])
								{
									*new_client_connection = client_connection;
									connection->client_list[client_index] = client_connection;
									break;
								}
							}

							if (client_index >= MAXIMUM_SERVER_CLIENT_CONNECTIONS)
							{
								error(_error_silent, "error adding new client");
							}
						}
						else
						{
							error(_error_silent, "accept_endpoint() returned NULL");
						}
					}
					else if (connection->connection.connection_rejection_procedure)
					{
						struct transport_endpoint *rejected_endpoint = accept_endpoint(endpoint);

						if (rejected_endpoint)
						{
							connection->connection.connection_rejection_procedure(rejected_endpoint);
							delete_transport_endpoint(rejected_endpoint);
						}
					}
					else
					{
						error_code = reject_endpoint(endpoint);
					}
				}
				else
				{
					long client_index;

					for (client_index = 0; client_index < MAXIMUM_SERVER_CLIENT_CONNECTIONS; client_index++)
					{
						if (connection->client_list[client_index] &&
							connection->client_list[client_index]->reliable_endpoint == endpoint)
						{
							success = network_connection_idle_client_reliable_endpoint(
								connection->client_list[client_index]);

							if (!success)
							{
								if (remove_endpoint_from_set(
									connection->client_list[client_index]->reliable_endpoint,
									connection->endpoint_set) != _transport_error_none)
								{
									error(_error_silent, "failed to remove a client endpoint from the server's endpoint set");
								}

								SET_FLAG(connection->client_list[client_index]->flags, _connection_closed_bit, TRUE);
								success = TRUE;
							}

							break;
						}
					}

					match_vassert(
						"c:\\halo\\SOURCE\\networking\\network_connection.c",
						1129,
						client_index < MAXIMUM_SERVER_CLIENT_CONNECTIONS,
						"rogue endpoint connected to the server");
				}
			}
		}
	}
	else if (error_code != _transport_result_poll_timeout)
	{
		error(_error_silent, "poll_endpoint_set() returned error '%s'", transport_error_to_string(error_code));
		success = FALSE;
	}

	return success;
}

static boolean network_connection_idle_client_reliable_endpoint(
	struct network_connection *connection)
{
	char buffer[RELIABLE_MESSAGE_MAXIMUM_SIZE];
	long free_space;
	unsigned long start_time = system_milliseconds();
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1153, connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1154, connection->reliable_endpoint);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1155, connection->reliable_incoming_queue);

	free_space = circular_queue_free_space(connection->reliable_incoming_queue);

	while (success && endpoint_readable(connection->reliable_endpoint, 0) && free_space > 0)
	{
		long bytes_read;

		if (free_space >= RELIABLE_MESSAGE_MAXIMUM_SIZE)
		{
			free_space = RELIABLE_MESSAGE_MAXIMUM_SIZE;
		}

		bytes_read = read_endpoint(connection->reliable_endpoint, buffer, free_space);

		if (bytes_read > 0)
		{
			connection->last_keep_alive_time = system_milliseconds();
			network_connection_log_traffic_event(
				connection,
				_network_connection_traffic_event_stream_bytes_received,
				bytes_read);

			if (!circular_queue_queue_data(connection->reliable_incoming_queue, buffer, bytes_read))
			{
				error(_error_silent, "circular_queue_queue_data() failed");
				success = FALSE;
			}

			free_space = circular_queue_free_space(connection->reliable_incoming_queue);
		}
		else
		{
			if (bytes_read == _transport_result_operation_would_block)
			{
				break;
			}

			if (bytes_read == _transport_error_connection_lost)
			{
				SET_FLAG(connection->flags, _connection_closed_bit, TRUE);
			}
			else if (bytes_read != 0)
			{
				error(
					_error_silent,
					"error '%s' reading from client reliable endpoint",
					transport_error_to_string((short)bytes_read));
			}
			else
			{
				error(_error_silent, "client reliable connection lost");
			}

			success = FALSE;
		}
	}

	if (system_milliseconds() - start_time > CONNECTION_BLOCKED_WARNING_MILLISECONDS)
	{
		error(_error_silent, "blocked in network_connection_idle_client_reliable_endpoint");
	}

	return success;
}

static void network_connection_log_traffic_event(
	struct network_connection *connection,
	long event,
	long amount)
{
	double seconds;
	struct transport_address address;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1228, connection);

	if (amount > 0)
	{
		switch (event)
		{
		case _network_connection_traffic_event_open:
			if (get_endpoint_address(connection->reliable_endpoint, &address) != _transport_error_none &&
				get_endpoint_address(connection->unreliable_endpoint, &address) != _transport_error_none)
			{
				memset(&address, 0, sizeof(address));
				address.address_length = IPV4_ADDRESS_LENGTH;
			}

			{
				long index;
				char traffic_log_name[TRAFFIC_LOG_NAME_LENGTH] = "";

				strcpy(traffic_log_name, transport_address_to_string(&address));

				for (index = 0; traffic_log_name[index]; index++)
				{
					if (traffic_log_name[index] == ':')
					{
						traffic_log_name[index] = 0;
						break;
					}
				}

				strcat(traffic_log_name, "_traffic_log.xls");
				connection->traffic_log = fopen(traffic_log_name, "w");

				if (connection->traffic_log)
				{
					fprintf(connection->traffic_log, "time, seconds\tudp bytes out\tudp bytes in\ttcp bytes out\ttcp bytes in\n");
					fflush(connection->traffic_log);
				}

				connection->traffic_log_start_time = system_milliseconds();
			}
			break;

		case _network_connection_traffic_event_close:
			if (connection->traffic_log)
			{
				if (get_endpoint_address(connection->reliable_endpoint, &address) != _transport_error_none)
				{
					memset(&address, 0, sizeof(address));
					address.address_length = IPV4_ADDRESS_LENGTH;
				}

				fprintf(connection->traffic_log, "\n\n");
				fprintf(connection->traffic_log, "datagrams sent\t%ld\n", connection->datagrams_sent);
				fprintf(connection->traffic_log, "datagrams received\t%ld\n", connection->datagrams_received);
				fprintf(connection->traffic_log, "stream messages sent\t%ld\n", connection->stream_messages_sent);
				fprintf(connection->traffic_log, "stream messages received\t%ld\n", connection->stream_messages_received);
				fprintf(connection->traffic_log, "datagram overhead (headers)\t%ld\tbytes per packet\n", UDP_HEADER_OVERHEAD);
				fprintf(connection->traffic_log, "stream overhead (headers)\t%ld\tbytes per chunk\n", TCP_HEADER_OVERHEAD);
				fprintf(connection->traffic_log, "NOTE: header overhead is not included in the above traffic graph\n");
				fprintf(connection->traffic_log, "connection lifetime\t%g\tseconds\n", TRAFFIC_LOG_SECONDS(connection));
				fprintf(connection->traffic_log, "connection's remote address was: %s\n", transport_address_to_string(&address));

				fclose(connection->traffic_log);

				connection->traffic_log = NULL;
			}
			break;

		case _network_connection_traffic_event_datagram_sent:
			if (connection->traffic_log)
			{
				seconds = TRAFFIC_LOG_SECONDS(connection);
				fprintf(connection->traffic_log, "%g\t%ld\t%ld\t%ld\t%ld\n", seconds, amount, 0, 0, 0);
				fflush(connection->traffic_log);
			}

			connection->datagrams_sent++;
			break;

		case _network_connection_traffic_event_datagram_received:
			if (connection->traffic_log)
			{
				seconds = TRAFFIC_LOG_SECONDS(connection);
				fprintf(connection->traffic_log, "%g\t%ld\t%ld\t%ld\t%ld\n", seconds, 0, amount, 0, 0);
				fflush(connection->traffic_log);
			}

			connection->datagrams_received++;
			break;

		case _network_connection_traffic_event_stream_bytes_sent:
			if (connection->traffic_log)
			{
				seconds = TRAFFIC_LOG_SECONDS(connection);
				fprintf(connection->traffic_log, "%g\t%ld\t%ld\t%ld\t%ld\n", seconds, 0, 0, amount, 0);
				fflush(connection->traffic_log);
			}
			break;

		case _network_connection_traffic_event_stream_bytes_received:
			if (connection->traffic_log)
			{
				seconds = TRAFFIC_LOG_SECONDS(connection);
				fprintf(connection->traffic_log, "%g\t%ld\t%ld\t%ld\t%ld\n", seconds, 0, 0, 0, amount);
				fflush(connection->traffic_log);
			}
			break;

		case _network_connection_traffic_event_stream_message_sent:
			connection->stream_messages_sent++;
			break;

		case _network_connection_traffic_event_stream_message_received:
			connection->stream_messages_received++;
			break;

		default:
			match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1367, !"unknown traffic event");
			break;
		}
	}

	return;
}
