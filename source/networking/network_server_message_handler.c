/*
NETWORK_SERVER_MESSAGE_HANDLER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "network_server_message_handler.h"
#include "network_game_globals.h"
#include "network_messages.h"
#include "network_client_manager.h"
#include "network_server_manager.h"
#include "network_connection.h"
#include "units.h"
#include "players.h"
#include "input.h"
#include "console.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "game_engine_list.h"

/* ---------- constants */

enum
{
	MAXIMUM_SERVER_MESSAGE_SIZE = 0x600, /* fake name */
	MAXIMUM_HOSTS_FILE_LINE_LENGTH = 32, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean handle_message_client_broadcast_game_search(struct network_game_server *server, struct transport_address *source_address, struct message_client_broadcast_game_search *client_message);
static boolean handle_message_client_ping(struct network_game_server *server, struct transport_address *source_address, struct message_client_ping *client_message);
static boolean network_game_server_handle_message_client_join_game_request(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_add_player_request_pregame(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_remove_player_request_pregame(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_settings_request(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_player_settings_request(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_game_start_request(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_graceful_game_exit_pregame(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_map_is_precached_pregame(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_loaded(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_add_player_request_ingame(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_remove_player_request_ingame(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_remove_player_request_postgame(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_handle_message_client_switch_to_pregame(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_size);
static boolean network_game_server_write(struct network_connection *connection, void *message, word message_size, struct transport_address *destination_address, boolean reliable);

/* ---------- globals */

/* ---------- public code */

boolean network_game_server_handle_datagram(
	struct network_game_server *server,
	message_header *message,
	short datagram_size,
	struct transport_address *source_address)
{
	word message_type;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_server_message_handler.c",
		64,
		server && message && source_address && (datagram_size > sizeof(message_header)) && (datagram_size == GET_MESSAGE_SIZE(*message)));

	message_type = (byte)GET_MESSAGE_TYPE(*message);

	if (GET_MESSAGE_FLAGS(*message))
	{
		network_event(
			"server received a datagram with invalid flags; sender= '%s'",
			transport_address_to_string(source_address));
	}
	else
	{
		switch (message_type)
		{
		case _message_type_packet:
		{
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;
			short packet_type = ((byte *)message)[datagram_size - 1];

			datagram_size -= sizeof(message_header);

			switch (packet_type)
			{
			case _message_type_client_broadcast_game_search:
				if (network_game_should_accept_remote_connections())
				{
					struct message_client_broadcast_game_search game_search;

					if (decode_network_game_message(
						&game_search,
						message + 1,
						&datagram_size,
						&packet_type,
						&packet_version,
						_message_class_unconnected_client))
					{
						if (!handle_message_client_broadcast_game_search(
							server,
							source_address,
							&game_search))
						{
							network_event(
								"server failed to advertise game to prospective client at '%s'",
								transport_address_to_string(source_address));
						}
					}
					else
					{
						network_event("failed to decode a message_client_broadcast_game_search packet");
					}
				}
				break;
			case _message_type_client_ping:
				if (network_game_should_accept_remote_connections())
				{
					struct message_client_ping ping;

					if (decode_network_game_message(
						&ping,
						message + 1,
						&datagram_size,
						&packet_type,
						&packet_version,
						_message_class_unconnected_client))
					{
						if (!handle_message_client_ping(
							server,
							source_address,
							&ping))
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
			case _message_type_client_game_update:
				if (network_game_server_get_state(server, NULL) == _network_game_server_state_ingame)
				{
					struct network_client_machine *client_machine = network_game_server_get_client_machine_at_address(
						server,
						source_address->address.ipv4_address);

					if (client_machine)
					{
						struct message_client_game_update game_update;

						if (decode_network_game_message(
							&game_update,
							message + 1,
							&datagram_size,
							&packet_type,
							&packet_version,
							_message_class_client_ingame))
						{
							network_game_server_handle_client_update_packet(
								server,
								client_machine,
								&game_update);
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
			default:
				network_event(
					"server received datagram with an unexpected packet type; sender= '%s'",
					transport_address_to_string(source_address));
				break;
			}
			break;
		}
		case _message_type_data:
			network_event(
				"server received a bad message type (_message_type_data); sender= '%s'",
				transport_address_to_string(source_address));
			break;
		case _message_type_error:
			if (datagram_size >= sizeof(message_header) + sizeof(struct message_error))
			{
				struct message_error *error_message = (struct message_error *)(message + 1);

				network_event(
					"server received low-level error message: error= #%d (%s); sender= '%s'",
					error_message->error_code,
					error_message->error_string,
					transport_address_to_string(source_address));
			}
			else
			{
				network_event(
					"server received a malformed/damaged message; sender= '%s'",
					transport_address_to_string(source_address));
			}
			break;
		default:
			network_event(
				"server received a datagram with an unknown message type (#%d); sender= '%s'",
				message_type,
				transport_address_to_string(source_address));
			break;
		}
	}

	return TRUE;
}

boolean network_game_server_handle_client_message(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_buffer_size)
{
	word message_type;
	boolean result = TRUE;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_server_message_handler.c",
		207,
		server && machine && message && (message_buffer_size == GET_MESSAGE_SIZE(*message)));

	message_type = (byte)GET_MESSAGE_TYPE(*message);

	if (GET_MESSAGE_FLAGS(*message))
	{
		network_event("server received client message with invalid flags");
	}
	else
	{
		switch (message_type)
		{
		case _message_type_packet:
		{
			byte packet_type = ((byte *)message)[message_buffer_size - 1];

			if (network_game_server_client_machine_is_joined_to_game(server, machine) ||
				packet_type == _message_type_client_join_game_request)
			{
				switch (packet_type)
				{
				case _message_type_client_join_game_request:
					result = network_game_server_handle_message_client_join_game_request(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_join_game_request() failed");
					}
					break;
				case _message_type_client_add_player_request_pregame:
					result = network_game_server_handle_message_client_add_player_request_pregame(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_add_player_request_pregame() failed");
					}
					break;
				case _message_type_client_remove_player_request_pregame:
					result = network_game_server_handle_message_client_remove_player_request_pregame(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_remove_player_request_pregame() failed");
					}
					break;
				case _message_type_client_settings_request:
					result = network_game_server_handle_message_client_settings_request(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_settings_request() failed");
					}
					break;
				case _message_type_client_player_settings_request:
					result = network_game_server_handle_message_client_player_settings_request(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_player_settings_request() failed");
					}
					break;
				case _message_type_client_game_start_request:
					result = network_game_server_handle_message_client_game_start_request(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_game_start_request() failed");
					}
					break;
				case _message_type_client_graceful_game_exit_pregame:
					result = network_game_server_handle_message_client_graceful_game_exit_pregame(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_graceful_game_exit_pregame() failed");
					}
					break;
				case _message_type_client_map_is_precached_pregame:
					result = network_game_server_handle_message_client_map_is_precached_pregame(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_graceful_game_exit_pregame() failed");
					}
					break;
				case _message_type_client_loaded:
					result = network_game_server_handle_message_client_loaded(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_loaded() failed");
					}
					break;
				case _message_type_client_add_player_request_ingame:
					result = network_game_server_handle_message_client_add_player_request_ingame(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_add_player_request_ingame() failed");
					}
					break;
				case _message_type_client_remove_player_request_ingame:
					result = network_game_server_handle_message_client_remove_player_request_ingame(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_remove_player_request_ingame() failed");
					}
					break;
				case _message_type_client_remove_player_request_postgame:
					result = network_game_server_handle_message_client_remove_player_request_postgame(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_remove_player_request_postgame() failed");
					}
					break;
				case _message_type_client_switch_to_pregame:
					result = network_game_server_handle_message_client_switch_to_pregame(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_switch_to_pregame() failed");
					}
					break;
				case _message_type_client_graceful_game_exit_postgame:
					result = network_game_server_handle_message_client_graceful_game_exit_pregame(
						server,
						machine,
						message,
						message_buffer_size);

					if (!result)
					{
						network_event("network_game_server_handle_message_client_graceful_game_exit_pregame() failed");
					}
					break;
				default:
					network_event(
						"bad or inappropriate packet type received from a client (#%d)",
						packet_type);
					break;
				}
			}
			else
			{
				network_event("an un-validated client sent something other than a join request message");
			}
			break;
		}
		case _message_type_data:
			network_event("server received a bad message type from a client (_message_type_data)");
			break;
		case _message_type_error:
			if (message_buffer_size >= sizeof(message_header) + sizeof(struct message_error))
			{
				struct message_error *error_message = (struct message_error *)(message + 1);

				network_event(
					"server received low-level error message from a client: error= #%d (%s)",
					error_message->error_code,
					error_message->error_string);
			}
			else
			{
				network_event("server received a malformed/damaged message from a client");
			}
			break;
		default:
			network_event(
				"server received a client message with an unknown message type (#%d)",
				message_type);
			break;
		}
	}

	return result;
}

boolean network_game_server_send_message_to_machine(
	struct network_game_server *server,
	struct network_machine *machine,
	message_header *message)
{
	boolean result = FALSE;
	struct network_connection *connection = network_game_server_get_machine_connection(
		server,
		machine);

	if (connection)
	{
		result = network_game_server_write(
			connection,
			message,
			GET_MESSAGE_SIZE(*message),
			NULL,
			TRUE);
	}

	return result;
}

boolean network_game_server_send_message_to_all_machines(
	struct network_game_server *server,
	message_header *message)
{
	byte message_buffer[MAXIMUM_SERVER_MESSAGE_SIZE];
	word message_length;
	long machine_index;
	boolean result = TRUE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_message_handler.c", 391, server && message);

	message_length = GET_MESSAGE_SIZE(*message);

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		struct network_client_machine *machine = network_game_server_get_client_machine_at_index(
			server,
			machine_index);

		if (network_game_server_client_machine_is_joined_to_game(server, machine))
		{
			struct network_connection *connection = network_game_server_get_client_connection(machine);

			if (connection && network_connection_active(connection))
			{
				match_assert(
					"c:\\halo\\SOURCE\\networking\\network_server_message_handler.c",
					410,
					message_length<=sizeof(message_buffer));

				memcpy(message_buffer, message, message_length);

				if (!network_game_server_write(
					connection,
					message_buffer,
					message_length,
					NULL,
					TRUE))
				{
					network_event("network_game_server_write() failed in network_game_server_send_message_to_all_machines()");
					result = FALSE;
				}
			}
		}
	}

	return result;
}

boolean network_game_server_send_player_joined_info_ingame(
	struct network_game_server *server,
	struct network_player *player)
{
	struct network_player message_player;
	message_header *message;
	boolean result;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_message_handler.c", 432, server && player);

	message_player = *player;
	message = create_network_game_message(
		_message_type_server_add_player_ingame,
		&message_player,
		sizeof(message_player));

	if (message)
	{
		result = network_game_server_send_message_to_all_machines(server, message);

		if (!result)
		{
			network_event("network_game_server_send_message_to_all_machines() failed in network_game_server_send_player_joined_info_ingame()");
		}
	}
	else
	{
		network_event("failed to create a message_server_add_player_ingame message");
		result = FALSE;
	}

	return result;
}

boolean network_game_server_send_game_data_pregame(
	struct network_game_server *server)
{
	struct network_game_data game_data;
	struct network_game_data *game;
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_message_handler.c", 456, server);

	game = network_game_server_get_game(server);

	if (game)
	{
		message_header *message;

		memcpy(&game_data, game, sizeof(game_data));
		message = create_network_game_message(
			_message_type_server_game_settings_update,
			&game_data,
			sizeof(game_data));

		if (message)
		{
			result = network_game_server_send_message_to_all_machines(server, message);

			if (!result)
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

static boolean handle_message_client_broadcast_game_search(
	struct network_game_server *server,
	struct transport_address *source_address,
	struct message_client_broadcast_game_search *client_message)
{
	boolean result = TRUE;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_server_message_handler.c",
		543,
		server && source_address && client_message);

	if (client_message->version == NETWORK_GAME_MESSAGE_VERSION)
	{
		struct network_game_data *game = network_game_server_get_game(server);

		if (game)
		{
			struct transport_address client_address;
			message_header *message;
			struct message_server_game_advertise advertisement = {0};

			client_address.address_length = IPV4_ADDRESS_LENGTH;
			client_address.address.ipv4_address = NONE;
			client_address.port = NETWORK_CLIENT_PORT;
			memcpy(
				advertisement.client_nonce,
				client_message->client_nonce,
				sizeof(client_message->client_nonce));
			transport_get_nonce(advertisement.server_nonce, sizeof(advertisement.server_nonce));
			advertisement.key_id = transport_get_key_id();
			advertisement.key = transport_get_key();
			advertisement.host_address = transport_get_xnaddr();
			advertisement.port = NETWORK_SERVER_PORT;
			advertisement.version = NETWORK_GAME_MESSAGE_VERSION;
			advertisement.platform = _game_platform_xbox;
			ustrncpy(advertisement.name, game->name, MAXIMUM_NETWORK_GAME_NAME_LENGTH - 1);
			advertisement.game_engine = (short)game->variant.game_engine_index;
			memcpy(&advertisement.map, &game->map, sizeof(game->map));
			advertisement.current_number_of_machines = game->machine_count;
			advertisement.current_number_of_players = game->player_count;
			advertisement.maximum_number_of_players = game->maximum_players;
			advertisement.score_to_win = (short)game->variant.universal_variant.score_to_win;
			advertisement.flags = 0;

			if (game->variant.universal_variant.teams == TRUE)
			{
				advertisement.flags = FLAG(_game_advertise_teams_enabled_bit);
			}

			if (game->variant.game_engine_index == _game_engine_oddball &&
				game->variant.game_engine_variant.oddball.oddball_ball_type == _oddball_terminator)
			{
				advertisement.flags |= FLAG(_game_advertise_terminator_bit);
			}

			if (network_game_server_game_is_open(server))
			{
				advertisement.flags |= FLAG(_game_advertise_open_bit);
			}

			network_game_generate_join_game_token(advertisement.join_token);
			message = create_network_game_message(
				_message_type_server_game_advertise,
				&advertisement,
				sizeof(advertisement));

			if (message)
			{
				result = network_game_server_write(
					network_game_server_get_connection(server),
					message,
					GET_MESSAGE_SIZE(*message),
					&client_address,
					FALSE);

				if (!result)
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
	struct network_game_server *server,
	struct transport_address *source_address,
	struct message_client_ping *client_message)
{
	struct message_server_pong pong;
	message_header *message;
	boolean result = FALSE;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_server_message_handler.c",
		618,
		server && source_address && client_message);

	pong.timestamp = client_message->timestamp;
	message = create_network_game_message(_message_type_server_pong, &pong, sizeof(pong));

	if (message)
	{
		struct transport_address client_address;

		client_address.address_length = IPV4_ADDRESS_LENGTH;
		client_address.address.ipv4_address = source_address->address.ipv4_address;
		client_address.port = client_message->port;
		result = network_game_server_write(
			network_game_server_get_connection(server),
			message,
			GET_MESSAGE_SIZE(*message),
			&client_address,
			FALSE);

		if (!result)
		{
			network_event("network_game_server_write() failed in handle_message_client_ping()");
		}
	}
	else
	{
		network_event("failed to create a message_server_pong message");
	}

	return result;
}

static boolean network_game_server_handle_message_client_join_game_request(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result = TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		struct message_client_join_game_request join_request;
		short packet_type = _message_type_client_join_game_request;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (network_game_server_client_machine_is_joined_to_game(server, machine))
		{
			network_event("ignoring redundant join request from machine");
		}
		else if (decode_network_game_message(
			&join_request,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_pregame))
		{
			struct transport_address client_address;

			network_connection_get_address(
				network_game_server_get_client_connection(machine),
				&client_address,
				NULL);

			if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame &&
				network_game_server_game_is_open(server))
			{
				byte join_game_token[NETWORK_JOIN_GAME_TOKEN_SIZE];

				network_game_generate_join_game_token(join_game_token);

				if (!memcmp(join_request.join_token, join_game_token, sizeof(join_game_token)))
				{
					FILE *hosts_file;
					boolean machine_in_hosts_file = TRUE;

					wide_to_ascii(
						join_request.machine_name,
						(char *)join_request.machine_name,
						sizeof(join_request.machine_name));
					hosts_file = fopen("d:\\hosts.txt", "r");

					if (hosts_file)
					{
						char host_name[MAXIMUM_HOSTS_FILE_LINE_LENGTH] = "";

						machine_in_hosts_file = FALSE;

						while (fgets(host_name, sizeof(host_name), hosts_file))
						{
							if (!strncmp(
								(char *)join_request.machine_name,
								host_name,
								strlen((char *)join_request.machine_name)))
							{
								machine_in_hosts_file = TRUE;
								break;
							}
						}

						fclose(hosts_file);
					}

					if (!machine_in_hosts_file)
					{
						struct message_server_machine_rejected rejection;
						message_header *reply;

						rejection.rejection_code = _rejection_code_blacklisted_machine;
						network_event(
							"server refused client '%s' because it is not in your hosts file",
							join_request.machine_name);
						reply = create_network_game_message(
							_message_type_server_machine_rejected,
							&rejection,
							sizeof(rejection));

						if (reply)
						{
							network_game_server_write(
								network_game_server_get_client_connection(machine),
								reply,
								GET_MESSAGE_SIZE(*reply),
								NULL,
								TRUE);
						}

						result = FALSE;
					}
					else
					{
						if (network_game_server_accept_client_machine_into_game(server, machine))
						{
							struct message_server_machine_accepted acceptance;
							struct network_machine *client_machine;
							message_header *reply;
							long machine_index = NONE;

							client_machine = network_game_server_get_client_machine(
								server,
								machine,
								&machine_index);
							network_game_server_get_game(server);
							match_assert(
								"c:\\halo\\SOURCE\\networking\\network_server_message_handler.c",
								718,
								network_machine_is_valid(client_machine));

							acceptance.machine_index = (short)machine_index;
							acceptance.server_random_seed = network_game_get_random_seed();
							reply = create_network_game_message(
								_message_type_server_machine_accepted,
								&acceptance,
								sizeof(acceptance));

							if (reply)
							{
								result = network_game_server_write(
									network_game_server_get_client_connection(machine),
									reply,
									GET_MESSAGE_SIZE(*reply),
									NULL,
									TRUE);

								if (!result)
								{
									network_event("network_game_server_write() failed in network_game_server_handle_message_client_join_game_request()");
								}
								else
								{
									network_event(
										"sent _message_type_server_machine_accepted message to %d",
										machine_index);
								}

								if (result == TRUE)
								{
									result = network_game_server_send_game_data_pregame(server);

									if (!result)
									{
										network_event("network_game_server_send_game_data_pregame() failed in network_game_server_handle_message_client_join_game_request()");
									}
								}
							}
							else
							{
								result = FALSE;
							}
						}
						else
						{
							struct message_server_machine_rejected rejection;
							message_header *reply;

							rejection.rejection_code = _rejection_code_game_is_closed;
							network_event(
								"server failed to accept valid client machine '%s' @%s into the game",
								join_request.machine_name,
								transport_address_to_string(&client_address));
							reply = create_network_game_message(
								_message_type_server_machine_rejected,
								&rejection,
								sizeof(rejection));

							if (reply)
							{
								if (!network_game_server_write(
									network_game_server_get_client_connection(machine),
									reply,
									GET_MESSAGE_SIZE(*reply),
									NULL,
									TRUE))
								{
									network_event("network_game_server_write() failed while sending a rejection reply");
								}
							}

							result = FALSE;
						}
					}
				}
				else
				{
					struct message_server_machine_rejected rejection;
					message_header *reply;

					rejection.rejection_code = _rejection_code_bad_join_token;
					network_event(
						"client machine '%s' @%s tried to join game with a bad join token",
						join_request.machine_name,
						transport_address_to_string(&client_address));
					reply = create_network_game_message(
						_message_type_server_machine_rejected,
						&rejection,
						sizeof(rejection));

					if (reply)
					{
						if (!network_game_server_write(
							network_game_server_get_client_connection(machine),
							reply,
							GET_MESSAGE_SIZE(*reply),
							NULL,
							TRUE))
						{
							network_event("network_game_server_write() failed while sending a rejection reply");
						}
					}
					else
					{
						network_event("failed to create a message_server_machine_rejected message");
					}

					result = FALSE;
				}
			}
			else
			{
				struct message_server_machine_rejected rejection;
				message_header *reply;

				rejection.rejection_code = _rejection_code_game_is_closed;
				network_event(
					"client machine '%s' @%s tried to join game when they should not be",
					join_request.machine_name,
					transport_address_to_string(&client_address));
				reply = create_network_game_message(
					_message_type_server_machine_rejected,
					&rejection,
					sizeof(rejection));

				if (reply)
				{
					if (!network_game_server_write(
						network_game_server_get_client_connection(machine),
						reply,
						GET_MESSAGE_SIZE(*reply),
						NULL,
						TRUE))
					{
						network_event("network_game_server_write() failed while sending a rejection reply");
					}
				}
				else
				{
					network_event("failed to create a message_server_machine_rejected message");
				}

				result = FALSE;
			}
		}
		else
		{
			network_event("server failed to decode a message_client_join_game_request packet");
			result = FALSE;
		}
	}

	return result;
}

static boolean network_game_server_handle_message_client_add_player_request_pregame(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		struct network_player player;
		short packet_type = _message_type_client_add_player_request_pregame;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (decode_network_game_message(
			&player,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_pregame))
		{
			if (network_game_server_add_player_to_game(server, machine, &player))
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

	return TRUE;
}

static boolean network_game_server_handle_message_client_remove_player_request_pregame(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		struct network_player player;
		short packet_type = _message_type_client_remove_player_request_pregame;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (decode_network_game_message(
			&player,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_pregame))
		{
			if (network_game_server_remove_player_from_game(server, machine, &player))
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

	return TRUE;
}

static boolean network_game_server_handle_message_client_settings_request(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		struct network_machine machine_settings;
		short packet_type = _message_type_client_settings_request;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (decode_network_game_message(
			&machine_settings,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_pregame))
		{
			if (network_game_server_adjust_machine_settings(server, machine, &machine_settings))
			{
				network_event(
					"server received machine settings for machine #%d/'%s'",
					machine_settings.machine_index,
					wide_to_ascii(
						machine_settings.name,
						(char *)machine_settings.name,
						sizeof(machine_settings.name)));

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

	return TRUE;
}

static boolean network_game_server_handle_message_client_player_settings_request(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		struct network_player player;
		short packet_type = _message_type_client_player_settings_request;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (decode_network_game_message(
			&player,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_pregame))
		{
			if (network_game_update_player(network_game_server_get_game(server), &player))
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

	return TRUE;
}

static boolean network_game_server_handle_message_client_game_start_request(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		struct message_client_game_start_request game_start_request;
		short packet_type = _message_type_client_player_settings_request;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (decode_network_game_message(
			&game_start_request,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_pregame))
		{
			network_game_server_update_countdown(server, game_start_request.request_type);
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

	return TRUE;
}

static boolean network_game_server_handle_message_client_graceful_game_exit_pregame(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		struct message_client_graceful_game_exit_pregame graceful_game_exit;
		short packet_type = _message_type_client_graceful_game_exit_pregame;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (decode_network_game_message(
			&graceful_game_exit,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_pregame))
		{
			if (network_game_server_remove_machine_from_game(
				server,
				network_game_server_get_client_machine(server, machine, NULL)))
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

	return TRUE;
}

static boolean network_game_server_handle_message_client_map_is_precached_pregame(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		struct message_client_map_is_precached_pregame map_is_precached;
		short packet_type = _message_type_client_map_is_precached_pregame;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (decode_network_game_message(
			&map_is_precached,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_pregame))
		{
			network_game_server_client_machine_is_precached(
				server,
				machine,
				map_is_precached.map_name);
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

	return TRUE;
}

static boolean network_game_server_handle_message_client_loaded(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result = TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_pregame)
	{
		struct message_client_loaded loaded;
		short packet_type = _message_type_client_loaded;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (decode_network_game_message(
			&loaded,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_ingame))
		{
			network_game_server_client_machine_game_loading_complete(server, machine);
		}
		else
		{
			network_event("server failed to decode a message_client_loaded packet");
			result = FALSE;
		}
	}
	else
	{
		network_event("failed to handle a message_client_loaded message because the server is not in pregame");
		result = FALSE;
	}

	return result;
}

static boolean network_game_server_handle_message_client_add_player_request_ingame(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	if (network_game_server_get_state(server, NULL) == _network_game_server_state_ingame)
	{
		struct network_player player;
		short packet_type = _message_type_client_add_player_request_ingame;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (decode_network_game_message(
			&player,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_ingame))
		{
			network_game_server_queue_player_for_addition(server, &player);
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

	return TRUE;
}

static boolean network_game_server_handle_message_client_remove_player_request_ingame(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result = TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_ingame)
	{
		struct network_player player;
		short packet_type = _message_type_client_add_player_request_ingame;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (decode_network_game_message(
			&player,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_ingame))
		{
			struct network_game_data *game = network_game_server_get_game(server);

			if (network_game_remove_player(game, &player))
			{
				struct message_server_remove_player_ingame remove_player;
				message_header *reply;

				remove_player.player = player;
				remove_player.quit_out_of_game_time = game_time_get() + QUIT_OUT_OF_GAME_DELAY_TICKS;
				reply = create_network_game_message(
					_message_type_server_remove_player_ingame,
					&remove_player,
					sizeof(remove_player));

				if (reply)
				{
					result = network_game_server_send_message_to_all_machines(server, reply);

					if (!result)
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
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	if (network_game_server_get_state(server, NULL) == _network_game_server_state_postgame)
	{
		struct network_player player;
		short packet_type = _message_type_client_remove_player_request_postgame;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (decode_network_game_message(
			&player,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_postgame))
		{
			if (!network_game_server_remove_player_from_game(server, machine, &player))
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

	return TRUE;
}

static boolean network_game_server_handle_message_client_switch_to_pregame(
	struct network_game_server *server,
	struct network_client_machine *machine,
	message_header *message,
	short message_size)
{
	boolean result = TRUE;

	if (network_game_server_get_state(server, NULL) == _network_game_server_state_postgame)
	{
		struct message_client_switch_to_pregame switch_to_pregame;
		short packet_type = _message_type_client_switch_to_pregame;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(message_header);

		if (decode_network_game_message(
			&switch_to_pregame,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_client_postgame))
		{
			result = network_game_server_switch_machine_from_postgame_to_pregame(server, machine);

			if (!result)
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

static boolean network_game_server_write(
	struct network_connection *connection,
	void *message,
	word message_size,
	struct transport_address *destination_address,
	boolean reliable)
{
	return network_connection_write(
		connection,
		message,
		message_size,
		destination_address,
		reliable);
}
