/*
NETWORK_MESSAGES.C

- wraps the data_packet_groups codec: create_network_game_message encodes a message structure
into a struct message_header the connection layer can send, decode_network_game_message unpacks one
*/

/* ---------- headers */

#include "cseries.h"
#include "errors.h"
#include "network_messages.h"
#include "memory/data_packets.h"
#include "network_game_manager.h"
#include "message_header.h"
#include "players.h"

/* ---------- constants */

enum
{
	// the decoded limit is also the size of create_network_game_message's stack buffer
	MAXIMUM_DECODED_NETWORK_GAME_MESSAGE_SIZE= 1536,
	MAXIMUM_ENCODED_NETWORK_GAME_MESSAGE_SIZE= 2048,

	// the classes the message handlers pass to decode_network_game_message as expected_packet_class
	NUMBER_OF_NETWORK_GAME_MESSAGE_CLASSES= 8
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean encode_network_game_message(void *message_struct, void *encoded_message, short *encoded_message_size, short packet_type, long packet_version);

/* ---------- globals */

// one object, so that every definition can point at the field array in front of it and every
// entry back at its definition; only the long and end field types have names, the rest go in
// as their type codes
static struct network_game_message_packet_table
{
	struct data_packet_field message_client_broadcast_game_search_packet_fields[3];
	struct data_packet_definition message_client_broadcast_game_search_packet;
	struct data_packet_field message_client_ping_packet_fields[4];
	struct data_packet_definition message_client_ping_packet;
	struct data_packet_field message_server_game_advertise_packet_fields[2];
	struct data_packet_definition message_server_game_advertise_packet;
	struct data_packet_field message_server_pong_packet_fields[2];
	struct data_packet_definition message_server_pong_packet;
	struct data_packet_field message_server_machine_accepted_packet_fields[4];
	struct data_packet_definition message_server_machine_accepted_packet;
	struct data_packet_field message_server_machine_rejected_packet_fields[2];
	struct data_packet_definition message_server_machine_rejected_packet;
	struct data_packet_field message_server_game_settings_update_packet_fields[2];
	struct data_packet_definition message_server_game_settings_update_packet;
	struct data_packet_field message_server_pregame_countdown_packet_fields[2];
	struct data_packet_definition message_server_pregame_countdown_packet;
	struct data_packet_field message_server_pregame_keep_alive_packet_fields[2];
	struct data_packet_definition message_server_pregame_keep_alive_packet;
	struct data_packet_field message_server_postgame_keep_alive_packet_fields[2];
	struct data_packet_definition message_server_postgame_keep_alive_packet;
	struct data_packet_field message_server_begin_game_packet_fields[2];
	struct data_packet_definition message_server_begin_game_packet;
	struct data_packet_field message_server_graceful_game_exit_pregame_packet_fields[2];
	struct data_packet_definition message_server_graceful_game_exit_pregame_packet;
	struct data_packet_field message_client_join_game_request_packet_fields[3];
	struct data_packet_definition message_client_join_game_request_packet;
	struct data_packet_field message_client_add_player_request_pregame_packet_fields[4];
	struct data_packet_definition message_client_add_player_request_pregame_packet;
	struct data_packet_field message_client_remove_player_request_pregame_packet_fields[4];
	struct data_packet_definition message_client_remove_player_request_pregame_packet;
	struct data_packet_field message_client_settings_request_packet_fields[4];
	struct data_packet_definition message_client_settings_request_packet;
	struct data_packet_field message_client_player_settings_request_packet_fields[4];
	struct data_packet_definition message_client_player_settings_request_packet;
	struct data_packet_field message_client_game_start_request_packet_fields[2];
	struct data_packet_definition message_client_game_start_request_packet;
	struct data_packet_field message_client_graceful_game_exit_pregame_packet_fields[2];
	struct data_packet_definition message_client_graceful_game_exit_pregame_packet;
	struct data_packet_field message_client_map_is_precached_pregame_packet_fields[2];
	struct data_packet_definition message_client_map_is_precached_pregame_packet;
	struct data_packet_field message_server_game_update_packet_fields[8];
	struct data_packet_definition message_server_game_update_packet;
	struct data_packet_field message_server_add_player_ingame_packet_fields[4];
	struct data_packet_definition message_server_add_player_ingame_packet;
	struct data_packet_field message_server_remove_player_ingame_packet_fields[5];
	struct data_packet_definition message_server_remove_player_ingame_packet;
	struct data_packet_field message_server_game_over_packet_fields[2];
	struct data_packet_definition message_server_game_over_packet;
	struct data_packet_field message_client_loaded_packet_fields[2];
	struct data_packet_definition message_client_loaded_packet;
	struct data_packet_field message_client_game_update_packet_fields[8];
	struct data_packet_definition message_client_game_update_packet;
	struct data_packet_field message_client_add_player_request_ingame_packet_fields[4];
	struct data_packet_definition message_client_add_player_request_ingame_packet;
	struct data_packet_field message_client_remove_player_request_ingame_packet_fields[4];
	struct data_packet_definition message_client_remove_player_request_ingame_packet;
	struct data_packet_field message_client_host_crashed_cry_for_help_packet_fields[4];
	struct data_packet_definition message_client_host_crashed_cry_for_help_packet;
	struct data_packet_field message_client_join_new_host_packet_fields[4];
	struct data_packet_definition message_client_join_new_host_packet;
	struct data_packet_field message_server_switch_to_pregame_packet_fields[2];
	struct data_packet_definition message_server_switch_to_pregame_packet;
	struct data_packet_field message_server_graceful_game_exit_postgame_packet_fields[2];
	struct data_packet_definition message_server_graceful_game_exit_postgame_packet;
	struct data_packet_field message_client_remove_player_request_postgame_packet_fields[4];
	struct data_packet_definition message_client_remove_player_request_postgame_packet;
	struct data_packet_field message_client_switch_to_pregame_packet_fields[2];
	struct data_packet_definition message_client_switch_to_pregame_packet;
	struct data_packet_field message_client_graceful_game_exit_postgame_packet_fields[2];
	struct data_packet_definition message_client_graceful_game_exit_postgame_packet;
	struct data_packet_entry packets[NUMBER_OF_NETWORK_GAME_MESSAGE_TYPES];
} data_0030aa68=
{
	{ {2, 2, 0, 0, 0}, {1, 8, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_broadcast_game_search_packet", 0, sizeof(struct message_client_broadcast_game_search), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_broadcast_game_search_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {2, 1, 0, 0, 0}, {0, 2, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_ping_packet", 0, sizeof(struct message_client_ping), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_ping_packet_fields, FALSE },
	{ {8, 276, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_game_advertise_packet", 0, sizeof(struct message_server_game_advertise), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_game_advertise_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_pong_packet", 0, sizeof(struct message_server_pong), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_pong_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {2, 1, 0, 0, 0}, {0, 2, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_machine_accepted_packet", 0, sizeof(struct message_server_machine_accepted), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_machine_accepted_packet_fields, FALSE },
	{ {2, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_machine_rejected_packet", 0, sizeof(struct message_server_machine_rejected), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_machine_rejected_packet_fields, FALSE },
	{ {8, 1076, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_game_settings_update_packet", 0, sizeof(struct message_server_game_settings_update), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_game_settings_update_packet_fields, FALSE },
	{ {2, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_pregame_countdown_packet", 0, sizeof(struct message_server_pregame_countdown), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_pregame_countdown_packet_fields, FALSE },
	{ {2, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_pregame_keep_alive_packet", 0, sizeof(struct message_server_pregame_keep_alive), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_pregame_keep_alive_packet_fields, FALSE },
	{ {2, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_postgame_keep_alive_packet", 0, sizeof(struct message_server_postgame_keep_alive), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_postgame_keep_alive_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_begin_game_packet", 0, sizeof(struct message_server_begin_game), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_begin_game_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_graceful_game_exit_pregame_packet", 0, sizeof(struct message_server_graceful_game_exit_pregame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_graceful_game_exit_pregame_packet_fields, FALSE },
	{ {2, 32, 0, 0, 0}, {1, 16, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_join_game_request_packet", 0, sizeof(struct message_client_join_game_request), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_join_game_request_packet_fields, FALSE },
	{ {2, 12, 0, 0, 0}, {2, 2, 0, 0, 0}, {1, 4, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_add_player_request_pregame_packet", 0, sizeof(struct message_client_add_player_request_pregame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_add_player_request_pregame_packet_fields, FALSE },
	{ {2, 12, 0, 0, 0}, {2, 2, 0, 0, 0}, {1, 4, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_remove_player_request_pregame_packet", 0, sizeof(struct message_client_remove_player_request_pregame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_remove_player_request_pregame_packet_fields, FALSE },
	{ {2, 32, 0, 0, 0}, {1, 1, 0, 0, 0}, {0, 3, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_settings_request_packet", 0, sizeof(struct message_client_settings_request), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_settings_request_packet_fields, FALSE },
	{ {2, 12, 0, 0, 0}, {2, 2, 0, 0, 0}, {1, 4, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_player_settings_request_packet", 0, sizeof(struct message_client_player_settings_request), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_player_settings_request_packet_fields, FALSE },
	{ {2, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_game_start_request_packet", 0, sizeof(struct message_client_game_start_request), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_game_start_request_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_graceful_game_exit_pregame_packet", 0, sizeof(struct message_client_graceful_game_exit_pregame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_graceful_game_exit_pregame_packet_fields, FALSE },
	{ {1, 256, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_map_is_precached_pregame_packet", 0, sizeof(struct message_client_map_is_precached_pregame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_map_is_precached_pregame_packet_fields, FALSE },
	{ {_data_packet_field_longs, 3, 0, 0, 0}, {0, 2, 0, 0, 0}, {7, 16, 0, 0, 0}, {_data_packet_field_longs, 6, 0, 0, 0}, {2, 3, 0, 0, 0}, {0, 2, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_game_update_packet", 0, sizeof(struct message_server_game_update), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_game_update_packet_fields, FALSE },
	{ {2, 12, 0, 0, 0}, {2, 2, 0, 0, 0}, {1, 4, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_add_player_ingame_packet", 0, sizeof(struct message_server_add_player_ingame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_add_player_ingame_packet_fields, FALSE },
	{ {2, 12, 0, 0, 0}, {2, 2, 0, 0, 0}, {1, 4, 0, 0, 0}, {_data_packet_field_longs, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_remove_player_ingame_packet", 0, sizeof(struct message_server_remove_player_ingame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_remove_player_ingame_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_game_over_packet", 0, sizeof(struct message_server_game_over), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_game_over_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_loaded_packet", 0, sizeof(struct message_client_loaded), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_loaded_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {0, 2, 0, 0, 0}, {7, 4, 0, 0, 0}, {_data_packet_field_longs, 6, 0, 0, 0}, {2, 3, 0, 0, 0}, {0, 2, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_game_update_packet", 0, sizeof(struct message_client_game_update), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_game_update_packet_fields, FALSE },
	{ {2, 12, 0, 0, 0}, {2, 2, 0, 0, 0}, {1, 4, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_add_player_request_ingame_packet", 0, sizeof(struct message_client_add_player_request_ingame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_add_player_request_ingame_packet_fields, FALSE },
	{ {2, 12, 0, 0, 0}, {2, 2, 0, 0, 0}, {1, 4, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_remove_player_request_ingame_packet", 0, sizeof(struct message_client_remove_player_request_ingame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_remove_player_request_ingame_packet_fields, FALSE },
	{ {_data_packet_field_longs, 3, 0, 0, 0}, {2, 1, 0, 0, 0}, {0, 2, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_host_crashed_cry_for_help_packet", 0, sizeof(struct message_client_host_crashed_cry_for_help), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_host_crashed_cry_for_help_packet_fields, FALSE },
	{ {_data_packet_field_longs, 3, 0, 0, 0}, {2, 1, 0, 0, 0}, {0, 2, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_join_new_host_packet", 0, sizeof(struct message_client_join_new_host), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_join_new_host_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_switch_to_pregame_packet", 0, sizeof(struct message_server_switch_to_pregame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_switch_to_pregame_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_server_graceful_game_exit_postgame_packet", 0, sizeof(struct message_server_graceful_game_exit_postgame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_server_graceful_game_exit_postgame_packet_fields, FALSE },
	{ {2, 12, 0, 0, 0}, {2, 2, 0, 0, 0}, {1, 4, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_remove_player_request_postgame_packet", 0, sizeof(struct message_client_remove_player_request_postgame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_remove_player_request_postgame_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_switch_to_pregame_packet", 0, sizeof(struct message_client_switch_to_pregame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_switch_to_pregame_packet_fields, FALSE },
	{ {_data_packet_field_longs, 1, 0, 0, 0}, {_data_packet_field_end, 0, 0, 0, 0} },
	{ "message_client_graceful_game_exit_postgame_packet", 0, sizeof(struct message_client_graceful_game_exit_postgame), NETWORK_GAME_MESSAGE_VERSION, data_0030aa68.message_client_graceful_game_exit_postgame_packet_fields, FALSE },
	// packet class then definition, indexed by message type
	// xxx rows 8..10 are shifted one against create_network_game_message's switch: type 8 encodes
	// under the pregame_keep_alive definition (2 bytes) though the switch asserts sizeof(begin_game),
	// 9 gets begin_game and 10 gets graceful_game_exit_pregame
	{
		{0, 0, &data_0030aa68.message_client_broadcast_game_search_packet},
		{0, 0, &data_0030aa68.message_client_ping_packet},
		{1, 0, &data_0030aa68.message_server_game_advertise_packet},
		{1, 0, &data_0030aa68.message_server_pong_packet},
		{2, 0, &data_0030aa68.message_server_machine_accepted_packet},
		{2, 0, &data_0030aa68.message_server_machine_rejected_packet},
		{2, 0, &data_0030aa68.message_server_game_settings_update_packet},
		{2, 0, &data_0030aa68.message_server_pregame_countdown_packet},
		{2, 0, &data_0030aa68.message_server_pregame_keep_alive_packet},
		{2, 0, &data_0030aa68.message_server_begin_game_packet},
		{2, 0, &data_0030aa68.message_server_graceful_game_exit_pregame_packet},
		{6, 0, &data_0030aa68.message_server_postgame_keep_alive_packet},
		{3, 0, &data_0030aa68.message_client_join_game_request_packet},
		{3, 0, &data_0030aa68.message_client_add_player_request_pregame_packet},
		{3, 0, &data_0030aa68.message_client_remove_player_request_pregame_packet},
		{3, 0, &data_0030aa68.message_client_settings_request_packet},
		{3, 0, &data_0030aa68.message_client_player_settings_request_packet},
		{3, 0, &data_0030aa68.message_client_game_start_request_packet},
		{3, 0, &data_0030aa68.message_client_graceful_game_exit_pregame_packet},
		{3, 0, &data_0030aa68.message_client_map_is_precached_pregame_packet},
		{4, 0, &data_0030aa68.message_server_game_update_packet},
		{4, 0, &data_0030aa68.message_server_add_player_ingame_packet},
		{4, 0, &data_0030aa68.message_server_remove_player_ingame_packet},
		{4, 0, &data_0030aa68.message_server_game_over_packet},
		{5, 0, &data_0030aa68.message_client_loaded_packet},
		{5, 0, &data_0030aa68.message_client_game_update_packet},
		{5, 0, &data_0030aa68.message_client_add_player_request_ingame_packet},
		{5, 0, &data_0030aa68.message_client_remove_player_request_ingame_packet},
		{5, 0, &data_0030aa68.message_client_host_crashed_cry_for_help_packet},
		{5, 0, &data_0030aa68.message_client_join_new_host_packet},
		{6, 0, &data_0030aa68.message_server_switch_to_pregame_packet},
		{6, 0, &data_0030aa68.message_server_graceful_game_exit_postgame_packet},
		{7, 0, &data_0030aa68.message_client_remove_player_request_postgame_packet},
		{7, 0, &data_0030aa68.message_client_switch_to_pregame_packet},
		{7, 0, &data_0030aa68.message_client_graceful_game_exit_postgame_packet}
	}
};

static struct data_packet_group_definition network_game_messages_group=
{
	"network_game_messages_group", NUMBER_OF_NETWORK_GAME_MESSAGE_TYPES, NUMBER_OF_NETWORK_GAME_MESSAGE_CLASSES,
	MAXIMUM_DECODED_NETWORK_GAME_MESSAGE_SIZE, MAXIMUM_ENCODED_NETWORK_GAME_MESSAGE_SIZE, data_0030aa68.packets
};

// create_network_game_message builds every outgoing message into this buffer
static byte bss_004566f0[MAXIMUM_DECODED_NETWORK_GAME_MESSAGE_SIZE+sizeof(struct message_header)];

/* ---------- public code */

void initialize_network_game_packets(
	void)
{
	data_packet_group_initialize(&network_game_messages_group);

	return;
}

void network_event(
	char *format,
	...)
{
	va_list list;

	match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 331, format);

	va_start(list, format);
	_vsnprintf(temporary, sizeof(temporary)-1, format, list);
	va_end(list);

	error(_error_log, temporary);

	return;
}

/* ---------- private code */

static boolean encode_network_game_message(
	void *message_struct,
	void *encoded_message,
	short *encoded_message_size,
	short packet_type,
	long packet_version)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 353,
		message_struct && encoded_message && encoded_message_size && (*encoded_message_size>0));

	return data_packet_group_encode_packet(&network_game_messages_group, message_struct, encoded_message,
		encoded_message_size, packet_type, packet_version);
}

/* ---------- public code */

struct message_header *create_network_game_message(
	short message_type,
	void *message_struct,
	short message_struct_size)
{
	struct message_header *message;
	byte encoded_message[MAXIMUM_DECODED_NETWORK_GAME_MESSAGE_SIZE];
	long encoded_message_size= sizeof(encoded_message);

	switch (message_type)
	{
		case _message_type_client_broadcast_game_search:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 160,
				message_struct_size==sizeof(struct message_client_broadcast_game_search));
			break;

		case _message_type_client_ping:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 161,
				message_struct_size==sizeof(struct message_client_ping));
			break;

		case _message_type_server_game_advertise:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 164,
				message_struct_size==sizeof(struct message_server_game_advertise));
			break;

		case _message_type_server_pong:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 165,
				message_struct_size==sizeof(struct message_server_pong));
			break;

		case _message_type_server_machine_accepted:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 168,
				message_struct_size==sizeof(struct message_server_machine_accepted));
			break;

		case _message_type_server_machine_rejected:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 169,
				message_struct_size==sizeof(struct message_server_machine_rejected));
			break;

		case _message_type_server_game_settings_update:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 170,
				message_struct_size==sizeof(struct message_server_game_settings_update));
			break;

		case _message_type_server_pregame_countdown:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 171,
				message_struct_size==sizeof(struct message_server_pregame_countdown));
			break;

		case _message_type_server_pregame_keep_alive:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 172,
				message_struct_size==sizeof(struct message_server_pregame_keep_alive));
			break;

		case _message_type_server_begin_game:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 173,
				message_struct_size==sizeof(struct message_server_begin_game));
			break;

		case _message_type_server_graceful_game_exit_pregame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 174,
				message_struct_size==sizeof(struct message_server_graceful_game_exit_pregame));
			break;

		case _message_type_server_postgame_keep_alive:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 177,
				message_struct_size==sizeof(struct message_server_postgame_keep_alive));
			break;

		case _message_type_client_join_game_request:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 180,
				message_struct_size==sizeof(struct message_client_join_game_request));
			break;

		case _message_type_client_add_player_request_pregame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 181,
				message_struct_size==sizeof(struct message_client_add_player_request_pregame));
			break;

		case _message_type_client_remove_player_request_pregame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 182,
				message_struct_size==sizeof(struct message_client_remove_player_request_pregame));
			break;

		case _message_type_client_settings_request:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 183,
				message_struct_size==sizeof(struct message_client_settings_request));
			break;

		case _message_type_client_player_settings_request:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 184,
				message_struct_size==sizeof(struct message_client_player_settings_request));
			break;

		case _message_type_client_game_start_request:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 185,
				message_struct_size==sizeof(struct message_client_game_start_request));
			break;

		case _message_type_client_graceful_game_exit_pregame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 186,
				message_struct_size==sizeof(struct message_client_graceful_game_exit_pregame));
			break;

		case _message_type_client_map_is_precached_pregame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 187,
				message_struct_size==sizeof(struct message_client_map_is_precached_pregame));
			break;

		case _message_type_server_game_update:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 190,
				message_struct_size==sizeof(struct message_server_game_update));
			break;

		case _message_type_server_add_player_ingame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 191,
				message_struct_size==sizeof(struct message_server_add_player_ingame));
			break;

		case _message_type_server_remove_player_ingame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 192,
				message_struct_size==sizeof(struct message_server_remove_player_ingame));
			break;

		case _message_type_server_game_over:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 193,
				message_struct_size==sizeof(struct message_server_game_over));
			break;

		case _message_type_client_loaded:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 196,
				message_struct_size==sizeof(struct message_client_loaded));
			break;

		case _message_type_client_game_update:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 197,
				message_struct_size==sizeof(struct message_client_game_update));
			break;

		case _message_type_client_add_player_request_ingame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 198,
				message_struct_size==sizeof(struct message_client_add_player_request_ingame));
			break;

		case _message_type_client_remove_player_request_ingame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 199,
				message_struct_size==sizeof(struct message_client_remove_player_request_ingame));
			break;

		case _message_type_client_host_crashed_cry_for_help:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 201,
				message_struct_size==sizeof(struct message_client_host_crashed_cry_for_help));
			break;

		case _message_type_client_join_new_host:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 202,
				message_struct_size==sizeof(struct message_client_join_new_host));
			break;

		case _message_type_server_switch_to_pregame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 205,
				message_struct_size==sizeof(struct message_server_switch_to_pregame));
			break;

		case _message_type_server_graceful_game_exit_postgame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 206,
				message_struct_size==sizeof(struct message_server_graceful_game_exit_postgame));
			break;

		case _message_type_client_remove_player_request_postgame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 209,
				message_struct_size==sizeof(struct message_client_remove_player_request_postgame));
			break;

		case _message_type_client_switch_to_pregame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 210,
				message_struct_size==sizeof(struct message_client_switch_to_pregame));
			break;

		case _message_type_client_graceful_game_exit_postgame:
			match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 211,
				message_struct_size==sizeof(struct message_client_graceful_game_exit_postgame));
			break;

		default:
			match_vhalt("c:\\halo\\SOURCE\\networking\\network_messages.c", 213,
				"unknown network game message structure type");
			break;
	}

	if (encode_network_game_message(message_struct, encoded_message, &encoded_message_size, message_type,
		NETWORK_GAME_MESSAGE_VERSION))
	{
		message= create_message(_message_type_packet, encoded_message, (word)encoded_message_size,
			bss_004566f0, sizeof(bss_004566f0));
		if (!message)
		{
			network_event("create_message() failed");
		}
	}
	else
	{
		network_event("encode_network_game_message() failed");
		message= NULL;
	}

	return message;
}

boolean decode_network_game_message(
	void *message_struct,
	void const *encoded_message,
	short *encoded_message_size,
	short *packet_type,
	short *packet_version,
	short expected_packet_class)
{
	boolean result;

	match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 313,
		message_struct && encoded_message && encoded_message_size && (*encoded_message_size>0) && packet_type && (*packet_type>=0) && packet_version && (*packet_version>0));

	result= data_packet_group_decode_packet(&network_game_messages_group, message_struct, encoded_message,
		encoded_message_size, packet_type, packet_version, expected_packet_class);

	if (!result)
	{
		network_event("decode_network_game_message() failed");
	}

	return result;
}
