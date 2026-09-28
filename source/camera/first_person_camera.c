/*
FIRST_PERSON_CAMERA.C

symbols in this file:
000772B0 0040:
	_first_person_camera_new (0000)
000772F0 00d0:
	_first_person_camera_deterministic (0000)
000773C0 04f0:
	_code_000773c0 (0000)
000778B0 0030:
	_first_person_camera_fake (0000)
000778E0 00c0:
	_first_person_camera_update (0000)
00256DBC 002c:
	??_C@_0CM@BADBGAHA@c?3?2halo?2SOURCE?2camera?2first_pers@ (0000)
00256DE8 0010:
	??_C@_0BA@DGFIDHMB@primary?5trigger?$AA@ (0000)
00256DF8 0039:
	??_C@_0DJ@GGFKGPIO@valid_real_vector3d_axes2?$CI?$CGresul@ (0000)
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
#include "console.h"
#include "physics_variables.h"
#include "vehicle_definitions.h"
#include "vehicles.h"
#include "director.h"
#include "observer.h"
#include "player_effects.h"
#include "first_person_camera.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
