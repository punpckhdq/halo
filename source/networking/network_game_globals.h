/*
NETWORK_GAME_GLOBALS.H

header included in hcex build.
*/

#ifndef __NETWORK_GAME_GLOBALS_H
#define __NETWORK_GAME_GLOBALS_H
#pragma once

/* ---------- headers */

#include "bungie_net/common/message_header.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/NETWORK_GAME_GLOBALS.C */

void dispose_global_network_game_client(void);
void dispose_global_network_game_server(void);
boolean network_game_client_start_frame(void);
boolean network_game_client_end_frame(void);
boolean network_game_server_start_frame(void);
void network_game_abort(void);

boolean network_game_is_active(void);
struct network_game_client *global_network_game_client_get(void);
struct network_game_server *global_network_game_server_get(void);

/* ---------- globals */

/* ---------- public code */

#endif // __NETWORK_GAME_GLOBALS_H
