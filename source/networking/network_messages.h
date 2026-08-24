/*
NETWORK_MESSAGES.H

header included in hcex build.
*/

#ifndef __NETWORK_MESSAGES_H
#define __NETWORK_MESSAGES_H
#pragma once

/* ---------- headers */

#include "message_header.h"
#include "transport.h"
#include "network_game_manager.h"

/* ---------- constants */

enum
{
	NETWORK_GAME_MESSAGE_VERSION= 1,
	NETWORK_JOIN_GAME_TOKEN_SIZE= 16
};

// these go out on the wire and index the packet table in network_messages.c, so do not reorder
enum
{
	_message_type_client_broadcast_game_search= 0,
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
	_rejection_code_version_too_old= 0,
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
	_game_start_request_delay_countdown= 0,
	_game_start_request_speed_countdown,
	_game_start_request_defer_countdown,
	_game_start_request_start_now,
	NUMBER_OF_GAME_START_REQUESTS
};

// bit 0 is not read by the client manager
enum
{
	_game_advertise_open_bit= 1,
	_game_advertise_teams_enabled_bit= 2,
	_game_advertise_oddball_terminator_bit= 3
};

/* ---------- macros */

#define message_client_game_update_is_out_of_sync(message) TEST_FLAG((message)->update_number, 31)
#define message_client_game_update_get_update_number(message) ((message)->update_number&0x7FFFFFFF)

/* ---------- structures */

// the size after each structure is the one create_network_game_message() asserts for its message type

struct message_client_broadcast_game_search
{
	word port;
	word version;
	byte nonce[8];
}; // 12

struct message_client_ping
{
	unsigned long system_milliseconds;
	word reply_to_port;
	word pad_06; // 0x06, alignment only: message_client_ping_packet skips it, so it never goes on the wire
}; // 8

struct message_server_game_advertise
{
	byte client_nonce[8]; // 0x00, echoed back from struct message_client_broadcast_game_search
	byte server_nonce[8]; // 0x08
	XNKID key_id; // 0x10
	XNKEY key; // 0x18
	XNADDR host_address; // 0x28, twelve bytes in this XDK
	word port; // 0x34, NETWORK_SERVER_PORT
	word version; // 0x36, NETWORK_GAME_MESSAGE_VERSION
	word platform; // 0x38
	wchar name[MAXIMUM_NETWORK_GAME_NAME_LENGTH]; // 0x3A
	byte pad_5A[26]; // 0x5A, zero-filled: the server never writes it and the client never reads it
	struct
	{
		long version; // 0x74
		char name[MAXIMUM_NETWORK_MAP_NAME_LENGTH]; // 0x78
	} map;
	short game_engine; // 0xF8
	short current_number_of_machines; // 0xFA
	short current_number_of_players; // 0xFC
	short maximum_number_of_players; // 0xFE
	short score_to_win; // 0x100
	word flags; // 0x102
	byte join_token[NETWORK_JOIN_GAME_TOKEN_SIZE]; // 0x104
}; // 276

struct message_server_pong
{
	unsigned long system_milliseconds; // 0x00, echoed back from message_client_ping
}; // 4

struct message_server_machine_accepted
{
	unsigned long server_random_seed; // 0x00
	short machine_index; // 0x04
	short pad_06; // 0x06, alignment only: message_server_machine_accepted_packet skips it, so it never goes on the wire
}; // 8

struct message_server_machine_rejected
{
	word reason;
}; // 2

struct message_server_game_settings_update
{
	struct network_game_data game;
}; // 1076

struct message_server_pregame_countdown
{
	short seconds_to_start;
}; // 2

struct message_server_pregame_keep_alive
{
	word unused;
}; // 2

struct message_server_begin_game
{
	long pad_00; // 0x00, filler; the message carries no payload
}; // 4

struct message_server_graceful_game_exit_pregame
{
	long pad_00; // 0x00, filler; the message carries no payload
}; // 4

struct message_server_postgame_keep_alive
{
	word unused;
}; // 2

struct message_client_join_game_request
{
	wchar machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH]; // 0x00
	byte join_token[NETWORK_JOIN_GAME_TOKEN_SIZE]; // 0x40
}; // 80

struct message_client_add_player_request_pregame
{
	struct network_player player;
}; // 32

struct message_client_remove_player_request_pregame
{
	struct network_player player;
}; // 32

struct message_client_settings_request
{
	struct network_machine machine;
}; // 68

struct message_client_player_settings_request
{
	struct network_player player;
}; // 32

struct message_client_game_start_request
{
	short request;
}; // 2

struct message_client_graceful_game_exit_pregame
{
	long pad_00; // 0x00, filler; the message carries no payload
}; // 4

struct message_client_map_is_precached_pregame
{
	char map_name[256];
}; // 256

struct message_server_game_update
{
	unsigned long update_number; // 0x00
	unsigned long debug_random_seed; // 0x04
	long debug_game_time; // 0x08
	// 0x0C, alignment only: message_server_game_update_packet's array field takes its element
	// count from the word just before the array data, so player_count has to land at 0x0E
	short pad_0C;
	short player_count; // 0x0E
	struct player_action action_update[NETWORK_GAME_MAXIMUM_PLAYER_COUNT]; // 0x10
}; // 528

struct message_server_add_player_ingame
{
	struct network_player player;
}; // 32

struct message_server_remove_player_ingame
{
	struct network_player player; // 0x00
	long time_of_quit_out_of_game; // 0x20
}; // 36

struct message_server_game_over
{
	long pad_00; // 0x00, filler; the message carries no payload
}; // 4

struct message_client_loaded
{
	long pad_00; // 0x00, filler; the message carries no payload
}; // 4

struct message_client_game_update
{
	unsigned long update_number; // 0x00, bit 31 = out of sync
	// 0x04, alignment only: message_client_game_update_packet's array field takes its element
	// count from the word just before the array data, so player_count has to land at 0x06
	short pad_04;
	short player_count; // 0x06
	struct player_action action_update[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS]; // 0x08
}; // 136

struct message_client_add_player_request_ingame
{
	struct network_player player;
}; // 32

struct message_client_remove_player_request_ingame
{
	struct network_player player;
}; // 32

struct message_client_host_crashed_cry_for_help
{
	// 0x00, message_client_host_crashed_cry_for_help_packet encodes three longs and a word
	// followed by two bytes of padding; nothing creates this message and neither manager handles it
	byte pad_00[16];
}; // 16

struct message_client_join_new_host
{
	// 0x00, message_client_join_new_host_packet encodes three longs and a word
	// followed by two bytes of padding; nothing creates this message and neither manager handles it
	byte pad_00[16];
}; // 16

struct message_server_switch_to_pregame
{
	long pad_00; // 0x00, filler; the message carries no payload
}; // 4

struct message_server_graceful_game_exit_postgame
{
	long pad_00; // 0x00, filler; the message carries no payload
}; // 4

struct message_client_remove_player_request_postgame
{
	struct network_player player;
}; // 32

struct message_client_switch_to_pregame
{
	long pad_00; // 0x00, filler; the message carries no payload
}; // 4

struct message_client_graceful_game_exit_postgame
{
	long pad_00; // 0x00, filler; the message carries no payload
}; // 4

/* ---------- prototypes/NETWORK_MESSAGES.C */

void initialize_network_game_packets(void);
void network_event(char *format, ...);
struct message_header *create_network_game_message(short message_type, void *message_struct, short message_struct_size);
boolean decode_network_game_message(void *message_struct, void const *encoded_message, short *encoded_message_size, short *packet_type, short *packet_version, short expected_packet_class);

/* ---------- globals */

/* ---------- public code */

#endif // __NETWORK_MESSAGES_H
