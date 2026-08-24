/*
NETWORK_SERVER_MANAGER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries_windows.h"
#include "network_server_manager.h"
#include "network_server_message_handler.h"
#include "network_client_manager.h"
#include "network_connection.h"
#include "network_game_globals.h"
#include "network_game_manager.h"
#include "network_game_ui.h"
#include "network_messages.h"
#include "game.h"
#include "players.h"
#include "interface.h"

/* ---------- constants */

enum
{
	_network_game_server_game_open_bit= 0,
	_network_game_server_game_valid_bit,
	NUMBER_OF_NETWORK_GAME_SERVER_FLAGS,

	_network_client_machine_connected_bit= 0,
	_network_client_machine_validated_bit,
	_network_client_machine_level_loaded_bit,
	_network_client_machine_precached_bit,
	NUMBER_OF_NETWORK_CLIENT_MACHINE_FLAGS,

	COUNTDOWN_TIMER_CANNOT_ACCLERATE_AMOUNT = (MILLISECONDS_PER_SECOND - 1),
	CLIENT_COUNTDOWN_TIMER_MODIFICATION_AMOUNT= 5*MILLISECONDS_PER_SECOND,
	LAN_INITIAL_SERVER_GAME_COUNTDOWN= ((31*MILLISECONDS_PER_SECOND)-1),
	LOCAL_INITIAL_SERVER_GAME_COUNTDOWN= ((11*MILLISECONDS_PER_SECOND)-1),
	MAXIMUM_SERVER_GAME_COUNTDOWN= ((31*MILLISECONDS_PER_SECOND) - 1)
};

/* ---------- macros */

/* ---------- structures */

struct countdown_timer
{
	long time_remaining;
	long last_system_milliseconds;
};

void countdown_timer_update(struct countdown_timer *timer);
long countdown_timer_get_time_remaining(struct countdown_timer *timer);
void countdown_timer_increment(struct countdown_timer *timer, long adjustment, long max);
void countdown_timer_decrement(struct countdown_timer *timer, long adjustment);
void countdown_timer_set_time_remaining(struct countdown_timer *timer, long time_remaining);

void countdown_timer_update(struct countdown_timer *timer)
{
	long now = system_milliseconds();

	if (now > timer->last_system_milliseconds)
	{
		long elapased_time = now - timer->last_system_milliseconds;

		if (elapased_time < timer->time_remaining)
		{
			timer->time_remaining -= elapased_time;
		}
		else
		{
			timer->time_remaining = 0;
		}
	}

	timer->last_system_milliseconds = now;

	return;
}

long countdown_timer_get_time_remaining(struct countdown_timer *timer)
{
	long time_remaining;

	countdown_timer_update(timer);

	time_remaining = timer->time_remaining;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 95, timer->time_remaining >= 0);

	return time_remaining;
}

void countdown_timer_increment(struct countdown_timer *timer, long adjustment, long max)
{
	countdown_timer_update(timer);

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 104, adjustment >= 0);

	if ((timer->time_remaining + adjustment) < (adjustment))
	{
		timer->time_remaining = max;
	}
	else
	{
		timer->time_remaining += adjustment;
		timer->time_remaining = MIN(timer->time_remaining, max);
	}

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 117, timer->time_remaining >= 0);

	return;
}

void countdown_timer_decrement(struct countdown_timer *timer, long adjustment)
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

void countdown_timer_set_time_remaining(struct countdown_timer *timer, long time_remaining)
{
	long now = system_milliseconds();

	timer->time_remaining = time_remaining;
	timer->last_system_milliseconds = now;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 149, timer->time_remaining >= 0);

	return;
}

typedef struct network_client_machine
{
	struct network_connection *connection;
	unsigned long last_received_update_sequence_number;
	unsigned long stall_start_time;
	short machine_index; // into the machine list stored in network_game_server.game, or NONE
	word flags;
} network_client_machine;

typedef struct network_game_server
{
	struct network_connection *connection;
	word state, flags;
	network_game_data game;
	network_client_machine client_machines[MAXIMUM_NETWORK_MACHINE_COUNT];
	long next_update_number;
	long time_of_last_keep_alive;

	unsigned long time_of_first_client_loading_completion;

	struct _countdown
	{
		struct countdown_timer timer;
		long time_of_last_countdown_message;
		boolean active;
		boolean paused;
		boolean adjusted_time_this_tick;
	} countdown;

	struct network_player queued_player;
	boolean queued_player_valid;

	boolean sent_start_game_message;
} network_game_server;

/* ---------- prototypes */

static boolean network_game_server_add_new_client(network_game_server *server, struct network_connection *new_connection);
static boolean network_game_server_handle_public_endpoint(network_game_server *server);
static boolean network_game_server_handle_client_machines(network_game_server *server);

static void network_game_server_send_rejection_message(transport_endpoint_ref endpoint, word reason);
static void network_game_server_reject_connection_game_is_full(transport_endpoint_ref endpoint);

static boolean network_game_server_idle_pregame_tasks(network_game_server *server);
static boolean network_game_server_idle_postgame_tasks(network_game_server *server);
static boolean network_game_server_setup_game_from_playlist(network_game_server *server);

static short network_game_server_number_of_machines_connected(network_game_server *server);
static void network_game_server_remove_players_from_machine_ingame(
	struct network_game_server *server,
	struct network_client_machine *machine);

static boolean is_name_unique(network_game_server *server, network_player *player);
static void network_game_server_dump(network_game_server *server);

/* ---------- globals */

network_game_server network_game_server_memory_do_not_use_directly;
boolean network_game_server_memory_do_not_use_directly_in_use = FALSE;

/* ---------- public code */

network_game_server *network_game_server_create(
	void)
{
	network_game_server *server = &network_game_server_memory_do_not_use_directly;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 224, !network_game_server_memory_do_not_use_directly_in_use);
	network_game_server_memory_do_not_use_directly_in_use = TRUE;

	csmemset(server, 0, sizeof(*server));

	if (server != NULL)
	{
		server->connection= network_connection_new(FLAG(_connection_create_server_bit), NETWORK_SERVER_PORT);

		if (server->connection != NULL)
		{
			int i;
			transport_server_initialize();

			server->state= _network_game_server_state_pregame;
			server->flags= FLAG(_network_game_server_game_valid_bit);
			csmemset(&server->game, 0, sizeof(server->game));

			network_connection_set_connection_rejection_procedure(server->connection, network_game_server_reject_connection_game_is_full);
			
			network_game_invalidate(&server->game);

			server->game.difficulty_level= main_get_difficulty();
			server->game.number_of_games_played= -1;

			for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
			{
				server->client_machines[i].connection= NULL;
				server->client_machines[i].last_received_update_sequence_number= 0;
				server->client_machines[i].stall_start_time= 0;
				server->client_machines[i].machine_index= NONE;
				server->client_machines[i].flags= 0;
				network_game_invalidate_machine(&server->game, i);
			}

			server->sent_start_game_message= FALSE;
			server->time_of_first_client_loading_completion= 0;

			if (!network_game_server_reset_to_pregame(server))
			{
				error(_error_silent, "failed to initialize server pregame settings");
				network_game_server_dispose(server);
				server= NULL;
			}
		}
		else
		{
			error(_error_silent, "failed to create the server connection");
			network_game_server_dispose(server);
			server= NULL;
		}
	}

	return server;
}

void network_game_server_dispose(
	network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 288, server);

	switch (server->state)
	{
		case _network_game_server_state_pregame:
		{
			message_server_graceful_game_exit_pregame message_packet;
			message_header *message;

			if ((message= create_network_game_message(_message_type_server_graceful_game_exit_pregame, &message_packet, sizeof(message_packet))) != NULL)
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
			message_server_graceful_game_exit_postgame message_packet;
			message_header *message;

			if ((message= create_network_game_message(_message_type_server_graceful_game_exit_postgame, &message_packet, sizeof(message_packet))) != NULL)
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
		error(_error_silent, "network_game_server_handle_client_machines() failed inside network_game_server_dispose()");
	}

	if (server->connection)
	{
		network_connection_delete(server->connection);
	}

	SleepEx(MILLISECONDS_PER_SECOND, FALSE);
	
	transport_server_terminate();

	csmemset(server, 0, sizeof(*server));

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 369, network_game_server_memory_do_not_use_directly_in_use);
	network_game_server_memory_do_not_use_directly_in_use = FALSE;

	network_event("network server disposed");

	return;
}

boolean network_game_server_idle(
	network_game_server *server)
{
	boolean success= TRUE;

	if (transport_network_available() == FALSE)
	{
		if (!network_game_is_splitscreen_local())
		{
			display_error_when_main_menu_loaded(_error_network_connection_lost);
			error(_error_silent, "network connection went down!");
			success= FALSE;
			goto exit;
		}
	}

	if (network_game_server_game_is_valid(server))
	{
		struct network_connection *new_client_connection= NULL;

		if ((success= network_connection_idle(server->connection, _connection_dont_timeout, &new_client_connection)) == TRUE)
		{
			if (new_client_connection)
			{
				if ((success= network_game_server_add_new_client(server, new_client_connection)) == TRUE)
				{
					transport_address client_address;

					network_connection_get_address(new_client_connection, &client_address, NULL);
					network_event("new client connected from ip %s (validation pending)", transport_address_to_string(&client_address));

				}
				else
				{
					network_event("failed to add new client connection to the game");
					success= network_server_close_client_connection(server->connection, new_client_connection);
				}
			}

			success= network_game_server_handle_public_endpoint(server);
			if (success)
			{
				success= network_game_server_handle_client_machines(server);
				if (success)
				{
					switch (server->state)
					{
						case _network_game_server_state_pregame:
							success= network_game_server_idle_pregame_tasks(server);
							break;

						case _network_game_server_state_ingame:
							break;

						case _network_game_server_state_postgame:
							success= network_game_server_idle_postgame_tasks(server);
							break;

						default:
							network_event("unknown server state");
							success= FALSE;
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

exit:
	return success;
}

boolean network_game_server_set_game_name(
	network_game_server *server,
	wchar name[MAXIMUM_NETWORK_GAME_NAME_LENGTH])
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 477, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 478, name);

	ustrncpy(server->game.name, name, MAXIMUM_NETWORK_GAME_NAME_LENGTH-1);
	server->game.name[MAXIMUM_NETWORK_GAME_NAME_LENGTH-1]= L'\0';

	return FALSE;
}

wchar *network_game_server_get_game_name(
	network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 489, server);

	return server->game.name;
}

word network_game_server_get_state(
	network_game_server *server,
	word *progress)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 498, server);
	if (progress) *progress= 0;

	return server->state;
}

void network_game_server_open_game(
	network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 508, server);
	SET_FLAG(server->flags, _network_game_server_game_open_bit, TRUE);
	network_server_allow_client_connections(server->connection, TRUE);
	network_event("opening game");
	
	return;
}

void network_game_server_close_game(
	network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 520, server);
	SET_FLAG(server->flags, _network_game_server_game_open_bit, FALSE);
	network_server_allow_client_connections(server->connection, FALSE);
	network_event("closing game");

	return;
}

boolean network_game_server_game_is_open(
	network_game_server *server)
{
	boolean game_is_open;
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 532, server);

	game_is_open = TEST_FLAG(server->flags, _network_game_server_game_open_bit);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 535, (TRUE == game_is_open) || (FALSE == game_is_open));

	return game_is_open;
}

boolean network_game_server_game_is_valid(
	network_game_server *server)
{
	boolean game_is_valid;
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 544, server);

	game_is_valid = TEST_FLAG(server->flags, _network_game_server_game_valid_bit);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 547, (TRUE == game_is_valid) || (FALSE == game_is_valid));

	return game_is_valid;
}

boolean network_game_server_remove_client_machine_from_game(
	network_game_server *server,
	network_client_machine *client)
{
	boolean success= FALSE;
	int i;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 559, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 560, client);

	if (_network_game_server_state_ingame == server->state)
	{
		network_game_server_remove_players_from_machine_ingame(server, client);
	}

	for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
	{
		if (server->game.machines[i].machine_index == client->machine_index)
		{
			if (!network_game_remove_machine(&server->game, &server->game.machines[i]))
			{
				error(_error_silent, "network_game_server_remove_client_machine_from_game() failed to remove the offending machine from the server's copy of the game");
			}
			break;
		}
	}
	for (i= 0; i<MAXIMUM_NETWORK_MACHINE_COUNT; i++)
	{
		if (&server->client_machines[i] == client)
		{
			if (server->client_machines[i].connection != NULL)
			{
				if (!network_server_close_client_connection(server->connection, server->client_machines[i].connection))
				{
					network_event("server failed to close a client's connection");
				}
			}

			server->client_machines[i].connection= NULL;
			server->client_machines[i].last_received_update_sequence_number= 0;
			server->client_machines[i].stall_start_time= 0;
			server->client_machines[i].machine_index= NONE;
			server->client_machines[i].flags= 0;
			success= TRUE;
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
	struct network_client_machine *machine)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 615, _network_game_server_state_ingame == server->state);

	{
		network_game_data *game= &server->game;
		struct network_player *player= NULL;
		int i;

		for (i= 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i++)
		{
			if (network_player_is_valid(&game->players[i]) && (game->players[i].machine_index == machine->machine_index))
			{
				player= &game->players[i];

				{
					boolean success;
					message_server_remove_player_ingame remove_player_ingame_message;
					message_header *message;

					remove_player_ingame_message.player= *player;
					remove_player_ingame_message.time_of_quit_out_of_game = game_time_get() + MAXIMUM_SERVER_SAVED_UPDATES + 1;

					error(_error_silent, "sending quit out of game, time = %x", remove_player_ingame_message.time_of_quit_out_of_game);
					
					if ((message= create_network_game_message(_message_type_server_remove_player_ingame, &remove_player_ingame_message, sizeof(remove_player_ingame_message))) != NULL)
					{
						if ((success= network_game_server_send_message_to_all_machines(server, message)) == FALSE)
						{
							network_event("network_game_server_send_message_to_all_machines() failed in network_game_server_handle_message_client_remove_player_request_ingame()");
						}
					}
				}
			}
		}
	}
	
	return;
};

boolean network_game_server_remove_machine_from_game(
	network_game_server *server,
	network_machine *machine)
{
	boolean success= FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 665, server);
		
	if (NONE == machine->machine_index)
	{
		network_event("network_game_server_remove_machine_from_game called with a machine_index of NONE");
	}

	if (network_machine_is_valid(machine))
	{
		int i;		

		for (i= 0; i<MAXIMUM_NETWORK_MACHINE_COUNT; i++)
		{
			if (server->client_machines[i].machine_index == machine->machine_index)
			{
				success= network_game_server_remove_client_machine_from_game(server, &server->client_machines[i]);

				if (!success)
				{
					network_event("network_game_server_remove_client_machine_from_game() failed in network_game_server_remove_machine_from_game()");
				}
				break;
			}
		}

		if (i == MAXIMUM_NETWORK_MACHINE_COUNT)
		{
			network_event("network_game_server_remove_machine_from_game() failed to find the specified machine");
		}

		if (machine->machine_index != NONE)
		{
			success= network_game_remove_machine(&server->game, machine);

			if (!success)
			{
				network_event("network_game_remove_machine() failed in network_game_server_remove_machine_from_game()");
			}
		}

		if (server->state == _network_game_server_state_pregame)
		{
			boolean sent_game_settings= network_game_server_send_game_data_pregame(server);
			
			if (!sent_game_settings)
			{
				network_event("network_game_server_remove_machine_from_game() failed to send updated game settings to remaining clients");
			}
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
	network_game_server *server)
{
	boolean success= TRUE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 734, server);

	if (server->sent_start_game_message == FALSE)
	{
		message_server_game_settings_update game_settings;
		message_server_begin_game message_packet= {0};
		message_header *message;

		csmemcpy(&game_settings.game, &server->game, sizeof(network_game_data));

		if (((message= create_network_game_message(_message_type_server_game_settings_update, &game_settings, sizeof(game_settings))) != NULL) &&
			network_game_server_send_message_to_all_machines(server, message) &&
			((message= create_network_game_message(_message_type_server_begin_game, &message_packet, sizeof(message_packet))) != NULL) &&
			network_game_server_send_message_to_all_machines(server, message))
		{
			network_event("signalling client machines to begin loading for network game");
			server->sent_start_game_message= TRUE;
			success= TRUE;
		}
		else
		{
			network_event("failed to signal client machines to begin loading for network game");
		}
	}
	server->next_update_number= 0;

	return success;
}

void network_game_server_switch_to_postgame(
	network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 767, server);
	
	if (server->state == _network_game_server_state_ingame)
	{
		message_server_game_over message_packet= {0};
		message_header *message;

		server->state= _network_game_server_state_postgame;
		if ((message= create_network_game_message(_message_type_server_game_over, &message_packet, sizeof(message_packet))) != NULL)
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
	network_game_server *server)
{
	boolean success= FALSE;
	message_server_switch_to_pregame message_packet= {0};
	message_header *message;
	int i;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 804, server);

	csmemset(&server->countdown, 0, sizeof(server->countdown));
				
	server->next_update_number= 0;
	server->time_of_first_client_loading_completion= 0;
	server->sent_start_game_message= FALSE;
	server->queued_player_valid= FALSE;

	server->game.number_of_games_played++;

	if (server->state == _network_game_server_state_postgame)
	{
		if (((message= create_network_game_message(_message_type_server_switch_to_pregame, &message_packet, sizeof(message_packet))) != NULL) &&
			network_game_server_send_message_to_all_machines(server, message))
		{
			network_event("server resetting to pregame");

			if (server->game.variant.universal_variant.teams)
			{
				for (i= 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i+= 1)
				{
					if (network_player_is_valid(&server->game.players[i]))
					{
						switch (server->game.players[i].team_index)
						{
							case 0:
								server->game.players[i].team_index= 1;
								break;
							case 1:
								server->game.players[i].team_index= 0;
								break;
						}
					}
				}
			}

			for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
			{
				server->client_machines[i].flags&= ~FLAG(_network_client_machine_level_loaded_bit);
				server->client_machines[i].last_received_update_sequence_number= 0;
				server->client_machines[i].stall_start_time= 0;
			}
			network_game_reset_for_next_round(&server->game, FALSE);

			if (network_game_server_setup_game_from_playlist(server))
			{
				message_server_game_settings_update game_settings;
				message_header *message;

				csmemcpy(&game_settings.game, &server->game, sizeof(network_game_data));

				if (((message= create_network_game_message(_message_type_server_game_settings_update,
					&game_settings, sizeof(game_settings))) != NULL) &&
					network_game_server_send_message_to_all_machines(server, message))
				{
					server->state= _network_game_server_state_pregame;
					success= TRUE;
				}
			}
			else
			{
				message_server_graceful_game_exit_pregame shutdown_message= {0};

				if (((message= create_network_game_message(_message_type_server_graceful_game_exit_pregame,
					&shutdown_message, sizeof(shutdown_message))) != NULL) &&
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
		success= network_game_server_setup_game_from_playlist(server);

		if (server->game.variant.universal_variant.teams)
		{
			for (i= 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i+= 1)
			{
				if (network_player_is_valid(&server->game.players[i]))
				{
					switch (server->game.players[i].team_index)
					{
						case 0:
							server->game.players[i].team_index= 1;
							break;
						case 1:
							server->game.players[i].team_index= 0;
							break;
					}
				}
			}
		}
	}

	return success;
}

boolean network_game_server_graceful_shutdown(
	network_game_server *server)
{
	boolean success= FALSE;
	message_header *message= NULL;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 927, server);

	switch (network_game_server_get_state(server, NULL))
	{
		case _network_game_server_state_pregame:
		{
			message_server_graceful_game_exit_pregame message_packet= {0};

			if ((message= create_network_game_message(_message_type_server_graceful_game_exit_pregame, &message_packet, sizeof(message_packet))) == NULL)
			{
				network_event("failed to create a message_server_graceful_game_exit_pregame");
			}
			break;
		}

		case _network_game_server_state_postgame:
		{
			message_server_graceful_game_exit_postgame message_packet= {0};

			if ((message= create_network_game_message(_message_type_server_graceful_game_exit_postgame, &message_packet, sizeof(message_packet))) == NULL)
			{
				network_event("failed to create a message_server_graceful_game_exit_postgame");
			}
			break;
		}
	}

	if (message)
	{
		if ((success= network_game_server_send_message_to_all_machines(server, message)) == TRUE)
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
	network_game_server *server,
	network_client_machine *machine)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 973, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 974, machine);

	return (machine->flags&FLAG(_network_client_machine_validated_bit)) ? TRUE : FALSE;
}

boolean network_game_server_accept_client_machine_into_game(
	network_game_server *server,
	network_client_machine *machine)
{
	boolean success= FALSE;
	int i;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 986, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 987, machine);

	for (i= 0; i<MAXIMUM_NETWORK_MACHINE_COUNT; i++)
	{
		if ((server->game.machines[i].machine_index >= 0)&&
			(server->game.machines[i].machine_index < MAXIMUM_NETWORK_MACHINE_COUNT))
		{
		}
		else
		{
			network_machine new_machine;

			csmemcpy(&new_machine, &server->game.machines[i], sizeof(new_machine));
			new_machine.machine_index= i;
			if ((success= network_game_add_machine(&server->game, &new_machine)) == TRUE)
			{
				transport_address client_address= {0};

				network_connection_get_address(machine->connection, &client_address, NULL);
				network_event("server added machine @ %s to the game at machine index #%d",
					transport_address_to_string(&client_address), i);
				machine->machine_index= i;
				machine->flags|= FLAG(_network_client_machine_validated_bit);
			}
			else
			{
				network_event("network_game_add_machine() failed in network_game_server_accept_client_machine_into_game()");
			}
			break;
		}
	}

	if (i == MAXIMUM_NETWORK_MACHINE_COUNT)
	{
		network_event("network_game_server_accept_client_machine_into_game() failed to find an available opening for the new machine");
	}

	return success;
}

static boolean is_name_unique(network_game_server *server, network_player *player)
{
	wchar *player_name= L"";
	int i, count;
	
	player_name= player->name;
	for (i= count= 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i++)
	{
		if (network_player_is_valid(&server->game.players[i]))
		{
			if (ustrcmp(server->game.players[i].name, player_name) == 0)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

void get_unique_random_name(
	network_game_server *server,
	network_player *player)
{
	boolean unique= FALSE;
	wchar *player_name= L"";

	do
	{
		int i, count;
		
		player_name= network_game_get_random_player_name();
		for (i= count= 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i++)
		{
			if (network_player_is_valid(&server->game.players[i]))
			{
				if (ustrcmp(server->game.players[i].name, player_name) == 0)
				{
					count++;
				}
			}
		}
		unique= (count==0);
	} while (!unique);
	ustrncpy(player->name, player_name, sizeof(player->name)/sizeof(wchar)-1);
	player->name[sizeof(player->name)/sizeof(wchar)-1]= L'\0';

	return;
}

void get_unique_random_color(
	network_game_server *server,
	network_player *player)
{
	boolean unique= FALSE;
	long unique_color;
	long retries= 0;
	long good_color_retries = 10;

	do
	{
		if (retries < good_color_retries) {
			unique_color= player_profile_get_random_good_color();
		}
		else {
			unique_color= player_profile_get_random_color();
		}

		{
			int i;
		
			unique = TRUE;

			for (i= 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i++)
			{
				if (network_player_is_valid(&server->game.players[i]))
				{
					if (server->game.players[i].primary_color_index == unique_color)
					{
						unique = FALSE;
						break;
					}
				}
			}
		}

		retries++;
	} while (!unique);

	player->primary_color_index = unique_color;

	return;
}

boolean network_game_server_add_player_to_game(
	network_game_server *server,
	network_client_machine *machine,
	network_player *player)
{
	static int flipflop = 0;
	boolean success= FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1132, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1133, machine);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1134, player);

	if (machine->machine_index == player->machine_index)
	{
		player->team_index = flipflop;

		flipflop = (flipflop + 1) % 2;

		if (player->name[0]==L'\0')
		{
			get_unique_random_name(server, player);
		}

		if (!is_name_unique(server, player))
		{
			get_unique_random_name(server, player);
		}

		if (NONE == player->primary_color_index)
		{
			get_unique_random_color(server, player);
		}

		if ((success= network_game_add_player(&server->game, player)) == TRUE)
		{
			network_event("server added player from machine #%d at controller index #%d to the game", player->machine_index, player->controller_index);
		}
		else
		{
			network_event("network_game_add_player() failed in network_game_server_add_player_to_game()");
		}
	}
	else
	{
		network_event("client machine tried to add a player with a non-matching machine identifier");
	}

	return success;
}

boolean network_game_server_remove_player_from_game(
	network_game_server *server,
	network_client_machine *machine,
	network_player *player)
{
	boolean success= FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1184, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1185, machine);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1186, player);

	if (machine->machine_index == player->machine_index)
	{
		if ((success= network_game_remove_player(&server->game, player)) == TRUE)
		{
			network_event("server removed player from machine #%d at controller index #%d from the game",
				player->machine_index, player->controller_index);
		}
		else
		{
			network_event("network_game_remove_player() failed in network_game_server_remove_player_from_game()");
		}
	}
	else
	{
		network_event("client machine tried to remove a player with a non-matching machine identifier");
	}

	return success;
}

boolean network_game_server_adjust_machine_settings(
	network_game_server *server,
	network_client_machine *machine,
	network_machine *machine_description)
{
	boolean success= FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1215, server && machine && machine_description);

	if (machine->machine_index == machine_description->machine_index)
	{
		if ((success= network_game_update_machine(&server->game, machine_description)) == TRUE)
		{
			network_event("server updated machine #%d settings", machine_description->machine_index);
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
	network_game_server *server)
{
	network_event("all machines have successfully loaded");
	
	server->state= _network_game_server_state_ingame;
	server->time_of_first_client_loading_completion= 0;

	server->game.local_data.game_objects_loaded= (global_network_game_client_get() != NULL ?
		network_game_client_get_game(global_network_game_client_get())->local_data.game_objects_loaded : FALSE);

	match_vassert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1248, server->game.local_data.game_objects_loaded, "local game data not loaded");

	return;
}

void network_game_server_client_machine_game_loading_complete(
	network_game_server *server,
	network_client_machine *machine)
{
	boolean all_machines_loaded= TRUE;
	int i;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1261, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1262, machine);

	machine->flags|= FLAG(_network_client_machine_level_loaded_bit);

	for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
	{
		if ((server->client_machines[i].machine_index>=0) &&
			(server->client_machines[i].machine_index<MAXIMUM_NETWORK_MACHINE_COUNT))
		{
			if (!(server->client_machines[i].flags & FLAG(_network_client_machine_level_loaded_bit)))
			{
				network_event("still waiting on machine #%d to finish loading", server->client_machines[i].machine_index);
				all_machines_loaded= FALSE;
			}
		}
	}

	if (all_machines_loaded == TRUE)
	{
		network_game_server_all_machines_have_loaded(server);
	}

	if (server->time_of_first_client_loading_completion == 0)
	{
		server->time_of_first_client_loading_completion= system_milliseconds();
	}

	return;
}

void network_game_server_client_machine_is_precached(struct network_game_server *server, struct network_client_machine *machine, char *map_name)
{
	char *server_map_name = main_get_multiplayer_map_name();

	if (0 == csstrcmp(server_map_name, map_name))
	{
		SET_FLAG(machine->flags, _network_client_machine_precached_bit, TRUE);
	}

	return;
}

void network_game_server_handle_client_update_packet(
	network_game_server *server,
	network_client_machine *machine,
	message_client_game_update *message_packet)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1310, server);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1311, machine);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1312, message_packet);

	if (message_client_game_update_is_out_of_sync(message_packet))
	{
		network_event("client machine #%d is out of sync @ game tick #%ld; switching to post-game", machine->machine_index, game_time_get());
		game_engine_switch_to_postgame();
	}
	else if (message_client_game_update_get_update_number(message_packet) < machine->last_received_update_sequence_number)
	{
		network_event("received an outdated client update packet; ignoring (#%d / #%d)",
			message_client_game_update_get_update_number(message_packet), machine->last_received_update_sequence_number);
	}
	else if ((message_packet->player_count < 0) || (message_packet->player_count>MAXIMUM_NUMBER_OF_LOCAL_PLAYERS))
	{
		network_event("client update packet from machine #%d had a bad player count; ignoring", machine->machine_index);
	}
	else
	{
		struct player_action_collection machine_update= {0};
		int i;

		for (i= 0; i < message_packet->player_count; i++)
		{
			machine_update.actions[i]= message_packet->action_update[i];
		}

		update_server_handle_client_update(machine->machine_index, &machine_update);
		machine->last_received_update_sequence_number= message_client_game_update_get_update_number(message_packet);
	}

	return;
}

boolean network_game_server_switch_machine_from_postgame_to_pregame(
	network_game_server *server,
	network_client_machine *machine)
{
	boolean success= TRUE;
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1351, server && machine);

	network_event("machine #%d has successfully switched to pregame", machine->machine_index);
	machine->flags&= ~FLAG(_network_client_machine_level_loaded_bit);

	return success;
}

void network_game_server_update_ticks(
	network_game_server *server,
	short ticks_elapsed)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1363, server);

	switch (network_game_server_get_state(server, NULL))
	{
		case _network_game_server_state_ingame:
			while (ticks_elapsed-->0)
			{
				struct game_update update;
				long update_number= server->next_update_number++;
				message_server_game_update message_struct;
				message_header *message;
				
				update_server_next_update();
				update_server_build_server_update(NONE, &update, &update_number);

				message_struct.update_number= update_number;
				message_struct.debug_random_seed= get_random_seed();
				message_struct.debug_game_time= game_time_get();
				message_struct.player_count= update.number_of_actions;
				csmemcpy(message_struct.action_update, update.actions, update.number_of_actions*sizeof(struct player_action));
				if ((message= create_network_game_message(_message_type_server_game_update, &message_struct, sizeof(message_struct))) != NULL)
				{
					if (network_game_server_send_message_to_all_machines(server, message) == FALSE)
					{
						network_event("server failed to send game update message to all machines; client machine may be out of sync");
					}
				}
			}

			if (server->queued_player_valid)
			{
				network_client_machine *machine= NULL;
				int i;

				for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
				{
					if (server->client_machines[i].machine_index == server->queued_player.machine_index)
					{
						machine= &server->client_machines[i];
						break;
					}
				}

				if (machine && network_game_server_add_player_to_game(server, machine, &server->queued_player))
				{
					if (!network_game_server_send_player_joined_info_ingame(server, &server->queued_player))
					{
						network_event("network_game_server_send_player_joined_info_ingame() failed in network_game_server_handle_message_client_add_player_request_ingame()");
					}
				}
				else
				{
					network_event("server failed to add a network player in-game");
				}
				server->queued_player_valid= FALSE;
			}
			break;

		case _network_game_server_state_postgame:

			game_engine_update();
			break;
	}

	return;
}

void network_game_server_stalled_on_client(
	network_game_server *server,
	boolean stalled)
{
	int i;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1438, server);

	if (stalled)
	{
		unsigned long oldest_update_number= UNSIGNED_LONG_MAX;
		int culprit= NONE;

		for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
		{
			if ((server->client_machines[i].machine_index >= 0) && (server->client_machines[i].machine_index < MAXIMUM_NETWORK_MACHINE_COUNT))
			{
				if (server->client_machines[i].last_received_update_sequence_number < oldest_update_number)
				{
					oldest_update_number= server->client_machines[i].last_received_update_sequence_number;
					culprit= i;
				}
			}
		}
		match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1457, culprit != NONE);
		if (server->client_machines[culprit].stall_start_time)
		{
			if ((system_milliseconds() - server->client_machines[culprit].stall_start_time) >= NETWORK_GAME_SERVER_MAXIMUM_CLIENT_STALL_DURATION)
			{
				char ascii_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH];
				boolean removed;

				network_event("forcibly removing client system '%s' due to timeout in-game",
					wide_to_ascii(server->game.machines[server->client_machines[culprit].machine_index].name, ascii_name, sizeof(ascii_name)) ?
						ascii_name : "<unknown name>");
				removed= network_game_server_remove_client_machine_from_game(server, &server->client_machines[culprit]);
				match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1473, removed);
			}
		}
		else
		{
			server->client_machines[culprit].stall_start_time= system_milliseconds();
		}

		for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
		{
			if (i != culprit) server->client_machines[i].stall_start_time= 0;
		}
	}
	else
	{
		for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
		{
			server->client_machines[i].stall_start_time= 0;
		}
	}

	return;
}

void network_game_server_queue_player_for_addition(
	network_game_server *server,
	network_player *player)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1502, server && player);

	if (!server->queued_player_valid && network_player_is_valid(player))
	{
		csmemcpy(&server->queued_player, player, sizeof(network_player));
		server->queued_player_valid= TRUE;
	}

	return;
}

void network_game_server_begin_game_start_countdown(
	network_game_server *server,
	unsigned long countdown_duration_milliseconds)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1517, server);

	if ((server->countdown.active == FALSE) &&
		(server->countdown.paused == FALSE))
	{
		countdown_timer_set_time_remaining(&server->countdown.timer, countdown_duration_milliseconds);
		server->countdown.adjusted_time_this_tick = FALSE;
		server->countdown.active= TRUE;

		network_event("server game start countdown started");
	}

	return;
}

boolean server_needs_more_teams(network_game_server *server)
{
	boolean need_more_teams = FALSE;

	if (server->game.variant.universal_variant.teams)
	{
		short team_player_count[MAXIMUM_NUMBER_OF_SCORED_TEAMS]= {0};
		int i;

		for (i= 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i++)
		{
			if (network_player_is_valid(&server->game.players[i]))
			{
				if ((server->game.players[i].team_index>=0) && (server->game.players[i].team_index<MAXIMUM_NUMBER_OF_SCORED_TEAMS))
				{
					team_player_count[server->game.players[i].team_index]+= 1;
				}
			}
		}

		for (i= 0; i < MAXIMUM_NUMBER_OF_SCORED_TEAMS; i++)
		{
			if (team_player_count[i] == 0)
			{
				need_more_teams= TRUE;
				break;
			}
		}
	}

	return need_more_teams;
}

boolean server_has_a_player_on_each_machine(network_game_server *server)
{
	boolean has_a_player_on_each_machine = TRUE;
	long machine_itr;
	
	for(machine_itr = 0; machine_itr < MAXIMUM_NETWORK_MACHINE_COUNT; machine_itr++)
	{
		boolean machine_is_valid;

		machine_is_valid = (server->client_machines[machine_itr].machine_index >= 0) && (server->client_machines[machine_itr].machine_index < MAXIMUM_NETWORK_MACHINE_COUNT);

		if (machine_is_valid)
		{
			boolean has_a_player_on_this_machine = FALSE;
			long player_itr;

			for (player_itr= 0; player_itr < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_itr++)
			{
				if (network_player_is_valid(&server->game.players[player_itr]))
				{
					if (server->game.players[player_itr].machine_index == server->client_machines[machine_itr].machine_index)
					{
						has_a_player_on_this_machine = TRUE;
					}
				}
			}

			if (!has_a_player_on_this_machine)
			{
				has_a_player_on_each_machine = FALSE;
				break;
			}
		}
	}
	return has_a_player_on_each_machine;
}

boolean server_has_enough_machines(network_game_server *server)
{
	boolean enough_machines;
	long min_machine_count = network_game_is_splitscreen_local() ? 1 : 2;
	long machine_count = 0;
	long machine_itr;

	for(machine_itr = 0; machine_itr < MAXIMUM_NETWORK_MACHINE_COUNT; machine_itr++)
	{
		boolean machine_is_valid;

		machine_is_valid = (server->client_machines[machine_itr].machine_index >= 0) && (server->client_machines[machine_itr].machine_index < MAXIMUM_NETWORK_MACHINE_COUNT);

		if (machine_is_valid)
		{
			machine_count++;
		}
	}

	enough_machines = machine_count >= min_machine_count;

	return enough_machines;
}

boolean server_ok_to_countdown(network_game_server *server)
{
	boolean ok_to_countdown;

	ok_to_countdown = TRUE;
	ok_to_countdown = ok_to_countdown && server_has_enough_machines(server);
	ok_to_countdown = ok_to_countdown && server_has_a_player_on_each_machine(server);
	ok_to_countdown = ok_to_countdown && (!server_needs_more_teams(server));
	ok_to_countdown = ok_to_countdown && (server->game.player_count >= server->game.minimum_players);

	return ok_to_countdown;
}

void network_game_server_update_countdown(
	network_game_server *server,
	short request)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1646, server && server->state == _network_game_server_state_pregame);

	if (server->countdown.paused == FALSE)
	{
		boolean ok_to_countdown = server_ok_to_countdown(server);

		if (ok_to_countdown || (_game_start_request_defer_countdown == request))
		{
			if (server->countdown.active == TRUE)
			{
				if (!server->countdown.adjusted_time_this_tick)
				{
					switch (request)
					{
						case _game_start_request_delay_countdown:
							server->countdown.adjusted_time_this_tick = TRUE;
							countdown_timer_increment(&server->countdown.timer, CLIENT_COUNTDOWN_TIMER_MODIFICATION_AMOUNT, MAXIMUM_SERVER_GAME_COUNTDOWN);
							break;
						case _game_start_request_speed_countdown:
							server->countdown.adjusted_time_this_tick = TRUE;

							if (countdown_timer_get_time_remaining(&server->countdown.timer) > COUNTDOWN_TIMER_CANNOT_ACCLERATE_AMOUNT )
							{
								countdown_timer_decrement(&server->countdown.timer, CLIENT_COUNTDOWN_TIMER_MODIFICATION_AMOUNT);

								if (countdown_timer_get_time_remaining(&server->countdown.timer) < COUNTDOWN_TIMER_CANNOT_ACCLERATE_AMOUNT)
								{
									countdown_timer_set_time_remaining(&server->countdown.timer, COUNTDOWN_TIMER_CANNOT_ACCLERATE_AMOUNT);
								}
							}							
							break;
						case _game_start_request_defer_countdown:
							server->countdown.adjusted_time_this_tick = TRUE;
							server->countdown.active= FALSE;
							break;
						case _game_start_request_start_now:
							server->countdown.adjusted_time_this_tick = TRUE;
							countdown_timer_set_time_remaining(&server->countdown.timer, 0);
							break;
					}
				}
			}
			else
			{
				unsigned long now= system_milliseconds();

				if (request == _game_start_request_start_now)
				{
					countdown_timer_set_time_remaining(&server->countdown.timer, 0);
					server->countdown.active= TRUE;
					server->countdown.adjusted_time_this_tick = FALSE;
				}
				else
				{
					if ((network_game_should_accept_remote_connections() == FALSE) ||
						(network_game_server_number_of_machines_connected(server)>1))
					{
						unsigned long countdown;

						if (network_game_is_splitscreen_local())
						{
							countdown= LOCAL_INITIAL_SERVER_GAME_COUNTDOWN;
						}
						else
						{
							countdown= LAN_INITIAL_SERVER_GAME_COUNTDOWN;
						}

						server->countdown.active= TRUE;
						countdown_timer_set_time_remaining(&server->countdown.timer, countdown);
						server->countdown.adjusted_time_this_tick = FALSE;
						server->countdown.time_of_last_countdown_message= 0;
					}
				}
			}
		}
	}

	return;
}

void network_game_server_invalidate_network_machine(
	network_machine *machine)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1737, machine);
	csmemset(machine, 0, sizeof(network_machine));
	machine->machine_index= NONE;

	return;
}

void network_game_generate_join_game_token(
	byte join_token[NETWORK_JOIN_GAME_TOKEN_SIZE])
{
	byte join_token_initializer[]=
	{
		0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20,
		0x69, 0x6E, 0x20, 0x61, 0x20, 0x62, 0x6F, 0x74,
		0x74, 0x6C, 0x65
	};

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1754, join_token);
	csmemset(join_token, 0, sizeof(join_token));
	csmemcpy(join_token, join_token_initializer, MIN(NETWORK_JOIN_GAME_TOKEN_SIZE, sizeof(join_token_initializer)));

	return;
}

network_machine *network_game_server_get_client_machine(
	network_game_server *server,
	network_client_machine *client_machine,
	long *machine_index)
{
	network_machine *machine;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1793, server && client_machine);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1794, client_machine->machine_index<MAXIMUM_NETWORK_MACHINE_COUNT);

	if (machine_index) *machine_index= NONE;

	machine= &server->game.machines[client_machine->machine_index];
	if (machine_index)
	{
		*machine_index= machine->machine_index;
	}

	return machine;
}

struct network_connection *network_game_server_get_connection(
	network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1810, server);

	return server->connection;
}

struct network_connection *network_game_server_get_client_connection(
	network_client_machine *client_machine)
{
	struct network_connection *connection;

	if (client_machine)
	{
		connection= client_machine->connection;
	}
	else
	{
		connection= NULL;
	}

	return connection;
}

struct network_connection *network_game_server_get_machine_connection(
	network_game_server *server,
	network_machine *machine)
{
	struct network_connection *connection= NULL;
	int i;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1839, server && network_machine_is_valid(machine));

	for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
	{
		if (server->client_machines[i].machine_index == machine->machine_index)
		{
			connection= server->client_machines[i].connection;
			break;
		}
	}

	return connection;
}

network_client_machine *network_game_server_get_client_machine_at_index(
	network_game_server *server,
	int index)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1857, server && (index<MAXIMUM_NETWORK_MACHINE_COUNT));

	return &server->client_machines[index];
}

network_client_machine *network_game_server_get_client_machine_at_address(
	network_game_server *server,
	unsigned long ip_address)
{
	network_client_machine *machine= NULL;
	int i;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1869, server && ip_address);

	for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
	{
		transport_address address;

		if ((server->client_machines[i].machine_index >= 0) && (server->client_machines[i].machine_index < MAXIMUM_NETWORK_MACHINE_COUNT))
		{
			match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1877, server->client_machines[i].connection);
			network_connection_get_address(server->client_machines[i].connection, &address, NULL);
			if (address.address.ipv4_address == ip_address)
			{
				machine= &server->client_machines[i];
				break;
			}
		}
	}
	if (i == MAXIMUM_NETWORK_MACHINE_COUNT)
	{
		network_event("no machine found @ ip #%lX", ip_address);
	}

	return machine;
}

network_game_data *network_game_server_get_game(
	network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1897, server);

	return &server->game;
}

unsigned long network_game_server_get_oldest_client_update_received(
	network_game_server *server)
{
	unsigned long update_number= UNSIGNED_LONG_MAX;
	int i;

	for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
	{
		if ((server->client_machines[i].machine_index >= 0) && (server->client_machines[i].machine_index < MAXIMUM_NETWORK_MACHINE_COUNT))
		{
			update_number= MIN(update_number, server->client_machines[i].last_received_update_sequence_number);
		}
	}

	return update_number;
}

boolean network_game_server_game_can_start(
	network_game_server *server)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1922, server);

	return ((server->state == _network_game_server_state_pregame) &&
		(server->game.player_count >= server->game.minimum_players));
}

void network_game_server_pause_countdown(
	network_game_server *server,
	boolean pause_countdown)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1932, server);

	if (pause_countdown == TRUE)
	{
		csmemset(&server->countdown, 0, sizeof(server->countdown));
	}
	server->countdown.paused= pause_countdown;

	return;
}

void network_game_server_change_map_name(
	network_game_server *server,
	char *map_name)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1947, server && map_name && map_name[0]);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1948, server->state == _network_game_server_state_pregame);

	{
		long i;

		for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
		{
			network_client_machine *machine = server->client_machines + i;

			if ((machine->machine_index>=0) && (machine->machine_index<MAXIMUM_NETWORK_MACHINE_COUNT))
			{
				SET_FLAG(machine->flags, _network_client_machine_precached_bit, FALSE);
			}
		}
	}

	csstrncpy(server->game.map.name, map_name, MAXIMUM_NETWORK_MAP_NAME_LENGTH-1);
	server->game.map.name[MAXIMUM_NETWORK_MAP_NAME_LENGTH-1]= '\0';

	if (!network_game_server_send_game_data_pregame(server))
	{
		network_event("network_game_server_change_map_name() failed to send updated game settings to clients");
	}

	return;
}

void network_game_server_change_game_variant(
	network_game_server *server,
	struct game_variant *variant)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1982, server && variant);
	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 1983, server->state == _network_game_server_state_pregame);

	csmemcpy(&server->game.variant, variant, sizeof(struct game_variant));

	if (!network_game_server_send_game_data_pregame(server))
	{
		network_event("network_game_server_change_game_variant() failed to send updated game settings to clients");
	}

	return;
}

static boolean network_game_server_add_new_client(
	network_game_server *server,
	struct network_connection *new_connection)
{
	boolean success= FALSE;
	int i;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2004, server && new_connection);
	if (network_game_server_game_is_open(server))
	{
		for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
		{
			if (server->client_machines[i].machine_index==NONE)
			{
				transport_address client_address= {0};

				network_connection_get_address(new_connection, &client_address, NULL);
				if (client_address.address.ipv4_address)
				{
					if (!network_game_should_accept_remote_connections() &&
						(client_address.address.ipv4_address != IPV4_LOOPBACK_ADDRESS))
					{
						network_event("remote system tried to join our server but we are not accepting remote connections: address= '%s'", transport_address_to_string(&client_address));
					}
					else
					{
						server->client_machines[i].connection= new_connection;
						network_game_invalidate_machine(&server->game, i);
						server->client_machines[i].machine_index= i;
						server->client_machines[i].flags= FLAG(_network_client_machine_connected_bit);
						if ((success= network_connection_server_accept_client_connection(server->connection, new_connection)) == TRUE)
						{
							network_event("new remote connection accepted from %s", transport_address_to_string(&client_address));
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
		if (i == MAXIMUM_NETWORK_MACHINE_COUNT)
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
	network_game_server *server)
{
	boolean success= TRUE;
	byte datagram_buffer[DATAGRAM_MAXIMUM_SIZE];
	message_header *message= (message_header *)datagram_buffer;
	word datagram_size= sizeof(datagram_buffer);
	transport_address source_address;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2064, server);

	while (success && network_connection_read(server->connection, message, &datagram_size, &source_address))
	{
		if ((success= network_game_server_handle_datagram(server, message, datagram_size, &source_address)) == FALSE)
		{
			network_event("network_game_server_handle_datagram() failed in network_game_server_handle_public_endpoint()");
		}
		datagram_size= sizeof(datagram_buffer);
	}

	return success;
}

static boolean network_game_server_handle_client_machines(
	network_game_server *server)
{
	boolean success= TRUE;
	int i;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2087, server);
	for (i= 0; success && (i < MAXIMUM_NETWORK_MACHINE_COUNT); i++)
	{
		if (server->client_machines[i].machine_index!=NONE)
		{
			if (!network_connection_active(server->client_machines[i].connection))
			{
				if (network_game_server_remove_machine_from_game(server, &server->game.machines[server->client_machines[i].machine_index]))
				{
					network_event("client machine %x removed from game", server->client_machines[i].machine_index);
					network_game_server_dump(server);
				}
				else
				{
					network_event("failed to remove client machine %x from game", server->client_machines[i].machine_index);
					network_game_server_dump(server);
				}
			}
			else if (network_connection_idle(server->client_machines[i].connection, _connection_dont_timeout, NULL) &&
				network_connection_connected(server->client_machines[i].connection))
			{
				byte message_buffer[MESSAGE_MAXIMUM_SIZE];
				message_header *message= (message_header *)message_buffer;
				word message_buffer_size= sizeof(message_buffer);

				while (success && network_connection_read(server->client_machines[i].connection, message, &message_buffer_size, NULL))
				{
					if (network_game_server_handle_client_message(server, server->client_machines+i, message, message_buffer_size))
					{
						message_buffer_size= sizeof(message_buffer);
					}
					else
					{
						network_event("network_game_server_handle_client_message() failed in network_game_server_handle_client_machines()");

						if (network_game_server_remove_machine_from_game(server, &server->game.machines[server->client_machines[i].machine_index]))
						{
							network_event("client machine removed from game", server->client_machines[i].machine_index);
						}
						else
						{
							if (!network_game_server_remove_client_machine_from_game(server, &server->client_machines[i]))
							{
								network_event("failed to remove client machine from game", server->client_machines[i].machine_index);
							}
						}
						break;
					}
				}
			}
			else
			{
				if (network_game_server_remove_machine_from_game(server, &server->game.machines[server->client_machines[i].machine_index]))
				{
					network_event("client machine removed from game", server->client_machines[i].machine_index);
				}
				else
				{
					network_event("failed to remove client machine from game", server->client_machines[i].machine_index);
				}
				continue;
			}
		}
	}

	return success;
}

static void network_game_server_send_rejection_message(
	transport_endpoint_ref endpoint,
	word reason)
{
	message_server_machine_rejected farewell_message= {reason};
	message_header *message;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2168, endpoint && (reason < NUMBER_OF_SERVER_REJECTION_CODES));
	if ((message= create_network_game_message(_message_type_server_machine_rejected, &farewell_message, sizeof(farewell_message))) != NULL)
	{
		int length= GET_MESSAGE_SIZE(*message), bytes_written;
		
		byte_swap_message_header(message, _byte_order_network);
		bytes_written= write_endpoint(endpoint, message, length);
		if (bytes_written != length)
		{
			network_event("error sending rejection message to client; transport error= '%s'",
				transport_error_to_string(bytes_written));
		}
	}
	else
	{
		network_event("failed to create a message_server_machine_rejected message in network_game_server_send_rejection_message");
	}

	return;
}

static void network_game_server_reject_connection_game_is_full(
	transport_endpoint_ref endpoint)
{
	network_event("client connection refused; game is full");
	network_game_server_send_rejection_message(endpoint, _rejection_code_game_is_full);

	return;
}

static boolean network_game_server_idle_postgame_tasks(
	network_game_server *server)
{
	unsigned long now= system_milliseconds();
	boolean success= TRUE;

	{
		long time_per_keep_alive = MILLISECONDS_PER_SECOND * 5;

		if (now > (server->time_of_last_keep_alive + time_per_keep_alive))
		{
			message_server_postgame_keep_alive message_packet;
			message_header *message;

			message_packet.unused = 0;
			message= create_network_game_message(_message_type_server_postgame_keep_alive, &message_packet, sizeof(message_packet));
			network_game_server_send_message_to_all_machines(server, message);

			server->time_of_last_keep_alive = now;
		}
	}

	return success;
}

static boolean network_game_server_have_all_machines_have_precached(
	network_game_server *server)
{
	long i;
	boolean all_machines_have_precached = TRUE;

	for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
	{
		network_client_machine *machine = server->client_machines + i;

		if ((machine->machine_index>=0) && (machine->machine_index<MAXIMUM_NETWORK_MACHINE_COUNT))
		{
			boolean client_has_precached = TEST_FLAG(server->client_machines[i].flags, _network_client_machine_precached_bit);

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
	network_game_server *server)
{
	long now= system_milliseconds();
	boolean success= TRUE;

	if (server->sent_start_game_message == FALSE)
	{
		{
			long itr;
			
			for (itr= 0; itr < MAXIMUM_NETWORK_MACHINE_COUNT; itr++)
			{
				network_client_machine *client_machine = server->client_machines + itr;

				if (NULL != client_machine->connection)
				{
					if (!network_connection_active(client_machine->connection))
					{
						network_event("booting dead client machine %d", itr);
						
						network_game_server_remove_client_machine_from_game(server, client_machine);
					}
				}
			}
		}

		if (server->countdown.active == TRUE)
		{
			boolean send_countdown_update= FALSE;
			boolean ok_to_countdown = server_ok_to_countdown(server);
			boolean time_expired = FALSE;	

			if (!ok_to_countdown)
			{
				csmemset(&server->countdown, 0, sizeof(server->countdown));
				send_countdown_update= TRUE;
			}

			else if ((0 == countdown_timer_get_time_remaining(&server->countdown.timer)) && network_game_server_have_all_machines_have_precached(server) &&
				(server->countdown.paused == FALSE))
			{
				network_game_server_close_game(server);
				if ((success= network_game_server_start_network_game(server)) != TRUE)
				{
					network_event("network_game_server_start_network_game() failed");
				}
			}

			else if ((now - server->countdown.time_of_last_countdown_message) > MILLISECONDS_PER_SECOND)
			{
				send_countdown_update= TRUE;
			}

			if (send_countdown_update == TRUE)
			{
				message_server_pregame_countdown message_packet;
				message_header *message;

				server->countdown.adjusted_time_this_tick = FALSE;

				if (ok_to_countdown)
				{
					long time_remaining = countdown_timer_get_time_remaining(&server->countdown.timer);

					message_packet.seconds_to_start= time_remaining / MILLISECONDS_PER_SECOND;
				}
				else
				{
					message_packet.seconds_to_start= NONE;
				}

				message= create_network_game_message(_message_type_server_pregame_countdown, &message_packet, sizeof(message_packet));

				if (message != NULL)
				{
					if (network_game_server_send_message_to_all_machines(server, message))
					{
						server->countdown.time_of_last_countdown_message= now;
					}
					else
					{
						network_event("failed to send a message_server_pregame_countdown to all clients");
					}
				}
			}
		}
		else
		{
			long time_per_keep_alive = MILLISECONDS_PER_SECOND * 5;

			if (now > (server->time_of_last_keep_alive + time_per_keep_alive))
			{
				message_server_pregame_keep_alive message_packet;
				message_header *message;

				message_packet.unused = 0;
				message= create_network_game_message(_message_type_server_pregame_keep_alive, &message_packet, sizeof(message_packet));
				network_game_server_send_message_to_all_machines(server, message);

				server->time_of_last_keep_alive = now;
			}
		}
	}
	else
	{
		if (server->time_of_first_client_loading_completion)
		{
			if ((system_milliseconds() - server->time_of_first_client_loading_completion) >= NETWORK_GAME_SERVER_MAXIMUM_WAIT_TIME_FOR_LEVEL_LOADING)
			{
				int i;

				for (i= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
				{
					if (TEST_FLAG(server->client_machines[i].flags, _network_client_machine_connected_bit))
					{
						if (!TEST_FLAG(server->client_machines[i].flags, _network_client_machine_level_loaded_bit))
						{
							char ascii_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH];
							boolean removed;

							network_event("forcibly removing client system '%s' due to timeout while loading for game",
								wide_to_ascii(server->game.machines[server->client_machines[i].machine_index].name, ascii_name, sizeof(ascii_name)) ?
									ascii_name : "<unknown name>");
							removed= network_game_server_remove_client_machine_from_game(server, &server->client_machines[i]);
							match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2382, removed);
						}
					}
				}

				network_game_server_all_machines_have_loaded(server);
			}
		}
	}
	
	return success;
}

static boolean network_game_server_setup_game_from_playlist(
	network_game_server *server)
{
	boolean success= FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_server_manager.c", 2401, server);

	network_event("setting up a net game");
	if (game_engine_get_current_stage(&server->game.variant, server->game.map.name))
	{
		wchar machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH]= L"<unknown>";

		network_game_generate_local_machine_name(machine_name);
		ustrncpy(server->game.name, machine_name, MAXIMUM_NETWORK_GAME_NAME_LENGTH-1);
		server->game.name[MAXIMUM_NETWORK_GAME_NAME_LENGTH-1]= L'\0';
		server->game.map.version= 0;
		server->game.minimum_players= 2;
		server->game.maximum_players= NETWORK_GAME_MAXIMUM_PLAYER_COUNT;

		if (server->game.variant.universal_variant.teams)
		{
			server->game.maximum_teams= 2;
		}
		else
		{
			server->game.maximum_teams= 1;
		}

		network_game_server_open_game(server);

		success= TRUE;
	}
	else
	{
		error(_error_silent, "network game setup failed; probably due to a missing playlist");
	}

	return success;
}

static short network_game_server_number_of_machines_connected(
	network_game_server *server)
{
	short count, i;

	for (i= count= 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
	{
		if ((server->client_machines[i].connection != NULL) &&
			(server->client_machines[i].machine_index != NONE))
		{
			count+= 1;
		}
	}

	return count;
}

static void dump_network_game_data(char *prefix, struct network_game_data *network_game_data)
{
	network_event("%snetwork_game_data", prefix);
	network_event("%smachine_count %d", prefix, network_game_data->machine_count);
	{
		long itr;

		for(itr = 0; itr < MAXIMUM_NETWORK_MACHINE_COUNT; itr++)
		{
			network_event("\t%smachine %d %x", prefix, itr, network_game_data->machines[itr].machine_index);
		}

	}

	network_event("%splayer_count %d", prefix, network_game_data->player_count);
	{
		long itr;

		for(itr = 0;itr < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; itr++)
		{
			network_event("%splayer %d", prefix, itr);
			network_event("%s\tmachine_index %x", prefix, network_game_data->players[itr].machine_index);
			network_event("%s\tcontroller_index %x", prefix, network_game_data->players[itr].controller_index);
			network_event("%s\tteam_index %x", prefix, network_game_data->players[itr].team_index);
			network_event("%s\tplayer_list_index %x", prefix, network_game_data->players[itr].player_list_index);
		}
	}

	network_event("%snetwork_game_random_seed %x", prefix, network_game_data->network_game_random_seed);
	network_event("%snumber_of_games_played %d", prefix, network_game_data->number_of_games_played);

	return;
}

static void network_game_server_dump(network_game_server *server)
{
	long itr;

	network_event("*************BEGIN*************");

	network_event("\tconnection %x", server->connection);
	network_event("\tstate %x", server->state);
	network_event("\tflags %x", server->flags);

	dump_network_game_data("\t", &server->game);

	network_event("client_machines:");
	for(itr = 0; itr < MAXIMUM_NETWORK_MACHINE_COUNT; itr++)
	{
		network_client_machine *client_machine = server->client_machines + itr;
		char *connection_status = "no connection";

		if (client_machine->connection != NULL)
		{
			connection_status = network_connection_active(client_machine->connection) ? "(active)" : "(dead)";
		}
		
		network_event("\tclient %d", itr);
		network_event("\t\tconnection %x %s", client_machine->connection, connection_status);
		network_event("\t\tlast_received_update_sequence_number %d", client_machine->last_received_update_sequence_number);
		network_event("\t\tstall_start_time %d", client_machine->stall_start_time);
		network_event("\t\tmachine_index %x", client_machine->machine_index);
		network_event("\t\tflags %x", client_machine->flags);
	}

	network_event("\tnext_update_number %d", server->next_update_number);
	network_event("\ttime_of_last_keep_alive %d", server->time_of_last_keep_alive);
	network_event("\ttime_of_first_client_loading_completion %d", server->time_of_first_client_loading_completion);

	network_event("*************END*************");
	return;
}

