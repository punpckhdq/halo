/*
PHYSICS_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __PHYSICS_DEFINITIONS_H
#define __PHYSICS_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

enum
{
	POINT_PHYSICS_DEFINITION_TAG= 'phys',
	POINT_PHYSICS_DEFINITION_VERSION= 1,
};

/* ---------- macros */

#define physics_definition_get(index) ((struct physics_definition *)tag_get(POINT_PHYSICS_DEFINITION_TAG, index))

/* ---------- structures */

struct physics_definition
{
	real radius;
	real moment;
	real mass;
	real_point3d center_of_mass;
	real density;
	real gravity_scale;
	real ground_friction;
	real ground_depth;
	real ground_damp_fraction;
	real ground_normal_k1;
	real ground_normal_k0;
	real ground_unused;
	real water_friction;
	real water_depth;
	real water_density;
	real water_unused;
	real air_friction;
	real air_unused;
	real xx_moment;
	real yy_moment;
	real zz_moment;
	struct tag_block inertial_matrix;
	struct tag_block powered_mass_points;
	struct tag_block mass_points;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __PHYSICS_DEFINITIONS_H
