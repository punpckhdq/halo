/*
GAME_ENGINE_ODDBALL.C

symbols in this file:
000A1630 0010:
	_oddball_engine_dispose (0000)
000A1640 0010:
	_oddball_engine_dispose_from_old_map (0000)
000A1650 0020:
	_oddball_engine_player_added (0000)
000A1670 0010:
	_oddball_engine_game_ending (0000)
000A1680 0010:
	_oddball_engine_game_starting (0000)
000A1690 0010:
	_oddball_engine_statistics_append (0000)
000A16A0 0010:
	_oddball_engine_handle_client_message (0000)
000A16B0 0010:
	_oddball_engine_handle_server_message (0000)
000A16C0 0010:
	_oddball_engine_pregame_post_rasterize (0000)
000A16D0 0010:
	_oddball_engine_post_rasterize (0000)
000A16E0 00d0:
	_oddball_add_score (0000)
000A17B0 0040:
	_oddball_add_time_with_ball (0000)
000A17F0 0030:
	_player_ball_count (0000)
000A1820 0010:
	_oddball_engine_player_damaged_player (0000)
000A1830 0030:
	_player_has_ball (0000)
000A1860 0040:
	_ball_available (0000)
000A18A0 02e0:
	_oddball_engine_display_score (0000)
000A1B80 0010:
	_oddball_engine_prespawn_player_update (0000)
000A1B90 0040:
	_oddball_weapon_drop (0000)
000A1BD0 0040:
	_oddball_get_score (0000)
000A1C10 0020:
	_oddball_ball_transfer_by_killing (0000)
000A1C30 0020:
	_accumulate_score_by_time (0000)
000A1C50 0020:
	_terminator_scoring_rules (0000)
000A1C70 0020:
	_oddball_test_flag (0000)
000A1C90 0050:
	_oddball_test_trait (0000)
000A1CE0 0050:
	_oddball_get_score_string (0000)
000A1D30 0070:
	_oddball_get_score_header_string (0000)
000A1DA0 0050:
	_oddball_get_team_score_string (0000)
000A1DF0 0140:
	_find_position_for_ball (0000)
000A1F30 0090:
	_create_the_ball (0000)
000A1FC0 0120:
	_oddball_engine_initialize_for_new_map (0000)
000A20E0 0070:
	_reset_ball (0000)
000A2150 0090:
	_update_ball_ownership (0000)
000A21E0 01c0:
	_oddball_engine_player_update (0000)
000A23A0 00c0:
	_oddball_engine_weapon_update (0000)
000A2460 00d0:
	_oddball_engine_update (0000)
000A2530 01c0:
	_oddball_engine_player_killed_player (0000)
000A26F0 00b0:
	_oddball_weapon_pickup (0000)
0025BE74 002a:
	??_C@_0CK@LKLFFONL@c?3?2halo?2SOURCE?2game?2game_engine_@ (0000)
0025BEA0 0034:
	??_C@_0DE@EJKHILOB@0?5?$CG?$CG?5?$CCthis?5map?5was?5not?5correctly@ (0000)
0025BED8 0040:
	??_C@_0EA@MPBOOMHB@?$CD?$CD?$CD?5failed?5to?5find?5any?5suitable?5@ (0000)
0025BF18 0023:
	??_C@_0CD@CKBFGHJI@?$CD?$CD?$CD?5failed?5to?5find?5the?5flag?5obje@ (0000)
0025BF3C 0028:
	??_C@_0CI@KJBBGHKH@?$CD?$CD?$CD?5failed?5to?5find?5ball?5spawn?5te@ (0000)
0025BF64 0028:
	??_C@_0CI@LAAKFGOG@?$CD?$CD?$CD?5failed?5to?5find?5ball?5spawn?5te@ (0000)
0025BF8C 000a:
	??_C@_09GMEHKGKL@ball_blue?$AA@ (0000)
0025BF98 000c:
	??_C@_0M@FNMEMKCB@target_blue?$AA@ (0000)
0025BFA4 002d:
	??_C@_0CN@DJFDFML@?$CBball_available?$CI?$CJ?5?$HM?$HM?5?$CIcapture_in@ (0000)
002DE560 0088:
	_oddball_engine (0000)
0043EBA8 0104:
	_bss_0043eba8 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "players.h"
#include "game_engine_list.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "rasterizer.h"
#include "actor_definitions.h"
#include "input.h"
#include "console.h"
#include "network_messages.h"
#include "sound_manager.h"
#include "text_group.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
#include "game_sound.h"
#include "font_group.h"
#include "network_server_message_handler.h"
#include "scenery.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
