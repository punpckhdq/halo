/*
RASTERIZER_XBOX_ENVIRONMENT.C
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

/* ---------- constants */

#define VECTORS_DEBUG_BITMAP_INDEX_SCALE 1000 /* fake name */

enum
{
	_vectors_debug_mode_normal_map = 50, /* fake name */
	NUMBER_OF_VECTORS_DEBUG_MODES, /* fake name */
};

/* ---------- globals */

static short specular_light_vertex_shader_permutation_index = NONE;

static real specular_light_brightness;
static boolean local_lightmap_has_no_data;
static real_rgb_color local_lightmap_ambient_color;

/* ---------- public code */

void _rasterizer_environment_lightmaps_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 17, global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_lightmaps);

	if ((rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_edge) &&
		rasterizer_debug_options.draw_environment_lightmaps)
	{
		rasterizer_set_texture_direct(3, global_rasterizer_data->vector_normalization.index, 0);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ALPHAREF, 127);
		SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		SetRenderStateSmart(D3DRS_ZWRITEENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);
	}

	return;
}

void _rasterizer_environment_lightmap_begin(
	struct bitmap_data const *lightmap)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 68, global_d3d_device);

	if ((rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_edge) &&
		rasterizer_debug_options.draw_environment_lightmaps)
	{
		if (lightmap)
		{
			rasterizer_set_texture_bitmap_data(2, lightmap);
			SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, rasterizer_debug_options.lightmap_filtering_enabled ? D3DTEXF_LINEAR : D3DTEXF_POINT);
			SetTextureStageStateSmart(2, D3DTSS_MINFILTER, rasterizer_debug_options.lightmap_filtering_enabled ? D3DTEXF_LINEAR : D3DTEXF_POINT);
			SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, rasterizer_debug_options.lightmap_filtering_enabled ? D3DTEXF_LINEAR : D3DTEXF_POINT);
			local_lightmap_has_no_data = FALSE;
		}
		else
		{
			IDirect3DDevice8_SetTexture(global_d3d_device, 2, NULL);
			local_lightmap_has_no_data = TRUE;
		}

		if (rasterizer_globals.lightmap_mode>_rasterizer_lightmap_mode_normal)
		{
			if (rasterizer_globals.lightmap_mode==_rasterizer_lightmap_mode_ambient)
			{
				local_lightmap_ambient_color.blue = rasterizer_debug_options.lightmap_ambient;
				local_lightmap_ambient_color.green = local_lightmap_ambient_color.blue;
				local_lightmap_ambient_color.red = local_lightmap_ambient_color.green;
			}
			else
			{
				unsigned long seed = (unsigned long)lightmap;

				local_lightmap_ambient_color.red = real_seed_random(&seed);
				local_lightmap_ambient_color.green = real_seed_random(&seed);
				local_lightmap_ambient_color.blue = real_seed_random(&seed);
			}
		}
	}

	return;
}

void _rasterizer_environment_lightmap_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 135, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_vectors)
	{
		if (vertex_buffer->type == _rasterizer_vertex_type_environment_compressed)
		{
			struct
			{
				short vertex_shader_permutation;
				unsigned long final_combiner_input;
			} vectors[NUMBER_OF_VECTORS_DEBUG_MODES] =
			{
				{0, PS_REGISTER_T0},
				{0, PS_REGISTER_T1},
				{0, PS_REGISTER_T2},
				{0, PS_REGISTER_T3},
				{1, PS_REGISTER_T0},
				{1, PS_REGISTER_T1},
				{1, PS_REGISTER_T2},
				{1, PS_REGISTER_V0 | PS_CHANNEL_ALPHA},
				{1, PS_REGISTER_T3},
				{NONE, NONE},
				{0, PS_REGISTER_V0},
				{0, PS_REGISTER_V1},
				{1, PS_REGISTER_V0},
				{1, PS_REGISTER_V1},
				{2, PS_REGISTER_V0},
				{2, PS_REGISTER_V1},
				{3, PS_REGISTER_V0},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{5, PS_REGISTER_V1},
				{2, PS_REGISTER_V0 | PS_CHANNEL_ALPHA},
				{2, PS_REGISTER_V1 | PS_CHANNEL_ALPHA},
				{2, PS_REGISTER_FOG | PS_CHANNEL_ALPHA},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{2, PS_REGISTER_T0},
				{2, PS_REGISTER_T1},
				{2, PS_REGISTER_T2},
				{2, PS_REGISTER_T3},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{3, PS_REGISTER_V1},
				{4, PS_REGISTER_V0},
				{4, PS_REGISTER_V1},
				{5, PS_REGISTER_V0},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{NONE, NONE},
				{3, PS_REGISTER_R0}
			};
			short bitmap_index = rasterizer_debug_options.pad3 / VECTORS_DEBUG_BITMAP_INDEX_SCALE;
			short vector_index = rasterizer_debug_options.pad3 % VECTORS_DEBUG_BITMAP_INDEX_SCALE;

			if (vector_index>=0 && vector_index<NUMBER_OF_VECTORS_DEBUG_MODES && vectors[vector_index].vertex_shader_permutation!=NONE)
			{
				boolean normal_map = vector_index==_vectors_debug_mode_normal_map;

				if (normal_map && global_rasterizer_data->test[3].index != NONE)
				{
					rasterizer_set_texture_direct(0, global_rasterizer_data->test[3].index, 0);
					SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
					SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
					SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
					SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
					SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
				}
				else
				{
					rasterizer_set_texture_direct(0, global_rasterizer_data->vector_normalization.index, bitmap_index);
					SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
					SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
					SetTextureStageStateSmart(0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
					SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
					SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
					SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
				}

				rasterizer_set_texture_direct(1, global_rasterizer_data->vector_normalization.index, bitmap_index);
				SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(1, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
				rasterizer_set_texture_direct(2, global_rasterizer_data->vector_normalization.index, bitmap_index);
				SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(2, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
				rasterizer_set_texture_direct(3, global_rasterizer_data->vector_normalization.index, bitmap_index);
				SetTextureStageStateSmart(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
				SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
				SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
				SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
				SetRenderStateSmart(D3DRS_ZWRITEENABLE, TRUE);
				SetRenderStateSmart(D3DRS_ZBIAS, 0);

				rasterizer_set_vertex_shader_permutation(_vs_environment_vectors_debug0, vertex_buffer->type, vectors[vector_index].vertex_shader_permutation);

				memset(&pixel_shader, 0, sizeof(pixel_shader));
				pixel_shader.PSTextureModes = PS_TEXTUREMODES(normal_map ? PS_TEXTUREMODES_PROJECT2D : PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP);

				if (normal_map)
				{
					real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
					{
						{rasterizer_debug_options.pad3_scale, 1.f, 1.f, 1.f},
						{1.f, 0.f, 0.f, 0.f},
						{0.f, 1.f, 0.f, 0.f}
					};

					D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
						VSH_CONSTANTS__TEXSCALE_OFFSET,
						vsh_constants__texscale,
						VSH_CONSTANTS__TEXSCALE_COUNT));

					pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(3, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
					pixel_shader.PSConstant0[0] = D3DCOLOR_ARGB(0, 255, 0, 0);
					pixel_shader.PSConstant1[0] = D3DCOLOR_ARGB(0, 0, 0, 255);
					pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T1 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL);
					pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT | PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
					pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_R1);
					pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
					pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_C0, PS_REGISTER_R1, PS_REGISTER_C1);
					pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
					pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_ZERO, vectors[vector_index].final_combiner_input);
				}
				else
				{
					pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
					pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, vectors[vector_index].final_combiner_input);
				}

				rasterizer_set_pixel_shader(&pixel_shader);

				rasterizer_draw_dynamic_triangles_static_vertices2(
					dynamic_triangle_buffer_index,
					first_triangle_index,
					triangle_count,
					&vertex_buffer[0],
					&vertex_buffer[!local_lightmap_has_no_data]);
			}
		}
	}
	else if ((rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_edge) &&
		rasterizer_debug_options.draw_environment_lightmaps)
	{
		struct shader_environment const *shader_environment;
		struct shader_environment_self_illumination_properties const *illumination;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 347, shader);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 348, vertex_buffer);

		shader_environment = shader_environment_get(shader);
		illumination = &shader_environment->environment.self_illumination;
		rasterizer_set_vertex_shader_permutation(_vs_environment_lightmap, vertex_buffer->type, shader_get_vertex_shader_permutation(shader));

		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, TEST_FLAG(shader_environment->environment.flags, _shader_environment_alpha_tested_bit) && rasterizer_debug_options.environment_alpha_testing_enabled);

		if (success)
		{
			rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_vector, TEST_FLAG(shader_environment->environment.flags, _shader_environment_bump_map_is_specular_mask_bit) ? NONE : shader_environment->environment.diffuse.bump_map.index, shader_permutation_index);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
			rasterizer_set_texture(1, _bitmap_type_2d, _bitmap_usage_additive, illumination->map.index, shader_permutation_index);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			if (TEST_FLAG(illumination->flags, _shader_environment_self_illumination_map_point_sampled_bit))
			{
				SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_POINT);
				SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
				SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_POINT);
			}
			else
			{
				SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
			}

			{
				real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
				{
					{shader_environment->environment.diffuse.runtime_bump_map_scale.i, shader_environment->environment.diffuse.runtime_bump_map_scale.j, illumination->map_scale, 1.f},
					{1.f, 0.f, 0.f, 0.f},
					{0.f, 1.f, 0.f, 0.f}
				};

				shader_environment_texture_animation_evaluate(
					shader,
					global_frame_parameters.game_time_sec,
					&vsh_constants__texscale[1][3],
					&vsh_constants__texscale[2][3]);
				D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__TEXSCALE_OFFSET,
					vsh_constants__texscale,
					VSH_CONSTANTS__TEXSCALE_COUNT));
			}

			if (illumination->map.index == NONE)
			{
				memset(&pixel_shader, 0, sizeof(pixel_shader));
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(2, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
				pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_T0, PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
				pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ONE, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_T0);
				pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_EF_PROD, PS_REGISTER_ZERO, PS_REGISTER_R0);
				pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_C0, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);
				pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, local_lightmap_has_no_data ? PS_TEXTUREMODES_NONE : PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_CUBEMAP);
				pixel_shader.PSRGBInputs[0] = rasterizer_debug_options.lightmap_incident_radiosity_enabled ?
					PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T2, PS_REGISTER_C0) :
					PS_COMBINERINPUTS(PS_REGISTER_ONE, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			}
			else
			{
				real primary;
				real secondary;
				real plasma;
				real intensity;
				real_rgb_color primary_color;
				real_rgb_color secondary_color;
				real_rgb_color plasma_on_color;
				real_rgb_color plasma_off_color;

				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 448, illumination->primary_animation_period!=0.0f);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 449, illumination->secondary_animation_period!=0.0f);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 450, illumination->plasma_animation_period!=0.0f);

				primary = periodic_function_evaluate(
					illumination->primary_animation_function,
					(global_frame_parameters.game_time_sec + illumination->primary_animation_phase) / illumination->primary_animation_period);
				secondary = periodic_function_evaluate(
					illumination->secondary_animation_function,
					(global_frame_parameters.game_time_sec + illumination->secondary_animation_phase) / illumination->secondary_animation_period);
				plasma = periodic_function_evaluate(
					illumination->plasma_animation_function,
					(global_frame_parameters.game_time_sec + illumination->plasma_animation_phase) / illumination->plasma_animation_period);

				intensity = 1.f;
				scale_vector3d((real_vector3d const *)&illumination->primary_off_color, (1.f - primary) * intensity, (real_vector3d *)&primary_color);
				scale_vector3d((real_vector3d const *)&illumination->secondary_off_color, (1.f - secondary) * intensity, (real_vector3d *)&secondary_color);
				vector_from_line3d(&primary_color, (real_vector3d const *)&illumination->primary_on_color, intensity * primary, &primary_color);
				vector_from_line3d(&secondary_color, (real_vector3d const *)&illumination->secondary_on_color, intensity * secondary, &secondary_color);
				scale_vector3d((real_vector3d const *)&illumination->plasma_on_color, intensity, (real_vector3d *)&plasma_on_color);
				scale_vector3d((real_vector3d const *)&illumination->plasma_off_color, intensity, (real_vector3d *)&plasma_off_color);

				memset(&pixel_shader, 0, sizeof(pixel_shader));
				pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSAlphaOutputs[4] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSRGBOutputs[4] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSRGBOutputs[5] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, local_lightmap_has_no_data ? PS_TEXTUREMODES_NONE : PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_CUBEMAP);
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(6, PS_COMBINERCOUNT_MUX_MSB | PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
				pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_T1 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_HALFBIAS_NEGATE, PS_REGISTER_ONE);
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_C0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_HALFBIAS_NEGATE, PS_REGISTER_ONE);
				pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY);
				pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTLEFT_2 | PS_COMBINEROUTPUT_AB_CD_MUX);
				pixel_shader.PSRGBInputs[1] = rasterizer_debug_options.lightmap_incident_radiosity_enabled ?
					PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T2, PS_REGISTER_C0) :
					PS_COMBINERINPUTS(PS_REGISTER_ONE, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_T0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
				pixel_shader.PSConstant0[2] = D3DCOLOR_ARGB(0, 255, 0, 0);
				pixel_shader.PSConstant1[2] = D3DCOLOR_ARGB(0, 0, 255, 0);
				pixel_shader.PSAlphaInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1, PS_REGISTER_ONE);
				pixel_shader.PSAlphaOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_T1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_T1, PS_REGISTER_C0, PS_REGISTER_T1, PS_REGISTER_C1);
				pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT | PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
				pixel_shader.PSAlphaInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_EXPAND_NORMAL);
				pixel_shader.PSAlphaOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_AB_CD_MUX);
				pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_C0, PS_REGISTER_R1, PS_REGISTER_C1);
				pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSAlphaInputs[4] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ONE, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_T0);
				pixel_shader.PSRGBInputs[4] = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_C1, PS_REGISTER_ONE);
				pixel_shader.PSRGBInputs[5] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1, PS_REGISTER_ONE);
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_EF_PROD, PS_REGISTER_ZERO, PS_REGISTER_R0);
				pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_C0, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);
				pixel_shader.PSConstant0[0] = real_alpha_to_pixel32(plasma);
				pixel_shader.PSConstant0[3] = real_rgb_color_to_pixel32(&primary_color);
				pixel_shader.PSConstant1[3] = real_rgb_color_to_pixel32(&secondary_color);
				pixel_shader.PSConstant0[4] = real_rgb_color_to_pixel32(&plasma_on_color);
				pixel_shader.PSConstant1[4] = real_rgb_color_to_pixel32(&plasma_off_color);
			}

			pixel_shader.PSFinalCombinerConstant0 = real_rgb_color_to_pixel32(&shader_environment->environment.diffuse.material_color);

			if (rasterizer_globals.lightmap_mode==_rasterizer_lightmap_mode_no_lightmap)
			{
				real vsh_constants__texanim[4][4] =
				{
					{0.5f, 0.6f, 0.6f, 1.f},
					{0.f, 0.f, 1.f, 0.1f},
					{0.9f, 0.9f, 0.8f, 0.f},
					{0.1f, 0.1f, 0.3f, 0.f}
				};

				D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__TEXANIM_OFFSET,
					vsh_constants__texanim,
					4));

				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ONE, PS_REGISTER_V0, PS_REGISTER_ZERO, PS_REGISTER_R0);
			}
			else if (rasterizer_globals.lightmap_mode==_rasterizer_lightmap_mode_ambient || rasterizer_globals.lightmap_mode==_rasterizer_lightmap_mode_random_color)
			{
				pixel_shader.PSFinalCombinerConstant1 = real_rgb_color_to_pixel32(&local_lightmap_ambient_color);
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ONE, PS_REGISTER_C1, PS_REGISTER_ZERO, PS_REGISTER_R0);
			}

			switch (rasterizer_debug_options.drawing_mode)
			{
			case _rasterizer_drawing_mode_normal:
				break;
			case _rasterizer_drawing_mode_bump_color:
			case _rasterizer_drawing_mode_specular_mask_times_bump_color:
			case _rasterizer_drawing_mode_diffuse_texture_times_bump_color:
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
				pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0);
				break;
			case _rasterizer_drawing_mode_bump_edge:
			case _rasterizer_drawing_mode_specular_mask_times_bump_edge:
			case _rasterizer_drawing_mode_diffuse_texture_times_bump_edge:
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
				pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_NEGATIVE_ONE, PS_REGISTER_ONE, PS_REGISTER_ONE);
				pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_COMBINEROUTPUT_SHIFTLEFT_2);
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R1 | PS_CHANNEL_ALPHA);
				break;
			case _rasterizer_drawing_mode_specular_mask:
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
				pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ONE);
				break;
			default:
				match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 575, FALSE, "### ERROR unsupported drawing mode in environment lightmap pass");
			}

			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_draw_dynamic_triangles_static_vertices2(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				&vertex_buffer[0],
				&vertex_buffer[!local_lightmap_has_no_data]);

			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.environment_lightmap_primitive_count++;
				rasterizer_frame_statistics.environment_lightmap_triangle_count += triangle_count;
				rasterizer_frame_statistics.environment_lightmap_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count);
			}
		}
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_environment_lightmap_draw failed");
	}

	return;
}

void _rasterizer_environment_lightmap_end(
	void)
{
	return;
}

void _rasterizer_environment_lightmaps_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_lightmaps);

	return;
}

void _rasterizer_environment_diffuse_lights_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 627, global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_diffuse_lights);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal && rasterizer_debug_options.draw_environment_diffuse_lights)
	{
		rasterizer_set_texture_direct(2, global_rasterizer_data->distance_attenuation.index, 0);
		SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
		SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
		SetTextureStageStateSmart(2, D3DTSS_ADDRESSW, D3DTADDRESS_BORDER);
		SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(2, D3DTSS_MINFILTER, D3DTEXF_POINT);
		SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, D3DTEXF_POINT);
		rasterizer_set_texture_direct(3, global_rasterizer_data->vector_normalization.index, 0);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(3, D3DTSS_MINFILTER, D3DTEXF_POINT);
		SetTextureStageStateSmart(3, D3DTSS_MIPFILTER, D3DTEXF_POINT);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ONE);
		SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ALPHAREF, 0);
		SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_EQUAL);
		SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_PROJECT3D, PS_TEXTUREMODES_CUBEMAP);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(4, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
		pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE, PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE);
		pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTLEFT_2);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T1, PS_REGISTER_T2, PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
		pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_C0, PS_REGISTER_R0, PS_REGISTER_C0);
		pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_C0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R1, PS_REGISTER_ZERO);
		rasterizer_set_pixel_shader(&pixel_shader);
	}

	return;
}

void _rasterizer_environment_diffuse_light_begin(
	long light_index)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 723, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal && rasterizer_debug_options.draw_environment_diffuse_lights)
	{
		struct rasterizer_light_submit_parameters const *light;
		real_matrix3x3 light_matrix;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 729, light_index>=0 && light_index<rasterizer_lights.light_count);

		light = &rasterizer_lights.lights[light_index];

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 732, light->radius>0.0f);

		rasterizer_set_texture(1, _bitmap_type_cube_map, _bitmap_usage_multiplicative, light->definition->gel.map.index, 0);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		{
			real yaw;
			real pitch;
			real roll;
			real_matrix4x3 animation_matrix;

			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 748, light->definition->gel.yaw_period>0.0f);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 749, light->definition->gel.pitch_period>0.0f);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 750, light->definition->gel.roll_period>0.0f);

			yaw = periodic_function_evaluate(
				light->definition->gel.yaw_function,
				global_frame_parameters.game_time_sec / light->definition->gel.yaw_period) * (2.f * _pi);
			pitch = periodic_function_evaluate(
				light->definition->gel.pitch_function,
				global_frame_parameters.game_time_sec / light->definition->gel.pitch_period) * (2.f * _pi);
			roll = periodic_function_evaluate(
				light->definition->gel.roll_function,
				global_frame_parameters.game_time_sec / light->definition->gel.roll_period) * (2.f * _pi);
			matrix4x3_rotation_from_angles(&animation_matrix, yaw, pitch, roll);
			matrix4x3_transform_normal(&animation_matrix, &light->forward, &light_matrix.forward);
			matrix4x3_transform_normal(&animation_matrix, &light->up, &light_matrix.up);
		}

		cross_product3d(&light_matrix.forward, &light_matrix.up, &light_matrix.left);
		normalize3d(&light_matrix.left);

		{
			real vsh_constants__pointlight[VSH_CONSTANTS__POINTLIGHT_COUNT][4] =
			{
				{light->position.x, light->position.y, light->position.z, 0.5f / light->radius},
				{-light_matrix.forward.i, -light_matrix.forward.j, -light_matrix.forward.k, 1.f},
				{-light_matrix.left.i, -light_matrix.left.j, -light_matrix.left.k, 1.f},
				{-light_matrix.up.i, -light_matrix.up.j, -light_matrix.up.k, 1.f},
				{0.f, 0.f, 0.f, 1.f}
			};

			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__POINTLIGHT_OFFSET,
				vsh_constants__pointlight,
				VSH_CONSTANTS__POINTLIGHT_COUNT);
		}

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSCONSTANT0_0, real_rgb_color_to_pixel32(&light->color));
	}

	return;
}

void _rasterizer_environment_diffuse_light_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 819, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal && rasterizer_debug_options.draw_environment_diffuse_lights)
	{
		struct shader_environment const *shader_environment;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 826, shader);

		shader_environment = shader_environment_get(shader);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 831, vertex_buffer);

		rasterizer_set_vertex_shader_permutation(_vs_environment_diffuse_light, vertex_buffer->type, shader_get_vertex_shader_permutation(shader));

		rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_vector, TEST_FLAG(shader_environment->environment.flags, _shader_environment_bump_map_is_specular_mask_bit) ? NONE : shader_environment->environment.diffuse.bump_map.index, shader_permutation_index);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		{
			real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
			{
				{shader_environment->environment.diffuse.runtime_bump_map_scale.i, shader_environment->environment.diffuse.runtime_bump_map_scale.j, 1.f, 1.f},
				{1.f, 0.f, 0.f, 0.f},
				{0.f, 1.f, 0.f, 0.f}
			};

			shader_environment_texture_animation_evaluate(
				shader,
				global_frame_parameters.game_time_sec,
				&vsh_constants__texscale[1][3],
				&vsh_constants__texscale[2][3]);
			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__TEXSCALE_OFFSET,
				vsh_constants__texscale,
				VSH_CONSTANTS__TEXSCALE_COUNT);
		}

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSFINALCOMBINERCONSTANT0, real_rgb_color_to_pixel32(&shader_environment->environment.diffuse.material_color));

		rasterizer_draw_dynamic_triangles_static_vertices(
			dynamic_triangle_buffer_index,
			first_triangle_index,
			triangle_count,
			vertex_buffer);

		if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.environment_light_primitive_count++;
			rasterizer_frame_statistics.environment_light_triangle_count += triangle_count;
			rasterizer_frame_statistics.environment_light_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count);
		}
	}

	return;
}

void _rasterizer_environment_diffuse_light_end(
	void)
{
	return;
}

void _rasterizer_environment_diffuse_lights_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_diffuse_lights);

	return;
}

void _rasterizer_environment_diffuse_textures_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 921, global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_textures);

	if ((rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_overdraw ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_edge) &&
		rasterizer_debug_options.draw_environment_textures)
	{
		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, rasterizer_debug_options.environment_specular_mask_enabled ? D3DCOLORWRITEENABLE_ALL : D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_overdraw ? D3DBLEND_ONE : D3DBLEND_DESTCOLOR);
		SetRenderStateSmart(D3DRS_DESTBLEND, rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_overdraw ? D3DBLEND_ONE : D3DBLEND_ZERO);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, rasterizer_debug_options.drawing_mode != _rasterizer_drawing_mode_overdraw);
		SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_EQUAL);
		SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		rasterizer_set_stencil_mode(_rasterizer_stencil_mode_reject_alpha_tested_decal);
	}

	return;
}

void _rasterizer_environment_diffuse_texture_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 968, global_d3d_device);

	if ((rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_overdraw ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_edge) &&
		rasterizer_debug_options.draw_environment_textures)
	{
		struct shader_environment const *shader_environment;
		real_vector2d primary_detail_rescale;
		real_vector2d secondary_detail_rescale;
		real_vector2d micro_detail_rescale;
		point2d base_map_dimensions;
		point2d primary_detail_map_dimensions;
		point2d secondary_detail_map_dimensions;
		point2d micro_detail_map_dimensions;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 981, shader);

		shader_environment = shader_environment_get(shader);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 995, vertex_buffer);

		rasterizer_set_vertex_shader_permutation(_vs_environment_texture, vertex_buffer->type, shader_get_vertex_shader_permutation(shader));

		base_map_dimensions = *rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, shader_environment->environment.diffuse.base_map.index, shader_permutation_index);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		primary_detail_map_dimensions = *rasterizer_set_texture(1, _bitmap_type_2d, _bitmap_usage_detail, shader_environment->environment.diffuse.primary_detail_map.index, shader_permutation_index);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		secondary_detail_map_dimensions = *rasterizer_set_texture(2, _bitmap_type_2d, _bitmap_usage_detail, shader_environment->environment.diffuse.secondary_detail_map.index, shader_permutation_index);
		SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		micro_detail_map_dimensions = *rasterizer_set_texture(3, _bitmap_type_2d, _bitmap_usage_detail, shader_environment->environment.diffuse.micro_detail_map.index, shader_permutation_index);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		if (TEST_FLAG(shader_environment->environment.diffuse.flags, _shader_environment_diffuse_rescale_detail_maps_bit))
		{
			primary_detail_rescale.i = (real)base_map_dimensions.x / (real)primary_detail_map_dimensions.x;
			primary_detail_rescale.j = (real)base_map_dimensions.y / (real)primary_detail_map_dimensions.y;
			secondary_detail_rescale.i = (real)base_map_dimensions.x / (real)secondary_detail_map_dimensions.x;
			secondary_detail_rescale.j = (real)base_map_dimensions.y / (real)secondary_detail_map_dimensions.y;
			micro_detail_rescale.i = (real)base_map_dimensions.x / (real)micro_detail_map_dimensions.x;
			micro_detail_rescale.j = (real)base_map_dimensions.y / (real)micro_detail_map_dimensions.y;
		}
		else
		{
			micro_detail_rescale.j = 1.f;
			micro_detail_rescale.i = 1.f;
			secondary_detail_rescale.j = 1.f;
			secondary_detail_rescale.i = 1.f;
			primary_detail_rescale.j = 1.f;
			primary_detail_rescale.i = 1.f;
		}

		{
			real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
			{
				{shader_environment->environment.diffuse.primary_detail_map_scale * primary_detail_rescale.i, shader_environment->environment.diffuse.primary_detail_map_scale * primary_detail_rescale.j, shader_environment->environment.diffuse.secondary_detail_map_scale * secondary_detail_rescale.i, shader_environment->environment.diffuse.secondary_detail_map_scale * secondary_detail_rescale.j},
				{1.f, 0.f, shader_environment->environment.diffuse.micro_detail_map_scale * micro_detail_rescale.i, 0.f},
				{0.f, 1.f, shader_environment->environment.diffuse.micro_detail_map_scale * micro_detail_rescale.j, 0.f}
			};

			shader_environment_texture_animation_evaluate(
				shader,
				global_frame_parameters.game_time_sec,
				&vsh_constants__texscale[1][3],
				&vsh_constants__texscale[2][3]);
			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__TEXSCALE_OFFSET,
				vsh_constants__texscale,
				VSH_CONSTANTS__TEXSCALE_COUNT);
		}

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(3, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);

		switch (shader_environment->environment.type)
		{
		case _shader_environment_type_normal:
			pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T2 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T2 | PS_CHANNEL_ALPHA, PS_REGISTER_T2 | PS_CHANNEL_ALPHA, PS_REGISTER_T1 | PS_CHANNEL_ALPHA);
			pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T2 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T2, PS_REGISTER_T2 | PS_CHANNEL_ALPHA, PS_REGISTER_T1);
			pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			break;
		case _shader_environment_type_blended:
			pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T2 | PS_CHANNEL_ALPHA, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1 | PS_CHANNEL_ALPHA);
			pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T2, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1);
			pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			break;
		case _shader_environment_type_blended_base_specular:
			pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T2, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1);
			pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1145, FALSE, "### ERROR unsupported environment shader type");
		}

		switch (shader_environment->environment.diffuse.detail_map_function)
		{
		case _shader_environment_detail_function_double_biased_multiply:
			pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_R0, PS_REGISTER_T0, PS_REGISTER_R0);
			pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			break;
		case _shader_environment_detail_function_multiply:
			pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_R0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			break;
		case _shader_environment_detail_function_double_biased_add:
			pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_ONE, PS_REGISTER_R0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE);
			pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1163, FALSE, "### ERROR unsupported environment shader detail function");
		}

		pixel_shader.PSAlphaInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T3 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSAlphaOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);

		switch (shader_environment->environment.diffuse.micro_detail_map_function)
		{
		case _shader_environment_detail_function_double_biased_multiply:
			pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T3, PS_REGISTER_R0, PS_REGISTER_T3);
			pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			break;
		case _shader_environment_detail_function_multiply:
			pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T3, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			break;
		case _shader_environment_detail_function_double_biased_add:
			pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_ONE, PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE);
			pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1184, FALSE, "### ERROR unsupported environment shader detail function");
		}

		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);

		switch (rasterizer_debug_options.drawing_mode)
		{
		case _rasterizer_drawing_mode_normal:
		case _rasterizer_drawing_mode_diffuse_texture_times_bump_color:
		case _rasterizer_drawing_mode_diffuse_texture_times_bump_edge:
			pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
			break;
		case _rasterizer_drawing_mode_overdraw:
			pixel_shader.PSFinalCombinerConstant0 = real_alpha_to_pixel32(0.33f);
			pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			break;
		case _rasterizer_drawing_mode_specular_mask:
		case _rasterizer_drawing_mode_specular_mask_times_bump_color:
		case _rasterizer_drawing_mode_specular_mask_times_bump_edge:
			pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA);
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1206, FALSE, "### ERROR unsupported drawing mode in environment texture pass");
		}

		rasterizer_set_pixel_shader(&pixel_shader);

		rasterizer_draw_dynamic_triangles_static_vertices(
			dynamic_triangle_buffer_index,
			first_triangle_index,
			triangle_count,
			vertex_buffer);

		if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.environment_texture_primitive_count++;
			rasterizer_frame_statistics.environment_texture_triangle_count += triangle_count;
			rasterizer_frame_statistics.environment_texture_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count);
		}
	}

	return;
}

void _rasterizer_environment_diffuse_textures_end(
	void)
{
	rasterizer_set_stencil_mode(_rasterizer_stencil_mode_reject);
	rasterizer_profile_end(_rasterizer_profile_environment_textures);

	return;
}

void _rasterizer_environment_specular_lights_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1253, global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_specular_lights);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal && rasterizer_debug_options.draw_environment_specular_lights)
	{
		rasterizer_set_texture_direct(2, global_rasterizer_data->vector_normalization.index, 0);
		SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(2, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		rasterizer_set_texture_direct(3, global_rasterizer_data->vector_normalization.index, 0);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
		SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ALPHAREF, 0);
		SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_EQUAL);
		SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);
	}

	return;
}

static void rasterizer_environment_specular_spot_light_begin(
	long light_index)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1311, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal && rasterizer_debug_options.draw_environment_specular_lights)
	{
		struct rasterizer_light_submit_parameters const *light;
		struct point_light_definition const *definition;
		long gel_map_index;
		real_matrix4x3 light_matrix;
		real specular_radius;
		real near_distance;
		real far_distance;
		real depth_scale;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1317, light_index>=0 && light_index<rasterizer_lights.light_count);

		light = &rasterizer_lights.lights[light_index];

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1320, light->radius>0.0f);

		definition = light->definition;
		gel_map_index = definition->gel.map.index;

		if (gel_map_index == NONE)
		{
			gel_map_index = definition->gel.secondary_map.index;
		}

		light_matrix.forward = light->forward;
		light_matrix.up = light->up;
		cross_product3d(&light->forward, &light->up, &light_matrix.left);
		normalize3d(&light_matrix.left);

		specular_radius = light->radius * definition->geometry.specular_radius_multiplier;
		far_distance = specular_radius;
		near_distance = 0.5f * far_distance;
		depth_scale = 1.f / (far_distance - near_distance);

		{
			real vsh_constants__pointlight[VSH_CONSTANTS__POINTLIGHT_COUNT][4] =
			{
				{light->position.x, light->position.y, light->position.z, 0.5f / specular_radius},
				{-light_matrix.forward.i, -light_matrix.forward.j, -light_matrix.forward.k, 1.f},
				{-light_matrix.left.i, -light_matrix.left.j, -light_matrix.left.k, 1.f},
				{-light_matrix.up.i, -light_matrix.up.j, -light_matrix.up.k, 1.f},
				{light_matrix.forward.i * depth_scale, light_matrix.forward.j * depth_scale, light_matrix.forward.k * depth_scale, -near_distance * depth_scale}
			};

			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__POINTLIGHT_OFFSET,
				vsh_constants__pointlight,
				VSH_CONSTANTS__POINTLIGHT_COUNT);
		}

		rasterizer_set_texture(1, _bitmap_type_cube_map, _bitmap_usage_multiplicative, gel_map_index, 0);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(6, PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
		pixel_shader.PSConstant0[0] = D3DCOLOR_ARGB(0, 0, 0, 255);
		pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE, PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE);
		pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_V0, PS_COMBINEROUTPUT_SHIFTLEFT_2);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
		pixel_shader.PSConstant0[1] = D3DCOLOR_ARGB(0, 0, 0, 255);
		pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE, PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE);
		pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_V1, PS_COMBINEROUTPUT_SHIFTLEFT_2);
		pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_T2 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_NEGATIVE_ONE);
		pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_COMBINEROUTPUT_SHIFTLEFT_1);
		pixel_shader.PSAlphaInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
		pixel_shader.PSAlphaOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T0, PS_REGISTER_T1);
		pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
		pixel_shader.PSAlphaInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R1, PS_REGISTER_R1, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_V1 | PS_CHANNEL_ALPHA);
		pixel_shader.PSAlphaOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_V1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1, PS_REGISTER_R0, PS_REGISTER_C0);
		pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSAlphaInputs[4] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_V1 | PS_CHANNEL_ALPHA, PS_REGISTER_R0 | PS_CHANNEL_ALPHA);
		pixel_shader.PSAlphaOutputs[4] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_V1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[4] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T1, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1);
		pixel_shader.PSRGBOutputs[4] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_1 | PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
		pixel_shader.PSAlphaInputs[5] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSAlphaOutputs[5] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[5] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_V1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1, PS_REGISTER_R1 | PS_CHANNEL_ALPHA);
		pixel_shader.PSRGBOutputs[5] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_EF_PROD, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_FOG | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R1, PS_REGISTER_ZERO);
		rasterizer_set_pixel_shader(&pixel_shader);
	}

	return;
}

void _rasterizer_environment_specular_light_begin(
	long light_index)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1472, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal && rasterizer_debug_options.draw_environment_specular_lights)
	{
		struct rasterizer_light_submit_parameters const *light;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1478, light_index>=0 && light_index<rasterizer_lights.light_count);

		light = &rasterizer_lights.lights[light_index];

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1481, light->radius);

		if (light->definition->geometry.runtime_cosine_falloff_angle == -1.f || (light->definition->gel.map.index == NONE && light->definition->gel.secondary_map.index == NONE))
		{
			specular_light_vertex_shader_permutation_index = _environment_specular_light_vertex_shader_permutation_point_light;
			specular_light_brightness = real_rgb_color_brightness(&light->color);

			{
				real inverse_specular_radius = 1.f / (light->radius * light->definition->geometry.specular_radius_multiplier);
				real vsh_constants__pointlight[VSH_CONSTANTS__POINTLIGHT_COUNT][4] =
				{
					{light->position.x, light->position.y, light->position.z, 0.5f * inverse_specular_radius},
					{0.f, 0.f, 0.f, 1.f},
					{0.f, 0.f, 0.f, 1.f},
					{0.f, 0.f, 0.f, 1.f},
					{0.f, 0.f, 0.f, 1.f}
				};

				IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__POINTLIGHT_OFFSET,
					vsh_constants__pointlight,
					VSH_CONSTANTS__POINTLIGHT_COUNT);
			}

			rasterizer_set_texture_direct(1, global_rasterizer_data->distance_attenuation.index, 0);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSW, D3DTADDRESS_BORDER);
			SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			memset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT3D, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP);
			pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(6, PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
			pixel_shader.PSConstant0[0] = D3DCOLOR_ARGB(0, 0, 0, 255);
			pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE, PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE);
			pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_V0, PS_COMBINEROUTPUT_SHIFTLEFT_2);
			pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
			pixel_shader.PSConstant0[1] = D3DCOLOR_ARGB(0, 0, 0, 255);
			pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE, PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE);
			pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_V1, PS_COMBINEROUTPUT_SHIFTLEFT_2);
			pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_T2 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_NEGATIVE_ONE);
			pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_COMBINEROUTPUT_SHIFTLEFT_1);
			pixel_shader.PSAlphaInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
			pixel_shader.PSAlphaOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T0, PS_REGISTER_T1);
			pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
			pixel_shader.PSAlphaInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R1, PS_REGISTER_R1, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_V1 | PS_CHANNEL_ALPHA);
			pixel_shader.PSAlphaOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_V1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1, PS_REGISTER_R0, PS_REGISTER_C0);
			pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSAlphaInputs[4] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_V1 | PS_CHANNEL_ALPHA, PS_REGISTER_R0 | PS_CHANNEL_ALPHA);
			pixel_shader.PSAlphaOutputs[4] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_V1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[4] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T1, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1);
			pixel_shader.PSRGBOutputs[4] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_1 | PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
			pixel_shader.PSAlphaInputs[5] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSAlphaOutputs[5] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[5] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_V1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1, PS_REGISTER_R1 | PS_CHANNEL_ALPHA);
			pixel_shader.PSRGBOutputs[5] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
			pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_EF_PROD, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_R1, PS_REGISTER_ZERO);
			rasterizer_set_pixel_shader(&pixel_shader);
		}
		else
		{
			specular_light_vertex_shader_permutation_index = _environment_specular_light_vertex_shader_permutation_spot_light;
			specular_light_brightness = real_rgb_color_brightness(&light->color);
			rasterizer_environment_specular_spot_light_begin(light_index);
		}
	}

	return;
}

void _rasterizer_environment_specular_light_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1632, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal && rasterizer_debug_options.draw_environment_specular_lights)
	{
		struct shader_environment const *shader_environment;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1639, shader);

		shader_environment = shader_environment_get(shader);

		if (shader_environment->environment.specular.brightness > 0.f && specular_light_brightness > 0.f)
		{
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1645, vertex_buffer);

			rasterizer_set_vertex_shader_permutation(_vs_environment_specular_light, vertex_buffer->type, specular_light_vertex_shader_permutation_index);

			rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_vector, shader_environment->environment.diffuse.bump_map.index, shader_permutation_index);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			{
				real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
				{
					{shader_environment->environment.diffuse.runtime_bump_map_scale.i, shader_environment->environment.diffuse.runtime_bump_map_scale.j, 1.f, 1.f},
					{1.f, 0.f, 0.f, 0.f},
					{0.f, 1.f, 0.f, 0.f}
				};

				shader_environment_texture_animation_evaluate(
					shader,
					global_frame_parameters.game_time_sec,
					&vsh_constants__texscale[1][3],
					&vsh_constants__texscale[2][3]);
				IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__TEXSCALE_OFFSET,
					vsh_constants__texscale,
					VSH_CONSTANTS__TEXSCALE_COUNT);
			}

			pixel_shader.PSConstant0[2] = real_alpha_to_pixel32(specular_light_brightness * shader_environment->environment.specular.brightness);
			pixel_shader.PSConstant1[2] = real_alpha_to_pixel32(specular_light_brightness * shader_environment->environment.specular.brightness);
			pixel_shader.PSConstant0[3] = real_rgb_color_to_pixel32(&shader_environment->environment.specular.view_perpendicular_color);
			pixel_shader.PSConstant1[3] = real_rgb_color_to_pixel32(&shader_environment->environment.specular.view_parallel_color);

			if (TEST_FLAG(shader_environment->environment.flags, _shader_environment_bump_map_is_specular_mask_bit))
			{
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_T2 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_NEGATIVE_ONE);
				pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_T1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
			}
			else
			{
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_T2 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_NEGATIVE_ONE);
				pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
			}

			pixel_shader.PSRGBOutputs[4] = TEST_FLAG(shader_environment->environment.specular.flags, _shader_environment_specular_overbright_bit) ?
				PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_2 | PS_COMBINEROUTPUT_CD_DOT_PRODUCT) :
				PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_1 | PS_COMBINEROUTPUT_CD_DOT_PRODUCT);

			if (TEST_FLAG(shader_environment->environment.specular.flags, _shader_environment_specular_extra_shiny_bit))
			{
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(8, PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
				pixel_shader.PSAlphaInputs[6] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[6] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSAlphaInputs[7] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[7] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			}
			else
			{
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(6, PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
				pixel_shader.PSAlphaInputs[6] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[6] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSAlphaInputs[7] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[7] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			}

			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_draw_dynamic_triangles_static_vertices(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				vertex_buffer);

			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.environment_specular_light_primitive_count++;
				rasterizer_frame_statistics.environment_specular_light_triangle_count += triangle_count;
				rasterizer_frame_statistics.environment_specular_light_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count);
			}
		}
	}

	return;
}

void _rasterizer_environment_specular_light_end(
	void)
{
	return;
}

void _rasterizer_environment_specular_lights_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_specular_lights);

	return;
}

void _rasterizer_environment_specular_lightmaps_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1785, global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_specular_lightmaps);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_specular_lightmaps &&
		rasterizer_globals.lightmap_mode==_rasterizer_lightmap_mode_normal)
	{
		rasterizer_set_texture_direct(2, global_rasterizer_data->vector_normalization.index, 0);
		SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(2, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		rasterizer_set_texture_direct(3, global_rasterizer_data->vector_normalization.index, 0);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
		SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ALPHAREF, 0);
		SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_EQUAL);
		SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(6, PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
		pixel_shader.PSConstant0[0] = D3DCOLOR_ARGB(128, 0, 0, 255);
		pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE, PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE);
		pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_V0, PS_COMBINEROUTPUT_SHIFTLEFT_2);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T1, PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_T1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT | PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
		pixel_shader.PSConstant0[1] = D3DCOLOR_ARGB(0, 0, 0, 255);
		pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE, PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE);
		pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_V1, PS_COMBINEROUTPUT_SHIFTLEFT_2);
		pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_T2 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_NEGATIVE_ONE);
		pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_COMBINEROUTPUT_SHIFTLEFT_1);
		pixel_shader.PSAlphaInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
		pixel_shader.PSAlphaOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T0, PS_REGISTER_T1);
		pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
		pixel_shader.PSAlphaInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R1, PS_REGISTER_R1, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_V1 | PS_CHANNEL_ALPHA);
		pixel_shader.PSAlphaOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_V1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1, PS_REGISTER_R0, PS_REGISTER_C0);
		pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSAlphaInputs[4] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_V1 | PS_CHANNEL_ALPHA, PS_REGISTER_R0 | PS_CHANNEL_ALPHA);
		pixel_shader.PSAlphaOutputs[4] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_V1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[4] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T1, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1);
		pixel_shader.PSRGBOutputs[4] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_1 | PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
		pixel_shader.PSAlphaInputs[5] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSAlphaOutputs[5] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[5] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_V1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1, PS_REGISTER_R1 | PS_CHANNEL_ALPHA);
		pixel_shader.PSRGBOutputs[5] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
		pixel_shader.PSFinalCombinerConstant0 = D3DCOLOR_ARGB(128, 0, 0, 0);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_EF_PROD, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_R1, PS_REGISTER_ZERO);
		rasterizer_set_pixel_shader(&pixel_shader);
	}

	return;
}

void _rasterizer_environment_specular_lightmap_begin(
	struct bitmap_data const *lightmap)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1874, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal && rasterizer_debug_options.draw_environment_specular_lightmaps)
	{
		if (lightmap)
		{
			rasterizer_set_texture_bitmap_data(1, lightmap);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, rasterizer_debug_options.lightmap_filtering_enabled ? D3DTEXF_LINEAR : D3DTEXF_POINT);
			SetTextureStageStateSmart(1, D3DTSS_MINFILTER, rasterizer_debug_options.lightmap_filtering_enabled ? D3DTEXF_LINEAR : D3DTEXF_POINT);
			SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, rasterizer_debug_options.lightmap_filtering_enabled ? D3DTEXF_LINEAR : D3DTEXF_POINT);
			local_lightmap_has_no_data = FALSE;
		}
		else
		{
			local_lightmap_has_no_data = TRUE;
		}
	}

	return;
}

void _rasterizer_environment_specular_lightmap_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1915, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_specular_lightmaps &&
		rasterizer_globals.lightmap_mode==_rasterizer_lightmap_mode_normal &&
		!local_lightmap_has_no_data)
	{
		struct shader_environment const *shader_environment;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 1924, shader);

		shader_environment = shader_environment_get(shader);

		if (shader_environment->environment.specular.brightness > 0.f && TEST_FLAG(shader_environment->environment.specular.flags, _shader_environment_specular_lightmap_bit))
		{
			rasterizer_set_vertex_shader_permutation(_vs_environment_specular_light, vertex_buffer->type, _environment_specular_light_vertex_shader_permutation_lightmap);

			rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_vector, shader_environment->environment.diffuse.bump_map.index, shader_permutation_index);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			{
				real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
				{
					{shader_environment->environment.diffuse.runtime_bump_map_scale.i, shader_environment->environment.diffuse.runtime_bump_map_scale.j, 1.f, 1.f},
					{1.f, 0.f, 0.f, 0.f},
					{0.f, 1.f, 0.f, 0.f}
				};

				shader_environment_texture_animation_evaluate(
					shader,
					global_frame_parameters.game_time_sec,
					&vsh_constants__texscale[1][3],
					&vsh_constants__texscale[2][3]);
				IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__TEXSCALE_OFFSET,
					vsh_constants__texscale,
					VSH_CONSTANTS__TEXSCALE_COUNT);
			}

			pixel_shader.PSConstant0[2] = real_alpha_to_pixel32(shader_environment->environment.specular.brightness);
			pixel_shader.PSConstant1[2] = real_alpha_to_pixel32(shader_environment->environment.specular.brightness);
			pixel_shader.PSConstant0[3] = real_rgb_color_to_pixel32(&shader_environment->environment.specular.view_perpendicular_color);
			pixel_shader.PSConstant1[3] = real_rgb_color_to_pixel32(&shader_environment->environment.specular.view_parallel_color);

			if (TEST_FLAG(shader_environment->environment.flags, _shader_environment_bump_map_is_specular_mask_bit))
			{
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T1, PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
				pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_T2 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_NEGATIVE_ONE);
				pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_T1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
			}
			else
			{
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T1, PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
				pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_T2 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_NEGATIVE_ONE);
				pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
			}

			pixel_shader.PSRGBOutputs[4] = TEST_FLAG(shader_environment->environment.specular.flags, _shader_environment_specular_overbright_bit) ?
				PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_2 | PS_COMBINEROUTPUT_CD_DOT_PRODUCT) :
				PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_1 | PS_COMBINEROUTPUT_CD_DOT_PRODUCT);

			if (TEST_FLAG(shader_environment->environment.specular.flags, _shader_environment_specular_extra_shiny_bit))
			{
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(8, PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
				pixel_shader.PSAlphaInputs[6] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[6] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSAlphaInputs[7] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[7] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			}
			else
			{
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(6, PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
				pixel_shader.PSAlphaInputs[6] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[6] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSAlphaInputs[7] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[7] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			}

			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_draw_dynamic_triangles_static_vertices2(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				&vertex_buffer[0],
				&vertex_buffer[1]);

			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.environment_specular_lightmap_primitive_count++;
				rasterizer_frame_statistics.environment_specular_lightmap_triangle_count += triangle_count;
				rasterizer_frame_statistics.environment_specular_lightmap_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count);
			}
		}
	}

	return;
}

void _rasterizer_environment_specular_lightmap_end(
	void)
{
	return;
}

void _rasterizer_environment_specular_lightmaps_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_specular_lightmaps);

	return;
}

void _rasterizer_environment_reflection_lightmap_masks_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2069, global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_reflection_lightmap_masks);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_reflection_lightmap_masks &&
		rasterizer_debug_options.draw_environment_reflections &&
		rasterizer_globals.lightmap_mode==_rasterizer_lightmap_mode_normal)
	{
		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
		SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_EQUAL);
		SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(2, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
		pixel_shader.PSConstant0[0] = D3DCOLOR_ARGB(0, 128, 176, 80);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_C0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
		pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_R0, PS_REGISTER_ONE);
		pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);
		rasterizer_set_pixel_shader(&pixel_shader);
	}

	return;
}

void _rasterizer_environment_reflection_lightmap_mask_begin(
	struct bitmap_data const *lightmap)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2120, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_reflection_lightmap_masks &&
		rasterizer_debug_options.draw_environment_reflections &&
		rasterizer_globals.lightmap_mode==_rasterizer_lightmap_mode_normal)
	{
		if (lightmap)
		{
			rasterizer_set_texture_bitmap_data(0, lightmap);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, rasterizer_debug_options.lightmap_filtering_enabled ? D3DTEXF_LINEAR : D3DTEXF_POINT);
			SetTextureStageStateSmart(0, D3DTSS_MINFILTER, rasterizer_debug_options.lightmap_filtering_enabled ? D3DTEXF_LINEAR : D3DTEXF_POINT);
			SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, rasterizer_debug_options.lightmap_filtering_enabled ? D3DTEXF_LINEAR : D3DTEXF_POINT);
			local_lightmap_has_no_data = FALSE;
		}
		else
		{
			local_lightmap_has_no_data = TRUE;
		}
	}

	return;
}

void _rasterizer_environment_reflection_lightmap_mask_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2163, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_reflection_lightmap_masks &&
		rasterizer_debug_options.draw_environment_reflections &&
		rasterizer_globals.lightmap_mode==_rasterizer_lightmap_mode_normal &&
		!local_lightmap_has_no_data)
	{
		struct shader_environment const *shader_environment;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2173, shader);

		shader_environment = shader_environment_get(shader);

		if ((shader_environment->environment.reflection.view_perpendicular_brightness > 0.f || shader_environment->environment.reflection.view_parallel_brightness > 0.f) &&
			shader_environment->environment.reflection.lightmap_brightness_scale < 1.f)
		{
			rasterizer_set_vertex_shader_permutation(_vs_environment_reflection_lightmap_mask, vertex_buffer->type, _vertex_shader_permutation_default);

			{
				real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
				{
					{shader_environment->environment.diffuse.runtime_bump_map_scale.i, shader_environment->environment.diffuse.runtime_bump_map_scale.j, 1.f, 1.f},
					{1.f, 0.f, 0.f, 0.f},
					{0.f, 1.f, 0.f, 0.f}
				};

				shader_environment_texture_animation_evaluate(
					shader,
					global_frame_parameters.game_time_sec,
					&vsh_constants__texscale[1][3],
					&vsh_constants__texscale[2][3]);
				IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__TEXSCALE_OFFSET,
					vsh_constants__texscale,
					VSH_CONSTANTS__TEXSCALE_COUNT);
			}

			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSCONSTANT1_0, real_alpha_to_pixel32(shader_environment->environment.reflection.lightmap_brightness_scale));

			rasterizer_draw_dynamic_triangles_static_vertices2(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				&vertex_buffer[0],
				&vertex_buffer[1]);

			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.environment_reflection_lightmap_mask_primitive_count++;
				rasterizer_frame_statistics.environment_reflection_lightmap_mask_triangle_count += triangle_count;
				rasterizer_frame_statistics.environment_reflection_lightmap_mask_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count);
			}
		}
	}

	return;
}

void _rasterizer_environment_reflection_lightmap_mask_end(
	void)
{
	return;
}

void _rasterizer_environment_reflection_lightmap_masks_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_reflection_lightmap_masks);

	return;
}

void _rasterizer_environment_reflection_mirrors_begin(
	void)
{
	rasterizer_profile_begin(_rasterizer_profile_environment_reflection_mirrors);

	return;
}

void _rasterizer_environment_reflection_mirror_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2271, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_reflection_mirrors &&
		global_window_parameters.has_mirror &&
		global_window_parameters.rasterizer_target == _rasterizer_target_render_primary)
	{
		struct shader_environment const *shader_environment;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2280, shader);

		shader_environment = shader_environment_get(shader);

		if (TEST_FLAG(shader_environment->environment.reflection.flags, _shader_environment_reflection_mirror_bit) &&
			(shader_environment->environment.reflection.view_perpendicular_brightness > 0.f || shader_environment->environment.reflection.view_parallel_brightness > 0.f))
		{
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2287, vertex_buffer);

			rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_vector, shader_environment->environment.diffuse.bump_map.index, shader_permutation_index);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
			rasterizer_set_texture_direct(1, global_rasterizer_data->vector_normalization.index, 0);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
			SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_POINT);
			rasterizer_set_texture_direct(2, global_rasterizer_data->vector_normalization.index, 0);
			SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(2, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(2, D3DTSS_MINFILTER, D3DTEXF_POINT);
			SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, D3DTEXF_POINT);
			rasterizer_set_target_as_texture(3, _rasterizer_target_render_secondary, 0);
			SetTextureStageStateSmart(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
			SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
			SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
			SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
			SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
			SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
			SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_EQUAL);
			SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ZBIAS, 0);

			rasterizer_set_vertex_shader_permutation(_vs_environment_reflection_mirror, vertex_buffer->type, _vertex_shader_permutation_default);

			{
				real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
				{
					{shader_environment->environment.diffuse.runtime_bump_map_scale.i, shader_environment->environment.diffuse.runtime_bump_map_scale.j, RASTERIZER_TARGET_RENDER_SECONDARY_WIDTH, RASTERIZER_TARGET_RENDER_SECONDARY_HEIGHT},
					{1.f, 0.f, 0.f, 0.f},
					{0.f, 1.f, 0.f, 0.f}
				};

				shader_environment_texture_animation_evaluate(
					shader,
					global_frame_parameters.game_time_sec,
					&vsh_constants__texscale[1][3],
					&vsh_constants__texscale[2][3]);
				IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__TEXSCALE_OFFSET,
					vsh_constants__texscale,
					VSH_CONSTANTS__TEXSCALE_COUNT);
			}

			memset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_PROJECT2D);
			pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(5, PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);

			if (shader_environment->environment.diffuse.bump_map.index == NONE)
			{
				real_rgb_color flat_normal;

				flat_normal.red = PIN(0.5f - 0.5f * global_window_parameters.camera.forward.i, 0.f, 1.f);
				flat_normal.green = PIN(0.5f - 0.5f * global_window_parameters.camera.forward.j, 0.f, 1.f);
				flat_normal.blue = PIN(0.5f - 0.5f * global_window_parameters.camera.forward.k, 0.f, 1.f);
				pixel_shader.PSConstant0[0] = real_rgb_color_to_pixel32(&flat_normal);
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_C0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T3, PS_REGISTER_T3);
			}
			else
			{
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T1 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T3, PS_REGISTER_T3);
			}

			pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
			pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_R1);
			pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_R1);
			pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSConstant0[3] = real_a_rgb_color_to_pixel32(shader_environment->environment.reflection.view_perpendicular_brightness, &shader_environment->environment.specular.view_perpendicular_color);
			pixel_shader.PSConstant1[3] = real_a_rgb_color_to_pixel32(shader_environment->environment.reflection.view_parallel_brightness, &shader_environment->environment.specular.view_parallel_color);
			pixel_shader.PSAlphaInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
			pixel_shader.PSAlphaOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1, PS_REGISTER_R0, PS_REGISTER_C0);
			pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[4] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R1, PS_REGISTER_R0, PS_REGISTER_T3);
			pixel_shader.PSRGBOutputs[4] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_EF_PROD, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, TEST_FLAG(shader_environment->environment.flags, _shader_environment_bump_map_is_specular_mask_bit) ? PS_REGISTER_T0 : PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_draw_dynamic_triangles_static_vertices(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				vertex_buffer);

			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.environment_reflection_primitive_count++;
				rasterizer_frame_statistics.environment_reflection_triangle_count += triangle_count;
				rasterizer_frame_statistics.environment_reflection_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count);
			}
		}
	}

	return;
}

void _rasterizer_environment_reflection_mirrors_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_reflection_mirrors);

	return;
}

void _rasterizer_environment_reflections_begin(
	void)
{
	rasterizer_profile_begin(_rasterizer_profile_environment_reflections);

	return;
}

void _rasterizer_environment_reflection_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2462, global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal && rasterizer_debug_options.draw_environment_reflections)
	{
		struct shader_environment const *shader_environment;
		short reflection_type;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2470, shader);

		shader_environment = shader_environment_get(shader);
		reflection_type = shader_environment->environment.reflection.type;

		if (reflection_type == _shader_environment_reflection_type_bumped || reflection_type == _shader_environment_reflection_type_radiosity)
		{
			if (TEST_FLAG(shader_environment->environment.flags, _shader_environment_bump_map_is_specular_mask_bit))
			{
				reflection_type = _shader_environment_reflection_type_flat;
			}

			if (shader_environment->environment.diffuse.bump_map.index == NONE)
			{
				reflection_type = _shader_environment_reflection_type_flat;
			}
		}

		if ((shader_environment->environment.reflection.view_perpendicular_brightness > 0.f || shader_environment->environment.reflection.view_parallel_brightness > 0.f) &&
			shader_environment->environment.reflection.map.index != NONE)
		{
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2495, vertex_buffer);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2496, reflection_type>=0 && reflection_type<NUMBER_OF_SHADER_ENVIRONMENT_REFLECTION_TYPES);

			rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_vector, shader_environment->environment.diffuse.bump_map.index, shader_permutation_index);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
			rasterizer_set_texture_direct(1, global_rasterizer_data->vector_normalization.index, 0);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
			SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_POINT);
			rasterizer_set_texture_direct(2, global_rasterizer_data->vector_normalization.index, 0);
			SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(2, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(2, D3DTSS_MINFILTER, D3DTEXF_POINT);
			SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, D3DTEXF_POINT);
			rasterizer_set_texture(3, _bitmap_type_cube_map, _bitmap_usage_additive, shader_environment->environment.reflection.map.index, shader_permutation_index);
			SetTextureStageStateSmart(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
			SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
			SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
			SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
			SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
			SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
			SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_EQUAL);
			SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ZBIAS, 0);

			rasterizer_set_vertex_shader_permutation(_vs_environment_reflection_flat, vertex_buffer->type, reflection_type);

			{
				real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
				{
					{shader_environment->environment.diffuse.runtime_bump_map_scale.i, shader_environment->environment.diffuse.runtime_bump_map_scale.j, RASTERIZER_TARGET_RENDER_SECONDARY_WIDTH, RASTERIZER_TARGET_RENDER_SECONDARY_HEIGHT},
					{1.f, 0.f, 0.f, 0.f},
					{0.f, 1.f, 0.f, 0.f}
				};

				shader_environment_texture_animation_evaluate(
					shader,
					global_frame_parameters.game_time_sec,
					&vsh_constants__texscale[1][3],
					&vsh_constants__texscale[2][3]);
				IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__TEXSCALE_OFFSET,
					vsh_constants__texscale,
					VSH_CONSTANTS__TEXSCALE_COUNT);
			}

			memset(&pixel_shader, 0, sizeof(pixel_shader));

			switch (reflection_type)
			{
			case _shader_environment_reflection_type_bumped:
			case _shader_environment_reflection_type_radiosity:
				pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_DOTPRODUCT, PS_TEXTUREMODES_DOT_RFLCT_DIFF, PS_TEXTUREMODES_DOT_RFLCT_SPEC);
				pixel_shader.PSInputTexture = PS_INPUTTEXTURE(0, 0, 0, 0);
				pixel_shader.PSDotMapping = PS_DOTMAPPING(PS_DOTMAPPING_ZERO_TO_ONE, PS_DOTMAPPING_MINUS1_TO_1_D3D, PS_DOTMAPPING_MINUS1_TO_1_D3D, PS_DOTMAPPING_MINUS1_TO_1_D3D);
				break;
			case _shader_environment_reflection_type_flat:
				pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP);
				break;
			default:
				match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2592, FALSE, "### ERROR unsupported reflection type");
			}

			pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(5, PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);

			if (reflection_type != _shader_environment_reflection_type_bumped &&
				reflection_type != _shader_environment_reflection_type_radiosity &&
				shader_environment->environment.diffuse.bump_map.index != NONE)
			{
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T1 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T3, PS_REGISTER_T3);
			}
			else
			{
				real_rgb_color flat_normal;

				flat_normal.red = PIN(0.5f - 0.5f * global_window_parameters.camera.forward.i, 0.f, 1.f);
				flat_normal.green = PIN(0.5f - 0.5f * global_window_parameters.camera.forward.j, 0.f, 1.f);
				flat_normal.blue = PIN(0.5f - 0.5f * global_window_parameters.camera.forward.k, 0.f, 1.f);
				pixel_shader.PSConstant0[0] = real_rgb_color_to_pixel32(&flat_normal);
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_C0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T3, PS_REGISTER_T3);
			}

			pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
			pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_R1);
			pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_R1);
			pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSConstant0[3] = real_a_rgb_color_to_pixel32(shader_environment->environment.reflection.view_perpendicular_brightness, &shader_environment->environment.specular.view_perpendicular_color);
			pixel_shader.PSConstant1[3] = real_a_rgb_color_to_pixel32(shader_environment->environment.reflection.view_parallel_brightness, &shader_environment->environment.specular.view_parallel_color);
			pixel_shader.PSAlphaInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
			pixel_shader.PSAlphaOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1, PS_REGISTER_R0, PS_REGISTER_C0);
			pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[4] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R1, PS_REGISTER_R0, PS_REGISTER_T3);
			pixel_shader.PSRGBOutputs[4] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_EF_PROD, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, TEST_FLAG(shader_environment->environment.flags, _shader_environment_bump_map_is_specular_mask_bit) ? PS_REGISTER_T0 : PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_draw_dynamic_triangles_static_vertices2(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				&vertex_buffer[0],
				&vertex_buffer[rasterizer_globals.lightmap_mode!=_rasterizer_lightmap_mode_normal && reflection_type==_shader_environment_reflection_type_radiosity]);

			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.environment_reflection_primitive_count++;
				rasterizer_frame_statistics.environment_reflection_triangle_count += triangle_count;
				rasterizer_frame_statistics.environment_reflection_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count);
			}
		}
	}

	return;
}

void _rasterizer_environment_reflections_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_reflections);

	return;
}

void _rasterizer_environment_transparent_geometry_begin(
	void)
{
	rasterizer_profile_begin(_rasterizer_profile_environment_transparents);
	rasterizer_transparent_geometry_groups_begin();

	return;
}

void _rasterizer_environment_transparent_geometry_submit(
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
	static struct transparent_geometry_group local_group;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2699, global_d3d_device);

	if (rasterizer_debug_options.draw_environment_transparent_geometry)
	{
		struct transparent_geometry_group *group;
		real_vector3d forward;
		real_plane3d zero_plane;
		static boolean warned = FALSE;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2706, shader);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2707, shader->base.type!=_shader_type_environment);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2708, shader_type_is_valid_for_environment(shader->base.type));
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2709, centroid);

		vector_from_points3d(&global_window_parameters.camera.position, centroid, &forward);

		if (plane)
		{
			SET_FLAG(geometry_flags, _rasterizer_geometry_no_sort_bit, TRUE);
		}

		if (shader_is_decal(shader))
		{
			SET_FLAG(geometry_flags, _rasterizer_geometry_no_sort_bit, TRUE);
			SET_FLAG(geometry_flags, _rasterizer_geometry_no_queue_bit, TRUE);
			SET_FLAG(geometry_flags, _rasterizer_geometry_no_fog_bit, TRUE);
		}

		if (TEST_FLAG(geometry_flags, _rasterizer_geometry_no_queue_bit))
		{
			group = &local_group;
			group->sorted_index = NONE;
		}
		else
		{
			group = rasterizer_transparent_geometry_new_group();
		}

		if (group)
		{
			zero_plane.n.i = 0.f;
			zero_plane.n.j = 0.f;
			zero_plane.n.k = 0.f;
			zero_plane.d = 0.f;

			group->geometry_flags = geometry_flags;
			group->shader = shader;
			group->shader_permutation_index = shader_permutation_index;
			group->dynamic_triangle_buffer_index = dynamic_triangle_buffer_index;
			group->first_triangle_index = first_triangle_index;
			group->triangle_count = triangle_count;
			group->vertex_buffers = vertex_buffers;
			group->lightmap = lightmap;
			group->object_index = 0;
			group->source_object_index = 0;
			group->effect.type = _render_model_effect_type_none;
			group->triangle_buffer = NULL;
			group->dynamic_vertex_buffer_index = NONE;
			group->z_sort = -dot_product3d(&global_window_parameters.camera.forward, &forward);
			group->centroid = *centroid;
			group->plane = plane ? *plane : zero_plane;
			group->prev_group_presorted_index = NONE;
			group->next_group_presorted_index = NONE;
			group->model_base_map_scale.j = 1.f;
			group->model_base_map_scale.i = 1.f;
			group->active_camouflage_transparent_source_object_index = 0;
			group->cortana_hack = FALSE;
			group->node_matrices = NULL;
			group->node_matrix_count = 0;
			group->lighting = rasterizer_memory_alloc_const(lighting, sizeof(*lighting));
			group->animation = NULL;

			if (shader->base.type == _shader_type_transparent_water)
			{
				rasterizer_water_set_visibility_for_window(TRUE);
			}

			if (shader->base.type == _shader_type_transparent_water && TEST_FLAG(shader_transparent_water_get(shader)->water.flags, _shader_transparent_water_draw_before_fog_bit))
			{
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c", 2782, !TEST_FLAG(geometry_flags, _rasterizer_geometry_no_queue_bit));

				SET_FLAG(group->geometry_flags, _rasterizer_geometry_no_queue_bit, TRUE);
				rasterizer_transparent_geometry_group_draw(group, FALSE);
				rasterizer_transparent_geometry_set_group_pending_status(group, TRUE);
				SET_FLAG(group->geometry_flags, _rasterizer_geometry_no_queue_bit, FALSE);
			}
			else if (TEST_FLAG(geometry_flags, _rasterizer_geometry_no_queue_bit))
			{
				rasterizer_transparent_geometry_group_draw(group, FALSE);
			}

			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.environment_transparent_primitive_count++;
				rasterizer_frame_statistics.environment_transparent_triangle_count += triangle_count;

				if (triangle_count > rasterizer_frame_statistics.environment_transparent_maximum_primitive_triangle_count)
				{
					rasterizer_frame_statistics.environment_transparent_maximum_primitive_triangle_count = triangle_count;
				}

				rasterizer_frame_statistics.environment_transparent_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count);
			}
		}
		else if (!warned)
		{
			error(_error_silent, "### ERROR too many transparent geometry groups");
			warned = TRUE;
		}
	}

	return;
}

void _rasterizer_environment_transparent_geometry_end(
	void)
{
	rasterizer_transparent_geometry_groups_end();
	rasterizer_profile_end(_rasterizer_profile_environment_transparents);

	return;
}
