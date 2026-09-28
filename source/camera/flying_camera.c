/*
FLYING_CAMERA.C

symbols in this file:
000779A0 0020:
	_flying_camera_new (0000)
000779C0 0050:
	_flying_camera_new_from_point_and_vector (0000)
00077A10 04b0:
	_flying_camera_update (0000)
00256E34 0026:
	??_C@_0CG@FECMEHCO@c?3?2halo?2SOURCE?2camera?2flying_cam@ (0000)
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
#include "bsp3d.h"
#include "bsp2d.h"
#include "collision_bsp_definitions.h"
#include "collision_bsp.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "unicode.h"
#include "index_resolution.h"
#include "game_engine.h"
#include "network_game_manager.h"
#include "players.h"
#include "collision_usage.h"
#include "collision_features.h"
#include "collisions.h"
#include "director.h"
#include "observer.h"
#include "flying_camera.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
