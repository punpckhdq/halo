/*
WEAPON_HUD_INTERFACE_DEFINITION.H

header included in hcex build.
*/

#ifndef __WEAPON_HUD_INTERFACE_DEFINITION_H
#define __WEAPON_HUD_INTERFACE_DEFINITION_H
#pragma once

/* ---------- headers */

#include "hud_definitions.h"

/* ---------- constants */

enum
{
	WEAPON_HUD_INTERFACE_TAG = 'wphi',
	WEAPON_HUD_INTERFACE_VERSION = 2,
	GRENADE_HUD_INTERFACE_TAG = 'grhi',
	GRENADE_HUD_INTERFACE_VERSION = 1,
	MAXIMUM_WEAPON_HUD_DEPTH = 16,
};

enum
{
	_grenade_overlay_on_flashing_bit = 0,
	_grenade_overlay_on_empty_bit,
	_grenade_overlay_on_default_bit,
	_grenade_overlay_on_always_bit,
	NUMBER_OF_GRENADE_OVERLAY_FLAGS,
};

enum
{
	_weapon_overlay_on_flashing_bit = 0,
	_weapon_overlay_on_empty_bit,
	_weapon_overlay_on_reload_bit,
	_weapon_overlay_on_default_bit,
	_weapon_overlay_on_always_bit,
	NUMBER_OF_WEAPON_OVERLAY_TYPE_FLAGS,
};

enum
{
	_weapon_state_total_ammo = 0,
	_weapon_state_loaded_ammo,
	_weapon_state_heat,
	_weapon_state_age,
	_weapon_state_secondary_total_ammo,
	_weapon_state_secondary_loaded_ammo,
	_weapon_state_distance_to_target,
	_weapon_state_elevation_to_target,
	NUMBER_OF_WEAPON_HUD_STATES,
};

enum
{
	_crosshair_state_aim = 0,
	_crosshair_state_zoom,
	_crosshair_state_charge,
	_crosshair_state_flash_ammo,
	_crosshair_state_flash_heat,
	_crosshair_state_flash_total_ammo,
	_crosshair_state_flash_total_battery,
	_crosshair_state_reload,
	_crosshair_state_fired_with_no_ammo,
	_crosshair_state_threw_with_no_grenade,
	_crosshair_state_flash_ammo_none_for_reload,
	_crosshair_state_flash_secondary_ammo,
	_crosshair_state_flash_secondary_total_ammo,
	_crosshair_state_secondary_reload,
	_crosshair_state_fired_secondary_with_no_ammo,
	_crosshair_state_flash_secondary_ammo_none_for_reload,
	_crosshair_state_primary_trigger_ready,
	_crosshair_state_secondary_trigger_ready,
	_crosshair_state_flash_fired_battery_depleted,
	NUMBER_OF_CROSSHAIR_STATES,
};

enum
{
	_hud_element_runtime_invalid_bit = 0,
	NUMBER_OF_HUD_RUNTIME_FLAGS,
};

enum
{
	_hud_use_parent_flash_parameters_bit = 0,
	NUMBER_OF_WEAPON_HUD_FLASHING_FLAGS,
};

enum
{
	_weapon_number_divide_by_clip_size_bit = 0,
	NUMBER_OF_WEAPON_NUMBER_FLAGS,
};

enum
{
	_hud_use_on_any_map_type = 0,
	_hud_use_on_solo_map,
	_hud_use_on_multiplayer_map,
	NUMBER_OF_HUD_USE_ON_MAP_TYPES,
};

enum
{
	_hud_overlay_flashes_bit = 0,
	NUMBER_OF_HUD_WEAPON_OVERLAY_FLAGS,
};

enum
{
	_hud_overlay_runtime_invalid_bit = NUMBER_OF_HUD_WEAPON_OVERLAY_FLAGS,
	NUMBER_OF_HUD_WEAPON_OVERLAY_RUNTIME_FLAGS,
};

enum
{
	_hud_crosshair_flashes_bit = 0,
	_hud_crosshair_not_a_sprite_bit,
	_hud_crosshair_not_on_default_zoom_bit,
	_hud_crosshair_show_sniper_data_bit,
	_hud_crosshair_hide_outside_area_bit,
	_hud_crosshair_one_zoom_level_bit,
	_hud_crosshair_only_on_default_zoom_bit,
	NUMBER_OF_HUD_WEAPON_CROSSHAIR_FLAGS,
};

enum
{
	_hud_crosshair_runtime_invalid_bit = NUMBER_OF_HUD_WEAPON_CROSSHAIR_FLAGS,
	NUMBER_OF_HUD_WEAPON_CROSSHAIR_RUNTIME_FLAGS,
};

/* ---------- macros */

#define weapon_hud_interface_definition_get(index) ((struct weapon_hud_interface_definition *)tag_get(WEAPON_HUD_INTERFACE_TAG, index))
#define grenade_hud_interface_definition_get(index) ((struct grenade_hud_interface_definition *)tag_get(GRENADE_HUD_INTERFACE_TAG, index))

/* ---------- structures */

struct weapon_flash_state_definition
{
	short flags;
	short pad;
	short total_ammo;
	short loaded_ammo;
	short heat;
	short age;
	long unused[8];
};

struct weapon_hud_element_header
{
	short state_type;
	short runtime_flags;
	short use_on_map_type;
	short pad;
	long unused[7];
};

struct weapon_hud_static_element
{
	struct weapon_hud_element_header header;
	struct static_hud_element_definition static_element;
	long unused[10];
};

struct weapon_hud_meter_element
{
	struct weapon_hud_element_header header;
	struct meter_hud_element_definition meter_element;
	long unused[10];
};

struct weapon_hud_number_element
{
	struct weapon_hud_element_header header;
	struct number_hud_element_definition number_element;
	word weapon_flags;
	short pad;
	long unused[9];
};

struct weapon_hud_crosshair_item
{
	struct hud_placement_definition placement;
	struct hud_color_definition colors;
	short frame_rate;
	short sequence_index;
	long flags;
	long unused[8];
};

struct weapon_hud_crosshair_definition
{
	struct tag_reference bitmap;
	struct tag_block items;		// weapon_hud_crosshair_item
};

struct weapon_hud_crosshairs_element
{
	short crosshair_type;
	short runtime_flags;
	short use_on_map_type;
	short pad;
	long unused[7];
	struct weapon_hud_crosshair_definition crosshairs;
	long unused2[10];
};

struct weapon_hud_overlay_item
{
	struct hud_placement_definition placement;
	struct hud_color_definition colors;
	short frame_rate;
	short pad;
	short sequence_index;
	short type;
	long flags;
	long unused[14];
};

struct weapon_hud_overlay_definition
{
	struct tag_reference bitmap;
	struct tag_block items;		// weapon_hud_overlay_item
};

struct weapon_hud_overlays_element
{
	short state_type;
	short runtime_flags;
	short use_on_map_type;
	short pad;
	long unused[7];
	struct weapon_hud_overlay_definition overlays;
	long unused2[10];
};

struct hud_screen_effect_definition
{
	long unused1[1];
	word mask_flags;
	word mask_pad;
	long mask_unused[4];
	struct tag_reference mask_fullscreen;
	struct tag_reference mask_splitscreen;
	long unused2[2];
	word convolution_flags;
	word convolution_pad;
	real convolution_radius_in_bounds[2];
	real convolution_radius_out_bounds[2];
	long unused3[6];
	word light_enhancement_flags;
	short light_enhancement_script_source;
	real light_enhancement_intensity;
	long unused4[6];
	word desaturation_flags;
	short desaturation_script_source;
	real desaturation_intensity;
	union real_rgb_color desaturation_tint;
	long unused5[6];
};

struct weapon_hud_interface_definition
{
	struct tag_reference parent_hud;
	struct weapon_flash_state_definition flash_cutoffs;
	struct hud_absolute_placement_definition absolute_placement;
	struct tag_block statics;			// weapon_hud_static_element
	struct tag_block meters;			// weapon_hud_meter_element
	struct tag_block numbers;			// weapon_hud_number_element
	struct tag_block crosshairs;		// weapon_hud_crosshairs_element
	struct tag_block overlays;			// weapon_hud_overlays_element
	unsigned long valid_crosshair_types_flags;
	struct tag_block warning_sounds;	// sound_hud_element_definition
	struct tag_block screen_effects;	// hud_screen_effect_definition
	long unused1[33];
	struct icon_hud_element_definition messaging_icon;
	long unused2[12];
};

struct grenade_count_panel_definition
{
	struct static_hud_element_definition background;
	struct number_hud_element_definition numbers;
	short flash_cutoff;
	short pad;
};

struct grenade_hud_interface_definition
{
	struct hud_absolute_placement_definition absolute_placement;
	struct static_hud_element_definition background;
	struct grenade_count_panel_definition grenade_count_panel;
	struct weapon_hud_overlay_definition overlays;
	struct tag_block warning_sounds;		// sound_hud_element_definition
	long unused0[17];
	struct tag_reference messaging_icon_bitmap;
	long unused2[12];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __WEAPON_HUD_INTERFACE_DEFINITION_H
