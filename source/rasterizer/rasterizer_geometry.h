/*
RASTERIZER_GEOMETRY.H

header included in hcex build.
*/

#ifndef __RASTERIZER_GEOMETRY_H
#define __RASTERIZER_GEOMETRY_H
#pragma once

/* ---------- constants */

enum
{
	RASTERIZER_MEMORY_POOL_SIZE = 0x18000,
	RASTERIZER_MAXIMUM_TRIANGLES_PER_TRIANGLE_BUFFER = 24576,
	RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES = 131072,
	RASTERIZER_MAXIMUM_DEBUG_VERTICES = 393216,
	RASTERIZER_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS = 384,
	RASTERIZER_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS2 = 32,
	RASTERIZER_MAXIMUM_DYNAMIC_TRIANGLES = 32768,
	RASTERIZER_MAXIMUM_DYNAMIC_TRIANGLE_BUFFERS = 1024,
	RASTERIZER_MAXIMUM_DYNAMIC_UNLIT_VERTICES = 8192,
	RASTERIZER_MAXIMUM_DYNAMIC_LIT_VERTICES = 2,
	RASTERIZER_MAXIMUM_DYNAMIC_SCREEN_VERTICES = 16384,
	RASTERIZER_MAXIMUM_DYNAMIC_MODEL_VERTICES = 2048,
	RASTERIZER_MAXIMUM_DYNAMIC_MODEL_PROCESSED_VERTICES = 8192,
	RASTERIZER_MAXIMUM_DYNAMIC_VERTEX_BUFFERS = 1024,
	RASTERIZER_MAXIMUM_DETAIL_OBJECTS_PER_FRAME = 4096,
	RASTERIZER_NODES_PER_MODEL_VERTEX = 2,
	RASTERIZER_MAXIMUM_NODES_PER_MODEL = 44,
	RASTERIZER_MAXIMUM_NEARBY_OPAQUE_MODEL_GEOMETRY_GROUPS_THAT_MIGHT_OBSCURE_THE_ENVIRONMENT_FOG_SCREEN_EFFECT = 1
};

enum
{
	_rasterizer_vertex_type_environment_uncompressed = 0,
	_rasterizer_vertex_type_environment_compressed,
	_rasterizer_vertex_type_environment_lightmap_uncompressed,
	_rasterizer_vertex_type_environment_lightmap_compressed,
	_rasterizer_vertex_type_model_uncompressed,
	_rasterizer_vertex_type_model_compressed,
	_rasterizer_vertex_type_dynamic_unlit,
	_rasterizer_vertex_type_dynamic_lit,
	_rasterizer_vertex_type_dynamic_screen,
	_rasterizer_vertex_type_debug,
	_rasterizer_vertex_type_decal,
	_rasterizer_vertex_type_detail_object,
	_rasterizer_vertex_type_environment_uncompressed_ff,
	_rasterizer_vertex_type_environment_lightmap_uncompressed_ff,
	_rasterizer_vertex_type_model_uncompressed_ff,
	_rasterizer_vertex_type_model_processed,
	_rasterizer_vertex_type_unlit_zsprite,
	_rasterizer_vertex_type_widget,
	NUMBER_OF_RASTERIZER_VERTEX_TYPES,
};

/* ---------- macros */

/* ---------- structures */

struct vertex_buffer
{
	short type;
	word pad;
	long count;
	long offset;
	void *base_address;
	void *hardware_format;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __RASTERIZER_GEOMETRY_H
