/*
UNITS.C

symbols in this file:
001977E0 0040:
	_units_initialize (0000)
00197820 0020:
	_units_initialize_for_new_map (0000)
00197840 0010:
	_units_dispose_from_old_map (0000)
00197850 0010:
	_units_dispose (0000)
00197860 0020:
	_unit_kill (0000)
00197880 0020:
	_unit_kill_silent (0000)
001978A0 0020:
	_unit_kill_no_statistics (0000)
001978C0 0010:
	_unit_delete (0000)
001978D0 0020:
	_units_update (0000)
001978F0 0180:
	_unit_export_function_values (0000)
00197A70 0070:
	_unit_persistent_control (0000)
00197AE0 01e0:
	_unit_get_seat_entrance_point (0000)
00197CC0 00e0:
	_unit_get_melee_range_and_ticks (0000)
00197DA0 0090:
	_unit_get_animation_frames_remaining (0000)
00197E30 0140:
	_code_00197e30 (0000)
00197F70 0020:
	_unit_get_zoom_level (0000)
00197F90 0080:
	_code_00197f90 (0000)
00198010 0040:
	_code_00198010 (0000)
00198050 0020:
	_code_00198050 (0000)
00198070 0060:
	_code_00198070 (0000)
001980D0 0060:
	_code_001980d0 (0000)
00198130 0040:
	_code_00198130 (0000)
00198170 0020:
	_code_00198170 (0000)
00198190 0060:
	_code_00198190 (0000)
001981F0 0040:
	_code_001981f0 (0000)
00198230 0040:
	_code_00198230 (0000)
00198270 0190:
	_unit_animation_start_action (0000)
00198400 01c0:
	_code_00198400 (0000)
001985C0 00c0:
	_unit_can_enter_seat (0000)
00198680 0070:
	_unit_impulse (0000)
001986F0 00d0:
	_unit_handle_weapon_state_change (0000)
001987C0 0250:
	_unit_record_damage (0000)
00198A10 00b0:
	_unit_test_spawning (0000)
00198AC0 0020:
	_unit_destroy (0000)
00198AE0 0040:
	_unit_get_head_position (0000)
00198B20 01a0:
	_unit_get_camera_position (0000)
00198CC0 0140:
	_unit_estimate_position (0000)
00198E00 0040:
	_unit_get_center_of_mass (0000)
00198E40 0190:
	_code_00198e40 (0000)
00198FD0 00d0:
	_code_00198fd0 (0000)
001990A0 00c0:
	_unit_test_animation_impulse (0000)
00199160 0080:
	_unit_get_aiming_unit_index (0000)
001991E0 0030:
	_unit_get_aiming_vector (0000)
00199210 0030:
	_unit_get_looking_vector (0000)
00199240 0020:
	_unit_get_facing_vector (0000)
00199260 00b0:
	_unit_inventory_next_grenade (0000)
00199310 0020:
	_unit_controllable (0000)
00199330 0040:
	_unit_set_controllable (0000)
00199370 0040:
	_unit_set_possessed (0000)
001993B0 0060:
	_unit_is_busy (0000)
00199410 0050:
	_unit_scripting_set_emotion_animation (0000)
00199460 0080:
	_unit_scripting_suspended (0000)
001994E0 0040:
	_unit_scripting_can_blink (0000)
00199520 0050:
	_unit_scripting_doesnt_drop_items (0000)
00199570 0110:
	_unit_scripting_vehicle_test_seat_list (0000)
00199680 00a0:
	_unit_scripting_vehicle_test_seat (0000)
00199720 0080:
	_unit_scripting_unit_riders (0000)
001997A0 0030:
	_unit_scripting_unit_driver (0000)
001997D0 0030:
	_unit_scripting_unit_gunner (0000)
00199800 0160:
	_unit_shield_sapping_update (0000)
00199960 00f0:
	_units_debug_get_next_unit (0000)
00199A50 00d0:
	_units_debug_get_closest_unit (0000)
00199B20 0120:
	_unit_debug_ninja_rope (0000)
00199C40 0060:
	_unit_set_user_animation (0000)
00199CA0 0070:
	_any_unit_is_dangerous (0000)
00199D10 00a0:
	_unit_can_see_point (0000)
00199DB0 0040:
	_unit_driven_by_ai (0000)
00199DF0 0040:
	_unit_gunned_by_ai (0000)
00199E30 0040:
	_unit_set_desired_flashlight_state (0000)
00199E70 0030:
	_unit_get_current_flashlight_state (0000)
00199EA0 0140:
	_unit_detach_from_parent (0000)
00199FE0 0070:
	_unit_seat_filled (0000)
0019A050 0060:
	_unit_seat_is_driver (0000)
0019A0B0 0060:
	_unit_seat_is_gunner (0000)
0019A110 0060:
	_unit_seat_allow_noncombatants (0000)
0019A170 00e0:
	_code_0019a170 (0000)
0019A250 0020:
	_unit_get_current_equipment (0000)
0019A270 0100:
	_unit_add_grenade_to_inventory (0000)
0019A370 0090:
	_unit_add_grenade_type_to_inventory (0000)
0019A400 0120:
	_unit_add_equipment_to_inventory (0000)
0019A520 0040:
	_unit_delete_current_equipment (0000)
0019A560 0080:
	_unit_delete_all_weapons (0000)
0019A5E0 0060:
	_unit_has_weapon_definition_index (0000)
0019A640 0030:
	_code_0019a640 (0000)
0019A670 0070:
	_unit_get_weapon_count (0000)
0019A6E0 0070:
	_unit_approve_weapon_pickup (0000)
0019A750 0070:
	_unit_get_grenade_count (0000)
0019A7C0 0060:
	_unit_get_current_grenade_type (0000)
0019A820 01d0:
	_code_0019a820 (0000)
0019A9F0 0310:
	_code_0019a9f0 (0000)
0019AD00 02d0:
	_code_0019ad00 (0000)
0019AFD0 0050:
	_code_0019afd0 (0000)
0019B020 0040:
	_code_0019b020 (0000)
0019B060 0050:
	_code_0019b060 (0000)
0019B0B0 00b0:
	_code_0019b0b0 (0000)
0019B160 0050:
	_code_0019b160 (0000)
0019B1B0 0080:
	_code_0019b1b0 (0000)
0019B230 0050:
	_code_0019b230 (0000)
0019B280 0190:
	_code_0019b280 (0000)
0019B410 00b0:
	_code_0019b410 (0000)
0019B4C0 0100:
	_code_0019b4c0 (0000)
0019B5C0 0040:
	_unit_handle_region_destroyed (0000)
0019B600 0080:
	_code_0019b600 (0000)
0019B680 02a0:
	_code_0019b680 (0000)
0019B920 0040:
	_unit_set_enterable_by_player (0000)
0019B960 0030:
	_unit_set_emotion (0000)
0019B990 0040:
	_unit_aim_without_turning (0000)
0019B9D0 0070:
	_unit_get_custom_animation_time (0000)
0019BA40 0030:
	_unit_is_playing_custom_animation (0000)
0019BA70 0170:
	_unit_start_user_animation (0000)
0019BBE0 0060:
	_unit_set_mouth_aperture (0000)
0019BC40 0060:
	_unit_inventory_get_must_be_readied_weapon (0000)
0019BCA0 0040:
	_unit_has_weapon (0000)
0019BCE0 0060:
	_unit_has_weapon_with_flag (0000)
0019BD40 00d0:
	_unit_start_running_blindly (0000)
0019BE10 0030:
	_unit_stop_running_blindly (0000)
0019BE40 0100:
	_unit_flame_to_death (0000)
0019BF40 0030:
	_unit_flying_through_air (0000)
0019BF70 04f0:
	_code_0019bf70 (0000)
0019C460 0200:
	_code_0019c460 (0000)
0019C660 0220:
	_code_0019c660 (0000)
0019C880 00d0:
	_unit_adjust_projectile_ray (0000)
0019C950 0200:
	_unit_render_debug (0000)
0019CB50 0580:
	_code_0019cb50 (0000)
0019D0D0 0020:
	_unit_abort_animation (0000)
0019D0F0 0290:
	_unit_find_nearby_seat (0000)
0019D380 0420:
	_unit_clip_to_aiming_bounds (0000)
0019D7A0 0060:
	_unit_inventory_get_weapon (0000)
0019D800 00b0:
	_unit_set_actively_controlled (0000)
0019D8B0 01a0:
	_vehicle_scripting_find_available_seats (0000)
0019DA50 0020:
	_unit_open (0000)
0019DA70 0020:
	_unit_close (0000)
0019DA90 0040:
	_unit_overcharged (0000)
0019DAD0 0030:
	_unit_set_seat (0000)
0019DB00 0080:
	_units_set_desired_flashlight_state (0000)
0019DB80 00a0:
	_code_0019db80 (0000)
0019DC20 0040:
	_unit_drop_current_equipment (0000)
0019DC60 0050:
	_unit_can_use_weapon (0000)
0019DCB0 00d0:
	_unit_approve_weapon_swap (0000)
0019DD80 0170:
	_code_0019dd80 (0000)
0019DEF0 0100:
	_unit_drop_current_weapon (0000)
0019DFF0 0030:
	_code_0019dff0 (0000)
0019E020 0020:
	_scripting_set_magic_base_seat (0000)
0019E040 0030:
	_unit_scripting_set_seat (0000)
0019E070 00c0:
	_unit_handle_deleted_object (0000)
0019E130 0250:
	_unit_cause_melee_damage (0000)
0019E380 0640:
	_unit_cause_player_melee_damage (0000)
0019E9C0 0030:
	_unit_stop_custom_animation (0000)
0019E9F0 0080:
	_unit_custom_animation_at_frame (0000)
0019EA70 0120:
	_code_0019ea70 (0000)
0019EB90 00a0:
	_unit_start_flaming_to_death (0000)
0019EC30 02e0:
	_code_0019ec30 (0000)
0019EF10 0090:
	_code_0019ef10 (0000)
0019EFA0 02e0:
	_code_0019efa0 (0000)
0019F280 03a0:
	_unit_control (0000)
0019F620 0780:
	_unit_preprocess_node_orientations (0000)
0019FDA0 0180:
	_unit_postprocess_node_matrices (0000)
0019FF20 06d0:
	_unit_euler_aiming_update (0000)
001A05F0 0090:
	_unit_has_animation_to_enter_seat (0000)
001A0680 0500:
	_unit_update_animation (0000)
001A0B80 00c0:
	_unit_unzoom (0000)
001A0C40 0050:
	_unit_get_zoom_magnification (0000)
001A0C90 0060:
	_code_001a0c90 (0000)
001A0CF0 0620:
	_code_001a0cf0 (0000)
001A1310 0120:
	_unit_start_animation_impulse (0000)
001A1430 0020:
	_unit_inventory_next_weapon (0000)
001A1450 0110:
	_unit_melee_attack_begin (0000)
001A1560 0090:
	_unit_leap_begin (0000)
001A15F0 00b0:
	_unit_unsuspecting (0000)
001A16A0 0130:
	_unit_add_weapon_to_inventory (0000)
001A17D0 01b0:
	_code_001a17d0 (0000)
001A1980 01d0:
	_unit_throw_grenade_begin (0000)
001A1B50 0030:
	_scripting_magic_melee_attack (0000)
001A1B80 0240:
	_unit_impact_melee_damage (0000)
001A1DC0 0140:
	_unit_place (0000)
001A1F00 0050:
	_unit_solo_player_integrated_night_vision_is_active (0000)
001A1F50 00e0:
	_code_001a1f50 (0000)
001A2030 0040:
	_code_001a2030 (0000)
001A2070 0400:
	_unit_new (0000)
001A2470 0250:
	_unit_enter_seat (0000)
001A26C0 0290:
	_unit_exit_seat_end (0000)
001A2950 0270:
	_unit_died (0000)
001A2BC0 0130:
	_unit_scripting_enter_vehicle (0000)
001A2CF0 0180:
	_vehicle_scripting_load_magic (0000)
001A2E70 0110:
	_unit_try_and_exit_seat (0000)
001A2F80 1730:
	_unit_update (0000)
001A46B0 0640:
	_unit_damage_aftermath (0000)
001A4CF0 0100:
	_vehicle_scripting_unload (0000)
001A4DF0 0040:
	_unit_scripting_exit_vehicle (0000)
002A4DCC 0006:
	??_C@_05PJLLPND@stand?$AA@ (0000)
002A4DD4 000c:
	??_C@_0M@CHPCMCIL@unit_update?$AA@ (0000)
002A4DE0 000d:
	??_C@_0N@KFMOFEIC@unit_globals?$AA@ (0000)
002A4DF0 001d:
	??_C@_0BN@LFBAPBJD@c?3?2halo?2SOURCE?2units?2units?4c?$AA@ (0000)
002A4E10 000d:
	??_C@_0N@IKNEFIIG@unit?5globals?$AA@ (0000)
002A4E20 0044:
	??_C@_0EE@MJKOADBA@VALID_FLAGS?$CIpersistent_control_f@ (0000)
002A4E64 000c:
	??_C@_0M@GBEBNJOA@?5enter?9hint?$AA@ (0000)
002A4E70 0010:
	??_C@_0BA@EKKAICAF@animation_state?$AA@ (0000)
002A4E80 0014:
	??_C@_0BE@DJIPKBF@MISSING?3?5?$CFs?5?8?$CFs?5?$CFs?8?$AA@ (0000)
002A4E94 001e:
	??_C@_0BO@COLKAFDO@best_new_attacker_index?$CB?$DNNONE?$AA@ (0000)
002A4EB8 0051:
	??_C@_0FB@GIBLCPPG@?$CIestimate_mode?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIestimat@ (0000)
002A4F0C 0024:
	??_C@_0CE@EFKICMFK@body_position?5?$CG?$CG?5estimated_posit@ (0000)
002A4F30 0005:
	??_C@_04IEJGKNJ@body?$AA@ (0000)
002A4F38 004c:
	??_C@_0EM@FBFJECKJ@animation_impulse?$DO?$DN0?5?$CG?$CG?5animatio@ (0000)
002A4F84 003f:
	??_C@_0DP@JJACFPIN@current_index?$DO?$DN0?5?$CG?$CG?5current_inde@ (0000)
002A4FC4 0029:
	??_C@_0CJ@EPNCNOAE@couldn?8t?5find?5the?5emotion?5animat@ (0000)
002A4FF0 0031:
	??_C@_0DB@OCHKLFEM@index?$DO?$DN0?5?$CG?$CG?5index?$DMNUMBER_OF_UNIT@ (0000)
002A5024 0004:
	__real@3ca3d70b (0000)
002A5028 0049:
	??_C@_0EJ@NJDBEHGI@equipment_definition?9?$DOequipment?4@ (0000)
002A5078 0045:
	??_C@_0EF@EDBHKOAK@?$CIgrenade_type?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIgrenade_@ (0000)
002A50C0 0011:
	??_C@_0BB@JFCLDGNC@grenade_count?$DO?$DN0?$AA@ (0000)
002A50D8 0049:
	??_C@_0EJ@KGELGNBO@equipment_definition?9?$DOequipment?4@ (0000)
002A5128 0046:
	??_C@_0EG@DMMDGKIO@equipment_definition?9?$DOequipment?4@ (0000)
002A5170 0055:
	??_C@_0FF@BHHPBICM@grenade_type?$DN?$DNNONE?5?$HM?$HM?5?$CIgrenade_t@ (0000)
002A51C8 0091:
	??_C@_0JB@KAHIGLEC@unit?9?$DOunit?4current_grenade_index@ (0000)
002A525C 000a:
	??_C@_09KBFFALMN@left?5hand?$AA@ (0000)
002A5268 003e:
	??_C@_0DO@JIFEEFPA@unit?9?$DOunit?4grenade_counts?$FLunit?9?$DO@ (0000)
002A52A8 0065:
	??_C@_0GF@CHMEGMC@unit?9?$DOunit?4current_grenade_index@ (0000)
002A5310 0040:
	??_C@_0EA@DPJNOHJJ@base_seat_index?$DO?$DN0?5?$CG?$CG?5base_seat_@ (0000)
002A5350 0046:
	??_C@_0EG@PHNFDNJE@base_weapon_index?$DO?$DN0?5?$CG?$CG?5base_wea@ (0000)
002A5398 0008:
	??_C@_07GDJPGPDM@unarmed?$AA@ (0000)
002A53A0 0011:
	??_C@_0BB@FJNDJOGI@?$CFs?3?5animation?5?$CFs?$AA@ (0000)
002A53B4 002b:
	??_C@_0CL@FEKGNBAM@index?$DO?$DN0?5?$CG?$CG?5index?$DMMAXIMUM_WEAPON@ (0000)
002A53E0 002d:
	??_C@_0CN@HHCMEDCD@item?9?$DOobject?4parent_object_index@ (0000)
002A5410 0038:
	??_C@_0DI@FHACCLPK@a?5?$CFs?5tried?5to?5drop?5a?5?$CFs?5which?5wa@ (0000)
002A5448 003c:
	??_C@_0DM@HPMFIBJI@WARNING?3?5?$CFs?5tried?5to?5die?5from?5fl@ (0000)
002A5484 001e:
	??_C@_0BO@JNFPEPKG@plan?9?$DOcoast_t?5?$CL?5actual_t?5?$DO?$DN?5t?$AA@ (0000)
002A54A4 0013:
	??_C@_0BD@PPEPHKIE@plan?9?$DOcoast_t?5?$DO?$DN?50?$AA@ (0000)
002A54B8 0011:
	??_C@_0BB@MPHLKAIB@coasting_vel?5?$DM?50?$AA@ (0000)
002A54CC 0007:
	??_C@_06IGEDIHAH@t?5?$DO?$DN?50?$AA@ (0000)
002A54D4 000a:
	??_C@_09FAMPJHMJ@disc?5?$DO?$DN?50?$AA@ (0000)
002A54E0 001a:
	??_C@_0BK@JAFCIDEL@REAL_MAX?5?$CB?$DN?5plan?9?$DOdecel_t?$AA@ (0000)
002A54FC 001a:
	??_C@_0BK@KHOHGFFP@REAL_MAX?5?$CB?$DN?5plan?9?$DOdecel_a?$AA@ (0000)
002A5518 001a:
	??_C@_0BK@JCCGJEF@REAL_MAX?5?$CB?$DN?5plan?9?$DOcoast_t?$AA@ (0000)
002A5534 001a:
	??_C@_0BK@LLGCLIBF@REAL_MAX?5?$CB?$DN?5plan?9?$DOaccel_t?$AA@ (0000)
002A5550 001a:
	??_C@_0BK@IMNHFOAB@REAL_MAX?5?$CB?$DN?5plan?9?$DOaccel_a?$AA@ (0000)
002A556C 0014:
	??_C@_0BE@OJLFFFPC@plan?9?$DOinitial_v?5?$DM?50?$AA@ (0000)
002A5580 001a:
	??_C@_0BK@KCJNEIPK@plan?9?$DOinitial_p?5?$DO?5?91e?903f?$AA@ (0000)
002A559C 0004:
	__real@ba83126f (0000)
002A55A0 003b:
	??_C@_0DL@NDCNCHBI@?$CIadjust_plan?9?$DOaccel_t?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CI@ (0000)
002A55DC 0010:
	??_C@_0BA@JMNOFACH@t_extension?5?$DO?50?$AA@ (0000)
002A55EC 0017:
	??_C@_0BH@KDNEJEKJ@MISSING?3?5?$CFs?5?8?$CFs?5?$CFs?5?$CFs?8?$AA@ (0000)
002A5604 001a:
	??_C@_0BK@GEHICBJO@parent_seat_index?5?$CB?$DN?5NULL?$AA@ (0000)
002A5620 001b:
	??_C@_0BL@MGKMGDBI@relative_aiming_angles?4yaw?$AA@ (0000)
002A563C 001d:
	??_C@_0BN@LENPEPCG@relative_aiming_angles?4pitch?$AA@ (0000)
002A565C 0011:
	??_C@_0BB@DNAJOPNC@?$CGrelative_vector?$AA@ (0000)
002A5670 0074:
	??_C@_0HE@KJEMFFL@?$CIseat_desire_type?5?$DN?$DN?5NONE?$CJ?5?$HM?$HM?5?$CI?$CI@ (0000)
002A56E4 0014:
	??_C@_0BE@BPHFKLIJ@seat_substring_name?$AA@ (0000)
002A56F8 003b:
	??_C@_0DL@HBBDCAFF@current_index?$DO?$DN0?5?$CG?$CG?5current_inde@ (0000)
002A5734 0004:
	__real@3d0f5c29 (0000)
002A5738 0016:
	??_C@_0BG@JEMBMHGH@?$CGunit?9?$DOobject?4forward?$AA@ (0000)
002A5750 0011:
	??_C@_0BB@GELHHNAL@?$CGunit?9?$DOobject?4up?$AA@ (0000)
002A5764 0011:
	??_C@_0BB@LPKMKLOA@alignment_vector?$AA@ (0000)
002A5778 0004:
	__real@3cab92a6 (0000)
002A577C 0004:
	__real@bcab92a6 (0000)
002A5780 0004:
	__real@417ea5dd (0000)
002A5784 0004:
	__real@3e567750 (0000)
002A5788 0004:
	__real@4087cfed (0000)
002A578C 0033:
	??_C@_0DD@FIEJMHNA@unit_verify_vectors?5FAILURE?0?5see@ (0000)
002A57C0 0038:
	??_C@_0DI@OGIFHHM@?5?5looking?5vector?5?$CF08X?5?$CF08X?5?$CF08X?5@ (0000)
002A57F8 0037:
	??_C@_0DH@EJEBMEHF@?5?5aiming?5vector?5?$CF08X?5?$CF08X?5?$CF08X?5v@ (0000)
002A5830 004f:
	??_C@_0EP@KHDNKPNP@?5?5desired?5facing?5?$CF08X?5?$CF08X?5?$CF08X?0@ (0000)
002A5880 0044:
	??_C@_0EE@NPCICDEJ@?5?5object?3?5pos?5?$CF08X?5?$CF08X?5?$CF08X?0?5fw@ (0000)
002A58C4 001f:
	??_C@_0BP@KOGKNNHJ@?5?5warning?0?5hex?5dump?5follows?4?4?4?$AA@ (0000)
002A58E4 002c:
	??_C@_0CM@MJPCCGNM@?5?5looking?5vector?5?$CFf?5?$CFf?5?$CFf?5veloci@ (0000)
002A5910 002b:
	??_C@_0CL@ECJMNHKM@?5?5aiming?5vector?5?$CFf?5?$CFf?5?$CFf?5velocit@ (0000)
002A593C 003d:
	??_C@_0DN@FMBPFOHH@?5?5desired?5facing?5?$CFf?5?$CFf?5?$CFf?0?5aimin@ (0000)
002A597C 003a:
	??_C@_0DK@HCAIDIOB@?$CK?$CK?$CK?$CK?5unit_verify_vectors?3?5proble@ (0000)
002A59B8 000d:
	??_C@_0N@GHHFFCIH@unit?9control?$AA@ (0000)
002A59C8 001e:
	??_C@_0BO@MDANOMAD@control_data?9?$DOprimary_trigger?$AA@ (0000)
002A59E8 0040:
	??_C@_0EA@IBGHKCGO@control_data?9?$DOzoom_level?$DN?$DNNONE?5?$HM@ (0000)
002A5A28 0082:
	??_C@_0IC@CHONDMKL@control_data?9?$DOgrenade_index?$DN?$DNNON@ (0000)
002A5AB0 007b:
	??_C@_0HL@IFOOEEOJ@control_data?9?$DOweapon_index?$DN?$DNNONE@ (0000)
002A5B2C 001e:
	??_C@_0BO@CKDHHENP@?$CGcontrol_data?9?$DOlooking_vector?$AA@ (0000)
002A5B4C 001d:
	??_C@_0BN@CPLNIAKI@?$CGcontrol_data?9?$DOaiming_vector?$AA@ (0000)
002A5B6C 001d:
	??_C@_0BN@PJECCKBL@?$CGcontrol_data?9?$DOfacing_vector?$AA@ (0000)
002A5B90 0047:
	??_C@_0EH@EBEINPCA@VALID_FLAGS?$CIcontrol_data?9?$DOcontro@ (0000)
002A5BD8 0059:
	??_C@_0FJ@GIIHEDDP@control_data?9?$DOaiming_speed?$DO?$DN0?5?$CG?$CG@ (0000)
002A5C38 0062:
	??_C@_0GC@CEOKMDLJ@control_data?9?$DOanimation_state?$DO?$DN0@ (0000)
002A5C9C 002b:
	??_C@_0CL@MKJGLHIO@magnitude3d?$CI?$CGcontrol_data?9?$DOthrot@ (0000)
002A5CC8 001c:
	??_C@_0BM@MNPGDCFG@relative_looking_angles?4yaw?$AA@ (0000)
002A5CE4 001e:
	??_C@_0BO@MGEJEHJF@relative_looking_angles?4pitch?$AA@ (0000)
002A5D04 0019:
	??_C@_0BJ@IONAJDB@?$CGrelative_looking_vector?$AA@ (0000)
002A5D20 0018:
	??_C@_0BI@PHNGNHBO@?$CGrelative_aiming_vector?$AA@ (0000)
002A5D38 001a:
	??_C@_0BK@KJJPMJMN@?$CGunit?9?$DOunit?4aiming_vector?$AA@ (0000)
002A5D54 0016:
	??_C@_0BG@OEBDAJFJ@unit?9preprocess?9nodes?$AA@ (0000)
002A5D6C 000e:
	??_C@_0O@ODHCBCIJ@aiming_vector?$AA@ (0000)
002A5D80 0061:
	??_C@_0GB@FBDPEFM@?$CIend_aiming_angles?4pitch?5?$DO?$DN?5aimi@ (0000)
002A5DE8 005d:
	??_C@_0FN@IHCHGFNM@?$CIend_aiming_angles?4yaw?5?$DO?$DN?5aiming@ (0000)
002A5E48 002b:
	??_C@_0CL@BAPCPLDO@end_aiming_angles?4yaw?5?$DO?$DN?5aiming_@ (0000)
002A5E74 002b:
	??_C@_0CL@ONCNMLAC@end_aiming_angles?4yaw?5?$DM?$DN?5aiming_@ (0000)
002A5EA0 002f:
	??_C@_0CP@FHKMNBOC@desired_aiming_angles?4yaw?5?$DO?$DN?5aim@ (0000)
002A5ED0 002f:
	??_C@_0CP@KKHDOBNO@desired_aiming_angles?4yaw?5?$DM?$DN?5aim@ (0000)
002A5F00 004f:
	??_C@_0EP@KBPKPPJL@?$CIangular_acceleration_limit?5?$DO?50?4@ (0000)
002A5F50 001f:
	??_C@_0BP@DPNLKBBM@angular_velocity_limit?5?$DO?$DN?50?40f?$AA@ (0000)
002A5F70 001e:
	??_C@_0BO@GAMKEDMN@desired_base_seat_index?$CB?$DNNONE?$AA@ (0000)
002A5F90 0038:
	??_C@_0DI@EIIFDJIB@desired_state?$DO?$DN0?5?$CG?$CG?5desired_stat@ (0000)
002A5FC8 0008:
	__real@4001475cd2000000 (0000)
002A5FD0 0027:
	??_C@_0CH@KNBNIKNG@unit?9?$DOobject?4parent_object_index@ (0000)
002A5FF8 0021:
	??_C@_0CB@MFPLMKIO@unit?9?$DOunit?4feign_death_timer?5?$DO?50@ (0000)
002A601C 0010:
	??_C@_0BA@JFPDPOCG@unit?9update?9end?$AA@ (0000)
002A602C 0024:
	??_C@_0CE@EIFKFLM@game_globals_first_person_interf@ (0000)
002A6050 002c:
	??_C@_0CM@JCKFNLKN@player?5illumination?3?5self?5?$CF?42f?5a@ (0000)
002A607C 0017:
	??_C@_0BH@GFOLCKHG@unit?9update?9postvector?$AA@ (0000)
002A6094 001b:
	??_C@_0BL@BHGJMDEB@?$CGunit?9?$DOunit?4looking_vector?$AA@ (0000)
002A60B0 0014:
	??_C@_0BE@MNAOFPAE@unit?9update?9look?93d?$AA@ (0000)
002A60C4 0017:
	??_C@_0BH@HICAMLFB@unit?9update?9look?9euler?$AA@ (0000)
002A60DC 0015:
	??_C@_0BF@NGCFEELO@unit?9update?9look?9set?$AA@ (0000)
002A60F4 0013:
	??_C@_0BD@DPFKBAB@unit?9update?9aim?93d?$AA@ (0000)
002A6108 0016:
	??_C@_0BG@NOLLOOND@unit?9update?9aim?9euler?$AA@ (0000)
002A6120 0014:
	??_C@_0BE@KGIBELMP@unit?9update?9aim?9set?$AA@ (0000)
002A6134 0022:
	??_C@_0CC@HPCCHHEP@?$CGunit?9?$DOunit?4desired_aiming_vecto@ (0000)
002A6158 0017:
	??_C@_0BH@PCBGODII@unit?9update?9prevectors?$AA@ (0000)
002A6170 0029:
	??_C@_0CJ@GMJCHFEH@?$CGgunner_unit?9?$DOunit?4desired_aimin@ (0000)
002A619C 0029:
	??_C@_0CJ@KNNFHIN@?$CGdriver_unit?9?$DOunit?4desired_facin@ (0000)
002A61C8 0012:
	??_C@_0BC@FNMDNBAN@unit?9update?9begin?$AA@ (0000)
002A61DC 0024:
	??_C@_0CE@MIPDNKIJ@p?$CFd?3?5body?5?$CF?42f?5shld?5?$CF?42f?5from?5?$CFs@ (0000)
00315C08 061c:
	_magic_base_animation_seat_index (0000)
	_base_seat_labels (0604)
004CCB18 0004:
	_bss_004ccb18 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "units.h"

#include "bipeds.h"
#include "unit_definitions.h"

#include "ai/actors.h"
#include "ai/ai_debug.h"
#include "bitmaps/bitmaps.h"
#include "cseries/errors.h"
#include "cseries/profile.h"
#include "effects/effects.h"
#include "game/cheats.h"
#include "game/game_globals.h"
#include "game/game_engine.h"
#include "game/players.h"
#include "items/projectiles.h"
#include "items/weapon_definitions.h"
#include "items/weapons.h"
#include "main/console.h"
#include "models/model_animation_definitions.h"
#include "objects/damage.h"
#include "objects/object_lights.h"
#include "physics/collision_bsp.h"
#include "physics/collision_models.h"
#include "physics/collision_usage.h"
#include "physics/collisions.h"
#include "scenario/scenario.h"
#include "sound/game_sound.h"

/* ---------- constants */

/* ---------- macros */

#define unit_get_current_weapon_index(unit_index) unit_inventory_get_weapon((unit_index), unit_get((unit_index))->unit.current_weapon_index)
#define unit_get_desired_weapon_index(unit_index) unit_inventory_get_weapon((unit_index), unit_get((unit_index))->unit.desired_weapon_index)

/* ---------- structures */

struct unit_globals
{
	short next_timer;
	short highest_timer;
	boolean used_time;
	char pad[3];
};

/* ---------- prototypes */

static char const *base_seat_label_get(short base_seat_index);
static short seat_label_to_base_seat_index(char const *seat_label);
static char const *base_weapon_label_get(short base_weapon_index);

static void unit_refresh_illumination(long unit_index);

static void unit_adjust_for_seat_change(long unit_index);
static short unit_weapon_next_index(long unit_index, short current_index, short delta);

static void unit_throw_grenade_move_to_hand(long unit_index);


static boolean unit_animation_busy(struct unit_animation *animation);

static boolean unit_set_or_test_seat_and_weapon_label(
	long object_index,
	char const *seat_label,
	char const *weapon_label,
	boolean change_flag);

static boolean unit_animation_set_state(
	long unit_index,
	short new_state);

static boolean unit_verify_inventory(long unit_index);
static void unit_throw_grenade_release(long unit_index, boolean premature);

static void unit_seat_update(long object_index);

static char const *unit_get_seat_label(long object_index);

static void unit_cause_continuous_melee_damage(long unit_index);

static long unit_get_weapon(struct unit_datum *unit, short index);
static void unit_drop_item(long unit_index, long item_index);

static void unit_verify_vectors(long unit_index, char const *debugstring);
static void unit_running_blind(long unit_index, real_vector3d *run_vector);

static boolean unit_integrated_night_vision_is_active(long unit_index);

/* ---------- globals */

short magic_base_animation_seat_index = NONE;

static struct unit_globals *unit_globals;

static struct profile_section unit_update_section = {"unit_update", NONE, TRUE};

boolean debug_objects_unit_mouth_apeture;
boolean debug_objects_unit_seats;
boolean debug_objects_unit_vectors;
boolean stun_enable;
boolean debug_damage_taken;
boolean debug_unit_illumination;
boolean debug_unit_animations;
boolean debug_unit_all_animations;

/* ---------- public code */

void units_update(
	void)
{
	unit_globals->next_timer = unit_globals->highest_timer;
	unit_globals->highest_timer = 0;
	unit_globals->used_time = 0;

	return;
}

boolean unit_update(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	boolean used_time = FALSE;
	boolean jetpack_cheat_active = FALSE;
	boolean light_change_state = FALSE;
	boolean allow_integrated_lights = game_engine_allow_integrated_lights(unit_index);

	profile_enter(unit_update_section);

	unit_verify_vectors(unit_index, "unit-update-begin");

	++unit->unit.timer;

	if (unit_globals->used_time!=0 || unit->unit.timer <= unit_globals->next_timer)
	{
		long timer;

		if (unit_globals->highest_timer>unit->unit.timer)
		{
			timer = unit_globals->highest_timer;
		}
		else
		{
			timer = unit->unit.timer;
		}

		unit_globals->highest_timer = timer;
	}
	else
	{
		used_time = TRUE;
		unit_globals->used_time = TRUE;
		unit->unit.timer = 0;
	}

	if (TEST_FLAG(unit->unit.flags, _unit_running_blindly_bit))
	{
		unit_running_blind(unit_index, &unit->unit.desired_facing_vector);
		unit->unit.desired_aiming_vector = unit->unit.desired_facing_vector;
		unit->unit.desired_looking_vector = unit->unit.desired_facing_vector;
		unit->unit.throttle = *global_forward3d;
		unit->unit.control_flags = 0;
	}
	else if (!TEST_FLAG(unit->unit.flags, _unit_actively_controlled_bit))
	{
		unit->unit.desired_looking_vector = unit->object.forward;
		unit->unit.desired_aiming_vector = unit->object.forward;
		unit->unit.desired_facing_vector = unit->object.forward;
		unit->unit.throttle = *global_zero_vector3d;
		unit->unit.control_flags = 0;
	}

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_simple_creature_bit))
	{
		boolean v4 = FALSE;

		if (unit->unit.persistent_control_timer>0)
		{
			unit->unit.control_flags = unit->unit.persistent_control_flags|unit->unit.control_flags;
			if (TEST_FLAG(unit->unit.persistent_control_flags, _unit_control_weapon_primary_trigger_bit))
			{
				SET_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit, (unit->unit.persistent_control_timer%7)==0);
				unit->unit.primary_trigger = 1.f;
			}
			else
			{
				unit->unit.primary_trigger = 0.f;
			}

			if (--unit->unit.persistent_control_timer==0)
			{
				unit->unit.persistent_control_flags = 0;
			}
		}

		if (!TEST_FLAG(unit->unit.flags, _unit_possessed_by_recording_bit))
		{
			if (unit->unit.driver_object_index!=NONE &&
				!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			{
				struct unit_datum *driver_unit = unit_get(unit->unit.driver_object_index);

				unit->object.owner_team_index = driver_unit->object.owner_team_index;
				v4 = TRUE;

				if (driver_unit->unit.player_index!=NONE ||
					driver_unit->unit.animation.state!=_unit_state_exiting_seat &&
					driver_unit->unit.animation.state!=_unit_state_entering_seat)
				{
					match_assert_valid_real_normal3d(
						"c:\\halo\\SOURCE\\units\\units.c",
						696,
						&driver_unit->unit.desired_facing_vector
					);
					unit->unit.control_flags |= driver_unit->unit.control_flags&UNIT_CONTROL_DRIVER_MASK;
					unit->unit.desired_facing_vector = driver_unit->unit.desired_facing_vector;
					unit->unit.throttle = driver_unit->unit.throttle;
				}
			}

			if (unit->unit.gunner_object_index!=NONE &&
				!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			{
				struct unit_datum *gunner_unit = unit_get(unit->unit.gunner_object_index);

				if (!v4)
				{
					unit->object.owner_team_index = gunner_unit->object.owner_team_index;
				}

				if (gunner_unit->unit.player_index!=NONE ||
					gunner_unit->unit.animation.state!=_unit_state_exiting_seat &&
					gunner_unit->unit.animation.state!=_unit_state_entering_seat)
				{
					match_assert_valid_real_normal3d(
						"c:\\halo\\SOURCE\\units\\units.c",
						720,
						&gunner_unit->unit.desired_aiming_vector
					);

					unit->unit.desired_aiming_vector = gunner_unit->unit.desired_aiming_vector;
					unit->unit.desired_looking_vector = gunner_unit->unit.desired_aiming_vector;
					unit->unit.control_flags |= gunner_unit->unit.control_flags&UNIT_CONTROL_GUNNER_MASK;
					unit->unit.primary_trigger = gunner_unit->unit.primary_trigger;
				}
			}

			if ((unit->unit.control_flags&UNIT_CONTROL_GUNNER_MASK)!=0)
			{
				unit->unit.gunner_inactive_ticks = 0;
			}
			else if (unit->unit.gunner_inactive_ticks<CHAR_MAX)
			{
				++unit->unit.gunner_inactive_ticks;
			}
		}

		/*
		* Jetpack cheat combo:
		* Press the following keys while the jetpack cheat is active
		* Grenade throw button
		* Primary weapon fire button
		*/
		if (cheat.jetpack &&
			TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit) &&
			TEST_FLAG(unit->unit.control_flags, _unit_control_throw_grenade_bit))
		{
			jetpack_cheat_active = TRUE;
		}

		if (TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit))
		{
			boolean set = FALSE;
			real active_camo_change_this_tick;

			if (game_engine_running() &&
				unit->unit.cause_for_camo_regrowth && 
				unit->unit.cause_for_camo_regrowth==cause_for_camo_regrowth_shot_fired)
			{
				long weapon_index = unit_get_current_weapon_index(unit_index);

				if (weapon_index!=NONE)
				{
					struct weapon_datum *weapon = weapon_get(weapon_index);
					struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

					if (weapon_definition->weapon.active_camo_regrowth_rate!=0.f)
					{
						active_camo_change_this_tick = weapon_definition->weapon.active_camo_regrowth_rate;
						set = TRUE;
					}
				}
			}

			if (!set)
			{
				active_camo_change_this_tick = 1.f/120.f;
			}


			unit->unit.active_camouflage += active_camo_change_this_tick;

			if (unit->unit.active_camouflage>1.f)
			{
				unit->unit.active_camouflage = 1.f;
				unit->unit.cause_for_camo_regrowth = cause_for_camo_regrowth_default;
			}
		}
		else
		{
			unit->unit.active_camouflage -= 1.f/120.f;

			if (unit->unit.active_camouflage<0.f)
			{
				unit->unit.active_camouflage = 0.f;
			}
		}

		if (TEST_FLAG(unit->unit.flags, _unit_super_camouflaged_bit))
		{
			unit->unit.active_camouflage_super_amount += 1.f/90.f;

			if (unit->unit.active_camouflage_super_amount>1.f)
			{
				unit->unit.active_camouflage_super_amount = 1.f;
			}
		}
		else
		{
			unit->unit.active_camouflage_super_amount -= 1.f/90.f;

			if (unit->unit.active_camouflage_super_amount<0.f)
			{
				unit->unit.active_camouflage_super_amount = 0.f;
			}
		}

		// Update stun timer
		if (unit->unit.body_stun_ticks>0)
		{
			unit->unit.body_stun_ticks--;
			// Stop stunning when tick timer is done
			if (unit->unit.body_stun_ticks==0)
			{
				unit->unit.body_stun = 0.f;
			}
		}

		// Drop weapon when drop delay is done
		if (unit->unit.weapon_drop_delay_ticks>0 && --unit->unit.weapon_drop_delay_ticks==0)
		{
			unit_drop_current_weapon(unit_index, TRUE);
		}

		// Update feign death timer
		if (unit->unit.feign_death_timer>0 &&
			TEST_FLAG(unit->object.flags, _object_at_rest_bit))
		{
			unit->unit.feign_death_timer--;

			// Die when the unit has met it's end
			if (!unit->unit.feign_death_timer)
			{
				if (unit->object.body_vitality>0.f)
				{
					short new_state = TEST_FLAG(unit->unit.animation.flags, 3) ? _unit_state_resurrect_front : _unit_state_resurrect_back;

					SET_FLAG(unit->object.damage_flags, _object_dead_bit, FALSE);

					unit_set_actively_controlled(unit_index, TRUE);
					unit_set_or_test_seat_and_weapon_label(
						unit_index,
						base_seat_label_get(_unit_animation_state_suspicious),
						NULL,
						TRUE
					);
					unit_animation_set_state(unit_index, new_state);

					SET_FLAG(unit->unit.animation.flags, 2, FALSE);

					if (unit->object.type==_object_type_biped)
					{
						biped_stop_limp_body_physics(unit_index);
					}

					unit_scream(unit_index, _unit_scream_resurrection);
				}
				else
				{
					unit_died(unit_index, FALSE);
				}
			}
		}
	}

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_has_no_aiming_bit))
	{
		real aim_scale;
		real aiming_velocity_limit;
		real aiming_angular_acceleration_limit;
		real_vector3d previous_aiming_vector;

		if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			if (TEST_FLAG(unit->object.damage_flags, _object_cannot_hold_weapon_bit))
			{
				unit_drop_current_weapon(unit_index, TRUE);
			}
			else if (unit->unit.desired_weapon_index!=unit->unit.current_weapon_index &&
				!unit_animation_busy(&unit->unit.animation))
			{
				long weapon_index = unit_get_desired_weapon_index(unit_index);

				if (weapon_index!=NONE && unit_can_use_weapon(unit_index, weapon_index))
				{
					unit_ready_desired_weapon(unit_index, TRUE);
				}
			}

			if (unit->unit.desired_grenade_index!=unit->unit.current_grenade_index &&
				!unit_animation_busy(&unit->unit.animation))
			{
				short grenade_index = unit_inventory_next_grenade(unit_index, unit->unit.desired_grenade_index, 0);

				if (grenade_index!=NONE)
				{
					unit->unit.current_grenade_index = grenade_index;
				}
			}

			if (cheat.infinite_ammo && unit->unit.player_index!=NONE)
			{
				short grenade_index;

				for (grenade_index = 0; grenade_index<NUMBEROF(unit->unit.grenade_counts); ++grenade_index)
				{
					unit->unit.grenade_counts[grenade_index] = unit->unit.grenade_counts[grenade_index]>1 ? unit->unit.grenade_counts[grenade_index] : 1;
				}

				if (unit->unit.desired_grenade_index==NONE)
				{
					unit->unit.desired_grenade_index = 0;
				}
			}

			if (unit->unit.desired_zoom_level!=unit->unit.current_zoom_level)
			{
				long local_player_index;

				unit->unit.current_zoom_level = unit->unit.desired_zoom_level;

				if (unit->unit.desired_zoom_level==NONE)
				{
					unit->unit.integrated_night_vision_power = 0.f;
				}

				if (player_index_from_unit_index(unit_index)!=NONE)
				{
					local_player_index = player_get(player_index_from_unit_index(unit_index))->local_player_index;
				}
				else
				{
					local_player_index = NONE;
				}

				if (local_player_index!=NONE)
				{
					long weapon_index = unit_get_current_weapon_index(unit_index);

					if (weapon_index!=NONE)
					{
						struct weapon_datum *weapon = weapon_get(weapon_index);
						struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
						long sound_index = unit->unit.current_zoom_level==NONE ? weapon_definition->weapon.zoom_out_sound.index : weapon_definition->weapon.zoom_in_sound.index;
						real scale = 1.f;

						if (unit->unit.current_zoom_level!=NONE && unit->unit.current_zoom_level>1)
						{
							scale = (real)unit->unit.current_zoom_level/(real)(unit->unit.current_zoom_level-1);
						}

						if (sound_index!=NONE)
						{
							unspatialized_impulse_sound_new(sound_index, scale);
						}
					}
				}
			}
		}

		unit_verify_vectors(unit_index, "unit-update-prevectors");

		if (unit->unit.aiming_speed==_unit_aiming_speed_casual)
		{
			aim_scale = unit_definition->unit.casual_aiming_modifier;
		}
		else
		{
			aim_scale = 1.f;
		}

		aiming_velocity_limit = (aim_scale*unit_definition->unit.aiming_velocity_maximum)/TICKS_PER_SECOND;
		aiming_angular_acceleration_limit = (aim_scale*unit_definition->unit.aiming_acceleration_maximum)/(TICKS_PER_SECOND*TICKS_PER_SECOND);
		previous_aiming_vector = unit->unit.aiming_vector;

		if (aiming_velocity_limit==0.f && aiming_angular_acceleration_limit==0.f)
		{
			match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 993, &unit->unit.desired_aiming_vector);
			
			unit->unit.aiming_vector = unit->unit.desired_aiming_vector;
			unit_clip_to_aiming_bounds(unit_index, &unit->unit.aiming_vector, TRUE);
			unit->unit.aiming_velocity = *global_zero_vector3d;
			unit_verify_vectors(unit_index, "unit-update-aim-set");
		}
		else if (unit->unit.animation.aiming_with_euler_screen)
		{
			real_matrix4x3 matrix;

			matrix.scale = 1.f;
			object_get_orientation(unit_index, &matrix.forward, &matrix.up);
			cross_product3d(&matrix.up, &matrix.forward, &matrix.left);
			matrix.position = *global_origin3d;

			unit_euler_aiming_update(
				&matrix,
				&unit->unit.aiming_vector,
				&unit->unit.desired_aiming_vector,
				&unit->unit.aiming_velocity,
				&unit->unit.animation.aiming_screen_bounds,
				aiming_velocity_limit,
				aiming_angular_acceleration_limit);

			unit_verify_vectors(unit_index, "unit-update-aim-euler");
		}
		else
		{
			angular_accelerate_to_position(
				&unit->unit.aiming_vector,
				&unit->unit.desired_aiming_vector,
				&unit->unit.aiming_velocity,
				aiming_velocity_limit,
				aiming_angular_acceleration_limit);
			unit_verify_vectors(unit_index, "unit-update-aim-3d");
		}

		{
			real angle = angle_between_vectors3d(&unit->unit.aiming_vector, &previous_aiming_vector);
			real change = angle / (unit_definition->unit.aiming_velocity_maximum/TICKS_PER_SECOND);
				
			unit->unit.aiming_change = (byte)(PIN(change, 0.f, 1.f) * 255.f);
		}

		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1032, &unit->unit.aiming_vector);

		{
			real looking_velocity_limit = (aim_scale*unit_definition->unit.looking_velocity_maximum)/((real)TICKS_PER_SECOND);
			real looking_angular_acceleration_limit = (aim_scale*unit_definition->unit.looking_acceleration_maximum)/900.f;

			if (looking_velocity_limit==0.f && looking_angular_acceleration_limit==0.f)
			{
				unit->unit.looking_vector = unit->unit.desired_looking_vector;
				unit_clip_to_aiming_bounds(unit_index, &unit->unit.aiming_vector, FALSE);
				unit->unit.looking_velocity = *global_zero_vector3d;

				unit_verify_vectors(unit_index, "unit-update-look-set");
			}
			else if (unit->unit.animation.looking_with_euler_screen)
			{
				real_matrix4x3 matrix;

				matrix.scale = 1.f;
				object_get_orientation(unit_index, &matrix.forward, &matrix.up);
				cross_product3d(&matrix.up, &matrix.forward, &matrix.left);
				matrix.position = *global_origin3d;

				unit_euler_aiming_update(
					&matrix,
					&unit->unit.aiming_vector,
					&unit->unit.desired_looking_vector,
					&unit->unit.looking_velocity,
					&unit->unit.animation.looking_screen_bounds,
					looking_velocity_limit,
					looking_angular_acceleration_limit);

				unit_verify_vectors(unit_index, "unit-update-look-euler");
			}
			else
			{
				angular_accelerate_to_position(
					&unit->unit.looking_vector,
					&unit->unit.desired_looking_vector,
					&unit->unit.looking_velocity,
					looking_velocity_limit,
					looking_angular_acceleration_limit);
				unit_verify_vectors(unit_index, "unit-update-look-3d");
			}
		}


		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1076, &unit->unit.looking_vector);

		unit_verify_vectors(unit_index, "unit-update-postvector");

		if (!jetpack_cheat_active)
		{
			boolean throw_grenade = TEST_FLAG(unit->unit.control_flags, _unit_control_throw_grenade_bit);

			switch (unit->unit.grenade_throw_state)
			{
			case _unit_grenade_throw_idle:
				if (throw_grenade)
				{
					unit_throw_grenade_begin(unit_index, NULL);
				}
				break;
			case _unit_grenade_throw_wind_up:
				if (unit->object.animation.state.frame_index>=2)
				{
					unit_throw_grenade_move_to_hand(unit_index);
				}
				break;
			case _unit_grenade_throw_ending:
				if (unit->unit.animation.state!=_unit_state_throw_grenade && !throw_grenade)
				{
					unit->unit.grenade_throw_state = _unit_grenade_throw_idle;
				}
				break;
			case _unit_grenade_throw_in_hand:
				++unit->unit.grenade_throw_ticks;
				if (unit->unit.animation.state!=_unit_state_throw_grenade)
				{
					unit_throw_grenade_release(unit_index, TRUE);
				}
				break;
			default:
				break;
			}
		}
	}

	if (unit->unit.current_weapon_index!=NONE)
	{
		long flags = 0;
		real primary_trigger = unit->unit.primary_trigger;

		if (unit->unit.current_weapon_index==unit->unit.desired_weapon_index)
		{
			boolean const time_remaining = unit->unit.persistent_control_timer>0 && TEST_FLAG(unit->unit.persistent_control_flags, 11);

			if (!jetpack_cheat_active)
			{
				if (allow_integrated_lights && TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit))
				{
					SET_FLAG(flags, 0, TRUE);
				}
				if (TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit))
				{
					SET_FLAG(flags, 1, TRUE);
				}
				if (TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_secondary_trigger_bit))
				{
					SET_FLAG(flags, 2, TRUE);
				}
			}

			if (TEST_FLAG(unit_definition_get(unit->definition_index)->unit.flags, _unit_integrated_light_controls_weapon_directly_bit))
			{
				weapon_set_integrated_light_power(unit_get_current_weapon_index(unit_index), unit->unit.integrated_light_power);
			}

			if (TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_reload_bit))
			{
				SET_FLAG(flags, 3, TRUE);
			}
			if (unit_animation_busy(&unit->unit.animation) && !time_remaining)
			{
				SET_FLAG(flags, 4, TRUE);
			}
			if (unit->object.type==_object_type_biped && ((struct biped_datum *)unit)->biped.player_melee_ticks > 0)
			{
				SET_FLAG(flags, 4, TRUE);
			}
			if (unit->unit.current_zoom_level!=NONE)
			{
				SET_FLAG(flags, 6, TRUE);
			}
		}
		else
		{
			SET_FLAG(flags, 5, TRUE);
		}

		weapon_owner_update(unit_get_current_weapon_index(unit_index), flags, primary_trigger);
	}

	match_assert_valid_real_vector3d_axes2("c:\\halo\\SOURCE\\units\\units.c", 1155, &unit->object.forward, &unit->object.up);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1156, &unit->unit.aiming_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1157, &unit->unit.looking_vector);

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_simple_creature_bit))
	{
		short seat_index;

		if (TEST_FLAG(unit->unit.animation.flags, 1))
		{
			unit_seat_update(unit_index);
			unit->unit.seat_acceleration.i = unit->unit.seat_desired_acceleration.i*0.3f + unit->unit.seat_acceleration.i*0.7f;
			unit->unit.seat_acceleration.j = unit->unit.seat_desired_acceleration.j*0.3f + unit->unit.seat_acceleration.j*0.7f;
			unit->unit.seat_acceleration.k = unit->unit.seat_desired_acceleration.k*0.3f + unit->unit.seat_acceleration.k*0.7f;
		}

		for (seat_index = 0; seat_index<unit_definition->unit.powered_seats.count; ++seat_index)
		{
			boolean v96 = FALSE;
			struct powered_seat_definition *powered_seat = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.powered_seats, seat_index, struct powered_seat_definition);

			if (seat_index==0)
			{
				if (unit->unit.driver_object_index!=NONE || TEST_FLAG(unit->unit.flags, 0))
				{
					v96 = TRUE;
				}
				else
				{
					v96 = FALSE;
				}
			}
			else
			{
				v96 = unit->unit.gunner_object_index!=NONE && unit->unit.gunner_object_index!=unit->unit.driver_object_index;
			}

			if (TEST_FLAG(unit->object.damage_flags, _object_dead_bit) || !v96)
			{
				if (unit->unit.seat_power[seat_index] != 0.f)
				{
					unit->unit.seat_power[seat_index] -= 1.f / (powered_seat->powerdown_time * TICKS_PER_SECOND);

					if (unit->unit.seat_power[seat_index]<0.f)
					{
						unit->unit.seat_power[seat_index] = 0.f;
					}
				}
			}
			else if (unit->unit.seat_power[seat_index] != 1.f)
			{
				unit->unit.seat_power[seat_index] += 1.f / (powered_seat->powerup_time * TICKS_PER_SECOND);

				if (unit->unit.seat_power[seat_index]>1.f)
				{
					unit->unit.seat_power[seat_index] = 1.f;
				}
			}
		}
	}

	if (unit->unit.delayed_damage_timer>0)
	{
		unit->unit.delayed_damage_timer--;

		if (unit->unit.delayed_damage_timer==0)
		{
			ai_handle_damage(
				unit_index,
				unit->unit.delayed_damage_attacker_object_index,
				unit->unit.last_damage_category,
				unit->unit.delayed_damage_peak,
				NULL,
				TRUE);

			unit->unit.last_damage_category = 0;
			unit->unit.delayed_damage_attacker_object_index = NONE;
			unit->unit.delayed_damage_peak = 0.f;
		}
	}

	unit_cause_continuous_melee_damage(unit_index);
	unit_dialogue_update(unit_index);

	if (used_time || unit->unit.player_index!=NONE)
	{
		unit_refresh_illumination(unit_index);

		if (debug_unit_illumination)
		{
			if (unit->unit.player_index !=NONE)
			{
				long current_time = game_time_get();
				static long last_time = NONE;

				if (current_time >= last_time+TICKS_PER_SECOND)
				{
					error(
						_error_silent,
						"player illumination: self %.2f ambient %.2f",
						unit->unit.self_illumination,
						unit->unit.ambient_illumination);
					last_time = current_time;
				}
			}
		}
	}

	if (unit->unit.flaming_death_delay>0)
	{
		if (--unit->unit.flaming_death_delay==0)
		{
			unit_flame_to_death(unit_index);
		}
	}

	interpolate_scalar(&unit->unit.mouth_aperture, 0.f, 0.1f);

	if (TEST_FLAG(unit->unit.flags, _unit_desired_integrated_light_on_bit))
	{
		light_change_state = TRUE;

		if (TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit))
		{
			light_change_state = FALSE;
		}

		SET_FLAG(unit->unit.flags, _unit_desired_integrated_light_on_bit, FALSE);
	}
	

	if (TEST_FLAG(unit->unit.flags, _unit_desired_integrated_light_off_bit))
	{
		if (TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit))
		{
			light_change_state = TRUE;
		}

		SET_FLAG(unit->unit.flags, _unit_desired_integrated_light_off_bit, FALSE);
	}

	if (allow_integrated_lights &&
		TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit) ||
		unit->unit.integrated_light_battery <= 0.f ||
		light_change_state)
	{
		boolean const active = unit_integrated_night_vision_is_active(unit_index);

		if (active && TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit))
		{
			long effect_index;
			struct game_globals *game_globals;
			struct game_globals_first_person_interface *game_globals_first_person_interface;

			game_globals = scenario_get_game_globals();

			match_assert("c:\\halo\\SOURCE\\units\\units.c", 1302, game_globals);
			game_globals_first_person_interface = TAG_BLOCK_GET_ELEMENT(&game_globals->first_person_interface, 0, struct game_globals_first_person_interface);
			match_assert("c:\\halo\\SOURCE\\units\\units.c", 1304, game_globals_first_person_interface);

			if (TEST_FLAG(unit->unit.flags, _unit_integrated_night_vision_on_bit))
			{
				effect_index = game_globals_first_person_interface->night_vision_on_off_effect.index;
			}
			else
			{
				effect_index = game_globals_first_person_interface->night_vision_off_on_effect.index;
			}

			if (effect_index!=NONE)
			{
				effect_new_from_object(effect_index, unit_index, unit_index, NONE, 0.f, 0.f, NULL, NULL);
			}

			unit->unit.flags ^= FLAG(_unit_integrated_night_vision_on_bit);
		}

		if (!(active && TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit)) &&
			(TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit) || unit->unit.integrated_light_battery > 0.2f) &&
			unit->object.parent_object_index==NONE)
		{
			effect_new_from_object(unit_definition->unit.integrated_light_toggle_effect.index, unit_index, unit_index, NONE, 0.f, 0.f, NULL, NULL);
			unit->unit.flags ^= FLAG(_unit_integrated_light_on_bit);
		}
	}

	if (TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit))
	{
		if (!TEST_FLAG(unit_definition->unit.flags, _unit_integrated_light_fucking_lasts_forever_bit))
		{
			unit->unit.integrated_light_battery = unit->unit.integrated_light_battery - 1.f/(2.f*TICKS_PER_MINUTE);
		}

		if (unit->object.parent_object_index!=NONE || TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			SET_FLAG(unit->unit.flags, _unit_integrated_light_on_bit, FALSE);
		}

		if (unit->unit.integrated_light_power!=1.f)
		{
			unit->unit.integrated_light_power += 1.f/6.f;

			if (unit->unit.integrated_light_power>1.f)
			{
				unit->unit.integrated_light_power = 1.f;
			}
		}
	}
	else
	{
		if (unit->unit.integrated_light_battery < 1.f)
		{
			unit->unit.integrated_light_battery += 1.f/900.f;
		}

		if (unit->unit.integrated_light_power != 0.f)
		{
			unit->unit.integrated_light_power -= 1.f/24.f;

			if (unit->unit.integrated_light_power<0.f)
			{
				unit->unit.integrated_light_power = 0.f;
			}
		}
	}
	

	
	if (unit_integrated_night_vision_is_active(unit_index))
	{
		if (TEST_FLAG(unit->unit.flags, _unit_integrated_night_vision_on_bit))
		{
			if (unit->unit.integrated_night_vision_power!=1.f)
			{
				unit->unit.integrated_night_vision_power += 1.f/12.f;

				if (unit->unit.integrated_night_vision_power>1.f)
				{
					unit->unit.integrated_night_vision_power = 1.f;
				}
			}
		}
		else if (unit->unit.integrated_night_vision_power!=0.f)
		{
			unit->unit.integrated_night_vision_power -= 1.f/24.f;

			if (unit->unit.integrated_night_vision_power<0.f)
			{
				unit->unit.integrated_night_vision_power = 0.f;
			}
		}
	}

	unit_verify_vectors(unit_index, "unit-update-end");
	
	profile_exit(unit_update_section);
	
	return TRUE;
}

void unit_unzoom(
	long unit_index)
{
	boolean play_sound;
	boolean current_weapon_zooms;
	long current_weapon_index;
	long local_player_index;

	struct unit_datum *unit = unit_get(unit_index);

	if (player_index_from_unit_index(unit_index)==NONE)
	{
		local_player_index = NONE;
	}
	else
	{
		local_player_index = player_get(player_index_from_unit_index(unit_index))->local_player_index;
	}

	current_weapon_zooms = unit->unit.current_zoom_level!=NONE;

	if (local_player_index!=NONE && current_weapon_zooms)
	{
		current_weapon_index = unit_get_current_weapon_index(unit_index);

		if (current_weapon_index!=NONE)
		{
			struct weapon_datum *weapon = weapon_get(current_weapon_index);
			struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
			long zoom_out_sound_index = weapon_definition->weapon.zoom_out_sound.index;

			play_sound = zoom_out_sound_index!=NONE;

			if (play_sound)
			{
				unspatialized_impulse_sound_new(zoom_out_sound_index, 1.f);
			}
		}
	}

	unit->unit.current_zoom_level = NONE;
	unit->unit.desired_zoom_level = NONE;
	unit->unit.integrated_night_vision_power = 0.f;
	player_control_unzoom(unit_index);

	return;
}

void unit_get_head_position(
	long unit_index,
	union real_point3d *head_position)
{
	struct object_marker head_marker;

	object_get_marker_by_name(unit_index, "head", &head_marker, 1);
	*head_position = head_marker.matrix.position;

	return;
}

void unit_get_camera_position(
	long unit_index,
	real_point3d *camera_position)
{
	struct object_marker marker;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit->object.parent_object_index!=NONE || TEST_FLAG(unit->object.damage_flags, _object_dead_bit) || unit->object.type!=_object_type_biped)
	{
		if (unit->object.parent_object_index==NONE)
		{
			if (unit->unit.gunner_object_index==NONE)
			{
				object_get_marker_by_name(unit_index, "head", &marker, 1);
				*camera_position = marker.matrix.position;
			}
			else
			{
				struct unit_datum *gunner_unit = unit_get(unit->unit.gunner_object_index);
				struct unit_seat *parent_seat = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.seats, gunner_unit->unit.parent_seat_index, struct unit_seat);

				object_get_marker_by_name(unit_index, parent_seat->marker_name, &marker, 1);
				*camera_position = marker.matrix.position;
			}
		}
		else
		{
			struct object_datum *object = object_get(unit->object.parent_object_index);

			*camera_position = object->object.position;

			if (TEST_FLAG(_object_mask_unit, object->object.type) && unit->unit.parent_seat_index!=NONE)
			{
				struct unit_definition *parent_unit_definition = unit_definition_get(object->definition_index);
				struct unit_seat *parent_seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

				if (object->object.type!=_object_type_vehicle || parent_seat->camera.marker_name[0])
				{
					object_get_marker_by_name(unit->object.parent_object_index, parent_seat->camera.marker_name, &marker, 1);
					*camera_position = marker.matrix.position;
				}
			}
		}
	}
	else
	{
		biped_get_sight_position(unit_index, 0.f, NULL, NULL, NULL, camera_position);
	}

	return;
}

boolean unit_clip_to_aiming_bounds(
	long unit_index,
	real_vector3d *vector,
	boolean use_aiming_screen)
{

	boolean aiming;
	real_rectangle2d *bounds;

	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	if (use_aiming_screen)
	{
		aiming = unit->unit.animation.aiming_with_euler_screen;
		bounds = &unit->unit.animation.aiming_screen_bounds;
	}
	else
	{
		aiming = unit->unit.animation.looking_with_euler_screen;
		bounds = &unit->unit.animation.looking_screen_bounds;
	}

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 5608, vector);

	if (aiming)
	{
		real_matrix4x3 matrix;
		real_euler_angles2d relative_aiming_angles;
		real_vector3d relative_vector;

		matrix.scale = 1.f;
		object_get_orientation(unit_index, &matrix.forward, &matrix.up);
		cross_product3d(&matrix.up, &matrix.forward, &matrix.left);
		matrix.position = *global_origin3d;
		matrix4x3_inverse_transform_normal(&matrix, vector, &relative_vector);

		match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\units.c", 5626, &relative_vector);

		euler_angles2d_from_vector3d(&relative_aiming_angles, &relative_vector);

		match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 5629, relative_aiming_angles.pitch);
		match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 5630, relative_aiming_angles.yaw);

		if (relative_aiming_angles.yaw<bounds->x0)
		{
			relative_aiming_angles.yaw = bounds->x0;
			result = TRUE;
		}
		else
		{
			if (relative_aiming_angles.yaw>bounds->x1)
			{
				relative_aiming_angles.yaw = bounds->x1;
				result = TRUE;
			}
		}

		if (relative_aiming_angles.pitch<bounds->y0)
		{
			result = TRUE;
			relative_aiming_angles.pitch = bounds->y0;
		}
		else
		{
			if (relative_aiming_angles.pitch>bounds->y1)
			{
				result = TRUE;
				relative_aiming_angles.pitch = bounds->y1;
			}
		}

		if (result)
		{
			match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 5661, relative_aiming_angles.pitch);
			match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 5662, relative_aiming_angles.yaw);
			vector3d_from_euler_angles2d(&relative_vector, &relative_aiming_angles);

			match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\units.c", 5665, &relative_vector);
			matrix4x3_transform_normal(&matrix, &relative_vector, vector);
			match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\units.c", 5667, vector);
		}
	}

	return result;
}

long unit_inventory_get_weapon(
	long unit_index, 
	short index)
{
	struct unit_datum *unit = unit_get(unit_index);

	return unit_get_weapon(unit, index);
}

void unit_ready_desired_weapon(
	long unit_index,
	boolean immediate)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	long desired_weapon_index = unit_get_desired_weapon_index(unit_index);
	long current_weapon_index = unit_get_current_weapon_index(unit_index);

	if (current_weapon_index!=NONE && weapon_put_away(current_weapon_index, immediate))
	{
		object_detach(current_weapon_index);
		object_disconnect_from_map(current_weapon_index);
		object_activate(current_weapon_index);
		object_set_visibility(current_weapon_index, FALSE);
		item_in_unit_inventory(current_weapon_index, unit_index);
		unit->unit.current_weapon_index = NONE;
	}

	if (unit->unit.current_weapon_index==NONE)
	{
		if (desired_weapon_index!=NONE)
		{
			struct animation_graph *animation_graph;
			struct animation_graph_unit_seat *unit_seat;
			struct animation_graph_weapon_class *weapon_class;

			unit_set_or_test_seat_and_weapon_label(unit_index, unit_get_seat_label(unit_index), weapon_get_label(desired_weapon_index), TRUE);

			animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
			unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
			weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);

			object_reconnect_to_map(desired_weapon_index, NULL);
			object_set_visibility(desired_weapon_index, TRUE);
			object_attach_to_marker(unit_index, weapon_class->hand_marker_name, desired_weapon_index, weapon_class->grip_marker_name);
			unit->unit.current_weapon_index = unit->unit.desired_weapon_index;
			
			if (unit->unit.desired_weapon_index!=NONE)
			{
				unit->unit.weapon_last_used_at_game_time[unit->unit.current_weapon_index] = game_time_get();
			}

			weapon_ready(desired_weapon_index);
		}
		else
		{
			unit_set_or_test_seat_and_weapon_label(unit_index, unit_get_seat_label(unit_index), base_weapon_label_get(_unit_base_weapon_none), TRUE);
			unit->unit.current_weapon_index = NONE;
		}
	}

	unit_unzoom(unit_index);

	return;
}

boolean unit_drop_current_weapon(
	long unit_index,
	boolean immediate)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	long current_weapon_index = unit_get_current_weapon_index(unit_index);
	long next_index = unit_weapon_next_index(unit_index, unit->unit.current_weapon_index, 1);

	boolean result = FALSE;

	if (current_weapon_index != NONE &&
		(next_index != unit->unit.current_weapon_index || immediate) &&
		!TEST_FLAG(object_get(current_weapon_index)->object.flags, _object_invisible_bit) &&
		weapon_put_away(current_weapon_index, immediate))
	{
		first_person_weapon_message_from_unit(unit_index, 13);
		unit_drop_item(unit_index, current_weapon_index);
		unit->unit.weapon_object_indices[unit->unit.current_weapon_index] = NONE;
		unit->unit.current_weapon_index = NONE;
		unit->unit.desired_weapon_index = unit_weapon_next_index(unit_index, NONE, 0);
		
		if (!weapon_can_be_fired(current_weapon_index))
		{
			object_delete(current_weapon_index);
		}

		result = TRUE;
	}

	return result;
}

/* ---------- private code */

char const *base_seat_labels[NUMBER_OF_UNIT_BASE_SEATS] = {"asleep", "alert", "stand", "crouch", "flee", "flaming"};

static char const *base_seat_label_get(
	short base_seat_index)
{
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 8214, base_seat_index>=0 && base_seat_index<NUMBER_OF_UNIT_BASE_SEATS);
	return base_seat_labels[base_seat_index];
}

static short seat_label_to_base_seat_index(
	char const *seat_label)
{
	short seat_index;
	short result = NONE;

	for (seat_index = 0; seat_index<NUMBER_OF_UNIT_BASE_SEATS; ++seat_index)
	{
		if (!_stricmp(seat_label, base_seat_labels[seat_index]))
		{
			result = seat_index;
			break;
		}
	}

	return result;
}

static char const *base_weapon_label_get(
	short base_weapon_index)
{
	char const *base_weapon_labels[NUMBER_OF_UNIT_BASE_WEAPONS] = {"unarmed"};

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 8266, base_weapon_index>=0 && base_weapon_index<NUMBER_OF_UNIT_BASE_WEAPONS);

	return base_weapon_labels[base_weapon_index];
}

static void unit_refresh_illumination(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_datum *parent_unit = unit_try_and_get(unit->object.parent_object_index);

	if (parent_unit==NULL)
	{
		real_rgb_color color;
		lights_illumination_at_point(&unit->object.position, &unit->object.location, &color);
		unit->unit.ambient_illumination = real_rgb_color_brightness(&color);
		unit->unit.self_illumination = object_get_self_illumination(unit_index);
	}
	else
	{
		unit->unit.ambient_illumination = parent_unit->unit.ambient_illumination;
		unit->unit.self_illumination = parent_unit->unit.self_illumination;
	}

	return;
}

static void unit_adjust_for_seat_change(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	unit->unit.desired_weapon_index = unit_weapon_next_index(unit_index, unit->unit.current_weapon_index, 0);
	unit_ready_desired_weapon(unit_index, TRUE);
	return;
}

static short unit_weapon_next_index(
	long unit_index,
	short current_index,
	short delta)
{
	long selected_weapon_last_used_at_game_time;
	short inventory_index;
	
	struct unit_datum *unit = unit_get(unit_index);
	short selected_weapon_index = NONE;

	if (current_index==NONE)
	{
		current_index = 0;
	}

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7744, current_index>=0 && current_index<MAXIMUM_WEAPONS_PER_UNIT);
	
	inventory_index = current_index;
	
	do
	{
		boolean current_weapon_must_be_readied;
		boolean select_weapon;
		long current_weapon_last_used_at_game_time = unit->unit.weapon_object_indices[inventory_index];

		unit_get(unit_index);
		weapon_get(unit_index);

		if (current_weapon_last_used_at_game_time !=NONE && unit_can_use_weapon(unit_index, current_weapon_last_used_at_game_time))
		{
			selected_weapon_last_used_at_game_time = unit->unit.weapon_last_used_at_game_time[inventory_index];

			if (delta || selected_weapon_index == NONE || selected_weapon_last_used_at_game_time < current_weapon_last_used_at_game_time)
			{
				selected_weapon_index = inventory_index;
			}

			if (weapon_must_be_readied(current_weapon_last_used_at_game_time) || inventory_index != current_index)
			{
				break;
			}
		}

		if (delta>=0)
		{
			inventory_index = inventory_index == (MAXIMUM_WEAPONS_PER_UNIT-1) ? 0 : inventory_index+1;
		}
		else
		{
			inventory_index = inventory_index ? inventory_index-1 : (MAXIMUM_WEAPONS_PER_UNIT-1);
		}

	}
	while (inventory_index != current_index);

	return selected_weapon_index;
}

static void unit_throw_grenade_move_to_hand(
	long unit_index)
{
	long object_index;

	struct object_placement_data placement_data;
	struct object_marker marker;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct game_globals_grenade *grenades = TAG_BLOCK_GET_ELEMENT(
		&scenario_get_game_globals()->grenades,
		unit->unit.current_grenade_index,
		struct game_globals_grenade);

	if ((
		unit->unit.player_index==NONE ||
		!cheat.infinite_ammo && !game_engine_infinite_grenades(unit->unit.player_index)) &&
		(unit->unit.actor_index==NONE ||
		!actor_has_unlimited_grenades(unit->unit.actor_index)))
	{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 7966, unit->unit.current_grenade_index>=0 && unit->unit.current_grenade_index<NUMBER_OF_UNIT_GRENADE_TYPES);
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 7967, unit->unit.grenade_counts[unit->unit.current_grenade_index]>0);
		--unit->unit.grenade_counts[unit->unit.current_grenade_index];
	}

	object_get_marker_by_name(unit_index, "left hand", &marker, 1);
	object_placement_data_new(&placement_data, grenades->projectile.index, unit_index);
	SET_FLAG(placement_data.flags, 1, TRUE);
	placement_data.forward= unit_get(unit_index)->unit.aiming_vector;
	normalize3d(perpendicular3d(&placement_data.forward, &placement_data.up));

	placement_data.position = marker.matrix.position;
	object_index = object_new(&placement_data);

	if (object_index!=NONE)
	{
		object_attach_to_node(unit_index, object_index, marker.node_index);
		unit->unit.grenade_object_index = object_index;
		unit->unit.grenade_throw_state = _unit_grenade_throw_in_hand;
	}
	else
	{
		unit->unit.grenade_throw_state = _unit_grenade_throw_ending;
	}

	return;
}

static boolean unit_animation_busy(
	struct unit_animation *animation)
{
	boolean result = FALSE;

	switch (animation->state)
	{
	case _unit_state_hard_ping:
	case _unit_state_dying_airborne:
	case _unit_state_dying:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_ai_impulse:
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_melee_continuous:
	case _unit_state_throw_grenade:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
	case _unit_state_leap_start:
	case _unit_state_leap_melee:
		result = TRUE;
		break;
	default:
		break;
	}

	return result;
}

static boolean unit_set_or_test_seat_and_weapon_label(
	long object_index,
	char const *seat_label,
	char const *weapon_label,
	boolean change_flag)
{
	short seat_index;

	struct unit_datum *unit = unit_get(object_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
	boolean result = FALSE;

	for (seat_index = 0; seat_index<animation_graph->unit_seats.count; ++seat_index)
	{
		struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, seat_index, struct animation_graph_unit_seat);

		if (!seat_label || !_stricmp(seat_label, unit_seat->label))
		{
			long weapon_class_index;

			for (weapon_class_index = 0; weapon_class_index<unit_seat->weapon_classes.count; ++weapon_class_index)
			{
				long weapon_type_index;

				struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, weapon_class_index, struct animation_graph_weapon_class);

				for (weapon_type_index = 0; weapon_type_index<weapon_class->weapon_types.count; ++weapon_type_index)
				{
					struct animation_graph_weapon_type *weapon_type = TAG_BLOCK_GET_ELEMENT(&weapon_class->weapon_types, weapon_type_index, struct animation_graph_weapon_type);
					
					if (*weapon_label=='\0' ||
						!strcmp(weapon_label, "unarmed") &&
						weapon_type->label[0]=='\0'||
						!_stricmp(weapon_label, weapon_type->label))
					{
						if (change_flag)
						{
							long anim_2 =
								unit_seat->animations.count <= 2 ?
								NONE :
								animation_graph_animation_index_get(&unit_seat->animations)[2].animation_index;
							boolean showing_acceleration = TRUE;
							
							if (anim_2==NONE)
							{
								long anim_3 =
									unit_seat->animations.count <= 3 ?
									NONE :
									animation_graph_animation_index_get(&unit_seat->animations)[3].animation_index;
								
								if (anim_3==NONE)
								{
									long anim_4 =
										unit_seat->animations.count <= 4 ?
										NONE :
										animation_graph_animation_index_get(&unit_seat->animations)[4].animation_index;
									showing_acceleration = FALSE;
								}
							}

							if (unit->unit.animation.state!=_unit_state_user_animation)
							{
								unit->unit.animation.state = NONE;
							}

							unit->unit.animation.weapon_type_index = weapon_type_index;
							unit->unit.animation.base_seat_index = seat_label_to_base_seat_index(seat_label);
							unit->unit.animation.weapon_index = weapon_class_index;

							SET_FLAG(unit->unit.animation.flags, _unit_animation_showing_acceleration_bit, showing_acceleration);
						}

						result = TRUE;
						break;
					}
				}
			}
		}
	}

	return result;
}

static boolean unit_animation_set_state(
	long unit_index,
	short new_state)
{
	short interpolation_frame_count;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
	struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
	struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);
	struct animation_graph_weapon_type *weapon_type = TAG_BLOCK_GET_ELEMENT(&weapon_class->weapon_types, unit->unit.animation.weapon_type_index, struct animation_graph_weapon_type);
	
	boolean valid_state = unit->unit.animation.state!=NONE;
	boolean v1 = FALSE;
	boolean result = TRUE;

	if (unit->unit.animation.state!=NONE || new_state!=unit->unit.animation.state)
	{
		boolean valid_animation_index;
		long animation_index;

		long weapon_class_animation_index = NONE;
		long unit_seat_animation_index = NONE;

		if (unit->unit.animation.state==_unit_state_throw_grenade)
		{
			unit_throw_grenade_release(unit_index, TRUE);
		}

		switch (new_state)
		{
		case _unit_state_idle:
			weapon_class_animation_index = _unit_weapon_class_animation_idle;
			break;
		case _unit_state_gesture:
			weapon_class_animation_index = _unit_weapon_class_animation_gesture;
			break;
		case _unit_state_turn_left:
			weapon_class_animation_index = _unit_weapon_class_animation_turning_left;
			break;
		case _unit_state_turn_right:
			weapon_class_animation_index = _unit_weapon_class_animation_turning_right;
			break;
		case _unit_state_move_front:
			weapon_class_animation_index = _unit_weapon_class_animation_diving_front;
			break;
		case _unit_state_move_back:
			weapon_class_animation_index = _unit_weapon_class_animation_diving_back;
			break;
		case _unit_state_move_left:
			weapon_class_animation_index = _unit_weapon_class_animation_diving_left;
			break;
		case _unit_state_move_right:
			weapon_class_animation_index = _unit_weapon_class_animation_diving_right;
			break;
		case _unit_state_stunned_move_front:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_wounded_front;
			break;
		case _unit_state_stunned_move_back:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_wounded_back;
			break;
		case _unit_state_stunned_move_left:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_wounded_left;
			break;
		case _unit_state_stunned_move_right:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_wounded_right;
			break;
		case _unit_state_slide_front:
			weapon_class_animation_index = _unit_weapon_class_animation_sliding_front;
			break;
		case _unit_state_slide_back:
			weapon_class_animation_index = _unit_weapon_class_animation_sliding_back;
			break;
		case _unit_state_slide_left:
			weapon_class_animation_index = _unit_weapon_class_animation_sliding_left;
			break;
		case _unit_state_slide_right:
			weapon_class_animation_index = _unit_weapon_class_animation_sliding_right;
			break;
		case _unit_state_flying_front:
			unit_seat_animation_index = _unit_seat_animation_flying_front;
			break;
		case _unit_state_flying_back:
			unit_seat_animation_index = _unit_seat_animation_flying_back;
			break;
		case _unit_state_flying_left:
			unit_seat_animation_index = _unit_seat_animation_flying_left;
			break;
		case _unit_state_flying_right:
			unit_seat_animation_index = _unit_seat_animation_flying_right;
			break;
		case _unit_state_airborne:
			weapon_class_animation_index = _unit_weapon_class_animation_airborne;
			break;
		case _unit_state_land_soft:
			weapon_class_animation_index = _unit_weapon_class_animation_land_soft;
			break;
		case _unit_state_land_hard:
			weapon_class_animation_index = _unit_weapon_class_animation_land_hard;
			break;
		case _unit_state_dying_airborne:
			unit_seat_animation_index = _unit_seat_animation_airborne_dead;
			break;
		case _unit_state_dying:
			unit_seat_animation_index = _unit_seat_animation_landing_dead;
			break;
		case _unit_state_melee_attack:
			weapon_class_animation_index = _unit_weapon_class_animation_melee_attack;
			break;
		case _unit_state_melee_airborne:
			weapon_class_animation_index = _unit_weapon_class_animation_melee_airborne;
			break;
		case _unit_state_melee_continuous:
			weapon_class_animation_index = _unit_weapon_class_animation_melee_continuous;
			break;
		case _unit_state_throw_grenade:
			weapon_class_animation_index = _unit_weapon_class_animation_throw_grenade;
			break;
		case _unit_state_resurrect_front:
			weapon_class_animation_index = _unit_weapon_class_animation_resurrect_front;
			break;
		case _unit_state_resurrect_back:
			weapon_class_animation_index = _unit_weapon_class_animation_resurrect_back;
			break;
		case _unit_state_feeding:
			weapon_class_animation_index = _unit_weapon_class_animation_feeding;
			break;
		case _unit_state_opening:
			unit_seat_animation_index = _unit_seat_animation_opening;
			break;
		case _unit_state_closing:
			unit_seat_animation_index = _unit_seat_animation_closing;
			break;
		case _unit_state_leap_start:
			weapon_class_animation_index = _unit_weapon_class_animation_leap_start;
			break;
		case _unit_state_leap_airborne:
			weapon_class_animation_index = _unit_weapon_class_animation_leap_airborne;
			break;
		case _unit_state_leap_melee:
			weapon_class_animation_index = _unit_weapon_class_animation_leap_melee;
			break;
		case _unit_state_hovering:
			unit_seat_animation_index = _unit_seat_animation_hovering;
			break;
		default:
			break;
		}

		// Get weapon class animation
		if (weapon_class_animation_index!=NONE)
		{
			if (weapon_class_animation_index>=0 && weapon_class_animation_index<weapon_class->animations.count)
			{
				animation_index = animation_graph_animation_index_get(&weapon_class->animations)[weapon_class_animation_index].animation_index;
			}
			else
			{
				animation_index = NONE;
			}
		}
		// Get unit seat animation
		else
		{
			if (unit_seat_animation_index!=NONE)
			{
				if (unit_seat_animation_index>=0 && unit_seat_animation_index<unit_seat->animations.count)
				{
					animation_index = animation_graph_animation_index_get(&unit_seat->animations)[unit_seat_animation_index].animation_index;
				}
				else
				{
					animation_index = NONE;
				}
			}
			else
			{
				animation_index = NONE;
			}
		}

		valid_animation_index = animation_index >= 0 && animation_index <animation_graph->animations.count;
		
		if (debug_unit_animations && unit->object.type==_object_type_biped && !valid_animation_index)
		{
			/*
			if (weapon_class_animation_index == NONE)
				Format = &Format_;
			else
				Format = j_animation_list_get_string(word_12C47CC, weapon_class_animation_index);
			if (unit_seat_animation_index == -1)
				element_with_size_2 = weapon_class;
			else
				element_with_size_2 = j_animation_list_get_string(&dword_12C47D4, unit_seat_animation_index);
			v2 = sub_86262C(unit_definition + 56);
			v3 = j_tag_name_strip_path(v2);
			
			console_warning(
				"MISSING: %s '%s %s %s'",
				tag_name_strip_path(unit_definition->object.animation_graph.name),
				unit_seat->label,
				weapon_class->label,
				animation_list_get_string(weapon_class_animation_list, aiming_screen_index);
				*/
		}
	}

	if (result)
	{

	}

	return result;
}

static boolean unit_verify_inventory(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	boolean result = valid_real_normal3d(&unit->unit.desired_facing_vector) &&
		valid_real_normal3d(&unit->unit.desired_aiming_vector) &&
		valid_real_normal3d(&unit->unit.desired_looking_vector) &&
		valid_real_vector3d_axes2(&unit->object.forward, &unit->object.up) &&
		valid_real_normal3d(&unit->unit.aiming_vector) &&
		valid_real_normal3d(&unit->unit.looking_vector);

	return result;
}

static void unit_throw_grenade_release(
	long unit_index,
	boolean premature)
{
	real_vector3d initial_velocity;
	real_point3d origin;
	real_vector3d forward;
	real_vector3d left;
	real_vector3d up;
	real_point3d new_grenade_origin;
	real_point3d camera_position;

	struct unit_datum* unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit->unit.grenade_throw_state==_unit_grenade_throw_in_hand)
	{
		if (unit->unit.grenade_object_index!=NONE)
		{
			long grenade_index = unit->unit.grenade_object_index;
			
			object_detach(grenade_index);
			
			// Get new grenade origin from the actor
			if (unit->unit.actor_index!=NONE)
			{
				object_get_origin(grenade_index, &origin);
				actor_aim_grenade(unit->unit.actor_index, &origin, &initial_velocity);
			}
			// Calculate the new position if unit is not an actor
			else
			{
				if (unit->unit.player_index!=NONE)
				{
					struct game_globals_player_information* player_information = TAG_BLOCK_GET_ELEMENT(
						&scenario_get_game_globals()->player_information,
						0,
						struct game_globals_player_information);

					forward = unit->unit.aiming_vector;
					
					if (normalize3d(cross_product3d(global_up3d, &forward, &up))==0.f)
					{
						up = *global_up3d;
					}

					normalize3d(cross_product3d(&forward, &up, &left));
					
					unit_get_camera_position(unit_index, &camera_position);
					point_from_line3d(&camera_position, &forward, player_information->grenade_origin.i, &camera_position);
					point_from_line3d(&camera_position, &up, player_information->grenade_origin.j, &camera_position);
					point_from_line3d(&camera_position, &left, player_information->grenade_origin.k, &camera_position);
					
					object_translate(grenade_index, &camera_position, NULL);
				}

				{
					real scale = unit_definition->unit.grenade_velocity/TICKS_PER_SECOND;
					scale_vector3d(&unit->unit.aiming_vector, scale, &initial_velocity);
				}
			}

			if (premature)
			{
				real power = (real)unit->unit.grenade_throw_ticks/(real)unit->unit.grenade_throw_full_power_ticks;

				if (power < 1.f)
				{
					real_vector3d aiming_vector;

					real range = real_random_range(0.020000001f, 0.046666667f);

					scale_vector3d(&unit->unit.aiming_vector, range, &aiming_vector);
					scale_vector3d(&initial_velocity, power, &initial_velocity);
					point_from_line3d((real_point3d *)&initial_velocity, &aiming_vector, 1.f-power, (real_point3d *)&initial_velocity);
				}
			}

			{
				struct object_datum *grenade = object_get(grenade_index);

				subtract_vectors3d(&initial_velocity, &grenade->object.translational_velocity, &initial_velocity);
				projectile_accelerate(grenade_index, &initial_velocity);
			}
			
			unit->unit.grenade_throw_state = _unit_grenade_throw_ending;
			unit->unit.grenade_object_index = NONE;

			unit_get_camera_position(unit_index, &new_grenade_origin);
			
			if (!object_force_inside_bsp(grenade_index, &new_grenade_origin))
			{
				object_delete(grenade_index);
			}
		}
		else
		{
			unit->unit.grenade_throw_state = _unit_grenade_throw_ending;
		}
	}

	return;
}

static void unit_seat_update(
	long object_index)
{
	real_vector3d *acceleration_scale;
	real_vector3d new_velocity;
	struct object_marker seat_marker;

	struct unit_datum *unit = unit_get(object_index);
	boolean found = FALSE;

	if (unit->object.parent_object_index!=NONE && unit->unit.parent_seat_index!=NONE)
	{
		struct unit_definition *parent_unit_definition = unit_definition_get(unit_get(unit->object.parent_object_index)->definition_index);
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

		if (object_get_marker_by_name(unit->object.parent_object_index, seat->marker_name, &seat_marker, 1))
		{
			object_get_origin(unit->object.parent_object_index, &seat_marker.matrix.position);
			acceleration_scale = &seat->acceleration_scale;
			found = TRUE;
		}
	}
	else
	{
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
		
		seat_marker.matrix.position = unit->object.position;
		seat_marker.matrix.forward = unit->object.forward;
		seat_marker.matrix.up = unit->object.up;
		acceleration_scale = &unit_definition->unit.acceleration_scale;
		found = TRUE;
	}

	if (found)
	{
		real_vector3d vp;
		real_vector3d last_velocity;
		last_velocity.i = seat_marker.matrix.position.x-unit->unit.seat_last_position.x;
		last_velocity.j = seat_marker.matrix.position.y-unit->unit.seat_last_position.y;
		last_velocity.k = seat_marker.matrix.position.z-unit->unit.seat_last_position.z;

		new_velocity.i = last_velocity.i - unit->unit.seat_last_velocity.i;
		new_velocity.j = last_velocity.j - unit->unit.seat_last_velocity.j;
		new_velocity.k = last_velocity.k - unit->unit.seat_last_velocity.k;
		
		cross_product3d(&seat_marker.matrix.up, &seat_marker.matrix.forward, &vp);
		
		unit->unit.seat_desired_acceleration.i = (dot_product3d(&new_velocity, &seat_marker.matrix.forward) * acceleration_scale->i) + 0.5f;
		unit->unit.seat_desired_acceleration.j = (dot_product3d(&new_velocity, &vp) * acceleration_scale->j) + 0.5f;
		unit->unit.seat_desired_acceleration.k = (dot_product3d(&new_velocity, &seat_marker.matrix.up) * acceleration_scale->k) + 0.5f;

		unit->unit.seat_desired_acceleration.i = PIN(unit->unit.seat_desired_acceleration.i, 0.f, 1.f);
		unit->unit.seat_desired_acceleration.j = PIN(unit->unit.seat_desired_acceleration.j, 0.f, 1.f);
		unit->unit.seat_desired_acceleration.k = PIN(unit->unit.seat_desired_acceleration.k, 0.f, 1.f);

		unit->unit.seat_last_position = seat_marker.matrix.position;
		unit->unit.seat_last_velocity = last_velocity;
	}
	else
	{
		unit->unit.seat_desired_acceleration.k = 0.5f;
		unit->unit.seat_desired_acceleration.j = 0.5f;
		unit->unit.seat_desired_acceleration.i = 0.5f;
	}

	return;
}

static char const *unit_get_seat_label(
	long object_index)
{
	char const *result;

	struct unit_datum *unit = unit_get(object_index);

	if (unit->object.parent_object_index==NONE || unit->unit.parent_seat_index==NONE)
	{
		short base_seat_index = unit->unit.animation.base_seat_index;
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 8214, base_seat_index>=0 && base_seat_index<NUMBER_OF_UNIT_BASE_SEATS);
		result = base_seat_labels[base_seat_index];
	}
	else
	{
		struct unit_datum *parent_unit = unit_get(unit->object.parent_object_index);
		struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
		result = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat)->label;
	}

	return result;
}

static long unit_get_weapon(
	struct unit_datum *unit,
	short index)
{
	long result = NONE;

	if (index!=NONE)
	{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 8371, index>=0 && index<MAXIMUM_WEAPONS_PER_UNIT);
		result = unit->unit.weapon_object_indices[index];
	}

	return result;
}

static void unit_drop_item(
	long unit_index,
	long item_index)
{
	real_vector3d unit_velocity;
	real_vector3d item_velocity;
	real_point3d camera_position;

	struct unit_datum *unit = unit_get(unit_index);
	struct item_datum *item = item_get(item_index);
	
	if (item->object.parent_object_index==NONE)
	{
		object_reconnect_to_map(item_index, 0);
		object_set_visibility(item_index, 1);
		object_attach_to_marker(unit_index, "left hand", item_index, "");
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 8396, item->object.parent_object_index==unit_index)
	}

	item_in_unit_inventory(item_index, NONE);
	object_detach(item_index);
	
	item->object.translational_velocity = *global_zero_vector3d;
	item->object.angular_velocity = *global_zero_vector3d;

	random_vector_in_cone3d(&unit->unit.aiming_vector, 0.f, 0.39269909f, &item_velocity);
	
	scale_vector3d(&item_velocity, real_random_range(0.026666667f, 0.040000003f), &item_velocity);
	object_get_velocities(unit_index, &unit_velocity, NULL);
	add_vectors3d(&item_velocity, &unit_velocity, &item_velocity);
	
	item_accelerate(item_index, &item_velocity, FALSE);

	unit_get_camera_position(unit_index, &camera_position);
	
	if (!object_force_inside_bsp(item_index, &camera_position) && !game_engine_running())
	{
		object_delete(item_index);
	}

	if (TEST_FLAG(unit->unit.flags, _unit_doesnt_drop_items_bit))
	{
		object_delete(item_index);
	}

	return;
}

// TODO: Fix
static void unit_cause_continuous_melee_damage(
	long unit_index)
{
	struct collision_model_test_vector_result vector_result;
	real_point3d collision_origin;
	real_point3d collision_point;

	struct collision_model_instance instance;
	real_vector3d collision_vector;
	real_plane3d collision_plane;
	struct damage_data damage_data;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit->unit.melee_attack_state==4 &&
		unit->object.parent_object_index!=NONE &&
		unit_definition->unit.melee_damage.index!=NONE)
	{
		boolean collision_passed = FALSE;

		if (unit->unit.melee_continuous_damage_effect_timer==0)
		{
			match_assert("c:\\halo\\SOURCE\\units\\units.c", 8941, global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH);

			global_current_collision_users[global_current_collision_user_depth++] = 8;

			if (collision_model_instance_new(&instance, unit->object.parent_object_index))
			{
				object_get_origin(unit_index, &collision_origin);
				scale_vector3d(&unit->object.forward, 0.2f, &collision_vector);
				point_from_line3d(&collision_origin, &collision_vector, -0.5f, &collision_origin);
				
				if (collision_model_test_vector(
					&instance,
					FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_back_facing_surfaces_bit),
					&collision_origin,
					&collision_vector,
					&vector_result))
				{
					point_from_line3d(&collision_origin, &collision_vector, vector_result.bsp_result.t, &collision_point);
					matrix4x3_transform_plane(&instance.matrices[vector_result.node_index], vector_result.bsp_result.plane, &collision_plane);
					
					if (0>vector_result.bsp_result.plane_designator)
					{
						plane3d_negate(&collision_plane, &collision_plane);
					}

					collision_passed = TRUE;
				}
			}

			match_assert("c:\\halo\\SOURCE\\units\\units.c", 8965, global_current_collision_user_depth > 1);
			--global_current_collision_user_depth;
		}
		
		damage_data_new(&damage_data, unit_definition->unit.melee_damage.index);
		
		damage_data.owner_object_index = unit_index;
		damage_data.owner_team_index = unit->object.owner_team_index;
		damage_data.owner_player_index = unit->unit.player_index;
		damage_data.scale = 1.f/30.f;

		if (collision_passed)
		{
			damage_data.epicenter = collision_point;
			damage_data.origin = collision_point;
			damage_data.direction = unit->object.forward;

			SET_FLAG(damage_data.flags, 1, TRUE);
			unit->unit.melee_continuous_damage_effect_timer = 10;

			object_cause_damage(
				&damage_data,
				unit->object.parent_object_index, 
				vector_result.node_index,
				vector_result.region_index,
				vector_result.bsp_result.material_index,
				&collision_plane.n);
		}
		else
		{
			object_cause_damage(
				&damage_data,
				unit->object.parent_object_index,
				NONE,
				NONE,
				NONE,
				NULL);
		}

		--unit->unit.melee_continuous_damage_effect_timer;
	}

	return;
}

static void unit_verify_vectors(
	long unit_index,
	char const *debugstring)
{
	char buffer[512];

	if (!unit_verify_inventory(unit_index))
	{
		struct unit_datum *unit = unit_get(unit_index);

		ai_debug_describe_actor(
			unit->unit.actor_index==NONE ? unit->unit.swarm_actor_index : unit->unit.actor_index,
			unit_index,
			1,
			buffer,
			NUMBEROF(buffer)
		);
		error(_error_silent, "**** unit_verify_vectors: problems with %s at location %s", buffer, debugstring);
		error(
			_error_silent,
			"  object: pos %f %f %f, fwd %f %f %f, up %f %f %f",
			unit->object.position.x,
			unit->object.position.y,
			unit->object.position.z,
			unit->object.forward.i,
			unit->object.forward.j,
			unit->object.forward.k,
			unit->object.up.i,
			unit->object.up.j,
			unit->object.up.k
		);
		error(
			_error_silent,
			"  desired facing %f %f %f, aiming %f %f %f, looking %f %f %f",
			unit->unit.desired_facing_vector.i,
			unit->unit.desired_facing_vector.j,
			unit->unit.desired_facing_vector.k,
			unit->unit.desired_aiming_vector.i,
			unit->unit.desired_aiming_vector.j,
			unit->unit.desired_aiming_vector.k,
			unit->unit.desired_looking_vector.i,
			unit->unit.desired_looking_vector.j,
			unit->unit.desired_looking_vector.k
		);
		error(
			_error_silent,
			"  aiming vector %f %f %f velocity %f %f %f",
			unit->unit.aiming_vector.i,
			unit->unit.aiming_vector.j,
			unit->unit.aiming_vector.k,
			unit->unit.aiming_velocity.i,
			unit->unit.aiming_velocity.j,
			unit->unit.aiming_velocity.k
		);
		error(
			_error_silent,
			"  looking vector %f %f %f velocity %f %f %f",
			unit->unit.looking_vector.i,
			unit->unit.looking_vector.j,
			unit->unit.looking_vector.k,
			unit->unit.looking_velocity.i,
			unit->unit.looking_velocity.j,
			unit->unit.looking_velocity.k
		);
		
		error(_error_silent, "  warning, hex dump follows...");

		error(
			_error_silent,
			"  object: pos %08X %08X %08X, fwd %08X %08X %08X, up %08X %08X %08X",
			*(long *)&unit->object.position.x,
			*(long *)&unit->object.position.y,
			*(long *)&unit->object.position.z,
			*(long *)&unit->object.forward.i,
			*(long *)&unit->object.forward.j,
			*(long *)&unit->object.forward.k,
			*(long *)&unit->object.up.i,
			*(long *)&unit->object.up.j,
			*(long *)&unit->object.up.k
		);
		error(
			_error_silent,
			"  desired facing %08X %08X %08X, aiming %08X %08X %08X, looking %08X %08X %08X",
			*(long *)&unit->unit.desired_facing_vector.i,
			*(long *)&unit->unit.desired_facing_vector.j,
			*(long *)&unit->unit.desired_facing_vector.k,
			*(long *)&unit->unit.desired_aiming_vector.i,
			*(long *)&unit->unit.desired_aiming_vector.j,
			*(long *)&unit->unit.desired_aiming_vector.k,
			*(long *)&unit->unit.desired_looking_vector.i,
			*(long *)&unit->unit.desired_looking_vector.j,
			*(long *)&unit->unit.desired_looking_vector.k
		);
		error(
			_error_silent,
			"  aiming vector %08X %08X %08X velocity %08X %08X %08X",
			*(long *)&unit->unit.aiming_vector.i,
			*(long *)&unit->unit.aiming_vector.j,
			*(long *)&unit->unit.aiming_vector.k,
			*(long *)&unit->unit.aiming_velocity.i,
			*(long *)&unit->unit.aiming_velocity.j,
			*(long *)&unit->unit.aiming_velocity.k
		);
		error(
			_error_silent,
			"  looking vector %08X %08X %08X velocity %08X %08X %08X",
			*(long *)&unit->unit.looking_vector.i,
			*(long *)&unit->unit.looking_vector.j,
			*(long *)&unit->unit.looking_vector.k,
			*(long *)&unit->unit.looking_velocity.i,
			*(long *)&unit->unit.looking_velocity.j,
			*(long *)&unit->unit.looking_velocity.k
		);

		match_vassert(
			"c:\\halo\\SOURCE\\units\\units.c",
			594,
			unit_verify_inventory(unit_index),
			"unit_verify_vectors FAILURE, see above for details"
		);
	}
	
	return;
}

static void unit_running_blind(
	long unit_index,
	real_vector3d *run_vector)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean actor_controlled = FALSE;
	
	if (unit->unit.actor_index==NONE || !actor_get_running_blind_vector(unit->unit.actor_index, run_vector))
	{
		*run_vector = *global_forward3d;
	}
	else
	{
		actor_controlled = TRUE;
	}

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 9541, run_vector);

	{
		real angular_acceleration_this_tick;

		real negative_angle_allowed = 1.f;
		real positive_angle_allowed = 1.f;

		if (actor_controlled)
		{
			real negative_angle_bounds_dist = DEGREES_TO_RADIANS(45)-unit->unit.run_blindly_angle;
			real positive_angle_bounds_dist = DEGREES_TO_RADIANS(45)+unit->unit.run_blindly_angle;

			negative_angle_allowed = MIN(1.f, negative_angle_bounds_dist / DEGREES_TO_RADIANS(13.5f));
			positive_angle_allowed = MIN(1.f, positive_angle_bounds_dist / DEGREES_TO_RADIANS(13.5f));
		}

		{
			real negative_velocity_bounds_dist = DEGREES_TO_RADIANS(12.f)-unit->unit.run_blindly_angle_delta;
			real positive_velocity_bounds_dist = DEGREES_TO_RADIANS(12.f)+unit->unit.run_blindly_angle_delta;

			negative_angle_allowed = MIN(negative_angle_allowed, negative_velocity_bounds_dist / DEGREES_TO_RADIANS(3.6f));
			positive_angle_allowed = MIN(positive_velocity_bounds_dist / DEGREES_TO_RADIANS(3.6f), positive_angle_allowed);

		}

		if (negative_angle_allowed>=positive_angle_allowed)
		{
			if (positive_angle_allowed>-1.f)
			{
				real max_allowed = MIN(1.f, positive_angle_allowed);
				angular_acceleration_this_tick = real_random_range(-DEGREES_TO_RADIANS(1.2f) * max_allowed, DEGREES_TO_RADIANS(1.2f));
			}
			else
			{
				angular_acceleration_this_tick = DEGREES_TO_RADIANS(1.2f);
			}
		}
		else if (negative_angle_allowed<-1.f)
		{
			angular_acceleration_this_tick = -DEGREES_TO_RADIANS(1.2f);
		}
		else
		{
			real min_allowed = MIN(1.f, negative_angle_allowed);
			angular_acceleration_this_tick = real_random_range(-DEGREES_TO_RADIANS(1.2f), DEGREES_TO_RADIANS(1.2f) * min_allowed);
		}

		unit->unit.run_blindly_angle_delta+=angular_acceleration_this_tick;
	}

	unit->unit.run_blindly_angle += unit->unit.run_blindly_angle_delta;
	unit->unit.run_blindly_angle += 2.f * (-PIN(unit->unit.run_blindly_angle, -_pi, _pi));
	
	rotate_vector_about_axis(run_vector, global_up3d, sine(unit->unit.run_blindly_angle), cosine(unit->unit.run_blindly_angle));

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 9612, run_vector)

	return;
}

static boolean unit_integrated_night_vision_is_active(
	long unit_index)
{
	boolean active = FALSE;

	if (unit_get(unit_index)->unit.current_zoom_level!=NONE)
	{
		long current_weapon_index = unit_get_current_weapon_index(unit_index);

		if (current_weapon_index!=NONE)
		{
			struct weapon_datum *weapon = weapon_get(current_weapon_index);
			struct weapon_definition* weapon_definition = weapon_definition_get(weapon->definition_index);
			
			if (TEST_FLAG(weapon_definition->weapon.flags, _weapon_enables_unit_integrated_night_vision_bit))
			{
				active = TRUE;
			}
		}
	}

	return active;
}
