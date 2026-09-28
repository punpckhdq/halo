/*
RASTERIZER_COMMON.C
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
#include "lruv_cache.h"
#include "memory_pool.h"
#include "rasterizer_console_vars.h"
#include "game_state.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "rasterizer_cinematics.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

struct game_globals_rasterizer_data *global_rasterizer_data;

/* ---------- public code */

void rasterizer_initialize_for_new_map(
	void)
{
	struct game_globals *game_globals = NULL;

	game_globals = scenario_get_game_globals();

	match_assert("c:\\halo\\SOURCE\\rasterizer\\common\\rasterizer_common.c", 24, game_globals);
	global_rasterizer_data = game_globals->rasterizer_data.count!=0 ? TAG_BLOCK_GET_ELEMENT(&game_globals->rasterizer_data, 0, struct game_globals_rasterizer_data) : NULL;
	match_assert("c:\\halo\\SOURCE\\rasterizer\\common\\rasterizer_common.c", 26, global_rasterizer_data);

	rasterizer_lights_reset_for_new_map();
	rasterizer_text_cache_flush();
	rasterizer_screen_effects_initialize_for_new_map();

	if (global_rasterizer_model_ambient_reflection_tint)
	{
		memset(global_rasterizer_model_ambient_reflection_tint, 0, sizeof(*global_rasterizer_model_ambient_reflection_tint));
	}

	rasterizer_set_near_clip_distance(0.f);

	return;
}

void rasterizer_dispose_from_old_map(
	void)
{
	rasterizer_screen_effects_dispose_from_old_map();
	rasterizer_text_cache_flush();
	global_rasterizer_data = NULL;

	return;
}

void rasterizer_frame_update(
	real dt)
{
	global_frame_parameters.dt = dt;

	return;
}
