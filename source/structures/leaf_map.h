/*
LEAF_MAP.H

header included in hcex build.
*/

#ifndef __LEAF_MAP_H
#define __LEAF_MAP_H
#pragma once

/* ---------- headers */

#include "tag_files/tag_groups.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct leaf_map
{
	const struct bsp3d *bsp;
	struct tag_block leaves;
	struct tag_block portals;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __LEAF_MAP_H
