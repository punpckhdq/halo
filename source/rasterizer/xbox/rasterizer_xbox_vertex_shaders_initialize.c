/*
RASTERIZER_XBOX_VERTEX_SHADERS_INITIALIZE.C
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

/* ---------- globals */

static unsigned long const vertex_declaration_debug[]= /* fake name */
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),
	D3DVSD_REG(9, D3DVSDT_D3DCOLOR),
	D3DVSD_END()
};

static unsigned long const vertex_declaration_decal[]= /* fake name */
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),
	D3DVSD_REG(4, D3DVSDT_NORMSHORT2),
	D3DVSD_END()
};

static unsigned long const vertex_declaration_detail_object[]= /* fake name */
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_PBYTE3),
	D3DVSD_REG(9, D3DVSDT_PBYTE3),
	D3DVSD_REG(8, D3DVSDT_SHORT1),
	D3DVSD_END()
};

static unsigned long const vertex_declaration_dynamic_unlit[]= /* fake name */
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),
	D3DVSD_REG(4, D3DVSDT_FLOAT2),
	D3DVSD_REG(9, D3DVSDT_D3DCOLOR),
	D3DVSD_END()
};

static unsigned long const vertex_declaration_unlit_zsprite[]= /* fake name */
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),
	D3DVSD_REG(4, D3DVSDT_FLOAT2),
	D3DVSD_REG(9, D3DVSDT_D3DCOLOR),
	D3DVSD_STREAM(1),
	D3DVSD_REG(11, D3DVSDT_PBYTE2),
	D3DVSD_END()
};

static unsigned long const vertex_declaration_dynamic_screen[]= /* fake name */
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT2),
	D3DVSD_REG(4, D3DVSDT_FLOAT2),
	D3DVSD_REG(9, D3DVSDT_D3DCOLOR),
	D3DVSD_END()
};

static unsigned long const vertex_declaration_model_compressed[]= /* fake name */
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),
	D3DVSD_REG(1, D3DVSDT_NORMPACKED3),
	D3DVSD_REG(2, D3DVSDT_NORMPACKED3),
	D3DVSD_REG(3, D3DVSDT_NORMPACKED3),
	D3DVSD_REG(4, D3DVSDT_NORMSHORT2),
	D3DVSD_REG(5, D3DVSDT_PBYTE2),
	D3DVSD_REG(6, D3DVSDT_NORMSHORT1),
	D3DVSD_END()
};

static unsigned long const vertex_declaration_environment_lightmap_compressed[]= /* fake name */
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),
	D3DVSD_REG(1, D3DVSDT_NORMPACKED3),
	D3DVSD_REG(2, D3DVSDT_NORMPACKED3),
	D3DVSD_REG(3, D3DVSDT_NORMPACKED3),
	D3DVSD_REG(4, D3DVSDT_FLOAT2),
	D3DVSD_STREAM(1),
	D3DVSD_REG(7, D3DVSDT_NORMPACKED3),
	D3DVSD_REG(8, D3DVSDT_NORMSHORT2),
	D3DVSD_END()
};

static unsigned long const vertex_declaration_environment_compressed[]= /* fake name */
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),
	D3DVSD_REG(1, D3DVSDT_NORMPACKED3),
	D3DVSD_REG(2, D3DVSDT_NORMPACKED3),
	D3DVSD_REG(3, D3DVSDT_NORMPACKED3),
	D3DVSD_REG(4, D3DVSDT_FLOAT2),
	D3DVSD_END()
};

/* ---------- public code */

boolean rasterizer_vertex_shaders_initialize(
	void)
{
	short vertex_shader_index;
	boolean success = TRUE;

	for (vertex_shader_index = 0; vertex_shader_index<NUMBER_OF_VERTEX_SHADERS; vertex_shader_index++)
	{
		vertex_shader_table[vertex_shader_index].declaration = NULL;
	}

	vertex_shader_table[_vertex_shader_screen].declaration = (unsigned long *)vertex_declaration_dynamic_screen;
	vertex_shader_table[_vertex_shader_screen2].declaration = (unsigned long *)vertex_declaration_dynamic_screen;
	vertex_shader_table[_vertex_shader_convolution].declaration = (unsigned long *)vertex_declaration_dynamic_screen;
	vertex_shader_table[_vertex_shader_detail_object_type0].declaration = (unsigned long *)vertex_declaration_detail_object;
	vertex_shader_table[_vertex_shader_effect].declaration = (unsigned long *)vertex_declaration_dynamic_unlit;
	vertex_shader_table[_vertex_shader_effect_multitexture].declaration = (unsigned long *)vertex_declaration_dynamic_unlit;
	vertex_shader_table[_vertex_shader_effect_multitexture_screenspace].declaration = (unsigned long *)vertex_declaration_dynamic_unlit;
	vertex_shader_table[_vertex_shader_lens_flare].declaration = (unsigned long *)vertex_declaration_dynamic_unlit;
	vertex_shader_table[_vertex_shader_debug].declaration = (unsigned long *)vertex_declaration_debug;
	vertex_shader_table[_vertex_shader_decal].declaration = (unsigned long *)vertex_declaration_decal;
	vertex_shader_table[_vertex_shader_effect_zsprite].declaration = (unsigned long *)vertex_declaration_unlit_zsprite;
	vertex_shader_table[_vertex_shader_environment_lightmap].declaration = (unsigned long *)vertex_declaration_environment_lightmap_compressed;
	vertex_shader_table[_vertex_shader_environment_diffuse_light].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_shadow].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_texture].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_specular_light].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_specular_spot_light].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_specular_lightmap].declaration = (unsigned long *)vertex_declaration_environment_lightmap_compressed;
	vertex_shader_table[_vertex_shader_environment_reflection_lightmap_mask].declaration = (unsigned long *)vertex_declaration_environment_lightmap_compressed;
	vertex_shader_table[_vertex_shader_environment_reflection_bumped].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_reflection_flat].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_reflection_radiosity].declaration = (unsigned long *)vertex_declaration_environment_lightmap_compressed;
	vertex_shader_table[_vertex_shader_environment_reflection_mirror].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_fog].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_fog_screen].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_model].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_detail_object_type1].declaration = (unsigned long *)vertex_declaration_detail_object;
	vertex_shader_table[_vertex_shader_model_fast].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_model_scenery].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_model_fogged].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_model_active_camouflage].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_model_shadow].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_model_zbuffer].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_model_fog_screen].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_generic].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_transparent_generic_screenspace].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_transparent_generic_reflection].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_transparent_generic_object_centered].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_transparent_generic_viewer_centered].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_transparent_generic_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_generic_lit_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_generic_screenspace_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_generic_reflection_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_generic_object_centered_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_generic_viewer_centered_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_glass_tint].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_transparent_glass_reflection_bumped].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_transparent_glass_reflection_flat].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_transparent_glass_reflection_mirror].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_transparent_glass_diffuse_light].declaration = (unsigned long *)vertex_declaration_environment_lightmap_compressed;
	vertex_shader_table[_vertex_shader_transparent_glass_tint_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_glass_reflection_bumped_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_glass_reflection_flat_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_glass_diffuse_light_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_water_opacity].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_transparent_water_reflection].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_transparent_water_opacity_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_water_reflection_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_meter].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_transparent_meter_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_transparent_plasma_m].declaration = (unsigned long *)vertex_declaration_model_compressed;
	vertex_shader_table[_vertex_shader_environment_vectors_debug0].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_vectors_debug1].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_vectors_debug2].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_vectors_debug3].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_vectors_debug4].declaration = (unsigned long *)vertex_declaration_environment_compressed;
	vertex_shader_table[_vertex_shader_environment_vectors_debug5].declaration = (unsigned long *)vertex_declaration_environment_compressed;

	for (vertex_shader_index = 0; vertex_shader_index<NUMBER_OF_VERTEX_SHADERS; vertex_shader_index++)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_vertex_shaders_initialize.c", 242, vertex_shader_table[vertex_shader_index].declaration);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_vertex_shaders_initialize.c", 243, vertex_shader_table[vertex_shader_index].code);

		D3DCALL(success, IDirect3DDevice8_CreateVertexShader(global_d3d_device, (DWORD*)vertex_shader_table[vertex_shader_index].declaration, (DWORD*)vertex_shader_table[vertex_shader_index].code, (DWORD*)&vertex_shader_table[vertex_shader_index].handle, 0));
	}

	if (!success)
	{
#ifdef DEBUG
		error(_error_silent, "### ERROR rasterizer_vertex_shaders_initialize failed");
#endif
	}

	return success;
}

void rasterizer_vertex_shaders_dispose(
	void)
{
	short vertex_shader_index;
	boolean success = TRUE;

	for (vertex_shader_index = 0; vertex_shader_index<NUMBER_OF_VERTEX_SHADERS; vertex_shader_index++)
	{
		D3DCALL(success, IDirect3DDevice8_DeleteVertexShader(global_d3d_device, (DWORD)vertex_shader_table[vertex_shader_index].handle));
	}

	if (!success)
	{
#ifdef DEBUG
		error(_error_silent, "### ERROR rasterizer_vertex_shaders_dispose failed");
#endif
	}

	return;
}
