/*
RASTERIZER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer.h"
#include "rasterizer_console_vars.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "bitmaps_inlines.h"
#include "game_state.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "rasterizer/xbox/rasterizer_xbox.h"
#include "shell.h"
#include "collisions.h"
#include "collision_usage.h"
#include "rasterizer_geometry.h"
#include "model_definitions.h"
#include "render_debug.h"

/* ---------- constants */

enum
{
	MAXIMUM_DEBUG_VERTICES = 2048, /* fake name */
	MAXIMUM_DEBUG_VERTEX_INDICES = 12 /* fake name */
};

/* ---------- structures */

struct debug_vertex_info /* fake name */
{
	real_point3d position;
	short triangle_indices[MAXIMUM_DEBUG_VERTEX_INDICES];
	short vertex_indices[MAXIMUM_DEBUG_VERTEX_INDICES];
	byte triangle_index_count;
	byte vertex_index_count;
};

/* ---------- globals */

static long rasterizer_debug_model_vertices_object_index; /* fake name */

struct rasterizer_globals_struct rasterizer_globals = {
	FALSE,					/* active */
	_rasterizer_lock_none,	/* current_lock_operation */
	{0},					/* screen_bounds */
	{0},					/* frame_bounds */
	{0},					/* __unknown14 */
	0,						/* frame_index */
	0,						/* flip_index */
	0,						/* vblank_index */
	0,						/* flip_vblank_index */
	768,					/* pushbuffer_size */
	0,						/* pushbuffer_kickoff_size */
	FALSE,					/* use_floating_point_zbuffer */
	TRUE,					/* use_rasterizer_frame_rate_throttle */
	FALSE,					/* use_rasterizer_frame_rate_stabilization */
	0,						/* refresh_rate */
	0.0625f,				/* z_near */
	1024.f,					/* z_far */
	0.01171875f,			/* z_near_first_person */
	1024.f					/* z_far_first_person */
};

struct rasterizer_debug_options_struct rasterizer_debug_options = {
	FALSE,								/* fps_accumulation */
	_rasterizer_statistics_mode_none,	/* statistics_mode */
	_rasterizer_drawing_mode_normal,	/* drawing_mode */
	FALSE,								/* wireframe_enabled */
	FALSE,								/* debug_model_vertices_enabled */
	NONE,								/* debug_model_lod */
	FALSE,								/* debug_transparent_geometry_enabled */
	FALSE,								/* debug_meter_shader_enabled */
	TRUE,								/* draw_models */
	TRUE,								/* draw_model_transparent_geometry */
	TRUE,								/* draw_first_person_weapon_first */
	TRUE,								/* stencil_mask_enabled */
	2,									/* draw_environment */
	TRUE,								/* draw_environment_lightmaps */
	TRUE,								/* draw_environment_shadows */
	TRUE,								/* draw_environment_diffuse_lights */
	TRUE,								/* draw_environment_textures */
	TRUE,								/* draw_environment_decals */
	TRUE,								/* draw_environment_specular_lights */
	TRUE,								/* draw_environment_specular_lightmaps */
	TRUE,								/* draw_environment_reflection_lightmap_masks */
	TRUE,								/* draw_environment_reflection_mirrors */
	TRUE,								/* draw_environment_reflections */
	TRUE,								/* draw_environment_transparent_geometry */
	TRUE,								/* draw_environment_fog */
	TRUE,								/* draw_environment_fog_screen */
	TRUE,								/* draw_water */
	TRUE,								/* draw_lens_flares */
	TRUE,								/* draw_dynamic_unlit_geometry */
	TRUE,								/* draw_dynamic_lit_geometry */
	TRUE,								/* draw_dynamic_screen_geometry */
	TRUE,								/* draw_hud_motion_sensor */
	TRUE,								/* draw_detail_objects */
	TRUE,								/* draw_debug_geometry */
	FALSE,								/* debug_geometry_multipass */
	TRUE,								/* fog_atmospheric_enabled */
	TRUE,								/* fog_planar_enabled */
	TRUE,								/* bump_mapping_enabled */
	1.f,								/* lightmap_ambient */
	0,									/* _lightmap_mode */
	0,									/* pad3 */
	TRUE,								/* lightmap_incident_radiosity_enabled */
	TRUE,								/* lightmap_filtering_enabled */
	0.f,								/* model_lighting_ambient */
	TRUE,								/* environment_alpha_testing_enabled */
	TRUE,								/* environment_specular_mask_enabled */
	TRUE,								/* shadow_convolution_enabled */
	FALSE,								/* shadow_debug_enabled */
	FALSE,								/* water_mipmapping_enabled */
	TRUE,								/* active_camouflage_enabled */
	TRUE,								/* active_camouflage_multipass_enabled */
	TRUE,								/* plasma_energy_enabled */
	TRUE,								/* lens_flare_occlusion_enabled */
	FALSE,								/* lens_flare_occlusion_debug */
	TRUE,								/* lens_flare_sun_glow_enabled */
	TRUE,								/* screen_flash_enabled */
	TRUE,								/* screen_effects_enabled */
	FALSE,								/* DXTC_noise_enabled */
	FALSE,								/* soft_filter_enabled */
	FALSE,								/* secondary_render_target_debug_enabled */
	FALSE,								/* profile_log_enabled */
	0.4f,								/* detail_object_screen_facing_offset_multiplier */
	8,									/* zbias */
	0.00390625f,						/* zoffset */
	FALSE,								/* force_all_player_views_to_default_player */
	FALSE,								/* safe_frame_bounds_adjust_enabled */
	0,									/* freeze_flying_camera */
	TRUE,								/* zsprite_enabled */
	TRUE,								/* filthy_decal_fog_hack_enabled */
	TRUE,								/* smart_states_enabled */
	FALSE,								/* splitscreen_VB_optimization_enabled */
	FALSE,								/* profile_print_locks */
	0.f,								/* profile_objectlock_time */
	1.f									/* pad3_scale */
};

struct rasterizer_global_defaults const rasterizer_global_defaults = {
	0.0625f,		/* z_near */
	1024.f,			/* z_far */
	0.01171875f,	/* z_near_first_person */
	1024.f			/* z_far_first_person */
};

real_argb_color *global_rasterizer_model_ambient_reflection_tint = NULL;

/* ---------- public code */

boolean rasterizer_initialize(
	void)
{
	global_rasterizer_model_ambient_reflection_tint = game_state_malloc("rasterizer model ambient reflection tint", NULL, sizeof(*global_rasterizer_model_ambient_reflection_tint));
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer.c", 289, global_rasterizer_model_ambient_reflection_tint);

	return _rasterizer_initialize();
}

void rasterizer_reset_state(
	void)
{
	_rasterizer_reset_state();

	return;
}

void rasterizer_frame_begin(
	struct rasterizer_frame_begin_parameters const *parameters)
{
	switch (rasterizer_debug_options.draw_environment)
	{
	case FALSE:
	case TRUE:
		rasterizer_debug_options.draw_environment_fog = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment_transparent_geometry = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment_reflections = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment_reflection_mirrors = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment_reflection_lightmap_masks = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment_specular_lightmaps = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment_specular_lights = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment_decals = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment_textures = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment_shadows = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment_diffuse_lights = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment_lightmaps = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment_fog_screen = rasterizer_debug_options.draw_environment;
		rasterizer_debug_options.draw_environment = 2;
		break;
	}

	if (rasterizer_globals.z_near==0.f)
	{
		rasterizer_globals.z_near = rasterizer_global_defaults.z_near;
	}

	if (rasterizer_globals.z_far==0.f)
	{
		rasterizer_globals.z_far = rasterizer_global_defaults.z_far;
	}

	if (rasterizer_globals.z_near_first_person==0.f)
	{
		rasterizer_globals.z_near_first_person = rasterizer_global_defaults.z_near_first_person;
	}

	if (rasterizer_globals.z_far_first_person==0.f)
	{
		rasterizer_globals.z_far_first_person = rasterizer_global_defaults.z_far_first_person;
	}

	_rasterizer_frame_begin(parameters);

	return;
}

boolean rasterizer_windows_begin(
	void)
{
	return _rasterizer_windows_begin();
}

void rasterizer_window_begin(
	struct rasterizer_window_begin_parameters const *parameters)
{
	_rasterizer_window_begin(parameters);

	return;
}

void rasterizer_window_get_fog(
	struct render_fog *fog)
{
	_rasterizer_window_get_fog(fog);

	return;
}

void rasterizer_window_set_fog(
	struct render_fog const *fog)
{
	_rasterizer_window_set_fog(fog);

	return;
}

void rasterizer_window_end(
	void)
{
	_rasterizer_window_end();

	return;
}

void rasterizer_windows_end(
	void)
{
	_rasterizer_windows_end();

	return;
}

void rasterizer_frame_end(
	void)
{
	_rasterizer_frame_end();

	return;
}

void rasterizer_present(
	struct bitmap_data *screenshot_bitmap,
	point2d const *screenshot_index)
{
	_rasterizer_present(screenshot_bitmap, screenshot_index);

	return;
}

void rasterizer_dispose(
	void)
{
	_rasterizer_dispose();

	return;
}

void rasterizer_set_vblank_callback(
	void (*callback)(unsigned long))
{
	_rasterizer_set_vblank_callback(callback);

	return;
}

void rasterizer_profile_enable(
	boolean enable)
{
	_rasterizer_profile_enable(enable);

	return;
}

long rasterizer_dynamic_triangles_new(
	long count)
{
	return _rasterizer_dynamic_triangles_new(count);
}

struct rasterizer_triangle *rasterizer_dynamic_triangles_lock(
	long dynamic_triangle_buffer_index)
{
	return _rasterizer_dynamic_triangles_lock(dynamic_triangle_buffer_index);
}

void rasterizer_dynamic_triangles_unlock(
	long dynamic_triangle_buffer_index)
{
	_rasterizer_dynamic_triangles_unlock(dynamic_triangle_buffer_index);

	return;
}

void rasterizer_dynamic_triangles_delete(
	long dynamic_triangle_buffer_index)
{
	_rasterizer_dynamic_triangles_delete(dynamic_triangle_buffer_index);

	return;
}

long rasterizer_dynamic_vertices_new(
	short type,
	long count)
{
	return _rasterizer_dynamic_vertices_new(type, count);
}

short rasterizer_dynamic_vertices_get_type(
	long dynamic_vertex_buffer_index)
{
	return _rasterizer_dynamic_vertices_get_type(dynamic_vertex_buffer_index);
}

void *rasterizer_dynamic_vertices_lock(
	long dynamic_vertex_buffer_index)
{
	return _rasterizer_dynamic_vertices_lock(dynamic_vertex_buffer_index);
}

void rasterizer_dynamic_vertices_unlock(
	long dynamic_vertex_buffer_index)
{
	_rasterizer_dynamic_vertices_unlock(dynamic_vertex_buffer_index);

	return;
}

void rasterizer_dynamic_vertices_delete(
	long dynamic_vertex_buffer_index)
{
	_rasterizer_dynamic_vertices_delete(dynamic_vertex_buffer_index);

	return;
}

void rasterizer_debug_immediate_begin(
	void)
{
	_rasterizer_debug_immediate_begin();

	return;
}

void rasterizer_debug_immediate_point(
	real_point3d const *p,
	real size,
	real_rgb_color const *color)
{
	real_point3d p0;
	real_point3d p1;

	size *= 0.5f;

	set_real_point3d(&p0, p->x-size, p->y, p->z);
	set_real_point3d(&p1, p->x+size, p->y, p->z);
	_rasterizer_debug_immediate_line(&p0, &p1, color, color);

	set_real_point3d(&p0, p->x, p->y-size, p->z);
	set_real_point3d(&p1, p->x, p->y+size, p->z);
	_rasterizer_debug_immediate_line(&p0, &p1, color, color);

	set_real_point3d(&p0, p->x, p->y, p->z-size);
	set_real_point3d(&p1, p->x, p->y, p->z+size);
	_rasterizer_debug_immediate_line(&p0, &p1, color, color);

	return;
}

void rasterizer_debug_immediate_vector(
	real_point3d const *p,
	real_vector3d const *v,
	real size,
	real_rgb_color const *color)
{
	real_point3d q;

	point_from_line3d(p, v, size, &q);
	_rasterizer_debug_immediate_line(p, &q, color, color);

	return;
}

void rasterizer_debug_immediate_line(
	real_point3d const *p0,
	real_point3d const *p1,
	real_rgb_color const *color0,
	real_rgb_color const *color1)
{
	_rasterizer_debug_immediate_line(p0, p1, color0, color1);

	return;
}

void rasterizer_debug_immediate_triangle(
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	real_rgb_color const *color0,
	real_rgb_color const *color1,
	real_rgb_color const *color2)
{
	_rasterizer_debug_immediate_triangle(p0, p1, p2, color0, color1, color2);

	return;
}

void rasterizer_debug_immediate_end(
	void)
{
	_rasterizer_debug_immediate_end();

	return;
}

void rasterizer_debug_immediate_begin_screenspace(
	void)
{
	_rasterizer_debug_immediate_begin_screenspace();

	return;
}

void rasterizer_debug_immediate_line_screenspace(
	point2d const *p0,
	point2d const *p1,
	real_rgb_color const *color0,
	real_rgb_color const *color1)
{
	_rasterizer_debug_immediate_line_screenspace(p0, p1, color0, color1);

	return;
}

void rasterizer_debug_immediate_linestrip_screenspace(
	point2d const *points,
	short point_count,
	real_rgb_color const *color)
{
	_rasterizer_debug_immediate_linestrip_screenspace(points, point_count, color);

	return;
}

void rasterizer_debug_immediate_end_screenspace(
	void)
{
	_rasterizer_debug_immediate_end_screenspace();

	return;
}

void rasterizer_decals_initialize(
	void)
{
	_rasterizer_decals_initialize();

	return;
}

void rasterizer_decals_update_function_pointers(
	void)
{
	_rasterizer_decals_update_function_pointers();

	return;
}

void rasterizer_decals_initialize_for_new_map(
	void)
{
	_rasterizer_decals_initialize_for_new_map();

	return;
}

void rasterizer_decals_dispose_from_old_map(
	void)
{
	_rasterizer_decals_dispose_from_old_map();

	return;
}

void rasterizer_decals_flush(
	void)
{
	_rasterizer_decals_flush();

	return;
}

void rasterizer_decals_dispose(
	void)
{
	_rasterizer_decals_dispose();

	return;
}

long rasterizer_decal_vertices_new(
	long cache_size)
{
	return _rasterizer_decal_vertices_new(cache_size);
}

void *rasterizer_decal_vertices_lock(
	long cache_index,
	long cache_size)
{
	return _rasterizer_decal_vertices_lock(cache_index, cache_size);
}

void rasterizer_decal_vertices_unlock(
	void)
{
	_rasterizer_decal_vertices_unlock();

	return;
}

void rasterizer_decal_vertices_delete(
	long cache_index)
{
	_rasterizer_decal_vertices_delete(cache_index);

	return;
}

void rasterizer_decals_begin(
	short layer)
{
	_rasterizer_decals_begin(layer);

	return;
}

void rasterizer_decals_draw(
	short cluster_index)
{
	_rasterizer_decals_draw(cluster_index);

	return;
}

void rasterizer_decals_end(
	void)
{
	_rasterizer_decals_end();

	return;
}

void rasterizer_detail_objects_begin(
	void)
{
	_rasterizer_detail_objects_begin();

	return;
}

void rasterizer_detail_objects_rebuild_vertices(
	struct detail_object_view_data const *detail_object_view_data)
{
	_rasterizer_detail_objects_rebuild_vertices(detail_object_view_data);

	return;
}

void rasterizer_detail_objects_draw(
	struct detail_object_view_data const *detail_object_view_data)
{
	_rasterizer_detail_objects_draw(detail_object_view_data);

	return;
}

void rasterizer_detail_objects_end(
	void)
{
	_rasterizer_detail_objects_end();

	return;
}

void rasterizer_screen_effect(
	struct rasterizer_screen_effect_parameters const *parameters)
{
	_rasterizer_screen_effect(parameters);

	return;
}

void rasterizer_screen_flash(
	void)
{
	_rasterizer_screen_flash();

	return;
}

void rasterizer_models_begin(
	boolean sky)
{
	if (!sky)
	{
		struct collision_result collision;
		real_vector3d vector;

		match_collision_log_begin_user("c:\\halo\\SOURCE\\rasterizer\\rasterizer.c", 740, _collision_user_debugging);
		scale_vector3d(&global_window_parameters.camera.forward, 10000.f, &vector);
		if (collision_test_vector(FLAG(_collision_test_objects_bit) | _collision_test_objects_all_types_flags, &global_window_parameters.camera.position, &vector, render.local_player_index, &collision))
		{
			rasterizer_debug_model_vertices_object_index = collision.object_index;
		}
		match_collision_log_end_user("c:\\halo\\SOURCE\\rasterizer\\rasterizer.c", 756);
	}
	else
	{
		rasterizer_debug_model_vertices_object_index = NONE;
	}

	_rasterizer_models_begin(sky);

	return;
}

void rasterizer_model_begin(
	struct rasterizer_model_begin_parameters const *parameters,
	boolean do_not_change_z_stencil_states)
{
	_rasterizer_model_begin(parameters, do_not_change_z_stencil_states);

	return;
}

void rasterizer_model_draw(
	struct shader const *shader,
	short shader_permutation_index,
	struct triangle_buffer const *triangle_buffer,
	long dynamic_triangle_buffer_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer,
	long dynamic_vertex_buffer_index)
{
	_rasterizer_model_draw(shader, shader_permutation_index, triangle_buffer, dynamic_triangle_buffer_index, triangle_count, vertex_buffer, dynamic_vertex_buffer_index);

	return;
}

void rasterizer_model_transparent_geometry_submit(
	struct shader const *shader,
	short shader_permutation_index,
	struct triangle_buffer const *triangle_buffer,
	long dynamic_triangle_buffer_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer,
	long dynamic_vertex_buffer_index,
	real_point3d const *centroid,
	struct render_sort_filth *sort_filth)
{
	_rasterizer_model_transparent_geometry_submit(shader, shader_permutation_index, triangle_buffer, dynamic_triangle_buffer_index, triangle_count, vertex_buffer, dynamic_vertex_buffer_index, centroid, sort_filth);

	return;
}

void rasterizer_model_end(
	void)
{
	_rasterizer_model_end();

	return;
}

void rasterizer_models_end(
	void)
{
	_rasterizer_models_end();

	return;
}

void rasterizer_debug_model_vertices(
	long target_object_index,
	struct render_skinning const *skinning,
	struct model_geometry_part const *part)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer.c", 830, part);

	if (rasterizer_debug_options.debug_model_vertices_enabled && target_object_index==rasterizer_debug_model_vertices_object_index)
	{
		struct debug_vertex_info debug_vertex_info[MAXIMUM_DEBUG_VERTICES];
		long debug_vertex_count = 0;
		long closest_vertex_index = NONE;
		real closest_vertex_distance;
		long vertex_index;
		short strip_index;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer.c", 857, part->triangle_buffer.type==_triangle_buffer_type_precompiled_strip);

		for (strip_index = 0; strip_index<part->triangle_buffer.count+2; strip_index++)
		{
			word model_vertex_index = ((word *)part->triangles.address)[strip_index];
			struct model_vertex_compressed const *vertex = (struct model_vertex_compressed *)part->compressed_vertices.address + model_vertex_index;
			real_point3d position;
			real_vector3d normal;

			{
				short node_index0 = (char)vertex->nodes[0]/3;
				real node_weight0 = (real)vertex->weights[0]/32767.f;
				real_point3d p0 = {0.f, 0.f, 0.f};
				real_vector3d n0 = {0.f, 0.f, 0.f};
				short node_index1 = (char)vertex->nodes[1]/3;
				real node_weight1 = 1.f-node_weight0;
				real_point3d p1 = {0.f, 0.f, 0.f};
				real_vector3d n1 = {0.f, 0.f, 0.f};
				real_vector3d n = uncompress_int32_to_real_vector3d(vertex->normal);

				match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer.c", 878, node_index0<skinning->node_matrix_count);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer.c", 879, node_index1<skinning->node_matrix_count);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer.c", 880, node_weight0>=0.0f && node_weight0<=1.0f);

				if (node_index0>=0)
				{
					matrix4x3_transform_point(&skinning->node_matrices[node_index0], &vertex->position, &p0);
					matrix4x3_transform_vector(&skinning->node_matrices[node_index0], &n, &n0);
				}

				if (node_index1>=0)
				{
					matrix4x3_transform_point(&skinning->node_matrices[node_index1], &vertex->position, &p1);
					matrix4x3_transform_vector(&skinning->node_matrices[node_index1], &n, &n1);
				}

				set_real_point3d(&position, node_weight0*p0.x + node_weight1*p1.x, node_weight0*p0.y + node_weight1*p1.y, node_weight0*p0.z + node_weight1*p1.z);
				set_real_vector3d(&normal, node_weight0*n0.i + node_weight1*n1.i, node_weight0*n0.j + node_weight1*n1.j, node_weight0*n0.k + node_weight1*n1.k);
				normalize3d(&normal);
			}

			for (vertex_index = 0; vertex_index<debug_vertex_count; vertex_index++)
			{
				struct debug_vertex_info *info = &debug_vertex_info[vertex_index];
				short index;

				if (position.x==info->position.x && position.y==info->position.y && position.z==info->position.z)
				{
					if (info->triangle_index_count<MAXIMUM_DEBUG_VERTEX_INDICES)
					{
						for (index = 0; index<info->triangle_index_count; index++)
						{
							if (info->triangle_indices[index]==strip_index)
							{
								break;
							}
						}

						if (index==info->triangle_index_count)
						{
							info->triangle_indices[info->triangle_index_count] = strip_index;
							info->triangle_index_count++;
						}
					}

					if (info->vertex_index_count<MAXIMUM_DEBUG_VERTEX_INDICES)
					{
						for (index = 0; index<info->vertex_index_count; index++)
						{
							if (info->vertex_indices[index]==model_vertex_index)
							{
								break;
							}
						}

						if (index==info->vertex_index_count)
						{
							info->vertex_indices[info->vertex_index_count] = model_vertex_index;
							info->vertex_index_count++;
						}
					}
					break;
				}
			}

			if (vertex_index==debug_vertex_count && debug_vertex_count<MAXIMUM_DEBUG_VERTICES)
			{
				struct debug_vertex_info *info = &debug_vertex_info[debug_vertex_count];
				real_vector3d vector;
				real distance;

				info->position = position;
				info->triangle_indices[0] = strip_index;
				info->vertex_indices[0] = model_vertex_index;
				info->triangle_index_count = 1;
				info->vertex_index_count = 1;

				vector_from_points3d(&global_window_parameters.camera.position, &position, &vector);
				normalize3d(&vector);
				distance = dot_product3d(&global_window_parameters.camera.forward, &vector);
				if ((dot_product3d(&normal, &vector)<0.f && closest_vertex_distance<distance) || closest_vertex_distance==-1.f)
				{
					closest_vertex_index = vertex_index;
					closest_vertex_distance = distance;
				}
				debug_vertex_count++;
			}
		}

		for (vertex_index = 0; vertex_index<debug_vertex_count; vertex_index++)
		{
			struct debug_vertex_info *info = &debug_vertex_info[vertex_index];

			if (vertex_index==closest_vertex_index)
			{
				short index;

				strcpy(temporary, "I=");
				for (index = 0; index<info->triangle_index_count; index++)
				{
					char temp2[256];

					sprintf(temp2, "%d%c", info->triangle_indices[index], index==info->triangle_index_count-1 ? ' ' : ',');
					strcat(temporary, temp2);
				}
				strcat(temporary, "\nV=");
				for (index = 0; index<info->vertex_index_count; index++)
				{
					char temp2[256];

					sprintf(temp2, "%d%c", info->vertex_indices[index], index==info->vertex_index_count-1 ? ' ' : ',');
					strcat(temporary, temp2);
				}
				render_debug_point(FALSE, &info->position, 0.03125f, global_real_argb_red);
				render_debug_string_at_point(FALSE, &info->position, temporary, global_real_argb_yellow);
			}
			else
			{
				render_debug_point(FALSE, &info->position, 0.03125f, global_real_argb_white);
			}
		}
	}

	return;
}

void rasterizer_environment_lightmaps_begin(
	void)
{
	_rasterizer_environment_lightmaps_begin();

	return;
}

void rasterizer_environment_lightmap_begin(
	struct bitmap_data const *lightmap)
{
	_rasterizer_environment_lightmap_begin(lightmap);

	return;
}

void rasterizer_environment_lightmap_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffers)
{
	_rasterizer_environment_lightmap_draw(shader, shader_permutation_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffers);

	return;
}

void rasterizer_environment_lightmap_end(
	void)
{
	_rasterizer_environment_lightmap_end();

	return;
}

void rasterizer_environment_lightmaps_end(
	void)
{
	_rasterizer_environment_lightmaps_end();

	return;
}

void rasterizer_environment_diffuse_lights_begin(
	void)
{
	_rasterizer_environment_diffuse_lights_begin();

	return;
}

void rasterizer_environment_diffuse_light_begin(
	long light_index)
{
	_rasterizer_environment_diffuse_light_begin(light_index);

	return;
}

void rasterizer_environment_diffuse_light_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_diffuse_light_draw(shader, shader_permutation_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);

	return;
}

void rasterizer_environment_diffuse_light_end(
	void)
{
	_rasterizer_environment_diffuse_light_end();

	return;
}

void rasterizer_environment_diffuse_lights_end(
	void)
{
	_rasterizer_environment_diffuse_lights_end();

	return;
}

void rasterizer_environment_shadows_begin(
	void)
{
	_rasterizer_environment_shadows_begin();

	return;
}

boolean rasterizer_environment_shadow_begin(
	long object_index,
	real_matrix4x3 const *shadow_matrix,
	real_rgb_color const *light_color,
	real object_bounding_radius,
	real *shadow_volume_bounding_radius)
{
	return _rasterizer_environment_shadow_begin(object_index, shadow_matrix, light_color, object_bounding_radius, shadow_volume_bounding_radius);
}

void rasterizer_environment_shadow_model_begin(
	struct rasterizer_model_begin_parameters const *parameters)
{
	_rasterizer_environment_shadow_model_begin(parameters);

	return;
}

void rasterizer_environment_shadow_model_draw(
	struct shader const *shader,
	short shader_permutation_index,
	struct triangle_buffer const *triangle_buffer,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_shadow_model_draw(shader, shader_permutation_index, triangle_buffer, vertex_buffer);

	return;
}

void rasterizer_environment_shadow_model_end(
	void)
{
	_rasterizer_environment_shadow_model_end();

	return;
}

void rasterizer_environment_shadow_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_shadow_draw(shader, shader_permutation_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);

	return;
}

void rasterizer_environment_shadow_end(
	void)
{
	_rasterizer_environment_shadow_end();

	return;
}

void rasterizer_environment_shadows_end(
	void)
{
	_rasterizer_environment_shadows_end();

	return;
}

void rasterizer_environment_diffuse_textures_begin(
	void)
{
	_rasterizer_environment_diffuse_textures_begin();

	return;
}

void rasterizer_environment_diffuse_texture_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_diffuse_texture_draw(shader, shader_permutation_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);

	return;
}

void rasterizer_environment_diffuse_textures_end(
	void)
{
	_rasterizer_environment_diffuse_textures_end();

	return;
}

void rasterizer_environment_specular_lights_begin(
	void)
{
	_rasterizer_environment_specular_lights_begin();

	return;
}

void rasterizer_environment_specular_light_begin(
	long light_index)
{
	_rasterizer_environment_specular_light_begin(light_index);

	return;
}

void rasterizer_environment_specular_light_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_specular_light_draw(shader, shader_permutation_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);

	return;
}

void rasterizer_environment_specular_light_end(
	void)
{
	_rasterizer_environment_specular_light_end();

	return;
}

void rasterizer_environment_specular_lights_end(
	void)
{
	_rasterizer_environment_specular_lights_end();

	return;
}

void rasterizer_environment_specular_lightmaps_begin(
	void)
{
	_rasterizer_environment_specular_lightmaps_begin();

	return;
}

void rasterizer_environment_specular_lightmap_begin(
	struct bitmap_data const *lightmap)
{
	_rasterizer_environment_specular_lightmap_begin(lightmap);

	return;
}

void rasterizer_environment_specular_lightmap_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_specular_lightmap_draw(shader, shader_permutation_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);

	return;
}

void rasterizer_environment_specular_lightmap_end(
	void)
{
	_rasterizer_environment_specular_lightmap_end();

	return;
}

void rasterizer_environment_specular_lightmaps_end(
	void)
{
	_rasterizer_environment_specular_lightmaps_end();

	return;
}

void rasterizer_environment_reflection_lightmap_masks_begin(
	void)
{
	_rasterizer_environment_reflection_lightmap_masks_begin();

	return;
}

void rasterizer_environment_reflection_lightmap_mask_begin(
	struct bitmap_data const *lightmap)
{
	_rasterizer_environment_reflection_lightmap_mask_begin(lightmap);

	return;
}

void rasterizer_environment_reflection_lightmap_mask_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_reflection_lightmap_mask_draw(shader, shader_permutation_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);

	return;
}

void rasterizer_environment_reflection_lightmap_mask_end(
	void)
{
	_rasterizer_environment_reflection_lightmap_mask_end();

	return;
}

void rasterizer_environment_reflection_lightmap_masks_end(
	void)
{
	_rasterizer_environment_reflection_lightmap_masks_end();

	return;
}

void rasterizer_environment_reflection_mirrors_begin(
	void)
{
	_rasterizer_environment_reflection_mirrors_begin();

	return;
}

void rasterizer_environment_reflection_mirror_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_reflection_mirror_draw(shader, shader_permutation_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);

	return;
}

void rasterizer_environment_reflection_mirrors_end(
	void)
{
	_rasterizer_environment_reflection_mirrors_end();

	return;
}

void rasterizer_environment_reflections_begin(
	void)
{
	_rasterizer_environment_reflections_begin();

	return;
}

void rasterizer_environment_reflection_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_reflection_draw(shader, shader_permutation_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);

	return;
}

void rasterizer_environment_reflections_end(
	void)
{
	_rasterizer_environment_reflections_end();

	return;
}

void rasterizer_environment_transparent_geometry_begin(
	void)
{
	_rasterizer_environment_transparent_geometry_begin();

	return;
}

void rasterizer_environment_transparent_geometry_submit(
	struct shader const *shader,
	short shader_permutation_index,
	struct bitmap_data const *lightmap,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffers,
	real_point3d const *centroid,
	real_plane3d const *plane,
	real_vector3d const *vector,
	struct render_lighting const *lighting,
	unsigned long geometry_flags)
{
	_rasterizer_environment_transparent_geometry_submit(shader, shader_permutation_index, lightmap, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffers, centroid, plane, vector, lighting, geometry_flags);

	return;
}

void rasterizer_environment_transparent_geometry_end(
	void)
{
	_rasterizer_environment_transparent_geometry_end();

	return;
}

void rasterizer_environment_fog_begin(
	void)
{
	_rasterizer_environment_fog_begin();

	return;
}

void rasterizer_environment_fog_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_fog_draw(shader, shader_permutation_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);

	return;
}

void rasterizer_environment_fog_end(
	void)
{
	_rasterizer_environment_fog_end();

	return;
}

void rasterizer_environment_fog_screen_wind_get_vector(
	short window_index,
	real dt,
	real_vector3d *wind_vector)
{
	_rasterizer_environment_fog_screen_wind_get_vector(window_index, dt, wind_vector);

	return;
}

void rasterizer_environment_fog_screen_begin(
	short pass)
{
	_rasterizer_environment_fog_screen_begin(pass);

	return;
}

void rasterizer_environment_fog_screen_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_fog_screen_draw(shader, shader_permutation_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);

	return;
}

void rasterizer_environment_fog_screen_end(
	void)
{
	_rasterizer_environment_fog_screen_end();

	return;
}

void rasterizer_hud_begin(
	void)
{
	_rasterizer_hud_begin();

	return;
}

void rasterizer_hud_end(
	void)
{
	_rasterizer_hud_end();

	return;
}

void rasterizer_dynamic_unlit_geometry_draw(
	struct shader const *shader,
	struct bitmap_data const *primary_map,
	struct render_animation const *animation,
	long dynamic_triangle_buffer_index,
	long dynamic_vertex_buffer_index,
	long triangle_count,
	real_point3d const *centroid,
	unsigned long geometry_flags)
{
	_rasterizer_dynamic_unlit_geometry_draw(shader, primary_map, animation, dynamic_triangle_buffer_index, dynamic_vertex_buffer_index, triangle_count, centroid, geometry_flags);

	return;
}

void rasterizer_dynamic_lit_geometry_draw(
	struct shader const *shader,
	struct bitmap_data const *primary_map,
	struct render_animation const *animation,
	struct render_lighting const *lighting,
	long dynamic_triangle_buffer_index,
	long dynamic_vertex_buffer_index,
	long triangle_count,
	real_point3d const *centroid,
	unsigned long geometry_flags)
{
	_rasterizer_dynamic_lit_geometry_draw(shader, primary_map, animation, lighting, dynamic_triangle_buffer_index, dynamic_vertex_buffer_index, triangle_count, centroid, geometry_flags);

	return;
}

void rasterizer_dynamic_screen_geometry_draw(
	struct rasterizer_dynamic_screen_geometry_parameters const *parameters,
	long dynamic_triangle_buffer_index,
	long dynamic_vertex_buffer_index,
	long triangle_count)
{
	_rasterizer_dynamic_screen_geometry_draw(parameters, dynamic_triangle_buffer_index, dynamic_vertex_buffer_index, triangle_count);

	return;
}

void rasterizer_dynamic_screen_geometry_add_multitexture_params_to_base(
	struct rasterizer_dynamic_screen_geometry_parameters *base,
	struct rasterizer_dynamic_screen_geometry_parameters const *multitext_params)
{
	_rasterizer_dynamic_screen_geometry_add_multitexture_params_to_base(base, multitext_params);

	return;
}

void rasterizer_psuedo_dynamic_screen_quad_draw(
	struct rasterizer_dynamic_screen_geometry_parameters const *parameters,
	struct dynamic_screen_vertex *verts)
{
	_rasterizer_psuedo_dynamic_screen_quad_draw(parameters, verts);

	return;
}

void rasterizer_widget_submit(
	long object_index,
	long widget_index,
	real_point3d const *centroid,
	void (*render_proc)(long, long))
{
	_rasterizer_widget_submit(object_index, widget_index, centroid, render_proc);

	return;
}

void rasterizer_widget_begin(
	short type,
	word flags)
{
	_rasterizer_widget_begin(type, flags);

	return;
}

boolean rasterizer_widget_set_texture(
	short stage_index,
	long bitmap_group_index,
	short sequence_index)
{
	return _rasterizer_widget_set_texture(stage_index, bitmap_group_index, sequence_index);
}

void rasterizer_widget_set_tint_factor(
	real tint_factor)
{
	_rasterizer_widget_set_tint_factor(tint_factor);

	return;
}

void rasterizer_widget_set_zbuffer_enable(
	boolean zbuffer_enable)
{
	_rasterizer_widget_set_zbuffer_enable(zbuffer_enable);

	return;
}

void rasterizer_widget_draw_sprite2d(
	real_point2d const *point,
	real radius,
	real_vector2d const *scale,
	real_vector2d const *texture_size,
	real rotation,
	pixel32 color)
{
	_rasterizer_widget_draw_sprite2d(point, radius, scale, texture_size, rotation, color);

	return;
}

void rasterizer_widget_draw_sprite3d(
	real_point3d const *point,
	real radius,
	real_vector2d const *scale,
	real rotation,
	pixel32 color)
{
	_rasterizer_widget_draw_sprite3d(point, radius, scale, rotation, color);

	return;
}

void rasterizer_widget_end(
	void)
{
	_rasterizer_widget_end();

	return;
}

long rasterizer_widget_submit_occlusion_test(
	real_point3d const *point,
	real radius,
	long index)
{
	return _rasterizer_widget_submit_occlusion_test(point, radius, index);
}

long rasterizer_widget_get_occlusion_test_result(
	long index)
{
	return _rasterizer_widget_get_occlusion_test_result(index);
}

void rasterizer_hud_motion_sensor_blip_begin(
	void)
{
	_rasterizer_hud_motion_sensor_blip_begin();

	return;
}

void rasterizer_hud_motion_sensor_blip_draw(
	real_point2d const *blip_position,
	real fade,
	real radius,
	real_rgb_color const *blip_color,
	boolean custom)
{
	_rasterizer_hud_motion_sensor_blip_draw(blip_position, fade, radius, blip_color, custom);

	return;
}

void rasterizer_hud_motion_sensor_blip_end(
	real_point2d const *center_point,
	real theta)
{
	_rasterizer_hud_motion_sensor_blip_end(center_point, theta);

	return;
}
