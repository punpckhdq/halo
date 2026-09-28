/*
RASTERIZER_XBOX_MOTION_SENSOR.C

symbols in this file:
0015D220 01b0:
	_code_0015d220 (0000)
0015D3D0 0050:
	_code_0015d3d0 (0000)
0015D420 0220:
	_code_0015d420 (0000)
0015D640 0060:
	_code_0015d640 (0000)
0015D6A0 0010:
	_code_0015d6a0 (0000)
0015D6B0 0020:
	_code_0015d6b0 (0000)
0015D6D0 0030:
	_code_0015d6d0 (0000)
0015D700 0010:
	_code_0015d700 (0000)
0015D710 0010:
	_code_0015d710 (0000)
0015D720 0010:
	_code_0015d720 (0000)
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
#include "cheats.h"
#include "game.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "object_definitions.h"
#include "rasterizer_geometry.h"
#include "model_animation_definitions.h"
#include "model_definitions.h"
#include "models.h"
#include "damage_resistances.h"
#include "shader_definitions.h"
#include "objects.h"
#include "bsp3d.h"
#include "bsp2d.h"
#include "collision_bsp_definitions.h"
#include "collision_bsp.h"
#include "leaf_map.h"
#include "render_cameras.h"
#include "structure_bsp_definitions.h"
#include "render.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "unicode.h"
#include "index_resolution.h"
#include "game_engine.h"
#include "network_game_manager.h"
#include "rasterizer.h"
#include "players.h"
#include "bitmap_macros.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "texture_cache.h"
#include "rasterizer/xbox/rasterizer_xbox.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
