/*
HUD.H

header included in hcex build.
*/

#ifndef __HUD_H
#define __HUD_H
#pragma once

/* ---------- headers */

#include "hud_definitions.h"

/* ---------- constants */

enum
{
    // (Not real enum names)
	HUD_STACK_BUFFER_LONG_COUNT = 128,
	HUD_STACK_BUFFER_BYTE = 0x62,
	HUD_STACK_BUFFER_LONG = 0x62626262
};

/* ---------- macros */

/* ---------- structures */

struct hud_scripted_globals_definition
{
	boolean show_hud;
	boolean show_hud_help_text;
	byte pad[2];
};

/* ---------- prototypes/HUD_UNIT.C */

struct player_datum;

void unit_hud_shield_meter_mapper_init(void);
void hud_initialize_unit_interface(void);
void hud_initialize_unit_interface_for_new_map(void);
void hud_dispose_unit_interface_from_old_map(void);
void hud_dispose_unit_interface(void);
void scripted_hud_show_health(boolean show);
void scripted_hud_blink_health(boolean blink);
void scripted_hud_show_shield(boolean show);
void scripted_hud_blink_shield(boolean blink);
void scripted_hud_show_motion_sensor(boolean show);
void scripted_hud_blink_motion_sensor(boolean blink);
void hud_play_unit_sounds(struct player_datum *player, boolean show_hud);
void hud_fix_unit_data(short old_local_player_index, short new_local_player_index);
void hud_render_damage_indicators(short local_player_index);
void hud_tick_shield(long player_index, real amount);
void hud_update_unit(void);
void hud_render_unit_interface(struct player_datum *player);

/* ---------- prototypes/HUD_NAV_POINTS.C */

short find_nav_point(char const *name);
void hud_initialize_nav_points(void);
void hud_initialize_nav_points_for_new_map(void);
void hud_dispose_nav_points_from_old_map(void);
void hud_dispose_nav_points(void);
void hud_activate_nav_point_with_game_engine_flag(short nav_index, long player_index, short flag_index, real vertical_offset);
void hud_activate_nav_point_with_flag(short nav_index, long player_index, short flag_index, real vertical_offset);
void hud_activate_nav_point_with_object(short nav_index, long player_index, long object_index, real vertical_offset);
void hud_activate_team_nav_point_with_game_engine_flag(short nav_index, short team_index, short flag_index, real vertical_offset);
void hud_activate_team_nav_point_with_flag(short nav_index, short team_index, short flag_index, real vertical_offset);
void hud_activate_team_nav_point_with_object(short nav_index, short team_index, long object_index, real vertical_offset);
void hud_activate_global_nav_point_with_game_engine_flag(short nav_index, short flag_index, real vertical_offset);
void hud_deactivate_nav_point_with_game_engine_flag(long player_index, short flag_index);
void hud_deactivate_nav_point_with_flag(long player_index, short flag_index);
void hud_deactivate_nav_point_with_object(long player_index, long object_index);
void hud_deactivate_team_nav_point_with_flag(short team_index, short flag_index);
void hud_deactivate_team_nav_point_with_object(short team_index, long object_index);
void hud_unit_activate_nav_point_with_flag(short nav_index, long unit_index, short flag_index, real vertical_offset);
void hud_unit_activate_nav_point_with_object(short nav_index, long unit_index, long object_index, real vertical_offset);
void hud_unit_deactivate_nav_point_with_flag(long unit_index, short flag_index);
void hud_unit_deactivate_nav_point_with_object(long unit_index, long object_index);
short hud_get_nav_point_render_type(short local_player_index, real_point3d const *head, real_point3d const *position, long reference_object_index);
void custom_render_nav_point(short local_player_index, real_point3d const *position_pointer, short nav_index, short waypoint_type);
void hud_render_nav_points(short local_player_index);
void hud_update_nav_points(void);

/* ---------- prototypes/HUD_SOUNDS.C */

void hud_play_sound(short local_player_index, long type_flags, struct tag_block *sounds, long *sound_handles, word *sound_flags);

/* ---------- prototypes/HUD_DRAW.C */

long get_return_eip(void);
real hud_globals_get_scale(boolean in_multiplayer);
void hud_retrieve_bitmap_and_bounding_rect(long bitmap_group_index, short sequence_index, short frame_index, struct bitmap_data const **bitmap, real_rectangle2d const **clip);
void hud_draw_bitmap_direct(struct bitmap_data const *bitmap, short placement, point2d const *point, real_rectangle2d const *clip, real scale, real theta, unsigned long color, boolean is_interface_bitmap);
void hud_draw_static_element(short local_player_index, struct hud_absolute_placement_definition const *placement, struct static_hud_element_definition const *static_element, short draw_flags, long flash_reference_time);
void hud_draw_meter(short local_player_index, struct hud_absolute_placement_definition const *placement, struct meter_hud_element_definition const *meter, byte min_value, byte max_value, short draw_flags, real reference_time, real reference_value);
void hud_calculate_point(short local_player_index, struct hud_absolute_placement_definition const *absolute_placement, struct hud_placement_definition const *placement, struct bitmap_data const *bitmap, boolean in_multiplayer, real override_scale, point2d *result);
long get_flash_duration(struct hud_color_definition const *color);

void hud_draw_numbers(short local_player_index, struct hud_absolute_placement_definition const *placement, struct number_hud_element_definition const *numbers, short value, short decimal_value, short draw_flags, real flash_reference_time, real override_scale);

/* ---------- globals */

extern struct hud_scripted_globals_definition *hud_scripted_globals;
extern struct hud_globals_definition *hud_globals;

/* ---------- public code */

__inline short check_stack_buffer(long const *buffer)
{
	short index;

	for (index = HUD_STACK_BUFFER_LONG_COUNT - 1; index >= 0; index--)
	{
		if (buffer[index] != HUD_STACK_BUFFER_LONG)
		{
			return index;
		}
	}

	return NONE;
}

#endif // __HUD_H
