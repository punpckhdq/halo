/*
PATH_SMOOTHING.C
*/

/* ---------- headers */

#include "cseries.h"
#include "path.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"

/* ---------- prototypes */

static void find_tangent_point(real_point2d const *point, real_point2d const *center, real radius, boolean clockwise, real_point2d *tangent_point);
static void find_avoidance_point(real_point2d const *tangent_points, real_point2d const *center, real_point2d const *start_point, real radius, real_point2d *avoidance_point);
static boolean choose_turning_point(real_point2d const *start_point, real_point2d const *clockwise_turning_point, real_point2d const *counterclockwise_turning_point, real_point2d const *unobstructed_path_point, real_point2d const *obstructed_path_point, real_point2d *result);
static boolean surface_is_walkable(struct collision_bsp const *bsp, byte const *pathfinding_surface_data, boolean ignore_broken_surfaces, byte const *breakable_surface_flags, long surface_index);
static boolean find_turning_point(struct structure_bsp const *structure, real_point2d const *p, real radius, long first_edge_index, boolean clockwise, boolean ignore_broken_surfaces, real_point2d *result);

/* ---------- public code */

void path_smooth(
	struct path_state *state,
	short raw_step_count,
	struct path_step const *raw_steps,
	short *smoothed_step_count,
	struct path_step *smoothed_steps,
	boolean *steps_finish_path)
{
	real const pill_radius = 0.3f;
	real const avoidance_radius = 0.35f;

	match_assert("c:\\halo\\SOURCE\\ai\\path_smoothing.c", 33, raw_step_count > 0);
	match_assert("c:\\halo\\SOURCE\\ai\\path_smoothing.c", 34, raw_steps);
	match_assert("c:\\halo\\SOURCE\\ai\\path_smoothing.c", 35, smoothed_step_count);
	match_assert("c:\\halo\\SOURCE\\ai\\path_smoothing.c", 36, smoothed_steps);
	match_assert("c:\\halo\\SOURCE\\ai\\path_smoothing.c", 37, steps_finish_path);

	if (raw_step_count > 1)
	{
		short step_index;
		short step_count = 0;
		boolean finished = FALSE;
		real_point2d start_point = *(real_point2d const *)&state->input.start_point;
		long start_surface_index = state->input.start_surface_index;

		for (step_index = 1; ; )
		{
			short test_step_index;
			boolean clockwise_found;
			boolean clockwise;
			real_point2d clockwise_turning_point;
			real_point2d counterclockwise_turning_point;
			real_point2d turning_point;
			real_point2d tangent_points[2];
			real_point2d avoidance_point;
			real_point2d old_start_point;
			struct path_step *smoothed_step;
			long obstructed_edge_index = NONE;
			short obstructed_step_index = NONE;
			boolean obstructed = FALSE;

			for (test_step_index = step_index; test_step_index < raw_step_count; test_step_index++)
			{
				struct path_collision_result result;

				if (structure_test_pill2d(
					state->structure,
					state->input.ignore_broken_surfaces,
					&start_point,
					start_surface_index,
					(real_point2d const *)&raw_steps[test_step_index].point,
					raw_steps[test_step_index].surface_index,
					pill_radius,
					FLAG(_structure_test_pill2d_skip_endpoint_test_bit),
					&result))
				{
					if (!obstructed)
					{
						obstructed_edge_index = result.edge_index;
						obstructed_step_index = test_step_index;
						obstructed = TRUE;
					}
				}
				else if (obstructed)
				{
					obstructed_edge_index = NONE;
					obstructed_step_index = NONE;
					obstructed = FALSE;
				}
			}

			if (!obstructed || obstructed_edge_index == NONE)
			{
				smoothed_steps[step_count++] = raw_steps[raw_step_count - 1];
				finished = TRUE;
				break;
			}

			clockwise_found = find_turning_point(state->structure, &start_point, pill_radius, obstructed_edge_index, TRUE, state->input.ignore_broken_surfaces, &clockwise_turning_point);

			if (!find_turning_point(state->structure, &start_point, pill_radius, obstructed_edge_index, FALSE, state->input.ignore_broken_surfaces, &counterclockwise_turning_point) || !clockwise_found)
			{
				break;
			}

			clockwise = choose_turning_point(
				&start_point,
				&clockwise_turning_point,
				&counterclockwise_turning_point,
				(real_point2d const *)&raw_steps[obstructed_step_index - 1].point,
				(real_point2d const *)&raw_steps[obstructed_step_index].point,
				&turning_point);
			find_tangent_point(&start_point, &turning_point, avoidance_radius, clockwise, &tangent_points[0]);
			find_tangent_point((real_point2d const *)&raw_steps[obstructed_step_index].point, &turning_point, avoidance_radius, !clockwise, &tangent_points[1]);
			find_avoidance_point(tangent_points, &turning_point, &start_point, avoidance_radius, &avoidance_point);

			old_start_point = start_point;
			start_point = avoidance_point;
			start_surface_index = structure_surface_index_from_point(state->structure, state->input.ignore_broken_surfaces, &old_start_point, start_surface_index, &start_point);

			smoothed_step = &smoothed_steps[step_count++];
			collision_surface_project_point2d(
				TAG_BLOCK_GET_ELEMENT(&state->structure->collision_bsp, 0, struct collision_bsp),
				start_surface_index,
				_z,
				TRUE,
				&start_point,
				&smoothed_step->point);
			smoothed_step->surface_index = start_surface_index;

			if (step_count >= MAXIMUM_SMOOTHED_PATH_STEPS)
			{
				break;
			}

			step_index = obstructed_step_index;
		}

		*smoothed_step_count = step_count;

		if (!finished)
		{
			*steps_finish_path = FALSE;
		}
	}
	else
	{
		*smoothed_step_count = 1;
		smoothed_steps[0] = raw_steps[0];
	}

	return;
}

/* ---------- private code */

static void find_tangent_point(
	real_point2d const *point,
	real_point2d const *center,
	real radius,
	boolean clockwise,
	real_point2d *tangent_point)
{
	real_vector2d center_to_point;
	real distance_squared;
	real scale;
	real discriminant;

	vector_from_points2d(center, point, &center_to_point);
	distance_squared = magnitude_squared2d(&center_to_point);
	scale = radius / distance_squared;
	discriminant = distance_squared - radius * radius;

	if (discriminant > 0.f)
	{
		real_point2d tangent_points[2];
		real_vector2d tangent_vectors[2];
		real root = square_root(discriminant);

		set_real_point2d(&tangent_points[0], (center_to_point.i * radius + center_to_point.j * root) * scale + center->x, (center_to_point.j * radius - center_to_point.i * root) * scale + center->y);
		set_real_point2d(&tangent_points[1], (center_to_point.i * radius - center_to_point.j * root) * scale + center->x, (center_to_point.j * radius + center_to_point.i * root) * scale + center->y);
		vector_from_points2d(point, &tangent_points[0], &tangent_vectors[0]);
		vector_from_points2d(point, &tangent_points[1], &tangent_vectors[1]);
		*tangent_point = tangent_points[(cross_product2d(&tangent_vectors[0], &tangent_vectors[1]) > 0.f) != clockwise];
	}
	else
	{
		real_vector2d radius_vector;

		vector_from_points2d(center, point, &radius_vector);

		if (normalize2d(&radius_vector) == 0.f)
		{
			radius_vector = *global_left2d;
		}

		point_from_line2d(center, &radius_vector, radius, tangent_point);
	}

	return;
}

static void find_avoidance_point(
	real_point2d const *tangent_points,
	real_point2d const *center,
	real_point2d const *start_point,
	real radius,
	real_point2d *avoidance_point)
{
	real_vector2d center_to_tangent[2];
	real cross;
	boolean found = FALSE;

	vector_from_points2d(center, &tangent_points[0], &center_to_tangent[0]);
	vector_from_points2d(center, &tangent_points[1], &center_to_tangent[1]);
	cross = cross_product2d(&center_to_tangent[0], &center_to_tangent[1]);

	if (!realcmp(cross, 0.f))
	{
		real scale = radius * radius / cross;

		set_real_point2d(
			avoidance_point,
			center->x - (center_to_tangent[0].j - center_to_tangent[1].j) * scale,
			(center_to_tangent[0].i - center_to_tangent[1].i) * scale + center->y);
		found = TRUE;
	}

	if (!found || distance_squared2d(center, avoidance_point) > radius * radius * 4.f)
	{
		real_vector2d tangent_vector;

		vector_from_points2d(start_point, &tangent_points[0], &tangent_vector);

		if (normalize2d(&tangent_vector) == 0.f)
		{
			tangent_vector = *global_left2d;
		}

		point_from_line2d(&tangent_points[0], &tangent_vector, radius, avoidance_point);
	}

	return;
}

static boolean choose_turning_point(
	real_point2d const *start_point,
	real_point2d const *clockwise_turning_point,
	real_point2d const *counterclockwise_turning_point,
	real_point2d const *unobstructed_path_point,
	real_point2d const *obstructed_path_point,
	real_point2d *result)
{
	real_vector2d clockwise_to_start;
	real_vector2d clockwise_to_unobstructed;
	real_vector2d clockwise_to_obstructed;
	real_vector2d counterclockwise_to_start;
	real_vector2d counterclockwise_to_unobstructed;
	real_vector2d counterclockwise_to_obstructed;
	real clockwise_angle;
	real counterclockwise_angle;

	normalize2d(vector_from_points2d(clockwise_turning_point, start_point, &clockwise_to_start));
	normalize2d(vector_from_points2d(clockwise_turning_point, unobstructed_path_point, &clockwise_to_unobstructed));
	normalize2d(vector_from_points2d(clockwise_turning_point, obstructed_path_point, &clockwise_to_obstructed));
	normalize2d(vector_from_points2d(counterclockwise_turning_point, start_point, &counterclockwise_to_start));
	normalize2d(vector_from_points2d(counterclockwise_turning_point, unobstructed_path_point, &counterclockwise_to_unobstructed));
	normalize2d(vector_from_points2d(counterclockwise_turning_point, obstructed_path_point, &counterclockwise_to_obstructed));

	clockwise_angle = signed_angle_between_vectors2d(&clockwise_to_start, &clockwise_to_unobstructed) + signed_angle_between_vectors2d(&clockwise_to_unobstructed, &clockwise_to_obstructed);
	counterclockwise_angle = signed_angle_between_vectors2d(&counterclockwise_to_start, &counterclockwise_to_unobstructed) + signed_angle_between_vectors2d(&counterclockwise_to_unobstructed, &counterclockwise_to_obstructed);

	if (clockwise_angle > -counterclockwise_angle)
	{
		*result = *clockwise_turning_point;
		return TRUE;
	}
	else
	{
		*result = *counterclockwise_turning_point;
		return FALSE;
	}
}

static boolean surface_is_walkable(
	struct collision_bsp const *bsp,
	byte const *pathfinding_surface_data,
	boolean ignore_broken_surfaces,
	byte const *breakable_surface_flags,
	long surface_index)
{
	boolean walkable = TEST_FLAG(pathfinding_surface_data[surface_index], PATHFINDING_SURFACE_WALKABLE_BIT);

	if (!ignore_broken_surfaces && walkable && TEST_FLAG(pathfinding_surface_data[surface_index], PATHFINDING_SURFACE_BREAKABLE_BIT))
	{
		struct collision_surface const *collision_surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);

		match_assert("c:\\halo\\SOURCE\\ai\\path_smoothing.c", 482, TEST_FLAG(collision_surface->flags, _collision_surface_breakable_bit));

		walkable = BIT_VECTOR_TEST_FLAG((long const *)breakable_surface_flags, collision_surface->breakable_surface_index);
	}

	return walkable;
}

static boolean find_turning_point(
	struct structure_bsp const *structure,
	real_point2d const *p,
	real radius,
	long first_edge_index,
	boolean clockwise,
	boolean ignore_broken_surfaces,
	real_point2d *result)
{
	long edge_index;
	long next_vertex_index;
	struct collision_bsp const *bsp = TAG_BLOCK_GET_ELEMENT(&structure->collision_bsp, 0, struct collision_bsp);
	byte const *pathfinding_surface_data = structure->pathfinding_surfaces.address;
	byte const *breakable_surface_flags = breakable_surface_flags_get();
	long first_vertex_index = NONE;
	long last_vertex_index = NONE;

	match_assert("c:\\halo\\SOURCE\\ai\\path_smoothing.c", 511, clockwise==TRUE || clockwise==FALSE);

	edge_index = first_edge_index;

	while (TRUE)
	{
		real_vector2d edge_vector;
		real_vector2d perpendicular;
		real_point2d p0;
		real_point2d p1;
		real_vector2d p0_to_vertex;
		real_vector2d p1_to_vertex;
		long start_edge_index;
		struct collision_edge const *collision_edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
		boolean walkable = surface_is_walkable(bsp, pathfinding_surface_data, ignore_broken_surfaces, breakable_surface_flags, collision_edge->surface_indices[0]);
		struct collision_vertex const *vertex0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, collision_edge->vertex_indices[walkable], struct collision_vertex);
		struct collision_vertex const *vertex1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, collision_edge->vertex_indices[!walkable], struct collision_vertex);
		boolean outside = FALSE;

		vector_from_points2d((real_point2d const *)&vertex0->point, (real_point2d const *)&vertex1->point, &edge_vector);
		set_real_vector2d(&perpendicular, edge_vector.j, -edge_vector.i);
		normalize2d(&perpendicular);
		point_from_line2d(p, &perpendicular, radius, &p0);
		point_from_line2d(p, &perpendicular, -radius, &p1);
		vector_from_points2d(&p0, (real_point2d const *)&vertex0->point, &p0_to_vertex);
		vector_from_points2d(&p1, (real_point2d const *)&vertex0->point, &p1_to_vertex);

		if ((dot_product2d(&p0_to_vertex, &edge_vector) < 0.f) == clockwise)
		{
			if (cross_product2d(&p0_to_vertex, &edge_vector) < 0.f)
			{
				outside = TRUE;
			}
		}

		if (cross_product2d(&p1_to_vertex, &edge_vector) < 0.f)
		{
			outside = TRUE;
		}

		if (first_vertex_index == NONE)
		{
			outside = TRUE;
		}

		next_vertex_index = collision_edge->vertex_indices[(outside != walkable) == clockwise];

		if (next_vertex_index == last_vertex_index)
		{
			break;
		}

		if (next_vertex_index == first_vertex_index)
		{
			return FALSE;
		}

		if (first_vertex_index == NONE)
		{
			first_vertex_index = next_vertex_index;
		}

		start_edge_index = edge_index;

		while (TRUE)
		{
			boolean forward = next_vertex_index == collision_edge->vertex_indices[1];

			if (surface_is_walkable(bsp, pathfinding_surface_data, ignore_broken_surfaces, breakable_surface_flags, collision_edge->surface_indices[!forward]) == clockwise)
			{
				break;
			}

			edge_index = collision_edge->edge_indices[!forward];
			collision_edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);

			if (edge_index == start_edge_index)
			{
				return FALSE;
			}

			match_assert("c:\\halo\\SOURCE\\ai\\path_smoothing.c", 631, collision_edge->vertex_indices[0]==next_vertex_index || collision_edge->vertex_indices[1]==next_vertex_index);
		}

		last_vertex_index = next_vertex_index;
	}

	*result = *(real_point2d const *)&TAG_BLOCK_GET_ELEMENT(&bsp->vertices, next_vertex_index, struct collision_vertex)->point;

	return TRUE;
}
