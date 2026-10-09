/*
NETWORK_GAME_GLOBALS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "network_game_globals.h"
#include "network_game_manager.h"
#include "network_messages.h"
#include "network_client_manager.h"
#include "network_server_manager.h"
#include "players.h"
#include "input.h"
#include "vehicles.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "ui_widget.h"
#include "bipeds.h"

/* ---------- constants */

enum
{
	PLAYER_ACTION_PACKET_DEFINITION = 1,
	PLAYER_ACTION_COLLECTION_PACKET_DEFINITION = 1,
	MINIMUM_CLIENT_UPDATE_INTERVAL_MILLISECONDS = 16, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static struct data_packet_field player_action_fields[] = /* fake name */
{
	{__pack_long, 6, 0, 0, 0},
	{__pack_short, 3, 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

struct data_packet_definition player_action_packet_definition =
{
	"player_action_packet_definition",
	0,
	sizeof(struct player_action),
	PLAYER_ACTION_PACKET_DEFINITION,
	player_action_fields,
	FALSE,
};

static struct data_packet_field player_action_collection_fields[] = /* fake name */
{
	{__pack_long, 6, 0, 0, 0},
	{__pack_short, 3, 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_long, 6, 0, 0, 0},
	{__pack_short, 3, 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_long, 6, 0, 0, 0},
	{__pack_short, 3, 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_long, 6, 0, 0, 0},
	{__pack_short, 3, 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

struct data_packet_definition player_action_collection_definition =
{
	"player_action_collection_definition",
	0,
	sizeof(struct player_action_collection),
	PLAYER_ACTION_COLLECTION_PACKET_DEFINITION,
	player_action_collection_fields,
	FALSE,
};

static unsigned long last_client_update_time; /* fake name */
static boolean want_to_teardown_networking;
static boolean quickstart_network_game_active;
static boolean accept_remote_connections;
static struct network_game_client *global_network_game_client;
static struct network_game_server *global_network_game_server;

/* ---------- public code */

boolean network_game_is_active(
	void)
{
	return global_network_game_client || global_network_game_server;
}

void network_game_set_number_of_games_played(
	long number_of_games_played)
{
	if (global_network_game_server)
	{
		network_game_server_get_game(global_network_game_server)->number_of_games_played = number_of_games_played;
	}

	if (global_network_game_client)
	{
		network_game_client_get_game(global_network_game_client)->number_of_games_played = number_of_games_played;
	}

	return;
}

long network_game_get_number_of_games_played(
	void)
{
	struct network_game_data *game = network_game_get_game();

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_globals.c", 85, game);

	return game->number_of_games_played;
}

void network_game_set_random_seed(
	unsigned long seed)
{
	if (global_network_game_server)
	{
		network_game_server_get_game(global_network_game_server)->network_game_random_seed = seed;
	}

	if (global_network_game_client)
	{
		network_game_client_get_game(global_network_game_client)->network_game_random_seed = seed;
	}

	return;
}

unsigned long network_game_get_random_seed(
	void)
{
	struct network_game_data *game = network_game_get_game();

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_globals.c", 115, game);

	return game->network_game_random_seed;
}

struct network_game_data *network_game_get_game(
	void)
{
	if (global_network_game_server)
	{
		return network_game_server_get_game(global_network_game_server);
	}

	if (global_network_game_client)
	{
		return network_game_client_get_game(global_network_game_client);
	}

	return NULL;
}

boolean network_game_player_is_local(
	struct network_player *player)
{
	if (player && network_player_is_valid(player) && global_network_game_client)
	{
		struct network_machine *machine = network_game_client_get_machine(global_network_game_client);

		if (!machine || machine->machine_index != player->machine_index)
		{
			return FALSE;
		}
	}
	else if (game_connection() == _game_connection_film_playback)
	{
		match_assert("c:\\halo\\SOURCE\\networking\\network_game_globals.c", 155, player);

		return (boolean)(player->machine_index == 0);
	}

	return TRUE;
}

void network_game_accept_remote_connections(
	boolean accept)
{
	accept_remote_connections = accept;

	return;
}

boolean network_game_should_accept_remote_connections(
	void)
{
	return accept_remote_connections;
}

boolean network_game_is_splitscreen_local(
	void)
{
	return global_network_game_server && !accept_remote_connections;
}

void network_game_set_quickstart_local(
	void)
{
	quickstart_network_game_active = TRUE;

	return;
}

boolean network_game_is_quickstart_local(
	void)
{
	return network_game_is_splitscreen_local() && quickstart_network_game_active == TRUE;
}

struct network_game_server *global_network_game_server_get(
	void)
{
	return global_network_game_server;
}

boolean create_global_network_game_server(
	void)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_game_globals.c", 214, global_network_game_server==NULL);

	global_network_game_server = network_game_server_create();

	if (global_network_game_server)
	{
		network_game_set_random_seed(local_random());
	}

	return global_network_game_server != NULL;
}

void dispose_global_network_game_server(
	void)
{
	if (global_network_game_server)
	{
		network_game_server_dispose(global_network_game_server);
		global_network_game_server = NULL;
		quickstart_network_game_active = FALSE;
	}

	return;
}

boolean network_game_server_start_frame(
	void)
{
	if (global_network_game_server)
	{
		return network_game_server_idle(global_network_game_server);
	}

	error(_error_silent, "no network game server");

	return TRUE;
}

struct network_game_client *global_network_game_client_get(
	void)
{
	return global_network_game_client;
}

boolean create_global_network_game_client(
	void)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_game_globals.c", 271, global_network_game_client==NULL);

	global_network_game_client = network_game_client_create();

	if (global_network_game_client)
	{
		want_to_teardown_networking = FALSE;
	}

	return global_network_game_client != NULL;
}

void dispose_global_network_game_client(
	void)
{
	if (global_network_game_client)
	{
		network_game_client_dispose(global_network_game_client);
		global_network_game_client = NULL;
	}

	want_to_teardown_networking = FALSE;

	return;
}

boolean network_game_client_start_frame(
	void)
{
	boolean success;
	static word last_state = NONE;

	if (want_to_teardown_networking == TRUE)
	{
		game_connection_set(_game_connection_local);
		network_game_end_and_load_ui(network_game_get_game());
		dispose_global_network_game_client();
		dispose_global_network_game_server();
		main_goto_main_menu();
		success = TRUE;
	}
	else
	{
		success = network_game_client_idle(global_network_game_client);

		if (success)
		{
			if (network_game_client_get_error(global_network_game_client) == _network_game_client_error_none)
			{
				word progress;
				word state = network_game_client_get_state(global_network_game_client, &progress);

				switch (state)
				{
				case _network_game_client_state_searching:
					if (last_state != state)
					{
						network_event("searching for a network game ...");
					}
					break;
				case _network_game_client_state_joining:
					if (last_state != state)
					{
						network_event("joining a network game ...");
					}
					break;
				case _network_game_client_state_pregame:
					if (last_state != state)
					{
						network_event("waiting for game to start ...");
					}
					break;
				case _network_game_client_state_ingame:
					if (last_state != state)
					{
						network_event("client signalled to begin loading for network game");
					}
					break;
				case _network_game_client_state_postgame:
					if (last_state != state)
					{
						network_event("waiting for game to restart ...");
					}
					break;
				default:
					match_vhalt("c:\\halo\\SOURCE\\networking\\network_game_globals.c", 352, "client is in an unknown state");
					break;
				}

				last_state = state;
			}
			else
			{
				network_event("internal networking error [network_game_client_get_error()!=0]");
				success = FALSE;
			}
		}
		else
		{
			network_event("internal networking error [network_game_client_idle() failed]");
		}
	}

	return success;
}

boolean network_game_client_end_frame(
	void)
{
	boolean success = TRUE;

	if (!global_network_game_client)
	{
		game_connection_set(_game_connection_local);
		main_menu_ensure_player_queues_exist();
	}
	else if (network_game_client_get_state(global_network_game_client, NULL) == _network_game_client_state_ingame)
	{
		unsigned long time = system_milliseconds();

		if (time - last_client_update_time >= MINIMUM_CLIENT_UPDATE_INTERVAL_MILLISECONDS &&
			network_game_client_server_has_started_game(global_network_game_client))
		{
			struct player_action_collection action_collection;
			struct message_client_game_update update_message;
			message_header *message;
			long update_number = network_game_client_get_next_update_number(global_network_game_client);
			struct network_game_data *game = network_game_client_get_game(global_network_game_client);

			update_client_build_client_update(&action_collection);

			if (network_client_get_oos(global_network_game_client))
			{
				update_message.bits = network_game_client_get_next_update_number(global_network_game_client) | FLAG(_client_update_out_of_sync_bit);
			}
			else
			{
				update_message.bits = network_game_client_get_next_update_number(global_network_game_client) & CLIENT_UPDATE_NUMBER_MASK;
			}

			memcpy(update_message.action_update, &action_collection, sizeof(action_collection));
			update_message.player_count = local_player_count();
			message = create_network_game_message(_message_type_client_game_update, &update_message, sizeof(update_message));

			if (message)
			{
				struct transport_address remote_server_address;

				network_game_client_get_remote_server_address(global_network_game_client, &remote_server_address);
				success = network_game_client_write(
					network_game_client_get_connection(global_network_game_client),
					message,
					GET_MESSAGE_SIZE(*message),
					&remote_server_address,
					FALSE);

				if (!success)
				{
					network_event("failed to send a game update to the server");
				}
			}
			else
			{
				network_event("failed to create a _message_type_client_game_update message");
				success = FALSE;
			}

			last_client_update_time = time;
		}
	}

	return success;
}

short network_game_client_get_local_machine_index(
	void)
{
	short machine_index = NONE;

	if (global_network_game_client)
	{
		struct network_machine *machine = network_game_client_get_machine(global_network_game_client);

		if (machine)
		{
			machine_index = machine->machine_index;
		}
	}

	return machine_index;
}

void network_game_client_local_player_quit(
	short controller_index)
{
	if (global_network_game_client)
	{
		struct network_machine *machine = network_game_client_get_machine(global_network_game_client);
		struct network_game_data *game = network_game_client_get_game(global_network_game_client);
		struct network_player *player = NULL;

		if (machine)
		{
			long player_index;

			for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
			{
				if (network_player_is_valid(&game->players[player_index]) &&
					game->players[player_index].machine_index == machine->machine_index &&
					game->players[player_index].controller_index == controller_index)
				{
					player = &game->players[player_index];
					break;
				}
			}
		}

		if (player && !network_game_client_request_remove_player(global_network_game_client, player))
		{
			error(_error_silent, "failed to request player removal in-game for player #%d", player->controller_index);
		}
	}

	return;
}

void network_game_abort(
	void)
{
	want_to_teardown_networking = TRUE;

	return;
}

void network_game_client_all_local_players_have_quit(
	void)
{
	want_to_teardown_networking = TRUE;

	return;
}

void network_game_client_request_immediate_start(
	void)
{
	if (global_network_game_client &&
		!network_game_client_request_start_time_change(global_network_game_client, _game_start_request_start_now))
	{
		error(_error_silent, "network_game_client_request_start() failed");
	}

	return;
}

/* ---------- private code */
