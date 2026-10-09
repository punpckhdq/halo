/*
RASTERIZER_XBOX_WATER.C

symbols in this file:
001688F0 01b0:
	_D3DDevice_SetRenderState (0000)
00168AA0 0050:
	_D3DDevice_SetTextureStageState (0000)
00168AF0 0020:
	_rasterizer_water_set_visibility_for_frame (0000)
00168B10 0010:
	_rasterizer_water_set_visibility_for_window (0000)
00168B20 0010:
	_rasterizer_water_get_visibility_for_window (0000)
00168B30 0220:
	_IDirect3DDevice8_SetRenderState@12 (0000)
00168D50 0060:
	_IDirect3DDevice8_SetTextureStageState@16 (0000)
00168DB0 0010:
	_IDirect3DDevice8_SetVertexShaderConstant@16 (0000)
00168DC0 0020:
	_IDirect3DDevice8_SetVertexData2f@16 (0000)
00168DE0 0010:
	_IDirect3DDevice8_SetVertexData2s@16 (0000)
00168DF0 0010:
	_IDirect3DDevice8_Begin@8 (0000)
00168E00 0010:
	_IDirect3DDevice8_End@4 (0000)
00168E10 0820:
	_rasterizer_water_build_bumpmap (0000)
00169630 08c0:
	_rasterizer_water_draw (0000)
0029CBA0 0030:
	??_C@_0DA@GMAFCPCK@?$CD?$CD?$CD?5ERROR?5rasterizer_water_build@ (0000)
0029CBD0 0043:
	??_C@_0ED@IHGINMLN@ripples?$FL2?$FN?4contibution_factor?5?$CL?5@ (0000)
0029CC18 0043:
	??_C@_0ED@IIOIOBNN@ripples?$FL0?$FN?4contibution_factor?5?$CL?5@ (0000)
0029CC5C 0024:
	??_C@_0CE@CFGOBBBI@ripples?$FLripple_index?$FN?4map_repeat@ (0000)
0029CC80 0037:
	??_C@_0DH@OMMLHAPF@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
004662E8 0002:
	_bss_004662e8 (0000)
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
