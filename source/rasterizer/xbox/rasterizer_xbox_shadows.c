/*
RASTERIZER_XBOX_SHADOWS.C

symbols in this file:
00161950 01b0:
	_D3DDevice_SetRenderState (0000)
00161B00 0050:
	_D3DDevice_SetTextureStageState (0000)
00161B50 0220:
	_IDirect3DDevice8_SetRenderState@12 (0000)
00161D70 0010:
	__rasterizer_environment_shadows_begin (0000)
00161D80 0060:
	_IDirect3DDevice8_SetTextureStageState@16 (0000)
00161DE0 0010:
	_IDirect3DDevice8_SetVertexShaderConstant@16 (0000)
00161DF0 00a0:
	__rasterizer_environment_shadow_model_begin (0000)
00161E90 0010:
	__rasterizer_environment_shadow_model_end (0000)
00161EA0 0020:
	_IDirect3DDevice8_SetVertexData2f@16 (0000)
00161EC0 0010:
	_IDirect3DDevice8_SetVertexData2s@16 (0000)
00161ED0 0010:
	_IDirect3DDevice8_Begin@8 (0000)
00161EE0 0010:
	_IDirect3DDevice8_End@4 (0000)
00161EF0 0080:
	__rasterizer_environment_shadow_end (0000)
00161F70 0010:
	__rasterizer_environment_shadows_end (0000)
00161F80 0300:
	_rasterizer_shadow_convolve (0000)
00162280 03b0:
	__rasterizer_environment_shadow_begin (0000)
00162630 02b0:
	__rasterizer_environment_shadow_model_draw (0000)
001628E0 05c0:
	__rasterizer_environment_shadow_draw (0000)
002929E0 0039:
	??_C@_0DJ@KIPLBCDL@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
00292A1C 0027:
	??_C@_0CH@OGPNPLFF@?$CD?$CD?$CD?5WARNING?5empty?5shadow?5has?5bee@ (0000)
00292A44 001c:
	??_C@_0BM@OJNHCFOK@object_bounding_radius?$DO0?40f?$AA@ (0000)
00292A60 0037:
	??_C@_0DH@LMLPNHOO@shadow_color?9?$DOblue?5?$DO?$DN0?40f?5?$CG?$CG?5sha@ (0000)
00292A98 0037:
	??_C@_0DH@OODBCLKP@shadow_color?9?$DOgreen?$DO?$DN0?40f?5?$CG?$CG?5sha@ (0000)
00292AD0 0035:
	??_C@_0DF@OLKILINN@shadow_color?9?$DOred?5?$DO?$DN0?40f?5?$CG?$CG?5shad@ (0000)
00292B08 000d:
	??_C@_0N@NKBPFDCL@shadow_color?$AA@ (0000)
00292B18 000e:
	??_C@_0O@MDCFBAFA@shadow_matrix?$AA@ (0000)
0030CF84 0001:
	_data_0030cf84 (0000)
0046628C 004a:
	_bss_0046628c (0000)
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

/* ---------- private code */
