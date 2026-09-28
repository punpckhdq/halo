/*
MATERIAL_EFFECTS.C

symbols in this file:
0008DA20 0080:
	_material_effect_visible (0000)
0008DAA0 0140:
	_material_effect_new (0000)
0008DBE0 0110:
	_material_effect_new_from_point (0000)
0043D589 0001:
	_debug_material_effects (0000)
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
#include "unit_definitions.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "unicode.h"
#include "index_resolution.h"
#include "game_engine.h"
#include "network_game_manager.h"
#include "players.h"
#include "collision_usage.h"
#include "sound_environment_definitions.h"
#include "collision_features.h"
#include "collisions.h"
#include "sound_manager.h"
#include "render_debug.h"
#include "game_sound.h"
#include "effect_definitions.h"
#include "observer.h"
#include "effects.h"
#include "material_effects.h"
#include "material_effect_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
