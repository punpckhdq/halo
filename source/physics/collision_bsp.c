/*
COLLISION_BSP.C
*/

/* ---------- headers */

#include "cseries.h"
#include "cheats.h"
#include "game.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "object_definitions.h"
#include "rasterizer_geometry.h"
#include "model_animation_definitions.h"
#include "model_definitions.h"
#include "models.h"
#include "damage_resistances.h"
#include "shader_definitions.h"
#include "objects.h"
#include "bsp3d.h"
#include "bsp2d.h"
#include "collision_bsp_definitions.h"
#include "collision_bsp.h"
#include "collision_usage.h"
#include "collision_features.h"
#include "collisions.h"
#ifdef DEBUG
#include "render_debug.h"
#endif


/* ---------- structures */

struct test_pill_new_data
{
	struct collision_bsp const *bsp;
	short breakable_surface_count;
	byte const *breakable_surface_flags;

	real_point3d const *point;
	real_vector3d const *vector;
	real radius;

	real *t;
	real_vector3d *normal;

	long last_leaf_index;
	byte last_contents;
	long last_plane_designator;
};

struct test_pill_data
{
	struct collision_bsp const *bsp;

	real_point3d const *point;
	real_vector3d const *vector;
	real radius;

	struct collision_bsp_test_pill_result *result;

	long stack_depth;
	long plane_stack[MAXIMUM_BSP3D_DEPTH];

	short projection_axis;
	byte projection_sign;
	real_point2d p2d;
	real_vector2d v2d;
};

struct test_vector_data
{
	unsigned long flags;
	struct collision_bsp const *bsp;
	short breakable_surface_count;
	byte const *breakable_surface_flags;

	real_point3d const *point;
	real_vector3d const *vector;

	struct collision_bsp_test_vector_result *result;

	long last_leaf_index;
	byte last_contents;
	long last_plane_index;
};

struct test_sphere_data
{
	struct collision_bsp const *bsp;
	short breakable_surface_count;
	byte const *breakable_surface_flags;

	real_point3d const *center;
	real radius;

	struct collision_bsp_test_sphere_result *result;

	long stack_depth;
	long plane_stack[MAXIMUM_BSP3D_DEPTH];

	short projection_axis;
	byte projection_sign;
	real_point2d center2d;
};

/* ---------- prototypes */

static void bsp3d_test_sphere_recursive(struct test_sphere_data *data, long child_index);
static void bsp2d_test_sphere_recursive(struct test_sphere_data *data, long child_index);
static void collision_surface_test_sphere(struct test_sphere_data *data, long surface_index);
static void add_feature(long *count, long *indices, long index);
static boolean collision_bsp_test_vector_recursive(struct test_vector_data *data, long child_index, real t0, real t1);
static long collision_leaf_test_vector(struct collision_bsp const *bsp, short breakable_surface_count, byte const *breakable_surface_flags, real_point3d const *point, real_vector3d const *vector, long leaf_index, long plane_index, real t, boolean test_surface_flag);
static boolean collision_surface_test_point(struct collision_bsp const *bsp, short breakable_surface_count, byte const *breakable_surface_flags, long surface_index, short projection_axis, boolean projection_sign, real_point2d const *point);
static boolean bsp3d_test_pill_recursive(struct test_pill_data *data, long child_index);
static boolean bsp2d_test_pill_recursive(struct test_pill_data *data, long child_index);
static boolean collision_surface_test_pill(struct test_pill_data *data, long surface_index);
static boolean pill_test_vector(real_point3d const *base, real_vector3d const *height, real width, real_point3d const *point, real_vector3d const *vector, real *t_reference, real *u_reference);
static boolean sphere_test_vector(real_point3d const *center, real radius, real_point3d const *point, real_vector3d const *vector, real *t_reference);
static boolean collision_bsp_test_pill_new_recursive(struct test_pill_new_data *data, long child_index, real t0, real t1);

/* ---------- globals */

static __int64 vector_start_time;
static __int64 sphere_start_time;

/* ---------- public code */

real collision_edge_length(
	struct collision_bsp const *bsp,
	long edge_index)
{
	struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
	struct collision_vertex *v0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[0], struct collision_vertex);
	struct collision_vertex *v1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[1], struct collision_vertex);

	return distance3d(&v0->point, &v1->point);
}

short collision_surface_edge_count(
	struct collision_bsp const *bsp,
	long surface_index)
{
	short edge_count = 0;
	struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);
	long first_edge_index = surface->first_edge_index;

	long edge_index = first_edge_index;

	do
	{
		struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
		boolean side = edge->surface_indices[1] == surface_index;

		edge_count++;

		edge_index = edge->edge_indices[side];
	}
	while (edge_index != first_edge_index);

	return edge_count;
}

real collision_surface_perimeter(
	struct collision_bsp const *bsp,
	long surface_index)
{
	real perimeter = 0.f;
	struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);
	long first_edge_index = surface->first_edge_index;

	long edge_index = first_edge_index;

	do
	{
		struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
		boolean side = edge->surface_indices[1] == surface_index;
		struct collision_vertex *v0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[side], struct collision_vertex);
		struct collision_vertex *v1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[!side], struct collision_vertex);

		perimeter += distance3d(&v0->point, &v1->point);

		edge_index = edge->edge_indices[side];
	}
	while (edge_index != first_edge_index);

	return perimeter;
}

real collision_surface_area(
	struct collision_bsp const *bsp,
	long surface_index)
{
	real area = 0.f;
	struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);
	struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, surface->first_edge_index, struct collision_edge);
	boolean side = edge->surface_indices[1] == surface_index;
	struct collision_vertex *origin = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[side], struct collision_vertex);
	real_plane3d plane;

	bsp3d_get_plane_from_designator(&bsp->bsp3d, surface->plane_designator, &plane);

	edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge->edge_indices[side], struct collision_edge);

	for (side = edge->surface_indices[1] == surface_index;
		edge->edge_indices[side] != surface->first_edge_index;
		side = edge->surface_indices[1] == surface_index)
	{
		struct collision_vertex *vertex0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[side], struct collision_vertex);
		struct collision_vertex *vertex1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[!side], struct collision_vertex);
		real_vector3d v0;
		real_vector3d v1;

		vector_from_points3d(&origin->point, &vertex0->point, &v0);
		vector_from_points3d(&origin->point, &vertex1->point, &v1);

		area += triple_product3d(&v0, &v1, &plane.n);

		edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge->edge_indices[side], struct collision_edge);
	}

	return MAX(area, 0.f);
}

short collision_surface_polygon(
	struct collision_bsp const *bsp,
	long surface_index,
	real_point3d *points)
{
	short point_count = 0;
	struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);
	long first_edge_index = surface->first_edge_index;
	long edge_index = first_edge_index;

	do
	{
		struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
		boolean side = edge->surface_indices[1] == surface_index;
		struct collision_vertex *vertex = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[side], struct collision_vertex);

		match_assert("c:\\halo\\SOURCE\\physics\\collision_bsp.c", 225, point_count<MAXIMUM_VERTICES_PER_COLLISION_SURFACE);
		points[point_count++] = vertex->point;
		edge_index = edge->edge_indices[side];
	}
	while (edge_index != first_edge_index);

	return point_count;
}

/* ---------- surface projection */

real_point3d *collision_surface_project_point2d(
	struct collision_bsp const *bsp,
	long surface_index,
	short projection_axis,
	boolean projection_sign,
	real_point2d const *p2d,
	real_point3d *p3d)
{
	struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);
	real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&bsp->bsp3d.planes, surface->plane_designator & LONG_MAX, real_plane3d);

	project_point2d(p2d, plane, projection_axis, projection_sign, p3d);

	return p3d;
}

boolean collision_surface_test_point2d(
	struct collision_bsp const *bsp,
	long surface_index,
	short projection_axis,
	boolean projection_sign,
	real_point2d const *point)
{
	struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);
	long first_edge_index = surface->first_edge_index;
	long edge_index = first_edge_index;
	boolean result = TRUE;

	do
	{
		struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
		boolean side = (edge->surface_indices[1] == surface_index);
		struct collision_vertex *vertex0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[side], struct collision_vertex);
		struct collision_vertex *vertex1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[!side], struct collision_vertex);
		real_point2d point0;
		real_point2d point1;
		real_vector2d v0;
		real_vector2d v1;

		project_point3d(&vertex0->point, projection_axis, projection_sign, &point0);
		project_point3d(&vertex1->point, projection_axis, projection_sign, &point1);
		vector_from_points2d(&point0, point, &v0);
		vector_from_points2d(&point1, point, &v1);
		if (cross_product2d(&v0, &v1) > 0.f)
		{
			result = FALSE;
			break;
		}

		edge_index = edge->edge_indices[side];
	}
	while (edge_index != first_edge_index);

	return result;
}

boolean collision_surface_find_closest_point2d(
	struct collision_bsp const *bsp,
	long surface_index,
	short projection_axis,
	boolean projection_sign,
	real_point2d const *point,
	real_point2d *result)
{
	struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);
	long first_edge_index = surface->first_edge_index;
	long edge_index = first_edge_index;
	boolean before_first_edge;
	boolean beyond_first_edge;
	boolean before_last_edge;
	boolean beyond_last_edge;

	do
	{
		struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
		boolean side = edge->surface_indices[1] == surface_index;
		struct collision_vertex *vertex0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[side], struct collision_vertex);
		struct collision_vertex *vertex1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[!side], struct collision_vertex);
		real_point2d q0;
		real_point2d q1;
		real_vector2d v0;
		real_vector2d v1;
		boolean before_edge = FALSE;
		boolean beyond_edge = FALSE;

		project_point3d(&vertex0->point, projection_axis, projection_sign, &q0);
		project_point3d(&vertex1->point, projection_axis, projection_sign, &q1);
		vector_from_points2d(&q0, point, &v0);
		vector_from_points2d(&q0, &q1, &v1);

		if (cross_product2d(&v0, &v1) > 0.f)
		{
			real numerator = dot_product2d(&v0, &v1);

			if (numerator < 0.f)
			{
				before_edge = TRUE;
			}
			else
			{
				real denominator = magnitude_squared2d(&v1);

				if (numerator > denominator)
				{
					beyond_edge = TRUE;
				}
				else
				{
					point_from_line2d(&q0, &v1, numerator / denominator, result);

					return FALSE;
				}
			}
		}

		if (edge_index != first_edge_index)
		{
			if ((beyond_last_edge && (before_edge || !beyond_edge)) ||
				(before_edge && (beyond_last_edge || !before_last_edge)))
			{
				*result = q0;
				return FALSE;
			}
		}
		else
		{
			before_first_edge = before_edge;
			beyond_first_edge = beyond_edge;
		}

		before_last_edge = before_edge;
		beyond_last_edge = beyond_edge;

		edge_index = edge->edge_indices[side];
	}
	while (edge_index != first_edge_index);

	if ((beyond_last_edge && (before_first_edge || !beyond_first_edge)) ||
		(before_first_edge && (beyond_last_edge || !before_last_edge)))
	{
		struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
		boolean side = edge->surface_indices[1] == surface_index;
		struct collision_vertex *vertex = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[side], struct collision_vertex);

		project_point3d(&vertex->point, projection_axis, projection_sign, result);
		return FALSE;
	}
	else
	{
		*result = *point;

		return TRUE;
	}
}

boolean collision_surface_test_line2d(
	struct collision_bsp const *bsp,
	long surface_index,
	short projection_axis,
	boolean projection_sign,
	real_point2d const *point,
	real_vector2d const *vector,
	struct collision_surface_test_line2d_result *result)
{
	struct collision_edge *edge;
	struct collision_vertex *v0, *v1;
	real distance;
	long first_edge_index, edge_index;
	real direction;
	boolean side;
	real t;
	real_vector2d edge_vector, point_vector;
	struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);

	first_edge_index = surface->first_edge_index;
	edge_index = first_edge_index;

	result->enter_t = -FLT_MAX;
	result->enter_edge_index = NONE;
	result->enter_surface_index = NONE;
	result->exit_t = FLT_MAX;
	result->exit_edge_index = NONE;
	result->exit_surface_index = NONE;

	do
	{
		edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
		side = edge->surface_indices[1] == surface_index;

		v0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[0], struct collision_vertex);
		v1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[1], struct collision_vertex);

		vector_from_points2d((real_point2d const *)&v0->point, (real_point2d const *)&v1->point, &edge_vector);
		vector_from_points2d((real_point2d const *)&v0->point, point, &point_vector);
		direction = cross_product2d(vector, &edge_vector);
		distance = cross_product2d(&edge_vector, &point_vector);

		if (direction != 0.f)
		{
			t = distance / direction;
			if ((direction < 0.f) != side)
			{
				if (t > result->enter_t)
				{
					result->enter_t = t;
					result->enter_edge_index = edge_index;
					result->enter_surface_index = edge->surface_indices[!side];
				}
			}
			else if (t < result->exit_t)
			{
				result->exit_t = t;
				result->exit_edge_index = edge_index;
				result->exit_surface_index = edge->surface_indices[!side];
			}
		}
		else if ((distance < 0.f) != side)
		{
			result->enter_t = FLT_MAX;
			result->enter_edge_index = edge_index;
			result->enter_surface_index = edge->surface_indices[!side];
			result->exit_t = -FLT_MAX;
			result->exit_edge_index = edge_index;
			result->exit_surface_index = edge->surface_indices[!side];
		}
		edge_index = edge->edge_indices[side];
	}
	while (edge_index != first_edge_index);

	return result->enter_t > result->exit_t;
}

boolean collision_bsp_test_sphere(
	struct collision_bsp const *bsp,
	short breakable_surface_count,
	byte const *breakable_surface_flags,
	real_point3d const *center,
	real radius,
	struct collision_bsp_test_sphere_result *result)
{
	struct test_sphere_data data;
	short collision_function = (bsp == global_collision_bsp) + _collision_function_sphere_intersect_bsp_object;

	collision_log_usage(collision_function);
	collision_log_start_time(&sphere_start_time);

	data.bsp = bsp;
	data.breakable_surface_count = breakable_surface_count;
	data.breakable_surface_flags = breakable_surface_flags;

	data.center = center;
	data.radius = radius;

	data.result = result;
	data.stack_depth = 0;

	result->leaf_count = 0;
	result->surface_count = 0;
	result->edge_count = 0;
	result->vertex_count = 0;

	bsp3d_test_sphere_recursive(&data, BSP3D_ROOT_NODE_INDEX);

	collision_log_end_time(collision_function, sphere_start_time);

	return result->surface_count > 0 || result->edge_count > 0;
}

static void bsp3d_test_sphere_recursive(
	struct test_sphere_data *data,
	long child_index)
{
	if (!(child_index & LONG_MIN))
	{
		struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp3d.nodes, child_index, struct bsp3d_node);
		real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp3d.planes, node->plane, real_plane3d);
		real distance = plane3d_distance_to_point(plane, data->center);
		boolean back = distance < data->radius;
		boolean front = distance > -data->radius;

		if (front && back)
		{
			match_assert("c:\\halo\\SOURCE\\physics\\collision_bsp.c", 518, data->stack_depth>=0 && data->stack_depth<MAXIMUM_BSP3D_DEPTH);

			data->plane_stack[data->stack_depth++] = node->plane | LONG_MIN;

			bsp3d_test_sphere_recursive(
				data,
				node->child_indices[0]);

			data->stack_depth--;

			match_assert("c:\\halo\\SOURCE\\physics\\collision_bsp.c", 528, data->stack_depth>=0 && data->stack_depth<MAXIMUM_BSP3D_DEPTH);

			data->plane_stack[data->stack_depth++] = node->plane & LONG_MAX;

			bsp3d_test_sphere_recursive(
				data,
				node->child_indices[1]);

			data->stack_depth--;
		}
		else
		{
			bsp3d_test_sphere_recursive(data, node->child_indices[front]);
		}
	}
	else if (child_index != NONE)
	{
		long leaf_index = child_index & LONG_MAX;
		struct collision_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&data->bsp->leaves, leaf_index, struct collision_leaf);
		long i;

		if (data->result->leaf_count < MAXIMUM_COLLISION_LEAVES_PER_TEST)
		{
			data->result->leaf_indices[data->result->leaf_count++] = leaf_index;
		}

		for (i = leaf->first_bsp2d_reference_index;
			i < leaf->first_bsp2d_reference_index + leaf->bsp2d_reference_count;
			i++)
		{
			struct bsp2d_reference *reference = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp2d_references, i, struct bsp2d_reference);
			short j;

			for (j = 0; j < data->stack_depth; j++)
			{
				if (data->plane_stack[j] == reference->plane_designator)
				{
					real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(
						&data->bsp->bsp3d.planes,
						reference->plane_designator & LONG_MAX,
						real_plane3d);
					real distance = plane3d_distance_to_point(plane, data->center);
					real_point3d center_projected_on_plane;

					point_from_line3d(
						data->center, &plane->n, -distance,
						&center_projected_on_plane);

					data->projection_axis = projection_from_vector3d(&plane->n);
					data->projection_sign =
						projection_sign_from_vector3d(&plane->n, data->projection_axis) !=
						((reference->plane_designator & LONG_MIN) != 0);
					project_point3d(&center_projected_on_plane,
						data->projection_axis, data->projection_sign, &data->center2d);

					bsp2d_test_sphere_recursive(data, reference->root_index);

					break;
				}
			}
		}
	}

	return;
}

static void bsp2d_test_sphere_recursive(
	struct test_sphere_data *data,
	long child_index)
{
	if (child_index & LONG_MIN)
	{
		collision_surface_test_sphere(data, child_index & LONG_MAX);
	}
	else
	{
		struct bsp2d_node *node = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp2d.nodes, child_index, struct bsp2d_node);
		real distance = plane2d_distance_to_point(&node->plane, &data->center2d);
		boolean back = distance <= data->radius;
		boolean front = distance >= -data->radius;

		if (back)
		{
			bsp2d_test_sphere_recursive(data, node->child_indices[0]);
		}

		if (front)
		{
			bsp2d_test_sphere_recursive(data, node->child_indices[1]);
		}
	}

	return;
}

static void collision_surface_test_sphere(
	struct test_sphere_data *data,
	long surface_index)
{
	struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&data->bsp->surfaces, surface_index, struct collision_surface);

	if (!TEST_FLAG(surface->flags, _collision_surface_breakable_bit) ||
		surface->breakable_surface_index >= data->breakable_surface_count ||
		BIT_VECTOR_TEST_FLAG((unsigned long const *)data->breakable_surface_flags, surface->breakable_surface_index))
	{
		boolean found = FALSE;

		{
			long edge_index = surface->first_edge_index;
			real radius_squared = data->radius * data->radius;
			real distance_squared;

			do
			{
				struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&data->bsp->edges, edge_index, struct collision_edge);
				boolean side = edge->surface_indices[1] == surface_index;
				long vertex_index = edge->vertex_indices[side];
				struct collision_vertex *vertex = TAG_BLOCK_GET_ELEMENT(&data->bsp->vertices, vertex_index, struct collision_vertex);

				distance_squared = fast_distance_squared3d(&vertex->point, data->center);

				if (distance_squared <= radius_squared)
				{
					add_feature(&data->result->vertex_count, data->result->vertex_indices, vertex_index);
					found = TRUE;
				}

				edge_index = edge->edge_indices[side];
			}
			while (edge_index != surface->first_edge_index);
		}

		{
			long edge_index = surface->first_edge_index;

			do
			{
				struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&data->bsp->edges, edge_index, struct collision_edge);
				boolean side = edge->surface_indices[1] == surface_index;
				struct collision_vertex *vertex0 = TAG_BLOCK_GET_ELEMENT(&data->bsp->vertices, edge->vertex_indices[side], struct collision_vertex);
				struct collision_vertex *vertex1 = TAG_BLOCK_GET_ELEMENT(&data->bsp->vertices, edge->vertex_indices[!side], struct collision_vertex);
				real_vector3d v;

				vector_from_points3d(&vertex0->point, &vertex1->point, &v);

				if (fast_vector_intersects_sphere(&vertex0->point, &v, data->center, data->radius))
				{
					add_feature(&data->result->edge_count, data->result->edge_indices, edge_index);
					found = TRUE;
				}

				edge_index = edge->edge_indices[side];
			}
			while (edge_index != surface->first_edge_index);
		}

		{
			boolean inside = TRUE;

			if (!found)
			{
				long edge_index = surface->first_edge_index;

				do
				{
					struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&data->bsp->edges, edge_index, struct collision_edge);
					boolean side = edge->surface_indices[1] == surface_index;
					struct collision_vertex *vertex0 = TAG_BLOCK_GET_ELEMENT(&data->bsp->vertices, edge->vertex_indices[side], struct collision_vertex);
					struct collision_vertex *vertex1 = TAG_BLOCK_GET_ELEMENT(&data->bsp->vertices, edge->vertex_indices[!side], struct collision_vertex);
					real_point2d p0;
					real_point2d p1;
					real_vector2d v0;
					real_vector2d v1;

					project_point3d(&vertex0->point, data->projection_axis, data->projection_sign, &p0);
					project_point3d(&vertex1->point, data->projection_axis, data->projection_sign, &p1);

					vector_from_points2d(&data->center2d, &p0, &v0);
					vector_from_points2d(&data->center2d, &p1, &v1);

					if (cross_product2d(&v0, &v1) < 0.f)
					{
						inside = FALSE;
						break;
					}

					edge_index = edge->edge_indices[side];
				}
				while (edge_index != surface->first_edge_index);
			}

			if (found || inside)
			{
				add_feature(&data->result->surface_count, data->result->surface_indices, surface_index);
			}
		}
	}

	return;
}

void render_debug_collision_vertex(
	struct collision_bsp const *bsp,
	long vertex_index,
	real_matrix4x3 const *matrix,
	real size,
	real_argb_color const *color)
{
	struct collision_vertex *vertex = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, vertex_index, struct collision_vertex);
	real_point3d transformed_point;
	real_point3d const *point = &vertex->point;

	if (matrix)
	{
		point = matrix4x3_transform_point(matrix, point, &transformed_point);
	}

	render_debug_point(TRUE, point, size, color);

	return;
}

void render_debug_collision_edge(
	struct collision_bsp const *bsp,
	long edge_index,
	real_matrix4x3 const *matrix,
	real_argb_color const *color)
{
	struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
	struct collision_vertex *v0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[0], struct collision_vertex);
	real_point3d *point0 = &v0->point;
	struct collision_vertex *v1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[1], struct collision_vertex);
	real_point3d *point1 = &v1->point;
	real_point3d transformed_point1;
	real_point3d transformed_point0;

	if (matrix)
	{
		point0 = matrix4x3_transform_point(matrix, &v0->point, &transformed_point0);
		point1 = matrix4x3_transform_point(matrix, &v1->point, &transformed_point1);
	}

	render_debug_line(TRUE, point0, point1, color);
}

void render_debug_collision_surface(
	struct collision_bsp const *bsp,
	long surface_index,
	real_matrix4x3 const *matrix,
	real_argb_color const *color)
{
	struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);
	long first_edge_index = surface->first_edge_index;
	long edge_index = first_edge_index;

	do
	{
		struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
		boolean side = edge->surface_indices[1] == surface_index;

		render_debug_collision_edge(bsp, edge_index, matrix, color);
		edge_index = edge->edge_indices[side];
	}
	while (edge_index != first_edge_index);

	return;
}

void render_debug_collision_bsp(
	struct collision_bsp *bsp,
	real_matrix4x3 const *matrix)
{
	long i;

	for (i = 0; i < bsp->edges.count; ++i)
	{
		render_debug_collision_edge(bsp, i, matrix, global_real_argb_green);
	}

	return;
}

static void add_feature(
	long *count,
	long *indices,
	long index)
{
	short i;

	for (i = 0; i < *count; i++)
	{
		if (indices[i] == index)
		{
			return;
		}
	}

	if (*count < MAXIMUM_COLLISION_FEATURES_PER_TEST)
	{
		indices[(*count)++] = index;
	}

	return;
}

boolean collision_bsp_test_vector(
	unsigned long flags,
	struct collision_bsp const *bsp,
	short breakable_surface_count,
	byte const *breakable_surface_flags,
	real_point3d const *point,
	real_vector3d const *vector,
	real maximum_t,
	struct collision_bsp_test_vector_result *result)
{
	struct test_vector_data data;
	short collision_function = (bsp == global_collision_bsp) + _collision_function_vector_intersect_bsp_object;
	boolean collision;

	collision_log_usage(collision_function);
	collision_log_start_time(&vector_start_time);

	data.flags = flags;
	data.bsp = bsp;
	data.breakable_surface_count = breakable_surface_count;
	data.breakable_surface_flags = breakable_surface_flags;

	data.point = point;
	data.vector = vector;

	data.result = result;

	result->t = maximum_t < 0.f ? 0.f : maximum_t;

	data.last_leaf_index = NONE;
	data.last_plane_index = NONE;
	result->leaf_count = 0;
	data.last_contents = _contents_unknown;

	collision = collision_bsp_test_vector_recursive(
		&data,
		BSP3D_ROOT_NODE_INDEX,
		0.f,
		PIN(maximum_t, 0.f, 1.f));

	collision_log_end_time(collision_function, vector_start_time);

	return collision;
}

static boolean collision_bsp_test_vector_recursive(
	struct test_vector_data *data,
	long child_index,
	real t0,
	real t1)
{
	if (!(child_index & LONG_MIN))
	{
		struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp3d.nodes, child_index, struct bsp3d_node);
		real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp3d.planes, node->plane, real_plane3d);
		real distance = plane3d_distance_to_point(plane, data->point);
		real direction = dot_product3d(data->vector, &plane->n);
		real distance0 = distance + direction * t0;
		real distance1 = distance + direction * t1;
		boolean back = distance0 < 0.f || distance1 < 0.f;
		boolean front = distance0 >= 0.f || distance1 >= 0.f;

		if (back && front)
		{
			boolean entering_positive = direction > 0.f;
			real t = -(distance / direction);

			if (collision_bsp_test_vector_recursive(data, node->child_indices[!entering_positive], t0, t))
			{
				return TRUE;
			}

			if (data->result->t <= t)
			{
				return FALSE;
			}

			data->last_plane_index = node->plane;
			if (collision_bsp_test_vector_recursive(data, node->child_indices[entering_positive], t, t1))
			{
				return TRUE;
			}
		}
		else if (collision_bsp_test_vector_recursive(data, node->child_indices[front], t0, t1))
		{
			return TRUE;
		}
	}
	else
	{
		long leaf_index = NONE;
		byte contents = _contents_solid;
		long test_leaf_index = NONE;
		boolean test_surface = FALSE;

		if (child_index != NONE)
		{
			struct collision_leaf *leaf;

			leaf_index = child_index & LONG_MAX;
			leaf = TAG_BLOCK_GET_ELEMENT(&data->bsp->leaves, leaf_index, struct collision_leaf);
			contents = TEST_FLAG(leaf->flags, _collision_leaf_contains_two_sided_bit) ? _contents_semi_empty : _contents_empty;
		}

		if (TEST_FLAG(data->flags, _collision_bsp_test_front_facing_surfaces_bit) &&
			(data->last_contents == _contents_empty || data->last_contents == _contents_semi_empty) && contents == _contents_solid)
		{
			test_leaf_index = data->last_leaf_index;
		}
		else if (TEST_FLAG(data->flags, _collision_bsp_test_back_facing_surfaces_bit) && data->last_contents == _contents_solid &&
			(contents == _contents_empty || contents == _contents_semi_empty))
		{
			test_leaf_index = leaf_index;
		}
		else if (!TEST_FLAG(data->flags, _collision_bsp_test_ignore_two_sided_surfaces_bit) &&
			data->last_contents == _contents_semi_empty && contents == _contents_semi_empty)
		{
			test_leaf_index = TEST_FLAG(data->flags, _collision_bsp_test_front_facing_surfaces_bit) ?
				data->last_leaf_index : leaf_index;
			test_surface = TRUE;
		}

		if (test_leaf_index != NONE)
		{
			long surface_index = collision_leaf_test_vector(
				data->bsp, data->breakable_surface_count, data->breakable_surface_flags,
				data->point, data->vector, test_leaf_index, data->last_plane_index, t0, test_surface);

			if (surface_index != NONE)
			{
				struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&data->bsp->surfaces, surface_index, struct collision_surface);

				if ((!TEST_FLAG(surface->flags, _collision_surface_invisible_bit) || !TEST_FLAG(data->flags, _collision_bsp_test_ignore_invisible_surfaces_bit)) &&
					(!TEST_FLAG(surface->flags, _collision_surface_breakable_bit) || !TEST_FLAG(data->flags, _collision_bsp_test_ignore_breakable_surfaces_bit)))
				{
					data->result->t = t0;
					data->result->plane = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp3d.planes, data->last_plane_index, real_plane3d);
					data->result->surface_index = surface_index;
					data->result->plane_designator = surface->plane_designator;
					data->result->flags = surface->flags;
					data->result->breakable_surface_index = surface->breakable_surface_index;
					data->result->material_index = surface->material_index;

					return TRUE;
				}
			}
		}

		if (leaf_index != NONE)
		{
			if (data->result->leaf_count < MAXIMUM_COLLISION_LEAVES_PER_TEST)
			{
				data->result->leaf_indices[data->result->leaf_count++] = leaf_index;
			}
			else
			{
				data->result->leaf_indices[MAXIMUM_COLLISION_LEAVES_PER_TEST - 1] = leaf_index;
			}
		}

		data->last_leaf_index = leaf_index;
		data->last_contents = contents;
	}

	return FALSE;
}

static long collision_leaf_test_vector(
	struct collision_bsp const *bsp,
	short breakable_surface_count,
	byte const *breakable_surface_flags,
	real_point3d const *point,
	real_vector3d const *vector,
	long leaf_index,
	long plane_index,
	real t,
	boolean test_surface_flag)
{
	struct collision_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&bsp->leaves, leaf_index, struct collision_leaf);
	long i;

	for (i = leaf->first_bsp2d_reference_index; i < leaf->first_bsp2d_reference_index + leaf->bsp2d_reference_count; i++)
	{
		struct bsp2d_reference *reference = TAG_BLOCK_GET_ELEMENT(&bsp->bsp2d_references, i, struct bsp2d_reference);

		if ((reference->plane_designator & LONG_MAX) == plane_index)
		{
			real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&bsp->bsp3d.planes, plane_index, real_plane3d);
			short projection_axis = projection_from_vector3d(&plane->n);
			boolean projection_sign = projection_sign_from_vector3d(&plane->n, projection_axis) != ((reference->plane_designator & LONG_MIN) != 0);
			real_point2d p2d;
			real_point3d p3d;
			long surface_index;

			point_from_line3d(point, vector, t, &p3d);
			project_point3d(&p3d, projection_axis, projection_sign, &p2d);

			surface_index = bsp2d_test_point(&bsp->bsp2d, &p2d, reference->root_index);

			if (!test_surface_flag || collision_surface_test_point(bsp,
				breakable_surface_count, breakable_surface_flags, surface_index,
				projection_axis, projection_sign, &p2d))
			{
				return surface_index;
			}
		}
	}

	return NONE;
}

static boolean collision_surface_test_point(
	struct collision_bsp const *bsp,
	short breakable_surface_count,
	byte const *breakable_surface_flags,
	long surface_index,
	short projection_axis,
	boolean projection_sign,
	real_point2d const *point)
{
	struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);

	if (!TEST_FLAG(surface->flags, _collision_surface_breakable_bit) ||
		surface->breakable_surface_index >= breakable_surface_count ||
		BIT_VECTOR_TEST_FLAG((unsigned long const *)breakable_surface_flags, surface->breakable_surface_index))
	{
		long edge_index = surface->first_edge_index;

		while (TRUE)
		{
			struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
			boolean side = edge->surface_indices[1] == surface_index;
			struct collision_vertex *vertex0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[side], struct collision_vertex);
			struct collision_vertex *vertex1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[!side], struct collision_vertex);
			real_point2d p0;
			real_point2d p1;
			real_vector2d v0;
			real_vector2d v1;

			project_point3d(&vertex0->point, projection_axis, projection_sign, &p0);
			project_point3d(&vertex1->point, projection_axis, projection_sign, &p1);
			vector_from_points2d(&p0, point, &v0);
			vector_from_points2d(&p0, &p1, &v1);

			if (cross_product2d(&v0, &v1) > 0.f)
			{
				break;
			}

			edge_index = edge->edge_indices[side];
			if (edge_index == surface->first_edge_index)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

boolean collision_bsp_test_pill(
	struct collision_bsp const *bsp,
	real_point3d const *point,
	real_vector3d const *vector,
	real radius,
	real maximum_t,
	struct collision_bsp_test_pill_result *result)
{
	struct test_pill_data data;

	data.bsp = bsp;
	data.point = point;
	data.vector = vector;
	data.radius = radius;
	data.result = result;
	data.stack_depth = 0;

	result->t = maximum_t < 0.f ? 0.f : maximum_t;
	result->leaf_count = 0;

	return bsp3d_test_pill_recursive(&data, BSP3D_ROOT_NODE_INDEX);
}

static boolean bsp3d_test_pill_recursive(
	struct test_pill_data *data,
	long child_index)
{
	boolean result = FALSE;

	if (!(child_index & LONG_MIN))
	{
		struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp3d.nodes, child_index, struct bsp3d_node);
		real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp3d.planes, node->plane, real_plane3d);

		real distance = plane3d_distance_to_point(plane, data->point);
		real direction = dot_product3d(data->vector, &plane->n);
		real distance0 = distance;
		real distance1 = distance + direction;

		boolean back =
			distance0 <= data->radius + 0.000244140625f ||
			distance1 <= data->radius + 0.000244140625f;
		boolean front =
			distance0 >= -data->radius - 0.000244140625f ||
			distance1 >= -data->radius - 0.000244140625f;

		if (back && front)
		{
			boolean entering_positive = direction > 0.f;

			match_assert("c:\\halo\\SOURCE\\physics\\collision_bsp.c", 1176, data->stack_depth>=0 && data->stack_depth<MAXIMUM_BSP3D_DEPTH);

			data->plane_stack[data->stack_depth++] =
				entering_positive ? node->plane | LONG_MIN : node->plane & LONG_MAX;

			if (bsp3d_test_pill_recursive(data, node->child_indices[!entering_positive]))
			{
				result = TRUE;
			}

			data->stack_depth--;

			if (bsp3d_test_pill_recursive(data, node->child_indices[entering_positive]))
			{
				result = TRUE;
			}
		}
		else if (bsp3d_test_pill_recursive(data, node->child_indices[front]))
		{
			result = TRUE;
		}
	}
	else
	{
		if (child_index != NONE)
		{
			long leaf_index = child_index & LONG_MAX;
			struct collision_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&data->bsp->leaves, leaf_index, struct collision_leaf);
			long i;

			for (i = leaf->first_bsp2d_reference_index;
				i < leaf->first_bsp2d_reference_index + leaf->bsp2d_reference_count;
				i++)
			{
				struct bsp2d_reference *reference = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp2d_references, i, struct bsp2d_reference);
				short j;

				for (j = 0; j < data->stack_depth; j++)
				{
					if (data->plane_stack[j] == reference->plane_designator)
					{
						real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp3d.planes, reference->plane_designator & LONG_MAX, real_plane3d);
						real distance = plane3d_distance_to_point(plane, data->point);
						real direction = dot_product3d(data->vector, &plane->n);
						real t = 0.f;

						if (direction != 0.f)
						{
							real inverse = 1.f / direction;

							t = -distance * inverse - fabs(inverse) * data->radius;
							t = PIN(t, 0.f, 1.f);
						}

						if (data->result->t > t)
						{
							real projection_distance;
							real_point3d p3d;
							real_point2d p2d;
							long surface_index;

							data->projection_axis = projection_from_vector3d(&plane->n);
							data->projection_sign =
								projection_sign_from_vector3d(&plane->n, data->projection_axis) !=
								((reference->plane_designator & LONG_MIN) != 0);

							point_from_line3d(data->point, data->vector, t, &p3d);
							projection_distance = plane3d_distance_to_point(plane, &p3d);
							point_from_line3d(&p3d, &plane->n, -projection_distance, &p3d);
							project_point3d(&p3d, data->projection_axis, data->projection_sign, &p2d);

							surface_index = bsp2d_test_point(&data->bsp->bsp2d, &p2d, reference->root_index);

							if (collision_surface_test_point(data->bsp, 0, NULL, surface_index,
								data->projection_axis, data->projection_sign, &p2d))
							{
								struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&data->bsp->surfaces, surface_index, struct collision_surface);

								data->result->t = t;

								if (reference->plane_designator & LONG_MIN)
								{
									plane3d_negate(plane, &data->result->plane);
								}
								else
								{
									data->result->plane = *plane;
								}

								data->result->surface_index = surface_index;
								data->result->material_index = surface->material_index;

								result = TRUE;
							}

							{
								real_point3d p3d;

								point_from_line3d(data->point, &plane->n, (-distance), &p3d);
								project_point3d(&p3d, data->projection_axis, data->projection_sign, &data->p2d);
							}

							{
								real_vector3d v3d;

								point_from_line3d((real_point3d const *)data->vector, &plane->n, (-direction), (real_point3d *)&v3d);
								project_point3d((real_point3d const *)&v3d, data->projection_axis, data->projection_sign, (real_point2d *)&data->v2d);
							}

							if (bsp2d_test_pill_recursive(data, reference->root_index))
							{
								result = TRUE;
							}
						}

						break;
					}
				}
			}

			if (data->result->leaf_count < MAXIMUM_COLLISION_LEAVES_PER_TEST)
			{
				data->result->leaf_indices[data->result->leaf_count++] = leaf_index;
			}
			else
			{
				data->result->leaf_indices[MAXIMUM_COLLISION_LEAVES_PER_TEST - 1] = leaf_index;
			}
		}
	}

	return result;
}

static boolean bsp2d_test_pill_recursive(
	struct test_pill_data *data,
	long child_index)
{
	boolean result = FALSE;

	if (!(child_index & LONG_MIN))
	{
		struct bsp2d_node *node = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp2d.nodes, child_index, struct bsp2d_node);
		real distance0 = plane2d_distance_to_point(&node->plane, &data->p2d);
		real direction = dot_product2d(&data->v2d, &node->plane.n);
		real distance1 = distance0 + direction;
		boolean back = distance0 <= data->radius + 0.0001220703125f || distance1 <= data->radius + 0.0001220703125f;
		boolean front = distance0 >= -data->radius - 0.0001220703125f || distance1 >= -data->radius - 0.0001220703125f;

		if ((back && bsp2d_test_pill_recursive(data, node->child_indices[0])) ||
			(front && bsp2d_test_pill_recursive(data, node->child_indices[1])))
		{
			result = TRUE;
		}
	}
	else
	{
		if (collision_surface_test_pill(data, child_index & LONG_MAX))
		{
			result = TRUE;
		}
	}

	return result;
}

static boolean collision_surface_test_pill(
	struct test_pill_data *data,
	long surface_index)
{
	boolean result = FALSE;
	struct collision_surface *surface = TAG_BLOCK_GET_ELEMENT(&data->bsp->surfaces, surface_index, struct collision_surface);
	long edge_index = surface->first_edge_index;

	do
	{
		struct collision_edge *edge = TAG_BLOCK_GET_ELEMENT(&data->bsp->edges, edge_index, struct collision_edge);
		boolean side = edge->surface_indices[1] == surface_index;
		struct collision_vertex *vertex0 = TAG_BLOCK_GET_ELEMENT(&data->bsp->vertices, edge->vertex_indices[side], struct collision_vertex);
		struct collision_vertex *vertex1 = TAG_BLOCK_GET_ELEMENT(&data->bsp->vertices, edge->vertex_indices[!side], struct collision_vertex);
		real_vector3d v;
		real t;
		real u;

		vector_from_points3d(&vertex0->point, &vertex1->point, &v);

		if (pill_test_vector(
			&vertex0->point, &v, data->radius, data->point, data->vector, &t, &u) &&
			data->result->t > t)
		{
			real_point3d p0;
			real_point3d p1;

			data->result->t = t;

			point_from_line3d(&vertex0->point, &v, u, &p0);
			point_from_line3d(data->point, data->vector, t, &p1);

			vector_from_points3d(&p0, &p1, &data->result->plane.n);
			normalize3d(&data->result->plane.n);
			data->result->plane.d = FLT_MAX;

			data->result->surface_index = surface_index;
			data->result->material_index = surface->material_index;

			result = TRUE;
		}

		edge_index = edge->edge_indices[side];
	}
	while (edge_index != surface->first_edge_index);

	return result;
}

static boolean pill_test_vector(
	real_point3d const *base,
	real_vector3d const *height,
	real width,
	real_point3d const *point,
	real_vector3d const *vector,
	real *t_reference,
	real *u_reference)
{
	real_vector3d v;
	real height_squared;
	real height_dot_vector;
	real vector_squared;
	real a;

	vector_from_points3d(base, point, &v);
	height_squared = magnitude_squared3d(height);
	height_dot_vector = dot_product3d(height, vector);
	vector_squared = magnitude_squared3d(vector);
	a = height_squared * vector_squared - height_dot_vector * height_dot_vector;

	if (a != 0.f)
	{
		real height_dot_v = dot_product3d(height, &v);
		real vector_dot_v = dot_product3d(vector, &v);
		real v_squared = magnitude_squared3d(&v);
		real width_squared = width * width;
		real b = height_dot_v * height_dot_vector - height_squared * vector_dot_v;
		real c = (v_squared - width_squared) * height_squared - height_dot_v * height_dot_v;
		real discriminant = b * b - a * c;

		if (discriminant >= 0.f)
		{
			real root = square_root(discriminant);
			real inverse = 1.f / a;
			real t0 = -(root + b) * inverse;
			real t1 = -(b - root) * inverse;

			if (t0 <= 1.f && t1 >= 0.f)
			{
				real t = t0 < 0.f ? 0.f : t0;
				real u = height_dot_vector * t + height_dot_v;

				if (u < 0.f)
				{
					if (sphere_test_vector(base, width, point, vector, t_reference))
					{
						*u_reference = 0.f;
						return TRUE;
					}
				}
				else if (u > height_squared)
				{
					real_point3d center;

					center.x = base->x + height->i;
					center.y = base->y + height->j;
					center.z = base->z + height->k;

					if (sphere_test_vector(&center, width, point, vector, t_reference))
					{
						*u_reference = 1.f;
						return TRUE;
					}
				}
				else
				{
					*t_reference = t;
					*u_reference = u / height_squared;

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

static boolean sphere_test_vector(
	real_point3d const *center,
	real radius,
	real_point3d const *point,
	real_vector3d const *vector,
	real *t_reference)
{
	real_vector3d v;
	real c;
	boolean result;

	vector_from_points3d(point, center, &v);
	c = magnitude_squared3d(&v) - radius * radius;

	if (c <= 0.f)
	{
		*t_reference = 0.f;

		result = TRUE;
	}
	else
	{
		real b = dot_product3d(&v, vector);

		if (b > 0.f)
		{
			real a = magnitude_squared3d(vector);
			real discriminant = b * b - a * c;

			if (discriminant >= 0.f)
			{
				real t = (b - square_root(discriminant)) / a;

				if (t <= 1.f)
				{
					*t_reference = t;

					result = TRUE;
				}
				else
				{
					result = FALSE;
				}
			}
			else
			{
				result = FALSE;
			}
		}
		else
		{
			result = FALSE;
		}
	}

	return result;
}

boolean collision_bsp_test_pill_new(
	struct collision_bsp const *bsp,
	short breakable_surface_count,
	byte const *breakable_surface_flags,
	real_point3d const *point,
	real_vector3d const *vector,
	real radius,
	real *t,
	real_vector3d *normal)
{
	struct test_pill_new_data data;

	data.bsp = bsp;
	data.breakable_surface_count = breakable_surface_count;
	data.breakable_surface_flags = breakable_surface_flags;

	data.point = point;
	data.vector = vector;
	data.radius = radius;

	data.t = t;
	data.normal = normal;

	data.last_leaf_index = NONE;
	data.last_contents = _contents_unknown;
	data.last_plane_designator = NONE;

	*t = FLT_MAX;

	return collision_bsp_test_pill_new_recursive(&data, 0, 0.f, 1.f);
}

static boolean collision_bsp_test_pill_new_recursive(
	struct test_pill_new_data *data,
	long child_index,
	real t0,
	real t1)
{
	if (!(child_index & LONG_MIN))
	{
		struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp3d.nodes, child_index, struct bsp3d_node);
		real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp3d.planes, node->plane, real_plane3d);
		real distance = plane3d_distance_to_point(plane, data->point);
		real direction = dot_product3d(&plane->n, data->vector);
		real distance0 = distance + direction * t0;
		real distance1 = distance + direction * t1;
		boolean back = distance0 < data->radius || distance1 < data->radius;
		boolean front = distance0 > -data->radius || distance1 > -data->radius;

		if (back && front)
		{
			boolean entering_positive = direction > 0.f;
			real minimum;
			real maximum;
			boolean result;

			if (direction != 0.f)
			{
				real inverse = 1.f / direction;
				real a = -(distance + data->radius) * inverse;
				real b = -(distance - data->radius) * inverse;

				maximum = MAX(a, b);
				minimum = MIN(a, b);

				maximum = PIN(maximum, t0, t1);
				minimum = PIN(minimum, t0, t1);
			}
			else
			{
				maximum = t1;
				minimum = t0;
			}

			result = collision_bsp_test_pill_new_recursive(data, node->child_indices[!entering_positive], t0, maximum);

			if (result)
			{
				if (minimum >= *data->t)
				{
					return TRUE;
				}

				t1 = *data->t;
			}

			data->last_plane_designator = entering_positive ? node->plane | LONG_MIN : node->plane & LONG_MAX;
			result |= collision_bsp_test_pill_new_recursive(data, node->child_indices[entering_positive], minimum, t1);

			return result;
		}

		return collision_bsp_test_pill_new_recursive(data, node->child_indices[front], t0, t1);
	}
	else
	{
		if (child_index == NONE && data->last_plane_designator != NONE)
		{
			real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&data->bsp->bsp3d.planes, data->last_plane_designator & LONG_MAX, real_plane3d);

			*data->t = t0;
			if (data->last_plane_designator & LONG_MIN)
			{
				negate_vector3d(&plane->n, data->normal);
			}
			else
			{
				*data->normal = plane->n;
			}

			return TRUE;
		}

		return FALSE;
	}
}
