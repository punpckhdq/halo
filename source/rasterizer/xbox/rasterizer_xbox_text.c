/*
RASTERIZER_XBOX_TEXT.C

symbols in this file:
00162EA0 01b0:
	_D3DDevice_SetRenderState (0000)
00163050 0050:
	_D3DDevice_SetTextureStageState (0000)
001630A0 0220:
	_IDirect3DDevice8_SetRenderState@12 (0000)
001632C0 0060:
	_IDirect3DDevice8_SetTextureStageState@16 (0000)
00163320 0010:
	_IDirect3DDevice8_SetVertexShaderConstant@16 (0000)
00163330 0010:
	_rasterizer_text_end (0000)
00163340 0020:
	_IDirect3DDevice8_SetVertexData2f@16 (0000)
00163360 0010:
	_IDirect3DDevice8_SetVertexDataColor@12 (0000)
00163370 0010:
	_IDirect3DDevice8_Begin@8 (0000)
00163380 0010:
	_IDirect3DDevice8_End@4 (0000)
00163390 0690:
	_rasterizer_text_begin (0000)
00163A20 0120:
	_rasterizer_text_draw_character (0000)
00292B28 0036:
	??_C@_0DG@NLGEIAMP@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
00292B60 0030:
	??_C@_0DA@FEBHLDDN@?$CD?$CD?$CD?5ERROR?5rasterizer_text_draw_c@ (0000)
00292B90 0087:
	??_C@_0IH@NAGGOICD@IDirect3DDevice8_SetVertexData2f@ (0000)
00292C18 007d:
	??_C@_0HN@NMADEKEL@IDirect3DDevice8_SetVertexData2f@ (0000)
00292C98 0058:
	??_C@_0FI@KKLBHBJN@IDirect3DDevice8_SetVertexDataCo@ (0000)
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
