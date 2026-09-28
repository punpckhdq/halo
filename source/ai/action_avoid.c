/*
ACTION_AVOID.C

symbols in this file:
00000EC0 0040:
	_action_avoid_setup (0000)
00000F00 0010:
	_action_avoid_begin (0000)
00000F10 0010:
	_action_avoid_end (0000)
00000F20 00c0:
	_action_avoid_perform (0000)
00000FE0 0010:
	_action_avoid_update (0000)
00000FF0 0090:
	_action_avoid_control (0000)
00242F3C 0021:
	??_C@_0CB@LDCPOHMO@c?3?2halo?2SOURCE?2ai?2action_avoid?4c@ (0000)
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
