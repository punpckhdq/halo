/*
CHEATS.C

symbols in this file:
00094C50 0020:
	_cheats_initialize (0000)
00094C70 0010:
	_cheats_dispose (0000)
00094C80 0010:
	_cheats_dispose_from_old_map (0000)
00094C90 00b0:
	_cheats_update (0000)
00094D40 0090:
	_cheats_load (0000)
00094DD0 0060:
	_cheat_active_camouflage_local_player (0000)
00094E30 0060:
	_code_00094e30 (0000)
00094E90 0010:
	_cheats_initialize_for_new_map (0000)
00094EA0 00b0:
	_cheat_teleport_to_camera (0000)
00094F50 0050:
	_cheat_active_camouflage (0000)
00094FA0 0150:
	_code_00094fa0 (0000)
000950F0 00d0:
	_cheat_all_weapons (0000)
000951C0 0050:
	_cheat_all_powerups (0000)
00095210 0050:
	_cheat_all_vehicles (0000)
0025AC64 003b:
	??_C@_0DL@LCJIJOPI@Cannot?5execute?5cheats?5attached?5t@ (0000)
0025ACA0 0005:
	??_C@_04JNJDPIIA@?$AN?6?7?$DL?$AA@ (0000)
0025ACA8 000e:
	??_C@_0O@KAFNGCBO@d?3?2cheats?4txt?$AA@ (0000)
0025ACB8 003a:
	??_C@_0DK@OACLODJP@Camera?5is?5outside?5BSP?4?4?4?5cannot?5@ (0000)
0025ACF4 001d:
	??_C@_0BN@DPPGFJCA@c?3?2halo?2SOURCE?2game?2cheats?4c?$AA@ (0000)
0025AD14 0004:
	__real@3ec90fdb (0000)
0043D808 0c81:
	_bss_0043d808 (0000)
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
#include "path.h"
#include "actor_definitions.h"
#include "input.h"
#include "console.h"
#include "item_definitions.h"
#include "physics_variables.h"
#include "vehicle_definitions.h"
#include "aim_assist.h"
#include "meter_definitions.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"
#include "observer.h"
#include "edit_text.h"
#include "terminal.h"
#include "equipment_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

struct cheat_globals cheat;

/* ---------- public code */

/* ---------- private code */
