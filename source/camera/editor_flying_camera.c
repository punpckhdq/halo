/*
EDITOR_FLYING_CAMERA.C

symbols in this file:
00075E70 00d0:
	_editor_camera_new (0000)
00075F40 0080:
	_editor_camera_get_focus (0000)
00075FC0 0080:
	_editor_camera_set_focus (0000)
00076040 00a0:
	_editor_camera_set_position (0000)
000760E0 0040:
	_editor_camera_bump_speed (0000)
00076120 0010:
	_editor_camera_get_speed (0000)
00076130 0030:
	_editor_camera_use_roll (0000)
00076160 0010:
	_editor_camera_get_unit_focus (0000)
00076170 00e0:
	_editor_camera_set_mode (0000)
00076250 0010:
	_editor_camera_get_mode (0000)
00076260 0010:
	_editor_camera_get_scripted (0000)
00076270 0070:
	_code_00076270 (0000)
000762E0 0010:
	_editor_camera_get_field_of_view (0000)
000762F0 0080:
	_editor_camera_move_to_point (0000)
00076370 01b0:
	_editor_camera_set_position_and_roll (0000)
00076520 0070:
	_editor_camera_set_unit_focus (0000)
00076590 0130:
	_editor_camera_update (0000)
000766C0 01b0:
	_editor_camera_set_scripted (0000)
00076870 0060:
	_code_00076870 (0000)
000768D0 0580:
	_code_000768d0 (0000)
00076E50 0460:
	_code_00076e50 (0000)
00256C64 0030:
	_rdata_00256c64 (0000)
00256C94 0008:
	??_C@_07NACDKLFL@exiting?$AA@ (0000)
00256C9C 0010:
	??_C@_0BA@NDFANOEE@orbiting?5camera?$AA@ (0000)
00256CAC 000e:
	??_C@_0O@GNANFALA@flying?5camera?$AA@ (0000)
00256CBC 0007:
	??_C@_06GNEAMJOC@angles?$AA@ (0000)
00256CC4 0009:
	??_C@_08OAGMDKAF@position?$AA@ (0000)
00256CD0 002d:
	??_C@_0CN@ELKCHNLL@c?3?2halo?2SOURCE?2camera?2editor_fly@ (0000)
00256D00 0011:
	??_C@_0BB@JCBBBJE@speed?5is?5now?5x?$CFf?$AA@ (0000)
00256D14 0025:
	??_C@_0CF@ONCNFIFO@translate_funcs?$FLmode?$FN?$FL_translate@ (0000)
00256D3C 002e:
	??_C@_0CO@LELHEFFL@translate_funcs?$FLcamera_mode?$FN?$FL_tr@ (0000)
00256D6C 001a:
	??_C@_0BK@JFLLMCBA@update_funcs?$FLcamera_mode?$FN?$AA@ (0000)
00256D88 0018:
	??_C@_0BI@CHKFHLBI@?$CFs?5scripted?5camera?5mode?$AA@ (0000)
00256DA0 0004:
	__real@3fc8a8ea (0000)
00256DA4 0004:
	__real@bfc8a8ea (0000)
00256DA8 0009:
	??_C@_08JLGBNAMD@controls?$AA@ (0000)
00256DB4 0004:
	__real@3fa0d97c (0000)
00256DB8 0004:
	__real@bfa0d97c (0000)
002DCC28 0034:
	_data_002dcc28 (0000)
	_editor_custom_render (0008)
0031D438 007c:
	_bss_0031d438 (0000)
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
#include "players.h"
#include "input.h"
#include "console.h"
#include "collision_usage.h"
#include "collision_features.h"
#include "collisions.h"
#include "director.h"
#include "observer.h"
#include "edit_text.h"
#include "terminal.h"
#include "camera_scripting.h"
#include "flying_camera.h"
#include "orbiting_camera.h"
#include "editor_flying_camera.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
