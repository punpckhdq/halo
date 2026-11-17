/*
COLLISIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "collisions.h"
#include "physics.h"
#include "collision_models.h"
#include "structure_bsp_definitions.h"
#include "bipeds.h"
#include "structures.h"
#include "fog_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean object_test_point(long object_index, unsigned long flags, real_point3d const *point, long ignore_object_index);

static boolean object_test_vector(long object_index, unsigned long flags, unsigned long bsp_flags, real_point3d const *point, real_vector3d const *vector, long ignore_object_index, struct collision_result *collision);

static void object_get_features_in_sphere(unsigned long flags, long object_index, real_point3d const *center, real radius, real height, real width, long ignore_object_index, struct collision_feature_list *features);

static void clip_position_to_plane(real_point3d *position, real_plane3d const *plane, real_point3d *result);
static void clip_velocity_to_plane(real_vector3d const *velocity, real_plane3d const *plane, real_vector3d *result);
static void clip_position_to_line(real_point3d const *position, real_point3d const *point, real_vector3d const *vector, real_point3d *result);
static void clip_velocity_to_line(real_vector3d const *velocity, real_point3d const *point, real_vector3d const *vector, real_vector3d *result);
static void collision_fix_pill_nudge_collision(unsigned long flags, real_point3d const *point, real_vector3d const *vector, long ignore_object_index, struct collision_plane *collision);

/* ---------- globals */

// Timer names are inferred; the cachebeta PDB does not retain them.
static __int64 features_start_time;
static __int64 structure_start_time;
static __int64 objects_start_time;

#ifdef DEBUG
boolean debug_collision_skip_objects = FALSE;
boolean debug_collision_skip_vectors = FALSE;
#endif

/* ---------- public code */

boolean collision_test_sphere(
	real_point3d const *center,
	real radius,
	long ignore_object_index)
{
	struct collision_bsp_test_sphere_result result;
	
	boolean success;

	if (bsp3d_test_point(global_bsp3d_get(), 0, center) == NONE || collision_bsp_test_sphere(global_collision_bsp_get(), MAXIMUM_BREAKABLE_SURFACES_PER_MAP, breakable_surface_flags_get(), center, radius, &result))
	{
		success = TRUE;
	}
	else
	{
		success = FALSE;
	}

	return success;
}

boolean collision_test_point(
	unsigned long flags,
	real_point3d const *point,
	long ignore_object_index)
{
	if (TEST_FLAG(flags, _collision_test_structure_bit) || TEST_FLAG(flags, _collision_test_media_bit) || TEST_FLAG(flags, _collision_test_objects_bit))
	{
		long leaf_index = bsp3d_test_point(global_bsp3d_get(), 0, point);
		boolean test_objects = TEST_FLAG(flags, _collision_test_objects_bit);

#ifdef DEBUG
		if (debug_collision_skip_objects)
		{
			test_objects = FALSE;
		}
#endif

		if (leaf_index == NONE)
		{
			return TRUE;
		}

		if (test_objects)
		{
			short cluster_index = TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->leaves, leaf_index & LONG_MAX, struct structure_leaf)->cluster_index;
			long state;
			long object_index;

			for (object_index = cluster_get_first_collideable_object(&state, cluster_index); object_index != NONE; object_index = cluster_get_next_collideable_object(&state))
			{
				if (object_test_point(object_index, flags, point, ignore_object_index))
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

boolean collision_test_vector(
	unsigned long flags,
	real_point3d const *point,
	real_vector3d const *vector,
	long ignore_object_index,
	struct collision_result *collision)
{
	boolean hit = FALSE;
	boolean test = TEST_FLAG(flags, _collision_test_structure_bit) || TEST_FLAG(flags, _collision_test_media_bit) || TEST_FLAG(flags, _collision_test_objects_bit);

#ifdef DEBUG
	if (debug_collision_skip_vectors)
	{
		test = FALSE;
	}
#endif

	collision->type = NONE;
	collision->start_location.leaf_index = NONE;
	collision->start_location.cluster_index = NONE;
	collision->location.leaf_index = NONE;
	collision->location.cluster_index = NONE;
	collision->t = 1.f;

	if (test)
	{
		struct structure_bsp *structure = global_structure_bsp_get();
		unsigned long bsp_flags = 0;
		boolean test_objects = TEST_FLAG(flags, _collision_test_objects_bit);
		struct collision_bsp_test_vector_result structure_result;

#ifdef DEBUG
		if (debug_collision_skip_objects)
		{
			test_objects = FALSE;
		}
#endif

		if (!TEST_FLAG(flags, _collision_test_front_facing_surfaces_bit) && !TEST_FLAG(flags, _collision_test_back_facing_surfaces_bit))
		{
			flags |= FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_back_facing_surfaces_bit);
		}
		
		SET_FLAG(bsp_flags, _collision_bsp_test_front_facing_surfaces_bit, TEST_FLAG(flags, _collision_test_front_facing_surfaces_bit));
		SET_FLAG(bsp_flags, _collision_bsp_test_back_facing_surfaces_bit, TEST_FLAG(flags, _collision_test_back_facing_surfaces_bit));
		SET_FLAG(bsp_flags, _collision_bsp_test_ignore_two_sided_surfaces_bit, TEST_FLAG(flags, _collision_test_ignore_two_sided_surfaces_bit));
		SET_FLAG(bsp_flags, _collision_bsp_test_ignore_invisible_surfaces_bit, TEST_FLAG(flags, _collision_test_ignore_invisible_surfaces_bit));
		SET_FLAG(bsp_flags, _collision_bsp_test_ignore_breakable_surfaces_bit, TEST_FLAG(flags, _collision_test_ignore_breakable_surfaces_bit));

		{
#ifdef DEBUG
			collision_log_usage(_collision_function_vector_structure);
			collision_log_start_time(&structure_start_time);
#endif
			
			if (collision_bsp_test_vector(bsp_flags, global_collision_bsp_get(), MAXIMUM_BREAKABLE_SURFACES_PER_MAP, breakable_surface_flags_get(), point, vector, REAL_MAX, &structure_result) && TEST_FLAG(flags, _collision_test_structure_bit))
			{
				collision->type = _collision_result_structure;
				collision->t = structure_result.t;
				collision->plane = *structure_result.plane;
				if (structure_result.plane_designator & LONG_MIN)
				{
					plane3d_negate(&collision->plane, &collision->plane);
				}
				collision->material_type = structure_result.material_index != NONE ? TAG_BLOCK_GET_ELEMENT(&structure->collision_materials, structure_result.material_index, struct structure_collision_material)->runtime_physics_material_type : NONE;
				collision->surface_index = structure_result.surface_index;
				collision->plane_designator = structure_result.plane_designator;
				collision->flags = structure_result.flags;
				collision->breakable_surface_index = structure_result.breakable_surface_index;
				collision->material_index = structure_result.material_index;
				hit = TRUE;
			}
			
			if (structure_result.leaf_count > 0)
			{
				collision->start_location.leaf_index = structure_result.leaf_indices[0];
				collision->start_location.cluster_index = collision->start_location.leaf_index == NONE ? NONE : TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->leaves, collision->start_location.leaf_index & LONG_MAX, struct structure_leaf)->cluster_index;
				collision->location.leaf_index = structure_result.leaf_indices[structure_result.leaf_count - 1];
				collision->location.cluster_index = collision->location.leaf_index == NONE ? NONE : TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->leaves, collision->location.leaf_index & LONG_MAX, struct structure_leaf)->cluster_index;
			}

#ifdef DEBUG
			collision_log_end_time(_collision_function_vector_structure, structure_start_time);
#endif
		}

		if (TEST_FLAG(flags, _collision_test_media_bit) && collision->location.cluster_index != NONE)
		{
			short fog_designator = TAG_BLOCK_GET_ELEMENT(&structure->clusters, collision->location.cluster_index, struct structure_cluster)->fog_designator;

			if (fog_designator != NONE && (fog_designator & 0x8000))
			{
				struct structure_fog_plane *fog_plane = TAG_BLOCK_GET_ELEMENT(&structure->fog_planes, fog_designator & SHORT_MAX, struct structure_fog_plane);

				if (fog_plane->runtime_material_type != NONE)
				{
					struct structure_fog_region *region = TAG_BLOCK_GET_ELEMENT(&structure->fog_regions, fog_plane->region_index, struct structure_fog_region);
					struct structure_fog_palette_entry *palette = TAG_BLOCK_GET_ELEMENT(&structure->fog_palette, region->fog_palette_index, struct structure_fog_palette_entry);
					struct fog_definition *fog = fog_definition_get(palette->fog.index);
					real_plane3d plane = fog_plane->plane;
					real distance, projection, t;

					plane.d -= fog->distance_to_water_plane;
					distance = plane3d_distance_to_point(&plane, point);
					projection = dot_product3d(vector, &plane.n);
					
					if ((distance > 0.f) != (projection > 0.f) && fabs(distance) < fabs(projection) && fabs(projection) >= _real_epsilon)
					{
						t = -distance / projection;
						if (collision->t > t)
						{
							boolean below_plane = distance < 0.f;

							collision->type = _collision_result_media;
							collision->t = t;
							collision->plane = plane;
							
							if (below_plane)
							{
								plane3d_negate(&collision->plane, &collision->plane);
							}
							
							collision->material_type = below_plane ? _material_water : fog_plane->runtime_material_type;
							hit = TRUE;
						}
					}
				}
			}
		}
		if (test_objects && structure_result.leaf_count > 0)
		{
			long index;

#ifdef DEBUG
			collision_log_usage(_collision_function_vector_objects);
			collision_log_start_time(&objects_start_time);
#endif

			if (!(flags & _collision_test_objects_all_types_flags))
			{
				flags |= _collision_test_objects_all_types_flags;
			}

			structure_cluster_marker_begin();
			object_marker_begin();
			
			for (index = 0; index < structure_result.leaf_count; index++)
			{
				long cluster_index = structure_result.leaf_indices[index] == NONE ? NONE : TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->leaves, structure_result.leaf_indices[index] & LONG_MAX, struct structure_leaf)->cluster_index;

				if (structure_cluster_mark(cluster_index))
				{
					long state;
					long object_index;

					for (object_index = cluster_get_first_collideable_object(&state, cluster_index); object_index != NONE; object_index = cluster_get_next_collideable_object(&state))
					{
						if (object_mark_function(object_index) && object_test_vector(object_index, flags, bsp_flags, point, vector, ignore_object_index, collision))
						{
							hit = TRUE;
						}
					}
				}
			}

			object_marker_end();
			structure_cluster_marker_end();
#ifdef DEBUG
			collision_log_end_time(_collision_function_vector_objects, objects_start_time);
#endif
		}

		if (!hit)
		{
			collision->t = 1.f;
		}

		point_from_line3d(point, vector, collision->t, &collision->point);
		
		if (TEST_FLAG(flags, _collision_test_try_to_keep_location_valid_bit) && hit && collision->location.leaf_index != NONE)
		{
			if (scenario_leaf_index_from_point(&collision->point) != collision->location.leaf_index)
			{
				point_from_line3d(&collision->point, &collision->plane.n, 1.f / 4096.f, &collision->point);
				scenario_location_from_point(&collision->location, &collision->point);
				if (collision->location.leaf_index == NONE)
				{
					real projection = dot_product3d(vector, &collision->plane.n);
					real step = projection != 0.f ? (1.f / 4096.f) / fabs(projection) : 1.f / 32.f;

					do
					{
						collision->t = MAX(collision->t - step, 0.f);
						point_from_line3d(point, vector, collision->t, &collision->point);
						scenario_location_from_point(&collision->location, &collision->point);
					} 
					while (!(collision->t <= 0.f) && collision->location.leaf_index == NONE);
				}
			}
		}
	}
	else
	{
		collision->t = 1.f;
		point_from_line3d(point, vector, 1.f, &collision->point);
		scenario_location_from_point(&collision->location, &collision->point);
	}

	return hit;
}

boolean collision_test_vector_exit(
	struct collision_result const *previous_collision,
	real_point3d const *point,
	real_vector3d const *vector,
	struct collision_result *exit_collision)
{
	boolean hit = FALSE;

	exit_collision->type = NONE;
	exit_collision->t = REAL_MAX;

	switch (previous_collision->type)
	{
	case _collision_result_object:
	{
		struct collision_model_instance instance;
		struct collision_model_test_vector_result result;
		real_point3d p;
		real_vector3d v;

		set_real_point3d(&p, point->x + vector->i, point->y + vector->j, point->z + vector->k);
		negate_vector3d(vector, &v);
		
		if (collision_model_instance_new(&instance, previous_collision->object_index) &&
			collision_model_test_vector(&instance, FLAG(_collision_bsp_test_front_facing_surfaces_bit), &p, &v, &result))
		{
			exit_collision->type = _collision_result_object;
			exit_collision->t = 1.f - result.bsp_result.t;
			matrix4x3_transform_plane(&instance.matrices[result.node_index], result.bsp_result.plane, &exit_collision->plane);
			
			if (result.bsp_result.plane_designator & LONG_MIN)
			{
				plane3d_negate(&exit_collision->plane, &exit_collision->plane);
			}

			exit_collision->material_type = collision_model_get_material_type(instance.model, result.bsp_result.material_index);
			exit_collision->object_index = previous_collision->object_index;
			exit_collision->region_index = result.region_index;
			exit_collision->node_index = result.node_index;
			exit_collision->bsp_index = result.bsp_index;
			exit_collision->surface_index = result.bsp_result.surface_index;
			exit_collision->plane_designator = result.bsp_result.plane_designator;
			exit_collision->flags = result.bsp_result.flags;
			exit_collision->breakable_surface_index = result.bsp_result.breakable_surface_index;
			exit_collision->material_index = result.bsp_result.material_index;
			hit = TRUE;
		}

		break;
	}
	default:
		break;
	}

	if (hit)
	{
		point_from_line3d(point, vector, exit_collision->t, &exit_collision->point);
	}

	return hit;
}

boolean collision_test_pill(
	unsigned long flags,
	real_point3d const *point,
	real_vector3d const *vector,
	real radius,
	long ignore_object_index,
	struct collision_result *collision)
{
	struct collision_bsp_test_pill_result structure_result;

	boolean hit = FALSE;

	collision->type = NONE;
	collision->t = REAL_MAX;

	if (collision_bsp_test_pill(global_collision_bsp_get(), point, vector, radius, REAL_MAX, &structure_result))
	{
		collision->t = structure_result.t;
		
		if (TEST_FLAG(flags, _collision_test_structure_bit))
		{
			collision->type = _collision_result_structure;
			collision->plane = structure_result.plane;
			collision->material_type = structure_result.material_index;
			collision->surface_index = structure_result.surface_index;
			collision->plane_designator = NONE;
			collision->flags = 0;
			collision->breakable_surface_index = 0;
			collision->material_index = structure_result.material_index;
			hit = TRUE;
		}
	}

	if (structure_result.leaf_count > 0)
	{
		collision->start_location.leaf_index = structure_result.leaf_indices[0];
		collision->start_location.cluster_index = collision->start_location.leaf_index == NONE ? NONE : TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->leaves, collision->start_location.leaf_index & LONG_MAX, struct structure_leaf)->cluster_index;
		collision->location.leaf_index = structure_result.leaf_indices[structure_result.leaf_count - 1];
		collision->location.cluster_index = collision->location.leaf_index == NONE ? NONE : TAG_BLOCK_GET_ELEMENT(&global_structure_bsp_get()->leaves, collision->location.leaf_index & LONG_MAX, struct structure_leaf)->cluster_index;
	}

	if (!hit)
	{
		collision->t = 1.f;
	}

	point_from_line3d(point, vector, collision->t, &collision->point);
	scenario_location_from_point(&collision->location, &collision->point);

	return hit;
}

boolean collision_test_pill_new(
	unsigned long flags,
	real_point3d const *point,
	real_vector3d const *vector,
	real radius,
	long ignore_object_index,
	struct collision_result *collision)
{
	real t;
	real_vector3d normal;

	boolean result = FALSE;

	collision->type = NONE;
	collision->start_location.leaf_index = NONE;
	collision->start_location.cluster_index = NONE;
	collision->location.leaf_index = NONE;
	collision->location.cluster_index = NONE;
	collision->t = 1.f;

	if (collision_bsp_test_pill_new(global_collision_bsp_get(), 0, NULL, point, vector, radius, &t, &normal))
	{
		collision->type = _collision_result_structure;
		collision->t = t;
		collision->plane.n = normal;
		collision->plane.d = REAL_MAX;
		collision->material_type = NONE;
		collision->surface_index = NONE;
		collision->plane_designator = NONE;
		collision->flags = 0;
		collision->breakable_surface_index = 0;
		collision->material_index = NONE;
		result = TRUE;
	}

	point_from_line3d(point, vector, collision->t, &collision->point);
	set_real_vector3d(&collision->plane.n, 0.f, 0.f, 0.f);

	return result;
}

boolean collision_get_features_in_sphere(
	unsigned long flags,
	real_point3d const *center,
	real radius,
	real height,
	real width,
	long ignore_object_index,
	struct collision_feature_list *features)
{
	collision_features_new(features);

	if (TEST_FLAG(flags, _collision_test_structure_bit) || TEST_FLAG(flags, _collision_test_media_bit) || TEST_FLAG(flags, _collision_test_objects_bit))
	{
		struct collision_bsp_test_sphere_result structure_result;

		struct structure_bsp *structure = global_structure_bsp_get();
		struct collision_bsp *bsp = global_collision_bsp_get();
		boolean test_objects = TEST_FLAG(flags, _collision_test_objects_bit);

#ifdef DEBUG
		if (debug_collision_skip_objects)
		{
			test_objects = FALSE;
		}
		
		collision_log_usage(_collision_function_vector_bounds_object);
		collision_log_start_time(&features_start_time);
#endif
		radius += 1.f / 16.f;
		
		if (collision_bsp_test_sphere(bsp, MAXIMUM_BREAKABLE_SURFACES_PER_MAP, breakable_surface_flags_get(), center, radius, &structure_result) && TEST_FLAG(flags, _collision_test_structure_bit))
		{
			collision_bsp_get_features_in_sphere(bsp, &structure_result, NULL, height, width, NONE, features);
		}

		if (test_objects && structure_result.leaf_count > 0)
		{
			short leaf_index_index;

			if (!(flags & _collision_test_objects_all_types_flags))
			{
				flags |= _collision_test_objects_all_types_flags;
			}

			structure_cluster_marker_begin();
			object_marker_begin();
			
			for (leaf_index_index = 0; leaf_index_index < structure_result.leaf_count; leaf_index_index++)
			{
				long leaf_index = structure_result.leaf_indices[leaf_index_index];
				struct structure_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&structure->leaves, leaf_index & LONG_MAX, struct structure_leaf);

				if (structure_cluster_mark(leaf->cluster_index))
				{
					long state;
					long object_index;

					for (object_index = cluster_get_first_collideable_object(&state, leaf->cluster_index); object_index != NONE; object_index = cluster_get_next_collideable_object(&state))
					{
						if (object_mark_function(object_index))
						{
							object_get_features_in_sphere(flags, object_index, center, radius, height, width, ignore_object_index, features);
						}
					}
				}
			}

			object_marker_end();
			structure_cluster_marker_end();
		}
#ifdef DEBUG
		collision_log_end_time(_collision_function_vector_bounds_object, features_start_time);
#endif
	}

	return features->count[_collision_feature_sphere] || features->count[_collision_feature_cylinder] || features->count[_collision_feature_prism];
}

boolean collision_fix_pill(
	unsigned long flags,
	real_point3d const *old_position,
	real distance,
	real height,
	real width,
	long ignore_object_index,
	real_point3d *new_position)
{
	struct collision_feature_list features;
	struct collision_plane initial_collision;

	boolean fixed_pill = FALSE;

#ifdef DEBUG
	match_assert("c:\\halo\\SOURCE\\physics\\collisions.c", 1272, global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH);
	global_current_collision_users[global_current_collision_user_depth++] = _collision_user_bipeds;
#endif
	
	{
		real_point3d test_center;
		real test_radius = height * 0.5f + width + distance;

		set_real_point3d(&test_center, old_position->x, old_position->y, old_position->z + height * 0.5f);
		collision_get_features_in_sphere(flags, &test_center, test_radius, height, width, ignore_object_index, &features);
	}

	if (collision_features_test_point(&features, old_position, &initial_collision) || collision_test_point(flags, old_position, ignore_object_index))
	{
		real_point3d valid_air_position;
		short offset_index;

		static real_vector3d offsets[] =
		{
			{ -1.f, 0.f, 0.f },
			{ 1.f, 0.f, 0.f },
			{ 0.f, -1.f, 0.f },
			{ 0.f, 1.f, 0.f },
			{ -M_SQRT1_2, -M_SQRT1_2, 0.f },
			{ M_SQRT1_2, M_SQRT1_2, 0.f },
			{ M_SQRT1_2, -M_SQRT1_2, 0.f },
			{ -M_SQRT1_2, M_SQRT1_2, 0.f },
			{ 0.f, 0.f, 1.f },
			{ -M_SQRT1_2, 0.f, M_SQRT1_2 },
			{ M_SQRT1_2, 0.f, M_SQRT1_2 },
			{ 0.f, -M_SQRT1_2, M_SQRT1_2 },
			{ 0.f, M_SQRT1_2, M_SQRT1_2 },
			{ -M_SQRT1_3, -M_SQRT1_3, M_SQRT1_3 },
			{ M_SQRT1_3, M_SQRT1_3, M_SQRT1_3 },
			{ M_SQRT1_3, -M_SQRT1_3, M_SQRT1_3 },
			{ -M_SQRT1_3, M_SQRT1_3, M_SQRT1_3 },
		};
		boolean found_valid_air_position = FALSE;

		for (offset_index = 0; offset_index < NUMBEROF(offsets); offset_index++)
		{
			real_point3d position;
			struct collision_plane collision;

			point_from_line3d(old_position, &offsets[offset_index], distance, &position);
			
			if (!collision_features_test_point(&features, &position, &collision) && !collision_test_point(flags, &position, ignore_object_index))
			{
				real_vector3d vector;

				scale_vector3d(global_down3d, distance, &vector);
				
				if (collision_features_test_vector(&features, &position, &vector, &collision) && collision.plane.n.k > 0.76604444f)
				{
					collision_fix_pill_nudge_collision(flags, &position, &vector, ignore_object_index, &collision);
					*new_position = collision.point;
					fixed_pill = TRUE;
					break;
				}

				if (!found_valid_air_position)
				{
					valid_air_position = position;
					found_valid_air_position = TRUE;
				}
			}
		}

		if (!fixed_pill && found_valid_air_position)
		{
			real_vector3d vector;
			struct collision_plane collision;

			vector_from_points3d(&valid_air_position, old_position, &vector);
			collision_features_test_vector(&features, &valid_air_position, &vector, &collision);
			collision_fix_pill_nudge_collision(flags, &valid_air_position, &vector, ignore_object_index, &collision);
			*new_position = collision.point;
			fixed_pill = TRUE;
		}
	}
	else
	{
		*new_position = *old_position;
		fixed_pill = TRUE;
	}

#ifdef DEBUG
	match_assert("c:\\halo\\SOURCE\\physics\\collisions.c", 1378, global_current_collision_user_depth > 1);
	global_current_collision_user_depth--;
#endif

	return fixed_pill;
}

short collision_move_point(
	real_point3d const *old_position,
	real_vector3d const *old_velocity,
	struct collision_feature_list const *features,
	real_point3d *new_position,
	real_vector3d *new_velocity,
	short maximum_collision_count,
	struct collision_plane *collisions)
{
	short clip_collisions[3];
	real_plane3d clip_plane;
	real_point3d clip_line_point;
	real_vector3d clip_line_vector;
	real_point3d clip_point;
	boolean features_full;

	short collision_count = 0;

	real_point3d position = *old_position;
	real_vector3d velocity = *old_velocity;
	real_point3d clipped_position = position;
	real_vector3d clipped_velocity = velocity;
	
	short clip_count = 0;

	match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\collisions.c", 941, old_position);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\collisions.c", 942, old_velocity);
	match_assert("c:\\halo\\SOURCE\\physics\\collisions.c", 943, features->count[_collision_feature_sphere]<=MAXIMUM_COLLISION_FEATURES_PER_TEST);
	match_assert("c:\\halo\\SOURCE\\physics\\collisions.c", 944, features->count[_collision_feature_cylinder]<=MAXIMUM_COLLISION_FEATURES_PER_TEST);
	match_assert("c:\\halo\\SOURCE\\physics\\collisions.c", 945, features->count[_collision_feature_prism]<=MAXIMUM_COLLISION_FEATURES_PER_TEST);

	if (features->count[_collision_feature_sphere]==MAXIMUM_COLLISION_FEATURES_PER_TEST ||
		features->count[_collision_feature_cylinder]==MAXIMUM_COLLISION_FEATURES_PER_TEST ||
		features->count[_collision_feature_prism]==MAXIMUM_COLLISION_FEATURES_PER_TEST)
	{
		features_full = TRUE;
	}

	while (!(fabs(clipped_velocity.i) < _real_epsilon &&
		fabs(clipped_velocity.j) < _real_epsilon &&
		fabs(clipped_velocity.k) < _real_epsilon))
	{
		struct collision_plane *collision = &collisions[collision_count];

		match_assert("c:\\halo\\SOURCE\\physics\\collisions.c", 959, collision_count<maximum_collision_count);
		match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\collisions.c", 960, &clipped_position);
		match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\collisions.c", 961, &clipped_velocity);

		if (collision_features_test_vector(features, &clipped_position, &clipped_velocity, collision))
		{
			short new_clip_collisions[3];
			short new_clip_count;

			collision_count++;
			position = collision->point;
			scale_vector3d(&velocity, 1.f - collision->t, &velocity);

			match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\collisions.c", 972, &position);
			match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\collisions.c", 973, &velocity);
			match_assert_valid_real_plane3d("c:\\halo\\SOURCE\\physics\\collisions.c", 974, &collision->plane);

			match_assert("c:\\halo\\SOURCE\\physics\\collisions.c", 977, clip_count<3);
			new_clip_count = 1;
			new_clip_collisions[0] = collision_count - 1;
			clip_plane = collisions[new_clip_collisions[0]].plane;
			clip_velocity_to_plane(&velocity, &clip_plane, &clipped_velocity);
			clip_position_to_plane(&position, &clip_plane, &clipped_position);

			if (clip_count > 0)
			{
				if (dot_product3d(&clipped_velocity, &collisions[clip_collisions[0]].plane.n) < -_real_epsilon &&
					line_from_planes3d(&collisions[new_clip_collisions[0]].plane, &collisions[clip_collisions[0]].plane, &clip_line_point, &clip_line_vector))
				{
					match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\collisions.c", 993, &clip_line_point);
					match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\collisions.c", 994, &clip_line_vector);

					new_clip_count = 2;
					new_clip_collisions[1] = clip_collisions[0];
					clip_velocity_to_line(&velocity, &clip_line_point, &clip_line_vector, &clipped_velocity);
					clip_position_to_line(&position, &clip_line_point, &clip_line_vector, &clipped_position);

					if (clip_count > 1)
					{
						if (dot_product3d(&clipped_velocity, &collisions[clip_collisions[1]].plane.n) < -_real_epsilon &&
							point_from_planes3d(&collisions[new_clip_collisions[0]].plane, &collisions[new_clip_collisions[1]].plane, &collisions[clip_collisions[1]].plane, &clip_point))
						{
							match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1006, &clip_point);

							new_clip_count = 3;
							new_clip_collisions[2] = clip_collisions[1];
							set_real_vector3d(&clipped_velocity, 0.f, 0.f, 0.f);
							clipped_position = clip_point;
						}
					}
				}
				else if (clip_count > 1 && dot_product3d(&clipped_velocity, &collisions[clip_collisions[1]].plane.n) < -_real_epsilon)
				{
					if (line_from_planes3d(&collisions[new_clip_collisions[0]].plane, &collisions[clip_collisions[1]].plane, &clip_line_point, &clip_line_vector))
					{
						match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1022, &clip_line_point);
						match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1023, &clip_line_vector);

						new_clip_count = 2;
						new_clip_collisions[1] = clip_collisions[1];
						clip_velocity_to_line(&velocity, &clip_line_point, &clip_line_vector, &clipped_velocity);
						clip_position_to_line(&position, &clip_line_point, &clip_line_vector, &clipped_position);
					}
				}
			}

			match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1034, &clipped_position);
			match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1035, &clipped_velocity);

			clip_count = new_clip_count;
			memcpy(clip_collisions, new_clip_collisions, new_clip_count * sizeof(short));
		}
		else
		{
			clipped_position = collision->point;
			match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1048, &clipped_position);
			break;
		}

		if (collision_count >= maximum_collision_count)
		{
			break;
		}
	}

	*new_position = clipped_position;
	switch (clip_count)
	{
	case 0:
		*new_velocity = *old_velocity;
		break;

	case 1:
		match_assert_valid_real_plane3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1064, &clip_plane);
		clip_velocity_to_plane(old_velocity, &clip_plane, new_velocity);
		break;

	case 2:
		match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1069, &clip_line_point);
		match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1070, &clip_line_vector);
		clip_velocity_to_line(old_velocity, &clip_line_point, &clip_line_vector, new_velocity);
		break;

	case 3:
		match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1075, &clip_point);
		set_real_vector3d(new_velocity, 0.f, 0.f, 0.f);
		break;

	default:
		match_assert("c:\\halo\\SOURCE\\physics\\collisions.c", 1080, !"unreachable");
		break;
	}

	match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1083, new_position);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1084, new_velocity);

	if (clip_count > 1 && collision_count < maximum_collision_count)
	{
		real lowest_k;
		short lowest_clip;
		short index;

		struct collision_plane *composite = &collisions[collision_count++];

		composite->t = collisions[clip_collisions[clip_count - 1]].t;
		composite->point = collisions[clip_collisions[clip_count - 1]].point;
		composite->object_index = NONE;
		composite->surface_index = NONE;
		composite->flags = 0;
		composite->breakable_surface_index = 0;
		composite->material_index = NONE;

		lowest_k = 0.f;
		lowest_clip = NONE;
		for (index = 0; index < clip_count; index++)
		{
			struct collision_plane const *clip_collision = &collisions[clip_collisions[index]];

			if (lowest_k > clip_collision->plane.n.k)
			{
				lowest_k = clip_collision->plane.n.k;
				lowest_clip = index;
			}
		}

		if (clip_count == 2)
		{
			if (lowest_clip != NONE)
			{
				struct collision_plane const *downmost_clip_collision = &collisions[clip_collisions[lowest_clip]];

				if (lowest_clip == 0)
				{
					cross_product3d(&clip_line_vector, &downmost_clip_collision->plane.n, &composite->plane.n);
				}
				else
				{
					cross_product3d(&downmost_clip_collision->plane.n, &clip_line_vector, &composite->plane.n);
				}
			}
			else
			{
				point_from_line3d((real_point3d const *)global_up3d, &clip_line_vector, -clip_line_vector.k / magnitude_squared3d(&clip_line_vector), (real_point3d *)&composite->plane.n);
			}

			if (normalize3d(&composite->plane.n) != 0.f)
			{
				composite->plane.d = dot_product3d((real_vector3d const *)&clip_line_point, &composite->plane.n);
			}
			else
			{
				collision_count--;
			}
		}
		else if (lowest_clip != NONE)
		{
			struct collision_plane *downmost_clip_collision = &collisions[clip_collisions[lowest_clip]];

			point_from_line3d((real_point3d const *)global_up3d, &downmost_clip_collision->plane.n, (-downmost_clip_collision->plane.n.k), (real_point3d *)&composite->plane.n);

			if (normalize3d(&composite->plane.n) != 0.f)
			{
				composite->plane.d = dot_product3d((real_vector3d const *)&clip_point, &composite->plane.n);
			}
			else
			{
				collision_count--;
			}
		}
		else
		{
			composite->plane.n = *global_up3d;
			composite->plane.d = dot_product3d((real_vector3d const *)&clip_point, &composite->plane.n);
		}
	}

	match_assert_valid_real_point3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1175, new_position);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\physics\\collisions.c", 1176, new_velocity);

	return collision_count;
}

short collision_move_pill(
	unsigned long flags,
	real_point3d const *old_position,
	real_vector3d const *old_velocity,
	real height,
	real width,
	long ignore_object_index,
	real_point3d *new_position,
	real_vector3d *new_velocity,
	short maximum_collision_count,
	struct collision_plane *collisions)
{
	struct collision_feature_list features;
	real_point3d test_center;
	real test_radius;

	short collision_count = 0;

#ifdef DEBUG
	match_assert("c:\\halo\\SOURCE\\physics\\collisions.c", 1214, global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH);
	
	global_current_collision_users[global_current_collision_user_depth++] = _collision_user_bipeds;
#endif
	point_from_line3d(old_position, old_velocity, 0.5f, &test_center);
	test_center.z += height * 0.5f;
	test_radius = magnitude3d(old_velocity) * 0.5f + height * 0.5f + width;

	if (collision_get_features_in_sphere(flags, &test_center, test_radius, height, width, ignore_object_index, &features))
	{
		collision_count = collision_move_point(old_position, old_velocity, &features, new_position, new_velocity, maximum_collision_count, collisions);
	}
	else
	{
		point_from_line3d(old_position, old_velocity, 1.f, new_position);
		*new_velocity = *old_velocity;
	}

#ifdef DEBUG
	match_assert("c:\\halo\\SOURCE\\physics\\collisions.c", 1230, global_current_collision_user_depth > 1);
	global_current_collision_user_depth--;
#endif

	return collision_count;
}

short collision_move_sphere(
	unsigned long flags,
	real_point3d const *old_position,
	real_vector3d const *old_velocity,
	real radius,
	long ignore_object_index,
	real_point3d *new_position,
	real_vector3d *new_velocity,
	short maximum_collision_count,
	struct collision_plane *collisions)
{
	return collision_move_pill(flags, old_position, old_velocity, 0.f, radius, ignore_object_index, new_position, new_velocity, maximum_collision_count, collisions);
}

/* ---------- private code */

static boolean object_test_point(
	long object_index,
	unsigned long flags,
	real_point3d const *point,
	long ignore_object_index)
{
	boolean hit = FALSE;

	do
	{
		struct object_datum *object = object_get(object_index);

		if (object_index != ignore_object_index && !TEST_FLAG(object->object.flags, _object_invisible_bit) &&
			TEST_FLAG(flags, _collision_test_objects_first_type_bit + object->object.type) &&
			point_in_sphere(point, &object->object.bounding_sphere_center, object->object.bounding_sphere_radius))
		{
			if (TEST_FLAG(_object_mask_vehicle, object->object.type) && TEST_FLAG(flags, _collision_test_use_vehicle_physics_bit))
			{
				struct physics_instance instance;

				if (physics_instance_new(&instance, object_index) && physics_test_point(&instance, point))
				{
					hit = TRUE;
					break;
				}
			}
			else
			{
				struct collision_model_instance instance;

				if (collision_model_instance_new(&instance, object_index) && collision_model_test_point(&instance, point))
				{
					hit = TRUE;
					break;
				}
			}

			if (object->object.first_child_object_index != NONE && object_test_point(object->object.first_child_object_index, flags, point, ignore_object_index))
			{
				hit = TRUE;
				break;
			}
		}

		object_index = object->object.next_object_index;
	}
	while (object_index != NONE);

	return hit;
}

static boolean object_test_vector(
	long object_index,
	unsigned long flags,
	unsigned long bsp_flags,
	real_point3d const *point,
	real_vector3d const *vector,
	long ignore_object_index,
	struct collision_result *collision)
{
	boolean hit = FALSE;

	do
	{
		struct object_datum *object = object_get(object_index);

		if (object_index != ignore_object_index && !TEST_FLAG(object->object.flags, _object_invisible_bit) &&
			TEST_FLAG(flags, _collision_test_objects_first_type_bit + object->object.type) &&
			fast_vector_intersects_sphere(point, vector, &object->object.bounding_sphere_center, (object->object.bounding_sphere_radius)))
		{
			if (TEST_FLAG(_object_mask_vehicle, object->object.type) && TEST_FLAG(flags, _collision_test_use_vehicle_physics_bit))
			{
				struct physics_instance instance;
				struct physics_test_vector_result result;

				if (physics_instance_new(&instance, object_index) && physics_test_vector(&instance, point, vector, &result) && collision->t > result.t)
				{
					collision->type = _collision_result_object;
					collision->t = result.t;
					collision->plane = result.plane;
					collision->material_type = NONE;
					collision->object_index = object_index;
					collision->region_index = NONE;
					collision->node_index = NONE;
					collision->bsp_index = NONE;
					collision->surface_index = NONE;
					collision->plane_designator = NONE;
					collision->flags = 0;
					collision->breakable_surface_index = 0;
					collision->material_index = NONE;
					hit = TRUE;
				}
			}
			else
			{
				struct collision_model_instance instance;
				struct collision_model_test_vector_result result;

				if (collision_model_instance_new(&instance, object_index) && collision_model_test_vector(&instance, bsp_flags, point, vector, &result) && collision->t > result.bsp_result.t)
				{
					collision->type = _collision_result_object;
					collision->t = result.bsp_result.t;
					matrix4x3_transform_plane(&instance.matrices[result.node_index], result.bsp_result.plane, &collision->plane);
					if (result.bsp_result.plane_designator & LONG_MIN)
					{
						plane3d_negate(&collision->plane, &collision->plane);
					}
					collision->material_type = collision_model_get_material_type(instance.model, result.bsp_result.material_index);
					collision->object_index = object_index;
					collision->region_index = result.region_index;
					collision->node_index = result.node_index;
					collision->bsp_index = result.bsp_index;
					collision->surface_index = result.bsp_result.surface_index;
					collision->plane_designator = result.bsp_result.plane_designator;
					collision->flags = result.bsp_result.flags;
					collision->breakable_surface_index = result.bsp_result.breakable_surface_index;
					collision->material_index = result.bsp_result.material_index;
					hit = TRUE;
				}
			}
			if (object->object.first_child_object_index != NONE && object_test_vector(object->object.first_child_object_index, flags, bsp_flags, point, vector, ignore_object_index, collision))
			{
				hit = TRUE;
			}
		}

		object_index = object->object.next_object_index;
	}
	while (object_index != NONE);

	return hit;
}

static void object_get_features_in_sphere(
	unsigned long flags,
	long object_index,
	real_point3d const *center,
	real radius,
	real height,
	real width,
	long ignore_object_index,
	struct collision_feature_list *features)
{
	do
	{
		struct object_datum *object = object_get(object_index);

		if (object_index != ignore_object_index &&
			!TEST_FLAG(object->object.flags, _object_invisible_bit) &&
			!TEST_FLAG(object->object.flags, _object_no_collisions_bit) &&
			(!TEST_FLAG(object->object.damage_flags, _object_dead_bit) || object->object.type != _object_type_biped))
		{
			real object_radius = object->object.bounding_sphere_radius;

			if (point_in_sphere(center, &object->object.bounding_sphere_center, object_radius + radius))
			{
				if (TEST_FLAG(flags, _collision_test_objects_first_type_bit + object->object.type))
				{
					switch (object->object.type)
					{
					case _object_type_biped:
						if ((!TEST_FLAG(flags, _collision_test_skip_passthrough_bipeds_bit) || !TEST_FLAG(((struct biped_datum *)object)->biped.flags, _biped_movement_passes_through_bipeds_bit)) &&
							(object->object.parent_object_index == NONE || ((struct biped_datum *)object)->unit.parent_seat_index == NONE))
						{
							real_point3d biped_base;
							real biped_height, biped_width;

							biped_get_physics_pill(object_index, &biped_base, &biped_height, &biped_width);
							biped_base.z += biped_height;
							collision_features_from_point(&biped_base, biped_height + height, biped_width + width, object_index, NONE, 0, NONE, NONE, features);
						}
						break;
					case _object_type_vehicle:
					case _object_type_scenery:
					case _object_type_machine:
					case _object_type_control:
						if (TEST_FLAG(_object_mask_vehicle, object->object.type) && TEST_FLAG(flags, _collision_test_use_vehicle_physics_bit))
						{
							struct physics_instance instance;

							if (physics_instance_new(&instance, object_index))
							{
								physics_get_features_in_sphere(&instance, center, radius, height, width, features);
							}
						}
						else
						{
							struct collision_model_instance instance;

							if (collision_model_instance_new(&instance, object_index))
							{
								collision_model_get_features_in_sphere(&instance, center, radius, height, width, features);
							}
						}
						break;
					}
				}

				if (object->object.first_child_object_index != NONE)
				{
					object_get_features_in_sphere(flags, object->object.first_child_object_index, center, radius, height, width, ignore_object_index, features);
				}
			}
		}

		object_index = object->object.next_object_index;
	}
	while (object_index != NONE);

	return;
}

static void clip_position_to_plane(
	real_point3d *position,
	real_plane3d const *plane,
	real_point3d *result)
{
	point_from_line3d(position, &plane->n, -plane3d_distance_to_point(plane, position), result);

	return;
}

static void clip_velocity_to_plane(
	real_vector3d const *velocity,
	real_plane3d const *plane,
	real_vector3d *result)
{
	point_from_line3d((real_point3d const *)velocity, &plane->n, -dot_product3d(velocity, &plane->n), (real_point3d *)result);

	return;
}

static void clip_position_to_line(
	real_point3d const *position,
	real_point3d const *point,
	real_vector3d const *vector,
	real_point3d *result)
{
	real_vector3d offset;

	vector_from_points3d(point, position, &offset);
	point_from_line3d(point, vector, dot_product3d(&offset, vector) / magnitude_squared3d(vector), result);

	return;
}

static void clip_velocity_to_line(
	real_vector3d const *velocity,
	real_point3d const *point,
	real_vector3d const *vector,
	real_vector3d *result)
{
	scale_vector3d(vector, dot_product3d(velocity, vector) / magnitude_squared3d(vector), result);

	return;
}

static void collision_fix_pill_nudge_collision(
	unsigned long flags,
	real_point3d const *point,
	real_vector3d const *vector,
	long ignore_object_index,
	struct collision_plane *collision)
{
	while (collision->t > 0.f && collision_test_point(flags, &collision->point, ignore_object_index))
	{
		collision->t -= 1.f / 32.f;
		point_from_line3d(point, vector, collision->t, &collision->point);
	}

	if (collision->t <= 0.f)
	{
		collision->point = *point;
	}

	return;
}
