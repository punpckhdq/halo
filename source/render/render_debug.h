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

void render_debug_point2d(boolean immediate, real_plane3d const *plane, short projection, boolean sign, real_point2d const *point, real size, real_argb_color const *color, real offset);
void render_debug_line2d(boolean immediate, real_plane3d const *plane, short projection, boolean sign, real_point2d const *p0, real_point2d const *p1, real_argb_color const *color, real offset);
void render_debug_vector2d(boolean immediate, real_plane3d const *plane, short projection, boolean sign, real_point2d const *point, real_vector2d const *vector, real scale, real_argb_color const *color, real offset);
void render_debug_circle(boolean immediate, real_plane3d const *plane, short projection, boolean sign, real_point2d const *center, real radius, real_argb_color const *color, real offset);

void render_debug_point(boolean immediate, real_point3d const *point, real size, real_argb_color const *color);
void render_debug_line(boolean immediate, real_point3d const *point0, real_point3d const *point1, real_argb_color const *color);

void render_debug_vector(boolean immediate, real_point3d const *point, real_vector3d const *vector, real size, real_argb_color const *color);
void render_debug_vectors(boolean immediate, real_point3d const *point, real_vector3d const *forward, real_vector3d const *up, real size);
void render_debug_quaternion(boolean immediate, real_point3d const *point, real_quaternion const *quaternion, real size);
void render_debug_tick(boolean immediate, real_point3d const *point, real_vector3d const *tick_vector, real tick_size, real_argb_color const *color);

void render_debug_line_offset(boolean immediate, real_point3d const *p0, real_point3d const *p1, real_argb_color const *color, real offset);

void render_debug_matrix(boolean immediate, real_matrix4x3 const *matrix, real size);

void render_debug_triangle(boolean immediate, real_point3d const *point0, real_point3d const *point1, real_point3d const *point2, real_argb_color const *color);
void render_debug_quadrilateral(boolean immediate, real_point3d const *point0, real_point3d const *point1, real_point3d const *point2, real_point3d const *point3, real_argb_color const *color);

void render_debug_sphere(boolean immediate, real_point3d const *center, real radius, real_argb_color const *color);
void render_debug_cylinder(boolean immediate, real_point3d const *base, real_vector3d const *height, real width, real_argb_color const *color);
void render_debug_pill(boolean immediate, real_point3d const *base, real_vector3d const *height, real width, real_argb_color const *color);

void render_debug_box2d_outline(boolean immediate, real_rectangle2d const *bounds, real_argb_color const *color);
void render_debug_box(boolean immediate, real_rectangle3d const *bounds, real_argb_color const *color);
void render_debug_box_outline(boolean immediate, real_rectangle3d const *bounds, real_argb_color const *color);

void render_debug_polygon(real_point3d const *points, short point_count, real_argb_color const *color);
void render_debug_polygon_edges(real_point3d const *points, short point_count, real_argb_color const *color);

void render_debug_string(boolean immediate, char const *string);
void render_debug_string_at_point(boolean immediate, real_point3d const *point, char const *string, real_argb_color const *color);

void render_debug_collision_surface(struct collision_bsp const *bsp, long surface_index, real_matrix4x3 const *matrix, real_argb_color const *color);

void render_debug(void);

/* ---------- globals */

extern boolean debug_camera;
extern boolean debug_player;
extern boolean debug_structure;
extern boolean debug_bsp;
extern boolean debug_input;
extern boolean debug_permanent_decals;

/* ---------- public code */

#endif // __RENDER_DEBUG_H
