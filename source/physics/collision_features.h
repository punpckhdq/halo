/*
COLLISION_FEATURES.H

header included in hcex build.
*/

#ifndef __COLLISION_FEATURES_H
#define __COLLISION_FEATURES_H
#pragma once

/* ---------- headers */

#include "cseries.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	MAXIMUM_COLLISION_FEATURES_PER_TEST = 256,
	MAXIMUM_POINTS_PER_COLLISION_PRISM = 8,
};

enum
{
	_collision_feature_sphere = 0,
	_collision_feature_cylinder,
	_collision_feature_prism,
	NUMBER_OF_COLLISION_FEATURE_TYPES,
};

/* ---------- macros */

/* ---------- structures */

struct collision_feature
{
	long object_index;
	long surface_index;
	byte flags;
	byte breakable_surface_index;
	short material_index;
};

struct collision_sphere
{
	long object_index;
	long surface_index;
	byte flags;
	byte breakable_surface_index;
	short material_index;
	real_point3d center;
	real radius;
};

struct collision_cylinder
{
	long object_index;
	long surface_index;
	byte flags;
	byte breakable_surface_index;
	short material_index;
	real_point3d base;
	real_vector3d height;
	real width;
};

struct collision_prism
{
	long object_index;
	long surface_index;
	byte flags;
	byte breakable_surface_index;
	short material_index;
	real_plane3d plane;
	real height;
	short projection_axis;
	boolean projection_sign;
	byte pad;
	long point_count;
	real_point2d points[MAXIMUM_POINTS_PER_COLLISION_PRISM];
};

struct collision_feature_list
{
	short count[NUMBER_OF_COLLISION_FEATURE_TYPES];
	struct collision_sphere spheres[MAXIMUM_COLLISION_FEATURES_PER_TEST];
	struct collision_cylinder cylinders[MAXIMUM_COLLISION_FEATURES_PER_TEST];
	struct collision_prism prisms[MAXIMUM_COLLISION_FEATURES_PER_TEST];
};

struct collision_plane
{
	real t;
	real_point3d point;
	real_plane3d plane;
	long object_index;
	long surface_index;
	byte flags;
	byte breakable_surface_index;
	short material_index;
};

/* ---------- prototypes/COLLISION_FEATURES.C */

void collision_features_new(
	struct collision_feature_list *features);

void collision_features_from_point(
	real_point3d const *point,
	real height,
	real width,
	long object_index,
	long surface_index,
	byte flags,
	byte breakable_surface_index,
	short material_index,
	struct collision_feature_list *features);

void collision_features_from_line(
	real_point3d const *point,
	real_vector3d const *vector,
	real height,
	real width,
	long object_index,
	long surface_index,
	byte flags,
	byte breakable_surface_index,
	short material_index,
	struct collision_feature_list *features);

void collision_features_from_polygon(
	short point_count,
	real_point3d const *points,
	real_plane3d const *plane,
	real height,
	real width,
	long object_index,
	long surface_index,
	byte flags,
	byte breakable_surface_index,
	short material_index,
	struct collision_feature_list *features);

void collision_features_from_vertex(
	struct collision_bsp const *bsp,
	long vertex_index,
	real_matrix4x3 const *matrix,
	real height,
	real width,
	long object_index,
	struct collision_feature_list *features);

void collision_features_from_edge(
	struct collision_bsp const *bsp,
	long edge_index,
	real_matrix4x3 const *matrix,
	real height,
	real width,
	long object_index,
	struct collision_feature_list *features);

void collision_features_from_surface(
	struct collision_bsp const *bsp,
	long surface_index,
	real_matrix4x3 const *matrix,
	real height,
	real width,
	long object_index,
	struct collision_feature_list *features);

void collision_bsp_get_features_in_sphere(
	struct collision_bsp const *bsp,
	struct collision_bsp_test_sphere_result const *result,
	real_matrix4x3 const *matrix,
	real height,
	real width,
	long object_index,
	struct collision_feature_list *features);

boolean collision_features_test_point(
	struct collision_feature_list const *features,
	real_point3d const *point,
	struct collision_plane *collision);

boolean collision_sphere_test_point(
	struct collision_sphere const *sphere,
	real_point3d const *point,
	real *depth,
	real_plane3d *plane);

boolean collision_cylinder_test_point(
	struct collision_cylinder const *cylinder,
	real_point3d const *point,
	real *depth,
	real_plane3d *plane);

boolean collision_prism_test_point(
	struct collision_prism const *prism,
	real_point3d const *point,
	real *depth,
	real_plane3d *plane);

boolean collision_features_test_vector(
	struct collision_feature_list const *features,
	real_point3d const *point,
	real_vector3d const *vector,
	struct collision_plane *collision);

boolean collision_sphere_test_vector(
	struct collision_sphere const *sphere,
	real_point3d const *point,
	real_vector3d const *vector,
	real *t,
	real_plane3d *plane);

boolean collision_cylinder_test_vector(
	struct collision_cylinder const *cylinder,
	real_point3d const *point,
	real_vector3d const *vector,
	real *t,
	real_plane3d *plane);

boolean collision_prism_test_vector(
	struct collision_prism const *prism,
	real_point3d const *point,
	real_vector3d const *vector,
	real *t,
	real_plane3d *plane);

void render_debug_collision_sphere(
	struct collision_sphere const *sphere,
	real_argb_color const *color);

void render_debug_collision_cylinder(
	struct collision_cylinder const *cylinder,
	real_argb_color const *color);

void render_debug_collision_prism(
	struct collision_prism const *prism,
	real_argb_color const *color);

void render_debug_collision_features(
	struct collision_feature_list const *features);

/* ---------- globals */

/* ---------- public code */

#endif // __COLLISION_FEATURES_H
