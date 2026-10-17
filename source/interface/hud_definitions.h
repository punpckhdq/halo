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
	HUD_GLOBALS_DEFINITION_TAG = 'hudg', /* fake name */
	HUD_NUMBER_DEFINITION_TAG = 'hud#', /* fake name */
};

enum
{
	_hud_draw_flashing_bit = 0,
	_hud_draw_disabled_bit,
	_hud_draw_in_multiplayer_bit,
	NUMBER_OF_HUD_DRAW_FLAGS,
};

enum
{
	MAXIMUM_NUMBER_OF_HUD_SOUNDS = 12,
	MAXIMUM_NUMBER_OF_WAYPOINTS = 16,
	MAXIMUM_NUMBER_OF_HUD_DECIMAL_DIGITS = 4,
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

enum
{
	_hud_flash_reverse_colors_bit = 0,
	NUMBER_OF_HUD_FLASH_FLAGS,
};

enum
{
	_hud_icon_use_text_bit = 0,
	_hud_icon_use_color_bit,
	_hud_icon_absolute_width_bit,
	NUMBER_OF_ICON_FLAGS,
};

enum
{
	_hud_dont_scale_offset_bit = 0,
	_hud_dont_scale_size_bit,
	NUMBER_OF_HUD_SCALING_FLAGS = 3,
};

enum
{
	_hud_number_decimal_index = 10,
	_hud_number_colon_index,
	_hud_number_negative_sign_index,
	_hud_number_meters_index,
	_hud_number_kilometers_index,
};

enum
{
	_hud_meter_switch_color_on_state_change = 0,
	_hud_meter_interpolates_between_min_max_bit,
	_hud_meter_interpolate_in_hsv_space_bit,
	_hud_meter_interpolate_along_farthest_hue_path_bit,
	_hud_meter_invert_interpolation_value_bit,
	NUMBER_OF_HUD_METER_FLAGS,
};

enum
{
	_hud_multitexture_overlay_effector_type_tint = 0,
	_hud_multitexture_overlay_effector_type_horizontal_offset,
	_hud_multitexture_overlay_effector_type_vertical_offset,
	_hud_multitexture_overlay_effector_type_alpha,
	NUMBER_OF_HUD_MULTITEXTURE_OVERLAY_EFFECTOR_TYPES,
};

enum
{
	_hud_multitexture_overlay_effector_destination_geometry_offset = 0,
	_hud_multitexture_overlay_effector_destination_primary_map,
	_hud_multitexture_overlay_effector_destination_secondary_map,
	_hud_multitexture_overlay_effector_destination_tertiary_map,
	NUMBER_OF_HUD_MULTITEXTURE_OVERLAY_EFFECTOR_DESTINATIONS,
};

enum
{
	_hud_multitexture_overlay_effector_source_player_pitch = 0,
	_hud_multitexture_overlay_effector_source_player_pitch_tangent,
	_hud_multitexture_overlay_effector_source_player_yaw,
	_hud_multitexture_overlay_effector_source_weapon_ammo_loaded,
	_hud_multitexture_overlay_effector_source_weapon_ammo_total,
	_hud_multitexture_overlay_effector_source_weapon_heat,
	_hud_multitexture_overlay_effector_source_explicit,
	_hud_multitexture_overlay_effector_source_zoom_level,
	NUMBER_OF_HUD_MULTITEXTURE_OVERLAY_EFFECTOR_SOURCES,
};

enum
{
	_hud_multitexture_overlay_hud_element_blend_function_add = 0,
	_hud_multitexture_overlay_hud_element_blend_function_subtract,
	_hud_multitexture_overlay_hud_element_blend_function_multiply,
	_hud_multitexture_overlay_hud_element_blend_function_multiply2x,
	_hud_multitexture_overlay_hud_element_blend_function_dot,
	NUMBER_OF_HUD_MULTITEXTURE_OVERLAY_BLEND_MODES,
};

/* ---------- macros */

#define hud_globals_definition_get(index) ((struct hud_globals_definition *)tag_get(HUD_GLOBALS_DEFINITION_TAG, (index))) /* fake name */
#define hud_number_definition_get(index) ((struct hud_number_definition *)tag_get(HUD_NUMBER_DEFINITION_TAG, (index))) /* fake name */

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
	word flash_flags;
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
	struct tag_block multitexture_overlays;		// multitexture_overlay_hud_element_definition
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
	struct tag_block multitexture_overlays;		// multitexture_overlay_hud_element_definition
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

struct icon_hud_element_definition
{
	short sequence_index;
	short width_offset;
	point2d offset;
	unsigned long color;
	char frame_rate;
	byte flags;
	short text_index;
};

struct hud_number_definition
{
	struct tag_reference number_bitmap;	// bitmap_group
	char character_width;
	char screen_width;
	char x_offset;
	char y_offset;
	char decimal_point_width;
	char colon_width;
	word pad;
	long unused[19];
};

struct multitexture_overlay_hud_element_effector_definition
{
	long unused1[16];
	short destination_type;
	short destination;
	short source;
	word pad1;
	real in_bounds[2];
	real out_bounds[2];
	long unused2[16];
	real_rgb_color tint_color_lower_bounds;
	real_rgb_color tint_color_upper_bounds;
	short periodic_function;
	word pad2;
	real periodic_function_period;
	real periodic_function_phase;
	long unused3[8];
};

struct multitexture_overlay_hud_element_definition
{
	word flags;
	short type;
	short framebuffer_blend_function;
	word pad1;
	long unused1[8];
	word map_flags[3];
	short map_blending_function[2];
	word pad2;
	real_vector2d map_scale[3];
	real_vector2d map_offset[3];
	struct tag_reference map[3];	// bitmap_group
	short map_clamp[3];
	word pad3;
	long unused2[46];
	struct tag_block functions;		// multitexture_overlay_hud_element_effector_definition
	long unused3[32];
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
	struct tag_block arrows;		// hud_waypoint_arrow
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
	struct tag_block button_icons;		// icon_hud_element_definition
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

/* ---------- globals */

extern char const *global_hud_anchor_names[NUMBER_OF_HUD_ANCHORS];

#endif // __HUD_DEFINITIONS_H
