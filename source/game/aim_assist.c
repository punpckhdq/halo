/*
AIM_ASSIST.C

symbols in this file:
00093C00 0050:
	_compute_attenuation (0000)
00093C50 0030:
	_compute_composite_attenuation (0000)
00093C80 00f0:
	_code_00093c80 (0000)
00093D70 00a0:
	_code_00093d70 (0000)
00093E10 0010:
	_reciprocal_square_root (0000)
00093E20 0060:
	_limit3d (0000)
00093E80 0020:
	_set_real_euler_angles2d (0000)
00093EA0 00f0:
	_aim_assist_clear_line_of_sight (0000)
00093F90 01a0:
	_code_00093f90 (0000)
00094130 01a0:
	_aim_assist_compute_target (0000)
000942D0 0110:
	_autoaim_compute_target (0000)
000943E0 0190:
	_code_000943e0 (0000)
00094570 0130:
	_code_00094570 (0000)
000946A0 0100:
	_aim_assist (0000)
000947A0 0340:
	_player_aim_projectile (0000)
00094AE0 0170:
	_local_player_aim_assist (0000)
0025AC3C 0021:
	??_C@_0CB@CDDNCJJI@c?3?2halo?2SOURCE?2game?2aim_assist?4c@ (0000)
0025AC60 0004:
	__real@43000000 (0000)
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
#include "leaf_map.h"
#include "render_cameras.h"
#include "structure_bsp_definitions.h"
#include "render.h"
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
#include "path.h"
#include "actor_definitions.h"
#include "bungie_net/network/transport.h"
#include "network_messages.h"
#include "collision_usage.h"
#include "collision_features.h"
#include "collisions.h"
#include "item_definitions.h"
#include "items.h"
#include "aim_assist.h"
#include "meter_definitions.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"
#include "director.h"
#include "biped_definitions.h"
#include "weapons.h"
#include "bipeds.h"
#include "structures.h"
#include "observer.h"
#include "physics_constants.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
