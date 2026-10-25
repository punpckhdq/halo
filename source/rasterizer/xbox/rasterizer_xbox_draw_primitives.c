/*
RASTERIZER_XBOX_DRAW_PRIMITIVES.C
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
#include "strings/resource.h"
#include "main.h"

/* ---------- constants */

enum
{
	RASTERIZER_MAXIMUM_PRIMITIVES_PER_DRAW_COMMAND = 10000
};

/* ---------- structures */

struct dynamic_vertex_buffer_group
{
	long vertex_count;
	long max_vertex_count;
	long total_vertex_count; /* fake name */
	IDirect3DVertexBuffer8 *d3d_vertex_buffer;
	boolean first_lock; /* fake name */
};

struct dynamic_vertex_buffer
{
	short type;
	long vertex_start_index;
	long vertex_count;
	void *data;
};

struct dynamic_triangle_buffer
{
	long triangle_start_index;
	long triangle_count;
	struct rasterizer_triangle *data;
};

/* ---------- globals */

static D3DPRIMITIVETYPE const d3d_primitive_type_table[NUMBER_OF_TRIANGLE_BUFFER_TYPES] =
{
	D3DPT_TRIANGLELIST,		// _triangle_buffer_type_triangles
	D3DPT_TRIANGLESTRIP		// _triangle_buffer_type_precompiled_strip
};

static struct
{
	struct dynamic_vertex_buffer_group groups[NUMBER_OF_RASTERIZER_VERTEX_TYPES];
	struct dynamic_vertex_buffer buffers[RASTERIZER_MAXIMUM_DYNAMIC_VERTEX_BUFFERS];
	long buffer_count;
} dynamic_vertices = {0};

static struct
{
	struct dynamic_triangle_buffer buffers[RASTERIZER_MAXIMUM_DYNAMIC_TRIANGLE_BUFFERS];
	long buffer_count;
	long triangle_count;
	IDirect3DIndexBuffer8 *d3d_index_buffer;
	boolean first_lock; /* fake name */
} dynamic_triangles = {0};

static IDirect3DVertexBuffer8 *aux_dynamic_unlit_vb = NULL;

/* ---------- public code */

boolean rasterizer_dynamic_geometry_initialize(
	void)
{
	short vertex_type;
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 93, global_d3d_device);

	D3DCALL(success, IDirect3DDevice8_CreateIndexBuffer(global_d3d_device,
		sizeof(struct rasterizer_triangle)*RASTERIZER_MAXIMUM_DYNAMIC_TRIANGLES,
		RASTERIZER_DYNAMIC_BUFFER_USAGE,
		D3DFMT_INDEX16,
		RASTERIZER_DYNAMIC_BUFFER_POOL,
		&dynamic_triangles.d3d_index_buffer));

	if (!dynamic_triangles.d3d_index_buffer)
	{
		success = FALSE;
	}

	if (!success)
	{
		dynamic_triangles.d3d_index_buffer = NULL;
		error(_error_silent, "### ERROR failed to create dynamic triangle buffer");
	}

	for (vertex_type = 0; success && vertex_type < NUMBER_OF_RASTERIZER_VERTEX_TYPES; vertex_type++)
	{
		long count;
		struct dynamic_vertex_buffer_group *group = &dynamic_vertices.groups[vertex_type];

		switch (vertex_type)
		{
		case _rasterizer_vertex_type_dynamic_unlit:
			count = RASTERIZER_MAXIMUM_DYNAMIC_UNLIT_VERTICES;
			break;
		case _rasterizer_vertex_type_debug:
			count = NUMBER_OF_VERTICES_PER_TRIANGLE * RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES;
			break;
		case _rasterizer_vertex_type_model_compressed:
			count = RASTERIZER_MAXIMUM_DYNAMIC_MODEL_VERTICES;
			break;
		case _rasterizer_vertex_type_dynamic_lit:
		case _rasterizer_vertex_type_dynamic_screen:
			count = 0;
			break;
		default:
			count = 0;
		}

		if (count <= 0)
		{
			group->d3d_vertex_buffer = NULL;
		}
		else
		{
			D3DCALL(success, IDirect3DDevice8_CreateVertexBuffer(global_d3d_device,
				rasterizer_geometry_get_vertex_size(vertex_type)*count,
				RASTERIZER_DYNAMIC_BUFFER_USAGE,
				0,
				RASTERIZER_DYNAMIC_BUFFER_POOL,
				&group->d3d_vertex_buffer));

			if (!group->d3d_vertex_buffer)
			{
				success = FALSE;
			}

			if (!success)
			{
				group->d3d_vertex_buffer = NULL;
				error(_error_silent, "### ERROR failed to create dynamic vertex buffer");
			}
		}

		group->max_vertex_count = count;
		group->total_vertex_count = count;
	}

	if (success)
	{
		D3DCALL(success, IDirect3DDevice8_CreateVertexBuffer(global_d3d_device,
			rasterizer_geometry_get_vertex_size(_rasterizer_vertex_type_dynamic_unlit)*RASTERIZER_MAXIMUM_DYNAMIC_UNLIT_VERTICES,
			RASTERIZER_DYNAMIC_BUFFER_USAGE,
			0,
			RASTERIZER_DYNAMIC_BUFFER_POOL,
			&aux_dynamic_unlit_vb));

		if (!aux_dynamic_unlit_vb)
		{
			success = FALSE;
		}

		if (!success)
		{
			aux_dynamic_unlit_vb = NULL;
		}
	}

	if (!success)
	{
		error(_error_silent, "### ERROR failed to initialize rasterizer dynamic geometry");
	}

	return success;
}

void rasterizer_dynamic_geometry_begin(
	void)
{
	long vertex_type;

	if (rasterizer_debug_options.splitscreen_VB_optimization_enabled)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 198, global_window_parameters.window_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 199, global_window_parameters.window_index<main_get_window_count());

		for (vertex_type = 0; vertex_type < NUMBER_OF_RASTERIZER_VERTEX_TYPES; vertex_type++)
		{
			if (global_window_parameters.window_index == 0)
			{
				dynamic_vertices.groups[vertex_type].vertex_count = 0;
				dynamic_vertices.groups[vertex_type].first_lock = TRUE;
			}
			else
			{
				dynamic_vertices.groups[vertex_type].max_vertex_count = dynamic_vertices.groups[vertex_type].total_vertex_count * (global_window_parameters.window_index + 1) / main_get_window_count();
			}
		}

		dynamic_triangles.triangle_count = 0;
		dynamic_triangles.first_lock = TRUE;

		if (global_window_parameters.window_index == 0)
		{
			dynamic_vertices.buffer_count = 0;
		}

		dynamic_triangles.buffer_count = 0;
	}
	else
	{
		for (vertex_type = 0; vertex_type < NUMBER_OF_RASTERIZER_VERTEX_TYPES; vertex_type++)
		{
			dynamic_vertices.groups[vertex_type].vertex_count = 0;
			dynamic_vertices.groups[vertex_type].first_lock = TRUE;
		}

		dynamic_triangles.triangle_count = 0;
		dynamic_triangles.first_lock = TRUE;
		dynamic_vertices.buffer_count = 0;
		dynamic_triangles.buffer_count = 0;
	}

	return;
}

void rasterizer_dynamic_geometry_end(
	void)
{
	return;
}

void rasterizer_dynamic_geometry_dispose(
	void)
{
	short vertex_type;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 263, global_d3d_device);

	for (vertex_type = 0; vertex_type < NUMBER_OF_RASTERIZER_VERTEX_TYPES; vertex_type++)
	{
		struct dynamic_vertex_buffer_group *group = &dynamic_vertices.groups[vertex_type];

		if (group->d3d_vertex_buffer)
		{
			IDirect3DVertexBuffer8_Release(group->d3d_vertex_buffer);
			group->d3d_vertex_buffer = NULL;
		}
	}

	if (aux_dynamic_unlit_vb)
	{
		IDirect3DVertexBuffer8_Release(aux_dynamic_unlit_vb);
		aux_dynamic_unlit_vb = NULL;
	}

	if (dynamic_triangles.d3d_index_buffer)
	{
		IDirect3DIndexBuffer8_Release(dynamic_triangles.d3d_index_buffer);
		dynamic_triangles.d3d_index_buffer = NULL;
	}

	return;
}

long _rasterizer_dynamic_triangles_new(
	long count)
{
	static boolean warned = FALSE;
	long dynamic_triangle_buffer_index = NONE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 284, count>=0);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 285, dynamic_triangles.d3d_index_buffer);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 286, global_d3d_device);

	if (count > 0)
	{
		if (dynamic_triangles.triangle_count >= RASTERIZER_MAXIMUM_DYNAMIC_TRIANGLES - count || dynamic_triangles.buffer_count >= RASTERIZER_MAXIMUM_DYNAMIC_TRIANGLE_BUFFERS - 1)
		{
			if (!warned)
			{
				error(_error_silent, "### ERROR too many dynamic triangles requested from rasterizer");
				warned = TRUE;
			}
		}
		else
		{
			struct dynamic_triangle_buffer *dynamic_triangle_buffer;

			dynamic_triangle_buffer_index = dynamic_triangles.buffer_count;
			dynamic_triangle_buffer = &dynamic_triangles.buffers[dynamic_triangle_buffer_index];
			dynamic_triangle_buffer->triangle_start_index = dynamic_triangles.triangle_count;
			dynamic_triangle_buffer->triangle_count = count;
			dynamic_triangles.triangle_count += count;
			dynamic_triangles.buffer_count++;

			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.dynamic_triangle_count += count;
				rasterizer_frame_statistics.dynamic_triangle_buffer_count++;
			}
		}
	}

	return dynamic_triangle_buffer_index;
}

struct rasterizer_triangle *_rasterizer_dynamic_triangles_lock(
	long dynamic_triangle_buffer_index)
{
	boolean success = TRUE;
	struct rasterizer_triangle *triangles = NULL;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 331, global_d3d_device);

	if (dynamic_triangle_buffer_index != NONE)
	{
		struct dynamic_triangle_buffer *dynamic_triangle_buffer;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 337, dynamic_triangle_buffer_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 338, dynamic_triangle_buffer_index<dynamic_triangles.buffer_count);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 339, dynamic_triangles.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 340, dynamic_triangles.d3d_index_buffer);

		dynamic_triangle_buffer = &dynamic_triangles.buffers[dynamic_triangle_buffer_index];
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 344, dynamic_triangle_buffer->triangle_count>0);

		D3DCALL(success, IDirect3DIndexBuffer8_Lock(dynamic_triangles.d3d_index_buffer,
			sizeof(struct rasterizer_triangle)*dynamic_triangle_buffer->triangle_start_index,
			sizeof(struct rasterizer_triangle)*dynamic_triangle_buffer->triangle_count,
			(BYTE **)&dynamic_triangle_buffer->data,
			dynamic_triangles.first_lock ? 0 : D3DLOCK_READONLY));
		dynamic_triangles.first_lock = FALSE;

		triangles = dynamic_triangle_buffer->data;
	}
	else
	{
		error(_error_silent, "### WARNING tried to lock dynamic triangles with index=NONE");
	}

	if (!success)
	{
		error(_error_silent, "### ERROR failed to lock dynamic triangles");
	}

	return triangles;
}

void _rasterizer_dynamic_triangles_unlock(
	long dynamic_triangle_buffer_index)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 373, global_d3d_device);

	if (dynamic_triangle_buffer_index != NONE)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 377, dynamic_triangle_buffer_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 378, dynamic_triangle_buffer_index<dynamic_triangles.buffer_count);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 379, dynamic_triangles.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 380, dynamic_triangles.d3d_index_buffer);

		D3DCALL(success, IDirect3DIndexBuffer8_Unlock(dynamic_triangles.d3d_index_buffer));
	}
	else
	{
		error(_error_silent, "### WARNING tried to unlock dynamic triangles with index=NONE");
	}

	if (!success)
	{
		error(_error_silent, "### ERROR failed to unlock dynamic triangles");
	}

	return;
}

void _rasterizer_dynamic_triangles_delete(
	long dynamic_triangle_buffer_index)
{
	return;
}

long _rasterizer_dynamic_vertices_new(
	short type,
	long count)
{
	static boolean warned = FALSE;
	long dynamic_vertex_buffer_index = NONE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 426, count>=0);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 427, type>=0 && type<NUMBER_OF_RASTERIZER_VERTEX_TYPES);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 428, dynamic_vertices.groups);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 429, dynamic_vertices.groups[type].d3d_vertex_buffer);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 430, global_d3d_device);

	if (count > 0)
	{
		struct dynamic_vertex_buffer_group *group = &dynamic_vertices.groups[type];

		if (group->vertex_count >= group->max_vertex_count - count || dynamic_vertices.buffer_count >= RASTERIZER_MAXIMUM_DYNAMIC_VERTEX_BUFFERS - 1)
		{
			if (!warned)
			{
				error(_error_silent, "### ERROR too many dynamic vertices requested from rasterizer");
				warned = TRUE;
			}
		}
		else
		{
			struct dynamic_vertex_buffer *dynamic_vertex_buffer;

			rasterizer_geometry_get_vertex_size(type);

			dynamic_vertex_buffer_index = dynamic_vertices.buffer_count;
			dynamic_vertex_buffer = &dynamic_vertices.buffers[dynamic_vertex_buffer_index];
			dynamic_vertex_buffer->type = type;
			dynamic_vertex_buffer->vertex_start_index = group->vertex_count;
			dynamic_vertex_buffer->vertex_count = count;
			group->vertex_count += count;
			dynamic_vertices.buffer_count++;

			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.dynamic_vertex_count += count;
				rasterizer_frame_statistics.dynamic_vertex_buffer_count++;
			}
		}
	}

	return dynamic_vertex_buffer_index;
}

short _rasterizer_dynamic_vertices_get_type(
	long dynamic_vertex_buffer_index)
{
	short type = NONE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 478, global_d3d_device);

	if (dynamic_vertex_buffer_index != NONE)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 484, dynamic_vertex_buffer_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 485, dynamic_vertex_buffer_index<dynamic_vertices.buffer_count);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 486, dynamic_vertices.buffers);

		type = dynamic_vertices.buffers[dynamic_vertex_buffer_index].type;
	}
	else
	{
		error(_error_silent, "### WARNING tried to query dynamic vertices with index=NONE");
	}

	return type;
}

static IDirect3DVertexBuffer8 *rasterizer_dynamic_vertices_get_buffer(
	struct dynamic_vertex_buffer_group const *group)
{
	IDirect3DVertexBuffer8 *d3d_vertex_buffer;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 504, group);

	if (group == &dynamic_vertices.groups[_rasterizer_vertex_type_dynamic_unlit] && (rasterizer_globals.frame_index & 1))
	{
		d3d_vertex_buffer = aux_dynamic_unlit_vb;
	}
	else
	{
		d3d_vertex_buffer = group->d3d_vertex_buffer;
	}

	return d3d_vertex_buffer;
}

void *_rasterizer_dynamic_vertices_lock(
	long dynamic_vertex_buffer_index)
{
	void *vertices = NULL;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 519, global_d3d_device);

	if (rasterizer_globals.current_lock_operation == _rasterizer_lock_none)
	{
		error(_error_silent, "### WARNING: tried to lock dynamic vertices without specifying a lock operation");
	}

	if (dynamic_vertex_buffer_index != NONE)
	{
		struct dynamic_vertex_buffer *dynamic_vertex_buffer;
		struct dynamic_vertex_buffer_group *group;
		IDirect3DVertexBuffer8 *d3d_vertex_buffer;
		long vertex_size;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 535, dynamic_vertex_buffer_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 536, dynamic_vertex_buffer_index<dynamic_vertices.buffer_count);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 537, dynamic_vertices.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 538, dynamic_vertices.groups);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 539, dynamic_triangles.d3d_index_buffer);

		dynamic_vertex_buffer = &dynamic_vertices.buffers[dynamic_vertex_buffer_index];
		vertex_size = rasterizer_geometry_get_vertex_size(dynamic_vertex_buffer->type);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 544, dynamic_vertex_buffer->type>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 545, dynamic_vertex_buffer->type<NUMBER_OF_RASTERIZER_VERTEX_TYPES);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 546, dynamic_vertex_buffer->vertex_count>0);

		group = &dynamic_vertices.groups[dynamic_vertex_buffer->type];
		d3d_vertex_buffer = rasterizer_dynamic_vertices_get_buffer(group);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 551, d3d_vertex_buffer);

		IDirect3DVertexBuffer8_Lock(d3d_vertex_buffer,
			vertex_size*dynamic_vertex_buffer->vertex_start_index,
			vertex_size*dynamic_vertex_buffer->vertex_count,
			(BYTE **)&dynamic_vertex_buffer->data,
			group->first_lock ? 0 : D3DLOCK_READONLY);
		group->first_lock = FALSE;

		vertices = dynamic_vertex_buffer->data;
	}
	else
	{
		error(_error_silent, "### WARNING tried to lock dynamic vertices with index=NONE");
	}

	return vertices;
}

void _rasterizer_dynamic_vertices_unlock(
	long dynamic_vertex_buffer_index)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 582, global_d3d_device);

	if (dynamic_vertex_buffer_index != NONE)
	{
		struct dynamic_vertex_buffer *buffer;
		IDirect3DVertexBuffer8 *d3d_vertex_buffer;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 588, dynamic_vertex_buffer_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 589, dynamic_vertex_buffer_index<dynamic_vertices.buffer_count);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 590, dynamic_vertices.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 591, dynamic_triangles.d3d_index_buffer);

		buffer = &dynamic_vertices.buffers[dynamic_vertex_buffer_index];
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 595, buffer->type>=0 && buffer->type<NUMBER_OF_RASTERIZER_VERTEX_TYPES);

		d3d_vertex_buffer = rasterizer_dynamic_vertices_get_buffer(&dynamic_vertices.groups[buffer->type]);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 600, d3d_vertex_buffer);

		IDirect3DVertexBuffer8_Unlock(d3d_vertex_buffer);
	}
	else
	{
		error(_error_silent, "### WARNING tried to unlock dynamic vertices with index=NONE");
	}

	return;
}

void _rasterizer_dynamic_vertices_delete(
	long dynamic_vertex_buffer_index)
{
	return;
}

void rasterizer_draw_dynamic_vertices(
	long first_primitive_index,
	long primitive_count,
	long dynamic_vertex_buffer_index,
	short vertices_per_primitive)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 651, global_d3d_device);

	while (primitive_count > 0 && dynamic_vertex_buffer_index != NONE)
	{
		struct dynamic_vertex_buffer *dynamic_vertex_buffer;
		struct dynamic_vertex_buffer_group *group;
		IDirect3DVertexBuffer8 *d3d_vertex_buffer;
		D3DPRIMITIVETYPE d3d_primitive_type;
		long vertex_size;
		long local_primitive_count;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 663, dynamic_triangles.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 664, dynamic_vertices.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 665, dynamic_vertices.groups);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 666, dynamic_vertex_buffer_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 667, dynamic_vertex_buffer_index<dynamic_vertices.buffer_count);

		switch (vertices_per_primitive)
		{
		case NUMBER_OF_VERTICES_PER_LINE:
			d3d_primitive_type = D3DPT_LINELIST;
			break;
		case NUMBER_OF_VERTICES_PER_TRIANGLE:
			d3d_primitive_type = D3DPT_TRIANGLELIST;
			break;
		case NUMBER_OF_VERTICES_PER_QUADRILATERAL:
			d3d_primitive_type = D3DPT_QUADLIST;
			break;
		default:
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 682, primitive_count==1);
			primitive_count = vertices_per_primitive - 2;
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 686, first_primitive_index==0);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 687, vertices_per_primitive<=RASTERIZER_MAXIMUM_PRIMITIVES_PER_DRAW_COMMAND);
			d3d_primitive_type = D3DPT_TRIANGLESTRIP;
		}

		dynamic_vertex_buffer = &dynamic_vertices.buffers[dynamic_vertex_buffer_index];
		vertex_size = rasterizer_geometry_get_vertex_size(dynamic_vertex_buffer->type);
		group = &dynamic_vertices.groups[dynamic_vertex_buffer->type];
		d3d_vertex_buffer = rasterizer_dynamic_vertices_get_buffer(group);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 701, d3d_vertex_buffer);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 704, dynamic_vertex_buffer->vertex_start_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 705, dynamic_vertex_buffer->vertex_start_index<=group->vertex_count - dynamic_vertex_buffer->vertex_count);

		local_primitive_count = MIN(primitive_count, RASTERIZER_MAXIMUM_PRIMITIVES_PER_DRAW_COMMAND);

		D3DCALL(success, IDirect3DDevice8_SetStreamSource(global_d3d_device, 0, d3d_vertex_buffer, vertex_size));
		D3DCALL(success, IDirect3DDevice8_DrawPrimitive(global_d3d_device,
			d3d_primitive_type,
			first_primitive_index*vertices_per_primitive + dynamic_vertex_buffer->vertex_start_index,
			local_primitive_count));

		first_primitive_index += local_primitive_count;
		primitive_count -= local_primitive_count;
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_draw_dynamic_vertices failed");
	}

	return;
}

void rasterizer_draw_dynamic_triangles_dynamic_vertices(
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	long dynamic_vertex_buffer_index)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 741, global_d3d_device);

	while (triangle_count > 0 && dynamic_triangle_buffer_index != NONE && dynamic_vertex_buffer_index != NONE)
	{
		struct dynamic_triangle_buffer *dynamic_triangle_buffer;
		struct dynamic_vertex_buffer *dynamic_vertex_buffer;
		struct dynamic_vertex_buffer_group *group;
		IDirect3DVertexBuffer8 *d3d_vertex_buffer;
		long vertex_size;
		long local_triangle_count;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 755, dynamic_triangles.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 756, dynamic_triangles.d3d_index_buffer);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 757, dynamic_vertices.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 758, dynamic_vertices.groups);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 759, dynamic_triangle_buffer_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 760, dynamic_triangle_buffer_index<dynamic_triangles.buffer_count);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 761, dynamic_vertex_buffer_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 762, dynamic_vertex_buffer_index<dynamic_vertices.buffer_count);

		dynamic_triangle_buffer = &dynamic_triangles.buffers[dynamic_triangle_buffer_index];
		dynamic_vertex_buffer = &dynamic_vertices.buffers[dynamic_vertex_buffer_index];
		vertex_size = rasterizer_geometry_get_vertex_size(dynamic_vertex_buffer->type);
		group = &dynamic_vertices.groups[dynamic_vertex_buffer->type];
		d3d_vertex_buffer = rasterizer_dynamic_vertices_get_buffer(group);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 773, d3d_vertex_buffer);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 776, dynamic_vertex_buffer->vertex_start_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 777, dynamic_vertex_buffer->vertex_start_index<=group->vertex_count - dynamic_vertex_buffer->vertex_count);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 780, dynamic_triangle_buffer->triangle_start_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 781, triangle_count>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 782, triangle_count<=dynamic_triangle_buffer->triangle_count - first_triangle_index);

		local_triangle_count = MIN(triangle_count, RASTERIZER_MAXIMUM_PRIMITIVES_PER_DRAW_COMMAND);

		D3DCALL(success, IDirect3DDevice8_SetStreamSource(global_d3d_device, 0, d3d_vertex_buffer, vertex_size));
		D3DCALL(success, IDirect3DDevice8_SetIndices(global_d3d_device,
			dynamic_triangles.d3d_index_buffer,
			dynamic_vertex_buffer->vertex_start_index));
		D3DCALL(success, IDirect3DDevice8_DrawIndexedPrimitive(global_d3d_device,
			D3DPT_TRIANGLELIST,
			0,
			dynamic_vertex_buffer->vertex_count,
			NUMBER_OF_VERTICES_PER_TRIANGLE*(first_triangle_index + dynamic_triangle_buffer->triangle_start_index),
			local_triangle_count));

		first_triangle_index += local_triangle_count;
		triangle_count -= local_triangle_count;
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_draw_dynamic_triangles_dynamic_vertices failed");
	}

	return;
}

void rasterizer_draw_dynamic_triangles_static_vertices(
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 823, global_d3d_device);

	while (triangle_count > 0 && dynamic_triangle_buffer_index != NONE && vertex_buffer && vertex_buffer->hardware_format)
	{
		struct dynamic_triangle_buffer *dynamic_triangle_buffer;
		long vertex_size;
		long local_triangle_count;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 833, dynamic_triangles.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 834, dynamic_triangles.d3d_index_buffer);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 835, dynamic_vertices.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 836, dynamic_vertices.groups);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 837, dynamic_triangle_buffer_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 838, dynamic_triangle_buffer_index<dynamic_triangles.buffer_count);

		dynamic_triangle_buffer = &dynamic_triangles.buffers[dynamic_triangle_buffer_index];
		vertex_size = rasterizer_geometry_get_vertex_size(vertex_buffer->type);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 845, dynamic_triangle_buffer->triangle_start_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 846, triangle_count>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 847, triangle_count<=dynamic_triangle_buffer->triangle_count - first_triangle_index);

		local_triangle_count = MIN(triangle_count, RASTERIZER_MAXIMUM_PRIMITIVES_PER_DRAW_COMMAND);

		D3DCALL(success, IDirect3DDevice8_SetStreamSource(global_d3d_device,
			0,
			(IDirect3DVertexBuffer8*)vertex_buffer->hardware_format,
			vertex_size));
		D3DCALL(success, IDirect3DDevice8_SetIndices(global_d3d_device, dynamic_triangles.d3d_index_buffer, 0));
		D3DCALL(success, IDirect3DDevice8_DrawIndexedPrimitive(global_d3d_device,
			D3DPT_TRIANGLELIST,
			0,
			vertex_buffer->count,
			NUMBER_OF_VERTICES_PER_TRIANGLE*(first_triangle_index + dynamic_triangle_buffer->triangle_start_index),
			local_triangle_count));

		first_triangle_index += local_triangle_count;
		triangle_count -= local_triangle_count;
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_draw_dynamic_triangles_static_vertices failed");
	}

	return;
}

void rasterizer_draw_dynamic_triangles_static_vertices2(
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer0,
	struct vertex_buffer const *vertex_buffer1)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 890, global_d3d_device);

	while (triangle_count > 0 && dynamic_triangle_buffer_index != NONE && vertex_buffer0 && vertex_buffer0->hardware_format && vertex_buffer1 && vertex_buffer1->hardware_format)
	{
		struct dynamic_triangle_buffer *dynamic_triangle_buffer;
		long vertex_size0;
		long vertex_size1;
		long local_triangle_count;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 902, dynamic_triangles.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 903, dynamic_triangles.d3d_index_buffer);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 904, dynamic_vertices.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 905, dynamic_vertices.groups);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 906, dynamic_triangle_buffer_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 907, dynamic_triangle_buffer_index<dynamic_triangles.buffer_count);

		dynamic_triangle_buffer = &dynamic_triangles.buffers[dynamic_triangle_buffer_index];
		vertex_size0 = rasterizer_geometry_get_vertex_size(vertex_buffer0->type);
		vertex_size1 = rasterizer_geometry_get_vertex_size(vertex_buffer1->type);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 915, dynamic_triangle_buffer->triangle_start_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 916, triangle_count>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 917, triangle_count<=dynamic_triangle_buffer->triangle_count - first_triangle_index);

		local_triangle_count = MIN(triangle_count, RASTERIZER_MAXIMUM_PRIMITIVES_PER_DRAW_COMMAND);

		D3DCALL(success, IDirect3DDevice8_SetStreamSource(global_d3d_device,
			0,
			(IDirect3DVertexBuffer8*)vertex_buffer0->hardware_format,
			vertex_size0));
		D3DCALL(success, IDirect3DDevice8_SetStreamSource(global_d3d_device,
			1,
			(IDirect3DVertexBuffer8*)vertex_buffer1->hardware_format,
			vertex_size1));
		D3DCALL(success, IDirect3DDevice8_SetIndices(global_d3d_device, dynamic_triangles.d3d_index_buffer, 0));
		D3DCALL(success, IDirect3DDevice8_DrawIndexedPrimitive(global_d3d_device,
			D3DPT_TRIANGLELIST,
			0,
			vertex_buffer0->count,
			NUMBER_OF_VERTICES_PER_TRIANGLE*(first_triangle_index + dynamic_triangle_buffer->triangle_start_index),
			local_triangle_count));

		first_triangle_index += local_triangle_count;
		triangle_count -= local_triangle_count;
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_draw_dynamic_triangles_static_vertices2 failed");
	}

	return;
}

void rasterizer_draw_static_triangles_dynamic_vertices(
	struct triangle_buffer const *triangle_buffer,
	long first_triangle_index,
	long triangle_count,
	long dynamic_vertex_buffer_index)
{
	boolean success = TRUE;
	long local_triangle_vertex_indices_offset = 0;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 963, global_d3d_device);

	while (triangle_count > 0 && triangle_buffer && triangle_buffer->hardware_format && dynamic_vertex_buffer_index != NONE)
	{
		struct dynamic_vertex_buffer *dynamic_vertex_buffer;
		struct dynamic_vertex_buffer_group *group;
		IDirect3DVertexBuffer8 *d3d_vertex_buffer;
		long vertex_size;
		long local_triangle_count;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 976, dynamic_triangles.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 977, dynamic_triangles.d3d_index_buffer);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 978, dynamic_vertices.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 979, dynamic_vertices.groups);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 980, dynamic_vertex_buffer_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 981, dynamic_vertex_buffer_index<dynamic_vertices.buffer_count);

		dynamic_vertex_buffer = &dynamic_vertices.buffers[dynamic_vertex_buffer_index];
		vertex_size = rasterizer_geometry_get_vertex_size(dynamic_vertex_buffer->type);
		group = &dynamic_vertices.groups[dynamic_vertex_buffer->type];
		d3d_vertex_buffer = rasterizer_dynamic_vertices_get_buffer(group);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 991, d3d_vertex_buffer);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 994, dynamic_vertex_buffer->vertex_start_index>=0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 995, dynamic_vertex_buffer->vertex_start_index<=group->vertex_count - dynamic_vertex_buffer->vertex_count);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 998, first_triangle_index==0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 999, triangle_buffer->type>=0 && triangle_buffer->type<NUMBER_OF_TRIANGLE_BUFFER_TYPES);

		local_triangle_count = MIN(triangle_count, RASTERIZER_MAXIMUM_PRIMITIVES_PER_DRAW_COMMAND);

		D3DCALL(success, IDirect3DDevice8_SetStreamSource(global_d3d_device, 0, d3d_vertex_buffer, vertex_size));
		D3DCALL(success, IDirect3DDevice8_SetIndices(global_d3d_device,
			(IDirect3DIndexBuffer8*)triangle_buffer->hardware_format,
			dynamic_vertex_buffer->vertex_start_index));
		D3DCALL(success, IDirect3DDevice8_DrawIndexedPrimitive(global_d3d_device,
			d3d_primitive_type_table[triangle_buffer->type],
			0,
			dynamic_vertex_buffer->vertex_count,
			local_triangle_vertex_indices_offset,
			local_triangle_count));

		triangle_count -= local_triangle_count;

		switch (triangle_buffer->type)
		{
		case _triangle_buffer_type_triangles:
			local_triangle_vertex_indices_offset += NUMBER_OF_VERTICES_PER_TRIANGLE * local_triangle_count;
			break;
		case _triangle_buffer_type_precompiled_strip:
			local_triangle_vertex_indices_offset += local_triangle_count;
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1030, FALSE, "### ERROR unsupported triangle buffer type");
		}
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_draw_static_triangles_dynamic_vertices failed");
	}

	return;
}

void rasterizer_draw_static_triangles_static_vertices(
	struct triangle_buffer const *triangle_buffer,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	boolean success = TRUE;
	long local_triangle_vertex_indices_offset = 0;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1063, global_d3d_device);

	while (triangle_count > 0 && triangle_buffer && triangle_buffer->hardware_format && vertex_buffer && vertex_buffer->hardware_format)
	{
		long vertex_size;
		long local_triangle_count;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1072, dynamic_triangles.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1073, dynamic_triangles.d3d_index_buffer);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1074, dynamic_vertices.buffers);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1075, dynamic_vertices.groups);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1078, first_triangle_index==0);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1079, triangle_buffer->type>=0 && triangle_buffer->type<NUMBER_OF_TRIANGLE_BUFFER_TYPES);

		vertex_size = rasterizer_geometry_get_vertex_size(vertex_buffer->type);
		local_triangle_count = MIN(triangle_count, RASTERIZER_MAXIMUM_PRIMITIVES_PER_DRAW_COMMAND);

		D3DCALL(success, IDirect3DDevice8_SetStreamSource(global_d3d_device,
			0,
			(IDirect3DVertexBuffer8*)vertex_buffer->hardware_format,
			vertex_size));
		D3DCALL(success, IDirect3DDevice8_SetIndices(global_d3d_device,
			(IDirect3DIndexBuffer8*)triangle_buffer->hardware_format,
			0));
		D3DCALL(success, IDirect3DDevice8_DrawIndexedPrimitive(global_d3d_device,
			d3d_primitive_type_table[triangle_buffer->type],
			0,
			vertex_buffer->count,
			local_triangle_vertex_indices_offset,
			local_triangle_count));

		triangle_count -= local_triangle_count;

		switch (triangle_buffer->type)
		{
		case _triangle_buffer_type_triangles:
			local_triangle_vertex_indices_offset += NUMBER_OF_VERTICES_PER_TRIANGLE * local_triangle_count;
			break;
		case _triangle_buffer_type_precompiled_strip:
			local_triangle_vertex_indices_offset += local_triangle_count;
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1113, FALSE, "### ERROR unsupported triangle buffer type");
		}
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_draw_static_triangles_static_vertices failed");
	}

	return;
}

void rasterizer_draw(
	struct triangle_buffer const *triangle_buffer,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer,
	long dynamic_vertex_buffer_index)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1145, triangle_buffer || dynamic_triangle_buffer_index!=NONE);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1146, !triangle_buffer || dynamic_triangle_buffer_index==NONE);

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1149, vertex_buffer || dynamic_vertex_buffer_index!=NONE);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_draw_primitives.c", 1150, !vertex_buffer || dynamic_vertex_buffer_index==NONE);

	if (triangle_buffer)
	{
		if (vertex_buffer)
		{
			rasterizer_draw_static_triangles_static_vertices(
				triangle_buffer,
				first_triangle_index,
				triangle_count,
				vertex_buffer);
		}
		else
		{
			rasterizer_draw_static_triangles_dynamic_vertices(
				triangle_buffer,
				first_triangle_index,
				triangle_count,
				dynamic_vertex_buffer_index);
		}
	}
	else
	{
		if (vertex_buffer)
		{
			rasterizer_draw_dynamic_triangles_static_vertices(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				vertex_buffer);
		}
		else
		{
			rasterizer_draw_dynamic_triangles_dynamic_vertices(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				dynamic_vertex_buffer_index);
		}
	}

	return;
}
