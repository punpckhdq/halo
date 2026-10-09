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

void structure_render_diffuse_light(long rasterizer_light_index, union real_point3d const *bounding_sphere_center, real bounding_sphere_radius, short cluster_count, short const *cluster_indices);
void structure_render_specular_light(long rasterizer_light_index, union real_point3d const *bounding_sphere_center, real bounding_sphere_radius, short cluster_count, short const *cluster_indices);

/* ---------- globals */

/* ---------- public code */

#endif // __STRUCTURE_RENDER_H
