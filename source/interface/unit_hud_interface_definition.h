/*
UNIT_HUD_INTERFACE_DEFINITION.H
*/

#ifndef __UNIT_HUD_INTERFACE_DEFINITION_H
#define __UNIT_HUD_INTERFACE_DEFINITION_H
#pragma once

/* ---------- headers */

#include "hud_definitions.h"

/* ---------- constants */

enum
{
	UNIT_HUD_INTERFACE_TAG = 'unhi',
};

enum
{
	_unit_hud_shield_recharging = 0,
	_unit_hud_shield_damage,
	_unit_hud_shield_low,
	_unit_hud_shield_empty,
	_unit_hud_health_low,
	_unit_hud_health_empty,
	_unit_hud_minor_damage,
	_unit_hud_major_damage,
	NUMBER_OF_UNIT_HUD_STATES,
};

enum
{
	_auxilary_meter_integrated_light = 0,
	_auxilary_overlay_team = 0,
	_auxilary_overlay_use_team_color_bit = 0,
};

/* ---------- structures */

struct metered_panel_definition
{
	struct static_hud_element_definition background;
	struct meter_hud_element_definition meter;
	union
	{
		struct
		{
			unsigned long overcharge_min_color;
			unsigned long overcharge_max_color;
			unsigned long overcharge_flash_color;
			unsigned long overcharge_empty_color;
		} shield_extras;
		struct
		{
			unsigned long mid_color;
			real max_cutoff;
			real min_cutoff;
			long pad;
		} health_extras;
		struct
		{
			real min_cutoff;
			unsigned long flags;
			long pad[2];
		} aux_extras;
	};
	long unused[4];
};

struct auxilary_panel_definition
{
	struct hud_absolute_placement_definition absolute_placement;
	struct tag_block auxilary_overlays;
	long unused[4];
};

struct motion_sensor_panel_definition
{
	struct static_hud_element_definition background;
	struct static_hud_element_definition foreground;
	long unused[8];
};

struct auxilary_overlay_definition
{
	struct static_hud_element_definition static_element;
	short type;
	unsigned short flags;
	long unused0[6];
};

struct unit_hud_interface_definition
{
	struct hud_absolute_placement_definition absolute_placement;
	struct static_hud_element_definition background;
	struct metered_panel_definition shield_meter;
	struct metered_panel_definition health_meter;
	struct motion_sensor_panel_definition motion_sensor;
	struct hud_placement_definition blip_placement;
	struct auxilary_panel_definition auxilary_panel;
	struct tag_block warning_sounds;
	struct tag_block auxilary_meters;
	long unused1[89];
	long unused2[12];
};

struct auxilary_meter_definition
{
	short type;
	short pad;
	long unused0[4];
	struct metered_panel_definition panel;
	long unused1[16];
};

#endif // __UNIT_HUD_INTERFACE_DEFINITION_H
