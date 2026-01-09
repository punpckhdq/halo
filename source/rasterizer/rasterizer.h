/*
RASTERIZER.H

header included in hcex build.
*/

#ifndef __RASTERIZER_H
#define __RASTERIZER_H
#pragma once

/* ---------- constants */

enum
{
	RASTERIZER_MEMORY_POOL_SIZE= 0x18000,
	RASTERIZER_MAXIMUM_TRIANGLES_PER_TRIANGLE_BUFFER= 24576,
	RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES= 131072,
	RASTERIZER_MAXIMUM_DEBUG_VERTICES= 393216,
	RASTERIZER_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS= 384,
	RASTERIZER_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS2= 32,
	RASTERIZER_MAXIMUM_DYNAMIC_TRIANGLES= 32768,
	RASTERIZER_MAXIMUM_DYNAMIC_TRIANGLE_BUFFERS= 1024,
	RASTERIZER_MAXIMUM_DYNAMIC_UNLIT_VERTICES= 8192,
	RASTERIZER_MAXIMUM_DYNAMIC_LIT_VERTICES= 2,
	RASTERIZER_MAXIMUM_DYNAMIC_SCREEN_VERTICES= 16384,
	RASTERIZER_MAXIMUM_DYNAMIC_MODEL_VERTICES= 2048,
	RASTERIZER_MAXIMUM_DYNAMIC_MODEL_PROCESSED_VERTICES= 8192,
	RASTERIZER_MAXIMUM_DYNAMIC_VERTEX_BUFFERS= 1024,
	RASTERIZER_MAXIMUM_DETAIL_OBJECTS_PER_FRAME= 4096,
	RASTERIZER_NODES_PER_MODEL_VERTEX= 2,
	RASTERIZER_MAXIMUM_NODES_PER_MODEL= 44,
	RASTERIZER_MAXIMUM_NEARBY_OPAQUE_MODEL_GEOMETRY_GROUPS_THAT_MIGHT_OBSCURE_THE_ENVIRONMENT_FOG_SCREEN_EFFECT= 1
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/RASTERIZER.C */

boolean rasterizer_initialize(void);
void rasterizer_dispose(void);

void rasterizer_decals_update_function_pointers(void);

/* ---------- prototypes/RASTERIZER_TEXT.C */

void rasterizer_draw_string(union rectangle2d const *bounds, union rectangle2d const *clip, union point2d *cursor_reference, short height_adjust, char const *string);

/* ---------- prototypes/RASTERIZER_MEMORY_POOL.C */

boolean rasterizer_memory_pool_initialize(void);
void rasterizer_memory_pool_begin(void);
void *rasterizer_memory_alloc(const void *src, unsigned long size);
const void *rasterizer_memory_alloc_const(const void *src, unsigned long size);
void rasterizer_memory_pool_end(void);
void rasterizer_memory_pool_dispose(void);

/* ---------- globals */

/* ---------- public code */

#endif // __RASTERIZER_H
