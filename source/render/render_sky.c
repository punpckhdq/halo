/*
RENDER_SKY.C

symbols in this file:
0017C290 04d0:
	_render_sky (0000)
002A007C 0004:
	__real@3f7fc000 (0000)
002A0080 0004:
	__real@447ff800 (0000)
002A0088 0048:
	??_C@_0EI@POBIDGNL@?$CBrender?4visible_sky_model?5?$HM?$HM?5sce@ (0000)
002A00D0 0023:
	??_C@_0CD@OBGDMKCP@c?3?2halo?2SOURCE?2render?2render_sky@ (0000)
004C04F8 0020:
	_bss_004c04f8 (0000)
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
#include "rasterizer.h"
#include "light_definitions.h"
#include "collision_usage.h"
#include "collision_features.h"
#include "physics_constants.h"
#include "object_lights.h"
#include "structures/radiosity/radiosity.h"
#include "vector_tree.h"
#include "sky_definitions.h"
#include "structures/radiosity/radiosity_io.h"
#include "object_types.h"
#include "objects.h"
#include "damage.h"
#include "models.h"
#include "collisions.h"
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
