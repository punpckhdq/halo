/*
HUD_DEFINITIONS.H
*/

#ifndef __HUD_DEFINITIONS_H
#define __HUD_DEFINITIONS_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	_hud_draw_flashing_bit = 0,
	_hud_draw_disabled_bit,
	_hud_draw_in_multiplayer_bit,
	NUMBER_OF_HUD_DRAW_FLAGS,
};

enum
{
	_hud_top_left = 0,
	_hud_top_right,
	_hud_bottom_left,
	_hud_bottom_right,
	_hud_center,
	NUMBER_OF_HUD_ANCHORS,
};

enum
{
	_waypoint_on_screen,
	_waypoint_off_screen,
	_waypoint_occluded,
	NUMBER_OF_WAYPOINT_TYPES,
};

enum
{
	_hud_waypoint_dont_rotate_offscreen,
	NUMBER_OF_WAYPOINT_FLAGS,
};

enum
{
	_hud_number_show_all_leading_zeros_bit,
	_hud_number_show_only_when_zoomed,
	_hud_number_show_trailing_m,
	NUMBER_OF_HUD_NUMBER_FLAGS,
};

/* ---------- structures */

struct hud_waypoint_arrow
{
	char name[TAG_STRING_LENGTH+1];
	long unused0[2];
	unsigned long color;
	real opacity;
	real fade;
	short sequence_indices[NUMBER_OF_WAYPOINT_TYPES];
	short pad;
	long unused1[4];
	unsigned long flags;
	long unused2[6];
};

struct sound_hud_element_definition
{
	struct tag_reference sound;
	long type_flags;
	real scale;
	long unused[3];
	byte pad0;
	byte pad[3];
	long unused2[4];
};

struct hud_placement_definition
{
	point2d offset;
	real_vector2d scale;
	short multiplayer_scaling_flags;
	short pad;
	long unused0[5];
};

struct hud_absolute_placement_definition
{
	short corner;
	short pad;
	long unused[8];
};

struct hud_color_definition
{
	unsigned long color;
	unsigned long flash_color;
	real flash_period;
	real flash_delay;
	short number_of_flashes;
	unsigned short flash_flags;
	real flash_length;
	unsigned long disabled_color;
	union
	{
		long unused;
		struct { short up_ticks, fade_ticks; } objective;
	} custom;
};

struct static_hud_element_definition
{
	struct hud_placement_definition placement;
	struct tag_reference interface_bitmap;
	struct hud_color_definition colors;
	short sequence_index;
	short pad;
	struct tag_block multitexture_overlays;
	long unused0[1];
};

struct meter_hud_element_definition
{
	struct hud_placement_definition placement;
	struct tag_reference meter_bitmap;
	unsigned long min_color;
	unsigned long max_color;
	unsigned long flash_color;
	unsigned long empty_color;
	byte meter_flags;
	byte minimum_value;
	short sequence_index;
	byte alpha_multiplier;
	byte alpha_bias;
	short value_scale;
	real opacity;
	real fade;
	unsigned long disabled_color;
	struct tag_block multitexture_overlays;
	long unused0[1];
};

struct number_hud_element_definition
{
	struct hud_placement_definition placement;
	struct hud_color_definition colors;
	char digits;
	byte number_flags;
	char fractional_digits;
	byte pad;
	long unused1[3];
};

struct hud_damage_indicators_definition
{
	short top_offset;
	short bottom_offset;
	short left_offset;
	short right_offset;
	long unused[8];
	struct tag_reference indicator_bitmap;
	short sequence_index;
	short multiplayer_sequence_index;
	unsigned long color;
	long unused2[4];
};

struct hud_multiplayer_parameters_definition
{
	real hud_scale;
	long unused[64];
};

struct hud_waypoint_definition
{
	real top_offset;
	real bottom_offset;
	real left_offset;
	real right_offset;
	long unused0[8];
	struct tag_reference arrow_bitmap;
	struct tag_block arrows;
	long unused1[20];
};

struct hud_defaults_definition
{
	struct tag_reference default_weapon_hud;
	real motion_sensor_range;
	real motion_sensor_velocity_sensitivity;
	real motion_sensor_scale;
	rectangle2d default_title_bounds;
	long unused[11];
};

struct hud_timer_definition
{
	struct hud_color_definition color;
	struct hud_color_definition time_up_color;
	long unused[10];
};

struct hud_messaging_parameters_definition
{
	struct hud_absolute_placement_definition absolute_placement;
	struct hud_placement_definition placement;
	struct tag_reference single_player_font;
	struct tag_reference multi_player_font;
	real up_time;
	real fade_time;
	real_argb_color state_color;
	real_argb_color text_color;
	real spacing;
	struct tag_reference hud_item_messages;
	struct tag_reference messaging_icons;
	struct tag_reference alternate_icon_text;
	struct tag_block button_icons;
	struct hud_color_definition color;
	struct tag_reference hud_messages;
	struct hud_color_definition objective_color;
};

struct hud_globals_definition
{
	struct hud_messaging_parameters_definition messaging;
	struct hud_waypoint_definition waypoint;
	struct hud_multiplayer_parameters_definition multiplayer;
	struct hud_defaults_definition defaults;
	struct hud_damage_indicators_definition damage_indicators;
	struct hud_timer_definition timer_definition;
	struct tag_reference carnage_report_bitmap;
	short loading_begin_index;
	short loading_end_index;
	short checkpoint_begin_index;
	short checkpoint_end_index;
	struct tag_reference checkpoint_sound;
	long unused2[24];
};

#endif // __HUD_DEFINITIONS_H
