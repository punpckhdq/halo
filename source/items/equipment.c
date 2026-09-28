/*
EQUIPMENT.C

symbols in this file:
000E5F40 0060:
	_equipment_place (0000)
000E5FA0 0040:
	_equipment_handle_pickup (0000)
000E5FE0 0030:
	_equipment_definition_handle_pickup (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "game.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "object_definitions.h"
#include "rasterizer_geometry.h"
#include "model_animation_definitions.h"
#include "model_definitions.h"
#include "damage_resistances.h"
#include "shader_definitions.h"
#include "unit_definitions.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "index_resolution.h"
#include "sound_environment_definitions.h"
#include "sound_manager.h"
#include "item_definitions.h"
#include "object_types.h"
#include "objects.h"
#include "damage.h"
#include "models.h"
#include "items.h"
#include "game_sound.h"
#include "equipment_definitions.h"
#include "equipment.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
