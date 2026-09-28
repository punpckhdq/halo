/*
DEAD_CAMERA.C

symbols in this file:
000740B0 0070:
	_code_000740b0 (0000)
00074120 00b0:
	_code_00074120 (0000)
000741D0 0120:
	_dead_camera_new (0000)
000742F0 04e0:
	_dead_camera_update (0000)
00256AE8 000c:
	_rdata_00256ae8 (0000)
00256AF4 0024:
	??_C@_0CE@LCNMEPOD@c?3?2halo?2SOURCE?2camera?2dead_camer@ (0000)
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
#include "ai.h"
#include "ai_constants.h"
#include "unit_definitions.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "ai_communication.h"
#include "unicode.h"
#include "units.h"
#include "index_resolution.h"
#include "game_engine.h"
#include "network_game_manager.h"
#include "players.h"
#include "director.h"
#include "observer.h"
#include "dead_camera.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
