/*
HUD_WEAPON.C

symbols in this file:
000C7E70 0040:
	_hud_initialize_weapon_interface (0000)
000C7EB0 0040:
	_hud_initialize_weapon_interface_for_new_map (0000)
000C7EF0 0010:
	_hud_dispose_weapon_interface_from_old_map (0000)
000C7F00 0010:
	_hud_dispose_weapon_interface (0000)
000C7F10 0030:
	_scripted_hud_show_crosshair (0000)
000C7F40 0070:
	_code_000c7f40 (0000)
000C7FB0 0070:
	_code_000c7fb0 (0000)
000C8020 0050:
	_code_000c8020 (0000)
000C8070 02e0:
	_code_000c8070 (0000)
000C8350 0020:
	_strip_path_name (0000)
000C8370 08d0:
	_code_000c8370 (0000)
000C8C40 00a0:
	_hud_fix_weapon_data (0000)
000C8CE0 05c0:
	_code_000c8ce0 (0000)
000C92A0 0a60:
	_code_000c92a0 (0000)
000C9D00 0270:
	_hud_update_weapon (0000)
000C9F70 01f0:
	_hud_render_weapon_interface (0000)
002702F8 0013:
	??_C@_0BD@NOPIHEPB@weapon_hud_globals?$AA@ (0000)
0027030C 0026:
	??_C@_0CG@DOLNIFA@c?3?2halo?2SOURCE?2interface?2hud_wea@ (0000)
00270334 0015:
	??_C@_0BF@HCPMNCNB@hud?5weapon?5interface?$AA@ (0000)
00270350 0008:
	__real@3ff4000000000000 (0000)
00270358 0052:
	??_C@_0FC@PHPDNACJ@frame?5index?5NONE?5when?5drawing?5cr@ (0000)
002703AC 0030:
	??_C@_0DA@GANDHJMM@too?5many?5levels?5in?5current?5weapo@ (0000)
00453AC4 0004:
	_bss_00453ac4 (0000)
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
#include "ai.h"
#include "ai_constants.h"
#include "unit_definitions.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "ai_communication.h"
#include "unicode.h"
#include "units.h"
#include "index_resolution.h"
#include "game_engine.h"
#include "network_game_manager.h"
#include "rasterizer.h"
#include "players.h"
#include "path.h"
#include "actor_definitions.h"
#include "bitmap_macros.h"
#include "bitmaps_inlines.h"
#include "lruv_cache.h"
#include "memory_pool.h"
#include "game_state.h"
#include "bungie_net/network/transport.h"
#include "network_messages.h"
#include "collision_usage.h"
#include "collision_features.h"
#include "collisions.h"
#include "item_definitions.h"
#include "items.h"
#include "aim_assist.h"
#include "meter_definitions.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"
#include "render_debug.h"
#include "director.h"
#include "weapons.h"
#include "draw_string.h"
#include "hud_definitions.h"
#include "texture_cache.h"
#include "physics_constants.h"
#include "hud.h"
#include "font_group.h"
#include "hud_messaging.h"
#include "weapon_hud_interface_definition.h"
#include "motion_sensor.h"
#include "interface_panels.h"
#include "inventory_displays.h"
#include "unit_hud_interface_definition.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
