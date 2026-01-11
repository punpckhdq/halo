/*
DAMAGE_RESISTANCES.H

header included in hcex build.
*/

#ifndef __DAMAGE_RESISTANCES_H
#define __DAMAGE_RESISTANCES_H
#pragma once

/* ---------- headers */

#include "tag_files/tag_groups.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct damage_resistance
{
	unsigned long flags;
	short indirect_damage_material_index;
	word pad2;
	real maximum_body_vitality;
	real body_system_shock;
	long body_vitality_unused[6];
	real body_stun_unused[7];
	real friendly_damage_resistance;
	long friendly_damage_unused[2];
	long body_unused[8];
	struct tag_reference localized_damage_effect;
	real area_damage_effect_threshold;
	struct tag_reference area_damage_effect;
	real body_damaged_effect_threshold;
	struct tag_reference body_damaged_effect;
	struct tag_reference body_depleted_effect;
	real body_destroyed_threshold;
	struct tag_reference body_destroyed_effect;
	real maximum_shield_vitality;
	word pad0;
	short shield_material_type;
	long shield_vitality_unused[6];
	short shield_failure_function;
	word pad1;
	real shield_failure_threshold;
	real maximum_shield_failure;
	long shield_failure_unused[4];
	real minimum_shield_stun_damage;
	real shield_stun_time;
	real shield_recharge_time;
	real shield_recharge_unused[4];
	long shield_unused[24];
	real shield_damaged_effect_threshold;
	struct tag_reference shield_damaged_effect;
	struct tag_reference shield_depleted_effect;
	struct tag_reference shield_recharging_effect;
	unsigned long unused2[2];
	real runtime_shield_recharge_velocity;
	unsigned long unused[28];
	struct tag_block materials;
	struct tag_block regions;
	struct tag_block modifiers;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __DAMAGE_RESISTANCES_H
