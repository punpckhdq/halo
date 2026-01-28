/*
RENDER_DEBUG.H

header included in hcex build.
*/

#ifndef __RENDER_DEBUG_H
#define __RENDER_DEBUG_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/RENDER_DEBUG.C */

void render_debug_string_at_point(boolean immediate, real_point3d *point, const char *string, real_argb_color const *color);

void render_debug_line(boolean immediate, real_point3d const *point0, real_point3d const *point1, real_argb_color const *color);

void render_debug_vector(boolean immediate, real_point3d const *point, real_vector3d const *vector, real size, real_argb_color const *color);

void render_debug_matrix(boolean immediate, const struct real_matrix4x3 *matrix, real size);

void render_debug_sphere(boolean, real_point3d const *center, real radius, real_argb_color const *color);

/* ---------- globals */

/* ---------- public code */

#endif // __RENDER_DEBUG_H
