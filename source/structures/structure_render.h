/*
STRUCTURE_RENDER.H
*/

#ifndef __STRUCTURE_RENDER_H
#define __STRUCTURE_RENDER_H
#pragma once

/* ---------- headers */


/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/STRUCTURE_RENDER.C */

void structure_render_preprocess(void);
void structure_render_lightmaps(void);
void structure_render_diffuse_texture(void);
void structure_render_specular_lightmaps(void);
void structure_render_reflection_lightmap_masks(void);
void structure_render_reflection_mirrors(void);
void structure_render_reflections(void);
void structure_render_transparent_geometry(void);
void structure_render_fog(void);
void structure_render_fog_screen(void);
void structure_render_specular_light(long rasterizer_light_index, union real_point3d const *bounding_sphere_center, real bounding_sphere_radius, short cluster_count, short const *cluster_indices);
void structure_render_diffuse_light(long rasterizer_light_index, union real_point3d const *bounding_sphere_center, real bounding_sphere_radius, short cluster_count, short const *cluster_indices);
void structure_render_shadow(real_point3d const *bounding_sphere_center, real bounding_radius, real_rectangle3d const *bounding_box, short bounding_polyhedra_count, real_plane3d const *bounding_polyhedra);

/* ---------- globals */

extern long debug_leaf_index;
extern long debug_leaf_portal_index;

/* ---------- public code */

#endif // __STRUCTURE_RENDER_H
