/*
EFFECTS.C

symbols in this file:
0008ACC0 0050:
	_effects_initialize (0000)
0008AD10 0020:
	_effects_initialize_for_new_map (0000)
0008AD30 0020:
	_effects_dispose_from_old_map (0000)
0008AD50 0020:
	_effects_dispose (0000)
0008AD70 0050:
	_code_0008ad70 (0000)
0008ADC0 00c0:
	_effect_delete (0000)
0008AE80 0100:
	_effects_stop_on_first_person_weapon (0000)
0008AF80 0080:
	_effects_information_get (0000)
0008B000 0010:
	_effects_disconnect_from_structure_bsp (0000)
0008B010 0030:
	_code_0008b010 (0000)
0008B040 0020:
	_code_0008b040 (0000)
0008B060 0080:
	_code_0008b060 (0000)
0008B0E0 0080:
	_code_0008b0e0 (0000)
0008B160 0080:
	_code_0008b160 (0000)
0008B1E0 0020:
	_code_0008b1e0 (0000)
0008B200 0090:
	_code_0008b200 (0000)
0008B290 0080:
	_code_0008b290 (0000)
0008B310 00e0:
	_code_0008b310 (0000)
0008B3F0 0050:
	_code_0008b3f0 (0000)
0008B440 00b0:
	_code_0008b440 (0000)
0008B4F0 0010:
	_real_local_random (0000)
0008B500 0020:
	_local_random_direction3d (0000)
0008B520 0090:
	_effect_stop (0000)
0008B5B0 00c0:
	_effects_reconnect_to_structure_bsp (0000)
0008B670 01f0:
	_dangerous_effects_near_player (0000)
0008B860 00e0:
	_code_0008b860 (0000)
0008B940 0160:
	_code_0008b940 (0000)
0008BAA0 00b0:
	_code_0008baa0 (0000)
0008BB50 00b0:
	_code_0008bb50 (0000)
0008BC00 0760:
	_code_0008bc00 (0000)
0008C360 03e0:
	_code_0008c360 (0000)
0008C740 00b0:
	_effects_start_on_first_person_weapon (0000)
0008C7F0 0190:
	_code_0008c7f0 (0000)
0008C980 0250:
	_code_0008c980 (0000)
0008CBD0 00b0:
	_code_0008cbd0 (0000)
0008CC80 0530:
	_code_0008cc80 (0000)
0008D1B0 00f0:
	_effect_new_looping (0000)
0008D2A0 0210:
	_effect_new_from_object (0000)
0008D4B0 02a0:
	_effect_new_attached_from_markers (0000)
0008D750 0240:
	_effect_new_unattached_from_markers (0000)
0008D990 0090:
	_effects_update (0000)
0025A424 000f:
	??_C@_0P@FBPJPKBN@effects_update?$AA@ (0000)
0025A434 0021:
	??_C@_0CB@HMDGHGFM@couldn?8t?5allocate?5effect?5globals@ (0000)
0025A458 0010:
	??_C@_0BA@DBNCDKDD@effect?5location?$AA@ (0000)
0025A468 0007:
	??_C@_06HOPIODND@effect?$AA@ (0000)
0025A470 0021:
	??_C@_0CB@KCIIEPGL@c?3?2halo?2SOURCE?2effects?2effects?4c@ (0000)
0025A494 0015:
	??_C@_0BF@IACBPFOC@location_datum_index?$AA@ (0000)
0025A4AC 0013:
	??_C@_0BD@IEHBIGCF@effect_index?$CB?$DNNONE?$AA@ (0000)
0025A4C0 004e:
	??_C@_0EO@BKJIDONM@cannot?5create?5objects?0?5lights?0?5o@ (0000)
0025A510 000f:
	??_C@_0P@HFFHMHLJ@?$CGeffect?9?$DOcolor?$AA@ (0000)
0025A520 000f:
	??_C@_0P@MDAIOCAG@count_delta?$DO?$DN0?$AA@ (0000)
0025A530 001c:
	??_C@_0BM@HEALEJFA@effect?5?$CFs?5has?5a?5bad?5part?5?$CFs?$AA@ (0000)
0025A54C 0021:
	??_C@_0CB@LPDGPLO@effect?9?$DOlocal_player_index?$DN?$DNNONE@ (0000)
0025A570 0009:
	??_C@_08FLABJHKG@?$CGforward?$AA@ (0000)
0025A57C 002c:
	??_C@_0CM@MOGGIEEI@?$CGmarker_list?9?$DOforwards?$FLeffect_ma@ (0000)
0025A5A8 0041:
	??_C@_0EB@KCJAOKOI@effect_marker_index?$DO?$DN0?5?$CG?$CG?5effect@ (0000)
0025A5EC 001b:
	??_C@_0BL@MLPMKFCB@created?5?$CFsviolent?5?$CFs?5on?5?$CFs?$AA@ (0000)
0025A608 0004:
	??_C@_03LNLHGKOH@non?$AA@ (0000)
0025A60C 0018:
	??_C@_0BI@JECIIDID@scale_b?5?$CFf?5not?5in?5?$FL0?01?$FN?$AA@ (0000)
0025A624 0018:
	??_C@_0BI@HOKOFOOB@scale_a?5?$CFf?5not?5in?5?$FL0?01?$FN?$AA@ (0000)
0025A63C 0010:
	??_C@_0BA@BNLBDOE@marker_forwards?$AA@ (0000)
0025A64C 000e:
	??_C@_0O@BFOEKDJK@marker_points?$AA@ (0000)
0025A65C 000f:
	??_C@_0P@DBOLECLB@marker_count?$DO0?$AA@ (0000)
0025A670 0047:
	??_C@_0EH@MLPNNEFC@?$CBtranslational_velocity?5?$HM?$HM?5valid@ (0000)
002DD1A0 0600:
	_effects_corpse_nonviolent (0000)
0043D584 0005:
	_bss_0043d584 (0000)
	_debug_effects_nonviolent (0004)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
