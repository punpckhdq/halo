/*
ACTOR_FIRING_POSITION.C

symbols in this file:
00012610 0050:
	_code_00012610 (0000)
00012660 0010:
	_code_00012660 (0000)
00012670 00d0:
	_code_00012670 (0000)
00012740 0240:
	_code_00012740 (0000)
00012980 00e0:
	_code_00012980 (0000)
00012A60 0180:
	_code_00012a60 (0000)
00012BE0 00e0:
	_code_00012be0 (0000)
00012CC0 00c0:
	_code_00012cc0 (0000)
00012D80 00e0:
	_code_00012d80 (0000)
00012E60 0040:
	_code_00012e60 (0000)
00012EA0 0070:
	_code_00012ea0 (0000)
00012F10 0050:
	_code_00012f10 (0000)
00012F60 0110:
	_code_00012f60 (0000)
00013070 0120:
	_actor_get_firing_position_group (0000)
00013190 0060:
	_actor_clear_discarded_firing_positions (0000)
000131F0 00c0:
	_actor_discard_firing_position (0000)
000132B0 0050:
	_actor_firing_position_discarded (0000)
00013300 0650:
	_code_00013300 (0000)
00013950 01d0:
	_code_00013950 (0000)
00013B20 0290:
	_code_00013b20 (0000)
00013DB0 01d0:
	_code_00013db0 (0000)
00013F80 0090:
	_code_00013f80 (0000)
00014010 0210:
	_actor_nearby_firing_positions (0000)
00014220 1480:
	_actor_select_firing_position (0000)
000156A0 0240:
	_actor_active_select_firing_position (0000)
000158E0 0140:
	_actor_change_firing_position (0000)
00015A20 0460:
	_code_00015a20 (0000)
00244794 0068:
	_global_pre_evaluator_table (0000)
	_global_post_evaluator_table (0038)
002447FC 002e:
	??_C@_0CO@CPIODGNI@?$CIevaluation?5?$DO?$DN?50?40f?$CJ?5?$CG?$CG?5?$CIevaluat@ (0000)
0024482C 002a:
	??_C@_0CK@HINGLJIA@c?3?2halo?2SOURCE?2ai?2actor_firing_p@ (0000)
00244858 0004:
	__real@447a0000 (0000)
0024485C 0004:
	__real@41080000 (0000)
00244860 0004:
	__real@41700000 (0000)
00244864 0004:
	__real@40a00000 (0000)
00244868 0004:
	__real@41400000 (0000)
0024486C 0004:
	__real@40f00000 (0000)
00244870 0004:
	__real@41a00000 (0000)
00244874 0020:
	??_C@_0CA@KIJFLGCC@firing_position?9?$DOvalid?5?$DN?$DN?5valid?$AA@ (0000)
00244898 0048:
	??_C@_0EI@IDDMBEDB@?$CIindex2?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIindex2?5?$DM?5globa@ (0000)
002448E0 0048:
	??_C@_0EI@BDJPJEGE@?$CIindex1?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIindex1?5?$DM?5globa@ (0000)
00244928 002c:
	??_C@_0CM@BJJOKNGI@global_temporary_sort_firing_pos@ (0000)
00244954 003b:
	??_C@_0DL@JAGLMMAN@?$CIindex?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIindex?5?$DM?5NUMBER_@ (0000)
00244990 0004:
	__real@42dfec4b (0000)
00244994 0004:
	__real@c1700000 (0000)
00244998 0004:
	__real@42800000 (0000)
0024499C 0004:
	__real@3be38e39 (0000)
002449A0 0004:
	__real@44610000 (0000)
002449A4 0004:
	__real@40200000 (0000)
002449A8 0060:
	??_C@_0GA@KIPPIIL@?$CIactor?9?$DOdanger_zone?4danger_type?5@ (0000)
00244A08 0004:
	__real@3fb504f3 (0000)
00244A0C 0004:
	__real@40555555 (0000)
00244A10 0004:
	__real@42440000 (0000)
00244A14 0004:
	__real@41800000 (0000)
00244A18 001f:
	??_C@_0BP@KIOMHHMP@evaluation_context?9?$DOhas_target?$AA@ (0000)
00244A38 0052:
	??_C@_0FC@JPOFKNDC@?$CItest_surface_index?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIte@ (0000)
00244A90 005b:
	??_C@_0FL@LHFKPKFO@fp?9eval?5?$CFs?3?5encounter?5?$CF3d?5consid@ (0000)
00244AEC 0007:
	??_C@_06GFDMOFFM@pursue?$AA@ (0000)
00244AF4 0006:
	??_C@_05MGHOCOML@cover?$AA@ (0000)
00244AFC 0015:
	??_C@_0BF@JGHAJKFH@?$CBexpected_to_discard?$AA@ (0000)
00244B14 001a:
	??_C@_0BK@FJCPIFLD@firing_position_count?5?$DO?50?$AA@ (0000)
00244B30 0004:
	__real@43c80000 (0000)
00244B34 0035:
	??_C@_0DF@NLMOLOFE@encounter?5?$CFs?5has?5too?5many?5firing@ (0000)
00244B6C 0016:
	??_C@_0BG@HHMJFCBB@area_path_state_valid?$AA@ (0000)
00244B84 0004:
	__real@42a00000 (0000)
00244B88 0020:
	??_C@_0CA@EEMPDNKJ@?$CBactor?9?$DOinput?4vehicle_passenger?$AA@ (0000)
00244BA8 0046:
	??_C@_0EG@KOFLIKBB@?$CItotal_groups?5?$CG?5currently_allowe@ (0000)
00244BF0 001e:
	??_C@_0BO@EAEPOCDN@actor_index?5?$CB?$DN?5previous_owner?$AA@ (0000)
00244C10 0024:
	??_C@_0CE@DHIELLBI@actor?9?$DOmeta?4encounter_index?5?$CB?$DN?5N@ (0000)
00244C34 0004:
	__real@41440000 (0000)
00244C38 0039:
	??_C@_0DJ@OIJCIDAH@?$CGevaluation_context?9?$DOattack_vect@ (0000)
00319CA0 0008:
	_global_temporary_sort_firing_position_count (0000)
	_global_temporary_sort_firing_position_array (0004)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
