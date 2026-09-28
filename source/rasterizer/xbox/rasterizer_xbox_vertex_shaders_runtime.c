/*
RASTERIZER_XBOX_VERTEX_SHADERS_RUNTIME.C

symbols in this file:
00168350 0010:
	_code_00168350 (0000)
00168360 0010:
	_code_00168360 (0000)
00168370 0010:
	_code_00168370 (0000)
00168380 0010:
	_code_00168380 (0000)
00168390 0560:
	_rasterizer_set_vertex_shader_permutation (0000)
0029C2F8 0528:
	_rdata_0029c2f8 (0000)
0029C820 0043:
	??_C@_0ED@PALLOMFH@IDirect3DDevice8_SetVertexShader@ (0000)
0029C868 0054:
	??_C@_0FE@NPNOIOBA@IDirect3DDevice8_SelectVertexSha@ (0000)
0029C8C0 0052:
	??_C@_0FC@JAKGKFDP@IDirect3DDevice8_LoadVertexShade@ (0000)
0029C918 0049:
	??_C@_0EJ@GJBDGHE@IDirect3DDevice8_SelectVertexSha@ (0000)
0029C964 0026:
	??_C@_0CG@FJEBEMLC@?$CD?$CD?$CD?5ERROR?5vertex?5shader?5was?5not?5@ (0000)
0029C98C 0038:
	??_C@_0DI@ENBGJMAN@?$CD?$CD?$CD?5ERROR?5packed?5vertex?5shaders?5@ (0000)
0029C9C8 0094:
	??_C@_0JE@FLCOGNAM@IDirect3DDevice8_GetVertexShader@ (0000)
0029CA60 004b:
	??_C@_0EL@IDNOJKOP@translation_table?$FLvertex_type?$CKpe@ (0000)
0029CAAC 003c:
	??_C@_0DM@HGGNMEDI@permutation_index?$DO?$DN0?5?$CG?$CG?5permutat@ (0000)
0029CAE8 0024:
	??_C@_0CE@IOILLCAH@?$CD?$CD?$CD?5ERROR?5unsupported?5vertex?5sha@ (0000)
0029CB10 0047:
	??_C@_0EH@NIDAECGF@vertex_shader_index?$DO?$DN0?5?$CG?$CG?5vertex@ (0000)
0029CB58 0048:
	??_C@_0EI@JLPCJIEP@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
0030D3B8 001c:
	_data_0030d3b8 (0000)
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
#include "bsp3d.h"
#include "shader_definitions.h"
#include "bsp2d.h"
#include "collision_bsp_definitions.h"
#include "collision_bsp.h"
#include "leaf_map.h"
#include "render_cameras.h"
#include "structures.h"
#include "render.h"
#include "structure_bsp_definitions.h"
#include "bitmap_macros.h"
#include "rasterizer.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "strings/resource.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "cheats.h"
#include "rasterizer/xbox/rasterizer_xbox.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
