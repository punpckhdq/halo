/*
COLLISION_BSP.H

header included in hcex build.
*/

#ifndef __COLLISION_BSP_H
#define __COLLISION_BSP_H
#pragma once

/* ---------- headers */

#include "collision_bsp_definitions.h"

/* ---------- constants */

enum
{
	MAXIMUM_COLLISION_LEAVES_PER_TEST = 256,
	MAXIMUM_COLLISION_FEATURES_PER_TEST = 256
};

enum
{
	_collision_bsp_test_front_facing_surfaces_bit = 0,
	_collision_bsp_test_back_facing_surfaces_bit,
	_collision_bsp_test_ignore_two_sided_surfaces_bit,
	_collision_bsp_test_ignore_invisible_surfaces_bit,
	_collision_bsp_test_ignore_breakable_surfaces_bit,
	NUMBER_OF_COLLISION_BSP_TEST_FLAGS
};

/* ---------- macros */

/* ---------- structures */

struct collision_bsp_test_vector_result
{
	real t;
	const real_plane3d *plane;
	long surface_index;
	long plane_designator;
	byte flags;
	byte breakable_surface_index;
	short material_index;
	long leaf_count;
	long leaf_indices[MAXIMUM_COLLISION_LEAVES_PER_TEST];
};

struct collision_bsp_test_pill_result
{
	real t;
	real_plane3d plane;
	long surface_index;
	byte flags;
	byte breakable_surface_index;
	short material_index;
	long leaf_count;
	long leaf_indices[MAXIMUM_COLLISION_LEAVES_PER_TEST];
};

struct collision_surface_test_line2d_result
{
	real enter_t;
	long enter_edge_index;
	long enter_surface_index;
	real exit_t;
	long exit_edge_index;
	long exit_surface_index;
};

struct collision_bsp_test_sphere_result
{
	long surface_count;
	long surface_indices[MAXIMUM_COLLISION_FEATURES_PER_TEST];
	long edge_count;
	long edge_indices[MAXIMUM_COLLISION_FEATURES_PER_TEST];
	long vertex_count;
	long vertex_indices[MAXIMUM_COLLISION_FEATURES_PER_TEST];
	long leaf_count;
	long leaf_indices[MAXIMUM_COLLISION_LEAVES_PER_TEST];
};

/* ---------- prototypes/COLLISION_BSP.C */

short collision_surface_edge_count(
	struct collision_bsp const *bsp,
	long surface_index);

short collision_surface_polygon(
	struct collision_bsp const *bsp,
	long surface_index,
	real_point3d *points);

void render_debug_collision_vertex(
	struct collision_bsp const *bsp,
	long vertex_index,
	real_matrix4x3 const *matrix,
	real size,
	real_argb_color const *color);

void render_debug_collision_edge(
	struct collision_bsp const *bsp,
	long edge_index,
	real_matrix4x3 const *matrix,
	real_argb_color const *color);

void render_debug_collision_surface(
	struct collision_bsp const *bsp,
	long surface_index,
	real_matrix4x3 const *matrix,
	real_argb_color const *color);

void render_debug_collision_bsp(
	struct collision_bsp *bsp,
	real_matrix4x3 const *matrix);

real collision_edge_length(
	struct collision_bsp const *bsp,
	long edge_index);

real collision_surface_perimeter(
	struct collision_bsp const *bsp,
	long surface_index);

real collision_surface_area(
	struct collision_bsp const *bsp,
	long surface_index);

real_point3d *collision_surface_project_point2d(
	struct collision_bsp const *bsp,
	long surface_index,
	short projection_axis,
	boolean projection_sign,
	real_point2d const *p2d,
	real_point3d *p3d);

boolean collision_surface_test_point2d(
	struct collision_bsp const *bsp,
	long surface_index,
	short projection_axis,
	boolean projection_sign,
	real_point2d const *point);

boolean collision_surface_find_closest_point2d(
	struct collision_bsp const *bsp,
	long surface_index,
	short projection_axis,
	boolean projection_sign,
	real_point2d const *point,
	real_point2d *result);

boolean collision_surface_test_line2d(
	struct collision_bsp const *bsp,
	long surface_index,
	short projection_axis,
	boolean projection_sign,
	real_point2d const *point,
	real_vector2d const *vector,
	struct collision_surface_test_line2d_result *result);

boolean collision_bsp_test_pill_new(
	struct collision_bsp const *bsp,
	short breakable_surface_count,
	byte const *breakable_surface_flags,
	real_point3d const *point,
	real_vector3d const *vector,
	real radius,
	real *t,
	real_vector3d *normal);

boolean collision_bsp_test_sphere(
	struct collision_bsp const *bsp,
	short breakable_surface_count,
	byte const *breakable_surface_flags,
	real_point3d const *center,
	real radius,
	struct collision_bsp_test_sphere_result *result);

boolean collision_bsp_test_vector(
	unsigned long flags,
	struct collision_bsp const *bsp,
	short breakable_surface_count,
	byte const *breakable_surface_flags,
	real_point3d const *point,
	real_vector3d const *vector,
	real maximum_t,
	struct collision_bsp_test_vector_result *result);

boolean collision_bsp_test_pill(
	struct collision_bsp const *bsp,
	real_point3d const *point,
	real_vector3d const *vector,
	real radius,
	real maximum_t,
	struct collision_bsp_test_pill_result *result);

/* ---------- globals */

/* ---------- public code */

#endif // __COLLISION_BSP_H
