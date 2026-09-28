/*
DEVICE_CONTROLS.C

symbols in this file:
00083D10 0010:
	_controls_initialize (0000)
00083D20 0010:
	_controls_dispose (0000)
00083D30 0010:
	_controls_initialize_for_new_map (0000)
00083D40 0010:
	_controls_dispose_from_old_map (0000)
00083D50 0070:
	_control_place (0000)
00083DC0 0030:
	_control_new (0000)
00083DF0 0010:
	_control_delete (0000)
00083E00 0030:
	_control_update (0000)
00083E30 0130:
	_code_00083e30 (0000)
00083F60 0040:
	_control_touched (0000)
00083FA0 0040:
	_control_destroyed (0000)
00259634 0029:
	??_C@_0CJ@LFHDKHDO@c?3?2halo?2SOURCE?2devices?2device_co@ (0000)
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
#include "ai_communication.h"
#include "units.h"
#include "effect_definitions.h"
#include "effects.h"
#include "object_lights.h"
#include "devices.h"
#include "device_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
