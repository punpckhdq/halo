/*
CINEMATICS.C

symbols in this file:
000813F0 0040:
	_cinematic_initialize (0000)
00081430 0010:
	_cinematic_dispose (0000)
00081440 0030:
	_cinematic_initialize_for_new_map (0000)
00081470 0020:
	_cinematic_dispose_from_old_map (0000)
00081490 0040:
	_cinematic_start (0000)
000814D0 0010:
	_cinematic_can_be_skipped (0000)
000814E0 0010:
	_cinematic_skip_start (0000)
000814F0 0010:
	_cinematic_skip_stop (0000)
00081500 0030:
	_cinematic_show_letterbox (0000)
00081530 0150:
	_draw_quad (0000)
00081680 0020:
	_cinematic_force_title (0000)
000816A0 0020:
	_cinematic_suppress_bsp_object_creation (0000)
000816C0 0050:
	_cinematic_stop (0000)
00081710 0010:
	_cinematic_in_progress (0000)
00081720 0090:
	_cinematic_set_title_delayed (0000)
000817B0 0500:
	_cinematic_render (0000)
00081CB0 0020:
	_cinematic_set_title (0000)
002589FC 0012:
	??_C@_0BC@NGABDHNK@cinematic_globals?$AA@ (0000)
00258A10 0025:
	??_C@_0CF@PIGNLDBH@c?3?2halo?2SOURCE?2cutscene?2cinemati@ (0000)
00258A38 0012:
	??_C@_0BC@PJBLDLNO@cinematic?5globals?$AA@ (0000)
00258A4C 0032:
	??_C@_0DC@GMMMKPOA@no?5free?5chapter?5title?5slots?5to?5d@ (0000)
00258A80 0004:
	__real@3e000000 (0000)
00435CA0 0004:
	_cinematic_globals (0000)
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
#include "ai.h"
#include "ai_constants.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "ai_communication.h"
#include "unicode.h"
#include "index_resolution.h"
#include "game_engine.h"
#include "network_game_manager.h"
#include "rasterizer.h"
#include "players.h"
#include "bitmap_macros.h"
#include "input.h"
#include "lruv_cache.h"
#include "memory_pool.h"
#include "game_state.h"
#include "sound_environment_definitions.h"
#include "sound_manager.h"
#include "input_abstraction.h"
#include "input_windows.h"
#include "items.h"
#include "text_group.h"
#include "event_manager.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "ui_widget_group.h"
#include "ui_widget.h"
#include "draw_string.h"
#include "hud_definitions.h"
#include "texture_cache.h"
#include "editor_stubs.h"
#include "hud.h"
#include "cinematics.h"
#include "projectiles.h"
#include "ai_globals.h"
#include "rasterizer_cinematics.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
