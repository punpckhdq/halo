/*
PHYSICS.H

header included in hcex build.
*/

#ifndef __PHYSICS_H
#define __PHYSICS_H
#pragma once

/* ---------- headers */

#include "physics_definitions.h"

/* ---------- constants */

enum
{
	_point_at_rest_bit = 0,
	_point_on_ground_bit,
	_point_on_volatile_surface_bit,
	_point_in_water_bit,
	_point_antigraving_bit,
	NUMBER_OF_POINT_FLAGS,
};

/* ---------- macros */

#define GRAVITY 9.78f /* fake name */

/* ---------- structures */

struct friction_datum
{
	real_vector3d friction;
	real_vector3d parallel;
	real_vector3d perpendicular;
};

struct powered_mass_point_datum
{
	real ground_friction_velocity;
	real water_friction_velocity;
	real air_friction_velocity;
	real water_lift_ratio;
	real air_lift_ratio;
	real thrust_fraction;
	real antigrav_fraction;
	real_quaternion rotation;
	real_matrix4x3 rotation_matrix;
};

struct mass_point_datum
{
	unsigned long flags;
	real_point3d position;
	real_vector3d forward;
	real_vector3d left;
	real_vector3d up;
	struct location location;
	real_vector3d radius;
	real_vector3d velocity;
	real_vector3d velocity_relative_to_ground;
	real_plane3d ground_plane;
	short ground_material_type;
	real ground_depth;
	short water_material_type;
	real water_depth;
	real normal_force_magnitude;
	real_vector3d normal_force;
	struct friction_datum ground_friction;
	real water_pressure_magnitude;
	real_vector3d water_pressure;
	struct friction_datum water_friction;
	struct friction_datum air_friction;
	real_vector3d powered_force;
	real_vector3d force;
	real_vector3d torque;
};

struct physics_instance
{
	long object_index;
	const struct physics_definition *physics;
	real_matrix4x3 world_matrix;
};

struct physics_test_vector_result
{
	real t;
	real_plane3d plane;
};

/* ---------- prototypes/PHYSICS.C */

real pin_fraction(real value, real value0, real value1);
boolean physics_instance_new(struct physics_instance *instance, long object_index);
boolean physics_test_point(struct physics_instance const *instance, real_point3d const *point);
boolean physics_test_vector(struct physics_instance const *instance, real_point3d const *point, real_vector3d const *vector, struct physics_test_vector_result *result);
boolean physics_get_features_in_sphere(struct physics_instance const *instance, real_point3d const *center, real radius, real height, real width, struct collision_feature_list *features);
void physics_compute_new(struct physics_instance const *instance, struct powered_mass_point_datum const *powered_mass_points, struct mass_point_datum *mass_points, real_vector3d *total_force, real_vector3d *total_torque);
void physics_update_new(struct physics_instance const *instance, struct powered_mass_point_datum const *powered_mass_points, struct mass_point_datum const *mass_points, real_vector3d const *total_force, real_vector3d const *total_torque);
void physics_update(long object_index, struct powered_mass_point_datum *powered_mass_points, struct mass_point_datum *mass_points, real_vector3d const *magic_force, real_vector3d const *magic_torque);

void render_debug_physics(struct physics_instance *instance);


/* ---------- globals */

extern real global_gravity;
extern real global_water_density;
extern real global_air_density;
extern real global_physics_collision_depth;
extern real_plane3d depths_of_hell;
extern boolean debug_physics_disable_penetration_freeze;

/* ---------- public code */

#endif // __PHYSICS_H
