/*
BSP3D.H

header included in hcex build.
*/

#ifndef __BSP3D_H
#define __BSP3D_H
#pragma once


/* ---------- headers */

#include "tag_files/tag_groups.h"
#include "real_math.h"

/* ---------- constants */

enum
{
	MAXIMUM_NODES_PER_BSP3D = 131072,
	MAXIMUM_LEAVES_PER_BSP3D = UNSIGNED_SHORT_MAX+1,
	MAXIMUM_PLANES_PER_BSP3D = UNSIGNED_SHORT_MAX+1,
	MAXIMUM_BSP3D_DEPTH = 128,
	BSP3D_ROOT_NODE_INDEX = 0,
};

/* ---------- structures */

struct bsp3d
{
	struct tag_block nodes;
	struct tag_block planes;
};

struct bsp3d_node
{
	long plane;
	long child_indices[2];
};

typedef void (*line_proc)(
	real_point3d const *p0,
	real_point3d const *p1,
	long leaf_index,
	void *data);

typedef void (*polygon_proc)(
	real_point3d const *vertices,
	short vertex_count,
	long leaf_index,
	long on_node_designator,
	void *data);

/* ---------- prototypes/BSP3D.C */

long bsp3d_test_point(struct bsp3d const *bsp, long node_index, real_point3d const *point);

long bsp3d_clip_line_to_leaves(
	struct bsp3d const *bsp,
	long node_index,
	real_point3d const *p0,
	real_point3d const *p1,
	line_proc handler,
	void *user_data);

long bsp3d_clip_polygon_to_leaves(
	struct bsp3d const *bsp,
	long node_index,
	real_point3d const *vertices,
	short vertex_count,
	real epsilon,
	polygon_proc handler,
	void *user_data);

/* ---------- public code */

__inline real_plane3d *bsp3d_get_plane_from_designator(
	struct bsp3d const *bsp,
	long plane_designator,
	real_plane3d *result)
{
	real_plane3d* plane = TAG_BLOCK_GET_ELEMENT(&bsp->planes, plane_designator & LONG_MAX, real_plane3d);

	if (plane_designator & LONG_MIN)
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
