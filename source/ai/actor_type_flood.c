/*
ACTOR_TYPE_FLOOD.C

symbols in this file:
00026810 0170:
	_flood_decide_action (0000)
00026980 0510:
	_actor_type_flood_desire_shamble (0000)
002463CC 0006:
	??_C@_05ONANONLM@flood?$AA@ (0000)
002463D4 0004:
	__real@41f80000 (0000)
002463D8 0032:
	??_C@_0DC@EGHABMGG@actor?9?$DOemotions?4crouch_switching@ (0000)
0024640C 0004:
	__real@bfb33333 (0000)
00246410 0004:
	__real@3fb33333 (0000)
00246414 0025:
	??_C@_0CF@GOCGFILI@c?3?2halo?2SOURCE?2ai?2actor_type_flo@ (0000)
002B6B64 0020:
	_actor_type_flood (0000)
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
#include "ai_debug.h"
#include "props.h"
#include "actor_types.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
