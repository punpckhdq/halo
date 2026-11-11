/*
RASTERIZER_XBOX_HARDWARE_GEOMETRY.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer_xbox.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"

/* ---------- constants */

/* ---------- macros */

#define RASTERIZER_STATIC_BUFFER_USAGE D3DUSAGE_WRITEONLY
#define RASTERIZER_STATIC_BUFFER_POOL D3DPOOL_MANAGED

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

boolean rasterizer_vertex_buffer_new(
	struct vertex_buffer *vertex_buffer,
	short type,
	long count,
	void *vertices,
	long buffer_size)
{
	IDirect3DVertexBuffer8 *d3d_vertex_buffer;
	byte *vertex_data;
	boolean success = TRUE;
	short vertex_size = (short)rasterizer_geometry_get_vertex_size(type);

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_geometry.c", 24, vertex_buffer);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_geometry.c", 25, vertex_size*count==buffer_size || !vertices);

	if (!count)
	{
		success = FALSE;
	}

	if (!global_d3d_device)
	{
		success = FALSE;
	}
	else if (success)
	{
		D3DCALL(success, IDirect3DDevice8_CreateVertexBuffer(global_d3d_device,
			buffer_size,
			RASTERIZER_STATIC_BUFFER_USAGE,
			0,
			RASTERIZER_STATIC_BUFFER_POOL,
			&d3d_vertex_buffer));

		if (!d3d_vertex_buffer)
		{
			success = FALSE;
		}

		if (!success)
		{
			d3d_vertex_buffer = NULL;
		}
	}

	if (vertices && success)
	{
		rasterizer_globals.current_lock_operation = _rasterizer_lock_vertexbuffer_new;
		D3DCALL(success, IDirect3DVertexBuffer8_Lock(d3d_vertex_buffer, 0, buffer_size, &vertex_data, 0));
		rasterizer_globals.current_lock_operation = _rasterizer_lock_none;

		if (!vertex_data)
		{
			success = FALSE;
		}

		if (success)
		{
			memcpy(vertex_data, vertices, buffer_size);
			D3DCALL(success, IDirect3DVertexBuffer8_Unlock(d3d_vertex_buffer));
			D3DVertexBuffer_MoveResourceMemory(d3d_vertex_buffer, D3DMEM_VIDEO);

			vertex_buffer->count = count;
			vertex_buffer->offset = 0;
			vertex_buffer->type = type;
			vertex_buffer->base_address = vertices;
			vertex_buffer->hardware_format = d3d_vertex_buffer;
		}
		else
		{
			vertex_data = NULL;
		}
	}

	if (!success)
	{
		memset(vertex_buffer, 0, sizeof(*vertex_buffer));
		error(_error_silent, "### ERROR failed to create vertex buffer hardware format");
	}

	return success;
}

void rasterizer_vertex_buffer_delete(
	struct vertex_buffer *vertex_buffer)
{
	if (vertex_buffer && vertex_buffer->hardware_format)
	{
		IDirect3DVertexBuffer8_Release((IDirect3DVertexBuffer8 *)vertex_buffer->hardware_format);
		vertex_buffer->hardware_format = NULL;
	}

	return;
}

boolean rasterizer_triangle_buffer_new(
	struct triangle_buffer *triangle_buffer,
	short type,
	long count,
	void const *triangles)
{
	IDirect3DIndexBuffer8 *d3d_index_buffer;
	byte *index_buffer_data;
	boolean success = TRUE;
	long buffer_size = 0;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_geometry.c", 115, triangle_buffer);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_geometry.c", 116, triangles);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_geometry.c", 117, count>0);

	switch (type)
	{
	case _triangle_buffer_type_triangles:
		buffer_size = count * sizeof(struct rasterizer_triangle);
		break;
	case _triangle_buffer_type_precompiled_strip:
		buffer_size = (count + 2) * sizeof(word);
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_geometry.c", 128, FALSE, "### ERROR unsupported triangle buffer type");
	}

	if (!global_d3d_device)
	{
		success = FALSE;
	}
	else
	{
		if (success)
		{
			D3DCALL(success, IDirect3DDevice8_CreateIndexBuffer(global_d3d_device,
				buffer_size,
				RASTERIZER_STATIC_BUFFER_USAGE,
				D3DFMT_INDEX16,
				RASTERIZER_STATIC_BUFFER_POOL,
				&d3d_index_buffer));

			if (!d3d_index_buffer)
			{
				success = FALSE;
			}

			if (!success)
			{
				d3d_index_buffer = NULL;
			}
		}

		if (success)
		{
			D3DCALL(success, IDirect3DIndexBuffer8_Lock(d3d_index_buffer, 0, buffer_size, &index_buffer_data, 0));

			if (!index_buffer_data)
			{
				success = FALSE;
			}

			if (!success)
			{
				index_buffer_data = NULL;
			}
		}

		if (success)
		{
			memcpy(index_buffer_data, triangles, buffer_size);
			D3DCALL(success, IDirect3DIndexBuffer8_Unlock(d3d_index_buffer));

			triangle_buffer->type = type;
			triangle_buffer->count = count;
			triangle_buffer->offset = (long)triangles;
			triangle_buffer->hardware_format = d3d_index_buffer;
		}
	}

	if (!success)
	{
		memset(triangle_buffer, 0, sizeof(*triangle_buffer));
		error(_error_silent, "### ERROR failed to create triangle buffer hardware format");
	}

	return success;
}

void rasterizer_triangle_buffer_delete(
	struct triangle_buffer *triangle_buffer)
{
	if (triangle_buffer && triangle_buffer->hardware_format)
	{
		IDirect3DIndexBuffer8_Release((IDirect3DIndexBuffer8 *)triangle_buffer->hardware_format);
		triangle_buffer->hardware_format = NULL;
	}

	return;
}

/* ---------- private code */
