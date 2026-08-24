/*
NETWORK_SERVER_MESSAGE_HANDLER.H

header included in hcex build.
*/

#ifndef __NETWORK_SERVER_MESSAGE_HANDLER_H
#define __NETWORK_SERVER_MESSAGE_HANDLER_H
#pragma once

/* ---------- headers */

#include "network_server_manager.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/NETWORK_SERVER_MESSAGE_HANDLER.C */

boolean network_game_server_send_message_to_machine(network_game_server *server, network_machine *machine, message_header *message);
boolean network_game_server_send_message_to_all_machines(network_game_server *server, message_header *message);
boolean network_game_server_send_player_joined_info_ingame(network_game_server *server, struct network_player *player);
boolean network_game_server_send_game_data_pregame(network_game_server *server);
boolean network_game_server_handle_datagram(network_game_server *server, message_header *message, short datagram_size, transport_address *source_address);
boolean network_game_server_handle_client_message(network_game_server *server, network_client_machine *machine, message_header *message, short message_buffer_size);

/* ---------- globals */

/* ---------- public code */

#endif // __NETWORK_SERVER_MESSAGE_HANDLER_H
