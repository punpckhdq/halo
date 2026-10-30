/*
AI.H

header included in hcex build.
*/

#ifndef __AI_H
#define __AI_H
#pragma once

/* ---------- constants */

enum
{
	_ai_line_of_sight_clear = 0,
	_ai_line_of_sight_occluded,
	_ai_line_of_sight_from_cover,
	_ai_line_of_sight_to_cover,
	_ai_line_of_sight_obstructed,
	NUMBER_OF_AI_LINE_OF_SIGHTS,
};

enum
{
	_ai_unit_effect_bump = 0,
	_ai_unit_effect_shooting,
	_ai_unit_effect_death_scream,
	_ai_unit_effect_magic_sight,
	NUMBER_OF_AI_UNIT_EFFECTS,
};

enum
{
	_ai_sound_volume_silent = 0,
	_ai_sound_volume_medium,
	_ai_sound_volume_loud,
	_ai_sound_volume_shout,
	_ai_sound_volume_quiet,
	NUMBER_OF_AI_SOUND_VOLUMES,
};

enum
{
	_ai_information_none = 0,
	_ai_information_allegiance,
	_ai_information_combat_stimulus,
	_ai_information_target_knowledge,
	_ai_information_flee,
	NUMBER_OF_AI_INFORMATION_TYPES,
};

enum
{
	_ai_information_look_none = 0,
	_ai_information_look_unit,
	_ai_information_look_object,
};

/* ---------- macros */

/* ---------- structures */

struct ai_unit_information_look_data
{
	long unit_index;
};

struct ai_object_information_look_data
{
	long object_index;
};

struct ai_information_look_data
{
	union
	{
		struct ai_unit_information_look_data unit;
		struct ai_object_information_look_data object;
	};
};

struct ai_allegiance_information_data
{
	short team1_index;
	short team2_index;
	boolean broken;
};

struct ai_combat_stimulus_information_data
{
	word pad;
};

struct ai_target_knowledge_information_data
{
	long prop_index;
};

struct ai_information_data
{
	union
	{
		struct ai_allegiance_information_data allegiance;
		struct ai_combat_stimulus_information_data combat_stimulus;
		struct ai_target_knowledge_information_data target_knowledge;
	};
};


struct ai_information_packet
{
	long target_unit_index;
	short communication_type;
	short dialogue_type_index;
	short damage_category;
	boolean updated_dialogue_timers;
	short look_priority;
	short look_type;
	struct ai_information_look_data look_data;
	short information_type;
	struct ai_information_data information_data;
};

/* ---------- prototypes/AI.C */

void ai_initialize(void);
void ai_dispose(void);
void ai_initialize_for_new_map(void);
void ai_dispose_from_old_map(void);
void ai_place(void);
void ai_update(void);
void ai_globals_ai_active(boolean enable);
void ai_globals_grenades_enabled(boolean enable);
void ai_get_major_upgrade_chance(short upgrade_type, boolean *force_major, boolean *is_random, real *random_chance);
boolean ai_consider_major_upgrade(long encounter_index, short squad_index, real upgrade_chance);
void ai_erase(long encounter_index, long platoon_index, long squad_index, boolean immediate);

boolean ai_release_inactive_swarms(char *result_description, unsigned char *more_to_release, unsigned char *working_memory, short working_memory_size);
void ai_find_inactive_encounters(unsigned char *working_memory, short working_memory_size);
boolean ai_release_inactive_encounters(char *result_description, unsigned char *more_to_release, unsigned char *working_memory, short working_memory_size);

boolean ai_test_line_of_fire(long actor_index, long ignore_unit_index, real_point3d const *origin, real_vector3d const *vector, long *prop_index_reference);
short ai_test_line_of_sight(real_point3d const *p0, short p0_cluster_index, real_point3d const *p1, short p1_cluster_index, short mode, boolean test_line_of_fire, long ignore_object_index, boolean ignore_vehicles);
boolean ai_test_ballistic_line_of_fire(long actor_index, real_point3d const *origin, real arc_time, real_vector3d const *arc_initial_velocity, real arc_acceleration, long ignore_object_index, boolean ignore_vehicles);
boolean ai_try_vehicle_eviction(long actor_index, long requesting_unit_index, boolean actually_evict);
long ai_get_responsible_unit(long object_index, boolean responsible_for_weapon_fire);

void ai_handle_allegiance_status_changed(short team1_index, short team2_index, boolean broken, boolean permanently_broken);
void ai_handle_allegiance_broken_notification(short team1_index, short team2_index, boolean broken);
void ai_update_team_status(void);

boolean ai_adjust_damage(long actor_index, struct damage_data *damage_data, real *damage);
void ai_handle_death(long unit_index, long owner_object_index, short damage_category);
boolean ai_handle_killing_spree(long unit_index, short killing_spree_count);
void ai_handle_bump(long biped_index, long object_index, real_vector3d const *old_velocity);
void ai_handle_damage(long unit_index, long shooter_object_index, short damage_category, float fraction, union real_vector3d *damage_velocity, boolean delayed);
void ai_create_mounted_weapons_for_unit(long unit_index);
void ai_handle_deleted_object(long deleted_object_index);
void ai_handle_editing(long encounter_index);
boolean ai_enemies_can_see_player(void);
boolean ai_enemies_attacking_player(void);
void ai_handle_unit_effect(long owner_unit_index, short effect_type, short volume);
void ai_handle_enter_vehicle(long unit_index, long vehicle_index);
void ai_handle_exit_vehicle(long unit_index, long vehicle_index);
void ai_handle_spatial_effect(long object_index, real_point3d const *position, short effect_type, short volume, short count);

void ai_globals_dialogue_triggers_enabled(boolean enable);
void ai_disconnect_from_structure_bsp(void);
void ai_reconnect_to_structure_bsp(void);
short ai_get_race_from_team_index(short team_index);

/* ---------- globals */

extern struct enum_definition ai_sound_volume_enum;

/* ---------- public code */

#endif // __AI_H
