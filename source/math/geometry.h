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

/* ---------- prototypes/GEOMETRY.C */

short convex_polygon3d_clip_to_plane(
	short count,
	real_point3d const *points,
	real_plane3d const *plane,
	short maximum_count,
	real_point3d *result,
	byte *clipped,
	real epsilon,
	boolean keep_coplanar);

short convex_polygon2d_clip_to_plane(short point_count, real_point2d const *points, real_plane2d const *plane, short maximum_point_count, real_point2d *result, unsigned long *clip_flags, boolean *clipped, real epsilon);

short convex_hull2d(short vertex_count, real_point2d const *points, short *hull_vertex_indices);
short convex_hull2d_intersect(short p_count, real_point2d const *p, short q_count, real_point2d const *q, short maximum_count, real_point2d *result, real epsilon);
real convex_hull2d_area(short count, real_point2d const *points);
real convex_hull2d_perimeter(short count, real_point2d const *points);
boolean convex_hull2d_test_circle(short count, union real_point2d const *points, union real_point2d const *p, real radius);
boolean convex_hull2d_test_point(short count, union real_point2d const *points, union real_point2d const *point, real epsilon);
boolean convex_hull2d_test_point_indexed(short count, short const *indices, real_point2d const *points, real_point2d const *point, real epsilon);

/* ---------- globals */

/* ---------- public code */

#endif // __GEOMETRY_H
