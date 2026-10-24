/*
GEOMETRY.H

header included in hcex build.
*/

#ifndef __GEOMETRY_H
#define __GEOMETRY_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct vertex3d
{
	boolean extant;
	short point_index;
	long edge_index;
	long user_data;
};

struct geosphere
{
	short segment_count;
	real_point3d *vertices;
	short *triangle_strip_vertex_indices;
	short vertex_count;
	short triangle_count;
	short triangle_strip_count;
};

struct edge3d
{
	boolean extant;
	long vertex_indices[2];
	long edge_indices[2];
	long surface_indices[2];
	long user_data;
};

struct surface3d
{
	boolean extant;
	real_plane3d plane;
	long edge_index;
	long user_data;
};

/* ---------- prototypes/GEOMETRY.C */

struct geosphere *geosphere_new(short segment_count);
void geosphere_dispose(struct geosphere *sphere);
void build_torus(real_matrix4x3 const *matrix, short *vertex_count_reference, short *triangle_strip_count_reference, real_point3d *points, real_point2d *texture_uvs, short *triangle_strip_vertex_indices, short ring_segment_count, real ring_radius, short cylinder_segment_count, real cylinder_radius);

short convex_hull2d(short vertex_count, real_point2d const *points, short *hull_vertex_indices);
boolean convex_hull2d_verify(short point_count, real_point2d const *points, short hull_count, short const *hull_indices);
boolean convex_hull2d_test_circle(short count, real_point2d const *points, real_point2d const *p, real radius);
boolean convex_hull2d_test_point(short count, real_point2d const *points, real_point2d const *point, real epsilon);
boolean convex_hull2d_test_point_indexed(short count, short const *indices, real_point2d const *points, real_point2d const *point, real epsilon);
real convex_hull2d_area(short count, real_point2d const *points);
real convex_hull2d_perimeter(short count, real_point2d const *points);
short convex_hull2d_intersect(short p_count, real_point2d const *p, short q_count, real_point2d const *q, short maximum_count, real_point2d *result, real epsilon);
boolean convex_hull2d_test_vector(short count, real_point2d const *points, real_point2d const *point, real_vector2d const *vector, real *t_in_reference, real *t_out_reference);

short convex_polygon2d_clip_to_plane(short count, real_point2d const *points, real_plane2d const *plane, short maximum_count, real_point2d *result, unsigned long *clip_flags, boolean *clipped, real epsilon);
boolean convex_polygon2d_verify(short count, real_point2d const *points);
short convex_polygon3d_clip_to_plane(short count, real_point3d const *points, real_plane3d const *plane, short maximum_count, real_point3d *result, boolean *clipped, real epsilon, boolean keep_coplanar);
boolean convex_polygon3d_verify(short count, real_point3d const *points);

boolean convex_hull3d_begin(short point_count, real_point3d const *points, short vertex_count, struct vertex3d *vertices, short edge_count, struct edge3d *edges, short surface_count, struct surface3d *surfaces);
boolean convex_hull3d_expand(short point_count, real_point3d const *points, short vertex_count, struct vertex3d *vertices, short edge_count, struct edge3d *edges, short surface_count, struct surface3d *surfaces, short point_index);
boolean convex_hull3d(short point_count, real_point3d const *points, short vertex_count, struct vertex3d *vertices, short edge_count, struct edge3d *edges, short surface_count, struct surface3d *surfaces);
boolean convex_hull3d_verify(short point_count, real_point3d const *points, short vertex_count, struct vertex3d const *vertices, short edge_count, struct edge3d const *edges, short surface_count, struct surface3d const *surfaces);
boolean convex_hull3d_test_point(short point_count, real_point3d const *points, short vertex_count, struct vertex3d const *vertices, short edge_count, struct edge3d const *edges, short surface_count, struct surface3d const *surfaces, real_point3d const *point);
boolean convex_hull3d_test_vector(short point_count, real_point3d const *points, short vertex_count, struct vertex3d const *vertices, short edge_count, struct edge3d const *edges, short surface_count, struct surface3d const *surfaces, real_point3d const *point, real_vector3d const *vector, real *t_in_reference, real *t_out_reference);

/* ---------- globals */

extern real global_convex_hull3d_delta;
extern real global_convex_hull3d_epsilon;

/* ---------- public code */

#endif // __GEOMETRY_H
