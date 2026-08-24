/*
NETWORK_SERVER_MANAGER.H

header included in hcex build.
*/

#ifndef __NETWORK_SERVER_MANAGER_H
#define __NETWORK_SERVER_MANAGER_H
#pragma once

/* ---------- headers */

#include "network_game_manager.h"
#include "network_messages.h"
#include "network_connection.h"

/* ---------- constants */

enum
{
	_network_game_server_state_pregame= 0,
	_network_game_server_state_ingame,
	_network_game_server_state_postgame,
	NUMBER_OF_NETWORK_GAME_SERVER_STATES
};

enum
{
	NETWORK_GAME_SERVER_MAXIMUM_CLIENT_STALL_DURATION= 2*MILLISECONDS_PER_SECOND,
	NETWORK_GAME_SERVER_MAXIMUM_WAIT_TIME_FOR_LEVEL_LOADING= 15*MILLISECONDS_PER_SECOND,
	MAXIMUM_SERVER_SAVED_UPDATES= 32
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/NETWORK_SERVER_MANAGER.C */

struct network_game_server *network_game_server_create(void);
void network_game_server_dispose(struct network_game_server *server);
boolean network_game_server_idle(struct network_game_server *server);
boolean network_game_server_set_game_name(struct network_game_server *server, wchar name[MAXIMUM_NETWORK_GAME_NAME_LENGTH]);
wchar *network_game_server_get_game_name(struct network_game_server *server);
word network_game_server_get_state(struct network_game_server *server, word *progress);
void network_game_server_open_game(struct network_game_server *server);
void network_game_server_close_game(struct network_game_server *server);
boolean network_game_server_game_is_open(struct network_game_server *server);
boolean network_game_server_game_is_valid(struct network_game_server *server);
boolean network_game_server_remove_client_machine_from_game(struct network_game_server *server, struct network_client_machine *client);
boolean network_game_server_remove_machine_from_game(struct network_game_server *server, struct network_machine *machine);
boolean network_game_server_start_network_game(struct network_game_server *server);
void network_game_server_switch_to_postgame(struct network_game_server *server);
boolean network_game_server_reset_to_pregame(struct network_game_server *server);
boolean network_game_server_graceful_shutdown(struct network_game_server *server);
boolean network_game_server_client_machine_is_joined_to_game(struct network_game_server *server, struct network_client_machine *machine);
boolean network_game_server_accept_client_machine_into_game(struct network_game_server *server, struct network_client_machine *machine);
void get_unique_random_name(struct network_game_server *server, struct network_player *player);
void get_unique_random_color(struct network_game_server *server, struct network_player *player);
boolean network_game_server_add_player_to_game(struct network_game_server *server, struct network_client_machine *machine, struct network_player *player);
boolean network_game_server_remove_player_from_game(struct network_game_server *server, struct network_client_machine *machine, struct network_player *player);
boolean network_game_server_adjust_machine_settings(struct network_game_server *server, struct network_client_machine *machine, struct network_machine *machine_description);
void network_game_server_all_machines_have_loaded(struct network_game_server *server);
void network_game_server_client_machine_game_loading_complete(struct network_game_server *server, struct network_client_machine *machine);
void network_game_server_client_machine_is_precached(struct network_game_server *server, struct network_client_machine *machine, char *map_name);
void network_game_server_handle_client_update_packet(struct network_game_server *server, struct network_client_machine *machine, struct message_client_game_update *message_packet);
boolean network_game_server_switch_machine_from_postgame_to_pregame(struct network_game_server *server, struct network_client_machine *machine);
void network_game_server_update_ticks(struct network_game_server *server, short ticks_elapsed);
void network_game_server_stalled_on_client(struct network_game_server *server, boolean stalled);
void network_game_server_queue_player_for_addition(struct network_game_server *server, struct network_player *player);
void network_game_server_begin_game_start_countdown(struct network_game_server *server, unsigned long countdown_duration_milliseconds);
boolean server_needs_more_teams(struct network_game_server *server);
boolean server_has_a_player_on_each_machine(struct network_game_server *server);
boolean server_has_enough_machines(struct network_game_server *server);
boolean server_ok_to_countdown(struct network_game_server *server);
void network_game_server_update_countdown(struct network_game_server *server, short request);
void network_game_server_invalidate_network_machine(struct network_machine *machine);
void network_game_generate_join_game_token(byte join_token[NETWORK_JOIN_GAME_TOKEN_SIZE]);
struct network_machine *network_game_server_get_client_machine(struct network_game_server *server, struct network_client_machine *client_machine, long *machine_index);
struct network_connection *network_game_server_get_connection(struct network_game_server *server);
struct network_connection *network_game_server_get_client_connection(struct network_client_machine *client_machine);
struct network_connection *network_game_server_get_machine_connection(struct network_game_server *server, struct network_machine *machine);
struct network_client_machine *network_game_server_get_client_machine_at_index(struct network_game_server *server, int index);
struct network_client_machine *network_game_server_get_client_machine_at_address(struct network_game_server *server, unsigned long ip_address);
struct network_game_data *network_game_server_get_game(struct network_game_server *server);
unsigned long network_game_server_get_oldest_client_update_received(struct network_game_server *server);
boolean network_game_server_game_can_start(struct network_game_server *server);
void network_game_server_pause_countdown(struct network_game_server *server, boolean pause_countdown);
void network_game_server_change_map_name(struct network_game_server *server, char *map_name);
void network_game_server_change_game_variant(struct network_game_server *server, struct game_variant *variant);

/* ---------- globals */

/* ---------- public code */

#endif // __NETWORK_SERVER_MANAGER_H
