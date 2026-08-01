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
	// These are fake names, but based on data found in sapien
	// 0x11ef060 - "flamethrower particle collision"
	// 0x11ef088 - "collides with structures"
	// 0x11ef0a8 - "collides with water surface"
	// 0x11ef0d0 - "uses simple wind#the wind on this point won't have high-frequency variations"
	// 0x11ef130 - "uses damped wind#the wind on this point will be artificially slow"
	// 0x11ef180 - "no gravity#the point is not affected by gravity"
	_point_physics_flag_flamethrower_particle_collision_bit = 0,
	_point_physics_flag_collides_with_structures_bit,
	_point_physics_flag_collides_with_water_surface_bit,
	_point_physics_flag_uses_simple_wind_bit,
	_point_physics_flag_uses_damped_wind_bit,
	_point_physics_flag_no_gravity_bit,

	NUMBER_OF_POINT_PHYSICS_FLAGS
};

/* ---------- macros */

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
