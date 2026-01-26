/*
COLLISION_BSP_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __COLLISION_BSP_DEFINITIONS_H
#define __COLLISION_BSP_DEFINITIONS_H
#pragma once


/* ---------- headers */

#include "bsp2d.h"
#include "bsp3d.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct collision_bsp
{
	struct bsp3d bsp3d;
	struct tag_block leaves;
	struct tag_block bsp2d_references;
	struct bsp2d bsp2d;
	struct tag_block surfaces;
	struct tag_block edges;
	struct tag_block vertices;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __COLLISION_BSP_DEFINITIONS_H
