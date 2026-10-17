/*
RENDER_CAMERAS.H

header included in hcex build.
*/

#ifndef __RENDER_CAMERAS_H
#define __RENDER_CAMERAS_H
#pragma once

/* ---------- headers */

/* ---------- constants */

enum
{
	MAXIMUM_RENDERED_DISTANT_LIGHTS = 2,
	MAXIMUM_RENDERED_POINT_LIGHTS = 2,
	MAXIMUM_RENDERED_ENVIRONMENT_SURFACES = 16384,
	MAXIMUM_RENDERED_CLUSTERS = 128,
	MAXIMUM_SURFACES_PER_STRUCTURE = 0x20000,
	MAXIMUM_RENDERED_LIGHTS = 128,
	MAXIMUM_LIGHTS_PER_MAP = 896,
	MAXIMUM_LENS_FLARES_PER_LIGHT = 8,
	MAXIMUM_QUEUED_LENS_FLARES = 8,
};

/* ---------- macros */

/* ---------- structures */

struct render_distant_light
{
	real_rgb_color color;
	real_vector3d direction;
};

struct render_lighting
{
	real_rgb_color ambient_color;
	short distant_light_count;
	word pad;
	struct render_distant_light distant_lights[MAXIMUM_RENDERED_DISTANT_LIGHTS];
	short point_light_count;
	word pad1;
	long point_light_indices[MAXIMUM_RENDERED_POINT_LIGHTS];
	real_argb_color reflection_tint_color;
	real_vector3d shadow_vector;
	real_rgb_color shadow_color;
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

struct render_mirror
{
	real_plane3d plane;
	real index_of_refraction;
	real depth;
	short cluster_index;
};

/* ---------- prototypes/RENDER_CAMERAS.C */

void render_camera_new(struct render_camera *camera);
void render_camera_mirror(struct render_camera const *camera, struct render_mirror const *mirror, struct render_camera *result);
void render_camera_hack_frustum_z(struct render_frustum *frustum, real z_near, real z_far);
void render_camera_build_frustum_bounds(struct render_camera const *camera, real_rectangle2d *frustum_bounds);
boolean render_camera_build_clipped_frustum_bounds(struct render_camera const *camera, real_rectangle2d const *clip, real_rectangle2d *frustum_bounds);
void render_camera_build_frustum(struct render_camera const *camera, real_rectangle2d const *frustum_bounds, struct render_frustum *frustum, boolean build_projection);
boolean render_camera_triangle_frontfacing(struct render_camera const *camera, real_point3d const *p0, real_point3d const *p1, real_point3d const *p2);
short render_frustum_build_point_flags(struct render_frustum const *frustum, real_point3d const *point);
boolean render_frustum_triangle_visible(struct render_frustum const *frustum, real_point3d const *p0, real_point3d const *p1, real_point3d const *p2);
short render_frustum_cube_visible(struct render_frustum const *frustum, real_rectangle3d const *bounds, boolean test_frustum_against_cube);
short render_frustum_sphere_visible(struct render_frustum const *frustum, real_point3d const *point, real radius);
real render_frustum_sphere_diameter_in_pixels(struct render_frustum const *frustum, real_point3d const *point, real radius);
real render_frustum_cube_view_fraction(struct render_frustum const *frustum, real_rectangle3d const *bounds);
void render_frustum_get_projection_bounds(struct render_frustum const *frustum, real_rectangle2d *bounds);
boolean render_camera_world_to_screen(struct render_camera const *camera, struct render_frustum const *frustum, real_point3d const *world_point, real_point2d *screen_point);
boolean render_camera_view_to_screen(struct render_camera const *camera, struct render_frustum const *frustum, real_point3d const *view_point, real_point2d *screen_point);
void render_camera_screen_to_view(struct render_camera const *camera, struct render_frustum const *frustum, real_point2d const *screen_point, real_vector3d *view_vector);
void render_camera_screen_to_world(struct render_camera const *camera, struct render_frustum const *frustum, real_point2d const *screen_point, real_point3d *world_point, real_vector3d *world_vector);
real render_camera_get_adjusted_field_of_view_tangent(real field_of_view);
void render_camera_debug_frustum(struct render_camera const *camera, struct render_frustum const *frustum);

/* ---------- globals */

extern boolean debug_no_frustum_clip;
extern boolean render_camera_debug_this_fucking_frustum;

/* ---------- public code */

#endif // __RENDER_CAMERAS_H
