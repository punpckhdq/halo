/*
BIPED_LIMP_NOODLE.C

symbols in this file:
0018ED80 0010:
	_biped_limp_noodle_get_max_relaxation_iterations (0000)
0018ED90 04e0:
	_code_0018ed90 (0000)
0018F270 07b0:
	_code_0018f270 (0000)
0018FA20 01f0:
	_validate_real_vector3d_axes3 (0000)
0018FC10 02c0:
	_code_0018fc10 (0000)
0018FED0 0100:
	_biped_limp_noodle_relax_nodes_onto_environment (0000)
002A3030 0045:
	??_C@_0EF@HHBJMIJE@realcmp?$CIplane3d_distance_to_poin@ (0000)
002A3078 0029:
	??_C@_0CJ@FPDJGKOB@c?3?2halo?2SOURCE?2units?2biped_limp_@ (0000)
002A30A4 0004:
	__real@3d8f5c29 (0000)
002A30A8 0004:
	__real@3f83d70a (0000)
002A30AC 0004:
	__real@bd036d41 (0000)
004C1C08 af08:
	_bss_004c1c08 (0000)
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
#include "ai.h"
#include "ai_constants.h"
#include "unit_definitions.h"
#include "ai_communication.h"
#include "units.h"
#include "console.h"
#include "collision_usage.h"
#include "collision_features.h"
#include "collisions.h"
#include "render_debug.h"
#include "biped_definitions.h"
#include "bipeds.h"
#include "physics_constants.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
