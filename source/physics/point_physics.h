/*
POINT_PHYSICS.H

header included in hcex build.
*/

#ifndef __POINT_PHYSICS_H
#define __POINT_PHYSICS_H
#pragma once

/* ---------- headers */

#include "real_math.h"

/* ---------- constants */

enum
{
	POINT_PHYSICS_DEFINITION_TAG = 'pphy',
	POINT_PHYSICS_DEFINITION_VERSION = 1
};

enum
{
	_point_physics_flamethrower_collision_bit = 0,
	_point_physics_structure_collisions_bit,
	_point_physics_water_collisions_bit,
	_point_physics_simple_wind_bit,
	_point_physics_damped_wind_bit,
	_point_physics_no_gravity_bit,
	NUMBER_OF_POINT_PHYSICS_DEFINITION_FLAGS,
	NUMBER_OF_POINT_PHYSICS_DEFINITION_VARIABLES = 6
};

enum
{
	_point_physics_in_air_bit = 0,
	_point_physics_in_water_bit,
	_point_physics_collided_with_structure_bit,
	_point_physics_collided_with_water_bit,
	NUMBER_OF_POINT_PHYSICS_RESULT_FLAGS,
};

enum
{
	_point_physics_ignore_position_bit = 0,
	_point_physics_ignore_position_under_water_bit,
	_point_physics_force_no_collisions_bit,
	NUMBER_OF_POINT_PHYSICS_FLAGS,
};

/* ---------- structures */

struct point_physics_definition
{
	unsigned long flags;
	real runtime_mass_over_radius_cubed;
	real runtime_water_buoyancy_scale;
	real runtime_air_buoyancy_scale;
	long unused1[4];
	real density;
	real air_friction;
	real water_friction;
	real contact_friction;
	real elasticity;
	long unused2[3];
};

/* ---------- prototypes/POINT_PHYSICS.C */

void point_physics_initialize_for_new_map(void);
void point_physics_dispose_from_old_map(void);

real point_physics_definition_get_mass(struct point_physics_definition *definition, real radius);

struct point_physics_definition *point_physics_definition_interpolate(
	struct point_physics_definition const *physics1,
	struct point_physics_definition const *physics2,
	real t,
	struct point_physics_definition *result);

unsigned long point_physics_update(
	unsigned long flags,
	struct point_physics_definition const *definition,
	struct location *location,
	short force_weather_palette_index,
	real_point3d *position,
	real_vector3d *translational_velocity,
	real_vector3d const *translational_force,
	real_vector3d *collision_normal,
	short *collision_material_type,
	real radius,
	real dt);

/* ---------- globals */

extern boolean debug_point_physics;

/* ---------- public code */

#endif // __POINT_PHYSICS_H
