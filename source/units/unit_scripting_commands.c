/*
UNIT_SCRIPTING_COMMANDS.C

symbols in this file:
00197370 0040:
	_unit_scripting_set_maximum_vitality (0000)
001973B0 0080:
	_units_scripting_set_maximum_vitality (0000)
00197430 0120:
	_unit_scripting_set_current_vitality (0000)
00197550 0050:
	_units_scripting_set_current_vitality (0000)
001975A0 0040:
	_unit_scripting_get_health (0000)
001975E0 0040:
	_unit_scripting_get_shield (0000)
00197620 0040:
	_unit_scripting_get_grenade_count (0000)
00197660 0070:
	_unit_scripting_impervious (0000)
001976D0 0080:
	_unit_scripting_start_user_animation_list (0000)
00197750 0030:
	_unit_scripting_has_weapon (0000)
00197780 0060:
	_unit_scripting_has_weapon_readied (0000)
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
#include "path.h"
#include "actor_definitions.h"
#include "console.h"
#include "bungie_net/network/transport.h"
#include "network_messages.h"
#include "item_definitions.h"
#include "items.h"
#include "aim_assist.h"
#include "meter_definitions.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
#include "object_lists.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
