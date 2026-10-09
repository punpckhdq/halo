/*
RASTERIZER_XBOX_MOTION_SENSOR.C

symbols in this file:
0015D220 01b0:
	_D3DDevice_SetRenderState (0000)
0015D3D0 0050:
	_D3DDevice_SetTextureStageState (0000)
0015D420 0220:
	_IDirect3DDevice8_SetRenderState@12 (0000)
0015D640 0060:
	_IDirect3DDevice8_SetTextureStageState@16 (0000)
0015D6A0 0010:
	_IDirect3DDevice8_SetVertexShaderConstant@16 (0000)
0015D6B0 0020:
	_IDirect3DDevice8_SetVertexData2f@16 (0000)
0015D6D0 0030:
	_IDirect3DDevice8_SetVertexData4f@24 (0000)
0015D700 0010:
	_IDirect3DDevice8_SetVertexData2s@16 (0000)
0015D710 0010:
	_IDirect3DDevice8_Begin@8 (0000)
0015D720 0010:
	_IDirect3DDevice8_End@4 (0000)
0015D730 0280:
	__rasterizer_hud_motion_sensor_blip_begin (0000)
0015D9B0 0180:
	__rasterizer_hud_motion_sensor_blip_draw (0000)
0015DB30 0780:
	__rasterizer_hud_motion_sensor_blip_end (0000)
00291ECC 003f:
	??_C@_0DP@DPMIHOMG@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
00291F0C 0004:
	__real@bd000000 (0000)
00465E27 0001:
	_bss_00465e27 (0000)
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

/* ---------- public code */

/* ---------- private code */
