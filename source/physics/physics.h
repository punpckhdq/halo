/*
PHYSICS.H

header included in hcex build.
*/

#ifndef __PHYSICS_H
#define __PHYSICS_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct physics_instance
{
	long object_index;
	const struct physics_definition *physics;
	real_matrix4x3 world_matrix;
};

/* ---------- prototypes/PHYSICS.C */

boolean physics_instance_new(struct physics_instance *instance, long object_index);

void render_debug_physics(struct physics_instance *instance);


/* ---------- globals */

/* ---------- public code */

#endif // __PHYSICS_H
