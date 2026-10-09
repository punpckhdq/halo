/*
GAME_ENGINE_SLAYER.C

symbols in this file:
000A3CF0 0050:
	_target_is_valid (0000)
000A3D40 0010:
	_slayer_engine_dispose (0000)
000A3D50 0030:
	_slayer_engine_initialize_for_new_map (0000)
000A3D80 0010:
	_slayer_engine_dispose_from_old_map (0000)
000A3D90 0030:
	_slayer_engine_player_added (0000)
000A3DC0 0010:
	_slayer_engine_game_ending (0000)
000A3DD0 0020:
	_slayer_engine_game_starting (0000)
000A3DF0 0010:
	_slayer_engine_statistics_append (0000)
000A3E00 0010:
	_slayer_engine_handle_client_message (0000)
000A3E10 0010:
	_slayer_engine_handle_server_message (0000)
000A3E20 0010:
	_slayer_engine_pregame_post_rasterize (0000)
000A3E30 0010:
	_slayer_engine_post_rasterize (0000)
000A3E40 0010:
	_slayer_engine_update (0000)
000A3E50 0010:
	_slayer_engine_allow_pick_up (0000)
000A3E60 0010:
	_slayer_engine_player_damaged_player (0000)
000A3E70 0110:
	_update_speed_for_score (0000)
000A3F80 0040:
	_slayer_engine_adjust_score (0000)
000A3FC0 0010:
	_slayer_engine_prespawn_player_update (0000)
000A3FD0 0040:
	_slayer_get_score (0000)
000A4010 0010:
	_slayer_test_flag (0000)
000A4020 0030:
	_slayer_get_score_string (0000)
000A4050 0060:
	_slayer_get_score_header_string (0000)
000A40B0 0030:
	_slayer_get_team_score_string (0000)
000A40E0 0190:
	_find_next_target (0000)
000A4270 0090:
	_slayer_engine_player_killed_player (0000)
000A4300 0280:
	_slayer_engine_display_score (0000)
000A4580 0190:
	_slayer_player_update (0000)
0025C190 0014:
	??_C@_0BE@HDNMKCGO@next_target?5?$CB?$DN?5NONE?$AA@ (0000)
0025C1A4 0029:
	??_C@_0CJ@GAGEHHLD@c?3?2halo?2SOURCE?2game?2game_engine_@ (0000)
0025C1D0 0036:
	??_C@_0DG@HCMPGGBD@?$CIindex?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIindex?5?$DM?5MULTIPL@ (0000)
0025C208 0004:
	__real@373a69dc (0000)
0025C20C 0004:
	__real@38e90453 (0000)
002DE670 0088:
	_slayer_engine (0000)
0043ED80 0080:
	_bss_0043ed80 (0000)
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
#include "console.h"
#include "network_messages.h"
#include "text_group.h"
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
