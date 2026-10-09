/*
RASTERIZER_LIGHTS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer.h"
#include "rasterizer_console_vars.h"
#include "rasterizer_widgets.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "bitmaps_inlines.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "rasterizer/xbox/rasterizer_xbox.h"
#include "rasterizer_geometry.h"
#include "main.h"
#include "widgets.h"
#include "scenario.h"
#include "structure_bsp_definitions.h"
#include "render_cameras.h"
#include "bitmaps.h"
#include "periodic_functions.h"

/* ---------- structures */

struct lens_flare_occlusion_test_results
{
	short light_identifier;
	byte data[MAXIMUM_LENS_FLARES_PER_LIGHT][MAXIMUM_WINDOWS];
};

/* ---------- prototypes */

static boolean screenshot_in_progress(void);
__inline static struct rasterizer_lens_flare_submit_parameters *lens_flare_parameters_get(short lens_flare_index);
static byte *lens_flare_occlusion_test_results_get(struct rasterizer_lens_flare_submit_parameters const *lens_flare_parameters);
static real lens_flare_evaluate_corona_rotation_function(short function, struct rasterizer_lens_flare_submit_parameters const *lens_flare_parameters);

/* ---------- globals */

byte local_lens_flare_occlusion_test_results2[MAXIMUM_LENS_FLARE_MARKERS_PER_STRUCTURE+MAXIMUM_QUEUED_LENS_FLARES][MAXIMUM_WINDOWS] = {0};
struct lens_flare_occlusion_test_results local_lens_flare_occlusion_test_results[MAXIMUM_LIGHTS_PER_MAP] = {0};
struct rasterizer_lens_flare_submit_parameters local_lens_flare_parameters[MAXIMUM_LENS_FLARES_PER_FRAME] = {0};
long local_lens_flare_count = 0;

/* ---------- public code */

static boolean screenshot_in_progress(
	void)
{
	return global_screenshot_count>1 || (global_screenshot_count==1 && global_screenshot_size>1);
}

__inline static struct rasterizer_lens_flare_submit_parameters *lens_flare_parameters_get(
	short lens_flare_index)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 67, lens_flare_index>=0 && lens_flare_index<local_lens_flare_count);

	return &local_lens_flare_parameters[lens_flare_index];
}

static byte *lens_flare_occlusion_test_results_get(
	struct rasterizer_lens_flare_submit_parameters const *lens_flare_parameters)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 76, lens_flare_parameters);

	if (lens_flare_parameters->light_index&FLAG(_lens_flare_parameters_light_index_structure_bit))
	{
		long window_index = lens_flare_parameters->compressed_window_index&_lens_flare_window_index_mask;
		long structure_lens_flare_index = ((lens_flare_parameters->light_index&_lens_flare_parameters_light_index_mask)<<16) | lens_flare_parameters->lens_flare_index;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 87, window_index>=0 && window_index<MAXIMUM_WINDOWS);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 88, structure_lens_flare_index>=0 && structure_lens_flare_index<(MAXIMUM_LENS_FLARE_MARKERS_PER_STRUCTURE+MAXIMUM_QUEUED_LENS_FLARES));

		return &local_lens_flare_occlusion_test_results2[structure_lens_flare_index][window_index];
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 94, lens_flare_parameters->light_index>=0 && lens_flare_parameters->light_index<MAXIMUM_LIGHTS_PER_MAP);

		return &local_lens_flare_occlusion_test_results[lens_flare_parameters->light_index].data[lens_flare_parameters->lens_flare_index][lens_flare_parameters->compressed_window_index&_lens_flare_window_index_mask];
	}

	return NULL;
}

static real lens_flare_evaluate_corona_rotation_function(
	short function,
	struct rasterizer_lens_flare_submit_parameters const *lens_flare_parameters)
{
	real_vector3d direction_vector;
	real_vector3d forward_vector;
	real_vector3d temp_vector;
	real dx = 1.f;
	real dy = 0.f;
	real result = 0.f;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 118, lens_flare_parameters);

	direction_vector = uncompress_int32_to_real_vector3d(lens_flare_parameters->compressed_direction);
	switch (function)
	{
	case _lens_flare_corona_rotation_function_none:
		break;
	case _lens_flare_corona_rotation_function_eye_in_light_space:
		cross_product3d(&direction_vector, &global_window_parameters.frustum.view_to_world.forward, &temp_vector);
		cross_product3d(&temp_vector, &direction_vector, &temp_vector);
		forward_vector = global_window_parameters.camera.forward;
		dy = dot_product3d(&forward_vector, &temp_vector);
		dx = -dot_product3d(&forward_vector, &direction_vector);
		break;
	case _lens_flare_corona_rotation_function_light_in_eye_space:
		negate_vector3d(&direction_vector, &forward_vector);
		dy = dot_product3d(&forward_vector, &global_window_parameters.frustum.view_to_world.forward);
		dx = -dot_product3d(&forward_vector, &global_window_parameters.frustum.view_to_world.up);
		break;
	case _lens_flare_corona_rotation_function_eye_to_light_in_light_space:
		cross_product3d(&direction_vector, &global_window_parameters.frustum.view_to_world.forward, &temp_vector);
		cross_product3d(&temp_vector, &direction_vector, &temp_vector);
		vector_from_points3d(&global_window_parameters.camera.position, &lens_flare_parameters->position, &forward_vector);
		dy = dot_product3d(&forward_vector, &temp_vector);
		dx = -dot_product3d(&forward_vector, &direction_vector);
		break;
	case _lens_flare_corona_rotation_function_eye_to_light_in_eye_space:
		vector_from_points3d(&global_window_parameters.camera.position, &lens_flare_parameters->position, &forward_vector);
		dy = dot_product3d(&forward_vector, &global_window_parameters.frustum.view_to_world.forward);
		dx = -dot_product3d(&forward_vector, &global_window_parameters.frustum.view_to_world.up);
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 151, FALSE, "### ERROR unsupported lens flare corona rotation function");
		break;
	}

	if (function!=_lens_flare_corona_rotation_function_none && dy!=0.f)
	{
		result = arctangent(dy, dx)/(2.f*_pi);
	}

	return result;
}

void rasterizer_lights_reset_for_new_map(
	void)
{
	memset(local_lens_flare_occlusion_test_results, 0, (MAXIMUM_LIGHTS_PER_MAP+1)*sizeof(*local_lens_flare_occlusion_test_results));
	memset(local_lens_flare_occlusion_test_results2, 0, sizeof(local_lens_flare_occlusion_test_results2));
	local_lens_flare_count = 0;

	return;
}

void rasterizer_lights_begin_for_new_frame(
	void)
{
	rasterizer_profile_begin(_rasterizer_profile_lens_flare_occlusion_query);

	if (rasterizer_debug_options.draw_lens_flares && !screenshot_in_progress())
	{
		short lens_flare_index;

		for (lens_flare_index = 0; lens_flare_index<local_lens_flare_count; lens_flare_index++)
		{
			struct rasterizer_lens_flare_submit_parameters *lens_flare_parameters = lens_flare_parameters_get(lens_flare_index);
			byte *occlusion = lens_flare_occlusion_test_results_get(lens_flare_parameters);
			byte new_occlusion;

			if (lens_flare_parameters->internal__occlusion_pixels>0)
			{
				long result = rasterizer_widget_get_occlusion_test_result(lens_flare_index);
				long occlusion_pixels = lens_flare_parameters->internal__occlusion_pixels;

				new_occlusion = (byte)MIN(UNSIGNED_CHAR_MAX, (UNSIGNED_CHAR_MAX*result + (occlusion_pixels>>1))/occlusion_pixels);
			}
			else
			{
				new_occlusion = 0;
			}

			if (!new_occlusion)
			{
				*occlusion = 0;
			}
			else if (new_occlusion>*occlusion)
			{
				*occlusion = (3*(*occlusion) + new_occlusion)/4;
			}
			else if (new_occlusion<*occlusion)
			{
				*occlusion = (*occlusion + new_occlusion)/2;
			}
		}

		local_lens_flare_count = 0;
	}

	rasterizer_profile_end(_rasterizer_profile_lens_flare_occlusion_query);

	return;
}

void rasterizer_lights_begin(
	void)
{
	rasterizer_lights.light_count = 0;

	return;
}

long rasterizer_light_submit(
	struct rasterizer_light_submit_parameters const *parameters)
{
	long light_index = NONE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 240, parameters);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 241, parameters->color.red >=0.0f && parameters->color.red <=1.0f);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 242, parameters->color.green>=0.0f && parameters->color.green<=1.0f);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 243, parameters->color.blue >=0.0f && parameters->color.blue <=1.0f);

	if (rasterizer_lights.light_count<MAXIMUM_LIGHTS_PER_WINDOW)
	{
		light_index = rasterizer_lights.light_count++;
		rasterizer_lights.lights[light_index] = *parameters;
		if (rasterizer_debug_options.statistics_mode==_rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.dynamic_light_count++;
		}
	}
	else
	{
		error(_error_silent, "### ERROR too many lights submitted to window");
	}

	return light_index;
}

void rasterizer_lens_flare_submit(
	struct rasterizer_lens_flare_submit_parameters const *parameters)
{
	static boolean warned = FALSE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 266, parameters);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 267, parameters->definition);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 268, (parameters->compressed_window_index&_lens_flare_window_index_mask)==global_window_parameters.window_index);

	if (rasterizer_debug_options.draw_lens_flares && !screenshot_in_progress() && global_window_parameters.rasterizer_target==_rasterizer_target_render_primary)
	{
		if (local_lens_flare_count<MAXIMUM_LENS_FLARES_PER_FRAME)
		{
			real_vector3d eye_to_corona_vector;
			real eye_to_corona_distance;

			vector_from_points3d(&global_window_parameters.camera.position, &parameters->position, &eye_to_corona_vector);
			eye_to_corona_distance = dot_product3d(&global_window_parameters.camera.forward, &eye_to_corona_vector);
			if ((parameters->definition->far_fade_distance==0.f || eye_to_corona_distance<parameters->definition->far_fade_distance) &&
				(parameters->compressed_light_color>>24)>0)
			{
				long lens_flare_index = local_lens_flare_count++;
				struct rasterizer_lens_flare_submit_parameters *lens_flare_parameters = lens_flare_parameters_get((short)lens_flare_index);

				memcpy(lens_flare_parameters, parameters, sizeof(*lens_flare_parameters));
				if (parameters->light_identifier==NONE)
				{
					if (parameters->light_index==NONE)
					{
						lens_flare_parameters->light_index = FLAG(_lens_flare_parameters_light_index_structure_bit);
						match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 310, parameters->lens_flare_index>=0 && parameters->lens_flare_index<MAXIMUM_QUEUED_LENS_FLARES);
					}
					else
					{
						long structure_lens_flare_index = (parameters->light_index<<16) | parameters->lens_flare_index;

						match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 321, structure_lens_flare_index>=0 && structure_lens_flare_index<MAXIMUM_LENS_FLARE_MARKERS_PER_STRUCTURE);
						lens_flare_parameters->lens_flare_index = (short)(structure_lens_flare_index+MAXIMUM_QUEUED_LENS_FLARES);
						lens_flare_parameters->light_index = (short)((structure_lens_flare_index>>16) | FLAG(_lens_flare_parameters_light_index_structure_bit));
					}
				}
				else
				{
					struct lens_flare_occlusion_test_results *occlusion_test_results = &local_lens_flare_occlusion_test_results[lens_flare_parameters->light_index];

					match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 338, parameters->light_index>=0 && parameters->light_index<MAXIMUM_LIGHTS_PER_MAP);
					if (parameters->light_identifier!=occlusion_test_results->light_identifier)
					{
						memset(occlusion_test_results->data, 0, sizeof(occlusion_test_results->data));
						occlusion_test_results->light_identifier = lens_flare_parameters->light_identifier;
					}
				}

				if (rasterizer_debug_options.statistics_mode==_rasterizer_statistics_mode_full)
				{
					rasterizer_frame_statistics.lens_flare_count++;
				}
			}
		}
		else if (!warned)
		{
			error(_error_silent, "### ERROR too many lens flares submitted to frame");
			warned = TRUE;
		}
	}

	return;
}

void rasterizer_lens_flare_submit_for_cluster(
	short cluster_index)
{
	if (rasterizer_debug_options.draw_lens_flares && !screenshot_in_progress())
	{
		struct structure_bsp *structure_bsp = global_structure_bsp_get();
		struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_index, struct structure_cluster);
		long marker_index;

		for (marker_index = 0; marker_index<cluster->lens_flare_marker_count; marker_index++)
		{
			long structure_lens_flare_marker_index = cluster->first_lens_flare_marker_index+marker_index;
			struct structure_lens_flare_marker *marker = TAG_BLOCK_GET_ELEMENT(&structure_bsp->lens_flare_markers, structure_lens_flare_marker_index, struct structure_lens_flare_marker);
			struct structure_lens_flare *lens_flare = TAG_BLOCK_GET_ELEMENT(&structure_bsp->lens_flares, marker->lens_flare_index, struct structure_lens_flare);
			struct rasterizer_lens_flare_submit_parameters parameters;

			{
				real_vector3d direction;
				real_vector3d up;

				set_real_vector3d(&direction, marker->i_direction/127.f, marker->j_direction/127.f, marker->k_direction/127.f);
				perpendicular3d(&direction, &up);
				normalize3d(&direction);
				normalize3d(&up);
				parameters.compressed_direction = compress_real_vector3d_to_int32_clamp(&direction);
				parameters.compressed_up = compress_real_vector3d_to_int32_clamp(&up);
				parameters.definition = lens_flare_definition_get(lens_flare->lens_flare.index);
				parameters.position = marker->position;
				parameters.compressed_light_color = 0xffffffff;
				parameters.compressed_light_scale = 0;
				parameters.light_identifier = NONE;
				parameters.light_index = (short)(structure_lens_flare_marker_index>>16);
				parameters.lens_flare_index = (short)structure_lens_flare_marker_index;
				parameters.compressed_window_index = (byte)render.window_index;
				rasterizer_lens_flare_submit(&parameters);
			}
		}
	}

	return;
}

void rasterizer_lights_end(
	void)
{
	return;
}

void rasterizer_lens_flares_submit_occlusion_tests(
	void)
{
	rasterizer_profile_begin(_rasterizer_profile_lens_flare_occlusion_submit);

	if (rasterizer_debug_options.draw_lens_flares && !screenshot_in_progress() && global_window_parameters.rasterizer_target==_rasterizer_target_render_primary && local_lens_flare_count>0)
	{
		short lens_flare_index;

		rasterizer_widget_begin(_widget_type_internal_occlusion_test, 1);
		for (lens_flare_index = 0; lens_flare_index<local_lens_flare_count; lens_flare_index++)
		{
			struct rasterizer_lens_flare_submit_parameters *lens_flare_parameters = lens_flare_parameters_get(lens_flare_index);
			struct lens_flare_definition *definition = lens_flare_parameters->definition;
			real_vector3d direction = uncompress_int32_to_real_vector3d(lens_flare_parameters->compressed_direction);

			if ((lens_flare_parameters->compressed_window_index&_lens_flare_window_index_mask)==global_window_parameters.window_index)
			{
				real_point3d point;
				real radius = definition->occlusion_radius;

				switch (definition->occlusion_offset_direction)
				{
				case _lens_flare_occlusion_offset_direction_toward_viewer:
					point_from_line3d(&lens_flare_parameters->position, &global_window_parameters.camera.forward, -definition->occlusion_radius, &point);
					break;
				case _lens_flare_occlusion_offset_direction_marker_forward:
					point_from_line3d(&lens_flare_parameters->position, &direction, definition->occlusion_radius*1.41421356f, &point);
					break;
				case _lens_flare_occlusion_offset_direction_none:
					point = lens_flare_parameters->position;
					break;
				default:
					match_vassert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 482, FALSE, "### ERROR unsupported lens flare occlusion offset direction");
				}

				lens_flare_parameters->internal__occlusion_pixels = rasterizer_widget_submit_occlusion_test(&point, radius, lens_flare_index);
			}
		}
		rasterizer_widget_end();
	}

	rasterizer_profile_end(_rasterizer_profile_lens_flare_occlusion_submit);

	return;
}

void rasterizer_lens_flares_draw(
	void)
{
	rasterizer_profile_begin(_rasterizer_profile_lens_flares);

	if (rasterizer_debug_options.draw_lens_flares && global_window_parameters.rasterizer_target==_rasterizer_target_render_primary && local_lens_flare_count>0)
	{
		short lens_flare_index;

		rasterizer_widget_begin(_widget_type_internal_sprite, 0);
		for (lens_flare_index = 0; lens_flare_index<local_lens_flare_count; lens_flare_index++)
		{
			struct rasterizer_lens_flare_submit_parameters *lens_flare_parameters = lens_flare_parameters_get(lens_flare_index);
			byte *occlusion_test_result = lens_flare_occlusion_test_results_get(lens_flare_parameters);
			real_vector3d direction = uncompress_int32_to_real_vector3d(lens_flare_parameters->compressed_direction);

			if ((lens_flare_parameters->compressed_window_index&_lens_flare_window_index_mask)==global_window_parameters.window_index)
			{
				struct lens_flare_definition *definition = lens_flare_parameters->definition;

				if (lens_flare_parameters->internal__occlusion_pixels>0 && (lens_flare_parameters->compressed_light_color>>24)>0 && definition->reflections.count>0)
				{
					real scale_functions[NUMBER_OF_LENS_FLARE_SCALE_FUNCTIONS];
					real_point3d corona_position;
					real intensity = 1.f;
					real_vector3d corona_axis;
					real_vector3d eye_to_corona_vector;
					real eye_to_corona_distance;
					real occlusion;
					real rotation;
					real angle;
					real falloff_scale;
					real falloff_offset;

					corona_position = lens_flare_parameters->position;
					vector_from_points3d(&global_window_parameters.camera.position, &corona_position, &eye_to_corona_vector);
					eye_to_corona_distance = dot_product3d(&global_window_parameters.camera.forward, &eye_to_corona_vector);
					scale_vector3d(&global_window_parameters.camera.forward, eye_to_corona_distance, &corona_axis);
					subtract_vectors3d(&corona_axis, &eye_to_corona_vector, &corona_axis);
					scale_vector3d(&corona_axis, 2.f, &corona_axis);
					occlusion = *occlusion_test_result/255.f;

					intensity = definition->far_fade_distance>0.f ? PIN((eye_to_corona_distance-definition->far_fade_distance)/(definition->near_fade_distance-definition->far_fade_distance), 0.f, 1.f) : 1.f;
					intensity *= occlusion;
					intensity *= uncompress_int8_to_real((byte)(lens_flare_parameters->compressed_light_color>>24));
					rotation = lens_flare_evaluate_corona_rotation_function(definition->corona_rotation_function, lens_flare_parameters)*definition->corona_rotation_function_scale;
					angle = arctangent(dot_product3d(&eye_to_corona_vector, &global_window_parameters.frustum.view_to_world.forward), dot_product3d(&eye_to_corona_vector, &global_window_parameters.frustum.view_to_world.left))/(_pi/180.f);
					falloff_scale = 1.f/(definition->runtime_cosine_falloff_angle-definition->runtime_cosine_cutoff_angle);
					falloff_offset = -falloff_scale*definition->runtime_cosine_cutoff_angle;
					normalize3d(&eye_to_corona_vector);

					scale_functions[_lens_flare_scale_function_none] = 1.f;
					scale_functions[_lens_flare_scale_function_non_local_incident_angle] = PIN(falloff_offset - dot_product3d(&direction, &global_window_parameters.camera.forward)*falloff_scale, 0.f, 1.f);
					scale_functions[_lens_flare_scale_function_local_incident_angle] = PIN(falloff_offset - dot_product3d(&direction, &eye_to_corona_vector)*falloff_scale, 0.f, 1.f);
					scale_functions[_lens_flare_scale_function_viewer_angle] = PIN(dot_product3d(&global_window_parameters.camera.forward, &eye_to_corona_vector)*falloff_scale + falloff_offset, 0.f, 1.f);

					if (intensity>0.f)
					{
						short reflection_index;
						real brightness_scale = uncompress_int8_to_real(lens_flare_parameters->compressed_light_scale);

						for (reflection_index = 0; reflection_index<definition->reflections.count; reflection_index++)
						{
							struct lens_flare_reflection *reflection = TAG_BLOCK_GET_ELEMENT(&definition->reflections, reflection_index, struct lens_flare_reflection);
							real brightness_lower_bound = reflection->brightness_lower_bounds;
							real brightness_upper_bound = reflection->brightness_upper_bounds;
							real brightness = (brightness_upper_bound-brightness_lower_bound)*brightness_scale + brightness_lower_bound;

							brightness *= scale_functions[reflection->brightness_scale_function];
							brightness *= intensity;

							if (reflection_index==0)
							{
								intensity = brightness;
							}

							if (brightness>0.f)
							{
								real_point3d point;
								real_vector2d reflection_scale;
								real reflection_rotation;
								real tint_factor;
								pixel32 color;
								real radius_lower_bound = reflection->radius_lower_bounds;
								real radius_upper_bound = reflection->radius_upper_bounds;
								real radius = (radius_upper_bound-radius_lower_bound)*brightness_scale + radius_lower_bound;

								if (reflection->tint_color.alpha==0.f && reflection->tint_color.red==0.f && reflection->tint_color.green==0.f && reflection->tint_color.blue==0.f)
								{
									color = (compress_real_to_int8(brightness)<<24) | (lens_flare_parameters->compressed_light_color&0x00ffffff);
									tint_factor = 1.f;
								}
								else
								{
									real_argb_color tint_color;

									tint_color.rgb = reflection->tint_color.rgb;
									tint_color.alpha = brightness;
									if (reflection->animation_function>_periodic_function_zero)
									{
										real_argb_color animation_color;
										real t;

										match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 651, reflection->animation_period!=0.0f);
										t = periodic_function_evaluate(reflection->animation_function, (global_frame_parameters.game_time_sec+reflection->animation_phase)/reflection->animation_period);
										rgb_colors_interpolate(
											&animation_color.rgb,
											reflection->animation_flags&(FLAG(_lens_flare_reflection_animation_color_interpolate_in_hsv_bit) | FLAG(_lens_flare_reflection_animation_color_interpolate_along_farthest_hue_path_bit)),
											&reflection->animation_color_lower_bound.rgb,
											&reflection->animation_color_upper_bound.rgb,
											t);
										scalars_interpolate(reflection->animation_color_lower_bound.alpha, reflection->animation_color_upper_bound.alpha, t, &animation_color.alpha);
										match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 667, animation_color.alpha>=0.0f && animation_color.alpha<=1.0f);
										match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 668, animation_color.red >=0.0f && animation_color.red <=1.0f);
										match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 669, animation_color.green>=0.0f && animation_color.green<=1.0f);
										match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 670, animation_color.blue >=0.0f && animation_color.blue <=1.0f);
										tint_color.alpha *= animation_color.alpha;
										tint_color.red *= animation_color.red;
										tint_color.green *= animation_color.green;
										tint_color.blue *= animation_color.blue;
									}
									color = real_argb_color_to_pixel32(&tint_color);
									tint_factor = reflection->tint_color.alpha;
								}

								if (reflection_index==0)
								{
									reflection_rotation = rotation+reflection->rotation_offset;
									reflection_scale = definition->corona_radius_scale;
								}
								else
								{
									reflection_rotation = reflection->rotation_offset;
									reflection_scale.i = reflection_scale.j = 1.f;
								}

								if (TEST_FLAG(reflection->flags, _lens_flare_reflection_rotate_from_center_of_screen_bit))
								{
									reflection_rotation += angle;
								}

								if (TEST_FLAG(reflection->flags, _lens_flare_reflection_radius_scaled_by_occlusion_bit))
								{
									radius = (occlusion+1.f)*radius*0.5f;
								}

								if (TEST_FLAG(reflection->flags, _lens_flare_reflection_radius_not_scaled_by_distance_bit))
								{
									radius *= eye_to_corona_distance;
								}

								point_from_line3d(&corona_position, &corona_axis, (reflection->offset), &point);
								if (rasterizer_widget_set_texture(0, definition->primary_map.index, reflection->bitmap_index))
								{
									break;
								}

								rasterizer_widget_set_tint_factor(tint_factor);
								if (TEST_FLAG(reflection->flags, _lens_flare_reflection_zbuffer_bit) && TEST_FLAG(lens_flare_parameters->compressed_window_index, _lens_flare_window_index_first_person_bit))
								{
									rasterizer_set_stencil_mode(_rasterizer_stencil_mode_reject);
								}
								else
								{
									rasterizer_set_stencil_mode(_rasterizer_stencil_mode_none);
								}
								rasterizer_widget_draw_sprite3d(&point, radius, &reflection_scale, reflection_rotation*(_pi/180.f), color);
							}
						}
					}
				}
			}
		}
		rasterizer_set_stencil_mode(_rasterizer_stencil_mode_none);
		rasterizer_widget_end();

		if (rasterizer_debug_options.lens_flare_sun_glow_enabled)
		{
			for (lens_flare_index = 0; lens_flare_index<local_lens_flare_count; lens_flare_index++)
			{
				struct rasterizer_lens_flare_submit_parameters *lens_flare_parameters = lens_flare_parameters_get(lens_flare_index);

				if (lens_flare_parameters->internal__occlusion_pixels>0 &&
					(lens_flare_parameters->compressed_window_index&_lens_flare_window_index_mask)==global_window_parameters.window_index &&
					(lens_flare_parameters->definition->occlusion_radius==50.f || TEST_FLAG(lens_flare_parameters->definition->flags, _lens_flare_sun_bit)))
				{
					rasterizer_sun_glow_draw(lens_flare_parameters);
				}
			}
		}
	}

	rasterizer_profile_end(_rasterizer_profile_lens_flares);

	return;
}
