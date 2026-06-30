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



boolean collision_model_test_sphere(
	struct collision_model_instance const *instance,
	real_point3d const *center,
	real radius);

boolean collision_model_test_point(
	struct collision_model_instance const *instance,
	real_point3d const *point);

boolean collision_model_test_vector(
		struct collision_model_instance const *instance,
		unsigned long flags,
		real_point3d const *point,
		real_vector3d const *vector,
		struct collision_model_test_vector_result *result);

boolean collision_model_test_pill(
		struct collision_model_instance const *instance,
		real_point3d const *point,
		real_vector3d const *vector,
		real radius,
		struct collision_model_test_pill_result *result);

boolean collision_model_get_features_in_sphere(
		const struct collision_model_instance *instance,
		const union real_point3d *center,
		real radius,
		real height,
		real width,
		struct collision_feature_list *features);

void render_debug_collision_model(struct collision_model_instance const *instance);

/* ---------- globals */

/* ---------- public code */

#endif // __COLLISION_MODELS_H
