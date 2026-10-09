/*
NETWORK_SERVER_MESSAGE_HANDLER.H

header included in hcex build.
*/

#ifndef __NETWORK_SERVER_MESSAGE_HANDLER_H
#define __NETWORK_SERVER_MESSAGE_HANDLER_H
#pragma once

/* ---------- headers */

#include "bungie_net/common/message_header.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/NETWORK_SERVER_MESSAGE_HANDLER.C */

boolean network_game_server_handle_datagram(struct network_game_server *server, message_header *message, short datagram_size, struct transport_address *source_address);
boolean network_game_server_handle_client_message(struct network_game_server *server, struct network_client_machine *machine, message_header *message, short message_buffer_size);
boolean network_game_server_send_message_to_machine(struct network_game_server *server, struct network_machine *machine, message_header *message);
boolean network_game_server_send_message_to_all_machines(struct network_game_server *server, message_header *message);
boolean network_game_server_send_player_joined_info_ingame(struct network_game_server *server, struct network_player *player);
boolean network_game_server_send_game_data_pregame(struct network_game_server *server);

/* ---------- globals */

/* ---------- public code */

#endif // __NETWORK_SERVER_MESSAGE_HANDLER_H
