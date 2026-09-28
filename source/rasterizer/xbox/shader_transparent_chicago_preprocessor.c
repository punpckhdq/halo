/*
SHADER_TRANSPARENT_CHICAGO_PREPROCESSOR.C

symbols in this file:
0016B4E0 0010:
	_code_0016b4e0 (0000)
0016B4F0 01b0:
	_shader_transparent_chicago_create (0000)
0029CE08 00d0:
	_rdata_0029ce08 (0000)
0029CED8 0025:
	??_C@_0CF@ILPKPMME@?$CD?$CD?$CD?5ERROR?5chicago?5shader?5has?5no?5@ (0000)
0029CF00 0049:
	??_C@_0EJ@GDMOILEK@c?3?2halo?2SOURCE?2rasterizer?2xbox?2s@ (0000)
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
#include "leaf_map.h"
#include "render_cameras.h"
#include "structure_bsp_definitions.h"
#include "render.h"
#include "rasterizer.h"
#include "bitmap_macros.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "rasterizer/xbox/rasterizer_xbox.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
