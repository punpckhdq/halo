/*
RASTERIZER_XBOX_LIGHTS.C
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

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

static void rasterizer_sun_glow_copy_source(
	short target,
	real_rectangle2d const *bounds)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_lights.c", 323, bounds);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_lights.c", 324, global_d3d_device);

	rasterizer_set_target_as_texture(0, _rasterizer_target_render_primary, 0);
	SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);

	rasterizer_set_texture_direct(1, global_rasterizer_data->glow.index, 0);
	SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

	SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
	SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);
	SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
	SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
	SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
	SetRenderStateSmart(D3DRS_ZBIAS, 0);

	rasterizer_set_vertex_shader_permutation(_vs_screen_texture_animation, _rasterizer_vertex_type_dynamic_screen, 0);

	{
		real vsh_constants__texanim[VSH_CONSTANTS__TEXANIM_COUNT][4] =
		{
			{bounds->x1 - bounds->x0, 0.f, 0.f, bounds->x0},
			{0.f, bounds->y1 - bounds->y0, 0.f, bounds->y0},
			{1.f, 0.f, 0.f, 0.f},
			{0.f, 1.f, 0.f, 0.f},
			{1.f, 0.f, 0.f, 0.f},
			{0.f, 1.f, 0.f, 0.f},
			{1.f, 0.f, 0.f, 0.f},
			{0.f, 1.f, 0.f, 0.f}
		};

		IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
			VSH_CONSTANTS__TEXANIM_OFFSET,
			vsh_constants__texanim,
			VSH_CONSTANTS__TEXANIM_COUNT);
	}

	memset(&pixel_shader, 0, sizeof(pixel_shader));
	pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
	pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
	pixel_shader.PSConstant0[0] = D3DCOLOR_ARGB(192, 0, 0, 0);
	pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
	pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_1);
	pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0 | PS_CHANNEL_ALPHA);
	pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, 0);
	rasterizer_set_pixel_shader(&pixel_shader);

	rasterizer_set_target(target, 0, 0, FALSE, FALSE);

	{
		real const scale = 1.f / 64.f;
		real const mysterious_horizontal_offset = -2.f * scale;

		IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 0);
		IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, scale - 1.0f + mysterious_horizontal_offset, scale + 1.0f);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 0);
		IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, scale + 1.0f + mysterious_horizontal_offset, scale + 1.0f);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 1);
		IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, scale + 1.0f + mysterious_horizontal_offset, scale - 1.0f);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 1);
		IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, scale - 1.0f + mysterious_horizontal_offset, scale - 1.0f);
		IDirect3DDevice8_End(global_d3d_device);
	}

	rasterizer_set_target(global_window_parameters.rasterizer_target, 0, 0, FALSE, TRUE);

	return;
}

static boolean rasterizer_project_billboard(
	real_point3d const *point,
	real radius,
	real_point3d *projected_center,
	real_vector2d *projected_axes)
{
	boolean result = FALSE;

	if (radius > 0.f)
	{
		real_point3d p_view;
		real x;
		real y;
		real z;
		real w;
		real axis_x;
		real axis_y;
		short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
		short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;

		matrix4x3_transform_point(&global_window_parameters.frustum.world_to_view, point, &p_view);
		x = global_window_parameters.frustum.projection_matrix[0][0] * p_view.x + global_window_parameters.frustum.projection_matrix[1][0] * p_view.y + global_window_parameters.frustum.projection_matrix[2][0] * p_view.z + global_window_parameters.frustum.projection_matrix[3][0];
		y = global_window_parameters.frustum.projection_matrix[0][1] * p_view.x + global_window_parameters.frustum.projection_matrix[1][1] * p_view.y + global_window_parameters.frustum.projection_matrix[2][1] * p_view.z + global_window_parameters.frustum.projection_matrix[3][1];
		z = global_window_parameters.frustum.projection_matrix[0][2] * p_view.x + global_window_parameters.frustum.projection_matrix[1][2] * p_view.y + global_window_parameters.frustum.projection_matrix[2][2] * p_view.z + global_window_parameters.frustum.projection_matrix[3][2];
		w = global_window_parameters.frustum.projection_matrix[0][3] * p_view.x + global_window_parameters.frustum.projection_matrix[1][3] * p_view.y + global_window_parameters.frustum.projection_matrix[2][3] * p_view.z + global_window_parameters.frustum.projection_matrix[3][3];
		axis_x = global_window_parameters.frustum.projection_matrix[0][0] * radius;
		axis_y = global_window_parameters.frustum.projection_matrix[1][1] * radius;

		if (z > 0.f)
		{
			real one_over_w = 1.f / w;

			projected_center->x = ((x * one_over_w + 1.f) * viewport_width - 1.f) * 0.5f;
			projected_center->y = ((1.f - y * one_over_w) * viewport_height - 1.f) * 0.5f;
			projected_center->z = MIN(1.f, z * one_over_w);
			projected_axes->i = 0.5f * axis_x * one_over_w * viewport_width;
			projected_axes->j = 0.5f * axis_y * one_over_w * viewport_height;
			result = TRUE;
		}
	}

	return result;
}

static short rasterizer_sun_glow_convolve(
	short primary_target,
	short secondary_target,
	short passes)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_lights.c", 427, primary_target>=0 && primary_target<NUMBER_OF_RASTERIZER_TARGETS);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_lights.c", 428, secondary_target>=0 && secondary_target<NUMBER_OF_RASTERIZER_TARGETS);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_lights.c", 429, global_d3d_device);

	if (passes > 0)
	{
		boolean success;
		short pass;

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
		SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		rasterizer_set_vertex_shader_permutation(_vs_screen_texture_animation, _rasterizer_vertex_type_dynamic_screen, 0);

		{
			real vsh_constants__texanim[VSH_CONSTANTS__TEXANIM_COUNT][4] =
			{
				{1.f, 0.f, 0.f, -1.f / 128.f},
				{0.f, 1.f, 0.f, -1.f / 128.f},
				{1.f, 0.f, 0.f, 1.f / 128.f},
				{0.f, 1.f, 0.f, 1.f / 128.f},
				{1.f, 0.f, 0.f, -1.f / 128.f},
				{0.f, 1.f, 0.f, 1.f / 128.f},
				{1.f, 0.f, 0.f, 1.f / 128.f},
				{0.f, 1.f, 0.f, -1.f / 128.f}
			};

			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__TEXANIM_OFFSET,
				vsh_constants__texanim,
				VSH_CONSTANTS__TEXANIM_COUNT);
		}

		success = TRUE;
		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(2, 0);
		pixel_shader.PSConstant0[0] = D3DCOLOR_ARGB(255, 0, 0, 0);
		pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_ONE_HALF, PS_REGISTER_T1, PS_REGISTER_ONE_HALF);
		pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_ONE_HALF, PS_REGISTER_T3, PS_REGISTER_ONE_HALF);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
		pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
		pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
		rasterizer_set_pixel_shader(&pixel_shader);

		for (pass = 0; pass < passes; pass++)
		{
			short stage;
			short source_target = (pass & 1) ? secondary_target : primary_target;
			short destination_target = (pass & 1) ? primary_target : secondary_target;

			for (stage = 0; stage < RASTERIZER_MAXIMUM_TEXTURE_STAGES; stage++)
			{
				rasterizer_set_target_as_texture(stage, source_target, 0);
				SetTextureStageStateSmart(stage, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
				SetTextureStageStateSmart(stage, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
				SetTextureStageStateSmart(stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(stage, D3DTSS_MIPFILTER, D3DTEXF_POINT);
			}

			rasterizer_set_target(destination_target, 0, 0, FALSE, FALSE);

			pixel_shader.PSConstant0[0] = D3DCOLOR_ARGB(pass > 0 ? 127 : 255, 0, 0, 0);
			rasterizer_set_pixel_shader(&pixel_shader);

			{
				real const scale = 1.f / 64.f;
				real const mysterious_horizontal_offset = -2.f * scale;

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

		if (!success)
		{
			error(_error_silent, "### ERROR rasterizer_sun_glow_convolve failed");
		}
	}

	return (passes & 1) ? secondary_target : primary_target;
}

void rasterizer_sun_glow_draw(
	struct rasterizer_lens_flare_submit_parameters const *flare)
{
	real brightness;
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_lights.c", 583, global_d3d_device);

	{
		real_vector3d eye_to_corona_vector;

		vector_from_points3d(&global_window_parameters.camera.position, &flare->position, &eye_to_corona_vector);
		normalize3d(&eye_to_corona_vector);
		brightness = dot_product3d(&global_window_parameters.camera.forward, &eye_to_corona_vector);
		brightness = PIN((brightness - cosine(_pi / 4.f)) / (1.f - cosine(_pi / 4.f)), 0.f, 1.f);
	}

	{
		short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
		short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;
		real vsh_constants__screenproj[VSH_CONSTANTS__SCREENPROJ_COUNT][4] =
		{
			{2.f / viewport_width, 0.f, 0.f, -1.f - 1.f / viewport_width},
			{0.f, -2.f / viewport_height, 0.f, 1.f / viewport_height + 1.f},
			{0.f, 0.f, 1.f, 0.f},
			{0.f, 0.f, 0.f, 1.f},
			{0.f, 0.f, 0.f, 1.f}
		};

		D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
			VSH_CONSTANTS__SCREENPROJ_OFFSET,
			vsh_constants__screenproj,
			VSH_CONSTANTS__SCREENPROJ_COUNT));
	}

	{
		real_point3d position;
		real_point3d center;
		real_vector2d axes;
		real_vector3d direction = uncompress_int32_to_real_vector3d(flare->compressed_direction);

		point_from_line3d(&flare->position, &direction, flare->definition->occlusion_radius, &position);

		if (rasterizer_project_billboard(&position, flare->definition->occlusion_radius, &center, &axes))
		{
			real_rectangle2d bounds;
			real_rectangle2d screen_bounds;

			center.x = (real)floor(center.x + 0.5f);
			center.y = (real)floor(center.y + 0.5f);
			bounds.x0 = center.x - 32.f;
			bounds.y0 = center.y - 32.f;
			bounds.x1 = center.x + 32.f;
			bounds.y1 = center.y + 32.f;
			screen_bounds.x0 = bounds.x0 + global_window_parameters.camera.viewport_bounds.x0;
			screen_bounds.y0 = bounds.y0 + global_window_parameters.camera.viewport_bounds.y0;
			screen_bounds.x1 = bounds.x1 + global_window_parameters.camera.viewport_bounds.x0;
			screen_bounds.y1 = bounds.y1 + global_window_parameters.camera.viewport_bounds.y0;

			if (screen_bounds.x0 < global_window_parameters.camera.viewport_bounds.x1 &&
				screen_bounds.y0 < global_window_parameters.camera.viewport_bounds.y1 &&
				screen_bounds.x1 > global_window_parameters.camera.viewport_bounds.x0 &&
				screen_bounds.y1 > global_window_parameters.camera.viewport_bounds.y0)
			{
				short target;
				short pass;

				rasterizer_set_vertex_shader_permutation(_vs_sun_glow, _rasterizer_vertex_type_dynamic_unlit, 0);

				SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
				SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
				SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ZBIAS, 0);

				memset(&pixel_shader, 0, sizeof(pixel_shader));
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
				pixel_shader.PSFinalCombinerConstant0 = D3DCOLOR_ARGB(0, 0, 0, 0);
				pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, 0);
				rasterizer_set_pixel_shader(&pixel_shader);

				D3DCALL(success, IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, bounds.x0, bounds.y0));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, bounds.x1, bounds.y0));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, bounds.x1, bounds.y1));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, bounds.x0, bounds.y1));
				D3DCALL(success, IDirect3DDevice8_End(global_d3d_device));

				rasterizer_set_vertex_shader_permutation(_vs_sun_glow, _rasterizer_vertex_type_dynamic_unlit, 0);
				rasterizer_set_texture_direct(0, global_rasterizer_data->glow.index, 0);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
				SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
				SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
				SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
				SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ZBIAS, 0);

				memset(&pixel_shader, 0, sizeof(pixel_shader));
				pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
				pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_2);
				pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, 0);
				rasterizer_set_pixel_shader(&pixel_shader);

				D3DCALL(success, IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 0));
				D3DCALL(success, IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, bounds.x0, bounds.y0, center.z, 1.f));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 0));
				D3DCALL(success, IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, bounds.x1, bounds.y0, center.z, 1.f));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 1));
				D3DCALL(success, IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, bounds.x1, bounds.y1, center.z, 1.f));
				D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 1));
				D3DCALL(success, IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, bounds.x0, bounds.y1, center.z, 1.f));
				D3DCALL(success, IDirect3DDevice8_End(global_d3d_device));

				rasterizer_sun_glow_copy_source(_rasterizer_target_sun_glow_primary, &screen_bounds);
				rasterizer_sun_glow_copy_source(_rasterizer_target_sun_glow_secondary, &screen_bounds);
				target = rasterizer_sun_glow_convolve(_rasterizer_target_sun_glow_primary, _rasterizer_target_sun_glow_secondary, 4);

				rasterizer_set_vertex_shader_permutation(_vs_sun_glow, _rasterizer_vertex_type_dynamic_unlit, 0);
				rasterizer_set_target_as_texture(0, target, 0);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
				SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
				SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
				SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
				SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ZBIAS, 0);

				memset(&pixel_shader, 0, sizeof(pixel_shader));
				pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(2, 0);
				pixel_shader.PSConstant0[0] = D3DCOLOR_ARGB(0, 176, 176, 128);
				pixel_shader.PSConstant1[0] = D3DCOLOR_ARGB(0, 255, 255, 255);
				pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
				pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C0, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_C1);
				pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
				pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, 0);
				rasterizer_set_pixel_shader(&pixel_shader);

				for (pass = 0; pass < 16; pass++)
				{
					real r = ((real)pass / 16.f) * 80.f - 4.f;

					D3DCALL(success, IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN));
					D3DCALL(success, IDirect3DDevice8_SetVertexData4f(global_d3d_device, 9, 0.0f, 0.0f, 0.0f, brightness/(real)(pass + 1)));
					D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 0));
					D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, bounds.x0 - r, bounds.y0 - r));
					D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 0));
					D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, bounds.x1 + r, bounds.y0 - r));
					D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 1));
					D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, bounds.x1 + r, bounds.y1 + r));
					D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 1));
					D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, bounds.x0 - r, bounds.y1 + r));
					D3DCALL(success, IDirect3DDevice8_End(global_d3d_device));
				}

				if (!success)
				{
					error(_error_silent, "### ERROR rasterizer_sun_glow_draw failed");
				}
			}
		}
	}

	return;
}

/* ---------- private code */
