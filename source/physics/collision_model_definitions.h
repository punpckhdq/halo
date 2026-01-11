/*
COLLISION_MODEL_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __COLLISION_MODEL_DEFINITIONS_H
#define __COLLISION_MODEL_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"
#include "objects/damage_resistances.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

// Couldn't find tag constants for this definition?

/* ---------- macros */

#define collision_model_definition_get(index) ((struct collision_model *)tag_get('coll', index))

/* ---------- structures */

struct pathfinding_sphere
{
	short node_index;
	word pad;
	long unused[3];
	real_point3d center;
	real radius;
};

struct collision_model
{
	struct damage_resistance resistance;
	long unused[4];
	real_rectangle3d pathfinding_box;
	struct tag_block pathfinding_spheres;	// pathfinding_sphere
	struct tag_block nodes;
};

/* ---------- prototypes/COLLISION_MODEL_DEFINITIONS.C */

/* ---------- globals */

/* ---------- public code */

#endif // __COLLISION_MODEL_DEFINITIONS_H
