/*
RASTERIZER_XBOX_WATER.C
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

enum
{
	MAXIMUM_WATER_RIPPLES = 4 /* fake name */
};

/* ---------- prototypes */

void rasterizer_water_build_bumpmap(struct shader const *shader);

/* ---------- globals */

static boolean water_needs_update_flag = FALSE;
static boolean water_visible_for_window_flag = FALSE;

/* ---------- public code */

void rasterizer_water_set_visibility_for_frame(
	boolean visibility)
{
	water_needs_update_flag = !visibility;
	water_visible_for_window_flag = visibility;

	return;
}

void rasterizer_water_set_visibility_for_window(
	boolean visibility)
{
	water_visible_for_window_flag = visibility;

	return;
}

boolean rasterizer_water_get_visibility_for_window(
	void)
{
	return water_visible_for_window_flag;
}

void rasterizer_water_build_bumpmap(
	struct shader const *shader)
{
	boolean success;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_water.c", 47, shader);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_water.c", 48, global_d3d_device);

	if (rasterizer_debug_options.draw_water)
	{
		struct shader_transparent_water_ripple ripples[MAXIMUM_WATER_RIPPLES];
		real vsh_constants__texanim[VSH_CONSTANTS__TEXANIM_COUNT][4];
		short ripple_index;
		short mipmap_index;
		struct shader_transparent_water const *water = shader_transparent_water_get(shader);
		short mipmap_levels = MIN(water->water.ripple_mipmap_levels, RASTERIZER_TARGET_WATER_MAX_MIPMAP_LEVELS);

		for (ripple_index = 0; ripple_index<MAXIMUM_WATER_RIPPLES; ripple_index++)
		{
			if (ripple_index<water->water.ripples.count)
			{
				ripples[ripple_index] = *TAG_BLOCK_GET_ELEMENT(&water->water.ripples, ripple_index, struct shader_transparent_water_ripple);
			}
			else
			{
				memset(&ripples[ripple_index], 0, sizeof(ripples[ripple_index]));
				ripples[ripple_index].map_repeats = 1;
			}
		}

		if (ripples[0].contibution_factor==0.f && ripples[1].contibution_factor==0.f)
		{
			ripples[1].contibution_factor = 1.f;
		}

		if (ripples[2].contibution_factor==0.f && ripples[3].contibution_factor==0.f)
		{
			ripples[3].contibution_factor = 1.f;
		}

		for (ripple_index = 0; ripple_index<MAXIMUM_WATER_RIPPLES; ripple_index++)
		{
			rasterizer_set_texture(ripple_index, _bitmap_type_2d, _bitmap_usage_vector, ripple_index<water->water.ripples.count ? water->water.ripple_maps.index : NONE, ripples[ripple_index].map_index);
			SetTextureStageStateSmart(ripple_index, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(ripple_index, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(ripple_index, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(ripple_index, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(ripple_index, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		}

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		rasterizer_set_vertex_shader_permutation(_vertex_shader_convolution, _rasterizer_vertex_type_dynamic_screen, _vertex_shader_permutation_default);

		for (ripple_index = 0; ripple_index<MAXIMUM_WATER_RIPPLES; ripple_index++)
		{
			real cosine_angle = cosine(ripples[ripple_index].animation_angle);
			real sine_angle = sine(ripples[ripple_index].animation_angle);

			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_water.c", 124, ripples[ripple_index].map_repeats>0);

			vsh_constants__texanim[2 * ripple_index][0] = (real)ripples[ripple_index].map_repeats;
			vsh_constants__texanim[2 * ripple_index][1] = 0.f;
			vsh_constants__texanim[2 * ripple_index][2] = 0.f;
			vsh_constants__texanim[2 * ripple_index][3] = global_frame_parameters.game_time_sec * ripples[ripple_index].animation_velocity * cosine_angle + ripples[ripple_index].map_offset.i;
			vsh_constants__texanim[2 * ripple_index + 1][0] = 0.f;
			vsh_constants__texanim[2 * ripple_index + 1][1] = (real)ripples[ripple_index].map_repeats;
			vsh_constants__texanim[2 * ripple_index + 1][2] = 0.f;
			vsh_constants__texanim[2 * ripple_index + 1][3] = global_frame_parameters.game_time_sec * ripples[ripple_index].animation_velocity * sine_angle + ripples[ripple_index].map_offset.j;
		}

		success = TRUE;

		D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
			VSH_CONSTANTS__TEXANIM_OFFSET,
			vsh_constants__texanim,
			VSH_CONSTANTS__TEXANIM_COUNT));

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(4, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1 | PS_INPUTMAPPING_EXPAND_NORMAL);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T3 | PS_INPUTMAPPING_EXPAND_NORMAL);
		pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_INPUTMAPPING_SIGNED_IDENTITY);
		pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTRIGHT_1);
		pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_ONE, PS_REGISTER_ONE_HALF, PS_REGISTER_ONE);
		pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_C0, PS_REGISTER_ZERO);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_water.c", 159, ripples[0].contibution_factor + ripples[1].contibution_factor>0.0f);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_water.c", 160, ripples[2].contibution_factor + ripples[3].contibution_factor>0.0f);

		pixel_shader.PSConstant0[0] = real_alpha_to_pixel32(ripples[0].contibution_factor / (ripples[0].contibution_factor + ripples[1].contibution_factor));
		pixel_shader.PSConstant0[1] = real_alpha_to_pixel32(ripples[2].contibution_factor / (ripples[2].contibution_factor + ripples[3].contibution_factor));
		pixel_shader.PSConstant0[2] = real_alpha_to_pixel32((ripples[0].contibution_factor + ripples[1].contibution_factor) / (ripples[0].contibution_factor + ripples[1].contibution_factor + ripples[2].contibution_factor + ripples[3].contibution_factor));
		rasterizer_set_pixel_shader(&pixel_shader);

		rasterizer_set_stencil_mode(_rasterizer_stencil_mode_none);

		for (mipmap_index = 0; mipmap_index<mipmap_levels; mipmap_index++)
		{
			if (water->water.ripple_mipmap_levels>1)
			{
				pixel_shader.PSFinalCombinerConstant0 = real_alpha_to_pixel32((real)mipmap_index / (water->water.ripple_mipmap_levels - 1) * water->water.ripple_mipmap_fade_factor) | 0x008080FF;
			}
			else
			{
				pixel_shader.PSFinalCombinerConstant0 = 0x007F7FFF;
			}

			rasterizer_set_pixel_shader(&pixel_shader);
			rasterizer_set_target(_rasterizer_target_water, mipmap_index, 0, FALSE, FALSE);

			{
				real scale = 1.f / (real)(RASTERIZER_TARGET_WATER_SIZE >> mipmap_index);
				real mysterious_horizontal_offset = scale * -2.f;

				D3DCALL(success, IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 0));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, scale - 1.0f + mysterious_horizontal_offset, scale + 1.0f));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 0));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, scale + 1.0f + mysterious_horizontal_offset, scale + 1.0f));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 1));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, scale + 1.0f + mysterious_horizontal_offset, scale - 1.0f));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 1));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, scale - 1.0f + mysterious_horizontal_offset, scale - 1.0f));
				D3DCALL(success, IDirect3DDevice8_End(global_d3d_device));
			}
		}

		rasterizer_set_target(global_window_parameters.rasterizer_target, 0, 0, FALSE, TRUE);
		rasterizer_set_stencil_mode(_rasterizer_stencil_mode_reject);

		if (!success)
		{
			error(_error_silent, "### ERROR rasterizer_water_build_bumpmap failed");
		}
	}

	return;
}

void rasterizer_water_draw(
	struct transparent_geometry_group const *group)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_water.c", 238, group);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_water.c", 239, global_d3d_device);

	if (rasterizer_debug_options.draw_water)
	{
		struct shader_transparent_water const *water = shader_transparent_water_get(group->shader);
		short vertex_shader_permutation = shader_get_vertex_shader_permutation(group->shader);
		short primary_vertex_type = rasterizer_transparent_geometry_get_primary_vertex_type(group);

		if (TEST_FLAG(water->water.flags, _shader_transparent_water_draw_before_fog_bit) &&
			!TEST_FLAG(group->geometry_flags, _rasterizer_geometry_no_queue_bit) &&
			!TEST_FLAG(group->geometry_flags, _rasterizer_geometry_sky_bit))
		{
			SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_NONE);
			SetRenderStateSmart(D3DRS_COLORWRITEENABLE, 0);
			SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
			SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			SetRenderStateSmart(D3DRS_ZWRITEENABLE, TRUE);

			rasterizer_set_vertex_shader_permutation(_vertex_shader_transparent_water_opacity, primary_vertex_type, vertex_shader_permutation);

			memset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_transparent_geometry_group_draw__internal(group, FALSE);
		}
		else
		{
			boolean zbuffer_write = !TEST_FLAG(group->geometry_flags, _rasterizer_geometry_sky_bit) && !TEST_FLAG(water->water.flags, _shader_transparent_water_draw_before_fog_bit);

			if (water_needs_update_flag)
			{
				rasterizer_water_build_bumpmap(group->shader);
				water_needs_update_flag = FALSE;
			}

			if (TEST_FLAG(water->water.flags, _shader_transparent_water_base_map_alpha_modulates_reflection_bit))
			{
				rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, water->water.base_map.index, group->shader_permutation_index);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				rasterizer_set_texture_direct(1, global_rasterizer_data->vector_normalization.index, 0);
				SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(1, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_NONE);
				SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
				SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
				SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
				SetRenderStateSmart(D3DRS_ZWRITEENABLE, zbuffer_write);

				rasterizer_set_vertex_shader_permutation(_vertex_shader_transparent_water_opacity, primary_vertex_type, vertex_shader_permutation);

				memset(&pixel_shader, 0, sizeof(pixel_shader));
				pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(2, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
				pixel_shader.PSConstant0[0] = real_alpha_to_pixel32(water->water.view_perpendicular_tint_color.alpha);
				pixel_shader.PSConstant1[0] = real_alpha_to_pixel32(water->water.view_parallel_tint_color.alpha);
				pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T1 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_T1, PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
				pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);
				rasterizer_set_pixel_shader(&pixel_shader);

				rasterizer_transparent_geometry_group_draw__internal(group, FALSE);
			}

			if (TEST_FLAG(water->water.flags, _shader_transparent_water_base_map_color_modulates_background_bit))
			{
				rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, water->water.base_map.index, group->shader_permutation_index);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_NONE);
				SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
				SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
				SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ZERO);
				SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
				SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
				SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
				SetRenderStateSmart(D3DRS_ZWRITEENABLE, zbuffer_write);

				rasterizer_set_vertex_shader_permutation(_vertex_shader_transparent_water_opacity, primary_vertex_type, vertex_shader_permutation);

				memset(&pixel_shader, 0, sizeof(pixel_shader));
				pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(TEST_FLAG(water->water.flags, _shader_transparent_water_atmospheric_fog_bit) ? PS_REGISTER_FOG | PS_CHANNEL_ALPHA : PS_REGISTER_ZERO, PS_REGISTER_ONE, PS_REGISTER_T0, PS_REGISTER_ZERO);
				rasterizer_set_pixel_shader(&pixel_shader);

				rasterizer_transparent_geometry_group_draw__internal(group, FALSE);
			}

			rasterizer_set_target_as_texture(0, _rasterizer_target_water, MIN(water->water.ripple_mipmap_levels, RASTERIZER_TARGET_WATER_MAX_MIPMAP_LEVELS));
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			rasterizer_set_texture(3, _bitmap_type_cube_map, _bitmap_usage_additive, water->water.reflection_map.index, group->shader_permutation_index);
			SetTextureStageStateSmart(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_NONE);
			SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
			SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, !TEST_FLAG(group->geometry_flags, _rasterizer_geometry_sky_bit));
			SetRenderStateSmart(D3DRS_SRCBLEND, TEST_FLAG(water->water.flags, _shader_transparent_water_base_map_alpha_modulates_reflection_bit) ? D3DBLEND_DESTALPHA : D3DBLEND_ONE);
			SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
			SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
			SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			SetRenderStateSmart(D3DRS_ZWRITEENABLE, zbuffer_write);

			rasterizer_set_vertex_shader_permutation(_vertex_shader_transparent_water_reflection, primary_vertex_type, vertex_shader_permutation);

			{
				real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4];

				vsh_constants__texscale[0][0] = water->water.ripple_scale;
				vsh_constants__texscale[0][1] = water->water.ripple_scale;
				vsh_constants__texscale[0][2] = cosine(water->water.ripple_animation_angle) * water->water.ripple_animation_velocity * global_frame_parameters.game_time_sec;
				vsh_constants__texscale[0][3] = sine(water->water.ripple_animation_angle) * water->water.ripple_animation_velocity * global_frame_parameters.game_time_sec;
				vsh_constants__texscale[1][0] = 0.f;
				vsh_constants__texscale[1][1] = 0.f;
				vsh_constants__texscale[1][2] = 0.f;
				vsh_constants__texscale[1][3] = 0.f;
				vsh_constants__texscale[2][0] = 0.f;
				vsh_constants__texscale[2][1] = 0.f;
				vsh_constants__texscale[2][2] = 0.f;
				vsh_constants__texscale[2][3] = 0.f;
				IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__TEXSCALE_OFFSET,
					vsh_constants__texscale,
					VSH_CONSTANTS__TEXSCALE_COUNT);
			}

			memset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_DOTPRODUCT, PS_TEXTUREMODES_DOTPRODUCT, PS_TEXTUREMODES_DOT_RFLCT_SPEC);
			pixel_shader.PSDotMapping = PS_DOTMAPPING(PS_DOTMAPPING_ZERO_TO_ONE, PS_DOTMAPPING_MINUS1_TO_1_D3D, PS_DOTMAPPING_MINUS1_TO_1_D3D, PS_DOTMAPPING_MINUS1_TO_1_D3D);

			if (TEST_FLAG(water->water.flags, _shader_transparent_water_atmospheric_fog_bit))
			{
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(4, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T3, PS_REGISTER_T3, PS_REGISTER_C0, PS_REGISTER_ONE);
				pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_T3);
				pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_FOG | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			}
			else
			{
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(2, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
				pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T3, PS_REGISTER_T3, PS_REGISTER_C0, PS_REGISTER_ONE);
				pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_EF_PROD, PS_REGISTER_T3, PS_REGISTER_ZERO);
				pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			}

			if (magnitude3d(&group->plane.n)>0.f)
			{
				real_rgb_color tint_color;
				real parallel = PIN(-dot_product3d(&global_window_parameters.camera.forward, &group->plane.n), 0.f, 1.f);

				tint_color.red = (1.f - parallel) * water->water.view_parallel_tint_color.red + parallel * water->water.view_perpendicular_tint_color.red;
				tint_color.green = (1.f - parallel) * water->water.view_parallel_tint_color.green + parallel * water->water.view_perpendicular_tint_color.green;
				tint_color.blue = (1.f - parallel) * water->water.view_parallel_tint_color.blue + parallel * water->water.view_perpendicular_tint_color.blue;
				pixel_shader.PSConstant0[0] = real_rgb_color_to_pixel32(&tint_color);
			}
			else
			{
				pixel_shader.PSConstant0[0] = 0x00FFFFFF;
			}

			rasterizer_set_pixel_shader(&pixel_shader);

			{
				real mipmap_lod_bias = -water->water.ripple_mipmap_lod_bias;

				SetTextureStageStateSmart(0, D3DTSS_MIPMAPLODBIAS, *(unsigned long *)&mipmap_lod_bias);
			}

			rasterizer_transparent_geometry_group_draw__internal(group, FALSE);
			SetTextureStageStateSmart(0, D3DTSS_MIPMAPLODBIAS, 0);
		}
	}

	return;
}
