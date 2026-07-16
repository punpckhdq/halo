/*
BSP2D.H

header included in hcex build.
*/

#ifndef __BSP2D_H
#define __BSP2D_H
#pragma once

/* ---------- headers */

#include "tag_files/tag_groups.h"
#include "real_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct bsp2d_node
{
	struct real_plane2d plane;
	long child_indices[2];
};

struct bsp2d
{
	struct tag_block nodes;
};

/* ---------- prototypes/BSP2D.C */

long bsp2d_test_point(struct bsp2d const *bsp, real_point2d const *point, long child_index);

/* ---------- globals */

/* ---------- public code */

#endif // __BSP2D_H
