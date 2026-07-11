/*
ENCOUNTERS.H

file has inline function assertions.
*/

#ifndef __ENCOUNTERS_H
#define __ENCOUNTERS_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"

/* ---------- constants */

#define MAXIMUM_SQUADS_PER_ENCOUNTER 64
#define MAXIMUM_SQUADS_PER_MAP 1024

#define MAXIMUM_PLATOONS_PER_ENCOUNTER 32
#define MAXIMUM_PLATOONS_PER_MAP 256

/* ---------- macros */

#define encounter_get(index)			((struct encounter_datum *)datum_get(encounter_data, (index)))
#define encounter_try_and_get(index)	((struct encounter_datum *)datum_try_and_get(encounter_data, (index)))

/* ---------- structures */

struct encounter_datum
{
	short identifier;
	short team_index;
	short squad_base;
	short squad_count;
	short platoon_base;
	short platoon_count;
	boolean force_active;
	boolean active;
	short remain_active_timer;
	long last_active_time;
	long first_actor_index;
	short original_count;
	short prebattle_living_count;
	short unique_leader_count;
	boolean is_prevehicle_encounter;
	short link_encounter_count;
	short link_encounter_indices[3];
	boolean status_dirty;
	short current_count;
	short current_swarm_count;
	short current_in_combat_count;
	short current_fighting_count;
	real current_strength_fraction;
	long first_pursuit_index;
	boolean respawn_enabled;
	short respawn_delay_ticks;
	boolean blind;
	boolean deaf;
	boolean stand_down;
	boolean enemy_target;
	boolean enemy_alive;
	boolean enemy_visible;
	boolean enemy_traitor;
	boolean post_combat;
	boolean post_combat_delay;
	short post_combat_delay_timer;
	short enemies_defeated;
	long enemy_visible_timer;
	long enemy_alive_timer;
	long corpse_ignore_time;
	long last_grenade_throw_time;
	boolean playfighting;
	short follow_target_type;
	
	union
	{
		long follow_target_unit_index;
		long follow_target_ai_index;
	};

	real follow_target_distance;
};

struct squad_datum
{
	unsigned long required_locations[1];
	unsigned long unused_locations[1];
	real major_upgrade_error;
	short respawn_actors_left;
	short respawn_delay_ticks;
	boolean automatic_migration_target;
	boolean delay_timer_started;
	short delay_timer;
	boolean disable_dormant;
	short original_count;
	short current_count;
	short current_swarm_count;
	real current_strength_fraction;
};

struct platoon_datum
{
	boolean defending;
	boolean maneuvering;
	boolean maneuver_disable;
	short original_count;
	short current_count;
	short current_swarm_count;
	real current_strength_fraction;
};

/* ---------- prototypes/ENCOUNTERS.C */

void encounter_compute_activation_cluster_bit_vector(long encounter_index, boolean update_actor_dormancy, long bit_vector_size, unsigned long const *active_area, unsigned long *bit_vector);

/* ---------- globals */

extern struct data_array *encounter_data;
extern struct platoon_datum *platoon_array;
extern struct squad_datum *squad_array;
extern struct data_array *pursuit_data;

/* ---------- public code */

__inline struct squad_datum *encounter_get_squad(
	struct encounter_datum *encounter,
	short squad_index)
{
	short squad_absolute_index;

	match_assert("c:\\halo\\source\\ai\\encounters.h", 220, squad_index>=0 && squad_index<MAXIMUM_SQUADS_PER_ENCOUNTER && squad_index<encounter->squad_count);

	squad_absolute_index = squad_index + encounter->squad_base;
	match_assert("c:\\halo\\source\\ai\\encounters.h", 223, squad_absolute_index>=0 && squad_absolute_index<MAXIMUM_SQUADS_PER_MAP);

	return &squad_array[squad_absolute_index];
}

__inline struct platoon_datum *encounter_get_platoon(
	struct encounter_datum *encounter,
	short platoon_index)
{
	short platoon_absolute_index;

	match_assert("c:\\halo\\source\\ai\\encounters.h", 234, platoon_index>=0 && platoon_index<MAXIMUM_PLATOONS_PER_ENCOUNTER && platoon_index<encounter->platoon_count);

	platoon_absolute_index = platoon_index + encounter->platoon_base;
	match_assert("c:\\halo\\source\\ai\\encounters.h", 237, platoon_absolute_index>=0 && platoon_absolute_index<MAXIMUM_SQUADS_PER_MAP);

	return &platoon_array[platoon_absolute_index];
}

#endif // __ENCOUNTERS_H
