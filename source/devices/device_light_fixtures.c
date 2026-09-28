/*
DEVICE_LIGHT_FIXTURES.C

symbols in this file:
00083FE0 0010:
	_light_fixtures_initialize (0000)
00083FF0 0010:
	_light_fixtures_dispose (0000)
00084000 0010:
	_light_fixtures_initialize_for_new_map (0000)
00084010 0010:
	_light_fixtures_dispose_from_old_map (0000)
00084020 0070:
	_light_fixture_place (0000)
00084090 0030:
	_light_fixture_new (0000)
000840C0 0010:
	_light_fixture_delete (0000)
000840D0 0030:
	_light_fixture_update (0000)
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
