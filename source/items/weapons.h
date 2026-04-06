/*
WEAPONS.H

header included in hcex build.
*/

#ifndef __WEAPONS_H
#define __WEAPONS_H
#pragma once

/* ---------- headers */

#include "items.h"

/* ---------- constants */

/* ---------- macros */

#define weapon_get(index)			((struct weapon_datum*)object_get_and_verify_type(index, _object_mask_weapon))
#define weapon_try_and_get(index)	((struct weapon_datum*)object_try_and_get_and_verify_type(index, _object_mask_weapon))

/* ---------- structures */

struct weapon_trigger
{
	char idle_ticks;
	char state;
	short state_timer;
	unsigned long flags;
	word firing_effects_used_flags;
	short firing_effect_index;
	short firing_effect_shots_remaining;
	short sequential_non_tracer_rounds;
	real rate_of_fire;
	real ejection_port_position;
	real illumination;
	real __noop;
	long charging_effect_index;
};

struct weapon_magazine
{
	short state;
	short state_timer;
	short original_time;
	short rounds_total;
	short rounds_loaded;
	short rounds_fractional_recharged;
};

struct _weapon_datum
{
	unsigned long flags;
	word control_flags;
	real primary_trigger;
	char state;
	char last_reported_state;
	short state_timer;
	real heat;
	real age;
	real overcharged;
	real integrated_light_power;
	char integrated_light_delay_ticks;
	long tracked_object_index;
	real recoil_angular_velocity;
	short recoil_recovery_time;
	short shots_until_demotion;
	short alternate_shots_loaded;
	struct weapon_trigger triggers[2];
	struct weapon_magazine magazines[2];
	struct animation_state animation;
	long overheated_effect_index;
	long game_time_last_fired;
};

struct weapon_datum
{
	long definition_index;
	struct _object_datum object;
	struct _item_datum item;
	struct _weapon_datum weapon;
};

/* ---------- prototypes/WEAPONS.C */

boolean weapon_is_flag(long weapon_index);

/* ---------- globals */

/* ---------- public code */

#endif // __WEAPONS_H
