/*
ORBITING_CAMERA.C

symbols in this file:
0007B580 0020:
	_orbiting_camera_new (0000)
0007B5A0 0470:
	_orbiting_camera_update (0000)
0025724C 0014:
	_rdata_0025724c (0000)
00257260 0028:
	??_C@_0CI@JLFPHGKF@c?3?2halo?2SOURCE?2camera?2orbiting_c@ (0000)
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
#include "orbiting_camera.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
