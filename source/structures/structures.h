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

/* ---------- prototypes/STRUCTURE_DETAIL_OBJECTS.C */

void structure_detail_objects_flush(void);

/* ---------- prototypes/STRUCTURE_LENS_FLARES.C */

long cluster_index_from_point(struct structure_bsp const *structure_bsp, union real_point3d const *point);

/* ---------- prototypes/STRUCTURE_RUNTIME_DECALS.C */

void structure_decals_reconnect_to_structure_bsp(void);
void structure_decals_disconnect_from_structure_bsp(void);
void structure_decals_update(unsigned long *old_combined_pvs, unsigned long *new_combined_pvs, short cluster_count);

/* ---------- prototypes/STRUCTURES.C */

void structure_cluster_marker_begin(void);
boolean structure_cluster_mark(short cluster_index);
void structure_cluster_marker_end(void);
short structure_clusters_in_sphere(short cluster_index, real_point3d const *position, real radius, short maximum_count, short *intersected_indices);
boolean structure_test_vector(real_point3d const *point, real_vector3d const *vector, real_point3d *collision_point, short *lightmap_index, short *material_index, long *surface_index, real *s, real *t);

/* ---------- prototypes/STRUCTURE_VISIBILITY.C */

short structure_visibility_find_objects(long *result_indices, short maximum_count, long (*cluster_get_first)(long *, short), long (*cluster_get_next)(long *), void (*get_bounding_sphere)(long, real_point3d *, real *), boolean (*unmarked)(long), boolean (*mark)(long));

/* ---------- globals */

/* ---------- public code */

#endif // __STRUCTURES_H
