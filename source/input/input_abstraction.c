/*
INPUT_ABSTRACTION.C

symbols in this file:
000BDA30 0020:
	_input_abstraction_dispose (0000)
000BDA50 0010:
	_input_abstraction_reset_controller_detection_timer (0000)
000BDA60 0080:
	_input_abstraction_get_local_player_preferences (0000)
000BDAE0 00b0:
	_input_abstraction_update_local_player_preferences (0000)
000BDB90 0050:
	_input_abstraction_get_input_state (0000)
000BDBE0 0080:
	_input_abstraction_update_device_changes (0000)
000BDC60 00e0:
	_code_000bdc60 (0000)
000BDD40 0050:
	_code_000bdd40 (0000)
000BDD90 00a0:
	_input_abstraction_initialize (0000)
000BDE30 0950:
	_input_abstraction_update (0000)
0026F3A0 0014:
	_rdata_0026f3a0 (0000)
0026F3B4 000c:
	??_C@_0M@OAMDCHEH@preferences?$AA@ (0000)
0026F3C0 0041:
	??_C@_0EB@NBKADHOE@?$CIlocal_player_index?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIloca@ (0000)
0026F404 0029:
	??_C@_0CJ@KPJFDJEG@c?3?2halo?2SOURCE?2input?2input_abstr@ (0000)
0026F430 0041:
	??_C@_0EB@LOCNLGGH@invalid?5controller?5preferences?$DL?5@ (0000)
0026F474 003d:
	??_C@_0DN@PDKODJDH@?$CIcontroller_index?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIcontro@ (0000)
0026F4B4 0039:
	??_C@_0DJ@DKGJGJAM@stopping?5bink?5playback?5to?5due?5to@ (0000)
0026F4F0 004c:
	??_C@_0EM@DKLJMFDI@?$CIcontroller_index?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIcontro@ (0000)
0026F53C 0018:
	??_C@_0BI@DEJBIOHJ@unknown?5joystick?5preset?$AA@ (0000)
0026F558 0008:
	__real@3fc6571840000000 (0000)
0026F560 0008:
	__real@3ffa313e30a3879f (0000)
0026F568 0008:
	__real@3fe38c3550000000 (0000)
004535C0 00e0:
	_input_abstraction_globals (0000)
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
#include "bungie_net/network/transport.h"
#include "network_messages.h"
#include "physics_variables.h"
#include "input_abstraction.h"
#include "vehicle_definitions.h"
#include "input_windows.h"
#include "text_group.h"
#include "vehicles.h"
#include "event_manager.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "ui_widget_group.h"
#include "ui_widget.h"
#include "cluster_partitions.h"
#include "network_client_manager.h"
#include "edit_text.h"
#include "terminal.h"
#include "player_ui.h"
#include "shell.h"
#include "bink_playback.h"
#include "virtual_keyboard.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
