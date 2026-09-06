/*
COLLISION_DEBUG.C
*/

/* ---------- headers */

#include "cseries.h"
#include "game/game.h"
#include "game/players.h"
#include "math/real_math.h"
#include "physics/breakable_surfaces.h"
#include "physics/collision_bsp.h"
#include "physics/collision_bsp_definitions.h"
#include "physics/collision_debug.h"
#include "physics/collision_features.h"
#include "physics/collision_model_definitions.h"
#include "physics/collision_models.h"
#include "physics/collision_usage.h"
#include "physics/collisions.h"
#include "render/render.h"
#include "render/render_debug.h"
#include "scenario/scenario.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

enum
{
	COLLISION_DEBUG_SPRAY_ROWS = 30,
	COLLISION_DEBUG_SPRAY_COLUMNS = 40,
	COLLISION_DEBUG_SPRAY_COUNT = COLLISION_DEBUG_SPRAY_ROWS * COLLISION_DEBUG_SPRAY_COLUMNS,

	MAXIMUM_COLLISION_DEBUG_MOVE_COLLISIONS = 14,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

long local_player_get_player_index(short local_player_index);
const char *material_get_name(short material_type);
void render_debug_string(boolean immediate, const char *string);
void render_debug_collision_surface(struct collision_bsp const *bsp, long surface_index, real_matrix4x3 const *matrix, real_argb_color const *color);
void render_frustum_get_projection_bounds(struct render_frustum const *frustum, real_rectangle2d *projection_bounds);

boolean collision_bsp_test_vector(
	unsigned long flags,
	struct collision_bsp const *bsp,
	short breakable_surface_count,
	byte const *breakable_surface_flags,
	real_point3d const *point,
	real_vector3d const *vector,
	real maximum_t,
	struct collision_bsp_test_vector_result *result);

boolean collision_test_point(unsigned long flags, real_point3d const *point, long ignore_object_index);
boolean collision_test_sphere(real_point3d const *center, real radius, long ignore_object_index);
boolean collision_get_features_in_sphere(unsigned long flags, real_point3d const *center, real radius, real height, real width, long ignore_object_index, struct collision_feature_list *features);
short collision_move_sphere(unsigned long flags, real_point3d const *old_position, real_vector3d const *old_velocity, real radius, long ignore_object_index, real_point3d *new_position, real_vector3d *new_velocity, short maximum_count, struct collision_plane *collisions);
short collision_move_pill(unsigned long flags, real_point3d const *old_position, real_vector3d const *old_velocity, real height, real width, long ignore_object_index, real_point3d *new_position, real_vector3d *new_velocity, short maximum_count, struct collision_plane *collisions);

/* ---------- globals */

boolean collision_debug;
boolean collision_debug_spray;
boolean collision_debug_features;
boolean collision_debug_repeat;
boolean collision_debug_flag_back_facing_surfaces;
boolean collision_debug_flag_ignore_two_sided_surfaces;
boolean collision_debug_flag_ignore_breakable_surfaces;
boolean collision_debug_flag_objects_bipeds;
boolean collision_debug_flag_objects_vehicles;
boolean collision_debug_flag_objects_weapons;
boolean collision_debug_flag_objects_equipment;
boolean collision_debug_flag_objects_projectiles;
boolean collision_debug_flag_objects_scenery;
boolean collision_debug_flag_objects_machines;
boolean collision_debug_flag_objects_controls;
boolean collision_debug_flag_objects_light_fixtures;
boolean collision_debug_flag_objects_placeholders;
boolean collision_debug_flag_try_to_keep_location_valid;
boolean collision_debug_flag_skip_passthrough_bipeds;
boolean collision_debug_flag_use_vehicle_physics;
unsigned long collision_debug_flags;
real_point3d collision_debug_point;
real_vector3d collision_debug_vector;
real_point3d collision_debug_phantom_bsp_point;

boolean collision_debug_flag_front_facing_surfaces = TRUE;
boolean collision_debug_flag_ignore_invisible_surfaces = TRUE;
boolean collision_debug_flag_structure = TRUE;
boolean collision_debug_flag_media = TRUE;
boolean collision_debug_flag_objects = TRUE;
real collision_debug_length = 100.0f;
long collision_debug_ignore_object_index = NONE;

static real_vector3d collision_debug_spray_vectors[COLLISION_DEBUG_SPRAY_ROWS][COLLISION_DEBUG_SPRAY_COLUMNS] = { 0 };
static real_point3d collision_debug_spray_points[COLLISION_DEBUG_SPRAY_ROWS][COLLISION_DEBUG_SPRAY_COLUMNS] = { 0 };
static unsigned long collision_debug_spray_hits[BIT_VECTOR_SIZE_IN_LONGS(COLLISION_DEBUG_SPRAY_COUNT)] = { 0 };

real collision_debug_width = 0.0f;
real collision_debug_height = 0.0f;
boolean collision_debug_phantom_bsp = FALSE;
boolean collision_debug_phantom_bsp_found = FALSE;

static const real_vector3d collision_debug_cube_vectors[8] =
{
	{ -1.0f, -1.0f, -1.0f },
	{ -1.0f, -1.0f,  1.0f },
	{ -1.0f,  1.0f, -1.0f },
	{ -1.0f,  1.0f,  1.0f },
	{  1.0f, -1.0f, -1.0f },
	{  1.0f, -1.0f,  1.0f },
	{  1.0f,  1.0f, -1.0f },
	{  1.0f,  1.0f,  1.0f },
};

/* ---------- public code */

void collision_debug_render(void)
{
	unsigned long flags;
	long ignore_object_index;
	real_point3d point;
	real_vector3d vector;

	match_assert("c:\\halo\\SOURCE\\physics\\collision_debug.c", 76, global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH);
	global_current_collision_users[global_current_collision_user_depth++] = _collision_user_debugging;

	if (collision_debug || collision_debug_spray || collision_debug_features || collision_debug_phantom_bsp)
	{
		flags = collision_debug_flag_front_facing_surfaces != FALSE;
		SET_FLAG(flags, _collision_test_back_facing_surfaces_bit, collision_debug_flag_back_facing_surfaces);
		SET_FLAG(flags, _collision_test_ignore_two_sided_surfaces_bit, collision_debug_flag_ignore_two_sided_surfaces);
		SET_FLAG(flags, _collision_test_ignore_invisible_surfaces_bit, collision_debug_flag_ignore_invisible_surfaces);
		SET_FLAG(flags, _collision_test_ignore_breakable_surfaces_bit, collision_debug_flag_ignore_breakable_surfaces);
		SET_FLAG(flags, _collision_test_structure_bit, collision_debug_flag_structure);
		SET_FLAG(flags, _collision_test_media_bit, collision_debug_flag_media);
		SET_FLAG(flags, _collision_test_objects_bit, collision_debug_flag_objects);
		SET_FLAG(flags, _collision_test_objects_bipeds_bit, collision_debug_flag_objects_bipeds);
		SET_FLAG(flags, _collision_test_objects_vehicles_bit, collision_debug_flag_objects_vehicles);
		SET_FLAG(flags, _collision_test_objects_weapons_bit, collision_debug_flag_objects_weapons);
		SET_FLAG(flags, _collision_test_objects_equipment_bit, collision_debug_flag_objects_equipment);
		SET_FLAG(flags, _collision_test_objects_projectiles_bit, collision_debug_flag_objects_projectiles);
		SET_FLAG(flags, _collision_test_objects_scenery_bit, collision_debug_flag_objects_scenery);
		SET_FLAG(flags, _collision_test_objects_machines_bit, collision_debug_flag_objects_machines);
		SET_FLAG(flags, _collision_test_objects_controls_bit, collision_debug_flag_objects_controls);
		SET_FLAG(flags, _collision_test_objects_light_fixtures_bit, collision_debug_flag_objects_light_fixtures);
		SET_FLAG(flags, _collision_test_objects_placeholders_bit, collision_debug_flag_objects_placeholders);
		SET_FLAG(flags, _collision_test_try_to_keep_location_valid_bit, collision_debug_flag_try_to_keep_location_valid);
		SET_FLAG(flags, _collision_test_skip_passthrough_bipeds_bit, collision_debug_flag_skip_passthrough_bipeds);
		SET_FLAG(flags, _collision_test_use_vehicle_physics_bit, collision_debug_flag_use_vehicle_physics);

		if (collision_debug_flags)
			flags = collision_debug_flags;

		if (!collision_debug_repeat)
		{
			if (local_player_get_player_index(render.local_player_index) == NONE)
				collision_debug_ignore_object_index = NONE;
			else
				collision_debug_ignore_object_index = player_get(local_player_get_player_index(render.local_player_index))->unit_index;
			collision_debug_point = render.camera.position;
			collision_debug_vector = render.camera.forward;
		}

		point = collision_debug_point;
		vector = collision_debug_vector;
		scale_vector3d(&vector, (real)fabs(collision_debug_length), &vector);
		ignore_object_index = collision_debug_ignore_object_index;
	}

	if (collision_debug_phantom_bsp)
	{
		struct collision_bsp_test_vector_result result;

		if (collision_bsp_test_vector(flags, global_collision_bsp_get(), 256, breakable_surface_flags_get(), &point, &vector, 1.0f, &result))
		{
			real_point3d collision_point;
			struct collision_feature_list features;

			point_from_line3d(&point, &vector, result.t, &collision_point);
			if (!collision_get_features_in_sphere(flags, &collision_point, 0.01f, 0.0f, 0.01f, ignore_object_index, &features))
			{
				collision_debug_phantom_bsp_found = TRUE;
				collision_debug_phantom_bsp_point = collision_point;
			}
		}

		if (collision_debug_phantom_bsp_found)
		{
			real_matrix4x3 matrix;
			real_point3d points[8];
			real yaw;
			real pitch;
			real roll;
			short index;

			yaw = (real)fmod((real)game_time_get() * 1.0f * _pi / 30.0f, 2.0f * _pi);
			pitch = (real)fmod((real)game_time_get() * -0.7f * _pi / 30.0f, 2.0f * _pi);
			roll = (real)fmod((real)game_time_get() * 1.4f * _pi / 30.0f, 2.0f * _pi);
			render_debug_string_at_point(TRUE, &collision_debug_phantom_bsp_point, "phantom bsp", global_real_argb_pink);
			matrix4x3_rotation_from_angles(&matrix, yaw, pitch, roll);

			for (index = 0; index < NUMBEROF(points); index++)
			{
				real_vector3d cube_vector;

				matrix4x3_transform_vector(&matrix, &collision_debug_cube_vectors[index], &cube_vector);
				point_from_line3d(&collision_debug_phantom_bsp_point, &cube_vector, 0.15f, &points[index]);
			}

			render_debug_line(TRUE, &points[0], &points[1], global_real_argb_pink);
			render_debug_line(TRUE, &points[1], &points[3], global_real_argb_pink);
			render_debug_line(TRUE, &points[3], &points[2], global_real_argb_pink);
			render_debug_line(TRUE, &points[2], &points[0], global_real_argb_pink);
			render_debug_line(TRUE, &points[4], &points[5], global_real_argb_pink);
			render_debug_line(TRUE, &points[5], &points[7], global_real_argb_pink);
			render_debug_line(TRUE, &points[7], &points[6], global_real_argb_pink);
			render_debug_line(TRUE, &points[6], &points[4], global_real_argb_pink);
			render_debug_line(TRUE, &points[0], &points[4], global_real_argb_pink);
			render_debug_line(TRUE, &points[1], &points[5], global_real_argb_pink);
			render_debug_line(TRUE, &points[2], &points[6], global_real_argb_pink);
			render_debug_line(TRUE, &points[3], &points[7], global_real_argb_pink);
		}
	}

	if (collision_debug)
	{
		if (collision_debug_length <= 0.0f)
		{
			add_vectors3d((real_vector3d *)&point, &vector, (real_vector3d *)&point);
			if (collision_debug_width <= 0.0f)
			{
				if (collision_test_point(flags, &point, ignore_object_index))
					render_debug_point(TRUE, &point, 0.1f, global_real_argb_red);
				else
					render_debug_point(TRUE, &point, 0.1f, global_real_argb_green);
			}
			else
			{
				if (collision_test_sphere(&point, collision_debug_width, ignore_object_index))
					render_debug_sphere(TRUE, &point, collision_debug_width, global_real_argb_red);
				else
					render_debug_sphere(TRUE, &point, collision_debug_width, global_real_argb_green);
			}
		}
		else if (collision_debug_width <= 0.0f)
		{
			struct collision_result result;
			if (collision_test_vector(flags, &point, &vector, ignore_object_index, &result))
			{
				struct collision_bsp *bsp = 0;
				real_matrix4x3 const *matrix = 0;

				render_debug_vector(TRUE, &point, &vector, result.t, global_real_argb_red);
				render_debug_point(TRUE, &result.point, 0.125f, global_real_argb_red);
				render_debug_vector(TRUE, &result.point, &result.plane.n, 0.25f, global_real_argb_red);

				if (result.type == _collision_result_structure)
				{
					bsp = global_collision_bsp_get();
				}
				else if (result.type == _collision_result_object && result.node_index != NONE)
				{
					struct collision_model_instance instance;
					if (collision_model_instance_new(&instance, result.object_index))
					{
						struct collision_node *node = TAG_BLOCK_GET_ELEMENT(&instance.model->nodes, result.node_index, struct collision_node);
						bsp = TAG_BLOCK_GET_ELEMENT(&node->bsps, result.bsp_index, struct collision_bsp);
						matrix = &instance.matrices[result.node_index];
					}
				}

				if (bsp)
				{
					char string[2048];
					struct collision_surface *surface;

					surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, result.surface_index, struct collision_surface);
					render_debug_collision_surface(bsp, result.surface_index, matrix, global_real_argb_red);
					_snprintf(string, sizeof(string),
						"plane #%d%s|nsurface #%d%s%s%s%s|n%s|n%f degrees",
						surface->plane_designator & LONG_MAX,
						TEST_FLAG(surface->plane_designator, 31) ? " negated" : "",
						result.surface_index,
						TEST_FLAG(result.flags, 0) ? " two-sided" : "",
						TEST_FLAG(result.flags, 1) ? " invisible" : "",
						TEST_FLAG(result.flags, 2) ? " climbable" : "",
						TEST_FLAG(result.flags, 3) ? " breakable" : "",
						material_get_name(result.material_type),
						arccosine(result.plane.n.k) * 360.0f / (2.0f * _pi));
					render_debug_string(TRUE, string);
				}
			}
			else
			{
				render_debug_line(TRUE, &point, &result.point, global_real_argb_green);
				render_debug_point(TRUE, &result.point, 0.125f, global_real_argb_green);
			}
		}
		else
		{
			real_vector3d old_velocity;
			real_point3d old_position;
			real_vector3d new_velocity;
			real_point3d new_position;
			struct collision_plane collisions[MAXIMUM_COLLISION_DEBUG_MOVE_COLLISIONS + 2];
			short count;
			short index;

			old_position = point;
			old_velocity = vector;
			if (collision_debug_height <= 0.0f)
				count = collision_move_sphere(flags, &old_position, &old_velocity, collision_debug_width, ignore_object_index, &new_position, &new_velocity, MAXIMUM_COLLISION_DEBUG_MOVE_COLLISIONS, collisions);
			else
				count = collision_move_pill(flags, &old_position, &old_velocity, collision_debug_height, collision_debug_width, ignore_object_index, &new_position, &new_velocity, MAXIMUM_COLLISION_DEBUG_MOVE_COLLISIONS, collisions);

			render_debug_vector(TRUE, &old_position, &old_velocity, 1.0f, global_real_argb_blue);
			match_assert("c:\\halo\\SOURCE\\physics\\collision_debug.c", 297, count<=14);

			csmemmove(collisions + 1, collisions, sizeof(*collisions) * count);
			collisions[0].point = old_position;
			csmemset(&collisions[0].plane, 0, sizeof(collisions[0].plane));
			count++;
			collisions[count].point = new_position;
			csmemset(&collisions[count].plane, 0, sizeof(collisions[count].plane));
			count++;

			for (index = 0; index < count; index++)
			{
				render_debug_point(TRUE, &collisions[index].point, 0.0625f, global_real_argb_red);
				if (index > 0)
					render_debug_line(TRUE, &collisions[index - 1].point, &collisions[index].point, global_real_argb_red);
				render_debug_vector(TRUE, &collisions[index].point, &collisions[index].plane.n, 0.125f, global_real_argb_red);
			}
			render_debug_vector(TRUE, &new_position, &new_velocity, 1.0f, global_real_argb_green);
		}
	}

	if (collision_debug_spray)
	{
		if (collision_debug_length <= 0.0f)
		{
			real_point3d center;
			real_point3d spray_point;

			point_from_line3d(&render.camera.position, &render.camera.forward, 2.0f, &center);
			for (spray_point.z = center.z - 0.125f; spray_point.z <= center.z + 0.125f; spray_point.z += 0.0625f)
			{
				for (spray_point.y = center.y - 0.125f; spray_point.y <= center.y + 0.125f; spray_point.y += 0.0625f)
				{
					for (spray_point.x = center.x - 0.125f; spray_point.x <= center.x + 0.125f; spray_point.x += 0.0625f)
					{
						render_debug_point(TRUE, &spray_point, 0.0625f,
							collision_test_point(flags, &spray_point, NONE) ? global_real_argb_red : global_real_argb_green);
					}
				}
			}
		}
		else
		{
			if (!collision_debug_repeat)
			{
				short row;
				short column;
				real_vector3d column_step;
				real_vector3d row_step;
				real_vector3d upper_left;

				{
					real_rectangle2d bounds;

					render_frustum_get_projection_bounds(&render.frustum, &bounds);
					set_real_vector3d(&column_step, (bounds.x1 - bounds.x0) * collision_debug_length / COLLISION_DEBUG_SPRAY_COLUMNS, 0.0f, 0.0f);
					set_real_vector3d(&row_step, 0.0f, (bounds.y1 - bounds.y0) * collision_debug_length / COLLISION_DEBUG_SPRAY_ROWS, 0.0f);
					set_real_vector3d(&upper_left, bounds.x0 * collision_debug_length, bounds.y0 * collision_debug_length, -collision_debug_length);
				}

				matrix4x3_transform_vector(&render.frustum.view_to_world, &column_step, &column_step);
				matrix4x3_transform_vector(&render.frustum.view_to_world, &row_step, &row_step);
				matrix4x3_transform_vector(&render.frustum.view_to_world, &upper_left, &upper_left);

				for (row = 0; row < COLLISION_DEBUG_SPRAY_ROWS; row++)
				{
					for (column = 0; column < COLLISION_DEBUG_SPRAY_COLUMNS; column++)
					{
						real_vector3d spray_vector;
						struct collision_result result;

						spray_vector.i = column_step.i * column + row_step.i * row + upper_left.i;
						spray_vector.j = column_step.j * column + row_step.j * row + upper_left.j;
						spray_vector.k = column_step.k * column + row_step.k * row + upper_left.k;
						if (collision_test_vector(flags, &render.camera.position, &spray_vector, ignore_object_index, &result))
						{
							BIT_VECTOR_SET_FLAG(collision_debug_spray_hits, column + COLLISION_DEBUG_SPRAY_COLUMNS * row, TRUE);
							collision_debug_spray_points[row][column] = result.point;
							collision_debug_spray_vectors[row][column] = result.plane.n;
						}
						else
						{
							BIT_VECTOR_SET_FLAG(collision_debug_spray_hits, column + COLLISION_DEBUG_SPRAY_COLUMNS * row, FALSE);
						}
					}
				}
			}

			{
				short row;
				short column;

				for (row = 0; row < COLLISION_DEBUG_SPRAY_ROWS; row++)
				{
					for (column = 0; column < COLLISION_DEBUG_SPRAY_COLUMNS; column++)
					{
						if (BIT_VECTOR_TEST_FLAG(collision_debug_spray_hits, column + COLLISION_DEBUG_SPRAY_COLUMNS * row))
						{
							render_debug_point(TRUE, &collision_debug_spray_points[row][column], 0.03125f, global_real_argb_red);
							render_debug_vector(TRUE, &collision_debug_spray_points[row][column], &collision_debug_spray_vectors[row][column], 0.0625f, global_real_argb_red);
						}
					}
				}
			}
		}
	}

	if (collision_debug_features)
	{
		struct collision_feature_list features;
		real_point3d center;
		real radius;

		point_from_line3d(&collision_debug_point, &collision_debug_vector, 0.5f, &center);
		center.z += collision_debug_height * 0.5f;
		radius = magnitude3d(&collision_debug_vector) * 0.5f + collision_debug_height * 0.5f + collision_debug_width;
		if (collision_get_features_in_sphere(flags, &center, radius, collision_debug_height, collision_debug_width, collision_debug_ignore_object_index, &features))
			render_debug_collision_features(&features);
	}

	match_assert("c:\\halo\\SOURCE\\physics\\collision_debug.c", 443, global_current_collision_user_depth > 1);
	--global_current_collision_user_depth;

	return;
}

/* ---------- private code */
