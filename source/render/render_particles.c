/*
RENDER_PARTICLES.C

symbols in this file:
0017BD20 00b0:
	_local_player_is_first_person (0000)
0017BDD0 0030:
	_code_0017bdd0 (0000)
0017BE00 0490:
	_render_particles (0000)
0030E180 05f8:
	_data_0030e180 (0000)
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
#include "bsp3d.h"
#include "bsp2d.h"
#include "collision_bsp_definitions.h"
#include "collision_bsp.h"
#include "leaf_map.h"
#include "render_cameras.h"
#include "ai.h"
#include "ai_constants.h"
#include "unit_definitions.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "ai_communication.h"
#include "unicode.h"
#include "index_resolution.h"
#include "game_engine.h"
#include "network_game_manager.h"
#include "players.h"
#include "console.h"
#include "render_debug.h"
#include "director.h"
#include "structures.h"
#include "render.h"
#include "structure_bsp_definitions.h"
#include "physics_constants.h"
#include "object_lights.h"
#include "first_person_weapons.h"
#include "particles.h"
#include "particle_definitions.h"
#include "object_types.h"
#include "objects.h"
#include "damage.h"
#include "models.h"
#include "units.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
