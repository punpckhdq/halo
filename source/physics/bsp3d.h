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

#endif // __BSP3D_H
