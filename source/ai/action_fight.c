/*
ACTION_FIGHT.C

symbols in this file:
00002C20 0040:
	_action_fight_setup (0000)
00002C60 0010:
	_action_fight_begin (0000)
00002C70 0010:
	_action_fight_end (0000)
00002C80 0070:
	_action_fight_update (0000)
00002CF0 0080:
	_action_fight_control (0000)
00002D70 03d0:
	_action_fight_perform (0000)
00243080 0021:
	??_C@_0CB@KJFGPOLC@c?3?2halo?2SOURCE?2ai?2action_fight?4c@ (0000)
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
#include "physics_variables.h"
#include "vehicle_definitions.h"
#include "vehicles.h"
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
