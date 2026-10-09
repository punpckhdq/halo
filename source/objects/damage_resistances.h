/*
DAMAGE_RESISTANCES.H

header included in hcex build.
*/

#ifndef __DAMAGE_RESISTANCES_H
#define __DAMAGE_RESISTANCES_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	_damage_part_gut = 0,
	_damage_part_chest,
	_damage_part_head,
	_damage_part_left_shoulder,
	_damage_part_left_arm,
	_damage_part_left_leg,
	_damage_part_left_foot,
	_damage_part_right_shoulder,
	_damage_part_right_arm,
	_damage_part_right_leg,
	_damage_part_right_foot,
	NUMBER_OF_DAMAGE_PARTS,
};

enum
{
	_damage_resistance_takes_shield_damage_for_children_bit = 0,
	_damage_resistance_takes_body_damage_for_children_bit,
	_damage_resistance_always_shields_friendly_damage_bit,
	_damage_resistance_children_take_area_damage_bit,
	_damage_resistance_parent_never_takes_body_damage_for_us_bit,
	_damage_resistance_only_hurt_by_explosives_bit,
	_damage_resistance_only_hurt_while_occupied_bit,
	NUMBER_OF_DAMAGE_RESISTANCE_FLAGS,
};

enum
{
	_damage_material_head_bit = 0,
	NUMBER_OF_DAMAGE_MATERIAL_FLAGS,
};

enum
{
	_object_region_lives_until_object_dies_bit = 0,
	_object_region_forces_object_to_die_bit,
	_object_region_dies_when_object_dies_bit,
	_object_region_dies_when_object_is_damaged_bit,
	_object_region_missing_when_shield_is_zero_bit,
	_object_region_inhibits_melee_attack_bit,
	_object_region_inhibits_ranged_attack_bit,
	_object_region_inhibits_walking_bit,
	_object_region_forces_drop_weapon_bit,
	_object_region_head_destroyed_scream_bit,
	NUMBER_OF_DAMAGE_REGION_FLAGS,
};

/* ---------- macros */

/* ---------- structures */

struct damage_material
{
	char name[32];
	unsigned long flags;
	short type;
	word pad;
	real shield_leak_fraction;
	real shield_damage_multiplier;
	real shield_unused[3];
	real body_damage_multiplier;
	long body_unused[2];
};

struct damage_region
{
	char name[TAG_STRING_LENGTH+1];
	unsigned long flags;
	long pad;
	real damage_threshold;
	long unused[3];
	struct tag_reference destroyed_effect;
	struct tag_block permutations;
};

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
	struct tag_block materials;		// damage_material
	struct tag_block regions;		// damage_region
	struct tag_block modifiers;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __DAMAGE_RESISTANCES_H
