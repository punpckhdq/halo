/*
RASTERIZER_FRAME_STATISTICS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer.h"
#include "rasterizer_console_vars.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "input.h"
#include "bitmaps_inlines.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "draw_string.h"
#include "rasterizer/xbox/rasterizer_xbox.h"
#include "main.h"
#include "players.h"
#include "interface.h"

/* ---------- constants */

enum
{
	RASTERIZER_FRAME_STATISTICS_FPS_SAMPLE_COUNT = 60,
	NUMBER_OF_PROFILE_LOG_FRAMES = 16, /* fake name */
};

/* ---------- structures */

struct rasterizer_memory_statistic /* fake name */
{
	char const *name;
	long size;
	long unused_size;
};

/* ---------- prototypes */

static boolean eat_my_shorts(short a, short b);

/* ---------- globals */

char *profile_log_path = "d:\\r-prof.txt";

static real profile_log_samples[NUMBER_OF_RASTERIZER_PROFILES][NUMBER_OF_PROFILE_LOG_FRAMES] = {0}; /* fake name */
static word *rasterizer_frame_statistics_temp_buffer = NULL;
static __int64 rasterizer_fps_accumulation_time = 0;
static __int64 rasterizer_fps_accumulation_frame_index = 0;
static FILE *local_profile_log = NULL;
static unsigned long time_samples[RASTERIZER_FRAME_STATISTICS_FPS_SAMPLE_COUNT] = {0};
static short fps_sample_count = 0;
static short profile_log_frame_index = 0; /* fake name */
static real profile_log_total_time = 0.f; /* fake name */
static long profile_log_total_pushbuffer = 0; /* fake name */

/* ---------- public code */

boolean rasterizer_frame_statistics_initialize(
	void)
{
	boolean success = TRUE;

	rasterizer_frame_statistics_temp_buffer = match_malloc("c:\\halo\\SOURCE\\rasterizer\\rasterizer_frame_statistics.c", 41, RASTERIZER_MAXIMUM_TRIANGLES_PER_TRIANGLE_BUFFER*sizeof(struct rasterizer_triangle));

	if (!rasterizer_frame_statistics_temp_buffer)
	{
		error(_error_silent, "### ERROR out of memory");
		success = FALSE;
	}

	return success;
}

void rasterizer_frame_statistics_begin(
	void)
{
	memset(&rasterizer_frame_statistics, 0, sizeof(rasterizer_frame_statistics));

	return;
}

void rasterizer_frame_statistics_get_fps(
	struct rasterizer_frame_statistics_s *frame_statistics)
{
	if (rasterizer_debug_options.statistics_mode && frame_statistics)
	{
		unsigned long time = system_milliseconds();

		if (fps_sample_count)
		{
			short sample_index;
			unsigned long minimum_delta = time - time_samples[0];
			unsigned long maximum_delta = time - time_samples[0];

			for (sample_index = fps_sample_count - 1; sample_index>0; sample_index--)
			{
				if (sample_index>1)
				{
					minimum_delta = MIN(time_samples[sample_index - 1] - time_samples[sample_index], minimum_delta);
					maximum_delta = MAX(time_samples[sample_index - 1] - time_samples[sample_index], maximum_delta);
				}

				time_samples[sample_index] = time_samples[sample_index - 1];
			}

			frame_statistics->fps = 1000.f / (real)MAX(time - time_samples[0], 1);
			frame_statistics->fps_sample_count = fps_sample_count;
			frame_statistics->fps_average = (real)fps_sample_count * 1000.f / (real)MAX(time - time_samples[fps_sample_count - 1], 1);
			frame_statistics->fps_max = 1000.f / (real)MAX(minimum_delta, 1);
			frame_statistics->fps_min = 1000.f / (real)MAX(maximum_delta, 1);
		}

		time_samples[0] = time;
		fps_sample_count = MIN(fps_sample_count + 1, RASTERIZER_FRAME_STATISTICS_FPS_SAMPLE_COUNT);
	}
	else
	{
		fps_sample_count = 0;
	}

	return;
}

void rasterizer_fps_accumulate(
	void)
{
	rasterizer_debug_options.fps_accumulation = TRUE;
	rasterizer_fps_accumulation_time = system_milliseconds();
	rasterizer_fps_accumulation_frame_index = rasterizer_globals.frame_index;

	return;
}

static boolean eat_my_shorts(
	short a,
	short b)
{
	if ((word)a>(word)b)
	{
		return TRUE;
	}
	else if ((word)a<(word)b)
	{
		return FALSE;
	}
	else
	{
		return FALSE;
	}
}

long rasterizer_frame_statistics_count_static_vertices(
	struct triangle_buffer const *triangle_buffer,
	struct vertex_buffer const *vertex_buffer)
{
	long vertex_count = 0;

	if (triangle_buffer && vertex_buffer)
	{
		if (triangle_buffer->type==_triangle_buffer_type_precompiled_strip)
		{
			vertex_count = triangle_buffer->count + 2;
		}
		else if (triangle_buffer->type==_triangle_buffer_type_triangles)
		{
			if (vertex_buffer)
			{
				vertex_count = vertex_buffer->count;
			}
			else
			{
				long index_index;
				long index_count = 3 * triangle_buffer->count;
				word last_index = NONE;

				match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_frame_statistics.c", 177, triangle_buffer->count<RASTERIZER_MAXIMUM_TRIANGLES_PER_TRIANGLE_BUFFER);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_frame_statistics.c", 178, triangle_buffer->offset);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_frame_statistics.c", 179, rasterizer_frame_statistics_temp_buffer);

				memcpy(rasterizer_frame_statistics_temp_buffer, (void *)triangle_buffer->offset, triangle_buffer->count * sizeof(struct rasterizer_triangle));
				qsort_2byte(rasterizer_frame_statistics_temp_buffer, index_count, eat_my_shorts);

				for (index_index = 0; index_index<index_count; index_index++)
				{
					word index = rasterizer_frame_statistics_temp_buffer[index_index];

					if (last_index!=index)
					{
						last_index = index;
						vertex_count++;
					}
				}
			}
		}
	}

	return vertex_count;
}

long rasterizer_frame_statistics_count_dynamic_vertices(
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count)
{
	long vertex_count = 0;

	if (dynamic_triangle_buffer_index>=0)
	{
		struct rasterizer_triangle *triangles = rasterizer_dynamic_triangles_lock(dynamic_triangle_buffer_index);

		if (triangles)
		{
			long index_index;
			long index_count = 3 * triangle_count;
			word last_index = NONE;

			match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_frame_statistics.c", 217, triangle_count<RASTERIZER_MAXIMUM_TRIANGLES_PER_TRIANGLE_BUFFER);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_frame_statistics.c", 218, rasterizer_frame_statistics_temp_buffer);

			memcpy(rasterizer_frame_statistics_temp_buffer, triangles + first_triangle_index, triangle_count * sizeof(struct rasterizer_triangle));
			qsort_2byte(rasterizer_frame_statistics_temp_buffer, index_count, eat_my_shorts);

			for (index_index = 0; index_index<index_count; index_index++)
			{
				word index = rasterizer_frame_statistics_temp_buffer[index_index];

				if (last_index!=index)
				{
					last_index = index;
					vertex_count++;
				}
			}

			rasterizer_dynamic_triangles_unlock(dynamic_triangle_buffer_index);
		}
	}
	else
	{
		short vertex_type = (short)-dynamic_triangle_buffer_index;

		if (vertex_type==3 || vertex_type==4)
		{
			vertex_count = triangle_count / (vertex_type - 2);
		}
		else
		{
			vertex_count = vertex_type;
		}
	}

	return vertex_count;
}

void rasterizer_frame_statistics_draw(
	void)
{
	if (rasterizer_debug_options.statistics_mode!=_rasterizer_statistics_mode_none)
	{
		char frame_statistics_string[0x3000];
		rectangle2d bounds;
		short tab_stops[] = { 100, 200, 300, 400, 500, 600 };
		short tab_stop_count = NUMBEROF(tab_stops);
		point2d cursor = { 0, 0 };
		real_argb_color statistics_color_plain = { 1.f, 0.66f, 1.f, 0.66f };
		real_argb_color statistics_color_bold = { 1.f, 1.f, 1.f, 1.f };
		real_argb_color white = { 1.f, 1.f, 1.f, 1.f };
		short height_adjust = -4;
		short line_adjust = -1;
		short x0 = rasterizer_globals.frame_bounds.x0;
		long environment_vertex_count =
			rasterizer_frame_statistics.environment_lightmap_vertex_count +
			rasterizer_frame_statistics.environment_shadow_vertex_count +
			rasterizer_frame_statistics.environment_light_vertex_count +
			rasterizer_frame_statistics.environment_texture_vertex_count +
			rasterizer_frame_statistics.environment_specular_light_vertex_count +
			rasterizer_frame_statistics.environment_specular_lightmap_vertex_count +
			rasterizer_frame_statistics.environment_reflection_lightmap_mask_vertex_count +
			rasterizer_frame_statistics.environment_reflection_vertex_count +
			rasterizer_frame_statistics.environment_transparent_vertex_count +
			rasterizer_frame_statistics.environment_fog_vertex_count;
		long environment_triangle_count =
			rasterizer_frame_statistics.environment_lightmap_triangle_count +
			rasterizer_frame_statistics.environment_shadow_triangle_count +
			rasterizer_frame_statistics.environment_light_triangle_count +
			rasterizer_frame_statistics.environment_texture_triangle_count +
			rasterizer_frame_statistics.environment_specular_light_triangle_count +
			rasterizer_frame_statistics.environment_specular_lightmap_triangle_count +
			rasterizer_frame_statistics.environment_reflection_lightmap_mask_triangle_count +
			rasterizer_frame_statistics.environment_reflection_triangle_count +
			rasterizer_frame_statistics.environment_transparent_triangle_count +
			rasterizer_frame_statistics.environment_fog_triangle_count;
		long environment_primitive_count =
			rasterizer_frame_statistics.environment_lightmap_primitive_count +
			rasterizer_frame_statistics.environment_shadow_primitive_count +
			rasterizer_frame_statistics.environment_light_primitive_count +
			rasterizer_frame_statistics.environment_texture_primitive_count +
			rasterizer_frame_statistics.environment_specular_light_primitive_count +
			rasterizer_frame_statistics.environment_specular_lightmap_primitive_count +
			rasterizer_frame_statistics.environment_reflection_lightmap_mask_primitive_count +
			rasterizer_frame_statistics.environment_reflection_primitive_count +
			rasterizer_frame_statistics.environment_transparent_primitive_count +
			rasterizer_frame_statistics.environment_fog_primitive_count;
		long model_vertex_count = rasterizer_frame_statistics.model_vertex_count + rasterizer_frame_statistics.model_transparent_vertex_count;
		long model_triangle_count = rasterizer_frame_statistics.model_triangle_count + rasterizer_frame_statistics.model_transparent_triangle_count;
		long model_primitive_count = rasterizer_frame_statistics.model_primitive_count + rasterizer_frame_statistics.model_transparent_primitive_count;
		long total_vertex_count = environment_vertex_count + model_vertex_count + rasterizer_frame_statistics.model_shadow_vertex_count;
		long total_triangle_count = environment_triangle_count + model_triangle_count + rasterizer_frame_statistics.model_shadow_triangle_count;
		long total_primitive_count = environment_primitive_count + model_primitive_count + rasterizer_frame_statistics.model_shadow_primitive_count;
		short tab_stop_index;

		for (tab_stop_index = 0; tab_stop_index<tab_stop_count; tab_stop_index++)
		{
			tab_stops[tab_stop_index] += x0;
		}

		bounds = rasterizer_globals.frame_bounds;
		offset_rectangle2d(&bounds, 0, 32);
		interface_set_bitmap_text_draw_mode(1, NONE, 0, 0, 5, 0);

		sprintf(frame_statistics_string, "|n|tframerate|taverage (of %d)|tmin|tmax", rasterizer_frame_statistics.fps_sample_count);
		tab_stops[0] = x0;
		draw_string_set_tab_stops(tab_stops, tab_stop_count);
		draw_string_set_color(&statistics_color_bold);
		rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
		bounds.y0 = cursor.y + line_adjust;

		if (rasterizer_debug_options.fps_accumulation)
		{
			__int64 frame_count = rasterizer_globals.frame_index - rasterizer_fps_accumulation_frame_index;
			__int64 milliseconds = system_milliseconds() - rasterizer_fps_accumulation_time;

			sprintf(
				frame_statistics_string,
				"|t%.0f|t%.0f/%.0f|t%.0f|t%.0f|n",
				rasterizer_frame_statistics.fps,
				rasterizer_frame_statistics.fps_average,
				(real)frame_count * 1000.f / (real)milliseconds,
				rasterizer_frame_statistics.fps_min,
				rasterizer_frame_statistics.fps_max);
		}
		else
		{
			sprintf(
				frame_statistics_string,
				"|t%.0f|t%.0f|t%.0f|t%.0f|n",
				rasterizer_frame_statistics.fps,
				rasterizer_frame_statistics.fps_average,
				rasterizer_frame_statistics.fps_min,
				rasterizer_frame_statistics.fps_max);
		}

		tab_stops[0] = x0;
		draw_string_set_tab_stops(tab_stops, tab_stop_count);
		draw_string_set_color(&statistics_color_plain);
		rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
		bounds.y0 = cursor.y + line_adjust;

		if (rasterizer_debug_options.statistics_mode==_rasterizer_statistics_mode_fps_only)
		{
			tab_stops[0] = x0;
			draw_string_set_tab_stops(tab_stops, tab_stop_count);
			draw_string_set_color(&statistics_color_plain);
			sprintf(
				frame_statistics_string,
				"|tfogged|t%d|n|tnormal|t%d|n|tfast|t%d|n|tscenery|t%d|n",
				rasterizer_frame_statistics.model_vertex_counts[0],
				rasterizer_frame_statistics.model_vertex_counts[1],
				rasterizer_frame_statistics.model_vertex_counts[2],
				rasterizer_frame_statistics.model_vertex_counts[3]);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;

			sprintf(
				frame_statistics_string,
				"|tskinning|t%d|n|tlighting|t%d|n|tvertex shaders|t%d|n",
				rasterizer_frame_statistics.model_skinning_bytes,
				rasterizer_frame_statistics.model_lighting_bytes,
				rasterizer_frame_statistics.model_vertex_shader_bytes);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;

			sprintf(
				frame_statistics_string,
				"|tlocal_player_count|t%d|n|tmain_get_window_count|t%d|n",
				local_player_count(),
				main_get_window_count());
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;
		}
		else if (rasterizer_debug_options.statistics_mode==_rasterizer_statistics_mode_full)
		{
			sprintf(frame_statistics_string, "|t|tvertices|ttriangles|tprimitives");
			tab_stops[0] = x0;
			draw_string_set_tab_stops(tab_stops, tab_stop_count);
			draw_string_set_color(&statistics_color_bold);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;

			sprintf(frame_statistics_string, "|ttotal|t%d|t%d|t%d|n", total_vertex_count, total_triangle_count, total_primitive_count);
			tab_stops[0] = x0;
			draw_string_set_tab_stops(tab_stops, tab_stop_count);
			draw_string_set_color(&statistics_color_plain);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;

			sprintf(frame_statistics_string, "|tenvironment|t%d|t%d|t%d", environment_vertex_count, environment_triangle_count, environment_primitive_count);
			tab_stops[0] = x0 + 25;
			draw_string_set_tab_stops(tab_stops, tab_stop_count);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;

			sprintf(
				frame_statistics_string,
				"|tlightmaps|t%d|t%d|t%d|n"
				"|tshadows (%d)|t%d|t%d|t%d|n"
				"|tlights|t%d|t%d|t%d|n"
				"|ttextures|t%d|t%d|t%d|n"
				"|tlights specular|t%d|t%d|t%d|n"
				"|tlightmaps specular|t%d|t%d|t%d|n"
				"|tlightmaps ref.mask|t%d|t%d|t%d|n"
				"|treflections|t%d|t%d|t%d|n"
				"|ttransparent|t%d|t%d/%d|t%d|n"
				"|tfog|t%d|t%d|t%d|n",
				rasterizer_frame_statistics.environment_lightmap_vertex_count,
				rasterizer_frame_statistics.environment_lightmap_triangle_count,
				rasterizer_frame_statistics.environment_lightmap_primitive_count,
				rasterizer_frame_statistics.environment_shadow_count,
				rasterizer_frame_statistics.environment_shadow_vertex_count,
				rasterizer_frame_statistics.environment_shadow_triangle_count,
				rasterizer_frame_statistics.environment_shadow_primitive_count,
				rasterizer_frame_statistics.environment_light_vertex_count,
				rasterizer_frame_statistics.environment_light_triangle_count,
				rasterizer_frame_statistics.environment_light_primitive_count,
				rasterizer_frame_statistics.environment_texture_vertex_count,
				rasterizer_frame_statistics.environment_texture_triangle_count,
				rasterizer_frame_statistics.environment_texture_primitive_count,
				rasterizer_frame_statistics.environment_specular_light_vertex_count,
				rasterizer_frame_statistics.environment_specular_light_triangle_count,
				rasterizer_frame_statistics.environment_specular_light_primitive_count,
				rasterizer_frame_statistics.environment_specular_lightmap_vertex_count,
				rasterizer_frame_statistics.environment_specular_lightmap_triangle_count,
				rasterizer_frame_statistics.environment_specular_lightmap_primitive_count,
				rasterizer_frame_statistics.environment_reflection_lightmap_mask_vertex_count,
				rasterizer_frame_statistics.environment_reflection_lightmap_mask_triangle_count,
				rasterizer_frame_statistics.environment_reflection_lightmap_mask_primitive_count,
				rasterizer_frame_statistics.environment_reflection_vertex_count,
				rasterizer_frame_statistics.environment_reflection_triangle_count,
				rasterizer_frame_statistics.environment_reflection_primitive_count,
				rasterizer_frame_statistics.environment_transparent_vertex_count,
				rasterizer_frame_statistics.environment_transparent_triangle_count,
				rasterizer_frame_statistics.environment_transparent_maximum_primitive_triangle_count,
				rasterizer_frame_statistics.environment_transparent_primitive_count,
				rasterizer_frame_statistics.environment_fog_vertex_count,
				rasterizer_frame_statistics.environment_fog_triangle_count,
				rasterizer_frame_statistics.environment_fog_primitive_count);
			tab_stops[0] = x0 + 50;
			draw_string_set_tab_stops(tab_stops, tab_stop_count);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;

			sprintf(
				frame_statistics_string,
				"|tmodel shadows (%d)|t%d|t%d|t%d",
				rasterizer_frame_statistics.model_shadow_count,
				rasterizer_frame_statistics.model_shadow_vertex_count,
				rasterizer_frame_statistics.model_shadow_triangle_count,
				rasterizer_frame_statistics.model_shadow_primitive_count);
			tab_stops[0] = x0 + 25;
			draw_string_set_tab_stops(tab_stops, tab_stop_count);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;

			sprintf(frame_statistics_string, "|tmodels (%d)|t%d|t%d|t%d", rasterizer_frame_statistics.model_count, model_vertex_count, model_triangle_count, model_primitive_count);
			tab_stops[0] = x0 + 25;
			draw_string_set_tab_stops(tab_stops, tab_stop_count);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;

			sprintf(
				frame_statistics_string,
				"|tsolid|t%d|t%d|t%d|n|ttransparent|t%d|t%d/%d|t%d|n",
				rasterizer_frame_statistics.model_vertex_count,
				rasterizer_frame_statistics.model_triangle_count,
				rasterizer_frame_statistics.model_primitive_count,
				rasterizer_frame_statistics.model_transparent_vertex_count,
				rasterizer_frame_statistics.model_transparent_triangle_count,
				rasterizer_frame_statistics.model_transparent_maximum_primitive_triangle_count,
				rasterizer_frame_statistics.model_transparent_primitive_count);
			tab_stops[0] = x0 + 50;
			draw_string_set_tab_stops(tab_stops, tab_stop_count);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;

			sprintf(
				frame_statistics_string,
				"|tdecals|t%d|t%d|t%d|n",
				rasterizer_frame_statistics.decal_vertex_count,
				rasterizer_frame_statistics.decal_triangle_count,
				rasterizer_frame_statistics.decal_primitive_count);
			tab_stops[0] = x0;
			draw_string_set_tab_stops(tab_stops, tab_stop_count);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;

			sprintf(
				frame_statistics_string,
				"|tdynamic geometry|t%d/%d|t%d/%d|n",
				rasterizer_frame_statistics.dynamic_vertex_count,
				rasterizer_frame_statistics.dynamic_vertex_buffer_count,
				rasterizer_frame_statistics.dynamic_triangle_count,
				rasterizer_frame_statistics.dynamic_triangle_buffer_count);
			tab_stops[0] = x0;
			draw_string_set_tab_stops(tab_stops, tab_stop_count);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;

			sprintf(
				frame_statistics_string,
				"|t%d dynamic lights|n|t%d lens flares|n",
				rasterizer_frame_statistics.dynamic_light_count,
				rasterizer_frame_statistics.lens_flare_count);
			tab_stops[0] = x0;
			draw_string_set_tab_stops(tab_stops, tab_stop_count);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;
		}
		else if (rasterizer_debug_options.statistics_mode==_rasterizer_statistics_mode_profile)
		{
			short profile;

			tab_stops[0] = x0;
			tab_stops[1] = x0 + 200;
			tab_stops[2] = x0 + 300;
			tab_stops[3] = 600;
			sprintf(frame_statistics_string, "|tGPU profile|ttime (msecs)|tdata (bytes)");
			draw_string_set_tab_stops(tab_stops, 4);
			draw_string_set_color(&statistics_color_bold);
			cursor.y -= 30;
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;
			draw_string_set_color(&statistics_color_plain);

			for (profile = 0; profile<NUMBER_OF_RASTERIZER_PROFILES; profile++)
			{
				real time = rasterizer_profile_query(profile);

				if (time>=0.f)
				{
					sprintf(frame_statistics_string, "|t%s|t%.2f|t%d", rasterizer_profile_get_string(profile), time * 1000.f, rasterizer_profile_query_pushbuffer(profile));
				}
				else
				{
					sprintf(frame_statistics_string, "|t%s|t----|t0", rasterizer_profile_get_string(profile));
				}

				rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
				bounds.y0 = cursor.y + line_adjust;
			}

			sprintf(frame_statistics_string, "|ttotal|t%.2f|t%d|n", rasterizer_profile_query(NUMBER_OF_RASTERIZER_PROFILES) * 1000.f, rasterizer_profile_query_pushbuffer(NUMBER_OF_RASTERIZER_PROFILES));
			draw_string_set_color(global_real_argb_yellow);
			bounds.y0 += 4;
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;
		}
		else if (rasterizer_debug_options.statistics_mode==_rasterizer_statistics_mode_memory)
		{
			short statistic_index;
			long total_size = 0;
			long total_used_size = 0;
			struct rasterizer_memory_statistic statistics[] =
			{
				{ "memory pool", 98304, 0 },
				{ "dynamic vertices (unlit)", 196608, 0 },
				{ "dynamic vertices (lit*)", 72, 307200 },
				{ "dynamic vertices (screen)", 327680, 0 },
				{ "dynamic vertices (model)", 65536, 0 },
				{ "dynamic vertices (detail objects)", 131072, 0 },
				{ "dynamic triangles", 196608, 0 },
				{ "transparent geometry groups", 61440, 0 },
				{ "bump map palette", 1024, 0 },
				{ "mirror buffers (includes z-buffer*)", 614400, 307200 },
				{ "shadow buffers", 65536, 0 },
				{ "sun glow buffers*", 32768, 32768 },
				{ "water buffers", 65536, 0 },
				{ "motion sensor buffers*", 81920, 81920 },
				{ "debug geometry*", 1769472, 1769472 },
				{ "vertex shaders|t~35k last i checked", 35840, 12288 },
			};
			MEMORYSTATUS memory_status;

			tab_stops[0] = x0;
			tab_stops[1] = x0 + 300;
			tab_stops[2] = 600;
			sprintf(frame_statistics_string, "|tallocation|tmemory usage (bytes)");
			draw_string_set_tab_stops(tab_stops, 3);
			draw_string_set_color(&statistics_color_bold);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;
			draw_string_set_color(&statistics_color_plain);

			for (statistic_index = 0; statistic_index<NUMBEROF(statistics); statistic_index++)
			{
				sprintf(frame_statistics_string, "|t%s|t%d", statistics[statistic_index].name, statistics[statistic_index].size);
				rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
				bounds.y0 = cursor.y + line_adjust;
				total_size += statistics[statistic_index].size;
				total_used_size += statistics[statistic_index].size - statistics[statistic_index].unused_size;
			}

			sprintf(frame_statistics_string, "|n|ttotal|t%d (%d)", total_size, total_used_size);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;

			GlobalMemoryStatus(&memory_status);
			draw_string_set_color(&statistics_color_bold);
			sprintf(frame_statistics_string, "|n|tsystem total|t%dKb", memory_status.dwTotalPhys >> 10);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;
			sprintf(frame_statistics_string, "|tsystem available|t%dKb", memory_status.dwAvailPhys >> 10);
			rasterizer_draw_string(&bounds, NULL, &cursor, height_adjust, frame_statistics_string);
			bounds.y0 = cursor.y + line_adjust;
			draw_string_set_color(&statistics_color_plain);
		}

		draw_string_set_tab_stops(NULL, 0);
		draw_string_set_color(&white);
	}

	if (rasterizer_debug_options.profile_log_enabled)
	{
		if (!local_profile_log)
		{
			local_profile_log = fopen(profile_log_path, "w");

			if (!local_profile_log)
			{
				error(_error_silent, "### ERROR failed to open rasterizer profile log (%s)", profile_log_path);
				rasterizer_debug_options.profile_log_enabled = FALSE;
			}

			profile_log_total_time = 0.f;
			profile_log_total_pushbuffer = 0;
		}

		if (local_profile_log)
		{
			short profile;
			short index;

			for (profile = 0; profile<NUMBER_OF_RASTERIZER_PROFILES; profile++)
			{
				profile_log_samples[profile][profile_log_frame_index] = rasterizer_profile_query(profile) * 1000.f;
			}

			profile_log_total_time += rasterizer_profile_query(NUMBER_OF_RASTERIZER_PROFILES) * 1000.f;
			profile_log_total_pushbuffer += rasterizer_profile_query_pushbuffer(NUMBER_OF_RASTERIZER_PROFILES);
			profile_log_frame_index++;

			if (profile_log_frame_index==NUMBER_OF_PROFILE_LOG_FRAMES)
			{
				fprintf(local_profile_log, "\n");

				for (profile = 0; profile<NUMBER_OF_RASTERIZER_PROFILES; profile++)
				{
					fprintf(local_profile_log, "%s", rasterizer_profile_get_string(profile));

					for (index = strlen(rasterizer_profile_get_string(profile)); index<32; index++)
					{
						fprintf(local_profile_log, ".");
					}

					for (index = 0; index<NUMBER_OF_PROFILE_LOG_FRAMES; index++)
					{
						if (profile_log_samples[profile][index]>=0.f)
						{
							fprintf(local_profile_log, "%6.2f", profile_log_samples[profile][index]);
						}
						else
						{
							fprintf(local_profile_log, "  ----");
						}
					}

					fprintf(local_profile_log, "\n");
				}

				fprintf(local_profile_log, "average total frame time= %.2f msecs\n", profile_log_total_time / NUMBER_OF_PROFILE_LOG_FRAMES);
				fprintf(local_profile_log, "average total pushbuffer= %d bytes\n", (profile_log_total_pushbuffer + NUMBER_OF_PROFILE_LOG_FRAMES / 2) / NUMBER_OF_PROFILE_LOG_FRAMES);
				fflush(local_profile_log);
				profile_log_frame_index = 0;
				profile_log_total_time = 0.f;
				profile_log_total_pushbuffer = 0;
			}
		}
	}
	else if (local_profile_log)
	{
		fclose(local_profile_log);
		local_profile_log = NULL;
	}

	return;
}

void rasterizer_frame_statistics_end(
	void)
{
	return;
}

void rasterizer_frame_statistics_dispose(
	void)
{
	if (rasterizer_frame_statistics_temp_buffer)
	{
		match_free("c:\\halo\\SOURCE\\rasterizer\\rasterizer_frame_statistics.c", 837, rasterizer_frame_statistics_temp_buffer);
	}

	return;
}
