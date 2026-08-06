/*
BSP3D.C
*/

/* ---------- headers */

#include "cseries.h"
#include "bsp3d.h"

#include "math/geometry.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	MAXIMUM_VERTICES_PER_CLIPPED_POLYGON = 64
};

#define CLIP_LINE_EPSILON (1.0f / 4096.0f)

/* ---------- prototypes */

static long bsp3d_clip_polygon_to_leaves_recursive(
	struct bsp3d const *bsp,
	long node_index,
	long on_node_designator,
	real_point3d const *vertices,
	short vertex_count,
	real epsilon,
	polygon_proc handler, 
	void *user_data);

/* ---------- globals */

long node_count;

/* ---------- public code */

long bsp3d_test_point(
	struct bsp3d const *bsp,
	long node_index,
	real_point3d const *point)
{
	do
	{
		struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&bsp->nodes, node_index, struct bsp3d_node);
		real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&bsp->planes, node->plane, real_plane3d);
		
		node_index = node->child_indices[plane3d_distance_to_point(plane, point) >= 0.0f];
	}
	while (!(node_index & LONG_MIN));

	return (node_index != NONE) ? (node_index & LONG_MAX) : NONE;
}

long bsp3d_clip_line_to_leaves(
	struct bsp3d const *bsp,
	long node_index,
	real_point3d const *p0,
	real_point3d const *p1,
	line_proc handler,
	void *user_data)
{
	real_point3d intersection;
	boolean p0_flags[2];
	boolean p1_flags[2];
	short child_index;

	long intersected_leaf_count = 0;
	struct bsp3d_node const *node = TAG_BLOCK_GET_ELEMENT(&bsp->nodes, node_index, struct bsp3d_node);
	real_plane3d const *plane = TAG_BLOCK_GET_ELEMENT(&bsp->planes, node->plane, real_plane3d);
	real p0_d = plane3d_distance_to_point(plane, p0);
	real p1_d = plane3d_distance_to_point(plane, p1);

	if (!node_index)
	{
		node_count = 0;
	}

	++node_count;

	p0_flags[0] = p0_d < -CLIP_LINE_EPSILON;
	p0_flags[1] = p0_d > CLIP_LINE_EPSILON;
	p1_flags[0] = p1_d < -CLIP_LINE_EPSILON;
	p1_flags[1] = p1_d > CLIP_LINE_EPSILON;

	if (p0_flags[0] && p1_flags[1] || p0_flags[1] && p1_flags[0])
	{
		real_vector3d p0p1;
		real t;

		vector_from_points3d(p0, p1, &p0p1);
		t = vector_intersect_plane3d(p0, &p0p1, plane);
		
		match_assert("c:\\halo\\SOURCE\\physics\\bsp3d.c", 73, t>0.f && t<1.f);

		point_from_line3d(p0, &p0p1, t, &intersection);
	}

	for (child_index = 0; child_index < 2; child_index++)
	{
		if (p0_flags[child_index] || p1_flags[child_index] ||
			!p0_flags[child_index==0] && !p1_flags[child_index==0])
		{
			real_point3d const *c0 = p0_flags[child_index == 0] ? &intersection : p0;
			real_point3d const *c1 = p1_flags[child_index == 0] ? &intersection : p1;
			long child = node->child_indices[child_index];

			if (child & LONG_MIN)
			{
				if (child != NONE)
				{
					if (handler)
					{
						handler(c0, c1, child & LONG_MAX, user_data);
					}

					++intersected_leaf_count;
				}
			}
			else
			{
				intersected_leaf_count += bsp3d_clip_line_to_leaves(bsp, child, c0, c1, handler, user_data);
			}
		}
	}

	return intersected_leaf_count;
}

long bsp3d_clip_polygon_to_leaves(
	struct bsp3d const *bsp,
	long node_index,
	real_point3d const *vertices,
	short vertex_count,
	real epsilon,
	polygon_proc handler,
	void *user_data)
{
	return bsp3d_clip_polygon_to_leaves_recursive(bsp, node_index, NONE, vertices, vertex_count, epsilon, handler, user_data);
}

/* ---------- private code */

static long bsp3d_clip_polygon_to_leaves_recursive(
	struct bsp3d const *bsp,
	long node_index,
	long on_node_designator,
	real_point3d const *vertices,
	short vertex_count,
	real epsilon,
	polygon_proc handler,
	void *user_data)
{
	real_point3d clipped_polygon_storage[2][MAXIMUM_VERTICES_PER_CLIPPED_POLYGON];
	real_point3d const *clipped_polygons[2];
	short clipped_counts[2];

	short vertex_index;
	short child_index;

	long intersected_leaf_count = 0;
	struct bsp3d_node const *node = TAG_BLOCK_GET_ELEMENT(&bsp->nodes, node_index, struct bsp3d_node);
	real_plane3d const *plane = TAG_BLOCK_GET_ELEMENT(&bsp->planes, node->plane, real_plane3d);

	match_assert("c:\\halo\\SOURCE\\physics\\bsp3d.c", 149, vertex_count>=NUMBER_OF_VERTICES_PER_TRIANGLE);

	match_assert("c:\\halo\\SOURCE\\physics\\bsp3d.c", 151, vertex_count<MAXIMUM_VERTICES_PER_CLIPPED_POLYGON);

	for (vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
	{
		if (!(fabs(plane3d_distance_to_point(plane, &vertices[vertex_index])-0.0f) < epsilon))
		{
			break;
		}
	}

	if (vertex_index == vertex_count)
	{
		real_vector3d p0p2;
		real_vector3d p0p1;
		real_vector3d n;
		boolean facing;

		// TODO: this is a fake match
		real_vector3d *edge2 = &p0p2;

		vector_from_points3d(&vertices[0], &vertices[1], &p0p1);
		vector_from_points3d(&vertices[0], &vertices[2], edge2);
		cross_product3d(edge2, &p0p1, &n);

		facing = dot_product3d(&n, &plane->n) > 0.0f;

		clipped_polygons[facing] = vertices;
		
		clipped_counts[facing] = vertex_count;
		clipped_counts[!facing] = 0;

		if (facing)
		{
			on_node_designator = LONG_MIN | node_index & LONG_MAX;
		}
		else
		{
			on_node_designator = node_index & LONG_MAX;
		}
	}
	else
	{
		real_plane3d back_plane;

		plane3d_negate(plane, &back_plane);

		clipped_counts[0] = convex_polygon3d_clip_to_plane(
			vertex_count,
			vertices,
			&back_plane,
			NUMBEROF(clipped_polygon_storage[0]),
			clipped_polygon_storage[0],
			NULL,
			epsilon,
			FALSE);
		clipped_counts[1] = convex_polygon3d_clip_to_plane(
			vertex_count,
			vertices,
			plane,
			NUMBEROF(clipped_polygon_storage[1]),
			clipped_polygon_storage[1],
			NULL,
			epsilon,
			FALSE);

		match_assert("c:\\halo\\SOURCE\\physics\\bsp3d.c", 185, clipped_counts[0]!=NONE && clipped_counts[1]!=NONE);
		
		clipped_polygons[0] = clipped_polygon_storage[0];
		clipped_polygons[1] = clipped_polygon_storage[1];
	}

	for (child_index = 0; child_index<NUMBEROF(clipped_counts); ++child_index)
	{
		if (clipped_counts[child_index])
		{
			if (node->child_indices[child_index] & LONG_MIN)
			{
				if (node->child_indices[child_index] != NONE)
				{
					if (handler)
					{
						handler(
							clipped_polygons[child_index],
							clipped_counts[child_index],
							node->child_indices[child_index] & LONG_MAX,
							on_node_designator,
							user_data);
					}

					++intersected_leaf_count;
				}
			}
			else
			{
				intersected_leaf_count += bsp3d_clip_polygon_to_leaves_recursive(
					bsp,
					node->child_indices[child_index],
					on_node_designator,
					clipped_polygons[child_index],
					clipped_counts[child_index],
					epsilon, 
					handler, 
					user_data);
			}
		}
	}

	return intersected_leaf_count;
}
