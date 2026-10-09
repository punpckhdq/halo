/*
BIPEDS.H

header included in hcex build.
*/

#ifndef __BIPEDS_H
#define __BIPEDS_H
#pragma once

/* ---------- headers */

#include "units.h"
#include "biped_definitions.h"

/* ---------- constants */

enum
{
	_biped_airborne_bit = 0,
	_biped_slipping_bit,
	_biped_absolute_movement_bit,
	_biped_no_collision_bit,
	_biped_movement_passes_through_bipeds_bit,
	_biped_limp_body_physics_active_bit,
	NUMBER_OF_BIPED_FLAGS,
};

/* ---------- macros */

#define biped_get(index)			((struct biped_datum *)object_get_and_verify_type((index), _object_mask_biped))
#define biped_try_and_get(index)	((struct biped_datum *)object_try_and_get_and_verify_type((index), _object_mask_biped))

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

void bipeds_initialize(void);
void bipeds_dispose(void);
void bipeds_initialize_for_new_map(void);
void bipeds_dispose_from_old_map(void);

void biped_adjust_placement(long object_index, struct object_placement_data *data);
boolean biped_new(long biped_index);
void biped_place(long biped_index, struct scenario_biped_datum *scenario_biped);
void biped_delete(long biped_index);
boolean biped_update(long biped_index);
void biped_export_function_values(long biped_index);
void biped_preprocess_node_orientations(long biped_index, struct real_orientation *node_orientations);
void biped_reset(long biped_index);
void biped_disconnect_from_structure_bsp(long biped_index);
void biped_render_debug(long biped_index);

void biped_get_sight_position(
	long biped_index,
	short estimate_mode,
	real_point3d const *estimated_body_position,
	real_vector3d *desired_facing,
	real_vector3d const *desired_gun_offset,
	real_point3d *sight_position);
void biped_get_physics_pill(long biped_index, real_point3d *base, real *height, real *width);
void biped_accelerate(long biped_index, real_vector3d *acceleration);

void biped_stop_limp_body_physics(long biped_index);
long biped_find_pathfinding_surface_index(long biped_index, real_point3d *pathfinding_point);

void biped_build_flying_axes(real_vector3d const *forward_vector, real_vector3d *left_vector, real_vector3d *up_vector);

/* ---------- globals */

/* ---------- public code */

#endif // __BIPEDS_H
