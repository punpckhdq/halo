/*
MODEL_ANIMATIONS.C

symbols in this file:
0010FE40 0010:
	_animation_loop_frame_index (0000)
0010FE50 0010:
	_animation_second_key_frame_index (0000)
0010FE60 0010:
	_animation_sound_frame_index (0000)
0010FE70 0050:
	_code_0010fe70 (0000)
0010FEC0 00a0:
	_build_damage_animation_index (0000)
0010FF60 0080:
	_animation_get_x_offsets (0000)
0010FFE0 0080:
	_animation_set_frame_size (0000)
00110060 0060:
	_quaternion_decompress_8byte (0000)
001100C0 00c0:
	_quaternion_decompress_6byte (0000)
00110180 0020:
	_quaternion_decompress_6byte_renormalized (0000)
001101A0 0060:
	_quaternion_compress_8byte (0000)
00110200 0090:
	_quaternion_compress_6byte (0000)
00110290 0160:
	_animation_graph_node_matrices_from_orientations (0000)
001103F0 0110:
	_interpolate_node_orientations (0000)
00110500 0060:
	_animation_graph_get_animation_by_name (0000)
00110560 01d0:
	_code_00110560 (0000)
00110730 0040:
	_animation_frame_get_xy_translation (0000)
00110770 00b0:
	_animation_choose_random_permutation_internal (0000)
00110820 0360:
	_inverse_kinematics_adjust_matrices (0000)
00110B80 0310:
	_code_00110b80 (0000)
00110E90 0300:
	_code_00110e90 (0000)
00111190 02f0:
	_code_00111190 (0000)
00111480 0130:
	_animation_update_internal (0000)
001115B0 0300:
	_animation_get_node_orientations (0000)
001118B0 01e0:
	_replacement_animation_apply (0000)
00111A90 0210:
	_overlay_animation_apply (0000)
00111CA0 0240:
	_overlay_animation_apply_scaled (0000)
00111EE0 03c0:
	_overlay_animation_apply_continuous (0000)
001122A0 0400:
	_overlay_animation_apply_continuous_scaled (0000)
001126A0 0620:
	_aiming_screen_apply (0000)
00112CC0 0040:
	_animation_get_root_matrix (0000)
00112D00 00b0:
	_animation_get_root_velocity (0000)
0027F208 000a:
	??_C@_09KCBFNMAJ@animation?$AA@ (0000)
0027F214 0029:
	??_C@_0CJ@JGGOLPOM@c?3?2halo?2SOURCE?2models?2model_anim@ (0000)
0027F240 0035:
	??_C@_0DF@NDBCCBGA@damage_part?$DO?$DN0?5?$CG?$CG?5damage_part?$DMNU@ (0000)
0027F278 004e:
	??_C@_0EO@PDDEEPOK@damage_direction?$DO?$DN0?5?$CG?$CG?5damage_di@ (0000)
0027F2C8 003f:
	??_C@_0DP@CAICGJCA@damage_type?$DO?$DN0?5?$CG?$CG?5damage_type?$DMNU@ (0000)
0027F308 0004:
	__real@38000100 (0000)
0027F30C 0024:
	??_C@_0CE@FLNDNLBA@write_index?$DMMAXIMUM_NODES_PER_MO@ (0000)
0027F330 0018:
	??_C@_0BI@NCOKEGAP@frame_index?$DMframe_count?$AA@ (0000)
0027F348 000e:
	??_C@_0O@HJPIDGAL@frame_count?$DO0?$AA@ (0000)
0027F358 007a:
	??_C@_0HK@JCHJDAIG@target_frame_index?$DO?$DNkeyframe_fra@ (0000)
0027F3D4 0035:
	??_C@_0DF@BHOCMEDJ@keyframe_index?$DO?$DN0?5?$CG?$CG?5keyframe_in@ (0000)
0027F40C 001b:
	??_C@_0BL@NIECDBGN@?$CL?$CLinfinite_loop_killer?$DM200?$AA@ (0000)
0027F428 0033:
	??_C@_0DD@DKIDONPD@keyframe_index?$DO?$DN0?5?$CG?$CG?5keyframe_in@ (0000)
0027F460 0055:
	??_C@_0FF@IFOFHFLN@target_frame_index?$DO?$DN0?5?$CG?$CG?5target_@ (0000)
0027F4B8 001c:
	??_C@_0BM@EIBPKAOD@keyframe_frame_indices?$FL0?$FN?$DO0?$AA@ (0000)
0027F4D4 0017:
	??_C@_0BH@FMNFCNKN@keyframe_frame_indices?$AA@ (0000)
0027F4EC 0011:
	??_C@_0BB@INOOFBHG@keyframe_count?$DO1?$AA@ (0000)
0027F500 008e:
	??_C@_0IO@GIPANHA@?$CIanimation_update_kind_affects_g@ (0000)
0027F590 0004:
	__real@3f7ae148 (0000)
0027F594 0032:
	??_C@_0DC@FHNJEFLD@real_frame_index?$DM?5?$CIreal?$CJnext_key@ (0000)
0027F5C8 0032:
	??_C@_0DC@FGDAEANG@real_frame_index?$DO?$DN?$CIreal?$CJthis_key@ (0000)
0027F600 0043:
	??_C@_0ED@ICKGJKPC@keyframe_frame_indices?$FLkeyframe_@ (0000)
0027F648 0048:
	??_C@_0EI@OOKDDBPM@frame_index?$DO?$DN0?5?$CG?$CG?5frame_index?$DM?$DNk@ (0000)
0027F690 0012:
	??_C@_0BC@GPLHPFMP@keyframe_count?$DO?$DN0?$AA@ (0000)
0027F6A4 002e:
	??_C@_0CO@NEGCNKNH@real_frame_index?$DM?$CIreal?$CJanimation@ (0000)
0027F6D4 0017:
	??_C@_0BH@JENGDKGC@real_frame_index?$DO?$DN0?40f?$AA@ (0000)
0027F6F0 006f:
	??_C@_0GP@FNDBHMKG@compressed?5?$HM?$HM?5?$CIbyte?5?$CK?$CJdefault_da@ (0000)
0027F760 006b:
	??_C@_0GL@JOCEOLB@compressed?5?$HM?$HM?5?$CIbyte?5?$CK?$CJdata?9?$CIbyte@ (0000)
0027F7D0 006d:
	??_C@_0GN@PACKELMI@compressed?5?$HM?$HM?5?$CI?$CIbyte?5?$CK?$CJdata?9?$CIbyt@ (0000)
0027F840 0077:
	??_C@_0HH@GKOAHDOE@compressed?5?$HM?$HM?5?$CI?$CIbyte?5?$CK?$CJnext_data@ (0000)
0027F8B8 0048:
	??_C@_0EI@JELMFEDP@?$CD?$CD?$CD?5ERROR?5animation?5frame?5index?5@ (0000)
0027F900 0048:
	??_C@_0EI@GGOLLJMH@?$CD?$CD?$CD?5ERROR?5animation?5frame?5index?5@ (0000)
0027F948 0026:
	??_C@_0CG@FPLKLLFK@d0?$DN?$DN?$CFf?5direction?$CI?$CFf?$CJ?5yaw_delta?$CI?$CF@ (0000)
0027F970 0019:
	??_C@_0BJ@LEBMJMIB@animation?9?$DOframe_count?$DO1?$AA@ (0000)
0030A388 0001:
	_hs_model_animation_compression_enabled (0000)
0045662C 0024:
	_hs_model_animation_data_compressed_size (0000)
	_hs_model_animation_data_uncompressed_size (0004)
	_hs_model_animation_data_compression_savings_in_bytes (0008)
	_hs_model_animation_data_compression_savings_in_bytes_at_import (000c)
	_hs_model_animation_data_compression_savings_in_percent (0010)
	_hs_model_animation_bullshit (0014)
*/

/* ---------- headers */

#include "cseries.h"
#include "model_animation_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
