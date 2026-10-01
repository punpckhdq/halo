/*
AI_GLOBALS.H

header included in hcex build.
*/

#ifndef __AI_GLOBALS_H
#define __AI_GLOBALS_H
#pragma once

/* ---------- constants */

enum
{
	_ai_spatial_effect_environmental_noise = 0,
	_ai_spatial_effect_weapon_impact,
	_ai_spatial_effect_weapon_detonation,
	NUMBER_OF_AI_SPATIAL_EFFECTS,
};

/* ---------- macros */

#define MAXIMUM_ENTERABLE_VEHICLES 32

/* ---------- structures */

struct recent_conversation
{
	short definition_index;
	boolean unable_to_begin;
	boolean finished_successfully;
	long finish_time;
	unsigned long unused[2];
};

struct ai_spatial_effect
{
	short type;
	short count;
	real_point3d position;
	long last_tick;
};

struct ai_vehicle_enterable
{
	long vehicle_index;
	real radius;
	short team_bitmask;
	short actor_type_bitmask;
	short ai_indices_count;
	long ai_indices[6];
};

struct ai_globals
{
	boolean ai_active;
	boolean ai_initialized_for_map;
	boolean ai_has_control_data;
	boolean time_given_this_frame;
	short last_highest_service_timer;
	short current_highest_service_timer;
	long first_encounterless_actor_index;
	real major_upgrade_error;
	boolean dialogue_triggers_enabled;
	long last_chatter_time[2];
	long last_talk_time[2];
	long last_shout_time[2];
	short recent_conversation_count;
	short recent_conversation_next_index;
	struct recent_conversation recent_conversations[16];
	short spatial_effects_first_index;
	short spatial_effects_last_index;
	struct ai_spatial_effect spatial_effects[32];
	boolean grenades_enabled;
	short enterable_vehicle_count;
	struct ai_vehicle_enterable enterable_vehicles[MAXIMUM_ENTERABLE_VEHICLES];
	short mounted_weapon_unit_count;
	long mounted_weapon_unit_indices[8];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

extern struct ai_globals *ai_globals;

/* ---------- public code */

#endif // __AI_GLOBALS_H
