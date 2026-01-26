/*
SCENARIO.C

symbols in this file:
0017DA90 0020:
	_code_0017da90 (0000)
0017DAB0 0020:
	_code_0017dab0 (0000)
0017DAD0 0020:
	_scenario_initialize (0000)
0017DAF0 0050:
	_scenario_initialize_for_new_map (0000)
0017DB40 0010:
	_scenario_dispose_from_old_map (0000)
0017DB50 0010:
	_scenario_frame_update (0000)
0017DB60 0070:
	_scenario_unload (0000)
0017DBD0 0030:
	_global_scenario_get (0000)
0017DC00 0010:
	_global_scenario_try_and_get (0000)
0017DC10 0030:
	_global_structure_bsp_get (0000)
0017DC40 0030:
	_global_collision_bsp_get (0000)
0017DC70 0030:
	_global_bsp3d_get (0000)
0017DCA0 0030:
	_scenario_get_game_globals (0000)
0017DCD0 0050:
	_global_structure_bsp_tag_index_get (0000)
0017DD20 0010:
	_scenario_location_award_bonus (0000)
0017DD30 0020:
	_default_material_definition_get (0000)
0017DD50 00c0:
	_scenario_material_definition_get (0000)
0017DE10 00d0:
	_scenario_location_deafening (0000)
0017DEE0 0010:
	_scenario_fog_at_point (0000)
0017DEF0 0080:
	_scenario_illumination_at_point (0000)
0017DF70 0050:
	_scenario_leaf_index_from_point (0000)
0017DFC0 0060:
	_scenario_get_sky_definition_index (0000)
0017E020 0030:
	_scenario_get_sky (0000)
0017E050 00a0:
	_scenario_test_pvs (0000)
0017E0F0 0070:
	_scenario_test_pas (0000)
0017E160 00a0:
	_scenario_location_potentially_visible_local (0000)
0017E200 00a0:
	_scenario_location_potentially_visible (0000)
0017E2A0 0060:
	_scenario_object_name_index_from_string (0000)
0017E300 0090:
	_scenario_fog_region_get_fog_index (0000)
0017E390 0190:
	_scenario_switch_structure_bsp (0000)
0017E520 0050:
	_scenario_reload_structure_bsp_if_necessary (0000)
0017E570 0070:
	_scenario_get_structure_reference_index_from_tag_index (0000)
0017E5E0 0170:
	_scenario_trigger_volume_test_point (0000)
0017E750 0030:
	_scenario_trigger_volume_test_object (0000)
0017E780 0150:
	_scenario_debug_to_file (0000)
0017E8D0 0010:
	_global_structure_bsp_index_get (0000)
0017E8E0 00f0:
	_scenario_load (0000)
0017E9D0 00b0:
	_scenario_location_from_point (0000)
0017EA80 0020:
	_scenario_location_from_line (0000)
0017EAA0 0080:
	_scenario_ensure_point_within_world (0000)
0017EB20 0110:
	_scenario_get_fog_region_index (0000)
0017EC30 0130:
	_scenario_location_underwater (0000)
0017ED60 00f0:
	_scenario_location_water_depth (0000)
0017EE50 0520:
	_scenario_get_sound_environment (0000)
0017F370 00a0:
	_code_0017f370 (0000)
0017F410 0330:
	_scenario_get_atmospheric_fog (0000)
002A053C 0011:
	??_C@_0BB@PPPFHENE@scenario?5globals?$AA@ (0000)
002A0550 0018:
	??_C@_0BI@KNDLJHDD@?$CBbink_playback_active?$CI?$CJ?$AA@ (0000)
002A0568 0023:
	??_C@_0CD@MHNEBKHK@c?3?2halo?2SOURCE?2scenario?2scenario@ (0000)
002A058C 0010:
	??_C@_0BA@HJMKGEMF@global_scenario?$AA@ (0000)
002A059C 0015:
	??_C@_0BF@JLHIBHPB@global_structure_bsp?$AA@ (0000)
002A05B4 0015:
	??_C@_0BF@KKAPCCOB@global_collision_bsp?$AA@ (0000)
002A05CC 000d:
	??_C@_0N@BGGBCKLE@global_bsp3d?$AA@ (0000)
002A05DC 0014:
	??_C@_0BE@GJAAGEGH@global_game_globals?$AA@ (0000)
002A05F0 0054:
	??_C@_0FE@CGGKIBFC@material_type?$DN?$DNNONE?5?$HM?$HM?5?$CImaterial@ (0000)
002A0648 0042:
	??_C@_0EC@HOLIKEMB@cluster_index1?$DO?$DN0?5?$CG?$CG?5cluster_ind@ (0000)
002A068C 0022:
	??_C@_0CC@NOBONILA@failed?5to?5load?5structure?5bsp?5?8?$CFs@ (0000)
002A06B0 0016:
	??_C@_0BG@HHCFBLGP@?$DMno?5scenario?5loaded?$DO?6?$AA@ (0000)
002A06C8 0007:
	??_C@_06FJDCLHHM@?5dead?6?$AA@ (0000)
002A06D0 002b:
	??_C@_0CL@BODEOCHI@?5at?5?$CI?$CF?42f?0?$CF?42f?0?$CF?42f?$CJ?5?$CIleaf?$CD?$CFd?0cl@ (0000)
002A06FC 000e:
	??_C@_0O@NJBGJAF@player?50x?$CF08x?$AA@ (0000)
002A070C 0015:
	??_C@_0BF@OHELHNCO@?$CC?$CFs?$CC?5bsp?5?$CC?$CFs?$CC?5?$CI?$CD?$CFd?$CJ?6?$AA@ (0000)
002A0724 0020:
	??_C@_0CA@IJIAGEDC@need?5to?5get?5the?5following?5tags?3?$AA@ (0000)
002A0744 0026:
	??_C@_0CG@BPOMKFJA@scenario?5doesn?8t?5have?5a?5structur@ (0000)
002A076C 0010:
	??_C@_0BA@EONNEPHC@globals?2globals?$AA@ (0000)
002A077C 000e:
	??_C@_0O@GBNBMLBO@scenario_load?$AA@ (0000)
002A078C 0004:
	__real@44160000 (0000)
002A0790 0004:
	__real@c4160000 (0000)
002A0794 0004:
	__real@3b449ba6 (0000)
002A0798 0004:
	__real@bb449ba6 (0000)
002A079C 0004:
	__real@3db851ec (0000)
002A07A0 0004:
	__real@bdb851ec (0000)
002A07A4 0004:
	__real@bcf5c28f (0000)
002A07A8 000b:
	??_C@_0L@LIOBMPBJ@?$HMn?$HMn?$HMn?$HMn?$CFs?$AA@ (0000)
002A07B4 0015:
	??_C@_0BF@LNAHLDLC@no?5sound?5environment?$AA@ (0000)
0030E790 006c:
	_global_scenario_index (0000)
	_global_structure_bsp_index (0004)
004C0520 0375:
	_bss_004c0520 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "scenario.h"

#include "physics/bsp3d.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

long scenario_leaf_index_from_point(
	const union real_point3d *point)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 213, global_bsp3d);
	
	return bsp3d_test_point(global_bsp3d, 0, point);
}

/* ---------- private code */
