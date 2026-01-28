/*
STRUCTURE_BSP_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __STRUCTURE_BSP_DEFINITIONS_H
#define __STRUCTURE_BSP_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "leaf_map.h"

#include "math/integer_math.h"
#include "rasterizer/rasterizer_geometry.h"
#include "render/render.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct structure_leaf
{
	byte_rectangle3d bounds;
	word pad;
	short cluster_index;
	short surface_reference_count;
	long first_surface_reference_index;
};

struct structure_material
{
	struct tag_reference shader;
	short permutation_index;
	word flags;
	long first_surface_index;
	long surface_count;
	real_point3d centroid;
	struct render_lighting lighting;
	real_plane3d plane;
	short breakable_surface_index;
	word copious_unused_space;
	struct vertex_buffer vertices;
	struct vertex_buffer lightmap_vertices;
	struct tag_data uncompressed_vertex_data;
	struct tag_data compressed_vertex_data;
};

struct structure_lightmap
{
	short bitmap_index;
	word pad;
	long unused[4];
	struct tag_block materials;	// structure_material
};

struct structure_bsp
{
	struct tag_reference lightmap_group;
	real vehicle_floor;
	real vehicle_ceiling;
	long sad_unused[5];
	struct render_lighting default_lighting;
	long lonely_unused;
	struct tag_block collision_materials;
	struct tag_block collision_bsp;			// collision_bsp
	struct tag_block nodes;
	real_rectangle3d world_bounds;
	struct tag_block leaves;				// structure_leaf
	struct tag_block surface_references;
	struct tag_block surfaces;
	struct tag_block lightmaps;				// structure_lightmap
	long render_unused[3];
	struct tag_block lens_flares;
	struct tag_block lens_flare_markers;
	struct tag_block clusters;
	struct tag_data cluster_data;
	struct tag_block cluster_portals;
	long cluster_unused[3];
	struct tag_block breakable_surfaces;
	struct tag_block fog_planes;
	struct tag_block fog_regions;
	struct tag_block fog_palette;
	long fog_unused[6];
	struct tag_block weather_palette;
	struct tag_block weather_polyhedra;
	long weather_unused[6];
	struct tag_block pathfinding_surfaces;
	struct tag_block pathfinding_edges;
	struct tag_block background_sound_palette;
	struct tag_block sound_environment_palette;
	struct tag_data sound_cluster_data;
	long sound_unused[6];
	struct tag_block markers;
	struct tag_block detail_object_data;
	struct tag_block runtime_decals;
	long diminishing_misc_unused[2];
	struct leaf_map leaf_map;
};

/* ---------- prototypes/STRUCTURE_BSP_DEFINITIONS.C */

unsigned long *structure_bsp_get_cluster_pvs(struct structure_bsp *structure_bsp, short cluster_index);
void structure_bsp_find_material_for_surface(struct structure_bsp *structure, long surface_index, short *lightmap_index, short *material_index);
void vertex_type_from_shader_tag(unsigned long group_tag, short *vertex_type, short *lightmap_vertex_type, boolean compressed);
byte *structure_bsp_get_cluster_encoded_sound_data(struct structure_bsp *structure_bsp, short row_index, short column_index);
byte structure_bsp_get_cluster_encoded_sound_distance(struct structure_bsp *structure_bsp, short from_cluster_index, short to_cluster_index);

/* ---------- globals */

/* ---------- public code */

#endif // __STRUCTURE_BSP_DEFINITIONS_H
