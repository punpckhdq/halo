/*
RENDER_CAMERAS.H

header included in hcex build.
*/

#ifndef __RENDER_CAMERAS_H
#define __RENDER_CAMERAS_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"
#include "math/integer_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

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

/* ---------- prototypes/RENDER_CAMERAS.C */

/* ---------- globals */

/* ---------- public code */

#endif // __RENDER_CAMERAS_H
