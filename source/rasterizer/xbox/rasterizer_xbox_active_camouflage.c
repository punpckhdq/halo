/*
RASTERIZER_XBOX_ACTIVE_CAMOUFLAGE.C
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

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static boolean local_active_camouflage_visibility_flag = FALSE;
static boolean local_active_camouflage_debug_cached_flag = FALSE;
static short local_active_camouflage_debug_cache_count = 0;

/* ---------- public code */

void rasterizer_active_camouflage_set_visibility(
	boolean visibility)
{
	local_active_camouflage_visibility_flag = visibility;

	if (!visibility)
	{
		local_active_camouflage_debug_cache_count = 0;
		local_active_camouflage_debug_cached_flag = FALSE;
	}

	return;
}

void rasterizer_active_camouflage_cache_primary_render_target(
	void)
{
	rectangle2d bounds;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_active_camouflage.c", 41, global_d3d_device);

	if (rasterizer_debug_options.active_camouflage_enabled && local_active_camouflage_visibility_flag)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_active_camouflage.c", 46, global_window_parameters.rasterizer_target==_rasterizer_target_render_primary);

		rasterizer_set_target_as_texture(0, _rasterizer_target_render_primary, 0);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_POINT);

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_CCW);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, 0);

		rasterizer_set_vertex_shader_permutation(4, _rasterizer_vertex_type_dynamic_screen, 0);

		{
			real vsh_constants__screenproj[5][4] =
			{
				{2.f/RASTERIZER_TARGET_RENDER_SECONDARY_WIDTH, 0.f, 0.f, -1.f - 1.f/RASTERIZER_TARGET_RENDER_SECONDARY_WIDTH},
				{0.f, -2.f/RASTERIZER_TARGET_RENDER_SECONDARY_HEIGHT, 0.f, 1.f/RASTERIZER_TARGET_RENDER_SECONDARY_HEIGHT + 1.f},
				{0.f, 0.f, 0.f, 0.5f},
				{0.f, 0.f, 0.f, 1.f},
				{1.f, 1.f, 0.f, 1.f}
			};

			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__SCREENPROJ_OFFSET,
				vsh_constants__screenproj,
				VSH_CONSTANTS__SCREENPROJ_COUNT);
		}

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0);
		rasterizer_set_pixel_shader(&pixel_shader);

		rasterizer_set_target(_rasterizer_target_render_secondary, 0, 0, FALSE, FALSE);
		rasterizer_set_stencil_mode(_rasterizer_stencil_mode_none);

		IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, global_window_parameters.camera.viewport_bounds.x0, global_window_parameters.camera.viewport_bounds.y1);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, 0, 0);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, global_window_parameters.camera.viewport_bounds.x1, global_window_parameters.camera.viewport_bounds.y1);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, RASTERIZER_TARGET_RENDER_SECONDARY_WIDTH, 0);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, global_window_parameters.camera.viewport_bounds.x1, global_window_parameters.camera.viewport_bounds.y0);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, RASTERIZER_TARGET_RENDER_SECONDARY_WIDTH, RASTERIZER_TARGET_RENDER_SECONDARY_HEIGHT);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, global_window_parameters.camera.viewport_bounds.x0, global_window_parameters.camera.viewport_bounds.y0);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, 0, RASTERIZER_TARGET_RENDER_SECONDARY_HEIGHT);
		IDirect3DDevice8_End(global_d3d_device);

		rasterizer_set_target(global_window_parameters.rasterizer_target, 0, 0, FALSE, TRUE);
		rasterizer_set_stencil_mode(_rasterizer_stencil_mode_reject);

		bounds.x0 = 512;
		bounds.x1 = 640;
		bounds.y0 = (local_active_camouflage_debug_cache_count + 1)*96;
		bounds.y1 = (local_active_camouflage_debug_cache_count + 2)*96;
		rasterizer_secondary_render_target_debug(&bounds);
		local_active_camouflage_debug_cache_count++;

		if (!rasterizer_debug_options.active_camouflage_multipass_enabled)
		{
			local_active_camouflage_visibility_flag = FALSE;
		}

		local_active_camouflage_debug_cached_flag = TRUE;
	}

	return;
}

void rasterizer_active_camouflage_draw(
	struct transparent_geometry_group const *group)
{
	struct rasterizer_model_begin_parameters parameters;
	real distance_falloff;
	real_rgb_color tint_color;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_active_camouflage.c", 152, group);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_active_camouflage.c", 153, group->shader);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_active_camouflage.c", 154, group->effect.type==_render_model_effect_type_active_camouflage);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_active_camouflage.c", 155, group->effect.intensity>0.0f);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_active_camouflage.c", 156, group->effect.intensity<=1.0f);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_active_camouflage.c", 157, global_d3d_device);

	if (rasterizer_debug_options.active_camouflage_enabled && global_window_parameters.rasterizer_target == _rasterizer_target_render_primary)
	{
		struct shader_model const *shader_model = shader_get_and_verify_type(group->shader, _shader_type_model);

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_active_camouflage.c", 164, local_active_camouflage_debug_cached_flag);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_active_camouflage.c", 165, !TEST_FLAG(group->geometry_flags, _rasterizer_geometry_no_queue_bit));

		if (TEST_FLAG(group->geometry_flags, _rasterizer_geometry_first_person_bit))
		{
			rasterizer_set_frustum_z(rasterizer_globals.z_near_first_person, rasterizer_globals.z_far_first_person);
		}

		if (group->effect.intensity == 1.f)
		{
			real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT*4];

			rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, shader_model->model.base_map.index, group->shader_permutation_index);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, TEST_FLAG(shader_model->model.flags, _shader_model_two_sided_bit) ? D3DCULL_NONE : D3DCULL_CCW);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_COLORWRITEENABLE, 0);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, FALSE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, TRUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHAREF, 127);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, TRUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, TRUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, 0);

			rasterizer_set_vertex_shader_permutation(13, rasterizer_transparent_geometry_get_primary_vertex_type(group), 0);

			vsh_constants__texscale[0] = shader_model->model.detail_map_scale;
			vsh_constants__texscale[1] = shader_model->model.detail_map_scale*shader_model->model.detail_map_v_scale;
			vsh_constants__texscale[2] = 1.f;
			vsh_constants__texscale[3] = 1.f;
			vsh_constants__texscale[4] = 1.f;
			vsh_constants__texscale[5] = 0.f;
			vsh_constants__texscale[6] = 0.f;
			vsh_constants__texscale[7] = 0.f;
			vsh_constants__texscale[8] = 0.f;
			vsh_constants__texscale[9] = 1.f;
			vsh_constants__texscale[10] = 0.f;
			vsh_constants__texscale[11] = 0.f;
			shader_texture_animation_evaluate(&shader_model->model.animation,
				group->animation,
				shader_model->model.map_scale.i*group->model_base_map_scale.i,
				shader_model->model.map_scale.j*group->model_base_map_scale.j,
				0.f,
				0.f,
				0.f,
				global_frame_parameters.game_time_sec,
				(real_vector4d *)&vsh_constants__texscale[4],
				(real_vector4d *)&vsh_constants__texscale[8]);
			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__TEXSCALE_OFFSET,
				vsh_constants__texscale,
				VSH_CONSTANTS__TEXSCALE_COUNT);

			memset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
			pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
			pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, 0);
			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_transparent_geometry_group_draw__internal(group, FALSE);
		}
		else
		{
			parameters.geometry_flags = group->geometry_flags & FLAG(_rasterizer_geometry_first_person_bit);
			parameters.skinning.node_matrices = (real_matrix4x3 *)group->node_matrices;
			parameters.skinning.node_matrix_count = group->node_matrix_count;
			parameters.centroid = group->centroid;
			memset(&parameters.effect, 0, sizeof(parameters.effect));
			parameters.base_map_scale = group->model_base_map_scale;

			if (group->lighting)
			{
				memcpy(&parameters.lighting, group->lighting, sizeof(parameters.lighting));
			}
			else
			{
				memset(&parameters.lighting, 0, sizeof(parameters.lighting));
			}

			if (group->animation)
			{
				parameters.animation = *group->animation;
			}
			else
			{
				memset(&parameters.animation, 0, sizeof(parameters.animation));
			}

			rasterizer_profile_enable(FALSE);
			rasterizer_models_begin(FALSE);
			rasterizer_model_begin(&parameters, TRUE);
			rasterizer_model_draw(group->shader,
				group->shader_permutation_index,
				group->triangle_buffer,
				group->dynamic_triangle_buffer_index,
				group->triangle_count,
				group->vertex_buffers,
				group->dynamic_vertex_buffer_index);
			rasterizer_model_end();
			rasterizer_models_end();
			rasterizer_profile_enable(TRUE);
		}

		rasterizer_set_texture_direct(0, global_rasterizer_data->active_camouflage_distortion.index, 0);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		rasterizer_set_target_as_texture(2, _rasterizer_target_render_secondary, 0);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MIPFILTER, D3DTEXF_POINT);

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, TEST_FLAG(shader_model->model.flags, _shader_model_two_sided_bit) ? D3DCULL_NONE : D3DCULL_CCW);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, TRUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZFUNC, D3DCMP_EQUAL);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, 0);

		if (group->effect.intensity < 1.f)
		{
			SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
			SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		}
		else
		{
			SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
		}

		rasterizer_set_vertex_shader_permutation(64, rasterizer_transparent_geometry_get_primary_vertex_type(group), 0);

		{
			real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT*4];

			distance_falloff = (1.f - group->effect.parameter)*global_rasterizer_data->active_camouflage_distance_falloff + global_rasterizer_data->active_camouflage_hyper_stealth_distance_falloff*group->effect.parameter;
			tint_color.red = (1.f - group->effect.parameter)*global_rasterizer_data->active_camouflage_tint_color.red + global_rasterizer_data->active_camouflage_hyper_stealth_tint_color.red*group->effect.parameter;
			tint_color.green = (1.f - group->effect.parameter)*global_rasterizer_data->active_camouflage_tint_color.green + global_rasterizer_data->active_camouflage_hyper_stealth_tint_color.green*group->effect.parameter;
			tint_color.blue = (1.f - group->effect.parameter)*global_rasterizer_data->active_camouflage_tint_color.blue + global_rasterizer_data->active_camouflage_hyper_stealth_tint_color.blue*group->effect.parameter;
			vsh_constants__texscale[0] = ((1.f - group->effect.parameter)*global_rasterizer_data->active_camouflage_refraction_amount + global_rasterizer_data->active_camouflage_hyper_stealth_refraction_amount*group->effect.parameter)*group->effect.intensity;
			vsh_constants__texscale[1] = distance_falloff;
			vsh_constants__texscale[2] = RASTERIZER_TARGET_RENDER_SECONDARY_WIDTH;
			vsh_constants__texscale[3] = RASTERIZER_TARGET_RENDER_SECONDARY_HEIGHT;
			vsh_constants__texscale[4] = 0.f;
			vsh_constants__texscale[5] = 0.f;
			vsh_constants__texscale[6] = 0.f;
			vsh_constants__texscale[7] = 0.f;
			vsh_constants__texscale[8] = tint_color.red;
			vsh_constants__texscale[9] = tint_color.green;
			vsh_constants__texscale[10] = tint_color.blue;
			vsh_constants__texscale[11] = 0.f;
			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__TEXSCALE_OFFSET,
				vsh_constants__texscale,
				VSH_CONSTANTS__TEXSCALE_COUNT);
		}

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_DOTPRODUCT, PS_TEXTUREMODES_DOT_ST, PS_TEXTUREMODES_NONE);
		pixel_shader.PSInputTexture = PS_INPUTTEXTURE(0, 0, 0, 0);
		pixel_shader.PSDotMapping = PS_DOTMAPPING(PS_DOTMAPPING_ZERO_TO_ONE, PS_DOTMAPPING_MINUS1_TO_1_D3D, PS_DOTMAPPING_MINUS1_TO_1_D3D, PS_DOTMAPPING_ZERO_TO_ONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(2, 0);
		pixel_shader.PSRGBInputs[0] = TEST_FLAG(global_rasterizer_data->active_camouflage_flags, _rasterizer_active_camouflage_tint_edge_density_bit) ?
			PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_V0) :
			PS_COMBINERINPUTS(PS_REGISTER_V0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
		pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_R0);
		pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
		pixel_shader.PSFinalCombinerConstant0 = real_alpha_to_pixel32(group->effect.intensity);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_R0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, 0);
		rasterizer_set_pixel_shader(&pixel_shader);

		rasterizer_transparent_geometry_group_draw__internal(group, FALSE);

		if (TEST_FLAG(group->geometry_flags, _rasterizer_geometry_first_person_bit))
		{
			rasterizer_set_frustum_z(0.f, 0.f);
		}
	}

	return;
}

/* ---------- private code */
