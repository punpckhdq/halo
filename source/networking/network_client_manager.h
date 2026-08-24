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

struct advertised_game_data
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
}; // 0xE4

struct network_game_join_parameters
{
	byte pad_00[2]; // 0x00, neither caller of network_game_client_initiate_join_game() fills it in and nothing reads it back
	wchar password[8]; // 0x02
	byte join_token[NETWORK_JOIN_GAME_TOKEN_SIZE]; // 0x12
}; // 0x22

/* ---------- prototypes/NETWORK_CLIENT_MANAGER.C */

struct network_game_client *network_game_client_create(void);
void network_game_client_dispose(struct network_game_client *client);
void network_game_client_keep_alive(struct network_game_client *client);
boolean network_game_client_idle(struct network_game_client *client);
word network_game_client_get_state(struct network_game_client *client, word *progress);
boolean network_game_client_initiate_join_game(struct network_game_client *client, struct advertised_game_data *game, struct network_game_join_parameters *join_parameters, struct transport_address *address);
boolean network_game_client_leave_game(struct network_game_client *client);
boolean network_game_client_set_machine(struct network_game_client *client, struct network_machine *machine);
struct network_machine *network_game_client_get_machine(struct network_game_client *client);
short network_game_client_get_machine_index(struct network_game_client *client);
boolean network_game_client_request_remove_player(struct network_game_client *client, struct network_player *player);
boolean network_game_client_remove_player(struct network_game_client *client, struct network_player *player, long time_of_death);
struct advertised_game_data *network_game_client_get_available_games(struct network_game_client *client);
word network_game_client_get_error(struct network_game_client *client);
short network_game_client_get_seconds_to_game_start(struct network_game_client *client);
boolean network_game_client_write(struct network_connection *connection, struct message_header *message, unsigned short message_size, struct transport_address *dest_address, boolean reliable);
boolean network_game_client_address_matches_server(struct network_game_client *client, struct transport_address *address);
void network_game_client_game_out_of_sync(struct network_game_client *client);
void network_game_client_new_advertised_game(struct network_game_client *client, struct message_server_game_advertise *message_packet);
void network_game_client_ponged(struct network_game_client *client, struct transport_address *source_address, unsigned long sent_time_system_milliseconds);
void network_game_client_accepted_into_game(struct network_game_client *client, struct transport_address *source_address, struct message_server_machine_accepted *message_packet);
void network_game_client_rejected_by_game(struct network_game_client *client, struct transport_address *source_address, word reason);
boolean network_game_client_game_settings_updated(struct network_game_client *client, struct message_server_game_settings_update *message_packet);
long unstrip_player_index(long stripped_player_index);
boolean network_game_client_game_has_started(struct network_game_client *client);
void network_game_client_game_shutdown(struct network_game_client *client);
boolean network_game_client_handle_game_update(struct network_game_client *client, struct message_server_game_update *message_packet);
boolean network_game_client_add_player_to_game(struct network_game_client *client, struct network_player *player);
void network_game_client_switch_to_postgame(struct network_game_client *client);
boolean network_game_client_switch_to_pregame(struct network_game_client *client);
struct network_connection *network_game_client_get_connection(struct network_game_client *client);
void network_game_client_get_remote_server_address(struct network_game_client *client, struct transport_address *remote_address);
struct network_game_data *network_game_client_get_game(struct network_game_client *client);
boolean network_game_client_server_has_started_game(struct network_game_client *client);
long network_game_client_get_next_update_number(struct network_game_client *client);
boolean network_client_get_oos(struct network_game_client *client);
void network_game_client_reset(struct network_game_client *client, boolean teardown_connection);
boolean network_game_client_add_player(struct network_game_client *client, short local_player_index);
boolean network_game_client_update_local_player_data(struct network_game_client *client, struct network_player *player);
boolean network_game_client_request_start_time_change(struct network_game_client *client, short request_type);
void network_game_client_countdown_timer_update(struct network_game_client *client, short seconds_to_start);
boolean network_game_client_advertised_game_is_valid(struct advertised_game_data *advertised_game);

/* ---------- globals */

extern boolean allow_out_of_sync;

/* ---------- public code */

#endif // __NETWORK_CLIENT_MANAGER_H
