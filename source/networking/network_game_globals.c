/*
NETWORK_GAME_GLOBALS.C

- owns the global server and client objects, and drives both of them once a frame
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries_windows.h"
#include "errors.h"
#include "network_game_globals.h"
#include "network_game_manager.h"
#include "network_client_manager.h"
#include "network_server_manager.h"
#include "network_messages.h"
#include "memory/data_packets.h"
#include "network_connection.h"
#include "game.h"
#include "main.h"
#include "players.h"
#include "real_math.h"

/* ---------- constants */

enum
{
	CLIENT_UPDATE_INTERVAL_MILLISECONDS= 16
};

/* ---------- globals */

// the field arrays and the definitions that point at them are one contiguous run of data, so
// that each definition can name the array in front of it; only the long and end field types
// have names, the rest go in as their type codes
static struct
{
	// a player action is six longs (control flags, facing, throttle and trigger), three shorts
	// and its two pad bytes
	struct data_packet_field player_action_packet_fields[4];
} data_0030a988=
{
	{ {_data_packet_field_longs, 6, 0, 0, 0}, {2, 3, 0, 0, 0}, {0, 2, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} }
};
struct
{
	struct data_packet_definition definition;
	long pad; // the array behind it starts eight-byte aligned
	// the collection is one player action per local player, spelled out action by action
	struct data_packet_field player_action_collection_fields[3*MAXIMUM_NUMBER_OF_LOCAL_PLAYERS+1];
} player_action_packet_definition=
{
	{
		"player_action_packet_definition", 0, sizeof(struct player_action), NETWORK_GAME_MESSAGE_VERSION,
		data_0030a988.player_action_packet_fields, FALSE
	},
	0,
	{
		{_data_packet_field_longs, 6, 0, 0, 0}, {2, 3, 0, 0, 0}, {0, 2, 0, 0, 0},
		{_data_packet_field_longs, 6, 0, 0, 0}, {2, 3, 0, 0, 0}, {0, 2, 0, 0, 0},
		{_data_packet_field_longs, 6, 0, 0, 0}, {2, 3, 0, 0, 0}, {0, 2, 0, 0, 0},
		{_data_packet_field_longs, 6, 0, 0, 0}, {2, 3, 0, 0, 0}, {0, 2, 0, 0, 0},
		{_data_packet_field_end, 0, 0, 0, 0}
	}
};
struct data_packet_definition player_action_collection_definition=
{
	"player_action_collection_definition", 0, sizeof(struct player_action_collection), NETWORK_GAME_MESSAGE_VERSION,
	player_action_packet_definition.player_action_collection_fields, FALSE
};

// a machine can be both at once; a server always runs a client alongside it
static network_game_server *global_network_game_server;
static network_game_client *global_network_game_client;

static struct
{
	boolean accept_remote_connections;
	boolean quickstart_local;
	boolean abort_requested;
	byte pad;
	unsigned long time_of_last_client_update;
} network_game_globals;

// the client state we last reported through network_event()
static word last_logged_client_state= NONE;

/* ---------- public code */

boolean network_game_is_active(
	void)
{
	return global_network_game_client || global_network_game_server;
}

void network_game_set_number_of_games_played(
	long number_of_games_played)
{
	if (global_network_game_server) network_game_server_get_game(global_network_game_server)->number_of_games_played= number_of_games_played;
	if (global_network_game_client) network_game_client_get_game(global_network_game_client)->number_of_games_played= number_of_games_played;

	return;
}

void network_game_set_random_seed(
	long random_seed)
{
	if (global_network_game_server) network_game_server_get_game(global_network_game_server)->network_game_random_seed= random_seed;
	if (global_network_game_client) network_game_client_get_game(global_network_game_client)->network_game_random_seed= random_seed;

	return;
}

long network_game_get_number_of_games_played(
	void)
{
	network_game_data *game= network_game_get_game();

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_globals.c", 85, game);

	return game->number_of_games_played;
}

unsigned long network_game_get_random_seed(
	void)
{
	network_game_data *game= network_game_get_game();

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_globals.c", 115, game);

	return game->network_game_random_seed;
}

network_game_data *network_game_get_game(
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
		network_machine *machine= network_game_client_get_machine(global_network_game_client);

		return (boolean)(machine && (machine->machine_index == player->machine_index));
	}

	if (game_connection()==_game_connection_film_playback)
	{
		match_assert("c:\\halo\\SOURCE\\networking\\network_game_globals.c", 155, player);

		return (boolean)(player->machine_index==0);
	}

	return TRUE;
}

void network_game_accept_remote_connections(
	boolean accept_remote_connections)
{
	network_game_globals.accept_remote_connections= accept_remote_connections;

	return;
}

boolean network_game_should_accept_remote_connections(
	void)
{
	return network_game_globals.accept_remote_connections;
}

boolean network_game_is_splitscreen_local(
	void)
{
	return global_network_game_server && !network_game_globals.accept_remote_connections;
}

void network_game_set_quickstart_local(
	void)
{
	network_game_globals.quickstart_local= TRUE;

	return;
}

boolean network_game_is_quickstart_local(
	void)
{
	return global_network_game_server && !network_game_globals.accept_remote_connections && (network_game_globals.quickstart_local==TRUE);
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

	global_network_game_server= network_game_server_create();
	if (global_network_game_server)
	{
		network_game_set_random_seed(seed_random(get_global_local_random_seed_address()));
	}

	return global_network_game_server != NULL;
}

void dispose_global_network_game_server(
	void)
{
	if (global_network_game_server)
	{
		network_game_server_dispose(global_network_game_server);
		global_network_game_server= NULL;
		network_game_globals.quickstart_local= FALSE;
	}

	return;
}

boolean network_game_server_start_frame(
	void)
{
	boolean result;

	if (global_network_game_server)
	{
		result= network_game_server_idle(global_network_game_server);
	}
	else
	{
		error(_error_silent, "no network game server");

		result= TRUE;
	}

	return result;
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

	global_network_game_client= network_game_client_create();
	if (global_network_game_client)
	{
		network_game_globals.abort_requested= FALSE;
	}

	return global_network_game_client != NULL;
}

void dispose_global_network_game_client(
	void)
{
	if (global_network_game_client)
	{
		network_game_client_dispose(global_network_game_client);
		global_network_game_client= NULL;
	}
	network_game_globals.abort_requested= FALSE;

	return;
}

boolean network_game_client_start_frame(
	void)
{
	boolean result;

	if (network_game_globals.abort_requested==TRUE)
	{
		game_connection_set(_game_connection_local);
		network_game_end_and_load_ui(network_game_get_game());
		dispose_global_network_game_client();
		dispose_global_network_game_server();
		main_goto_main_menu();

		result= TRUE;
	}
	else
	{
		result= network_game_client_idle(global_network_game_client);
		if (result)
		{
			if (network_game_client_get_error(global_network_game_client)==_network_game_client_error_none)
			{
				word progress;
				word state= network_game_client_get_state(global_network_game_client, &progress);

				switch (state)
				{
					case _network_game_client_state_searching:
						if (last_logged_client_state != state) network_event("searching for a network game ...");
						break;
					case _network_game_client_state_joining:
						if (last_logged_client_state != state) network_event("joining a network game ...");
						break;
					case _network_game_client_state_pregame:
						if (last_logged_client_state != state) network_event("waiting for game to start ...");
						break;
					case _network_game_client_state_ingame:
						if (last_logged_client_state != state) network_event("client signalled to begin loading for network game");
						break;
					case _network_game_client_state_postgame:
						if (last_logged_client_state != state) network_event("waiting for game to restart ...");
						break;
					default:
						match_vhalt("c:\\halo\\SOURCE\\networking\\network_game_globals.c", 352, "client is in an unknown state");
						break;
				}
				last_logged_client_state= state;
			}
			else
			{
				network_event("internal networking error [network_game_client_get_error()!=0]");

				result= FALSE;
			}
		}
		else
		{
			network_event("internal networking error [network_game_client_idle() failed]");
		}
	}

	return result;
}

boolean network_game_client_end_frame(
	void)
{
	boolean result= TRUE;

	if (!global_network_game_client)
	{
		game_connection_set(_game_connection_local);
		main_menu_ensure_player_queues_exist();
	}
	else
	{
		if (network_game_client_get_state(global_network_game_client, NULL)==_network_game_client_state_ingame)
		{
			unsigned long now= system_milliseconds();

			if ((now-network_game_globals.time_of_last_client_update >= CLIENT_UPDATE_INTERVAL_MILLISECONDS) &&
				network_game_client_server_has_started_game(global_network_game_client))
			{
				struct player_action_collection actions;
				message_client_game_update message_packet;
				message_header *message;

				network_game_client_get_next_update_number(global_network_game_client);
				network_game_client_get_game(global_network_game_client);
				update_client_build_client_update(&actions);

				if (network_client_get_oos(global_network_game_client))
				{
					message_packet.update_number= network_game_client_get_next_update_number(global_network_game_client) | FLAG(31);
				}
				else
				{
					message_packet.update_number= network_game_client_get_next_update_number(global_network_game_client) & ~FLAG(31);
				}
				memcpy(message_packet.action_update, &actions, sizeof(actions));
				message_packet.player_count= local_player_count();

				message= create_network_game_message(_message_type_client_game_update, &message_packet, sizeof(message_packet));
				if (message)
				{
					transport_address address;

					network_game_client_get_remote_server_address(global_network_game_client, &address);
					result= network_game_client_write(network_game_client_get_connection(global_network_game_client), message, GET_MESSAGE_SIZE(*message), &address, FALSE);
					if (!result)
					{
						network_event("failed to send a game update to the server");
					}
				}
				else
				{
					network_event("failed to create a _message_type_client_game_update message");

					result= FALSE;
				}

				network_game_globals.time_of_last_client_update= now;
			}
		}
	}

	return result;
}

short network_game_client_get_local_machine_index(
	void)
{
	short machine_index= NONE;

	if (global_network_game_client)
	{
		network_machine *machine= network_game_client_get_machine(global_network_game_client);

		if (machine) machine_index= machine->machine_index;
	}

	return machine_index;
}

void network_game_client_local_player_quit(
	short local_player_index)
{
	if (global_network_game_client)
	{
		network_machine *machine= network_game_client_get_machine(global_network_game_client);
		network_game_data *game= network_game_client_get_game(global_network_game_client);

		if (machine)
		{
			struct network_player *player= NULL;
			long i;

			for (i= 0; i<NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i++)
			{
				if (network_player_is_valid(&game->players[i]) && (game->players[i].machine_index==machine->machine_index) &&
					(game->players[i].controller_index==local_player_index))
				{
					player= &game->players[i];
					break;
				}
			}

			if (player && !network_game_client_request_remove_player(global_network_game_client, player))
			{
				error(_error_silent, "failed to request player removal in-game for player #%d", player->controller_index);
			}
		}
	}

	return;
}

void network_game_abort(
	void)
{
	network_game_globals.abort_requested= TRUE;

	return;
}

void network_game_client_all_local_players_have_quit(
	void)
{
	network_game_globals.abort_requested= TRUE;

	return;
}

void network_game_client_request_immediate_start(
	void)
{
	if (global_network_game_client && !network_game_client_request_start_time_change(global_network_game_client, _game_start_request_start_now))
	{
		error(_error_silent, "network_game_client_request_start() failed");
	}

	return;
}
