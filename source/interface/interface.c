/*
INTERFACE.C

symbols in this file:
000CE180 0020:
	_interface_initialize (0000)
000CE1A0 0010:
	_interface_dispose_from_old_map (0000)
000CE1B0 0020:
	_interface_dispose (0000)
000CE1D0 0080:
	_interface_get_tag_index (0000)
000CE250 0080:
	_interface_get_real_argb_color (0000)
000CE2D0 0050:
	_interface_set_bitmap_text_draw_mode (0000)
000CE320 01c0:
	_code_000ce320 (0000)
000CE4E0 03a0:
	_interface_draw_screen (0000)
000CE880 0080:
	_profile_graph_toggle (0000)
000CE900 0110:
	_code_000ce900 (0000)
000CEA10 08e0:
	_code_000cea10 (0000)
000CF2F0 00e0:
	_code_000cf2f0 (0000)
000CF3D0 0060:
	_interface_initialize_for_new_map (0000)
000CF430 0070:
	_interface_get_rgb_color (0000)
000CF4A0 0020:
	_interface_draw_fullscreen_overlays (0000)
000CF4C0 0180:
	_interface_draw_bitmap (0000)
000CF640 01c0:
	_interface_draw_bitmap_modulated (0000)
000CF800 0160:
	_interface_draw_bitmap_modulated_p32 (0000)
00270988 0047:
	??_C@_0EH@NDHILNFF@interface_tag_index?$DO?$DN0?5?$CG?$CG?5interf@ (0000)
002709D0 0025:
	??_C@_0CF@HPKANGD@c?3?2halo?2SOURCE?2interface?2interfa@ (0000)
002709F8 001f:
	??_C@_0BP@HBELBMEI@drawingbuf_counts?$FLindex?$FN?5?$DM?5512?$AA@ (0000)
00270A18 000d:
	??_C@_0N@KBDODLDN@?$HMn?$HMn?$HMn?$HMn?$HMn?$HMn?$AA@ (0000)
00270A28 0011:
	??_C@_0BB@EMOPPCDP@?$CF?55d?5particles?$HMn?$AA@ (0000)
00270A3C 002e:
	??_C@_0CO@GJNLBLAB@?$CF?55d?5active?5of?5?$CF?55d?5effects?5?$CI?$CF5d@ (0000)
00270A6C 002e:
	??_C@_0CO@ENADKIPF@?$CF?55d?5active?5of?5?$CF?55d?5objects?5?$CI?$CF?53@ (0000)
00270A9C 0020:
	??_C@_0CA@DMKIAGNI@?$CF?56?41fk?5free?5of?5?$CF?56?41fk?5total?$HMn?$AA@ (0000)
00270ABC 0004:
	__real@3a800000 (0000)
00270AC0 0010:
	??_C@_0BA@DAFKNDHA@window_count?$DN?$DN4?$AA@ (0000)
002E4C88 18918:
	_profile_game_value_count (0000)
	_profile_game_values (0008)
	_profile_frame_value_count (8308)
	_profile_frame_values (8310)
	_profile_graph_value_count (10610)
	_profile_graph_values (10618)
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
#include "input.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "bungie_net/network/transport.h"
#include "light_definitions.h"
#include "shaders.h"
#include "network_messages.h"
#include "rasterizer/common/rasterizer_common.h"
#include "collision_usage.h"
#include "item_definitions.h"
#include "input_abstraction.h"
#include "input_windows.h"
#include "items.h"
#include "aim_assist.h"
#include "event_manager.h"
#include "meter_definitions.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "director.h"
#include "ui_widget_group.h"
#include "ui_widget.h"
#include "weapons.h"
#include "draw_string.h"
#include "hud_definitions.h"
#include "effect_definitions.h"
#include "hud.h"
#include "font_group.h"
#include "effects.h"
#include "edit_text.h"
#include "terminal.h"
#include "cinematics.h"
#include "ai_profile.h"
#include "first_person_weapons.h"
#include "particles.h"
#include "weapon_hud_interface_definition.h"
#include "motion_sensor.h"
#include "rasterizer_cinematics.h"
#include "interface_panels.h"
#include "color_table_group.h"
#include "dialogs.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
