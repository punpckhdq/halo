/*
BSP3D.C

*/

/* ---------- headers */
#include "cseries.h"
#include "bsp3d.h"
#include "real_math.h"
#include "geometry.h"

/* ---------- constants */

#define MAXIMUM_VERTICES_PER_CLIPPED_POLYGON 64

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static long bsp3d_clip_polygon_to_leaves_recursive(
	struct bsp3d const *bsp,
	long node_index,
	long on_node_designator,
	real_point3d const *vertices,
	short vertex_count,
	real epsilon,
	bsp3d_clip_polygon_to_leaves_proc handler,
	void *data);

/* ---------- globals */

long node_count;

/* ---------- public code */

long bsp3d_test_point(struct bsp3d const *bsp, long node_index, real_point3d const *point)
{
	do
	{
		struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&bsp->nodes, node_index, struct bsp3d_node);
		real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&bsp->planes, node->plane, real_plane3d);
		real d = plane3d_distance_to_point(plane, point);

		node_index = node->child_indices[d >= 0.0f];
	}
	while (!(node_index & LONG_MIN));

	return (node_index != NONE) ? (node_index & LONG_MAX) : NONE;
}

long bsp3d_clip_line_to_leaves(
	struct bsp3d const *bsp,
	long node_index,
	real_point3d const *p0,
	real_point3d const *p1,
	bsp3d_clip_line_to_leaves_proc handler,
	void *data)
{
	real_point3d intersection;
	boolean p0_side[2];
	boolean p1_side[2];
	short i;
	long count = 0;
	struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&bsp->nodes, node_index, struct bsp3d_node);
	real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&bsp->planes, node->plane, real_plane3d);
	real d0 = plane3d_distance_to_point(plane, p0);
	real d1 = plane3d_distance_to_point(plane, p1);

	if (!node_index)
	{
		node_count = 0;
	}
	node_count++;

#define CLIP_LINE_EPSILON (1.0f / 4096.0f)

	p0_side[0] = d0 < -CLIP_LINE_EPSILON;
	p0_side[1] = d0 > CLIP_LINE_EPSILON;
	p1_side[0] = d1 < -CLIP_LINE_EPSILON;
	p1_side[1] = d1 > CLIP_LINE_EPSILON;

	if ((p0_side[0] && p1_side[1]) || (p0_side[1] && p1_side[0]))
	{
		real_vector3d delta;
		real t;

		vector_from_points3d(p0, p1, &delta);
		t = vector_intersect_plane3d(p0, &delta, plane);
		match_assert("c:\\halo\\SOURCE\\physics\\bsp3d.c", 73, t>0.f && t<1.f);
		point_from_line3d(p0, &delta, t, &intersection);
	}

	for (i = 0; i < 2; i++)
	{
		if (p0_side[i] || p1_side[i] || (!p0_side[i == 0] && !p1_side[i == 0]))
		{
			real_point3d const *segment_p0 = p0_side[i == 0] ? &intersection : p0;
			real_point3d const *segment_p1 = p1_side[i == 0] ? &intersection : p1;
			long child = node->child_indices[i];
			if (child & LONG_MIN)
			{
				if (child != NONE)
				{
					if (handler)
					{
						handler(segment_p0, segment_p1, child & LONG_MAX, data);
					}
					count++;
				}
			}
			else
			{
				count += bsp3d_clip_line_to_leaves(bsp, child, segment_p0, segment_p1, handler, data);
			}
		}
	}

	return count;
}

long bsp3d_clip_polygon_to_leaves(
	struct bsp3d const *bsp,
	long node_index,
	real_point3d const *vertices,
	short vertex_count,
	real epsilon,
	bsp3d_clip_polygon_to_leaves_proc handler,
	void *data)
{
	return bsp3d_clip_polygon_to_leaves_recursive(bsp, node_index, NONE, vertices, vertex_count, epsilon, handler, data);
}

/* ---------- private code */

static long bsp3d_clip_polygon_to_leaves_recursive(
	struct bsp3d const *bsp,
	long node_index,
	long on_node_designator,
	real_point3d const *vertices,
	short vertex_count,
	real epsilon,
	bsp3d_clip_polygon_to_leaves_proc handler,
	void *data)
{
	short clipped_counts[2];
	real_point3d const *clipped_polygons[2];
	short i;
	long count = 0;
	struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&bsp->nodes, node_index, struct bsp3d_node);
	real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&bsp->planes, node->plane, real_plane3d);
	real_point3d clipped_vertices[2][MAXIMUM_VERTICES_PER_CLIPPED_POLYGON];

	match_assert("c:\\halo\\SOURCE\\physics\\bsp3d.c", 149, vertex_count>=NUMBER_OF_VERTICES_PER_TRIANGLE);
	match_assert("c:\\halo\\SOURCE\\physics\\bsp3d.c", 151, vertex_count<MAXIMUM_VERTICES_PER_CLIPPED_POLYGON);

	for (i = 0; i < vertex_count; i++)
	{
		// plane3d_distance_to_point()
		real distance = plane->n.j*vertices[i].y + plane->n.k*vertices[i].z + plane->n.i*vertices[i].x - plane->d;
		if (!(fabs(distance) < epsilon))
		{
			break;
		}
	}

	if (i == vertex_count)
	{
		real_vector3d edge1, edge2, normal;
		real_vector3d *edges[2];
		boolean facing;

		edges[0] = &edge1;
		edges[1] = &edge2;
		vector_from_points3d(&vertices[0], &vertices[1], edges[0]);
		vector_from_points3d(&vertices[0], &vertices[2], edges[1]);
		cross_product3d(edges[1], edges[0], &normal);
		facing = dot_product3d(&normal, &plane->n) > 0.0f;

		clipped_counts[facing] = vertex_count;
		clipped_polygons[facing] = vertices;
		clipped_counts[!facing] = 0;
		on_node_designator = facing ? (node_index | LONG_MIN) : (node_index & LONG_MAX);
	}
	else
	{
		real_plane3d negated_plane;
		plane3d_negate(plane, &negated_plane);

		clipped_counts[0] = convex_polygon3d_clip_to_plane(vertex_count, vertices, &negated_plane, MAXIMUM_VERTICES_PER_CLIPPED_POLYGON, clipped_vertices[0], NULL, epsilon, FALSE);
		clipped_counts[1] = convex_polygon3d_clip_to_plane(vertex_count, vertices, plane, MAXIMUM_VERTICES_PER_CLIPPED_POLYGON, clipped_vertices[1], NULL, epsilon, FALSE);
		match_assert("c:\\halo\\SOURCE\\physics\\bsp3d.c", 185, clipped_counts[0]!=NONE && clipped_counts[1]!=NONE);
		clipped_polygons[0] = clipped_vertices[0];
		clipped_polygons[1] = clipped_vertices[1];
	}

	for (i = 0; i < 2; i++)
	{
		if (clipped_counts[i])
		{
			if (node->child_indices[i] & LONG_MIN)
			{
				if (node->child_indices[i] != NONE)
				{
					if (handler)
					{
						handler(clipped_polygons[i], clipped_counts[i], node->child_indices[i] & LONG_MAX, on_node_designator, data);
					}
					count++;
				}
			}
			else
			{
				count += bsp3d_clip_polygon_to_leaves_recursive(bsp, node->child_indices[i], on_node_designator, clipped_polygons[i], clipped_counts[i], epsilon, handler, data);
			}
		}
	}

	return count;
}
