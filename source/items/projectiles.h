/*
PROJECTILES.H

header included in hcex build.
*/

#ifndef __PROJECTILES_H
#define __PROJECTILES_H
#pragma once

/* ---------- headers */

#include "items.h"

/* ---------- constants */

enum
{
	_projectile_has_nonzero_angular_velocity_bit = 0,
	_projectile_tracer_bit,
	_projectile_collided_once_bit,
	_projectile_attached_bit,
	_projectile_stopped_after_collision_bit,
	_projectile_counting_down_bit,
	_projectile_already_super_exploded_bit,
	_projectile_will_super_explode_bit,
	NUMBER_OF_PROJECTILE_DATUM_FLAGS,
};

enum
{
	_projectile_action_none = 0,
	_projectile_action_detonate,
	_projectile_action_disappear,
	NUMBER_OF_PROJECTILE_ACTIONS,
};

/* ---------- macros */

#define projectile_get(index)			((struct projectile_datum*)object_get_and_verify_type(index, _object_mask_projectile))
#define projectile_try_and_get(index)	((struct projectile_datum*)object_try_and_get_and_verify_type(index, _object_mask_projectile))

/* ---------- structures */

struct _projectile_datum
{
	unsigned long flags;
	short action;
	short hit_material_type;
	long ignore_object_index;
	long target_object_index;
	long tracer_attachment_index_index;
	real detonation_timer;
	real detonation_timer_delta;
	real arming_time;
	real arming_time_delta;
	real odometer;
	real deceleration_timer;
	real deceleration_timer_delta;
	real deceleration;
	real maximum_damage_distance;
	real_vector3d rotation_axis;
	real rotation_sine;
	real rotation_cosine;
};

struct projectile_datum
{
	long definition_index;
	struct _object_datum object;
	struct _item_datum item;
	struct _projectile_datum projectile;
};

/* ---------- prototypes/PROJECTILES.C */

void projectiles_initialize(void);
void projectiles_initialize_for_new_map(void);
void projectiles_dispose_from_old_map(void);
void projectiles_dispose(void);
void projectile_kill_tracer(long projectile_index);
void projectiles_delete_all(void);
boolean projectile_new(long projectile_index);
void projectile_delete(long projectile_index);
void projectile_set_target_object_index(long projectile_index, long target_object_index);
void projectile_make_tracer(long projectile_index);
boolean projectile_update(long projectile_index);
real projectile_get_ballistic_acceleration(struct projectile_definition const *projectile_definition);
boolean projectile_aim_ballistic(real base_velocity, real gravity_scale, real_point3d const *origin, real_point3d const *target_point, real *target_velocity_min, real *target_ballistic_fraction_min, real *forced_velocity, boolean lob, real_vector3d *result_aim_vector, real *result_velocity, real *result_ticks, real *result_distance, real *result_vertical_velocity, real *result_horizontal_velocity);
boolean projectile_aim_linear(real base_velocity, real_point3d const *origin, real_point3d const *target_point, real_vector3d *result_aim_vector, real *result_velocity, real *result_ticks, real *result_distance);
boolean projectile_aim(struct projectile_definition const *projectile_definition, real_point3d const *origin, real_point3d const *target_point, real *override_velocity_max, real *target_velocity_min, real *target_ballistic_fraction_min, real *forced_velocity, boolean lob, real_vector3d *result_aim_vector, real *result_velocity, real *result_ticks, real *result_distance, boolean *result_linear);
real projectile_estimate_time_to_target(struct projectile_definition const *projectile_definition, real target_distance);
void projectile_accelerate(long projectile_index, real_vector3d const *acceleration);
boolean dangerous_projectiles_near_player(void);
void projectile_handle_deleted_object(long projectile_index, long deleted_object_index);
void projectile_export_function_values(long projectile_index);
boolean projectile_handle_parent_destroyed(long projectile_index);

/* ---------- globals */

/* ---------- public code */

#endif // __PROJECTILES_H
