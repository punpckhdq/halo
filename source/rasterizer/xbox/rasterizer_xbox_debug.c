/*
RASTERIZER_XBOX_DEBUG.C
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

void rasterizer_debug_drawing_begin(
	boolean opaque,
	long z_bias)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_debug.c", 19, global_d3d_device);

	if (rasterizer_debug_options.draw_debug_geometry)
	{
		rasterizer_set_vertex_shader_permutation(0, _rasterizer_vertex_type_debug, 0);

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_NONE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, TRUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, rasterizer_debug_options.zbias);

		memset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
		pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
		pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_V0);

		if (opaque)
		{
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, FALSE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, TRUE);
		}
		else
		{
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, TRUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, FALSE);
			pixel_shader.PSFinalCombinerInputsEFG = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_V0 | PS_CHANNEL_ALPHA, 0);
		}

		rasterizer_set_pixel_shader(&pixel_shader);
	}

	return;
}

void rasterizer_debug_drawing_end(
	void)
{
	return;
}

void _rasterizer_debug_immediate_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_debug.c", 81, global_d3d_device);

	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_NONE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, FALSE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, TRUE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, TRUE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, rasterizer_debug_options.zbias);

	rasterizer_set_vertex_shader_permutation(0, _rasterizer_vertex_type_debug, 0);

	memset(&pixel_shader, 0, sizeof(pixel_shader));
	pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
	pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
	pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_V0);
	rasterizer_set_pixel_shader(&pixel_shader);

	return;
}

void _rasterizer_debug_immediate_line(
	real_point3d const *p0,
	real_point3d const *p1,
	real_rgb_color const *color0,
	real_rgb_color const *color1)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_debug.c", 116, p0 && p1 && color0);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_debug.c", 117, global_d3d_device);

	IDirect3DDevice8_Begin(global_d3d_device, D3DPT_LINELIST);
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_TEXCOORD0, color0->red, color0->green, color0->blue, 1.f);
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, p0->x, p0->y, p0->z, 1.f);

	if (color1)
	{
		IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_TEXCOORD0, color1->red, color1->green, color1->blue, 1.f);
	}

	IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, p1->x, p1->y, p1->z, 1.f);
	IDirect3DDevice8_End(global_d3d_device);

	return;
}

void _rasterizer_debug_immediate_triangle(
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	real_rgb_color const *color0,
	real_rgb_color const *color1,
	real_rgb_color const *color2)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_debug.c", 139, p0 && p1 && p2 && color0);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_debug.c", 140, global_d3d_device);

	IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLELIST);
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_TEXCOORD0, color0->red, color0->green, color0->blue, 1.f);
	IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, p0->x, p0->y, p0->z, 1.f);

	if (color1)
	{
		IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_TEXCOORD0, color1->red, color1->green, color1->blue, 1.f);
	}

	IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, p1->x, p1->y, p1->z, 1.f);

	if (color2)
	{
		IDirect3DDevice8_SetVertexData4f(global_d3d_device, D3DVSDE_TEXCOORD0, color2->red, color2->green, color2->blue, 1.f);
	}

	IDirect3DDevice8_SetVertexData4f(global_d3d_device, VSDE_VERTEX, p2->x, p2->y, p2->z, 1.f);
	IDirect3DDevice8_End(global_d3d_device);

	return;
}

void _rasterizer_debug_immediate_end(
	void)
{
	return;
}

void _rasterizer_debug_immediate_begin_screenspace(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_debug.c", 167, global_d3d_device);

	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_NONE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, FALSE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, FALSE);
	IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, 0);

	rasterizer_set_vertex_shader_permutation(4, _rasterizer_vertex_type_dynamic_screen, 0);

	{
		short viewport_width = global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0;
		short viewport_height = global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0;
		real vsh_constants__screenproj[5][4] =
		{
			{2.f/viewport_width, 0.f, 0.f, -1.f - 1.f/viewport_width},
			{0.f, -2.f/viewport_height, 0.f, 1.f/viewport_height + 1.f},
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
	pixel_shader.PSTextureModes = PS_TEXTUREMODES(PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE, PS_TEXTUREMODES_NONE);
	pixel_shader.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
	pixel_shader.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_ZERO, PS_REGISTER_V0);
	rasterizer_set_pixel_shader(&pixel_shader);

	return;
}

void _rasterizer_debug_immediate_line_screenspace(
	point2d const *p0,
	point2d const *p1,
	real_rgb_color const *color0,
	real_rgb_color const *color1)
{
	pixel32 pixel_color0;
	pixel32 pixel_color1;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_debug.c", 221, p0 && p1 && color0);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_debug.c", 222, global_d3d_device);

	pixel_color0 = real_rgb_color_to_pixel32(color0);

	if (color1)
	{
		pixel_color1 = real_rgb_color_to_pixel32(color1);
	}

	IDirect3DDevice8_Begin(global_d3d_device, D3DPT_LINESTRIP);
	IDirect3DDevice8_SetVertexDataColor(global_d3d_device, D3DVSDE_TEXCOORD0, pixel_color0);
	IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, p0->x, p0->y);

	if (color1)
	{
		IDirect3DDevice8_SetVertexDataColor(global_d3d_device, D3DVSDE_TEXCOORD0, pixel_color1);
	}

	IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, p1->x, p1->y);
	IDirect3DDevice8_End(global_d3d_device);

	return;
}

void _rasterizer_debug_immediate_linestrip_screenspace(
	point2d const *points,
	short point_count,
	real_rgb_color const *color)
{
	pixel32 pixel_color;
	short point_index;
	boolean success;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_debug.c", 249, points && color && point_count>1);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_debug.c", 250, global_d3d_device);

	pixel_color = real_rgb_color_to_pixel32(color);
	IDirect3DDevice8_Begin(global_d3d_device, D3DPT_LINESTRIP);
	IDirect3DDevice8_SetVertexDataColor(global_d3d_device, D3DVSDE_TEXCOORD0, pixel_color);
	success = TRUE;

	for (point_index = 0; point_index < point_count; point_index++)
	{
		D3DCALL(success, IDirect3DDevice8_SetVertexData2s(global_d3d_device, VSDE_VERTEX, points[point_index].x, points[point_index].y));
	}

	D3DCALL(success, IDirect3DDevice8_End(global_d3d_device));

	return;
}

void _rasterizer_debug_immediate_end_screenspace(
	void)
{
	return;
}

/* ---------- private code */
