/*
DAMAGE.H

header included in hcex build.
*/

#ifndef __DAMAGE_H
#define __DAMAGE_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	_damage_area_of_effect_bit = 0,
	_damage_create_localized_effect_bit,
	_damage_kill_instantly_bit,
	_damage_from_weapon_bit,
	_damage_silent_bit,
	_damage_bypasses_shields_bit,
	_damage_damaged_one_object_bit,
	_damage_no_statistics_bit,
	NUMBER_OF_DAMAGE_DATA_FLAGS,
};

enum
{
	_object_being_damaged_body_depleted_bit = 0,
	_object_being_damaged_region_destroyed_bit,
	_object_being_damaged_body_destroyed_bit,
	_object_being_damaged_shield_depleted_bit,
	_object_being_damaged_by_friendly_bit,
	_object_being_damaged_multiplied_by_difficulty_bit,
	_object_being_damaged_killed_instantly_bit,
	_object_being_damaged_force_hard_ping_bit,
	NUMBER_OF_OBJECT_BEING_DAMAGED_FLAGS,
};

/* ---------- macros */

/* ---------- structures */

struct damage_data
{
	long definition_index;
	unsigned long flags;
	long owner_player_index;
	long owner_object_index;
	short owner_team_index;
	struct location location;
	real_point3d origin;
	real_point3d epicenter;
	real_vector3d direction;
	real scale;
	real multiplier;
	real material_effect_scale;
	short material_type;
	struct projectile_material_response_definition const *material_response;
};

/* ---------- prototypes/DAMAGE.C */

void damage_initialize(void);
void damage_dispose(void);
void damage_initialize_for_new_map(void);
void damage_dispose_from_old_map(void);
void damage_render_debug(void);
void object_initialize_vitality(long object_index, real *custom_body_vitality, real *custom_shield_vitality);
real object_get_actual_body_vitality(long object_index, boolean ignore_difficulty);
real object_get_actual_shield_vitality(long object_index, boolean ignore_difficulty);
real object_get_maximum_body_vitality(long object_index, boolean ignore_difficulty);
real object_get_maximum_shield_vitality(long object_index, boolean ignore_difficulty);
void object_damage_update(long object_index);
void damage_data_new(struct damage_data *damage_data, long definition_index);
boolean object_restore_body(long object_index);
void object_deplete_body(long object_index);
void object_deplete_shield(long object_index);
boolean object_double_charge_shield(long object_index);
void object_destroy(long object_index);
void area_of_effect_cause_damage(struct damage_data *damage_data, long unlucky_object_index);
void object_cause_damage(struct damage_data *damage_data, long object_index, short node_index, short region_index, short material_index, real_vector3d const *object_normal);
void object_can_take_damage(long object_list_index);
void object_cannot_take_damage(long object_list_index);
void object_set_ranged_attack_inhibited(long object_index, boolean inhibited);
void object_set_melee_attack_inhibited(long object_index, boolean inhibited);
void render_debug_object_damage(void);

/* ---------- globals */

extern boolean debug_damage;

/* ---------- public code */

#endif // __DAMAGE_H
