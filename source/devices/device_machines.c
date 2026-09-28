/*
DEVICE_MACHINES.C

symbols in this file:
00084100 0010:
	_machines_initialize (0000)
00084110 0010:
	_machines_dispose (0000)
00084120 0010:
	_machines_initialize_for_new_map (0000)
00084130 0010:
	_machines_dispose_from_old_map (0000)
00084140 0080:
	_machine_place (0000)
000841C0 0080:
	_machine_new (0000)
00084240 0010:
	_machine_delete (0000)
00084250 0030:
	_machine_bumped (0000)
00084280 0050:
	_machine_try_to_open_with_damage (0000)
000842D0 0400:
	_machine_update (0000)
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
#include "ai_communication.h"
#include "units.h"
#include "biped_definitions.h"
#include "bipeds.h"
#include "physics_constants.h"
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
