/*
BSP2D.C

*/

/* ---------- headers */
#include "cseries.h"
#include "bsp2d.h"
#include "real_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

int bsp2d_test_point(const struct bsp2d *bsp, const real_point2d *point, int child_index)
{
	while ((child_index & 0x80000000) == 0)
	{
		struct bsp2d_node *node = TAG_BLOCK_GET_ELEMENT(&bsp->nodes, child_index, struct bsp2d_node);

		child_index = node->children[plane2d_distance_to_point(&node->plane, point) >= 0.0f];
	}

	return (child_index != NONE) ? (child_index & LONG_MAX) : NONE;
}

/* ---------- private code */
