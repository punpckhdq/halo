/*
MODELS.H

header included in hcex build.
*/

#ifndef __MODELS_H
#define __MODELS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/MODELS.C */

void model_get_node_orientations(struct model const *model, real_orientation *node_orientations);

short model_get_marker_by_name(
	long model_index,
	char const *name,
	unsigned char const *region_permutations,
	short const *node_remapping_table,
	short node_count, 
	real_matrix4x3 const *node_matrices,
	boolean mirrored_flag,
	struct object_marker *markers,
	short);

/* ---------- globals */

/* ---------- public code */

#endif // __MODELS_H
