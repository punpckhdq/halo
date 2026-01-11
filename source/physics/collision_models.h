/*
COLLISION_MODELS.H

header included in hcex build.
*/

#ifndef __COLLISION_MODELS_H
#define __COLLISION_MODELS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct collision_model_instance
{
	long object_index;
	const struct collision_model *model;
	const byte *region_permutation_indices;
	const struct real_matrix4x3 *matrices;
};

/* ---------- prototypes/COLLISION_MODELS.C */

boolean collision_model_instance_new(struct collision_model_instance *instance, long object_index);

void render_debug_collision_model(struct collision_model_instance *instance);

/* ---------- globals */

/* ---------- public code */

#endif // __COLLISION_MODELS_H
