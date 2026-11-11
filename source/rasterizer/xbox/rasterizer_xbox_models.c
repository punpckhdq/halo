/*
RASTERIZER_XBOX_MODELS.C
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

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void rasterizer_model_set_pixel_shader(pixel32 self_illumination_color, short detail_function, short detail_mask, pixel32 diffuse_change_color, pixel32 cc0, pixel32 cc0_error, pixel32 cc1, boolean detail_after_reflection, boolean true_atmospheric_fog);

/* ---------- globals */

static boolean local_pixel_shader_dirty_flag = TRUE;

static boolean local_do_not_change_z_stencil_states;
static boolean local_environment_fog_screen_flag;
static boolean local_planar_fog_flag;
static boolean local_sky_flag;
static short local_model_effect_type;
static boolean local_parameters_queued_flag;
static struct rasterizer_model_begin_parameters const *local_parameters;
static struct transparent_geometry_group local_immediate_group; /* fake name */
static real_matrix4x3 const *local_queued_node_matrices; /* fake name */
static short local_queued_node_matrix_count; /* fake name */
static struct render_lighting const *local_queued_lighting; /* fake name */
static struct render_animation const *local_queued_animation; /* fake name */

/* ---------- public code */

static void rasterizer_model_set_pixel_shader(
	pixel32 self_illumination_color,
	short detail_function,
	short detail_mask,
	pixel32 diffuse_change_color,
	pixel32 cc0,
	pixel32 cc0_error,
	pixel32 cc1,
	boolean detail_after_reflection,
	boolean true_atmospheric_fog)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 106, detail_function>=0 && detail_function<NUMBER_OF_SHADER_MODEL_DETAIL_FUNCTIONS);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 107, detail_mask>=0 && detail_mask<NUMBER_OF_SHADER_MODEL_DETAIL_MASKS);

	{
		unsigned long detail_function_inputs[NUMBER_OF_SHADER_MODEL_DETAIL_FUNCTIONS] =
		{
			PS_REGISTER_ZERO | PS_INPUTMAPPING_HALFBIAS_NEGATE,	// _shader_model_detail_function_double_biased_multiply
			PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT,	// _shader_model_detail_function_multiply
			PS_REGISTER_ZERO | PS_INPUTMAPPING_HALFBIAS_NEGATE	// _shader_model_detail_function_double_biased_add
		};
		unsigned long detail_mask_inputs[NUMBER_OF_SHADER_MODEL_DETAIL_MASKS] =
		{
			PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT,						// _shader_model_detail_mask_none
			PS_REGISTER_R1 | PS_INPUTMAPPING_UNSIGNED_INVERT,						// _shader_model_detail_mask_reflection_mask_inverse
			PS_REGISTER_R1,															// _shader_model_detail_mask_reflection_mask
			PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT,	// _shader_model_detail_mask_self_illumination_mask_inverse
			PS_REGISTER_R0 | PS_CHANNEL_ALPHA,										// _shader_model_detail_mask_self_illumination_mask
			PS_REGISTER_T1 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT,	// _shader_model_detail_mask_change_color_mask_inverse
			PS_REGISTER_T1 | PS_CHANNEL_ALPHA,										// _shader_model_detail_mask_change_color_mask
			PS_REGISTER_T2 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT,	// _shader_model_detail_mask_multipurpose_alpha_mask_inverse
			PS_REGISTER_T2 | PS_CHANNEL_ALPHA										// _shader_model_detail_mask_multipurpose_alpha_mask
		};

		pixel_shader.PSConstant0[1] = self_illumination_color;
		pixel_shader.PSConstant0[2] = diffuse_change_color;
		pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(detail_mask_inputs[detail_mask] ^ PS_INPUTMAPPING_UNSIGNED_INVERT, detail_function_inputs[detail_function], detail_mask_inputs[detail_mask], PS_REGISTER_T1);
	}

	if (detail_after_reflection)
	{
		unsigned long detail_inputs[NUMBER_OF_SHADER_MODEL_DETAIL_FUNCTIONS] =
		{
			PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T1, PS_REGISTER_R0, PS_REGISTER_T1),																											// _shader_model_detail_function_double_biased_multiply
			PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T1, PS_REGISTER_ZERO, PS_REGISTER_ZERO),																										// _shader_model_detail_function_multiply
			PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T1 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT)	// _shader_model_detail_function_double_biased_add
		};

		pixel_shader.PSRGBInputs[6] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_V0, PS_REGISTER_T3, PS_REGISTER_R1 | PS_CHANNEL_ALPHA);
		pixel_shader.PSRGBOutputs[6] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
		pixel_shader.PSRGBInputs[7] = detail_inputs[detail_function];
		pixel_shader.PSRGBOutputs[7] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
	}
	else
	{
		unsigned long detail_inputs[NUMBER_OF_SHADER_MODEL_DETAIL_FUNCTIONS] =
		{
			PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_T1, PS_REGISTER_T0, PS_REGISTER_T1),																											// _shader_model_detail_function_double_biased_multiply
			PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_T1, PS_REGISTER_ZERO, PS_REGISTER_ZERO),																										// _shader_model_detail_function_multiply
			PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T1 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT)	// _shader_model_detail_function_double_biased_add
		};

		pixel_shader.PSRGBInputs[6] = detail_inputs[detail_function];
		pixel_shader.PSRGBOutputs[6] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_T0, 0);
		pixel_shader.PSRGBInputs[7] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_V0, PS_REGISTER_T3, PS_REGISTER_R1 | PS_CHANNEL_ALPHA);
		pixel_shader.PSRGBOutputs[7] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
	}

	if (true_atmospheric_fog)
	{
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_FOG | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_FOG, PS_REGISTER_ZERO);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, 0);
	}
	else
	{
		pixel_shader.PSConstant0[5] = cc0_error;
		pixel_shader.PSConstant1[5] = cc1;
		pixel_shader.PSFinalCombinerConstant0 = cc0;
		pixel_shader.PSFinalCombinerConstant1 = cc1;
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_EF_PROD, PS_REGISTER_C0, PS_REGISTER_R1);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, 0);
	}

	if (local_pixel_shader_dirty_flag)
	{
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_CUBEMAP);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(8, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
		pixel_shader.PSConstant0[0] = D3DCOLOR_ARGB(0, 255, 0, 0);
		pixel_shader.PSConstant1[0] = D3DCOLOR_ARGB(0, 0, 255, 0);
		pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_C1, PS_REGISTER_T2, PS_REGISTER_C0);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT | PS_COMBINEROUTPUT_CD_DOT_PRODUCT);
		pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_T1, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_V0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_C0);
		pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_V0, 0);
		pixel_shader.PSAlphaInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R1, PS_REGISTER_V1 | PS_CHANNEL_ALPHA);
		pixel_shader.PSAlphaOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_C0);
		pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
		pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_T1, 0);
		pixel_shader.PSRGBInputs[4] = PS_COMBINERINPUTS(PS_REGISTER_T3, PS_REGISTER_V1, PS_REGISTER_V0, PS_REGISTER_R0);
		pixel_shader.PSRGBOutputs[4] = PS_COMBINEROUTPUTS(PS_REGISTER_T3, PS_REGISTER_V0, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSRGBInputs[5] = PS_COMBINERINPUTS(PS_REGISTER_C1, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_C0 | PS_INPUTMAPPING_SIGNED_NEGATE);
		pixel_shader.PSRGBOutputs[5] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, 0);
		rasterizer_set_pixel_shader(&pixel_shader);
		local_pixel_shader_dirty_flag = FALSE;
	}
	else
	{
		D3DDevice_SetRenderState(D3DRS_PSCONSTANT0_1, pixel_shader.PSConstant0[1]);
		D3DDevice_SetRenderState(D3DRS_PSCONSTANT0_2, pixel_shader.PSConstant0[2]);
		D3DDevice_SetRenderState(D3DRS_PSCONSTANT0_5, pixel_shader.PSConstant0[5]);
		D3DDevice_SetRenderState(D3DRS_PSCONSTANT1_5, pixel_shader.PSConstant1[5]);
		D3DDevice_SetRenderState(D3DRS_PSRGBINPUTS3, pixel_shader.PSRGBInputs[3]);
		D3DDevice_SetRenderState(D3DRS_PSRGBINPUTS6, pixel_shader.PSRGBInputs[6]);
		D3DDevice_SetRenderState(D3DRS_PSRGBOUTPUTS6, pixel_shader.PSRGBOutputs[6]);
		D3DDevice_SetRenderState(D3DRS_PSRGBINPUTS7, pixel_shader.PSRGBInputs[7]);
		D3DDevice_SetRenderState(D3DRS_PSRGBOUTPUTS7, pixel_shader.PSRGBOutputs[7]);
		D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERCONSTANT0, pixel_shader.PSFinalCombinerConstant0);
		D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERCONSTANT1, pixel_shader.PSFinalCombinerConstant1);
		D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERINPUTSABCD, pixel_shader.PSFinalCombinerInputsABCD);
		D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERINPUTSEFG, pixel_shader.PSFinalCombinerInputsEFG);

		if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.pixel_shader_bytes += 44;
		}
	}

	return;
}

void rasterizer_model_draw_environment_shader(
	struct shader const *shader,
	short shader_permutation_index,
	struct triangle_buffer const *triangle_buffer,
	long dynamic_triangle_buffer_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer,
	long dynamic_vertex_buffer_index)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 245, global_d3d_device);

	if (rasterizer_debug_options.draw_models)
	{
		struct shader_environment const *shader_environment;
		real_vector3d forward;
		short vertex_shader_permutation;
		pixel32 cc0_pixel;
		pixel32 cc0_error_pixel;
		pixel32 cc1_pixel;
		boolean alpha_blended_decal = FALSE;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 251, local_parameters);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 252, shader);

		shader_environment = shader_environment_get(shader);
		vector_from_points3d(&global_window_parameters.camera.position, &local_parameters->centroid, &forward);

		if (TEST_FLAG(local_parameters->geometry_flags, _rasterizer_geometry_no_zbuffer_bit))
		{
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, FALSE);
		}
		else
		{
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, TRUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, !alpha_blended_decal);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		}

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, alpha_blended_decal ? rasterizer_debug_options.zbias : 0);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_CCW);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, alpha_blended_decal);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, !alpha_blended_decal && TEST_FLAG(shader_environment->environment.flags, _shader_environment_alpha_tested_bit));
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHAREF, 127);

		rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, shader_environment->environment.diffuse.base_map.index, shader_permutation_index);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		rasterizer_set_texture(1, _bitmap_type_2d, _bitmap_usage_detail, shader_environment->environment.diffuse.primary_detail_map.index, shader_permutation_index);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		rasterizer_set_texture(2, _bitmap_type_2d, _bitmap_usage_multiplicative, TEST_FLAG(shader_environment->environment.flags, _shader_environment_alpha_tested_bit) ? shader_environment->environment.diffuse.bump_map.index : NONE, shader_permutation_index);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		rasterizer_set_texture(3, _bitmap_type_cube_map, _bitmap_usage_additive, shader_environment->environment.reflection.map.index, shader_permutation_index);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		if (TEST_FLAG(shader_environment->environment.flags, _shader_environment_true_atmospheric_fog_bit))
		{
			vertex_shader_permutation = 2;
		}
		else if (local_planar_fog_flag)
		{
			vertex_shader_permutation = 0;
		}
		else if (local_parameters->lighting.point_light_count > 0)
		{
			vertex_shader_permutation = 1;
		}
		else if (shader_environment->environment.reflection.map.index == NONE && local_parameters->skinning.node_matrix_count <= 1)
		{
			vertex_shader_permutation = 3;
		}
		else
		{
			vertex_shader_permutation = 2;
		}

		rasterizer_set_vertex_shader_permutation(_vs_model, vertex_buffer ? vertex_buffer->type : rasterizer_dynamic_vertices_get_type(dynamic_vertex_buffer_index), vertex_shader_permutation);

		if (rasterizer_debug_options.statistics_mode >= _rasterizer_statistics_mode_fps_only)
		{
			rasterizer_frame_statistics.model_vertex_counts[vertex_shader_permutation] += vertex_buffer->count;
		}

		{
			real_argb_color reflection_view_perpendicular_color;
			real_argb_color reflection_view_parallel_color;
			real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4];
			real vsh_constants__effect[VSH_CONSTANTS__EFFECT_COUNT][4];

			reflection_view_perpendicular_color.alpha = shader_environment->environment.reflection.view_perpendicular_brightness * local_parameters->lighting.reflection_tint_color.alpha;
			reflection_view_perpendicular_color.red = shader_environment->environment.specular.view_perpendicular_color.red * local_parameters->lighting.reflection_tint_color.red;
			reflection_view_perpendicular_color.green = shader_environment->environment.specular.view_perpendicular_color.green * local_parameters->lighting.reflection_tint_color.green;
			reflection_view_perpendicular_color.blue = shader_environment->environment.specular.view_perpendicular_color.blue * local_parameters->lighting.reflection_tint_color.blue;
			reflection_view_parallel_color.alpha = shader_environment->environment.reflection.view_parallel_brightness * local_parameters->lighting.reflection_tint_color.alpha;
			reflection_view_parallel_color.red = shader_environment->environment.specular.view_parallel_color.red * local_parameters->lighting.reflection_tint_color.red;
			reflection_view_parallel_color.green = shader_environment->environment.specular.view_parallel_color.green * local_parameters->lighting.reflection_tint_color.green;
			reflection_view_parallel_color.blue = shader_environment->environment.specular.view_parallel_color.blue * local_parameters->lighting.reflection_tint_color.blue;

			vsh_constants__texscale[0][0] = shader_environment->environment.diffuse.primary_detail_map_scale;
			vsh_constants__texscale[0][1] = shader_environment->environment.diffuse.primary_detail_map_scale;
			vsh_constants__texscale[0][2] = 1.f;
			vsh_constants__texscale[0][3] = 1.f;
			vsh_constants__texscale[1][0] = local_parameters->base_map_scale.i;
			vsh_constants__texscale[1][1] = 0.f;
			vsh_constants__texscale[1][2] = 0.f;
			vsh_constants__texscale[1][3] = 0.f;
			vsh_constants__texscale[2][0] = 0.f;
			vsh_constants__texscale[2][1] = local_parameters->base_map_scale.j;
			vsh_constants__texscale[2][2] = 0.f;
			vsh_constants__texscale[2][3] = 0.f;

			vsh_constants__effect[0][0] = reflection_view_perpendicular_color.red - reflection_view_parallel_color.red;
			vsh_constants__effect[0][1] = reflection_view_perpendicular_color.green - reflection_view_parallel_color.green;
			vsh_constants__effect[0][2] = reflection_view_perpendicular_color.blue - reflection_view_parallel_color.blue;
			vsh_constants__effect[0][3] = reflection_view_perpendicular_color.alpha - reflection_view_parallel_color.alpha;
			vsh_constants__effect[1][0] = reflection_view_parallel_color.red;
			vsh_constants__effect[1][1] = reflection_view_parallel_color.green;
			vsh_constants__effect[1][2] = reflection_view_parallel_color.blue;
			vsh_constants__effect[1][3] = reflection_view_parallel_color.alpha;

			D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__TEXSCALE_OFFSET,
				vsh_constants__texscale,
				VSH_CONSTANTS__TEXSCALE_COUNT));
			D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__EFFECT_OFFSET,
				vsh_constants__effect,
				VSH_CONSTANTS__EFFECT_COUNT));
		}

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 436, global_window_parameters.fog.atmospheric_maximum_distance>global_window_parameters.fog.atmospheric_minimum_distance);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 437, global_window_parameters.fog.atmospheric_maximum_distance>0.0f);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 438, global_window_parameters.fog.planar_maximum_distance>0.0f);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 439, global_window_parameters.fog.planar_maximum_depth>0.0f);

		if (rasterizer_debug_options.draw_environment_fog && !TEST_FLAG(local_parameters->geometry_flags, _rasterizer_geometry_no_fog_bit))
		{
			real_argb_color cc0;
			real_rgb_color cc0_error;
			real_rgb_color cc1;
			real planar_distance = plane3d_distance_to_point(&global_window_parameters.fog.plane, &global_window_parameters.camera.position);
			real planar_eye_density = PIN(planar_distance / global_window_parameters.fog.atmospheric_maximum_distance, 0.f, 1.f);
			real atmospheric_distance = dot_product3d(&global_window_parameters.camera.forward, &forward);
			real atmospheric_density = PIN((atmospheric_distance - global_window_parameters.fog.atmospheric_minimum_distance) / (global_window_parameters.fog.atmospheric_maximum_distance - global_window_parameters.fog.atmospheric_minimum_distance), 0.f, 1.f) * global_window_parameters.fog.atmospheric_maximum_density;

			if (TEST_FLAG(global_window_parameters.fog.fog_definition_flags, _fog_definition_atmosphere_dominant_bit))
			{
				planar_eye_density = 1.f;
			}

			cc0.alpha = 1.f - atmospheric_density;
			cc0.red = global_window_parameters.fog.planar_color.red - ((1.f - planar_eye_density) * global_window_parameters.fog.atmospheric_color.red + planar_eye_density * global_window_parameters.fog.planar_color.red) * atmospheric_density;
			cc0.green = global_window_parameters.fog.planar_color.green - ((1.f - planar_eye_density) * global_window_parameters.fog.atmospheric_color.green + planar_eye_density * global_window_parameters.fog.planar_color.green) * atmospheric_density;
			cc0.blue = global_window_parameters.fog.planar_color.blue - ((1.f - planar_eye_density) * global_window_parameters.fog.atmospheric_color.blue + planar_eye_density * global_window_parameters.fog.planar_color.blue) * atmospheric_density;
			cc0_error.red = PIN(-cc0.red, 0.f, 1.f);
			cc0_error.green = PIN(-cc0.green, 0.f, 1.f);
			cc0_error.blue = PIN(-cc0.blue, 0.f, 1.f);
			cc0.red = PIN(cc0.red, 0.f, 1.f);
			cc0.green = PIN(cc0.green, 0.f, 1.f);
			cc0.blue = PIN(cc0.blue, 0.f, 1.f);
			cc1.red = global_window_parameters.fog.atmospheric_color.red * atmospheric_density;
			cc1.green = global_window_parameters.fog.atmospheric_color.green * atmospheric_density;
			cc1.blue = global_window_parameters.fog.atmospheric_color.blue * atmospheric_density;

			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 478, cc0.red >=0.0f && cc0.red <=1.0f);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 479, cc0.green>=0.0f && cc0.green<=1.0f);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 480, cc0.blue >=0.0f && cc0.blue <=1.0f);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 481, cc1.red >=0.0f && cc1.red <=1.0f);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 482, cc1.green>=0.0f && cc1.green<=1.0f);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 483, cc1.blue >=0.0f && cc1.blue <=1.0f);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 484, cc0_error.red >=0.0f && cc0_error.red <=1.0f);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 485, cc0_error.green>=0.0f && cc0_error.green<=1.0f);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 486, cc0_error.blue >=0.0f && cc0_error.blue <=1.0f);

			cc0_pixel = real_argb_color_to_pixel32(&cc0);
			cc0_error_pixel = real_rgb_color_to_pixel32(&cc0_error);
			cc1_pixel = real_rgb_color_to_pixel32(&cc1);
		}
		else
		{
			cc1_pixel = PIXEL32_FROM_ARGB(255, 0, 0, 0);
			cc0_error_pixel = PIXEL32_FROM_ARGB(255, 0, 0, 0);
			cc0_pixel = PIXEL32_FROM_ARGB(255, 0, 0, 0);
		}

		rasterizer_model_set_pixel_shader(
			0,
			shader_environment->environment.diffuse.detail_map_function,
			_shader_model_detail_mask_none,
			0xffffffff,
			cc0_pixel,
			cc0_error_pixel,
			cc1_pixel,
			FALSE,
			TEST_FLAG(shader_environment->environment.flags, _shader_environment_true_atmospheric_fog_bit));

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_CCW);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSRGBINPUTS0, PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_C1, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1 | PS_CHANNEL_ALPHA));
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSRGBOUTPUTS0, PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT));
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSFINALCOMBINERINPUTSEFG, PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T2 | PS_CHANNEL_ALPHA, 0));
		rasterizer_draw(triangle_buffer, dynamic_triangle_buffer_index, 0, triangle_count, vertex_buffer, dynamic_vertex_buffer_index);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSRGBINPUTS0, PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_C1, PS_REGISTER_T2, PS_REGISTER_C0));
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSRGBOUTPUTS0, PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT | PS_COMBINEROUTPUT_CD_DOT_PRODUCT));
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_PSFINALCOMBINERINPUTSEFG, PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, 0));

		if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.pixel_shader_bytes += 24;
			rasterizer_frame_statistics.model_primitive_count++;
			rasterizer_frame_statistics.model_triangle_count += triangle_count;
			rasterizer_frame_statistics.model_vertex_count += rasterizer_frame_statistics_count_static_vertices(triangle_buffer, vertex_buffer);
		}
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_model_draw_environment failed");
	}

	return;
}

void _rasterizer_models_begin(
	boolean sky)
{
	if (rasterizer_debug_options.draw_models)
	{
		local_pixel_shader_dirty_flag = TRUE;
		local_sky_flag = sky;

		if (sky)
		{
			rasterizer_profile_begin(_rasterizer_profile_model_sky);
		}
		else
		{
			rasterizer_profile_begin(_rasterizer_profile_models);
		}
	}

	return;
}

void _rasterizer_model_begin(
	struct rasterizer_model_begin_parameters const *parameters,
	boolean do_not_change_z_stencil_states)
{
	if (rasterizer_debug_options.draw_models)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 587, parameters);

		if (TEST_FLAG(parameters->geometry_flags, _rasterizer_geometry_first_person_bit) && !do_not_change_z_stencil_states)
		{
			rasterizer_set_stencil_mode(_rasterizer_stencil_mode_write);
			rasterizer_set_frustum_z(rasterizer_globals.z_near_first_person, rasterizer_globals.z_far_first_person);
		}

		local_parameters = parameters;
		local_parameters_queued_flag = FALSE;
		local_do_not_change_z_stencil_states = do_not_change_z_stencil_states;

		if (rasterizer_debug_options.active_camouflage_enabled &&
			global_window_parameters.rasterizer_target == _rasterizer_target_render_primary &&
			parameters->effect.type == _render_model_effect_type_active_camouflage &&
			parameters->effect.intensity > 0.f)
		{
			local_model_effect_type = _render_model_effect_type_active_camouflage;
		}
		else if (parameters->effect.type == _render_model_effect_type_transparent_zbuffered)
		{
			local_model_effect_type = _render_model_effect_type_transparent_zbuffered;
		}
		else
		{
			long lighting_bytes;
			long skinning_bytes = rasterizer_frame_statistics.skinning_bytes;

			rasterizer_set_model_skinning(&parameters->skinning);
			skinning_bytes = rasterizer_frame_statistics.skinning_bytes - skinning_bytes;
			lighting_bytes = rasterizer_frame_statistics.lighting_bytes;
			rasterizer_set_model_lighting(&parameters->lighting);
			rasterizer_frame_statistics.model_skinning_bytes += skinning_bytes;
			rasterizer_frame_statistics.model_lighting_bytes += rasterizer_frame_statistics.lighting_bytes - lighting_bytes;
			local_model_effect_type = _render_model_effect_type_none;
		}

		local_planar_fog_flag = global_window_parameters.fog.planar_mode &&
			!TEST_FLAG(parameters->geometry_flags, _rasterizer_geometry_no_fog_bit) &&
			(!TEST_FLAG(parameters->geometry_flags, _rasterizer_geometry_atmospheric_fog_but_no_planar_fog_bit) || dot_product3d((real_vector3d const *)&global_window_parameters.camera.position, &global_window_parameters.fog.plane.n) - global_window_parameters.fog.plane.d < 0.f);
		local_environment_fog_screen_flag = !local_sky_flag && rasterizer_environment_fog_screen_model_begin(parameters);

		if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
		{
			rasterizer_frame_statistics.model_count++;
		}
	}

	return;
}

void _rasterizer_model_draw(
	struct shader const *shader,
	short shader_permutation_index,
	struct triangle_buffer const *triangle_buffer,
	long dynamic_triangle_buffer_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer,
	long dynamic_vertex_buffer_index)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 671, global_d3d_device);

	if (rasterizer_debug_options.draw_models)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 675, local_parameters);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 676, shader);

		if (local_parameters->effect.modifier_shader)
		{
			boolean submit_modifier_shader = TRUE;

			if (local_parameters->effect.modifier_shader->base.type == _shader_type_transparent_plasma)
			{
				struct _shader_transparent_plasma const *plasma = &shader_transparent_plasma_get(local_parameters->effect.modifier_shader)->plasma;

				if (plasma->intensity_source >= 1 &&
					plasma->intensity_source <= 4 &&
					local_parameters->effect.modifier_animation.values &&
					local_parameters->effect.modifier_animation.values[plasma->intensity_source - 1] == 0.f)
				{
					submit_modifier_shader = FALSE;
				}
			}

			if (submit_modifier_shader)
			{
				struct transparent_geometry_group *group = _rasterizer_model_transparent_geometry_submit(
					local_parameters->effect.modifier_shader,
					shader_permutation_index,
					triangle_buffer,
					dynamic_triangle_buffer_index,
					triangle_count,
					vertex_buffer,
					dynamic_vertex_buffer_index,
					&local_parameters->centroid,
					NULL);

				if (group)
				{
					group->animation = rasterizer_memory_alloc(&local_parameters->effect.modifier_animation, sizeof(struct render_animation));
				}
			}
		}

		if (local_model_effect_type == _render_model_effect_type_active_camouflage)
		{
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 729, shader->base.type==_shader_type_model);

			rasterizer_model_transparent_geometry_submit(
				shader,
				shader_permutation_index,
				triangle_buffer,
				dynamic_triangle_buffer_index,
				triangle_count,
				vertex_buffer,
				dynamic_vertex_buffer_index,
				&local_parameters->centroid,
				NULL);
			rasterizer_active_camouflage_set_visibility(TRUE);
		}
		else if (local_model_effect_type == _render_model_effect_type_none)
		{
			if (shader->base.type == _shader_type_environment)
			{
				rasterizer_model_draw_environment_shader(
					shader,
					shader_permutation_index,
					triangle_buffer,
					dynamic_triangle_buffer_index,
					triangle_count,
					vertex_buffer,
					dynamic_vertex_buffer_index);
			}
			else
			{
				real_vector3d forward;
				real distance;
				real reflection_scale;
				real_rgb_color self_illumination_color;
				real_rgb_color diffuse_change_color;
				short vertex_shader_permutation;
				long vertex_shader_bytes;
				pixel32 cc0_pixel;
				pixel32 cc0_error_pixel;
				pixel32 cc1_pixel;
				struct shader_model const *shader_model = shader_model_get(shader);
				boolean alpha_blended_decal = TEST_FLAG(shader_model->model.flags, _shader_model_alpha_blended_decal_bit);

				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 771, local_model_effect_type==_render_model_effect_type_none);

				vector_from_points3d(&global_window_parameters.camera.position, &local_parameters->centroid, &forward);
				distance = dot_product3d(&global_window_parameters.camera.forward, &forward);
				reflection_scale = shader_model->model.reflection_cutoff_distance != 0.f ? PIN((distance - shader_model->model.reflection_cutoff_distance) / (shader_model->model.reflection_falloff_distance - shader_model->model.reflection_cutoff_distance), 0.f, 1.f) : 1.f;

				if (TEST_FLAG(local_parameters->geometry_flags, _rasterizer_geometry_no_zbuffer_bit))
				{
					SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
					SetRenderStateSmart(D3DRS_ZBIAS, 0);
				}
				else
				{
					SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
					SetRenderStateSmart(D3DRS_ZWRITEENABLE, !alpha_blended_decal);
					SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
					SetRenderStateSmart(D3DRS_ZBIAS, alpha_blended_decal ? rasterizer_debug_options.zbias : 0);
				}

				SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
				SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
				SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, alpha_blended_decal);
				SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				SetRenderStateSmart(D3DRS_ALPHATESTENABLE, !alpha_blended_decal && !TEST_FLAG(shader_model->model.flags, _shader_model_not_alpha_tested_bit));
				SetRenderStateSmart(D3DRS_ALPHAREF, 127);

				rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, shader_model->model.base_map.index, shader_permutation_index);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
				SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
				SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				rasterizer_set_texture(1, _bitmap_type_2d, _bitmap_usage_detail, shader_model->model.detail_map.index, shader_permutation_index);
				SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
				SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
				SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				rasterizer_set_texture(2, _bitmap_type_2d, _bitmap_usage_multiplicative, shader_model->model.multipurpose_map.index, shader_permutation_index);
				SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
				SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
				SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				rasterizer_set_texture(3, _bitmap_type_cube_map, _bitmap_usage_additive, shader_model->model.reflection_map.index, shader_permutation_index);
				SetTextureStageStateSmart(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
				SetTextureStageStateSmart(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				SetTextureStageStateSmart(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				{
					real_vector3d color_delta;
					unsigned long seed = local_parameters->unique_id;
					real animation_phase = TEST_FLAG(shader_model->model.self_illumination_flags, _shader_model_self_illumination_no_random_phase_bit) ? 0.f : real_seed_random(&seed);

					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 864, shader_model->model.self_illumination_animation_period!=0.0f);

					subtract_vectors3d((real_vector3d *)&shader_model->model.self_illumination_animation_color_upper_bound, (real_vector3d *)&shader_model->model.self_illumination_animation_color_lower_bound, &color_delta);
					vector_from_line3d(
						&shader_model->model.self_illumination_animation_color_lower_bound,
						&color_delta,
						periodic_function_evaluate(shader_model->model.self_illumination_animation_function, global_frame_parameters.game_time_sec / shader_model->model.self_illumination_animation_period + animation_phase),
						&self_illumination_color);
				}

				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 876, self_illumination_color.red >=0.0f && self_illumination_color.red <=1.0f);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 877, self_illumination_color.green>=0.0f && self_illumination_color.green<=1.0f);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 878, self_illumination_color.blue >=0.0f && self_illumination_color.blue <=1.0f);

				if (shader_model->model.self_illumination_color_source > 0 && shader_model->model.self_illumination_color_source < 5)
				{
					real_rgb_color const *external_color = &local_parameters->animation.colors[shader_model->model.self_illumination_color_source - 1];

					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 887, external_color->red >=0.0f && external_color->red <=1.0f);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 888, external_color->green>=0.0f && external_color->green<=1.0f);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 889, external_color->blue >=0.0f && external_color->blue <=1.0f);

					self_illumination_color.red *= external_color->red;
					self_illumination_color.green *= external_color->green;
					self_illumination_color.blue *= external_color->blue;
				}

				if (shader_model->model.diffuse_change_color_source > 0 && shader_model->model.diffuse_change_color_source < 5)
				{
					diffuse_change_color = local_parameters->animation.colors[shader_model->model.diffuse_change_color_source - 1];

					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 904, diffuse_change_color.red >=0.0f && diffuse_change_color.red <=1.0f);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 905, diffuse_change_color.green>=0.0f && diffuse_change_color.green<=1.0f);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 906, diffuse_change_color.blue >=0.0f && diffuse_change_color.blue <=1.0f);
				}
				else
				{
					diffuse_change_color = *global_real_rgb_white;
				}

				vertex_shader_bytes = rasterizer_frame_statistics.vertex_shader_bytes;

				if (TEST_FLAG(shader_model->model.flags, _shader_model_true_atmospheric_fog_bit))
				{
					vertex_shader_permutation = 2;
				}
				else if (local_planar_fog_flag)
				{
					vertex_shader_permutation = 0;
				}
				else if (local_parameters->lighting.point_light_count > 0)
				{
					vertex_shader_permutation = 1;
				}
				else if (local_parameters->skinning.node_matrix_count <= 1 &&
					(shader_model->model.multipurpose_map.index == NONE ||
					(shader_model->model.detail_mask == _shader_model_detail_mask_none &&
					self_illumination_color.red == 0.f &&
					self_illumination_color.green == 0.f &&
					self_illumination_color.blue == 0.f &&
					diffuse_change_color.red == 1.f &&
					diffuse_change_color.green == 1.f &&
					diffuse_change_color.blue == 1.f)) &&
					(shader_model->model.reflection_map.index == NONE || !(reflection_scale > 0.f)))
				{
					vertex_shader_permutation = 3;
				}
				else
				{
					vertex_shader_permutation = 2;
				}

				rasterizer_set_vertex_shader_permutation(_vs_model, vertex_buffer ? vertex_buffer->type : rasterizer_dynamic_vertices_get_type(dynamic_vertex_buffer_index), vertex_shader_permutation);

				if (rasterizer_debug_options.statistics_mode >= _rasterizer_statistics_mode_fps_only)
				{
					rasterizer_frame_statistics.model_vertex_shader_bytes += rasterizer_frame_statistics.vertex_shader_bytes - vertex_shader_bytes;
					rasterizer_frame_statistics.model_vertex_counts[vertex_shader_permutation] += vertex_buffer->count;
				}

				{
					real_argb_color reflection_view_perpendicular_color;
					real_argb_color reflection_view_parallel_color;
					real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4];
					real vsh_constants__effect[VSH_CONSTANTS__EFFECT_COUNT][4];

					reflection_view_perpendicular_color.alpha = (reflection_scale * shader_model->model.reflection_view_perpendicular_color.alpha) * local_parameters->lighting.reflection_tint_color.alpha;
					reflection_view_perpendicular_color.red = shader_model->model.reflection_view_perpendicular_color.red * local_parameters->lighting.reflection_tint_color.red;
					reflection_view_perpendicular_color.green = shader_model->model.reflection_view_perpendicular_color.green * local_parameters->lighting.reflection_tint_color.green;
					reflection_view_perpendicular_color.blue = shader_model->model.reflection_view_perpendicular_color.blue * local_parameters->lighting.reflection_tint_color.blue;
					reflection_view_parallel_color.alpha = (reflection_scale * shader_model->model.reflection_view_parallel_color.alpha) * local_parameters->lighting.reflection_tint_color.alpha;
					reflection_view_parallel_color.red = shader_model->model.reflection_view_parallel_color.red * local_parameters->lighting.reflection_tint_color.red;
					reflection_view_parallel_color.green = shader_model->model.reflection_view_parallel_color.green * local_parameters->lighting.reflection_tint_color.green;
					reflection_view_parallel_color.blue = shader_model->model.reflection_view_parallel_color.blue * local_parameters->lighting.reflection_tint_color.blue;

					vsh_constants__texscale[0][0] = shader_model->model.detail_map_scale;
					vsh_constants__texscale[0][1] = shader_model->model.detail_map_scale * shader_model->model.detail_map_v_scale;
					vsh_constants__texscale[0][2] = 1.f;
					vsh_constants__texscale[0][3] = 1.f;
					vsh_constants__texscale[1][0] = 1.f;
					vsh_constants__texscale[1][1] = 0.f;
					vsh_constants__texscale[1][2] = 0.f;
					vsh_constants__texscale[1][3] = 0.f;
					vsh_constants__texscale[2][0] = 0.f;
					vsh_constants__texscale[2][1] = 1.f;
					vsh_constants__texscale[2][2] = 0.f;
					vsh_constants__texscale[2][3] = 0.f;

					vsh_constants__effect[0][0] = reflection_view_perpendicular_color.red - reflection_view_parallel_color.red;
					vsh_constants__effect[0][1] = reflection_view_perpendicular_color.green - reflection_view_parallel_color.green;
					vsh_constants__effect[0][2] = reflection_view_perpendicular_color.blue - reflection_view_parallel_color.blue;
					vsh_constants__effect[0][3] = reflection_view_perpendicular_color.alpha - reflection_view_parallel_color.alpha;
					vsh_constants__effect[1][0] = reflection_view_parallel_color.red;
					vsh_constants__effect[1][1] = reflection_view_parallel_color.green;
					vsh_constants__effect[1][2] = reflection_view_parallel_color.blue;
					vsh_constants__effect[1][3] = reflection_view_parallel_color.alpha;

					shader_texture_animation_evaluate(
						&shader_model->model.animation,
						&local_parameters->animation,
						local_parameters->base_map_scale.i * shader_model->model.map_scale.i,
						local_parameters->base_map_scale.j * shader_model->model.map_scale.j,
						0.f,
						0.f,
						0.f,
						global_frame_parameters.game_time_sec,
						(real_vector4d *)vsh_constants__texscale[1],
						(real_vector4d *)vsh_constants__texscale[2]);
					vsh_constants__texscale[2][2] = shader_model->model.translucency;

					D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
						VSH_CONSTANTS__TEXSCALE_OFFSET,
						vsh_constants__texscale,
						VSH_CONSTANTS__TEXSCALE_COUNT));
					D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
						VSH_CONSTANTS__EFFECT_OFFSET,
						vsh_constants__effect,
						VSH_CONSTANTS__EFFECT_COUNT));

					if (global_rasterizer_model_ambient_reflection_tint &&
						(global_rasterizer_model_ambient_reflection_tint->alpha > 0.f ||
						global_rasterizer_model_ambient_reflection_tint->red > 0.f ||
						global_rasterizer_model_ambient_reflection_tint->green > 0.f ||
						global_rasterizer_model_ambient_reflection_tint->blue > 0.f))
					{
						vsh_constants__effect[0][0] = 0.f;
						vsh_constants__effect[0][1] = 0.f;
						vsh_constants__effect[0][2] = 0.f;
						vsh_constants__effect[0][3] = 0.f;
						vsh_constants__effect[1][0] = global_rasterizer_model_ambient_reflection_tint->alpha;
						vsh_constants__effect[1][1] = global_rasterizer_model_ambient_reflection_tint->red;
						vsh_constants__effect[1][2] = global_rasterizer_model_ambient_reflection_tint->green;
						vsh_constants__effect[1][3] = global_rasterizer_model_ambient_reflection_tint->blue;

						D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
							VSH_CONSTANTS__EFFECT_OFFSET,
							vsh_constants__effect,
							VSH_CONSTANTS__EFFECT_COUNT));
					}
				}

				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1074, global_window_parameters.fog.atmospheric_maximum_distance>global_window_parameters.fog.atmospheric_minimum_distance);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1075, global_window_parameters.fog.atmospheric_maximum_distance>0.0f);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1076, global_window_parameters.fog.planar_maximum_distance>0.0f);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1077, global_window_parameters.fog.planar_maximum_depth>0.0f);

				if (rasterizer_debug_options.draw_environment_fog && !TEST_FLAG(local_parameters->geometry_flags, _rasterizer_geometry_no_fog_bit))
				{
					real_argb_color cc0;
					real_rgb_color cc0_error;
					real_rgb_color cc1;
					real planar_distance = plane3d_distance_to_point(&global_window_parameters.fog.plane, &global_window_parameters.camera.position);
					real planar_eye_density = PIN(planar_distance / global_window_parameters.fog.atmospheric_maximum_distance, 0.f, 1.f);
					real atmospheric_density = PIN((distance - global_window_parameters.fog.atmospheric_minimum_distance) / (global_window_parameters.fog.atmospheric_maximum_distance - global_window_parameters.fog.atmospheric_minimum_distance), 0.f, 1.f) * global_window_parameters.fog.atmospheric_maximum_density;

					if (TEST_FLAG(global_window_parameters.fog.fog_definition_flags, _fog_definition_atmosphere_dominant_bit))
					{
						planar_eye_density = 1.f;
					}

					cc0.alpha = 1.f - atmospheric_density;
					cc0.red = global_window_parameters.fog.planar_color.red - ((1.f - planar_eye_density) * global_window_parameters.fog.atmospheric_color.red + planar_eye_density * global_window_parameters.fog.planar_color.red) * atmospheric_density;
					cc0.green = global_window_parameters.fog.planar_color.green - ((1.f - planar_eye_density) * global_window_parameters.fog.atmospheric_color.green + planar_eye_density * global_window_parameters.fog.planar_color.green) * atmospheric_density;
					cc0.blue = global_window_parameters.fog.planar_color.blue - ((1.f - planar_eye_density) * global_window_parameters.fog.atmospheric_color.blue + planar_eye_density * global_window_parameters.fog.planar_color.blue) * atmospheric_density;
					cc0_error.red = PIN(-cc0.red, 0.f, 1.f);
					cc0_error.green = PIN(-cc0.green, 0.f, 1.f);
					cc0_error.blue = PIN(-cc0.blue, 0.f, 1.f);
					cc0.red = PIN(cc0.red, 0.f, 1.f);
					cc0.green = PIN(cc0.green, 0.f, 1.f);
					cc0.blue = PIN(cc0.blue, 0.f, 1.f);
					cc1.red = global_window_parameters.fog.atmospheric_color.red * atmospheric_density;
					cc1.green = global_window_parameters.fog.atmospheric_color.green * atmospheric_density;
					cc1.blue = global_window_parameters.fog.atmospheric_color.blue * atmospheric_density;

					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1115, cc0.red >=0.0f && cc0.red <=1.0f);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1116, cc0.green>=0.0f && cc0.green<=1.0f);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1117, cc0.blue >=0.0f && cc0.blue <=1.0f);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1118, cc1.red >=0.0f && cc1.red <=1.0f);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1119, cc1.green>=0.0f && cc1.green<=1.0f);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1120, cc1.blue >=0.0f && cc1.blue <=1.0f);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1121, cc0_error.red >=0.0f && cc0_error.red <=1.0f);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1122, cc0_error.green>=0.0f && cc0_error.green<=1.0f);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1123, cc0_error.blue >=0.0f && cc0_error.blue <=1.0f);

					cc0_pixel = real_argb_color_to_pixel32(&cc0);
					cc0_error_pixel = real_rgb_color_to_pixel32(&cc0_error);
					cc1_pixel = real_rgb_color_to_pixel32(&cc1);
				}
				else
				{
					cc1_pixel = PIXEL32_FROM_ARGB(255, 0, 0, 0);
					cc0_error_pixel = PIXEL32_FROM_ARGB(255, 0, 0, 0);
					cc0_pixel = PIXEL32_FROM_ARGB(255, 0, 0, 0);
				}

				rasterizer_model_set_pixel_shader(
					real_rgb_color_to_pixel32(&self_illumination_color),
					shader_model->model.detail_function,
					shader_model->model.detail_mask,
					real_rgb_color_to_pixel32(&diffuse_change_color),
					cc0_pixel,
					cc0_error_pixel,
					cc1_pixel,
					TEST_FLAG(shader_model->model.flags, _shader_model_detail_after_reflection_bit),
					TEST_FLAG(shader_model->model.flags, _shader_model_true_atmospheric_fog_bit));
				SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
				rasterizer_draw(triangle_buffer, dynamic_triangle_buffer_index, 0, triangle_count, vertex_buffer, dynamic_vertex_buffer_index);

				if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
				{
					rasterizer_frame_statistics.model_primitive_count++;
					rasterizer_frame_statistics.model_triangle_count += triangle_count;
					rasterizer_frame_statistics.model_vertex_count += rasterizer_frame_statistics_count_static_vertices(triangle_buffer, vertex_buffer);
				}

				if (TEST_FLAG(shader_model->model.flags, _shader_model_two_sided_bit))
				{
					real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4];

					vsh_constants__texscale[0][0] = shader_model->model.detail_map_scale;
					vsh_constants__texscale[0][1] = shader_model->model.detail_map_scale * shader_model->model.detail_map_v_scale;
					vsh_constants__texscale[0][2] = 1.f;
					vsh_constants__texscale[0][3] = -1.f;
					vsh_constants__texscale[1][0] = 1.f;
					vsh_constants__texscale[1][1] = 0.f;
					vsh_constants__texscale[1][2] = 0.f;
					vsh_constants__texscale[1][3] = 0.f;
					vsh_constants__texscale[2][0] = 0.f;
					vsh_constants__texscale[2][1] = 1.f;
					vsh_constants__texscale[2][2] = 0.f;
					vsh_constants__texscale[2][3] = 0.f;

					shader_texture_animation_evaluate(
						&shader_model->model.animation,
						&local_parameters->animation,
						local_parameters->base_map_scale.i * shader_model->model.map_scale.i,
						local_parameters->base_map_scale.j * shader_model->model.map_scale.j,
						0.f,
						0.f,
						0.f,
						global_frame_parameters.game_time_sec,
						(real_vector4d *)vsh_constants__texscale[1],
						(real_vector4d *)vsh_constants__texscale[2]);
					vsh_constants__texscale[2][2] = shader_model->model.translucency;

					D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
						VSH_CONSTANTS__TEXSCALE_OFFSET,
						vsh_constants__texscale,
						VSH_CONSTANTS__TEXSCALE_COUNT));
					SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CW);
					rasterizer_draw(triangle_buffer, dynamic_triangle_buffer_index, 0, triangle_count, vertex_buffer, dynamic_vertex_buffer_index);

					if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
					{
						rasterizer_frame_statistics.model_primitive_count++;
						rasterizer_frame_statistics.model_triangle_count += triangle_count;
						rasterizer_frame_statistics.model_vertex_count += rasterizer_frame_statistics_count_static_vertices(triangle_buffer, vertex_buffer);
					}
				}
			}

			if (local_environment_fog_screen_flag)
			{
				rasterizer_environment_fog_screen_model_submit(
					shader,
					shader_permutation_index,
					triangle_buffer,
					dynamic_triangle_buffer_index,
					triangle_count,
					vertex_buffer,
					dynamic_vertex_buffer_index);
			}
		}
		else
		{
			error(_error_silent, "### ERROR model effect type #%d can't render opaque shader", local_model_effect_type);
		}
	}

	if (!success)
	{
		error(_error_silent, "### ERROR rasterizer_model_draw failed");
	}

	return;
}

struct transparent_geometry_group *_rasterizer_model_transparent_geometry_submit(
	struct shader const *shader,
	short shader_permutation_index,
	struct triangle_buffer const *triangle_buffer,
	long dynamic_triangle_buffer_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer,
	long dynamic_vertex_buffer_index,
	real_point3d const *centroid,
	struct render_sort_filth *sort_filth)
{
	struct transparent_geometry_group *result = NULL;

	if (rasterizer_debug_options.draw_models && rasterizer_debug_options.draw_model_transparent_geometry)
	{
		boolean alpha_blended_decal = shader && shader->base.type == _shader_type_model && TEST_FLAG(shader_model_get(shader)->model.flags, _shader_model_alpha_blended_decal_bit);
		boolean draw_immediately_allowed = local_model_effect_type != _render_model_effect_type_active_camouflage || (shader && shader->base.type == _shader_type_model && shader_model_get(shader)->model.flags);

		if (!alpha_blended_decal)
		{
			struct transparent_geometry_group *group;
			unsigned long geometry_flags;
			static boolean warned = FALSE;

			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1301, shader);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1302, shader_type_is_valid_for_model(shader->base.type));
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1303, centroid);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1304, local_parameters);

			geometry_flags = local_parameters->geometry_flags;

			if (draw_immediately_allowed && shader_is_decal(shader))
			{
				SET_FLAG(geometry_flags, _rasterizer_geometry_no_sort_bit, TRUE);
				SET_FLAG(geometry_flags, _rasterizer_geometry_no_queue_bit, TRUE);
			}

			if (draw_immediately_allowed && TEST_FLAG(geometry_flags, _rasterizer_geometry_no_queue_bit))
			{
				group = &local_immediate_group;
				group->sorted_index = NONE;
			}
			else
			{
				if (local_model_effect_type == _render_model_effect_type_active_camouflage && shader->base.type != _shader_type_model)
				{
					group = rasterizer_transparent_geometry_new_group2();
				}
				else
				{
					result = rasterizer_transparent_geometry_new_group();
					group = result;
				}

				if (sort_filth)
				{
					sort_filth->group_index = rasterizer_transparent_geometry_get_group_presorted_index(group);
					sort_filth->prev_group_presorted_index_reference = &group->prev_group_presorted_index;
					sort_filth->next_group_presorted_index_reference = &group->next_group_presorted_index;
				}
			}

			if (group)
			{
				real_vector3d forward;
				real_plane3d zero_plane = {{0.f, 0.f, 0.f}, 0.f};

				group->geometry_flags = geometry_flags;
				group->object_index = local_parameters->unique_id;

				if (local_parameters->effect.type == _render_model_effect_type_none)
				{
					group->source_object_index = 0;
					group->centroid = *centroid;
				}
				else
				{
					match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1356, local_parameters->effect.source_object_index!=0);

					group->source_object_index = local_parameters->effect.source_object_index;
					group->centroid = local_parameters->effect.source_object_centroid;
				}

				group->shader_permutation_index = shader_permutation_index;
				group->shader = shader;
				group->effect = local_parameters->effect;
				group->dynamic_triangle_buffer_index = dynamic_triangle_buffer_index;
				group->triangle_buffer = triangle_buffer;
				group->triangle_count = triangle_count;
				group->dynamic_vertex_buffer_index = dynamic_vertex_buffer_index;
				group->vertex_buffers = vertex_buffer;
				group->first_triangle_index = 0;
				group->lightmap = NULL;
				vector_from_points3d(&global_window_parameters.camera.position, &group->centroid, &forward);
				group->z_sort = -dot_product3d(&forward, &global_window_parameters.camera.forward);
				group->plane = zero_plane;
				group->model_base_map_scale = local_parameters->base_map_scale;
				group->prev_group_presorted_index = NONE;
				group->next_group_presorted_index = NONE;

				if (local_model_effect_type == _render_model_effect_type_active_camouflage)
				{
					if (shader->base.type == _shader_type_model)
					{
						group->active_camouflage_transparent_source_object_index = 0;
					}
					else
					{
						group->active_camouflage_transparent_source_object_index = local_parameters->effect.source_object_index;
						match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1389, group->active_camouflage_transparent_source_object_index);
					}
				}
				else
				{
					group->active_camouflage_transparent_source_object_index = 0;
				}

				group->cortana_hack = rasterizer_model_cortana_hack;

				if (TEST_FLAG(geometry_flags, _rasterizer_geometry_no_queue_bit))
				{
					group->node_matrices = local_parameters->skinning.node_matrices;
					group->node_matrix_count = local_parameters->skinning.node_matrix_count;
					group->lighting = &local_parameters->lighting;
					group->animation = &local_parameters->animation;
					rasterizer_transparent_geometry_groups_begin();
					rasterizer_transparent_geometry_group_draw(group, FALSE);
					rasterizer_transparent_geometry_groups_end();
					local_pixel_shader_dirty_flag = TRUE;
				}
				else
				{
					if (!local_parameters_queued_flag)
					{
						local_queued_node_matrices = rasterizer_memory_alloc_const(local_parameters->skinning.node_matrices, sizeof(*local_parameters->skinning.node_matrices) * local_parameters->skinning.node_matrix_count);
						local_queued_node_matrix_count = local_parameters->skinning.node_matrix_count;
						local_queued_lighting = rasterizer_memory_alloc_const(&local_parameters->lighting, sizeof(local_parameters->lighting));
						local_queued_animation = rasterizer_memory_alloc_const(&local_parameters->animation, sizeof(local_parameters->animation));
						local_parameters_queued_flag = TRUE;
					}

					group->node_matrices = local_queued_node_matrices;
					group->node_matrix_count = local_queued_node_matrix_count;
					group->lighting = local_queued_lighting;
					group->animation = local_queued_animation;
				}

				if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
				{
					rasterizer_frame_statistics.model_transparent_primitive_count++;
					rasterizer_frame_statistics.model_transparent_triangle_count += triangle_count;

					if (triangle_count > rasterizer_frame_statistics.model_transparent_maximum_primitive_triangle_count)
					{
						rasterizer_frame_statistics.model_transparent_maximum_primitive_triangle_count = triangle_count;
					}

					rasterizer_frame_statistics.model_transparent_vertex_count += rasterizer_frame_statistics_count_static_vertices(triangle_buffer, vertex_buffer);
				}
			}
			else if (!warned)
			{
				error(_error_silent, "### ERROR too many transparent geometry groups");
				warned = TRUE;
			}
		}
		else if (sort_filth)
		{
			sort_filth->group_index = NONE;
			sort_filth->prev_group_presorted_index_reference = NULL;
			sort_filth->next_group_presorted_index_reference = NULL;
		}
	}

	return result;
}

void _rasterizer_model_end(
	void)
{
	if (rasterizer_debug_options.draw_models)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_models.c", 1491, local_parameters);

		if (local_environment_fog_screen_flag)
		{
			rasterizer_environment_fog_screen_model_end();
		}

		if (TEST_FLAG(local_parameters->geometry_flags, _rasterizer_geometry_first_person_bit) && !local_do_not_change_z_stencil_states)
		{
			rasterizer_set_stencil_mode(_rasterizer_stencil_mode_reject);
			rasterizer_set_frustum_z(0.f, 0.f);
		}

		local_parameters = NULL;
	}

	return;
}

void _rasterizer_models_end(
	void)
{
	if (rasterizer_debug_options.draw_models)
	{
		if (local_sky_flag)
		{
			rasterizer_profile_end(_rasterizer_profile_model_sky);
		}
		else
		{
			rasterizer_profile_end(_rasterizer_profile_models);
		}
	}

	return;
}

void rasterizer_model_ambient_reflection_tint(
	real brightness,
	real red,
	real green,
	real blue)
{
	if (global_rasterizer_model_ambient_reflection_tint)
	{
		global_rasterizer_model_ambient_reflection_tint->alpha = brightness;
		global_rasterizer_model_ambient_reflection_tint->red = red;
		global_rasterizer_model_ambient_reflection_tint->green = green;
		global_rasterizer_model_ambient_reflection_tint->blue = blue;
	}

	return;
}

/* ---------- private code */
