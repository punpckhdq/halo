/*
PATH_OBSTACLES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "path.h"
#include "collision_bsp_definitions.h"
#include "object_lights.h"
#include "devices.h"
#include "device_definitions.h"
#include "collision_model_definitions.h"
#include "collisions.h"
#include "render_debug.h"

/* ---------- prototypes */

static boolean circle_intersect_ray(real_point2d const *center, real radius, real_point2d const *point, real_vector2d const *direction, real *distance);
static void circle_tangents(real_vector2d const *direction, real distance, real radius, real_vector2d *right_direction, real_vector2d *left_direction, real *tangent_distance);

/* ---------- globals */

static real_argb_color obstacle_colors[MAXIMUM_DISC_COUNT] = /* fake name */
{
	{ 1.f, 1.f, 1.f, .5f },
	{ 1.f, 1.f, 1.f, 0.f },
	{ 1.f, 1.f, .5f, 1.f },
	{ 1.f, 1.f, .5f, .5f },
	{ 1.f, 1.f, .5f, 0.f },
	{ 1.f, 1.f, 0.f, 1.f },
	{ 1.f, 1.f, 0.f, .5f },
	{ 1.f, 1.f, 0.f, 0.f },
	{ 1.f, .5f, 1.f, 1.f },
	{ 1.f, .5f, 1.f, .5f },
	{ 1.f, .5f, 1.f, 0.f },
	{ 1.f, .5f, .5f, 1.f },
	{ 1.f, .5f, .5f, .5f },
	{ 1.f, .5f, .5f, 0.f },
	{ 1.f, .5f, 0.f, 1.f },
	{ 1.f, .5f, 0.f, .5f },
	{ 1.f, .5f, 0.f, 0.f },
	{ 1.f, 0.f, 1.f, 1.f },
	{ 1.f, 0.f, 1.f, .5f },
	{ 1.f, 0.f, 1.f, 0.f },
	{ 1.f, 0.f, .5f, 1.f },
	{ 1.f, 0.f, .5f, .5f },
	{ 1.f, 0.f, .5f, 0.f },
	{ 1.f, 0.f, 0.f, 1.f },
	{ 1.f, 0.f, 0.f, .5f }
};

/* ---------- public code */

void obstacles_new(
	struct obstacles *obstacles)
{
	obstacles->obstacle_count = 0;
	obstacles->disc_count = 0;
	obstacles->disc_optional_count = 0;

	return;
}

static boolean circle_intersect_ray(
	real_point2d const *center,
	real radius,
	real_point2d const *point,
	real_vector2d const *direction,
	real *distance)
{
	real_vector2d vector;
	real projection;

	vector_from_points2d(point, center, &vector);
	projection = dot_product2d(&vector, direction);

	if (projection > 0.f)
	{
		real discriminant;
		real c = magnitude_squared2d(&vector) - radius * radius;

		if (c <= 0.f)
		{
			*distance = 0.f;
			return TRUE;
		}

		discriminant = projection * projection - c;

		if (discriminant >= 0.f)
		{
			*distance = projection - square_root(discriminant);
			return TRUE;
		}
	}

	return FALSE;
}

static void circle_tangents(
	real_vector2d const *direction,
	real distance,
	real radius,
	real_vector2d *right_direction,
	real_vector2d *left_direction,
	real *tangent_distance)
{
	real cosine_angle;
	real sine_angle = radius / distance;

	if (sine_angle > 1.f)
	{
		sine_angle = 1.f;
	}

	cosine_angle = square_root(1.f - sine_angle * sine_angle);
	rotate_vector2d(direction, -sine_angle, cosine_angle, right_direction);
	rotate_vector2d(direction, sine_angle, cosine_angle, left_direction);
	*tangent_distance = distance * cosine_angle;

	return;
}

boolean obstacles_add_disc(
	struct obstacles *obstacles,
	long object_index,
	short flags,
	real_point3d const *center,
	real radius)
{
	struct disc *disc;

	match_assert("c:\\halo\\SOURCE\\ai\\path_obstacles.c", 104, obstacles->disc_count>=0 && obstacles->disc_count<=MAXIMUM_DISC_COUNT);
	match_assert("c:\\halo\\SOURCE\\ai\\path_obstacles.c", 105, obstacles->obstacle_count>=0 && obstacles->obstacle_count<=obstacles->disc_count);

	if (obstacles->disc_count == MAXIMUM_DISC_COUNT)
	{
		return FALSE;
	}

	disc = &obstacles->discs[obstacles->disc_count++];

	if (TEST_FLAG(flags, _disc_optional_bit))
	{
		obstacles->disc_optional_count++;
	}

	disc->flags = flags;
	disc->obstacle_index = NONE;
	disc->object_index = object_index;
	project_point3d(center, _z, TRUE, &disc->center);
	disc->radius = radius;
	disc->z = center->z;

	return TRUE;
}

void obstacles_get_discs_in_sphere(
	struct obstacles *obstacles,
	real_point3d const *center,
	real radius,
	real_vector3d const *movement_direction,
	long ignore_source_object_index,
	long ignore_target_object_index)
{
	long object_indices[256];
	short object_count;
	short object_index_index;

	match_vassert("c:\\halo\\SOURCE\\ai\\path_obstacles.c", 152, ignore_source_object_index != NONE, "matt is bad.");

	object_count = objects_in_sphere(
		FLAG(_object_class_collideable),
		_object_mask_unit | _object_mask_scenery | _object_mask_machine,
		&object_get(ignore_source_object_index)->object.location,
		center,
		radius,
		object_indices,
		NUMBEROF(object_indices));

	for (object_index_index = 0; object_index_index < object_count; object_index_index++)
	{
		long object_index = object_indices[object_index_index];
		struct object_datum *object = object_get(object_index);

		if (object_index != ignore_source_object_index &&
			object_index != ignore_target_object_index &&
			!TEST_FLAG(object->object.flags, _object_invisible_bit) &&
			(object->object.type != _object_type_biped || !TEST_FLAG(object->object.damage_flags, _object_dead_bit)))
		{
			boolean pathfinding_obstacle = TRUE;

			if (object->object.type == _object_type_machine)
			{
				struct machine_definition *machine_definition = machine_definition_get(object->definition_index);

				if (!TEST_FLAG(machine_definition->machine.flags, _machine_is_pathfinding_obstacle_bit) ||
					(TEST_FLAG(machine_definition->machine.flags, _machine_is_not_pathfinding_obstacle_when_open_bit) && ((struct machine_datum *)object)->device.position == 1.f))
				{
					pathfinding_obstacle = FALSE;
				}
			}

			if (pathfinding_obstacle)
			{
				real bounding_sphere_radius = object->object.bounding_sphere_radius;

				if (point_in_sphere(center, &object->object.bounding_sphere_center, bounding_sphere_radius + radius))
				{
					struct object_definition *object_definition = object_definition_get(object->definition_index);
					struct collision_model *collision_model = collision_model_definition_get(object_definition->object.collision_model.index);

					if (!TEST_FLAG(object_definition->object.flags, _object_not_pathfinding_obstacle_bit) && collision_model->pathfinding_spheres.count > 0)
					{
						real_matrix4x3 world_matrix;
						short sphere_index;

						object_get_world_matrix(object_index, &world_matrix);

						for (sphere_index = 0; sphere_index < collision_model->pathfinding_spheres.count; sphere_index++)
						{
							real_point3d transformed_center;
							real sphere_radius;
							struct pathfinding_sphere *sphere = TAG_BLOCK_GET_ELEMENT(&collision_model->pathfinding_spheres, sphere_index, struct pathfinding_sphere);

							if (sphere->node_index != NONE)
							{
								real_matrix4x3 *node_matrix = object_get_node_matrix(object_index, sphere->node_index);

								matrix4x3_transform_point(node_matrix, &sphere->center, &transformed_center);
								sphere_radius = sphere->radius * node_matrix->scale;
							}
							else
							{
								matrix4x3_transform_point(&world_matrix, &sphere->center, &transformed_center);
								sphere_radius = world_matrix.scale * sphere->radius;
							}

							if (!(transformed_center.z + sphere_radius + 0.5f < center->z && movement_direction->k > -0.2f) &&
								!(transformed_center.z - sphere_radius - 0.5f > center->z && movement_direction->k < 0.2f))
							{
								real_vector3d offset;

								vector_from_points3d(center, &transformed_center, &offset);

								if (offset.i * offset.i + offset.j * offset.j + 4.f * offset.k * offset.k <= (sphere_radius + radius) * (sphere_radius + radius))
								{
									short disc_flags = 0;

									if (object->object.type == _object_type_biped &&
										dot_product3d(&offset, movement_direction) > 0.f &&
										dot_product3d(&object->object.translational_velocity, movement_direction) > 2.f / TICKS_PER_SECOND)
									{
										SET_FLAG(disc_flags, _disc_optional_bit, TRUE);
									}

									obstacles_add_disc(obstacles, object_index, disc_flags, &transformed_center, sphere_radius);
								}
							}
						}
					}
				}
			}
		}
	}

	return;
}

short obstacles_test_circle(
	struct obstacles const *obstacles,
	short ignore_disc_index,
	real_point2d const *center,
	real radius)
{
	short disc_index;

	for (disc_index = 0; disc_index < obstacles->disc_count; disc_index++)
	{
		if (disc_index != ignore_disc_index)
		{
			struct disc const *disc = obstacles_get_disc(obstacles, disc_index);
			real disc_radius = disc->radius;

			if (point_in_circle(center, &disc->center, disc_radius + radius))
			{
				return disc_index;
			}
		}
	}

	return NONE;
}

boolean obstacles_test_pill(
	struct obstacles const *obstacles,
	short ignore_disc_index,
	real_point2d const *point,
	real_vector2d const *direction,
	real radius,
	real distance,
	boolean ignore_optional,
	struct obstacles_test_pill_result *result)
{
	short disc_index;

	result->distance = distance;
	result->disc_index = NONE;
	result->obstacle_index = NONE;

	for (disc_index = 0; disc_index < obstacles->disc_count; disc_index++)
	{
		if (disc_index != ignore_disc_index)
		{
			struct disc const *disc = obstacles_get_disc(obstacles, disc_index);

			if (!ignore_optional || !TEST_FLAG(disc->flags, _disc_optional_bit))
			{
				real disc_distance;

				if (circle_intersect_ray(&disc->center, disc->radius + radius, point, direction, &disc_distance))
				{
					if (result->distance > disc_distance)
					{
						result->distance = disc_distance;
						result->disc_index = disc_index;
						result->obstacle_index = disc->obstacle_index;
					}
				}
			}
		}
	}

	return result->disc_index != NONE;
}

void obstacles_disc_tangents(
	struct obstacles const *obstacles,
	short disc_index,
	real_point2d const *point,
	real radius,
	real_vector2d *right_direction,
	real_vector2d *left_direction,
	real *tangent_distance)
{
	real_vector2d direction;
	real distance;
	struct disc const *disc = obstacles_get_disc(obstacles, disc_index);

	vector_from_points2d(point, &disc->center, &direction);
	distance = normalize2d(&direction);
	circle_tangents(&direction, distance, disc->radius + radius + 1.f / 256.f, right_direction, left_direction, tangent_distance);

	return;
}

void obstacles_disc_neighborhood(
	struct obstacles const *obstacles,
	real radius,
	short seed_disc_index,
	byte *disc_flags)
{
	match_assert("c:\\halo\\SOURCE\\ai\\path_obstacles.c", 387, obstacles->disc_count>=0 && obstacles->disc_count<=MAXIMUM_DISC_COUNT);

	memset(disc_flags, 0, BIT_VECTOR_SIZE_IN_BYTES(obstacles->disc_count));

	if (seed_disc_index != NONE)
	{
		short stack[MAXIMUM_DISC_COUNT];
		short stack_top = 0;

		match_assert("c:\\halo\\SOURCE\\ai\\path_obstacles.c", 396, seed_disc_index>=0 && seed_disc_index<obstacles->disc_count);
		BIT_VECTOR_SET_FLAG((unsigned long *)disc_flags, seed_disc_index, TRUE);
		match_assert("c:\\halo\\SOURCE\\ai\\path_obstacles.c", 400, stack_top<MAXIMUM_DISC_COUNT);
		stack[stack_top++] = seed_disc_index;

		while (stack_top > 0)
		{
			short disc_index;
			struct disc const *disc = obstacles_get_disc(obstacles, stack[--stack_top]);

			for (disc_index = 0; disc_index < obstacles->disc_count; disc_index++)
			{
				if (!BIT_VECTOR_TEST_FLAG((unsigned long *)disc_flags, disc_index))
				{
					struct disc const *neighbor = obstacles_get_disc(obstacles, disc_index);

					if (point_in_circle(&disc->center, &neighbor->center, (disc->radius + radius) + (neighbor->radius + radius)))
					{
						BIT_VECTOR_SET_FLAG((unsigned long *)disc_flags, disc_index, TRUE);
						match_assert("c:\\halo\\SOURCE\\ai\\path_obstacles.c", 421, stack_top<MAXIMUM_DISC_COUNT);
						stack[stack_top++] = disc_index;
					}
				}
			}
		}
	}

	return;
}

void obstacles_recompute(
	struct obstacles *obstacles,
	real radius)
{
	short disc_index;

	obstacles->obstacle_count = 0;

	for (disc_index = 0; disc_index < obstacles->disc_count; disc_index++)
	{
		obstacles->discs[disc_index].obstacle_index = NONE;
	}

	for (disc_index = 0; disc_index < obstacles->disc_count; disc_index++)
	{
		if (obstacles->discs[disc_index].obstacle_index == NONE)
		{
			unsigned long obstacle_flags[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_DISC_COUNT)];
			short neighbor_index;
			short obstacle_index = obstacles->obstacle_count++;

			obstacles_disc_neighborhood(obstacles, radius, disc_index, (byte *)obstacle_flags);

			for (neighbor_index = 0; neighbor_index < obstacles->disc_count; neighbor_index++)
			{
				if (BIT_VECTOR_TEST_FLAG(obstacle_flags, neighbor_index))
				{
					obstacles->discs[neighbor_index].obstacle_index = obstacle_index;
				}
			}
		}
	}

	return;
}

void render_debug_obstacles(
	struct obstacles const *obstacles,
	real radius)
{
	short disc_index;

	for (disc_index = 0; disc_index < obstacles->disc_count; disc_index++)
	{
		real_point3d center;
		real_vector3d vector;
		struct collision_result collision;
		struct disc const *disc = obstacles_get_disc(obstacles, disc_index);

		match_assert("c:\\halo\\SOURCE\\ai\\path_obstacles.c", 529, disc->obstacle_index>=0 && disc->obstacle_index<obstacles->obstacle_count);
		set_real_point3d(&center, disc->center.x, disc->center.y, disc->z);
		set_real_vector3d(&vector, 0.f, 0.f, (disc->radius + radius) * -2.f);

		if (collision_test_vector(FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_structure_bit), &center, &vector, NONE, &collision))
		{
			render_debug_circle(TRUE, &collision.plane, _z, TRUE, &disc->center, disc->radius, &obstacle_colors[disc->obstacle_index], 1.f / 64.f);

			if (radius > 0.f)
			{
				render_debug_circle(TRUE, &collision.plane, _z, TRUE, &disc->center, disc->radius + radius, &obstacle_colors[disc->obstacle_index], 1.f / 64.f);
			}
		}
		else
		{
			render_debug_sphere(TRUE, &center, disc->radius, &obstacle_colors[disc->obstacle_index]);

			if (radius > 0.f)
			{
				render_debug_sphere(TRUE, &center, disc->radius + radius, &obstacle_colors[disc->obstacle_index]);
			}
		}
	}

	return;
}
