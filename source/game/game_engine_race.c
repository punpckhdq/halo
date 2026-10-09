/*
GAME_ENGINE_RACE.C

symbols in this file:
000A27A0 0010:
	_race_engine_dispose (0000)
000A27B0 0080:
	_delete_all_vehicles (0000)
000A2830 00f0:
	_race_get_vehicle_to_spawn (0000)
000A2920 0090:
	_race_flags_make_unique (0000)
000A29B0 0010:
	_race_engine_dispose_from_old_map (0000)
000A29C0 0030:
	_race_engine_player_added (0000)
000A29F0 0010:
	_race_engine_game_ending (0000)
000A2A00 0010:
	_race_engine_game_starting (0000)
000A2A10 0010:
	_race_engine_statistics_append (0000)
000A2A20 0010:
	_race_engine_handle_client_message (0000)
000A2A30 0010:
	_race_engine_handle_server_message (0000)
000A2A40 0010:
	_race_engine_pregame_post_rasterize (0000)
000A2A50 0010:
	_race_engine_post_rasterize (0000)
000A2A60 0190:
	_race_complete_lap (0000)
000A2BF0 0120:
	_can_touch_team (0000)
000A2D10 0010:
	_race_engine_weapon_update (0000)
000A2D20 0090:
	_race_team_can_win_game (0000)
000A2DB0 00e0:
	_build_player_speeds (0000)
000A2E90 0010:
	_race_engine_player_damaged_player (0000)
000A2EA0 0010:
	_race_engine_player_killed_player (0000)
000A2EB0 0480:
	_race_engine_display_score (0000)
000A3330 0010:
	_race_engine_prespawn_player_update (0000)
000A3340 0060:
	_race_goal_matches_player (0000)
000A33A0 0020:
	_count_bits_32 (0000)
000A33C0 00e0:
	_race_engine_get_score (0000)
000A34A0 0040:
	_race_get_score_string (0000)
000A34E0 0070:
	_race_get_score_header_string (0000)
000A3550 0030:
	_race_get_team_score_string (0000)
000A3580 00b0:
	_race_engine_did_player_win (0000)
000A3630 00e0:
	_find_closest_vehicle (0000)
000A3710 0130:
	_create_race_vehicles (0000)
000A3840 00f0:
	_new_rally_flag (0000)
000A3930 0150:
	_race_touch_flag (0000)
000A3A80 00a0:
	_race_engine_player_update (0000)
000A3B20 00c0:
	_race_engine_update (0000)
000A3BE0 0110:
	_race_engine_initialize_for_new_map (0000)
0025BFD4 0027:
	??_C@_0CH@MLHAJCND@c?3?2halo?2SOURCE?2game?2game_engine_@ (0000)
0025BFFC 0019:
	??_C@_0BJ@BIAKIHCM@itr?5?$DM?5MAXIMUM_RACE_FLAGS?$AA@ (0000)
0025C018 0036:
	??_C@_0DG@CFFMMMPN@?$CB?$CCrace?5goal?5matches?5player?5calle@ (0000)
0025C050 0011:
	??_C@_0BB@BCKDFNEI@?$CBcan_team_win?$FL1?$FN?$AA@ (0000)
0025C064 0011:
	??_C@_0BB@CAKPLNGL@new_flag?5?$CB?$DN?5NONE?$AA@ (0000)
0025C078 000a:
	??_C@_09DMMLFBJB@count?5?$DO?50?$AA@ (0000)
0025C084 0037:
	??_C@_0DH@EODBECII@?$CB?$CI?$CKlap_bit_vector?5?$CG?5?$HOrace_global@ (0000)
0025C0BC 0028:
	??_C@_0CI@ONDKDHNM@?$CBTEST_FLAG?$CI?$CKlap_bit_vector?0?5team@ (0000)
0025C0E8 0053:
	??_C@_0FD@CPKHPGHJ@_race_type_normal?5?$CB?$DN?5game_engine@ (0000)
0025C140 0050:
	??_C@_0FA@IHEEDPIP@one?5of?5the?5netgameflags?5that?5def@ (0000)
002DE5E8 0088:
	_race_engine (0000)
0043ECB0 00d0:
	_bss_0043ecb0 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "players.h"
#include "game_engine_list.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "actor_definitions.h"
#include "input.h"
#include "console.h"
#include "network_messages.h"
#include "text_group.h"
#include "vehicles.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
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
