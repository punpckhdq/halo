/*
STRUCTURE_RUNTIME_DECALS.C

symbols in this file:
00185AE0 0040:
	_structure_decals_initialize (0000)
00185B20 0030:
	_structure_decals_initialize_for_new_map (0000)
00185B50 0030:
	_structure_decals_reconnect_to_structure_bsp (0000)
00185B80 0070:
	_structure_decals_disconnect_from_structure_bsp (0000)
00185BF0 0010:
	_structure_decals_dispose_from_old_map (0000)
00185C00 0010:
	_structure_decals_dispose (0000)
00185C10 0210:
	_structure_decals_update (0000)
002A1A8C 0019:
	??_C@_0BJ@EHFGPMAL@structure_decals_globals?$AA@ (0000)
002A1AA8 0035:
	??_C@_0DF@EMFPFLCM@c?3?2halo?2SOURCE?2structures?2struct@ (0000)
002A1AE0 0011:
	??_C@_0BB@PDFGKABJ@structure?5decals?$AA@ (0000)
004C0CE8 0004:
	_bss_004c0ce8 (0000)
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
#include "shader_definitions.h"
#include "bsp3d.h"
#include "bsp2d.h"
#include "collision_bsp_definitions.h"
#include "collision_bsp.h"
#include "leaf_map.h"
#include "render_cameras.h"
#include "structures.h"
#include "render.h"
#include "structure_bsp_definitions.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "unicode.h"
#include "index_resolution.h"
#include "game_engine.h"
#include "network_game_manager.h"
#include "rasterizer.h"
#include "players.h"
#include "lruv_cache.h"
#include "memory_pool.h"
#include "game_state.h"
#include "collision_usage.h"
#include "collision_features.h"
#include "collisions.h"
#include "decal_definitions.h"
#include "decals.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
