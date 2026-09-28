/*
STRUCTURES.H

header included in hcex build.
*/

#ifndef __STRUCTURES_H
#define __STRUCTURES_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/STRUCTURES.C */

short structure_clusters_in_sphere(short cluster_index, real_point3d const *position, real radius, short maximum_count, short *intersected_indices);

/* ---------- prototypes/STRUCTURE_DETAIL_OBJECTS.C */

void structure_detail_objects_flush(void);

/* ---------- prototypes/STRUCTURE_LENS_FLARES.C */

long cluster_index_from_point(struct structure_bsp const *structure_bsp, union real_point3d const *point);

/* ---------- prototypes/STRUCTURE_RUNTIME_DECALS.C */

void structure_decals_update(unsigned long *old_combined_pvs, unsigned long *new_combined_pvs, short cluster_count);

/* ---------- globals */

/* ---------- public code */

#endif // __STRUCTURES_H
