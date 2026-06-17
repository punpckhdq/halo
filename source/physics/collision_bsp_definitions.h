/*
COLLISION_BSP_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __COLLISION_BSP_DEFINITIONS_H
#define __COLLISION_BSP_DEFINITIONS_H
#pragma once


/* ---------- headers */

#include "bsp2d.h"
#include "bsp3d.h"

/* ---------- constants */

enum
{
	MAXIMUM_BSP2D_REFERENCES_PER_COLLISION_BSP = 131072,
	MAXIMUM_SURFACES_PER_COLLISION_BSP = 131072,
	MAXIMUM_EDGES_PER_COLLISION_BSP = 262144,
	MAXIMUM_VERTICES_PER_COLLISION_BSP = 131072,
	MAXIMUM_VERTICES_PER_COLLISION_SURFACE = 8,
	MAXIMUM_EDGES_PER_COLLISION_SURFACE = 8,
};

/* ---------- macros */

/* ---------- structures */

struct collision_bsp
{
	struct bsp3d bsp3d;
	struct tag_block leaves;
	struct tag_block bsp2d_references;
	struct bsp2d bsp2d;
	struct tag_block surfaces;
	struct tag_block edges;
	struct tag_block vertices;
};

struct collision_node
{
	char name[32];
	short region_index;
	short parent_node_index;
	short next_sibling_node_index;
	short first_child_node_index;
	long unused[2];
	word pad;
	short runtime_damage_part;
	struct tag_block bsps;
};

struct collision_surface
{
	long plane_designator;
	long first_edge_index;
	unsigned char flags;
	unsigned char breakable_surface_index;
	short material_index;
};

struct collision_edge
{
	long vertex_indices[2];
	long edge_indices[2];
	long surface_indices[2];
};

struct collision_vertex
{
	real_point3d point; // 0x0
	long first_edge_index; // 0xC
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __COLLISION_BSP_DEFINITIONS_H
