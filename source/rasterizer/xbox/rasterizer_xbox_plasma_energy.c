/*
RASTERIZER_XBOX_PLASMA_ENERGY.C

symbols in this file:
0015E2B0 01b0:
	_code_0015e2b0 (0000)
0015E460 0050:
	_code_0015e460 (0000)
0015E4B0 0220:
	_code_0015e4b0 (0000)
0015E6D0 0060:
	_code_0015e6d0 (0000)
0015E730 0010:
	_code_0015e730 (0000)
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
#include "game.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "object_definitions.h"
#include "object_types.h"
#include "objects.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "rasterizer_geometry.h"
#include "model_animation_definitions.h"
#include "model_definitions.h"
#include "models.h"
#include "damage_resistances.h"
#include "bsp3d.h"
#include "shader_definitions.h"
#include "bsp2d.h"
#include "collision_bsp_definitions.h"
#include "collision_bsp.h"
#include "leaf_map.h"
#include "render_cameras.h"
#include "structures.h"
#include "render.h"
#include "structure_bsp_definitions.h"
#include "bitmap_macros.h"
#include "rasterizer.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "cheats.h"
#include "rasterizer/xbox/rasterizer_xbox.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
