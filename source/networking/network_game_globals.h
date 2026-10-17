/*
NETWORK_GAME_GLOBALS.H

header included in hcex build.
*/

#ifndef __NETWORK_GAME_GLOBALS_H
#define __NETWORK_GAME_GLOBALS_H
#pragma once

/* ---------- headers */

#include "bungie_net/common/message_header.h"
#include "network_game_manager.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/NETWORK_GAME_GLOBALS.C */

boolean network_game_is_active(void);
void network_game_set_number_of_games_played(long number_of_games_played);
long network_game_get_number_of_games_played(void);
void network_game_set_random_seed(unsigned long seed);
unsigned long network_game_get_random_seed(void);
struct network_game_data *network_game_get_game(void);
boolean network_game_player_is_local(struct network_player *player);
void network_game_accept_remote_connections(boolean accept);
boolean network_game_should_accept_remote_connections(void);
boolean network_game_is_splitscreen_local(void);
void network_game_set_quickstart_local(void);
boolean network_game_is_quickstart_local(void);
struct network_game_server *global_network_game_server_get(void);
boolean create_global_network_game_server(void);
void dispose_global_network_game_server(void);
boolean network_game_server_start_frame(void);
struct network_game_client *global_network_game_client_get(void);
boolean create_global_network_game_client(void);
void dispose_global_network_game_client(void);
boolean network_game_client_start_frame(void);
boolean network_game_client_end_frame(void);
short network_game_client_get_local_machine_index(void);
void network_game_client_local_player_quit(short controller_index);
void network_game_abort(void);
void network_game_client_all_local_players_have_quit(void);
void network_game_client_request_immediate_start(void);

/* ---------- globals */

/* ---------- public code */

#endif // __NETWORK_GAME_GLOBALS_H
