/*
CAMERA_SCRIPTING.C

symbols in this file:
00073650 0020:
	_scripted_camera_enable (0000)
00073670 00d0:
	_scripted_camera_set_animation (0000)
00073740 0040:
	_scripted_camera_set_first_person (0000)
00073780 0040:
	_scripted_camera_set_dead (0000)
000737C0 0030:
	_scripted_camera_object_is_first_person_camera (0000)
000737F0 00e0:
	_scripted_camera_set (0000)
000738D0 0020:
	_scripted_camera_set_absolute (0000)
000738F0 00d0:
	_scripted_camera_set_camera_point_relative (0000)
000739C0 0030:
	_scripted_camera_set_camera_point_absolute (0000)
000739F0 0010:
	_scripted_camera_next_camera_point (0000)
00073A00 0010:
	_scripted_camera_object_relative_to (0000)
00073A10 0020:
	_scripted_camera_time (0000)
00073A30 0680:
	_scripted_camera_update (0000)
00256A7C 003d:
	??_C@_0DN@JNLEKBID@cannot?5set?5first?5person?5camera?5o@ (0000)
00256ABC 0029:
	??_C@_0CJ@GLCFCHKA@c?3?2halo?2SOURCE?2camera?2camera_scr@ (0000)
002DCB60 0040:
	_data_002dcb60 (0000)
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
#include "bsp3d.h"
#include "bsp2d.h"
#include "collision_bsp_definitions.h"
#include "collision_bsp.h"
#include "leaf_map.h"
#include "render_cameras.h"
#include "structures.h"
#include "render.h"
#include "structure_bsp_definitions.h"
#include "ai.h"
#include "ai_constants.h"
#include "unit_definitions.h"
#include "ai_communication.h"
#include "units.h"
#include "console.h"
#include "director.h"
#include "observer.h"
#include "player_effects.h"
#include "dead_camera.h"
#include "first_person_camera.h"
#include "camera_scripting.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
