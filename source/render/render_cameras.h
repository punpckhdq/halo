/*
RENDER_CAMERAS.H

header included in hcex build.
*/

#ifndef __RENDER_CAMERAS_H
#define __RENDER_CAMERAS_H
#pragma once

/* ---------- headers */


/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct render_screen_flash
{
	short type;
	real intensity;
	real_argb_color color;
};

struct render_screen_effect
{
	short type;
	real intensity;
};

struct render_camera
{
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	boolean mirrored;
	real vertical_field_of_view;
	rectangle2d viewport_bounds;
	rectangle2d window_bounds;
	real z_near;
	real z_far;
	real_plane3d mirror_plane;
};

struct render_window
{
	short local_player_index;
	boolean console_window;
	struct render_camera render_camera;
	struct render_camera rasterizer_camera;
};

struct render_frustum
{
	real_rectangle2d frustum_bounds;
	real_matrix4x3 world_to_view;
	real_matrix4x3 view_to_world;
	real_plane3d world_planes[6];
	real z_near;
	real z_far;
	real_point3d world_vertices[5];
	real_point3d world_midpoint;
	real_rectangle3d world_bounds;
	boolean projection_valid;
	real projection_matrix[4][4];
	real_vector2d projection_world_to_screen;
};

struct render_fog
{
	word fog_definition_flags;
	word runtime_flags;
	real_rgb_color atmospheric_color;
	real atmospheric_maximum_density;
	real atmospheric_minimum_distance;
	real atmospheric_maximum_distance;
	short planar_mode;
	real_plane3d plane;
	real_rgb_color planar_color;
	real planar_maximum_density;
	real planar_maximum_distance;
	real planar_maximum_depth;
	const struct fog_screen *screen;
	real screen_external_intensity;
};

/* ---------- prototypes/RENDER_CAMERAS.C */

boolean render_camera_view_to_screen(struct render_camera const *camera, struct render_frustum const *frustum, real_point3d const *view_point, real_point2d *screen_point);
void render_camera_build_frustum(const struct render_camera *camera, const real_rectangle2d *frustum_bounds, struct render_frustum *frustum, boolean build_projection);
void render_frustum_get_projection_bounds(struct render_frustum const *frustum, real_rectangle2d *projection_bounds);

/* ---------- globals */

/* ---------- public code */

#endif // __RENDER_CAMERAS_H
