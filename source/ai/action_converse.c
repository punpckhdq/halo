/*
ACTION_CONVERSE.C

symbols in this file:
000028A0 00b0:
	_action_converse_setup (0000)
00002950 0010:
	_action_converse_begin (0000)
00002960 0100:
	_action_converse_perform (0000)
00002A60 0020:
	_action_converse_update (0000)
00002A80 0090:
	_action_converse_control (0000)
00002B10 0030:
	_action_converse_replace_prop (0000)
00002B40 0080:
	_actor_conversation_control (0000)
00002BC0 0030:
	_actor_conversation_end (0000)
00002BF0 0030:
	_action_converse_end (0000)
0024305C 0024:
	??_C@_0CE@NNMIMHKJ@c?3?2halo?2SOURCE?2ai?2action_convers@ (0000)
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

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
