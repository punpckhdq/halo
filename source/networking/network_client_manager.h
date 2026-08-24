/*
NETWORK_CLIENT_MANAGER.H

header included in hcex build.
*/

#ifndef __NETWORK_CLIENT_MANAGER_H
#define __NETWORK_CLIENT_MANAGER_H
#pragma once

/* ---------- headers */

#include "network_game_manager.h"
#include "network_messages.h"
#include "network_connection.h"

/* ---------- constants */

enum
{
	MAXIMUM_ADVERTISED_GAMES= 9,
	MAXIMUM_CLIENT_SAVED_UPDATES= 32
};

enum
{
	_network_game_client_state_searching= 0,
	_network_game_client_state_joining,
	_network_game_client_state_pregame,
	_network_game_client_state_ingame,
	_network_game_client_state_postgame,
	NUMBER_OF_NETWORK_GAME_CLIENT_STATES
};

// 2..7 are not read by the client manager
enum
{
	_network_game_client_error_none= 0,
	_network_game_client_error_unknown,
	_network_game_client_error_host_closed_down= 8,
	NUMBER_OF_NETWORK_GAME_CLIENT_ERROR_CODES
};

/* ---------- macros */

/* ---------- structures */

typedef struct advertised_game_data
{
	XNKID key_id; // 0x00
	XNKEY key; // 0x08
	XNADDR xnet_address; // 0x18
	byte server_nonce[8]; // 0x24
	long time_in_milliseconds_of_last_news; // 0x2C
	wchar name[MAXIMUM_NETWORK_GAME_NAME_LENGTH]; // 0x30
	struct
	{
		long version; // 0x50
		char name[MAXIMUM_NETWORK_MAP_NAME_LENGTH]; // 0x54
	} map;
	word game_engine; // 0xD4
	word current_number_of_machines; // 0xD6
	word current_number_of_players; // 0xD8
	word maximum_number_of_players; // 0xDA
	word score_to_win; // 0xDC
	word platform; // 0xDE
	boolean open; // 0xE0
	boolean valid; // 0xE1
	boolean teams_enabled; // 0xE2
	boolean terminator; // 0xE3
} advertised_game_data; // 0xE4

typedef struct network_game_join_parameters
{
	byte pad_00[2]; // 0x00, neither caller of network_game_client_initiate_join_game() fills it in and nothing reads it back
	wchar password[8]; // 0x02
	byte join_token[NETWORK_JOIN_GAME_TOKEN_SIZE]; // 0x12
} network_game_join_parameters; // 0x22

typedef struct network_game_client network_game_client;

/* ---------- prototypes/NETWORK_CLIENT_MANAGER.C */

network_game_client *network_game_client_create(void);
void network_game_client_dispose(network_game_client *client);
void network_game_client_keep_alive(struct network_game_client *client);
boolean network_game_client_idle(network_game_client *client);
word network_game_client_get_state(network_game_client *client, word *progress);
boolean network_game_client_initiate_join_game(network_game_client *client, struct advertised_game_data *game, network_game_join_parameters *join_parameters, transport_address *address);
boolean network_game_client_leave_game(network_game_client *client);
boolean network_game_client_set_machine(network_game_client *client, network_machine *machine);
network_machine *network_game_client_get_machine(network_game_client *client);
short network_game_client_get_machine_index(network_game_client *client);
boolean network_game_client_request_remove_player(network_game_client *client, struct network_player *player);
boolean network_game_client_remove_player(network_game_client *client, struct network_player *player, long time_of_death);
advertised_game_data *network_game_client_get_available_games(network_game_client *client);
word network_game_client_get_error(network_game_client *client);
short network_game_client_get_seconds_to_game_start(network_game_client *client);
boolean network_game_client_write(struct network_connection *connection, message_header *message, unsigned short message_size, transport_address *dest_address, boolean reliable);
boolean network_game_client_address_matches_server(network_game_client *client, transport_address *address);
void network_game_client_game_out_of_sync(network_game_client *client);
void network_game_client_new_advertised_game(network_game_client *client, message_server_game_advertise *message_packet);
void network_game_client_ponged(network_game_client *client, transport_address *source_address, unsigned long sent_time_system_milliseconds);
void network_game_client_accepted_into_game(network_game_client *client, transport_address *source_address, message_server_machine_accepted *message_packet);
void network_game_client_rejected_by_game(network_game_client *client, transport_address *source_address, word reason);
boolean network_game_client_game_settings_updated(network_game_client *client, message_server_game_settings_update *message_packet);
long unstrip_player_index(long stripped_player_index);
boolean network_game_client_game_has_started(network_game_client *client);
void network_game_client_game_shutdown(network_game_client *client);
boolean network_game_client_handle_game_update(network_game_client *client, message_server_game_update *message_packet);
boolean network_game_client_add_player_to_game(network_game_client *client, struct network_player *player);
void network_game_client_switch_to_postgame(network_game_client *client);
boolean network_game_client_switch_to_pregame(network_game_client *client);
struct network_connection *network_game_client_get_connection(network_game_client *client);
void network_game_client_get_remote_server_address(network_game_client *client, transport_address *remote_address);
network_game_data *network_game_client_get_game(network_game_client *client);
boolean network_game_client_server_has_started_game(network_game_client *client);
long network_game_client_get_next_update_number(network_game_client *client);
boolean network_client_get_oos(network_game_client *client);
void network_game_client_reset(network_game_client *client, boolean teardown_connection);
boolean network_game_client_add_player(network_game_client *client, short local_player_index);
boolean network_game_client_update_local_player_data(network_game_client *client, struct network_player *player);
boolean network_game_client_request_start_time_change(network_game_client *client, short request_type);
void network_game_client_countdown_timer_update(network_game_client *client, short seconds_to_start);
boolean network_game_client_advertised_game_is_valid(struct advertised_game_data *advertised_game);

/* ---------- globals */

extern boolean allow_out_of_sync;

/* ---------- public code */

#endif // __NETWORK_CLIENT_MANAGER_H
