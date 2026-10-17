/*
NETWORK_CLIENT_MANAGER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries_windows.h"
#include "network_client_manager.h"
#include "network_client_message_handler.h"
#include "network_server_manager.h"
#include "network_game_manager.h"
#include "network_game_globals.h"
#include "network_connection.h"
#include "input.h"
#include "game.h"
#include "game_engine.h"
#include "game_engine_list.h"
#include "players.h"
#include "units.h"
#include "player_profile.h"
#include "player_ui.h"
#include "main.h"
#include "cache_files.h"
#include "ui_widget.h"
#include "saved_films.h"

/* ---------- constants */

enum
{
	NETWORK_GAME_CLIENT_SEARCH_IDLE_TIMEOUT_MILLISECONDS = 5 * MILLISECONDS_PER_SECOND, /* fake name */
	NETWORK_GAME_CLIENT_IDLE_TIMEOUT_MILLISECONDS = 15 * MILLISECONDS_PER_SECOND, /* fake name */
	NETWORK_GAME_CLIENT_PRECACHE_STATUS_INTERVAL_MILLISECONDS = MILLISECONDS_PER_SECOND, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean check_networking_and_generate_error(void);
static boolean add_advertised_game(struct advertised_game_data *available_games, struct message_server_game_advertise *message_packet);
static void network_game_client_set_error(struct network_game_client *client, word error);
static boolean network_game_client_idle_searching(struct network_game_client *client);
static boolean network_game_client_idle_joining(struct network_game_client *client);
static boolean network_game_client_idle_pregame(struct network_game_client *client);
static boolean network_game_client_idle_ingame(struct network_game_client *client);
static boolean network_game_client_idle_postgame(struct network_game_client *client);
static void network_game_client_update_precache_status(struct network_game_client *client);
static boolean network_game_client_process_incoming_messages(struct network_game_client *client);

/* ---------- globals */

boolean allow_out_of_sync = FALSE;
boolean network_game_client_dont_use_directly_in_use = FALSE;
struct network_game_client network_game_client_dont_use_directly;

/* ---------- public code */

static boolean check_networking_and_generate_error(
	void)
{
	boolean network_available = TRUE;

	if (!network_game_is_splitscreen_local())
	{
		network_available = transport_network_available();

		if (!network_available)
		{
			error(_error_silent, "network connection went down!");
			display_error_when_main_menu_loaded(_error_network_connection_lost);
		}
	}

	return network_available;
}

struct network_game_client *network_game_client_create(
	void)
{
	struct network_game_client *client = &network_game_client_dont_use_directly;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 148, !network_game_client_dont_use_directly_in_use);

	network_game_client_dont_use_directly_in_use = TRUE;
	memset(client, 0, sizeof(*client));
	client->connection = network_connection_new(FLAG(_connection_create_clientside_client_bit), NETWORK_CLIENT_PORT);

	if (client->connection)
	{
		network_game_client_reset(client, FALSE);
	}
	else
	{
		network_event("network_game_create_client() failed; could not create network connection");
		network_game_client_dispose(client);
		client = NULL;
	}

	return client;
}

void network_game_client_dispose(
	struct network_game_client *client)
{
	if (client)
	{
		if (client->connection)
		{
			network_connection_delete(client->connection);
		}

		match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 178, network_game_client_dont_use_directly_in_use);

		network_game_client_dont_use_directly_in_use = FALSE;
	}

	network_event("network client disposed");

	return;
}

boolean network_game_client_idle(
	struct network_game_client *client)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 198, client);

	switch (client->state)
	{
	case _network_game_client_state_searching:
		success = network_game_client_idle_searching(client);

		if (!success)
		{
			network_event("network_game_client_idle_searching() failed");
		}

		break;
	case _network_game_client_state_joining:
		success = network_game_client_idle_joining(client);

		if (!success)
		{
			network_event("network_game_client_idle_joining() failed");
		}

		break;
	case _network_game_client_state_pregame:
		success = network_game_client_idle_pregame(client);

		if (!success)
		{
			network_event("network_game_client_idle_pregame() failed");
		}

		break;
	case _network_game_client_state_ingame:
		success = network_game_client_idle_ingame(client);

		if (!success)
		{
			network_event("network_game_client_idle_ingame() failed");
		}

		break;
	case _network_game_client_state_postgame:
		success = network_game_client_idle_postgame(client);

		if (!success)
		{
			network_event("network_game_client_idle_postgame() failed");
		}

		break;
	default:
		match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 238, !"unknown client state");
		break;
	}

	return success;
}

void network_game_client_keep_alive(
	struct network_game_client *client)
{
	network_connection_keep_alive(client->connection);

	return;
}

word network_game_client_get_state(
	struct network_game_client *client,
	word *progress)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 249, client);

	if (progress)
	{
		*progress = 0;

		if (client->state == _network_game_client_state_joining)
		{
			*progress = (100 * system_milliseconds() - 100 * client->connection_start_time) / NETWORK_GAME_CLIENT_CONNECTION_PROCESS_TIMEOUT_MILLISECONDS_DEFAULT;
		}
	}

	return client->state;
}

boolean network_game_client_initiate_join_game(
	struct network_game_client *client,
	struct advertised_game_data *game,
	struct network_game_join_parameters *join_parameters,
	struct transport_address *address)
{
	boolean success;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		343,
		client && (client->state == _network_game_client_state_searching) && game && join_parameters && client->connection && !network_connection_connected(client->connection) && (game->platform == network_game_get_local_platform()));

	client->packet_sequence_number = NETWORK_GAME_CLIENT_INITIAL_PACKET_SEQUENCE_NUMBER;
	client->connect_process = NULL;
	client->connection_start_time = system_milliseconds();
	memcpy(&client->join_parameters, join_parameters, sizeof(client->join_parameters));
	success = network_connection_connect(client->connection, address, NULL);

	if (success == TRUE)
	{
		client->state = _network_game_client_state_joining;
		network_event(
			"attempting to connect to game @ %s",
			transport_address_to_string(address));
	}
	else
	{
		display_error_when_main_menu_loaded(_error_network_failed_to_join_game);
		network_event(
			"failed attempt to initiate a connection to game @ %s",
			transport_address_to_string(address));
	}

	return success;
}

boolean network_game_client_leave_game(
	struct network_game_client *client)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 377, client && client->connection);

	network_event("leaving network game");

	switch (client->state)
	{
	case _network_game_client_state_searching:
		match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 384, !network_connection_connected(client->connection));
		break;
	case _network_game_client_state_joining:
		if (client->connect_process)
		{
			cancel_connect_process(client->connect_process);
			client->connect_process = NULL;
		}

		if (network_connection_connected(client->connection))
		{
			success = network_connection_disconnect(client->connection);

			if (!success)
			{
				network_event("network_connection_disconnect() failed _network_game_client_state_joining");
			}
		}

		break;
	case _network_game_client_state_pregame:
	{
		struct message_client_graceful_game_exit_pregame exit_message;

		exit_message.unused = 0;

		if (network_connection_connected(client->connection))
		{
			message_header *message = create_network_game_message(
				_message_type_client_graceful_game_exit_pregame,
				&exit_message,
				sizeof(exit_message));

			if (message)
			{
				if (!network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
				{
					network_event("network_game_client_write() failed while sending a message_client_graceful_game_exit_pregame message");
				}
			}
			else
			{
				network_event("failed to create a message_client_graceful_game_exit_pregame message");
			}

			success = network_connection_disconnect(client->connection);

			if (!success)
			{
				network_event("network_connection_disconnect() failed _network_game_client_state_pregame");
			}
		}

		break;
	}
	case _network_game_client_state_ingame:
		if (network_connection_connected(client->connection))
		{
			success = network_connection_disconnect(client->connection);

			if (!success)
			{
				network_event("network_connection_disconnect() failed _network_game_client_state_ingame");
			}
		}

		break;
	case _network_game_client_state_postgame:
	{
		struct message_client_graceful_game_exit_postgame exit_message;

		exit_message.unused = 0;

		if (network_connection_connected(client->connection))
		{
			message_header *message = create_network_game_message(
				_message_type_client_graceful_game_exit_postgame,
				&exit_message,
				sizeof(exit_message));

			if (message && !network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
			{
				network_event("network_game_client_write() failed while sending a message_client_graceful_game_exit_postgame message");
			}

			success = network_connection_disconnect(client->connection);

			if (!success)
			{
				network_event("network_connection_disconnect() failed _network_game_client_state_postgame");
			}
		}

		break;
	}
	default:
		network_event("client is in an unknown state");
		break;
	}

	network_game_invalidate(&client->game);
	client->state = _network_game_client_state_searching;

	return success;
}

boolean network_game_client_set_machine(
	struct network_game_client *client,
	struct network_machine *machine)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		481,
		client && (client->machine_index<MAXIMUM_NETWORK_MACHINE_COUNT) && network_machine_is_valid(machine));

	memcpy(&client->game.machines[client->machine_index], machine, sizeof(*machine));

	return TRUE;
}

struct network_machine *network_game_client_get_machine(
	struct network_game_client *client)
{
	struct network_machine *machine;

	if (client && client->machine_index < MAXIMUM_NETWORK_MACHINE_COUNT)
	{
		machine = &client->game.machines[client->machine_index];
	}
	else
	{
		machine = NULL;
	}

	return machine;
}

short network_game_client_get_machine_index(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 509, client);

	return client->machine_index;
}

boolean network_game_client_request_remove_player(
	struct network_game_client *client,
	struct network_player *player)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 520, client && network_player_is_valid(player));
	match_vassert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		521,
		client->game.machines[client->machine_index].machine_index == player->machine_index,
		"client's can only remove players from their own machines");

	network_event(
		"requesting a player removal (controller index #%d)",
		player->controller_index);

	switch (client->state)
	{
	case _network_game_client_state_searching:
	case _network_game_client_state_joining:
		network_event("can't remove players from a game until after a game is joined");
		success = FALSE;
		break;
	case _network_game_client_state_pregame:
	{
		struct message_client_remove_player_request_pregame request;
		message_header *message;

		memcpy(&request.player, player, sizeof(request.player));
		message = create_network_game_message(
			_message_type_client_remove_player_request_pregame,
			&request,
			sizeof(request));

		if (message)
		{
			success = network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE);
		}
		else
		{
			network_event("failed to create a message_client_remove_player_request_pregame mesage");
			success = FALSE;
		}

		break;
	}
	case _network_game_client_state_ingame:
	{
		struct message_client_remove_player_request_ingame request;
		message_header *message;

		memcpy(&request.player, player, sizeof(request.player));
		message = create_network_game_message(
			_message_type_client_remove_player_request_ingame,
			&request,
			sizeof(request));

		if (message)
		{
			success = network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE);
		}
		else
		{
			network_event("failed to create a message_client_remove_player_request_ingame message");
			success = FALSE;
		}

		break;
	}
	case _network_game_client_state_postgame:
	{
		struct message_client_remove_player_request_postgame request;
		message_header *message;

		memcpy(&request.player, player, sizeof(request.player));
		message = create_network_game_message(
			_message_type_client_remove_player_request_postgame,
			&request,
			sizeof(request));

		if (message)
		{
			success = network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE);

			if (!success)
			{
				network_event("network_game_client_write() failed while sending a message_client_remove_player_request_postgame message");
			}
		}
		else
		{
			network_event("failed to create a message_client_remove_player_request_postgame message");
			success = FALSE;
		}

		break;
	}
	default:
		network_event("client is in an unknown state");
		break;
	}

	return success;
}

boolean network_game_client_remove_player(
	struct network_game_client *client,
	struct network_player *player,
	long quit_out_of_game_time)
{
	long game_player_index;
	long player_index;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 627, client && player);

	for (game_player_index = 0; game_player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; game_player_index++)
	{
		if (network_player_is_valid(&client->game.players[game_player_index]) &&
			client->game.players[game_player_index].machine_index == player->machine_index &&
			client->game.players[game_player_index].controller_index == player->controller_index)
		{
			player_index = unstrip_player_index(client->game.players[game_player_index].player_list_index);
			success = network_game_remove_player(&client->game, player);

			if (success && client->game.local_data.game_objects_loaded)
			{
				if (player_index && player_index != NONE)
				{
					long local_player_game_index;
					struct player_datum *player_datum = player_get(player_index);
					boolean local_player_found = FALSE;

					if (quit_out_of_game_time != NONE)
					{
						error(
							_error_silent,
							"%x quit of of game at tick %d (now %d)",
							player_index,
							quit_out_of_game_time,
							game_time_get());
						player_datum->quit_out_of_game_time = quit_out_of_game_time;
					}

					for (local_player_game_index = 0;
						local_player_game_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT && !local_player_found;
						local_player_game_index++)
					{
						if (network_player_is_valid(&client->game.players[local_player_game_index]) &&
							client->game.players[local_player_game_index].machine_index == client->machine_index)
						{
							local_player_found = TRUE;
							break;
						}
					}

					if (local_player_game_index == NETWORK_GAME_MAXIMUM_PLAYER_COUNT)
					{
						network_game_client_all_local_players_have_quit();
						network_event("no local players remain in the game, exiting the game now");
					}
				}
				else
				{
					error(
						_error_silent,
						"network game tried to delete a player with a phony player index (#0x%08lX)",
						player_index);
					success = FALSE;
				}
			}

			break;
		}
	}

	return success;
}

struct advertised_game_data *network_game_client_get_available_games(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 684, client);

	return client->available_games;
}

word network_game_client_get_error(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 692, client);

	return client->error;
}

short network_game_client_get_seconds_to_game_start(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 700, client);

	return client->seconds_to_game_start;
}

boolean network_game_client_write(
	struct network_connection *connection,
	message_header *message,
	word message_size,
	struct transport_address *address,
	boolean reliable)
{
	return network_connection_write(connection, message, message_size, address, reliable);
}

boolean network_game_client_address_matches_server(
	struct network_game_client *client,
	struct transport_address *address)
{
	struct transport_address server_address;
	boolean matches;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 722, client != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 723, client->connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 724, address != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 725, address->address.ipv4_address);

	network_connection_get_address(client->connection, &server_address, NULL);
	matches = server_address.address.ipv4_address == address->address.ipv4_address;

	return matches;
}

void network_game_client_game_out_of_sync(
	struct network_game_client *client)
{
	if (!allow_out_of_sync)
	{
		network_event("local machine is out of sync with the server");

		if (!client->out_of_sync)
		{
			short local_player_index;

			for (local_player_index = local_player_get_next(NONE);
				local_player_index != NONE;
				local_player_index = local_player_get_next(local_player_index))
			{
				display_error(_error_network_out_of_sync_alert, local_player_index, TRUE, FALSE);
			}
		}

		client->out_of_sync = TRUE;
	}

	return;
}

void network_game_client_new_advertised_game(
	struct network_game_client *client,
	struct message_server_game_advertise *message_packet)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 764, client && message_packet);

	add_advertised_game(client->available_games, message_packet);

	return;
}

void network_game_client_ponged(
	struct network_game_client *client,
	struct transport_address *source_address,
	unsigned long timestamp)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 775, client && source_address);

	if (client->pinging && client->ping_address.address.ipv4_address == source_address->address.ipv4_address)
	{
		unsigned long current_time = system_milliseconds();

		if (timestamp <= current_time)
		{
			client->average_ping = (current_time - timestamp + client->ping_count * client->average_ping) / (client->ping_count + 1);
			client->ping_count++;
		}
		else
		{
			network_event("received a pong from the future");
		}
	}
	else
	{
		network_event("received a pong from a system we aren't interested in");
	}

	return;
}

void network_game_client_accepted_into_game(
	struct network_game_client *client,
	struct transport_address *source_address,
	struct message_server_machine_accepted *message_packet)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		807,
		client && source_address && message_packet && (client->state == _network_game_client_state_joining));

	if (message_packet->machine_index >= 0 && message_packet->machine_index < MAXIMUM_NETWORK_MACHINE_COUNT)
	{
		struct message_client_settings_request settings_request;
		message_header *message;

		client->machine_index = message_packet->machine_index;
		client->game.machines[message_packet->machine_index].machine_index = message_packet->machine_index;
		client->state = _network_game_client_state_pregame;
		network_game_set_random_seed(message_packet->server_random_seed);
		network_event(
			"successfully joined a net game; our machine is #%d",
			message_packet->machine_index);
		network_game_generate_local_machine_name(settings_request.machine.name);
		settings_request.machine.machine_index = message_packet->machine_index;
		message = create_network_game_message(
			_message_type_client_settings_request,
			&settings_request,
			sizeof(settings_request));

		if (message)
		{
			if (!network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
			{
				network_event("network_game_client_write() failed while sending a message_client_settings_request message");
			}
		}
		else
		{
			network_event("failed to create a message_client_settings_request message");
		}
	}
	else
	{
		network_event("received a message_server_machine_accepted message with a bad machine_index");
	}

	return;
}

void network_game_client_rejected_by_game(
	struct network_game_client *client,
	struct transport_address *source_address,
	word rejection_code)
{
	char *rejection_string = "<unknown>";

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 858, client && source_address);

	client->state = _network_game_client_state_searching;

	switch (rejection_code)
	{
	case _rejection_code_version_too_old:
		rejection_string = "_rejection_code_version_too_old";
		break;
	case _rejection_code_version_too_new:
		rejection_string = "_rejection_code_version_too_new";
		break;
	case _rejection_code_bad_join_token:
		rejection_string = "_rejection_code_bad_join_token";
		break;
	case _rejection_code_bad_password:
		rejection_string = "_rejection_code_bad_password";
		break;
	case _rejection_code_game_is_full:
		rejection_string = "_rejection_code_game_is_full";
		break;
	case _rejection_code_game_is_closed:
		rejection_string = "_rejection_code_game_is_closed";
		break;
	case _rejection_code_blacklisted_machine:
		rejection_string = "_rejection_code_blacklisted_machine";
		break;
	}

	network_event(
		"unable to join game: reason= #%d/%s",
		rejection_code,
		rejection_string);
	network_game_client_reset(client, TRUE);

	return;
}

boolean network_game_client_game_settings_updated(
	struct network_game_client *client,
	struct message_server_game_settings_update *message_packet)
{
	boolean success;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 885, client && message_packet);

	if (message_packet->game.machine_count >= 0 && message_packet->game.machine_count <= MAXIMUM_NETWORK_MACHINE_COUNT &&
		message_packet->game.player_count >= 0 && message_packet->game.player_count <= NETWORK_GAME_MAXIMUM_PLAYER_COUNT)
	{
		struct network_game_data old_game;

		if (strcmp(message_packet->game.map.name, client->game.map.name))
		{
			network_event(
				"precaching map '%s'...",
				message_packet->game.map.name);
			main_set_multiplayer_map_name(message_packet->game.map.name);
		}

		memcpy(&old_game, &client->game, sizeof(old_game));
		memcpy(&client->game, &message_packet->game, sizeof(client->game));
		memcpy(&client->game.local_data, &old_game.local_data, sizeof(client->game.local_data));

		network_event(
			"received updated game settings from the server; there are %d players on %d machines in the game",
			message_packet->game.player_count,
			message_packet->game.machine_count);
		network_event(
			"player count %d machine count %d",
			message_packet->game.player_count,
			message_packet->game.machine_count);
		success = TRUE;
	}
	else
	{
		network_event(
			"invalid message_server_game_settings_update message received player count %d machine count %d",
			message_packet->game.player_count,
			message_packet->game.machine_count);
		success = FALSE;
	}

	return success;
}

long unstrip_player_index(
	long player_index)
{
	struct data_iterator iterator;
	long result = NONE;

	data_iterator_new(&iterator, player_data);

	while (data_iterator_next(&iterator))
	{
		if (DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index) == DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index))
		{
			result = iterator.index;
			break;
		}
	}

	return result;
}

boolean network_game_client_game_has_started(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 944, client && (client->state == _network_game_client_state_pregame));

	client->seconds_to_game_start = NONE;
	network_connection_keep_alive(client->connection);

	if (network_game_create_game_objects(&client->game))
	{
		struct message_client_loaded loaded_message;
		message_header *message;
		long game_player_index;

		for (game_player_index = 0; game_player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; game_player_index++)
		{
			if (client->game.players[game_player_index].machine_index == client->machine_index)
			{
				while (client->game.players[game_player_index].machine_index == client->machine_index &&
					network_player_is_valid(&client->game.players[game_player_index]))
				{
					local_player_set_player_index(
						client->game.players[game_player_index].controller_index,
						unstrip_player_index(client->game.players[game_player_index].player_list_index));
					game_player_index++;
				}

				break;
			}
		}

		network_connection_keep_alive(client->connection);
		loaded_message.unused = 0;
		message = create_network_game_message(_message_type_client_loaded, &loaded_message, sizeof(loaded_message));

		if (message)
		{
			if (network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
			{
				network_event("local machine is loaded & ready to play");
				client->state = _network_game_client_state_ingame;
				client->next_update_number = 0;
				client->last_update_time = 0;
				client->connection_going_stale = FALSE;
				ui_widgets_close_all();
				game_time_start();
				game_initial_pulse();
			}
			else
			{
				network_event("network_game_client_write() failed while sending a message_client_loaded message");
			}
		}
		else
		{
			network_event("failed to create a message_client_loaded message");
		}
	}
	else
	{
		network_event("failed to load the necessary game data");
	}

	return client->state == _network_game_client_state_ingame;
}

void network_game_client_game_shutdown(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1020, client);

	network_game_client_set_error(client, _network_game_client_error_host_closed_down);
	network_event("the game host is shutting down");
	network_game_client_all_local_players_have_quit();

	return;
}

boolean network_game_client_handle_game_update(
	struct network_game_client *client,
	struct message_server_game_update *message_packet)
{
	struct game_update update;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1037, client && message_packet);

	if (message_packet->player_count < client->game.player_count)
	{
		memset(
			&message_packet->action_update[message_packet->player_count],
			0,
			(client->game.player_count - message_packet->player_count) * sizeof(struct player_action));
		message_packet->player_count = client->game.player_count;
	}

	if (message_packet->update_number != client->next_update_number)
	{
		network_event(
			"out of sync: missed a server update (expected #%ld, got #%ld)",
			client->next_update_number,
			message_packet->update_number);
		network_game_client_game_out_of_sync(client);
	}
	else if (!global_network_game_server_get())
	{
		if (game_time_get() == message_packet->update_number && message_packet->debug_game_time != game_time_get())
		{
			network_event(
				"not a bug, but update %d time %d our time %d",
				message_packet->update_number,
				message_packet->debug_game_time,
				game_time_get());
		}

		if (game_time_get() == message_packet->debug_game_time)
		{
			unsigned long server_random_seed = message_packet->debug_random_seed;

			if (server_random_seed != get_random_seed())
			{
				network_event(
					"out of sync: client/server random seed mismatch, update= #%ld, game time= #%ld (%ld) (#%lx/#%lx)",
					message_packet->update_number,
					game_time_get(),
					message_packet->debug_game_time,
					get_random_seed(),
					server_random_seed);
				network_game_client_game_out_of_sync(client);
			}
		}

		if (!(message_packet->update_number % TICKS_PER_SECOND))
		{
			network_event(
				"client is lagging behind the server by #%d game ticks",
				message_packet->update_number - game_time_get());
		}
	}

	update.number_of_actions = message_packet->player_count;
	memcpy(update.actions, message_packet->action_update, update.number_of_actions * sizeof(struct player_action));
	update_client_handle_server_update(&update, message_packet->update_number);
	client->next_update_number++;
	client->last_update_time = system_milliseconds();

	return TRUE;
}

boolean network_game_client_add_player_to_game(
	struct network_game_client *client,
	struct network_player *player)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1122, client && player);

	if (network_player_is_valid(player))
	{
		success = network_game_add_player(&client->game, player);

		if (success)
		{
			if (client->state == _network_game_client_state_ingame)
			{
				player = &client->game.players[client->game.player_count - 1];
				success = network_game_spawn_player(player);

				if (success)
				{
					long player_index = unstrip_player_index(player->player_list_index);

					if (player->machine_index == client->machine_index)
					{
						local_player_set_player_index(player->controller_index, player_index);
					}

					update_client_add_player(player_index);

					if (global_network_game_server_get())
					{
						update_server_add_player(player_index);
					}
				}
			}

			if (success)
			{
				network_event(
					"added new player to the game (machine #%d / controller #%d)",
					player->machine_index,
					player->controller_index);
			}
		}
	}

	return success;
}

void network_game_client_switch_to_postgame(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1164, client);

	game_engine_switch_to_postgame();
	client->state = _network_game_client_state_postgame;
	network_event("switching to postgame");

	return;
}

boolean network_game_client_switch_to_pregame(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1177, client);

	if (client->state != _network_game_client_state_pregame)
	{
		network_game_reset_for_next_round(&client->game, TRUE);
		network_connection_keep_alive(client->connection);
		client->next_update_number = 0;
		client->packet_sequence_number = NETWORK_GAME_CLIENT_INITIAL_PACKET_SEQUENCE_NUMBER;
		client->last_update_time = 0;
		client->connection_going_stale = FALSE;
		client->state = _network_game_client_state_pregame;
		client->out_of_sync = FALSE;
		network_event("switching to pregame");
		network_game_reset_to_pregame_ui();
		network_connection_keep_alive(client->connection);
	}

	return TRUE;
}

struct network_connection *network_game_client_get_connection(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1203, client);

	return client->connection;
}

void network_game_client_get_remote_server_address(
	struct network_game_client *client,
	struct transport_address *address)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1212, client);

	network_connection_get_address(client->connection, address, NULL);

	return;
}

struct network_game_data *network_game_client_get_game(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1229, client);

	return &client->game;
}

boolean network_game_client_server_has_started_game(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1237, client);

	return client->next_update_number > 0;
}

long network_game_client_get_next_update_number(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1245, client);

	return client->next_update_number;
}

boolean network_client_get_oos(
	struct network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1253, client);

	return client->out_of_sync;
}

void network_game_client_reset(
	struct network_game_client *client,
	boolean teardown_connection)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1262, client);

	network_game_invalidate(&client->game);
	client->machine_index = NONE;
	client->state = _network_game_client_state_searching;

	if (teardown_connection && client->connection && network_connection_connected(client->connection))
	{
		client->packet_sequence_number = NETWORK_GAME_CLIENT_INITIAL_PACKET_SEQUENCE_NUMBER;

		if (network_connection_disconnect(client->connection))
		{
			SET_FLAG(client->flags, _network_game_client_connected_to_server_bit, FALSE);
		}
		else
		{
			network_game_client_set_error(client, _network_game_client_error_unknown);
			network_event("failed to reinitialize network game client");
		}
	}

	SET_FLAG(client->flags, _network_game_client_sent_join_request_to_server_bit, FALSE);
	client->error = _network_game_client_error_none;
	client->last_broadcast_search_time = 0;
	client->next_update_number = 0;
	client->last_update_time = 0;
	client->connection_going_stale = FALSE;
	client->out_of_sync = FALSE;
	client->seconds_to_game_start = NONE;

	return;
}

boolean network_game_client_add_player(
	struct network_game_client *client,
	short local_player_index)
{
	struct player_profile profile;
	struct network_player player;
	boolean success = TRUE;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		1328,
		client && (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	player_ui_get_active_player_profile(local_player_index, &profile);
	player.controller_index = local_player_index;
	player.machine_index = client->machine_index;
	ustrncpy(player.name, profile.player_name, MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH - 1);
	player.name[MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH - 1] = 0;
	player.primary_color_index = profile.primary_color_index;
	player.icon_index = NONE;
	player.team_index = NONE;
	player.player_list_index = NONE;
	network_event(
		"requesting a player addition (controller index #%d)",
		player.controller_index);

	switch (client->state)
	{
	case _network_game_client_state_searching:
	case _network_game_client_state_joining:
		network_event("can't add players to a game until after a game is joined");
		success = FALSE;
		break;
	case _network_game_client_state_pregame:
	{
		struct message_client_add_player_request_pregame request;
		message_header *message;

		memcpy(&request.player, &player, sizeof(request.player));
		message = create_network_game_message(
			_message_type_client_add_player_request_pregame,
			&request,
			sizeof(request));

		if (message)
		{
			success = network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE);

			if (!success)
			{
				network_event("network_game_client_write() failed while sending a message_client_add_player_request_pregame message");
			}
		}
		else
		{
			network_event("failed to create a message_client_add_player_request_pregame message");
		}

		break;
	}
	case _network_game_client_state_ingame:
	{
		struct message_client_add_player_request_ingame request;
		message_header *message;

		memcpy(&request.player, &player, sizeof(request.player));
		message = create_network_game_message(
			_message_type_client_add_player_request_ingame,
			&request,
			sizeof(request));

		if (message)
		{
			success = network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE);

			if (!success)
			{
				network_event("network_game_client_write() failed while sending a message_client_add_player_request_ingame message");
			}
		}
		else
		{
			network_event("failed to create a message_client_add_player_request_ingame message");
		}

		break;
	}
	case _network_game_client_state_postgame:
		network_event("client tried to add a new player in post-game");
		success = FALSE;
		break;
	default:
		network_event("client is in an unknown state");
		break;
	}

	return success;
}

boolean network_game_client_update_local_player_data(
	struct network_game_client *client,
	struct network_player *player)
{
	struct message_client_player_settings_request request;
	message_header *message;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1415, client && player);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1416, player->machine_index==client->machine_index);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1417, network_player_is_valid(player));

	memcpy(&request.player, player, sizeof(request.player));

	if (request.player.team_index == NONE)
	{
		request.player.team_index = 0;
	}

	message = create_network_game_message(_message_type_client_player_settings_request, &request, sizeof(request));

	if (message)
	{
		if (network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
		{
			success = TRUE;
		}
		else
		{
			network_event("network_game_client_update_local_player_data() failed while sending a message_client_player_settings_request message");
		}
	}

	return success;
}

boolean network_game_client_request_start_time_change(
	struct network_game_client *client,
	short request_type)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1445, client);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1446, (request_type>=0) && (request_type<NUMBER_OF_GAME_START_REQUESTS));

	if (client->state == _network_game_client_state_pregame)
	{
		struct message_client_game_start_request request;
		message_header *message;

		request.request_type = request_type;
		message = create_network_game_message(_message_type_client_game_start_request, &request, sizeof(request));

		if (message && !network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
		{
			network_event("network_game_client_request_start_time_change() failed to send a message_client_game_start_request message");
		}
	}
	else
	{
		network_event("failed to send a message_client_game_start_request because we are not in the pregame state");
	}

	return TRUE;
}

void network_game_client_countdown_timer_update(
	struct network_game_client *client,
	short seconds_to_game_start)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1475, client);

	client->seconds_to_game_start = seconds_to_game_start;

	return;
}

boolean network_game_client_advertised_game_is_valid(
	struct advertised_game_data *advertised_game)
{
	boolean valid = TRUE;

	if (!advertised_game->valid ||
		(long)(system_milliseconds() - advertised_game->time_in_milliseconds_of_last_news) > NETWORK_GAME_CLIENT_GAME_ADVERTISED_GAME_TIMEOUT)
	{
		valid = FALSE;
	}

	return valid;
}

/* ---------- private code */

static boolean add_advertised_game(
	struct advertised_game_data *available_games,
	struct message_server_game_advertise *message_packet)
{
	long game_index;
	boolean success = FALSE;
	boolean open = TEST_FLAG(message_packet->flags, _game_advertise_open_bit) && message_packet->current_number_of_machines < MAXIMUM_NETWORK_MACHINE_COUNT;
	struct advertised_game_data *game = NULL;

	for (game_index = 0; game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES; game_index++)
	{
		if (!network_game_client_advertised_game_is_valid(&available_games[game_index]))
		{
			memset(&available_games[game_index], 0, sizeof(available_games[game_index]));
		}
	}

	for (game_index = 0; game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES; game_index++)
	{
		struct advertised_game_data *current = &available_games[game_index];

		if (transport_nonce_is_equal(current->server_nonce, message_packet->server_nonce))
		{
			game = current;
			break;
		}
	}

	if (!game)
	{
		for (game_index = 0; game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES; game_index++)
		{
			struct advertised_game_data *current = &available_games[game_index];

			if (!current->valid)
			{
				game = current;
				break;
			}
		}
	}

	if (!game && open)
	{
		for (game_index = 0; game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES; game_index++)
		{
			struct advertised_game_data *current = &available_games[game_index];

			match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1567, current->valid);

			if (!current->open)
			{
				game = current;
				memset(game, 0, sizeof(*game));
				break;
			}
		}
	}

	if (game)
	{
		game->valid = TRUE;
		game->key = message_packet->key;
		game->key_id = message_packet->key_id;
		game->host_address = message_packet->host_address;
		memcpy(game->server_nonce, message_packet->server_nonce, sizeof(game->server_nonce));
		game->time_in_milliseconds_of_last_news = system_milliseconds();
		game->platform = message_packet->platform;

		if (message_packet->name[0])
		{
			ustrncpy(game->name, message_packet->name, MAXIMUM_NETWORK_GAME_NAME_LENGTH - 1);
		}
		else
		{
			ustrncpy(game->name, L"???", MAXIMUM_NETWORK_GAME_NAME_LENGTH - 1);
		}

		game->name[MAXIMUM_NETWORK_GAME_NAME_LENGTH - 1] = 0;
		game->game_engine = message_packet->game_engine;
		memcpy(&game->map, &message_packet->map, sizeof(game->map));
		game->current_number_of_machines = message_packet->current_number_of_machines;
		game->current_number_of_players = message_packet->current_number_of_players;
		game->maximum_number_of_players = message_packet->maximum_number_of_players;
		game->score_to_win = message_packet->score_to_win;
		game->open = open;
		game->teams_enabled = TEST_FLAG(message_packet->flags, _game_advertise_teams_enabled_bit);
		game->terminator = game->game_engine == _game_engine_oddball && TEST_FLAG(message_packet->flags, _game_advertise_terminator_bit);
		success = TRUE;

		network_event(
			"there is %s %s net game with %d players and %d machines",
			open ? "an open" : "a closed",
			game->platform == _game_platform_xbox ? "XBox" : (game->platform == _game_platform_mswindows ? "PC" : "<unknown platform>"),
			game->current_number_of_players,
			game->current_number_of_machines);
	}
	else
	{
		error(_error_silent, "not fatal, but we have to many active network games cannot add more to the list");
	}

	return success;
}

static void network_game_client_set_error(
	struct network_game_client *client,
	word error)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1634, client);

	if (error >= NUMBER_OF_NETWORK_GAME_CLIENT_ERROR_CODES)
	{
		error = _network_game_client_error_unknown;
	}

	if (client->error == _network_game_client_error_none)
	{
		client->error = error;
	}

	return;
}

static boolean network_game_client_idle_searching(
	struct network_game_client *client)
{
	boolean success;
	unsigned long current_time = system_milliseconds();

	network_connection_keep_alive(client->connection);
	success = check_networking_and_generate_error();

	if (success == TRUE)
	{
		if (global_network_game_server_get())
		{
			struct network_game_join_parameters join_parameters;
			struct transport_address server_address;
			struct advertised_game_data game = { 0 };

			server_address.address.ipv4_address = INADDR_LOOPBACK;
			server_address.port = NETWORK_SERVER_PORT;
			server_address.address_length = IPV4_ADDRESS_LENGTH;
			game.platform = network_game_get_local_platform();
			transport_get_nonce(game.server_nonce, NETWORK_GAME_NONCE_BYTES);
			join_parameters.password[0] = 0;
			network_game_generate_join_game_token(join_parameters.join_token);

			if (!network_game_client_initiate_join_game(client, &game, &join_parameters, &server_address))
			{
				success = FALSE;
				display_error_when_main_menu_loaded(_error_network_failed_to_join_game);
				network_event("network_game_client_initiate_join_game() failed");
			}
		}
		else
		{
			success = network_connection_idle(client->connection, NETWORK_GAME_CLIENT_SEARCH_IDLE_TIMEOUT_MILLISECONDS, NULL);

			if (!success)
			{
				display_error_when_main_menu_loaded(_error_network_failed_to_join_game);
				network_event("network_connection_idle() failed in network_game_client_idle_searching()");
			}
			else
			{
				success = network_game_client_process_incoming_messages(client);

				if (!success)
				{
					network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_searching()");
				}
				else
				{
					if (current_time - client->last_broadcast_search_time > NETWORK_GAME_CLIENT_GAME_SEARCH_INTERVAL_MILLISECONDS)
					{
						if (!global_network_game_server_get())
						{
							struct message_client_broadcast_game_search search;
							struct transport_address broadcast_address;
							message_header *message;

							search.port = NETWORK_CLIENT_PORT;
							search.version = NETWORK_GAME_MESSAGE_VERSION;
							transport_get_nonce(search.client_nonce, NETWORK_GAME_NONCE_BYTES);
							broadcast_address.address_length = IPV4_ADDRESS_LENGTH;
							broadcast_address.address.ipv4_address = INADDR_BROADCAST;
							broadcast_address.port = NETWORK_SERVER_PORT;
							message = create_network_game_message(
								_message_type_client_broadcast_game_search,
								&search,
								sizeof(search));

							if (message)
							{
								success = network_game_client_write(
									client->connection,
									message,
									GET_MESSAGE_SIZE(*message),
									&broadcast_address,
									FALSE);

								if (success == TRUE)
								{
									network_event("sent out a broadcast game search packet");
									client->last_broadcast_search_time = current_time;
								}
								else
								{
									network_event("network_game_client_write() failed while sending a message_client_broadcast_game_search message");
								}
							}
							else
							{
								network_event("failed to create a message_client_broadcast_game_search message");
							}
						}
					}
					else if (client->pinging == TRUE &&
						current_time -client->last_ping_time > NETWORK_GAME_CLIENT_GAME_PING_INTERVAL_MILLISECONDS)
					{
						struct message_client_ping ping;
						message_header *message;

						ping.timestamp = current_time;
						ping.port = NETWORK_CLIENT_PORT;
						message = create_network_game_message(_message_type_client_ping, &ping, sizeof(ping));

						if (message)
						{
							if (network_game_client_write(
								client->connection,
								message,
								GET_MESSAGE_SIZE(*message),
								&client->ping_address,
								FALSE))
							{
								client->last_ping_time = current_time;
							}
							else
							{
								network_event("network_game_client_write() failed while sending a message_client_ping message");
							}
						}
						else
						{
							network_event("failed to create a message_client_ping message");
						}
					}
				}
			}
		}
	}

	return success;
}

static boolean network_game_client_idle_joining(
	struct network_game_client *client)
{
	boolean success = check_networking_and_generate_error();

	if (success == TRUE)
	{
		if (network_connection_connected(client->connection))
		{
			if (!TEST_FLAG(client->flags, _network_game_client_sent_join_request_to_server_bit))
			{
				struct message_client_join_game_request request;
				message_header *message;

				memset(&request, 0, sizeof(request));
				network_game_generate_local_machine_name(request.machine_name);
				memcpy(request.join_token, client->join_parameters.join_token, sizeof(request.join_token));
				message = create_network_game_message(
					_message_type_client_join_game_request,
					&request,
					sizeof(request));

				if (message)
				{
					if (network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
					{
						SET_FLAG(client->flags, _network_game_client_sent_join_request_to_server_bit, TRUE);
					}
					else
					{
						network_event("network_game_client_write() failed to send a message_client_join_game_request message");
					}
				}
				else
				{
					network_event("failed to create a message_client_join_game_request message");
				}
			}

			client->connect_process = NULL;
		}
		else if (client->connect_process &&
			system_milliseconds() - client->connection_start_time > NETWORK_GAME_CLIENT_CONNECTION_PROCESS_TIMEOUT_MILLISECONDS_DEFAULT)
		{
			network_event("client connection process has timed out; aborting connection attempt");
			cancel_connect_process(client->connect_process);
			client->connect_process = NULL;
			success = FALSE;
		}

		if (success)
		{
			success = network_connection_idle(client->connection, NETWORK_GAME_CLIENT_SEARCH_IDLE_TIMEOUT_MILLISECONDS, NULL);

			if (success)
			{
				success = network_game_client_process_incoming_messages(client);

				if (!success)
				{
					network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_joining()");
				}
			}
			else
			{
				network_event("network_connection_idle() failed in network_game_client_idle_joining()");
			}
		}
	}

	return success;
}

static boolean network_game_client_idle_pregame(
	struct network_game_client *client)
{
	boolean success = check_networking_and_generate_error();

	if (success)
	{
		if (network_connection_active(client->connection) && network_connection_connected(client->connection))
		{
			network_game_client_update_precache_status(client);
			success = network_connection_idle(client->connection, NETWORK_GAME_CLIENT_IDLE_TIMEOUT_MILLISECONDS, NULL);

			if (!success)
			{
				network_event("network_connection_idle() failed in network_game_client_idle_pregame()");
			}
			else
			{
				success = network_game_client_process_incoming_messages(client);

				if (!success)
				{
					network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_pregame()");
				}
			}
		}
		else
		{
			success = FALSE;
		}
	}

	if (!success && !network_connection_active(client->connection))
	{
		display_error_when_main_menu_loaded(_error_network_server_shut_down);
		success = FALSE;
	}

	return success;
}

static boolean network_game_client_idle_ingame(
	struct network_game_client *client)
{
	boolean success = TRUE;

	if (network_connection_active(client->connection) && network_connection_connected(client->connection))
	{
		if (!network_game_is_splitscreen_local())
		{
			boolean connection_going_stale = network_connection_going_stale(client->connection);

			if (!transport_network_available())
			{
				display_error_when_main_menu_loaded(_error_network_connection_lost);
				network_event("network connection went down (idle in game)!");
				success = FALSE;
			}
			else if (connection_going_stale && !client->connection_going_stale)
			{
				short local_player_index;

				for (local_player_index = local_player_get_next(NONE);
					local_player_index != NONE;
					local_player_index = local_player_get_next(local_player_index))
				{
					display_error(_error_network_trouble_is_brewing, local_player_index, FALSE, FALSE);
				}

				network_event("network client connection has been silent for a dangerously long amount of time");
			}

			client->connection_going_stale = connection_going_stale;
		}
	}
	else
	{
		error(_error_silent, "new idle in game abort hit");
		display_error_when_main_menu_loaded(_error_network_server_shut_down);
		success = FALSE;
	}

	if (success == TRUE)
	{
		success = network_connection_idle(client->connection, NETWORK_GAME_CLIENT_IDLE_TIMEOUT_MILLISECONDS, NULL);

		if (success)
		{
			success = network_game_client_process_incoming_messages(client);

			if (!success)
			{
				network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_ingame()");
			}
		}
		else
		{
			if (!network_connection_active(client->connection) || !network_connection_connected(client->connection))
			{
				error(_error_silent, "new2 idle in game abort hit");
				display_error_when_main_menu_loaded(_error_network_server_shut_down);
				success = FALSE;
			}

			network_event("network_connection_idle() failed in network_game_client_idle_ingame()");
		}
	}

	return success;
}

static boolean network_game_client_idle_postgame(
	struct network_game_client *client)
{
	boolean success = check_networking_and_generate_error();

	if (success)
	{
		success = network_connection_idle(client->connection, NETWORK_GAME_CLIENT_IDLE_TIMEOUT_MILLISECONDS, NULL);

		if (!success)
		{
			network_event("network_connection_idle() failed in network_game_client_idle_postgame()");
		}
		else
		{
			success = network_game_client_process_incoming_messages(client);

			if (!success)
			{
				network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_postgame()");
			}
		}
	}

	if (!success && !network_connection_active(client->connection))
	{
		display_error_when_main_menu_loaded(_error_network_server_shut_down);
		success = FALSE;
	}

	return success;
}

static void network_game_client_update_precache_status(
	struct network_game_client *client)
{
	long current_time = system_milliseconds();

	if (current_time > client->last_precache_time + NETWORK_GAME_CLIENT_PRECACHE_STATUS_INTERVAL_MILLISECONDS)
	{
		char *map_name = main_get_multiplayer_map_name();

		client->last_precache_time = current_time;

		if (cache_files_give_time_to_precache(map_name))
		{
			message_header *message;
			struct message_client_map_is_precached_pregame precached_message = { 0 };

			strncpy(precached_message.map_name, map_name, sizeof(precached_message.map_name));
			message = create_network_game_message(
				_message_type_client_map_is_precached_pregame,
				&precached_message,
				sizeof(precached_message));

			if (message && !network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
			{
				network_event("network_game_client_write() failed while sending a message_client_graceful_game_exit_pregame message");
			}
		}
	}

	return;
}

static boolean network_game_client_process_incoming_messages(
	struct network_game_client *client)
{
	word message[NETWORK_GAME_CLIENT_MESSAGE_BUFFER_SIZE / sizeof(word)];
	struct transport_address source_address;
	boolean success = TRUE;
	word message_size = sizeof(message);

	do
	{
		if (!network_connection_read(client->connection, message, &message_size, &source_address))
		{
			break;
		}

		success = network_game_client_handle_message(client, message, message_size, &source_address);

		if (!success)
		{
			network_event("network_game_client_handle_message() failed in network_game_client_process_incoming_messages()");
		}

		message_size = sizeof(message);
	}
	while (success);

	return success;
}
