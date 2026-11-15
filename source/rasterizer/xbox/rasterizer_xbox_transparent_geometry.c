/*
RASTERIZER_XBOX_TRANSPARENT_GEOMETRY.C
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
#include "render_debug.h"
#include "main.h"
#include "draw_string.h"
#include "hud.h"

/* ---------- constants */

enum
{
	RASTERIZER_TRANSPARENT_GEOMETRY_TEXCOORD_STREAM_SIZE = 8192,
	NUMBER_OF_SHADER_TRANSPARENT_MAPS = 4 /* fake name */
};

/* ---------- globals */

static long rasterizer_xbox_transparent_geometry_last_source_object_index = 0; /* fake name */
static IDirect3DVertexBuffer8 *rasterizer_xbox_transparent_geometry_texcoord_stream = NULL;
static boolean rasterizer_xbox_transparent_geometry_no_more_active_camouflage = FALSE; /* fake name */
static unsigned long rasterizer_xbox_transparent_geometry_pixel_count = 0; /* fake name */

/* ---------- public code */

void rasterizer_transparent_geometry_groups_begin(
	void)
{
	rasterizer_xbox_transparent_geometry_last_source_object_index = 0;
	rasterizer_xbox_transparent_geometry_no_more_active_camouflage = FALSE;

	if (rasterizer_debug_options.__unknown88 && rasterizer_debug_options.transparent_pixel_counter_enabled && global_window_parameters.window_index!=NONE)
	{
		IDirect3DDevice8_BeginVisibilityTest(global_d3d_device);
	}

	return;
}

void rasterizer_transparent_geometry_group_draw__internal(
	struct transparent_geometry_group const *group,
	boolean has_lightmap)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 109, group);

	if (group->triangle_buffer)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 113, !has_lightmap);

		if (group->vertex_buffers)
		{
			rasterizer_draw_static_triangles_static_vertices(group->triangle_buffer, group->first_triangle_index, group->triangle_count, group->vertex_buffers);
		}
		else
		{
			rasterizer_draw_static_triangles_dynamic_vertices(group->triangle_buffer, group->first_triangle_index, group->triangle_count, group->dynamic_vertex_buffer_index);
		}
	}
	else if (group->vertex_buffers)
	{
		if (has_lightmap)
		{
			rasterizer_draw_dynamic_triangles_static_vertices2(group->dynamic_triangle_buffer_index, group->first_triangle_index, group->triangle_count, group->vertex_buffers, group->vertex_buffers + 1);
		}
		else
		{
			rasterizer_draw_dynamic_triangles_static_vertices(group->dynamic_triangle_buffer_index, group->first_triangle_index, group->triangle_count, group->vertex_buffers);
		}
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 156, !has_lightmap);

		if (group->dynamic_triangle_buffer_index>=0)
		{
			rasterizer_draw_dynamic_triangles_dynamic_vertices(group->dynamic_triangle_buffer_index, group->first_triangle_index, group->triangle_count, group->dynamic_vertex_buffer_index);
		}
		else
		{
			short primitive_count;
			short vertices_per_primitive = (short)-group->dynamic_triangle_buffer_index;

			if (vertices_per_primitive==3 || vertices_per_primitive==4)
			{
				primitive_count = (short)(group->triangle_count / (vertices_per_primitive - 2));
			}
			else
			{
				primitive_count = 1;
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 180, group->triangle_count==vertices_per_primitive-2);
			}

			rasterizer_draw_dynamic_vertices(0, primitive_count, group->dynamic_vertex_buffer_index, vertices_per_primitive);
		}
	}

	return;
}

void rasterizer_transparent_geometry_group_draw(
	struct transparent_geometry_group const *group,
	boolean dirty)
{
	boolean success = TRUE;
	boolean draw_active_camouflage_groups = FALSE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 232, group);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 233, global_d3d_device);

	if ((!group->active_camouflage_transparent_source_object_index || dirty) && rasterizer_transparent_geometry_get_group_pending_status(group))
	{
		rasterizer_transparent_geometry_set_group_pending_status(group, FALSE);

		if (group->prev_group_presorted_index!=NONE)
		{
			rasterizer_transparent_geometry_group_draw(rasterizer_transparent_geometry_get_group_from_presorted_index(group->prev_group_presorted_index), dirty);
		}

		if (rasterizer_debug_options.debug_transparent_geometry_enabled)
		{
			if (!TEST_FLAG(group->geometry_flags, _rasterizer_geometry_no_queue_bit) && group->shader && group->sorted_index!=NONE)
			{
				struct render_skinning skinning;
				unsigned long seed;
				real_argb_color color;
				real minimum;
				real maximum;
				real scale;
				long component_index;
				short vertex_shader_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES] = { _vertex_shader_environment_fog, _vertex_shader_environment_fog, _vertex_shader_environment_fog, _vertex_shader_environment_fog, _vertex_shader_model_zbuffer, _vertex_shader_model_zbuffer, _vertex_shader_effect, _vertex_shader_effect, NONE, NONE, NONE, NONE };
				short vertex_type = rasterizer_transparent_geometry_get_primary_vertex_type(group);
				boolean accumulate = rasterizer_debug_options.pad3>=1000 || rasterizer_debug_options.pad3<0;

				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 280, vertex_type>=0 && vertex_type<NUMBER_OF_RASTERIZER_VERTEX_TYPES);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 281, vertex_shader_table[vertex_type]!=NONE);

				rasterizer_set_vertex_shader_permutation(vertex_shader_table[vertex_type], vertex_type, _vertex_shader_permutation_default);

				SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_NONE);
				SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
				SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, accumulate);
				SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ONE);
				SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
				SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
				SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
				SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
				SetRenderStateSmart(D3DRS_ZBIAS, 0);

				memset(&pixel_shader, 0, sizeof(pixel_shader));

				seed = rasterizer_debug_options.pad3 + group->sorted_index;
				color.alpha = 1.f;
				color.red = real_seed_random(&seed);
				color.green = real_seed_random(&seed);
				color.blue = real_seed_random(&seed);
				minimum = MIN(color.red, MIN(color.green, color.blue));
				maximum = MAX(color.red, MAX(color.green, color.blue));
				scale = (0.18f / (maximum - minimum));

				for (component_index = 0; component_index<NUMBEROF(color.rgb.n); component_index++)
				{
					color.rgb.n[component_index] = (color.rgb.n[component_index] - minimum) * scale + 0.15f;
				}

				if (accumulate)
				{
					real intensity = PIN(rasterizer_debug_options.pad3_scale, 0.f, 1.f);

					if (intensity==0.f)
					{
						intensity = 1.f / 32.f;
					}

					if (rasterizer_debug_options.pad3>=1000)
					{
						color.red *= intensity;
						color.green *= intensity;
						color.blue *= intensity;
					}
					else
					{
						color.blue = intensity;
						color.green = intensity;
						color.red = intensity;
					}
				}

				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 342, color.red >=0.0f && color.red <=1.0f);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 343, color.green>=0.0f && color.green<=1.0f);
				match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 344, color.blue >=0.0f && color.blue <=1.0f);

				pixel_shader.PSFinalCombinerConstant0 = real_rgb_color_to_pixel32(&color.rgb);
				pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
				pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_C0);
				rasterizer_set_pixel_shader(&pixel_shader);

				if (group->node_matrices && group->node_matrix_count)
				{
					skinning.node_matrix_count = group->node_matrix_count;
					skinning.node_matrices = (real_matrix4x3 *)group->node_matrices;
				}
				else
				{
					skinning.node_matrix_count = 1;
					skinning.node_matrices = (real_matrix4x3 *)global_identity4x3;
				}

				rasterizer_set_model_skinning(&skinning);

				{
					real vsh_constants__inverse[VSH_CONSTANTS__INVERSE_COUNT][4] =
					{
						{1.f, 0.f, 0.f, 0.f},
						{0.f, 1.f, 0.f, 0.f},
						{0.f, 0.f, 1.f, 0.f}
					};

					if (TEST_FLAG(group->geometry_flags, _rasterizer_geometry_viewspace_bit))
					{
						vsh_constants__inverse[0][0] = global_window_parameters.frustum.view_to_world.forward.i;
						vsh_constants__inverse[0][1] = global_window_parameters.frustum.view_to_world.left.i;
						vsh_constants__inverse[0][2] = global_window_parameters.frustum.view_to_world.up.i;
						vsh_constants__inverse[0][3] = global_window_parameters.camera.position.x;
						vsh_constants__inverse[1][0] = global_window_parameters.frustum.view_to_world.forward.j;
						vsh_constants__inverse[1][1] = global_window_parameters.frustum.view_to_world.left.j;
						vsh_constants__inverse[1][2] = global_window_parameters.frustum.view_to_world.up.j;
						vsh_constants__inverse[1][3] = global_window_parameters.camera.position.y;
						vsh_constants__inverse[2][0] = global_window_parameters.frustum.view_to_world.forward.k;
						vsh_constants__inverse[2][1] = global_window_parameters.frustum.view_to_world.left.k;
						vsh_constants__inverse[2][2] = global_window_parameters.frustum.view_to_world.up.k;
						vsh_constants__inverse[2][3] = global_window_parameters.camera.position.z;
					}

					D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__INVERSE_OFFSET, vsh_constants__inverse, VSH_CONSTANTS__INVERSE_COUNT));
				}

				rasterizer_transparent_geometry_group_draw__internal(group, FALSE);

				if (!accumulate)
				{
					char string[256];

					sprintf(string, "%.03f", group->z_sort);
					color.red = PIN(color.red * 3.f, 0.f, 1.f);
					color.green = PIN(color.red * 3.f, 0.f, 1.f);
					color.blue = PIN(color.red * 3.f, 0.f, 1.f);
					render_debug_string_at_point(FALSE, &group->centroid, string, &color);
				}
			}
		}
		else
		{
			if (group->effect.type==_render_model_effect_type_transparent_zbuffered)
			{
				long source_object_index = group->source_object_index;

				if (source_object_index!=rasterizer_xbox_transparent_geometry_last_source_object_index && !dirty)
				{
					struct transparent_geometry_group const *source_group = group;

					rasterizer_set_vertex_shader_permutation(_vertex_shader_model_zbuffer, rasterizer_transparent_geometry_get_primary_vertex_type(group), _vertex_shader_permutation_default);

					SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_NONE);
					SetRenderStateSmart(D3DRS_COLORWRITEENABLE, 0);
					SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
					SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
					SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
					SetRenderStateSmart(D3DRS_ZWRITEENABLE, TRUE);
					SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
					SetRenderStateSmart(D3DRS_ZBIAS, 0);

					memset(&pixel_shader, 0, sizeof(pixel_shader));
					pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
					rasterizer_set_pixel_shader(&pixel_shader);

					do
					{
						if (source_group->source_object_index!=source_object_index || source_group->effect.type!=_render_model_effect_type_transparent_zbuffered)
						{
							break;
						}

						if (!shader_ignores_effect(source_group->shader))
						{
							struct render_skinning skinning;

							if (source_group->node_matrices && source_group->node_matrix_count)
							{
								skinning.node_matrix_count = source_group->node_matrix_count;
								skinning.node_matrices = (real_matrix4x3 *)source_group->node_matrices;
							}
							else
							{
								skinning.node_matrix_count = 1;
								skinning.node_matrices = (real_matrix4x3 *)global_identity4x3;
							}

							rasterizer_set_model_skinning(&skinning);

							if (group->lighting)
							{
								rasterizer_set_model_lighting(source_group->lighting);
							}

							rasterizer_transparent_geometry_group_draw__internal(source_group, FALSE);
						}
					}
					while (source_group = rasterizer_transparent_geometry_next_group(source_group));
				}
			}

			if (!TEST_FLAG(group->geometry_flags, _rasterizer_geometry_no_queue_bit) && global_window_parameters.rasterizer_target==_rasterizer_target_render_primary && !dirty)
			{
				if (rasterizer_debug_options.active_camouflage_multipass_enabled ?
					(group->shader && group->shader->base.type==_shader_type_model && group->effect.type==_render_model_effect_type_active_camouflage && group->source_object_index!=rasterizer_xbox_transparent_geometry_last_source_object_index) :
					(!group->shader || (group->shader->base.type!=_shader_type_transparent_water && !shader_is_water_decal(group->shader))))
				{
					rasterizer_active_camouflage_cache_primary_render_target();
				}
			}

			if (!TEST_FLAG(group->geometry_flags, _rasterizer_geometry_no_queue_bit) && global_window_parameters.rasterizer_target==_rasterizer_target_render_primary && group->effect.type==_render_model_effect_type_active_camouflage && group->shader && group->shader->base.type==_shader_type_model && !dirty)
			{
				struct transparent_geometry_group const *next_group = rasterizer_transparent_geometry_next_group(group);

				if (!next_group || next_group->effect.type!=_render_model_effect_type_active_camouflage || next_group->source_object_index!=group->source_object_index || !next_group->shader || next_group->shader->base.type!=_shader_type_model)
				{
					draw_active_camouflage_groups = TRUE;
				}
			}

			if (!group->shader)
			{
				((void (*)(long, long))group->triangle_buffer)(group->first_triangle_index, group->triangle_count);
			}
			else
			{
				short pass;
				short shader_permutation_index = shader_get_vertex_shader_permutation(group->shader);
				short vertex_type = rasterizer_transparent_geometry_get_primary_vertex_type(group);

				if (!TEST_FLAG(group->geometry_flags, _rasterizer_geometry_no_queue_bit))
				{
					struct render_skinning skinning;

					if (group->node_matrices && group->node_matrix_count)
					{
						skinning.node_matrix_count = group->node_matrix_count;
						skinning.node_matrices = (real_matrix4x3 *)group->node_matrices;
					}
					else
					{
						skinning.node_matrix_count = 1;
						skinning.node_matrices = (real_matrix4x3 *)global_identity4x3;
					}

					rasterizer_set_model_skinning(&skinning);

					if (group->lighting)
					{
						rasterizer_set_model_lighting(group->lighting);
					}
				}

				if (TEST_FLAG(group->geometry_flags, _rasterizer_geometry_no_zbuffer_bit))
				{
					if (global_window_parameters.rasterizer_target==_rasterizer_target_render_primary)
					{
						rasterizer_set_frustum_z(1.f / 256.f, 1024.f);
					}

					SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
					SetRenderStateSmart(D3DRS_ZBIAS, 0);
				}
				else
				{
					SetRenderStateSmart(D3DRS_ZENABLE, TRUE);
					SetRenderStateSmart(D3DRS_ZWRITEENABLE, FALSE);
					SetRenderStateSmart(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
					SetRenderStateSmart(D3DRS_ZBIAS, shader_is_decal(group->shader) ? rasterizer_debug_options.zbias : 0);
				}

				for (pass = 0; pass<2; pass++)
				{
					if (TEST_FLAG(group->geometry_flags, _rasterizer_geometry_first_person_bit))
					{
						if (group->effect.type==_render_model_effect_type_active_camouflage)
						{
							if (pass>0)
							{
								break;
							}

							rasterizer_set_frustum_z(rasterizer_globals.z_near_first_person, rasterizer_globals.z_far_first_person);
						}
						else if (!pass)
						{
							if (group->shader && group->shader->base.type==_shader_type_effect && TEST_FLAG(shader_effect_get(group->shader)->effect.flags, _shader_effect_flags_dont_overdraw_first_person_weapon_bit))
							{
								continue;
							}

							rasterizer_set_stencil_mode(_rasterizer_stencil_mode_reject_invert);
						}
						else
						{
							rasterizer_set_stencil_mode(_rasterizer_stencil_mode_reject);
							SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
						}
					}
					else if (pass>0)
					{
						break;
					}

					switch (group->shader->base.type)
					{
					case _shader_type_model:
						switch (group->effect.type)
						{
						case _render_model_effect_type_active_camouflage:
							if (rasterizer_xbox_transparent_geometry_no_more_active_camouflage)
							{
								return;
							}

							rasterizer_active_camouflage_draw(group);
							break;
						default:
							match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 683, "### ERROR unsupported model effect type in transparent group");
						}

						continue;
					case _shader_type_effect:
					{
						short combiner_count;
						struct shader_effect const *shader_effect = shader_effect_get(group->shader);
						boolean secondary_map_active = shader_effect->effect.secondary_map.index!=NONE && shader_effect->effect.secondary_map_anchor!=_shader_effect_secondary_map_zsprite;
						boolean uses_nonlinear_tint = TEST_FLAG(shader_effect->effect.flags, _shader_effect_uses_nonlinear_tint_bit);

						rasterizer_set_texture_bitmap_data(0, group->lightmap);
						SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, TEST_FLAG(shader_effect->effect.primary_map_flags, _shader_effect_map_u_clamp_bit) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
						SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, TEST_FLAG(shader_effect->effect.primary_map_flags, _shader_effect_map_v_clamp_bit) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
						SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, TEST_FLAG(shader_effect->effect.primary_map_flags, _shader_effect_map_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
						SetTextureStageStateSmart(0, D3DTSS_MINFILTER, TEST_FLAG(shader_effect->effect.primary_map_flags, _shader_effect_map_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
						SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, TEST_FLAG(shader_effect->effect.primary_map_flags, _shader_effect_map_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);

						if (shader_effect->effect.secondary_map.index!=NONE)
						{
							rasterizer_set_texture(1, _bitmap_type_2d, _bitmap_usage_multiplicative, shader_effect->effect.secondary_map.index, group->shader_permutation_index);
							SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, TEST_FLAG(shader_effect->effect.secondary_map_flags, _shader_effect_map_u_clamp_bit) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
							SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, TEST_FLAG(shader_effect->effect.secondary_map_flags, _shader_effect_map_v_clamp_bit) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
							SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, TEST_FLAG(shader_effect->effect.secondary_map_flags, _shader_effect_map_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
							SetTextureStageStateSmart(1, D3DTSS_MINFILTER, TEST_FLAG(shader_effect->effect.secondary_map_flags, _shader_effect_map_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
							SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, TEST_FLAG(shader_effect->effect.secondary_map_flags, _shader_effect_map_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
						}

						SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_NONE);
						SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
						SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
						SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
						rasterizer_set_framebuffer_blend_function(shader_effect->effect.framebuffer_blend_function);
						rasterizer_set_vertex_shader_permutation(_vertex_shader_effect, vertex_type, shader_permutation_index);

						{
							real vsh_constants__inverse[VSH_CONSTANTS__INVERSE_COUNT][4] =
							{
								{1.f, 0.f, 0.f, 0.f},
								{0.f, 1.f, 0.f, 0.f},
								{0.f, 0.f, 1.f, 0.f}
							};
							real vsh_constants__texanim[4][4] =
							{
								{1.f, 0.f, 0.f, 0.f},
								{0.f, 1.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f}
							};

							if (TEST_FLAG(group->geometry_flags, _rasterizer_geometry_viewspace_bit))
							{
								vsh_constants__inverse[0][0] = global_window_parameters.frustum.view_to_world.forward.i;
								vsh_constants__inverse[0][1] = global_window_parameters.frustum.view_to_world.left.i;
								vsh_constants__inverse[0][2] = global_window_parameters.frustum.view_to_world.up.i;
								vsh_constants__inverse[0][3] = global_window_parameters.camera.position.x;
								vsh_constants__inverse[1][0] = global_window_parameters.frustum.view_to_world.forward.j;
								vsh_constants__inverse[1][1] = global_window_parameters.frustum.view_to_world.left.j;
								vsh_constants__inverse[1][2] = global_window_parameters.frustum.view_to_world.up.j;
								vsh_constants__inverse[1][3] = global_window_parameters.camera.position.y;
								vsh_constants__inverse[2][0] = global_window_parameters.frustum.view_to_world.forward.k;
								vsh_constants__inverse[2][1] = global_window_parameters.frustum.view_to_world.left.k;
								vsh_constants__inverse[2][2] = global_window_parameters.frustum.view_to_world.up.k;
								vsh_constants__inverse[2][3] = global_window_parameters.camera.position.z;
							}

							if (secondary_map_active)
							{
								shader_texture_animation_evaluate(&shader_effect->effect.secondary_map_animation, group->animation, group->model_base_map_scale.i, group->model_base_map_scale.j, 0.f, 0.f, 0.f, global_frame_parameters.game_time_sec, (real_vector4d *)vsh_constants__texanim[2], (real_vector4d *)vsh_constants__texanim[3]);
							}

							D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__INVERSE_OFFSET, vsh_constants__inverse, VSH_CONSTANTS__INVERSE_COUNT));
							D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__TEXANIM_OFFSET, vsh_constants__texanim, 4));
						}

						if (rasterizer_debug_options.zsprite_enabled && shader_effect->effect.secondary_map_anchor==_shader_effect_secondary_map_zsprite && shader_effect->effect.secondary_map.index!=NONE && !TEST_FLAG(group->geometry_flags, _rasterizer_geometry_first_person_bit))
						{
							real vsh_constants__zsprite[VSH_CONSTANTS__ZSPRITE_COUNT][4];
							struct render_camera const *camera = &global_window_parameters.camera;
							real radius_scale = shader_effect->effect.zsprite_radius_scale!=0.f ? shader_effect->effect.zsprite_radius_scale : 1.f;
							real depth_scale = rasterizer_globals.use_floating_point_zbuffer ? 1.0e30f : 16777215.f;
							real z_near = camera->z_near;
							real z_far = camera->z_far;

							vsh_constants__zsprite[0][0] = z_far * depth_scale / (z_far - z_near);
							vsh_constants__zsprite[0][1] = -(vsh_constants__zsprite[0][0] * z_near);
							vsh_constants__zsprite[0][2] = radius_scale * shader_effect->effect.secondary_map_radius;
							vsh_constants__zsprite[0][3] = camera->z_near + 0.01f;
							vsh_constants__zsprite[1][0] = camera->forward.i;
							vsh_constants__zsprite[1][1] = camera->forward.j;
							vsh_constants__zsprite[1][2] = camera->forward.k;
							vsh_constants__zsprite[1][3] = -dot_product3d(&camera->forward, (real_vector3d const *)&camera->position);

							D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__ZSPRITE_OFFSET, vsh_constants__zsprite, VSH_CONSTANTS__ZSPRITE_COUNT));
						}

						memset(&pixel_shader, 0, sizeof(pixel_shader));
						pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_V1 | PS_CHANNEL_ALPHA);
						pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_V0, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
						pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);

						if (uses_nonlinear_tint)
						{
							pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_T0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_V1 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_V1, PS_REGISTER_T0);
							pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							combiner_count = 3;
						}
						else
						{
							pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_V1, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							combiner_count = 1;
						}

						if (secondary_map_active)
						{
							pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T1, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							combiner_count++;
						}

						if (rasterizer_debug_options.zsprite_enabled && shader_effect->effect.secondary_map_anchor==_shader_effect_secondary_map_zsprite && shader_effect->effect.secondary_map.index!=NONE && !TEST_FLAG(group->geometry_flags, _rasterizer_geometry_first_person_bit))
						{
							pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_DOTPRODUCT, PS_TEXTUREMODES_DOT_ZW);
							pixel_shader.PSInputTexture = PS_INPUTTEXTURE(0, 0, 1, 1);
							pixel_shader.PSDotMapping = PS_DOTMAPPING(PS_DOTMAPPING_ZERO_TO_ONE, PS_DOTMAPPING_ZERO_TO_ONE, PS_DOTMAPPING_ZERO_TO_ONE, PS_DOTMAPPING_ZERO_TO_ONE);
							SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
							SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
							SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
							SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
							SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
							D3DCALL(success, IDirect3DDevice8_SetStreamSource(global_d3d_device, 1, rasterizer_xbox_transparent_geometry_texcoord_stream, 2*sizeof(byte)));
							rasterizer_set_texture(1, _bitmap_type_2d, _bitmap_usage_additive, shader_effect->effect.secondary_map.index, group->shader_permutation_index);
						}
						else
						{
							pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, secondary_map_active ? PS_TEXTUREMODES_PROJECT2D : PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
						}

						if (rasterizer_debug_options.draw_environment_fog && !TEST_FLAG(group->geometry_flags, _rasterizer_geometry_no_fog_bit))
						{
							switch (shader_effect->effect.framebuffer_blend_function)
							{
							case _shader_framebuffer_blend_function_alpha_blend:
								pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
								pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
								break;
							case _shader_framebuffer_blend_function_multiply:
							case _shader_framebuffer_blend_function_min:
								pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT);
								pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
								break;
							case _shader_framebuffer_blend_function_double_multiply:
								pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE_HALF, PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT);
								pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
								break;
							case _shader_framebuffer_blend_function_add:
							case _shader_framebuffer_blend_function_reverse_subtract:
							case _shader_framebuffer_blend_function_max:
								pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
								pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
								break;
							case _shader_framebuffer_blend_function_alpha_multiply_add:
								pixel_shader.PSAlphaInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
								pixel_shader.PSAlphaOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
								pixel_shader.PSRGBInputs[combiner_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
								pixel_shader.PSRGBOutputs[combiner_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
								break;
							default:
								match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 930, "### ERROR unsupported framebuffer blend function");
							}

							combiner_count++;
						}

						pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(combiner_count, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
						break;
					}
					case _shader_type_transparent_generic:
					{
						real vsh_constants__texanim[VSH_CONSTANTS__TEXANIM_COUNT][4];
						short layer_index;
						short map_index;
						short stage_index;
						struct shader_transparent_generic const *shader_transparent_generic = shader_transparent_generic_get(group->shader);
						short bitmap_sequence_index = group->shader_permutation_index;

						for (layer_index = 0; layer_index<shader_transparent_generic->generic.extra_layers.count; layer_index++)
						{
							struct transparent_geometry_group layer_group;

							memcpy(&layer_group, group, sizeof(layer_group));
							layer_group.sorted_index = NONE;
							layer_group.shader = shader_definition_get(TAG_BLOCK_GET_ELEMENT(&shader_transparent_generic->generic.extra_layers, layer_index, struct tag_reference)->index);
							rasterizer_transparent_geometry_group_draw(&layer_group, dirty);
						}

						rasterizer_set_vertex_shader_permutation(_vertex_shader_transparent_generic, vertex_type, shader_permutation_index);

						SetRenderStateSmart(D3DRS_CULLMODE, TEST_FLAG(shader_transparent_generic->generic.flags, _shader_transparent_generic_two_sided_bit) ? D3DCULL_NONE : D3DCULL_CCW);
						SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
						SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
						SetRenderStateSmart(D3DRS_ALPHATESTENABLE, TEST_FLAG(shader_transparent_generic->generic.flags, _shader_transparent_generic_alpha_tested_bit));
						SetRenderStateSmart(D3DRS_ALPHAREF, 127);
						rasterizer_set_framebuffer_blend_function(shader_transparent_generic->generic.framebuffer_blend_function);

						if (TEST_FLAG(shader_transparent_generic->generic.flags, _shader_transparent_generic_numeric_bit) && group->animation && shader_transparent_generic->generic.maps.count>0)
						{
							short digit_index;
							short frame_count = (short)bitmap_group_get(TAG_BLOCK_GET_ELEMENT(&shader_transparent_generic->generic.maps, 0, struct shader_transparent_generic_map)->map.index)->bitmaps.count;
							short function_index = frame_count==8 ? 3 : 0;
							short counter_limit = shader_transparent_generic->generic.numeric_counter_limit;
							short counter_value = (short)PIN(fast_ftol((real)floor(counter_limit * group->animation->values[function_index] + 0.5f)), 0, counter_limit);

							for (digit_index = 0; digit_index<group->shader_permutation_index; digit_index++)
							{
								counter_value /= frame_count;
							}

							bitmap_sequence_index = counter_value % frame_count;
						}

						if (shader_transparent_generic->generic.maps.count>0)
						{
							for (map_index = 0; map_index<NUMBER_OF_SHADER_TRANSPARENT_MAPS; map_index++)
							{
								if (map_index<shader_transparent_generic->generic.maps.count)
								{
									unsigned long address_u;
									unsigned long address_v;
									struct shader_transparent_generic_map const *map = TAG_BLOCK_GET_ELEMENT(&shader_transparent_generic->generic.maps, map_index, struct shader_transparent_generic_map);
									short type = shader_transparent_generic->generic.type;
									short map_type_bitmap_types[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_TYPES] = { _bitmap_type_2d, _bitmap_type_cube_map, _bitmap_type_cube_map, _bitmap_type_cube_map };
									unsigned long map_type_address_modes[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_TYPES] = { D3DTADDRESS_WRAP, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP };
									short bitmap_type = map_index ? _bitmap_type_2d : map_type_bitmap_types[type];

									match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1030, !TEST_FLAG(shader_transparent_generic->shader.radiosity.flags, _shader_radiosity_FILTHY_transparent_lit_bit) || shader_transparent_generic->generic.type==_shader_transparent_generic_type_2d_map);
									match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1031, type>=0 && type<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_TYPES);

									rasterizer_set_texture(map_index, bitmap_type, _bitmap_usage_additive, map->map.index, bitmap_sequence_index);

									if (!bitmap_type && TEST_FLAG(map->flags, _shader_transparent_generic_map_u_clamped_bit))
									{
										address_u = D3DTADDRESS_CLAMP;
									}
									else if (map_index)
									{
										address_u = D3DTADDRESS_WRAP;
									}
									else
									{
										address_u = map_type_address_modes[type];
									}

									if (!bitmap_type && TEST_FLAG(map->flags, _shader_transparent_generic_map_v_clamped_bit))
									{
										address_v = D3DTADDRESS_CLAMP;
									}
									else if (map_index)
									{
										address_v = D3DTADDRESS_WRAP;
									}
									else
									{
										address_v = map_type_address_modes[type];
									}

									SetTextureStageStateSmart(map_index, D3DTSS_ADDRESSU, address_u);
									SetTextureStageStateSmart(map_index, D3DTSS_ADDRESSV, address_v);
									SetTextureStageStateSmart(map_index, D3DTSS_ADDRESSW, map_index ? D3DTADDRESS_WRAP : map_type_address_modes[type]);
									SetTextureStageStateSmart(map_index, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
									SetTextureStageStateSmart(map_index, D3DTSS_MINFILTER, TEST_FLAG(map->flags, _shader_transparent_generic_map_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
									SetTextureStageStateSmart(map_index, D3DTSS_MIPFILTER, TEST_FLAG(map->flags, _shader_transparent_generic_map_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
								}

								if (map_index<shader_transparent_generic->generic.maps.count && (map_index>0 || shader_transparent_generic->generic.type==_shader_transparent_generic_type_2d_map))
								{
									real_vector2d map_scale;
									struct shader_transparent_generic_map const *map = TAG_BLOCK_GET_ELEMENT(&shader_transparent_generic->generic.maps, map_index, struct shader_transparent_generic_map);

									map_scale.i = map->scale.i;
									map_scale.j = map->scale.j;

									if (!map_index && TEST_FLAG(shader_transparent_generic->generic.flags, _shader_transparent_generic_scale_first_map_with_distance_bit))
									{
										map_scale.i = -(map_scale.i * group->z_sort);
										map_scale.j = -(map_scale.j * group->z_sort);
									}

									if (map_index>0 || !TEST_FLAG(shader_transparent_generic->generic.flags, _shader_transparent_generic_first_map_is_in_screenspace_bit))
									{
										map_scale.i *= group->model_base_map_scale.i;
										map_scale.j *= group->model_base_map_scale.j;
									}

									shader_texture_animation_evaluate(&map->animation, group->animation, map_scale.i, map_scale.j, map->offset.i, map->offset.j, map->rotation, global_frame_parameters.game_time_sec, (real_vector4d *)vsh_constants__texanim[map_index * 2], (real_vector4d *)vsh_constants__texanim[map_index * 2 + 1]);
								}
								else if (map_index<shader_transparent_generic->generic.maps.count && TEST_FLAG(shader_transparent_generic->generic.flags, _shader_transparent_generic_first_map_is_in_screenspace_bit))
								{
									vsh_constants__texanim[map_index * 2][0] = global_window_parameters.frustum.view_to_world.forward.i;
									vsh_constants__texanim[map_index * 2][1] = global_window_parameters.frustum.view_to_world.forward.j;
									vsh_constants__texanim[map_index * 2][2] = global_window_parameters.frustum.view_to_world.forward.k;
									vsh_constants__texanim[map_index * 2 + 1][0] = global_window_parameters.frustum.view_to_world.left.i;
									vsh_constants__texanim[map_index * 2 + 1][1] = global_window_parameters.frustum.view_to_world.left.j;
									vsh_constants__texanim[map_index * 2 + 1][2] = global_window_parameters.frustum.view_to_world.left.k;
									vsh_constants__texanim[map_index * 2][3] = 0.f;
									vsh_constants__texanim[map_index * 2 + 1][3] = 0.f;
								}
								else
								{
									vsh_constants__texanim[map_index * 2][0] = 1.f;
									vsh_constants__texanim[map_index * 2][1] = 0.f;
									vsh_constants__texanim[map_index * 2][2] = 0.f;
									vsh_constants__texanim[map_index * 2 + 1][0] = 0.f;
									vsh_constants__texanim[map_index * 2 + 1][1] = 1.f;
									vsh_constants__texanim[map_index * 2 + 1][2] = 0.f;
									vsh_constants__texanim[map_index * 2][3] = 0.f;
									vsh_constants__texanim[map_index * 2 + 1][3] = 0.f;
								}
							}

							D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__TEXANIM_OFFSET, vsh_constants__texanim, VSH_CONSTANTS__TEXANIM_COUNT));
						}

						success = success && shader_transparent_generic_create(group->shader, &pixel_shader);

						if (rasterizer_debug_options.draw_environment_fog)
						{
							short stage_count = (short)FLOOR(shader_transparent_generic->generic.stages.count, 1);

							if (TEST_FLAG(group->geometry_flags, _rasterizer_geometry_sky_bit) && shader_transparent_generic->generic.framebuffer_blend_function==_shader_framebuffer_blend_function_alpha_blend)
							{
								struct render_fog const *fog = &global_window_parameters.fog;
								struct render_camera const *camera = &global_window_parameters.camera;
								real plane_distance = plane3d_distance_to_point(&fog->plane, &camera->position);
								real planar_fog_density = PIN(-(plane_distance / fog->planar_maximum_depth), 0.f, 1.f) * fog->planar_maximum_density;

								pixel_shader.PSConstant0[stage_count] = real_a_rgb_color_to_pixel32(planar_fog_density, &fog->planar_color);
								pixel_shader.PSRGBInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_C0);
								pixel_shader.PSRGBOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							}
							else
							{
								unsigned long fade_input;
								real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
								{
									{0.f, 0.f, 0.f, 0.f},
									{0.f, 0.f, 0.f, 0.f},
									{0.f, 0.f, 1.f, 0.f}
								};
								short fade_source = shader_transparent_generic->generic.framebuffer_fade_source;

								if (group->effect.type==_render_model_effect_type_active_camouflage)
								{
									vsh_constants__texscale[2][2] *= PIN(1.f - group->effect.intensity, 0.f, 1.f);
								}

								if (fade_source>0 && group->animation && group->animation->values)
								{
									vsh_constants__texscale[2][2] *= group->animation->values[fade_source - 1];
								}

								D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__TEXSCALE_OFFSET, vsh_constants__texscale, VSH_CONSTANTS__TEXSCALE_COUNT));

								switch (shader_transparent_generic->generic.framebuffer_fade_mode)
								{
								case _shader_framebuffer_fade_mode_none:
									fade_input = PS_REGISTER_V0 | PS_CHANNEL_ALPHA;
									break;
								case _shader_framebuffer_fade_mode_fade_when_perpendicular:
									fade_input = PS_REGISTER_V1 | PS_CHANNEL_ALPHA;
									break;
								case _shader_framebuffer_fade_mode_fade_when_parallel:
									fade_input = PS_REGISTER_V1;
									break;
								default:
									match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1185, "### ERROR unsupported framebuffer fade mode");
								}

								switch (shader_transparent_generic->generic.framebuffer_blend_function)
								{
								case _shader_framebuffer_blend_function_alpha_blend:
									pixel_shader.PSAlphaInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, fade_input, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
									pixel_shader.PSAlphaOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
									break;
								case _shader_framebuffer_blend_function_multiply:
								case _shader_framebuffer_blend_function_min:
									pixel_shader.PSRGBInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, fade_input, PS_REGISTER_ONE, fade_input ^ PS_INPUTMAPPING_UNSIGNED_INVERT);
									pixel_shader.PSRGBOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
									break;
								case _shader_framebuffer_blend_function_double_multiply:
									pixel_shader.PSRGBInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, fade_input, PS_REGISTER_ONE_HALF, fade_input ^ PS_INPUTMAPPING_UNSIGNED_INVERT);
									pixel_shader.PSRGBOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
									break;
								case _shader_framebuffer_blend_function_add:
								case _shader_framebuffer_blend_function_reverse_subtract:
								case _shader_framebuffer_blend_function_max:
									pixel_shader.PSRGBInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, fade_input, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
									pixel_shader.PSRGBOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
									break;
								case _shader_framebuffer_blend_function_alpha_multiply_add:
									pixel_shader.PSAlphaInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, fade_input, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
									pixel_shader.PSAlphaOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
									pixel_shader.PSRGBInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, fade_input, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
									pixel_shader.PSRGBOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
									break;
								default:
									match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1216, "### ERROR unsupported framebuffer blend function");
								}
							}
						}

						for (stage_index = 0; stage_index<shader_transparent_generic->generic.stages.count; stage_index++)
						{
							real_argb_color constant_color0;
							real_argb_color color_delta;
							real function_value;
							long component_index;
							struct shader_transparent_generic_stage const *stage = TAG_BLOCK_GET_ELEMENT(&shader_transparent_generic->generic.stages, stage_index, struct shader_transparent_generic_stage);

							match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1235, stage->constant_color0_animation_period!=0.0f);

							if (group->animation && TEST_FLAG(stage->flags, _shader_transparent_generic_stage_a_out_controls_color0_animation_bit))
							{
								function_value = group->animation->values[0];
							}
							else
							{
								function_value = periodic_function_evaluate(stage->constant_color0_animation_function, global_frame_parameters.game_time_sec / stage->constant_color0_animation_period);
							}

							for (component_index = 0; component_index<NUMBEROF(color_delta.n); component_index++)
							{
								color_delta.n[component_index] = stage->constant_color0_animation_upper_bound.n[component_index] - stage->constant_color0_animation_lower_bound.n[component_index];
							}

							for (component_index = 0; component_index<NUMBEROF(constant_color0.n); component_index++)
							{
								constant_color0.n[component_index] = function_value * color_delta.n[component_index] + stage->constant_color0_animation_lower_bound.n[component_index];
							}

							match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1256, constant_color0.red >=0.0f && constant_color0.red <=1.0f);
							match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1257, constant_color0.green>=0.0f && constant_color0.green<=1.0f);
							match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1258, constant_color0.blue >=0.0f && constant_color0.blue <=1.0f);

							if (stage->constant_color0_source>0 && stage->constant_color0_source<5 && group->animation && group->animation->colors)
							{
								real_rgb_color const *external_color = &group->animation->colors[stage->constant_color0_source - 1];

								match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1269, external_color->red >=0.0f && external_color->red <=1.0f);
								match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1270, external_color->green>=0.0f && external_color->green<=1.0f);
								match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1271, external_color->blue >=0.0f && external_color->blue <=1.0f);

								constant_color0.red *= external_color->red;
								constant_color0.green *= external_color->green;
								constant_color0.blue *= external_color->blue;
							}

							pixel_shader.PSConstant0[stage_index] = real_argb_color_to_pixel32(&constant_color0);
						}
						break;
					}
					case _shader_type_transparent_chicago:
					{
						real vsh_constants__texanim[VSH_CONSTANTS__TEXANIM_COUNT][4];
						short layer_index;
						short map_index;
						struct shader_transparent_chicago const *shader_transparent_chicago = shader_transparent_chicago_get(group->shader);
						short bitmap_sequence_index = group->shader_permutation_index;

						for (layer_index = 0; layer_index<shader_transparent_chicago->chicago.extra_layers.count;)
						{
							struct transparent_geometry_group layer_group;

							memcpy(&layer_group, group, sizeof(layer_group));
							layer_group.sorted_index = NONE;
							layer_group.shader = shader_definition_get(TAG_BLOCK_GET_ELEMENT(&shader_transparent_chicago->chicago.extra_layers, layer_index, struct tag_reference)->index);
							rasterizer_transparent_geometry_group_draw(&layer_group, dirty);
						}

						rasterizer_set_vertex_shader_permutation(_vertex_shader_transparent_generic, vertex_type, shader_permutation_index);

						SetRenderStateSmart(D3DRS_CULLMODE, TEST_FLAG(shader_transparent_chicago->chicago.flags, _shader_transparent_chicago_two_sided_bit) ? D3DCULL_NONE : D3DCULL_CCW);
						SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
						SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
						SetRenderStateSmart(D3DRS_ALPHATESTENABLE, TEST_FLAG(shader_transparent_chicago->chicago.flags, _shader_transparent_chicago_alpha_tested_bit));
						SetRenderStateSmart(D3DRS_ALPHAREF, 127);
						rasterizer_set_framebuffer_blend_function(shader_transparent_chicago->chicago.framebuffer_blend_function);

						if (TEST_FLAG(shader_transparent_chicago->chicago.flags, _shader_transparent_chicago_numeric_bit) && group->animation && shader_transparent_chicago->chicago.maps.count>0)
						{
							short frame_count = (short)bitmap_group_get(TAG_BLOCK_GET_ELEMENT(&shader_transparent_chicago->chicago.maps, 0, struct shader_transparent_chicago_map)->map.index)->bitmaps.count;

							if (TEST_FLAG(shader_transparent_chicago->chicago.extra_flags, _shader_transparent_chicago_numeric_countdown_timer_bit))
							{
								bitmap_sequence_index = numeric_countdown_timer_get(group->shader_permutation_index);
							}
							else
							{
								short digit_index;
								short function_index = frame_count==8 ? 3 : 0;
								short counter_limit = shader_transparent_chicago->chicago.numeric_counter_limit;
								short counter_value = (short)PIN(fast_ftol((real)floor(counter_limit * group->animation->values[function_index] + 0.5f)), 0, counter_limit);

								for (digit_index = 0; digit_index<group->shader_permutation_index; digit_index++)
								{
									counter_value /= frame_count;
								}

								bitmap_sequence_index = counter_value % frame_count;
							}
						}

						for (map_index = 0; map_index<NUMBER_OF_SHADER_TRANSPARENT_MAPS; map_index++)
						{
							if (map_index<shader_transparent_chicago->chicago.maps.count)
							{
								unsigned long address_u;
								unsigned long address_v;
								struct shader_transparent_chicago_map const *map = TAG_BLOCK_GET_ELEMENT(&shader_transparent_chicago->chicago.maps, map_index, struct shader_transparent_chicago_map);
								short type = shader_transparent_chicago->chicago.type;
								short map_type_bitmap_types[NUMBER_OF_SHADER_TRANSPARENT_CHICAGO_TYPES] = { _bitmap_type_2d, _bitmap_type_cube_map, _bitmap_type_cube_map, _bitmap_type_cube_map };
								unsigned long map_type_address_modes[NUMBER_OF_SHADER_TRANSPARENT_CHICAGO_TYPES] = { D3DTADDRESS_WRAP, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP };
								short bitmap_type = map_index ? _bitmap_type_2d : map_type_bitmap_types[type];

								match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1383, !TEST_FLAG(shader_transparent_chicago->shader.radiosity.flags, _shader_radiosity_FILTHY_transparent_lit_bit) || shader_transparent_chicago->chicago.type==_shader_transparent_chicago_type_2d_map);
								match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1384, type>=0 && type<NUMBER_OF_SHADER_TRANSPARENT_CHICAGO_TYPES);

								rasterizer_set_texture(map_index, bitmap_type, _bitmap_usage_additive, map->map.index, bitmap_sequence_index);

								if (!bitmap_type && TEST_FLAG(map->flags, _shader_transparent_chicago_map_u_clamped_bit))
								{
									address_u = D3DTADDRESS_CLAMP;
								}
								else if (map_index)
								{
									address_u = D3DTADDRESS_WRAP;
								}
								else
								{
									address_u = map_type_address_modes[type];
								}

								if (!bitmap_type && TEST_FLAG(map->flags, _shader_transparent_chicago_map_v_clamped_bit))
								{
									address_v = D3DTADDRESS_CLAMP;
								}
								else if (map_index)
								{
									address_v = D3DTADDRESS_WRAP;
								}
								else
								{
									address_v = map_type_address_modes[type];
								}

								IDirect3DDevice8_SetTextureStageState(global_d3d_device, map_index, D3DTSS_ADDRESSU, address_u);
								IDirect3DDevice8_SetTextureStageState(global_d3d_device, map_index, D3DTSS_ADDRESSV, address_v);
								IDirect3DDevice8_SetTextureStageState(global_d3d_device, map_index, D3DTSS_ADDRESSW, map_index ? D3DTADDRESS_WRAP : map_type_address_modes[type]);
								IDirect3DDevice8_SetTextureStageState(global_d3d_device, map_index, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
								IDirect3DDevice8_SetTextureStageState(global_d3d_device, map_index, D3DTSS_MINFILTER, TEST_FLAG(map->flags, _shader_transparent_chicago_map_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
								IDirect3DDevice8_SetTextureStageState(global_d3d_device, map_index, D3DTSS_MIPFILTER, TEST_FLAG(map->flags, _shader_transparent_chicago_map_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
							}

							if (map_index<shader_transparent_chicago->chicago.maps.count && (map_index>0 || shader_transparent_chicago->chicago.type==_shader_transparent_chicago_type_2d_map))
							{
								real_vector2d map_scale;
								struct shader_transparent_chicago_map const *map = TAG_BLOCK_GET_ELEMENT(&shader_transparent_chicago->chicago.maps, map_index, struct shader_transparent_chicago_map);

								map_scale.i = map->scale.i;
								map_scale.j = map->scale.j;

								if (!map_index && TEST_FLAG(shader_transparent_chicago->chicago.flags, _shader_transparent_chicago_scale_first_map_with_distance_bit))
								{
									map_scale.i = -(map_scale.i * group->z_sort);
									map_scale.j = -(map_scale.j * group->z_sort);
								}

								if (map_index>0 || !TEST_FLAG(shader_transparent_chicago->chicago.flags, _shader_transparent_chicago_first_map_is_in_screenspace_bit))
								{
									map_scale.i *= group->model_base_map_scale.i;
									map_scale.j *= group->model_base_map_scale.j;
								}

								shader_texture_animation_evaluate(&map->animation, group->animation, map_scale.i, map_scale.j, map->offset.i, map->offset.j, map->rotation, global_frame_parameters.game_time_sec, (real_vector4d *)vsh_constants__texanim[map_index * 2], (real_vector4d *)vsh_constants__texanim[map_index * 2 + 1]);
							}
							else if (map_index<shader_transparent_chicago->chicago.maps.count && TEST_FLAG(shader_transparent_chicago->chicago.flags, _shader_transparent_chicago_first_map_is_in_screenspace_bit))
							{
								vsh_constants__texanim[map_index * 2][0] = global_window_parameters.frustum.view_to_world.forward.i;
								vsh_constants__texanim[map_index * 2][1] = global_window_parameters.frustum.view_to_world.forward.j;
								vsh_constants__texanim[map_index * 2][2] = global_window_parameters.frustum.view_to_world.forward.k;
								vsh_constants__texanim[map_index * 2 + 1][0] = global_window_parameters.frustum.view_to_world.left.i;
								vsh_constants__texanim[map_index * 2 + 1][1] = global_window_parameters.frustum.view_to_world.left.j;
								vsh_constants__texanim[map_index * 2 + 1][2] = global_window_parameters.frustum.view_to_world.left.k;
								vsh_constants__texanim[map_index * 2][3] = 0.f;
								vsh_constants__texanim[map_index * 2 + 1][3] = 0.f;
							}
							else
							{
								vsh_constants__texanim[map_index * 2][0] = 1.f;
								vsh_constants__texanim[map_index * 2][1] = 0.f;
								vsh_constants__texanim[map_index * 2][2] = 0.f;
								vsh_constants__texanim[map_index * 2 + 1][0] = 0.f;
								vsh_constants__texanim[map_index * 2 + 1][1] = 1.f;
								vsh_constants__texanim[map_index * 2 + 1][2] = 0.f;
								vsh_constants__texanim[map_index * 2][3] = 0.f;
								vsh_constants__texanim[map_index * 2 + 1][3] = 0.f;
							}
						}

						D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__TEXANIM_OFFSET, vsh_constants__texanim, VSH_CONSTANTS__TEXANIM_COUNT));

						success = success && shader_transparent_chicago_create(group->shader, &pixel_shader);

						if (rasterizer_debug_options.draw_environment_fog)
						{
							short stage_count = (short)shader_transparent_chicago->chicago.maps.count;

							if (TEST_FLAG(group->geometry_flags, _rasterizer_geometry_sky_bit) && shader_transparent_chicago->chicago.framebuffer_blend_function==_shader_framebuffer_blend_function_alpha_blend)
							{
								struct render_fog const *fog = &global_window_parameters.fog;
								struct render_camera const *camera = &global_window_parameters.camera;
								real plane_distance = plane3d_distance_to_point(&fog->plane, &camera->position);
								real planar_fog_density = PIN(-(plane_distance / fog->planar_maximum_depth), 0.f, 1.f) * fog->planar_maximum_density;

								pixel_shader.PSConstant0[stage_count] = real_a_rgb_color_to_pixel32(planar_fog_density, &fog->planar_color);
								pixel_shader.PSRGBInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_C0);
								pixel_shader.PSRGBOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							}
							else
							{
								unsigned long fade_input;
								real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4] =
								{
									{0.f, 0.f, 0.f, 0.f},
									{0.f, 0.f, 0.f, 0.f},
									{0.f, 0.f, 1.f, 0.f}
								};
								short fade_source = shader_transparent_chicago->chicago.framebuffer_fade_source;

								if (group->effect.type==_render_model_effect_type_active_camouflage && !TEST_FLAG(shader_transparent_chicago->chicago.extra_flags, _shader_transparent_chicago_dont_fade_active_camouflage_bit))
								{
									vsh_constants__texscale[2][2] *= PIN(1.f - group->effect.intensity, 0.f, 1.f);
								}

								if (fade_source>0 && group->animation && group->animation->values)
								{
									vsh_constants__texscale[2][2] *= group->animation->values[fade_source - 1];
								}

								D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__TEXSCALE_OFFSET, vsh_constants__texscale, VSH_CONSTANTS__TEXSCALE_COUNT));

								switch (shader_transparent_chicago->chicago.framebuffer_fade_mode)
								{
								case _shader_framebuffer_fade_mode_none:
									fade_input = PS_REGISTER_V0 | PS_CHANNEL_ALPHA;
									break;
								case _shader_framebuffer_fade_mode_fade_when_perpendicular:
									fade_input = PS_REGISTER_V1 | PS_CHANNEL_ALPHA;
									break;
								case _shader_framebuffer_fade_mode_fade_when_parallel:
									fade_input = PS_REGISTER_V1;
									break;
								default:
									match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1539, "### ERROR unsupported framebuffer fade mode");
								}

								switch (shader_transparent_chicago->chicago.framebuffer_blend_function)
								{
								case _shader_framebuffer_blend_function_alpha_blend:
									pixel_shader.PSAlphaInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, fade_input, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
									pixel_shader.PSAlphaOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
									break;
								case _shader_framebuffer_blend_function_multiply:
								case _shader_framebuffer_blend_function_min:
									pixel_shader.PSRGBInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, fade_input, PS_REGISTER_ONE, fade_input ^ PS_INPUTMAPPING_UNSIGNED_INVERT);
									pixel_shader.PSRGBOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
									break;
								case _shader_framebuffer_blend_function_double_multiply:
									pixel_shader.PSRGBInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, fade_input, PS_REGISTER_ONE_HALF, fade_input ^ PS_INPUTMAPPING_UNSIGNED_INVERT);
									pixel_shader.PSRGBOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
									break;
								case _shader_framebuffer_blend_function_add:
								case _shader_framebuffer_blend_function_reverse_subtract:
								case _shader_framebuffer_blend_function_max:
									pixel_shader.PSRGBInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, fade_input, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
									pixel_shader.PSRGBOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
									break;
								case _shader_framebuffer_blend_function_alpha_multiply_add:
									pixel_shader.PSAlphaInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, fade_input, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
									pixel_shader.PSAlphaOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
									pixel_shader.PSRGBInputs[stage_count] = PS_COMBINERINPUTS(PS_REGISTER_R0, fade_input, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
									pixel_shader.PSRGBOutputs[stage_count] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
									break;
								default:
									match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1570, "### ERROR unsupported framebuffer blend function");
								}
							}
						}
						break;
					}
					case _shader_type_transparent_water:
						rasterizer_water_draw(group);
						continue;
					case _shader_type_transparent_glass:
					{
						struct shader_transparent_glass const *shader_transparent_glass = shader_transparent_glass_get(group->shader);
						short reflection_type = shader_transparent_glass->glass.reflection_type;

						if (reflection_type==_shader_transparent_glass_reflection_type_mirror)
						{
							if (!global_window_parameters.has_mirror || global_window_parameters.rasterizer_target!=_rasterizer_target_render_primary)
							{
								continue;
							}
						}
						else if (reflection_type==_shader_transparent_glass_reflection_type_bumped && (TEST_FLAG(shader_transparent_glass->glass.flags, _shader_transparent_glass_bump_map_is_specular_mask_bit) || shader_transparent_glass->glass.reflection_bump_map.index==NONE))
						{
							reflection_type = _shader_transparent_glass_reflection_type_flat;
						}

						if (shader_transparent_glass->glass.tint_map.index!=NONE || shader_transparent_glass->glass.tint_color.red!=0.f || shader_transparent_glass->glass.tint_color.green!=0.f || shader_transparent_glass->glass.tint_color.blue!=0.f)
						{
							real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4];

							rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, shader_transparent_glass->glass.tint_map.index, group->shader_permutation_index);
							SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
							SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
							SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
							SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
							SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
							SetRenderStateSmart(D3DRS_CULLMODE, TEST_FLAG(shader_transparent_glass->glass.flags, _shader_transparent_glass_two_sided_bit) ? D3DCULL_NONE : D3DCULL_CCW);
							SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
							SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
							SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ZERO);
							SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
							SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
							SetRenderStateSmart(D3DRS_ALPHATESTENABLE, TRUE);
							SetRenderStateSmart(D3DRS_ALPHAREF, 0);
							rasterizer_set_vertex_shader_permutation(_vertex_shader_transparent_glass_tint, vertex_type, shader_permutation_index);

							vsh_constants__texscale[0][0] = group->model_base_map_scale.i * shader_transparent_glass->glass.tint_map_scale;
							vsh_constants__texscale[0][1] = group->model_base_map_scale.j * shader_transparent_glass->glass.tint_map_scale;
							vsh_constants__texscale[0][2] = 1.f;
							vsh_constants__texscale[0][3] = 1.f;
							vsh_constants__texscale[1][0] = 0.f;
							vsh_constants__texscale[1][1] = 0.f;
							vsh_constants__texscale[1][2] = 0.f;
							vsh_constants__texscale[1][3] = 0.f;
							vsh_constants__texscale[2][0] = 0.f;
							vsh_constants__texscale[2][1] = 0.f;
							vsh_constants__texscale[2][2] = 0.f;
							vsh_constants__texscale[2][3] = 0.f;

							D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__TEXSCALE_OFFSET, vsh_constants__texscale, VSH_CONSTANTS__TEXSCALE_COUNT));

							memset(&pixel_shader, 0, sizeof(pixel_shader));
							pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
							pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
							pixel_shader.PSConstant0[0] = real_rgb_color_to_pixel32(&shader_transparent_glass->glass.tint_color);
							pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_C0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);

							if (group->effect.type==_render_model_effect_type_active_camouflage)
							{
								pixel_shader.PSConstant1[0] = real_alpha_to_pixel32(group->effect.intensity);
								pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_C1 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
								pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_V0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							}

							pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_R0, PS_REGISTER_ONE, PS_REGISTER_ZERO);
							pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);
							rasterizer_set_pixel_shader(&pixel_shader);
							rasterizer_transparent_geometry_group_draw__internal(group, FALSE);
						}

						if ((shader_transparent_glass->glass.reflection_view_perpendicular_color.alpha>0.f || shader_transparent_glass->glass.reflection_view_parallel_color.alpha>0.f) && (shader_transparent_glass->glass.reflection_map.index!=NONE || reflection_type==_shader_transparent_glass_reflection_type_mirror))
						{
							real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4];

							match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1694, reflection_type>=0 && reflection_type<NUMBER_OF_SHADER_TRANSPARENT_GLASS_REFLECTION_TYPES);

							rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_vector, shader_transparent_glass->glass.reflection_bump_map.index, group->shader_permutation_index);
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

							if (reflection_type==_shader_transparent_glass_reflection_type_mirror)
							{
								rasterizer_set_target_as_texture(3, _rasterizer_target_render_secondary, 0);
								SetTextureStageStateSmart(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
								SetTextureStageStateSmart(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
								SetTextureStageStateSmart(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
								SetTextureStageStateSmart(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
								SetTextureStageStateSmart(3, D3DTSS_MIPFILTER, D3DTEXF_POINT);
							}
							else
							{
								rasterizer_set_texture(3, _bitmap_type_cube_map, _bitmap_usage_additive, shader_transparent_glass->glass.reflection_map.index, group->shader_permutation_index);
								SetTextureStageStateSmart(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
								SetTextureStageStateSmart(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
								SetTextureStageStateSmart(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
								SetTextureStageStateSmart(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
								SetTextureStageStateSmart(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
								SetTextureStageStateSmart(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
							}

							SetRenderStateSmart(D3DRS_CULLMODE, TEST_FLAG(shader_transparent_glass->glass.flags, _shader_transparent_glass_two_sided_bit) ? D3DCULL_NONE : D3DCULL_CCW);
							SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
							SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
							SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
							SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
							SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
							SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
							rasterizer_set_vertex_shader_permutation(_vertex_shader_transparent_glass_reflection_flat, vertex_type, reflection_type);

							vsh_constants__texscale[0][0] = group->model_base_map_scale.i * shader_transparent_glass->glass.reflection_bump_map_scale;
							vsh_constants__texscale[0][1] = group->model_base_map_scale.j * shader_transparent_glass->glass.reflection_bump_map_scale;
							vsh_constants__texscale[0][2] = 320.f;
							vsh_constants__texscale[0][3] = 240.f;
							vsh_constants__texscale[1][0] = 0.f;
							vsh_constants__texscale[1][1] = 0.f;
							vsh_constants__texscale[1][2] = 0.f;
							vsh_constants__texscale[1][3] = 0.f;
							vsh_constants__texscale[2][0] = 0.f;
							vsh_constants__texscale[2][1] = 0.f;
							vsh_constants__texscale[2][2] = 0.f;
							vsh_constants__texscale[2][3] = 0.f;

							D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__TEXSCALE_OFFSET, vsh_constants__texscale, VSH_CONSTANTS__TEXSCALE_COUNT));

							memset(&pixel_shader, 0, sizeof(pixel_shader));

							switch (reflection_type)
							{
							case _shader_transparent_glass_reflection_type_bumped:
								pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_DOTPRODUCT, PS_TEXTUREMODES_DOT_RFLCT_DIFF, PS_TEXTUREMODES_DOT_RFLCT_SPEC);
								pixel_shader.PSInputTexture = PS_INPUTTEXTURE(0, 0, 0, 0);
								pixel_shader.PSDotMapping = PS_DOTMAPPING(PS_DOTMAPPING_ZERO_TO_ONE, PS_DOTMAPPING_MINUS1_TO_1_D3D, PS_DOTMAPPING_MINUS1_TO_1_D3D, PS_DOTMAPPING_MINUS1_TO_1_D3D);
								break;
							case _shader_transparent_glass_reflection_type_flat:
								pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP);
								break;
							case _shader_transparent_glass_reflection_type_mirror:
								pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_CUBEMAP, PS_TEXTUREMODES_PROJECT2D);
								break;
							default:
								match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 1799, "### ERROR unsupported reflection type");
							}

							pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(5, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);

							if (reflection_type!=_shader_transparent_glass_reflection_type_bumped && shader_transparent_glass->glass.reflection_bump_map.index!=NONE)
							{
								pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T1 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T3, PS_REGISTER_T3);
							}
							else
							{
								real_rgb_color view_color;

								view_color.red = PIN(0.5f - global_window_parameters.camera.forward.i * 0.5f, 0.f, 1.f);
								view_color.green = PIN(0.5f - global_window_parameters.camera.forward.j * 0.5f, 0.f, 1.f);
								view_color.blue = PIN(0.5f - global_window_parameters.camera.forward.k * 0.5f, 0.f, 1.f);
								pixel_shader.PSConstant0[0] = real_rgb_color_to_pixel32(&view_color);
								pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_C0 | PS_INPUTMAPPING_EXPAND_NORMAL, PS_REGISTER_T3, PS_REGISTER_T3);
							}

							pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
							pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_R1);
							pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);

							if (group->effect.type==_render_model_effect_type_active_camouflage)
							{
								pixel_shader.PSConstant1[1] = real_alpha_to_pixel32(group->effect.intensity);
								pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_C1 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
								pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_V0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							}

							pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_R1);
							pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							pixel_shader.PSConstant0[3] = real_argb_color_to_pixel32(&shader_transparent_glass->glass.reflection_view_perpendicular_color);
							pixel_shader.PSConstant1[3] = real_argb_color_to_pixel32(&shader_transparent_glass->glass.reflection_view_parallel_color);
							pixel_shader.PSAlphaOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							pixel_shader.PSRGBOutputs[4] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							pixel_shader.PSAlphaInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_R0, PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
							pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C1, PS_REGISTER_R0, PS_REGISTER_C0);
							pixel_shader.PSRGBInputs[4] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R1, PS_REGISTER_R0, PS_REGISTER_T3);
							pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_EF_PROD, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, TEST_FLAG(shader_transparent_glass->glass.flags, _shader_transparent_glass_bump_map_is_specular_mask_bit) ? PS_REGISTER_T0 : PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_ZERO);
							rasterizer_set_pixel_shader(&pixel_shader);
							rasterizer_transparent_geometry_group_draw__internal(group, FALSE);
						}

						if (shader_transparent_glass->glass.diffuse_map.index!=NONE || shader_transparent_glass->glass.diffuse_detail_map.index!=NONE)
						{
							real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4];

							rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, shader_transparent_glass->glass.diffuse_map.index, group->shader_permutation_index);
							SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
							SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
							SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
							SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
							SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
							rasterizer_set_texture(1, _bitmap_type_2d, _bitmap_usage_detail, shader_transparent_glass->glass.diffuse_detail_map.index, group->shader_permutation_index);
							SetTextureStageStateSmart(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
							SetTextureStageStateSmart(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
							SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
							SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
							SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
							SetRenderStateSmart(D3DRS_CULLMODE, TEST_FLAG(shader_transparent_glass->glass.flags, _shader_transparent_glass_two_sided_bit) ? D3DCULL_NONE : D3DCULL_CCW);
							SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
							SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
							SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
							SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
							SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
							SetRenderStateSmart(D3DRS_ALPHATESTENABLE, TRUE);
							SetRenderStateSmart(D3DRS_ALPHAREF, 0);

							if (group->lightmap)
							{
								rasterizer_set_texture_bitmap_data(2, group->lightmap);
								SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
								SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
								SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
								SetTextureStageStateSmart(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
								SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
							}
							else
							{
								rasterizer_set_texture(2, _bitmap_type_2d, _bitmap_usage_additive, NONE, 0);
								SetTextureStageStateSmart(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
								SetTextureStageStateSmart(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
								SetTextureStageStateSmart(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
								SetTextureStageStateSmart(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
								SetTextureStageStateSmart(2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
							}

							rasterizer_set_vertex_shader_permutation(_vertex_shader_transparent_glass_diffuse_light, vertex_type, shader_permutation_index);

							vsh_constants__texscale[0][0] = shader_transparent_glass->glass.diffuse_map_scale * group->model_base_map_scale.i;
							vsh_constants__texscale[0][1] = shader_transparent_glass->glass.diffuse_map_scale * group->model_base_map_scale.j;
							vsh_constants__texscale[0][2] = shader_transparent_glass->glass.diffuse_detail_map_scale * group->model_base_map_scale.i;
							vsh_constants__texscale[0][3] = shader_transparent_glass->glass.diffuse_detail_map_scale * group->model_base_map_scale.j;
							vsh_constants__texscale[1][0] = 0.f;
							vsh_constants__texscale[1][1] = 0.f;
							vsh_constants__texscale[1][2] = 0.f;
							vsh_constants__texscale[1][3] = 0.f;
							vsh_constants__texscale[2][0] = 0.f;
							vsh_constants__texscale[2][1] = 0.f;
							vsh_constants__texscale[2][2] = 0.f;
							vsh_constants__texscale[2][3] = 0.f;

							D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__TEXSCALE_OFFSET, vsh_constants__texscale, VSH_CONSTANTS__TEXSCALE_COUNT));

							memset(&pixel_shader, 0, sizeof(pixel_shader));
							pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE);
							pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(3, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
							pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_T1 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_T1, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_SHIFTLEFT_1);
							pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_T2, PS_REGISTER_ONE, PS_REGISTER_V0, PS_REGISTER_ONE);
							pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R1, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_R1, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
							pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
							pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
							pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);
							rasterizer_set_pixel_shader(&pixel_shader);
							rasterizer_transparent_geometry_group_draw__internal(group, group->lightmap!=NULL);
						}

						continue;
					}
					case _shader_type_transparent_meter:
					{
						real_rgb_color flash_color;
						real flash_alpha;
						pixel32 gradient_min_pixel;
						pixel32 gradient_max_pixel;
						pixel32 background_pixel;
						pixel32 flash_pixel;
						pixel32 tint_pixel;
						real vsh_constants__texscale[VSH_CONSTANTS__TEXSCALE_COUNT][4];
						struct shader_transparent_meter const *shader_transparent_meter = shader_transparent_meter_get(group->shader);
						real meter_brightness = 1.f;
						real flash_brightness = 1.f;
						real meter_value = 1.f;
						real gradient_value = 1.f;
						real flash_extension = 1.f;

						if (group->animation && group->animation->values)
						{
							if (shader_transparent_meter->meter.meter_brightness_source>=1 &&shader_transparent_meter->meter.meter_brightness_source<=4)
							{
								meter_brightness = group->animation->values[shader_transparent_meter->meter.meter_brightness_source - 1];
							}

							if (shader_transparent_meter->meter.flash_brightness_source>=1 &&shader_transparent_meter->meter.flash_brightness_source<=4)
							{
								flash_brightness = group->animation->values[shader_transparent_meter->meter.flash_brightness_source - 1];
							}

							if (shader_transparent_meter->meter.value_source>=1 &&shader_transparent_meter->meter.value_source<=4)
							{
								meter_value = group->animation->values[shader_transparent_meter->meter.value_source - 1];
							}

							if (shader_transparent_meter->meter.gradient_source>=1 &&shader_transparent_meter->meter.gradient_source<=4)
							{
								gradient_value = group->animation->values[shader_transparent_meter->meter.gradient_source - 1];
							}

							if (shader_transparent_meter->meter.flash_extension_source>=1 &&shader_transparent_meter->meter.flash_extension_source<=4)
							{
								flash_extension = group->animation->values[shader_transparent_meter->meter.flash_extension_source - 1];
							}
						}

						if (rasterizer_debug_options.debug_meter_shader_enabled)
						{
							real debug_value = periodic_function_evaluate(_periodic_function_cosine, global_frame_parameters.game_time_sec / rasterizer_debug_options.pad3_scale);

							meter_brightness = rasterizer_debug_options.f[0]>=0.f ? rasterizer_debug_options.f[0] : debug_value;
							flash_brightness = rasterizer_debug_options.f[1]>=0.f ? rasterizer_debug_options.f[1] : debug_value;
							meter_value = rasterizer_debug_options.f[2]>=0.f ? rasterizer_debug_options.f[2] : debug_value;
							gradient_value = rasterizer_debug_options.f[3]>=0.f ? rasterizer_debug_options.f[3] : debug_value;
							flash_extension = rasterizer_debug_options.f[4]>=0.f ? rasterizer_debug_options.f[4] : debug_value;
						}

						flash_color.red = flash_brightness * shader_transparent_meter->meter.flash_color.red;
						flash_color.green = flash_brightness * shader_transparent_meter->meter.flash_color.green;
						flash_color.blue = flash_brightness * shader_transparent_meter->meter.flash_color.blue;
						flash_alpha = 1.f / MAX(gradient_value * 8.f, 1.f);

						if (TEST_FLAG(shader_transparent_meter->meter.flags, _shader_transparent_meter_tint_mode_2_bit))
						{
							real_rgb_color tint_color;
							real background_alpha = shader_transparent_meter->meter.background_transparency;
							real tint_alpha = shader_transparent_meter->meter.meter_transparency;

							tint_color.red = meter_brightness * shader_transparent_meter->meter.tint_color.red;
							tint_color.green = meter_brightness * shader_transparent_meter->meter.tint_color.green;
							tint_color.blue = meter_brightness * shader_transparent_meter->meter.tint_color.blue;
							gradient_min_pixel = real_a_rgb_color_to_pixel32(meter_value, &shader_transparent_meter->meter.gradient_min_color);
							gradient_max_pixel = real_a_rgb_color_to_pixel32(flash_alpha, &shader_transparent_meter->meter.gradient_max_color);
							background_pixel = real_a_rgb_color_to_pixel32(background_alpha, &shader_transparent_meter->meter.background_color);
							flash_pixel = real_a_rgb_color_to_pixel32(flash_extension, &flash_color);
							tint_pixel = real_a_rgb_color_to_pixel32(tint_alpha, &tint_color);
						}
						else
						{
							gradient_min_pixel = real_a_rgb_color_to_pixel32(meter_value, &shader_transparent_meter->meter.gradient_min_color);
							gradient_max_pixel = real_a_rgb_color_to_pixel32(flash_alpha, &shader_transparent_meter->meter.gradient_max_color);
							background_pixel = real_a_rgb_color_to_pixel32(0.f, &shader_transparent_meter->meter.background_color);
							flash_pixel = real_a_rgb_color_to_pixel32(flash_extension, &flash_color);
							tint_pixel = real_a_rgb_color_to_pixel32(meter_brightness, &shader_transparent_meter->meter.tint_color);
						}

						rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, shader_transparent_meter->meter.map.index, group->shader_permutation_index);
						SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
						SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
						SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, TEST_FLAG(shader_transparent_meter->meter.flags, _shader_transparent_meter_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
						SetTextureStageStateSmart(0, D3DTSS_MINFILTER, TEST_FLAG(shader_transparent_meter->meter.flags, _shader_transparent_meter_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
						SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, TEST_FLAG(shader_transparent_meter->meter.flags, _shader_transparent_meter_point_sampled_bit) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
						SetRenderStateSmart(D3DRS_CULLMODE, TEST_FLAG(shader_transparent_meter->meter.flags, _shader_transparent_meter_two_sided_bit) ? D3DCULL_NONE : D3DCULL_CCW);
						SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
						SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
						SetRenderStateSmart(D3DRS_SRCBLEND, TEST_FLAG(shader_transparent_meter->meter.flags, _shader_transparent_meter_tint_mode_2_bit) ? D3DBLEND_CONSTANTCOLOR : D3DBLEND_CONSTANTALPHA);
						SetRenderStateSmart(D3DRS_DESTBLEND, TEST_FLAG(shader_transparent_meter->meter.flags, _shader_transparent_meter_tint_mode_2_bit) ? D3DBLEND_SRCALPHA : D3DBLEND_CONSTANTCOLOR);
						SetRenderStateSmart(D3DRS_BLENDCOLOR, tint_pixel);
						SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
						SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
						rasterizer_set_vertex_shader_permutation(_vertex_shader_transparent_meter, vertex_type, shader_permutation_index);

						vsh_constants__texscale[0][0] = 1.f;
						vsh_constants__texscale[0][1] = 1.f;
						vsh_constants__texscale[0][2] = 1.f;
						vsh_constants__texscale[0][3] = 1.f;
						vsh_constants__texscale[1][0] = group->model_base_map_scale.i;
						vsh_constants__texscale[1][1] = 0.f;
						vsh_constants__texscale[1][2] = 0.f;
						vsh_constants__texscale[1][3] = 0.f;
						vsh_constants__texscale[2][0] = 0.f;
						vsh_constants__texscale[2][1] = group->model_base_map_scale.j;
						vsh_constants__texscale[2][2] = 0.f;
						vsh_constants__texscale[2][3] = 0.f;

						D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, VSH_CONSTANTS__TEXSCALE_OFFSET, vsh_constants__texscale, VSH_CONSTANTS__TEXSCALE_COUNT));

						memset(&pixel_shader, 0, sizeof(pixel_shader));
						SetTextureStageStateSmart(0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_ENABLE);
						pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
						pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(4, PS_COMBINERCOUNT_MUX_MSB | PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
						pixel_shader.PSConstant0[0] = flash_pixel;
						pixel_shader.PSConstant1[0] = gradient_max_pixel;
						pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_T0, PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_T0);
						pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTLEFT_2);
						pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_T0 | PS_INPUTMAPPING_SIGNED_NEGATE, PS_REGISTER_ONE);
						pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_SHIFTLEFT_2);
						pixel_shader.PSConstant0[1] = gradient_min_pixel;
						pixel_shader.PSConstant1[1] = gradient_max_pixel;
						pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_INPUTMAPPING_EXPAND_NEGATE, PS_REGISTER_ONE, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
						pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_C0, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_C1);
						pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						pixel_shader.PSConstant0[2] = gradient_min_pixel;
						pixel_shader.PSConstant1[2] = flash_pixel;
						pixel_shader.PSAlphaInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_ONE, PS_REGISTER_C0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_HALFBIAS_NEGATE, PS_REGISTER_ONE);
						pixel_shader.PSAlphaOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						pixel_shader.PSRGBInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_ONE, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, TEST_FLAG(shader_transparent_meter->meter.flags, _shader_transparent_meter_flash_color_is_negative_bit) ? PS_REGISTER_C1 | PS_INPUTMAPPING_SIGNED_NEGATE : PS_REGISTER_C1);
						pixel_shader.PSRGBOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_IDENTITY | PS_COMBINEROUTPUT_AB_MULTIPLY | PS_COMBINEROUTPUT_AB_CD_SUM);
						pixel_shader.PSConstant0[3] = background_pixel;
						pixel_shader.PSConstant1[3] = tint_pixel;
						pixel_shader.PSAlphaInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_C1 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_ONE);
						pixel_shader.PSAlphaOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_AB_CD_MUX);
						pixel_shader.PSRGBInputs[3] = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_ONE, PS_REGISTER_C0, PS_REGISTER_ONE);
						pixel_shader.PSRGBOutputs[3] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, PS_COMBINEROUTPUT_AB_CD_MUX);
						pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_R0, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
						pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO);

						if (rasterizer_debug_options.debug_meter_shader_enabled && rasterizer_debug_options.pad3)
						{
							memset(&pixel_shader, 0, sizeof(pixel_shader));
							SetTextureStageStateSmart(0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE);
							SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, FALSE);
							pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
							pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, PS_COMBINERCOUNT_MUX_LSB | PS_COMBINERCOUNT_SAME_C0 | PS_COMBINERCOUNT_SAME_C1);
							pixel_shader.PSFinalCombinerInputsABCD = (rasterizer_debug_options.pad3<=1 ? PS_CHANNEL_RGB : PS_CHANNEL_ALPHA) + PS_REGISTER_T0;
						}

						rasterizer_set_pixel_shader(&pixel_shader);
						rasterizer_transparent_geometry_group_draw__internal(group, FALSE);
						SetTextureStageStateSmart(0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE);
						continue;
					}
					case _shader_type_transparent_plasma:
						rasterizer_plasma_energy_draw(group);
						continue;
					default:
						error(_error_silent, "### ERROR unsupported shader type");
						success = FALSE;
						continue;
					}

					rasterizer_set_pixel_shader(&pixel_shader);
					rasterizer_transparent_geometry_group_draw__internal(group, FALSE);
				}

				if (TEST_FLAG(group->geometry_flags, _rasterizer_geometry_no_zbuffer_bit) && global_window_parameters.rasterizer_target==_rasterizer_target_render_primary)
				{
					rasterizer_set_frustum_z(0.f, 0.f);
				}

				if (TEST_FLAG(group->geometry_flags, _rasterizer_geometry_first_person_bit) && group->effect.type==_render_model_effect_type_active_camouflage)
				{
					rasterizer_set_frustum_z(0.f, 0.f);
				}
			}
		}

		if (!dirty)
		{
			rasterizer_xbox_transparent_geometry_last_source_object_index = group->source_object_index;
		}

		if (group->next_group_presorted_index!=NONE)
		{
			rasterizer_transparent_geometry_group_draw(rasterizer_transparent_geometry_get_group_from_presorted_index(group->next_group_presorted_index), dirty);
		}

		if (draw_active_camouflage_groups)
		{
			short dirty_count;
			short group_index;
			struct transparent_geometry_group const *groups2 = rasterizer_transparent_geometry_get_groups2(&dirty_count);

			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_transparent_geometry.c", 2272, !dirty);

			for (group_index = 0; group_index<dirty_count; group_index++)
			{
				if (groups2[group_index].active_camouflage_transparent_source_object_index==group->source_object_index && groups2[group_index].effect.type==_render_model_effect_type_active_camouflage)
				{
					rasterizer_transparent_geometry_group_draw(&groups2[group_index], TRUE);

					if (rasterizer_debug_options.pad3)
					{
						rasterizer_xbox_transparent_geometry_no_more_active_camouflage = TRUE;
					}
				}
			}
		}

		if (!success)
		{
			error(_error_silent, "### ERROR rasterizer_transparent_geometry_group_draw failed");
		}
	}

	return;
}

void rasterizer_transparent_geometry_groups_end(
	void)
{
	if (rasterizer_debug_options.__unknown88 && rasterizer_debug_options.transparent_pixel_counter_enabled)
	{
		ULONGLONG timestamp;
		HRESULT hr;
		long index = 0xFFF;
		unsigned long pixel_count = NONE;
		boolean success = TRUE;

		if (global_window_parameters.window_index==0)
		{
			rasterizer_xbox_transparent_geometry_pixel_count = 0;
		}

		D3DCALL(success, IDirect3DDevice8_EndVisibilityTest(global_d3d_device, index));

		do
		{
			hr = IDirect3DDevice8_GetVisibilityTestResult(global_d3d_device, index, &pixel_count, &timestamp);
		}
		while (hr==D3DERR_TESTINCOMPLETE);

		D3DCALL(success, hr);

		rasterizer_xbox_transparent_geometry_pixel_count += pixel_count;

		if (global_window_parameters.window_index==main_get_window_count() - 1)
		{
			long font_index = hud_globals->messaging.single_player_font.index;

			if (font_index!=NONE)
			{
				char string[256];
				rectangle2d bounds = global_window_parameters.camera.window_bounds;
				long window_area = (short)(global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0) * (short)(global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0);

				sprintf(string, "%.02f", (real)rasterizer_xbox_transparent_geometry_pixel_count / window_area);
				bounds.x0 = bounds.x1 - 160;
				bounds.y0 = bounds.y1 - 50;
				bounds.x1 -= 50;
				draw_string_set_format(NONE, 1, 0);
				draw_string_set_color(global_real_argb_yellow);
				draw_string_set_font(font_index);
				rasterizer_set_stencil_mode(_rasterizer_stencil_mode_none);
				rasterizer_draw_string(&bounds, NULL, NULL, 0, string);
			}
		}

		if (!success)
		{
			error(_error_silent, "### ERROR rasterizer_transparent_geometry_groups_begin failed");
		}
	}

	return;
}

real_vector4d *subtract_vectors4d(
	real_vector4d const *a,
	real_vector4d const *b,
	real_vector4d *result)
{
	result->i = a->i - b->i;
	result->j = a->j - b->j;
	result->k = a->k - b->k;
	result->l = a->l - b->l;

	return result;
}

real_vector4d *offset_vector4d(
	real_vector4d const *point,
	real_vector4d const *vector,
	real scale,
	real_vector4d *result)
{
	result->i = vector->i * scale + point->i;
	result->j = vector->j * scale + point->j;
	result->k = vector->k * scale + point->k;
	result->l = vector->l * scale + point->l;

	return result;
}

boolean rasterizer_transparent_geometry_initialize_aux_buffer(
	void)
{
	byte *vertices = NULL;
	boolean success = TRUE;

	D3DCALL(success, IDirect3DDevice8_CreateVertexBuffer(global_d3d_device, RASTERIZER_TRANSPARENT_GEOMETRY_TEXCOORD_STREAM_SIZE*(2*sizeof(byte)), RASTERIZER_STATIC_BUFFER_USAGE, 0, RASTERIZER_STATIC_BUFFER_POOL, &rasterizer_xbox_transparent_geometry_texcoord_stream));

	rasterizer_globals.current_lock_operation = _rasterizer_lock_vertexbuffer_new;
	D3DCALL(success, IDirect3DVertexBuffer8_Lock(rasterizer_xbox_transparent_geometry_texcoord_stream, 0, RASTERIZER_TRANSPARENT_GEOMETRY_TEXCOORD_STREAM_SIZE*(2*sizeof(byte)), (unsigned char**)&vertices, 0));
	rasterizer_globals.current_lock_operation = _rasterizer_lock_none;

	if (success && vertices)
	{
		long quad_index;
		char sequence[8] = { 0, 0, 0, -1, -1, -1, -1, 0 };

		for (quad_index = 0; quad_index<RASTERIZER_TRANSPARENT_GEOMETRY_TEXCOORD_STREAM_SIZE / sizeof(sequence); quad_index++)
		{
			memcpy(vertices, sequence, sizeof(sequence));
			vertices += sizeof(sequence);
		}

		D3DCALL(success, IDirect3DVertexBuffer8_Unlock(rasterizer_xbox_transparent_geometry_texcoord_stream));
	}
	else
	{
		success = FALSE;
		error(_error_silent, "### ERROR failed to allocate texcoord stream");
	}

	return success;
}

void rasterizer_transparent_geometry_dispose_aux_buffer(
	void)
{
	if (rasterizer_xbox_transparent_geometry_texcoord_stream)
	{
		IDirect3DVertexBuffer8_Release(rasterizer_xbox_transparent_geometry_texcoord_stream);
		rasterizer_xbox_transparent_geometry_texcoord_stream = NULL;
	}

	return;
}
