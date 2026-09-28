/*
ACTION_WAIT.C

symbols in this file:
0000A4B0 0010:
	_action_wait_begin (0000)
0000A4C0 0180:
	_action_wait_perform (0000)
0000A640 00b0:
	_action_wait_control (0000)
0000A6F0 00b0:
	_action_wait_setup (0000)
0000A7A0 00e0:
	_action_wait_update (0000)
00243B18 0004:
	__real@41000000 (0000)
00243B1C 0020:
	??_C@_0CA@JCBCAIHB@c?3?2halo?2SOURCE?2ai?2action_wait?4c?$AA@ (0000)
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
