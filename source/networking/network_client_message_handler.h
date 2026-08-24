/*
NETWORK_CLIENT_MESSAGE_HANDLER.H

header included in hcex build.
*/

#ifndef __NETWORK_CLIENT_MESSAGE_HANDLER_H
#define __NETWORK_CLIENT_MESSAGE_HANDLER_H
#pragma once

/* ---------- headers */

#include "network_client_manager.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/NETWORK_CLIENT_MESSAGE_HANDLER.C */

boolean network_game_client_handle_message(network_game_client *client, message_header *message, short message_size, transport_address *source_address);

/* ---------- globals */

/* ---------- public code */

#endif // __NETWORK_CLIENT_MESSAGE_HANDLER_H
