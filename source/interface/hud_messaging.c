/*
HUD_MESSAGING.C

symbols in this file:
000C3810 0080:
	_code_000c3810 (0000)
000C3890 0190:
	_code_000c3890 (0000)
000C3A20 0020:
	_hud_messaging_initialize (0000)
000C3A40 0030:
	_hud_messaging_initialize_for_new_map (0000)
000C3A70 0010:
	_hud_messaging_dispose_from_old_map (0000)
000C3A80 0010:
	_hud_messaging_dispose (0000)
000C3A90 0050:
	_scripted_hud_set_state_message (0000)
000C3AE0 0040:
	_scripted_hud_set_flashing_state (0000)
000C3B20 0040:
	_scripted_hud_restart_flashing (0000)
000C3B60 00a0:
	_scripted_hud_set_objective (0000)
000C3C00 0080:
	_scripted_hud_set_timer_time (0000)
000C3C80 0020:
	_scripted_hud_set_timer_warning_cutoff (0000)
000C3CA0 0060:
	_scripted_hud_set_timer_position (0000)
000C3D00 0020:
	_scripted_hud_show_timer (0000)
000C3D20 0050:
	_scripted_hud_pause_timer (0000)
000C3D70 0050:
	_scripted_hud_get_timer_ticks (0000)
000C3DC0 0030:
	_scripted_hud_time_code_show (0000)
000C3DF0 0040:
	_scripted_hud_time_code_start (0000)
000C3E30 0020:
	_scripted_hud_time_code_reset (0000)
000C3E50 02c0:
	_hud_render_timer (0000)
000C4110 00a0:
	_hud_set_state_message (0000)
000C41B0 0060:
	_hud_set_state_message_icon (0000)
000C4210 0070:
	_hud_set_state_message_text (0000)
000C4280 0070:
	_hud_enable_custom_state_message (0000)
000C42F0 0040:
	_hud_set_state_text (0000)
000C4330 00c0:
	_hud_messaging_get_objective (0000)
000C43F0 0080:
	_code_000c43f0 (0000)
000C4470 0030:
	_code_000c4470 (0000)
000C44A0 0040:
	_scripted_hud_messages_clear (0000)
000C44E0 0020:
	_hud_get_font_index (0000)
000C4500 0030:
	_hud_get_text_color (0000)
000C4530 0010:
	_hud_messaging_globals_update (0000)
000C4540 0080:
	_hud_print_message (0000)
000C45C0 00a0:
	_hud_add_item_message (0000)
000C4660 0070:
	_hud_broadcast_team_message (0000)
000C46D0 0b70:
	_hud_messaging_update (0000)
0026FEE4 000e:
	??_C@_0O@JAAAMDJP@hud?5messaging?$AA@ (0000)
0026FEF4 003f:
	??_C@_0DP@DEHGFPPE@trying?5to?5restart?5help?5text?5flas@ (0000)
0026FF34 002b:
	??_C@_0CL@KDPBOCMB@objective?5text?5MUST?5only?5be?5text@ (0000)
0026FF60 0008:
	__real@4004000000000000 (0000)
0026FF68 0029:
	??_C@_0CJ@FOFBAHGJ@c?3?2halo?2SOURCE?2interface?2hud_mes@ (0000)
0026FF94 0026:
	??_C@_0CG@FLCOGOLF@element?9?$DOtype?$DN?$DN_hud_message_type@ (0000)
0026FFBC 001a:
	??_C@_0BK@PKALLFKO@message?9?$DOelement_count?$DN?$DN1?$AA@ (0000)
0026FFD8 0022:
	??_C@_1CC@FMBGLMAK@?$AA?$DM?$AAn?$AAo?$AA?5?$AAb?$AAu?$AAt?$AAt?$AAo?$AAn?$AA?5?$AAi?$AAc?$AAo?$AAn?$AA?$DO?$AA?$AA@ (0000)
0026FFFC 0022:
	??_C@_0CC@OCKONGBA@help?5text?5cannot?5use?5custom?5icon@ (0000)
00270020 0037:
	??_C@_0DH@PPCCMLMB@help?5message?5using?5old?5code?4?5get@ (0000)
00270058 0014:
	??_C@_1BE@ELICABCK@?$AA?$DM?$AAu?$AAn?$AAk?$AAn?$AAo?$AAw?$AAn?$AA?$DO?$AA?$AA@ (0000)
0027006C 0037:
	??_C@_0DH@GJFOKBEC@global_scenario_get?$CI?$CJ?9?$DOcustom_ob@ (0000)
002700A4 0028:
	??_C@_0CI@PMMKILCE@custom_index?$DMNUMBER_OF_HUD_CUSTO@ (0000)
002700CC 0013:
	??_C@_0BD@LPFKFODL@show_state_message?$AA@ (0000)
002700E0 0054:
	??_C@_0FE@EGLPDJAB@hud_messaging_globals?9?$DOobjective@ (0000)
002E4C4C 001c:
	_data_002e4c4c (0000)
	_time_code_time (0010)
	_time_code_stop_time (0014)
00453AB8 0004:
	_bss_00453ab8 (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
