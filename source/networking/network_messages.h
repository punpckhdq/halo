/*
NETWORK_MESSAGES.H

header included in hcex build.
*/

#ifndef __NETWORK_MESSAGES_H
#define __NETWORK_MESSAGES_H
#pragma once

/* ---------- headers */

#include "bungie_net/common/message_header.h"
#include "bungie_net/network/transport.h"
#include "players.h"

/* ---------- constants */

enum
{
	_message_class_unconnected_client = 0,
	_message_class_unconnected_server,
	_message_class_server_pregame,
	_message_class_client_pregame,
	_message_class_server_ingame,
	_message_class_client_ingame,
	_message_class_server_postgame,
	_message_class_client_postgame,
	NUMBER_OF_NETWORK_GAME_MESSAGE_CLASSES
};

enum
{
	_message_type_client_broadcast_game_search = 0,
	_message_type_client_ping,
	_message_type_server_game_advertise,
	_message_type_server_pong,
	_message_type_server_machine_accepted,
	_message_type_server_machine_rejected,
	_message_type_server_game_settings_update,
	_message_type_server_pregame_countdown,
	_message_type_server_begin_game,
	_message_type_server_graceful_game_exit_pregame,
	_message_type_server_pregame_keep_alive,
	_message_type_server_postgame_keep_alive,
	_message_type_client_join_game_request,
	_message_type_client_add_player_request_pregame,
	_message_type_client_remove_player_request_pregame,
	_message_type_client_settings_request,
	_message_type_client_player_settings_request,
	_message_type_client_game_start_request,
	_message_type_client_graceful_game_exit_pregame,
	_message_type_client_map_is_precached_pregame,
	_message_type_server_game_update,
	_message_type_server_add_player_ingame,
	_message_type_server_remove_player_ingame,
	_message_type_server_game_over,
	_message_type_client_loaded,
	_message_type_client_game_update,
	_message_type_client_add_player_request_ingame,
	_message_type_client_remove_player_request_ingame,
	_message_type_client_host_crashed_cry_for_help,
	_message_type_client_join_new_host,
	_message_type_server_switch_to_pregame,
	_message_type_server_graceful_game_exit_postgame,
	_message_type_client_remove_player_request_postgame,
	_message_type_client_switch_to_pregame,
	_message_type_client_graceful_game_exit_postgame,
	NUMBER_OF_NETWORK_GAME_MESSAGE_TYPES
};

enum
{
	NETWORK_GAME_MESSAGE_VERSION = 1,
	MAXIMUM_DECODED_NETWORK_GAME_MESSAGE_SIZE = 1536,
	NETWORK_GAME_NONCE_BYTES = 8,
	NETWORK_JOIN_GAME_TOKEN_SIZE = 16,
};

enum
{
	_rejection_code_version_too_old = 0,
	_rejection_code_version_too_new,
	_rejection_code_bad_join_token,
	_rejection_code_bad_password,
	_rejection_code_game_is_full,
	_rejection_code_game_is_closed,
	_rejection_code_blacklisted_machine,
	NUMBER_OF_SERVER_REJECTION_CODES
};

enum
{
	_game_start_request_delay_countdown = 0,
	_game_start_request_speed_countdown,
	_game_start_request_defer_countdown,
	_game_start_request_start_now,
	NUMBER_OF_GAME_START_REQUESTS
};

enum
{
	_game_platform_xbox = 0,
	_game_platform_mswindows,
	NUMBER_OF_NETWORK_GAME_PLATFORMS
};

enum
{
	_game_advertise_open_bit = 1, /* fake name */
	_game_advertise_teams_enabled_bit, /* fake name */
	_game_advertise_terminator_bit, /* fake name */
};

enum
{
	_client_update_out_of_sync_bit = 31, /* fake name */
	CLIENT_UPDATE_NUMBER_MASK = FLAG(_client_update_out_of_sync_bit) - 1, /* fake name */
};

enum
{
	QUIT_OUT_OF_GAME_DELAY_TICKS = 33, /* fake name */
};

/* ---------- macros */

#define network_game_get_local_platform() _game_platform_xbox

/* ---------- structures */

struct message_client_broadcast_game_search
{
	word port; /* fake name */
	word version; /* fake name */
	char client_nonce[NETWORK_GAME_NONCE_BYTES]; /* fake name */
};

struct message_client_ping
{
	unsigned long timestamp; /* fake name */
	word port; /* fake name */
	word pad; /* fake name */
};

struct message_server_game_advertise
{
	char client_nonce[NETWORK_GAME_NONCE_BYTES];
	char server_nonce[NETWORK_GAME_NONCE_BYTES];
	XNKID key_id;
	XNKEY key;
	XNADDR host_address;
	word port;
	word version;
	word platform;
	wchar_t name[MAXIMUM_NETWORK_GAME_NAME_LENGTH];
	wchar_t human_readable_game_description[12];
	struct network_map map;
	short game_engine;
	short current_number_of_machines;
	short current_number_of_players;
	short maximum_number_of_players;
	short score_to_win;
	word flags;
	byte join_token[NETWORK_JOIN_GAME_TOKEN_SIZE];
};

struct message_server_pong
{
	unsigned long timestamp; /* fake name */
};

struct message_server_machine_accepted
{
	unsigned long server_random_seed;
	short machine_index;
	word pad;
};

struct message_server_machine_rejected
{
	word rejection_code; /* fake name */
};

struct message_server_game_settings_update
{
	struct network_game_data game; /* fake name */
};

struct message_server_pregame_countdown
{
	short seconds_to_game_start; /* fake name */
};

struct message_server_begin_game
{
	long unused; /* fake name */
};

struct message_server_graceful_game_exit_pregame
{
	long unused; /* fake name */
};

struct message_server_pregame_keep_alive
{
	short unused; /* fake name */
};

struct message_server_postgame_keep_alive
{
	short unused; /* fake name */
};

struct message_client_join_game_request
{
	wchar_t machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH]; /* fake name */
	byte join_token[NETWORK_JOIN_GAME_TOKEN_SIZE]; /* fake name */
};

struct message_client_add_player_request_pregame
{
	struct network_player player; /* fake name */
};

struct message_client_remove_player_request_pregame
{
	struct network_player player; /* fake name */
};

struct message_client_settings_request
{
	struct network_machine machine; /* fake name */
};

struct message_client_player_settings_request
{
	struct network_player player; /* fake name */
};

struct message_client_game_start_request
{
	short request_type; /* fake name */
};

struct message_client_graceful_game_exit_pregame
{
	long unused; /* fake name */
};

struct message_client_map_is_precached_pregame
{
	char map_name[MAXIMUM_FILENAME_LENGTH + 1]; /* fake name */
};

struct message_server_game_update
{
	unsigned long update_number;
	unsigned long debug_random_seed;
	unsigned long debug_game_time;
	word pad;
	short player_count;
	struct player_action action_update[NETWORK_GAME_MAXIMUM_PLAYER_COUNT];
};

struct message_server_add_player_ingame
{
	struct network_player player; /* fake name */
};

struct message_server_remove_player_ingame
{
	struct network_player player; /* fake name */
	long quit_out_of_game_time; /* fake name */
};

struct message_server_game_over
{
	long unused; /* fake name */
};

struct message_client_loaded
{
	long unused; /* fake name */
};

struct message_client_game_update
{
	unsigned long bits;
	word pad;
	short player_count;
	struct player_action action_update[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
};

struct message_client_add_player_request_ingame
{
	struct network_player player; /* fake name */
};

struct message_client_remove_player_request_ingame
{
	struct network_player player; /* fake name */
};

struct message_client_host_crashed_cry_for_help
{
	long unused[4]; /* fake name */
};

struct message_client_join_new_host
{
	long unused[4]; /* fake name */
};

struct message_server_switch_to_pregame
{
	long unused; /* fake name */
};

struct message_server_graceful_game_exit_postgame
{
	long unused; /* fake name */
};

struct message_client_remove_player_request_postgame
{
	struct network_player player; /* fake name */
};

struct message_client_switch_to_pregame
{
	long unused; /* fake name */
};

struct message_client_graceful_game_exit_postgame
{
	long unused; /* fake name */
};

typedef struct message_client_broadcast_game_search message_client_broadcast_game_search;
typedef struct message_client_ping message_client_ping;
typedef struct message_server_game_advertise message_server_game_advertise;
typedef struct message_server_pong message_server_pong;
typedef struct message_server_machine_accepted message_server_machine_accepted;
typedef struct message_server_machine_rejected message_server_machine_rejected;
typedef struct message_server_game_settings_update message_server_game_settings_update;
typedef struct message_server_pregame_countdown message_server_pregame_countdown;
typedef struct message_server_begin_game message_server_begin_game;
typedef struct message_server_graceful_game_exit_pregame message_server_graceful_game_exit_pregame;
typedef struct message_server_pregame_keep_alive message_server_pregame_keep_alive;
typedef struct message_server_postgame_keep_alive message_server_postgame_keep_alive;
typedef struct message_client_join_game_request message_client_join_game_request;
typedef struct message_client_add_player_request_pregame message_client_add_player_request_pregame;
typedef struct message_client_remove_player_request_pregame message_client_remove_player_request_pregame;
typedef struct message_client_settings_request message_client_settings_request;
typedef struct message_client_player_settings_request message_client_player_settings_request;
typedef struct message_client_game_start_request message_client_game_start_request;
typedef struct message_client_graceful_game_exit_pregame message_client_graceful_game_exit_pregame;
typedef struct message_client_map_is_precached_pregame message_client_map_is_precached_pregame;
typedef struct message_server_game_update message_server_game_update;
typedef struct message_server_add_player_ingame message_server_add_player_ingame;
typedef struct message_server_remove_player_ingame message_server_remove_player_ingame;
typedef struct message_server_game_over message_server_game_over;
typedef struct message_client_loaded message_client_loaded;
typedef struct message_client_game_update message_client_game_update;
typedef struct message_client_add_player_request_ingame message_client_add_player_request_ingame;
typedef struct message_client_remove_player_request_ingame message_client_remove_player_request_ingame;
typedef struct message_client_host_crashed_cry_for_help message_client_host_crashed_cry_for_help;
typedef struct message_client_join_new_host message_client_join_new_host;
typedef struct message_server_switch_to_pregame message_server_switch_to_pregame;
typedef struct message_server_graceful_game_exit_postgame message_server_graceful_game_exit_postgame;
typedef struct message_client_remove_player_request_postgame message_client_remove_player_request_postgame;
typedef struct message_client_switch_to_pregame message_client_switch_to_pregame;
typedef struct message_client_graceful_game_exit_postgame message_client_graceful_game_exit_postgame;

/* ---------- prototypes/NETWORK_MESSAGES.C */

void initialize_network_game_packets(void);
message_header *create_network_game_message(short message_type, void const *message_struct, short message_struct_size);
boolean decode_network_game_message(void *message_struct, void const *encoded_message, short *encoded_message_size, short *packet_type, short *packet_version, short expected_packet_class);
void network_event(char *format, ...);

/* ---------- globals */

/* ---------- public code */

#endif // __NETWORK_MESSAGES_H
