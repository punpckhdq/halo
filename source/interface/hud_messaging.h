/*
HUD_MESSAGING.H

header included in hcex build.
*/

#ifndef __HUD_MESSAGING_H
#define __HUD_MESSAGING_H
#pragma once

/* ---------- headers */

#include "hud_definitions.h"

/* ---------- constants */

enum
{
	_hud_message_pickup = 0,
	_hud_message_swap_powerup,
	_hud_message_touch_device,
	_hud_message_custom_device,
	_hud_message_swap_weapon,
	_hud_message_remind_to_switch_weapons,
	_hud_message_enter_vehicle,
	_hud_message_exit_vehicle,
	_hud_message_flip_vehicle,
	_hud_message_respawn_failed_moving,
	_hud_message_respawn_failed_unsafe,
	_hud_message_respawn_failed_combat,
	_hud_message_respawn_failed_vehicle,
	NUMBER_OF_HUD_STATE_MESSAGE_TYPES,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/HUD_MESSAGING.C */

void hud_messaging_initialize(void);
void hud_messaging_initialize_for_new_map(void);
void hud_messaging_dispose_from_old_map(void);
void hud_messaging_dispose(void);
void scripted_hud_set_state_message(short message_index);
void scripted_hud_set_flashing_state(boolean flash);
void scripted_hud_restart_flashing(void);
void scripted_hud_set_objective(short message_index);
void scripted_hud_set_timer_time(short minutes, short seconds);
void scripted_hud_set_timer_warning_cutoff(short minutes, short seconds);
void scripted_hud_set_timer_position(short x, short y, short corner);
void scripted_hud_show_timer(boolean enabled);
void scripted_hud_pause_timer(boolean paused);
short scripted_hud_get_timer_ticks(void);
void scripted_hud_time_code_show(boolean show);
void scripted_hud_time_code_start(boolean start);
void scripted_hud_time_code_reset(void);
void hud_render_timer(void);
void hud_set_state_message(short local_player_index, short message_index);
void hud_set_state_message_icon(short local_player_index, short custom_icon_index, struct icon_hud_element_definition const *icon);
void hud_set_state_message_text(short local_player_index, short custom_icon_index, short icon_string_index, boolean uses_scenario_names);
void hud_enable_custom_state_message(short local_player_index, boolean enabled);
void hud_set_state_text(short local_player_index, wchar_t const *message);
wchar_t const *hud_messaging_get_objective(void);
void scripted_hud_messages_clear(void);
long hud_get_font_index(void);
real_argb_color *hud_get_text_color(real_argb_color *result);
void hud_messaging_globals_update(void);
void hud_print_message(short local_player_index, wchar_t const *text);
void hud_add_item_message(short local_player_index, long item_definition_index, short quantity, char message_offset);
void hud_broadcast_team_message(long victim_player_index, wchar_t const *message);
void hud_messaging_update(short local_player_index);

/* ---------- globals */

/* ---------- public code */

#endif // __HUD_MESSAGING_H
