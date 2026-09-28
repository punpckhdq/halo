/*
SHADER_DEFINITIONS.C

symbols in this file:
0017FF00 0060:
	_shader_get_and_verify_type (0000)
002A082C 001f:
	??_C@_0BP@FJMHFJCB@shader?9?$DObase?4type?$DN?$DNshader_type?$AA@ (0000)
002A084C 002c:
	??_C@_0CM@OMIDGMIG@c?3?2halo?2SOURCE?2shaders?2shader_de@ (0000)
0030E800 016c:
	_global_shader_effect_additive (0000)
	_global_shader_effect_alpha_blended (00b8)
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
#include "light_definitions.h"
#include "object_types.h"
#include "objects.h"
#include "damage.h"
#include "models.h"
#include "structures.h"
#include "render.h"
#include "structure_bsp_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
