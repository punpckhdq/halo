/*
RASTERIZER_CINEMATICS.C

symbols in this file:
0016D140 0020:
	_code_0016d140 (0000)
0016D160 0040:
	_rasterizer_screen_effects_initialize (0000)
0016D1A0 0030:
	_rasterizer_screen_effects_initialize_for_new_map (0000)
0016D1D0 0010:
	_rasterizer_screen_effects_dispose_from_old_map (0000)
0016D1E0 0010:
	_rasterizer_screen_effects_dispose (0000)
0016D1F0 0030:
	_rasterizer_script_screen_effect_set_value (0000)
0016D220 0030:
	_rasterizer_script_screen_effect_get_value (0000)
0016D250 0040:
	_rasterizer_screen_effect_start (0000)
0016D290 0070:
	_rasterizer_screen_effect_set_convolution (0000)
0016D300 0070:
	_rasterizer_screen_effect_set_filter (0000)
0016D370 0020:
	_rasterizer_screen_effect_set_filter_desaturation_tint (0000)
0016D390 0120:
	_rasterizer_screen_effect_set_video (0000)
0016D4B0 0010:
	_rasterizer_screen_effect_stop (0000)
0016D4C0 0250:
	_rasterizer_screen_effect_get_cinematic_parameters (0000)
0016D710 0020:
	_rasterizer_set_near_clip_distance (0000)
0016D730 0030:
	_rasterizer_get_near_clip_distance (0000)
0029D844 0020:
	??_C@_0CA@HAHKAAEK@cinematic_screen_effect_globals?$AA@ (0000)
0029D864 0032:
	??_C@_0DC@FCKAGPCD@c?3?2halo?2SOURCE?2rasterizer?2raster@ (0000)
0029D898 0014:
	??_C@_0BE@COKLOHBN@screen?5effect?5filth?$AA@ (0000)
0029D8B0 004a:
	??_C@_0EK@IJBENNIM@?$CD?$CD?$CD?5ERROR?5cinematics?5failed?5to?5s@ (0000)
0029D900 008d:
	??_C@_0IN@PCKCIAIJ@?$CD?$CD?$CD?5FATAL_ERROR?5screen?5effects?5c@ (0000)
004662F4 0004:
	_bss_004662f4 (0000)
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
#include "rasterizer.h"
#include "bitmap_macros.h"
#include "bitmaps_inlines.h"
#include "lruv_cache.h"
#include "memory_pool.h"
#include "rasterizer_console_vars.h"
#include "game_state.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "main.h"
#include "rasterizer_cinematics.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
