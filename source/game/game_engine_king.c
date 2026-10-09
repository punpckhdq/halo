/*
GAME_ENGINE_KING.C

symbols in this file:
000A0030 0010:
	_king_engine_dispose (0000)
000A0040 0020:
	_point3d_to_point2d (0000)
000A0060 0350:
	_find_hill (0000)
000A03B0 0010:
	_king_engine_dispose_from_old_map (0000)
000A03C0 0020:
	_king_engine_player_added (0000)
000A03E0 0010:
	_king_engine_game_ending (0000)
000A03F0 0020:
	_king_engine_game_starting (0000)
000A0410 0010:
	_king_engine_statistics_append (0000)
000A0420 0010:
	_king_engine_handle_client_message (0000)
000A0430 0010:
	_king_engine_handle_server_message (0000)
000A0440 0010:
	_king_engine_pregame_post_rasterize (0000)
000A0450 0090:
	_player_inside_hill (0000)
000A04E0 0160:
	_king_engine_player_update (0000)
000A0640 01c0:
	_king_calculate_hill_state (0000)
000A0800 0010:
	_king_engine_player_damaged_player (0000)
000A0810 0010:
	_king_engine_player_killed_player (0000)
000A0820 0190:
	_king_engine_display_score (0000)
000A09B0 0010:
	_king_engine_prespawn_player_update (0000)
000A09C0 0040:
	_king_get_score (0000)
000A0A00 0090:
	_render_dynamic_quad_initialize (0000)
000A0A90 02b0:
	_render_dynamic_quad (0000)
000A0D40 0040:
	_king_get_score_string (0000)
000A0D80 0060:
	_king_get_score_header_string (0000)
000A0DE0 0030:
	_king_get_team_score_string (0000)
000A0E10 0020:
	_king_engine_goal_matches_player (0000)
000A0E30 0070:
	_find_next_hill (0000)
000A0EA0 0110:
	_king_engine_initialize_for_new_map (0000)
000A0FB0 03a0:
	_king_engine_post_rasterize (0000)
000A1350 0110:
	_king_engine_update (0000)
0025BDC0 000d:
	??_C@_0N@DGPCNCJC@NULL?5?$CB?$DN?5flag?$AA@ (0000)
0025BDD0 0027:
	??_C@_0CH@IHPMMFJJ@c?3?2halo?2SOURCE?2game?2game_engine_@ (0000)
0025BDF8 0023:
	??_C@_0CD@DHKICDEA@king_globals?4hill_point_count?5?$CB?$DN@ (0000)
0025BE1C 0014:
	??_C@_0BE@EEOONBNI@FAILED?5TO?5FIND?5HILL?$AA@ (0000)
0025BE30 000b:
	??_C@_0L@HEFOOJCG@crown_blue?$AA@ (0000)
0025BE3C 0038:
	??_C@_0DI@GPEOKAOO@failed?5to?5find?5hill?5?$CD?$CFd?5most?5lik@ (0000)
002DE488 0088:
	_king_engine (0000)
0043E948 0230:
	_bss_0043e948 (0000)
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
#include "shaders.h"
#include "network_messages.h"
#include "text_group.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "render_debug.h"
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
