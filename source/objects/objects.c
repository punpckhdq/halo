/*
OBJECTS.C

symbols in this file:
0012CDC0 0020:
	_cluster_get_first_noncollideable_object (0000)
0012CDE0 0020:
	_cluster_get_next_noncollideable_object (0000)
0012CE00 0020:
	_cluster_get_first_collideable_object (0000)
0012CE20 0020:
	_cluster_get_next_collideable_object (0000)
0012CE40 0050:
	_object_get_next_cluster (0000)
0012CE90 0040:
	_object_try_and_get_and_verify_type (0000)
0012CED0 0070:
	_object_get_and_verify_type (0000)
0012CF40 0040:
	_object_iterator_new (0000)
0012CF80 00c0:
	_object_iterator_next (0000)
0012D040 0080:
	_object_get_ultimate_parent (0000)
0012D0C0 0010:
	_object_add_scenario_permutation (0000)
0012D0D0 0030:
	_object_set_object_index_for_name_index (0000)
0012D100 0070:
	_objects_fix_for_deleted_object (0000)
0012D170 0230:
	_object_set_garbage (0000)
0012D3A0 0010:
	_garbage_collect_now (0000)
0012D3B0 0080:
	_objects_information_get (0000)
0012D430 0030:
	_object_pvs_set_object (0000)
0012D460 00a0:
	_object_pvs_set_camera_point (0000)
0012D500 0010:
	_object_pvs_clear (0000)
0012D510 00e0:
	_objects_get_activating_cluster_index (0000)
0012D5F0 0030:
	_object_definition_predict (0000)
0012D620 00b0:
	_object_predict (0000)
0012D6D0 0050:
	_object_beautify (0000)
0012D720 00a0:
	_code_0012d720 (0000)
0012D7C0 0050:
	_code_0012d7c0 (0000)
0012D810 0090:
	_object_header_block_get (0000)
0012D8A0 0150:
	_object_header_block_allocate (0000)
0012D9F0 0050:
	_code_0012d9f0 (0000)
0012DA40 0200:
	_code_0012da40 (0000)
0012DC40 0070:
	_code_0012dc40 (0000)
0012DCB0 0090:
	_code_0012dcb0 (0000)
0012DD40 0020:
	_code_0012dd40 (0000)
0012DD60 00c0:
	_code_0012dd60 (0000)
0012DE20 01e0:
	_code_0012de20 (0000)
0012E000 0380:
	_code_0012e000 (0000)
0012E380 0040:
	_object_scripting_set_collideable (0000)
0012E3C0 0050:
	_object_marker_begin (0000)
0012E410 0040:
	_object_marker_end (0000)
0012E450 0050:
	_object_unmarked_function (0000)
0012E4A0 0060:
	_object_mark_function (0000)
0012E500 01b0:
	_code_0012e500 (0000)
0012E6B0 00a0:
	_code_0012e6b0 (0000)
0012E750 0040:
	_code_0012e750 (0000)
0012E790 0010:
	_code_0012e790 (0000)
0012E7A0 0020:
	_code_0012e7a0 (0000)
0012E7C0 0080:
	_code_0012e7c0 (0000)
0012E840 0050:
	_code_0012e840 (0000)
0012E890 0020:
	_code_0012e890 (0000)
0012E8B0 0020:
	_objects_place (0000)
0012E8D0 00a0:
	_code_0012e8d0 (0000)
0012E970 0260:
	_find_objects_from_point_vector (0000)
0012EBD0 0030:
	_code_0012ebd0 (0000)
0012EC00 0090:
	_code_0012ec00 (0000)
0012EC90 0070:
	_code_0012ec90 (0000)
0012ED00 0360:
	_objects_dump_memory (0000)
0012F060 0140:
	_objects_initialize (0000)
0012F1A0 00a0:
	_objects_initialize_for_new_map (0000)
0012F240 00d0:
	_objects_dispose_from_old_map (0000)
0012F310 0070:
	_objects_dispose (0000)
0012F380 0050:
	_object_activate (0000)
0012F3D0 0040:
	_object_deactivate (0000)
0012F410 0060:
	_object_reset (0000)
0012F470 00e0:
	_object_placement_data_new (0000)
0012F550 0110:
	_object_disconnect_from_map (0000)
0012F660 0060:
	_object_get_first_cluster (0000)
0012F6C0 0030:
	_object_get_node_matrices (0000)
0012F6F0 0050:
	_object_get_attachment_marker_name (0000)
0012F740 0060:
	_object_has_node (0000)
0012F7A0 0070:
	_object_set_automatic_deactivation (0000)
0012F810 00b0:
	_object_set_visibility (0000)
0012F8C0 00c0:
	_object_get_velocities (0000)
0012F980 0030:
	_object_get_location (0000)
0012F9B0 00d0:
	_object_start_interpolation (0000)
0012FA80 0090:
	_object_offset_interpolation (0000)
0012FB10 00e0:
	_object_permute_region (0000)
0012FBF0 0080:
	_object_get_function_value (0000)
0012FC70 0300:
	_objects_in_clusters_by_indices (0000)
0012FF70 0030:
	_object_index_from_name_index (0000)
0012FFA0 0090:
	_objects_disconnect_from_structure_bsp (0000)
00130030 01f0:
	_object_visible_to_any_player (0000)
00130220 0030:
	_object_pvs_activate (0000)
00130250 00d0:
	_code_00130250 (0000)
00130320 0080:
	_code_00130320 (0000)
001303A0 0030:
	_code_001303a0 (0000)
001303D0 0040:
	_objects_scripting_set_scale (0000)
00130410 0100:
	_code_00130410 (0000)
00130510 0020:
	_object_delete (0000)
00130530 01d0:
	_object_reconnect_to_map (0000)
00130700 0060:
	_object_get_node_matrix (0000)
00130760 0110:
	_object_get_marker_by_name (0000)
00130870 01a0:
	_object_align_marker_to_matrix (0000)
00130A10 0130:
	_object_detach (0000)
00130B40 0070:
	_object_get_origin (0000)
00130BB0 0120:
	_object_get_orientation (0000)
00130CD0 0060:
	_object_get_world_matrix (0000)
00130D30 0110:
	_object_inverse_kinematics (0000)
00130E40 01d0:
	_objects_in_sphere (0000)
00131010 0140:
	_objects_reconnect_to_structure_bsp (0000)
00131150 0070:
	_objects_paparazzi (0000)
001311C0 0200:
	_object_export_function_values (0000)
001313C0 19a0:
	_object_compute_node_matrices (0000)
00132D60 0040:
	_objects_scripting_detach (0000)
00132DA0 04b0:
	_object_render_debug (0000)
00133250 00e0:
	_code_00133250 (0000)
00133330 0100:
	_object_set_position (0000)
00133430 00a0:
	_object_translate (0000)
001334D0 05c0:
	_object_new (0000)
00133A90 01b0:
	_object_attach_to_node (0000)
00133C40 0100:
	_object_force_inside_bsp (0000)
00133D40 01b0:
	_code_00133d40 (0000)
00133EF0 0090:
	_object_compute_node_matrices_recursive (0000)
00133F80 0040:
	_code_00133f80 (0000)
00133FC0 00f0:
	_object_new_from_scenario (0000)
001340B0 00e0:
	_object_attach_to_marker (0000)
00134190 0070:
	_object_new_by_name (0000)
00134200 0130:
	_code_00134200 (0000)
00134330 0050:
	_objects_scripting_attach (0000)
00134380 0020:
	_object_delete_immediately (0000)
001343A0 0620:
	_objects_garbage_collection (0000)
001349C0 0320:
	_objects_update (0000)
00134CE0 0020:
	_objects_memory_compact (0000)
00289D98 0018:
	_rdata_00289d98 (0000)
00289DB0 000f:
	??_C@_0P@LFOALBOL@objects_update?$AA@ (0000)
00289DC0 008c:
	??_C@_0IM@GHLCLNJO@iterator?9?$DOcluster_partition?$DN?$DN?$CGco@ (0000)
00289E4C 0021:
	??_C@_0CB@HJEGKKGJ@c?3?2halo?2SOURCE?2objects?2objects?4c@ (0000)
00289E70 004a:
	??_C@_0EK@DDHBGNFK@got?5an?5object?5type?5we?5didn?8t?5exp@ (0000)
00289EBC 0038:
	??_C@_0DI@NJFAJKCK@uninitialized?5iterator?5passed?5to@ (0000)
00289EF4 003d:
	??_C@_0DN@LIGLDCO@TEST_FLAG?$CIgarbage_object?9?$DOobject@ (0000)
00289F34 0004:
	__real@35800000 (0000)
00289F38 0040:
	??_C@_0EA@MOANJMN@object_pvs_set_camera_point?3?5cam@ (0000)
00289F78 008b:
	??_C@_0IL@PJOKFAFE@parent_object?9?$DOobject?4location?4c@ (0000)
0028A004 003c:
	??_C@_0DM@FAGPPIPE@reference?9?$DOoffset?$CLreference?9?$DOsiz@ (0000)
0028A040 0014:
	??_C@_0BE@CMKNCLBJ@reference?9?$DOoffset?$DO0?$AA@ (0000)
0028A058 005e:
	??_C@_0FO@DFFLELNG@block_reference_offset?$CLsizeof?$CIst@ (0000)
0028A0B8 001a:
	??_C@_0BK@GCHGANBE@block_reference_offset?$DO?$DN0?$AA@ (0000)
0028A0D4 0029:
	??_C@_0CJ@KJGPJOHH@object_header?9?$DOdata_size?$CLsize?$DM?$DNS@ (0000)
0028A100 0004:
	__real@3f364cd7 (0000)
0028A104 0004:
	__real@43d79081 (0000)
0028A108 0004:
	__real@4412c849 (0000)
0028A10C 0004:
	__real@439df252 (0000)
0028A110 0004:
	__real@443a07f2 (0000)
0028A114 001e:
	??_C@_0BO@OPMGCPJ@?$CKfirst_object_reference?$CB?$DNNONE?$AA@ (0000)
0028A134 002b:
	??_C@_0CL@FNFNCGNP@?$CBobject_globals?9?$DOobject_marker_i@ (0000)
0028A160 002a:
	??_C@_0CK@IJOLKBMD@object_globals?9?$DOobject_marker_in@ (0000)
0028A18C 0011:
	??_C@_0BB@IDGMLLCF@object_name_list?$AA@ (0000)
0028A1A0 0011:
	??_C@_0BB@BEMCKCAE@object?5name?5list?$AA@ (0000)
0028A1B4 002d:
	??_C@_0CN@BNDNJDAG@an?5object?5with?5the?5name?5?8?$CFs?8?5alr@ (0000)
0028A1E4 003e:
	??_C@_0DO@EEADIGAP@name_index?$DO?$DN0?5?$CG?$CG?5name_index?$DMMAXI@ (0000)
0028A224 0031:
	??_C@_0DB@KFCPDNIH@?$CF?56d?5?$CI?$CF?56d?$CJ?5?$FL?$CF?57d?1?$CF?57d?1?$CF?57d?1?$CF?57d@ (0000)
0028A258 0063:
	??_C@_0GD@CPALGMGH@WARNING?3?5overflowed?5MAXIMUM_DUMP@ (0000)
0028A2BC 0017:
	??_C@_0BH@PBBFFOC@OBJECTS?5BY?5DEFINITION?6?$AA@ (0000)
0028A2D8 0043:
	??_C@_0ED@BEMDNONK@number?5?$CIactive?$CJ?5?$FLgarbage?1?5?5?5dead@ (0000)
0028A31C 0011:
	??_C@_0BB@GNMAIFEG@OBJECTS?5BY?5TYPE?6?$AA@ (0000)
0028A330 003d:
	??_C@_0DN@LIBJGINJ@?$CD?$CFd?5objects?5?$CI?$CD?$CFd?5active?$CJ?5using?5?$CF@ (0000)
0028A370 0015:
	??_C@_0BF@MGBBKKEM@d?3?2object_memory?4txt?$AA@ (0000)
0028A388 003f:
	??_C@_0DP@CAHEHEJH@?$CIheader?9?$DOtype?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIheader?9?$DO@ (0000)
0028A3C8 0016:
	??_C@_0BG@KNAIILHO@noncollideable?5object?$AA@ (0000)
0028A3E0 0013:
	??_C@_0BD@EPIKGCAI@collideable?5object?$AA@ (0000)
0028A3F4 000f:
	??_C@_0P@GCGFDHI@object_globals?$AA@ (0000)
0028A404 000f:
	??_C@_0P@CJDMFPHM@object?5globals?$AA@ (0000)
0028A414 0029:
	??_C@_0CJ@MPENCKAI@object_header_data?5?$CG?$CG?5object_mem@ (0000)
0028A440 0008:
	??_C@_07JPLKJBI@objects?$AA@ (0000)
0028A448 003e:
	??_C@_0DO@BAPNCPID@TEST_FLAG?$CIobject?9?$DOobject?4flags?0?5@ (0000)
0028A488 0028:
	??_C@_0CI@CPMOGOIB@DATUM_INDEX_TO_IDENTIFIER?$CIobject@ (0000)
0028A4B0 0041:
	??_C@_0EB@MPFJDMPL@?$CBTEST_FLAG?$CI_object_mask_cannot_i@ (0000)
0028A4F8 0048:
	??_C@_0EI@JLAEJIFD@function_index?$DO?$DN0?5?$CG?$CG?5function_in@ (0000)
0028A540 0020:
	??_C@_0CA@BCPJMOMG@?$CB?$CIweapon_is_flag?$CIobject_index?$CJ?$CJ?$AA@ (0000)
0028A560 003f:
	??_C@_0DP@KDEDEENE@?$CBTEST_FLAG?$CIobject?9?$DOobject?4flags?0@ (0000)
0028A5A0 002a:
	??_C@_0CK@OPKDPNPM@object_has_node?$CIobject_index?0?5no@ (0000)
0028A5CC 0017:
	??_C@_0BH@NBICGHKL@maximum_marker_count?$DO0?$AA@ (0000)
0028A5E4 0029:
	??_C@_0CJ@HEAMOBGB@valid_real_matrix4x3?$CIdestination@ (0000)
0028A610 0013:
	??_C@_0BD@EIOHMDGK@destination_matrix?$AA@ (0000)
0028A624 000d:
	??_C@_0N@BFMOEIFC@child_marker?$AA@ (0000)
0028A634 000f:
	??_C@_0P@DMPMGCLF@object_indices?$AA@ (0000)
0028A644 0009:
	??_C@_08CBBFJFMD@location?$AA@ (0000)
0028A650 0004:
	__real@3e22f983 (0000)
0028A658 0008:
	__real@3fefd70a40000000 (0000)
0028A660 002e:
	??_C@_0CO@LMCMMCPP@object_compute_node_matrices?5roo@ (0000)
0028A690 0019:
	??_C@_0BJ@DILLIKA@?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5scale?5?$CFf?$AA@ (0000)
0028A6AC 001e:
	??_C@_0BO@GOPFPFFJ@?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5posn?5?$CFf?5?$CFf?5?$CFf?$AA@ (0000)
0028A6CC 001e:
	??_C@_0BO@LGPKFCMO@?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5up?5?5?5?$CFf?5?$CFf?5?$CFf?$AA@ (0000)
0028A6EC 001e:
	??_C@_0BO@MAAPDIJN@?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5left?5?$CFf?5?$CFf?5?$CFf?$AA@ (0000)
0028A70C 001e:
	??_C@_0BO@NCBDHGGN@computed?5matrix?5fwd?5?5?$CFf?5?$CFf?5?$CFf?$AA@ (0000)
0028A72C 0011:
	??_C@_0BB@FAFHKCNL@?5?5no?5parent?5node?$AA@ (0000)
0028A740 0034:
	??_C@_0DE@DOJKDEHM@?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5scale?5?$CIjaso@ (0000)
0028A774 0023:
	??_C@_0CD@OGEIOHJN@?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5posn?5?$CFf?5?$CFf?5@ (0000)
0028A798 0023:
	??_C@_0CD@DOEHEAAK@?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5up?5?5?5?$CFf?5?$CFf?5@ (0000)
0028A7BC 0023:
	??_C@_0CD@EILCCKFJ@?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5?5left?5?$CFf?5?$CFf?5@ (0000)
0028A7E0 0023:
	??_C@_0CD@LAKOFIJF@?5?5parent?9node?5matrix?5fwd?5?5?$CFf?5?$CFf?5@ (0000)
0028A804 0019:
	??_C@_0BJ@GEKCHFFM@?5?5origin?9offset?5?$CFf?5?$CFf?5?$CFf?$AA@ (0000)
0028A820 0026:
	??_C@_0CG@BFFOPNJD@?5?5center?9of?9mass?5translation?5?$CFf?5@ (0000)
0028A848 0032:
	??_C@_0DC@GBFDLHME@?5?5object?3?5pos?5?$CFf?5?$CFf?5?$CFf?0?5fwd?5?$CFf?5?$CF@ (0000)
0028A87C 0038:
	??_C@_0DI@HJPDAFKC@object_compute_node_matrices?5FAI@ (0000)
0028A8B4 0018:
	??_C@_0BI@DIJPGKII@?$CFs?5as?5parent?5node?5of?5?$CFs?$AA@ (0000)
0028A8D0 004f:
	??_C@_0EP@NMLLJPBP@?$CFs?5had?5a?5bad?5forward?5and?5up?5befo@ (0000)
0028A920 003e:
	??_C@_0DO@KBCIODOG@?$CFs?5had?5a?5bad?5position?5before?5com@ (0000)
0028A960 000f:
	??_C@_0P@HHCPIEKG@frame_index?$DO?$DN0?$AA@ (0000)
0028A970 000d:
	??_C@_0N@NCPDDPGK@new_position?$AA@ (0000)
0028A980 0021:
	??_C@_0CB@KICDPCDH@OUT?5OF?5OBJECTS?3?5cannot?5create?5?$CFs@ (0000)
0028A9A4 001e:
	??_C@_0BO@GAJHNFGD@?$CGdata?9?$DOtranslational_velocity?$AA@ (0000)
0028A9C4 0018:
	??_C@_0BI@EDEMGAJJ@?$CGdata?9?$DOangular_velocity?$AA@ (0000)
0028A9DC 000f:
	??_C@_0P@JNFNPCBA@?$CGdata?9?$DOforward?$AA@ (0000)
0028A9EC 000a:
	??_C@_09MMNHALDO@?$CGdata?9?$DOup?$AA@ (0000)
0028A9F8 002f:
	??_C@_0CP@KPEIPFOB@cannot?5attach?5an?5object?5to?5one?5o@ (0000)
0028AA28 0038:
	??_C@_0DI@OBHOMKDM@object_has_node?$CIparent_object_in@ (0000)
0028AA60 0020:
	??_C@_0CA@POCDJJBP@garbage?5collection?5warning?5?$CI?$CFs?$CJ?$AA@ (0000)
0028AA80 0015:
	??_C@_0BF@NBHLMPKH@removing?5objects?3?5?$CFs?$AA@ (0000)
0028AA98 0023:
	??_C@_0CD@DCKJKCF@garbage?5collection?5?$CFscritical?5?$CI?$CF@ (0000)
0028AABC 0007:
	??_C@_06BODLOOIB@still?5?$AA@ (0000)
0028AAC4 000e:
	??_C@_0O@ONNNEJPN@?$CFd?5slots?5free?$AA@ (0000)
0028AAD4 0014:
	??_C@_0BE@LODKOAIM@?$CF4?42f?$CF?$CF?5memory?5free?$AA@ (0000)
0028AAE8 0031:
	??_C@_0DB@GPANNPOD@compacted?5to?5?$CD?$CFd?5with?50x?$CFx?5conti@ (0000)
0028AB1C 002f:
	??_C@_0CP@FJGNMMGE@WARNING?3?5garbage?5collecting?5a?5li@ (0000)
0028AB4C 002d:
	??_C@_0CN@JMJCMKCA@garbage_object_count?$DMMAXIMUM_OBJ@ (0000)
0028AB7C 0034:
	??_C@_0DE@KFENFAAJ@?$CD?$CFd?5objects?5using?50x?$CFx?5bytes?5?$CI0x@ (0000)
0028ABB0 0039:
	??_C@_0DJ@NOLFDCHG@object_get?$CIobject_index?$CJ?9?$DOobject@ (0000)
0028ABEC 003b:
	??_C@_0DL@CLCAHNEH@object_get?$CIobject_index?$CJ?9?$DOobject@ (0000)
0028AC28 0023:
	??_C@_0CD@JHKFHGNF@object_header?9?$DOcluster_index?$CB?$DNNO@ (0000)
0030C3C0 05fa:
	_data_0030c3c0 (0000)
	_debug_objects_bounding_spheres (05f8)
	_debug_objects_collision_models (05f9)
00456E9C 000d:
	_bss_00456e9c (0000)
	_debug_objects_position_velocity (000c)
*/

/* ---------- headers */

#include "cseries.h"
#include "objects.h"

#include "damage.h"
#include "object_lights.h"
#include "object_types.h"

#include "ai/ai_debug.h"
#include "cache/predicted_resources.h"
#include "cseries/errors.h"
#include "devices/device_definitions.h"
#include "devices/devices.h"
#include "editor/editor_stubs.h"
#include "effects/contrails.h"
#include "effects/effects.h"
#include "effects/particle_systems.h"
#include "game/game.h"
#include "game/game_engine.h"
#include "game/players.h"
#include "items/weapons.h"
#include "main/console.h"
#include "memory/memory_pool.h"
#include "models/model_animation_definitions.h"
#include "models/model_definitions.h"
#include "models/models.h"
#include "objects/widgets/widgets.h"
#include "physics/collision_model_definitions.h"
#include "physics/collision_models.h"
#include "physics/physics.h"
#include "render/render_debug.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "sound/game_sound.h"
#include "structures/structure_bsp_definitions.h"

/* ---------- constants */


/* ---------- macros */

#define MAXIMUM_DUMPS 1024

/* ---------- structures */

struct object_globals
{
	boolean initial_placement;
	boolean object_marker_initialized;
	boolean force_garbage_collection;
	short active_garbage_object_count;
	long first_garbage_object_index;
	unsigned long last_active_cluster_bits[16];
	unsigned long active_cluster_bits[16];
	long last_garbage_warn_time;
	short pvs_activation_type;
	union
	{
		long object_index;
		short cluster_index;
	} pvs_activation;
};


struct object_memory_release_function
{
	void(__cdecl *init_function)(unsigned __int8 *, __int16);
	boolean(__cdecl *function)(char *, unsigned __int8 *, unsigned __int8 *, __int16);
};

struct dump_datum
{
	long definition_index;
	short object_type;
	short maximum_size;
	long total_size;
	short count;
	short active_count;
	short garbage_count;
	short dead_count;
	short outside_map_count;
	short at_rest_count;
};

/* ---------- prototypes */

static void object_connect_lights(long object_index, boolean disconnect, boolean reconnect);
static void object_name_list_allocate(void);
static void object_name_list_free(void);
static void object_name_list_clear(void);
static void object_name_list_new(long object_index, short name_index);
static void object_name_list_delete(long object_index);
static long object_name_list_lookup(short name_index);
static long recursive_object_adder(
	long object_index,
	boolean(*add_object_function)(long, void *),
	void *custom_data,
	long object_count,
	long maximum_count,
	long *object_indices);
static long sort_dumps(struct dump_datum const *dump1, struct dump_datum const *dump2);
static void object_add_to_dump(long object_index, struct dump_datum *dump);
static void object_dump_write(FILE *stream, struct dump_datum *dump);
static short object_find_region_permutations_available_with_variant(struct model_region *region, short variant_number, short *available_indices);
static boolean object_select_random_region_permutations_by_variant(long object_index, struct model *model, short variant_number);
static short object_determine_variant_number(long object_index, struct model *model);
static void object_choose_random_region_permutations(long object_index);
static void object_add_to_list(long *first_object_reference, long object_index);
static void object_remove_from_list(long *first_object_reference, long object_index);
static void attachments_delete(long object_index);
static void object_create_attachments(long object_index);
static void object_delete_attachments(long object_index);
static void object_delete_recursive(long object_index, boolean delete_siblings);

/* ---------- globals */

static long *object_name_list;
static struct memory_pool *object_memory_pool;
static struct object_globals *object_globals;
boolean debug_objects_position_velocity;

/* ---------- public code */

long cluster_get_first_noncollideable_object(
	long *reference_index,
	short cluster_index)
{
	return cluster_partition_get_first_datum(
		&noncollideable_object_cluster_partition,
		reference_index,
		cluster_index
	);
}

long cluster_get_next_noncollideable_object(
	long *reference_index)
{
	return cluster_partition_get_next_datum(
		&noncollideable_object_cluster_partition,
		reference_index
	);
}

long cluster_get_first_collideable_object(
	long *reference_index,
	short cluster_index)
{
	return cluster_partition_get_first_datum(
		&collideable_object_cluster_partition,
		reference_index,
		cluster_index
	);
}

long cluster_get_next_collideable_object(
	long *reference_index)
{
	return cluster_partition_get_next_datum(
		&collideable_object_cluster_partition,
		reference_index
	);
}

short object_get_next_cluster(
	struct object_cluster_iterator *iterator)
{
	match_assert(
		"c:\\halo\\SOURCE\\objects\\objects.c",
		1049,
		iterator->cluster_partition==&collideable_object_cluster_partition || iterator->cluster_partition==&noncollideable_object_cluster_partition
	);

	return cluster_partition_get_next_cluster(iterator->cluster_partition, &iterator->reference_index);
}

void *object_try_and_get_and_verify_type(
	long object_index,
	unsigned long valid_type_flags)
{
	struct object_header_datum* header= object_header_try_and_get(object_index);
	void *result= NULL;

	if (header)
	{
		if (TEST_FLAG(valid_type_flags, header->type))
		{
			result= header->datum;
		}
	}

	return result;
}

void *object_get_and_verify_type(
	long object_index,
	unsigned long valid_type_flags)
{
	struct object_datum *result= object_header_get(object_index)->datum;

	match_vassert(
		"c:\\halo\\SOURCE\\objects\\objects.c",
		1690,
		TEST_FLAG(valid_type_flags, result->object.type),
		csprintf(
			temporary,
			"got an object type we didn't expect (expected one of 0x%08x but got #%d).",
			valid_type_flags,
			result->object.type
		)
	);

	return result;
}

void object_iterator_new(
	struct object_iterator *iterator,
	unsigned long type_flags,
	byte flags)
{
	data_verify(object_header_data);
	iterator->signature= OBJECT_ITERATOR_SIGNATURE;
	iterator->type_flags= type_flags;
	iterator->flags= flags;
	iterator->absolute_index= 0;
	iterator->index= NONE;

	return;
}

void *object_iterator_next(
	struct object_iterator *iterator)
{
	short abs_index;
	struct object_header_datum *header;
	void *result= NULL;

	match_vassert(
		"c:\\halo\\SOURCE\\objects\\objects.c",
		1720,
		iterator->signature==OBJECT_ITERATOR_SIGNATURE,
		"uninitialized iterator passed to object_iterator_next()"
	);

	data_verify(object_header_data);
	
	abs_index= iterator->absolute_index;

	// The operation to get this header is inlined in the original code?
	header= (struct object_header_datum *)(
		(char*)object_header_data->data + sizeof(struct object_header_datum) * abs_index
	);

	while (abs_index<object_header_data->count)
	{
		long index= DATUM_INDEX_NEW(abs_index++, header->identifier);
		if (header->identifier)
		{
			if ((iterator->flags&header->flags)==iterator->flags &&
				TEST_FLAG(iterator->type_flags, header->type))
			{
				iterator->index= index;
				result= header->datum;
				break;
			}
		}
		++header;
	}
	iterator->absolute_index= abs_index;

	return result;
}

long object_get_ultimate_parent(
	long object_index)
{
	long result= NONE;
	struct object_datum *object;
	for (;object_index!=NONE; object_index= object->object.parent_object_index)
	{
		result= object_index;
		object= object_get(object_index);
	}

	return result;
}

void object_add_scenario_permutation(
	long object_index,
	struct scenario_object_permutation *permutation)
{
	return;
}

void object_set_object_index_for_name_index(
	short name_index,
	long object_index)
{
	if (VALID_INDEX(name_index, global_scenario_get()->object_names.count))
	{
		object_name_list[name_index]= object_index;
	}

	return;
}

void objects_fix_for_deleted_object(
	long deleted_object_index)
{
	struct object_datum *object;

	struct object_iterator iterator;
	object_iterator_new(&iterator, _object_mask_all, 0);

	for (
		object= (struct object_datum *)object_iterator_next(&iterator);
		object;
		object= (struct object_datum *)object_iterator_next(&iterator)
	)
	{
		if (object->object.umbrella_shield_object_index==deleted_object_index)
		{
			object->object.umbrella_shield_object_index= NONE;
		}
		object_type_handle_deleted_object(iterator.index, deleted_object_index);
	}

	return;
}

void object_set_garbage(
	long object_index,
	boolean garbage)
{
	struct object_datum *garbage_object;
	struct object_datum* object= object_get(object_index);
	
	long garbage_object_index;
	for (
		garbage_object_index= object_globals->first_garbage_object_index;
		garbage_object_index!=NONE;
		garbage_object_index= garbage_object->object.next_garbage_object_index
	)
	{
		garbage_object= object_get(garbage_object_index);
		match_assert(
			"c:\\halo\\SOURCE\\objects\\objects.c",
			1952,
			TEST_FLAG(garbage_object->object.flags, _object_garbage_bit)
		);
	}

	if (garbage)
	{
		if (!TEST_FLAG(object->object.flags, _object_garbage_bit))
		{
			if (!TEST_FLAG(object->object.flags, _object_cannot_be_garbage_bit))
			{
				object->object.next_garbage_object_index= object_globals->first_garbage_object_index;
				object_globals->first_garbage_object_index= object_index;
				SET_FLAG(object->object.flags, _object_garbage_bit, TRUE);
			}
		}
	}
	else
	{
		if (TEST_FLAG(object->object.flags, _object_garbage_bit))
		{
			long *garbage_object_index;
			for (
				garbage_object_index= &object_globals->first_garbage_object_index;
				*garbage_object_index!=object_index;
				garbage_object_index= &object_get(*garbage_object_index)->object.next_garbage_object_index
			)
			{
			}

			*garbage_object_index= object->object.next_garbage_object_index;
			object->object.next_garbage_object_index= NONE;
			SET_FLAG(object->object.flags, _object_garbage_bit, FALSE);
		}
	}

	for (
		garbage_object_index= object_globals->first_garbage_object_index;
		garbage_object_index!=NONE;
		garbage_object_index= garbage_object->object.next_garbage_object_index
	)
	{
		garbage_object= object_get(garbage_object_index);
		match_assert(
			"c:\\halo\\SOURCE\\objects\\objects.c",
			2006,
			TEST_FLAG(garbage_object->object.flags, _object_garbage_bit)
		);
	}

	return;
}

void garbage_collect_now(
	void)
{
	object_globals->force_garbage_collection= TRUE;

	return;
}

void objects_information_get(
	struct objects_information *information)
{
	short i;
	struct object_header_datum *header;

	memset(information, 0, sizeof(*information));
	
	header= (struct object_header_datum *)object_header_data->data;
	for (i= 0; i<object_header_data->count; header++)
	{
		if (header->identifier)
		{
			information->object_count++;
			if (TEST_FLAG(header->flags, _object_header_active_bit))
			{
				information->active_object_count++;
			}
		}

		i++;
	}
	
	information->used_memory= 1.f - (real)memory_pool_get_contiguous_free_size(object_memory_pool) / 1048576.f;
	
	return;
}

void object_pvs_set_object(
	long object_index)
{
	if (object_index == NONE)
	{
		object_globals->pvs_activation_type= _pvs_activation_normal;
	}
	else
	{
		object_globals->pvs_activation_type= _pvs_activation_object;
		object_globals->pvs_activation.object_index= object_index;
	}

	return;
}

void object_pvs_set_camera_point(
	short camera_point_index)
{
	if (camera_point_index == NONE)
	{
		object_globals->pvs_activation_type= _pvs_activation_normal;
	}
	else
	{
		struct location location;
		struct scenario_cutscene_camera_point *point= (struct scenario_cutscene_camera_point *)TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->cutscene_camera_points,
			camera_point_index,
			struct scenario_cutscene_camera_point
		);

		scenario_location_from_point(&location, &point->position);
		if (location.cluster_index==NONE)
		{
			error(
				_error_silent,
				  "object_pvs_set_camera_point: camera point %s is outside the map",
				  point->name
			);
			object_globals->pvs_activation_type= FALSE;
		}
		else
		{
			object_globals->pvs_activation_type= _pvs_activation_cluster;
			object_globals->pvs_activation.cluster_index= location.cluster_index;
		}
	}

	return;
}

void object_pvs_clear(void)
{
	object_globals->pvs_activation_type= _pvs_activation_normal;

	return;
}

short objects_get_activating_cluster_index(
	void)
{
	struct object_header_datum *header;
	short result= NONE;

	switch (object_globals->pvs_activation_type)
	{
	case _pvs_activation_cluster:
		result= object_globals->pvs_activation.cluster_index;
		break;
	case _pvs_activation_object:
	{
		long index= object_globals->pvs_activation.object_index;

		header= object_header_try_and_get(index);
		if (header && FLAG(header->type) && header->datum)
		{
			struct object_datum *parent_object= object_get(object_get_ultimate_parent(object_globals->pvs_activation.object_index));

			if (TEST_FLAG(parent_object->object.flags, _object_connected_to_map_bit))
			{
				if (parent_object->object.location.cluster_index!=NONE)
				{
					match_assert(
						"c:\\halo\\SOURCE\\objects\\objects.c",
						2279,
						parent_object->object.location.cluster_index>=0 &&
						parent_object->object.location.cluster_index<global_structure_bsp_get()->clusters.count
					);
					result= parent_object->object.location.cluster_index;
				}
			}
		}
		else
		{
			object_globals->pvs_activation_type= _pvs_activation_normal;
		}
		break;
	}
	default:
		break;
	}

	return result;
}

void object_definition_predict(
	long definition_index)
{
	if (definition_index!=NONE)
	{
		struct object_definition *object_definition= object_definition_get(definition_index);
		predicted_resources_precache(&object_definition->object.predicted_resources);
	}

	return;
}

void object_predict(
	long object_index)
{
	while (object_index!=NONE)
	{
		struct object_datum *object= object_get(object_index);

		object_definition_predict(object->definition_index);
		object_predict(object->object.first_child_object_index);
		
		object_index= object->object.next_object_index;
	}

	return;
}

void object_beautify(
	long object_index,
	boolean beautiful)
{
	if (object_index!=NONE)
	{
		SET_FLAG(object_get(object_index)->object.flags, _object_movie_star_bit, beautiful);
	}

	return;
}

void *object_header_block_get(
	long object_index,
	struct object_header_block_reference *reference)
{
	struct object_header_datum *object_header= object_header_get(object_index);
	struct object_datum *object= object_get(object_index);

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2443, reference->offset>0);
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2444, reference->offset+reference->size<=object_header->data_size);

	return (char *)object + reference->offset;
}

static long object_header_new(
	struct data_array *data, 
	long index,
	short size)
{
	index= (index==NONE ? datum_new(data) : datum_new_at_index(data, index));
	
	if (index!=NONE)
	{
		struct object_header_datum *header= (struct object_header_datum *)datum_get(data, index);
		if (memory_pool_block_allocate(object_memory_pool, (void**)&header->datum, size))
		{
			header->data_size= size;
			memset(header->datum, 0, size);
		}
		else
		{
			datum_delete(data, index);
			index= NONE;
		}
	}

	return index;
}

static void object_header_delete(
	struct data_array *data,
	long object_index)
{
	struct object_header_datum *object_header= object_header_get(object_index);

	if (object_header->datum)
	{
		memory_pool_block_free(object_memory_pool, (void**)&object_header->datum);
	}

	datum_delete(data, object_index);
	object_header->flags= 0;
	object_header->datum= NULL;

	return;
}

boolean object_header_block_allocate(
	long object_index,
	short block_reference_offset,
	short size)
{
	boolean result;
	struct object_header_datum *object_header= object_header_get(object_index);

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2459, size>=0);
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2460, object_header->data_size+size<=SHORT_MAX);

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2462, block_reference_offset>=0);
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2463, block_reference_offset+sizeof(struct object_header_block_reference)<=object_header->data_size);

	if (memory_pool_block_reallocate(
		object_memory_pool,
		(void **)&object_header->datum,
		size + object_header->data_size))
	{
		struct object_header_block_reference *block;
		short original_size= object_header->data_size;

		object_header->data_size+= size;
		block= (struct object_header_block_reference *)((byte *)object_get(object_index) + block_reference_offset);
		
		block->offset= original_size;
		block->size= size;

		memset(((byte *)object_header->datum) + block->offset, 0, size);
		result= TRUE;
	}
	else
	{
		result= FALSE;
	}

	return result;
}

void object_scripting_set_collideable(
	long object_index,
	boolean collideable)
{
	if (object_index!=NONE)
	{
		struct object_datum *object= object_get(object_index);
		SET_FLAG(object->object.flags, _object_no_collisions_bit, !collideable);
	}

	return;
}

void object_marker_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 3503, !object_globals->object_marker_initialized);

	++global_object_marker;
	object_globals->object_marker_initialized= TRUE;

	return;
}

void object_marker_end(
	void)
{
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 3514, object_globals->object_marker_initialized);

	object_globals->object_marker_initialized= FALSE;

	return;
}

boolean object_unmarked_function(
	long object_index)
{
	struct object_datum *object= object_get(object_index);
	boolean result;

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 3526, object_globals->object_marker_initialized);

	result= object->object.magic_number!=global_object_marker;
	
	return result;
}

boolean object_mark_function(
	long object_index)
{
	boolean result;
	struct object_datum *object= object_get(object_index);

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 3543, object_globals->object_marker_initialized);

	if (object->object.magic_number!=global_object_marker)
	{
		object->object.magic_number= global_object_marker;
		result= TRUE;
	}
	else
	{
		result= FALSE;
	}

	return result;
}

// TODO: match
long find_objects_from_point_vector(
	real_point3d const *position,
	real_vector3d const *direction, 
	boolean (*add_object_function)(long, void *),
	void *custom_data,
	long maximum_object_count,
	long *object_indices)
{
	long result= 0;

	if (scenario_leaf_index_from_point(position)!=NONE)
	{
		short cluster_index= TAG_BLOCK_GET_ELEMENT(
			&global_structure_bsp_get()->leaves,
			scenario_leaf_index_from_point(position) & LONG_MAX,
			struct structure_leaf
		)->cluster_index;

		if (cluster_index!=NONE)
		{
			unsigned long* cluster_pvs;
			long i;
			long bit_vector_size;

			object_marker_begin();

			cluster_pvs= structure_bsp_get_cluster_pvs(global_structure_bsp_get(), cluster_index);

			bit_vector_size= BIT_VECTOR_SIZE_IN_LONGS(global_structure_bsp_get()->clusters.count);

			for (i=0; i<bit_vector_size; ++i)
			{
				if (cluster_pvs[i])
				{
					long j;
					long size= MIN((LONG_BITS * i) + LONG_BITS, global_structure_bsp_get()->clusters.count);

					for (j= LONG_BITS*i; j<size; ++j)
					{
						if (BIT_VECTOR_TEST_FLAG(cluster_pvs, j))
						{
							long reference;
							long k;
							for (
								k= cluster_get_first_collideable_object(&reference, j);
								k!=NONE;
								k= cluster_get_next_collideable_object(&reference))
							{
								if (object_mark_function(k))
								{
									result= recursive_object_adder(
										k,
										add_object_function,
										custom_data,
										result,
										maximum_object_count,
										object_indices);
								}
							}
						}
					}
				}
			}

			object_marker_end();
		}
	}

	return result;
}

void objects_dump_memory(
	void)
{
	struct dump_datum dumps[MAXIMUM_DUMPS];
	struct dump_datum dumps_by_type[NUMBER_OF_OBJECT_TYPES];
	struct object_iterator iterator;
	long type;
	struct object_datum *object;
	FILE *file;

	long object_count= 0;
	long overflowed_object_count= 0;

	memset(dumps, 0, sizeof(dumps));
	memset(dumps_by_type, 0, sizeof(dumps_by_type));
	
	for (type= 0; type < NUMBER_OF_OBJECT_TYPES; type++)
	{
		dumps_by_type[type].object_type= type;
		dumps_by_type[type].definition_index= NONE;
	}

	object_iterator_new(&iterator, _object_mask_all, 0);
	
	while (object= (struct object_datum *)object_iterator_next(&iterator))
	{
		long object_num;
		struct object_header_datum * header;
		short index= NONE;
		
		for (object_num= 0; object_num<object_count; object_num++)
		{
			if (dumps[object_num].definition_index==object->definition_index)
			{
				index= object_num;
				break;
			}
		}

		if (index==NONE)
		{
			if (object_count<NUMBEROF(dumps))
			{
				index= object_count++;
				dumps[index].object_type= NONE;
				dumps[index].definition_index= object->definition_index;
			}
			else
			{
				++overflowed_object_count;
			}
		}

		header= object_header_get(iterator.index);

		if (index!=NONE)
		{
			object_add_to_dump(iterator.index, &dumps[index]);
		}

		match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 4905, VALID_INDEX(header->type, NUMBER_OF_OBJECT_TYPES));
		object_add_to_dump(iterator.index, &dumps_by_type[header->type]);
	}

	qsort(
		dumps,
		object_count,
		sizeof(struct dump_datum),
		(int(__cdecl *)(const void *, const void *))sort_dumps
	);
	qsort(
		dumps_by_type,
		NUMBEROF(dumps_by_type),
		sizeof(struct dump_datum),
		(int(__cdecl *)(const void *, const void *))sort_dumps
	);

	file= fopen("d:\\object_memory.txt", "a+b");
	if (file)
	{
		long object_num;
		struct objects_information information;
		
		objects_information_get(&information);
		fprintf(
			file,
			"#%d objects (#%d active) using %3.2f%% of available memory\n\n",
			information.object_count,
			information.active_object_count,
			information.used_memory * 100.f
		);
		
		fprintf(file, "OBJECTS BY TYPE\n");
		fprintf(file, "number (active) [garbage/   dead/outside/at-rest] maxsize totsize\n");
		for (type= 0; type<NUMBER_OF_OBJECT_TYPES; type++)
		{
			object_dump_write(file, &dumps_by_type[type]);
		}

		fprintf(file, "OBJECTS BY DEFINITION\n");
		fprintf(file, "number (active) [garbage/   dead/outside/at-rest] maxsize totsize\n");
		for (object_num= 0; object_num<object_count; object_num++)
		{
			object_dump_write(file, &dumps[object_num]);
		}

		fprintf(file, "\n");

		if (overflowed_object_count>0)
		{
			fprintf(
				file,
				"WARNING: overflowed MAXIMUM_DUMPS (%d), this dump does not include %d objects that would not fit!\n",
				MAXIMUM_DUMPS,
				overflowed_object_count
			);
		}

		fprintf(file, "\n");
		fclose(file);
	}


	return;
}

void objects_initialize(
	void)
{
	damage_initialize();
	widgets_initialize();
	object_types_initialize();
	lights_initialize();

	if (!game_in_editor())
	{
		object_header_data= game_state_data_new("object", MAXIMUM_OBJECTS_PER_MAP, sizeof(struct object_header_datum));
		object_memory_pool= game_state_memory_pool_new("objects", OBJECT_MEMORY_POOL_SIZE);
	}
	else
	{
		object_header_data= data_new("object", MAXIMUM_OBJECTS_PER_MAP*5, sizeof(struct object_header_datum));
		object_memory_pool= memory_pool_new("objects", OBJECT_MEMORY_POOL_SIZE*5);
	}
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 216, object_header_data && object_memory_pool);

	object_globals= (struct object_globals *)game_state_malloc("object globals", NULL, sizeof(*object_globals));
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 219, object_globals);

	object_name_list_allocate();
	cluster_partition_new(&collideable_object_cluster_partition, "collideable object");
	cluster_partition_new(&noncollideable_object_cluster_partition, "noncollideable object");
	return;
}

void objects_initialize_for_new_map(
	void)
{
	damage_initialize_for_new_map();
	widgets_initialize_for_new_map();
	object_types_initialize_for_new_map();
	lights_initialize_for_new_map();
	
	data_make_valid(object_header_data);
	memset(object_name_list, NONE, MAXIMUM_OBJECT_NAMES_PER_SCENARIO*sizeof(*object_name_list));

	cluster_partition_make_valid(&collideable_object_cluster_partition);
	cluster_partition_make_valid(&noncollideable_object_cluster_partition);
	memset(object_globals->last_active_cluster_bits, 0, 0x40u);
	memset(object_globals->active_cluster_bits, 0, 0x40u);
	object_globals->pvs_activation_type= 0;
	object_globals->object_marker_initialized= FALSE;
	global_object_marker= 0;
	object_globals->first_garbage_object_index= NONE;
	object_globals->active_garbage_object_count= 0;
	object_globals->last_garbage_warn_time= 0;

	return;
}

void objects_dispose_from_old_map(
	void)
{
	damage_dispose_from_old_map();
	widgets_dispose_from_old_map();
	object_types_dispose_from_old_map();
	lights_dispose_from_old_map();

	if (object_header_data->valid)
	{
		long i;
		for (
			i= data_next_index(object_header_data, NONE);
			i!=NONE;
			i= data_next_index(object_header_data, i))
		{
			struct data_array *data= object_header_data;
			struct object_header_datum *header= object_header_get(i);
			
			if (header->datum)
			{
				memory_pool_block_free(object_memory_pool, (void**)&header->datum);
			}

			datum_delete(data, i);
			header->datum= NULL;
			header->flags= 0;
		}
		data_make_invalid(object_header_data);
	}

	cluster_partition_make_invalid(&collideable_object_cluster_partition);
	cluster_partition_make_invalid(&noncollideable_object_cluster_partition);

	return;
}

void objects_dispose(
	void)
{
	widgets_dispose();
	object_types_dispose();
	lights_dispose();

	if (!game_in_editor())
	{
		if (object_header_data)
		{
			object_header_data= 0;
		}
	}
	else
	{
		data_dispose(object_header_data);
	}

	if (object_memory_pool)
	{
		object_memory_pool= NULL;
	}

	cluster_partition_delete(&collideable_object_cluster_partition);
	cluster_partition_delete(&noncollideable_object_cluster_partition);

	return;
}

void object_activate(
	long object_index)
{
	struct object_header_datum *header= object_header_get(object_index);
	struct object_datum *object= object_get(object_index);

	if (!TEST_FLAG(header->flags, _object_header_active_bit) &&
		!TEST_FLAG(object->object.flags, _object_cannot_be_activated_bit) &&
		object->object.parent_object_index==NONE)
	{
		SET_FLAG(header->flags, _object_header_active_bit, TRUE);
	}

	return;
}

void object_deactivate(
	long object_index)
{
	struct object_header_datum *header= object_header_get(object_index);
	struct object_datum *object= object_get(object_index);

	if (TEST_FLAG(header->flags, _object_header_active_bit))
	{
		SET_FLAG(header->flags, _object_header_active_bit, FALSE);
	}

	return;
}

void object_reset(
	long object_index)
{
	struct object_datum *object= object_get(object_index);
	object->object.translational_velocity= *global_zero_vector3d;
	object->object.angular_velocity= *global_zero_vector3d;
	SET_FLAG(object->object.flags, _object_at_rest_bit, FALSE);
	object_type_reset(object_index);

	return;
}

void object_placement_data_new(
	struct object_placement_data *data,
	long definition_index,
	long owner_object_index)
{
	long i;
	struct object_header_datum *header;

	memset(data, 0, sizeof(*data));

	data->definition_index= definition_index;
	data->flags= 0;
	data->forward= *global_forward3d;
	data->up= *global_up3d;
	data->variant_number= 0;

	header= object_header_try_and_get(owner_object_index);
	if (header && FLAG(header->type) && header->datum)
	{
		struct object_datum *object= header->datum;

		data->owner_object_index= owner_object_index;
		data->owner_player_index= object->object.owner_player_index;
		data->owner_team_index= object->object.owner_team_index;
	}
	else
	{
		data->owner_object_index= NONE;
		data->owner_player_index= NONE;
		data->owner_team_index= NONE;
	}

	i= 0;
	do
	{
		data->change_colors[i]= *global_real_rgb_white;
		i++;
	}
	while (i<NUMBEROF(data->change_colors));

	return;
}

void object_disconnect_from_map(
	long object_index)
{
	struct object_header_datum *header= object_header_get(object_index);
	struct object_datum *object= header->datum;

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 957, DATUM_INDEX_TO_IDENTIFIER(object_index));
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 957, TEST_FLAG(object->object.flags, _object_connected_to_map_bit));

	if (object->object.parent_object_index==NONE)
	{
		object_remove_from_list(&object_get(object_index)->object.first_child_object_index, object_index);
	}
	else
	{
		cluster_partition_disconnect(
			(TEST_FLAG(object_get(object_index)->object.flags, _object_has_collision_model_bit) ?
				&collideable_object_cluster_partition :
				&noncollideable_object_cluster_partition
			),
			object_index,
			&object->object.first_cluster_reference_index
		);

		if (TEST_FLAG(header->flags, _object_header_automatically_deactivate_bit))
		{
			struct object_header_datum *deactivated_header= object_header_get(object_index);
			object_get(object_index);

			if (TEST_FLAG(deactivated_header->flags, _object_header_active_bit))
			{
				SET_FLAG(deactivated_header->flags, _object_header_active_bit, FALSE);
			}
		}
	}

	return;
}

short object_get_first_cluster(
	struct object_cluster_iterator *iterator,
	long object_index)
{
	long ultimate_parent_index= object_get_ultimate_parent(object_index);
	
	iterator->cluster_partition= TEST_FLAG(object_get(ultimate_parent_index)->object.flags, _object_has_collision_model_bit) ?
		&collideable_object_cluster_partition :
		&noncollideable_object_cluster_partition;

	return cluster_partition_get_first_cluster(
		iterator->cluster_partition,
		&iterator->reference_index,
		object_get(ultimate_parent_index)->object.first_cluster_reference_index
	);
}

real_matrix4x3 *object_get_node_matrices(
	long object_index)
{
	return (real_matrix4x3 *)object_header_block_get(object_index, &object_get(object_index)->object.node_matrices);
}

char const *object_get_attachment_marker_name(
	long object_index,
	short attachment_index)
{
	const char *result;

	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object->definition_index);

	if (VALID_INDEX(attachment_index, object_definition->object.attachments.count))
	{
		result= TAG_BLOCK_GET_ELEMENT(
			&object_definition->object.attachments,
			attachment_index,
			struct object_attachment_definition
		)->marker_name;
	}
	else
	{
		result= NULL;
	}

	return result;
}

boolean object_has_node(
	long object_index,
	short node_index)
{
	struct object_datum *object= object_get(object_index);
	long model_index= object_definition_get(object->definition_index)->object.model.index;
	boolean result= FALSE;

	if (model_index==NONE)
	{
		if (!node_index)
		{
			result= TRUE;
		}
	}
	else
	{
		struct model *model= model_definition_get(model_index);
		if (VALID_INDEX(node_index, model->nodes.count))
		{
			result= TRUE;
		}
	}

	return result;
}

void object_set_automatic_deactivation(
	long object_index,
	boolean automatic_deactivation)
{
	struct object_header_datum *header= object_header_get(object_index);
	struct object_datum *object= object_get(object_index);

	SET_FLAG(header->flags, _object_header_automatically_deactivate_bit, automatic_deactivation);
	
	if (automatic_deactivation)
	{
		if (object->object.parent_object_index==NONE && object->object.location.cluster_index==NONE)
		{
			object_deactivate(object_index);
		}
	}
	else
	{
		if (!TEST_FLAG(header->flags, _object_header_active_bit))
		{
			object_activate(object_index);
		}
	}

	return;
}

void object_set_visibility(
	long object_index,
	boolean visible_flag)
{
	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object->definition_index);

	if (object_definition->object.model.index!=NONE)
	{
		if (TEST_FLAG(object->object.flags, _object_invisible_bit) && visible_flag)
		{
			object_connect_lights(object_index, FALSE, TRUE);
		}
		else if (!TEST_FLAG(object->object.flags, _object_invisible_bit) && !visible_flag)
		{
			object_connect_lights(object_index, TRUE, FALSE);
		}
	}

	if (!visible_flag || object_definition->object.model.index!=NONE)
	{
		struct object_header_datum *header= object_header_get(object_index);

		SET_FLAG(object->object.flags, _object_invisible_bit, !visible_flag);
		SET_FLAG(header->flags, _object_header_visible_bit, visible_flag);
	}

	return;
}

void object_get_velocities(
	long object_index,
	real_vector3d *translational_velocity,
	real_vector3d *angular_velocity)
{
	struct object_datum *ultimate_parent= object_get(object_index);

	while (ultimate_parent->object.parent_object_index!=NONE)
	{
		ultimate_parent= object_get(ultimate_parent->object.parent_object_index);
	}

	if (translational_velocity)
	{
		*translational_velocity= ultimate_parent->object.translational_velocity;
	}

	if (angular_velocity)
	{
		*angular_velocity= ultimate_parent->object.angular_velocity;
	}

	return;
}

struct location *object_get_location(
	long object_index,
	struct location *location)
{
	*location= object_get(object_get_ultimate_parent(object_index))->object.location;

	return location;
}

void object_start_interpolation(
	long object_index,
	short frame_count)
{
	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object->definition_index);
	short node_count= model_definition_get(object_definition->object.model.index)->nodes.count;

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1523, !TEST_FLAG(_object_mask_cannot_interpolate, object->object.type));

	memcpy(
		object_header_block_get(object_index, &object->object.original_node_orientations),
		object_header_block_get(object_index, &object->object.node_orientations),
		sizeof(real_orientation)*node_count
	);
	if (frame_count>=
		object->object.animation.interpolation_frame_count-
		object->object.animation.interpolation_frame_index
	)
	{
		object->object.animation.interpolation_frame_index= 0;
		object->object.animation.interpolation_frame_count= frame_count;
	}

	return;
}

void object_offset_interpolation(
	long object_index,
	real_vector3d *offset)
{
	struct object_datum *object= object_get(object_index);

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1546, !TEST_FLAG(_object_mask_cannot_interpolate, object->object.type));

	if (object->object.animation.interpolation_frame_count)
	{
		real_orientation* orientation= (real_orientation *)object_header_block_get(
			object_index,
			&object->object.original_node_orientations
		);
		orientation->translation.x+= offset->i;
		orientation->translation.y+= offset->j;
		orientation->translation.z+= offset->k;
	}

	return;
}

// TODO: match
void object_permute_region(
	long object_index,
	char *permutation_name,
	short desired_region_index,
	boolean active_flag)
{
	struct object_datum *object= object_get(object_index);
	long model_index= object_definition_get(object_index)->object.model.index;
	if (model_index!=NONE)
	{
		long region_index;
		struct model* model= model_definition_get(model_index);
		
		for (region_index= 0; region_index<model->regions.count; region_index++)
		{
			if (desired_region_index==NONE || desired_region_index==region_index)
			{
				long permutation_index;
				struct model_region *region= TAG_BLOCK_GET_ELEMENT(
					&model->regions,
					region_index,
					struct model_region
				);

				for (permutation_index= 0; permutation_index<region->permutations.count; permutation_index++)
				{
					struct model_region_permutation *permutation= TAG_BLOCK_GET_ELEMENT(
						&region->permutations,
						permutation_index,
						struct model_region_permutation
					);
					
					if (!_stricmp(permutation->name, permutation_name))
					{
						object->object.region_permutations[region_index]= active_flag ? region_index : 0;
						break;
					}
				}
			}
		}
	}

	return;
}

boolean object_get_function_value(
	long object_index,
	short function_index,
	real *value_reference)
{
	struct object_datum *object= object_get(object_index);

	if (function_index==NONE)
	{
		*value_reference= 1.f;
		return TRUE;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1654, VALID_INDEX(function_index, NUMBER_OF_OUTGOING_OBJECT_FUNCTIONS));

		*value_reference= object->object.outgoing_function_values[function_index];
		return TEST_FLAG(object->object.functions_active_flags, function_index);
	}
}

short objects_in_clusters_by_indices(
	unsigned long class_flags,
	short cluster_count,
	short const *cluster_indices,
	short maximum_object_count,
	long *object_indices)
{
	return NONE;
}

long object_index_from_name_index(
	short name_index)
{
	return NONE;
}

void objects_disconnect_from_structure_bsp(
	void)
{
	struct object_datum *object;
	struct object_iterator iterator;

	object_iterator_new(&iterator, _object_mask_all, 0);
	while (object= (struct object_datum *)object_iterator_next(&iterator))
	{
		if (TEST_FLAG(object->object.flags, 11) && object->object.parent_object_index==NONE)
		{
			object_disconnect_from_map(iterator.index);

			SET_FLAG(object->object.flags, 11, TRUE);
		}

		object_type_disconnect_from_structure_bsp(iterator.index);
	}

	return;
}

boolean object_visible_to_any_player(
	long object_index)
{
	struct object_header_datum *header= object_header_get(object_index);
	struct object_datum *object= object_get(object_index);
	boolean result= FALSE;

	if (TEST_FLAG(header->flags, _object_header_active_bit))
	{
		if (TEST_FLAG(object->object.flags, _object_connected_to_map_bit) &&
			!TEST_FLAG(object->object.flags, _object_outside_of_map_bit))
		{
			const unsigned long* combined_pvs= players_get_combined_pvs();
		}
	}

	return result;
}

void object_pvs_activate(long object_index)
{
	object_pvs_set_object(object_index);

	return;
}

void objects_scripting_set_scale(long object_index, real scale, short interpolation_frame_count)
{
	if (object_index!=NONE)
	{
		struct object_datum *object= object_get(object_index);
		object->object.scale= scale;
		if (!TEST_FLAG(_object_mask_cannot_interpolate, object->object.type))
		{
			object_start_interpolation(object_index, interpolation_frame_count);
		}
	}

	return;
}

static void object_delete_initial_recursive(long object_index, boolean delete_siblings)
{
	struct object_header_datum *header= object_header_get(object_index);
	struct object_datum *object= object_get(object_index);

	if (game_engine_running() && object->object.type == _object_type_weapon)
	{
		match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 829, !(weapon_is_flag(object_index)));
	}
	
	if (object->object.first_child_object_index!=NONE)
	{
		object_delete_initial_recursive(object->object.first_child_object_index, TRUE);
	}

	if (delete_siblings)
	{
		if (object->object.next_object_index!=NONE)
		{
			object_delete_initial_recursive(object->object.next_object_index, TRUE);
		}
	}

	SET_FLAG(header->flags, _object_header_being_deleted_bit, TRUE);

	if (object_definition_get(object->definition_index)->object.model.index!=NONE &&
		!TEST_FLAG(object->object.flags, _object_invisible_bit))
	{
		object_connect_lights(object_index, TRUE, FALSE);
	}

	header= object_header_get(object_index);
	SET_FLAG(header->flags, _object_header_active_bit, TRUE);
	SET_FLAG(header->flags, _object_header_visible_bit, FALSE);
	object_name_list_delete(object_index);

	return;
}

void object_delete(long object_index)
{
	object_delete_initial_recursive(object_index, FALSE);

	return;
}

void object_reconnect_to_map(long object_index, struct location const *location)
{
	struct object_header_datum *header= object_header_get(object_index);
	struct object_datum *object= header->datum;

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 879, DATUM_INDEX_TO_IDENTIFIER(object_index));
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 880, !TEST_FLAG(object->object.flags, _object_connected_to_map_bit));

	if (object->object.parent_object_index!=NONE)
	{
		struct object_datum *parent_object= object_get(object->object.parent_object_index);
		object_add_to_list(&parent_object->object.first_child_object_index, object_index);

		SET_FLAG(header->flags, _object_header_child_bit, TRUE);
		object->object.location.cluster_index=NONE;
	}
	else
	{
		if (!location)
		{
			struct location bounding_sphere_location;
			
			scenario_location_from_point(&bounding_sphere_location, &object->object.bounding_sphere_center);
			location= &bounding_sphere_location;

			if (bounding_sphere_location.cluster_index==NONE)
			{
				scenario_location_from_point(&bounding_sphere_location, &object->object.position);
			}
		}

		if (location->cluster_index!=NONE)
		{
			object->object.location= *location;
			header->cluster_index= object->object.location.cluster_index;

			SET_FLAG(object->object.flags, _object_outside_of_map_bit, FALSE);
		}
		else
		{
			SET_FLAG(object->object.flags, _object_outside_of_map_bit, TRUE);
		}

		SET_FLAG(header->flags, _object_header_child_bit, FALSE);

		cluster_partition_reconnect(
			(TEST_FLAG(object_get(object_index)->object.flags, _object_has_collision_model_bit) ?
				&collideable_object_cluster_partition :
				&noncollideable_object_cluster_partition
			),
			object_index,
			&object->object.first_cluster_reference_index,
			&object->object.bounding_sphere_center,
			object->object.bounding_sphere_radius,
			&object->object.location
		);


		if (TEST_FLAG(header->flags, _object_header_automatically_deactivate_bit))
		{
			if (header->cluster_index==NONE || !BIT_VECTOR_TEST_FLAG(players_get_combined_pvs(), header->cluster_index))
			{
				if (TEST_FLAG(object->object.flags, _object_deleted_when_deactivated_bit))
				{
					object_delete(object_index);
				}
			}
			else
			{
				object_activate(object_index);
			}
		}
	}

	SET_FLAG(object->object.flags, _object_connected_to_map_bit, TRUE);
	SET_FLAG(header->flags, _object_header_connected_to_map_bit, TRUE);
	
	return;
}

real_matrix4x3 *object_get_node_matrix(long object_index, short node_index)
{
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1060, object_has_node(object_index, node_index));

	return &((real_matrix4x3 *)object_header_block_get(object_index, &object_get(object_index)->object.node_matrices))[node_index];
}

short object_get_marker_by_name(
	long object_index,
	char const *name,
	struct object_marker *markers,
	short maximum_marker_count)
{
	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object->definition_index);
	real_matrix4x3 *nodes= (real_matrix4x3 *)object_header_block_get(object_index, &object_get(object_index)->object.node_matrices);
	boolean mirrored= TEST_FLAG(object->object.flags, _object_mirrored_bit);

	short marker= model_get_marker_by_name(
			 object_definition->object.model.index,
			 name,
			 object->object.region_permutations,
			 FALSE,
			 NONE,
			 nodes,
			 mirrored,
			 markers,
			 maximum_marker_count
	);

	if (!marker)
	{
		match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1113, maximum_marker_count>0);
		
		markers->node_index= 0;
		matrix4x3_identity(&markers->node_matrix);
	
		markers->matrix= *object_get_node_matrix(object_index, 0);

		if (TEST_FLAG(object->object.flags, _object_mirrored_bit))
		{
			markers->matrix.n[1][0]= -markers->matrix.n[1][0];
			markers->matrix.n[1][1]= -markers->matrix.n[1][1];
			markers->matrix.n[1][2]= -markers->matrix.n[1][2];
		}

		if (name && name[0]=='\0')
		{
			marker= 1;
		}
	}

	return marker;
}

// TODO: match
void object_align_marker_to_matrix(
	struct object_datum *object,
	struct object_marker const *child_marker,
	real_matrix4x3 const *destination_matrix)
{
	real_matrix4x3 marker_transform_matrix;
	real_matrix4x3 final_matrix;

	real_vector3d vector;

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1173, object);
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1174, child_marker);
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1175, destination_matrix);

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1177, valid_real_matrix4x3(destination_matrix));

	matrix4x3_from_point_and_vectors(&final_matrix, &object->object.position, &object->object.forward, &object->object.up);
	
	matrix4x3_inverse(&final_matrix, &marker_transform_matrix);
	matrix4x3_multiply(&marker_transform_matrix, &child_marker->matrix, &marker_transform_matrix);
	matrix4x3_inverse(&marker_transform_matrix, &marker_transform_matrix);
	matrix4x3_multiply(destination_matrix, &marker_transform_matrix, &final_matrix);

	object->object.position= final_matrix.position;
	object->object.forward= final_matrix.forward;

	cross_product3d(&final_matrix.forward, &final_matrix.up, &vector);
	cross_product3d(&vector, &final_matrix.forward, &object->object.up);

	normalize3d(&object->object.forward);
	normalize3d(&object->object.up);

	return;
}

void object_detach(
	long child_object_index)
{
	real_matrix4x3 object_translation_matrix;
	real_matrix4x3 matrix;
	real_matrix4x3 object_rotation_matrix;

	real_matrix4x3 *node_matrix;

	struct object_datum *child_object= object_get(child_object_index);
	struct object_datum *parent_object= object_get(child_object->object.parent_object_index);

	object_disconnect_from_map(child_object_index);
	node_matrix= object_get_node_matrix(child_object->object.parent_object_index, child_object->object.parent_node_index);

	matrix4x3_translation(&object_translation_matrix, &child_object->object.position);
	matrix4x3_rotation_from_vectors(&object_rotation_matrix, &child_object->object.forward, &child_object->object.up);
	matrix4x3_multiply(node_matrix, &object_translation_matrix, &matrix);
	matrix4x3_multiply(&matrix, &object_rotation_matrix, &matrix);
	matrix4x3_to_point_and_vectors(
		&matrix,
		&child_object->object.position,
		&child_object->object.forward,
		&child_object->object.up
	);

	child_object->object.translational_velocity= parent_object->object.translational_velocity;
	child_object->object.angular_velocity= parent_object->object.angular_velocity;
	child_object->object.parent_node_index= NONE;
	child_object->object.parent_object_index= NONE;

	object_reconnect_to_map(child_object_index, NULL);
	object_activate(child_object_index);

	return;
}

real_point3d *object_get_origin(
	long object_index,
	real_point3d *origin)
{
	struct object_datum *object= object_get(object_index);

	if (object->object.parent_object_index==NONE)
	{
		*origin= object->object.position;
	}
	else
	{
		real_matrix4x3 *matrix= object_get_node_matrix(object->object.parent_object_index, object->object.parent_node_index);
		matrix4x3_transform_point(matrix, &object->object.position, origin);
	}

	return origin;
}

void object_get_orientation(
	long object_index,
	real_vector3d *forward,
	real_vector3d *up)
{
	struct object_datum *object= object_get(object_index);

	if (object->object.parent_object_index==NONE)
	{
		if (forward)
		{
			*forward= object->object.forward;
		}
		if (up)
		{
			*up= object->object.up;
		}
	}
	else
	{
		const real_matrix4x3 *node_matrix= object_get_node_matrix(
			object->object.parent_object_index,
			object->object.parent_node_index
		);

		if (forward)
		{
			matrix4x3_transform_normal(node_matrix, &object->object.forward, forward);
		}
		if (up)
		{
			matrix4x3_transform_normal(node_matrix, &object->object.up, up);
		}

		if (forward && up)
		{
			match_assert_valid_real_vector3d_axes2("c:\\halo\\SOURCE\\objects\\objects.c", 1462, forward, up);
		}
	}

	return;
}

real_matrix4x3 *object_get_world_matrix(
	long object_index,
	real_matrix4x3 *matrix)
{
	struct object_datum *object= object_get(object_index);
	matrix4x3_from_point_and_vectors(matrix, &object->object.position, &object->object.forward, &object->object.up);

	if (object->object.parent_object_index!=NONE)
	{
		real_matrix4x3* node_matrix= object_get_node_matrix(object->object.parent_object_index, object->object.parent_node_index);
		matrix4x3_multiply(node_matrix, matrix, matrix);
	}

	return matrix;
}

void object_inverse_kinematics(
	long hand_object_index,
	char *hand_marker_name,
	long grip_object_index,
	char *grip_marker_name,
	real_matrix4x3 *node_matrices)
{
	struct object_marker hand_marker;
	struct object_marker grip_marker;
	real_matrix4x3 desired_hand_matrix;

	struct object_datum *hand_object= object_get(hand_object_index);
	struct object_definition *hand_object_definition= object_definition_get(hand_object->definition_index);
	struct model *hand_model= model_definition_get(hand_object_definition->object.model.index);
	
	if (object_get_marker_by_name(hand_object_index, hand_marker_name, &hand_marker, 1) && 
		object_get_marker_by_name(grip_object_index, grip_marker_name, &grip_marker, 1))
	{
		short hand_node_index= hand_marker.node_index;
		short hand_parent_node_index= TAG_BLOCK_GET_ELEMENT(&hand_model->nodes, hand_node_index, struct model_node)->parent_node_index;
		short desired_hand_index= NONE;

		if (hand_parent_node_index!=NONE)
		{
			desired_hand_index= TAG_BLOCK_GET_ELEMENT(&hand_model->nodes, hand_parent_node_index, struct model_node)->parent_node_index;
		}

		if (desired_hand_index!=NONE)
		{
			matrix4x3_inverse(&hand_marker.node_matrix, &desired_hand_matrix);
			matrix4x3_multiply(&grip_marker.matrix, &desired_hand_matrix, &desired_hand_matrix);
			inverse_kinematics_adjust_matrices(
				&desired_hand_matrix,
				&node_matrices[desired_hand_index],
				&node_matrices[hand_parent_node_index],
				&node_matrices[hand_node_index]
			);
		}
	}

	return;
}

short objects_in_sphere(
	unsigned long class_flags,
	unsigned long type_flags,
	struct location const *location,
	real_point3d const *center,
	real radius,
	long *object_indices,
	short maximum_count)
{
	return NONE;
}

void objects_reconnect_to_structure_bsp(
	void)
{
	return;
}

void objects_paparazzi(
	void)
{
	struct object_datum *object;
	struct object_iterator iterator;

	object_iterator_new(&iterator, _object_mask_all, 0);

	while (object= (struct object_datum *)object_iterator_next(&iterator))
	{
		if (TEST_FLAG(object->object.flags, _object_movie_star_bit))
		{
			object_delete_initial_recursive(iterator.index, 0);
		}
	}

	return;
}

void object_export_function_values(
	long object_index)
{
	return;
}

void object_compute_node_matrices(
	long object_index)
{
	real_orientation cannot_interpolate_node_orientations_storage[MAXIMUM_NODES_PER_MODEL];
	short node_stack[MAXIMUM_NODES_PER_MODEL];
	real_matrix4x3 node_matrix;
	real_matrix4x3 object_translation_matrix;
	real_matrix4x3 object_rotation_matrix;
	real_matrix4x3 center_matrix;
	real_point3d center;
	real_matrix4x3 offset_matrix;
	real_matrix4x3 parent_node_matrix_no_scale_no_mirror;

	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object->definition_index);
	
	real_matrix4x3 *object_nodes= (real_matrix4x3 *)object_header_block_get(object_index, &object->object.node_matrices);
	real_orientation *node_orientations=
		TEST_FLAG(_object_mask_cannot_interpolate, object->object.type) ?
		cannot_interpolate_node_orientations_storage :
		(real_orientation *)object_header_block_get(object_index, &object->object.node_orientations);

	if (object_definition->object.model.index!=NONE)
	{
		struct model *model;
		real_matrix4x3 *object_node_matrix;
		boolean world_relative;

		object_type_definition_get(object->object.type);
		
		model=  model_definition_get(object_definition->object.model.index);
		object_node_matrix=
			object->object.parent_object_index!=NONE ?
			object_get_node_matrix(object->object.parent_object_index, object->object.parent_node_index) :
			0;

		world_relative= FALSE;
		if (object->object.animation.animation_graph_index!=NONE &&
			object->object.animation.state.index!=NONE)
		{
			short frame_index;

			struct animation_graph *animation_graph= animation_graph_definition_get(
				object->object.animation.animation_graph_index
			);
			struct animation* animation= TAG_BLOCK_GET_ELEMENT(
				&animation_graph->animations,
				object->object.animation.state.index,
				struct animation
			);
		
			if (TEST_FLAG(object->object.flags, _object_animates_automatically_bit) &&
				animation->frame_count>0)
			{
				frame_index= object->object.animation.state.frame_index;
			}
			else
			{
				frame_index= (object_index + game_time_get()) % animation->frame_count;
				match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2704, frame_index>=0)
			}
			animation_get_node_orientations(model, animation, frame_index, node_orientations);
			world_relative= TEST_FLAG(animation->flags, _animation_world_relative_bit);
		}
		else
		{
			model_get_node_orientations(model, node_orientations);
		}

		if (object_definition->object.animation_graph.index!=NONE)
		{
			struct animation_graph *animation_graph= animation_graph_definition_get(
			   object->object.animation.animation_graph_index
			);
		}

		// Scale the object orientations
		if (object->object.scale>0.f)
		{
			node_orientations->scale*= object->object.scale;
			node_orientations->translation.x*= object->object.scale;
			node_orientations->translation.y*= object->object.scale;
			node_orientations->translation.z*= object->object.scale;
		}


		if (object_definition->object.animation_graph.index!=NONE)
		{
			object_type_preprocess_node_orientations(object_index, node_orientations);
		}

		if (object->object.animation.interpolation_frame_count>0)
		{
			match_assert(
				"c:\\halo\\SOURCE\\objects\\objects.c",
				2777,
				!TEST_FLAG(_object_mask_cannot_interpolate, object->object.type)
			);

			interpolate_node_orientations(
				model->nodes.count,
				(real_orientation *)object_header_block_get(
					object_index,
					&object->object.original_node_orientations
				),
				node_orientations,
				object->object.animation.interpolation_frame_index,
				object->object.animation.interpolation_frame_count
			);
		}
	}
	else
	{
		object_nodes[0].scale= 1.f;
		object_nodes[0].forward= object->object.forward;
		object_nodes[0].up= object->object.up;
		cross_product3d(&object_nodes[0].up, &object_nodes[0].forward, &object_nodes[0].left);
		object_nodes[0].position= object->object.position;
	}

	matrix4x3_transform_point(
		object_nodes,
		&object_definition->object.bounding_offset,
		&object->object.bounding_sphere_center
	);
	object->object.bounding_sphere_radius= object_definition->object.bounding_radius;
	if (object->object.scale > 0.f)
	{
		object->object.bounding_sphere_radius*= object->object.scale;
	}

	return;
}

static void object_postprocess_node_matrices(
	long object_index)
{
	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object->definition_index);

	if (object_definition->object.model.index!=NONE &&
		object_definition->object.animation_graph.index != NONE)
	{
		real_matrix4x3 *matrices= (real_matrix4x3 *)object_header_block_get(
				object_index,
				&object->object.node_matrices
		);
		object_type_postprocess_node_matrices(object_index, matrices);
	}

	return;
}

static void object_choose_random_change_colors(
	long object_index,
	real_rgb_color *placement_change_colors)
{
	long i;

	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object->definition_index);

	for (i=0; i<NUMBEROF(object->object.base_change_colors); i++)
	{
		object->object.base_change_colors[i]= placement_change_colors[i];
	}

	return;
}

void objects_scripting_detach(
	long parent_object_index,
	long child_object_index)
{
	if (parent_object_index!=NONE &&
		child_object_index!=NONE &&
		object_get(child_object_index)->object.parent_object_index == parent_object_index)
	{
		object_detach(child_object_index);
	}

	return;
}

void object_render_debug(
	long object_index)
{
	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object->definition_index);

	if (debug_objects_root_node)
	{
		render_debug_matrix(TRUE, object_get_node_matrix(object_index, 0), 0.3f);
	}

	if (debug_objects_names)
	{
		if (object->object.name_index!=NONE)
		{
			real_point3d origin;

			struct scenario_object_name* object_name= TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->object_names,
				object->object.name_index,
				struct scenario_object_name
			);
			object_get_origin(object_index, &origin);
			render_debug_string_at_point(FALSE, &origin, object_name->name, global_real_argb_purple);
		}
	}

	if (debug_objects_position_velocity)
	{
		real_vector3d velocity;
		real_matrix4x3 world_matrix;
		
		char* model_name= strrchr(object_definition->object.model.name, '\\');
		object_get_world_matrix(object_index, &world_matrix);
		object_get_velocities(object_index, &velocity, NULL);

		model_name= model_name ? &model_name[1] : object_definition->object.model.name;
		
		render_debug_string_at_point(0, &world_matrix.position, model_name, global_real_argb_orange);
		render_debug_matrix(TRUE, &world_matrix, object->object.bounding_sphere_radius);
		render_debug_vector(TRUE, &world_matrix.position, &velocity, 1.f, global_real_argb_yellow);
	}

	if (debug_objects_bounding_spheres &&
		(object_definition->object.model.index!=NONE ||object_definition->object.collision_model.index!=NONE))
	{
		const real_argb_color *draw_color= global_real_argb_black;

		// Bounding sphere is colored differently based on a couple factors:
		// Red: Invalid bounding sphere
		// Yellow: Active object
		// Black: Inactive object
		if (object->object.bounding_sphere_radius<=0.f)
		{
			draw_color= global_real_argb_red;
		}
		else
		{
			if (TEST_FLAG(object_header_get(object_index)->flags, _object_header_active_bit))
			{
				draw_color= global_real_argb_yellow;
			}
		}

		render_debug_sphere(
			TRUE,
			&object->object.bounding_sphere_center,
			object->object.bounding_sphere_radius>0.f ? object->object.bounding_sphere_radius : 1.f,
			draw_color
		);
	}

	if (debug_objects_collision_models)
	{
		struct collision_model_instance collision_model_instance;
		if (collision_model_instance_new(&collision_model_instance, object_index))
		{
			render_debug_collision_model(&collision_model_instance);
		}
	}

	if (debug_objects_physics)
	{
		struct physics_instance physics_instance;
		if (physics_instance_new(&physics_instance, object_index))
		{
			render_debug_physics(&physics_instance);
		}
	}

	if (debug_objects_pathfinding_spheres)
	{
		if (!TEST_FLAG(object->object.flags, _object_invisible_bit) &&
			!TEST_FLAG(object->object.damage_flags, _object_dead_bit))
		{
			if (
				object->object.type != _object_type_machine ||
				TEST_FLAG(((struct machine_definition *)object_definition)->machine.flags, _machine_is_pathfinding_obstacle_bit) &&
				(
					!TEST_FLAG(
						((struct machine_definition *)object_definition)->machine.flags, 
						_machine_is_not_pathfinding_obstacle_when_open_bit
					) ||
					((struct machine_datum *)object)->device.position!=1.f)
				)
			{
				if (object_definition->object.collision_model.index!=NONE)
				{
					real_matrix4x3 world_matrix;
					short sphere_index;

					struct collision_model* collision_model= collision_model_definition_get(
						object_definition->object.collision_model.index
					);
					
					object_get_world_matrix(object_index, &world_matrix);

					// Render all pathfinding spheres

					for (sphere_index= 0; sphere_index<collision_model->pathfinding_spheres.count; ++sphere_index)
					{
						real_point3d origin;

						struct pathfinding_sphere *pathfinding_sphere= TAG_BLOCK_GET_ELEMENT(
							&collision_model->pathfinding_spheres,
							sphere_index,
							struct pathfinding_sphere
						);

						if (pathfinding_sphere->node_index!=NONE)
						{
							real_matrix4x3 *node_matrix= object_get_node_matrix(
								object_index,
								pathfinding_sphere->node_index
							);
							matrix4x3_transform_point(
								node_matrix,
								&pathfinding_sphere->center,
								&origin
							);
						}
						else
						{
							matrix4x3_transform_point(
								&world_matrix,
								&pathfinding_sphere->center,
								&origin
							);
						}
						render_debug_sphere(
							TRUE,
							&origin,
							pathfinding_sphere->radius,
							global_real_argb_blue
						);
					}


					if (collision_model->pathfinding_box.x0<collision_model->pathfinding_box.x1 &&
						collision_model->pathfinding_box.y0<collision_model->pathfinding_box.y1 &&
						collision_model->pathfinding_box.z0<collision_model->pathfinding_box.z1)
					{
						real_point3d line_points[NUMBER_OF_VERTICES_PER_LINE][NUMBER_OF_VERTICES_PER_LINE][NUMBER_OF_VERTICES_PER_LINE];
						short i;
						short j;
						short k;

						for (i= 0; i<NUMBER_OF_VERTICES_PER_LINE; ++i)
						{
							for (j= 0; j<NUMBER_OF_VERTICES_PER_LINE; ++j)
							{
								for (k= 0; k<NUMBER_OF_VERTICES_PER_LINE; ++k)
								{
									line_points[i][j][k].x= collision_model->pathfinding_box.n[i + 0];	// x0 and x1
									line_points[i][j][k].y= collision_model->pathfinding_box.n[j + 2];	// y0 and y1
									line_points[i][j][k].z= collision_model->pathfinding_box.n[k + 4];	// z0 and z1
									matrix4x3_transform_point(
										&world_matrix,
										&line_points[i][j][k],
										&line_points[i][j][k]
									);
								}
							}
						}


						for (i= 0; i < NUMBER_OF_VERTICES_PER_LINE; i++)
						{
							for (j= 0; j < NUMBER_OF_VERTICES_PER_LINE; j++)
							{
								for (k= 0; k < NUMBER_OF_VERTICES_PER_LINE; k++)
								{
									if (i==0)
									{
										render_debug_line(
											TRUE,
											&line_points[0][j][k],
											&line_points[1][j][k],
											global_real_argb_blue
										);
									}
										
									if (j==0)
									{
										render_debug_line(
											TRUE,
											&line_points[i][0][k],
											&line_points[i][1][k],
											global_real_argb_blue
										);
									}

									if (k==0)
									{
										render_debug_line(
											TRUE,
											&line_points[i][j][0],
											&line_points[i][j][1],
											global_real_argb_blue
										);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return;
}

void object_set_position(
	long object_index,
	real_point3d const *position,
	real_vector3d const *forward,
	real_vector3d const *up)
{
	return;
}

void object_translate(
	long object_index,
	real_point3d const *new_position,
	struct location const *new_location)
{
	struct object_datum *object= object_get(object_index);

	match_assert_valid_real_point3d(
		"c:\\halo\\SOURCE\\objects\\objects.c",
		562, 
		new_position
	);

	object_disconnect_from_map(object_index);
	object->object.position= *new_position;
	object_reconnect_to_map(object_index, new_location);

	return;
}

long object_new(
	struct object_placement_data *data)
{
	long definition_index;

	long object_index= NONE;

	match_assert_valid_real_point3d("c:\\halo\\SOURCE\\objects\\objects.c", 618, &data->position)
	match_assert_valid_real_vector3d_axes2("c:\\halo\\SOURCE\\objects\\objects.c", 619, &data->forward, &data->up);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\objects\\objects.c", 620, &data->angular_velocity);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\objects\\objects.c", 621, &data->translational_velocity);

	definition_index= data->definition_index;
	if (game_engine_running())
	{
		if (definition_index!=NONE)
		{
			definition_index= game_engine_remap_object_definition(definition_index);
		}
	}

	if (definition_index!=NONE)
	{
		struct object_definition *object_definition;
		struct object_type_definition *type_definition;

		

		object_definition= object_definition_get(definition_index);
		type_definition= object_type_definition_get(object_definition->object.type);
		object_index= object_header_new(object_header_data, NONE, type_definition->game_datum_size);
	
		if (object_index!=NONE)
		{
			boolean can_create_object;
			short node_count;

			struct object_header_datum *header= object_header_get(object_index);
			struct object_datum *object= object_get(object_index);

			SET_FLAG(header->flags, _object_header_being_created_bit, TRUE);
			SET_FLAG(header->flags, _object_header_automatically_deactivate_bit, TRUE);
			
			header->type= object_definition->object.type;
			object->definition_index= definition_index;
		
			can_create_object= TRUE;

			object->object.type= object_definition->object.type;
			object_type_adjust_placement(object_index, data);
			object->object.position= data->position;
			object->object.forward= data->forward;
			object->object.up= data->up;
			object->object.translational_velocity= data->translational_velocity;
			object->object.angular_velocity= data->angular_velocity;

			// TODO: real math function here to calculate new position
			object->object.position.x+= data->height* object->object.up.i;
			object->object.position.y+= data->height* object->object.up.j;
			object->object.position.z+= data->height* object->object.up.k;

			SET_FLAG(object->object.flags, _object_mirrored_bit, TEST_FLAG(data->flags, _new_object_mirrored_bit));

			object->object.location.cluster_index= NONE;
			header->cluster_index= NONE;
			object->object.magic_number= global_object_marker-1;
			object->object.umbrella_shield_object_index= NONE;
			object->object.first_cluster_reference_index= NONE;
			object->object.animation.state.index= NONE;
			object->object.animation.animation_graph_index= object_definition->object.animation_graph.index;
			object->object.cached_render_state_index= NONE;
			object->object.parent_object_index= NONE;
			object->object.next_object_index= NONE;
			object->object.first_child_object_index= NONE;
			object->object.name_index= NONE;
			object->object.shield_damage_decay_timer= NONE;
			object->object.body_damage_decay_timer= NONE;

			if (TEST_FLAG(object_definition->object.flags, _object_does_not_cast_shadow_bit))
			{
				SET_FLAG(object->object.flags, _object_shadowless_bit, TRUE);
			}

			SET_FLAG(object->object.flags, _object_no_collisions_bit, object_definition->object.collision_model.index!=NONE);

			object_set_visibility(object_index, object_definition->object.model.index!=NONE);
			object->object.owner_team_index= data->owner_team_index;
			object->object.owner_player_index= data->owner_player_index;
			object->object.owner_object_index= data->owner_object_index;
			object->object.variant_number= data->variant_number;
			object->object.forced_shader_permutation_index= object_definition->object.forced_shader_permutation_index;
		
			if (object_definition->object.model.index==NONE)
			{
				node_count=1;
			}
			else
			{
				node_count= model_definition_get(object_definition->object.model.index)->nodes.count;
			}

			if (!object_header_block_allocate(
					object_index,
					offsetof(struct object_datum, object.node_matrices),
					sizeof(struct real_matrix4x3) * node_count) ||
				TEST_FLAG(_object_mask_cannot_interpolate, object_definition->object.type) && 
				!object_header_block_allocate(
					object_index,
					offsetof(struct object_datum, object.node_orientations),
					sizeof(struct real_orientation) * node_count)
				|| !object_header_block_allocate(
						object_index,
						offsetof(struct object_datum, object.original_node_orientations),
						sizeof(struct real_orientation) * node_count)
				)
			{
				can_create_object= FALSE;
			}

			// Get the object, we do this so we can verify that we're able to retrieve it in the future
			object= object_get(object_index);

			if (can_create_object && object_type_new(object_index))
			{
				boolean original_deleted_when_deactivated= TEST_FLAG(object->object.flags, _object_deleted_when_deactivated_bit);
				if (TEST_FLAG(object->object.flags, _object_deleted_when_deactivated_bit) &&
					TEST_FLAG(data->flags, _new_object_never_automatically_delete_bit))
				{
					SET_FLAG(object->object.flags, _object_deleted_when_deactivated_bit, FALSE);
				}

				object_choose_random_change_colors(object_index, data->change_colors);
				object_choose_random_region_permutations(object_index);
				object_initialize_vitality(object_index, NULL, NULL);
				object_compute_node_matrices(object_index);
				object_reconnect_to_map(object_index, NULL);
				object_postprocess_node_matrices(object_index);
				object_type_export_function_values(object_index);
				object_compute_function_values();
				object_compute_change_colors();

				object_create_attachments(object_index);

				SET_FLAG(object->object.flags, _object_deleted_when_deactivated_bit, original_deleted_when_deactivated);	// Restore value

				if (!TEST_FLAG(header->flags, _object_header_active_bit) &&
					TEST_FLAG(object->object.flags, _object_deleted_when_deactivated_bit) &&
					!TEST_FLAG(data->flags, _new_object_never_automatically_delete_bit) ||
					object->object.location.cluster_index!=NONE)
				{
					object_delete(object_index);
				}

				if (object_definition->object.creation_effect.index!=NONE)
				{
					effect_new_from_object(
						object_definition->object.creation_effect.index,
						object_index,
						object_index,
						NONE,
						0.f,
						0.f,
						NULL,
						NULL
					);
				}
			}
			else
			{
				object_type_delete(object_index);
				object_header_delete(object_header_data, object_index);
				object_index= NONE;
			}
		}

		if (object_index==NONE)
		{
			char string[512];
			sprintf(string, "OUT OF OBJECTS: cannot create %s", tag_name_strip_path(tag_get_name(definition_index)));
			console_printf(FALSE, "%s", string);
			error(_error_log, "%s", string);
		}
	}
	
	return object_index;
}


void object_attach_to_node(
	long parent_object_index,
	long child_object_index,
	short parent_node_index)
{
	long object_index;
	struct object_datum *object;
	struct object_datum *child_object;
	struct object_datum *parent_object;
	boolean connected_to_map;
	real_matrix4x3 inverse_node_matrix;

	for (
		object_index= parent_object_index;
		object_index!=NONE;
		object_index= object->object.parent_object_index)
	{
		object= object_get(object_index);
		match_vassert(
			"c:\\halo\\SOURCE\\objects\\objects.c",
			1225,
			object_index != child_object_index,
			"cannot attach an object to one of its children"
		);
	}

	child_object= object_get(child_object_index);
	parent_object= object_get(parent_object_index);

	connected_to_map= TEST_FLAG(child_object->object.flags, _object_connected_to_map_bit);

	match_assert(
		"c:\\halo\\SOURCE\\objects\\objects.c",
		1235,
		object_has_node(parent_object_index, parent_node_index)
	);

	if (connected_to_map)
	{
		object_disconnect_from_map(child_object_index);
	}

	matrix4x3_inverse(object_get_node_matrix(parent_object_index, parent_node_index), &inverse_node_matrix);
	matrix4x3_transform_point(&inverse_node_matrix, &child_object->object.position, &child_object->object.position);
	matrix4x3_transform_normal(&inverse_node_matrix, &child_object->object.forward, &child_object->object.forward);
	matrix4x3_transform_normal(&inverse_node_matrix, &child_object->object.up, &child_object->object.up);
	child_object->object.parent_object_index= parent_object_index;
	child_object->object.parent_node_index= parent_node_index;

	if (connected_to_map)
	{
		object_reconnect_to_map(child_object_index, NULL);
	}
	
	object_deactivate(child_object_index);
	object_compute_node_matrices(child_object_index);

	return;
}

boolean object_force_inside_bsp(
	long object_index,
	long ignore_object_index,
	real_point3d const *known_good_point)
{
	return FALSE;
}

void object_compute_node_matrices_recursive(
	long object_index)
{
	long child_index;
	struct object_datum *child_object;
	struct object_datum *object= object_get(object_index);
	
	object_compute_node_matrices(object_index);

	for (
		child_index= object->object.first_child_object_index;
		child_index!=NONE;
		child_index= child_object->object.next_object_index)
	{
		child_object= object_get(child_index);
		object_compute_node_matrices_recursive(child_index);
	}
	
	return;
}

long object_new_from_scenario(
	struct scenario_object_datum *scenario_object,
	struct tag_block *palette)
{
	struct object_placement_data placement_data;
	
	long result= NONE;

	if (scenario_object->palette_entry_index!=NONE &&
		(
			!object_globals->initial_placement ||
			!TEST_FLAG(scenario_object->placement_flags, _scenario_object_placement_not_automatic_bit)
		))
	{
		if (scenario_object->name_index==NONE ||
			object_name_list_lookup(scenario_object->name_index)==NONE)
		{
			long definition_index= TAG_BLOCK_GET_ELEMENT(
				palette,
				scenario_object->palette_entry_index,
				struct scenario_object_palette_entry
			)->reference.index;

			if (definition_index!=NONE)
			{
				object_placement_data_new(&placement_data, definition_index, NONE);
				placement_data.position= scenario_object->position;
				vectors3d_from_euler_angles3d(
					&placement_data.forward,
					&placement_data.up,
					&scenario_object->rotation
				);
				placement_data.variant_number= scenario_object->variant_number;
				
				result= object_new(&placement_data);
				if (result!=NONE)
				{
					object_type_place(result, scenario_object);
					if (scenario_object->name_index!=NONE)
					{
						object_name_list_new(result, scenario_object->name_index);
					}
				}
			}
		}
	}

	return result;
}

void object_attach_to_marker(
	long parent_object_index,
	char const *parent_marker_name,
	long child_object_index,
	char const *child_marker_name)
{
	struct object_marker parent_marker;
	struct object_marker child_marker;
	real_matrix4x3 inverted_matrix;

	struct object_datum *child_object= object_get(child_object_index);

	object_get_marker_by_name(parent_object_index, parent_marker_name, &parent_marker, 1);
	object_get_marker_by_name(child_object_index, child_marker_name, &child_marker, 1);
	object_disconnect_from_map(child_object_index);
	
	if (child_marker_name && *child_marker_name)
	{
		object_align_marker_to_matrix(child_object, &child_marker, &parent_marker.matrix);
	}
	else
	{
		matrix4x3_inverse(&child_marker.node_matrix, &inverted_matrix);
		matrix4x3_transform_point(&inverted_matrix, &parent_marker.matrix.position, &child_object->object.position);
		matrix4x3_transform_normal(&inverted_matrix, &parent_marker.matrix.forward, &child_object->object.forward);
		matrix4x3_transform_normal(&inverted_matrix, &parent_marker.matrix.up, &child_object->object.up);
	}

	object_reconnect_to_map(child_object_index, NULL);
	object_attach_to_node(parent_object_index, child_object_index, parent_marker.node_index);

	return;
}

long object_new_by_name(
	short name_index)
{
	long placement_tag_block_element_size;
	struct scenario *scenario= global_scenario_get();


	struct scenario_object_name* name= TAG_BLOCK_GET_ELEMENT(
		&scenario->object_names,
		name_index,
		struct scenario_object_name
	);

	struct tag_block* datum= scenario_get_object_type_scenario_datums(
		scenario,
		name->runtime_object_type,
		&placement_tag_block_element_size
	);

	struct tag_block *palette= scenario_get_object_type_scenario_palette(
		scenario,
		name->runtime_object_type
	);

	struct scenario_object_datum *object= (struct scenario_object_datum *)tag_block_get_element_with_size(
			datum,
			name->runtime_scenario_datum_index,
			placement_tag_block_element_size
	);

	return object_new_from_scenario(
		object,
		palette
	);
}

void objects_scripting_attach(
	long parent_object_index,
	char const *parent_marker_name,
	long child_object_index,
	char const *child_marker_name)
{
	if (parent_object_index!=NONE &&
		child_object_index !=NONE &&
		object_get(child_object_index)->object.parent_object_index==NONE)
	{
		object_attach_to_marker(parent_object_index, parent_marker_name, child_object_index, child_marker_name);
	}

	return;
}

void object_delete_immediately(
	long object_index)
{
	object_delete_initial_recursive(object_index, FALSE);
	object_delete_recursive(object_index, FALSE);

	return;
}

void objects_garbage_collection(
	void)
{
	long garbage_object_indices[MAXIMUM_OBJECTS_PER_MAP];
	char warningbuf[512];
	unsigned char release_proc_working_memory[4096];
	char tempbuffer[512];
	char released_resultbuf[512];

	long garbage_collect_mode= NONE;

	if (object_globals->force_garbage_collection)
	{
		garbage_collect_mode= _garbage_collect_everything;
	}
	else if (memory_pool_get_contiguous_free_size(object_memory_pool) > GARBAGE_LIMIT_FREE_MEMORY_CRITICAL)
	{
		if (MAXIMUM_OBJECTS_PER_MAP - object_header_data->actual_count > GARBAGE_LIMIT_FREE_OBJECTS_TRIGGER)
		{
			if (object_globals->active_garbage_object_count>=GARBAGE_LIMIT_ACTIVE_GARBAGE_TRIGGER)
			{
				garbage_collect_mode= _garbage_collect_active_objects;
			}
		}
		else
		{
			garbage_collect_mode= _garbage_collect_for_space;
		}
	}
	else
	{
		memory_pool_compact(object_memory_pool);
		if (memory_pool_get_contiguous_free_size(object_memory_pool)<=GARBAGE_LIMIT_FREE_MEMORY_TRIGGER)
		{
			garbage_collect_mode= _garbage_collect_for_space;
		}
		else
		{
			object_globals->force_garbage_collection= FALSE;
		}
	}
	
	if (garbage_collect_mode!=NONE)
	{
		long garbage_object_index;
		struct object_datum *object;

		short garbage_object_count= 0;
		boolean more_to_release= FALSE;

		if (debug_object_garbage_collection)
		{
			console_printf(
				FALSE,
				"#%d objects using 0x%x bytes (0x%x contiguous free)",
				object_header_data->actual_count,
				OBJECT_MEMORY_POOL_SIZE - memory_pool_get_free_size(object_memory_pool),
				memory_pool_get_contiguous_free_size(object_memory_pool)
			);
		}

		for (
			garbage_object_index= object_globals->first_garbage_object_index;
			garbage_object_index!=NONE;
			garbage_object_index=object->object.next_garbage_object_index)
		{
			object= object_get(garbage_object_index);
		
			match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 4289, garbage_object_index<MAXIMUM_OBJECTS_PER_MAP);

			garbage_object_indices[garbage_object_count]= garbage_object_index;
			++garbage_object_count;
		}

		while (TRUE)
		{
			long object_index;
			struct object_header_datum *header;

			switch (garbage_collect_mode)
			{
			case _garbage_collect_everything:
				more_to_release= FALSE;
				break;
			case _garbage_collect_active_objects:
				more_to_release= object_globals->active_garbage_object_count<=GARBAGE_LIMIT_ACTIVE_GARBAGE_TARGET;
				break;
			case _garbage_collect_for_space:
				more_to_release= TRUE;
				break;
			}

			if (more_to_release || !garbage_object_count)
			{
				break;
			}


			object_index= garbage_object_indices[--garbage_object_count];
			header= object_header_get(object_index);

			if (!object_visible_to_any_player(object_index) && garbage_collect_mode==_garbage_collect_active_objects)
			{
				struct object_datum *object= object_get(object_index);

				if (TEST_FLAG(_object_mask_unit, object->object.type) && !TEST_FLAG(object->object.damage_flags, _object_dead_bit))
				{
					error(
						_error_silent,
						"WARNING: garbage collecting a living unit (%s)",
						ai_debug_describe_actor(NONE, object_index, TRUE, temporary, NUMBEROF(temporary))
					);
				}


				if (TEST_FLAG(header->flags, _object_header_active_bit))
				{
					--object_globals->active_garbage_object_count;
				}
				object_set_garbage(object_index, FALSE);
				object_delete_immediately(object_index);
			}
		}

		memory_pool_compact(object_memory_pool);
		
		if (debug_object_garbage_collection)
		{
			console_printf(
				FALSE,
				"compacted to #%d with 0x%x contiguous bytes free",
				object_header_data->actual_count,
				memory_pool_get_contiguous_free_size(object_memory_pool)
			);
		}

		if (!more_to_release)
		{

		}
	}


	object_globals->force_garbage_collection= FALSE;

	return;
}


void objects_update(
	void)
{
	return;
}

void objects_memory_compact(
	void)
{
	objects_garbage_collection();
	memory_pool_compact(object_memory_pool);
	return;
}

/* ---------- private code */

static void object_connect_lights(
	long object_index,
	boolean disconnect,
	boolean reconnect)
{
	struct object_datum *object= object_get(object_index);

	if (TEST_FLAG(object->object.flags, _object_has_attached_lights_bit))
	{
		long i;
		struct object_definition *object_definition= object_definition_get(object->definition_index);
		
		for (i=0; i<object_definition->object.attachments.count; i++)
		{
			if (!object->object.attachment_types[i] && object->object.attachment_indices[i]!=NONE)
			{
				if (disconnect)
				{
					light_disconnect_from_map(object->object.attachment_indices[i]);
				}
				if (reconnect)
				{
					light_reconnect_to_map(object->object.attachment_indices[i]);
				}
			}
		}
	}

	return;
}

static void object_name_list_allocate(
	void)
{
	object_name_list= (long*)game_state_malloc("object name list", NULL, MAXIMUM_OBJECT_NAMES_PER_SCENARIO * sizeof(*object_name_list));
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 4072, object_name_list);

	return;
}

static void object_name_list_free(void)
{

	return;
}

static void object_name_list_clear(void)
{

	return;
}

static void object_name_list_new(long object_index, short name_index)
{
	struct object_datum *object= object_get(object_index);

	match_assert(
		"c:\\halo\\SOURCE\\objects\\objects.c",
		4099,
		name_index>=0 && name_index<MAXIMUM_OBJECT_NAMES_PER_SCENARIO
	);

	if (object_name_list[name_index]==NONE)
	{
		object_name_list[name_index]= object_index;
		object->object.name_index= name_index;
	}
	else
	{
		error(
			_error_silent,
			"an object with the name '%s' already exists!", 
			TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->object_names, name_index, struct scenario_object_name)->name
		);
	}

	return;
}

static void object_name_list_delete(
	long object_index)
{
	struct object_datum *object= object_get(object_index);

	if (object->object.name_index!=NONE)
	{
		short i;
		struct scenario *scenario= global_scenario_get();
		object->object.name_index= NONE;

		for (i=0; i<scenario->object_names.count; ++i)
		{
			if (object_name_list[i]==object_index)
			{
				object_name_list[i]= NONE;
			}
		}
	}

	return;
}

static long object_name_list_lookup(short name_index)
{
	return VALID_INDEX(name_index, MAXIMUM_OBJECT_NAMES_PER_SCENARIO) ? object_name_list[name_index] : NONE;
}

static long recursive_object_adder(
	long object_index,
	boolean(*add_object_function)(long, void *),
	void *custom_data,
	long object_count,
	long maximum_count,
	long *object_indices)
{
	struct object_datum *object= object_get(object_index);
	
	if (object_count<maximum_count)
	{
		if (!add_object_function || add_object_function(object_index, custom_data))
		{
			object_indices[object_count++]= object_index;
		}
		
		if (object->object.first_child_object_index!=NONE)
		{
			object_count= recursive_object_adder(
					   object->object.first_child_object_index,
					   add_object_function,
					   custom_data,
					   object_count,
					   maximum_count,
					   object_indices
			);
		}

		if (object->object.next_object_index!=NONE)
		{
			object_count= recursive_object_adder(
				object->object.next_object_index,
				add_object_function,
				custom_data,
				object_count,
				maximum_count,
				object_indices
			);
		}
	}
	return object_count;
}

static long sort_dumps(
	struct dump_datum const *dump1,
	struct dump_datum const *dump2)
{
	long result;
	if (dump1->total_size < dump2->total_size)
	{
		result= 1;
	}
	else if (dump1->total_size <= dump2->total_size)
	{
		result= 0;
	}
	else
	{
		result= -1;
	}

	return result;
}

static void object_add_to_dump(
	long object_index,
	struct dump_datum *dump)
{
	long parent_index;
	struct object_datum *parent;
	struct object_header_datum *header= object_header_get(object_index);
	struct object_datum *object= object_get(object_index);


	if (header->data_size>dump->maximum_size)
	{
		dump->maximum_size= header->data_size;
	}

	dump->total_size+= header->data_size;
	dump->count++;

	if (TEST_FLAG(header->flags, _object_header_active_bit))
	{
		dump->active_count++;
	}

	if (TEST_FLAG(object->object.flags, _object_garbage_bit))
	{
		dump->garbage_count++;
	}

	if (TEST_FLAG(object->object.damage_flags, _object_dead_bit))
	{
		dump->dead_count++;
	}

	if (TEST_FLAG(object->object.flags, _object_at_rest_bit))
	{
		dump->at_rest_count++;
	}

	parent_index= object_get_ultimate_parent(object_index);
	parent= object_get(parent_index);
	if (TEST_FLAG(parent->object.flags, _object_outside_of_map_bit) || parent->object.location.cluster_index==NONE)
	{
		dump->outside_map_count++;
	}
	return;
}

static void object_dump_write(
	FILE *stream,
	struct dump_datum *dump)
{
	const char *name= "unknown";
	if (dump->definition_index!=NONE)
	{
		name= tag_get_name(dump->definition_index);
	}
	else
	{
		if (dump->object_type!=NONE)
		{
			name= object_type_get_name(dump->object_type);
		}
	}

	fprintf(
		   stream,
		   "% 6d (% 6d) [% 7d/% 7d/% 7d/% 7d] % 7d % 7d %s\r\n",
		   dump->count,
		   dump->active_count,
		   dump->garbage_count,
		   dump->dead_count,
		   dump->outside_map_count,
		   dump->at_rest_count,
		   dump->maximum_size,
		   dump->total_size,
		   name);

	return;
}

static short object_find_region_permutations_available_with_variant(
	struct model_region *region,
	short variant_number,
	short *available_indices)
{
	return 0;
}

static boolean object_select_random_region_permutations_by_variant(
	long object_index,
	struct model *model,
	short variant_number)
{
	short available_permutation_indices[32];
	long region_index;

	struct object_datum *object= object_get(object_index);
	boolean result= TRUE;

	for (region_index=0; region_index<model->regions.count && !result; region_index++)
	{
		short permutation_index;
		struct model_region *region= TAG_BLOCK_GET_ELEMENT(&model->regions, region_index, struct model_region);
		
		permutation_index= object_find_region_permutations_available_with_variant(region, variant_number, available_permutation_indices);
		if (permutation_index || variant_number!=NONE)
		{
			permutation_index= object_find_region_permutations_available_with_variant(region, 0, available_permutation_indices);
		}

		if (permutation_index)
		{
			short permutation_num;
			if (permutation_index==1)
			{
				permutation_num= 0;
			}
			else
			{
				permutation_num= seed_random_range(get_global_random_seed_address(), 0, permutation_index);
			}
			object->object.region_permutations[region_index]= available_permutation_indices[permutation_num];
		}
		else
		{
			object->object.region_permutations[region_index]= 0;
			result= FALSE;
		}
	}

	return result;
}

static short object_determine_variant_number(
	long object_index,
	struct model *model)
{
	long region_index;
	short result;

	struct object_datum *object= object_get(object_index);
	
	for (region_index=0; region_index<model->regions.count&& !result; region_index++)
	{
		struct model_region* region= TAG_BLOCK_GET_ELEMENT(&model->regions, region_index, struct model_region);
		if (object->object.region_permutations[region_index] < region->permutations.count)
		{
			result= TAG_BLOCK_GET_ELEMENT(
				&region->permutations,
				object->object.region_permutations[region_index],
				struct model_region_permutation
			)->variant_number;
		}
	}

	return result;
}

static void object_choose_random_region_permutations(long object_index)
{
	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object->definition_index);

	if (object_definition->object.model.index!=NONE)
	{
		struct model* model= model_definition_get(object_definition->object.model.index);
		if (object->object.variant_number <= 0 ||
			!object_select_random_region_permutations_by_variant(object_index, model, object->object.variant_number))
		{
			object_select_random_region_permutations_by_variant(object_index, model, NONE);
			object->object.variant_number= object_determine_variant_number(object_index, model);
			if (object->object.variant_number > 0)
			{
				object_select_random_region_permutations_by_variant(object_index, model, object->object.variant_number);
			}
		}
	}

	return;
}

static void object_add_to_list(
	long *first_object_reference,
	long object_index)
{
	struct object_datum *object= object_get(object_index);
	object->object.next_object_index= *first_object_reference;
	*first_object_reference= object_index;

	return;
}

static void object_remove_from_list(
	long *first_object_reference,
	long object_index)
{
	while (*first_object_reference!=NONE)
	{
		struct object_datum *object= object_get(object_index);

		if (*first_object_reference==object_index)
		{
			*first_object_reference= object->object.next_object_index;
			object->object.next_object_index= NONE;
			break;
		}

		*first_object_reference= object->object.next_object_index;

		match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 3179, *first_object_reference!=NONE);
	}
	
	return;
}

static void attachments_delete(
	long object_index)
{
	long attachment_index;
	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object_index);

	for (attachment_index=0; attachment_index<object_definition->object.attachments.count; attachment_index++)
	{
		if (object->object.attachment_types[attachment_index]!=NONE && object->object.attachment_indices[attachment_index]!=NONE)
		{
			switch (object->object.attachment_types[attachment_index])
			{
			case _object_attachment_type_light:
				light_delete(object->object.attachment_indices[attachment_index]);
				break;
			case _object_attachment_type_looping_sound:
				game_looping_sound_delete(object->object.attachment_indices[attachment_index]);
				break;
			case _object_attachment_type_effect:
				effect_delete(object->object.attachment_indices[attachment_index]);
				break;
			case _object_attachment_type_contrail:
				object_compute_node_matrices(object_index);
				contrail_owner_collision(object->object.attachment_indices[attachment_index], TRUE, 0.f);
				break;
			case _object_attachment_type_particle_system:
				particle_system_orphan(object->object.attachment_indices[attachment_index]);
				break;
			default:
				break;
			}
		}
	}

	return;
}

static void object_create_attachments(
	long object_index)
{
	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object->definition_index);

	widgets_new(object_index);
	attachments_new(object_index);

	return;
}

static void object_delete_attachments(
	long object_index)
{
	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object_index);

	object_type_definition_get(object->object.type);
	widgets_delete(object_index);
	attachments_delete(object_index);

	return;
}

static void object_delete_recursive(
	long object_index,
	boolean delete_siblings)
{
	struct object_datum *object= object_get(object_index);
	struct object_definition *object_definition= object_definition_get(object_index);


	if (TEST_FLAG(object->object.flags, _object_garbage_bit))
	{
		object_set_garbage(object_index, FALSE);
	}

	object_deleted_procs_call(object_index);
	if (object->object.first_child_object_index!=NONE)
	{
		object_delete_recursive(object->object.first_child_object_index, TRUE);
	}

	if (delete_siblings)
	{
		if (object->object.next_object_index!=NONE)
		{
			object_delete_recursive(object->object.next_object_index, TRUE);
		}
	}

	object_deactivate(object_index);
	object_delete_attachments(object_index);

	if (TEST_FLAG(object->object.flags, _object_connected_to_map_bit))
	{
		object_disconnect_from_map(object_index);
	}
	
	object_type_delete(object_index);
	object_header_delete(object_header_data, object_index);

	return;
}
