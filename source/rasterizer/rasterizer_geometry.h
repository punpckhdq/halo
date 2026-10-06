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
	RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES = 8192,
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
	NUMBER_OF_RASTERIZER_VERTEX_TYPES,
};

enum
{
	_triangle_buffer_type_triangles = 0,
	_triangle_buffer_type_precompiled_strip,
	NUMBER_OF_TRIANGLE_BUFFER_TYPES,
};

/* ---------- macros */

/* ---------- structures */

struct environment_vertex_uncompressed
{
	real_point3d position;
	real_vector3d normal;
	real_vector3d binormal;
	real_vector3d tangent;
	real_point2d texcoord;
};

struct environment_vertex_compressed
{
	real_point3d position;
	unsigned long normal;
	unsigned long binormal;
	unsigned long tangent;
	real_point2d texcoord;
};

struct environment_lightmap_vertex_uncompressed
{
	real_vector3d incident_radiosity;
	real_point2d texcoord;
};

struct environment_lightmap_vertex_compressed
{
	unsigned long incident_radiosity;
	short lightmap_u;
	short lightmap_v;
};

struct model_vertex_uncompressed
{
	real_point3d position;
	real_vector3d normal;
	real_vector3d binormal;
	real_vector3d tangent;
	real_point2d texcoord;
	short nodes[2];
	real weights[2];
};

struct model_vertex_compressed
{
	real_point3d position;
	unsigned long normal;
	unsigned long binormal;
	unsigned long tangent;
	short texcoord_u;
	short texcoord_v;
	byte nodes[2];
	short weights[1];
};

struct triangle_buffer
{
	short type;
	word pad;
	long count;
	long offset;
	void *hardware_format;
};

struct vertex_buffer
{
	short type;
	word pad;
	long count;
	long offset;
	void *base_address;
	void *hardware_format;
};

/* ---------- prototypes/RASTERIZER_GEOMETRY.C */

byte compress_real_to_int8(real z);
byte compress_real_to_int8_clamp(real z);
short compress_real_to_int16(real z);
short compress_real_to_int16_clamp(real z);
unsigned long compress_real_vector3d_to_int32(real_vector3d const *v);
unsigned long compress_real_vector3d_to_int32_clamp(real_vector3d const *v);
real uncompress_int8_to_real(byte i);
real uncompress_int16_to_real(short i);
real_vector3d uncompress_int32_to_real_vector3d(unsigned long i);
long rasterizer_geometry_get_vertex_size(short type);
void rasterizer_geometry_byte_swap_vertices(short type, long count, void *vertices, long buffer_size);
void rasterizer_geometry_compress_vertices(short type, long count, void *compressed, long compressed_size, void const *uncompressed, long uncompressed_size);
void rasterizer_geometry_uncompress_vertices(short type, long count, void *uncompressed, long uncompressed_size, void const *compressed, long compressed_size);
boolean rasterizer_geometry_stripify(struct triangle_buffer *triangle_buffer, struct vertex_buffer *vertex_buffer);
void environment_vertex_compressed_get_point(struct environment_vertex_compressed const *vertex, real_point3d *point);
void environment_vertex_compressed_get_normal(struct environment_vertex_compressed const *vertex, real_vector3d *normal);
void environment_vertex_compressed_get_texcoord(struct environment_vertex_compressed const *vertex, real_point2d *texcoord);
void environment_lightmap_vertex_compressed_get_incident_radiosity(struct environment_lightmap_vertex_compressed const *vertex, real_vector3d *normal);
void environment_lightmap_vertex_compressed_get_texcoord(struct environment_lightmap_vertex_compressed const *vertex, real_point2d *texcoord);

/* ---------- globals */

/* ---------- public code */

#endif // __RASTERIZER_GEOMETRY_H
