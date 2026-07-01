/*
BSP3D.H

header included in hcex build.
*/

#ifndef __BSP3D_H
#define __BSP3D_H
#pragma once


/* ---------- headers */

#include "tag_files/tag_groups.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct bsp3d
{
	struct tag_block nodes;
	struct tag_block planes;
};

/* ---------- prototypes/BSP3D.C */

long bsp3d_test_point(struct bsp3d const *bsp, long node_index, union real_point3d const *point);

/* ---------- globals */

/* ---------- public code */

__inline real_plane3d *bsp3d_get_plane_from_designator(
	struct bsp3d const *bsp,
	long plane_designator,
	real_plane3d *result)
{
	real_plane3d* plane = TAG_BLOCK_GET_ELEMENT(&bsp->planes, plane_designator & LONG_MAX, real_plane3d);

	if (plane_designator<0)
	{
		plane3d_negate(plane, result);
	}
	else
	{
		*result = *plane;
	}

	return result;
}

#endif // __BSP3D_H
