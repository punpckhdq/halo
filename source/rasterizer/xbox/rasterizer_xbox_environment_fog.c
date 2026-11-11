/*
RASTERIZER_XBOX_ENVIRONMENT_FOG.C
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
#include "fog_definitions.h"
#include "bitmap_group.h"

/* ---------- constants */

enum
{
	MAXIMUM_FOG_SCREEN_LAYERS = 4 /* fake name */
};

/* ---------- structures */

struct fog_screen_wind /* fake name */
{
	real_vector2d direction;
	real velocity;
	real_vector2d target_direction;
	real target_velocity;
	real target_time;
	real target_period;
};

struct fog_screen_window /* fake name */
{
	short base_layer_index;
	real rotation;
	real offset;
	real_point2d layer_offsets[MAXIMUM_FOG_SCREEN_LAYERS];
	struct fog_screen_wind wind;
};

/* ---------- prototypes */

static boolean rasterizer_environment_fog_screen_is_active(void);
static void rasterizer_environment_fog_screen_wind_update(struct fog_screen const *screen, struct fog_screen_wind *wind);

/* ---------- globals */

static short local_model_node_matrix_count = 0; /* fake name */
static real_matrix4x3 const *local_model_node_matrices = NULL; /* fake name */
static real_matrix4x3 local_world_to_view[MAXIMUM_WINDOWS] = {0}; /* fake name */
static boolean local_model_multipass = FALSE; /* fake name */
static boolean local_environment_multipass = FALSE; /* fake name */
static short local_animation_indices[MAXIMUM_FOG_SCREEN_LAYERS] = {0}; /* fake name */
static real_rgb_color local_layer_colors[MAXIMUM_FOG_SCREEN_LAYERS] = {0}; /* fake name */
static real local_fog_eye_density = 0.f;
static short local_fog_pass = 0;
static struct fog_screen_window local_fog_screen_windows[MAXIMUM_WINDOWS] = {0}; /* fake name */
static struct transparent_geometry_group *local_opaque_model_groups = NULL; /* fake name */
static long local_opaque_model_group_count = 0; /* fake name */
static boolean local_fog_screen_active[MAXIMUM_WINDOWS] = {0}; /* fake name */
static __int64 local_fog_screen_frame_index[MAXIMUM_WINDOWS] = {0}; /* fake name */

static boolean local_first_time = TRUE; /* fake name */

/* ---------- private code */

static boolean rasterizer_environment_fog_screen_is_active(
	void)
{
	boolean result;
	short window_index = global_window_parameters.window_index;

	if (window_index >= 0 && window_index < MAXIMUM_WINDOWS)
	{
		local_fog_screen_active[window_index] = FALSE;

		if (rasterizer_globals.frame_index != local_fog_screen_frame_index[window_index])
		{
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 92, global_window_parameters.fog.planar_maximum_depth!=0.0f);

			if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
				rasterizer_debug_options.draw_environment_fog_screen &&
				global_window_parameters.rasterizer_target == _rasterizer_target_render_primary &&
				global_window_parameters.fog.screen &&
				global_window_parameters.fog.screen->layer_count > 0 &&
				global_window_parameters.fog.screen->map.index != NONE &&
				global_window_parameters.fog.screen->far_distance != 0.f &&
				global_window_parameters.fog.screen->far_density != 0.f &&
				(!TEST_FLAG(global_window_parameters.fog.runtime_flags, _render_fog_runtime_screen_use_sky_interpolator_bit) || global_window_parameters.fog.screen_external_intensity > 0.f))
			{
				real planar_maximum_depth = global_window_parameters.fog.planar_maximum_depth;
				real eye_distance = dot_product3d((real_vector3d *)&global_window_parameters.camera.position, &global_window_parameters.fog.plane.n) - global_window_parameters.fog.plane.d;
				real start_distance = global_window_parameters.fog.screen->start_distance_from_fog_plane;
				real bottom_distance = -planar_maximum_depth;

				if (start_distance == bottom_distance)
				{
					start_distance = 0.0001f - planar_maximum_depth;
				}

				if (TEST_FLAG(global_window_parameters.fog.runtime_flags, _render_fog_runtime_screen_use_sky_interpolator_bit))
				{
					local_fog_eye_density = global_window_parameters.fog.screen_external_intensity;
				}
				else
				{
					local_fog_eye_density = PIN((eye_distance - start_distance) / (bottom_distance - start_distance), 0.f, 1.f);
				}

				if (local_fog_eye_density > 0.f)
				{
					local_fog_screen_active[window_index] = TRUE;
				}
			}
		}

		result = local_fog_screen_active[window_index];
	}
	else
	{
		result = FALSE;
	}

	return result;
}

static void rasterizer_environment_fog_screen_wind_update(
	struct fog_screen const *screen,
	struct fog_screen_wind *wind)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 163, screen);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 164, wind);

	if (screen->wind_velocity_upper_bound > 0.f)
	{
		scale_vector2d(&wind->direction, 1.f - screen->wind_acceleration_weight, &wind->direction);
		point_from_line2d((real_point2d *)&wind->direction, &wind->target_direction, screen->wind_acceleration_weight, (real_point2d *)&wind->direction);

		if (normalize2d(&wind->direction) == 0.f)
		{
			set_real_vector2d(&wind->direction, 1.f, 0.f);
		}

		scalars_interpolate(wind->velocity, wind->target_velocity, screen->wind_acceleration_weight, &wind->velocity);

		if (global_frame_parameters.game_time_sec - wind->target_time >= wind->target_period)
		{
			real_vector2d perpendicular;
			real perpendicular_magnitude = power(real_local_random(), 1.f - screen->wind_perpendicular_weight);
			real perpendicular_sign = local_random_boolean() ? -1.f : 1.f;

			perpendicular2d(&wind->direction, &perpendicular);
			scale_vector2d(&perpendicular, perpendicular_sign * perpendicular_magnitude, &perpendicular);
			point_from_line2d((real_point2d *)&perpendicular, &wind->direction, 1.f - perpendicular_magnitude, (real_point2d *)&wind->target_direction);

			if (normalize2d(&wind->target_direction) == 0.f)
			{
				set_real_vector2d(&wind->target_direction, 1.f, 0.f);
			}

			wind->target_velocity = real_local_random_range(screen->wind_velocity_lower_bound, screen->wind_velocity_upper_bound);
			wind->target_time = global_frame_parameters.game_time_sec;
			wind->target_period = real_local_random_range(screen->wind_period_lower_bound, screen->wind_period_upper_bound);
		}
	}

	return;
}

/* ---------- public code */

boolean rasterizer_environment_fog_screen_initialize(
	void)
{
	boolean success = TRUE;

	local_opaque_model_groups = match_malloc("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 248, RASTERIZER_MAXIMUM_NEARBY_OPAQUE_MODEL_GEOMETRY_GROUPS_THAT_MIGHT_OBSCURE_THE_ENVIRONMENT_FOG_SCREEN_EFFECT * sizeof(*local_opaque_model_groups));
	local_opaque_model_group_count = 0;

	if (!local_opaque_model_groups)
	{
		error(_error_silent, "### ERROR failed to allocate opaque model geometry buffer for environment fog screen");
		success = FALSE;
	}

	return success;
}

void rasterizer_environment_fog_screen_window_begin(
	void)
{
	local_opaque_model_group_count = 0;

	return;
}

void rasterizer_environment_fog_screen_window_end(
	void)
{
	return;
}

void rasterizer_environment_fog_screen_dispose(
	void)
{
	if (local_opaque_model_groups)
	{
		match_free("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 279, local_opaque_model_groups);
	}

	return;
}

void _rasterizer_environment_fog_begin(
	void)
{
	static short warning_count = 0;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 290, global_window_parameters.fog.atmospheric_maximum_distance>0.0f);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 291, global_window_parameters.fog.planar_maximum_distance>0.0f);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 292, global_window_parameters.fog.planar_maximum_depth>0.0f);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 293, global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_fog);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal && rasterizer_debug_options.draw_environment_fog)
	{
		real eye_distance = dot_product3d((real_vector3d *)&global_window_parameters.camera.position, &global_window_parameters.fog.plane.n) - global_window_parameters.fog.plane.d;
		real atmospheric_density = PIN(eye_distance / global_window_parameters.fog.atmospheric_maximum_distance, 0.f, 1.f);
		real planar_density = PIN(-eye_distance / global_window_parameters.fog.planar_maximum_depth, 0.f, 1.f);

		if (TEST_FLAG(global_window_parameters.fog.fog_definition_flags, _fog_definition_atmosphere_dominant_bit))
		{
			atmospheric_density = 1.f;

			if (eye_distance < -0.3f && warning_count < 20)
			{
				error(_error_silent, "### WARNING camera went below atmosphere-dominant fog plane");
				warning_count++;
			}
		}

		rasterizer_set_texture_direct(0, global_rasterizer_data->atmospheric_fog_density.index, 0);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		rasterizer_set_texture_direct(1, global_rasterizer_data->planar_fog_density.index, 0);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ONE);
		SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ALPHAREF, 0);
		SetRenderStateSmart(D3DRS_ZENABLE, D3DZB_TRUE);
		SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_EQUAL);
		SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(2, PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
		pixel_shader.PSConstant0[0] = real_alpha_intensity_to_pixel32(global_window_parameters.fog.atmospheric_maximum_density * atmospheric_density, global_window_parameters.fog.atmospheric_maximum_density);
		pixel_shader.PSConstant1[0] = real_alpha_intensity_to_pixel32(global_window_parameters.fog.planar_maximum_density * planar_density, (1.f - planar_density) * global_window_parameters.fog.planar_maximum_density);
		pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_C1, PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_T1);
		pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_C0, PS_REGISTER_T0 | PS_CHANNEL_ALPHA);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_V0, PS_REGISTER_T0, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSConstant0[1] = real_a_rgb_color_to_pixel32(atmospheric_density, &global_window_parameters.fog.atmospheric_color);
		pixel_shader.PSConstant1[1] = real_rgb_color_to_pixel32(&global_window_parameters.fog.planar_color);
		pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0 | PS_CHANNEL_ALPHA);
		pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_T0, PS_REGISTER_C1, PS_REGISTER_R0 | PS_CHANNEL_ALPHA);
		pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R1 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ZERO, PS_REGISTER_EF_PROD);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R1, PS_REGISTER_V0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, 0);
		rasterizer_set_pixel_shader(&pixel_shader);
	}

	return;
}

void _rasterizer_environment_fog_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 423, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal && rasterizer_debug_options.draw_environment_fog)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 430, shader);

		shader_environment_get(shader);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 435, vertex_buffer);

		rasterizer_set_vertex_shader_permutation(_vs_environment_fog, vertex_buffer->type, shader_get_vertex_shader_permutation(shader));
		rasterizer_draw_dynamic_triangles_static_vertices(
			dynamic_triangle_buffer_index,
			first_triangle_index,
			triangle_count,
			vertex_buffer);

		if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.environment_fog_primitive_count++;
			rasterizer_frame_statistics.environment_fog_triangle_count += triangle_count;
			rasterizer_frame_statistics.environment_fog_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count);
		}
	}

	return;
}

void _rasterizer_environment_fog_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_fog);

	return;
}

void _rasterizer_environment_fog_screen_wind_get_vector(
	short window_index,
	real dt,
	real_vector3d *wind_vector)
{
	struct fog_screen_wind const *wind = &local_fog_screen_windows[window_index].wind;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 484, window_index>=0 && window_index<MAXIMUM_WINDOWS);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 485, wind_vector);

	wind_vector->i = wind->velocity * wind->direction.i * dt;
	wind_vector->j = wind->direction.j * wind->velocity * dt;
	wind_vector->k = 0.f;

	return;
}

void _rasterizer_environment_fog_screen_begin(
	short pass)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 499, pass==0 || pass==1);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 500, global_d3d_device);

	local_fog_pass = pass;

	if (pass == 0)
	{
		rasterizer_profile_begin(_rasterizer_profile_environment_fog_screen);
	}

	if (rasterizer_environment_fog_screen_is_active())
	{
		struct fog_screen_window *window = &local_fog_screen_windows[global_window_parameters.window_index];
		struct fog_screen const *screen = global_window_parameters.fog.screen;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 514, local_fog_eye_density>0.0f);

		if (pass == 0)
		{
			real_matrix4x3 transform;
			real layer_spacing;
			real one_over_depth;

			{
				real_vector3d wind_vector;
				real_matrix4x3 *world_to_view = &local_world_to_view[global_window_parameters.window_index];
				real_matrix4x3 wind_transform = *global_identity4x3;

				if (local_first_time)
				{
					short window_index;

					bitmap_group_get(screen->map.index);

					for (window_index = 0; window_index < MAXIMUM_WINDOWS; window_index++)
					{
						short layer_index;

						memset(&local_fog_screen_windows[window_index], 0, sizeof(local_fog_screen_windows[window_index]));

						for (layer_index = 0; layer_index < screen->layer_count; layer_index++)
						{
							set_real_point2d(&local_fog_screen_windows[window_index].layer_offsets[layer_index], real_local_random(), real_local_random());
						}

						memcpy(&local_world_to_view[window_index], &global_window_parameters.camera, sizeof(local_world_to_view[window_index]));
					}

					local_first_time = FALSE;
				}

				rasterizer_environment_fog_screen_wind_update(screen, &window->wind);
				rasterizer_environment_fog_screen_wind_get_vector(global_window_parameters.window_index, global_frame_parameters.dt, &wind_vector);
				wind_transform.position.x = wind_vector.i;
				wind_transform.position.y = wind_vector.j;
				matrix4x3_inverse(world_to_view, &transform);
				matrix4x3_multiply(&wind_transform, &transform, &transform);
				matrix4x3_multiply(&global_window_parameters.frustum.world_to_view, &transform, &transform);
				memcpy(&local_world_to_view[global_window_parameters.window_index], &global_window_parameters.frustum.world_to_view, sizeof(local_world_to_view[global_window_parameters.window_index]));
			}

			layer_spacing = (screen->far_distance - screen->near_distance) / screen->layer_count;
			one_over_depth = 1.f / (screen->far_distance - screen->near_distance);

			{
				real vsh_constants__layers[2 * MAXIMUM_FOG_SCREEN_LAYERS][4];
				real layer_animation_t[MAXIMUM_FOG_SCREEN_LAYERS];
				real sine_rotation;
				real cosine_rotation;
				short layer_index;
				short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
				short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;
				real aspect_ratio = (real)viewport_height / viewport_width;
				real strafing_scale = screen->map_scale * 0.5f / (tangent(global_window_parameters.camera.vertical_field_of_view * 0.5f) * screen->far_distance * aspect_ratio);
				real vsh_constants__screenproj[VSH_CONSTANTS__SCREENPROJ_COUNT][4] =
				{
					{2.f / viewport_width, 0.f, 0.f, -1.f - 1.f / viewport_width},
					{0.f, -2.f / viewport_height, 0.f, 1.f / viewport_height + 1.f},
					{0.f, 0.f, 0.f, 0.5f},
					{0.f, 0.f, 0.f, 1.f},
					{0.f, 0.f, 0.f, 1.f}
				};

				{
					real_vector3d forward = {1.f, 0.f, 0.f};

					matrix4x3_transform_normal(&transform, &forward, &forward);
					window->rotation -= -arctangent(forward.j, forward.i) * screen->rotation_multiplier;
				}

				cosine_rotation = cosine(window->rotation);
				sine_rotation = sine(window->rotation);

				if (screen->far_distance - screen->near_distance > 0.01f)
				{
					short layer_offset;

					window->offset -= screen->zoom_multiplier / layer_spacing * transform.position.z;
					layer_offset = (short)fast_ftol(floor(window->offset));

					if (layer_offset > 0)
					{
						set_real_point2d(&window->layer_offsets[(short)((word)(window->base_layer_index - 1) % screen->layer_count)], real_local_random(), real_local_random());
					}
					else if (layer_offset < 0)
					{
						set_real_point2d(&window->layer_offsets[window->base_layer_index], real_local_random(), real_local_random());
					}

					window->base_layer_index = (short)((word)(window->base_layer_index - layer_offset) % screen->layer_count);
					window->offset -= layer_offset;

					if (window->offset < 0.f || window->offset >= 1.f)
					{
						error(
							_error_silent,
							"### ERROR fog_screen: base z failure (near=%f, far=%f, z=%f, offset=%d) -- tell Bernie!",
							screen->near_distance,
							screen->far_distance,
							window->offset,
							layer_offset);
						return;
					}
				}

				for (layer_index = 0; layer_index < screen->layer_count; layer_index++)
				{
					real const phases[MAXIMUM_FOG_SCREEN_LAYERS] = {0.f, 0.7135f, 0.3422f, 0.5798f};
					struct bitmap_group *bitmap_group = bitmap_group_get(screen->map.index);

					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 653, bitmap_group && bitmap_group->bitmaps.count>0);

					if (screen->animation_period > 0.f)
					{
						short animation_index;
						real t = bitmap_group->bitmaps.count * phases[layer_index] + global_frame_parameters.game_time_sec / screen->animation_period;
						real animation_time = PIN(t - floor(t), 0.f, 1.f);

						layer_animation_t[layer_index] = animation_time;
						animation_index = (short)(fast_ftol(floor(t)) % bitmap_group->bitmaps.count);

						if (animation_index < 0)
						{
							static boolean warned = FALSE;

							if (!warned)
							{
								error(_error_silent, "### ERROR fog screen animation index is invalid -- tell Bernie!!");
								error(_error_silent, "\tindex=%d", animation_index);
								error(_error_silent, "\ttime=%f[%x]", global_frame_parameters.game_time_sec, *(unsigned long *)&global_frame_parameters.game_time_sec);
								error(_error_silent, "\tperiod=%f[%x]", screen->animation_period, *(unsigned long *)&screen->animation_period);
								error(_error_silent, "\tphase=%f[%x]", phases[layer_index], *(unsigned long *)&phases[layer_index]);
								error(_error_silent, "\tt=%f[%x]", t, *(unsigned long *)&t);
								error(_error_silent, "\tanimation_time=%f[%x]", animation_time, *(unsigned long *)&layer_animation_t[layer_index]);
								warned = TRUE;
							}

							animation_index = 0;
						}

						match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 682, animation_index>=0 && animation_index<bitmap_group->bitmaps.count);
						local_animation_indices[layer_index] = animation_index;
					}
					else
					{
						layer_animation_t[layer_index] = 0.f;
						local_animation_indices[layer_index] = (short)(layer_index % bitmap_group->bitmaps.count);
					}
				}

				for (layer_index = 0; layer_index < screen->layer_count; layer_index++)
				{
					short layer = (short)((word)(window->base_layer_index + layer_index) % screen->layer_count);
					real layer_distance = (layer_index + window->offset) * layer_spacing + screen->near_distance;
					real fade = 2.f * ((layer_distance - screen->near_distance) * one_over_depth) - 1.f;
					real layer_density = local_fog_eye_density * screen->far_density * pow(1.f - pow(fabs(fade), 3.f), 2.f);
					real layer_scale = screen->map_scale / screen->far_distance * layer_distance;
					real_point3d layer_center = {0.f, 0.f, -layer_distance};

					matrix4x3_transform_point(&transform, &layer_center, &layer_center);
					window->layer_offsets[layer].x -= (layer_center.y * sine_rotation + layer_center.x * cosine_rotation) * screen->strafing_multiplier * strafing_scale;
					window->layer_offsets[layer].y -= (layer_center.y * cosine_rotation - layer_center.x * sine_rotation) * screen->strafing_multiplier * strafing_scale;

					if (screen->animation_period != 0.f)
					{
						real_rgb_color *color = &local_layer_colors[layer_index];
						real t = layer_animation_t[layer];

						color->red = (1.f - t) * (1.f - t) * layer_density;
						color->green = (t + t) * (1.f - t) * layer_density;
						color->blue = t * t * layer_density;
					}
					else
					{
						real_rgb_color *color = &local_layer_colors[layer_index];

						color->red = 0.f;
						color->green = layer_density;
						color->blue = 0.f;
					}

					set_real_vector4d(
						(real_vector4d *)vsh_constants__layers[2 * layer_index],
						layer_scale * cosine_rotation * 0.5f,
						layer_scale * sine_rotation * aspect_ratio * 0.5f,
						0.f,
						window->layer_offsets[layer].x);
					set_real_vector4d(
						(real_vector4d *)vsh_constants__layers[2 * layer_index + 1],
						layer_scale * sine_rotation * -0.5f,
						layer_scale * cosine_rotation * aspect_ratio * 0.5f,
						0.f,
						window->layer_offsets[layer].y);
				}

				IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__DETAILOBJ_OFFSET,
					vsh_constants__layers,
					2 * MAXIMUM_FOG_SCREEN_LAYERS);
				IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__SCREENPROJ_OFFSET,
					vsh_constants__screenproj,
					VSH_CONSTANTS__SCREENPROJ_COUNT);
			}

			if (screen->near_density == screen->far_density)
			{
				local_environment_multipass = FALSE;
				local_model_multipass = FALSE;
			}
			else
			{
				word flags = global_window_parameters.fog.screen->flags;

				local_environment_multipass = !TEST_FLAG(flags, _fog_screen_no_environment_multipass_bit);
				local_model_multipass = !TEST_FLAG(flags, _fog_screen_no_model_multipass_bit) && local_opaque_model_group_count > 0;
			}
		}

		if (local_environment_multipass || local_model_multipass)
		{
			boolean success = TRUE;
			boolean clear_zbuffer = pass == 0 &&
				!(TEST_FLAG(screen->flags, _fog_screen_no_environment_multipass_bit) && TEST_FLAG(screen->flags, _fog_screen_no_model_multipass_bit)) &&
				(TEST_FLAG(screen->flags, _fog_screen_no_environment_multipass_bit) ||
				(TEST_FLAG(screen->flags, _fog_screen_no_model_multipass_bit) && local_opaque_model_group_count > 0) ||
				(rasterizer_water_get_visibility_for_window() && rasterizer_debug_options.draw_water));

			D3DCALL(success, IDirect3DDevice8_Clear(global_d3d_device,
				0,
				NULL,
				D3DCLEAR_TARGET_A | (clear_zbuffer ? D3DCLEAR_ZBUFFER : 0),
				0,
				1.f,
				0));

			SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
			SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
			SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ZENABLE, D3DZB_TRUE);
			SetRenderStateSmart(D3DRS_ZFUNC, pass ? D3DCMP_EQUAL : D3DCMP_LESSEQUAL);
			SetRenderStateSmart(D3DRS_ZWRITEENABLE, pass == 0);
			SetRenderStateSmart(D3DRS_ZBIAS, 0);

			if (pass == 0)
			{
				real one_over_depth = 1.f / (screen->far_distance - screen->near_distance);
				real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
				{
					{
						global_window_parameters.camera.forward.i * one_over_depth,
						global_window_parameters.camera.forward.j * one_over_depth,
						global_window_parameters.camera.forward.k * one_over_depth,
						-(dot_product3d((real_vector3d *)&global_window_parameters.camera.position, &global_window_parameters.camera.forward) + screen->near_distance) * one_over_depth
					},
					{0.f, 0.f, 0.f, 0.f},
					{0.f, 0.f, 0.f, 0.f}
				};

				D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__TEXSCALE_OFFSET,
					vsh_constants__texscale,
					1));
			}

			memset(&pixel_shader, 0, sizeof(pixel_shader));

			if (TEST_FLAG(global_window_parameters.fog.screen->flags, _fog_screen_no_texture_based_falloff_bit))
			{
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
				pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_FOG | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, 0);
			}
			else
			{
				rasterizer_set_texture_direct(0, global_rasterizer_data->planar_fog_density.index, 0);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 856, screen->far_density!=0.0f);

				pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
				pixel_shader.PSConstant0[0] = real_alpha_to_pixel32(screen->near_density / screen->far_density);
				pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T0, PS_REGISTER_ONE);
				pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
				pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, 0);
			}

			rasterizer_set_pixel_shader(&pixel_shader);

			if (local_model_multipass)
			{
				short group_index;

				for (group_index = 0; group_index < local_opaque_model_group_count; group_index++)
				{
					real_matrix4x3 const *node_matrices;
					struct render_skinning skinning;
					struct transparent_geometry_group const *group = &local_opaque_model_groups[group_index];

					if (group->shader->base.type == _shader_type_model)
					{
						struct shader_model const *shader_model = shader_model_get(group->shader);

						if (!TEST_FLAG(shader_model->model.flags, _shader_model_not_alpha_tested_bit))
						{
							SetRenderStateSmart(D3DRS_PSTEXTUREMODES, PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE));
							SetTextureStageStateSmart(1, D3DTSS_ALPHAKILL, D3DTALPHAKILL_ENABLE);
							rasterizer_set_texture(1, _bitmap_type_2d, _bitmap_usage_multiplicative, shader_model->model.base_map.index, group->shader_permutation_index);
							SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
							SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
							SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
							SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
							SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

							{
								real vsh_constants__texscale_1[VSH_CONSTANTS__TEXSCALE_COUNT - 1][4] =
								{
									{shader_model->model.map_scale.i * group->model_base_map_scale.i, 0.f, 0.f, 0.f},
									{0.f, shader_model->model.map_scale.j * group->model_base_map_scale.j, 0.f, 0.f}
								};

								D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
									VSH_CONSTANTS__TEXSCALE_OFFSET+1,
									vsh_constants__texscale_1,
									VSH_CONSTANTS__TEXSCALE_COUNT-1));
							}
						}
						else
						{
							SetRenderStateSmart(D3DRS_PSTEXTUREMODES, PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE));
							SetTextureStageStateSmart(1, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE);
						}
					}
					else if (group->shader->base.type == _shader_type_environment || group->shader->base.type == _shader_type_transparent_glass)
					{
						SetRenderStateSmart(D3DRS_PSTEXTUREMODES, PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE));
						SetTextureStageStateSmart(1, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE);
					}

					rasterizer_set_vertex_shader_permutation(_vs_model_fog_screen, rasterizer_transparent_geometry_get_primary_vertex_type(group), 0);

					node_matrices = group->node_matrices;

					if (node_matrices && group->node_matrix_count)
					{
						skinning.node_matrix_count = group->node_matrix_count;
						skinning.node_matrices = (real_matrix4x3 *)node_matrices;
					}
					else
					{
						skinning.node_matrix_count = 1;
						skinning.node_matrices = (real_matrix4x3 *)global_identity4x3;
					}

					rasterizer_set_model_skinning(&skinning);

					if (success)
					{
						rasterizer_transparent_geometry_group_draw__internal(group, FALSE);

						if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
						{
							rasterizer_frame_statistics.environment_fog_screen_model_primitive_count++;
							rasterizer_frame_statistics.environment_fog_screen_model_triangle_count += group->triangle_count;
							rasterizer_frame_statistics.environment_fog_screen_model_vertex_count += rasterizer_frame_statistics_count_static_vertices(group->triangle_buffer, group->vertex_buffers);
						}
					}
				}

				SetTextureStageStateSmart(1, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE);
			}

			if (!success)
			{
				error(_error_silent, "### ERROR rasterizer_environment_fog_screen_begin failed");
			}
		}
	}

	return;
}

void _rasterizer_environment_fog_screen_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 988, global_d3d_device);

	if (rasterizer_environment_fog_screen_is_active())
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 992, local_fog_pass==0 || local_fog_pass==1);

		if (local_environment_multipass)
		{
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 996, global_window_parameters.fog.screen);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 997, global_window_parameters.window_index>=0 && global_window_parameters.window_index<MAXIMUM_WINDOWS);

			rasterizer_set_vertex_shader_permutation(_vs_environment_fog_screen, vertex_buffer->type, shader_get_vertex_shader_permutation(shader));
			rasterizer_draw_dynamic_triangles_static_vertices(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				vertex_buffer);

			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.environment_fog_screen_primitive_count++;
				rasterizer_frame_statistics.environment_fog_screen_triangle_count += triangle_count;
				rasterizer_frame_statistics.environment_fog_screen_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count);
			}
		}
	}

	return;
}

void _rasterizer_environment_fog_screen_end(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 1036, global_d3d_device);

	if (rasterizer_environment_fog_screen_is_active())
	{
		short layer_index;
		struct fog_screen const *screen = global_window_parameters.fog.screen;
		struct fog_screen_window const *window = &local_fog_screen_windows[global_window_parameters.window_index];
		boolean multipass = local_environment_multipass || local_model_multipass;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 1045, local_fog_pass==0 || local_fog_pass==1);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 1047, global_window_parameters.fog.screen);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 1048, global_window_parameters.window_index>=0 && global_window_parameters.window_index<MAXIMUM_WINDOWS);

		for (layer_index = 0; layer_index < screen->layer_count; layer_index++)
		{
			rasterizer_set_texture_direct(layer_index, screen->map.index, local_animation_indices[(short)((word)(window->base_layer_index + layer_index) % screen->layer_count)]);
			SetTextureStageStateSmart(layer_index, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(layer_index, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(layer_index, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(layer_index, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(layer_index, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		}

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, local_fog_pass == 0 && multipass ? D3DCOLORWRITEENABLE_ALPHA : D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, multipass ? D3DBLEND_INVDESTALPHA : D3DBLEND_ONE);
		SetRenderStateSmart(D3DRS_DESTBLEND, multipass ? D3DBLEND_ONE : D3DBLEND_SRCALPHA);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, D3DZB_FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		rasterizer_set_vertex_shader_permutation(_vs_screen_texture_animation, _rasterizer_vertex_type_dynamic_screen, 0);

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(
			PS_TEXTUREMODES_PROJECT2D,
			screen->layer_count > 1 ? PS_TEXTUREMODES_PROJECT2D : PS_TEXTUREMODES_NONE,
			screen->layer_count > 2 ? PS_TEXTUREMODES_PROJECT2D : PS_TEXTUREMODES_NONE,
			screen->layer_count > 3 ? PS_TEXTUREMODES_PROJECT2D : PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(4, PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
		pixel_shader.PSConstant0[0] = real_rgb_color_to_pixel32(&local_layer_colors[0]);
		pixel_shader.PSConstant1[0] = real_rgb_color_to_pixel32(&local_layer_colors[1]);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_C0, PS_REGISTER_T1, PS_REGISTER_C1);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_T0, PS_REGISTER_T1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT | PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
		pixel_shader.PSConstant0[1] = real_rgb_color_to_pixel32(&local_layer_colors[2]);
		pixel_shader.PSConstant1[1] = real_rgb_color_to_pixel32(&local_layer_colors[3]);
		pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_UNSIGNED_INVERT, screen->layer_count > 1 ? PS_REGISTER_T1 | PS_INPUTMAPPING_UNSIGNED_INVERT : PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_C0, PS_REGISTER_T3, PS_REGISTER_C1);
		pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_T2, PS_REGISTER_T3, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT | PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
		pixel_shader.PSAlphaInputs[2] = PS_COMBINERINPUTS(screen->layer_count > 2 ? PS_REGISTER_T2 | PS_INPUTMAPPING_UNSIGNED_INVERT : PS_REGISTER_ONE, screen->layer_count > 3 ? PS_REGISTER_T3 | PS_INPUTMAPPING_UNSIGNED_INVERT : PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSAlphaOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(screen->layer_count > 2 ? PS_REGISTER_T2 | PS_INPUTMAPPING_UNSIGNED_INVERT : PS_REGISTER_ONE, screen->layer_count > 3 ? PS_REGISTER_T3 : PS_REGISTER_ZERO, screen->layer_count > 2 ? PS_REGISTER_T2 : PS_REGISTER_ZERO, PS_REGISTER_ONE);
		pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
		pixel_shader.PSAlphaInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSAlphaOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(screen->layer_count > 1 ? PS_REGISTER_T1 | PS_INPUTMAPPING_UNSIGNED_INVERT : PS_REGISTER_ONE, PS_REGISTER_R0, screen->layer_count > 1 ? PS_REGISTER_T1 : PS_REGISTER_ZERO, PS_REGISTER_ONE);
		pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);

		if (screen->color)
		{
			pixel_shader.PSFinalCombinerConstant0 = screen->color;
		}
		else
		{
			pixel_shader.PSFinalCombinerConstant0 = real_rgb_color_to_pixel32(&global_window_parameters.fog.planar_color);
		}

		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_C0, PS_REGISTER_EF_PROD, PS_REGISTER_ZERO);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_C0, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, 0);
		rasterizer_set_pixel_shader(&pixel_shader);

		IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, -1, 1);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, -1, 1);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 1);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, 1, 1);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, -1);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, 1, -1);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, -1, -1);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, -1, -1);
		IDirect3DDevice8_End(global_d3d_device);

		if (local_fog_pass == 0 && multipass)
		{
			SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
			SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_INVDESTALPHA);
			SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);

			memset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
			rasterizer_set_pixel_shader(&pixel_shader);

			IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
			IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, -1, 1);
			IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, -1, 1);
			IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 1);
			IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, 1, 1);
			IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, -1);
			IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, 1, -1);
			IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, -1, -1);
			IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, -1, -1);
			IDirect3DDevice8_End(global_d3d_device);
		}
	}

	if (local_fog_pass != 0)
	{
		rasterizer_profile_end(_rasterizer_profile_environment_fog_screen);
	}

	return;
}

static struct rasterizer_model_begin_parameters const *local_model_parameters = NULL; /* fake name */
static boolean local_model_node_matrices_valid = FALSE; /* fake name */
static struct render_lighting const *local_model_lighting = NULL; /* fake name */
static struct render_animation const *local_model_animation = NULL; /* fake name */

boolean rasterizer_environment_fog_screen_model_begin(
	struct rasterizer_model_begin_parameters const *parameters)
{
	boolean result = FALSE;

	if (rasterizer_debug_options.draw_environment_fog_screen &&
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_environment_fog_screen_is_active())
	{
		real_vector3d camera_to_model;
		struct fog_screen const *screen = global_window_parameters.fog.screen;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 1177, parameters);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment_fog.c", 1178, screen);

		vector_from_points3d(&global_window_parameters.camera.position, &parameters->centroid, &camera_to_model);

		if (dot_product3d(&global_window_parameters.camera.forward, &camera_to_model) < screen->far_distance)
		{
			local_model_parameters = parameters;
			local_model_node_matrices_valid = FALSE;
			result = TRUE;

			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.environment_fog_screen_model_count++;
			}
		}
	}

	return result;
}

void rasterizer_environment_fog_screen_model_submit(
	struct shader const *shader,
	short shader_permutation_index,
	struct triangle_buffer const *triangle_buffer,
	long dynamic_triangle_buffer_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer,
	long dynamic_vertex_buffer_index)
{
	static boolean warned = FALSE;

	if (rasterizer_debug_options.draw_environment_fog_screen && rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal)
	{
		if (local_opaque_model_group_count < RASTERIZER_MAXIMUM_NEARBY_OPAQUE_MODEL_GEOMETRY_GROUPS_THAT_MIGHT_OBSCURE_THE_ENVIRONMENT_FOG_SCREEN_EFFECT)
		{
			struct transparent_geometry_group *group = &local_opaque_model_groups[local_opaque_model_group_count++];

			group->shader = shader;
			group->shader_permutation_index = shader_permutation_index;
			group->dynamic_triangle_buffer_index = dynamic_triangle_buffer_index;
			group->triangle_buffer = triangle_buffer;
			group->first_triangle_index = 0;
			group->triangle_count = triangle_count;
			group->dynamic_vertex_buffer_index = dynamic_vertex_buffer_index;
			group->vertex_buffers = vertex_buffer;
			group->model_base_map_scale = local_model_parameters->base_map_scale;

			if (!local_model_node_matrices_valid)
			{
				local_model_node_matrices = rasterizer_memory_alloc_const(local_model_parameters->skinning.node_matrices, local_model_parameters->skinning.node_matrix_count * sizeof(*local_model_parameters->skinning.node_matrices));
				local_model_node_matrix_count = local_model_parameters->skinning.node_matrix_count;
				local_model_node_matrices_valid = TRUE;
			}

			group->node_matrices = local_model_node_matrices;
			group->node_matrix_count = local_model_node_matrix_count;
			group->lighting = local_model_lighting;
			group->animation = local_model_animation;
		}
		else if (!warned)
		{
			error(_error_silent, "### ERROR too many opaque model groups obscuring fog screen (max=#%d)", RASTERIZER_MAXIMUM_NEARBY_OPAQUE_MODEL_GEOMETRY_GROUPS_THAT_MIGHT_OBSCURE_THE_ENVIRONMENT_FOG_SCREEN_EFFECT);
			warned = TRUE;
		}
	}

	return;
}

void rasterizer_environment_fog_screen_model_end(
	void)
{
	local_model_parameters = NULL;

	return;
}
