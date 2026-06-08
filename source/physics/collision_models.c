/*
COLLISION_MODELS.C

*/

/* ---------- headers */
#include "cseries.h"
#include "object_types.h"
#include "objects.h"
#include "tag_groups.h"

#include "collisions.h"
#include "collision_model_definitions.h"
#include "collision_models.h"
#include "collision_bsp_definitions.h"
#include "collision_bsp.h"
#include "collision_usage.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */
int bsp3d_test_point(const struct bsp3d *bsp, int node_index, const real_point3d *point);
real_matrix4x3 *object_get_node_matrices(int object_index);
void collision_log_start_time(union _LARGE_INTEGER *lpPerformanceCount);
boolean collision_bsp_test_vector(
		unsigned int flags,
		const struct collision_bsp *bsp,
		short breakable_surface_count,
		const unsigned __int8 *breakable_surface_flags,
		const real_point3d *point,
		const real_vector3d *vector,
		real maximum_t,
		struct collision_bsp_test_vector_result *result);
boolean collision_bsp_test_pill(
		const struct collision_bsp *bsp,
		const real_point3d *point,
		const real_vector3d *vector,
		real radius,
		real maximum_t,
		struct collision_bsp_test_pill_result *result);
void collision_bsp_get_features_in_sphere(
		const struct collision_bsp *bsp,
		const struct collision_bsp_test_sphere_result *result,
		const real_matrix4x3 *matrix,
		real height,
		real width,
		int object_index,
		struct collision_feature_list *features);
void render_debug_collision_bsp(struct collision_bsp *bsp, const real_matrix4x3 *mat);
/* ---------- globals */

/* ---------- public code */
boolean collision_model_instance_new(
	struct collision_model_instance *instance,
	int object_index)
{
	struct object_datum *object;
	struct object_definition *obj_def;

	object = object_get(object_index);
	obj_def = object_definition_get(object->definition_index);

	if (obj_def->object.collision_model.index != NONE)
	{
		instance->object_index = object_index;
		instance->model = collision_model_definition_get(obj_def->object.collision_model.index);
		instance->region_permutation_indices = object->object.region_permutations;
		instance->matrices = object_get_node_matrices(object_index);

		return TRUE;
	}

	return FALSE;
}

boolean collision_model_test_sphere(
	const struct collision_model_instance *instance,
	const real_point3d *center,
	real radius)
{
	const struct collision_node *coll_node;
	struct collision_bsp *coll_bsp;
	short i;
	short perm;
	real_point3d transformed_center;
	real_matrix4x3 inverted_matrix;

	for (i = 0; i < instance->model->nodes.count; i++)
	{
		coll_node = TAG_BLOCK_GET_ELEMENT(&instance->model->nodes, i, struct collision_node);

		if (coll_node->region_index != NONE)
		{
			perm = instance->region_permutation_indices[coll_node->region_index];

			if (perm != NONE && coll_node->bsps.count > 0)
			{
				perm = PIN(perm, 0, coll_node->bsps.count - 1);

				coll_bsp = TAG_BLOCK_GET_ELEMENT(&coll_node->bsps, perm, struct collision_bsp);

				if (coll_bsp->bsp3d.nodes.count > 0)
				{
					matrix4x3_inverse(&instance->matrices[i], &inverted_matrix);
					matrix4x3_transform_point(&inverted_matrix, center, &transformed_center);
				}
			}
		}
	}

	return FALSE;
}

boolean collision_model_test_point(
	const struct collision_model_instance *instance,
	const real_point3d *point)
{
	const struct collision_node *coll_node;
	struct collision_bsp *coll_bsp;
	short i;
	short perm;
	real_point3d transformed_point;

	for (i = 0; i < instance->model->nodes.count; i++)
	{
		coll_node = TAG_BLOCK_GET_ELEMENT(&instance->model->nodes, i, struct collision_node);

		if (coll_node->region_index != NONE)
		{
			perm = instance->region_permutation_indices[coll_node->region_index];

			if (perm != NONE && coll_node->bsps.count > 0)
			{
				perm = PIN(perm, 0, coll_node->bsps.count - 1);

				coll_bsp = TAG_BLOCK_GET_ELEMENT(&coll_node->bsps, perm, struct collision_bsp);

				if (coll_bsp->bsp3d.nodes.count > 0)
				{
					matrix4x3_inverse_transform_point(&instance->matrices[i], point, &transformed_point);

					if (bsp3d_test_point(&coll_bsp->bsp3d, 0, &transformed_point) == NONE)
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

static __int64 collision_model_test_vector_TIME;

boolean collision_model_test_vector(
		const struct collision_model_instance *instance,
		unsigned int flags,
		const real_point3d *point,
		const real_vector3d *vector,
		struct collision_model_test_vector_result *result)
{
	const struct collision_node *coll_node;
	struct collision_bsp *coll_bsp;
	short i;
	short perm;
	boolean return_value;
	real_matrix4x3 inverted_matrix;
	real_point3d transformed_point;
	real_vector3d transformed_vector;

	return_value = FALSE;

	collision_log_usage(3);
	collision_log_start_time(&collision_model_test_vector_TIME);

	result->bsp_result.t = FLT_MAX;

	for (i = 0; i < instance->model->nodes.count; i++)
	{
		coll_node = TAG_BLOCK_GET_ELEMENT(&instance->model->nodes, i, struct collision_node);

		if (coll_node->region_index != NONE)
		{
			perm = instance->region_permutation_indices[coll_node->region_index];

			if (perm != NONE && coll_node->bsps.count > 0)
			{
				perm = PIN(perm, 0, coll_node->bsps.count - 1);

				coll_bsp = TAG_BLOCK_GET_ELEMENT(&coll_node->bsps, perm, struct collision_bsp);

				if (coll_bsp->bsp3d.nodes.count > 0)
				{
					matrix4x3_inverse(&instance->matrices[i], &inverted_matrix);
					matrix4x3_transform_point(&inverted_matrix, point, &transformed_point);
					matrix4x3_transform_vector(&inverted_matrix, vector, &transformed_vector);

					if (collision_bsp_test_vector(flags, coll_bsp, 0, 0, &transformed_point, &transformed_vector, result->bsp_result.t, &result->bsp_result))
					{
						result->node_index = i;
						result->region_index = coll_node->region_index;
						result->bsp_index = perm;

						return_value = TRUE;
					}
				}
			}
		}
	}

	collision_log_end_time(3, collision_model_test_vector_TIME);

	return return_value;
}

boolean collision_model_test_pill(
		const struct collision_model_instance *instance,
		const real_point3d *point,
		const real_vector3d *vector,
		real radius,
		struct collision_model_test_pill_result *result)
{
	const struct collision_node *coll_node;
	struct collision_bsp *coll_bsp;
	short i;
	short perm;
	real_point3d transformed_point;
	real_matrix4x3 inverted_matrix;
	real_vector3d transformed_vector;
	boolean returnval;
	real scaled_radius;

	result->bsp_result.t = FLT_MAX;

	returnval = FALSE;

	for (i = 0; i < instance->model->nodes.count; i++)
	{
		coll_node = TAG_BLOCK_GET_ELEMENT(&instance->model->nodes, i, struct collision_node);

		if (coll_node->region_index != NONE)
		{
			perm = instance->region_permutation_indices[coll_node->region_index];

			if (perm != NONE && coll_node->bsps.count > 0)
			{
				perm = PIN(perm, 0, coll_node->bsps.count - 1);

				coll_bsp = TAG_BLOCK_GET_ELEMENT(&coll_node->bsps, perm, struct collision_bsp);

				if (coll_bsp->bsp3d.nodes.count > 0)
				{
					matrix4x3_inverse(&instance->matrices[i], &inverted_matrix);
					matrix4x3_transform_point(&inverted_matrix, point, &transformed_point);
					matrix4x3_transform_vector(&inverted_matrix, vector, &transformed_vector);

					scaled_radius = inverted_matrix.scale * radius;

					if (collision_bsp_test_pill(
						coll_bsp,
						&transformed_point,
						&transformed_vector,
						scaled_radius,
						result->bsp_result.t,
						&result->bsp_result))
					{
						result->node_index = i;
						result->region_index = coll_node->region_index;
						result->bsp_index = perm;

						returnval = TRUE;
					}
				}
			}
		}
	}

	return returnval;
}

boolean collision_model_get_features_in_sphere(
		const struct collision_model_instance *instance,
		const real_point3d *center,
		real radius,
		real height,
		real width,
		struct collision_feature_list *features)
{
	const struct collision_node *coll_node;
	struct collision_bsp *coll_bsp;
	short i;
	short perm;
	real_point3d transformed_center;
	real_matrix4x3 inverted_matrix;
	boolean returnval;
	struct collision_bsp_test_sphere_result result;
	real scaled_radius;
	const real_matrix4x3 *mat;

	returnval = FALSE;

	for (i = 0; i < instance->model->nodes.count; i++)
	{
		coll_node = TAG_BLOCK_GET_ELEMENT(&instance->model->nodes, i, struct collision_node);

		if (coll_node->region_index != NONE)
		{
			perm = instance->region_permutation_indices[coll_node->region_index];

			if (perm != NONE && coll_node->bsps.count > 0)
			{
				perm = PIN(perm, 0, coll_node->bsps.count - 1);

				coll_bsp = TAG_BLOCK_GET_ELEMENT(&coll_node->bsps, perm, struct collision_bsp);

				if (coll_bsp->bsp3d.nodes.count > 0)
				{
					mat = &instance->matrices[i];
					matrix4x3_inverse(mat, &inverted_matrix);
					matrix4x3_transform_point(&inverted_matrix, center, &transformed_center);

					scaled_radius = inverted_matrix.scale * radius;

					if (collision_bsp_test_sphere(
						coll_bsp,
						0,
						0,
						&transformed_center,
						scaled_radius,
						&result))
					{
						collision_bsp_get_features_in_sphere(
							coll_bsp,
							&result,
							mat,
							height,
							width,
							instance->object_index,
							features);

						returnval = TRUE;
					}
				}
			}
		}
	}

	return returnval;
}

void render_debug_collision_model(const struct collision_model_instance *instance)
{
	const struct collision_node *coll_node;
	struct collision_bsp *coll_bsp;
	short i;
	short perm;
	real_point3d trans_point;

	for (i = 0; i < instance->model->nodes.count; i++)
	{
		coll_node = TAG_BLOCK_GET_ELEMENT(&instance->model->nodes, i, struct collision_node);

		if (coll_node->region_index != NONE)
		{
			perm = instance->region_permutation_indices[coll_node->region_index];

			if (perm != NONE && coll_node->bsps.count > 0)
			{
				perm = PIN(perm, 0, coll_node->bsps.count - 1);

				coll_bsp = TAG_BLOCK_GET_ELEMENT(&coll_node->bsps, perm, struct collision_bsp);

				if (coll_bsp->surfaces.count > 0 && coll_bsp->edges.count > 0 && coll_bsp->vertices.count > 0)
				{
					render_debug_collision_bsp(coll_bsp, &instance->matrices[i]);
				}
			}
		}
	}
}

/* ---------- private code */
