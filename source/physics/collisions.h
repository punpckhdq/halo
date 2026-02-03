/*
COLLISIONS.H

header included in hcex build.
*/

#ifndef __COLLISIONS_H
#define __COLLISIONS_H
#pragma once

/* ---------- headers */

#include "objects.h"

/* ---------- constants */

enum
{
	_collision_test_front_facing_surfaces_bit= 0,
	_collision_test_back_facing_surfaces_bit,
	_collision_test_ignore_two_sided_surfaces_bit,
	_collision_test_ignore_invisible_surfaces_bit,
	_collision_test_ignore_breakable_surfaces_bit,
	_collision_test_structure_bit,
	_collision_test_media_bit,
	_collision_test_objects_bit,

	_collision_test_objects_first_type_bit,
	_collision_test_objects_bipeds_bit= _collision_test_objects_first_type_bit,
	_collision_test_objects_vehicles_bit,
	_collision_test_objects_weapons_bit,
	_collision_test_objects_equipment_bit,
	_collision_test_objects_garbage_bit,
	_collision_test_objects_projectiles_bit,
	_collision_test_objects_scenery_bit,
	_collision_test_objects_machines_bit,
	_collision_test_objects_controls_bit,
	_collision_test_objects_light_fixtures_bit,
	_collision_test_objects_placeholders_bit,
	_collision_test_objects_last_type_bit,

	_collision_test_try_to_keep_location_valid_bit,
	_collision_test_skip_passthrough_bipeds_bit,
	_collision_test_use_vehicle_physics_bit,

	NUMBER_OF_COLLISION_TEST_FLAGS,

	_collision_test_objects_sight_blocking_flags= 
		FLAG(_collision_test_objects_vehicles_bit) |
		FLAG(_collision_test_objects_scenery_bit) |
		FLAG(_collision_test_objects_machines_bit),

	_collision_test_objects_all_types_flags=
		FLAG(_collision_test_objects_last_type_bit+1)-FLAG(_collision_test_objects_first_type_bit),
	
	_collision_test_environment_flags= 
		FLAG(_collision_test_front_facing_surfaces_bit) |
		FLAG(_collision_test_structure_bit) |
		FLAG(_collision_test_media_bit),
	
	_collision_test_for_projectiles_flags= 0x1000E9,
	_collision_test_for_projectiles_fat_flags= 0x89,
	_collision_test_for_line_of_sight_flags= 0xC2AD,
	_collision_test_for_bipeds_living_flags= 0x20C3A0,
	_collision_test_for_bipeds_passthrough_living_flags= 0xC2A0,
	_collision_test_for_bipeds_dead_flags= 0xC0A0,
	_collision_test_for_vehicles_flags= 0xC0A0,
};

/* ---------- macros */

/* ---------- structures */

struct collision_result
{
	short type;
	struct location start_location;
	struct location location;
	real t;
	real_point3d point;
	real_plane3d plane;
	short material_type;
	long object_index;
	short region_index;
	short node_index;
	short bsp_index;
	long surface_index;
	long plane_designator;
	byte flags;
	byte breakable_surface_index;
	short material_index;
};

/* ---------- prototypes/COLLISIONS.C */

boolean collision_test_vector(unsigned long flags, real_point3d const *point, real_vector3d const *vector, long ignore_object_index, struct collision_result *collision);

/* ---------- globals */

/* ---------- public code */

__inline boolean collision_test_line(
	unsigned long flags,
	real_point3d const *point0,
	real_point3d const *point1,
	long ignore_object_index,
	struct collision_result *collision)
{
	real_vector3d vector;

	vector_from_points3d(point0, point1, &vector);
	return collision_test_vector(flags, point0, &vector, ignore_object_index, collision);
}


#endif // __COLLISIONS_H
