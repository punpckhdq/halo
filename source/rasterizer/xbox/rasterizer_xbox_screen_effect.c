/*
RASTERIZER_XBOX_SCREEN_EFFECT.C
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
#include "main.h"
#include "strings/resource.h"
#include "rasterizer_cinematics.h"

/* ---------- public code */

static real_vector2d __reciprocal_vector2d(
	real_vector2d const *v)
{
	real_vector2d result;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 30, v);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 31, v->i!=0.0f && v->j!=0.0f);

	result.i = 1.f / v->i;
	result.j = 1.f / v->j;

	return result;
}

static void rasterizer_screen_effect_set_texture_transforms(
	struct rasterizer_screen_effect_parameters const *parameters,
	short pass,
	short pass_count)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 47, parameters);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 48, global_d3d_device);

	{
		struct bitmap_data const *stage0_bitmap;
		struct bitmap_data const *stage1_bitmap;
		struct bitmap_data const *stage2_bitmap;
		struct bitmap_data const *stage3_bitmap;
		real_vector2d stage0_size;
		real_vector2d stage1_size;
		real_vector2d stage2_size;
		real_vector2d stage3_size;
		real_vector2d stage0_scale;
		real_vector2d stage1_scale;
		real_vector2d stage2_scale;
		real_vector2d stage3_scale;
		real_vector2d unit_scale;
		short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
		short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;
		struct bitmap_data screen_bitmap = {BITMAP_SIGNATURE, viewport_width, viewport_height, 1, _bitmap_type_2d, NONE, FLAG(_bitmap_linear_bit)};

		if (parameters->convolution_mask && (pass>0 || pass_count==1 || parameters->convolution_type!=_rasterizer_screen_effect_convolution_type_none))
		{
			stage0_bitmap = parameters->convolution_mask;
		}
		else
		{
			stage0_bitmap = &screen_bitmap;
		}

		stage1_bitmap = parameters->video_on ? parameters->video_scanline_map : &screen_bitmap;
		stage2_bitmap = parameters->video_on ? parameters->video_noise_map : &screen_bitmap;
		stage3_bitmap = &screen_bitmap;

		stage0_size.i = (real)stage0_bitmap->width;
		stage0_size.j = (real)stage0_bitmap->height;
		stage1_size.i = (real)stage1_bitmap->width;
		stage1_size.j = (real)stage1_bitmap->height;
		stage2_size.i = (real)stage2_bitmap->width;
		stage2_size.j = (real)stage2_bitmap->height;
		stage3_size.i = (real)stage3_bitmap->width;
		stage3_size.j = (real)stage3_bitmap->height;
		unit_scale.i = 1.f;
		unit_scale.j = 1.f;

		stage0_scale = TEST_FLAG(stage0_bitmap->flags, _bitmap_linear_bit) ? unit_scale : __reciprocal_vector2d(&stage0_size);
		stage1_scale = TEST_FLAG(stage1_bitmap->flags, _bitmap_linear_bit) ? unit_scale : __reciprocal_vector2d(&stage1_size);
		stage2_scale = TEST_FLAG(stage2_bitmap->flags, _bitmap_linear_bit) ? unit_scale : __reciprocal_vector2d(&stage2_size);
		stage3_scale = TEST_FLAG(stage3_bitmap->flags, _bitmap_linear_bit) ? unit_scale : __reciprocal_vector2d(&stage3_size);

		{
			real vsh_constants__texanim[8][4] =
			{
				{stage0_scale.i, 0.f, 0.f, (stage0_size.i + 1.f - (real)screen_bitmap.width) * stage0_scale.i * 0.5f},
				{0.f, stage0_scale.j, 0.f, (stage0_size.j + 1.f - (real)screen_bitmap.height) * stage0_scale.j * 0.5f},
				{stage1_scale.i, 0.f, 0.f, (stage1_size.i + 1.f - (real)screen_bitmap.width) * stage1_scale.i * 0.5f},
				{0.f, stage1_scale.j, 0.f, (stage1_size.j + 1.f - (real)screen_bitmap.height) * stage1_scale.j * 0.5f},
				{stage2_scale.i, 0.f, 0.f, (stage2_size.i + 1.f - (real)screen_bitmap.width) * stage2_scale.i * 0.5f},
				{0.f, stage2_scale.j, 0.f, (stage2_size.j + 1.f - (real)screen_bitmap.height) * stage2_scale.j * 0.5f},
				{stage3_scale.i, 0.f, 0.f, (stage3_size.i + 1.f - (real)screen_bitmap.width) * stage3_scale.i * 0.5f},
				{0.f, stage3_scale.j, 0.f, (stage3_size.j + 1.f - (real)screen_bitmap.height) * stage3_scale.j * 0.5f}
			};
			short window_center_x = (global_window_parameters.camera.window_bounds.x1 + global_window_parameters.camera.window_bounds.x0) / 2;
			short window_center_y = (global_window_parameters.camera.window_bounds.y1 + global_window_parameters.camera.window_bounds.y0) / 2;
			short offset_x = (global_window_parameters.camera.viewport_bounds.x0 + global_window_parameters.camera.viewport_bounds.x1) / 2 - window_center_x;
			short offset_y = (global_window_parameters.camera.viewport_bounds.y0 + global_window_parameters.camera.viewport_bounds.y1) / 2 - window_center_y;

			if (stage0_bitmap==&screen_bitmap)
			{
				vsh_constants__texanim[0][3] += (real)global_window_parameters.camera.viewport_bounds.x0;
				vsh_constants__texanim[1][3] += (real)global_window_parameters.camera.viewport_bounds.y0;
			}
			else if (stage0_bitmap==parameters->convolution_mask)
			{
				vsh_constants__texanim[0][3] += (real)offset_x * stage0_scale.i;
				vsh_constants__texanim[1][3] += (real)offset_y * stage0_scale.j;
			}

			if (stage1_bitmap==&screen_bitmap)
			{
				vsh_constants__texanim[2][3] += (real)global_window_parameters.camera.viewport_bounds.x0;
				vsh_constants__texanim[3][3] += (real)global_window_parameters.camera.viewport_bounds.y0;
			}
			else if (stage1_bitmap==parameters->convolution_mask)
			{
				vsh_constants__texanim[2][3] += (real)offset_x * stage1_scale.i;
				vsh_constants__texanim[3][3] += (real)offset_y * stage1_scale.j;
			}

			if (parameters->convolution_type==_rasterizer_screen_effect_convolution_type_blur)
			{
				real radius = parameters->convolution_radius;

				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 176, !parameters->video_on);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 177, main_get_window_count()<=1);

				vsh_constants__texanim[0][3] += parameters->convolution_mask ? 0.f : stage0_scale.i * radius;
				vsh_constants__texanim[1][3] += parameters->convolution_mask ? 0.f : stage0_scale.j * radius;
				vsh_constants__texanim[2][3] -= stage1_scale.i * radius;
				vsh_constants__texanim[3][3] -= stage1_scale.j * radius;
				vsh_constants__texanim[4][3] += stage2_scale.i * radius;
				vsh_constants__texanim[5][3] -= stage2_scale.j * radius;
				vsh_constants__texanim[6][3] -= stage3_scale.i * radius;
				vsh_constants__texanim[7][3] += stage3_scale.j * radius;
			}
			else if (parameters->convolution_type==_rasterizer_screen_effect_convolution_type_warp)
			{
				real radius = parameters->convolution_radius;
				real stage0_offset = parameters->convolution_mask ? 0.f : -radius;
				real stage1_offset = 0.f;
				real stage2_offset = radius;
				real stage3_offset = parameters->convolution_mask ? -radius : 2.f * radius;

				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 198, !parameters->video_on);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 199, main_get_window_count()<=1);

				vsh_constants__texanim[0][0] *= 1.f - stage0_offset / stage0_size.i;
				vsh_constants__texanim[1][1] *= 1.f - stage0_offset / stage0_size.j;
				vsh_constants__texanim[2][0] *= 1.f - stage1_offset / stage1_size.i;
				vsh_constants__texanim[3][1] *= 1.f - stage1_offset / stage1_size.j;
				vsh_constants__texanim[4][0] *= 1.f - stage2_offset / stage2_size.i;
				vsh_constants__texanim[5][1] *= 1.f - stage2_offset / stage2_size.j;
				vsh_constants__texanim[6][0] *= 1.f - stage3_offset / stage3_size.i;
				vsh_constants__texanim[7][1] *= 1.f - stage3_offset / stage3_size.j;
				vsh_constants__texanim[0][3] += stage0_offset * 0.5f;
				vsh_constants__texanim[1][3] += stage0_offset * 0.5f;
				vsh_constants__texanim[2][3] += stage1_offset * 0.5f;
				vsh_constants__texanim[3][3] += stage1_offset * 0.5f;
				vsh_constants__texanim[4][3] += stage2_offset * 0.5f;
				vsh_constants__texanim[5][3] += stage2_offset * 0.5f;
				vsh_constants__texanim[6][3] += stage3_offset * 0.5f;
				vsh_constants__texanim[7][3] += stage3_offset * 0.5f;
			}
			else if (pass==1 && parameters->video_on)
			{
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 223, main_get_window_count()<=1);

				vsh_constants__texanim[4][3] += stage2_scale.i * real_local_random() * stage2_size.i;
				vsh_constants__texanim[5][3] += stage2_scale.j * real_local_random() * stage2_size.j;
			}

			D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__TEXANIM_OFFSET,
				vsh_constants__texanim,
				VSH_CONSTANTS__TEXANIM_COUNT));
		}
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_screen_effect_set_texture_transforms failed");
	}

	return;
}

void _rasterizer_screen_effect(
	struct rasterizer_screen_effect_parameters const *parameters)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 251, global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_screen_effect);
	parameters = rasterizer_screen_effect_get_cinematic_parameters(parameters);

	if (parameters &&
		(parameters->convolution_type!=_rasterizer_screen_effect_convolution_type_none ||
		parameters->convolution_mask ||
		parameters->filter_light_enhancement_intensity>0.f ||
		parameters->filter_desaturation_intensity>0.f ||
		parameters->video_on) &&
		rasterizer_debug_options.screen_effects_enabled &&
		global_window_parameters.rasterizer_target==_rasterizer_target_render_primary)
	{
		short pass;
		short pass_count = 2 * (parameters->convolution_extra_passes + 1);

		if (parameters->video_on)
		{
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 276, parameters->video_scanline_map, "video effect must specify scanline map");
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 277, parameters->video_noise_map, "video effect must specify noise map");
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 278, parameters->convolution_extra_passes == 0, "video effect cannot specify extra convolution passes");
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 279, parameters->convolution_type == _rasterizer_screen_effect_convolution_type_none, "video effect cannot specify convolution type");
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 280, parameters->convolution_radius == 0.f, "video effect cannot specify nonzero convolution radius");
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 281, !parameters->convolution_mask, "video effect cannot specify convolution mask");
		}

		match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 286, parameters->convolution_type != _rasterizer_screen_effect_convolution_type_blur || !parameters->convolution_mask, "blur effect cannot specify convolution mask");

		rasterizer_set_vertex_shader_permutation(_vs_convolution, _rasterizer_vertex_type_dynamic_screen, _vertex_shader_permutation_default);

		for (pass = 0; pass<pass_count; pass++)
		{
			short source_target;
			short destination_target;

			if (pass_count==1)
			{
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 313, pass==0);
				source_target = NONE;
				destination_target = NONE;
			}
			else if ((pass & 1)==0)
			{
				source_target = _rasterizer_target_render_primary;
				destination_target = _rasterizer_target_z;
			}
			else
			{
				source_target = _rasterizer_target_z;
				destination_target = _rasterizer_target_render_primary;
			}

			if ((pass & 1) && parameters->video_on)
			{
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 334, pass==1);

				rasterizer_set_target_as_texture(0, source_target, 0);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_POINT);
				SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_POINT);
				SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);

				rasterizer_set_texture_bitmap_data(1, parameters->video_scanline_map);
				SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_POINT);
				SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
				SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_POINT);

				match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 351, !TEST_FLAG(parameters->video_noise_map->flags, _bitmap_linear_bit), "video effect noise map cannot be in linear format");
				rasterizer_set_texture_bitmap_data(2, parameters->video_noise_map);
				SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
				SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
				SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, D3DTEXF_POINT);
			}
			else
			{
				short stage;

				for (stage = 0; stage<RASTERIZER_MAXIMUM_TEXTURE_STAGES; stage++)
				{
					if (parameters->convolution_type==_rasterizer_screen_effect_convolution_type_none)
					{
						if (pass_count==1)
						{
							match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 371, pass==0);

							if (stage==0)
							{
								rasterizer_set_texture_bitmap_data(stage, parameters->convolution_mask);
							}
							else
							{
								continue;
							}
						}
						else if (pass==0)
						{
							if (stage==0)
							{
								rasterizer_set_target_as_texture(stage, source_target, 0);
							}
							else
							{
								continue;
							}
						}
						else if (pass==1)
						{
							if (parameters->convolution_mask)
							{
								if (stage==0)
								{
									rasterizer_set_texture_bitmap_data(stage, parameters->convolution_mask);
								}
								else if (stage==1)
								{
									rasterizer_set_target_as_texture(stage, source_target, 0);
								}
								else
								{
									continue;
								}
							}
							else
							{
								if (stage==0)
								{
									rasterizer_set_target_as_texture(stage, source_target, 0);
								}
								else
								{
									continue;
								}
							}
						}
						else
						{
							match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 398, "### ERROR non-convolution effect tried to render more than 2 passes");
						}
					}
					else
					{
						if (parameters->convolution_mask)
						{
							if (stage==0)
							{
								rasterizer_set_texture_bitmap_data(stage, parameters->convolution_mask);
							}
							else if (stage==1)
							{
								rasterizer_set_target_as_texture(stage, source_target, 0);
							}
							else if (stage==2)
							{
								rasterizer_set_target_as_texture(stage, source_target, 0);
							}
							else if (stage==3)
							{
								rasterizer_set_target_as_texture(stage, source_target, 0);
							}
							else
							{
								continue;
							}
						}
						else
						{
							if (stage==0)
							{
								rasterizer_set_target_as_texture(stage, source_target, 0);
							}
							else if (stage==1)
							{
								rasterizer_set_target_as_texture(stage, source_target, 0);
							}
							else if (stage==2)
							{
								rasterizer_set_target_as_texture(stage, source_target, 0);
							}
							else if (stage==3)
							{
								rasterizer_set_target_as_texture(stage, source_target, 0);
							}
							else
							{
								continue;
							}
						}
					}

					SetTextureStageStateSmart(stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
					SetTextureStageStateSmart(stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
					SetTextureStageStateSmart(stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
					SetTextureStageStateSmart(stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
					SetTextureStageStateSmart(stage, D3DTSS_MIPFILTER, D3DTEXF_POINT);
				}
			}

			SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_NONE);
			SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
			SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ZBIAS, 0);

			rasterizer_screen_effect_set_texture_transforms(parameters, pass, pass_count);

			if (destination_target!=NONE)
			{
				rasterizer_set_target(destination_target, 0, 0, FALSE, FALSE);
			}

			if (parameters->video_on)
			{
				memset(&pixel_shader, 0, sizeof(pixel_shader));

				if (pass==0)
				{
					pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
					pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
					pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0);
				}
				else if (pass==1)
				{
					real noise_intensity;
					long overbright_shifts[NUMBER_OF_RASTERIZER_SCREEN_EFFECT_VIDEO_OVERBRIGHT_MODES] = {PS_COMBINEROUTPUT_IDENTITY, PS_COMBINEROUTPUT_SHIFTLEFT_1, PS_COMBINEROUTPUT_SHIFTLEFT_2};

					pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE);
					pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(4, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
					noise_intensity = PIN(parameters->video_noise_intensity, 0.f, 1.f);
					pixel_shader.PSConstant0[0] = real_alpha_to_pixel32(noise_intensity);
					pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ONE, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T2 | PS_CHANNEL_ALPHA);
					pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
					pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ONE, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T2);
					pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
					pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T1, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1 | PS_CHANNEL_ALPHA);
					pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_V0, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
					pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_T0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 497, parameters->video_overbright_mode>=0 && parameters->video_overbright_mode<NUMBER_OF_RASTERIZER_SCREEN_EFFECT_VIDEO_OVERBRIGHT_MODES);
					pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R1, PS_REGISTER_R1, PS_REGISTER_T0, PS_REGISTER_ONE);
					pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_T0, PS_REGISTER_DISCARD, overbright_shifts[parameters->video_overbright_mode]);
					pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R1, PS_REGISTER_T0, PS_REGISTER_ZERO);
					pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_V0, PS_REGISTER_ZERO);

					SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
					SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA);
					SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ZERO);
					SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				}
				else
				{
					match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 513, "### ERROR video effect tried to render more than 2 passes");
				}

				rasterizer_set_pixel_shader(&pixel_shader);
			}
			else
			{
				long light_enhancement_input;
				long desaturation_input;
				short combiner_count = 0;

				memset(&pixel_shader, 0, sizeof(pixel_shader));

				if (parameters->convolution_type==_rasterizer_screen_effect_convolution_type_none)
				{
					if (pass_count==1)
					{
						match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 529, pass==0);

						pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
						pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
						pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						combiner_count = 1;
						pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
						pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0, PS_REGISTER_ZERO);
					}
					else if (pass==0)
					{
						pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
						pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
						pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						combiner_count = 1;
						pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
						pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					}
					else if (pass==1)
					{
						if (parameters->convolution_mask)
						{
							pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
							pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T1, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							combiner_count = 1;
							pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
							pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0, PS_REGISTER_ZERO);
						}
						else
						{
							pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
							pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							combiner_count = 1;
							pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
							pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
						}
					}
					else
					{
						match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 584, "### ERROR non-convolution effect tried to render more than 2 passes");
					}
				}
				else
				{
					if (parameters->convolution_mask)
					{
						pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D);
						pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T1 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_ONE, PS_REGISTER_T2 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_ONE);
						pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						pixel_shader.PSConstant0[1] = real_alpha_to_pixel32(1.f / 3.f);
						pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T3 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
						pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_ONE, PS_REGISTER_ONE_HALF, PS_REGISTER_ONE);
						pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T1, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_R0);
						pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						combiner_count = 4;
						pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
						pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0, PS_REGISTER_ZERO);
					}
					else
					{
						pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D);
						pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_ONE, PS_REGISTER_T1 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_ONE);
						pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTRIGHT_1);
						pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T2 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_ONE, PS_REGISTER_T3 | PS_INPUTMAPPING_HALFBIAS_NORMAL, PS_REGISTER_ONE);
						pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_COMBINEROUTPUT_SHIFTRIGHT_1);
						pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_ONE, PS_REGISTER_R1 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_ONE);
						pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTRIGHT_1);
						pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_SIGNED_IDENTITY, PS_REGISTER_ONE, PS_REGISTER_ONE_HALF, PS_REGISTER_ONE);
						pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						combiner_count = 4;
						pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
						pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					}
				}

				light_enhancement_input = parameters->filter_light_enhancement_uses_convolution_mask ? PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NEGATE : PS_REGISTER_ONE;
				desaturation_input = parameters->filter_desaturation_uses_convolution_mask ? PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NEGATE : PS_REGISTER_ONE;
				pixel_shader.PSConstant0[combiner_count] = real_alpha_to_pixel32(parameters->filter_light_enhancement_intensity);
				pixel_shader.PSConstant1[combiner_count] = real_alpha_to_pixel32(parameters->filter_desaturation_intensity);
				pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA, light_enhancement_input, PS_REGISTER_C1 | PS_CHANNEL_ALPHA, desaturation_input);
				pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);

				if (pass==pass_count - 1 && parameters->filter_light_enhancement_intensity>0.f)
				{
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
					combiner_count++;
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R1, PS_REGISTER_R1, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
					combiner_count++;
					pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_R1 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ZERO);
					pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, parameters->convolution_mask ? PS_REGISTER_T0 : PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				}

				if (pass==pass_count - 1 && parameters->filter_desaturation_intensity>0.f)
				{
					real_rgb_color gray = {1.f / 3.f, 1.f / 3.f, 1.f / 3.f};

					if (parameters->filter_light_enhancement_intensity>0.f)
					{
						pixel_shader.PSConstant0[combiner_count] = real_alpha_to_pixel32(parameters->filter_light_enhancement_intensity);
						pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_R1 | PS_INPUTMAPPING_UNSIGNED_INVERT);
						pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						combiner_count++;
					}

					pixel_shader.PSConstant0[combiner_count] = real_rgb_color_to_pixel32(&gray);
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_C0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
					combiner_count++;

					if (parameters->filter_desaturation_is_additive)
					{
						pixel_shader.PSFinalCombinerConstant0 = real_rgb_color_to_pixel32(&parameters->filter_desaturation_tint);
						pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_EF_PROD, PS_REGISTER_ZERO, PS_REGISTER_R0);
						pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_R1, parameters->convolution_mask ? PS_REGISTER_T0 : PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					}
					else
					{
						pixel_shader.PSFinalCombinerConstant0 = real_rgb_color_to_pixel32(&parameters->filter_desaturation_tint);
						pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_EF_PROD, PS_REGISTER_R0, PS_REGISTER_ZERO);
						pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_R1, parameters->convolution_mask ? PS_REGISTER_T0 : PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					}
				}

				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 705, combiner_count<=RASTERIZER_MAXIMUM_COMBINER_STAGES);
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(combiner_count, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
				rasterizer_set_pixel_shader(&pixel_shader);

				if (pass_count==1)
				{
					SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
					SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ZERO);
					SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
					SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				}
				else if (pass==pass_count - 1)
				{
					SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
					SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA);
					SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ZERO);
					SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				}
			}

			{
				real_rectangle2d vertex_bounds;
				short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
				short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;

				if (pass==0 && main_get_window_count()>1 && pass_count!=1)
				{
					vertex_bounds.x0 = (real)(2 * global_window_parameters.camera.viewport_bounds.x0) / RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH - 1.f;
					vertex_bounds.x1 = (real)(2 * global_window_parameters.camera.viewport_bounds.x1) / RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH - 1.f;
					vertex_bounds.y0 = (real)(-2 * global_window_parameters.camera.viewport_bounds.y0) / RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT + 1.f;
					vertex_bounds.y1 = (real)(-2 * global_window_parameters.camera.viewport_bounds.y1) / RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT + 1.f;
				}
				else
				{
					vertex_bounds.y1 = -1.f;
					vertex_bounds.x0 = -1.f;
					vertex_bounds.y0 = 1.f;
					vertex_bounds.x1 = 1.f;
				}

				D3DCALL(success, IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, viewport_height));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, vertex_bounds.x0, vertex_bounds.y1));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, viewport_width, viewport_height));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, vertex_bounds.x1, vertex_bounds.y1));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, viewport_width, 0));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, vertex_bounds.x1, vertex_bounds.y0));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 0));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, vertex_bounds.x0, vertex_bounds.y0));
				D3DCALL(success, IDirect3DDevice8_End(global_d3d_device));
			}
		}

		rasterizer_set_target(global_window_parameters.rasterizer_target, 0, 0, FALSE, TRUE);
	}

	rasterizer_profile_end(_rasterizer_profile_screen_effect);

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_screen_effect failed");
	}

	return;
}

void _rasterizer_screen_flash(
	void)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 786, global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_screen_flash);

	if (rasterizer_debug_options.screen_flash_enabled && global_window_parameters.screen_flash.type!=_render_screen_flash_type_none)
	{
		real_argb_color color;
		real_argb_color inverse_color;
		pixel32 color_pixel;
		pixel32 inverse_color_pixel;
		pixel32 constant_color;
		unsigned long rgb_inputs;
		unsigned long alpha_inputs;

		color.alpha = global_window_parameters.screen_flash.intensity * global_window_parameters.screen_flash.color.alpha;
		color.red = global_window_parameters.screen_flash.intensity * global_window_parameters.screen_flash.color.red;
		color.green = global_window_parameters.screen_flash.intensity * global_window_parameters.screen_flash.color.green;
		color.blue = global_window_parameters.screen_flash.intensity * global_window_parameters.screen_flash.color.blue;
		inverse_color.alpha = color.alpha;
		inverse_color.red = (1.f - global_window_parameters.screen_flash.color.red) * global_window_parameters.screen_flash.intensity;
		inverse_color.green = (1.f - global_window_parameters.screen_flash.color.green) * global_window_parameters.screen_flash.intensity;
		inverse_color.blue = (1.f - global_window_parameters.screen_flash.color.blue) * global_window_parameters.screen_flash.intensity;
		color_pixel = real_argb_color_to_pixel32(&color);
		inverse_color_pixel = real_argb_color_to_pixel32(&inverse_color);
		rgb_inputs = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		alpha_inputs = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);

		switch (global_window_parameters.screen_flash.type)
		{
		case _render_screen_flash_type_lighten:
			constant_color = color_pixel;
			SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ONE);
			SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			rgb_inputs = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			alpha_inputs = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			break;
		case _render_screen_flash_type_darken:
			constant_color = color_pixel;
			SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ONE);
			SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
			SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
			rgb_inputs = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			alpha_inputs = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			break;
		case _render_screen_flash_type_max:
			constant_color = color_pixel;
			SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
			SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVCONSTANTCOLOR);
			SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_MAX);
			SetRenderStateSmart(D3DRS_BLENDCOLOR, constant_color);
			rgb_inputs = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_ONE, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_NEGATIVE_ONE);
			alpha_inputs = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			break;
		case _render_screen_flash_type_min:
			constant_color = color_pixel;
			SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
			SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVCONSTANTCOLOR);
			SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_MIN);
			SetRenderStateSmart(D3DRS_BLENDCOLOR, constant_color);
			rgb_inputs = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_ONE, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE);
			alpha_inputs = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			break;
		case _render_screen_flash_type_invert:
			constant_color = color_pixel;
			SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
			SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVCONSTANTCOLOR);
			SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			SetRenderStateSmart(D3DRS_BLENDCOLOR, constant_color);
			rgb_inputs = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_C0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE);
			alpha_inputs = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			break;
		case _render_screen_flash_type_tint:
			SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ONE);
			SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVCONSTANTCOLOR);
			SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			constant_color = inverse_color_pixel;
			SetRenderStateSmart(D3DRS_BLENDCOLOR, constant_color);
			rgb_inputs = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			alpha_inputs = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			break;
		default:
			match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c", 875, "### ERROR unsupported screen flash type");
		}

		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		rasterizer_set_vertex_shader_permutation(_vs_screen, _rasterizer_vertex_type_dynamic_screen, _screen_vertex_shader_permutation_screen);

		{
			short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
			short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;
			real vsh_constants__screenproj[5][4] =
			{
				{2.f / viewport_width, 0.f, 0.f, -1.f - 1.f / viewport_width},
				{0.f, -2.f / viewport_height, 0.f, 1.f / viewport_height + 1.f},
				{0.f, 0.f, 0.f, 0.5f},
				{0.f, 0.f, 0.f, 1.f},
				{0.f, 0.f, 0.f, 1.f}
			};

			D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__SCREENPROJ_OFFSET,
				vsh_constants__screenproj,
				VSH_CONSTANTS__SCREENPROJ_COUNT));
		}

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
		pixel_shader.PSConstant0[0] = constant_color;
		pixel_shader.PSAlphaInputs[0] = alpha_inputs;
		pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSRGBInputs[0] = rgb_inputs;
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);
		rasterizer_set_pixel_shader(&pixel_shader);

		{
			short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
			short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;

			D3DCALL(success, IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN));
			D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, 0, 0));
			D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, viewport_width, 0));
			D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, viewport_width, viewport_height));
			D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, 0, viewport_height));
			D3DCALL(success, IDirect3DDevice8_End(global_d3d_device));
		}
	}

	rasterizer_profile_end(_rasterizer_profile_screen_flash);

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_screen_flash failed");
	}

	return;
}
