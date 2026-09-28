/*
ACTION_SEARCH.C

symbols in this file:
00007D60 0080:
	_action_search_setup_target (0000)
00007DE0 0100:
	_action_search_setup_pursuit (0000)
00007EE0 0070:
	_action_search_setup_undirected (0000)
00007F50 0180:
	_action_search_update (0000)
000080D0 0030:
	_action_search_flush_position_indices (0000)
00008100 0030:
	_action_search_flush_structure_indices (0000)
00008130 0150:
	_action_search_control (0000)
00008280 0090:
	_action_search_begin (0000)
00008310 0350:
	_action_search_perform (0000)
00243944 0022:
	??_C@_0CC@PIOOIGM@c?3?2halo?2SOURCE?2ai?2action_search?4@ (0000)
00243968 0004:
	__real@3f23d70b (0000)
0024396C 0004:
	__real@40c80000 (0000)
00243970 0004:
	__real@3efae147 (0000)
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
#include "scenario.h"
#include "rasterizer_geometry.h"
#include "models.h"
#include "damage_resistances.h"
#include "shader_definitions.h"
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
#include "props.h"
#include "actor_types.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
