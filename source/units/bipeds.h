/*
BIPEDS.H

header included in hcex build.
*/

#ifndef __BIPEDS_H
#define __BIPEDS_H
#pragma once

/* ---------- headers */

#include "units.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct _biped_datum
{
	unsigned long flags;
	char landing_recovery_counter;
	char landing_recovery_time;
	char state;
	char elevator_ticks;
	long elevator_object_index;
	long support_surface_index;
	long pathfinding_surface_index;
	real_point3d pathfinding_point;
	long last_pathfinding_attempt_time;
	long last_pathfinding_surface_index;
	long impact_target_object_index;
	long last_falling_communication_time;
	long bump_object_index;
	char bump_ticks;
	char airborne_ticks;
	char slipping_ticks;
	char stop_ticks;
	char jump_recovery_timer;
	char player_melee_ticks;
	char player_melee_attack_tick;
	short landing;
	real crouch;
	real bank;
	real_plane3d ground_plane;
	byte limp_body_current_relaxation_iterations;
	byte limp_body_max_relaxation_iterations;
};

struct biped_datum
{
	long definition_index;
	struct _object_datum object;
	struct _unit_datum unit;
	struct _biped_datum biped;
};

/* ---------- prototypes/BIPEDS.C */

void biped_stop_limp_body_physics(long biped_index);

void biped_get_sight_position(long biped_index, short estimate_mode, real_point3d const *estimated_body_position, real_vector3d *desired_facing, real_vector3d const *desired_gun_offset, real_point3d *sight_position);

/* ---------- globals */

/* ---------- public code */

#endif // __BIPEDS_H
