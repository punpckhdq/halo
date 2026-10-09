/*
RASTERIZER_XBOX_PLASMA_ENERGY.C

symbols in this file:
0015E2B0 01b0:
	_D3DDevice_SetRenderState (0000)
0015E460 0050:
	_D3DDevice_SetTextureStageState (0000)
0015E4B0 0220:
	_IDirect3DDevice8_SetRenderState@12 (0000)
0015E6D0 0060:
	_IDirect3DDevice8_SetTextureStageState@16 (0000)
0015E730 0010:
	_IDirect3DDevice8_SetVertexShaderConstant@16 (0000)
0015E740 0590:
	_rasterizer_plasma_energy_draw (0000)
00291F10 0004:
	__real@3a03126f (0000)
00291F14 0033:
	??_C@_0DD@OEJHPIIG@plasma?9?$DOsecondary_noise_map_anim@ (0000)
00291F48 0031:
	??_C@_0DB@CGFOFGKP@plasma?9?$DOprimary_noise_map_animat@ (0000)
00291F7C 003f:
	??_C@_0DP@HOMIPMOO@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
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
