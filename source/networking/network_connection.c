/*
NETWORK_CONNECTION.C

- a connection owns two endpoints: a reliable one over tcp and an unreliable one over udp

- a server connection additionally owns an endpoint set and the list of client connections
it has accepted, and embeds the plain connection as its first member so the two can be
used interchangeably

- writes go out on the reliable endpoint unless the caller asks for the unreliable one
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries_windows.h"
#include "errors.h"
#include "network_connection.h"
#include "circular_queue.h"

/* ---------- constants */

enum
{
	MAXIMUM_RESERVED_NETWORK_PORT= 1023,

	MAXIMUM_CLIENT_CONNECTIONS= 4,
	MAXIMUM_SERVER_ENDPOINTS= MAXIMUM_CLIENT_CONNECTIONS+1, // the listening endpoint plus the clients

	RELIABLE_INCOMING_QUEUE_SIZE= 0x8000,
	SERVER_UNRELIABLE_INCOMING_QUEUE_SIZE= 6400,
	CLIENT_UNRELIABLE_INCOMING_QUEUE_SIZE= 1600,

	DATAGRAM_HEADER_OVERHEAD= 28, // bytes per packet, reported in the traffic log
	STREAM_HEADER_OVERHEAD= 40, // bytes per chunk

	MAXIMUM_IDLE_BLOCK_MILLISECONDS= MILLISECONDS_PER_SECOND
};

// bits 0..2 are the create flags from network_connection.h; the names of 4 and 5 are not
// (bit 4 is set when a client's reliable endpoint dies, bit 5 is recomputed
// every idle from the keep-alive age)
enum
{
	_connection_inactive_bit= 4,
	_connection_going_stale_bit,
	NUMBER_OF_CONNECTION_FLAGS
};

// the traffic log columns, in the order network_connection_log_traffic_event() switches on
enum
{
	_traffic_event_connection_created= 0,
	_traffic_event_connection_deleted,
	_traffic_event_udp_bytes_out,
	_traffic_event_udp_bytes_in,
	_traffic_event_tcp_bytes_out,
	_traffic_event_tcp_bytes_in,
	_traffic_event_stream_message_sent,
	_traffic_event_stream_message_received,
	NUMBER_OF_TRAFFIC_EVENTS
};

/* ---------- macros */

/* ---------- structures */

struct network_connection
{
	transport_endpoint_ref reliable_endpoint; // 0x00
	transport_endpoint_ref unreliable_endpoint; // 0x04
	unsigned long time_of_last_keep_alive; // 0x08
	connection_rejection_procedure rejection_procedure; // 0x0C
	struct circular_queue *reliable_incoming_queue; // 0x10
	struct circular_queue *unreliable_incoming_queue; // 0x14
	FILE *traffic_log; // 0x18
	unsigned long traffic_log_start_time; // 0x1C
	long datagrams_sent; // 0x20
	long datagrams_received; // 0x24
	long stream_messages_sent; // 0x28
	long stream_messages_received; // 0x2C
	unsigned long flags; // 0x30
	word well_known_port; // 0x34
	word pad;
}; // 0x38

struct network_server_connection
{
	struct network_connection connection; // 0x00
	struct endpoint_set *endpoint_set; // 0x38
	struct network_connection *client_list[MAXIMUM_CLIENT_CONNECTIONS]; // 0x3C
	boolean allow_client_connections; // 0x4C
}; // 0x50

/* ---------- prototypes */

static struct network_connection *network_connection_create_client_from_endpoint(transport_endpoint_ref reliable_endpoint);

static boolean network_client_reliable_connection_read(struct network_connection *connection, message_header *message, word *buffer_size, transport_address *source_address);
static boolean network_client_unreliable_connection_read(struct network_connection *connection, message_header *message, word *buffer_size, transport_address *source_address);

static boolean network_connection_idle_server_reliable_endpoint(struct network_server_connection *connection, struct network_connection **new_client_connection);
static boolean network_connection_idle_client_reliable_endpoint(struct network_connection *connection);

static void network_connection_log_traffic_event(struct network_connection *connection, long event_type, long size);

/* ---------- globals */

boolean global_connection_dont_timeout= FALSE;

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
	struct network_connection *connection= NULL;
	long reliable_queue_size, unreliable_queue_size;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 157, (flags&FLAG(_connection_create_server_bit))|| (flags&FLAG(_connection_create_clientside_client_bit)));

	if (flags&FLAG(_connection_create_server_bit))
	{
		struct network_server_connection *server;

		match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 163, well_known_port > MAXIMUM_RESERVED_NETWORK_PORT);

		server= (struct network_server_connection *)debug_malloc(sizeof(struct network_server_connection), TRUE, "c:\\halo\\SOURCE\\networking\\network_connection.c", 165);
		if (server)
		{
			server->allow_client_connections= TRUE;
			server->endpoint_set= create_endpoint_set(MAXIMUM_SERVER_ENDPOINTS);
			if (server->endpoint_set)
			{
				connection= &server->connection;
				reliable_queue_size= 0;
				unreliable_queue_size= SERVER_UNRELIABLE_INCOMING_QUEUE_SIZE;
			}
			else network_connection_delete(&server->connection);
		}
	}
	else if (flags&FLAG(_connection_create_clientside_client_bit))
	{

		connection= (struct network_connection *)debug_malloc(sizeof(struct network_connection), TRUE, "c:\\halo\\SOURCE\\networking\\network_connection.c", 182);
		if (connection)
		{
			reliable_queue_size= RELIABLE_INCOMING_QUEUE_SIZE;
			unreliable_queue_size= CLIENT_UNRELIABLE_INCOMING_QUEUE_SIZE;
		}
	}

	if (connection)
	{
		boolean success= TRUE;

		connection->time_of_last_keep_alive= system_milliseconds();
		connection->flags= flags;

		connection->reliable_endpoint= create_transport_endpoint(_transport_type_tcp);
		if (!connection->reliable_endpoint) success= FALSE;

		if (success && (flags&FLAG(_connection_create_server_bit)))
		{
			transport_address address= {0};

			address.address_length= IPV4_ADDRESS_LENGTH;
			address.port= well_known_port;
			if ((bind_endpoint(connection->reliable_endpoint, &address) != _transport_error_none) ||
				(set_endpoint_blocking(connection->reliable_endpoint, FALSE) != _transport_error_none) ||
				(listen_endpoint(connection->reliable_endpoint) != _transport_error_none) ||
				(add_endpoint_to_set(connection->reliable_endpoint, ((struct network_server_connection *)connection)->endpoint_set) != _transport_error_none))
			{
				success= FALSE;
			}
		}

		if (success)
		{
			connection->unreliable_endpoint= create_transport_endpoint(_transport_type_udp);
			if (connection->unreliable_endpoint)
			{
				transport_address address;

				address.address_length= IPV4_ADDRESS_LENGTH;
				address.address.ipv4_address= 0;
				address.port= well_known_port;
				connection->well_known_port= well_known_port;
				if ((bind_endpoint(connection->unreliable_endpoint, &address) != _transport_error_none) ||
					(set_endpoint_blocking(connection->unreliable_endpoint, FALSE) != _transport_error_none))
				{
					success= FALSE;
				}
			}
			else
			{
				success= FALSE;
			}
		}

		if (success && reliable_queue_size)
		{
			connection->reliable_incoming_queue= circular_queue_new("incoming-reliable", reliable_queue_size);
			if (!connection->reliable_incoming_queue) success= FALSE;
		}

		if (success && unreliable_queue_size)
		{
			connection->unreliable_incoming_queue= circular_queue_new("incoming-unreliable", unreliable_queue_size);
			if (!connection->unreliable_incoming_queue) success= FALSE;
		}

		if (!success)
		{
			network_connection_delete(connection);
			connection= NULL;
		}
		else
		{
			network_connection_log_traffic_event(connection, _traffic_event_connection_created, 1);
		}
	}

	return connection;
}

void network_connection_delete(
	struct network_connection *connection)
{
	if (connection)
	{
		network_connection_log_traffic_event(connection, _traffic_event_connection_deleted, 1);

		if (connection->reliable_endpoint) delete_transport_endpoint(connection->reliable_endpoint);
		if (connection->unreliable_endpoint) delete_transport_endpoint(connection->unreliable_endpoint);
		if (connection->reliable_incoming_queue) circular_queue_delete(connection->reliable_incoming_queue);
		if (connection->unreliable_incoming_queue) circular_queue_delete(connection->unreliable_incoming_queue);

		if (connection->flags&FLAG(_connection_create_server_bit))
		{
			struct network_server_connection *server= (struct network_server_connection *)connection;

			if (server->client_list)
			{
				int i;

				for (i= 0; i<MAXIMUM_CLIENT_CONNECTIONS; i++)
				{
					if (server->client_list[i])
					{
						if (server->endpoint_set) remove_endpoint_from_set(server->client_list[i]->reliable_endpoint, server->endpoint_set);
						network_connection_delete(server->client_list[i]);
					}
				}
			}
			if (server->endpoint_set) delete_endpoint_set(server->endpoint_set);
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

	((struct network_server_connection *)server_connection)->allow_client_connections= allow_client_connections;

	return;
}

boolean network_connection_connected(
	struct network_connection *connection)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 348, connection);

	return ((connection->flags&FLAG(_connection_create_clientside_client_bit)) || (connection->flags&FLAG(_connection_create_serverside_client_bit))) &&
		connection->reliable_endpoint && endpoint_connected(connection->reliable_endpoint);
}

#pragma inline_depth(0) // the 2342 object calls network_connection_log_traffic_event() for the datagram writes; 3823 would inline them
boolean network_connection_write(
	struct network_connection *connection,
	message_header *message,
	unsigned short buffer_size,
	transport_address *dest_address,
	boolean reliable)
{
	long bytes_written= 0;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 368, message);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 369, buffer_size);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 370, connection);

	match_vassert("c:\\halo\\SOURCE\\networking\\network_connection.c", 372, GET_MESSAGE_SIZE(*message)==buffer_size, "bad message or buffer_size parameter");

	byte_swap_message_header(message, _byte_order_network);

	if (connection->flags&FLAG(_connection_create_server_bit))
	{

		match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 379, !reliable);
		match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 380, dest_address);

		if (buffer_size > DATAGRAM_MAXIMUM_SIZE)
		{
			error(_error_silent, "buffer size was %d max is %d", buffer_size, DATAGRAM_MAXIMUM_SIZE);
		}
		match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 386, buffer_size <= DATAGRAM_MAXIMUM_SIZE);

		bytes_written= write_to_endpoint(connection->unreliable_endpoint, message, buffer_size, dest_address);
		network_connection_log_traffic_event(connection, _traffic_event_udp_bytes_out, buffer_size);
	}
	else
	{
		if (reliable)
		{
			long result;

			match_vassert("c:\\halo\\SOURCE\\networking\\network_connection.c", 398, buffer_size <= MESSAGE_MAXIMUM_SIZE, "message size exceeds maximum allowed size");

			match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 400, (connection->flags&FLAG(_connection_create_clientside_client_bit)) || (connection->flags&FLAG(_connection_create_serverside_client_bit)));

retry:
			result= write_endpoint(connection->reliable_endpoint, message, buffer_size);
			if (result > 0)
			{
				bytes_written= 1;
				if (connection->traffic_log)
				{
					unsigned long elapsed_time= system_milliseconds()-connection->traffic_log_start_time;
					double seconds= elapsed_time*0.001;

					fprintf(connection->traffic_log, "%g\t%ld\t%ld\t%ld\t%ld\n", seconds, 0, 0, result, 0);
					fflush(connection->traffic_log);
				}
				connection->stream_messages_sent+= 1;
			}
			else if (result==_transport_result_operation_would_block)
			{
				goto retry;
			}
			else
			{
				error(_error_silent, "client call to write_endpoint() returned error '%s'", transport_error_to_string(result));
			}
		}
		else
		{
			if (connection->unreliable_endpoint)
			{
				match_vassert("c:\\halo\\SOURCE\\networking\\network_connection.c", 432, buffer_size <= DATAGRAM_MAXIMUM_SIZE, "message size exceeds maximum allowed size");

				if (!dest_address)
				{
					if (endpoint_connected(connection->unreliable_endpoint))
					{
						write_endpoint(connection->unreliable_endpoint, message, buffer_size);
						network_connection_log_traffic_event(connection, _traffic_event_udp_bytes_out, buffer_size);
					}
				}
				else
				{
					write_to_endpoint(connection->unreliable_endpoint, message, buffer_size, dest_address);
					network_connection_log_traffic_event(connection, _traffic_event_udp_bytes_out, buffer_size);
				}
			}

		}
	}

	if (!reliable) return TRUE;

	return (boolean)(bytes_written > 0);
}
#pragma inline_depth()

boolean network_connection_read(
	struct network_connection *connection,
	message_header *message,
	word *buffer_size,
	transport_address *source_address)
{
	boolean success;

	if (connection->flags&FLAG(_connection_create_server_bit))
	{
		return network_client_unreliable_connection_read(connection, message, buffer_size, source_address);
	}

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 480, connection->flags&FLAG(_connection_create_clientside_client_bit) || connection->flags&FLAG(_connection_create_serverside_client_bit));

	success= network_client_reliable_connection_read(connection, message, buffer_size, source_address);
	if (!success && (connection->flags&FLAG(_connection_create_clientside_client_bit)))
	{
		success= network_client_unreliable_connection_read(connection, message, buffer_size, source_address);
	}

	return success;
}

boolean network_server_close_client_connection(
	struct network_connection *server_connection,
	struct network_connection *client_connection)
{
	struct network_server_connection *server= (struct network_server_connection *)server_connection;
	int i;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 503, server_connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 504, client_connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 505, server_connection->flags & FLAG(_connection_create_server_bit));
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 506, server->endpoint_set);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 507, server->client_list);

	for (i= 0; i<MAXIMUM_SERVER_ENDPOINTS; i++)
	{
		if (server->client_list[i] && (server->client_list[i]==client_connection))
		{
			if (client_connection->reliable_endpoint && (remove_endpoint_from_set(server->client_list[i]->reliable_endpoint, server->endpoint_set) != _transport_error_none))
			{
				error(_error_silent, "failed to remove a client endpoint from the server's endpoint set (maybe it was already removed)");
			}
			network_connection_delete(server->client_list[i]);
			server->client_list[i]= NULL;

			return TRUE;
		}
	}

	return FALSE;
}

boolean network_connection_idle(
	struct network_connection *connection,
	long timeout,
	struct network_connection **new_client_connection)
{
	unsigned long now= system_milliseconds();
	boolean success= TRUE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 541, connection);

	connection->flags&= ~FLAG(_connection_going_stale_bit);
	if (timeout)
	{
		if (now > connection->time_of_last_keep_alive+_connection_timeout_fast)
		{
			connection->flags|= FLAG(_connection_going_stale_bit);
		}
		if (now > connection->time_of_last_keep_alive+timeout)
		{
			if (global_connection_dont_timeout)
			{
				error(_error_silent, "dont timeout is active so not timing out of a connection");
				connection->time_of_last_keep_alive= now;
			}
			else
			{
				error(_error_silent, "timeout in network_connection_idle");
				return FALSE;
			}
		}
	}
	else
	{
		connection->time_of_last_keep_alive= now;
	}

	if (connection->flags&FLAG(_connection_create_server_bit))
	{
		success= network_connection_idle_server_reliable_endpoint((struct network_server_connection *)connection, new_client_connection);
		if (!success)
		{
			error(_error_silent, "network_connection_idle_server_reliable_endpoint failed");
			goto done;
		}
	}
	else if ((connection->flags&FLAG(_connection_create_clientside_client_bit)) || (connection->flags&FLAG(_connection_create_serverside_client_bit)))
	{
		success= network_connection_idle_client_reliable_endpoint(connection);
		if (!success)
		{
			error(_error_silent, "network_connection_idle_client_reliable_endpoint failed");
			goto done;
		}
	}

	if (connection->unreliable_endpoint)
	{
		long free_space= circular_queue_free_space(connection->unreliable_incoming_queue);

		while (success && (free_space >= DATAGRAM_MAXIMUM_SIZE+sizeof(unsigned long)))
		{
			char buffer[DATAGRAM_MAXIMUM_SIZE+sizeof(unsigned long)];
			transport_address address;
			long bytes_read;
			unsigned long ipv4_address;
			if (endpoint_connected(connection->unreliable_endpoint))
			{
				bytes_read= read_endpoint(connection->unreliable_endpoint, buffer, DATAGRAM_MAXIMUM_SIZE);
				if (bytes_read > 0)
				{
					if (get_endpoint_address(connection->unreliable_endpoint, &address) != _transport_error_none)
					{
						memset(&address, 0, sizeof(address));
						address.address_length= IPV4_ADDRESS_LENGTH;
					}
					network_connection_log_traffic_event(connection, _traffic_event_udp_bytes_in, bytes_read);
				}
			}
			else
			{
				bytes_read= read_from_endpoint(connection->unreliable_endpoint, buffer, DATAGRAM_MAXIMUM_SIZE, &address);
				if (bytes_read > 0)
				{
					network_connection_log_traffic_event(connection, _traffic_event_udp_bytes_in, bytes_read);
				}
			}

			match_vassert("c:\\halo\\SOURCE\\networking\\network_connection.c", 621, bytes_read <= DATAGRAM_MAXIMUM_SIZE, "endpoint read buffer overflowed");

			if (bytes_read > 0)
			{
				ipv4_address= address.address.ipv4_address;

				if (ipv4_address)
				{
					memcpy(buffer+bytes_read, &ipv4_address, sizeof(ipv4_address));
					success= circular_queue_queue_data(connection->unreliable_incoming_queue, buffer, bytes_read+sizeof(unsigned long));

					match_vassert("c:\\halo\\SOURCE\\networking\\network_connection.c", 633, success, "circular_queue_queue_data() failed though it should have had enough room");
				}
				else
				{
					error(_error_silent, "datagram received from unknown address");
				}
				free_space= circular_queue_free_space(connection->unreliable_incoming_queue);
			}
			else
			{
				break;
			}
		}
	}

done:
	return success;
}

void network_connection_get_address(
	struct network_connection *connection,
	transport_address *remote_address,
	transport_address *local_address)
{

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 658, connection);

	if (remote_address)
	{
		if (!connection->reliable_endpoint || (get_endpoint_address(connection->reliable_endpoint, remote_address) != _transport_error_none))
		{
			memset(remote_address, 0, sizeof(transport_address));
			remote_address->address_length= IPV4_ADDRESS_LENGTH;
		}
	}

	if (local_address)
	{
		if (!connection->unreliable_endpoint || (get_endpoint_address(connection->unreliable_endpoint, local_address) != _transport_error_none))
		{
			memset(local_address, 0, sizeof(transport_address));
			local_address->address_length= IPV4_ADDRESS_LENGTH;
		}
	}

	return;
}

boolean network_connection_connect(
	struct network_connection *connection,
	transport_address *remote_address,
	transport_connect_process_ref *connect_process)
{
	boolean success;
	short error_code;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 704, connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 705, remote_address);

	if (!connection->reliable_endpoint && !connection->unreliable_endpoint) return FALSE;

	success= TRUE;

	if (connection->unreliable_endpoint)
	{
		error_code= connect_endpoint(connection->unreliable_endpoint, remote_address);
		if (error_code != _transport_error_none)
		{
			error(_error_silent, "connect_endpoint() on unreliable endpoint returned error '%s'", transport_error_to_string(error_code));
			return FALSE;
		}
	}

	if (connection->reliable_endpoint)
	{
		if (connect_process)
		{
			error_code= connect_endpoint_async(connection->reliable_endpoint, remote_address, connect_process);
			if ((error_code==_transport_error_none) || (error_code==_transport_result_connect_in_progress)) return TRUE;

			error(_error_silent, "connect_endpoint_async() returned error '%s'", transport_error_to_string(error_code));
			success= FALSE;
		}
		else
		{
			error_code= connect_endpoint(connection->reliable_endpoint, remote_address);
			if (error_code != _transport_error_none)
			{
				error(_error_silent, "connect_endpoint() on reliable endpoint returned error '%s'", transport_error_to_string(error_code));
				success= FALSE;
			}
		}
	}

	return success;
}

boolean network_connection_disconnect(
	struct network_connection *connection)
{
	boolean success= TRUE;

	if (network_connection_connected(connection))
	{
		if ((connection->flags&FLAG(_connection_create_clientside_client_bit)) || (connection->flags&FLAG(_connection_create_serverside_client_bit)))
		{
			network_connection_idle_client_reliable_endpoint(connection);
		}
		disconnect_endpoint(connection->reliable_endpoint);
	}

	if (connection->unreliable_endpoint && connection->well_known_port)
	{
		transport_address address;

		address.address_length= IPV4_ADDRESS_LENGTH;
		address.address.ipv4_address= 0;
		address.port= connection->well_known_port;

		delete_transport_endpoint(connection->unreliable_endpoint);
		connection->unreliable_endpoint= create_transport_endpoint(_transport_type_udp);
		success= connection->unreliable_endpoint &&
			(bind_endpoint(connection->unreliable_endpoint, &address)==_transport_error_none) &&
			(set_endpoint_blocking(connection->unreliable_endpoint, FALSE)==_transport_error_none);
	}

	return success;
}

void network_connection_set_connection_rejection_procedure(
	struct network_connection *connection,
	connection_rejection_procedure procedure)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 792, connection);

	connection->rejection_procedure= procedure;

	return;
}

boolean network_connection_server_accept_client_connection(
	struct network_connection *server_connection,
	struct network_connection *client_connection)
{

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 804, server_connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 805, server_connection->flags&FLAG(_connection_create_server_bit));
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 806, client_connection);

	return (boolean)(add_endpoint_to_set(client_connection->reliable_endpoint, ((struct network_server_connection *)server_connection)->endpoint_set)==_transport_error_none);
}

boolean network_connection_active(
	struct network_connection *connection)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 816, connection);

	return !TEST_FLAG(connection->flags, _connection_inactive_bit);
}

boolean network_connection_going_stale(
	struct network_connection *connection)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 824, connection);

	return TEST_FLAG(connection->flags, _connection_going_stale_bit);
}

/* ---------- private code */

static struct network_connection *network_connection_create_client_from_endpoint(
	transport_endpoint_ref reliable_endpoint)
{
	struct network_connection *connection;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 837, reliable_endpoint);

	connection= (struct network_connection *)debug_malloc(sizeof(struct network_connection), TRUE, "c:\\halo\\SOURCE\\networking\\network_connection.c", 839);
	if (connection)
	{
		connection->flags= FLAG(_connection_create_serverside_client_bit);
		connection->reliable_endpoint= reliable_endpoint;
		connection->reliable_incoming_queue= circular_queue_new("incoming-reliable", RELIABLE_INCOMING_QUEUE_SIZE);
		if (!connection->reliable_incoming_queue)
		{
			network_connection_delete(connection);
			connection= NULL;
		}
		else
		{
			network_connection_log_traffic_event(connection, _traffic_event_connection_created, 1);
		}
	}

	return connection;
}

static boolean network_client_reliable_connection_read(
	struct network_connection *connection,
	message_header *message,
	word *buffer_size,
	transport_address *source_address)
{
	word header; // the 2342 message_header is the packed size word alone (sizeof 2); message_header.h pads it to 4
	word message_size;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 881, connection && connection->reliable_incoming_queue);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 882, message);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 883, buffer_size);
	match_vassert("c:\\halo\\SOURCE\\networking\\network_connection.c", 884, *buffer_size>sizeof(header), "*buffer_size>sizeof(message_header)");

	if (!circular_queue_dequeue_data(connection->reliable_incoming_queue, &header, sizeof(header), FALSE)) return FALSE;

	byte_swap_message_header((message_header *)&header, _byte_order_host);
	message_size= header>>4;
	if (message_size > MESSAGE_MAXIMUM_SIZE)
	{
		error(_error_silent, "got an unusually large message (#d bytes); resetting reliable incoming queue", message_size);
		goto failed;
	}

	if (message_size > *buffer_size)
	{
		error(_error_silent, "packet in queue is #%d bytes, but we can only handle #%d bytes!; resetting reliable incoming queue", message_size, *buffer_size);
		goto failed;
	}

	if (message_size > circular_queue_size(connection->reliable_incoming_queue)) return FALSE;
	if (!circular_queue_dequeue_data(connection->reliable_incoming_queue, message, message_size, TRUE)) return FALSE;

	message->packed_size= header;

	match_vassert("c:\\halo\\SOURCE\\networking\\network_connection.c", 916, !(header&1), "encryption should not be active");

	if (source_address && (get_endpoint_address(connection->reliable_endpoint, source_address) != _transport_error_none))
	{
		memset(source_address, 0, sizeof(transport_address));
		source_address->address_length= IPV4_ADDRESS_LENGTH;
	}

	*buffer_size= message_size;
	network_connection_log_traffic_event(connection, _traffic_event_stream_message_received, 1);

	return TRUE;

failed:
	circular_queue_reset(connection->reliable_incoming_queue);

	return FALSE;
}

static boolean network_client_unreliable_connection_read(
	struct network_connection *connection,
	message_header *message,
	word *buffer_size,
	transport_address *source_address)
{
	unsigned long ipv4_address;
	word header;
	word message_size;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 954, connection && connection->unreliable_incoming_queue && !(connection->flags&FLAG(_connection_create_serverside_client_bit)));
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 955, message);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 956, buffer_size);
	match_vassert("c:\\halo\\SOURCE\\networking\\network_connection.c", 957, *buffer_size>sizeof(header), "*buffer_size>sizeof(message_header)");

	if (circular_queue_dequeue_data(connection->unreliable_incoming_queue, &header, sizeof(header), FALSE))
	{

		byte_swap_message_header((message_header *)&header, _byte_order_host);
		message_size= header>>4;
		if (message_size > DATAGRAM_MAXIMUM_SIZE)
		{
			error(_error_silent, "got an unusually large datagram (#d bytes); resetting unreliable incoming queue", message_size);
		}
		else if (message_size > *buffer_size)
		{
			error(_error_silent, "packet in queue is #%d bytes, but we can only handle #%d bytes!; resetting unreliable incoming queue", message_size, *buffer_size);
		}
		else if ((message_size+sizeof(ipv4_address) > circular_queue_size(connection->unreliable_incoming_queue)) ||
			!circular_queue_dequeue_data(connection->unreliable_incoming_queue, message, message_size, TRUE) ||
			!circular_queue_dequeue_data(connection->unreliable_incoming_queue, &ipv4_address, sizeof(ipv4_address), TRUE))
		{
			error(_error_silent, "partial datagram in queue (#%d of #%d bytes); resetting queue", circular_queue_size(connection->unreliable_incoming_queue), message_size);
		}
		else
		{
			message->packed_size= header;

			match_vassert("c:\\halo\\SOURCE\\networking\\network_connection.c", 991, !(header&1), "encryption should not be active");

			if (source_address)
			{
				source_address->address.ipv4_address= ipv4_address;
				source_address->port= 0;
				source_address->address_length= IPV4_ADDRESS_LENGTH;
			}

			*buffer_size= message_size;

			return TRUE;
		}
		circular_queue_reset(connection->unreliable_incoming_queue);
	}

	return FALSE;
}
static boolean network_connection_idle_server_reliable_endpoint(
	struct network_server_connection *connection,
	struct network_connection **new_client_connection)
{
	boolean success= TRUE;
	short error_code;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1029, connection != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1030, connection->connection.reliable_endpoint);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1031, connection->endpoint_set);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1032, new_client_connection);

	*new_client_connection= NULL;

	error_code= poll_endpoint_set(connection->endpoint_set, 0);
	if (error_code==_transport_error_none)
	{
		transport_endpoint_ref endpoint;

		rewind_endpoint_set(connection->endpoint_set);
		do
		{
			endpoint= get_next_endpoint_from_set(connection->endpoint_set);
			if (!endpoint || (error_code != _transport_error_none)) break;

			if (endpoint_readable(endpoint, 0))
			{
				if (endpoint==connection->connection.reliable_endpoint)
				{
					// the listening endpoint
					if (connection->allow_client_connections && (count_endpoints_in_set(connection->endpoint_set) < MAXIMUM_SERVER_ENDPOINTS))
					{
						transport_endpoint_ref client_endpoint= accept_endpoint(endpoint);
						struct network_connection *client_connection;

						if (client_endpoint && (set_endpoint_blocking(client_endpoint, FALSE)==_transport_error_none) &&
							((client_connection= network_connection_create_client_from_endpoint(client_endpoint)) != NULL))
						{
							int i;

							for (i= 0; i<MAXIMUM_CLIENT_CONNECTIONS; i++)
							{
								if (!connection->client_list[i])
								{
									*new_client_connection= client_connection;
									connection->client_list[i]= client_connection;
									break;
								}
							}
							if (i>=MAXIMUM_CLIENT_CONNECTIONS)
							{
								error(_error_silent, "error adding new client");
							}
						}
						else
						{
							error(_error_silent, "accept_endpoint() returned NULL");
						}
					}
					else if (connection->connection.rejection_procedure)
					{
						transport_endpoint_ref rejected_endpoint= accept_endpoint(endpoint);

						if (rejected_endpoint)
						{
							connection->connection.rejection_procedure(rejected_endpoint);
							delete_transport_endpoint(rejected_endpoint);
						}
					}
					else
					{
						error_code= reject_endpoint(endpoint);
					}
				}
				else
				{
					// one of the clients
					int i;

					for (i= 0; i<MAXIMUM_CLIENT_CONNECTIONS; i++)
					{
						if (connection->client_list[i] && (connection->client_list[i]->reliable_endpoint==endpoint))
						{
							success= network_connection_idle_client_reliable_endpoint(connection->client_list[i]);
							if (!success)
							{
								if (remove_endpoint_from_set(connection->client_list[i]->reliable_endpoint, connection->endpoint_set) != _transport_error_none)
								{
									error(_error_silent, "failed to remove a client endpoint from the server's endpoint set");
								}
								connection->client_list[i]->flags|= FLAG(_connection_inactive_bit);
								success= TRUE;
							}
							break;
						}
					}

					match_vassert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1129, i<MAXIMUM_CLIENT_CONNECTIONS, "rogue endpoint connected to the server");
				}
			}
		} while (success);
	}
	else if (error_code != _transport_result_poll_timeout)
	{
		error(_error_silent, "poll_endpoint_set() returned error '%s'", transport_error_to_string(error_code));
		return FALSE;
	}

	return success;
}
static boolean network_connection_idle_client_reliable_endpoint(struct network_connection *connection)
{
	char buffer[MESSAGE_MAXIMUM_SIZE]; unsigned long start_time= system_milliseconds();
	boolean success= TRUE; long free_space;

	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1153, connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1154, connection->reliable_endpoint);
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1155, connection->reliable_incoming_queue);

	free_space= circular_queue_free_space(connection->reliable_incoming_queue);
	do
	{
		long bytes_read;

		if (!endpoint_readable(connection->reliable_endpoint, 0) || (free_space <= 0)) break;

		if (free_space >= MESSAGE_MAXIMUM_SIZE) free_space= MESSAGE_MAXIMUM_SIZE;
		bytes_read= read_endpoint(connection->reliable_endpoint, buffer, free_space);
		if (bytes_read > 0)
		{
			connection->time_of_last_keep_alive= system_milliseconds();
			network_connection_log_traffic_event(connection, _traffic_event_tcp_bytes_in, bytes_read);
			if (!circular_queue_queue_data(connection->reliable_incoming_queue, buffer, bytes_read))
			{
				error(_error_silent, "circular_queue_queue_data() failed");
				success= FALSE;
			}
			free_space= circular_queue_free_space(connection->reliable_incoming_queue);
		}
		else
		{
			if (bytes_read != _transport_result_operation_would_block)
			{
				if (bytes_read==_transport_error_connection_lost)
				{
					connection->flags|= FLAG(_connection_inactive_bit);
				}
				else if (bytes_read)
				{
					error(_error_silent, "error '%s' reading from client reliable endpoint", transport_error_to_string(bytes_read));
				}
				else
				{
					error(_error_silent, "client reliable connection lost");
				}
				success= FALSE;
			}
			break;
		}
	} while (success);

	if (system_milliseconds()-start_time > MAXIMUM_IDLE_BLOCK_MILLISECONDS)
	{
		error(_error_silent, "blocked in network_connection_idle_client_reliable_endpoint");
	}

	return success;
}

static void network_connection_log_traffic_event(
	struct network_connection *connection,
	long event_type,
	long size)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1228, connection);

	if (size > 0)
	{
		switch (event_type)
		{
			case _traffic_event_connection_created:
				{
					transport_address address;

					if ((get_endpoint_address(connection->reliable_endpoint, &address) != _transport_error_none) &&
						(get_endpoint_address(connection->unreliable_endpoint, &address) != _transport_error_none))
					{
						memset(&address, 0, sizeof(address));
						address.address_length= IPV4_ADDRESS_LENGTH;
					}

					{
						char file_name[256]= "";
						long i;

						strcpy(file_name, transport_address_to_string(&address));
						for (i= 0; file_name[i]; i++)
						{
							if (file_name[i]==':')
							{
								file_name[i]= 0;
								break;
							}
						}
						strcat(file_name, "_traffic_log.xls");
						connection->traffic_log= fopen(file_name, "w");
						if (connection->traffic_log)
						{
							fprintf(connection->traffic_log, "time, seconds\tudp bytes out\tudp bytes in\ttcp bytes out\ttcp bytes in\n");
							fflush(connection->traffic_log);
						}
						connection->traffic_log_start_time= system_milliseconds();
					}
				}
				break;

			case _traffic_event_connection_deleted:
				if (connection->traffic_log)
				{
					transport_address address;

					if (get_endpoint_address(connection->reliable_endpoint, &address) != _transport_error_none)
					{
						memset(&address, 0, sizeof(address));
						address.address_length= IPV4_ADDRESS_LENGTH;
					}
					fprintf(connection->traffic_log, "\n\n");
					fprintf(connection->traffic_log, "datagrams sent\t%ld\n", connection->datagrams_sent);
					fprintf(connection->traffic_log, "datagrams received\t%ld\n", connection->datagrams_received);
					fprintf(connection->traffic_log, "stream messages sent\t%ld\n", connection->stream_messages_sent);
					fprintf(connection->traffic_log, "stream messages received\t%ld\n", connection->stream_messages_received);
					fprintf(connection->traffic_log, "datagram overhead (headers)\t%ld\tbytes per packet\n", DATAGRAM_HEADER_OVERHEAD);
					fprintf(connection->traffic_log, "stream overhead (headers)\t%ld\tbytes per chunk\n", STREAM_HEADER_OVERHEAD);
					fprintf(connection->traffic_log, "NOTE: header overhead is not included in the above traffic graph\n");
					fprintf(connection->traffic_log, "connection lifetime\t%g\tseconds\n", (system_milliseconds()-connection->traffic_log_start_time)*0.001);
					fprintf(connection->traffic_log, "connection's remote address was: %s\n", transport_address_to_string(&address));
					fclose(connection->traffic_log);
					connection->traffic_log= NULL;
				}
				break;

			case _traffic_event_udp_bytes_out:
				if (connection->traffic_log)
				{
					unsigned long elapsed_time= system_milliseconds()-connection->traffic_log_start_time;
					double seconds= elapsed_time*0.001;

					fprintf(connection->traffic_log, "%g\t%ld\t%ld\t%ld\t%ld\n", seconds, size, 0, 0, 0);
					fflush(connection->traffic_log);
				}
				connection->datagrams_sent+= 1;
				break;

			case _traffic_event_udp_bytes_in:
				if (connection->traffic_log)
				{
					unsigned long elapsed_time= system_milliseconds()-connection->traffic_log_start_time;
					double seconds= elapsed_time*0.001;

					fprintf(connection->traffic_log, "%g\t%ld\t%ld\t%ld\t%ld\n", seconds, 0, size, 0, 0);
					fflush(connection->traffic_log);
				}
				connection->datagrams_received+= 1;
				break;

			case _traffic_event_tcp_bytes_out:
				if (connection->traffic_log)
				{
					unsigned long elapsed_time= system_milliseconds()-connection->traffic_log_start_time;
					double seconds= elapsed_time*0.001;

					fprintf(connection->traffic_log, "%g\t%ld\t%ld\t%ld\t%ld\n", seconds, 0, 0, size, 0);
					fflush(connection->traffic_log);
				}
				break;

			case _traffic_event_tcp_bytes_in:
				if (connection->traffic_log)
				{
					unsigned long elapsed_time= system_milliseconds()-connection->traffic_log_start_time;
					double seconds= elapsed_time*0.001;

					fprintf(connection->traffic_log, "%g\t%ld\t%ld\t%ld\t%ld\n", seconds, 0, 0, 0, size);
					fflush(connection->traffic_log);
				}
				break;

			case _traffic_event_stream_message_sent:
				connection->stream_messages_sent+= 1;
				break;

			case _traffic_event_stream_message_received:
				connection->stream_messages_received+= 1;
				break;

			default:
				match_assert("c:\\halo\\SOURCE\\networking\\network_connection.c", 1367, !"unknown traffic event");
				break;
		}
	}

	return;
}

void network_connection_keep_alive(
	struct network_connection *connection)
{
	connection->time_of_last_keep_alive= system_milliseconds();

	return;
}
