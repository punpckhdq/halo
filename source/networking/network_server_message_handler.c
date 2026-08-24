/*
NETWORK_SERVER_MESSAGE_HANDLER.C

- only broadcast game searches, pings and client game updates arrive as datagrams on the server's
public endpoint; everything else has to come in on an accepted client connection
- an un-validated client is allowed to send nothing but a join game request
*/

/* ---------- headers */

#include "cseries.h"
#include "network_server_manager.h"
#include "network_server_message_handler.h"
#include "network_client_manager.h"
#include "network_connection.h"
#include "network_game_manager.h"
#include "network_game_globals.h"
#include "network_messages.h"
#include "transport.h"
#include "message_header.h"
#include "players.h"
#include "game.h"
#include "game_engine.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean network_game_server_write(struct network_connection *connection, message_header *message, word message_size, transport_address *dest_address, boolean reliable);

static boolean handle_message_client_broadcast_game_search(network_game_server *server, transport_address *source_address, message_client_broadcast_game_search *client_message);
static boolean handle_message_client_ping(network_game_server *server, transport_address *source_address, message_client_ping *client_message);

static boolean network_game_server_handle_message_client_join_game_request(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_add_player_request_pregame(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_remove_player_request_pregame(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_settings_request(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_player_settings_request(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_game_start_request(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_graceful_game_exit_pregame(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_map_is_precached_pregame(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_loaded(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_add_player_request_ingame(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_remove_player_request_ingame(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_remove_player_request_postgame(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_switch_to_pregame(network_game_server *server, network_client_machine *machine, message_header *message, short message_size);

/* ---------- globals */

/* ---------- public code */

boolean network_game_server_handle_datagram(
	network_game_server *server,
	message_header *message,
	short datagram_size,
	transport_address *source_address)
{
	boolean result= TRUE;
	word header;
	word message_type;

	match_vassert("c:\\halo\\SOURCE\\networking\\network_server_message_handler.c", 64,
		server && message && source_address && (datagram_size > sizeof(word)) && (datagram_size == GET_MESSAGE_SIZE(*message)),
		"server && message && source_address && (datagram_size > sizeof(message_header)) && (datagram_size == GET_MESSAGE_SIZE(*message))");

	header= *(word *)message;
	message_type= GET_MESSAGE_TYPE(header);

	if (GET_MESSAGE_FLAGS(header) != 0)
	{
		network_event("server received a datagram with invalid flags; sender= '%s'",
			transport_address_to_string(source_address));
	}
	else
	{
		switch (message_type)
		{
			case 1:
			{
				// the low-level error datagram is 128 characters of text followed by the
				// one-byte error code, so the whole message is sizeof(message_header)+129
				if (datagram_size >= sizeof(word)+129)
				{
					byte *error_message= (byte *)message + sizeof(word);

					network_event("server received low-level error message: error= #%d (%s); sender= '%s'",
						error_message[128], error_message, transport_address_to_string(source_address));
				}
				else
				{
					network_event("server received a malformed/damaged message; sender= '%s'",
						transport_address_to_string(source_address));
				}
				break;
			}

			case 2:
			{
				network_event("server received a bad message type (_message_type_data); sender= '%s'",
					transport_address_to_string(source_address));
				break;
			}

			case _message_type_packet:
			{
				short packet_version= NETWORK_GAME_MESSAGE_VERSION;
				short packet_type= ((byte *)message)[datagram_size-1];

				datagram_size -= sizeof(word);

				switch (packet_type)
				{
					case _message_type_client_broadcast_game_search:
					{
						if (network_game_should_accept_remote_connections())
						{
							message_client_broadcast_game_search message_packet;

							if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
								&datagram_size, &packet_type, &packet_version, 0))
							{
								if (!handle_message_client_broadcast_game_search(server, source_address, &message_packet))
								{
									network_event("server failed to advertise game to prospective client at '%s'",
										transport_address_to_string(source_address));
								}
							}
							else
							{
								network_event("failed to decode a message_client_broadcast_game_search packet");
							}
						}
						break;
					}

					case _message_type_client_ping:
					{
						if (network_game_should_accept_remote_connections())
						{
							message_client_ping message_packet;

							if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
								&datagram_size, &packet_type, &packet_version, 0))
							{
								if (!handle_message_client_ping(server, source_address, &message_packet))
								{
									network_event("server failed to handle a client ping");
								}
							}
							else
							{
								network_event("failed to decode a message_client_ping packet");
							}
						}
						break;
					}

					case _message_type_client_game_update:
					{
						if (network_game_server_get_state(server, NULL) == _network_game_server_state_ingame)
						{
							network_client_machine *machine= network_game_server_get_client_machine_at_address(server, source_address->address.ipv4_address);

							if (machine)
							{
								message_client_game_update message_packet;

								if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
									&datagram_size, &packet_type, &packet_version, 5))
								{
									network_game_server_handle_client_update_packet(server, machine, &message_packet);
								}
								else
								{
									network_event("failed to decode a message_client_game_update packet");
								}
							}
							else
							{
								network_event("failed to handle a message_client_game_update message; this client doesn't seem to be in the game");
							}
						}
						else
						{
							network_event("ignoring a message_client_game_update message; we are not in game");
						}
						break;
					}

					default:
					{
						network_event("server received datagram with an unexpected packet type; sender= '%s'",
							transport_address_to_string(source_address));
						break;
					}
				}
				break;
			}

			default:
			{
				network_event("server received a datagram with an unknown message type (#%d); sender= '%s'",
					message_type, transport_address_to_string(source_address));
				break;
			}
		}
	}

	return result;
}

boolean network_game_server_handle_client_message(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_buffer_size)
{
	boolean result= TRUE;
	word header;
	word message_type;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_message_handler.c", 207,
		server && machine && message && (message_buffer_size == GET_MESSAGE_SIZE(*message)));

	header= *(word *)message;
	message_type= GET_MESSAGE_TYPE(header);

	if (GET_MESSAGE_FLAGS(header) != 0)
	{
		network_event("server received client message with invalid flags");
	}
	else
	{
		switch (message_type)
		{
			case 1:
			{
				if (message_buffer_size >= sizeof(word)+129)
				{
					byte *error_message= (byte *)message + sizeof(word);

					network_event("server received low-level error message from a client: error= #%d (%s)",
						error_message[128], error_message);
				}
				else
				{
					network_event("server received a malformed/damaged message from a client");
				}
				break;
			}

			case 2:
			{
				network_event("server received a bad message type from a client (_message_type_data)");
				break;
			}

			case _message_type_packet:
			{
				byte packet_type= ((char *)message)[message_buffer_size-1];

				if (network_game_server_client_machine_is_joined_to_game(server, machine) ||
					(packet_type == _message_type_client_join_game_request))
				{
					switch (packet_type)
					{
						case _message_type_client_join_game_request:
						{
							if (!(result= network_game_server_handle_message_client_join_game_request(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_join_game_request() failed");
							}
							break;
						}

						case _message_type_client_add_player_request_pregame:
						{
							if (!(result= network_game_server_handle_message_client_add_player_request_pregame(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_add_player_request_pregame() failed");
							}
							break;
						}

						case _message_type_client_remove_player_request_pregame:
						{
							if (!(result= network_game_server_handle_message_client_remove_player_request_pregame(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_remove_player_request_pregame() failed");
							}
							break;
						}

						case _message_type_client_settings_request:
						{
							if (!(result= network_game_server_handle_message_client_settings_request(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_settings_request() failed");
							}
							break;
						}

						case _message_type_client_player_settings_request:
						{
							if (!(result= network_game_server_handle_message_client_player_settings_request(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_player_settings_request() failed");
							}
							break;
						}

						case _message_type_client_game_start_request:
						{
							if (!(result= network_game_server_handle_message_client_game_start_request(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_game_start_request() failed");
							}
							break;
						}

						case _message_type_client_graceful_game_exit_pregame:
						{
							if (!(result= network_game_server_handle_message_client_graceful_game_exit_pregame(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_graceful_game_exit_pregame() failed");
							}
							break;
						}

						case _message_type_client_map_is_precached_pregame:
						{
							if (!(result= network_game_server_handle_message_client_map_is_precached_pregame(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_graceful_game_exit_pregame() failed");
							}
							break;
						}

						case _message_type_client_loaded:
						{
							if (!(result= network_game_server_handle_message_client_loaded(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_loaded() failed");
							}
							break;
						}

						case _message_type_client_add_player_request_ingame:
						{
							if (!(result= network_game_server_handle_message_client_add_player_request_ingame(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_add_player_request_ingame() failed");
							}
							break;
						}

						case _message_type_client_remove_player_request_ingame:
						{
							if (!(result= network_game_server_handle_message_client_remove_player_request_ingame(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_remove_player_request_ingame() failed");
							}
							break;
						}

						case _message_type_client_remove_player_request_postgame:
						{
							if (!(result= network_game_server_handle_message_client_remove_player_request_postgame(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_remove_player_request_postgame() failed");
							}
							break;
						}

						case _message_type_client_switch_to_pregame:
						{
							if (!(result= network_game_server_handle_message_client_switch_to_pregame(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_switch_to_pregame() failed");
							}
							break;
						}

						case _message_type_client_graceful_game_exit_postgame:
						{
							if (!(result= network_game_server_handle_message_client_graceful_game_exit_pregame(server, machine, message, message_buffer_size)))
							{
								network_event("network_game_server_handle_message_client_graceful_game_exit_pregame() failed");
							}
							break;
						}

						default:
						{
							network_event("bad or inappropriate packet type received from a client (#%d)", packet_type);
							break;
						}
					}
				}
				else
				{
					network_event("an un-validated client sent something other than a join request message");
				}
				break;
			}

			default:
			{
				network_event("server received a client message with an unknown message type (#%d)", message_type);
				break;
			}
		}
	}

	return result;
}

boolean network_game_server_send_message_to_machine(
	network_game_server *server,
	network_machine *machine,
	message_header *message)
{
	boolean result= FALSE;
	struct network_connection *connection= network_game_server_get_machine_connection(server, machine);

	if (connection)
	{
		result= network_game_server_write(connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE);
	}

	return result;
}

boolean network_game_server_send_message_to_all_machines(
	network_game_server *server,
	message_header *message)
{
	boolean result= TRUE;
	word message_length;
	long i;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_message_handler.c", 391, server && message);

	message_length= GET_MESSAGE_SIZE(*message);

	for (i= 0; i<MAXIMUM_NETWORK_MACHINE_COUNT; i++)
	{
		network_client_machine *client_machine= network_game_server_get_client_machine_at_index(server, i);

		if (network_game_server_client_machine_is_joined_to_game(server, client_machine))
		{
			struct network_connection *connection= network_game_server_get_client_connection(client_machine);

			if (connection && network_connection_active(connection))
			{
				byte message_buffer[1536];

				match_assert("c:\\halo\\SOURCE\\networking\\network_server_message_handler.c", 410, message_length<=sizeof(message_buffer));

				csmemcpy(message_buffer, message, message_length);

				if (!network_game_server_write(connection, (message_header *)message_buffer, message_length, NULL, TRUE))
				{
					network_event("network_game_server_write() failed in network_game_server_send_message_to_all_machines()");
					result= FALSE;
				}
			}
		}
	}

	return result;
}

boolean network_game_server_send_player_joined_info_ingame(
	network_game_server *server,
	network_player *player)
{
	boolean result;
	message_server_add_player_ingame message_packet;
	message_header *message;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_message_handler.c", 432, server && player);

	message_packet.player= *player;

	if ((message= create_network_game_message(_message_type_server_add_player_ingame, &message_packet, sizeof(message_packet))) != NULL)
	{
		if (!(result= network_game_server_send_message_to_all_machines(server, message)))
		{
			network_event("network_game_server_send_message_to_all_machines() failed in network_game_server_send_player_joined_info_ingame()");
		}
	}
	else
	{
		network_event("failed to create a message_server_add_player_ingame message");
		result= FALSE;
	}

	return result;
}

boolean network_game_server_send_game_data_pregame(
	network_game_server *server)
{
	boolean result= FALSE;
	network_game_data *game;
	message_server_game_settings_update message_packet;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_message_handler.c", 456, server);

	game= network_game_server_get_game(server);
	if (game)
	{
		message_header *message;

		csmemcpy(&message_packet, game, sizeof(message_packet));

		if ((message= create_network_game_message(_message_type_server_game_settings_update, &message_packet, sizeof(message_packet))) != NULL)
		{
			if (!(result= network_game_server_send_message_to_all_machines(server, message)))
			{
				network_event("failed to send message_server_game_settings_update message to all machines");
			}
		}
		else
		{
			network_event("failed to create a message_server_game_settings_update message");
		}
	}
	else
	{
		network_event("failed to handle a message_server_game_settings_update because their was no server game");
	}

	return result;
}

/* ---------- private code */

static boolean network_game_server_write(
	struct network_connection *connection,
	message_header *message,
	word message_size,
	transport_address *dest_address,
	boolean reliable)
{
	return network_connection_write(connection, message, message_size, dest_address, reliable);
}

static boolean handle_message_client_broadcast_game_search(
	network_game_server *server,
	transport_address *source_address,
	message_client_broadcast_game_search *client_message)
{
	boolean result= TRUE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_message_handler.c", 543, server && source_address && client_message);

	if (client_message->version == NETWORK_GAME_MESSAGE_VERSION)
	{
		network_game_data *game= network_game_server_get_game(server);

		if (game)
		{
			message_server_game_advertise game_advertisement= {0};
			transport_address reply_address;
			message_header *message;

			reply_address.address_length= IPV4_ADDRESS_LENGTH;
			reply_address.address.ipv4_address= IPV4_BROADCAST_ADDRESS;
			reply_address.port= NETWORK_CLIENT_PORT;

			csmemcpy(game_advertisement.client_nonce, client_message->nonce, sizeof(game_advertisement.client_nonce));
			transport_get_nonce(game_advertisement.server_nonce, sizeof(game_advertisement.server_nonce));
			game_advertisement.key_id= transport_get_key_id();
			game_advertisement.key= transport_get_key();
			game_advertisement.host_address= transport_get_xnaddr();
			game_advertisement.port= NETWORK_SERVER_PORT;
			game_advertisement.version= NETWORK_GAME_MESSAGE_VERSION;
			game_advertisement.platform= 0;

			ustrncpy(game_advertisement.name, game->name, MAXIMUM_NETWORK_GAME_NAME_LENGTH-1);
			game_advertisement.game_engine= (short)game->variant.engine_index;
			csmemcpy(&game_advertisement.map, &game->map, sizeof(game_advertisement.map));
			game_advertisement.current_number_of_machines= game->machine_count;
			game_advertisement.current_number_of_players= game->player_count;
			game_advertisement.maximum_number_of_players= game->maximum_players;
			game_advertisement.score_to_win= (short)game->variant.universal_variant.score_to_win;
			game_advertisement.flags= 0;
			if (game->variant.universal_variant.teams == TRUE)
			{
				game_advertisement.flags= FLAG(_game_advertise_teams_enabled_bit);
			}

			if ((game->variant.engine_index == _game_engine_oddball) && (game->variant.oddball.ball_type == 2))
			{
				game_advertisement.flags |= FLAG(_game_advertise_oddball_terminator_bit);
			}

			if (network_game_server_game_is_open(server))
			{
				game_advertisement.flags |= FLAG(_game_advertise_open_bit);
			}

			network_game_generate_join_game_token(game_advertisement.join_token);

			if ((message= create_network_game_message(_message_type_server_game_advertise, &game_advertisement, sizeof(game_advertisement))) != NULL)
			{
				if (!(result= network_game_server_write(network_game_server_get_connection(server), message,
					GET_MESSAGE_SIZE(*message), &reply_address, FALSE)))
				{
					network_event("network_game_server_write() failed in handle_message_client_broadcast_game_search()");
				}
			}
			else
			{
				network_event("failed to create a message_server_game_advertise message");
			}
		}
	}

	return result;
}

static boolean handle_message_client_ping(
	network_game_server *server,
	transport_address *source_address,
	message_client_ping *client_message)
{
	boolean result;
	message_server_pong pong_message;
	message_header *message;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_message_handler.c", 618, server && source_address && client_message);

	pong_message.system_milliseconds= client_message->system_milliseconds;

	if ((message= create_network_game_message(_message_type_server_pong, &pong_message, sizeof(pong_message))) != NULL)
	{
		transport_address reply_address;

		reply_address.address_length= IPV4_ADDRESS_LENGTH;
		reply_address.address.ipv4_address= source_address->address.ipv4_address;
		reply_address.port= client_message->reply_to_port;

		if (!(result= network_game_server_write(network_game_server_get_connection(server), message,
			GET_MESSAGE_SIZE(*message), &reply_address, FALSE)))
		{
			network_event("network_game_server_write() failed in handle_message_client_ping()");
		}
	}
	else
	{
		network_event("failed to create a message_server_pong message");
		result= FALSE;
	}

	return result;
}

static boolean network_game_server_handle_message_client_join_game_request(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		message_client_join_game_request message_packet;
		short packet_type= _message_type_client_join_game_request;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (network_game_server_client_machine_is_joined_to_game(server, machine))
		{
			network_event("ignoring redundant join request from machine");
		}
		else if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 3))
		{
			transport_address remote_address;
			message_header *reply;

			network_connection_get_address(network_game_server_get_client_connection(machine), &remote_address, NULL);

			if ((network_game_server_get_state(server, NULL) == _network_game_server_state_pregame) &&
				network_game_server_game_is_open(server))
			{
				byte join_token[NETWORK_JOIN_GAME_TOKEN_SIZE];

				network_game_generate_join_game_token(join_token);

				if (csmemcmp(message_packet.join_token, join_token, sizeof(join_token)) == 0)
				{
					FILE *hosts_file;

					wide_to_ascii(message_packet.machine_name, (char *)message_packet.machine_name, sizeof(message_packet.machine_name));

					if ((hosts_file= fopen("d:\\hosts.txt", "r")) != NULL)
					{
						char line[32]= {0};
						boolean found= FALSE;

						while (fgets(line, sizeof(line), hosts_file))
						{
							if (csstrncmp((char *)message_packet.machine_name, line, csstrlen((char *)message_packet.machine_name)) == 0)
							{
								found= TRUE;
								break;
							}
						}

						fclose(hosts_file);

						if (!found)
						{
							message_server_machine_rejected rejection_message;

							rejection_message.reason= _rejection_code_blacklisted_machine;

							network_event("server refused client '%s' because it is not in your hosts file", message_packet.machine_name);

							if ((reply= create_network_game_message(_message_type_server_machine_rejected, &rejection_message, sizeof(rejection_message))) != NULL)
							{
								network_game_server_write(network_game_server_get_client_connection(machine), reply,
									GET_MESSAGE_SIZE(*reply), NULL, TRUE);
							}

							result= FALSE;

							goto done;
						}
					}

					if (network_game_server_accept_client_machine_into_game(server, machine))
					{
						message_server_machine_accepted accepted_message;
						long machine_index= NONE;
						network_machine *client_machine= network_game_server_get_client_machine(server, machine, &machine_index);
						network_game_data *game= network_game_server_get_game(server);

						match_assert("c:\\halo\\SOURCE\\networking\\network_server_message_handler.c", 718, network_machine_is_valid(client_machine));

						accepted_message.machine_index= (short)machine_index;
						accepted_message.server_random_seed= network_game_get_random_seed();

						if ((reply= create_network_game_message(_message_type_server_machine_accepted, &accepted_message, sizeof(accepted_message))) != NULL)
						{
							if (!(result= network_game_server_write(network_game_server_get_client_connection(machine), reply,
								GET_MESSAGE_SIZE(*reply), NULL, TRUE)))
							{
								network_event("network_game_server_write() failed in network_game_server_handle_message_client_join_game_request()");
							}
							else
							{
								network_event("sent _message_type_server_machine_accepted message to %d", machine_index);
							}

							if (result == TRUE)
							{
								if (!(result= network_game_server_send_game_data_pregame(server)))
								{
									network_event("network_game_server_send_game_data_pregame() failed in network_game_server_handle_message_client_join_game_request()");
								}
							}
						}
						else
						{
							result= FALSE;
						}
					}
					else
					{
						message_server_machine_rejected rejection_message;

						rejection_message.reason= _rejection_code_game_is_closed;

						network_event("server failed to accept valid client machine '%s' @%s into the game",
							message_packet.machine_name, transport_address_to_string(&remote_address));

						if ((reply= create_network_game_message(_message_type_server_machine_rejected, &rejection_message, sizeof(rejection_message))) != NULL)
						{
							if (!network_game_server_write(network_game_server_get_client_connection(machine), reply,
								GET_MESSAGE_SIZE(*reply), NULL, TRUE))
							{
								network_event("network_game_server_write() failed while sending a rejection reply");
							}
						}

						result= FALSE;
					}
				}
				else
				{
					message_server_machine_rejected rejection_message;

					rejection_message.reason= _rejection_code_bad_join_token;

					network_event("client machine '%s' @%s tried to join game with a bad join token",
						message_packet.machine_name, transport_address_to_string(&remote_address));

					if ((reply= create_network_game_message(_message_type_server_machine_rejected, &rejection_message, sizeof(rejection_message))) != NULL)
					{
						if (!network_game_server_write(network_game_server_get_client_connection(machine), reply,
							GET_MESSAGE_SIZE(*reply), NULL, TRUE))
						{
							network_event("network_game_server_write() failed while sending a rejection reply");
						}
					}
					else
					{
						network_event("failed to create a message_server_machine_rejected message");
					}

					result= FALSE;
				}
			}
			else
			{
				message_server_machine_rejected rejection_message;

				rejection_message.reason= _rejection_code_game_is_closed;

				network_event("client machine '%s' @%s tried to join game when they should not be",
					message_packet.machine_name, transport_address_to_string(&remote_address));

				if ((reply= create_network_game_message(_message_type_server_machine_rejected, &rejection_message, sizeof(rejection_message))) != NULL)
				{
					if (!network_game_server_write(network_game_server_get_client_connection(machine), reply,
						GET_MESSAGE_SIZE(*reply), NULL, TRUE))
					{
						network_event("network_game_server_write() failed while sending a rejection reply");
					}
				}
				else
				{
					network_event("failed to create a message_server_machine_rejected message");
				}

				result= FALSE;
			}
		}
		else
		{
			network_event("server failed to decode a message_client_join_game_request packet");
			result= FALSE;
		}
	}

done:
	return result;
}

static boolean network_game_server_handle_message_client_add_player_request_pregame(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		message_client_add_player_request_pregame message_packet;
		short packet_type= _message_type_client_add_player_request_pregame;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 3))
		{
			if (network_game_server_add_player_to_game(server, machine, &message_packet.player))
			{
				if (!network_game_server_send_game_data_pregame(server))
				{
					network_event("server failed to send pregame game data in network_game_server_handle_message_client_add_player_request_pregame()");
				}
			}
			else
			{
				network_event("server failed to add a network player in network_game_server_handle_message_client_add_player_request_pregame()");
			}
		}
		else
		{
			network_event("server failed to decode a message_client_add_player_request_pregame packet");
		}
	}
	else
	{
		network_event("failed to handle a message_client_add_player_request_pregame because the server is not in pregame");
	}

	return result;
}

static boolean network_game_server_handle_message_client_remove_player_request_pregame(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		message_client_remove_player_request_pregame message_packet;
		short packet_type= _message_type_client_remove_player_request_pregame;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 3))
		{
			if (network_game_server_remove_player_from_game(server, machine, &message_packet.player))
			{
				if (!network_game_server_send_game_data_pregame(server))
				{
					network_event("server failed to send pregame game data in network_game_server_handle_message_client_remove_player_request_pregame()");
				}
			}
			else
			{
				network_event("server failed to remove a network player in network_game_server_handle_message_client_remove_player_request_pregame()");
			}
		}
		else
		{
			network_event("server failed to decode a message_client_remove_player_request_pregame packet");
		}
	}
	else
	{
		network_event("failed to handle a message_client_remove_player_request_pregame because the server is not in pregame");
	}

	return result;
}

static boolean network_game_server_handle_message_client_settings_request(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		message_client_settings_request message_packet;
		short packet_type= _message_type_client_settings_request;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 3))
		{
			if (network_game_server_adjust_machine_settings(server, machine, &message_packet.machine))
			{
				network_event("server received machine settings for machine #%d/'%s'",
					message_packet.machine.machine_index,
					wide_to_ascii(message_packet.machine.name, (char *)message_packet.machine.name, sizeof(message_packet.machine.name)));

				if (!network_game_server_send_game_data_pregame(server))
				{
					network_event("server failed to send pregame game data in network_game_server_handle_message_client_settings_request()");
				}
			}
			else
			{
				network_event("network_game_server_adjust_machine_settings() failed in network_game_server_handle_message_client_settings_request()");
			}
		}
		else
		{
			network_event("server failed to decode a message_client_settings_request packet");
		}
	}
	else
	{
		network_event("failed to handle a message_client_settings_request because the server is not in pregame");
	}

	return result;
}

static boolean network_game_server_handle_message_client_player_settings_request(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		message_client_player_settings_request message_packet;
		short packet_type= _message_type_client_player_settings_request;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 3))
		{
			if (network_game_update_player(network_game_server_get_game(server), &message_packet.player))
			{
				network_event("server received updated player settings");

				if (!network_game_server_send_game_data_pregame(server))
				{
					network_event("server failed to send pregame game data in network_game_server_handle_message_client_player_settings_request()");
				}
			}
			else
			{
				network_event("network_game_update_player() failed in network_game_server_handle_message_client_player_settings_request()");
			}
		}
		else
		{
			network_event("server failed to decode a message_client_player_settings_request packet");
		}
	}
	else
	{
		network_event("failed to handle a message_client_player_settings_request because the server is not in pregame");
	}

	return result;
}

static boolean network_game_server_handle_message_client_game_start_request(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		message_client_game_start_request message_packet;
		// xxx copied from the player settings handler above; should be
		// _message_type_client_game_start_request
		short packet_type= _message_type_client_player_settings_request;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 3))
		{
			network_game_server_update_countdown(server, message_packet.request);
		}
		else
		{
			network_event("server failed to decode a message_client_game_start_request packet");
		}
	}
	else
	{
		network_event("failed to handle a message_client_game_start_request because the server is not in pregame");
	}

	return result;
}

static boolean network_game_server_handle_message_client_graceful_game_exit_pregame(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		message_client_graceful_game_exit_pregame message_packet;
		short packet_type= _message_type_client_graceful_game_exit_pregame;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 3))
		{
			if (network_game_server_remove_machine_from_game(server, network_game_server_get_client_machine(server, machine, NULL)))
			{
				if (!network_game_server_send_game_data_pregame(server))
				{
					network_event("server failed to send pregame game data in network_game_server_handle_message_client_graceful_game_exit_pregame()");
				}
			}
			else
			{
				network_event("network_game_server_remove_machine_from_game() failed in network_game_server_handle_message_client_graceful_game_exit_pregame()");
			}
		}
		else
		{
			network_event("server failed to decode a message_client_graceful_game_exit_pregame packet");
		}
	}
	else
	{
		network_event("failed to handle a message_client_graceful_game_exit_pregame message because the server is not in pregame");
	}

	return result;
}

static boolean network_game_server_handle_message_client_map_is_precached_pregame(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		message_client_map_is_precached_pregame message_packet;
		short packet_type= _message_type_client_map_is_precached_pregame;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 3))
		{
			network_game_server_client_machine_is_precached(server, machine, message_packet.map_name);
		}
		else
		{
			network_event("server failed to decode a message_type_client_map_is_precached_pregame packet");
		}
	}
	else
	{
		network_event("failed to handle a message_type_client_map_is_precached_pregame because the server is not in pregame");
	}

	return result;
}

static boolean network_game_server_handle_message_client_loaded(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		message_client_loaded message_packet;
		short packet_type= _message_type_client_loaded;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 5))
		{
			network_game_server_client_machine_game_loading_complete(server, machine);
		}
		else
		{
			network_event("server failed to decode a message_client_loaded packet");
			result= FALSE;
		}
	}
	else
	{
		network_event("failed to handle a message_client_loaded message because the server is not in pregame");
		result= FALSE;
	}

	return result;
}

static boolean network_game_server_handle_message_client_add_player_request_ingame(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_ingame)
	{
		message_client_add_player_request_ingame message_packet;
		short packet_type= _message_type_client_add_player_request_ingame;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 5))
		{
			network_game_server_queue_player_for_addition(server, &message_packet.player);
		}
		else
		{
			network_event("server failed to decode a message_client_add_player_request_ingame packet");
		}
	}
	else
	{
		network_event("failed to handle a message_client_add_player_request_ingame because the server is not in game");
	}

	return result;
}

static boolean network_game_server_handle_message_client_remove_player_request_ingame(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_ingame)
	{
		message_client_remove_player_request_ingame message_packet;
		// xxx copied from the add-player handler above; should be
		// _message_type_client_remove_player_request_ingame
		short packet_type= _message_type_client_add_player_request_ingame;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 5))
		{
			network_game_data *game= network_game_server_get_game(server);

			if (network_game_remove_player(game, &message_packet.player))
			{
				message_server_remove_player_ingame remove_player_ingame_message;
				message_header *reply;

				remove_player_ingame_message.player= message_packet.player;
				remove_player_ingame_message.time_of_quit_out_of_game= game_time_get() + MAXIMUM_SERVER_SAVED_UPDATES + 1;

				if ((reply= create_network_game_message(_message_type_server_remove_player_ingame, &remove_player_ingame_message, sizeof(remove_player_ingame_message))) != NULL)
				{
					if ((result= network_game_server_send_message_to_all_machines(server, reply)) == FALSE)
					{
						network_event("network_game_server_send_message_to_all_machines() failed in network_game_server_handle_message_client_remove_player_request_ingame()");
					}
				}
			}
		}
		else
		{
			network_event("server failed to decode a message_client_remove_player_request_ingame packet");
		}
	}
	else
	{
		network_event("failed to handle a message_client_remove_player_request_ingame because the server is not in game");
	}

	return result;
}

static boolean network_game_server_handle_message_client_remove_player_request_postgame(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_postgame)
	{
		message_client_remove_player_request_postgame message_packet;
		short packet_type= _message_type_client_remove_player_request_postgame;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 7))
		{
			if (!network_game_server_remove_player_from_game(server, machine, &message_packet.player))
			{
				network_event("server failed to remove a network player post-game");
			}
		}
		else
		{
			network_event("server failed to decode a message_client_remove_player_request_postgame packet");
		}
	}
	else
	{
		network_event("failed to handle a message_client_remove_player_request_postgame because the server is not in post-game");
	}

	return result;
}

static boolean network_game_server_handle_message_client_switch_to_pregame(
	network_game_server *server,
	network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result= TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_postgame)
	{
		message_client_switch_to_pregame message_packet;
		short packet_type= _message_type_client_switch_to_pregame;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(word);

		if (decode_network_game_message(&message_packet, (char *)message + sizeof(word),
			&message_size, &packet_type, &packet_version, 7))
		{
			if (!(result= network_game_server_switch_machine_from_postgame_to_pregame(server, machine)))
			{
				network_event("network_game_server_switch_machine_from_postgame_to_pregame() failed");
			}
		}
		else
		{
			network_event("server failed to decode a message_client_remove_player_request_postgame packet");
		}
	}
	else
	{
		network_event("failed to handle a message_client_switch_to_pregame because the server is not in post-game");
	}

	return result;
}
