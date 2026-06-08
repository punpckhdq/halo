/*
WEAPON_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __WEAPON_DEFINITIONS_H
#define __WEAPON_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "item_definitions.h"

#include "game/aim_assist.h"

/* ---------- constants */

enum
{
	WEAPON_DEFINITION_TAG = 'weap',
	WEAPON_DEFINITION_VERSION = 2,
};

enum
{
	_weapon_vertical_heat_display_bit = 0,
	_weapon_mutually_exclusive_triggers_bit,
	_weapon_attacks_on_bump_bit,
	_weapon_must_be_readied_bit,
	_weapon_doesnt_count_toward_maximum_bit,
	_weapon_aim_assists_only_when_zoomed_bit,
	_weapon_prevents_grenade_throwing_bit,
	_weapon_must_be_picked_up_bit,
	_weapon_holds_triggers_when_dropped_bit,
	_weapon_prevents_melee_attack_bit,
	_weapon_detonates_when_dropped_bit,
	_weapon_cannot_fire_at_maximum_age_bit,
	_weapon_secondary_trigger_overrides_grenades_bit,
	_weapon_does_not_depower_active_camo_bit,
	_weapon_enables_unit_integrated_night_vision_bit,
	_weapon_non_players_use_melee_damage_bit,
	NUMBER_OF_WEAPON_DEFINITION_FLAGS,
};

/* ---------- macros */

#define weapon_definition_get(index) ((struct weapon_definition *)tag_get(WEAPON_DEFINITION_TAG, index))

/* ---------- structures */

struct weapon_interface_definition
{
	struct tag_reference first_person_model;
	struct tag_reference first_person_animations;
	long unused[1];
	struct tag_reference hud_interface;
};

struct _weapon_definition
{
	unsigned long flags;
	char label[32];
	short secondary_trigger_mode;
	short maximum_alternate_shots_loaded;
	short function_modes[4];
	real ready_time;
	struct tag_reference ready_effect;
	real heat_recovery_threshold;
	real heat_overheated_threshold;
	real heat_detonation_threshold;
	real overheated_explosion_fraction;
	real heat_loss_per_second;
	real heat_illumination;
	real heat_unused[4];
	struct tag_reference overheated_effect;
	struct tag_reference detonation_effect;
	struct tag_reference melee_attack_damage;
	struct tag_reference melee_attack_response;
	unsigned long unused_sabine_women[2];
	struct tag_reference ai_firing_parameters;
	real near_reticle_range;
	real far_reticle_range;
	real intersection_reticle_range;
	word pad;
	short zoom_level_count;
	real zoom_magnification_minimum;
	real zoom_magnification_maximum;
	struct aim_assist_parameters aim_assist_parameters;
	short movement_penalty_mode;
	word pad1;
	real forward_movement_penalty;
	real sideways_movement_penalty;
	real properties_unused;
	real ai_minimum_target_range;
	real ai_idle_look_time_modifier;
	real ai_unused;
	real integrated_light_poweron_time;
	real integrated_light_poweroff_time;
	struct tag_reference integrated_light_on_effect;
	struct tag_reference integrated_light_off_effect;
	real age_heat_recovery_penalty;
	real age_rate_of_fire_penalty;
	real age_misfire_start;
	real age_misfire_chance;
	real age_unused[3];
	struct weapon_interface_definition interface_definition;
	struct tag_reference pickup_sound;
	struct tag_reference zoom_in_sound;
	struct tag_reference zoom_out_sound;
	struct tag_block unused_block;
	real active_camo_ding;
	real active_camo_regrowth_rate;
	long unused4[3];
	short unused5;
	short weapon_type;
	struct tag_block predicted_resources;
	struct tag_block magazines;
	struct tag_block triggers;
};

struct weapon_definition
{
	struct _object_definition object;
	struct _item_definition item;
	struct _weapon_definition weapon;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __WEAPON_DEFINITIONS_H
