/*
STRUCTURE_BSP_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __STRUCTURE_BSP_DEFINITIONS_H
#define __STRUCTURE_BSP_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "render_cameras.h"
#include "leaf_map.h"
#include "rasterizer_geometry.h"

/* ---------- constants */

enum
{
	STRUCTURE_BSP_TAG = 'sbsp',
	STRUCTURE_BSP_VERSION = 5,
};

enum
{
	MAXIMUM_COLLISION_MATERIALS_PER_STRUCTURE = 512,
	MAXIMUM_SURFACE_REFERENCES_PER_STRUCTURE = 0x40000,
	MAXIMUM_LIGHTMAPS_PER_STRUCTURE = 128,
	MAXIMUM_MATERIALS_PER_STRUCTURE_LIGHTMAP = 2048,
	MAXIMUM_SURFACES_PER_STRUCTURE_MATERIAL = 20000,
	MAXIMUM_VERTICES_PER_STRUCTURE_MATERIAL = 64000,
	MAXIMUM_CLUSTER_PORTALS_PER_CLUSTER = 128,
	MAXIMUM_MIRRORS_PER_CLUSTER = 16,
	MAXIMUM_SUBCLUSTERS_PER_CLUSTER = 4096,
	MAXIMUM_SURFACES_PER_SUBCLUSTER = 128,
	MAXIMUM_SURFACES_PER_CLUSTER = SHORT_MAX+1,
	MAXIMUM_VERTICES_PER_MIRROR = 512,
	MAXIMUM_TEMPORARY_CLUSTERS_PER_STRUCTURE = 8192,
	MAXIMUM_CLUSTERS_PER_STRUCTURE = 512,
	MAXIMUM_CLUSTER_DATA_SIZE = UNSIGNED_SHORT_MAX+1,
	MAXIMUM_CLUSTER_PORTALS_PER_STRUCTURE = 512,
	MAXIMUM_VERTICES_PER_CLUSTER_PORTAL = 128,
	MAXIMUM_FOG_PLANES_PER_STRUCTURE = 32,
	MAXIMUM_VERTICES_PER_STRUCTURE_FOG_PLANE = 4096,
	MAXIMUM_FOG_REGIONS_PER_STRUCTURE = 32,
	MAXIMUM_FOG_PALETTE_ENTRIES_PER_STRUCTURE = 32,
	MAXIMUM_WEATHER_PALETTE_ENTRIES_PER_STRUCTURE = 32,
	MAXIMUM_WEATHER_POLYHEDRA_PER_STRUCTURE = 32,
	MAXIMUM_PLANES_PER_WEATHER_POLYHEDRON = 16,
	MAXIMUM_BACKGROUND_SOUND_PALETTE_ENTRIES_PER_STRUCTURE = 64,
	MAXIMUM_SOUND_ENVIRONMENT_PALETTE_ENTRIES_PER_STRUCTURE = 64,
	MAXIMUM_MARKERS_PER_STRUCTURE = 1024,
	MAXIMUM_LENS_FLARES_PER_STRUCTURE = 256,
	MAXIMUM_LENS_FLARE_MARKERS_PER_STRUCTURE = UNSIGNED_SHORT_MAX+1,
	MAXIMUM_DECALS_PER_STRUCTURE = 6144,
};

/* ---------- macros */

#define structure_bsp_definition_get(index) ((struct structure_bsp *)tag_get(STRUCTURE_BSP_TAG, index)) /* fake name */

/* ---------- structures */

struct structure_lens_flare
{
	struct tag_reference lens_flare;
};

struct structure_lens_flare_marker
{
	real_point3d position;
	char i_direction;
	char j_direction;
	char k_direction;
	byte lens_flare_index;
};

struct structure_cluster
{
	short sky_index;
	short fog_designator;
	short background_sound_palette_index;
	short sound_environment_palette_index;
	short weather_palette_index;
	short transitions_to_structure_bsp_index;
	short first_runtime_decal_index;
	unsigned short runtime_decal_count;
	long unused1[6];
	struct tag_block predicted_resources;	// predicted_resource
	struct tag_block subclusters;			// structure_subcluster
	unsigned short first_lens_flare_marker_index;
	unsigned short lens_flare_marker_count;
	struct tag_block surface_indices;		// long
	struct tag_block mirrors;				// structure_mirror
	struct tag_block portal_indices;		// short
};

struct structure_fog_plane
{
	short region_index;
	short runtime_material_type;
	real_plane3d plane;
	struct tag_block vertices;	// real_point3d
};

struct structure_fog_region
{
	long unused[9];
	short fog_palette_index;
	short weather_palette_index;
};

struct structure_fog_palette_entry
{
	char name[TAG_STRING_LENGTH+1];
	struct tag_reference fog;	// fog_definition
	word pad;
	short runtime_global_function_index;
	char global_function_name[TAG_STRING_LENGTH+1];
	long unused[13];
};

struct structure_weather_palette_entry
{
	char name[TAG_STRING_LENGTH+1];
	struct tag_reference particle_system;	// weather_particle_system_definition
	word pad1;
	short runtime_particle_system_global_function_index;
	char particle_system_global_function_name[TAG_STRING_LENGTH+1];
	long particle_system_unused[11];
	struct tag_reference wind;				// wind_definition
	real_vector3d wind_direction;
	real wind_magnitude;
	word pad2;
	short wind_global_function_index;
	char wind_global_function_name[TAG_STRING_LENGTH+1];
	long wind_unused[11];
};

struct structure_background_sound_palette_entry
{
	char name[TAG_STRING_LENGTH+1];
	struct tag_reference background_sound;	// looping_sound_definition
	word pad;
	short runtime_global_function_index;
	char global_function_name[TAG_STRING_LENGTH+1];
	long unused[8];
};

struct structure_sound_environment_palette_entry
{
	char name[TAG_STRING_LENGTH+1];
	struct tag_reference sound_environment;	// sound_environment
	long unused[8];
};

struct structure_leaf
{
	byte_rectangle3d bounds;
	word pad;
	short cluster_index;
	short surface_reference_count;
	long first_surface_reference_index;
};

struct structure_surface
{
	word vertex_indices[3];
};

struct structure_material
{
	struct tag_reference shader;	// _shader
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
	struct tag_reference lightmap_group;	// bitmap_group
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

struct structure_collision_material
{
	struct tag_reference shader;	// _shader
	word pad;
	short runtime_physics_material_type;
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
