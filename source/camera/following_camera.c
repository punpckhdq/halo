/*
FOLLOWING_CAMERA.C

symbols in this file:
00077EC0 0060:
	_following_camera_new (0000)
00077F20 0080:
	_code_00077f20 (0000)
00077FA0 0010:
	_arcsine (0000)
00077FB0 00e0:
	_uniform_cubic_spline (0000)
00078090 0090:
	_uniform_cubic_spline_vector3d (0000)
00078120 0150:
	_code_00078120 (0000)
00078270 00d0:
	_following_camera_deterministic (0000)
00078340 0630:
	_following_camera_update (0000)
00256E5C 0029:
	??_C@_0CJ@CPNCEFIH@c?3?2halo?2SOURCE?2camera?2following_@ (0000)
00256E88 001c:
	??_C@_0BM@HKHJJNLK@t?5?$DO?$DN?5t0?5?$CG?$CG?5t?5?$DM?$DN?5t0?5?$CL?53?40f?$CKh?$AA@ (0000)
00256EA4 0009:
	??_C@_08OMLDILI@h?5?$DO?50?40f?$AA@ (0000)
00256EB0 0028:
	??_C@_0CI@JCGGGL@camera_track?9?$DOcontrol_points?4cou@ (0000)
00256ED8 0004:
	__real@3ea2f983 (0000)
00256EE0 0053:
	??_C@_0FD@PCPIEKIH@magnitude3d?$CI?$CGresult?9?$DOforward?$CJ?5?$DO?5@ (0000)
00256F34 0004:
	__real@3f800347 (0000)
00256F38 0004:
	__real@3f7ff972 (0000)
002DCC5C 0010:
	_following_camera_zoom_levels (0000)
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
#include "camera_track_definitions.h"
#include "following_camera.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
