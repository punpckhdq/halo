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
	long children[2];
};

struct bsp2d
{
	struct tag_block nodes;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

int bsp2d_test_point(const struct bsp2d *bsp, const union real_point2d *point, int child_index);

#endif // __BSP2D_H
