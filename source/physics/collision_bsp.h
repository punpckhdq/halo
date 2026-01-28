/*
COLLISION_BSP.H

header included in hcex build.
*/

#ifndef __COLLISION_BSP_H
#define __COLLISION_BSP_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

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
