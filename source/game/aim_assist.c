/*
AIM_ASSIST.C

symbols in this file:
00093C00 0050:
	_compute_attenuation (0000)
00093C50 0030:
	_compute_composite_attenuation (0000)
00093C80 00f0:
	_unit_get_aim_assist_parameters (0000)
00093D70 00a0:
	_compare_targets (0000)
00093E10 0010:
	_reciprocal_square_root (0000)
00093E20 0060:
	_limit3d (0000)
00093E80 0020:
	_set_real_euler_angles2d (0000)
00093EA0 00f0:
	_aim_assist_clear_line_of_sight (0000)
00093F90 01a0:
	_object_compute_autoaim_target (0000)
00094130 01a0:
	_aim_assist_compute_target (0000)
000942D0 0110:
	_autoaim_compute_target (0000)
000943E0 0190:
	_find_aim_assist_targets_recursive (0000)
00094570 0130:
	_find_aim_assist_targets (0000)
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
#include "aim_assist.h"
#include "players.h"
#include "render.h"
#include "network_game_globals.h"
#include "actor_definitions.h"
#include "network_messages.h"
#include "collisions.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "director.h"
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
