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

void structure_detail_objects_initialize(void);
void structure_detail_objects_initialize_for_new_map(void);
void structure_detail_objects_dispose_from_old_map(void);
void structure_detail_objects_dispose(void);
void structure_detail_objects_flush(void);
void structure_render_detail_objects(void);
void detail_object_offset(real offset);
void render_debug_detail_objects(void);

/* ---------- prototypes/STRUCTURE_LENS_FLARES.C */

long cluster_index_from_point(struct structure_bsp const *structure_bsp, union real_point3d const *point);

/* ---------- prototypes/STRUCTURE_RENDER.C */

void structure_render_set_fog_offset(real_vector3d const *vector_offset);

/* ---------- prototypes/STRUCTURE_RUNTIME_DECALS.C */

void structure_decals_initialize(void);
void structure_decals_initialize_for_new_map(void);
void structure_decals_reconnect_to_structure_bsp(void);
void structure_decals_disconnect_from_structure_bsp(void);
void structure_decals_dispose_from_old_map(void);
void structure_decals_dispose(void);
void structure_decals_update(unsigned long *old_combined_pvs, unsigned long *new_combined_pvs, short cluster_count);

/* ---------- prototypes/STRUCTURES.C */

void structures_initialize(void);
void structures_initialize_for_new_map(void);
void structures_dispose_from_old_map(void);
void structures_dispose(void);
boolean sphere_intersects_cluster_portal(struct structure_bsp *structure, short portal_index, real_point3d const *point, real radius);
short structure_clusters_in_sphere(short cluster_index, real_point3d const *position, real radius, short maximum_count, short *intersected_indices);
short structure_clusters_in_cone(short position_cluster_index, real_point3d const *position, real_vector3d const *direction, real distance, real sine, real cosine, short maximum_cluster_count, short *cluster_indices);
void structure_cluster_marker_begin(void);
boolean structure_cluster_unmarked(short cluster_index);
boolean structure_cluster_mark(short cluster_index);
void structure_cluster_marker_end(void);
boolean structure_render_surface_from_point_and_leaf(real_point3d const *point, long leaf_index, long plane_index, short *lightmap_index, short *material_index, long *surface_index, real *s, real *t);
boolean structure_test_vector(real_point3d const *p, real_vector3d const *v, real_point3d *collision_point, short *lightmap_index, short *material_index, long *surface_index, real *s, real *t);
long structure_get_planar_fog_definition_index(struct structure_bsp *structure_bsp, short cluster_index, boolean sky);
void structure_get_planar_fog(short cluster_index, struct render_fog *fog);
void render_debug_fog_planes(void);

/* ---------- prototypes/STRUCTURE_VISIBILITY.C */

void debug_pvs(boolean enable);
void structure_visibility_find_camera(struct render_camera const *camera);
boolean structure_visibility_find_mirror(struct render_camera const *camera, struct render_frustum const *frustum, struct render_mirror *result);
void structure_visibility_compute(void);
short structure_visibility_build_surfaces(long *surface_indices, short maximum_count, real_point3d const *bounding_sphere_center, real bounding_sphere_radius, real_rectangle3d const *bounding_box, short bounding_surface_count, real_plane3d const *bounding_surfaces, short cluster_count, short const *cluster_indices);
short structure_visibility_find_objects(long *result_indices, short maximum_count, long (*cluster_get_first)(long *, short), long (*cluster_get_next)(long *), void (*get_bounding_sphere)(long, real_point3d *, real *), boolean (*unmarked)(long), boolean (*mark)(long));

/* ---------- globals */

extern boolean debug_detail_objects;
extern boolean debug_fog_planes;
extern boolean debug_leaf_portals;
extern boolean debug_portals;
extern boolean structures_use_pvs_for_vs;
extern real structure_sphere_threshold;

/* ---------- public code */

#endif // __STRUCTURES_H
