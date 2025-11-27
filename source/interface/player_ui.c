/*
PLAYER_UI.C

symbols in this file:
000CFC30 0020:
	_player_ui_dispose (0000)
000CFC50 0020:
	_player_ui_reset_single_player_local_player_controllers (0000)
000CFC70 0080:
	_player_ui_set_single_player_local_player_controller (0000)
000CFCF0 0050:
	_player_ui_get_single_player_local_player_controller (0000)
000CFD40 0030:
	_player_ui_get_single_player_local_player_from_controller (0000)
000CFD70 0050:
	_player_ui_local_player_joined_multiplayer_game (0000)
000CFDC0 0050:
	_player_ui_local_player_wants_to_play_multiplayer (0000)
000CFE10 0050:
	_player_ui_clear_multiplayer_autojoin_for_local_player (0000)
000CFE60 0030:
	_player_ui_autojoin_players_to_next_multiplayer_game (0000)
000CFE90 0020:
	_player_ui_clear_multiplayer_variant (0000)
000CFEB0 0060:
	_player_ui_get_active_player_profile (0000)
000CFF10 0030:
	_player_ui_get_active_player_profile_index (0000)
000CFF40 0050:
	_player_ui_get_last_single_player_level_played (0000)
000CFF90 0050:
	_player_ui_set_game_variant (0000)
000CFFE0 0050:
	_player_ui_game_variant_specified (0000)
000D0030 0050:
	_player_ui_rumble_disabled (0000)
000D0080 00a0:
	_player_ui_autolevel_enabled (0000)
000D0120 0040:
	_player_ui_get_path_to_local_player_profile_directory (0000)
000D0160 0060:
	_player_ui_remember_player1_profile (0000)
000D01C0 0040:
	_player_ui_get_player1_last_used_profile_index (0000)
000D0200 00b0:
	_player_ui_fast_setup_network_server (0000)
000D02B0 0050:
	_player_ui_edit_profile_is_default_profile (0000)
000D0300 0070:
	_player_ui_edit_profile_name_is_dirty (0000)
000D0370 0060:
	_player_ui_prompt_user_to_rename_edit_profile (0000)
000D03D0 0020:
	_player_ui_get_edit_player_profile (0000)
000D03F0 0020:
	_player_ui_get_edit_playlist_profile (0000)
000D0410 00f0:
	_player_ui_edit_profile_is_dirty (0000)
000D0500 0030:
	_player_ui_activate_all_solo_levels (0000)
000D0530 0050:
	_code_000d0530 (0000)
000D0580 0010:
	_player0_look_pitch_is_inverted (0000)
000D0590 0020:
	_player0_joystick_set_is_normal (0000)
000D05B0 0040:
	_code_000d05b0 (0000)
000D05F0 0210:
	_code_000d05f0 (0000)
000D0800 0010:
	_code_000d0800 (0000)
000D0810 0070:
	_code_000d0810 (0000)
000D0880 00a0:
	_player_ui_initialize (0000)
000D0920 00a0:
	_player_ui_clear_multiplayer_joins (0000)
000D09C0 0070:
	_player_ui_set_active_player_profile (0000)
000D0A30 00b0:
	_player_ui_begin_editing_profile (0000)
000D0AE0 01b0:
	_player_ui_save_profile (0000)
000D0C90 0010:
	_player_ui_end_editing_profile (0000)
000D0CA0 0070:
	_player0_look_invert_pitch (0000)
00270BDC 0019:
	??_C@_0BJ@DFIFBFHJ@invalid?5controller?5index?$AA@ (0000)
00270BF8 001b:
	??_C@_0BL@FPFMMDCA@invalid?5local?5player?5index?$AA@ (0000)
00270C14 0025:
	??_C@_0CF@DLLNKDKO@c?3?2halo?2SOURCE?2interface?2player_@ (0000)
00270C40 0050:
	??_C@_0FA@COKNKHJP@?$CIlocal_player_index?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIloca@ (0000)
00270C90 0065:
	??_C@_0GF@IIJJCBPM@?$CIlocal_player_index?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIloca@ (0000)
00270CF8 0008:
	??_C@_07IFGLIMJI@variant?$AA@ (0000)
00270D00 002f:
	??_C@_0CP@JOJHLPFO@player?51?5has?5no?5active?5player?5pr@ (0000)
00270D30 006a:
	??_C@_0GK@DKOJLEOE@failed?5to?5load?5network?5pregame?5s@ (0000)
00270D9C 002d:
	??_C@_0CN@JHOFCMHM@failed?5to?5initiate?5a?5multiplayer@ (0000)
00270DD0 0056:
	??_C@_0FG@FFFNJFPJ@ui?2shell?2main_menu?2multiplayer_t@ (0000)
00270E28 002a:
	??_C@_0CK@FMHFAECE@unknown?5saved?5game?5file?5type?5bei@ (0000)
00270E54 0028:
	??_C@_0CI@CHDBALJG@not?5currently?5editing?5a?5saved?5ga@ (0000)
00270E7C 0022:
	??_C@_0CC@KDPBKPKM@unknown?5profile?5type?5being?5edite@ (0000)
00270EA0 0008:
	??_C@_07LNMDFKEJ@profile?$AA@ (0000)
00270EA8 0035:
	??_C@_0DF@EJKBCKFH@failed?5to?5retrieve?5player?5profil@ (0000)
00270EE0 0037:
	??_C@_0DH@HABNMGLM@failed?5to?5retrieve?5playlist?5prof@ (0000)
00270F18 001f:
	??_C@_0BP@IGBDNJOH@invalid?5profile?5index?5?$CI?$CD?$CF08lX?$CJ?$AA@ (0000)
00270F38 0032:
	??_C@_0DC@OODOEIMJ@?$CD?$CD?$CD?5WARNING?3?5saving?5over?5a?5defau@ (0000)
00270F70 0049:
	??_C@_0EJ@JACFDNDO@cannot?5save?5over?5default?5profile@ (0000)
00270FBC 0027:
	??_C@_0CH@HGHAKCCE@failed?5to?5save?5renamed?5profile?5t@ (0000)
00270FE8 0046:
	??_C@_0EG@JJOABCL@?$CD?$CD?$CD?5WARNING?3?5saving?5player?5profi@ (0000)
00271030 0036:
	??_C@_0DG@JHHBBKEF@failed?5to?5save?5profile?5because?5w@ (0000)
00271068 001e:
	??_C@_0BO@KMODNFHM@ui?2shell?2strings?2temp_strings?$AA@ (0000)
002FD5A4 0004:
	_data_002fd5a4 (0000)
00453D00 0330:
	_player_ui_globals (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
