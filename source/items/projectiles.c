/*
PROJECTILES.C

symbols in this file:
000E7460 0010:
	_projectiles_initialize (0000)
000E7470 0010:
	_projectiles_initialize_for_new_map (0000)
000E7480 0010:
	_projectiles_dispose_from_old_map (0000)
000E7490 0010:
	_projectiles_dispose (0000)
000E74A0 0030:
	_projectile_kill_tracer (0000)
000E74D0 0040:
	_projectiles_delete_all (0000)
000E7510 0010:
	_projectile_delete (0000)
000E7520 0020:
	_projectile_set_target_object_index (0000)
000E7540 0030:
	_projectile_make_tracer (0000)
000E7570 0020:
	_projectile_get_ballistic_acceleration (0000)
000E7590 0030:
	_projectile_estimate_time_to_target (0000)
000E75C0 0040:
	_dangerous_projectiles_near_player (0000)
000E7600 0030:
	_projectile_handle_deleted_object (0000)
000E7630 0020:
	_code_000e7630 (0000)
000E7650 0060:
	_code_000e7650 (0000)
000E76B0 00e0:
	_projectile_export_function_values (0000)
000E7790 0060:
	_code_000e7790 (0000)
000E77F0 0070:
	_projectile_handle_parent_destroyed (0000)
000E7860 0030:
	_random_vector_in_cone3d (0000)
000E7890 0370:
	_projectile_aim_ballistic (0000)
000E7C00 00c0:
	_projectile_aim_linear (0000)
000E7CC0 00c0:
	_projectile_aim (0000)
000E7D80 00b0:
	_code_000e7d80 (0000)
000E7E30 00e0:
	_code_000e7e30 (0000)
000E7F10 0200:
	_code_000e7f10 (0000)
000E8110 0410:
	_code_000e8110 (0000)
000E8520 01b0:
	_projectile_new (0000)
000E86D0 01f0:
	_projectile_accelerate (0000)
000E88C0 0b70:
	_code_000e88c0 (0000)
000E9430 0fe0:
	_projectile_update (0000)
00279028 000b:
	??_C@_0L@IKCANML@reflection?$AA@ (0000)
00279034 0012:
	??_C@_0BC@MIGMEKLO@negative?5incident?$AA@ (0000)
00279048 0009:
	??_C@_08NAKCOKKJ@incident?$AA@ (0000)
00279054 0012:
	??_C@_0BC@IEANEAFD@projectile_update?$AA@ (0000)
00279068 0004:
	_rdata_00279068 (0000)
0027906C 0023:
	??_C@_0CD@LFGANAAE@c?3?2halo?2SOURCE?2items?2projectiles@ (0000)
00279090 002f:
	??_C@_0CP@JLELNEII@projectile?9?$DOobject?4parent_object@ (0000)
002790C0 0012:
	??_C@_0BC@KMKCONEA@result_aim_vector?$AA@ (0000)
002790D4 0014:
	??_C@_0BE@NNDLGLKI@v_desired_sq?5?$DO?50?40f?$AA@ (0000)
002790E8 0016:
	??_C@_0BG@HPLEINHD@t_squared_max?5?$DO?$DN?50?40f?$AA@ (0000)
00279100 0014:
	??_C@_0BE@BBPDBBBA@4?40f?5?$CK?5a?5?$CK?5c?5?$DO?50?40f?$AA@ (0000)
00279114 002b:
	??_C@_0CL@KDJFFEJL@?$CGprojectile?9?$DOobject?4translationa@ (0000)
00279140 000d:
	??_C@_0N@OKFAMOHD@acceleration?$AA@ (0000)
00279150 0004:
	__real@3c088889 (0000)
00279154 0004:
	__real@3f22f983 (0000)
00279158 001e:
	??_C@_0BO@CBMNHFJH@collision?9?$DOobject_index?$CB?$DNNONE?$AA@ (0000)
00279178 001c:
	??_C@_0BM@JNPELLAN@?$CGprojectile?9?$DOobject?4forward?$AA@ (0000)
00279194 0017:
	??_C@_0BH@CAHIPIOP@?$CGprojectile?9?$DOobject?4up?$AA@ (0000)
002791AC 000e:
	??_C@_0O@DJJKDDEB@?$CGnew_position?$AA@ (0000)
002791BC 002a:
	??_C@_0CK@HFBLHMOF@projectile?5velocity?5is?5bad?5after@ (0000)
002791E8 002f:
	??_C@_0CP@HBPNPJKN@projectile?5velocity?5is?5bad?5after@ (0000)
00279218 0004:
	__real@3f7d70a4 (0000)
0027921C 002b:
	??_C@_0CL@BHNFKMBE@projectile?5velocity?5is?5bad?5after@ (0000)
00306B30 060c:
	_data_00306b30 (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
