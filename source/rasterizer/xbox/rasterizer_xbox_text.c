/*
RASTERIZER_XBOX_TEXT.C
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

void rasterizer_text_begin(
	struct rasterizer_dynamic_screen_geometry_parameters const *parameters)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_text.c", 13, global_d3d_device);

	if (rasterizer_debug_options.draw_dynamic_screen_geometry && global_window_parameters.rasterizer_target==_rasterizer_target_render_primary)
	{
		short map_index;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_text.c", 18, parameters);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_text.c", 20, parameters->map[0]);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_text.c", 22, !parameters->map[2] || parameters->map[1]);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_text.c", 24, !parameters->map[1] || !parameters->meter_parameters);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_NONE);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		rasterizer_set_framebuffer_blend_function(parameters->framebuffer_blend_function);

		{
			short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
			short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;
			real_vector2d geometry_offset =
			{
				parameters->offset ? parameters->offset->i*2.f/viewport_width : 0.f,
				parameters->offset ? parameters->offset->j*-2.f/viewport_height : 0.f
			};
			real vsh_constants__screenproj[5][4] =
			{
				{2.f/viewport_width, 0.f, 0.f, geometry_offset.i - (1.f/viewport_width + 1.f)},
				{0.f, -2.f/viewport_height, 0.f, 1.f/viewport_height + geometry_offset.j + 1.f},
				{0.f, 0.f, 0.f, 0.5f},
				{0.f, 0.f, 0.f, 1.f},
				{parameters->map_texture_scale[0].i, parameters->map_texture_scale[0].j, 0.f, 1.f}
			};
			real vsh_constants__screenproj2[6][4] =
			{
				{parameters->map_texture_scale[1].i, parameters->map_texture_scale[1].j, parameters->map_texture_scale[2].i, parameters->map_texture_scale[2].j},
				{parameters->map_anchor_screen[0] ? 1.f : 0.f, parameters->map_anchor_screen[0] ? 0.f : 1.f, parameters->map_anchor_screen[1] ? 1.f : 0.f, parameters->map_anchor_screen[1] ? 0.f : 1.f},
				{parameters->map_anchor_screen[2] ? 1.f : 0.f, parameters->map_anchor_screen[2] ? 0.f : 1.f, parameters->map_offset[0] ? parameters->map_offset[0]->x : 0.f, parameters->map_offset[0] ? parameters->map_offset[0]->y : 0.f},
				{parameters->map_offset[1] ? parameters->map_offset[1]->x : 0.f, parameters->map_offset[1] ? parameters->map_offset[1]->y : 0.f, parameters->map_offset[2] ? parameters->map_offset[2]->x : 0.f, parameters->map_offset[2] ? parameters->map_offset[2]->y : 0.f},
				{parameters->map_scale[0].i, parameters->map_scale[0].j, parameters->map_scale[1].i, parameters->map_scale[1].j},
				{parameters->map_scale[2].i, parameters->map_scale[2].j, 0.f, 0.f}
			};

			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__SCREENPROJ_OFFSET,
				vsh_constants__screenproj,
				VSH_CONSTANTS__SCREENPROJ_COUNT);
			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__SCREENPROJ2_OFFSET,
				vsh_constants__screenproj2,
				VSH_CONSTANTS__SCREENPROJ2_COUNT);
		}

		for (map_index = 0; map_index<NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS; map_index++)
		{
			if (!parameters->map[map_index])
			{
				break;
			}

			rasterizer_set_texture_bitmap_data(map_index, parameters->map[map_index]);
			SetTextureStageStateSmart(map_index, D3DTSS_ADDRESSU, parameters->map_wrapped[map_index] ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(map_index, D3DTSS_ADDRESSV, parameters->map_wrapped[map_index] ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(map_index, D3DTSS_MAGFILTER, parameters->point_sampled ? D3DTEXF_POINT : D3DTEXF_LINEAR);
			SetTextureStageStateSmart(map_index, D3DTSS_MINFILTER, parameters->point_sampled ? D3DTEXF_POINT : D3DTEXF_LINEAR);
			SetTextureStageStateSmart(map_index, D3DTSS_MIPFILTER, parameters->point_sampled ? D3DTEXF_POINT : D3DTEXF_LINEAR);
		}

		rasterizer_set_vertex_shader_permutation(_vertex_shader_screen, _rasterizer_vertex_type_dynamic_screen, _screen_vertex_shader_permutation_screen2);

		if (parameters->map[0])
		{
			real_argb_color tint_colors[NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS];

			memset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.PSTextureModes = PS_TEXTUREMODES(parameters->map[0] ? PS_TEXTUREMODES_PROJECT2D : PS_TEXTUREMODES_NONE, parameters->map[1] ? PS_TEXTUREMODES_PROJECT2D : PS_TEXTUREMODES_NONE, parameters->map[2] ? PS_TEXTUREMODES_PROJECT2D : PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);

			tint_colors[0].rgb = *(parameters->map_tint[0] ? parameters->map_tint[0] : global_real_rgb_white);
			tint_colors[1].rgb = *(parameters->map_tint[1] ? parameters->map_tint[1] : global_real_rgb_white);
			tint_colors[2].rgb = *(parameters->map_tint[2] ? parameters->map_tint[2] : global_real_rgb_white);
			tint_colors[0].alpha = parameters->map_fade[0] ? *parameters->map_fade[0] : 1.f;
			tint_colors[1].alpha = parameters->map_fade[1] ? *parameters->map_fade[1] : 1.f;
			tint_colors[2].alpha = parameters->map_fade[2] ? *parameters->map_fade[2] : 1.f;
			pixel_shader.PSConstant0[0] = real_argb_color_to_pixel32(&tint_colors[0]);
			pixel_shader.PSConstant1[0] = real_argb_color_to_pixel32(&tint_colors[1]);
			pixel_shader.PSConstant0[1] = real_argb_color_to_pixel32(&tint_colors[2]);
			pixel_shader.PSConstant0[4] = real_argb_color_to_pixel32(&parameters->plasma_fade);
			pixel_shader.PSConstant0[5] = real_argb_color_to_pixel32(&parameters->plasma_fade);
			pixel_shader.PSConstant0[6] = real_argb_color_to_pixel32(&parameters->plasma_fade);
			pixel_shader.PSConstant0[7] = real_argb_color_to_pixel32(&parameters->plasma_fade);

			pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_C0, PS_REGISTER_T1, PS_REGISTER_C1);
			pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_T0, PS_REGISTER_T1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_C1 | PS_CHANNEL_ALPHA);
			pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_T0, PS_REGISTER_T1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_C0, PS_REGISTER_T0, PS_REGISTER_V0);
			pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_T2, PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T2 | PS_CHANNEL_ALPHA, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_V0 | PS_CHANNEL_ALPHA);
			pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_T2, PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(2, PS_COMBINERCOUNT_MUX_MSB | PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
			pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
			pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);
		}

		rasterizer_set_pixel_shader(&pixel_shader);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);

		rasterizer_set_vertex_shader_permutation(_vertex_shader_screen, _rasterizer_vertex_type_dynamic_screen, _screen_vertex_shader_permutation_screen);
		SetTextureStageStateSmart(0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE);
	}

	return;
}

void rasterizer_text_draw_character(
	struct dynamic_screen_vertex const *vertices)
{
	boolean success;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_text.c", 216, global_d3d_device);

	if (rasterizer_debug_options.draw_dynamic_screen_geometry && global_window_parameters.rasterizer_target==_rasterizer_target_render_primary)
	{
		short vertex_index;

		success = TRUE;
		D3DCALL(success, IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN));

		for (vertex_index = 0; vertex_index<NUMBER_OF_VERTICES_PER_QUADRILATERAL; vertex_index++)
		{
			D3DCALL(success, IDirect3DDevice8_SetVertexDataColor(global_d3d_device, 9, vertices[vertex_index].color));
			D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, vertices[vertex_index].texcoord.u, vertices[vertex_index].texcoord.v));
			D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, vertices[vertex_index].position.x, vertices[vertex_index].position.y));
		}

		D3DCALL(success, IDirect3DDevice8_End(global_d3d_device));

		if (!success)
		{
			error(_error_silent, "### ERROR rasterizer_text_draw_character failed");
		}
	}

	return;
}

void rasterizer_text_end(
	void)
{
	return;
}
