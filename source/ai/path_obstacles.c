/*
PATH_OBSTACLES.C

symbols in this file:
000503F0 0020:
	_obstacles_new (0000)
00050410 0020:
	_set_real_point2d (0000)
00050430 0030:
	_rotate_vector2d (0000)
00050460 0090:
	_project_point3d (0000)
000504F0 0040:
	_point_in_circle (0000)
00050530 0050:
	_point_in_sphere (0000)
00050580 0090:
	_code_00050580 (0000)
00050610 0080:
	_code_00050610 (0000)
00050690 00d0:
	_obstacles_add_disc (0000)
00050760 0320:
	_obstacles_get_discs_in_sphere (0000)
00050A80 00a0:
	_obstacles_test_circle (0000)
00050B20 00f0:
	_obstacles_test_pill (0000)
00050C10 00e0:
	_obstacles_disc_tangents (0000)
00050CF0 0230:
	_obstacles_disc_neighborhood (0000)
00050F20 00b0:
	_obstacles_recompute (0000)
00050FD0 01c0:
	_render_debug_obstacles (0000)
0024E900 000b:
	??_C@_0L@LKENDBEE@?$HO?$CIsign?$CG?$HO1?$CJ?$AA@ (0000)
0024E90C 0021:
	??_C@_0CB@FEGCJDLB@projection?$DO?$DN_x?5?$CG?$CG?5projection?$DM?$DN_z@ (0000)
0024E930 0014:
	??_C@_0BE@DKJODJEH@?4?4?2math?2real_math?4h?$AA@ (0000)
0024E948 0051:
	??_C@_0FB@IMJPHNIA@obstacles?9?$DOobstacle_count?$DO?$DN0?5?$CG?$CG?5@ (0000)
0024E9A0 0046:
	??_C@_0EG@PILJPJIJ@obstacles?9?$DOdisc_count?$DO?$DN0?5?$CG?$CG?5obst@ (0000)
0024E9E8 0023:
	??_C@_0CD@MNFANEEP@c?3?2halo?2SOURCE?2ai?2path_obstacles@ (0000)
0024EA0C 000d:
	??_C@_0N@IDCAPIB@matt?5is?5bad?4?$AA@ (0000)
0024EA1C 0004:
	__real@3b800000 (0000)
0024EA20 003c:
	??_C@_0DM@HAPAFMPD@seed_disc_index?$DO?$DN0?5?$CG?$CG?5seed_disc_@ (0000)
0024EA5C 0004:
	__real@c0000000 (0000)
0024EA60 004a:
	??_C@_0EK@GINAEHH@disc?9?$DOobstacle_index?$DO?$DN0?5?$CG?$CG?5disc?9@ (0000)
002B7578 0800:
	_data_002b7578 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "path.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
