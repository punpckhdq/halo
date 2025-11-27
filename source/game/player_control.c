/*
PLAYER_CONTROL.C

symbols in this file:
000A56F0 0050:
	_player_control_get (0000)
000A5740 0020:
	_player_control_initialize (0000)
000A5760 0010:
	_player_control_dispose (0000)
000A5770 0010:
	_player_control_dispose_from_old_map (0000)
000A5780 0020:
	_code_000a5780 (0000)
000A57A0 0030:
	_scripted_player_control_set_camera_control (0000)
000A57D0 0060:
	_player_control_inhibit_buttons (0000)
000A5830 00f0:
	_evaluate_piecewise_linear_function (0000)
000A5920 0010:
	_code_000a5920 (0000)
000A5930 0060:
	_player_control_get_aiming_unit_index (0000)
000A5990 0070:
	_player_control_get_target_object_index (0000)
000A5A00 00b0:
	_player_control_get_field_of_view (0000)
000A5AB0 0130:
	_player_control_get_unit_camera_info (0000)
000A5BE0 0050:
	_player_control_get_unit_index (0000)
000A5C30 0080:
	_player_control_get_desired_weapon (0000)
000A5CB0 0050:
	_player_control_get_autoaim_level (0000)
000A5D00 0040:
	_unit_get_local_player_index (0000)
000A5D40 0050:
	_players_unzoom_all (0000)
000A5D90 0050:
	_player_control_unzoom (0000)
000A5DE0 0020:
	_player_control_get_zoom_level (0000)
000A5E00 0020:
	_player_control_action_test_reset (0000)
000A5E20 0020:
	_player_control_action_test_accept (0000)
000A5E40 0020:
	_player_control_action_test_back (0000)
000A5E60 0020:
	_player_control_action_test_action (0000)
000A5E80 0010:
	_player_control_action_test_jump (0000)
000A5E90 0010:
	_player_control_action_test_primary_trigger (0000)
000A5EA0 0010:
	_player_control_action_test_grenade_trigger (0000)
000A5EB0 0010:
	_player_control_action_test_zoom (0000)
000A5EC0 0020:
	_player_control_action_test_move_relative_all_directions (0000)
000A5EE0 0020:
	_player_control_action_test_look_relative_all_directions (0000)
000A5F00 0010:
	_player_control_action_test_look_relative_left (0000)
000A5F10 0010:
	_player_control_action_test_look_relative_right (0000)
000A5F20 0010:
	_player_control_action_test_look_relative_up (0000)
000A5F30 0010:
	_player_control_action_test_look_relative_down (0000)
000A5F40 0200:
	_code_000a5f40 (0000)
000A6140 0040:
	_signed_angular_difference (0000)
000A6180 0050:
	_limit2d (0000)
000A61D0 0040:
	_interpolate_scalar (0000)
000A6210 0120:
	_player_control_set_facing (0000)
000A6330 00f0:
	_player_control_new_unit (0000)
000A6420 0d80:
	_code_000a6420 (0000)
000A71A0 00e0:
	_player_control_get_facing_angles (0000)
000A7280 0030:
	_player_control_get_facing_direction (0000)
000A72B0 0050:
	_player_control_set_desired_weapon (0000)
000A7300 0630:
	_code_000a7300 (0000)
000A7930 0090:
	_player_control_initialize_for_new_map (0000)
000A79C0 0620:
	_code_000a79c0 (0000)
000A7FE0 0080:
	_player_control_update (0000)
000A8060 0040:
	_player_control_permanent_impulse (0000)
0025C614 0016:
	??_C@_0BG@KLOMMKO@player_control_update?$AA@ (0000)
0025C62C 000c:
	_MOUSE_YAW_SCALE (0000)
	_MOUSE_PITCH_SCALE (0004)
0025C638 0025:
	??_C@_0CF@CDJDDDIC@c?3?2halo?2SOURCE?2game?2player_contr@ (0000)
0025C660 0017:
	??_C@_0BH@GDMLDNGJ@player?5control?5globals?$AA@ (0000)
0025C678 0046:
	??_C@_0EG@FFIEHMMD@function?5?$CG?$CG?5low_index?$DO?$DN0?5?$CG?$CG?5low_@ (0000)
0025C6C0 0004:
	__real@3f9c61aa (0000)
0025C6C4 000c:
	??_C@_0M@KHNJLLND@camera_info?$AA@ (0000)
0025C6D0 0004:
	__real@c0490fdb (0000)
0025C6D4 0023:
	??_C@_0CD@PHGHILFD@player_control?9?$DOdesired_angles?4y@ (0000)
0025C6F8 0025:
	??_C@_0CF@JIDICGJ@player_control?9?$DOdesired_angles?4p@ (0000)
0025C720 0017:
	??_C@_0BH@BJMHFMPG@input?9?$DOprimary_trigger?$AA@ (0000)
0025C738 0004:
	__real@3f75dcc7 (0000)
0025C73C 0004:
	__real@3d567750 (0000)
0025C740 0004:
	__real@bd567750 (0000)
0025C744 0004:
	__real@3dd67750 (0000)
0025C748 0004:
	__real@bdd67750 (0000)
0025C74C 0027:
	??_C@_0CH@OIFLGJJD@constants?9?$DOlook_acceleration_tim@ (0000)
0025C774 0021:
	??_C@_0CB@KGNPCEEG@constants?9?$DOlook_function?4count?$DO1@ (0000)
0025C798 002e:
	??_C@_0CO@HHADHJPE@valid_euler_angles2d?$CI?$CGplayer?9?$DOde@ (0000)
0025C7C8 0004:
	__real@bfbf0243 (0000)
0025C7CC 0004:
	__real@3fbf0243 (0000)
0025C7D0 0004:
	__real@3c490fdb (0000)
0025C7D4 0004:
	__real@bc490fdb (0000)
0025C7D8 0004:
	__real@3da3d70a (0000)
0025C7DC 001d:
	??_C@_0BN@JFNDAOJI@player?9?$DOdesired_angles?4pitch?$AA@ (0000)
0025C800 0008:
	__real@3fe45f3060000000 (0000)
0025C808 001a:
	??_C@_0BK@GKNPAGDI@action?4desired_facing?4yaw?$AA@ (0000)
0025C824 001c:
	??_C@_0BM@BMIDFCCA@action?4desired_facing?4pitch?$AA@ (0000)
0025C840 0018:
	??_C@_0BI@OPEALALJ@player?9?$DOprimary_trigger?$AA@ (0000)
0025C858 001b:
	??_C@_0BL@MOACFICP@player?9?$DOdesired_angles?4yaw?$AA@ (0000)
0025C874 0006:
	??_C@_05NCGEDJPM@delta?$AA@ (0000)
002DE808 0600:
	_player_autoaim_flag (0000)
	_player_magnetism_flag (0001)
	_controls_swapped (0002)
	_player_look_zoomed_scale (0004)
0043EE30 002b:
	_bss_0043ee30 (0000)
	_debug_input_target (0004)
	_player_look_yaw_rate (0008)
	_player_look_pitch_rate (0018)
	_controls_enable_crouch (0028)
	_controls_enable_doubled_spin (0029)
	_controls_swap_doubled_spin_state (002a)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
