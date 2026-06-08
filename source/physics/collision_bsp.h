/*
COLLISION_BSP.H

header included in hcex build.
*/

#ifndef __COLLISION_BSP_H
#define __COLLISION_BSP_H
#pragma once

#include "bsp2d.h"
#include "bsp3d.h"

/* ---------- constants */

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
	long leaf_indices[256];
};

struct collision_model_test_vector_result
{
	short node_index;
	short region_index;
	short bsp_index;
	struct collision_bsp_test_vector_result bsp_result;
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
	long leaf_indices[256];
};

struct collision_model_test_pill_result
{
	short node_index;
	short region_index;
	short bsp_index;
	struct collision_bsp_test_pill_result bsp_result;
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
	long surface_indices[256];
	long edge_count;
	long edge_indices[256];
	long vertex_count;
	long vertex_indices[256];
	long leaf_count;
	long leaf_indices[256];
};

/* ---------- prototypes/COLLISION_BSP.C */

boolean collision_bsp_test_sphere(
	struct collision_bsp const *bsp,
	short breakable_surface_count,
	byte const *breakable_surface_flags,
	union real_point3d const *center,
	real radius,
	struct collision_bsp_test_sphere_result *result);

/* ---------- globals */

/* ---------- public code */

#endif // __COLLISION_BSP_H
