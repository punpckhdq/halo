/*
ACTOR_COMBAT.C

symbols in this file:
0000F590 0090:
	_code_0000f590 (0000)
0000F620 0030:
	_actor_combat_fire_wildly (0000)
0000F650 0040:
	_actor_combat_disable_bursts (0000)
0000F690 0030:
	_actor_firing_blindly (0000)
0000F6C0 0040:
	_actor_combat_currently_firing_burst (0000)
0000F700 0040:
	_actor_get_weapon_definition (0000)
0000F740 00c0:
	_code_0000f740 (0000)
0000F800 0080:
	_actor_combat_get_firing_variant_definition (0000)
0000F880 00a0:
	_code_0000f880 (0000)
0000F920 0040:
	_code_0000f920 (0000)
0000F960 0020:
	_fast_ftol (0000)
0000F980 0010:
	_sine (0000)
0000F990 0010:
	_cosine (0000)
0000F9A0 0010:
	_tangent (0000)
0000F9B0 0020:
	_cross_product2d (0000)
0000F9D0 0030:
	_add_vectors3d (0000)
0000FA00 0020:
	_random_boolean (0000)
0000FA20 0020:
	_valid_real (0000)
0000FA40 0160:
	_code_0000fa40 (0000)
0000FBA0 00b0:
	_code_0000fba0 (0000)
0000FC50 00d0:
	_code_0000fc50 (0000)
0000FD20 01c0:
	_code_0000fd20 (0000)
0000FEE0 0110:
	_code_0000fee0 (0000)
0000FFF0 0100:
	_actor_combat_find_grenade_target (0000)
000100F0 0370:
	_actor_combat_check_collateral_damage (0000)
00010460 0120:
	_actor_combat_plan_grenade_trajectory (0000)
00010580 0040:
	_valid_realcmp (0000)
000105C0 0060:
	_valid_real_normal3d (0000)
00010620 00b0:
	_code_00010620 (0000)
000106D0 02d0:
	_actor_aim_projectile (0000)
000109A0 07b0:
	_code_000109a0 (0000)
00011150 0060:
	_code_00011150 (0000)
000111B0 0220:
	_actor_aim_grenade (0000)
000113D0 1240:
	_actor_combat_update (0000)
002444A0 000e:
	??_C@_0O@HEGILGAC@weapon_vector?$AA@ (0000)
002444B0 0021:
	??_C@_0CB@NKOAMEKE@c?3?2halo?2SOURCE?2ai?2actor_combat?4c@ (0000)
002444D4 0035:
	??_C@_0DF@COLBENAC@burst_geometry_reference?5?$CG?$CG?5firi@ (0000)
0024450C 0004:
	__real@3dcccccd (0000)
00244510 0004:
	__real@3fd9999a (0000)
00244514 0016:
	??_C@_0BG@ODFAEBCF@projectile_definition?$AA@ (0000)
00244530 0042:
	??_C@_0EC@CDIDOCJH@grenade_origin?5?$CG?$CG?5desired_impact@ (0000)
00244574 0004:
	__real@3e4ccccd (0000)
00244578 0008:
	__real@3f50624de0000000 (0000)
00244580 0043:
	??_C@_0ED@MKICAFGB@actor?9?$DOcontrol?4current_fire_targ@ (0000)
002445C4 002b:
	??_C@_0CL@GJIEKICC@?$CFs?3?5assert_valid_real_normal3d?$CI?$CF@ (0000)
002445F0 0007:
	??_C@_06NBFPPGEG@vector?$AA@ (0000)
002445F8 0004:
	__real@3f490fdb (0000)
002445FC 003b:
	??_C@_0DL@NNPMKEAI@?$CFs?3?5proj?5?$CF?41f?5rof?5?$CF?41f?5dmg?1s?5?$CF?41@ (0000)
00244638 0020:
	??_C@_0CA@FHOKFHD@?$CFs?3?5manual?5damage?5modifier?5?$CF?42f?$AA@ (0000)
00244658 0010:
	??_C@_0BA@FHPPJCFM@?$CGnew_aim_vector?$AA@ (0000)
00244668 0021:
	??_C@_0CB@CPANGHMG@?$CGactor?9?$DOcontrol?4burst_aim_vector@ (0000)
0024468C 000f:
	??_C@_0P@DBEDAMI@aiming_success?$AA@ (0000)
0024469C 0012:
	??_C@_0BC@CHLPMLFC@?$CBhold_burst_start?$AA@ (0000)
002446B0 0025:
	??_C@_0CF@EHKIDLPJ@new_state?$CB?$DNactor?9?$DOcontrol?4fire_s@ (0000)
002446D8 0018:
	??_C@_0BI@MMOBPCCM@firing_decision?5?$DN?$DN?5NONE?$AA@ (0000)
002446F0 0004:
	__real@3f3504f3 (0000)
002446F4 0004:
	__real@3e3377ed (0000)
002446F8 004b:
	??_C@_0EL@FHBLJBEG@actor?9?$DOcontrol?4current_fire_targ@ (0000)
00244744 0015:
	??_C@_0BF@GLLAMKHP@weapon_index?5?$CB?$DN?5NONE?$AA@ (0000)
0024475C 0036:
	??_C@_0DG@EHGBJHNC@actor?9?$DOcontrol?4current_fire_targ@ (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
