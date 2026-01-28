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
	_rasterizer_vertex_type_environment_uncompressed= 0,
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
