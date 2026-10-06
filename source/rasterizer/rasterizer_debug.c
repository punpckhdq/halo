/*
RASTERIZER_DEBUG.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer.h"
#include "rasterizer_console_vars.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "bitmaps_inlines.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "rasterizer/xbox/rasterizer_xbox.h"
#include "rasterizer_geometry.h"

/* ---------- structures */

struct debug_vertex /* fake name */
{
	real_point3d point;
	pixel32 color;
};

struct debug_primitive /* fake name */
{
	struct debug_vertex vertices[3];
	short vertex_count;
	word pad;
	real sort_distance;
	boolean opaque;
};

struct debug_data /* fake name */
{
	boolean initialized;
	struct debug_primitive *opaque_triangles;
	long opaque_triangle_count;
	struct debug_primitive *opaque_lines;
	long opaque_line_count;
	struct debug_primitive *non_opaque_primitives;
	long non_opaque_primitive_count;
	long primitive_count;
};

/* ---------- globals */

static struct debug_data debug_data;
static boolean overflow_warning_given = FALSE; /* fake name */

/* ---------- public code */

long rasterizer_debug_new_primitive(
	long *count)
{
	long index = NONE;

	if (*count<RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES && debug_data.primitive_count<RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES)
	{
		index = (*count)++;
		debug_data.primitive_count++;
		if (rasterizer_debug_options.statistics_mode==_rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.debug_primitive_count++;
		}
	}
	else if (!overflow_warning_given)
	{
		error(_error_silent, "### WARNING debug geometry buffer overflow");
		overflow_warning_given = TRUE;
	}

	return index;
}

boolean rasterizer_debug_initialize(
	void)
{
	boolean success = TRUE;

	debug_data.opaque_triangles = match_malloc("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 96, RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES*sizeof(struct debug_primitive));
	debug_data.opaque_lines = match_malloc("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 97, RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES*sizeof(struct debug_primitive));
	debug_data.non_opaque_primitives = match_malloc("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 98, RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES*sizeof(struct debug_primitive));

	if (debug_data.opaque_triangles && debug_data.opaque_lines && debug_data.non_opaque_primitives)
	{
		debug_data.initialized = TRUE;
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate debug buffers");
		success = FALSE;
		debug_data.initialized = success;
	}

	return success;
}

void rasterizer_debug_begin(
	void)
{
	debug_data.opaque_triangle_count = 0;
	debug_data.opaque_line_count = 0;
	debug_data.non_opaque_primitive_count = 0;
	debug_data.primitive_count = 0;

	return;
}

void rasterizer_debug_end(
	void)
{
	return;
}

void rasterizer_debug_dispose(
	void)
{
	if (debug_data.initialized)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 137, debug_data.opaque_triangles);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 138, debug_data.opaque_lines);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 139, debug_data.non_opaque_primitives);

		match_free("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 141, debug_data.opaque_triangles);
		match_free("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 142, debug_data.opaque_lines);
		match_free("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 143, debug_data.non_opaque_primitives);
		debug_data.initialized = FALSE;
	}

	return;
}

void rasterizer_debug_line(
	real_point3d const *p0,
	real_point3d const *p1,
	real_argb_color const *color)
{
	rasterizer_debug_line_shaded(p0, p1, color, color);

	return;
}

void rasterizer_debug_line_shaded(
	real_point3d const *p0,
	real_point3d const *p1,
	real_argb_color const *color0,
	real_argb_color const *color1)
{
	if (debug_data.initialized && rasterizer_debug_options.draw_debug_geometry)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 171, p0 && p1 && color0 && color1);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 172, debug_data.opaque_triangles);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 173, debug_data.opaque_lines);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 174, debug_data.non_opaque_primitives);

		if (color0->alpha>0.f || color1->alpha>0.f)
		{
			boolean opaque = color0->alpha==1.f && color1->alpha==1.f;
			long primitive_index = rasterizer_debug_new_primitive(opaque ? &debug_data.opaque_line_count : &debug_data.non_opaque_primitive_count);

			if (primitive_index!=NONE)
			{
				struct debug_primitive *primitive = opaque ? &debug_data.opaque_lines[primitive_index] : &debug_data.non_opaque_primitives[primitive_index];
				real_vector3d v0;
				real_vector3d v1;

				vector_from_points3d(p0, &global_window_parameters.camera.position, &v0);
				vector_from_points3d(p1, &global_window_parameters.camera.position, &v1);
				primitive->vertex_count = 2;
				primitive->vertices[0].point = *p0;
				primitive->vertices[1].point = *p1;
				primitive->vertices[0].color = real_argb_color_to_pixel32(color0);
				primitive->vertices[1].color = real_argb_color_to_pixel32(color1);
				primitive->sort_distance = MIN(dot_product3d(&v0, &global_window_parameters.camera.forward), dot_product3d(&v1, &global_window_parameters.camera.forward));
				primitive->opaque = opaque;
			}
		}
	}

	return;
}

void rasterizer_debug_triangle(
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	real_argb_color const *color)
{
	rasterizer_debug_triangle_shaded(p0, p1, p2, color, color, color);

	return;
}

void rasterizer_debug_triangle_shaded(
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	real_argb_color const *color0,
	real_argb_color const *color1,
	real_argb_color const *color2)
{
	if (debug_data.initialized && rasterizer_debug_options.draw_debug_geometry)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 229, p0 && p1 && p2 && color0 && color1 && color2);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 230, debug_data.opaque_triangles);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 231, debug_data.opaque_lines);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 232, debug_data.non_opaque_primitives);

		if (color0->alpha>0.f || color1->alpha>0.f || color2->alpha>0.f)
		{
			boolean opaque = color0->alpha==1.f && color1->alpha==1.f && color2->alpha==1.f;
			long primitive_index = rasterizer_debug_new_primitive(opaque ? &debug_data.opaque_triangle_count : &debug_data.non_opaque_primitive_count);

			if (primitive_index!=NONE)
			{
				struct debug_primitive *primitive = opaque ? &debug_data.opaque_triangles[primitive_index] : &debug_data.non_opaque_primitives[primitive_index];
				real_vector3d v0;
				real_vector3d v1;
				real_vector3d v2;

				vector_from_points3d(p0, &global_window_parameters.camera.position, &v0);
				vector_from_points3d(p1, &global_window_parameters.camera.position, &v1);
				vector_from_points3d(p2, &global_window_parameters.camera.position, &v2);
				primitive->vertex_count = 3;
				primitive->vertices[0].point = *p0;
				primitive->vertices[1].point = *p1;
				primitive->vertices[2].point = *p2;
				primitive->vertices[0].color = real_argb_color_to_pixel32(color0);
				primitive->vertices[1].color = real_argb_color_to_pixel32(color1);
				primitive->vertices[2].color = real_argb_color_to_pixel32(color2);
				primitive->sort_distance = MIN(dot_product3d(&v0, &global_window_parameters.camera.forward), MIN(dot_product3d(&v1, &global_window_parameters.camera.forward), dot_product3d(&v2, &global_window_parameters.camera.forward)));
				primitive->opaque = opaque;
			}
		}
	}

	return;
}

void rasterizer_debug_test(
	void)
{
	return;
}

static long debug_primitive_compare( /* fake name */
	struct debug_primitive const *a,
	struct debug_primitive const *b)
{
	long result = 0;

	if (!a->opaque && !b->opaque)
	{
		if (a->sort_distance>b->sort_distance)
		{
			result = 1;
		}

		if (a->sort_distance<b->sort_distance)
		{
			result = -1;
		}
	}
	else
	{
		if (a->opaque)
		{
			result -= a->vertex_count;
		}

		if (b->opaque)
		{
			result += b->vertex_count;
		}
	}

	return result;
}

void rasterizer_debug_draw(
	void)
{
	boolean success = TRUE;

	if (debug_data.initialized && debug_data.primitive_count>0 && rasterizer_debug_options.draw_debug_geometry)
	{
		long primitive_index;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 320, debug_data.opaque_triangles);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 321, debug_data.opaque_lines);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 322, debug_data.non_opaque_primitives);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 323, debug_data.opaque_triangle_count <=RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 324, debug_data.opaque_line_count <=RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 325, debug_data.non_opaque_primitive_count<=RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_debug.c", 326, debug_data.primitive_count <=RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES);

		qsort(debug_data.non_opaque_primitives, debug_data.non_opaque_primitive_count, sizeof(struct debug_primitive), (int(__cdecl *)(const void *, const void *))debug_primitive_compare);

		rasterizer_globals.current_lock_operation = _rasterizer_lock_debug;

		if (success && debug_data.opaque_triangle_count>0)
		{
			long dynamic_vertex_buffer_index = rasterizer_dynamic_vertices_new(_rasterizer_vertex_type_debug, 3*debug_data.opaque_triangle_count);

			if (dynamic_vertex_buffer_index!=NONE)
			{
				struct debug_vertex *vertices = rasterizer_dynamic_vertices_lock(dynamic_vertex_buffer_index);

				if (vertices)
				{
					long primitive_count = debug_data.opaque_triangle_count;
					long vertex_index = 0;

					for (primitive_index = 0; primitive_index<primitive_count; primitive_index++)
					{
						struct debug_primitive *primitive = &debug_data.opaque_triangles[primitive_index];

						memcpy(&vertices[vertex_index], primitive, primitive->vertex_count*sizeof(struct debug_vertex));
						vertex_index += primitive->vertex_count;
					}
					rasterizer_dynamic_vertices_unlock(dynamic_vertex_buffer_index);
					rasterizer_debug_drawing_begin(TRUE, 0);
					rasterizer_draw_dynamic_vertices(0, primitive_count, dynamic_vertex_buffer_index, 3);
					rasterizer_debug_drawing_end();
				}
				else
				{
					error(_error_silent, "### ERROR failed to lock dynamic vertex buffers for debug primitives");
					success = FALSE;
				}
				rasterizer_dynamic_vertices_delete(dynamic_vertex_buffer_index);
			}
			else
			{
				success = FALSE;
			}
		}

		if (success && debug_data.opaque_line_count>0)
		{
			long dynamic_vertex_buffer_index = rasterizer_dynamic_vertices_new(_rasterizer_vertex_type_debug, 2*debug_data.opaque_line_count);

			if (dynamic_vertex_buffer_index!=NONE)
			{
				struct debug_vertex *vertices = rasterizer_dynamic_vertices_lock(dynamic_vertex_buffer_index);

				if (vertices)
				{
					long primitive_count = debug_data.opaque_line_count;
					long vertex_index = 0;

					for (primitive_index = 0; primitive_index<primitive_count; primitive_index++)
					{
						struct debug_primitive *primitive = &debug_data.opaque_lines[primitive_index];

						memcpy(&vertices[vertex_index], primitive, primitive->vertex_count*sizeof(struct debug_vertex));
						vertex_index += primitive->vertex_count;
					}
					rasterizer_dynamic_vertices_unlock(dynamic_vertex_buffer_index);
					rasterizer_debug_drawing_begin(TRUE, 16);
					rasterizer_draw_dynamic_vertices(0, primitive_count, dynamic_vertex_buffer_index, 2);
					rasterizer_debug_drawing_end();
				}
				else
				{
					error(_error_silent, "### ERROR failed to lock dynamic vertex buffers for debug primitives");
					success = FALSE;
				}
				rasterizer_dynamic_vertices_delete(dynamic_vertex_buffer_index);
			}
			else
			{
				success = FALSE;
			}
		}

		for (primitive_index = 0; success && primitive_index<debug_data.non_opaque_primitive_count; primitive_index++)
		{
			struct debug_primitive *primitive = &debug_data.non_opaque_primitives[primitive_index];
			long dynamic_vertex_buffer_index = rasterizer_dynamic_vertices_new(_rasterizer_vertex_type_debug, primitive->vertex_count);

			if (dynamic_vertex_buffer_index!=NONE)
			{
				struct debug_vertex *vertices = rasterizer_dynamic_vertices_lock(dynamic_vertex_buffer_index);

				if (vertices)
				{
					memcpy(vertices, primitive, primitive->vertex_count*sizeof(struct debug_vertex));
					rasterizer_dynamic_vertices_unlock(dynamic_vertex_buffer_index);
					rasterizer_debug_drawing_begin(FALSE, 0);
					rasterizer_draw_dynamic_vertices(0, 1, dynamic_vertex_buffer_index, primitive->vertex_count);
					rasterizer_debug_drawing_end();
				}
				else
				{
					error(_error_silent, "### ERROR failed to lock dynamic vertex buffers for debug primitives");
					success = FALSE;
				}
				rasterizer_dynamic_vertices_delete(dynamic_vertex_buffer_index);
			}
			else
			{
				success = FALSE;
			}
		}

		rasterizer_globals.current_lock_operation = _rasterizer_lock_none;
	}

	return;
}
