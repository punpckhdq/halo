/*
GAME_SOUND.C

symbols in this file:
001B6B20 0030:
	_game_sound_initialize (0000)
001B6B50 0020:
	_game_sound_dispose (0000)
001B6B70 0030:
	_game_sound_initialize_for_new_map (0000)
001B6BA0 00b0:
	_game_sound_clear (0000)
001B6C50 00d0:
	_game_sound_restore (0000)
001B6D20 0100:
	_game_looping_sound_new (0000)
001B6E20 00a0:
	_game_looping_sound_delete (0000)
001B6EC0 00b0:
	_unattached_impulse_sound_new (0000)
001B6F70 0080:
	_unspatialized_impulse_sound_new (0000)
001B6FF0 0050:
	_scripted_sound_time (0000)
001B7040 0050:
	_scripted_sound_stop (0000)
001B7090 00b0:
	_scripted_foley_predict (0000)
001B7140 0070:
	_scripted_looping_sound_set_scale (0000)
001B71B0 0050:
	_scripted_looping_sound_set_alternate (0000)
001B7200 0060:
	_unattached_looping_sound_start (0000)
001B7260 0030:
	_unattached_looping_sound_stop (0000)
001B7290 0270:
	_code_001b7290 (0000)
001B7500 00f0:
	_track_object_impulse_sound (0000)
001B75F0 0040:
	_game_sound_set_mouth_aperture (0000)
001B7630 00f0:
	_code_001b7630 (0000)
001B7720 0070:
	_code_001b7720 (0000)
001B7790 0070:
	_code_001b7790 (0000)
001B7800 0060:
	_code_001b7800 (0000)
001B7860 00d0:
	_code_001b7860 (0000)
001B7930 0030:
	_game_sound_dispose_from_old_map (0000)
001B7960 0110:
	_object_impulse_sound_new (0000)
001B7A70 0160:
	_scripted_sound_new (0000)
001B7BD0 0060:
	_scripted_looping_sound_stop (0000)
001B7C30 01d0:
	_game_sound_update (0000)
001B7E00 0200:
	_compute_sound_obstruction (0000)
001B8000 0090:
	_scripted_looping_sound_start (0000)
002AA9B4 0013:
	??_C@_0BD@LHEMKJEF@game?5sound?5globals?$AA@ (0000)
002AA9C8 0016:
	??_C@_0BG@BOAENECE@object?5looping?5sounds?$AA@ (0000)
002AA9E0 003a:
	??_C@_0DK@LEFFPEOA@TEST_FLAG?$CIsound?9?$DOflags?0?5_game_lo@ (0000)
002AAA1C 0022:
	??_C@_0CC@BGEOADFC@c?3?2halo?2SOURCE?2sound?2game_sound?4@ (0000)
002AAA40 000c:
	??_C@_0M@GMDOBMHG@marker_name?$AA@ (0000)
002AAA50 0075:
	??_C@_0HF@HCOEOKBN@definition?9?$DOruntime_scripting_so@ (0000)
002AAAC8 003f:
	??_C@_0DP@OPHINILO@definition?9?$DOruntime_scripting_so@ (0000)
002AAB08 0019:
	??_C@_0BJ@CPMEKCKK@scale?$DO?$DN0?4f?5?$CG?$CG?5scale?$DM?$DN1?4f?$AA@ (0000)
002AAB28 0044:
	??_C@_0EE@BEMDGHLB@TEST_FLAG?$CIdefinition?9?$DOflags?0?5_lo@ (0000)
002AAB6C 0010:
	??_C@_0BA@EPHFINEK@attachment_data?$AA@ (0000)
002AAB80 0064:
	??_C@_0GE@GAKNCJMB@location?9?$DOcluster_index?$DO?$DNNONE?5?$CG?$CG@ (0000)
002AABE4 0014:
	??_C@_0BE@DCLHABOE@position?5?$CG?$CG?5forward?$AA@ (0000)
002AABF8 0041:
	??_C@_0EB@JEFKCLHK@source?9?$DOspatialization_mode?$DN?$DN_so@ (0000)
002AAC3C 0030:
	??_C@_0DA@LJCJMFEJ@definition?9?$DOruntime_scripting_so@ (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
