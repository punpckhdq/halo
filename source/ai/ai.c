/*
AI.C

symbols in this file:
0002DC80 0080:
	_ai_initialize (0000)
0002DD00 0030:
	_ai_dispose (0000)
0002DD30 0040:
	_ai_dispose_from_old_map (0000)
0002DD70 0010:
	_ai_place (0000)
0002DD80 0040:
	_ai_globals_ai_active (0000)
0002DDC0 0050:
	_ai_globals_dialogue_triggers_enabled (0000)
0002DE10 0050:
	_ai_globals_grenades_enabled (0000)
0002DE60 00b0:
	_ai_get_major_upgrade_chance (0000)
0002DF10 0070:
	_ai_adjust_damage (0000)
0002DF80 00d0:
	_ai_erase (0000)
0002E050 00c0:
	_ai_release_inactive_swarms (0000)
0002E110 0040:
	_code_0002e110 (0000)
0002E150 0150:
	_ai_find_inactive_encounters (0000)
0002E2A0 0130:
	_ai_release_inactive_encounters (0000)
0002E3D0 0070:
	_ai_try_vehicle_eviction (0000)
0002E440 0080:
	_ai_get_responsible_unit (0000)
0002E4C0 0090:
	_ai_handle_death (0000)
0002E550 00d0:
	_ai_handle_killing_spree (0000)
0002E620 0140:
	_ai_handle_allegiance_status_changed (0000)
0002E760 0130:
	_ai_handle_allegiance_broken_notification (0000)
0002E890 00e0:
	_ai_update_team_status (0000)
0002E970 0100:
	_ai_handle_bump (0000)
0002EA70 0110:
	_ai_handle_damage (0000)
0002EB80 0120:
	_code_0002eb80 (0000)
0002ECA0 0070:
	_ai_create_mounted_weapons_for_unit (0000)
0002ED10 0160:
	_ai_handle_deleted_object (0000)
0002EE70 0140:
	_ai_handle_unit_effect (0000)
0002EFB0 0040:
	_ai_handle_enter_vehicle (0000)
0002EFF0 0060:
	_ai_handle_exit_vehicle (0000)
0002F050 0040:
	_code_0002f050 (0000)
0002F090 0500:
	_ai_disconnect_from_structure_bsp (0000)
0002F590 00c0:
	_ai_reconnect_to_structure_bsp (0000)
0002F650 0050:
	_ai_get_race_from_team_index (0000)
0002F6A0 00f0:
	_ai_initialize_for_new_map (0000)
0002F790 00d0:
	_ai_update (0000)
0002F860 0170:
	_ai_consider_major_upgrade (0000)
0002F9D0 0060:
	_code_0002f9d0 (0000)
0002FA30 0170:
	_code_0002fa30 (0000)
0002FBA0 0150:
	_ai_test_line_of_fire (0000)
0002FCF0 03f0:
	_ai_test_line_of_sight (0000)
000300E0 03b0:
	_ai_test_ballistic_line_of_fire (0000)
00030490 0510:
	_ai_handle_editing (0000)
000309A0 0210:
	_code_000309a0 (0000)
00030BB0 0010:
	_ai_enemies_can_see_player (0000)
00030BC0 0010:
	_ai_enemies_attacking_player (0000)
00030BD0 046a:
	_ai_handle_spatial_effect (0000)
00247120 0006:
	??_C@_05PMBAOCDH@quiet?$AA@ (0000)
00247128 0006:
	??_C@_05BAOFOMKD@shout?$AA@ (0000)
00247130 0005:
	??_C@_04FFPBHBOD@loud?$AA@ (0000)
00247138 0007:
	??_C@_06HIDOEFBI@medium?$AA@ (0000)
00247140 0007:
	??_C@_06MJMIHDAA@silent?$AA@ (0000)
00247148 000a:
	??_C@_09EPLOMCJK@ai_update?$AA@ (0000)
00247154 000b:
	??_C@_0L@MPBGDIHA@ai_globals?$AA@ (0000)
00247160 0017:
	??_C@_0BH@OFFADIKK@c?3?2halo?2SOURCE?2ai?2ai?4c?$AA@ (0000)
00247178 000b:
	??_C@_0L@OAAMDEHE@ai?5globals?$AA@ (0000)
00247184 002a:
	??_C@_0CK@KLHIAEEK@force_major?5?$CG?$CG?5is_random?5?$CG?$CG?5rand@ (0000)
002471B0 000f:
	??_C@_0P@DJKGCMDL@?$CFd?5swarm?5units?$AA@ (0000)
002471C0 0026:
	??_C@_0CG@KJABFHAE@result_description?5?$CG?$CG?5more_to_re@ (0000)
002471E8 0045:
	??_C@_0EF@INALGHA@working_memory_size?5?$DO?$DN?5sizeof?$CIst@ (0000)
00247230 0018:
	??_C@_0BI@IJNFHBDG@encounter?5?$CFs?5?$CI?$CFd?5units?$CJ?$AA@ (0000)
00247248 0017:
	??_C@_0BH@EHECFHBL@encounterless?9actor?5?$CFs?$AA@ (0000)
00247260 0015:
	??_C@_0BF@IJHLDNKE@?$CFs?5killing?5spree?3?5?$CFd?$AA@ (0000)
00247278 0009:
	??_C@_08MAAODMKK@player?$CFd?$AA@ (0000)
00247284 0028:
	??_C@_0CI@COCLKLAG@allegiance?5between?5teams?5?$CFs?5and?5@ (0000)
002472AC 0009:
	??_C@_08ELLPFBJE@reformed?$AA@ (0000)
002472B8 0007:
	??_C@_06HKHHAOPL@broken?$AA@ (0000)
002472C0 000d:
	??_C@_0N@PLDCHNDI@?5permanently?$AA@ (0000)
002472D0 0036:
	??_C@_0DG@IOHAMJKK@allegiance?5between?5teams?5?$CFs?5and?5@ (0000)
00247308 005f:
	??_C@_0FP@HJJPEFKF@WARNING?3?5cannot?5create?5mounted?5w@ (0000)
00247368 0038:
	??_C@_0DI@LLKOJPAJ@effect_type?$DO?$DN0?5?$CG?$CG?5effect_type?$DMNU@ (0000)
002473A0 002f:
	??_C@_0CP@KEBPDGDE@volume?$DO?$DN0?5?$CG?$CG?5volume?$DMNUMBER_OF_AI@ (0000)
002473D0 001a:
	??_C@_0BK@IPBEOPCE@actor?9?$DOmeta?4encounterless?$AA@ (0000)
002473EC 0029:
	??_C@_0CJ@ECGMOGMP@?$CFs?3?5disconnect?5and?5transition?5to@ (0000)
00247418 0030:
	??_C@_0DA@HNMNMDJM@?$CFs?3?5?$CFd?5of?5?$CFd?5units?5outside?5PVS?0?5@ (0000)
00247448 0010:
	??_C@_0BA@OOMJBPJL@?5?$CIdeleted?5some?$CJ?$AA@ (0000)
00247458 002b:
	??_C@_0CL@CHFDAENO@?$CFs?3?5no?5units?5inside?5PVS?0?5do?5not?5@ (0000)
00247484 002f:
	??_C@_0CP@IOHLMJDL@?$CFs?3?5all?5swarm?5inside?5PVS?0?5transi@ (0000)
002474B8 0041:
	??_C@_0EB@GAAEJBAL@components_outside_pvs_count?5?$DM?5M@ (0000)
002474FC 0027:
	??_C@_0CH@BCDOMGFK@target_prop?9?$DOparent_prop_index?5?$CB@ (0000)
00247524 0003:
	??_C@_02DKCKIIND@?$CFs?$AA@ (0000)
00247528 0056:
	??_C@_0FG@MCBCGPDK@?$CFs?1?$CFs?5major?5upgrade?5chance?5?$CF?42f?3@ (0000)
00247580 0003:
	??_C@_02NFLNFON@NO?$AA@ (0000)
00247584 0004:
	??_C@_03IPHJCBHH@YES?$AA@ (0000)
00247588 0004:
	__real@beaaaaab (0000)
00247590 005d:
	??_C@_0FN@FHDONHFP@adjust_encounter?9?$DOplatoon_base?5?$CL@ (0000)
002475F0 0055:
	??_C@_0FF@ENLKAKCD@adjust_encounter?9?$DOplatoon_base?5?$DO@ (0000)
00247648 0057:
	??_C@_0FH@PKEMGHNJ@adjust_encounter?9?$DOsquad_base?5?$CL?5a@ (0000)
002476A0 004f:
	??_C@_0EP@KKLKNEDM@adjust_encounter?9?$DOsquad_base?5?$DO?$DN?5@ (0000)
002476F0 003c:
	??_C@_0DM@GLFLBCJN@editing?5caused?5an?5overflow?5of?5MA@ (0000)
0024772C 003a:
	??_C@_0DK@JGCJOKPE@editing?5caused?5an?5overflow?5of?5MA@ (0000)
00247768 004d:
	??_C@_0EN@KGPGOCFG@?$CIdistance_to_effect?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIdi@ (0000)
002477B8 0007:
	??_C@_06MCBHDBOL@submit?$AA@ (0000)
002477C0 003b:
	??_C@_0DL@MFAGBNKG@effect_type?$DO?$DN0?5?$CG?$CG?5effect_type?$DMNU@ (0000)
002477FC 0008:
	??_C@_07NBEECHDO@count?$DO0?$AA@ (0000)
002B6CF0 0618:
	_data_002b6cf0 (0000)
	_ai_sound_volume_enum (060c)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
