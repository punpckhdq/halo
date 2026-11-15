/*
RASTERIZER_XBOX_VERTEX_SHADERS.H
December 6, 2000 12:15:09 PM
	template created.
*/

#include "platform.h"
#ifdef xbox

#ifndef __RASTERIZER_XBOX_VERTEX_SHADERS_H__
#define __RASTERIZER_XBOX_VERTEX_SHADERS_H__
#pragma once

enum // vertex shaders
{
	/*000*/_vertex_shader_debug,
	/*001*/_vertex_shader_decal,
	/*002*/_vertex_shader_effect_multitexture,
	/*003*/_vertex_shader_screen2,
	/*004*/_vertex_shader_screen,
	/*005*/_vertex_shader_model_fog_screen,
	/*006*/_vertex_shader_environment_fog,
	/*007*/_vertex_shader_environment_vectors_debug1,
	/*008*/_vertex_shader_environment_fog_screen,
	/*009*/_vertex_shader_model_fast,
	/*010*/_vertex_shader_model,
	/*011*/_vertex_shader_detail_object_type1,
	/*012*/_vertex_shader_effect_multitexture_screenspace,
	/*013*/_vertex_shader_model_zbuffer,
	/*014*/_vertex_shader_transparent_water_reflection_m,
	/*015*/_vertex_shader_transparent_plasma_m,
	/*016*/_vertex_shader_environment_lightmap,
	/*017*/_vertex_shader_model_fogged,
	/*018*/_vertex_shader_transparent_water_opacity_m,
	/*019*/_vertex_shader_transparent_generic_object_centered,
	/*020*/_vertex_shader_transparent_water_opacity,
	/*021*/_vertex_shader_environment_specular_light,
	/*022*/_vertex_shader_transparent_meter,
	/*023*/_vertex_shader_transparent_water_reflection,
	/*024*/_vertex_shader_transparent_generic,
	/*025*/_vertex_shader_transparent_glass_diffuse_light,
	/*026*/_vertex_shader_environment_reflection_bumped,
	/*027*/_vertex_shader_model_scenery,
	/*028*/_vertex_shader_transparent_glass_reflection_bumped,
	/*029*/_vertex_shader_environment_shadow,
	/*030*/_vertex_shader_transparent_glass_tint_m,
	/*031*/_vertex_shader_transparent_generic_lit_m,
	/*032*/_vertex_shader_transparent_meter_m,
	/*033*/_vertex_shader_detail_object_type0,
	/*034*/_vertex_shader_transparent_generic_reflection,
	/*035*/_vertex_shader_transparent_generic_viewer_centered,
	/*036*/_vertex_shader_transparent_glass_reflection_flat_m,
	/*037*/_vertex_shader_environment_vectors_debug0,
	/*038*/_vertex_shader_convolution,
	/*039*/_vertex_shader_model_shadow,
	/*040*/_vertex_shader_environment_texture,
	/*041*/_vertex_shader_environment_specular_spot_light,
	/*042*/_vertex_shader_environment_reflection_flat,
	/*043*/_vertex_shader_transparent_glass_reflection_flat,
	/*044*/_vertex_shader_environment_reflection_radiosity,
	/*045*/_vertex_shader_transparent_generic_object_centered_m,
	/*046*/_vertex_shader_transparent_glass_tint,
	/*047*/_vertex_shader_transparent_generic_m,
	/*048*/_vertex_shader_transparent_generic_screenspace,
	/*049*/_vertex_shader_environment_diffuse_light,
	/*050*/_vertex_shader_transparent_glass_diffuse_light_m,
	/*051*/_vertex_shader_environment_reflection_mirror,
	/*052*/_vertex_shader_environment_vectors_debug5,
	/*053*/_vertex_shader_environment_vectors_debug4,
	/*054*/_vertex_shader_environment_vectors_debug2,
	/*055*/_vertex_shader_environment_vectors_debug3,
	/*056*/_vertex_shader_lens_flare,
	/*057*/_vertex_shader_transparent_generic_reflection_m,
	/*058*/_vertex_shader_environment_reflection_lightmap_mask,
	/*059*/_vertex_shader_environment_specular_lightmap,
	/*060*/_vertex_shader_transparent_generic_screenspace_m,
	/*061*/_vertex_shader_transparent_glass_reflection_mirror,
	/*062*/_vertex_shader_transparent_generic_viewer_centered_m,
	/*063*/_vertex_shader_transparent_glass_reflection_bumped_m,
	/*064*/_vertex_shader_model_active_camouflage,
	/*065*/_vertex_shader_effect,
	/*066*/_vertex_shader_effect_zsprite,
	NUMBER_OF_VERTEX_SHADERS
};

struct vertex_shader_desc
{
	const unsigned long *declaration;
	const unsigned long *code;
	unsigned long handle;
	unsigned long size;
};

extern struct vertex_shader_desc vertex_shader_table[NUMBER_OF_VERTEX_SHADERS];
extern char vertex_shader_names[NUMBER_OF_VERTEX_SHADERS][MAXIMUM_FILENAME_LENGTH+1];

#endif // __RASTERIZER_XBOX_VERTEX_SHADERS_H__

#endif // xbox
