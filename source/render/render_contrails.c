/*
RENDER_CONTRAILS.C

symbols in this file:
001777D0 0090:
	_contrail_fade (0000)
00177860 07a0:
	_code_00177860 (0000)
00178000 00b0:
	_code_00178000 (0000)
001780B0 0010:
	_render_contrails_ground_mapped (0000)
001780C0 0010:
	_render_contrails_media_mapped (0000)
001780D0 0010:
	_render_contrails_normal (0000)
0029FC28 002d:
	??_C@_0CN@ILEHFPFH@contrail?5?$CFs?5uses?5an?5unsupported?5@ (0000)
0029FC58 0016:
	??_C@_0BG@PBIEBPJJ@triangles?5?$CG?$CG?5vertices?$AA@ (0000)
0029FC70 0029:
	??_C@_0CJ@KBEDIECC@c?3?2halo?2SOURCE?2render?2render_con@ (0000)
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
#include "texture_cache.h"
#include "contrails.h"
#include "contrail_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
