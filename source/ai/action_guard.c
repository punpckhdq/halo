/*
ACTION_GUARD.C

symbols in this file:
00003E80 0080:
	_action_guard_setup_current_position (0000)
00003F00 00d0:
	_action_guard_setup_find_position (0000)
00003FD0 0160:
	_action_guard_setup_postcombat (0000)
00004130 0040:
	_action_guard_begin (0000)
00004170 0040:
	_action_guard_end (0000)
000041B0 0140:
	_code_000041b0 (0000)
000042F0 01d0:
	_action_guard_update (0000)
000044C0 0080:
	_action_guard_flush_position_indices (0000)
00004540 0030:
	_action_guard_flush_structure_indices (0000)
00004570 00a0:
	_action_guard_modify_color (0000)
00004610 0050:
	_action_guard_replace_prop (0000)
00004660 01c0:
	_action_guard_setup_from_combat_transition (0000)
00004820 01c0:
	_action_guard_setup_from_fleeing (0000)
000049E0 01c0:
	_action_guard_perform (0000)
00004BA0 03d0:
	_action_guard_control (0000)
002431D8 0021:
	??_C@_0CB@GJJCDDDH@c?3?2halo?2SOURCE?2ai?2action_guard?4c@ (0000)
002431FC 0018:
	??_C@_0BI@NGADMAPA@state_data?9?$DOpost_combat?$AA@ (0000)
00243214 0034:
	??_C@_0DE@PILBLNDH@actor?9?$DOstimuli?4combat_transition@ (0000)
00243248 0021:
	??_C@_0CB@CNNLKLII@actor?9?$DOstimuli?4combat_transition@ (0000)
0024326C 0004:
	__real@41100000 (0000)
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
#include "ai.h"
#include "ai_constants.h"
#include "unit_definitions.h"
#include "ai_communication.h"
#include "units.h"
#include "path.h"
#include "actor_definitions.h"
#include "actions.h"
#include "actors.h"
#include "props.h"
#include "actor_types.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
