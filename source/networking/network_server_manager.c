/*
NETWORK_SERVER_MANAGER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "network_server_manager.h"
#include "network_server_message_handler.h"
#include "network_client_manager.h"
#include "network_connection.h"
#include "network_game_manager.h"
#include "network_game_globals.h"
#include "network_game_ui.h"
#include "bungie_net/network/transport.h"
#include "tag_groups.h"
#include "bungie_net/common/message_header.h"
#include "players.h"
#include "game_engine_list.h"
#include "game_engine.h"
#include "player_profile.h"
#include "main.h"
#include "ui_widget.h"
#include "cache_files.h"

/* ---------- constants */

enum
{
	_network_game_server_game_open_bit = 0, /* fake name */
	_network_game_server_game_valid_bit, /* fake name */
	NUMBER_OF_NETWORK_GAME_SERVER_FLAGS /* fake name */
};

enum
{
	_client_machine_connected_bit = 0, /* fake name */
	_client_machine_joined_bit, /* fake name */
	_client_machine_loaded_bit, /* fake name */
	_client_machine_precached_bit, /* fake name */
	NUMBER_OF_CLIENT_MACHINE_FLAGS /* fake name */
};

enum
{
	NUMBER_OF_MULTIPLAYER_TEAMS = 2, /* fake name */
	MAXIMUM_GOOD_COLOR_ATTEMPTS = 10, /* fake name */
	MAXIMUM_CLIENT_MESSAGE_SIZE = 0x800, /* fake name */
	CLIENT_STALL_TIMEOUT_MILLISECONDS = 2 * MILLISECONDS_PER_SECOND, /* fake name */
	KEEP_ALIVE_INTERVAL_MILLISECONDS = 5 * MILLISECONDS_PER_SECOND, /* fake name */
	COUNTDOWN_MESSAGE_INTERVAL_MILLISECONDS = MILLISECONDS_PER_SECOND, /* fake name */
	LEVEL_LOADING_TIMEOUT_MILLISECONDS = 15 * MILLISECONDS_PER_SECOND, /* fake name */
	GAME_START_COUNTDOWN_MILLISECONDS = 31 * MILLISECONDS_PER_SECOND - 1, /* fake name */
	SPLITSCREEN_GAME_START_COUNTDOWN_MILLISECONDS = 11 * MILLISECONDS_PER_SECOND - 1, /* fake name */
	MINIMUM_GAME_START_COUNTDOWN_MILLISECONDS = MILLISECONDS_PER_SECOND - 1, /* fake name */
	COUNTDOWN_ADJUSTMENT_MILLISECONDS = 5 * MILLISECONDS_PER_SECOND, /* fake name */
	SERVER_SHUTDOWN_DELAY_MILLISECONDS = MILLISECONDS_PER_SECOND, /* fake name */
};

/* ---------- macros */

#define client_machine_is_valid(client_machine) ((client_machine)->machine_index >= 0 && (client_machine)->machine_index < MAXIMUM_NETWORK_MACHINE_COUNT) /* fake name */

/* ---------- structures */

/* ---------- prototypes */

static void network_game_server_remove_players_from_machine_ingame(struct network_game_server *server, struct network_client_machine *client_machine);
static boolean is_name_unique(struct network_game_server *server, wchar_t const *name);
static boolean network_game_server_add_new_client(struct network_game_server *server, struct network_connection *new_connection);
static boolean network_game_server_handle_public_endpoint(struct network_game_server *server);
static boolean network_game_server_handle_client_machines(struct network_game_server *server);
static void network_game_server_send_rejection_message(struct transport_endpoint *endpoint, word reason);
static void network_game_server_reject_connection_game_is_full(struct transport_endpoint *endpoint);
static boolean network_game_server_idle_postgame_tasks(struct network_game_server *server);
static boolean network_game_server_have_all_machines_have_precached(struct network_game_server *server);
static boolean network_game_server_idle_pregame_tasks(struct network_game_server *server);
static boolean network_game_server_setup_game_from_playlist(struct network_game_server *server);
static short network_game_server_number_of_machines_connected(struct network_game_server *server);
static void dump_network_game_data(char *prefix, struct network_game_data *game);
static void network_game_server_dump(struct network_game_server *server);

/* ---------- globals */

struct network_game_server network_game_server_memory_do_not_use_directly;
boolean network_game_server_memory_do_not_use_directly_in_use = FALSE;

/* ---------- public code */

void countdown_timer_update(
	struct countdown_timer *timer)
{
	unsigned long current_time = system_milliseconds();

	if ((long)current_time > (long)timer->last_update_time)
	{
		long elapsed_time = current_time - timer->last_update_time;

		if (elapsed_time < timer->time_remaining)
		{
			timer->time_remaining -= elapsed_time;
		}
		else
		{
			timer->time_remaining = 0;
		}
	}

	timer->last_update_time = current_time;

	return;
}

long countdown_timer_get_time_remaining(
	struct countdown_timer *timer)
{
	long time_remaining;
	unsigned long current_time = system_milliseconds();
	unsigned long last_update_time = timer->last_update_time;

	if ((long)current_time > (long)last_update_time)
	{
		long elapsed_time = current_time - last_update_time;

		if (elapsed_time < timer->time_remaining)
		{
			timer->time_remaining -= elapsed_time;
		}
		else
		{
			timer->time_remaining = 0;
		}
	}

	time_remaining = timer->time_remaining;
	timer->last_update_time = current_time;
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 95, timer->time_remaining >= 0);

	return time_remaining;
}

void countdown_timer_increment(
	struct countdown_timer *timer,
	long adjustment,
	long maximum)
{
	countdown_timer_update(timer);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 104, adjustment >= 0);

	if (timer->time_remaining + adjustment < adjustment)
	{
		timer->time_remaining = maximum;
	}
	else
	{
		timer->time_remaining += adjustment;
		timer->time_remaining = MIN(timer->time_remaining, maximum);
	}

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 117, timer->time_remaining >= 0);

	return;
}

void countdown_timer_decrement(
	struct countdown_timer *timer,
	long adjustment)
{
	countdown_timer_update(timer);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 126, adjustment >= 0);

	if (timer->time_remaining > adjustment)
	{
		timer->time_remaining -= adjustment;
	}
	else
	{
		timer->time_remaining = 0;
	}

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 137, timer->time_remaining >= 0);

	return;
}

void countdown_timer_set_time_remaining(
	struct countdown_timer *timer,
	long time_remaining)
{
	unsigned long current_time = system_milliseconds();

	timer->time_remaining = time_remaining;
	timer->last_update_time = current_time;
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 149, timer->time_remaining >= 0);

	return;
}

struct network_game_server *network_game_server_create(
	void)
{
	struct network_game_server *server = &network_game_server_memory_do_not_use_directly;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 224, !network_game_server_memory_do_not_use_directly_in_use);

	network_game_server_memory_do_not_use_directly_in_use = TRUE;
	memset(server, 0, sizeof(*server));
	server->connection = network_connection_new(
		FLAG(_connection_create_server_bit),
		NETWORK_SERVER_PORT);

	if (server->connection)
	{
		long machine_index;

		transport_server_initialize();
		server->state = _network_game_server_state_pregame;
		server->flags = FLAG(_network_game_server_game_valid_bit);
		memset(&server->game, 0, sizeof(server->game));
		network_connection_set_connection_rejection_procedure(
			server->connection,
			network_game_server_reject_connection_game_is_full);
		network_game_invalidate(&server->game);
		server->game.difficulty_level = main_get_difficulty();
		server->game.number_of_games_played = NONE;

		for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
		{
			server->client_machines[machine_index].connection = NULL;
			server->client_machines[machine_index].last_received_update_sequence_number = 0;
			server->client_machines[machine_index].stall_start_time = 0;
			server->client_machines[machine_index].machine_index = NONE;
			server->client_machines[machine_index].flags = 0;
			network_game_invalidate_machine(&server->game, machine_index);
		}

		server->sent_start_game_message = FALSE;
		server->time_of_first_client_loading_completion = 0;

		if (!network_game_server_reset_to_pregame(server))
		{
			error(_error_silent, "failed to initialize server pregame settings");
			network_game_server_dispose(server);
			server = NULL;
		}
	}
	else
	{
		error(_error_silent, "failed to create the server connection");
		network_game_server_dispose(server);
		server = NULL;
	}

	return server;
}

void network_game_server_dispose(
	struct network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 288, server);

	switch (server->state)
	{
	case _network_game_server_state_pregame:
	{
		struct message_server_graceful_game_exit_pregame graceful_game_exit;
		message_header *message = create_network_game_message(
			_message_type_server_graceful_game_exit_pregame,
			&graceful_game_exit,
			sizeof(graceful_game_exit));

		if (message)
		{
			if (network_game_server_send_message_to_all_machines(server, message))
			{
				network_event("notified all clients that we are going down");
			}
			else
			{
				network_event("failed to notify all clients that we are going down");
			}
		}
		else
		{
			network_event("failed to create a _message_type_server_graceful_game_exit_pregame message");
		}
		break;
	}

	case _network_game_server_state_ingame:
		break;

	case _network_game_server_state_postgame:
	{
		struct message_server_graceful_game_exit_postgame graceful_game_exit;
		message_header *message = create_network_game_message(
			_message_type_server_graceful_game_exit_postgame,
			&graceful_game_exit,
			sizeof(graceful_game_exit));

		if (message)
		{
			if (network_game_server_send_message_to_all_machines(server, message))
			{
				network_event("notified all clients that we are going down");
			}
			else
			{
				network_event("failed to notify all clients that we are going down");
			}
		}
		else
		{
			network_event("failed to create a _message_type_server_graceful_game_exit_postgame message");
		}
		break;
	}
	}

	if (!network_game_server_handle_client_machines(server))
	{
		error(
			_error_silent,
			"network_game_server_handle_client_machines() failed inside network_game_server_dispose()");
	}

	if (server->connection)
	{
		network_connection_delete(server->connection);
	}

	SleepEx(SERVER_SHUTDOWN_DELAY_MILLISECONDS, FALSE);
	transport_server_terminate();
	memset(server, 0, sizeof(*server));
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 369, network_game_server_memory_do_not_use_directly_in_use);

	network_game_server_memory_do_not_use_directly_in_use = FALSE;
	network_event("network server disposed");

	return;
}

boolean network_game_server_idle(
	struct network_game_server *server)
{
	boolean success = TRUE;

	if (!transport_network_available() && !network_game_is_splitscreen_local())
	{
		display_error_when_main_menu_loaded(_error_network_connection_lost);
		error(_error_silent, "network connection went down!");
		success = FALSE;
	}
	else if (network_game_server_game_is_valid(server))
	{
		struct network_connection *new_client_connection = NULL;

		success = network_connection_idle(server->connection, 0, &new_client_connection);

		if (success == TRUE)
		{
			if (new_client_connection)
			{
				if (network_game_server_add_new_client(server, new_client_connection) == success)
				{
					struct transport_address client_address;

					network_connection_get_address(new_client_connection, &client_address, NULL);
					network_event(
						"new client connected from ip %s (validation pending)",
						transport_address_to_string(&client_address));
				}
				else
				{
					network_event("failed to add new client connection to the game");
					network_server_close_client_connection(
						server->connection,
						new_client_connection);
				}
			}

			success = network_game_server_handle_public_endpoint(server);

			if (success)
			{
				success = network_game_server_handle_client_machines(server);

				if (success)
				{
					switch (server->state)
					{
					case _network_game_server_state_pregame:
						success = network_game_server_idle_pregame_tasks(server);
						break;
					case _network_game_server_state_ingame:
						break;
					case _network_game_server_state_postgame:
						success = network_game_server_idle_postgame_tasks(server);
						break;
					default:
						network_event("unknown server state");
						success = FALSE;
						break;
					}
				}
				else
				{
					network_event("network_game_server_handle_client_machines() failed");
				}
			}
			else
			{
				network_event("network_game_server_handle_public_endpoint() failed");
			}
		}
		else
		{
			network_event("network_connection_idle() failed");
		}
	}
	else
	{
		network_event("the server's game is invalid");
	}

	return success;
}

boolean network_game_server_set_game_name(
	struct network_game_server *server,
	wchar_t const *name)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 477, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 478, name);

	ustrncpy(server->game.name, name, MAXIMUM_NETWORK_GAME_NAME_LENGTH - 1);
	server->game.name[MAXIMUM_NETWORK_GAME_NAME_LENGTH - 1] = 0;

	return FALSE;
}

wchar_t *network_game_server_get_game_name(
	struct network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 489, server);

	return server->game.name;
}

word network_game_server_get_state(
	struct network_game_server *server,
	word *progress)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 498, server);

	if (progress)
	{
		*progress = 0;
	}

	return server->state;
}

void network_game_server_open_game(
	struct network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 508, server);

	SET_FLAG(server->flags, _network_game_server_game_open_bit, TRUE);
	network_server_allow_client_connections(server->connection, TRUE);
	network_event("opening game");

	return;
}

void network_game_server_close_game(
	struct network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 520, server);

	SET_FLAG(server->flags, _network_game_server_game_open_bit, FALSE);
	network_server_allow_client_connections(server->connection, FALSE);
	network_event("closing game");

	return;
}

boolean network_game_server_game_is_open(
	struct network_game_server *server)
{
	boolean game_is_open;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 532, server);

	game_is_open = TEST_FLAG(server->flags, _network_game_server_game_open_bit);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 535, (TRUE == game_is_open) || (FALSE == game_is_open));

	return game_is_open;
}

boolean network_game_server_game_is_valid(
	struct network_game_server *server)
{
	boolean game_is_valid;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 544, server);

	game_is_valid = TEST_FLAG(server->flags, _network_game_server_game_valid_bit);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 547, (TRUE == game_is_valid) || (FALSE == game_is_valid));

	return game_is_valid;
}

boolean network_game_server_remove_client_machine_from_game(
	struct network_game_server *server,
	struct network_client_machine *client)
{
	long index;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 559, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 560, client);

	if (server->state == _network_game_server_state_ingame)
	{
		network_game_server_remove_players_from_machine_ingame(server, client);
	}

	for (index = 0; index < MAXIMUM_NETWORK_MACHINE_COUNT; index++)
	{
		if (server->game.machines[index].machine_index == client->machine_index)
		{
			if (!network_game_remove_machine(&server->game, &server->game.machines[index]))
			{
				error(
					_error_silent,
					"network_game_server_remove_client_machine_from_game() failed to remove the offending machine from the server's copy of the game");
			}
			break;
		}
	}

	for (index = 0; index < MAXIMUM_NETWORK_MACHINE_COUNT; index++)
	{
		if (&server->client_machines[index] == client)
		{
			if (server->client_machines[index].connection &&
				!network_server_close_client_connection(
					server->connection,
					server->client_machines[index].connection))
			{
				network_event("server failed to close a client's connection");
			}

			server->client_machines[index].connection = NULL;
			server->client_machines[index].last_received_update_sequence_number = 0;
			server->client_machines[index].stall_start_time = 0;
			server->client_machines[index].machine_index = NONE;
			server->client_machines[index].flags = 0;
			success = TRUE;
			break;
		}
	}

	if (!success)
	{
		network_event("network_game_server_remove_client_machine_from_game() failed to find the specified machine");
	}

	return success;
}

static void network_game_server_remove_players_from_machine_ingame(
	struct network_game_server *server,
	struct network_client_machine *client_machine)
{
	long player_index;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 615, _network_game_server_state_ingame == server->state);

	for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
	{
		struct network_player *player = &server->game.players[player_index];

		if (network_player_is_valid(player) && player->machine_index == client_machine->machine_index)
		{
			struct message_server_remove_player_ingame remove_player;
			message_header *message;

			remove_player.player = *player;
			remove_player.quit_out_of_game_time = game_time_get() + QUIT_OUT_OF_GAME_DELAY_TICKS;
			error(
				_error_silent,
				"sending quit out of game, time = %x",
				remove_player.quit_out_of_game_time);
			message = create_network_game_message(
				_message_type_server_remove_player_ingame,
				&remove_player,
				sizeof(remove_player));

			if (message && !network_game_server_send_message_to_all_machines(server, message))
			{
				network_event("network_game_server_send_message_to_all_machines() failed in network_game_server_handle_message_client_remove_player_request_ingame()");
			}
		}
	}

	return;
}

boolean network_game_server_remove_machine_from_game(
	struct network_game_server *server,
	struct network_machine *machine)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 665, server);

	if (machine->machine_index == NONE)
	{
		network_event("network_game_server_remove_machine_from_game called with a machine_index of NONE");
	}

	if (network_machine_is_valid(machine))
	{
		long index;

		for (index = 0; index < MAXIMUM_NETWORK_MACHINE_COUNT; index++)
		{
			if (server->client_machines[index].machine_index == machine->machine_index)
			{
				success = network_game_server_remove_client_machine_from_game(
					server,
					&server->client_machines[index]);

				if (!success)
				{
					network_event("network_game_server_remove_client_machine_from_game() failed in network_game_server_remove_machine_from_game()");
				}
				break;
			}
		}

		if (index == MAXIMUM_NETWORK_MACHINE_COUNT)
		{
			network_event("network_game_server_remove_machine_from_game() failed to find the specified machine");
		}

		if (machine->machine_index != NONE)
		{
			success = network_game_remove_machine(&server->game, machine);

			if (!success)
			{
				network_event("network_game_remove_machine() failed in network_game_server_remove_machine_from_game()");
			}
		}

		if (server->state == _network_game_server_state_pregame && !network_game_server_send_game_data_pregame(server))
		{
			network_event("network_game_server_remove_machine_from_game() failed to send updated game settings to remaining clients");
		}
	}
	else
	{
		network_event("attempted to remove an invalid machine from the game in network_game_server_remove_machine_from_game()");
		network_event("machine name = <not implemented>");
		network_event("machine index = %x", machine->machine_index);
		network_game_server_dump(server);
	}

	return success;
}

boolean network_game_server_start_network_game(
	struct network_game_server *server)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 734, server);

	if (!server->sent_start_game_message)
	{
		struct network_game_data game;
		message_header *message;
		struct message_server_begin_game begin_game = {0};

		memcpy(&game, &server->game, sizeof(game));

		if ((message = create_network_game_message(
				_message_type_server_game_settings_update,
				&game,
				sizeof(game))) &&
			network_game_server_send_message_to_all_machines(server, message) &&
			(message = create_network_game_message(
				_message_type_server_begin_game,
				&begin_game,
				sizeof(begin_game))) &&
			network_game_server_send_message_to_all_machines(server, message))
		{
			network_event("signalling client machines to begin loading for network game");
			server->sent_start_game_message = TRUE;
			success = TRUE;
		}
		else
		{
			network_event("failed to signal client machines to begin loading for network game");
		}
	}

	server->next_update_number = 0;

	return success;
}

void network_game_server_switch_to_postgame(
	struct network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 767, server);

	if (server->state == _network_game_server_state_ingame)
	{
		message_header *message;
		struct message_server_game_over game_over = {0};

		server->state = _network_game_server_state_postgame;
		message = create_network_game_message(
			_message_type_server_game_over,
			&game_over,
			sizeof(game_over));

		if (message)
		{
			if (network_game_server_send_message_to_all_machines(server, message))
			{
				network_event("server sent message_game_over to all clients");
			}
			else
			{
				network_event("failed to signal all client machines to switch to postgame");
			}
		}
		else
		{
			network_event("failed to create a _message_type_server_game_over message");
		}
	}

	return;
}

boolean network_game_server_reset_to_pregame(
	struct network_game_server *server)
{
	message_header *message;
	long player_index;
	boolean success = FALSE;
	struct message_server_switch_to_pregame switch_to_pregame = {0};

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 804, server);

	memset(&server->countdown, 0, sizeof(server->countdown));
	server->next_update_number = 0;
	server->time_of_first_client_loading_completion = 0;
	server->sent_start_game_message = FALSE;
	server->queued_player_valid = FALSE;
	server->game.number_of_games_played++;

	if (server->state == _network_game_server_state_postgame)
	{
		message = create_network_game_message(
			_message_type_server_switch_to_pregame,
			&switch_to_pregame,
			sizeof(switch_to_pregame));

		if (message && network_game_server_send_message_to_all_machines(server, message))
		{
			long machine_index;

			network_event("server resetting to pregame");

			if (server->game.variant.universal_variant.teams)
			{
				for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
				{
					if (network_player_is_valid(&server->game.players[player_index]))
					{
						switch (server->game.players[player_index].team_index)
						{
						case _team_red:
							server->game.players[player_index].team_index = _team_blue;
							break;
						case _team_blue:
							server->game.players[player_index].team_index = _team_red;
							break;
						}
					}
				}
			}

			for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
			{
				SET_FLAG(
					server->client_machines[machine_index].flags,
					_client_machine_loaded_bit,
					FALSE);
				server->client_machines[machine_index].last_received_update_sequence_number = 0;
				server->client_machines[machine_index].stall_start_time = 0;
			}

			network_game_reset_for_next_round(&server->game, FALSE);

			if (network_game_server_setup_game_from_playlist(server))
			{
				struct network_game_data game;

				memcpy(&game, &server->game, sizeof(server->game));
				message = create_network_game_message(
					_message_type_server_game_settings_update,
					&game,
					sizeof(game));

				if (message && network_game_server_send_message_to_all_machines(server, message))
				{
					server->state = _network_game_server_state_pregame;
					success = TRUE;
				}
			}
			else
			{
				struct message_server_graceful_game_exit_pregame graceful_game_exit = {0};

				message = create_network_game_message(
					_message_type_server_graceful_game_exit_pregame,
					&graceful_game_exit,
					sizeof(graceful_game_exit));

				if (message &&
					network_game_server_send_message_to_all_machines(server, message) &&
					network_game_server_handle_client_machines(server))
				{
					network_event("the playlist has ended - server going down");
				}
				else
				{
					network_event("the playlist has ended - server going down, but failed to alert client machines");
				}
			}
		}
		else
		{
			network_event("failed to signal all client machines to switch to pregame");
		}
	}
	else
	{
		success = network_game_server_setup_game_from_playlist(server);

		if (server->game.variant.universal_variant.teams)
		{
			for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
			{
				if (network_player_is_valid(&server->game.players[player_index]))
				{
					switch (server->game.players[player_index].team_index)
					{
					case _team_red:
						server->game.players[player_index].team_index = _team_blue;
						break;
					case _team_blue:
						server->game.players[player_index].team_index = _team_red;
						break;
					}
				}
			}
		}
	}

	return success;
}

boolean network_game_server_graceful_shutdown(
	struct network_game_server *server)
{
	message_header *message = NULL;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 927, server);

	switch (network_game_server_get_state(server, NULL))
	{
	case _network_game_server_state_pregame:
	{
		struct message_server_graceful_game_exit_pregame graceful_game_exit = {0};

		message = create_network_game_message(
			_message_type_server_graceful_game_exit_pregame,
			&graceful_game_exit,
			sizeof(graceful_game_exit));

		if (!message)
		{
			network_event("failed to create a message_server_graceful_game_exit_pregame");
		}
		break;
	}
	case _network_game_server_state_postgame:
	{
		struct message_server_graceful_game_exit_postgame graceful_game_exit = {0};

		message = create_network_game_message(
			_message_type_server_graceful_game_exit_postgame,
			&graceful_game_exit,
			sizeof(graceful_game_exit));

		if (!message)
		{
			network_event("failed to create a message_server_graceful_game_exit_postgame");
		}
		break;
	}
	}

	if (message)
	{
		success = network_game_server_send_message_to_all_machines(server, message);

		if (success == TRUE)
		{
			network_event("server closing down; all client machines were properly informed");
		}
		else
		{
			network_event("server going down, but failed to properly inform all client machines");
		}
	}

	return success;
}

boolean network_game_server_client_machine_is_joined_to_game(
	struct network_game_server *server,
	struct network_client_machine *machine)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 973, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 974, machine);

	return TEST_FLAG(machine->flags, _client_machine_joined_bit);
}

boolean network_game_server_accept_client_machine_into_game(
	struct network_game_server *server,
	struct network_client_machine *machine)
{
	long machine_index;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 986, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 987, machine);

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		if (server->game.machines[machine_index].machine_index < 0 ||
			server->game.machines[machine_index].machine_index >= MAXIMUM_NETWORK_MACHINE_COUNT)
		{
			struct network_machine new_machine;

			memcpy(&new_machine, &server->game.machines[machine_index], sizeof(new_machine));
			new_machine.machine_index = (char)machine_index;
			success = network_game_add_machine(&server->game, &new_machine);

			if (success == TRUE)
			{
				struct transport_address address = {0};

				network_connection_get_address(machine->connection, &address, NULL);
				network_event(
					"server added machine @ %s to the game at machine index #%d",
					transport_address_to_string(&address),
					machine_index);
				SET_FLAG(machine->flags, _client_machine_joined_bit, TRUE);
				machine->machine_index = (short)machine_index;
			}
			else
			{
				network_event("network_game_add_machine() failed in network_game_server_accept_client_machine_into_game()");
			}
			break;
		}
	}

	if (machine_index == MAXIMUM_NETWORK_MACHINE_COUNT)
	{
		network_event("network_game_server_accept_client_machine_into_game() failed to find an available opening for the new machine");
	}

	return success;
}

static boolean is_name_unique(
	struct network_game_server *server,
	wchar_t const *name)
{
	long player_index;

	for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
	{
		struct network_player *player = &server->game.players[player_index];

		if (network_player_is_valid(player) && !ustrcmp(player->name, name))
		{
			return FALSE;
		}
	}

	return TRUE;
}

void get_unique_random_name(
	struct network_game_server *server,
	struct network_player *player)
{
	wchar_t const *name;
	long duplicate_count;

	do
	{
		long player_index;

		name = network_game_get_random_player_name();
		duplicate_count = 0;

		for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
		{
			struct network_player *existing_player = &server->game.players[player_index];

			if (network_player_is_valid(existing_player) && !ustrcmp(existing_player->name, name))
			{
				duplicate_count++;
			}
		}
	}
	while (duplicate_count);

	ustrncpy(player->name, name, MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH - 1);
	player->name[MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH - 1] = 0;

	return;
}

void get_unique_random_color(
	struct network_game_server *server,
	struct network_player *player)
{
	long color_index;
	boolean unique;
	long attempt_count = 0;

	do
	{
		long player_index;

		color_index = attempt_count < MAXIMUM_GOOD_COLOR_ATTEMPTS ?
			player_profile_get_random_good_color() :
			player_profile_get_random_color();
		unique = TRUE;

		for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
		{
			if (network_player_is_valid(&server->game.players[player_index]) &&
				server->game.players[player_index].primary_color_index == color_index)
			{
				unique = FALSE;
				break;
			}
		}

		attempt_count++;
	}
	while (!unique);

	player->primary_color_index = (short)color_index;

	return;
}

boolean network_game_server_add_player_to_game(
	struct network_game_server *server,
	struct network_client_machine *machine,
	struct network_player *player)
{
	boolean success;
	static long next_team_index = 0;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1132, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1133, machine);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1134, player);

	if (machine->machine_index == player->machine_index)
	{
		player->team_index = (char)next_team_index;
		next_team_index = (next_team_index + 1) % NUMBER_OF_MULTIPLAYER_TEAMS;

		if (!player->name[0])
		{
			get_unique_random_name(server, player);
		}

		if (!is_name_unique(server, player->name))
		{
			get_unique_random_name(server, player);
		}

		if (player->primary_color_index == NONE)
		{
			get_unique_random_color(server, player);
		}

		success = network_game_add_player(&server->game, player);

		if (success == TRUE)
		{
			network_event(
				"server added player from machine #%d at controller index #%d to the game",
				player->machine_index,
				player->controller_index);
		}
		else
		{
			network_event("network_game_add_player() failed in network_game_server_add_player_to_game()");
		}
	}
	else
	{
		network_event("client machine tried to add a player with a non-matching machine identifier");
		success = FALSE;
	}

	return success;
}

boolean network_game_server_remove_player_from_game(
	struct network_game_server *server,
	struct network_client_machine *machine,
	struct network_player *player)
{
	boolean success;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1184, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1185, machine);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1186, player);

	if (machine->machine_index == player->machine_index)
	{
		success = network_game_remove_player(&server->game, player);

		if (success == TRUE)
		{
			network_event(
				"server removed player from machine #%d at controller index #%d from the game",
				player->machine_index,
				player->controller_index);
		}
		else
		{
			network_event("network_game_remove_player() failed in network_game_server_remove_player_from_game()");
		}
	}
	else
	{
		network_event("client machine tried to remove a player with a non-matching machine identifier");
		success = FALSE;
	}

	return success;
}

boolean network_game_server_adjust_machine_settings(
	struct network_game_server *server,
	struct network_client_machine *machine,
	struct network_machine *machine_description)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1215, server && machine && machine_description);

	if (machine->machine_index == machine_description->machine_index)
	{
		success = network_game_update_machine(&server->game, machine_description);

		if (success == TRUE)
		{
			network_event(
				"server updated machine #%d settings",
				machine_description->machine_index);
		}
		else
		{
			network_event("network_game_update_machine() failed in network_game_server_adjust_machine_settings()");
		}
	}
	else
	{
		network_event("client machine tried to update itself with a non-matching machine identifier");
	}

	return success;
}

void network_game_server_all_machines_have_loaded(
	struct network_game_server *server)
{
	network_event("all machines have successfully loaded");
	server->state = _network_game_server_state_ingame;
	server->time_of_first_client_loading_completion = 0;
	server->game.local_data.game_objects_loaded = global_network_game_client_get() ?
		network_game_client_get_game(global_network_game_client_get())->local_data.game_objects_loaded :
		FALSE;
	match_vassert(
		"c:\\halo\\SOURCE\\networking\\network_server_manager.c",
		1248,
		server->game.local_data.game_objects_loaded,
		"local game data not loaded");

	return;
}

void network_game_server_client_machine_game_loading_complete(
	struct network_game_server *server,
	struct network_client_machine *machine)
{
	long machine_index;
	boolean all_machines_loaded = TRUE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1261, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1262, machine);

	SET_FLAG(machine->flags, _client_machine_loaded_bit, TRUE);

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		struct network_client_machine *client_machine = &server->client_machines[machine_index];

		if (client_machine_is_valid(client_machine) &&
			!TEST_FLAG(client_machine->flags, _client_machine_loaded_bit))
		{
			network_event(
				"still waiting on machine #%d to finish loading",
				client_machine->machine_index);
			all_machines_loaded = FALSE;
		}
	}

	if (all_machines_loaded == TRUE)
	{
		network_game_server_all_machines_have_loaded(server);
	}

	if (!server->time_of_first_client_loading_completion)
	{
		server->time_of_first_client_loading_completion = system_milliseconds();
	}

	return;
}

void network_game_server_client_machine_is_precached(
	struct network_game_server *server,
	struct network_client_machine *machine,
	char const *map_name)
{
	char *multiplayer_map_name = main_get_multiplayer_map_name();

	if (!strcmp(multiplayer_map_name, map_name))
	{
		SET_FLAG(machine->flags, _client_machine_precached_bit, TRUE);
	}

	return;
}

void network_game_server_handle_client_update_packet(
	struct network_game_server *server,
	struct network_client_machine *machine,
	struct message_client_game_update *message_packet)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1310, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1311, machine);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1312, message_packet);

	if (TEST_FLAG(message_packet->bits, _client_update_out_of_sync_bit))
	{
		network_event(
			"client machine #%d is out of sync @ game tick #%ld; switching to post-game",
			machine->machine_index,
			game_time_get());
		game_engine_switch_to_postgame();
	}
	else if ((message_packet->bits & CLIENT_UPDATE_NUMBER_MASK) < machine->last_received_update_sequence_number)
	{
		network_event(
			"received an outdated client update packet; ignoring (#%d / #%d)",
			message_packet->bits & CLIENT_UPDATE_NUMBER_MASK,
			machine->last_received_update_sequence_number);
	}
	else if (message_packet->player_count < 0 || message_packet->player_count > MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
	{
		network_event(
			"client update packet from machine #%d had a bad player count; ignoring",
			machine->machine_index);
	}
	else
	{
		long player_index;
		struct player_action actions[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS] = {0};

		for (player_index = 0; player_index < message_packet->player_count; player_index++)
		{
			actions[player_index] = message_packet->action_update[player_index];
		}

		update_server_handle_client_update(machine->machine_index, actions);
		machine->last_received_update_sequence_number = message_packet->bits & CLIENT_UPDATE_NUMBER_MASK;
	}

	return;
}

boolean network_game_server_switch_machine_from_postgame_to_pregame(
	struct network_game_server *server,
	struct network_client_machine *machine)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1351, server && machine);

	network_event("machine #%d has successfully switched to pregame", machine->machine_index);
	SET_FLAG(machine->flags, _client_machine_loaded_bit, FALSE);

	return TRUE;
}

void network_game_server_update_ticks(
	struct network_game_server *server,
	short tick_count)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1363, server);

	switch (network_game_server_get_state(server, NULL))
	{
	case _network_game_server_state_ingame:
	{
		long machine_index;
		short tick_index;
		struct network_client_machine *client_machine = NULL;

		for (tick_index = 0; tick_index < tick_count; tick_index++)
		{
			struct message_server_game_update game_update;
			struct game_update update;
			message_header *message;
			long update_number = server->next_update_number++;

			update_server_next_update();
			update_server_build_server_update(NONE, &update, &update_number);
			game_update.update_number = update_number;
			game_update.debug_random_seed = get_random_seed();
			game_update.debug_game_time = game_time_get();
			game_update.player_count = update.number_of_actions;
			memcpy(
				game_update.action_update,
				update.actions,
				update.number_of_actions * sizeof(struct player_action));
			message = create_network_game_message(
				_message_type_server_game_update,
				&game_update,
				sizeof(game_update));

			if (message && !network_game_server_send_message_to_all_machines(server, message))
			{
				network_event("server failed to send game update message to all machines; client machine may be out of sync");
			}
		}

		if (server->queued_player_valid)
		{
			for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
			{
				if (server->client_machines[machine_index].machine_index == server->queued_player.machine_index)
				{
					client_machine = &server->client_machines[machine_index];
					break;
				}
			}

			if (client_machine &&
				network_game_server_add_player_to_game(
					server,
					client_machine,
					&server->queued_player))
			{
				if (!network_game_server_send_player_joined_info_ingame(
					server,
					&server->queued_player))
				{
					network_event("network_game_server_send_player_joined_info_ingame() failed in network_game_server_handle_message_client_add_player_request_ingame()");
				}
			}
			else
			{
				network_event("server failed to add a network player in-game");
			}

			server->queued_player_valid = FALSE;
		}
		break;
	}
	case _network_game_server_state_postgame:
		game_engine_update();
		break;
	}

	return;
}

void network_game_server_stalled_on_client(
	struct network_game_server *server,
	boolean stalled)
{
	long machine_index;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1438, server);

	if (stalled)
	{
		unsigned long oldest_update = (unsigned long)NONE;
		long culprit = NONE;

		for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
		{
			if (client_machine_is_valid(&server->client_machines[machine_index]) &&
				server->client_machines[machine_index].last_received_update_sequence_number < oldest_update)
			{
				oldest_update = server->client_machines[machine_index].last_received_update_sequence_number;
				culprit = machine_index;
			}
		}

		match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1457, culprit != NONE);

		if (server->client_machines[culprit].stall_start_time)
		{
			if (system_milliseconds() - server->client_machines[culprit].stall_start_time >= CLIENT_STALL_TIMEOUT_MILLISECONDS)
			{
				char machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH];
				boolean removed;

				network_event(
					"forcibly removing client system '%s' due to timeout in-game",
					wide_to_ascii(
						server->game.machines[server->client_machines[culprit].machine_index].name,
						machine_name,
						sizeof(machine_name)) ? machine_name : "<unknown name>");
				removed = network_game_server_remove_client_machine_from_game(
					server,
					&server->client_machines[culprit]);
				match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1473, removed);
			}
		}
		else
		{
			server->client_machines[culprit].stall_start_time = system_milliseconds();
		}

		for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
		{
			if (machine_index != culprit)
			{
				server->client_machines[machine_index].stall_start_time = 0;
			}
		}
	}
	else
	{
		for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
		{
			server->client_machines[machine_index].stall_start_time = 0;
		}
	}

	return;
}

void network_game_server_queue_player_for_addition(
	struct network_game_server *server,
	struct network_player *player)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1502, server && player);

	if (!server->queued_player_valid && network_player_is_valid(player))
	{
		memcpy(&server->queued_player, player, sizeof(server->queued_player));
		server->queued_player_valid = TRUE;
	}

	return;
}

void network_game_server_begin_game_start_countdown(
	struct network_game_server *server,
	long time_remaining)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1517, server);

	if (!server->countdown.active && !server->countdown.paused)
	{
		countdown_timer_set_time_remaining(&server->countdown.timer, time_remaining);
		server->countdown.adjusted_this_tick = FALSE;
		server->countdown.active = TRUE;
		network_event("server game start countdown started");
	}

	return;
}

boolean server_needs_more_teams(
	struct network_game_server *server)
{
	boolean needs_more_teams = FALSE;

	if (server->game.variant.universal_variant.teams)
	{
		long player_index;
		long team_index;
		short team_player_counts[NUMBER_OF_MULTIPLAYER_TEAMS] = {0, 0};

		for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
		{
			struct network_player *player = &server->game.players[player_index];

			if (network_player_is_valid(player) &&
				player->team_index >= 0 &&
				player->team_index < NUMBER_OF_MULTIPLAYER_TEAMS)
			{
				team_player_counts[player->team_index]++;
			}
		}

		for (team_index = 0; team_index < NUMBER_OF_MULTIPLAYER_TEAMS; team_index++)
		{
			if (!team_player_counts[team_index])
			{
				needs_more_teams = TRUE;
				break;
			}
		}
	}

	return needs_more_teams;
}

boolean server_has_a_player_on_each_machine(
	struct network_game_server *server)
{
	long machine_index;
	boolean result = TRUE;

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		struct network_client_machine *client_machine = &server->client_machines[machine_index];

		if (client_machine_is_valid(client_machine))
		{
			long player_index;
			boolean machine_has_player = FALSE;

			for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
			{
				if (network_player_is_valid(&server->game.players[player_index]) &&
					server->game.players[player_index].machine_index == client_machine->machine_index)
				{
					machine_has_player = TRUE;
				}
			}

			if (!machine_has_player)
			{
				result = FALSE;
				break;
			}
		}
	}

	return result;
}

boolean server_has_enough_machines(
	struct network_game_server *server)
{
	boolean has_enough_machines;
	long machine_index;
	long minimum_machine_count = network_game_is_splitscreen_local() ? 1 : 2;
	long machine_count = 0;

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		if (client_machine_is_valid(&server->client_machines[machine_index]))
		{
			machine_count++;
		}
	}

	has_enough_machines = machine_count >= minimum_machine_count;

	return has_enough_machines;
}

boolean server_ok_to_countdown(
	struct network_game_server *server)
{
	if (server_has_enough_machines(server) &&
		server_has_a_player_on_each_machine(server) &&
		!server_needs_more_teams(server) &&
		server->game.player_count >= server->game.minimum_players)
	{
		return TRUE;
	}

	return FALSE;
}

void network_game_server_update_countdown(
	struct network_game_server *server,
	short countdown_event)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1646, server && server->state == _network_game_server_state_pregame);

	if (!server->countdown.paused && (server_ok_to_countdown(server) || countdown_event == _countdown_event_stop))
	{
		if (server->countdown.active == TRUE)
		{
			if (!server->countdown.adjusted_this_tick)
			{
				switch (countdown_event)
				{
				case _countdown_event_player_left:
					server->countdown.adjusted_this_tick = TRUE;
					countdown_timer_increment(
						&server->countdown.timer,
						COUNTDOWN_ADJUSTMENT_MILLISECONDS,
						GAME_START_COUNTDOWN_MILLISECONDS);
					break;
				case _countdown_event_player_joined:
					server->countdown.adjusted_this_tick = TRUE;

					if (countdown_timer_get_time_remaining(&server->countdown.timer) > MINIMUM_GAME_START_COUNTDOWN_MILLISECONDS)
					{
						countdown_timer_decrement(
							&server->countdown.timer,
							COUNTDOWN_ADJUSTMENT_MILLISECONDS);

						if (countdown_timer_get_time_remaining(&server->countdown.timer) < MINIMUM_GAME_START_COUNTDOWN_MILLISECONDS)
						{
							countdown_timer_set_time_remaining(
								&server->countdown.timer,
								MINIMUM_GAME_START_COUNTDOWN_MILLISECONDS);
						}
					}
					break;
				case _countdown_event_stop:
					server->countdown.active = FALSE;
					server->countdown.adjusted_this_tick = TRUE;
					break;
				case _countdown_event_start_immediately:
					server->countdown.adjusted_this_tick = TRUE;
					countdown_timer_set_time_remaining(&server->countdown.timer, 0);
					break;
				}
			}
		}
		else
		{
			unsigned long current_time = system_milliseconds();

			if (countdown_event == _countdown_event_start_immediately)
			{
				countdown_timer_set_time_remaining(&server->countdown.timer, 0);
				server->countdown.active = TRUE;
				server->countdown.adjusted_this_tick = FALSE;
			}
			else if (!network_game_should_accept_remote_connections() ||
				network_game_server_number_of_machines_connected(server) > 1)
			{
				long countdown_time = network_game_is_splitscreen_local() ?
					SPLITSCREEN_GAME_START_COUNTDOWN_MILLISECONDS :
					GAME_START_COUNTDOWN_MILLISECONDS;

				server->countdown.active = TRUE;
				countdown_timer_set_time_remaining(&server->countdown.timer, countdown_time);
				server->countdown.adjusted_this_tick = FALSE;
				server->countdown.last_message_time = 0;
			}
		}
	}

	return;
}

void network_game_server_invalidate_network_machine(
	struct network_machine *machine)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1737, machine);

	memset(machine, 0, sizeof(*machine));
	machine->machine_index = NONE;

	return;
}

void network_game_generate_join_game_token(
	byte join_token[NETWORK_JOIN_GAME_TOKEN_SIZE])
{
	char const join_token_seed[19] = {'m', 'e', 's', 's', 'a', 'g', 'e', ' ', 'i', 'n', ' ', 'a', ' ', 'b', 'o', 't', 't', 'l', 'e'}; // no \0 terminate

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1754, join_token);

	memset(join_token, 0, sizeof(join_token));
	memcpy(join_token, join_token_seed, MIN(NETWORK_JOIN_GAME_TOKEN_SIZE, sizeof(join_token_seed)));

	return;
}

struct network_machine *network_game_server_get_client_machine(
	struct network_game_server *server,
	struct network_client_machine *client_machine,
	long *machine_index)
{
	struct network_machine *machine;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1793, server && client_machine);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1794, client_machine->machine_index<MAXIMUM_NETWORK_MACHINE_COUNT);

	if (machine_index)
	{
		*machine_index = NONE;
	}

	machine = &server->game.machines[client_machine->machine_index];

	if (machine_index)
	{
		*machine_index = machine->machine_index;
	}

	return machine;
}

struct network_connection *network_game_server_get_connection(
	struct network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1810, server);

	return server->connection;
}

struct network_connection *network_game_server_get_client_connection(
	struct network_client_machine *client_machine)
{
	return client_machine ? client_machine->connection : NULL;
}

struct network_connection *network_game_server_get_machine_connection(
	struct network_game_server *server,
	struct network_machine *machine)
{
	long index;
	struct network_connection *connection = NULL;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1839, server && network_machine_is_valid(machine));

	for (index = 0; index < MAXIMUM_NETWORK_MACHINE_COUNT; index++)
	{
		if (server->client_machines[index].machine_index == machine->machine_index)
		{
			connection = server->client_machines[index].connection;
			break;
		}
	}

	return connection;
}

struct network_client_machine *network_game_server_get_client_machine_at_index(
	struct network_game_server *server,
	long index)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1857, server && (index<MAXIMUM_NETWORK_MACHINE_COUNT));

	return &server->client_machines[index];
}

struct network_client_machine *network_game_server_get_client_machine_at_address(
	struct network_game_server *server,
	unsigned long ip_address)
{
	long i;
	struct network_client_machine *client_machine = NULL;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1869, server && ip_address);

	for (i = 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
	{
		if (client_machine_is_valid(&server->client_machines[i]))
		{
			struct transport_address address;

			match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1877, server->client_machines[i].connection);

			network_connection_get_address(server->client_machines[i].connection, &address, NULL);

			if (address.address.ipv4_address == ip_address)
			{
				client_machine = &server->client_machines[i];
				break;
			}
		}
	}

	if (i == MAXIMUM_NETWORK_MACHINE_COUNT)
	{
		network_event("no machine found @ ip #%lX", ip_address);
	}

	return client_machine;
}

struct network_game_data *network_game_server_get_game(
	struct network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1897, server);

	return &server->game;
}

unsigned long network_game_server_get_oldest_client_update_received(
	struct network_game_server *server)
{
	long index;
	unsigned long oldest_update = (unsigned long)NONE;

	for (index = 0; index < MAXIMUM_NETWORK_MACHINE_COUNT; index++)
	{
		struct network_client_machine *client_machine = &server->client_machines[index];

		if (client_machine_is_valid(client_machine))
		{
			oldest_update = MIN(
				oldest_update,
				client_machine->last_received_update_sequence_number);
		}
	}

	return oldest_update;
}

boolean network_game_server_game_can_start(
	struct network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1922, server);

	return server->state == _network_game_server_state_pregame && server->game.player_count >= server->game.minimum_players;
}

void network_game_server_pause_countdown(
	struct network_game_server *server,
	boolean pause)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1932, server);

	if (pause == TRUE)
	{
		memset(&server->countdown, 0, sizeof(server->countdown));
	}

	server->countdown.paused = pause;

	return;
}

void network_game_server_change_map_name(
	struct network_game_server *server,
	char const *map_name)
{
	long machine_index;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1947, server && map_name && map_name[0]);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1948, server->state == _network_game_server_state_pregame);

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		struct network_client_machine *client_machine = &server->client_machines[machine_index];

		if (client_machine_is_valid(client_machine))
		{
			SET_FLAG(client_machine->flags, _client_machine_precached_bit, FALSE);
		}
	}

	strncpy(server->game.map.name, map_name, MAXIMUM_NETWORK_MAP_NAME_LENGTH - 1);
	server->game.map.name[MAXIMUM_NETWORK_MAP_NAME_LENGTH - 1] = 0;

	if (!network_game_server_send_game_data_pregame(server))
	{
		network_event("network_game_server_change_map_name() failed to send updated game settings to clients");
	}

	return;
}

void network_game_server_change_game_variant(
	struct network_game_server *server,
	struct game_variant *variant)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1982, server && variant);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1983, server->state == _network_game_server_state_pregame);

	memcpy(&server->game.variant, variant, sizeof(server->game.variant));

	if (!network_game_server_send_game_data_pregame(server))
	{
		network_event("network_game_server_change_game_variant() failed to send updated game settings to clients");
	}

	return;
}

/* ---------- private code */

static boolean network_game_server_add_new_client(
	struct network_game_server *server,
	struct network_connection *new_connection)
{
	long machine_index;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2004, server && new_connection);

	if (network_game_server_game_is_open(server))
	{
		for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
		{
			if (server->client_machines[machine_index].machine_index == NONE)
			{
				struct transport_address client_address = {0};

				network_connection_get_address(new_connection, &client_address, NULL);

				if (client_address.address.ipv4_address)
				{
					if (!network_game_should_accept_remote_connections() &&
						client_address.address.ipv4_address != INADDR_LOOPBACK)
					{
						network_event(
							"remote system tried to join our server but we are not accepting remote connections: address= '%s'",
							transport_address_to_string(&client_address));
					}
					else
					{
						server->client_machines[machine_index].connection = new_connection;
						network_game_invalidate_machine(&server->game, machine_index);
						server->client_machines[machine_index].machine_index = (short)machine_index;
						server->client_machines[machine_index].flags = FLAG(_client_machine_connected_bit);
						success = network_connection_server_accept_client_connection(
							server->connection,
							new_connection);

						if (success == TRUE)
						{
							network_event(
								"new remote connection accepted from %s",
								transport_address_to_string(&client_address));
						}
					}
				}
				else
				{
					network_event("network_connection_get_address() failed to get a valid address in network_game_server_add_new_client()");
				}
				break;
			}
		}

		if (machine_index == MAXIMUM_NETWORK_MACHINE_COUNT)
		{
			network_event("failed to find an available machine slot in network_game_server_add_new_client()");
		}
	}
	else
	{
		network_event("network_game_server_add_new_client() failed because the game is closed");
	}

	return success;
}

static boolean network_game_server_handle_public_endpoint(
	struct network_game_server *server)
{
	word datagram[DATAGRAM_MAXIMUM_SIZE / sizeof(word)];
	struct transport_address source_address;
	boolean success = TRUE;
	word datagram_size = sizeof(datagram);

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2064, server);

	while (success && network_connection_read(
		server->connection,
		datagram,
		&datagram_size,
		&source_address))
	{
		success = network_game_server_handle_datagram(
			server,
			datagram,
			datagram_size,
			&source_address);

		if (!success)
		{
			network_event("network_game_server_handle_datagram() failed in network_game_server_handle_public_endpoint()");
		}

		datagram_size = sizeof(datagram);
	}

	return success;
}

static boolean network_game_server_handle_client_machines(
	struct network_game_server *server)
{
	long machine_index;
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2087, server);

	for (machine_index = 0; success && machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		if (server->client_machines[machine_index].machine_index != NONE)
		{
			if (!network_connection_active(server->client_machines[machine_index].connection))
			{
				if (network_game_server_remove_machine_from_game(
					server,
					&server->game.machines[server->client_machines[machine_index].machine_index]))
				{
					network_event(
						"client machine %x removed from game",
						server->client_machines[machine_index].machine_index);
					network_game_server_dump(server);
				}
				else
				{
					network_event(
						"failed to remove client machine %x from game",
						server->client_machines[machine_index].machine_index);
					network_game_server_dump(server);
				}
			}
			else if (network_connection_idle(server->client_machines[machine_index].connection, 0, NULL) &&
				network_connection_connected(server->client_machines[machine_index].connection))
			{
				word message[MAXIMUM_CLIENT_MESSAGE_SIZE / sizeof(word)];
				word message_size = sizeof(message);

				while (success && network_connection_read(
					server->client_machines[machine_index].connection,
					message,
					&message_size,
					NULL))
				{
					if (network_game_server_handle_client_message(
						server,
						&server->client_machines[machine_index],
						message,
						message_size))
					{
						message_size = sizeof(message);
					}
					else
					{
						network_event("network_game_server_handle_client_message() failed in network_game_server_handle_client_machines()");

						if (network_game_server_remove_machine_from_game(
							server,
							&server->game.machines[server->client_machines[machine_index].machine_index]))
						{
							network_event(
								"client machine removed from game",
								server->client_machines[machine_index].machine_index);
						}
						else if (!network_game_server_remove_client_machine_from_game(
							server,
							&server->client_machines[machine_index]))
						{
							network_event(
								"failed to remove client machine from game",
								server->client_machines[machine_index].machine_index);
						}

						break;
					}
				}
			}
			else if (network_game_server_remove_machine_from_game(
				server,
				&server->game.machines[server->client_machines[machine_index].machine_index]))
			{
				network_event(
					"client machine removed from game",
					server->client_machines[machine_index].machine_index);
			}
			else
			{
				network_event(
					"failed to remove client machine from game",
					server->client_machines[machine_index].machine_index);
			}
		}
	}

	return success;
}

static void network_game_server_send_rejection_message(
	struct transport_endpoint *endpoint,
	word reason)
{
	message_header *message;
	struct message_server_machine_rejected rejection = {reason};

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2168, endpoint && (reason < NUMBER_OF_SERVER_REJECTION_CODES));

	message = create_network_game_message(
		_message_type_server_machine_rejected,
		&rejection,
		sizeof(rejection));

	if (message)
	{
		long bytes_written;
		long message_size = GET_MESSAGE_SIZE(*message);

		byte_swap_message_header(message, _byte_order_network);
		bytes_written = write_endpoint(endpoint, message, message_size);

		if (bytes_written != message_size)
		{
			network_event(
				"error sending rejection message to client; transport error= '%s'",
				transport_error_to_string((short)bytes_written));
		}
	}
	else
	{
		network_event("failed to create a message_server_machine_rejected message in network_game_server_send_rejection_message");
	}

	return;
}

static void network_game_server_reject_connection_game_is_full(
	struct transport_endpoint *endpoint)
{
	network_event("client connection refused; game is full");
	network_game_server_send_rejection_message(endpoint, _rejection_code_game_is_full);

	return;
}

static boolean network_game_server_idle_postgame_tasks(
	struct network_game_server *server)
{
	unsigned long current_time = system_milliseconds();

	if (current_time > server->time_of_last_keep_alive + KEEP_ALIVE_INTERVAL_MILLISECONDS)
	{
		message_header *message;
		struct message_server_postgame_keep_alive keep_alive = {0};

		message = create_network_game_message(
			_message_type_server_postgame_keep_alive,
			&keep_alive,
			sizeof(keep_alive));
		network_game_server_send_message_to_all_machines(server, message);
		server->time_of_last_keep_alive = current_time;
	}

	return TRUE;
}

static boolean network_game_server_have_all_machines_have_precached(
	struct network_game_server *server)
{
	long machine_index;
	boolean all_machines_have_precached = TRUE;

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		struct network_client_machine *client_machine = &server->client_machines[machine_index];

		if (client_machine_is_valid(client_machine))
		{
			boolean client_has_precached = TEST_FLAG(client_machine->flags, _client_machine_precached_bit);

			if (!client_has_precached)
			{
				all_machines_have_precached = FALSE;
				break;
			}
		}
	}

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2248, !all_machines_have_precached || cache_files_precache_map_loaded(main_get_multiplayer_map_name()));

	return all_machines_have_precached;
}

static boolean network_game_server_idle_pregame_tasks(
	struct network_game_server *server)
{
	unsigned long current_time = system_milliseconds();
	boolean success = TRUE;

	if (!server->sent_start_game_message)
	{
		long machine_index;

		for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
		{
			struct network_client_machine *client_machine = &server->client_machines[machine_index];

			if (client_machine->connection && !network_connection_active(client_machine->connection))
			{
				network_event("booting dead client machine %d", machine_index);
				network_game_server_remove_client_machine_from_game(server, client_machine);
			}
		}

		if (server->countdown.active == TRUE)
		{
			boolean send_countdown_update = FALSE;
			boolean ok_to_countdown = server_ok_to_countdown(server);

			if (!ok_to_countdown)
			{
				memset(&server->countdown, 0, sizeof(server->countdown));
				send_countdown_update = TRUE;
			}
			else if (!countdown_timer_get_time_remaining(&server->countdown.timer) &&
				network_game_server_have_all_machines_have_precached(server) &&
				!server->countdown.paused)
			{
				network_game_server_close_game(server);
				success = network_game_server_start_network_game(server);

				if (success != TRUE)
				{
					network_event("network_game_server_start_network_game() failed");
				}
			}
			else if ((long)(current_time - server->countdown.last_message_time) > COUNTDOWN_MESSAGE_INTERVAL_MILLISECONDS)
			{
				send_countdown_update = TRUE;
			}

			if (send_countdown_update == TRUE)
			{
				struct message_server_pregame_countdown countdown;
				message_header *message;

				server->countdown.adjusted_this_tick = FALSE;

				if (ok_to_countdown)
				{
					countdown.seconds_to_game_start = (short)(countdown_timer_get_time_remaining(&server->countdown.timer) / MILLISECONDS_PER_SECOND);
				}
				else
				{
					countdown.seconds_to_game_start = NONE;
				}

				message = create_network_game_message(
					_message_type_server_pregame_countdown,
					&countdown,
					sizeof(countdown));

				if (message)
				{
					if (network_game_server_send_message_to_all_machines(server, message))
					{
						server->countdown.last_message_time = current_time;
					}
					else
					{
						network_event("failed to send a message_server_pregame_countdown to all clients");
					}
				}
			}
		}
		else if ((long)current_time > (long)(server->time_of_last_keep_alive + KEEP_ALIVE_INTERVAL_MILLISECONDS))
		{
			message_header *message;
			struct message_server_pregame_keep_alive keep_alive = {0};

			message = create_network_game_message(
				_message_type_server_pregame_keep_alive,
				&keep_alive,
				sizeof(keep_alive));
			network_game_server_send_message_to_all_machines(server, message);
			server->time_of_last_keep_alive = current_time;
		}
	}
	else if (server->time_of_first_client_loading_completion &&
		system_milliseconds() - server->time_of_first_client_loading_completion >= LEVEL_LOADING_TIMEOUT_MILLISECONDS)
	{
		long machine_index;

		for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
		{
			struct network_client_machine *client_machine = &server->client_machines[machine_index];

			if (TEST_FLAG(client_machine->flags, _client_machine_connected_bit) &&
				!TEST_FLAG(client_machine->flags, _client_machine_loaded_bit))
			{
				char machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH];
				boolean removed;

				network_event(
					"forcibly removing client system '%s' due to timeout while loading for game",
					wide_to_ascii(
						server->game.machines[client_machine->machine_index].name,
						machine_name,
						sizeof(machine_name)) ? machine_name : "<unknown name>");
				removed = network_game_server_remove_client_machine_from_game(
					server,
					client_machine);
				match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2382, removed);
			}
		}

		network_game_server_all_machines_have_loaded(server);
	}

	return success;
}

static boolean network_game_server_setup_game_from_playlist(
	struct network_game_server *server)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2401, server);

	network_event("setting up a net game");

	if (game_engine_get_current_stage(&server->game.variant, server->game.map.name))
	{
		wchar_t machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH] = L"<unknown>";

		network_game_generate_local_machine_name(machine_name);
		ustrncpy(server->game.name, machine_name, MAXIMUM_NETWORK_GAME_NAME_LENGTH - 1);
		server->game.name[MAXIMUM_NETWORK_GAME_NAME_LENGTH - 1] = 0;
		server->game.map.version = 0;
		server->game.minimum_players = MINIMUM_NETWORK_GAME_PLAYER_COUNT;
		server->game.maximum_players = NETWORK_GAME_MAXIMUM_PLAYER_COUNT;
		server->game.maximum_teams = server->game.variant.universal_variant.teams ? NUMBER_OF_MULTIPLAYER_TEAMS : 1;
		network_game_server_open_game(server);
		success = TRUE;
	}
	else
	{
		error(_error_silent, "network game setup failed; probably due to a missing playlist");
	}

	return success;
}

static short network_game_server_number_of_machines_connected(
	struct network_game_server *server)
{
	short machine_index;
	short client_machine_count = 0;

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		if (server->client_machines[machine_index].connection &&
			server->client_machines[machine_index].machine_index != NONE)
		{
			client_machine_count++;
		}
	}

	return client_machine_count;
}

static void dump_network_game_data(
	char *prefix,
	struct network_game_data *game)
{
	long machine_index;
	long player_index;

	network_event("%snetwork_game_data", prefix);
	network_event("%smachine_count %d", prefix, game->machine_count);

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		network_event(
			"\t%smachine %d %x",
			prefix,
			machine_index,
			game->machines[machine_index].machine_index);
	}

	network_event("%splayer_count %d", prefix, game->player_count);

	for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
	{
		network_event("%splayer %d", prefix, player_index);
		network_event(
			"%s\tmachine_index %x",
			prefix,
			game->players[player_index].machine_index);
		network_event(
			"%s\tcontroller_index %x",
			prefix,
			game->players[player_index].controller_index);
		network_event(
			"%s\tteam_index %x",
			prefix,
			game->players[player_index].team_index);
		network_event(
			"%s\tplayer_list_index %x",
			prefix,
			game->players[player_index].player_list_index);
	}

	network_event("%snetwork_game_random_seed %x", prefix, game->network_game_random_seed);
	network_event("%snumber_of_games_played %d", prefix, game->number_of_games_played);

	return;
}

static void network_game_server_dump(
	struct network_game_server *server)
{
	long machine_index;

	network_event("*************BEGIN*************");
	network_event("\tconnection %x", server->connection);
	network_event("\tstate %x", server->state);
	network_event("\tflags %x", server->flags);
	dump_network_game_data("\t", &server->game);
	network_event("client_machines:");

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		struct network_client_machine *client_machine = &server->client_machines[machine_index];
		char *connection_status = "no connection";

		if (client_machine->connection)
		{
			connection_status = network_connection_active(client_machine->connection) ? "(active)" : "(dead)";
		}

		network_event("\tclient %d", machine_index);
		network_event("\t\tconnection %x %s", client_machine->connection, connection_status);
		network_event(
			"\t\tlast_received_update_sequence_number %d",
			client_machine->last_received_update_sequence_number);
		network_event("\t\tstall_start_time %d", client_machine->stall_start_time);
		network_event("\t\tmachine_index %x", client_machine->machine_index);
		network_event("\t\tflags %x", client_machine->flags);
	}

	network_event("\tnext_update_number %d", server->next_update_number);
	network_event("\ttime_of_last_keep_alive %d", server->time_of_last_keep_alive);
	network_event(
		"\ttime_of_first_client_loading_completion %d",
		server->time_of_first_client_loading_completion);
	network_event("*************END*************");

	return;
}
