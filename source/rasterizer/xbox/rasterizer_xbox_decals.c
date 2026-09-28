/*
RASTERIZER_XBOX_DECALS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "cheats.h"
#include "game.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "object_definitions.h"
#include "rasterizer_geometry.h"
#include "model_animation_definitions.h"
#include "model_definitions.h"
#include "models.h"
#include "damage_resistances.h"
#include "shader_definitions.h"
#include "objects.h"
#include "bsp3d.h"
#include "bsp2d.h"
#include "collision_bsp_definitions.h"
#include "collision_bsp.h"
#include "leaf_map.h"
#include "render_cameras.h"
#include "structure_bsp_definitions.h"
#include "render.h"
#include "rasterizer.h"
#include "bitmap_macros.h"
#include "bitmaps_inlines.h"
#include "lruv_cache.h"
#include "memory_pool.h"
#include "rasterizer_console_vars.h"
#include "game_state.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "collision_usage.h"
#include "collision_features.h"
#include "collisions.h"
#include "decal_definitions.h"
#include "decals.h"
#include "rasterizer/xbox/rasterizer_xbox.h"

/* ---------- prototypes */

/* ---------- globals */

static short local_layer = 0;
static long local_bitmap_group_index = 0;
static short local_bitmap_index = 0;
static short local_framebuffer_blend_function = 0;
static IDirect3DVertexBuffer8 *local_d3d_vertex_buffer = NULL;
static struct lruv_cache *local_vertex_cache = NULL;
static boolean locked_warning_issued = FALSE;
static boolean permanent_warning_issued = FALSE;
static boolean local_filthy_decal_fog_hack_enabled = FALSE;

static long last_decal_index_queried_by_lruv_cache = NONE;

/* ---------- private code */

static void rasterizer_decal_vertices_purge_proc(
	long decal_index)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 29, lruv_has_locked_proc(local_vertex_cache));
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 30, decal_index);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 31, decal_index!=NONE);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 32, decal_index!=0);

	if (TEST_FLAG(decal_get(decal_index)->flags, _decal_locked_bit) && !locked_warning_issued)
	{
		error(_error_silent, "### ERROR decals: deleting locked decal (#%d, queried=#%d) in rasterizer -- tell Bernie!!", decal_index, last_decal_index_queried_by_lruv_cache);
		locked_warning_issued = TRUE;
	}

	if (TEST_FLAG(decal_get(decal_index)->flags, _decal_permanent_bit) && !permanent_warning_issued)
	{
		error(_error_silent, "### ERROR decals: deleting permanent decal (#%d, queried=#%d) in rasterizer -- tell Bernie!!", decal_index, last_decal_index_queried_by_lruv_cache);
		permanent_warning_issued = TRUE;
	}

	decal_delete(decal_index);

	return;
}

static boolean rasterizer_decal_vertices_locked_proc(
	long decal_index)
{
	struct decal_datum *decal;
	boolean locked;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 71, decal_index);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 72, decal_index!=NONE);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 73, decal_index!=0);

	decal = decal_get(decal_index);

	locked = TEST_FLAG(decal->flags, _decal_locked_bit) || TEST_FLAG(decal->flags, _decal_permanent_bit);

	last_decal_index_queried_by_lruv_cache = decal_index;

	return locked;
}

/* ---------- public code */

void _rasterizer_decals_initialize(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 89, global_d3d_device);

	local_d3d_vertex_buffer = match_malloc("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 91, sizeof(IDirect3DVertexBuffer8));
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 92, local_d3d_vertex_buffer);
	local_d3d_vertex_buffer->Common = D3DCOMMON_TYPE_VERTEXBUFFER | 1;
	local_d3d_vertex_buffer->Data = (unsigned long)game_state_gpu_malloc("decal vertices", NULL, MAXIMUM_DECAL_VERTICES_PER_MAP*sizeof(struct decal_vertex));
	local_d3d_vertex_buffer->Lock = 0;
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 96, local_d3d_vertex_buffer->Data);

	IDirect3DVertexBuffer8_Register(local_d3d_vertex_buffer, NULL);

	local_vertex_cache = game_state_lruv_cache_new("decal vertex cache",
		MAXIMUM_DECAL_VERTICES_PER_MAP*sizeof(struct decal_vertex)/64,
		6,
		MAXIMUM_DECALS_PER_MAP,
		rasterizer_decal_vertices_purge_proc,
		rasterizer_decal_vertices_locked_proc);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 106, local_vertex_cache);

	return;
}

void _rasterizer_decals_update_function_pointers(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 116, local_vertex_cache);

	lruv_update_function_pointers(local_vertex_cache, rasterizer_decal_vertices_purge_proc, rasterizer_decal_vertices_locked_proc);

	return;
}

void _rasterizer_decals_initialize_for_new_map(
	void)
{
	return;
}

void _rasterizer_decals_dispose_from_old_map(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 131, local_vertex_cache);

	decals_unlock(TRUE);
	lruv_flush(local_vertex_cache);

	return;
}

void _rasterizer_decals_flush(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 142, local_vertex_cache);

	decals_unlock(FALSE);
	lruv_flush(local_vertex_cache);

	return;
}

void _rasterizer_decals_dispose(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 153, local_vertex_cache);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 154, local_d3d_vertex_buffer);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 155, global_d3d_device);

	if (local_d3d_vertex_buffer)
	{
		IDirect3DVertexBuffer8_Release(local_d3d_vertex_buffer);
		local_d3d_vertex_buffer = NULL;
	}

	lruv_delete(local_vertex_cache);

	return;
}

void rasterizer_decal_vertices_begin_update(
	void)
{
	lruv_idle(local_vertex_cache);

	return;
}

void rasterizer_decal_vertices_end_update(
	void)
{
	return;
}

long _rasterizer_decal_vertices_new(
	long cache_size)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 204, cache_size>sizeof(struct decal_vertex));
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 205, cache_size%sizeof(struct decal_vertex)==0);

	return lruv_block_new(local_vertex_cache, cache_size);
}

void *_rasterizer_decal_vertices_lock(
	long cache_index,
	long cache_size)
{
	byte *result = NULL;
	unsigned long cache_offset;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 217, cache_index!=NONE);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 218, local_vertex_cache);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 219, global_d3d_device);

	cache_offset = lruv_block_get_address(local_vertex_cache, cache_index);

	rasterizer_globals.current_lock_operation = _rasterizer_lock_decal_vertices;

	IDirect3DVertexBuffer8_Lock(local_d3d_vertex_buffer, cache_offset, cache_size, &result, D3DLOCK_READONLY);

	rasterizer_globals.current_lock_operation = _rasterizer_lock_none;

	return result;
}

void _rasterizer_decal_vertices_unlock(
	void)
{
	IDirect3DVertexBuffer8_Unlock(local_d3d_vertex_buffer);

	return;
}

void _rasterizer_decal_vertices_delete(
	long cache_index)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 262, cache_index!=NONE);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 263, local_vertex_cache);

	lruv_block_delete(local_vertex_cache, cache_index);

	return;
}

void _rasterizer_decals_begin(
	short layer)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 283, global_d3d_device);

	{
		short const decal_profiles[NUMBER_OF_DECAL_LAYERS] =
		{
			_rasterizer_profile_environment_decals_primary,
			_rasterizer_profile_environment_decals_secondary,
			_rasterizer_profile_environment_decals_light,
			_rasterizer_profile_environment_decals_alpha_tested,
			_rasterizer_profile_environment_decals_water
		};
		if (layer>=0 && layer<NUMBER_OF_DECAL_LAYERS)
		{
			rasterizer_profile_begin(decal_profiles[layer]);
		}
	}

	local_layer = layer;

	if (!rasterizer_debug_options.drawing_mode && rasterizer_debug_options.draw_environment_decals)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 307, layer>=0 && layer<NUMBER_OF_DECAL_LAYERS);

		local_framebuffer_blend_function = NONE;
		local_bitmap_index = NONE;
		local_bitmap_group_index = NONE;

		local_filthy_decal_fog_hack_enabled = FALSE;

		rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, NONE, 0);

		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_CCW);

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, TRUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, rasterizer_debug_options.zbias);

		if (layer == _decal_layer_alpha_tested)
		{
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, TRUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHAREF, 127);

			rasterizer_set_stencil_mode(_rasterizer_stencil_mode_write_alpha_tested_decal);
		}
		else
		{

			if (rasterizer_debug_options.filthy_decal_fog_hack_enabled && global_window_parameters.fog.atmospheric_maximum_density == 1.f)
			{
				local_filthy_decal_fog_hack_enabled = TRUE;
			}

			if (local_filthy_decal_fog_hack_enabled)
			{
				IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, TRUE);
				IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHAREF, 0);
			}
			else
			{
				IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
			}
		}

		rasterizer_set_vertex_shader_permutation(1, _rasterizer_vertex_type_decal, 0);

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
		pixel_shader.PSAlphaOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
		pixel_shader.PSRGBOutputs[1] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);

		if (local_filthy_decal_fog_hack_enabled)
		{
			pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(3, 0);
			pixel_shader.PSConstant0[0] = D3DCOLOR_ARGB(1, 0, 0, 0);
			pixel_shader.PSAlphaInputs[2] = PS_COMBINERINPUTS(PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_V1 | PS_CHANNEL_ALPHA, PS_REGISTER_C0 | PS_CHANNEL_ALPHA, PS_REGISTER_V1 | PS_CHANNEL_ALPHA);
			pixel_shader.PSAlphaOutputs[2] = PS_COMBINEROUTPUTS(PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, PS_REGISTER_R0, 0);
		}
		else
		{
			pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(2, 0);
		}

		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, 0);

		IDirect3DDevice8_SetStreamSource(global_d3d_device, 0, local_d3d_vertex_buffer, sizeof(struct decal_vertex));
	}

	return;
}

void _rasterizer_decals_draw(
	short cluster_index)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 408, global_d3d_device);

	if (!rasterizer_debug_options.drawing_mode && rasterizer_debug_options.draw_environment_decals)
	{
		long decal_index;
		struct decal_datum *decal;

		for (decal_index=decal_get_first_decal_index(cluster_index, local_layer); decal_index!=NONE; decal_index=decal->next_decal_index)
		{
			struct decal_definition *definition;
			struct _shader_decal *shader;
			pixel32 color;
			unsigned long intensity;
			unsigned long vertex_data_offset;

			decal = decal_get(decal_index);
			definition = decal_definition_get(decal->definition_index);
			shader = &definition->shader.decal;

			if (local_framebuffer_blend_function != shader->framebuffer_blend_function)
			{
				local_framebuffer_blend_function = shader->framebuffer_blend_function;

				if (local_framebuffer_blend_function == _shader_framebuffer_blend_function_multiply || local_framebuffer_blend_function == _shader_framebuffer_blend_function_double_multiply)
				{
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);
				}
				else
				{
					IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
				}

				switch (local_framebuffer_blend_function)
				{
				case _shader_framebuffer_blend_function_add:
				case _shader_framebuffer_blend_function_reverse_subtract:
				case _shader_framebuffer_blend_function_max:
					pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_V0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					break;
				case _shader_framebuffer_blend_function_multiply:
				case _shader_framebuffer_blend_function_min:
					pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_V0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					break;
				case _shader_framebuffer_blend_function_double_multiply:
					pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_INPUTMAPPING_HALFBIAS_NEGATE, PS_REGISTER_V0 | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T0, PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT);
					pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_HALFBIAS_NEGATE);
					pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO | PS_INPUTMAPPING_HALFBIAS_NEGATE);
					break;
				case _shader_framebuffer_blend_function_alpha_blend:
					pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_V0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					break;
				case _shader_framebuffer_blend_function_alpha_multiply_add:
					pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_V0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSRGBInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_R0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					pixel_shader.PSAlphaInputs[1] = PS_COMBINERINPUTS(PS_REGISTER_V0 | PS_CHANNEL_ALPHA | PS_INPUTMAPPING_UNSIGNED_INVERT, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
					break;
				default:
					match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 470, FALSE, "### ERROR unsupported framebuffer blend function");

				}

				rasterizer_set_framebuffer_blend_function(local_framebuffer_blend_function);
				rasterizer_set_pixel_shader(&pixel_shader);

				if (rasterizer_debug_options.statistics_mode == 2)
				{
					rasterizer_frame_statistics.decal_shader_count++;
				}
			}

			if (local_bitmap_group_index != shader->map.index || local_bitmap_index != decal->bitmap_index)
			{
				local_bitmap_group_index = shader->map.index;
				local_bitmap_index = decal->bitmap_index;

				rasterizer_set_texture(0, _bitmap_type_2d, _bitmap_usage_multiplicative, local_bitmap_group_index, local_bitmap_index);

				if (rasterizer_debug_options.statistics_mode == 2)
				{
					rasterizer_frame_statistics.decal_texture_count++;
				}
			}

			vertex_data_offset = lruv_block_get_address(local_vertex_cache, decal_index);

			color = decal->color;
			intensity = ((color>>24)*decal->intensity+127)>>8;

			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 510, intensity<=PIXEL32_COMPONENT_MASK);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_decals.c", 511, vertex_data_offset%sizeof(struct decal_vertex)==0);

			IDirect3DDevice8_SetVertexData4ub(global_d3d_device, D3DVSDE_TEXCOORD0, (byte)(color>>16), (byte)(color>>8), (byte)color, (byte)(PIXEL32_COMPONENT_MASK-intensity));
			IDirect3DDevice8_DrawPrimitive(global_d3d_device, D3DPT_QUADLIST, vertex_data_offset/sizeof(struct decal_vertex), decal->quad_count);

			if (rasterizer_debug_options.statistics_mode == 2)
			{
				rasterizer_frame_statistics.decal_primitive_count++;
				rasterizer_frame_statistics.decal_triangle_count += decal->quad_count*2;
				rasterizer_frame_statistics.decal_vertex_count += decal->quad_count*4;
			}
		}
	}

	return;
}

void _rasterizer_decals_end(
	void)
{
	if (local_layer == _decal_layer_alpha_tested)
	{
		rasterizer_set_stencil_mode(_rasterizer_stencil_mode_reject);
	}
	{
		short const decal_profiles[NUMBER_OF_DECAL_LAYERS] =
		{
			_rasterizer_profile_environment_decals_primary,
			_rasterizer_profile_environment_decals_secondary,
			_rasterizer_profile_environment_decals_light,
			_rasterizer_profile_environment_decals_alpha_tested,
			_rasterizer_profile_environment_decals_water
		};
		if (local_layer>=0 && local_layer<NUMBER_OF_DECAL_LAYERS)
		{
			rasterizer_profile_end(decal_profiles[local_layer]);
		}
	}

	return;
}
