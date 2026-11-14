/*
RASTERIZER_XBOX_SHADOWS.C
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

/* ---------- prototypes */

static void rasterizer_shadow_convolve(void);

/* ---------- globals */

static struct rasterizer_model_begin_parameters const *local_parameters = NULL;
static boolean local_shadow_draw_initialized = FALSE; /* fake name */
static boolean local_shadow_cast = FALSE; /* fake name */
static real_matrix4x3 local_shadow_matrix; /* fake name */
static real local_object_bounding_radius; /* fake name */
static real_rgb_color local_shadow_color; /* fake name */
static boolean window_target_restored = TRUE; /* fake name */

/* ---------- private code */

static void rasterizer_shadow_convolve(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 31, global_d3d_device);

	if (rasterizer_debug_options.draw_environment_shadows && rasterizer_debug_options.shadow_convolution_enabled)
	{
		short stage;

		for (stage = 0; stage<RASTERIZER_MAXIMUM_TEXTURE_STAGES; stage++)
		{
			rasterizer_set_target_as_texture(stage, _rasterizer_target_shadow_primary, 0);
			SetTextureStageStateSmart(stage, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
			SetTextureStageStateSmart(stage, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
			SetTextureStageStateSmart(stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(stage, D3DTSS_MIPFILTER, D3DTEXF_POINT);
		}

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		rasterizer_set_vertex_shader_permutation(_vs_convolution, _rasterizer_vertex_type_dynamic_screen, _vertex_shader_permutation_default);

		{
			real vsh_constants__texanim[8][4] =
			{
				{1.f, 0.f, 0.f, -1.f / 256.f},
				{0.f, 1.f, 0.f, -1.f / 256.f},
				{1.f, 0.f, 0.f, 1.f / 256.f},
				{0.f, 1.f, 0.f, 1.f / 256.f},
				{1.f, 0.f, 0.f, -1.f / 256.f},
				{0.f, 1.f, 0.f, 1.f / 256.f},
				{1.f, 0.f, 0.f, 1.f / 256.f},
				{0.f, 1.f, 0.f, -1.f / 256.f}
			};

			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__TEXANIM_OFFSET,
				vsh_constants__texanim,
				VSH_CONSTANTS__TEXANIM_COUNT);
		}

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
		pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_ONE_HALF, PS_REGISTER_T1, PS_REGISTER_ONE_HALF);
		pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTRIGHT_1);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_ONE_HALF, PS_REGISTER_T3, PS_REGISTER_ONE_HALF);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTRIGHT_1);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA);
		rasterizer_set_pixel_shader(&pixel_shader);

		rasterizer_set_target(_rasterizer_target_shadow_secondary, 0, 0, FALSE, FALSE);

		{
			real const offset = 1.f / 128.f;

			IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
			IDirect3DDevice8_SetVertexData2s(global_d3d_device, D3DVSDE_SPECULAR, 0, 0);
			IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, -1.f - offset, 1.f + offset);
			IDirect3DDevice8_SetVertexData2s(global_d3d_device, D3DVSDE_SPECULAR, 1, 0);
			IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, 1.f - offset, 1.f + offset);
			IDirect3DDevice8_SetVertexData2s(global_d3d_device, D3DVSDE_SPECULAR, 1, 1);
			IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, 1.f - offset, offset - 1.f);
			IDirect3DDevice8_SetVertexData2s(global_d3d_device, D3DVSDE_SPECULAR, 0, 1);
			IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, -1.f - offset, offset - 1.f);
			IDirect3DDevice8_End(global_d3d_device);
		}
	}

	return;
}

/* ---------- public code */

void _rasterizer_environment_shadows_begin(
	void)
{
	rasterizer_profile_begin(_rasterizer_profile_environment_shadows);

	return;
}

boolean _rasterizer_environment_shadow_begin(
	long object_index,
	real_matrix4x3 const *shadow_matrix,
	real_rgb_color const *shadow_color,
	real object_bounding_radius,
	real *shadow_volume_bounding_radius)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 147, global_d3d_device);

	if (global_window_parameters.rasterizer_target==_rasterizer_target_render_primary && rasterizer_debug_options.draw_environment_shadows)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 153, shadow_matrix);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 154, shadow_color);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 155, shadow_color->red >=0.0f && shadow_color->red <=1.0f);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 156, shadow_color->green>=0.0f && shadow_color->green<=1.0f);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 157, shadow_color->blue >=0.0f && shadow_color->blue <=1.0f);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 158, object_bounding_radius>0.0f);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, TRUE);
		SetRenderStateSmart(D3DRS_ALPHAREF, 127);
		SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ONE);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);
		rasterizer_set_pixel_shader(&pixel_shader);

		{
			real vsh_constants__screenproj[5][4] =
			{
				{
					(1.f / object_bounding_radius) * shadow_matrix->forward.i,
					(1.f / object_bounding_radius) * shadow_matrix->forward.j,
					(1.f / object_bounding_radius) * shadow_matrix->forward.k,
					-(1.f / object_bounding_radius) * dot_product3d(&shadow_matrix->forward, (real_vector3d const *)&shadow_matrix->position)
				},
				{
					(1.f / object_bounding_radius) * shadow_matrix->left.i,
					(1.f / object_bounding_radius) * shadow_matrix->left.j,
					(1.f / object_bounding_radius) * shadow_matrix->left.k,
					-(1.f / object_bounding_radius) * dot_product3d(&shadow_matrix->left, (real_vector3d const *)&shadow_matrix->position)
				},
				{0.f, 0.f, 0.f, 0.5f},
				{0.f, 0.f, 0.f, 1.f},
				{0.f, 0.f, 0.f, 0.f}
			};

			D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__SCREENPROJ_OFFSET, vsh_constants__screenproj, VSH_CONSTANTS__SCREENPROJ_COUNT));
		}

		rasterizer_set_target(_rasterizer_target_shadow_primary, 0, rasterizer_debug_options.shadow_debug_enabled ? 0x88888888 : 0, TRUE, FALSE);
		rasterizer_set_stencil_mode(_rasterizer_stencil_mode_none);

		local_shadow_matrix = *shadow_matrix;
		local_shadow_color = *shadow_color;
		local_object_bounding_radius = object_bounding_radius;

		if (shadow_volume_bounding_radius)
		{
			*shadow_volume_bounding_radius = object_bounding_radius;
		}

		local_parameters = NULL;
		local_shadow_draw_initialized = FALSE;
		local_shadow_cast = FALSE;
		window_target_restored = FALSE;

		if (rasterizer_debug_options.statistics_mode==_rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.environment_shadow_count++;
		}
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_environment_shadow_begin failed");
	}

	return TRUE;
}

void _rasterizer_environment_shadow_model_begin(
	struct rasterizer_model_begin_parameters const *parameters)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 239, global_d3d_device);

	if (global_window_parameters.rasterizer_target==_rasterizer_target_render_primary && rasterizer_debug_options.draw_environment_shadows)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 245, parameters);

		rasterizer_set_model_skinning(&parameters->skinning);
		local_parameters = parameters;
		local_shadow_cast = TRUE;

		if (rasterizer_debug_options.statistics_mode==_rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.model_shadow_count++;
		}
	}

	return;
}

void _rasterizer_environment_shadow_model_draw(
	struct shader const *shader,
	short shader_permutation_index,
	struct triangle_buffer const *triangle_buffer,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 274, global_d3d_device);

	if (global_window_parameters.rasterizer_target==_rasterizer_target_render_primary && rasterizer_debug_options.draw_environment_shadows)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 280, shader);

		if (shader->base.type==_shader_type_model)
		{
			struct shader_model const *shader_model = shader_model_get(shader);

			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 286, vertex_buffer);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 287, triangle_buffer);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 288, local_parameters);

			if (TEST_FLAG(shader_model->model.flags, _shader_model_two_sided_bit))
			{
				SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_NONE);
			}
			else
			{
				SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
			}

			rasterizer_set_vertex_shader_permutation(_vs_model_shadow, vertex_buffer->type, _vertex_shader_permutation_default);

			if (TEST_FLAG(shader_model->model.flags, _shader_model_not_alpha_tested_bit))
			{
				SetRenderStateSmart(D3DRS_PSTEXTUREMODES, PS_TEXTUREMODES(PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE));
			}
			else
			{
				SetRenderStateSmart(D3DRS_PSTEXTUREMODES, PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE));
				rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, shader_model->model.base_map.index, shader_permutation_index);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
				SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
			}

			{
				real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
				{
					{shader_model->model.detail_map_scale, shader_model->model.detail_map_scale*shader_model->model.detail_map_v_scale, 1.f, 1.f},
					{1.f, 0.f, 0.f, 0.f},
					{0.f, 1.f, 0.f, 0.f}
				};

				shader_texture_animation_evaluate(&shader_model->model.animation,
					&local_parameters->animation,
					local_parameters->base_map_scale.i*shader_model->model.map_scale.i,
					local_parameters->base_map_scale.j*shader_model->model.map_scale.j,
					0.f,
					0.f,
					0.f,
					global_frame_parameters.game_time_sec,
					(real_vector4d *)vsh_constants__texscale[1],
					(real_vector4d *)vsh_constants__texscale[2]);
				IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
					VSH_CONSTANTS__TEXSCALE_OFFSET,
					vsh_constants__texscale,
					VSH_CONSTANTS__TEXSCALE_COUNT);
			}

			rasterizer_draw_static_triangles_static_vertices(triangle_buffer, 0, triangle_buffer->count, vertex_buffer);

			if (rasterizer_debug_options.statistics_mode==_rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.model_shadow_primitive_count++;
				rasterizer_frame_statistics.model_shadow_triangle_count += triangle_buffer->count;
				rasterizer_frame_statistics.model_shadow_vertex_count +=rasterizer_frame_statistics_count_static_vertices(triangle_buffer, vertex_buffer);
			}
		}
	}

	return;
}

void _rasterizer_environment_shadow_model_end(
	void)
{
	local_parameters = NULL;

	return;
}

void _rasterizer_environment_shadow_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 404, global_d3d_device);

	if (global_window_parameters.rasterizer_target==_rasterizer_target_render_primary && rasterizer_debug_options.draw_environment_shadows)
	{
		if (!local_shadow_draw_initialized)
		{
			if (rasterizer_debug_options.shadow_convolution_enabled)
			{
				rasterizer_shadow_convolve();
			}

			rasterizer_set_target_as_texture(0, rasterizer_debug_options.shadow_convolution_enabled ? _rasterizer_target_shadow_secondary : _rasterizer_target_shadow_primary, 0);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
			SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
			SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			rasterizer_set_texture_direct(1, global_rasterizer_data->linear_corner_fade.index, 0);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			rasterizer_set_texture_direct(2, global_rasterizer_data->vector_normalization.index, 0);
			SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(2, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
			SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
			SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);
			SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
			SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
			SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			SetRenderStateSmart(D3DRS_ALPHATESTENABLE, TRUE);
			SetRenderStateSmart(D3DRS_ALPHAREF, 0);
			SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
			SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_EQUAL);
			SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
			SetRenderStateSmart(D3DRS_ZBIAS, 0);

			memset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
			pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(4, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
			pixel_shader.PSConstant0[0] = real_rgb_color_to_pixel32(&local_shadow_color);
			pixel_shader.PSConstant1[0] = D3DCOLOR_ARGB(0, 255, 255, 255);
			pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T1 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_T0, PS_REGISTER_C0 | PS_INPUTMAPPING_UNSIGNED_INVERT);
			pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ONE, PS_REGISTER_R0, PS_REGISTER_R1 | PS_INPUTMAPPING_UNSIGNED_INVERT);
			pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
			pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
			pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT);
			pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R1, PS_REGISTER_ZERO);

			if (rasterizer_debug_options.shadow_debug_enabled)
			{
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
				SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
			}

			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_set_vertex_shader_permutation(_vs_environment_shadow,vertex_buffer->type, shader_get_vertex_shader_permutation(shader));

			{
				real one_over_radius = 1.f / local_object_bounding_radius;
				real one_over_quadruple_radius = 1.f / (local_object_bounding_radius * 4.f);
				real one_over_half_radius = 1.f / (local_object_bounding_radius * 0.5f);
				real vsh_constants__shadow[VSH_CONSTANTS__SHADOW_COUNT][4] =
				{
					{
						local_shadow_matrix.forward.i * one_over_radius * 0.5f,
						local_shadow_matrix.forward.j * one_over_radius * 0.5f,
						local_shadow_matrix.forward.k * one_over_radius * 0.5f,
						(1.f - dot_product3d(&local_shadow_matrix.forward, (real_vector3d const *)&local_shadow_matrix.position) * one_over_radius) * 0.5f
					},
					{
						local_shadow_matrix.left.i * one_over_radius * -0.5f,
						local_shadow_matrix.left.j * one_over_radius * -0.5f,
						local_shadow_matrix.left.k * one_over_radius * -0.5f,
						(dot_product3d(&local_shadow_matrix.left, (real_vector3d const *)&local_shadow_matrix.position) * one_over_radius + 1.f) * 0.5f
					},
					{
						local_shadow_matrix.up.i * one_over_quadruple_radius,
						local_shadow_matrix.up.j * one_over_quadruple_radius,
						local_shadow_matrix.up.k * one_over_quadruple_radius,
						-(dot_product3d(&local_shadow_matrix.up, (real_vector3d const *)&local_shadow_matrix.position) * one_over_quadruple_radius)
					},
					{
						-(local_shadow_matrix.up.i * one_over_half_radius),
						-(local_shadow_matrix.up.j * one_over_half_radius),
						-(local_shadow_matrix.up.k * one_over_half_radius),
						dot_product3d(&local_shadow_matrix.up, (real_vector3d const *)&local_shadow_matrix.position) * one_over_half_radius
					},
					{
						local_shadow_matrix.up.i,
						local_shadow_matrix.up.j,
						local_shadow_matrix.up.k,
						0.f
					}
				};

				D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__SHADOW_OFFSET, vsh_constants__shadow, VSH_CONSTANTS__SHADOW_COUNT));
			}

			if (!window_target_restored)
			{
				rasterizer_set_target(global_window_parameters.rasterizer_target, 0, 0, FALSE, TRUE);
				window_target_restored = TRUE;
			}

			local_shadow_draw_initialized = TRUE;
		}

		rasterizer_set_stencil_mode(_rasterizer_stencil_mode_reject);
		rasterizer_draw_dynamic_triangles_static_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);

		if (rasterizer_debug_options.statistics_mode==_rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.environment_shadow_primitive_count++;
			rasterizer_frame_statistics.environment_shadow_triangle_count += triangle_count;
			rasterizer_frame_statistics.environment_shadow_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count);
		}
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_environment_shadow_draw failed");
	}

	return;
}

void _rasterizer_environment_shadow_end(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_shadows.c", 563, global_d3d_device);

	if (global_window_parameters.rasterizer_target==_rasterizer_target_render_primary && rasterizer_debug_options.draw_environment_shadows)
	{
		if (!local_shadow_cast)
		{
			error(_error_silent, "### WARNING empty shadow has been cast");
		}

		if (!window_target_restored)
		{
			rasterizer_set_target(global_window_parameters.rasterizer_target, 0, 0, FALSE, TRUE);
			window_target_restored = TRUE;
		}
	}

	return;
}

void _rasterizer_environment_shadows_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_shadows);

	return;
}
