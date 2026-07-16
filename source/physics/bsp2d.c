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

long bsp2d_test_point(
	struct bsp2d const *bsp,
	real_point2d const *point,
	long child_index)
{
	while (!(child_index & LONG_MIN))
	{
		struct bsp2d_node *node = TAG_BLOCK_GET_ELEMENT(&bsp->nodes, child_index, struct bsp2d_node);
		real d = plane2d_distance_to_point(&node->plane, point);

		child_index = node->child_indices[d >= 0.0f];
	}

	return (child_index != NONE) ? (child_index & LONG_MAX) : NONE;
}

/* ---------- private code */
