/*
RASTERIZER_XBOX_PLASMA_ENERGY.C
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

/* ---------- public code */

void rasterizer_plasma_energy_draw(
	struct transparent_geometry_group const *group)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_plasma_energy.c", 21, global_d3d_device);

	if (rasterizer_debug_options.plasma_energy_enabled)
	{
		struct _shader_transparent_plasma const *plasma = &shader_transparent_plasma_get(group->shader)->plasma;
		struct render_animation const *animation = group->animation;
		real_rgb_color const *tint_color = global_real_rgb_white;
		real intensity = 1.f;
		real offset = 0.f;

		if (animation)
		{
			real const *values;

			if (animation->colors &&
				plasma->tint_color_source>=_object_function_reference_a &&
				plasma->tint_color_source<=_object_function_reference_d)
			{
				tint_color = &animation->colors[plasma->tint_color_source - _object_function_reference_a];
			}

			values = animation->values;

			if (values)
			{
				if (plasma->intensity_source>=_object_function_reference_a && plasma->intensity_source<=_object_function_reference_d)
				{
					intensity = power(values[plasma->intensity_source - _object_function_reference_a], plasma->intensity_exponent);
				}

				if (plasma->offset_source>=_object_function_reference_a && plasma->offset_source<=_object_function_reference_d)
				{
					offset = power(values[plasma->offset_source - _object_function_reference_a], plasma->offset_exponent) * plasma->offset_amount;
				}
			}
		}

		rasterizer_set_texture(0, _bitmap_type_3d, _bitmap_usage_additive, plasma->primary_noise_map.index, group->shader_permutation_index);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSW, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		rasterizer_set_texture(1, _bitmap_type_3d, _bitmap_usage_additive, plasma->secondary_noise_map.index, group->shader_permutation_index);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(1, D3DTSS_ADDRESSW, D3DTADDRESS_WRAP);
		SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_NONE);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		rasterizer_set_vertex_shader_permutation(_vertex_shader_transparent_plasma_m, rasterizer_transparent_geometry_get_primary_vertex_type(group), _vertex_shader_permutation_default);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_plasma_energy.c", 105, plasma->primary_noise_map_animation_period!=0.0f);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_plasma_energy.c", 106, plasma->secondary_noise_map_animation_period!=0.0f);

		if (offset<0.0005f)
		{
			offset = 0.f;
		}

		{
			real vsh_constants__texanim[6][4];
			real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4];
			real primary_time = global_frame_parameters.game_time_sec / plasma->primary_noise_map_animation_period;
			real secondary_time = global_frame_parameters.game_time_sec / plasma->secondary_noise_map_animation_period;
			real primary_scale = plasma->primary_noise_map_scale;
			real secondary_scale = plasma->secondary_noise_map_scale;

			vsh_constants__texanim[0][0] = primary_scale;
			vsh_constants__texanim[0][1] = 0.f;
			vsh_constants__texanim[0][2] = offset;
			vsh_constants__texanim[0][3] = plasma->primary_noise_map_animation_direction.i * primary_time;
			vsh_constants__texanim[1][0] = 0.f;
			vsh_constants__texanim[1][1] = primary_scale;
			vsh_constants__texanim[1][2] = 0.f;
			vsh_constants__texanim[1][3] = plasma->primary_noise_map_animation_direction.j * primary_time;
			vsh_constants__texanim[2][0] = 0.f;
			vsh_constants__texanim[2][1] = 0.f;
			vsh_constants__texanim[2][2] = primary_scale;
			vsh_constants__texanim[2][3] = plasma->primary_noise_map_animation_direction.k * primary_time;
			vsh_constants__texanim[3][0] = secondary_scale;
			vsh_constants__texanim[3][1] = 0.f;
			vsh_constants__texanim[3][2] = 0.f;
			vsh_constants__texanim[3][3] = plasma->secondary_noise_map_animation_direction.i * secondary_time;
			vsh_constants__texanim[4][0] = 0.f;
			vsh_constants__texanim[4][1] = secondary_scale;
			vsh_constants__texanim[4][2] = 0.f;
			vsh_constants__texanim[4][3] = plasma->secondary_noise_map_animation_direction.j * secondary_time;
			vsh_constants__texanim[5][0] = 0.f;
			vsh_constants__texanim[5][1] = 0.f;
			vsh_constants__texanim[5][2] = secondary_scale;
			vsh_constants__texanim[5][3] = plasma->secondary_noise_map_animation_direction.k * secondary_time;

			vsh_constants__texscale[0][0] = 1.f;
			vsh_constants__texscale[0][1] = 1.f;
			vsh_constants__texscale[0][2] = 1.f;
			vsh_constants__texscale[0][3] = 1.f;
			vsh_constants__texscale[1][0] = (plasma->view_perpendicular_color.red - plasma->view_parallel_color.red) * tint_color->red;
			vsh_constants__texscale[1][1] = (plasma->view_perpendicular_color.green - plasma->view_parallel_color.green) * tint_color->green;
			vsh_constants__texscale[1][2] = (plasma->view_perpendicular_color.blue - plasma->view_parallel_color.blue) * tint_color->blue;
			vsh_constants__texscale[1][3] = (plasma->view_perpendicular_color.alpha - plasma->view_parallel_color.alpha) * intensity;
			vsh_constants__texscale[2][0] = plasma->view_parallel_color.red * tint_color->red;
			vsh_constants__texscale[2][1] = plasma->view_parallel_color.green * tint_color->green;
			vsh_constants__texscale[2][2] = plasma->view_parallel_color.blue * tint_color->blue;
			vsh_constants__texscale[2][3] = intensity * plasma->view_parallel_color.alpha;

			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__TEXANIM_OFFSET, vsh_constants__texanim, 6);
			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__TEXSCALE_OFFSET, vsh_constants__texscale, VSH_CONSTANTS__TEXSCALE_COUNT);
		}

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT3D, PS_TEXTUREMODES_PROJECT3D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(4, PS_COMBINERCOUNT_MUX_MSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
		pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_ONE, PS_REGISTER_T1 | PS_INPUTMAPPING_HALFBIAS_NEGATE, PS_REGISTER_ONE);
		pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_T0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_HALFBIAS_NEGATE, PS_REGISTER_ONE);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_R0, PS_REGISTER_R0);
		pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTLEFT_2 | PS_COMBINEROUTPUT_AB_CD_MUX);
		pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSAlphaInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_EXPAND_NORMAL);
		pixel_shader.PSAlphaOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_COMBINEROUTPUT_AB_CD_MUX);
		pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSAlphaInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_V1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSAlphaOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_V0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_V1, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE_HALF);
		pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_EF_PROD, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);
		rasterizer_set_pixel_shader(&pixel_shader);

		rasterizer_transparent_geometry_group_draw__internal(group, FALSE);
	}

	return;
}
