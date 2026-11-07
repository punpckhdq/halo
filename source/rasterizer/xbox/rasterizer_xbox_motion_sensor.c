/*
RASTERIZER_XBOX_MOTION_SENSOR.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer_xbox.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "texture_cache.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static boolean rasterizer_motion_sensor_begin_said_to_draw;

/* ---------- public code */

void _rasterizer_hud_motion_sensor_blip_begin(
	void)
{
	struct bitmap_data *blip_bitmap = bitmap_group_try_and_get_bitmap(interface_get_tag_index(_interface_bitmap_motion_blip), 0);
	struct bitmap_data *custom_blip_bitmap = bitmap_group_try_and_get_bitmap(interface_get_tag_index(_interface_bitmap_iface_map1), 0);

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_motion_sensor.c", 27, global_d3d_device);

	rasterizer_motion_sensor_begin_said_to_draw = FALSE;

	if (rasterizer_debug_options.draw_hud_motion_sensor &&
		_texture_cache_bitmap_get_hardware_format(blip_bitmap, FALSE, TRUE) &&
		_texture_cache_bitmap_get_hardware_format(custom_blip_bitmap, FALSE, TRUE))
	{
		rasterizer_motion_sensor_begin_said_to_draw = TRUE;

		rasterizer_set_target(_rasterizer_target_sun_glow_primary, 0, 0, TRUE, FALSE);
		rasterizer_set_texture_bitmap_data(0, blip_bitmap);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
		SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ONE);
		SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ONE);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		rasterizer_set_vertex_shader_permutation(_vertex_shader_screen,_rasterizer_vertex_type_dynamic_screen, 0);

		{
			real vsh_constants__screenproj[5][4] =
			{
				{1.f, 0.f, 0.f, 0.f},
				{0.f, 1.f, 0.f, 0.f},
				{0.f, 0.f, 1.f, 0.f},
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
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_V0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		rasterizer_set_pixel_shader(&pixel_shader);
	}

	return;
}

void _rasterizer_hud_motion_sensor_blip_draw(
	real_point2d const *blip_position,
	real fade,
	real radius,
	real_rgb_color const *blip_color,
	boolean custom)
{
	struct bitmap_data *blip_bitmap = bitmap_group_try_and_get_bitmap(interface_get_tag_index(_interface_bitmap_motion_blip), 0);
	struct bitmap_data *custom_blip_bitmap = bitmap_group_try_and_get_bitmap(interface_get_tag_index(_interface_bitmap_iface_map1), 0);

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_motion_sensor.c", 109, global_d3d_device);

	rasterizer_set_texture_bitmap_data(0, custom ? custom_blip_bitmap : blip_bitmap);

	if (rasterizer_debug_options.draw_hud_motion_sensor && rasterizer_motion_sensor_begin_said_to_draw)
	{
		real_point2d blip_draw_position;
		real const pixel_size = 1.f / 64.f;
		real blip_size = 4.f * pixel_size * radius;

		blip_draw_position.x = -blip_position->x / 32.f;
		blip_draw_position.y = -blip_position->y / 32.f;

		IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
		IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_TEXCOORD0, fade * blip_color->red, fade * blip_color->green, fade * blip_color->blue, 1.f);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 0);
		IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, blip_draw_position.x - blip_size, blip_draw_position.y + blip_size);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 0);
		IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, blip_draw_position.x + blip_size, blip_draw_position.y + blip_size);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 1);
		IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, blip_draw_position.x + blip_size, blip_draw_position.y - blip_size);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 1);
		IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, blip_draw_position.x - blip_size, blip_draw_position.y - blip_size);
		IDirect3DDevice8_End(global_d3d_device);
	}

	return;
}

void _rasterizer_hud_motion_sensor_blip_end(
	real_point2d const *center_point,
	real theta)
{
	boolean success = TRUE;
	struct bitmap_data *sweep_bitmap = bitmap_group_try_and_get_bitmap(interface_get_tag_index(_interface_bitmap_motion_sweep), 0);
	struct bitmap_data *sweep_mask_bitmap = bitmap_group_try_and_get_bitmap(interface_get_tag_index(_interface_bitmap_motion_sweep_mask), 0);

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_motion_sensor.c", 156, global_d3d_device);

	if (rasterizer_debug_options.draw_hud_motion_sensor &&
		rasterizer_motion_sensor_begin_said_to_draw &&
		_texture_cache_bitmap_get_hardware_format(sweep_bitmap, FALSE, TRUE) &&
		_texture_cache_bitmap_get_hardware_format(sweep_mask_bitmap, FALSE, TRUE))
	{
		real size;
		real const pixel_size = 1.f / 64.f;
		real const border_size = 1.f / 32.f;
		real minimum = pixel_size - (border_size + 1.f);
		real maximum = border_size + 1.f + pixel_size;

		rasterizer_set_texture_bitmap_data(0, sweep_bitmap);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
		SetTextureStageStateSmart(0, D3DTSS_BORDERCOLOR, D3DCOLOR_ARGB(70, 0, 0, 0));
		SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ONE);
		SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		rasterizer_set_vertex_shader_permutation(_vertex_shader_screen,_rasterizer_vertex_type_dynamic_screen, 0);

		{
			real vsh_constants__screenproj[5][4] =
			{
				{1.f, 0.f, 0.f, 0.f},
				{0.f, 1.f, 0.f, 0.f},
				{0.f, 0.f, 1.f, 0.f},
				{0.f, 0.f, 0.f, 1.f},
				{1.f, 1.f, 0.f, 1.f}
			};

			D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__SCREENPROJ_OFFSET,
				vsh_constants__screenproj,
				VSH_CONSTANTS__SCREENPROJ_COUNT));
		}

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
		pixel_shader.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0, PS_REGISTER_V0, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0 | PS_CHANNEL_ALPHA, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, PS_REGISTER_ZERO, PS_REGISTER_ZERO);
		pixel_shader.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, PS_REGISTER_DISCARD, PS_REGISTER_DISCARD, 0);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_R0 | PS_CHANNEL_ALPHA, 0);
		rasterizer_set_pixel_shader(&pixel_shader);

		D3DCALL(success, IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN));
		D3DCALL(success, IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_TEXCOORD0, 0.4588f, 0.7294f, 1.f, 1.f));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, theta*0.5f + 0.5f, 0.5f - theta*0.5f));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, minimum, maximum));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, 0.5f - theta*0.5f, 0.5f - theta*0.5f));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, maximum, maximum));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, 0.5f - theta*0.5f, theta*0.5f + 0.5f));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, maximum, minimum));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, theta*0.5f + 0.5f, theta*0.5f + 0.5f));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, minimum, minimum));
		D3DCALL(success, IDirect3DDevice8_End(global_d3d_device));

		rasterizer_set_texture_bitmap_data(0, sweep_mask_bitmap);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, 0);
		rasterizer_set_pixel_shader(&pixel_shader);

		D3DCALL(success, IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN));
		D3DCALL(success, IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_TEXCOORD0, 0.4f, 0.8f, 0.4f, 1.f));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, 1.f, 0.f));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, minimum, maximum));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, 0.f, 0.f));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, maximum, maximum));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, 0.f, 1.f));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, maximum, minimum));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, 1.f, 1.f));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, minimum, minimum));
		D3DCALL(success, IDirect3DDevice8_End(global_d3d_device));

		rasterizer_set_target(global_window_parameters.rasterizer_target, 0, 0, FALSE, TRUE);
		rasterizer_set_target_as_texture(0, _rasterizer_target_sun_glow_primary, 0);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		SetTextureStageStateSmart(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		SetTextureStageStateSmart(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);
		SetTextureStageStateSmart(0, D3DTSS_BORDERCOLOR, 0);

		SetRenderStateSmart(D3DRS_CULLMODE, D3DCULL_CCW);
		SetRenderStateSmart(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ONE);
		SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZBIAS, 0);

		rasterizer_set_vertex_shader_permutation(_vertex_shader_screen,_rasterizer_vertex_type_dynamic_screen, 0);

		{
			short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
			short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;
			real vsh_constants__screenproj[5][4] =
			{
				{2.f / viewport_width, 0.f, 0.f, -1.f - 1.f / viewport_width},
				{0.f, -2.f / viewport_height, 0.f, 1.f / viewport_height + 1.f},
				{0.f, 0.f, 0.f, 0.5f},
				{0.f, 0.f, 0.f, 1.f},
				{1.f, 1.f, 0.f, 1.f}
			};

			D3DCALL(success, IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device,
				VSH_CONSTANTS__SCREENPROJ_OFFSET,
				vsh_constants__screenproj,
				VSH_CONSTANTS__SCREENPROJ_COUNT));
		}

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0);
		pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_T0 | PS_CHANNEL_ALPHA, 0);
		rasterizer_set_pixel_shader(&pixel_shader);

		size = (local_player_count() > 1) ? 32.f : 42.f;

		D3DCALL(success, IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 0));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, center_point->x - size, center_point->y - size));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 0));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, center_point->x + size, center_point->y - size));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 1, 1));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, center_point->x + size, center_point->y + size));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 1));
		D3DCALL(success, IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, center_point->x - size, center_point->y + size));
		D3DCALL(success, IDirect3DDevice8_End(global_d3d_device));
	}
	else if (rasterizer_debug_options.draw_hud_motion_sensor && rasterizer_motion_sensor_begin_said_to_draw)
	{
		rasterizer_set_target(global_window_parameters.rasterizer_target, 0, 0, FALSE, TRUE);
	}

	return;
}

/* ---------- private code */
