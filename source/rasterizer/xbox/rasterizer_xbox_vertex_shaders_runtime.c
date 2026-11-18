/*
RASTERIZER_XBOX_VERTEX_SHADERS_RUNTIME.C
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

enum
{
	NUMBER_OF_PACKED_VERTEX_SHADERS = 3, /* fake name */
	MAXIMUM_PACKED_VERTEX_SHADER_SLOTS = 136 /* fake name */
};

/* ---------- structures */

struct packed_vertex_shader /* fake name */
{
	short vertex_shader_index;
	boolean loaded; /* fake name */
	unsigned long offset;
};

/* ---------- globals */

static short current_vertex_shader_index = NONE; /* fake name */
static struct packed_vertex_shader packed_shaders[NUMBER_OF_PACKED_VERTEX_SHADERS]=
{
	{ _vertex_shader_model, FALSE, 0 },
	{ _vertex_shader_model_fast, FALSE, 0 },
	{ _vertex_shader_model_scenery, FALSE, 0 }
};

/* ---------- public code */

void rasterizer_set_vertex_shader_permutation(
	short vertex_shader_index,
	short vertex_type,
	short permutation_index)
{
	boolean success = TRUE;
	short const *translation_table = NULL;
	short permutation_count = 1;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_vertex_shaders_runtime.c", 135, vertex_shader_index>=0 && vertex_shader_index<NUMBER_OF_VERTEX_SHADERS);

	switch (vertex_shader_index)
	{
	case _vertex_shader_debug:
		{
			static short const debug_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				_vertex_shader_debug,
				NONE,
				NONE
			};

			translation_table = debug_translation_table;
		}
		break;
	case _vertex_shader_decal:
		{
			static short const decal_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				_vertex_shader_decal,
				NONE
			};

			translation_table = decal_translation_table;
		}
		break;
	case _vertex_shader_screen:
		{
			static short const screen_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES*2]=
			{
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				_vertex_shader_screen, _vertex_shader_screen2,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE
			};

			permutation_count = 2;
			translation_table = screen_translation_table;
		}
		break;
	case _vertex_shader_convolution:
		{
			static short const screen_texture_animation_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				_vertex_shader_convolution,
				NONE,
				NONE,
				NONE
			};

			translation_table = screen_texture_animation_translation_table;
		}
		break;
	case _vertex_shader_effect:
		{
			static short const transparent_effect_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES*4]=
			{
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				_vertex_shader_effect, _vertex_shader_effect_multitexture, _vertex_shader_effect_multitexture_screenspace, _vertex_shader_effect_zsprite,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE
			};

			translation_table = transparent_effect_translation_table;
			permutation_count = 4;
		}
		break;
	case _vertex_shader_lens_flare:
		{
			static short const sun_glow_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				_vertex_shader_lens_flare,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = sun_glow_translation_table;
		}
		break;
	case _vertex_shader_environment_lightmap:
		{
			static short const environment_lightmap_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_environment_lightmap,
				_vertex_shader_environment_lightmap,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = environment_lightmap_translation_table;
		}
		break;
	case _vertex_shader_environment_diffuse_light:
		{
			static short const environment_diffuse_light_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_environment_diffuse_light,
				_vertex_shader_environment_diffuse_light,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = environment_diffuse_light_translation_table;
		}
		break;
	case _vertex_shader_environment_shadow:
		{
			static short const environment_shadow_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_environment_shadow,
				_vertex_shader_environment_shadow,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = environment_shadow_translation_table;
		}
		break;
	case _vertex_shader_environment_texture:
		{
			static short const environment_texture_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_environment_texture,
				_vertex_shader_environment_texture,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = environment_texture_translation_table;
		}
		break;
	case _vertex_shader_environment_specular_light:
		{
			static short const environment_specular_light_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES*3]=
			{
				_vertex_shader_environment_specular_light, _vertex_shader_environment_specular_spot_light, _vertex_shader_environment_specular_lightmap,
				_vertex_shader_environment_specular_light, _vertex_shader_environment_specular_spot_light, _vertex_shader_environment_specular_lightmap,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE
			};

			translation_table = environment_specular_light_translation_table;
			permutation_count = 3;
		}
		break;
	case _vertex_shader_environment_reflection_lightmap_mask:
		{
			static short const environment_reflection_lightmap_mask_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_environment_reflection_lightmap_mask,
				_vertex_shader_environment_reflection_lightmap_mask,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = environment_reflection_lightmap_mask_translation_table;
		}
		break;
	case _vertex_shader_environment_reflection_flat:
		{
			static short const environment_reflection_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES*3]=
			{
				_vertex_shader_environment_reflection_bumped, _vertex_shader_environment_reflection_flat, _vertex_shader_environment_reflection_radiosity,
				_vertex_shader_environment_reflection_bumped, _vertex_shader_environment_reflection_flat, _vertex_shader_environment_reflection_radiosity,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE
			};

			translation_table = environment_reflection_translation_table;
			permutation_count = 3;
		}
		break;
	case _vertex_shader_environment_reflection_mirror:
		{
			static short const environment_reflection_mirror_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_environment_reflection_mirror,
				_vertex_shader_environment_reflection_mirror,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = environment_reflection_mirror_translation_table;
		}
		break;
	case _vertex_shader_environment_fog:
		{
			static short const environment_fog_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_environment_fog,
				_vertex_shader_environment_fog,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = environment_fog_translation_table;
		}
		break;
	case _vertex_shader_environment_fog_screen:
		{
			static short const environment_fog_screen_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_environment_fog_screen,
				_vertex_shader_environment_fog_screen,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = environment_fog_screen_translation_table;
		}
		break;
	case _vertex_shader_environment_vectors_debug0:
		{
			static short const environment_vectors_debug_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES*6]=
			{
				_vertex_shader_environment_vectors_debug0, _vertex_shader_environment_vectors_debug1, _vertex_shader_environment_vectors_debug2, _vertex_shader_environment_vectors_debug3, _vertex_shader_environment_vectors_debug4, _vertex_shader_environment_vectors_debug5,
				_vertex_shader_environment_vectors_debug0, _vertex_shader_environment_vectors_debug1, _vertex_shader_environment_vectors_debug2, _vertex_shader_environment_vectors_debug3, _vertex_shader_environment_vectors_debug4, _vertex_shader_environment_vectors_debug5,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE
			};

			translation_table = environment_vectors_debug_translation_table;
			permutation_count = 6;
		}
		break;
	case _vertex_shader_model:
		{
			static short const model_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES*4]=
			{
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				_vertex_shader_model_fogged, _vertex_shader_model, _vertex_shader_model_fast, _vertex_shader_model_scenery,
				_vertex_shader_model_fogged, _vertex_shader_model, _vertex_shader_model_fast, _vertex_shader_model_scenery,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE
			};

			translation_table = model_translation_table;
			permutation_count = 4;
		}
		break;
	case _vertex_shader_model_active_camouflage:
		{
			static short const model_active_camouflage_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				NONE,
				NONE,
				NONE,
				NONE,
				_vertex_shader_model_active_camouflage,
				_vertex_shader_model_active_camouflage,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = model_active_camouflage_translation_table;
		}
		break;
	case _vertex_shader_model_shadow:
		{
			static short const model_shadow_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				NONE,
				NONE,
				NONE,
				NONE,
				_vertex_shader_model_shadow,
				_vertex_shader_model_shadow,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = model_shadow_translation_table;
		}
		break;
	case _vertex_shader_model_zbuffer:
		{
			static short const model_zbuffer_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				NONE,
				NONE,
				NONE,
				NONE,
				_vertex_shader_model_zbuffer,
				_vertex_shader_model_zbuffer,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = model_zbuffer_translation_table;
		}
		break;
	case _vertex_shader_model_fog_screen:
		{
			static short const model_fog_screen_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				NONE,
				NONE,
				NONE,
				NONE,
				_vertex_shader_model_fog_screen,
				_vertex_shader_model_fog_screen,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = model_fog_screen_translation_table;
		}
		break;
	case _vertex_shader_transparent_generic:
		{
			static short const transparent_generic_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES*6]=
			{
				_vertex_shader_transparent_generic, _vertex_shader_transparent_generic_screenspace, _vertex_shader_transparent_generic_reflection, _vertex_shader_transparent_generic_object_centered, _vertex_shader_transparent_generic_viewer_centered, NONE,
				_vertex_shader_transparent_generic, _vertex_shader_transparent_generic_screenspace, _vertex_shader_transparent_generic_reflection, _vertex_shader_transparent_generic_object_centered, _vertex_shader_transparent_generic_viewer_centered, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				_vertex_shader_transparent_generic_m, _vertex_shader_transparent_generic_screenspace_m, _vertex_shader_transparent_generic_reflection_m, _vertex_shader_transparent_generic_object_centered_m, _vertex_shader_transparent_generic_viewer_centered_m, _vertex_shader_transparent_generic_lit_m,
				_vertex_shader_transparent_generic_m, _vertex_shader_transparent_generic_screenspace_m, _vertex_shader_transparent_generic_reflection_m, _vertex_shader_transparent_generic_object_centered_m, _vertex_shader_transparent_generic_viewer_centered_m, _vertex_shader_transparent_generic_lit_m,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE,
				NONE, NONE, NONE, NONE, NONE, NONE
			};

			translation_table = transparent_generic_translation_table;
			permutation_count = 6;
		}
		break;
	case _vertex_shader_transparent_water_opacity:
		{
			static short const transparent_water_opacity_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_transparent_water_opacity,
				_vertex_shader_transparent_water_opacity,
				NONE,
				NONE,
				_vertex_shader_transparent_water_opacity_m,
				_vertex_shader_transparent_water_opacity_m,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = transparent_water_opacity_translation_table;
		}
		break;
	case _vertex_shader_transparent_water_reflection:
		{
			static short const transparent_water_reflection_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_transparent_water_reflection,
				_vertex_shader_transparent_water_reflection,
				NONE,
				NONE,
				_vertex_shader_transparent_water_reflection_m,
				_vertex_shader_transparent_water_reflection_m,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = transparent_water_reflection_translation_table;
		}
		break;
	case _vertex_shader_transparent_glass_tint:
		{
			static short const transparent_glass_tint_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_transparent_glass_tint,
				_vertex_shader_transparent_glass_tint,
				NONE,
				NONE,
				_vertex_shader_transparent_glass_tint_m,
				_vertex_shader_transparent_glass_tint_m,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = transparent_glass_tint_translation_table;
		}
		break;
	case _vertex_shader_transparent_glass_reflection_flat:
		{
			static short const transparent_glass_reflection_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES*3]=
			{
				_vertex_shader_transparent_glass_reflection_bumped, _vertex_shader_transparent_glass_reflection_flat, _vertex_shader_transparent_glass_reflection_mirror,
				_vertex_shader_transparent_glass_reflection_bumped, _vertex_shader_transparent_glass_reflection_flat, _vertex_shader_transparent_glass_reflection_mirror,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				_vertex_shader_transparent_glass_reflection_bumped_m, _vertex_shader_transparent_glass_reflection_flat_m, NONE,
				_vertex_shader_transparent_glass_reflection_bumped_m, _vertex_shader_transparent_glass_reflection_flat_m, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE,
				NONE, NONE, NONE
			};

			translation_table = transparent_glass_reflection_translation_table;
			permutation_count = 3;
		}
		break;
	case _vertex_shader_transparent_glass_diffuse_light:
		{
			static short const transparent_glass_diffuse_light_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_transparent_glass_diffuse_light,
				_vertex_shader_transparent_glass_diffuse_light,
				NONE,
				NONE,
				_vertex_shader_transparent_glass_diffuse_light_m,
				_vertex_shader_transparent_glass_diffuse_light_m,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = transparent_glass_diffuse_light_translation_table;
		}
		break;
	case _vertex_shader_transparent_meter:
		{
			static short const transparent_meter_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				_vertex_shader_transparent_meter,
				_vertex_shader_transparent_meter,
				NONE,
				NONE,
				_vertex_shader_transparent_meter_m,
				_vertex_shader_transparent_meter_m,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = transparent_meter_translation_table;
		}
		break;
	case _vertex_shader_transparent_plasma_m:
		{
			static short const transparent_plasma_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES]=
			{
				NONE,
				NONE,
				NONE,
				NONE,
				_vertex_shader_transparent_plasma_m,
				_vertex_shader_transparent_plasma_m,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE,
				NONE
			};

			translation_table = transparent_plasma_translation_table;
		}
		break;
	case _vertex_shader_detail_object_type0:
		{
			static short const detail_object_translation_table[NUMBER_OF_RASTERIZER_VERTEX_TYPES*2]=
			{
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				NONE, NONE,
				_vertex_shader_detail_object_type0, _vertex_shader_detail_object_type1
			};

			translation_table = detail_object_translation_table;
			permutation_count = 2;
		}
		break;
	default:
		match_vhalt("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_vertex_shaders_runtime.c", 876, "### ERROR unsupported vertex shader");
	}

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_vertex_shaders_runtime.c", 879, vertex_type>=0 && vertex_type<NUMBER_OF_RASTERIZER_VERTEX_TYPES);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_vertex_shaders_runtime.c", 880, permutation_index>=0 && permutation_index<permutation_count);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_vertex_shaders_runtime.c", 881, translation_table[vertex_type*permutation_count + permutation_index]!=NONE);

	vertex_shader_index = translation_table[vertex_type*permutation_count + permutation_index];

	if (vertex_shader_index!=current_vertex_shader_index)
	{
		short packed_shader_index;

		if (!packed_shaders[1].offset)
		{
			long offset = 0;

			for (packed_shader_index = 0; packed_shader_index<NUMBER_OF_PACKED_VERTEX_SHADERS; packed_shader_index++)
			{
				unsigned long size;

				D3DCALL(success, IDirect3DDevice8_GetVertexShaderSize(global_d3d_device, vertex_shader_table[packed_shaders[packed_shader_index].vertex_shader_index].handle, &size));
				packed_shaders[packed_shader_index].offset = offset;
				offset += size;
			}

			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_vertex_shaders_runtime.c", 930, offset<=MAXIMUM_PACKED_VERTEX_SHADER_SLOTS, "### ERROR packed vertex shaders don't fit in GPU memory");
		}

		for (packed_shader_index = 0; packed_shader_index<NUMBER_OF_PACKED_VERTEX_SHADERS; packed_shader_index++)
		{
			if (vertex_shader_index==packed_shaders[packed_shader_index].vertex_shader_index)
			{
				break;
			}
		}

		if (packed_shader_index<NUMBER_OF_PACKED_VERTEX_SHADERS && packed_shaders[packed_shader_index].loaded)
		{
			unsigned long offset = packed_shaders[packed_shader_index].offset;

			D3DCALL(success, IDirect3DDevice8_SelectVertexShader(global_d3d_device, 0L, (UINT)offset));
		}
		else
		{
			if (packed_shader_index<NUMBER_OF_PACKED_VERTEX_SHADERS)
			{
				unsigned long offset = packed_shaders[packed_shader_index].offset;
				unsigned long handle = vertex_shader_table[packed_shaders[packed_shader_index].vertex_shader_index].handle;

				match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_vertex_shaders_runtime.c", 956, handle, "### ERROR vertex shader was not valid");

				D3DCALL(success, IDirect3DDevice8_LoadVertexShader(global_d3d_device, (DWORD)handle, (UINT)offset));
				D3DCALL(success, IDirect3DDevice8_SelectVertexShader(global_d3d_device, (DWORD)handle, (UINT)offset));
				packed_shaders[packed_shader_index].loaded = TRUE;
			}
			else
			{
				unsigned long handle = vertex_shader_table[vertex_shader_index].handle;

				match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_vertex_shaders_runtime.c", 969, handle, "### ERROR vertex shader was not valid");

				D3DCALL(success, IDirect3DDevice8_SetVertexShader(global_d3d_device, (DWORD)handle));

				for (packed_shader_index = 0; packed_shader_index<NUMBER_OF_PACKED_VERTEX_SHADERS; packed_shader_index++)
				{
					packed_shaders[packed_shader_index].loaded = FALSE;
				}
			}

			if (rasterizer_debug_options.statistics_mode)
			{
				rasterizer_frame_statistics.vertex_shader_bytes += vertex_shader_table[vertex_shader_index].size;
			}
		}

		current_vertex_shader_index = vertex_shader_index;

#ifdef DEBUG
		if (!success)
		{
			error(_error_silent, "### ERROR rasterizer_set_vertex_shader failed");
		}
#endif
	}

	return;
}
