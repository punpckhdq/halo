/*
RASTERIZER_XBOX_DYNAVOBGEOM.C
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

/* ---------- public code */

void _rasterizer_hud_begin(
	void)
{
	rasterizer_profile_begin(_rasterizer_profile_hud);

	return;
}

void _rasterizer_hud_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_hud);

	return;
}

void _rasterizer_dynamic_unlit_geometry_draw(
	struct shader const *shader,
	struct bitmap_data const *primary_map,
	struct render_animation const *animation,
	long dynamic_triangle_buffer_index,
	long dynamic_vertex_buffer_index,
	long triangle_count,
	real_point3d const *centroid,
	unsigned long geometry_flags)
{
	static boolean warned = FALSE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 38, global_d3d_device);

	if (rasterizer_debug_options.draw_dynamic_unlit_geometry)
	{
		struct transparent_geometry_group *group;
		real_plane3d zero_plane;
		real_vector3d forward;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 42, !TEST_FLAG(geometry_flags, _rasterizer_geometry_viewspace_bit) || shader->base.type==_shader_type_effect);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 44, shader->base.type==_shader_type_effect);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 52, centroid);

		vector_from_points3d(&global_window_parameters.camera.position, centroid, &forward);
		group = rasterizer_transparent_geometry_new_group();

		if (group)
		{
			zero_plane.n.i = 0.f;
			zero_plane.n.j = 0.f;
			zero_plane.n.k = 0.f;
			zero_plane.d = 0.f;

			group->geometry_flags = geometry_flags;
			group->object_index = 0;
			group->source_object_index = 0;
			group->shader = shader;
			group->shader_permutation_index = 0;
			group->effect.type = _render_model_effect_type_none;
			group->dynamic_triangle_buffer_index = dynamic_triangle_buffer_index;
			group->triangle_buffer = NULL;
			group->first_triangle_index = 0;
			group->triangle_count = triangle_count;
			group->dynamic_vertex_buffer_index = dynamic_vertex_buffer_index;
			group->vertex_buffers = NULL;
			group->lightmap = primary_map;
			group->z_sort = -dot_product3d(&global_window_parameters.camera.forward, &forward);
			group->centroid = *centroid;
			group->plane = zero_plane;
			group->model_base_map_scale.j = 1.f;
			group->model_base_map_scale.i = 1.f;
			group->prev_group_presorted_index = NONE;
			group->next_group_presorted_index = NONE;
			group->active_camouflage_transparent_source_object_index = 0;
			group->cortana_hack = FALSE;

			if (shader->base.type == _shader_type_effect)
			{
				struct shader_effect const *shader_effect = shader_get_and_verify_type(shader, _shader_type_effect);

				if (TEST_FLAG(shader_effect->effect.flags, _shader_effect_sort_bias_bit))
				{
					group->z_sort += 0.25f;
				}
			}

			group->node_matrices = NULL;
			group->node_matrix_count = 0;
			group->lighting = NULL;
			group->animation = NULL;

			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_full)
			{
				rasterizer_frame_statistics.dynamic_unlit_primitive_count++;
				rasterizer_frame_statistics.dynamic_unlit_triangle_count += triangle_count;

				if (triangle_count > rasterizer_frame_statistics.dynamic_unlit_maximum_primitive_triangle_count)
				{
					rasterizer_frame_statistics.dynamic_unlit_maximum_primitive_triangle_count = triangle_count;
				}

				rasterizer_frame_statistics.dynamic_unlit_vertex_count += rasterizer_frame_statistics_count_dynamic_vertices(dynamic_triangle_buffer_index, 0, triangle_count);
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

void _rasterizer_dynamic_lit_geometry_draw(
	struct shader const *shader,
	struct bitmap_data const *primary_map,
	struct render_animation const *animation,
	struct render_lighting const *lighting,
	long dynamic_triangle_buffer_index,
	long dynamic_vertex_buffer_index,
	long triangle_count,
	real_point3d const *centroid,
	unsigned long geometry_flags)
{
	return;
}

void _rasterizer_dynamic_screen_geometry_add_multitexture_params_to_base(
	struct rasterizer_dynamic_screen_geometry_parameters *base,
	struct rasterizer_dynamic_screen_geometry_parameters const *multitex_params)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 220, base);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 221, multitex_params);

	base->offset = multitex_params->offset;
	base->map_anchor_screen[1] = multitex_params->map_anchor_screen[1];
	base->map_anchor_screen[2] = multitex_params->map_anchor_screen[2];
	base->map[1] = multitex_params->map[1];
	base->map[2] = multitex_params->map[2];
	base->map_wrapped[1] = multitex_params->map_wrapped[1];
	base->map_wrapped[2] = multitex_params->map_wrapped[2];
	base->map_offset[1] = multitex_params->map_offset[1];
	base->map_offset[2] = multitex_params->map_offset[2];
	base->map_scale[1] = multitex_params->map_scale[1];
	base->map_scale[2] = multitex_params->map_scale[2];
	base->map_texture_scale[1] = multitex_params->map_texture_scale[1];
	base->map_texture_scale[2] = multitex_params->map_texture_scale[2];
	base->map_tint[1] = multitex_params->map_tint[1];
	base->map_tint[2] = multitex_params->map_tint[2];
	base->map_fade[1] = multitex_params->map_fade[1];
	base->map_fade[2] = multitex_params->map_fade[2];
	base->map0_to_1_blend_function = multitex_params->map0_to_1_blend_function;
	base->map1_to_2_blend_function = multitex_params->map1_to_2_blend_function;

	return;
}

void _rasterizer_dynamic_screen_geometry_draw(
	struct rasterizer_dynamic_screen_geometry_parameters const *parameters,
	long dynamic_triangle_buffer_index,
	long dynamic_vertex_buffer_index,
	long triangle_count)
{
	match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 255, FALSE, "_rasterizer_dynamic_screen_geometry_draw not supported no mo'");

	return;
}

static void submit_screen_vertex(
	struct dynamic_screen_vertex const *vertex)
{
	IDirect3DDevice8_SetVertexDataColor(global_d3d_device, D3DVSDE_TEXCOORD0, vertex->color);
	IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, vertex->texcoord.x, vertex->texcoord.y);
	IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, vertex->position.x, vertex->position.y);

	return;
}

void _rasterizer_psuedo_dynamic_screen_quad_draw(
	struct rasterizer_dynamic_screen_geometry_parameters const *parameters,
	struct dynamic_screen_vertex *verts)
{
	short map_index;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 756, global_d3d_device);

	if (rasterizer_debug_options.draw_dynamic_screen_geometry && global_window_parameters.rasterizer_target == _rasterizer_target_render_primary)
	{
		struct rasterizer_meter_parameters const *meter;
		short vertex_index;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 761, parameters);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 763, parameters->map[0]);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 765, !parameters->map[2] || parameters->map[1]);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 767, !parameters->map[1] || !parameters->meter_parameters);

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_NONE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, 0);

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

		for (map_index = 0; map_index < NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS; map_index++)
		{
			if (!parameters->map[map_index])
			{
				break;
			}

			rasterizer_set_texture_bitmap_data(map_index, parameters->map[map_index]);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, map_index, D3DTSS_ADDRESSU, parameters->map_wrapped[map_index] ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, map_index, D3DTSS_ADDRESSV, parameters->map_wrapped[map_index] ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, map_index, D3DTSS_MAGFILTER, parameters->point_sampled ? D3DTEXF_POINT : D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, map_index, D3DTSS_MINFILTER, parameters->point_sampled ? D3DTEXF_POINT : D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, map_index, D3DTSS_MIPFILTER, parameters->point_sampled ? D3DTEXF_POINT : D3DTEXF_LINEAR);
		}

		rasterizer_set_vertex_shader_permutation(4, _rasterizer_vertex_type_dynamic_screen, 1);
		meter = parameters->meter_parameters;

		if (meter)
		{
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 886, meter->tint_mode_2);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_dynavobgeom.c", 887, meter->gradient==1.0f);

			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, TRUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_SRCBLEND, D3DBLEND_CONSTANTCOLOR);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_BLENDCOLOR, meter->tint_color);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_ENABLE);

			memset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
			pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(4, PS_COMBINERCOUNT_MUX_MSB | PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);

			pixel_shader.PSConstant0[0] = meter->gradient_min_color;
			pixel_shader.PSConstant1[0] = PIXEL32_RGB_BITS(meter->gradient_max_color) | real_alpha_to_pixel32(1.f/MAX(meter->gradient*8.f, 1.f));
			pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_T0, PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_T0);
			pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T0 | PS_INPUTMAPPING_SIGNED_NEGATE, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
			pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTLEFT_2);
			pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTLEFT_2);

			pixel_shader.PSConstant0[1] = meter->gradient_min_color;
			pixel_shader.PSConstant1[1] = meter->gradient_max_color;
			pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_EXPAND_NEGATE, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
			pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C0, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_C1);
			pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);

			pixel_shader.PSConstant0[2] = meter->gradient_min_color;
			pixel_shader.PSConstant1[2] = meter->flash_color;
			pixel_shader.PSAlphaInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_HALFBIAS_NEGATE, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
			pixel_shader.PSAlphaOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
			pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, meter->flash_color_is_negative ? PS_REGISTER_C1 | PS_INPUTMAPPING_SIGNED_NEGATE : PS_REGISTER_C1);
			pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);

			pixel_shader.PSConstant0[3] = meter->background_color;
			pixel_shader.PSConstant1[3] = meter->tint_color;
			pixel_shader.PSAlphaInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
			pixel_shader.PSAlphaOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_AB_CD_MUX);
			pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
			pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_AB_CD_MUX);

			pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
			pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, 0);
		}
		else if (parameters->map[0])
		{
			real_argb_color tint_colors[NUMBER_OF_DYNAMIC_SCREEN_GEOMETRY_MAPS];
			short combiner_count;

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
			pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_T0, PS_REGISTER_T1, PS_REGISTER_DISCARD, 0);
			pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_C1 | PS_CHANNEL_ALPHA);
			pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_T0, PS_REGISTER_T1, PS_REGISTER_DISCARD, 0);
			pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_C0, PS_REGISTER_T0, PS_REGISTER_V0);
			pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_T2, PS_REGISTER_R0, PS_REGISTER_DISCARD, 0);
			pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T2 | PS_CHANNEL_ALPHA, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_V0 | PS_CHANNEL_ALPHA);
			pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_T2, PS_REGISTER_R0, PS_REGISTER_DISCARD, 0);
			combiner_count = 2;

			if (parameters->map[1])
			{
				switch (parameters->map0_to_1_blend_function)
				{
				case _screen_multitexture_blend_func_add:
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T1, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
					break;
				case _screen_multitexture_blend_func_multiply:
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T1, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
					break;
				case _screen_multitexture_blend_func_subtract:
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T1 | PS_INPUTMAPPING_SIGNED_NEGATE, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T1 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_SIGNED_NEGATE, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
					break;
				case _screen_multitexture_blend_func_multiply2X:
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T1, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_1);
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_1);
					break;
				case _screen_multitexture_blend_func_dot:
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T1, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
					break;
				case _screen_multitexture_blend_func_phatty_plasma:
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T1 | PS_INPUTMAPPING_HALFBIAS_NEGATE, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_HALFBIAS_NEGATE, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
					combiner_count++;
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_R0, PS_REGISTER_R0);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTLEFT_2 | PS_COMBINEROUTPUT_AB_CD_MUX);
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBOutputs[combiner_count] = 0;
					combiner_count++;
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_EXPAND_NORMAL);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_COMBINEROUTPUT_AB_CD_MUX);
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBOutputs[combiner_count] = 0;
					combiner_count++;
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_HALFBIAS_NEGATE, PS_REGISTER_R1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_HALFBIAS_NEGATE);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
					break;
				}

				combiner_count++;
			}

			if (parameters->map[2])
			{
				switch (parameters->map1_to_2_blend_function)
				{
				case _screen_multitexture_blend_func_add:
					if (parameters->map0_to_1_blend_function == _screen_multitexture_blend_func_phatty_plasma)
					{
						pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_C0, PS_REGISTER_T2, parameters->doing_plasma_effect ? PS_REGISTER_V0 : PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					}
					else
					{
						pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T2, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					}

					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T2 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
					break;
				case _screen_multitexture_blend_func_multiply:
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T2, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T2 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
					break;
				case _screen_multitexture_blend_func_subtract:
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T2 | PS_INPUTMAPPING_SIGNED_NEGATE, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T2 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_SIGNED_NEGATE, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
					break;
				case _screen_multitexture_blend_func_multiply2X:
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T2, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_1);
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T2 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_1);
					break;
				case _screen_multitexture_blend_func_dot:
					pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T2, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
					pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T2 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
					break;
				}

				combiner_count++;
			}

			pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(combiner_count, PS_COMBINERCOUNT_MUX_MSB | PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
			pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
			pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, 0);
		}

		rasterizer_set_pixel_shader(&pixel_shader);

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_CCW);

		rasterizer_set_vertex_shader_permutation(4, _rasterizer_vertex_type_dynamic_screen, 1);

		IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);

		for (vertex_index = 0; vertex_index < NUMBER_OF_VERTICES_PER_QUADRILATERAL; vertex_index++)
		{
			submit_screen_vertex(&verts[vertex_index]);
		}

		IDirect3DDevice8_End(global_d3d_device);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE);
	}

	return;
}

/* ---------- private code */
