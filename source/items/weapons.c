/*
WEAPONS.C

symbols in this file:
000EA4D0 0010:
	_animation_convert_frame_to_pal (0000)
000EA4E0 0010:
	_animation_key_frame_index (0000)
000EA4F0 0020:
	_animation_update (0000)
000EA510 0020:
	_animation_choose_random_permutation (0000)
000EA530 0010:
	_weapons_initialize (0000)
000EA540 0010:
	_weapons_initialize_for_new_map (0000)
000EA550 0010:
	_weapons_dispose_from_old_map (0000)
000EA560 0010:
	_weapons_dispose (0000)
000EA570 00d0:
	_weapon_place (0000)
000EA640 0050:
	_weapon_preprocess_node_orientations (0000)
000EA690 0030:
	_weapon_get_label (0000)
000EA6C0 0020:
	_weapon_set_integrated_light_power (0000)
000EA6E0 0080:
	_weapon_estimate_time_to_target (0000)
000EA760 0090:
	_weapon_can_be_fired (0000)
000EA7F0 0030:
	_weapon_useful (0000)
000EA820 0070:
	_weapon_compute_movement_penalty (0000)
000EA890 0010:
	_weapon_melee_attack (0000)
000EA8A0 0030:
	_weapon_must_be_readied (0000)
000EA8D0 0030:
	_weapon_is_flag (0000)
000EA900 0050:
	_weapon_prevents_grenade_throwing (0000)
000EA950 01b0:
	_weapon_get_first_person_animation_time (0000)
000EAB00 0030:
	_weapon_overcharged (0000)
000EAB30 0050:
	_code_000eab30 (0000)
000EAB80 0050:
	_code_000eab80 (0000)
000EABD0 0050:
	_code_000eabd0 (0000)
000EAC20 0040:
	_code_000eac20 (0000)
000EAC60 0030:
	_code_000eac60 (0000)
000EAC90 0040:
	_code_000eac90 (0000)
000EACD0 0050:
	_code_000eacd0 (0000)
000EAD20 0090:
	_code_000ead20 (0000)
000EADB0 00f0:
	_code_000eadb0 (0000)
000EAEA0 0050:
	_code_000eaea0 (0000)
000EAEF0 00f0:
	_code_000eaef0 (0000)
000EAFE0 0070:
	_code_000eafe0 (0000)
000EB050 0040:
	_code_000eb050 (0000)
000EB090 0090:
	_code_000eb090 (0000)
000EB120 0080:
	_code_000eb120 (0000)
000EB1A0 0050:
	_code_000eb1a0 (0000)
000EB1F0 0020:
	_code_000eb1f0 (0000)
000EB210 0020:
	_code_000eb210 (0000)
000EB230 01b0:
	_code_000eb230 (0000)
000EB3E0 0120:
	_weapon_set_total_rounds (0000)
000EB500 0010:
	_power (0000)
000EB510 0010:
	_random (0000)
000EB520 0190:
	_weapon_new (0000)
000EB6B0 0060:
	_weapon_delete (0000)
000EB710 0390:
	_weapon_export_function_values (0000)
000EBAA0 0220:
	_weapon_handle_potential_inventory_item (0000)
000EBCC0 00a0:
	_weapon_owner_update (0000)
000EBD60 0140:
	_weapon_build_weapon_interface_state (0000)
000EBEA0 0080:
	_weapon_reloading (0000)
000EBF20 0070:
	_weapon_rotate_zoom_level (0000)
000EBF90 0160:
	_weapon_get_zoom_magnification (0000)
000EC0F0 0050:
	_weapon_get_field_of_view (0000)
000EC140 0060:
	_weapon_prevents_melee_attack (0000)
000EC1A0 0160:
	_code_000ec1a0 (0000)
000EC300 00e0:
	_code_000ec300 (0000)
000EC3E0 00c0:
	_code_000ec3e0 (0000)
000EC4A0 0080:
	_code_000ec4a0 (0000)
000EC520 00c0:
	_code_000ec520 (0000)
000EC5E0 0090:
	_code_000ec5e0 (0000)
000EC670 0060:
	_code_000ec670 (0000)
000EC6D0 0060:
	_code_000ec6d0 (0000)
000EC730 0190:
	_code_000ec730 (0000)
000EC8C0 00a0:
	_code_000ec8c0 (0000)
000EC960 0030:
	_code_000ec960 (0000)
000EC990 0160:
	_weapon_set_current_amount (0000)
000ECAF0 0080:
	_weapon_ready (0000)
000ECB70 00a0:
	_weapon_put_away (0000)
000ECC10 0110:
	_weapon_aim (0000)
000ECD20 0010:
	_weapon_stop_reload (0000)
000ECD30 0050:
	_code_000ecd30 (0000)
000ECD80 0720:
	_code_000ecd80 (0000)
000ED4A0 07c0:
	_code_000ed4a0 (0000)
000EDC60 0270:
	_code_000edc60 (0000)
000EDED0 00d0:
	_code_000eded0 (0000)
000EDFA0 0100:
	_code_000edfa0 (0000)
000EE0A0 0080:
	_code_000ee0a0 (0000)
000EE120 0af0:
	_weapon_update (0000)
00279248 000e:
	??_C@_0O@NGIMIMAN@weapon_update?$AA@ (0000)
00279258 0010:
	??_C@_0BA@HKDAKBBH@?$HOsecondary?9blur?$AA@ (0000)
00279268 000e:
	??_C@_0O@MICLHJCM@?$HOprimary?9blur?$AA@ (0000)
00279278 001f:
	??_C@_0BP@EKCOHDKN@c?3?2halo?2SOURCE?2items?2weapons?4c?$AA@ (0000)
00279298 004b:
	??_C@_0EL@OJBCMDA@trigger_index?$DO?$DN0?5?$CG?$CG?5trigger_inde@ (0000)
002792E8 004e:
	??_C@_0EO@GOIBLAPE@magazine_index?$DO?$DN0?5?$CG?$CG?5magazine_in@ (0000)
00279338 0033:
	??_C@_0DD@MMDDCEO@new_state?$DO?$DN0?5?$CG?$CG?5new_state?$DMNUMBER@ (0000)
00279370 0048:
	??_C@_0EI@IEKIMHHO@trigger_index?$DO?$DN0?5?$CG?$CG?5trigger_inde@ (0000)
002793B8 000d:
	??_C@_0N@PABHPCND@rounds_array?$AA@ (0000)
002793C8 001e:
	??_C@_0BO@MNJMFFGN@?$CBweapon_is_flag?$CIweapon_index?$CJ?$AA@ (0000)
002793E8 001f:
	??_C@_0BP@BIBKIDBE@weapon?9?$DOweapon?4primary_trigger?$AA@ (0000)
00279408 0013:
	??_C@_0BD@CJIOBLLL@magnification?$DO0?40f?$AA@ (0000)
0027941C 000e:
	??_C@_0O@NODOJABJ@magnification?$AA@ (0000)
0027942C 0004:
	__real@40470d23 (0000)
00279430 0004:
	__real@3d00adfd (0000)
00279434 0012:
	??_C@_0BC@IMIGIGHG@secondary?5trigger?$AA@ (0000)
00279448 0004:
	__real@3d2aaaab (0000)
00307140 0600:
	_data_00307140 (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
