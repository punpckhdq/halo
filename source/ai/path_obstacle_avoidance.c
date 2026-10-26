/*
PATH_OBSTACLE_AVOIDANCE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ai_scenario_definitions.h"
#include "ai_constants.h"
#include "path.h"
#include "game_globals.h"
#include "interface.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "collision_bsp.h"
#include "structure_bsp_definitions.h"
#include "render_debug.h"
#include "console.h"
#include "input.h"

/* ---------- structures */

struct path_test_pill2d_result
{
	real distance;
	long surface_index;
	long edge_index;
	short disc_index;
	short obstacle_index;
};

/* ---------- prototypes */

static void error_heap(short priority, struct obstacle_path *path);
static boolean heap_verify(struct obstacle_path *path, char const *string);
static void heap_up(struct obstacle_path *path, short heap_index);
static void heap_down(struct obstacle_path *path, short heap_index);
static boolean heap_insert(struct obstacle_path *path, short step_index);
static short heap_remove(struct obstacle_path *path);
static short path_add_step(struct obstacle_path *path, real_point2d const *point, long surface_index, short obstacle_index, byte obstacle_direction_index, real previous_distance, short previous_step_index);
static void path_new(struct obstacle_path *path, struct obstacles const *obstacles, struct structure_bsp const *structure, boolean ignore_broken_surfaces, real radius, real_point2d const *start, long start_surface_index, real_point2d const *goal, long goal_surface_index, boolean finishing_path, boolean ignore_optional);
static boolean path_test_pill2d(struct structure_bsp const *structure, boolean ignore_broken_surfaces, struct obstacles const *obstacles, short ignore_disc_index, real_point2d const *point, long surface_index, real_vector2d const *direction, real radius, real distance, boolean ignore_structure, boolean endpoint_is_goal, boolean ignore_optional, struct path_test_pill2d_result *result);
static void path_add_steps(struct obstacle_path *path, short step_index, short seed_disc_index);
static boolean path_iterate(struct obstacle_path *path);
static boolean path_find(struct obstacle_path *path, boolean ignore_broken_surfaces, struct obstacles const *obstacles, real radius, real_point2d const *start, long start_surface_index, real_point2d const *goal, long goal_surface_index, boolean finishing_path, boolean ignore_optional);

/* ---------- globals */

boolean debug_obstacle_path_on_failure = FALSE;
boolean debug_obstacle_path = FALSE;
real_point3d debug_obstacle_path_start_point;
long debug_obstacle_path_start_surface_index;
real_point3d debug_obstacle_path_goal_point;
long debug_obstacle_path_goal_surface_index;
real debug_obstacle_path_radius;
boolean debug_obstacle_path_finishing;
boolean debug_ignore_broken_surfaces;

static struct obstacles debug_obstacles; /* fake name */
static struct obstacle_path debug_path; /* fake name */

/* ---------- public code */

__inline struct step *path_get_step(
	struct obstacle_path *path,
	short step_index)
{
	match_assert("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 40, step_index>=0 && step_index<path->step_count && path->step_count<=MAXIMUM_OBSTACLE_AVOIDANCE_STEPS);

	return &path->steps[step_index];
}

__inline short path_get_step_index(
	struct obstacle_path *path,
	short heap_index)
{
	match_assert("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 49, heap_index>=0 && heap_index<path->heap_count && path->heap_count<=MAXIMUM_OBSTACLE_AVOIDANCE_STEPS);

	return path->heap[heap_index];
}

__inline short heap_parent_index(
	short heap_index)
{
	match_assert("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 57, heap_index>0);

	return (heap_index - 1) >> 1;
}

__inline short heap_left_index(
	short heap_index)
{
	return 2 * heap_index + 1;
}

__inline short heap_right_index(
	short heap_index)
{
	return 2 * heap_index + 2;
}

__inline real heap_cost(
	struct obstacle_path *path,
	short heap_index)
{
	return path_get_step(path, path_get_step_index(path, heap_index))->total_distance;
}

static void error_heap(
	short priority,
	struct obstacle_path *path)
{
	short heap_index;

	for (heap_index = 0; heap_index < path->heap_count; heap_index++)
	{
		real cost = heap_cost(path, heap_index);

		error(priority, "%3d. %.12g (%x)", heap_index, cost, *(long *)&cost);
	}

	return;
}

static boolean heap_verify(
	struct obstacle_path *path,
	char const *string)
{
	short heap_index;

	for (heap_index = 1; heap_index < path->heap_count; heap_index++)
	{
		if (heap_cost(path, heap_parent_index(heap_index)) > heap_cost(path, heap_index))
		{
			error(_error_silent, "heap_verify failed (%s)", string);
			error(_error_silent, "please give this debug.txt to chucky");
			error_heap(_error_log, path);

			return FALSE;
		}
	}

	return TRUE;
}

static void heap_up(
	struct obstacle_path *path,
	short heap_index)
{
	while (heap_index > 0)
	{
		short parent_index = heap_parent_index(heap_index);

		if (heap_cost(path, parent_index) > heap_cost(path, heap_index))
		{
			short temporary = path->heap[parent_index];

			path->heap[parent_index] = path->heap[heap_index];
			path->heap[heap_index] = temporary;
			heap_index = parent_index;
		}
		else
		{
			break;
		}
	}

	return;
}

static void heap_down(
	struct obstacle_path *path,
	short heap_index)
{
	if (heap_index < path->heap_count)
	{
		while (TRUE)
		{
			short temporary;
			short smallest_index = heap_index;
			short left_index = heap_left_index(heap_index);
			short right_index = heap_right_index(heap_index);

			if (left_index < path->heap_count && heap_cost(path, heap_index) > heap_cost(path, left_index))
			{
				smallest_index = left_index;
			}

			if (right_index < path->heap_count && heap_cost(path, smallest_index) > heap_cost(path, right_index))
			{
				smallest_index = right_index;
			}

			if (smallest_index == heap_index)
			{
				break;
			}

			temporary = path->heap[smallest_index];
			path->heap[smallest_index] = path->heap[heap_index];
			path->heap[heap_index] = temporary;
			heap_index = smallest_index;
		}
	}

	return;
}

static boolean heap_insert(
	struct obstacle_path *path,
	short step_index)
{
	boolean success = FALSE;

	heap_verify(path, "heap_insert input");

	if (path->heap_count < MAXIMUM_OBSTACLE_AVOIDANCE_STEPS)
	{
		short heap_index = path->heap_count++;

		path_get_step(path, step_index);
		path->heap[heap_index] = step_index;
		heap_up(path, heap_index);
		heap_verify(path, "heap_insert output");
		success = TRUE;
	}

	return success;
}

static short heap_remove(
	struct obstacle_path *path)
{
	heap_verify(path, "heap_remove input");

	if (path->heap_count > 0)
	{
		short step_index = path->heap[0];

		path->heap_count--;
		path->heap[0] = path->heap[path->heap_count];
		heap_down(path, 0);
		heap_verify(path, "heap_remove output");

		return step_index;
	}

	return NONE;
}

void render_debug_path(
	struct obstacle_path const *path)
{
	if (path->step_count > 0)
	{
		unsigned long goal_step_flags[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_OBSTACLE_AVOIDANCE_STEPS)];
		short step_index;
		struct collision_bsp *bsp = TAG_BLOCK_GET_ELEMENT(&path->structure->collision_bsp, 0, struct collision_bsp);

		{
			real_point3d start_point;
			real_point3d goal_point;

			collision_surface_project_point2d(bsp, path->steps[0].surface_index, _z, TRUE, &path->steps[0].point, &start_point);
			collision_surface_project_point2d(bsp, path->goal_surface_index, _z, TRUE, &path->goal, &goal_point);
			render_debug_point(TRUE, &start_point, 0.125f, path->goal_step_index != NONE ? global_real_argb_white : global_real_argb_yellow);
			render_debug_point(TRUE, &goal_point, 0.125f, path->goal_step_index != NONE ? global_real_argb_white : global_real_argb_yellow);
		}

		step_index = path->goal_step_index;
		memset(goal_step_flags, 0, BIT_VECTOR_SIZE_IN_BYTES(path->step_count));

		while (step_index != NONE)
		{
			struct step const *step = path_get_step((struct obstacle_path *)path, step_index);

			BIT_VECTOR_SET_FLAG(goal_step_flags, step_index, TRUE);
			step_index = step->previous_step_index;
		}

		for (step_index = 0; step_index < path->step_count; step_index++)
		{
			struct step const *step = path_get_step((struct obstacle_path *)path, step_index);

			if (step->previous_step_index != NONE)
			{
				real_argb_color const *color;
				real_point3d point0;
				real_point3d point1;
				struct step const *previous_step = path_get_step((struct obstacle_path *)path, step->previous_step_index);

				collision_surface_project_point2d(bsp, previous_step->surface_index, _z, TRUE, &previous_step->point, &point0);
				collision_surface_project_point2d(bsp, step->surface_index, _z, TRUE, &step->point, &point1);

				if (BIT_VECTOR_TEST_FLAG(goal_step_flags, step_index))
				{
					color = global_real_argb_white;
				}
				else
				{
					color = step->obstacle_direction_index ? global_real_argb_green : global_real_argb_red;
				}

				render_debug_line(TRUE, &point0, &point1, color);
			}
		}
	}

	return;
}

static short path_add_step(
	struct obstacle_path *path,
	real_point2d const *point,
	long surface_index,
	short obstacle_index,
	byte obstacle_direction_index,
	real previous_distance,
	short previous_step_index)
{
	short step_index = NONE;

	if (path->step_count < MAXIMUM_OBSTACLE_AVOIDANCE_STEPS)
	{
		real_vector2d desired_direction;
		short test_step_index;
		boolean goal_blocked = FALSE;
		boolean valid = TRUE;

		vector_from_points2d(point, &path->goal, &desired_direction);
		test_step_index = previous_step_index;

		while (test_step_index != NONE)
		{
			struct step *test_step = path_get_step(path, test_step_index);

			if (test_step->obstacle_index != obstacle_index)
			{
				if (obstacle_index == path->goal_obstacle_index && path->goal_obstacle_index != NONE)
				{
					short other_step_index = test_step->obstructed_goal_step_indices[obstacle_direction_index == 0];

					goal_blocked = TRUE;

					if (other_step_index != NONE)
					{
						struct step *other_step = path_get_step(path, other_step_index);
						struct step *previous_step = path_get_step(path, previous_step_index);

						if (dot_product2d(&other_step->direction, &desired_direction) > 0.f &&
							cross_product2d(&other_step->direction, &desired_direction) * cross_product2d(&other_step->direction, &previous_step->direction) < 0.f)
						{
							valid = FALSE;
						}
					}

					if (valid &&
						(test_step->obstructed_goal_step_indices[obstacle_direction_index] == previous_step_index ||
						test_step->obstructed_goal_step_indices[obstacle_direction_index] == NONE))
					{
						test_step->obstructed_goal_step_indices[obstacle_direction_index] = path->step_count;
					}
				}

				break;
			}

			if (test_step->obstacle_direction_index != obstacle_direction_index)
			{
				valid = FALSE;
				break;
			}

			test_step_index = test_step->previous_step_index;
		}

		if (valid)
		{
			struct step *step;

			step_index = path->step_count++;
			step = path_get_step(path, step_index);
			step->point = *point;
			step->surface_index = surface_index;
			step->direction = desired_direction;
			step->distance = normalize2d(&step->direction);
			step->obstacle_index = obstacle_index;
			step->obstacle_direction_index = obstacle_direction_index;
			step->total_distance = previous_distance + step->distance;
			step->previous_step_index = previous_step_index;
			memset(step->obstructed_goal_step_indices, NONE, sizeof(step->obstructed_goal_step_indices));

			if (goal_blocked && step->distance < path->best_goal_blocked_distance)
			{
				path->best_goal_blocked_distance = step->distance;
				path->best_goal_blocked_step_index = step_index;
			}

			if (!heap_insert(path, step_index))
			{
				match_halt("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 420);
			}
		}
	}

	return step_index;
}

static void path_new(
	struct obstacle_path *path,
	struct obstacles const *obstacles,
	struct structure_bsp const *structure,
	boolean ignore_broken_surfaces,
	real radius,
	real_point2d const *start,
	long start_surface_index,
	real_point2d const *goal,
	long goal_surface_index,
	boolean finishing_path,
	boolean ignore_optional)
{
	match_assert_valid_real("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 440, radius);
	match_assert_valid_real_point2d("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 441, start);
	match_assert_valid_real_point2d("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 442, goal);

	path->radius = radius;
	path->obstacles = obstacles;
	path->structure = structure;
	path->ignore_broken_surfaces = ignore_broken_surfaces;
	path->goal_found_exactly = FALSE;
	path->goal = *goal;
	path->goal_surface_index = goal_surface_index;
	path->goal_obstacle_index = obstacle_from_disc(obstacles, obstacles_test_circle(obstacles, NONE, goal, radius));
	path->goal_step_index = NONE;
	path->best_goal_blocked_distance = REAL_MAX;
	path->best_goal_blocked_step_index = NONE;
	path->finishing = finishing_path;
	path->ignore_optional = ignore_optional;
	path->step_count = 0;
	path->heap_count = 0;
	path_add_step(path, start, start_surface_index, NONE, 0, 0.f, NONE);

	return;
}

static boolean path_test_pill2d(
	struct structure_bsp const *structure,
	boolean ignore_broken_surfaces,
	struct obstacles const *obstacles,
	short ignore_disc_index,
	real_point2d const *point,
	long surface_index,
	real_vector2d const *direction,
	real radius,
	real distance,
	boolean ignore_structure,
	boolean endpoint_is_goal,
	boolean ignore_optional,
	struct path_test_pill2d_result *result)
{
	boolean blocked;

	result->distance = distance;
	result->surface_index = NONE;
	result->edge_index = NONE;
	result->disc_index = NONE;
	result->obstacle_index = NONE;

	if (endpoint_is_goal)
	{
		result->distance = distance - radius;
	}

	if (!ignore_structure)
	{
		struct structure_test_ray2d_result middle;
		struct structure_test_ray2d_result left;
		struct structure_test_ray2d_result right;
		real_vector2d offset;
		real_point2d left_point;
		real_point2d right_point;

		if (structure_test_ray2d(structure, ignore_broken_surfaces, point, surface_index, direction, result->distance, &middle) && result->distance > middle.distance)
		{
			result->distance = middle.distance;
			result->edge_index = middle.edge_index;
		}

		perpendicular2d(direction, &offset);
		point_from_line2d(point, &offset, radius, &left_point);
		structure_test_ray2d(structure, ignore_broken_surfaces, point, surface_index, &offset, radius, &left);

		if (structure_test_ray2d(structure, ignore_broken_surfaces, &left_point, left.surface_index, direction, result->distance, &left) && result->distance > left.distance)
		{
			result->distance = left.distance;
			result->edge_index = left.edge_index;
		}

		point_from_line2d(point, &offset, -radius, &right_point);
		structure_test_ray2d(structure, ignore_broken_surfaces, point, surface_index, &offset, radius, &right);

		if (structure_test_ray2d(structure, ignore_broken_surfaces, &right_point, right.surface_index, direction, result->distance, &right) && result->distance > right.distance)
		{
			result->distance = right.distance;
			result->edge_index = right.edge_index;
		}
	}

	{
		struct obstacles_test_pill_result obstacle;

		if (obstacles_test_pill(obstacles, ignore_disc_index, point, direction, radius, result->distance, ignore_optional, &obstacle) && result->distance > obstacle.distance)
		{
			result->distance = obstacle.distance;
			result->edge_index = NONE;
			result->disc_index = obstacle.disc_index;
			result->obstacle_index = obstacle.obstacle_index;
		}
	}

	if (result->edge_index == NONE && result->disc_index == NONE)
	{
		blocked = FALSE;
		result->distance = distance;
	}
	else
	{
		blocked = TRUE;
	}

	{
		struct structure_test_ray2d_result final_result;

		structure_test_ray2d(structure, ignore_broken_surfaces, point, surface_index, direction, result->distance, &final_result);
		result->surface_index = final_result.surface_index;
	}

	return blocked;
}

static void path_add_steps(
	struct obstacle_path *path,
	short step_index,
	short seed_disc_index)
{
	unsigned long visited_disc_flags[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_DISC_COUNT)];
	short stack[MAXIMUM_DISC_COUNT];
	struct step *step = path_get_step(path, step_index);
	short stack_top = 0;

	match_assert("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 594, path->obstacles->disc_count>=0 && path->obstacles->disc_count<=MAXIMUM_DISC_COUNT);
	memset(visited_disc_flags, 0, BIT_VECTOR_SIZE_IN_BYTES(path->obstacles->disc_count));
	match_assert("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 597, seed_disc_index>=0 && seed_disc_index<path->obstacles->disc_count);
	BIT_VECTOR_SET_FLAG(visited_disc_flags, seed_disc_index, TRUE);
	match_assert("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 599, stack_top<MAXIMUM_DISC_COUNT);
	stack[stack_top++] = seed_disc_index;

	while (stack_top > 0)
	{
		real tangent_distance;
		real_vector2d direction[2];
		short direction_index;
		short disc_index = stack[--stack_top];
		short obstacle_index = obstacle_from_disc(path->obstacles, disc_index);

		obstacles_disc_tangents(path->obstacles, disc_index, &step->point, path->radius, &direction[0], &direction[1], &tangent_distance);

		if (tangent_distance < path->radius)
		{
			tangent_distance = path->radius;
		}

		for (direction_index = 0; direction_index < (short)NUMBEROF(direction); direction_index++)
		{
			struct path_test_pill2d_result result;

			path_test_pill2d(
				path->structure,
				path->ignore_broken_surfaces,
				path->obstacles,
				disc_index,
				&step->point,
				step->surface_index,
				&direction[direction_index],
				path->radius,
				path->radius + path->radius + tangent_distance,
				FALSE,
				FALSE,
				path->ignore_optional,
				&result);

			if (result.disc_index != NONE)
			{
				match_assert("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 625, result.disc_index>=0 && result.disc_index<path->obstacles->disc_count);

				if (!BIT_VECTOR_TEST_FLAG(visited_disc_flags, result.disc_index))
				{
					BIT_VECTOR_SET_FLAG(visited_disc_flags, result.disc_index, TRUE);
					match_assert("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 631, stack_top<MAXIMUM_DISC_COUNT);
					stack[stack_top++] = result.disc_index;
				}
			}

			if (result.distance > tangent_distance && result.obstacle_index != obstacle_index)
			{
				struct structure_test_ray2d_result structure_result;
				real_point2d point;
				real distance = (result.distance + tangent_distance) * 0.5f;

				structure_test_ray2d(path->structure, path->ignore_broken_surfaces, &step->point, step->surface_index, &direction[direction_index], distance, &structure_result);
				point_from_line2d(&step->point, &direction[direction_index], distance, &point);
				path_add_step(path, &point, structure_result.surface_index, obstacle_index, (byte)direction_index, step->total_distance - step->distance + distance, step_index);
			}
		}
	}

	return;
}

static boolean path_iterate(
	struct obstacle_path *path)
{
	short step_index = heap_remove(path);

	if (step_index != NONE)
	{
		struct path_test_pill2d_result path_result;
		struct step *step = path_get_step(path, step_index);

		path_test_pill2d(
			path->structure,
			path->ignore_broken_surfaces,
			path->obstacles,
			NONE,
			&step->point,
			step->surface_index,
			&step->direction,
			path->radius,
			step->distance,
			step->previous_step_index == NONE,
			TRUE,
			path->ignore_optional,
			&path_result);

		if (path_result.edge_index == NONE)
		{
			if (path_result.disc_index == NONE)
			{
				boolean reached_goal = FALSE;

				if (path_result.surface_index == path->goal_surface_index)
				{
					reached_goal = TRUE;
				}
				else
				{
					reached_goal = structure_surfaces_are_equivalent(path->structure, &path->goal, path->goal_surface_index, path_result.surface_index);
				}

				if (reached_goal)
				{
					real_point2d point;

					point_from_line2d(&step->point, &step->direction, path_result.distance, &point);
					path->goal_step_index = path_add_step(path, &point, path_result.surface_index, NONE, 0, step->total_distance - step->distance + path_result.distance, step_index);
				}
			}
			else
			{
				if (path_result.obstacle_index == path->goal_obstacle_index && step->distance < path->best_goal_blocked_distance)
				{
					path->best_goal_blocked_distance = step->distance;
					path->best_goal_blocked_step_index = step_index;
				}

				path_add_steps(path, step_index, path_result.disc_index);
			}
		}
	}

	return path->goal_step_index == NONE && path->heap_count > 0;
}

static boolean path_find(
	struct obstacle_path *path,
	boolean ignore_broken_surfaces,
	struct obstacles const *obstacles,
	real radius,
	real_point2d const *start,
	long start_surface_index,
	real_point2d const *goal,
	long goal_surface_index,
	boolean finishing_path,
	boolean ignore_optional)
{
	path_new(path, obstacles, global_structure_bsp_get(), ignore_broken_surfaces, radius, start, start_surface_index, goal, goal_surface_index, finishing_path, ignore_optional);

	while (path_iterate(path));

	if (path->goal_step_index != NONE)
	{
		path->goal_found_exactly = TRUE;
	}
	else if (path->best_goal_blocked_step_index != NONE)
	{
		path->goal_step_index = path->best_goal_blocked_step_index;
	}

	return path->goal_step_index != NONE;
}

boolean path_avoid_obstacles(
	struct path_state *state,
	short input_step_count,
	struct path_step const *input_steps,
	short *avoided_step_count,
	struct path_step *avoided_steps,
	boolean *steps_finish_path)
{
	real_point3d last_step_end_point;
	long last_step_end_surface_index;
	short input_step_index;
	struct collision_bsp *bsp = global_collision_bsp_get();
	real radius = MAX(state->input.pathfinding_radius, 0.2f);
	boolean success = TRUE;

	if (state->debug && !state->debug->debug_use_stored_obstacles)
	{
		state->debug->stored_obstacle_step_count = 0;
		match_assert("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 761, input_step_count <= MAXIMUM_SMOOTHED_PATH_STEPS);
	}

	for (input_step_index = 0; input_step_index < input_step_count; input_step_index++)
	{
		struct obstacle_path local_obstacle_path;
		struct path_step temporary_steps[MAXIMUM_OBSTACLE_AVOIDANCE_STEPS];
		struct obstacles local_obstacles;
		real_point3d const *start_point;
		long start_surface_index;
		real_point3d const *end_point;
		long end_surface_index;
		real_vector3d movement_direction;
		boolean path_found;
		struct obstacles *obstacles = &local_obstacles;
		struct obstacle_path *obstacle_path = &local_obstacle_path;
		boolean finishing = input_step_index == input_step_count - 1 && *steps_finish_path;

		if (input_step_index > 0)
		{
			start_point = &last_step_end_point;
			start_surface_index = last_step_end_surface_index;
		}
		else
		{
			start_point = &state->input.start_point;
			start_surface_index = state->input.start_surface_index;
		}

		end_point = &input_steps[input_step_index].point;
		end_surface_index = input_steps[input_step_index].surface_index;
		vector_from_points3d(start_point, end_point, &movement_direction);
		normalize3d(&movement_direction);

		if (!state->debug ||
			(obstacles = &state->debug->path_obstacles[input_step_index], obstacle_path = &state->debug->path_obstacle_paths[input_step_index], !state->debug->debug_use_stored_obstacles) ||
			input_step_index >= state->debug->stored_obstacle_step_count)
		{
			obstacles_new(obstacles);
			obstacles_get_discs_in_sphere(obstacles, start_point, 4.f, &movement_direction, state->input.ignore_source_object_index, state->input.ignore_target_object_index);

			if (state->input.attractor_valid)
			{
				obstacles_add_disc(obstacles, state->input.attractor_object_index, FLAG(_disc_optional_bit), &state->input.attractor_point, state->input.attractor_radius);
			}

			obstacles_recompute(obstacles, radius);

			if (state->debug && !state->debug->debug_use_stored_obstacles)
			{
				state->debug->stored_obstacle_step_count++;
			}
		}

		path_found = path_find(obstacle_path, state->input.ignore_broken_surfaces, obstacles, radius, (real_point2d const *)start_point, start_surface_index, (real_point2d const *)end_point, end_surface_index, finishing, FALSE);

		if (!path_found && obstacles->disc_optional_count > 0)
		{
			path_found = path_find(obstacle_path, state->input.ignore_broken_surfaces, obstacles, radius, (real_point2d const *)start_point, start_surface_index, (real_point2d const *)end_point, end_surface_index, finishing, TRUE);
		}

		if (path_found)
		{
			short obstacle_step_index;
			short temporary_step_index;
			short avoided_step_index;
			boolean overflow = FALSE;
			short temporary_step_count = 0;

			if (obstacle_path->goal_found_exactly)
			{
				last_step_end_point = *end_point;
				last_step_end_surface_index = end_surface_index;
			}
			else
			{
				struct step *goal_step = path_get_step(obstacle_path, obstacle_path->goal_step_index);

				last_step_end_surface_index = goal_step->surface_index;
				collision_surface_project_point2d(bsp, last_step_end_surface_index, _z, TRUE, &goal_step->point, &last_step_end_point);
			}

			obstacle_step_index = obstacle_path->goal_step_index;

			while (obstacle_step_index != 0)
			{
				struct path_step *temporary_step = &temporary_steps[temporary_step_count++];
				struct step *obstacle_step = path_get_step(obstacle_path, obstacle_step_index);

				temporary_step->surface_index = obstacle_step->surface_index;
				collision_surface_project_point2d(bsp, obstacle_step->surface_index, _z, TRUE, &obstacle_step->point, &temporary_step->point);
				obstacle_step_index = obstacle_step->previous_step_index;
				match_assert("c:\\halo\\SOURCE\\ai\\path_obstacle_avoidance.c", 875, (obstacle_step_index >= 0) && (obstacle_step_index < obstacle_path->step_count));

				if (temporary_step_count >= MAXIMUM_OBSTACLE_AVOIDANCE_STEPS)
				{
					overflow = TRUE;
					break;
				}
			}

			avoided_step_index = *avoided_step_count;

			for (temporary_step_index = temporary_step_count - 1; temporary_step_index >= 0; temporary_step_index--)
			{
				if (avoided_step_index >= MAXIMUM_SMOOTHED_PATH_STEPS)
				{
					overflow = TRUE;
					break;
				}

				avoided_steps[avoided_step_index++] = temporary_steps[temporary_step_index];
			}

			*avoided_step_count = avoided_step_index;

			if (overflow)
			{
				*steps_finish_path = FALSE;
				break;
			}
		}
		else
		{
			if (debug_obstacle_path_on_failure)
			{
				error(_error_silent, "obstacle path failure!");
				debug_obstacle_path_on_failure = FALSE;
				debug_obstacle_path = TRUE;
				debug_obstacle_path_start_point = *start_point;
				debug_obstacle_path_start_surface_index = start_surface_index;
				debug_obstacle_path_goal_point = *end_point;
				debug_obstacle_path_goal_surface_index = end_surface_index;
				debug_obstacle_path_radius = radius;
				debug_obstacle_path_finishing = finishing;
				debug_ignore_broken_surfaces = state->input.ignore_broken_surfaces;
				debug_obstacles = *obstacles;
				debug_path = *obstacle_path;
			}

			success = FALSE;
			break;
		}
	}

	return success;
}

void render_debug_obstacle_path(
	void)
{
	if (debug_obstacle_path)
	{
		if (!console_is_active())
		{
			if (input_key_is_down(_key_r))
			{
				debug_obstacle_path_on_failure = TRUE;
				debug_obstacle_path = FALSE;
			}

			if (input_key_is_down(_key_g))
			{
				debug_obstacle_path_on_failure = FALSE;
				debug_obstacle_path = FALSE;
			}

			if (input_key_is_down(_key_n))
			{
				path_new(
					&debug_path,
					&debug_obstacles,
					global_structure_bsp_get(),
					debug_ignore_broken_surfaces,
					debug_obstacle_path_radius,
					(real_point2d const *)&debug_obstacle_path_start_point,
					debug_obstacle_path_start_surface_index,
					(real_point2d const *)&debug_obstacle_path_goal_point,
					debug_obstacle_path_goal_surface_index,
					debug_obstacle_path_finishing,
					FALSE);
			}

			if (input_key_is_down(_key_i))
			{
				path_iterate(&debug_path);
			}
		}

		render_debug_obstacles(&debug_obstacles, debug_obstacle_path_radius);
		render_debug_path(&debug_path);
	}

	return;
}
