/*
GAME_ENGINE_CTF.C

symbols in this file:
0009EB60 0060:
	_create_the_flag_at_position (0000)
0009EBC0 0010:
	_ctf_engine_dispose (0000)
0009EBD0 0060:
	_create_the_flag (0000)
0009EC30 0040:
	_ctf_single_flag_what_is_up_message (0000)
0009EC70 0010:
	_ctf_engine_dispose_from_old_map (0000)
0009EC80 0020:
	_ctf_engine_player_added (0000)
0009ECA0 0010:
	_ctf_engine_game_ending (0000)
0009ECB0 0010:
	_ctf_engine_game_starting (0000)
0009ECC0 0010:
	_ctf_engine_statistics_append (0000)
0009ECD0 0010:
	_ctf_engine_handle_client_message (0000)
0009ECE0 0010:
	_ctf_engine_handle_server_message (0000)
0009ECF0 0010:
	_ctf_engine_pregame_post_rasterize (0000)
0009ED00 0010:
	_ctf_engine_post_rasterize (0000)
0009ED10 00c0:
	_player_score (0000)
0009EDD0 0040:
	_ctf_flag_failure_sound (0000)
0009EE10 0070:
	_get_player_with_this_flag (0000)
0009EE80 0080:
	_ctf_engine_allow_pick_up (0000)
0009EF00 0010:
	_ctf_engine_player_damaged_player (0000)
0009EF10 0010:
	_ctf_engine_player_killed_player (0000)
0009EF20 03a0:
	_ctf_engine_display_score (0000)
0009F2C0 0010:
	_ctf_engine_prespawn_player_update (0000)
0009F2D0 0010:
	_ctf_state_message_update_warning (0000)
0009F2E0 0040:
	_ctf_sound_update_warning (0000)
0009F320 0020:
	_ctf_set_flag_warning (0000)
0009F340 0040:
	_ctf_weapon_drop (0000)
0009F380 0040:
	_ctf_get_score (0000)
0009F3C0 0010:
	_ctf_test_flag (0000)
0009F3D0 0040:
	_ctf_get_score_string (0000)
0009F410 0060:
	_ctf_get_score_header_string (0000)
0009F470 0030:
	_ctf_get_team_score_string (0000)
0009F4A0 03d0:
	_ctf_engine_initialize_for_new_map (0000)
0009F870 0050:
	_weapon_reset_flag (0000)
0009F8C0 0090:
	_player_reset_flag (0000)
0009F950 0050:
	_in_scoring_range (0000)
0009F9A0 0150:
	_ctf_engine_player_update (0000)
0009FAF0 0240:
	_ctf_engine_weapon_update (0000)
0009FD30 0080:
	_ctf_engine_update (0000)
0009FDB0 0160:
	_ctf_weapon_pickup (0000)
0009FF10 0120:
	_ctf_engine_starting_location_rating (0000)
0025BBC4 000f:
	??_C@_0P@HPMPGFLN@created?5a?5flag?$AA@ (0000)
0025BBD4 001a:
	??_C@_0BK@JCPFMGLB@failed?5to?5create?5the?5flag?$AA@ (0000)
0025BBF0 0015:
	??_C@_0BF@KKINPGPB@NONE?5?$CB?$DN?5weapon_index?$AA@ (0000)
0025BC08 0026:
	??_C@_0CG@KLNGAFMF@c?3?2halo?2SOURCE?2game?2game_engine_@ (0000)
0025BC30 0013:
	??_C@_0BD@MAOGAFHI@NONE?5?$CB?$DN?5team_index?$AA@ (0000)
0025BC44 0018:
	??_C@_0BI@HLCGDKNJ@game_engine_can_score?$CI?$CJ?$AA@ (0000)
0025BC60 004b:
	??_C@_0EL@BLLOIBND@NETGAME_FLAG_WARNING?5starting?5lo@ (0000)
0025BCAC 003c:
	??_C@_0DM@LOHOHMDO@NETGAME_FLAG_WARNING?5starting?5lo@ (0000)
0025BCE8 002f:
	??_C@_0CP@KMKIMAED@?$CIflag_to_create?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIflag_t@ (0000)
0025BD18 0024:
	??_C@_0CE@FKOBANOL@failed?5to?5find?5one?5of?5the?5ctf?5fl@ (0000)
0025BD3C 001d:
	??_C@_0BN@PNGPDONM@ctf?5started?5up?5without?5teams?$AA@ (0000)
0025BD5C 0013:
	??_C@_0BD@CBKEKJFH@NONE?5?$CB?$DN?5unit_index?$AA@ (0000)
0025BD70 0036:
	??_C@_0DG@PMEHEDDI@weapon?9?$DOobject?4owner_team_index?5@ (0000)
0025BDA8 000a:
	??_C@_09NOBKAKN@flag_blue?$AA@ (0000)
0025BDB8 0008:
	__real@3fd51eb860000000 (0000)
002DE400 0088:
	_ctf_engine (0000)
0043E914 0030:
	_bss_0043e914 (0000)
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
#include "text_group.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "main.h"
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
