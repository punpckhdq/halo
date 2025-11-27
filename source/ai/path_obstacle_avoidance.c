/*
PATH_OBSTACLE_AVOIDANCE.C

symbols in this file:
0004E6E0 0050:
	_obstacles_get_disc (0000)
0004E730 0030:
	_obstacle_from_disc (0000)
0004E760 0050:
	_path_get_step (0000)
0004E7B0 0060:
	_path_get_step_index (0000)
0004E810 0040:
	_heap_parent_index (0000)
0004E850 0010:
	_heap_left_index (0000)
0004E860 0010:
	_heap_right_index (0000)
0004E870 0060:
	_heap_cost (0000)
0004E8D0 00d0:
	_code_0004e8d0 (0000)
0004E9A0 01b0:
	_code_0004e9a0 (0000)
0004EB50 0190:
	_code_0004eb50 (0000)
0004ECE0 02a0:
	_code_0004ece0 (0000)
0004EF80 0060:
	_code_0004ef80 (0000)
0004EFE0 0070:
	_code_0004efe0 (0000)
0004F050 0260:
	_render_debug_path (0000)
0004F2B0 0040:
	_valid_real_point2d (0000)
0004F2F0 0220:
	_code_0004f2f0 (0000)
0004F510 01e0:
	_code_0004f510 (0000)
0004F6F0 0200:
	_code_0004f6f0 (0000)
0004F8F0 0330:
	_code_0004f8f0 (0000)
0004FC20 0130:
	_code_0004fc20 (0000)
0004FD50 0070:
	_code_0004fd50 (0000)
0004FDC0 0550:
	_path_avoid_obstacles (0000)
00050310 00e0:
	_render_debug_obstacle_path (0000)
0024E4E0 005f:
	??_C@_0FP@EDEGFDAA@disc_index?$DO?$DN0?5?$CG?$CG?5disc_index?$DMobst@ (0000)
0024E540 0019:
	??_C@_0BJ@LMDHFJFI@c?3?2halo?2source?2ai?2path?4h?$AA@ (0000)
0024E560 0063:
	??_C@_0GD@BIALPJBH@step_index?$DO?$DN0?5?$CG?$CG?5step_index?$DMpath@ (0000)
0024E5C4 002c:
	??_C@_0CM@IMJGFEPB@c?3?2halo?2SOURCE?2ai?2path_obstacle_@ (0000)
0024E5F0 0063:
	??_C@_0GD@EOCGPJDJ@heap_index?$DO?$DN0?5?$CG?$CG?5heap_index?$DMpath@ (0000)
0024E654 000d:
	??_C@_0N@FHNAKGNC@heap_index?$DO0?$AA@ (0000)
0024E664 0010:
	??_C@_0BA@OJJDPBG@?$CF3d?4?5?$CF?412g?5?$CI?$CFx?$CJ?$AA@ (0000)
0024E674 0025:
	??_C@_0CF@NHJKJPIK@please?5give?5this?5debug?4txt?5to?5ch@ (0000)
0024E69C 0018:
	??_C@_0BI@GONMKDOC@heap_verify?5failed?5?$CI?$CFs?$CJ?$AA@ (0000)
0024E6B4 0013:
	??_C@_0BD@OPIGCKBC@heap_insert?5output?$AA@ (0000)
0024E6C8 0012:
	??_C@_0BC@FFIJKMBF@heap_insert?5input?$AA@ (0000)
0024E6DC 0013:
	??_C@_0BD@JEBDACBO@heap_remove?5output?$AA@ (0000)
0024E6F0 0012:
	??_C@_0BC@HBIKAONP@heap_remove?5input?$AA@ (0000)
0024E704 0005:
	??_C@_04PBNHMBFG@goal?$AA@ (0000)
0024E70C 0026:
	??_C@_0CG@LBHAPEOA@?$CFs?3?5assert_valid_real_point2d?$CI?$CFf@ (0000)
0024E734 0006:
	??_C@_05FAGFPHJG@start?$AA@ (0000)
0024E73C 0021:
	??_C@_0CB@CJEIPHLL@?$CFs?3?5assert_valid_real?$CI0x?$CF08X?5?$CFf?$CJ@ (0000)
0024E760 001d:
	??_C@_0BN@KOMMMEHH@stack_top?$DMMAXIMUM_DISC_COUNT?$AA@ (0000)
0024E780 0046:
	??_C@_0EG@PAHGCCFL@result?4disc_index?$DO?$DN0?5?$CG?$CG?5result?4d@ (0000)
0024E7C8 0042:
	??_C@_0EC@PIPIECEP@seed_disc_index?$DO?$DN0?5?$CG?$CG?5seed_disc_@ (0000)
0024E810 0052:
	??_C@_0FC@HKCAOOKO@path?9?$DOobstacles?9?$DOdisc_count?$DO?$DN0?5?$CG@ (0000)
0024E864 0017:
	??_C@_0BH@FKFIAHK@obstacle?5path?5failure?$CB?$AA@ (0000)
0024E880 0050:
	??_C@_0FA@MIKGHDND@?$CIobstacle_step_index?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIo@ (0000)
0024E8D0 0030:
	??_C@_0DA@BINHGJIJ@input_step_count?5?$DM?$DN?5MAXIMUM_SMOO@ (0000)
00319D08 2142:
	_bss_00319d08 (0000)
	_debug_obstacle_path_on_failure (2140)
	_debug_obstacle_path (2141)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
