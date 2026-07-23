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

boolean convex_hull2d_test_point(short count, union real_point2d const *points, union real_point2d const *point, real epsilon);

/* ---------- globals */

/* ---------- public code */

#endif // __GEOMETRY_H
