/*
ANTENNA.C

symbols in this file:
00120710 0030:
	_antennas_initialize (0000)
00120740 0010:
	_antennas_initialize_for_new_map (0000)
00120750 0010:
	_antennas_dispose_from_old_map (0000)
00120760 0020:
	_antennas_dispose (0000)
00120780 0200:
	_antenna_new (0000)
00120980 0020:
	_antenna_delete (0000)
001209A0 0130:
	_code_001209a0 (0000)
00120AD0 0170:
	_code_00120ad0 (0000)
00120C40 0310:
	_code_00120c40 (0000)
00120F50 0090:
	_antenna_render (0000)
00120FE0 00b0:
	_antennas_update (0000)
00288EEC 0022:
	??_C@_0CC@JBAGLFPJ@couldn?8t?5allocate?5antenna?5global@ (0000)
00288F10 0008:
	??_C@_07HJCCDMBN@antenna?$AA@ (0000)
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
#include "rasterizer.h"
#include "lruv_cache.h"
#include "memory_pool.h"
#include "game_state.h"
#include "point_physics.h"
#include "antenna.h"
#include "antenna_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
