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
	/*000*/_vertex_shader_convolution,
	/*001*/_vertex_shader_debug,
	/*002*/_vertex_shader_decal,
	/*003*/_vertex_shader_detail_object_type0,
	/*004*/_vertex_shader_detail_object_type1,
	/*005*/_vertex_shader_effect,
	/*006*/_vertex_shader_effect_multitexture,
	/*007*/_vertex_shader_effect_multitexture_screenspace,
	/*008*/_vertex_shader_effect_zsprite,
	/*009*/_vertex_shader_environment_diffuse_light,
	/*010*/_vertex_shader_environment_fog,
	/*011*/_vertex_shader_environment_fog_screen,
	/*012*/_vertex_shader_environment_lightmap,
	/*013*/_vertex_shader_environment_reflection_bumped,
	/*014*/_vertex_shader_environment_reflection_flat,
	/*015*/_vertex_shader_environment_reflection_lightmap_mask,
	/*016*/_vertex_shader_environment_reflection_mirror,
	/*017*/_vertex_shader_environment_reflection_radiosity,
	/*018*/_vertex_shader_environment_shadow,
	/*019*/_vertex_shader_environment_specular_light,
	/*020*/_vertex_shader_environment_specular_lightmap,
	/*021*/_vertex_shader_environment_specular_spot_light,
	/*022*/_vertex_shader_environment_texture,
	/*023*/_vertex_shader_environment_vectors_debug0,
	/*024*/_vertex_shader_environment_vectors_debug1,
	/*025*/_vertex_shader_environment_vectors_debug2,
	/*026*/_vertex_shader_environment_vectors_debug3,
	/*027*/_vertex_shader_environment_vectors_debug4,
	/*028*/_vertex_shader_environment_vectors_debug5,
	/*029*/_vertex_shader_lens_flare,
	/*030*/_vertex_shader_model,
	/*031*/_vertex_shader_model_active_camouflage,
	/*032*/_vertex_shader_model_fast,
	/*033*/_vertex_shader_model_fogged,
	/*034*/_vertex_shader_model_fog_screen,
	/*035*/_vertex_shader_model_scenery,
	/*036*/_vertex_shader_model_shadow,
	/*037*/_vertex_shader_model_zbuffer,
	/*038*/_vertex_shader_screen,
	/*039*/_vertex_shader_screen2,
	/*040*/_vertex_shader_transparent_generic,
	/*041*/_vertex_shader_transparent_generic_lit_m,
	/*042*/_vertex_shader_transparent_generic_m,
	/*043*/_vertex_shader_transparent_generic_object_centered,
	/*044*/_vertex_shader_transparent_generic_object_centered_m,
	/*045*/_vertex_shader_transparent_generic_reflection,
	/*046*/_vertex_shader_transparent_generic_reflection_m,
	/*047*/_vertex_shader_transparent_generic_screenspace,
	/*048*/_vertex_shader_transparent_generic_screenspace_m,
	/*049*/_vertex_shader_transparent_generic_viewer_centered,
	/*050*/_vertex_shader_transparent_generic_viewer_centered_m,
	/*051*/_vertex_shader_transparent_glass_diffuse_light,
	/*052*/_vertex_shader_transparent_glass_diffuse_light_m,
	/*053*/_vertex_shader_transparent_glass_reflection_bumped,
	/*054*/_vertex_shader_transparent_glass_reflection_bumped_m,
	/*055*/_vertex_shader_transparent_glass_reflection_flat,
	/*056*/_vertex_shader_transparent_glass_reflection_flat_m,
	/*057*/_vertex_shader_transparent_glass_reflection_mirror,
	/*058*/_vertex_shader_transparent_glass_tint,
	/*059*/_vertex_shader_transparent_glass_tint_m,
	/*060*/_vertex_shader_transparent_meter,
	/*061*/_vertex_shader_transparent_meter_m,
	/*062*/_vertex_shader_transparent_plasma_m,
	/*063*/_vertex_shader_transparent_water_opacity,
	/*064*/_vertex_shader_transparent_water_opacity_m,
	/*065*/_vertex_shader_transparent_water_reflection,
	/*066*/_vertex_shader_transparent_water_reflection_m,
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
