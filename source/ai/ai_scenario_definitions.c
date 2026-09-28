/*
AI_SCENARIO_DEFINITIONS.C

symbols in this file:
00042490 0060:
	_scenario_get_encounter_by_name (0000)
000424F0 0060:
	_encounter_definition_get_squad_by_name (0000)
00042550 0060:
	_encounter_definition_get_platoon_by_name (0000)
000425B0 00e0:
	_choose_random_array_element (0000)
0024BDF8 0012:
	??_C@_0BC@MAOKEIKD@guard_at_position?$AA@ (0000)
0024BE0C 000c:
	??_C@_0M@CJJPMLKE@move_random?$AA@ (0000)
0024BE18 0011:
	??_C@_0BB@OJDOAOPK@move_loop_random?$AA@ (0000)
0024BE2C 0019:
	??_C@_0BJ@IJBEFDLI@move_loop_back_and_forth?$AA@ (0000)
0024BE48 000a:
	??_C@_09KGOMJPLJ@move_loop?$AA@ (0000)
0024BE54 000c:
	??_C@_0M@EEPMEDCG@move_repeat?$AA@ (0000)
002B7544 0030:
	_global_ai_default_state_names (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "game.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "model_definitions.h"
#include "model_animation_definitions.h"
#include "object_definitions.h"
#include "object_types.h"
#include "objects.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "bsp3d.h"
#include "bsp2d.h"
#include "collision_bsp_definitions.h"
#include "render_cameras.h"
#include "rasterizer_geometry.h"
#include "leaf_map.h"
#include "structures.h"
#include "render.h"
#include "structure_bsp_definitions.h"
#include "scenario.h"
#include "models.h"
#include "damage_resistances.h"
#include "shader_definitions.h"
#include "collision_bsp.h"
#include "ai.h"
#include "ai_constants.h"
#include "unit_definitions.h"
#include "ai_communication.h"
#include "units.h"
#include "path.h"
#include "actor_definitions.h"
#include "actions.h"
#include "actors.h"
#include "encounters.h"
#include "actor_types.h"
#include "sound_definitions.h"
#include "ai_script.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
