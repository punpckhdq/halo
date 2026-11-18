/*
NETWORK_MESSAGES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "network_messages.h"
#include "network_game_manager.h"
#include "data_packet_groups.h"
#include "bungie_net/common/message_header.h"

/* ---------- constants */

enum
{
	MAXIMUM_NETWORK_GAME_ENCODED_MESSAGE_SIZE = 2048, /* fake name */
	NETWORK_GAME_MESSAGE_BUFFER_SIZE = 1540, /* fake name */
};

/* ---------- macros */

#define FIELD_COUNT(structure, first, last, element) ((offsetof(structure, last) - offsetof(structure, first)) / sizeof(element) + 1) /* fake name */
#define CASE_MESSAGE_TYPE(type, line) case _message_type_##type: match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", line, message_struct_size==sizeof(message_##type)); break /* fake name */

/* ---------- structures */

/* ---------- prototypes */

static boolean encode_network_game_message(void *message_struct, void *encoded_message, short *encoded_message_size, short packet_type, short packet_version);

/* ---------- globals */

static struct data_packet_field message_client_broadcast_game_search_fields[] = /* fake name */
{
	{__pack_short, FIELD_COUNT(struct message_client_broadcast_game_search, port, version, word), 0, 0, 0},
	{__pack_char, NETWORK_GAME_NONCE_BYTES, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_broadcast_game_search_packet =
{
	"message_client_broadcast_game_search_packet",
	0,
	sizeof(message_client_broadcast_game_search),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_broadcast_game_search_fields,
	FALSE,
};

static struct data_packet_field message_client_ping_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_short, 1, 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_ping_packet =
{
	"message_client_ping_packet",
	0,
	sizeof(message_client_ping),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_ping_fields,
	FALSE,
};

static struct data_packet_field message_server_game_advertise_fields[] = /* fake name */
{
	{__pack_fixed_data, sizeof(message_server_game_advertise), 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_game_advertise_packet =
{
	"message_server_game_advertise_packet",
	0,
	sizeof(message_server_game_advertise),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_game_advertise_fields,
	FALSE,
};

static struct data_packet_field message_server_pong_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_pong_packet =
{
	"message_server_pong_packet",
	0,
	sizeof(message_server_pong),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_pong_fields,
	FALSE,
};

static struct data_packet_field message_server_machine_accepted_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_short, 1, 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_machine_accepted_packet =
{
	"message_server_machine_accepted_packet",
	0,
	sizeof(message_server_machine_accepted),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_machine_accepted_fields,
	FALSE,
};

static struct data_packet_field message_server_machine_rejected_fields[] = /* fake name */
{
	{__pack_short, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_machine_rejected_packet =
{
	"message_server_machine_rejected_packet",
	0,
	sizeof(message_server_machine_rejected),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_machine_rejected_fields,
	FALSE,
};

static struct data_packet_field message_server_game_settings_update_fields[] = /* fake name */
{
	{__pack_fixed_data, sizeof(message_server_game_settings_update), 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_game_settings_update_packet =
{
	"message_server_game_settings_update_packet",
	0,
	sizeof(message_server_game_settings_update),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_game_settings_update_fields,
	FALSE,
};

static struct data_packet_field message_server_pregame_countdown_fields[] = /* fake name */
{
	{__pack_short, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_pregame_countdown_packet =
{
	"message_server_pregame_countdown_packet",
	0,
	sizeof(message_server_pregame_countdown),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_pregame_countdown_fields,
	FALSE,
};

static struct data_packet_field message_server_pregame_keep_alive_fields[] = /* fake name */
{
	{__pack_short, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_pregame_keep_alive_packet =
{
	"message_server_pregame_keep_alive_packet",
	0,
	sizeof(message_server_pregame_keep_alive),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_pregame_keep_alive_fields,
	FALSE,
};

static struct data_packet_field message_server_postgame_keep_alive_fields[] = /* fake name */
{
	{__pack_short, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_postgame_keep_alive_packet =
{
	"message_server_postgame_keep_alive_packet",
	0,
	sizeof(message_server_postgame_keep_alive),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_postgame_keep_alive_fields,
	FALSE,
};

static struct data_packet_field message_server_begin_game_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_begin_game_packet =
{
	"message_server_begin_game_packet",
	0,
	sizeof(message_server_begin_game),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_begin_game_fields,
	FALSE,
};

static struct data_packet_field message_server_graceful_game_exit_pregame_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_graceful_game_exit_pregame_packet =
{
	"message_server_graceful_game_exit_pregame_packet",
	0,
	sizeof(message_server_graceful_game_exit_pregame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_graceful_game_exit_pregame_fields,
	FALSE,
};

static struct data_packet_field message_client_join_game_request_fields[] = /* fake name */
{
	{__pack_short, MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH, 0, 0, 0},
	{__pack_char, NETWORK_JOIN_GAME_TOKEN_SIZE, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_join_game_request_packet =
{
	"message_client_join_game_request_packet",
	0,
	sizeof(message_client_join_game_request),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_join_game_request_fields,
	FALSE,
};

static struct data_packet_field message_client_add_player_request_pregame_fields[] = /* fake name */
{
	{__pack_short, MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH, 0, 0, 0},
	{__pack_short, FIELD_COUNT(struct network_player, primary_color_index, icon_index, short), 0, 0, 0},
	{__pack_char, FIELD_COUNT(struct network_player, machine_index, player_list_index, char), 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_add_player_request_pregame_packet =
{
	"message_client_add_player_request_pregame_packet",
	0,
	sizeof(message_client_add_player_request_pregame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_add_player_request_pregame_fields,
	FALSE,
};

static struct data_packet_field message_client_remove_player_request_pregame_fields[] = /* fake name */
{
	{__pack_short, MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH, 0, 0, 0},
	{__pack_short, FIELD_COUNT(struct network_player, primary_color_index, icon_index, short), 0, 0, 0},
	{__pack_char, FIELD_COUNT(struct network_player, machine_index, player_list_index, char), 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_remove_player_request_pregame_packet =
{
	"message_client_remove_player_request_pregame_packet",
	0,
	sizeof(message_client_remove_player_request_pregame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_remove_player_request_pregame_fields,
	FALSE,
};

static struct data_packet_field message_client_settings_request_fields[] = /* fake name */
{
	{__pack_short, MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH, 0, 0, 0},
	{__pack_char, 1, 0, 0, 0},
	{__pack_pad, 3, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_settings_request_packet =
{
	"message_client_settings_request_packet",
	0,
	sizeof(message_client_settings_request),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_settings_request_fields,
	FALSE,
};

static struct data_packet_field message_client_player_settings_request_fields[] = /* fake name */
{
	{__pack_short, MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH, 0, 0, 0},
	{__pack_short, FIELD_COUNT(struct network_player, primary_color_index, icon_index, short), 0, 0, 0},
	{__pack_char, FIELD_COUNT(struct network_player, machine_index, player_list_index, char), 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_player_settings_request_packet =
{
	"message_client_player_settings_request_packet",
	0,
	sizeof(message_client_player_settings_request),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_player_settings_request_fields,
	FALSE,
};

static struct data_packet_field message_client_game_start_request_fields[] = /* fake name */
{
	{__pack_short, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_game_start_request_packet =
{
	"message_client_game_start_request_packet",
	0,
	sizeof(message_client_game_start_request),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_game_start_request_fields,
	FALSE,
};

static struct data_packet_field message_client_graceful_game_exit_pregame_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_graceful_game_exit_pregame_packet =
{
	"message_client_graceful_game_exit_pregame_packet",
	0,
	sizeof(message_client_graceful_game_exit_pregame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_graceful_game_exit_pregame_fields,
	FALSE,
};

static struct data_packet_field message_client_map_is_precached_pregame_fields[] = /* fake name */
{
	{__pack_char, sizeof(message_client_map_is_precached_pregame), 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_map_is_precached_pregame_packet =
{
	"message_client_map_is_precached_pregame_packet",
	0,
	sizeof(message_client_map_is_precached_pregame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_map_is_precached_pregame_fields,
	FALSE,
};

static struct data_packet_field message_server_game_update_fields[] = /* fake name */
{
	{__pack_long, FIELD_COUNT(struct message_server_game_update, update_number, debug_game_time, long), 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_array, NETWORK_GAME_MAXIMUM_PLAYER_COUNT, 0, 0, 0},
	{__pack_long, FIELD_COUNT(struct player_action, control_flags, primary_trigger, long), 0, 0, 0},
	{__pack_short, FIELD_COUNT(struct player_action, desired_weapon_index, desired_zoom_level, short), 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_game_update_packet =
{
	"message_server_game_update_packet",
	0,
	sizeof(message_server_game_update),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_game_update_fields,
	FALSE,
};

static struct data_packet_field message_server_add_player_ingame_fields[] = /* fake name */
{
	{__pack_short, MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH, 0, 0, 0},
	{__pack_short, FIELD_COUNT(struct network_player, primary_color_index, icon_index, short), 0, 0, 0},
	{__pack_char, FIELD_COUNT(struct network_player, machine_index, player_list_index, char), 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_add_player_ingame_packet =
{
	"message_server_add_player_ingame_packet",
	0,
	sizeof(message_server_add_player_ingame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_add_player_ingame_fields,
	FALSE,
};

static struct data_packet_field message_server_remove_player_ingame_fields[] = /* fake name */
{
	{__pack_short, MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH, 0, 0, 0},
	{__pack_short, FIELD_COUNT(struct network_player, primary_color_index, icon_index, short), 0, 0, 0},
	{__pack_char, FIELD_COUNT(struct network_player, machine_index, player_list_index, char), 0, 0, 0},
	{__pack_long, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_remove_player_ingame_packet =
{
	"message_server_remove_player_ingame_packet",
	0,
	sizeof(message_server_remove_player_ingame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_remove_player_ingame_fields,
	FALSE,
};

static struct data_packet_field message_server_game_over_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_game_over_packet =
{
	"message_server_game_over_packet",
	0,
	sizeof(message_server_game_over),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_game_over_fields,
	FALSE,
};

static struct data_packet_field message_client_loaded_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_loaded_packet =
{
	"message_client_loaded_packet",
	0,
	sizeof(message_client_loaded),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_loaded_fields,
	FALSE,
};

static struct data_packet_field message_client_game_update_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_array, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS, 0, 0, 0},
	{__pack_long, FIELD_COUNT(struct player_action, control_flags, primary_trigger, long), 0, 0, 0},
	{__pack_short, FIELD_COUNT(struct player_action, desired_weapon_index, desired_zoom_level, short), 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_game_update_packet =
{
	"message_client_game_update_packet",
	0,
	sizeof(message_client_game_update),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_game_update_fields,
	FALSE,
};

static struct data_packet_field message_client_add_player_request_ingame_fields[] = /* fake name */
{
	{__pack_short, MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH, 0, 0, 0},
	{__pack_short, FIELD_COUNT(struct network_player, primary_color_index, icon_index, short), 0, 0, 0},
	{__pack_char, FIELD_COUNT(struct network_player, machine_index, player_list_index, char), 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_add_player_request_ingame_packet =
{
	"message_client_add_player_request_ingame_packet",
	0,
	sizeof(message_client_add_player_request_ingame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_add_player_request_ingame_fields,
	FALSE,
};

static struct data_packet_field message_client_remove_player_request_ingame_fields[] = /* fake name */
{
	{__pack_short, MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH, 0, 0, 0},
	{__pack_short, FIELD_COUNT(struct network_player, primary_color_index, icon_index, short), 0, 0, 0},
	{__pack_char, FIELD_COUNT(struct network_player, machine_index, player_list_index, char), 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_remove_player_request_ingame_packet =
{
	"message_client_remove_player_request_ingame_packet",
	0,
	sizeof(message_client_remove_player_request_ingame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_remove_player_request_ingame_fields,
	FALSE,
};

static struct data_packet_field message_client_host_crashed_cry_for_help_fields[] = /* fake name */
{
	{__pack_long, 3, 0, 0, 0},
	{__pack_short, 1, 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_host_crashed_cry_for_help_packet =
{
	"message_client_host_crashed_cry_for_help_packet",
	0,
	sizeof(message_client_host_crashed_cry_for_help),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_host_crashed_cry_for_help_fields,
	FALSE,
};

static struct data_packet_field message_client_join_new_host_fields[] = /* fake name */
{
	{__pack_long, 3, 0, 0, 0},
	{__pack_short, 1, 0, 0, 0},
	{__pack_pad, 2, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_join_new_host_packet =
{
	"message_client_join_new_host_packet",
	0,
	sizeof(message_client_join_new_host),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_join_new_host_fields,
	FALSE,
};

static struct data_packet_field message_server_switch_to_pregame_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_switch_to_pregame_packet =
{
	"message_server_switch_to_pregame_packet",
	0,
	sizeof(message_server_switch_to_pregame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_switch_to_pregame_fields,
	FALSE,
};

static struct data_packet_field message_server_graceful_game_exit_postgame_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_server_graceful_game_exit_postgame_packet =
{
	"message_server_graceful_game_exit_postgame_packet",
	0,
	sizeof(message_server_graceful_game_exit_postgame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_server_graceful_game_exit_postgame_fields,
	FALSE,
};

static struct data_packet_field message_client_remove_player_request_postgame_fields[] = /* fake name */
{
	{__pack_short, MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH, 0, 0, 0},
	{__pack_short, FIELD_COUNT(struct network_player, primary_color_index, icon_index, short), 0, 0, 0},
	{__pack_char, FIELD_COUNT(struct network_player, machine_index, player_list_index, char), 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_remove_player_request_postgame_packet =
{
	"message_client_remove_player_request_postgame_packet",
	0,
	sizeof(message_client_remove_player_request_postgame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_remove_player_request_postgame_fields,
	FALSE,
};

static struct data_packet_field message_client_switch_to_pregame_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_switch_to_pregame_packet =
{
	"message_client_switch_to_pregame_packet",
	0,
	sizeof(message_client_switch_to_pregame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_switch_to_pregame_fields,
	FALSE,
};

static struct data_packet_field message_client_graceful_game_exit_postgame_fields[] = /* fake name */
{
	{__pack_long, 1, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition message_client_graceful_game_exit_postgame_packet =
{
	"message_client_graceful_game_exit_postgame_packet",
	0,
	sizeof(message_client_graceful_game_exit_postgame),
	NETWORK_GAME_MESSAGE_VERSION,
	message_client_graceful_game_exit_postgame_fields,
	FALSE,
};

static struct data_packet_group_packet network_game_messages[NUMBER_OF_NETWORK_GAME_MESSAGE_TYPES] = /* fake name */
{
	{_message_class_unconnected_client, &message_client_broadcast_game_search_packet},
	{_message_class_unconnected_client, &message_client_ping_packet},
	{_message_class_unconnected_server, &message_server_game_advertise_packet},
	{_message_class_unconnected_server, &message_server_pong_packet},
	{_message_class_server_pregame, &message_server_machine_accepted_packet},
	{_message_class_server_pregame, &message_server_machine_rejected_packet},
	{_message_class_server_pregame, &message_server_game_settings_update_packet},
	{_message_class_server_pregame, &message_server_pregame_countdown_packet},
	{_message_class_server_pregame, &message_server_pregame_keep_alive_packet},
	{_message_class_server_pregame, &message_server_begin_game_packet},
	{_message_class_server_pregame, &message_server_graceful_game_exit_pregame_packet},
	{_message_class_server_postgame, &message_server_postgame_keep_alive_packet},
	{_message_class_client_pregame, &message_client_join_game_request_packet},
	{_message_class_client_pregame, &message_client_add_player_request_pregame_packet},
	{_message_class_client_pregame, &message_client_remove_player_request_pregame_packet},
	{_message_class_client_pregame, &message_client_settings_request_packet},
	{_message_class_client_pregame, &message_client_player_settings_request_packet},
	{_message_class_client_pregame, &message_client_game_start_request_packet},
	{_message_class_client_pregame, &message_client_graceful_game_exit_pregame_packet},
	{_message_class_client_pregame, &message_client_map_is_precached_pregame_packet},
	{_message_class_server_ingame, &message_server_game_update_packet},
	{_message_class_server_ingame, &message_server_add_player_ingame_packet},
	{_message_class_server_ingame, &message_server_remove_player_ingame_packet},
	{_message_class_server_ingame, &message_server_game_over_packet},
	{_message_class_client_ingame, &message_client_loaded_packet},
	{_message_class_client_ingame, &message_client_game_update_packet},
	{_message_class_client_ingame, &message_client_add_player_request_ingame_packet},
	{_message_class_client_ingame, &message_client_remove_player_request_ingame_packet},
	{_message_class_client_ingame, &message_client_host_crashed_cry_for_help_packet},
	{_message_class_client_ingame, &message_client_join_new_host_packet},
	{_message_class_server_postgame, &message_server_switch_to_pregame_packet},
	{_message_class_server_postgame, &message_server_graceful_game_exit_postgame_packet},
	{_message_class_client_postgame, &message_client_remove_player_request_postgame_packet},
	{_message_class_client_postgame, &message_client_switch_to_pregame_packet},
	{_message_class_client_postgame, &message_client_graceful_game_exit_postgame_packet},
};

static struct data_packet_group_definition network_game_messages_group =
{
	"network_game_messages_group",
	NUMBER_OF_NETWORK_GAME_MESSAGE_TYPES,
	NUMBER_OF_NETWORK_GAME_MESSAGE_CLASSES,
	MAXIMUM_DECODED_NETWORK_GAME_MESSAGE_SIZE,
	MAXIMUM_NETWORK_GAME_ENCODED_MESSAGE_SIZE,
	network_game_messages,
};

static byte network_game_message_buffer[NETWORK_GAME_MESSAGE_BUFFER_SIZE]; /* fake name */

/* ---------- public code */

void initialize_network_game_packets(
	void)
{
	data_packet_group_initialize(&network_game_messages_group);

	return;
}

message_header *create_network_game_message(
	short message_type,
	void *message_struct,
	short message_struct_size)
{
	message_header *message;
	byte encoded_message[MAXIMUM_DECODED_NETWORK_GAME_MESSAGE_SIZE];
	short encoded_message_size = sizeof(encoded_message);

	switch (message_type)
	{
	CASE_MESSAGE_TYPE(client_broadcast_game_search, 160);
	CASE_MESSAGE_TYPE(client_ping, 161);


	CASE_MESSAGE_TYPE(server_game_advertise, 164);
	CASE_MESSAGE_TYPE(server_pong, 165);


	CASE_MESSAGE_TYPE(server_machine_accepted, 168);
	CASE_MESSAGE_TYPE(server_machine_rejected, 169);
	CASE_MESSAGE_TYPE(server_game_settings_update, 170);
	CASE_MESSAGE_TYPE(server_pregame_countdown, 171);
	CASE_MESSAGE_TYPE(server_pregame_keep_alive, 172);
	CASE_MESSAGE_TYPE(server_begin_game, 173);
	CASE_MESSAGE_TYPE(server_graceful_game_exit_pregame, 174);


	CASE_MESSAGE_TYPE(server_postgame_keep_alive, 177);


	CASE_MESSAGE_TYPE(client_join_game_request, 180);
	CASE_MESSAGE_TYPE(client_add_player_request_pregame, 181);
	CASE_MESSAGE_TYPE(client_remove_player_request_pregame, 182);
	CASE_MESSAGE_TYPE(client_settings_request, 183);
	CASE_MESSAGE_TYPE(client_player_settings_request, 184);
	CASE_MESSAGE_TYPE(client_game_start_request, 185);
	CASE_MESSAGE_TYPE(client_graceful_game_exit_pregame, 186);
	CASE_MESSAGE_TYPE(client_map_is_precached_pregame, 187);


	CASE_MESSAGE_TYPE(server_game_update, 190);
	CASE_MESSAGE_TYPE(server_add_player_ingame, 191);
	CASE_MESSAGE_TYPE(server_remove_player_ingame, 192);
	CASE_MESSAGE_TYPE(server_game_over, 193);


	CASE_MESSAGE_TYPE(client_loaded, 196);
	CASE_MESSAGE_TYPE(client_game_update, 197);
	CASE_MESSAGE_TYPE(client_add_player_request_ingame, 198);
	CASE_MESSAGE_TYPE(client_remove_player_request_ingame, 199);

	CASE_MESSAGE_TYPE(client_host_crashed_cry_for_help, 201);
	CASE_MESSAGE_TYPE(client_join_new_host, 202);


	CASE_MESSAGE_TYPE(server_switch_to_pregame, 205);
	CASE_MESSAGE_TYPE(server_graceful_game_exit_postgame, 206);


	CASE_MESSAGE_TYPE(client_remove_player_request_postgame, 209);
	CASE_MESSAGE_TYPE(client_switch_to_pregame, 210);
	CASE_MESSAGE_TYPE(client_graceful_game_exit_postgame, 211);
	default:
		match_vhalt("c:\\halo\\SOURCE\\networking\\network_messages.c", 213, "unknown network game message structure type");
		break;
	}

	if (encode_network_game_message(message_struct, encoded_message, &encoded_message_size, message_type, NETWORK_GAME_MESSAGE_VERSION))
	{
		message = create_message(
			_message_type_packet,
			encoded_message,
			encoded_message_size,
			network_game_message_buffer,
			sizeof(network_game_message_buffer));

		if (!message)
		{
			network_event("create_message() failed");
		}
	}
	else
	{
		network_event("encode_network_game_message() failed");
		message = NULL;
	}

	return message;
}

boolean decode_network_game_message(
	void *message_struct,
	void *encoded_message,
	short *encoded_message_size,
	short *packet_type,
	short *packet_version,
	short expected_packet_class)
{
	boolean success;

	match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 313, message_struct && encoded_message && encoded_message_size && (*encoded_message_size>0) && packet_type && (*packet_type>=0) && packet_version && (*packet_version>0));

	success = data_packet_group_decode_packet(
		&network_game_messages_group,
		message_struct,
		encoded_message,
		encoded_message_size,
		packet_type,
		packet_version,
		expected_packet_class);

	if (!success)
	{
		network_event("decode_network_game_message() failed");
	}

	return success;
}

#ifdef DEBUG
void network_event(
	char *format,
	...)
{
	va_list arglist;

	match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 331, format);

	va_start(arglist, format);
	_vsnprintf(temporary, NUMBEROF(temporary) - 1, format, arglist);
	error(_error_log, temporary);

	return;
}
#endif

/* ---------- private code */

static boolean encode_network_game_message(
	void *message_struct,
	void *encoded_message,
	short *encoded_message_size,
	short packet_type,
	short packet_version)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_messages.c", 353, message_struct && encoded_message && encoded_message_size && (*encoded_message_size>0));

	return data_packet_group_encode_packet(
		&network_game_messages_group,
		message_struct,
		encoded_message,
		encoded_message_size,
		packet_type,
		packet_version);
}
