/*
PATH_STRUCTURE_BSP.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ai_constants.h"
#include "path.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"

/* ---------- constants */

enum
{
	NUMBER_OF_QUANTIZED_PATHFINDING_SURFACE_SIZES = 8 /* fake name */
};

/* ---------- globals */

static real const quantized_pathfinding_surface_widths[NUMBER_OF_QUANTIZED_PATHFINDING_SURFACE_SIZES] =
{
	0.2f,
	0.4f,
	0.6f,
	0.8f,
	1.f,
	1.5f,
	2.f,
	4.f
};

static real const quantized_pathfinding_surface_heights[NUMBER_OF_QUANTIZED_PATHFINDING_SURFACE_SIZES] =
{
	0.25f,
	0.5f,
	0.75f,
	1.f,
	1.5f,
	2.f,
	4.f,
	8.f
};

/* ---------- public code */

boolean structure_test_ray2d(
	struct structure_bsp const *structure,
	boolean ignore_broken_surfaces,
	real_point2d const *point,
	long surface_index,
	real_vector2d const *direction,
	real distance,
	struct structure_test_ray2d_result *result)
{
	struct collision_surface_test_line2d_result surface_result;
	boolean hit;
	struct collision_bsp const *bsp = TAG_BLOCK_GET_ELEMENT(&structure->collision_bsp, 0, struct collision_bsp);
	byte *breakable_surface_flags = breakable_surface_flags_get();
	byte const *pathfinding_surfaces = structure->pathfinding_surfaces.address;

	collision_surface_test_line2d(bsp, surface_index, _z, TRUE, point, direction, &surface_result);

	while (TRUE)
	{
		long next_surface_index = NONE;

		if (distance < surface_result.enter_t && pathfinding_surfaces[surface_result.enter_surface_index])
		{
			boolean walkable = TRUE;

			if (!ignore_broken_surfaces && TEST_FLAG(pathfinding_surfaces[surface_result.enter_surface_index], PATHFINDING_SURFACE_BREAKABLE_BIT))
			{
				struct collision_surface const *collision_surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_result.enter_surface_index, struct collision_surface);

				match_assert("c:\\halo\\SOURCE\\ai\\path_structure_bsp.c", 105, TEST_FLAG(collision_surface->flags, _collision_surface_breakable_bit));

				walkable = BIT_VECTOR_TEST_FLAG((long *)breakable_surface_flags, collision_surface->breakable_surface_index);
			}

			if (walkable)
			{
				next_surface_index = surface_result.enter_surface_index;
			}
		}

		if (next_surface_index == NONE && distance > surface_result.exit_t && pathfinding_surfaces[surface_result.exit_surface_index])
		{
			boolean walkable = TRUE;

			if (!ignore_broken_surfaces && TEST_FLAG(pathfinding_surfaces[surface_result.exit_surface_index], PATHFINDING_SURFACE_BREAKABLE_BIT))
			{
				struct collision_surface const *collision_surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_result.exit_surface_index, struct collision_surface);

				match_assert("c:\\halo\\SOURCE\\ai\\path_structure_bsp.c", 126, TEST_FLAG(collision_surface->flags, _collision_surface_breakable_bit));

				walkable = BIT_VECTOR_TEST_FLAG((long *)breakable_surface_flags, collision_surface->breakable_surface_index);
			}

			if (walkable)
			{
				next_surface_index = surface_result.exit_surface_index;
			}
		}

		if (next_surface_index == NONE)
		{
			break;
		}

		surface_index = next_surface_index;
		collision_surface_test_line2d(bsp, surface_index, _z, TRUE, point, direction, &surface_result);
	}

	if (distance < surface_result.enter_t)
	{
		result->distance = surface_result.enter_t;
		result->surface_index = surface_index;
		result->edge_index = surface_result.enter_edge_index;
		hit = TRUE;
	}
	else if (distance > surface_result.exit_t)
	{
		result->distance = surface_result.exit_t;
		result->surface_index = surface_index;
		result->edge_index = surface_result.exit_edge_index;
		hit = TRUE;
	}
	else
	{
		result->distance = distance;
		result->surface_index = surface_index;
		result->edge_index = NONE;
		hit = FALSE;
	}

	return hit;
}

boolean structure_surfaces_are_equivalent(
	struct structure_bsp const *structure,
	real_point2d const *destination_point,
	long destination_surface_index,
	long test_surface_index)
{
	struct collision_bsp const *bsp = TAG_BLOCK_GET_ELEMENT(&structure->collision_bsp, 0, struct collision_bsp);
	boolean equivalent = FALSE;

	if (destination_surface_index != NONE && test_surface_index != NONE)
	{
		real_point3d destination_surface_point;
		real_point3d test_surface_point;

		collision_surface_project_point2d(bsp, destination_surface_index, _z, TRUE, destination_point, &destination_surface_point);
		collision_surface_project_point2d(bsp, test_surface_index, _z, TRUE, destination_point, &test_surface_point);
		equivalent = fabs(destination_surface_point.z - test_surface_point.z) < 0.05f;
	}

	return equivalent;
}

boolean structure_test_line2d(
	struct structure_bsp const *structure,
	boolean ignore_broken_surfaces,
	real_point2d const *p0,
	long p0_surface_index,
	real_point2d const *p1,
	long p1_surface_index,
	struct path_collision_result *result)
{
	real_vector2d p0p1;
	struct collision_bsp const *bsp = TAG_BLOCK_GET_ELEMENT(&structure->collision_bsp, 0, struct collision_bsp);
	byte const *pathfinding_surfaces = structure->pathfinding_surfaces.address;
	byte *breakable_surface_flags = breakable_surface_flags_get();
	long surface_index = p0_surface_index;
	boolean retried = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\path_structure_bsp.c", 217, result);

	vector_from_points2d(p0, p1, &p0p1);

	while (TRUE)
	{
		struct collision_surface const *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);
		long edge_index = surface->first_edge_index;
		real_point3d point_in_surface = *global_origin3d;
		short vertex_count = 0;
		boolean exits_surface = FALSE;
		boolean adjacent_to_p1_surface = FALSE;

		while (TRUE)
		{
			real_vector2d e0e1;
			real_vector2d e0p1;
			real_vector2d p0e0;
			real_vector2d p0e1;
			struct collision_edge const *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
			boolean reversed = surface_index == edge->surface_indices[1];
			struct collision_vertex const *vertex0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[!reversed], struct collision_vertex);
			struct collision_vertex const *vertex1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[reversed], struct collision_vertex);

			vector_from_points2d((real_point2d const *)&vertex0->point, (real_point2d const *)&vertex1->point, &e0e1);
			vector_from_points2d((real_point2d const *)&vertex0->point, p1, &e0p1);
			vector_from_points2d(p0, (real_point2d const *)&vertex0->point, &p0e0);
			vector_from_points2d(p0, (real_point2d const *)&vertex1->point, &p0e1);

			if (edge->surface_indices[!reversed] == p1_surface_index)
			{
				adjacent_to_p1_surface = TRUE;
			}

			point_in_surface.x += vertex0->point.x;
			point_in_surface.y += vertex0->point.y;
			point_in_surface.z += vertex0->point.z;
			vertex_count++;

			if (cross_product2d(&e0e1, &e0p1) > 0.f)
			{
				exits_surface = TRUE;

				if (cross_product2d(&p0p1, &p0e0) > 0.f && cross_product2d(&p0e1, &p0p1) > 0.f)
				{
					long adjacent_surface_index = edge->surface_indices[!reversed];
					boolean walkable = TEST_FLAG(pathfinding_surfaces[adjacent_surface_index], PATHFINDING_SURFACE_WALKABLE_BIT);

					if (!ignore_broken_surfaces && walkable && TEST_FLAG(pathfinding_surfaces[adjacent_surface_index], PATHFINDING_SURFACE_BREAKABLE_BIT))
					{
						struct collision_surface const *collision_surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, adjacent_surface_index, struct collision_surface);

						match_assert("c:\\halo\\SOURCE\\ai\\path_structure_bsp.c", 274, TEST_FLAG(collision_surface->flags, _collision_surface_breakable_bit));

						walkable = BIT_VECTOR_TEST_FLAG((long *)breakable_surface_flags, collision_surface->breakable_surface_index);
					}

					if (walkable)
					{
						surface_index = adjacent_surface_index;
						break;
					}
					else
					{
						real_point2d p2d;
						real t = (cross_product2d(&e0e1, &p0e0) - magnitude2d(&e0e1) / 128.f) / cross_product2d(&e0e1, &p0p1);

						point_from_line2d(p0, &p0p1, t, &p2d);
						collision_surface_project_point2d(bsp, surface_index, _z, TRUE, &p2d, &result->point);
						result->surface_index = surface_index;
						result->edge_index = edge_index;
						result->collision = TRUE;
						result->t = t;

						return TRUE;
					}
				}
			}

			edge_index = edge->edge_indices[reversed];

			if (edge_index == surface->first_edge_index)
			{
				if (exits_surface)
				{
					struct path_collision_result p0_result;

					match_assert("c:\\halo\\SOURCE\\ai\\path_structure_bsp.c", 316, surface_index>=0 && surface_index<structure->pathfinding_surfaces.count);

					point_in_surface.x /= vertex_count;
					point_in_surface.y /= vertex_count;

					if (!retried && pathfinding_surfaces[surface_index] && !structure_test_line2d(structure, ignore_broken_surfaces, (real_point2d const *)&point_in_surface, surface_index, p0, NONE, &p0_result))
					{
						retried = TRUE;
						surface_index = p0_result.surface_index;
						break;
					}
					else
					{
						collision_surface_project_point2d(bsp, p0_surface_index, _z, TRUE, p0, &result->point);
						result->surface_index = NONE;
						result->edge_index = NONE;
						result->collision = TRUE;
						result->t = 0.f;

						return TRUE;
					}
				}
				else if (surface_index == p1_surface_index || adjacent_to_p1_surface || p1_surface_index == NONE)
				{
					collision_surface_project_point2d(bsp, surface_index, _z, TRUE, p1, &result->point);
					result->surface_index = surface_index;
					result->edge_index = NONE;
					result->collision = FALSE;
					result->t = 1.f;

					return FALSE;
				}
				else
				{
					collision_surface_project_point2d(bsp, p0_surface_index, _z, TRUE, p0, &result->point);
					result->surface_index = NONE;
					result->edge_index = NONE;
					result->collision = TRUE;
					result->t = 0.f;

					return TRUE;
				}
			}
		}
	}
}

long structure_surface_index_from_point(
	struct structure_bsp const *structure,
	boolean ignore_broken_surfaces,
	real_point2d const *known_point,
	long known_surface_index,
	real_point2d *point)
{
	long surface_index;

	if (known_surface_index != NONE)
	{
		struct path_collision_result result;

		structure_test_line2d(structure, ignore_broken_surfaces, known_point, known_surface_index, point, NONE, &result);
		*point = *(real_point2d *)&result.point;
		surface_index = result.surface_index == NONE ? known_surface_index : result.surface_index;
	}
	else
	{
		surface_index = NONE;
	}

	return surface_index;
}

boolean structure_test_pill2d(
	struct structure_bsp const *structure,
	boolean ignore_broken_surfaces,
	real_point2d const *start_point,
	long start_surface_index,
	real_point2d const *end_point,
	long end_surface_index,
	real radius,
	unsigned long flags,
	struct path_collision_result *result)
{
	real_vector2d line_vector;
	real_vector2d perpendicular;
	boolean collision = FALSE;

	vector_from_points2d(start_point, end_point, &line_vector);
	set_real_vector2d(&perpendicular, -line_vector.j, line_vector.i);

	if (normalize2d(&perpendicular) > 0.f)
	{
		real_point2d p0;
		real_point2d p1;
		real_point2d q0;
		real_point2d q1;
		long p0_surface_index;
		long p1_surface_index;
		long q0_surface_index;
		long q1_surface_index;
		struct path_collision_result result0;
		struct path_collision_result result1;
		struct path_collision_result temporary;
		struct path_collision_result *best_result;

		point_from_line2d(start_point, &perpendicular, radius, &p0);
		p0_surface_index = structure_surface_index_from_point(structure, ignore_broken_surfaces, start_point, start_surface_index, &p0);
		point_from_line2d(end_point, &perpendicular, radius, &p1);
		p1_surface_index = structure_surface_index_from_point(structure, ignore_broken_surfaces, end_point, end_surface_index, &p1);
		point_from_line2d(start_point, &perpendicular, -radius, &q0);
		q0_surface_index = structure_surface_index_from_point(structure, ignore_broken_surfaces, start_point, start_surface_index, &q0);
		point_from_line2d(end_point, &perpendicular, -radius, &q1);
		q1_surface_index = structure_surface_index_from_point(structure, ignore_broken_surfaces, end_point, end_surface_index, &q1);

		if (p0_surface_index == NONE)
		{
			result0.collision = FALSE;
		}
		else if (structure_test_line2d(structure, ignore_broken_surfaces, &p0, p0_surface_index, &p1, p1_surface_index, &result0) &&
			result0.surface_index != NONE &&
			!TEST_FLAG(flags, _structure_test_pill2d_skip_endpoint_test_bit) &&
			!structure_test_line2d(structure, ignore_broken_surfaces, (real_point2d const *)&result0.point, result0.surface_index, end_point, end_surface_index, &temporary))
		{
			result0.collision = FALSE;
		}

		if (q0_surface_index == NONE)
		{
			result1.collision = FALSE;
		}
		else if (structure_test_line2d(structure, ignore_broken_surfaces, &q0, q0_surface_index, &q1, q1_surface_index, &result1) &&
			result1.surface_index != NONE &&
			!TEST_FLAG(flags, _structure_test_pill2d_skip_endpoint_test_bit) &&
			!structure_test_line2d(structure, ignore_broken_surfaces, (real_point2d const *)&result1.point, result1.surface_index, end_point, end_surface_index, &temporary))
		{
			result1.collision = FALSE;
		}

		if (result0.collision && result1.collision)
		{
			best_result = result0.t < result1.t ? &result0 : &result1;
		}
		else if (result0.collision)
		{
			best_result = &result0;
		}
		else if (result1.collision)
		{
			best_result = &result1;
		}
		else
		{
			best_result = NULL;
		}

		if (best_result && distance_squared2d((real_point2d const *)&best_result->point, end_point) < radius * radius)
		{
			best_result = NULL;
		}

		if (best_result)
		{
			*result = *best_result;
			collision = TRUE;
		}
		else
		{
			*result = result0;
		}
	}

	return collision;
}

boolean clip_empty_interval_by_solid_interval(
	real *empty_t0,
	real *empty_t1,
	real solid_t0,
	real solid_t1)
{
	real t1 = MAX(*empty_t0, solid_t1);
	real t0 = MIN(solid_t0, *empty_t1);

	if (*empty_t1 - t1 > t0 - *empty_t0)
	{
		*empty_t0 = t1;
	}
	else
	{
		*empty_t1 = t0;
	}

	return *empty_t0 > *empty_t1;
}
