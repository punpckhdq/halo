/*
RENDER_CAMERAS.C

symbols in this file:
00174FC0 00a0:
	_code_00174fc0 (0000)
00175060 0020:
	_render_camera_new (0000)
00175080 0120:
	_render_camera_hack_frustum_z (0000)
001751A0 0120:
	_render_camera_build_frustum_bounds (0000)
001752C0 0060:
	_render_frustum_sphere_diameter_in_pixels (0000)
00175320 04b0:
	_render_frustum_cube_view_fraction (0000)
001757D0 00d0:
	_render_frustum_get_projection_bounds (0000)
001758A0 01e0:
	_render_camera_view_to_screen (0000)
00175A80 0100:
	_render_camera_screen_to_view (0000)
00175B80 0130:
	_render_camera_screen_to_world (0000)
00175CB0 0020:
	_render_camera_get_adjusted_field_of_view_tangent (0000)
00175CD0 0160:
	_render_camera_build_clipped_frustum_bounds (0000)
00175E30 00b0:
	_render_camera_triangle_frontfacing (0000)
00175EE0 0100:
	_render_frustum_build_point_flags (0000)
00175FE0 0060:
	_render_frustum_triangle_visible (0000)
00176040 02d0:
	_render_frustum_cube_visible (0000)
00176310 01b0:
	_render_frustum_sphere_visible (0000)
001764C0 00d0:
	_render_camera_world_to_screen (0000)
00176590 01b0:
	_render_camera_debug_frustum (0000)
00176740 0360:
	_render_camera_mirror (0000)
00176AA0 0d30:
	_render_camera_build_frustum (0000)
0029F768 0042:
	??_C@_0EC@HLNCAGFG@?$CD?$CD?$CD?5ERROR?5cameras?3?5frustum?9integ@ (0000)
0029F7AC 0035:
	??_C@_0DF@KOAPDDIF@id?$DO?$DN0?5?$CG?$CG?5id?$DMMAXIMUM_RENDER_CAMER@ (0000)
0029F7E4 0027:
	??_C@_0CH@JIJIIAGP@c?3?2halo?2SOURCE?2render?2render_cam@ (0000)
0029F80C 0025:
	??_C@_0CF@NAAMLNBF@frustum?5?$CG?$CG?5frustum?9?$DOprojection_v@ (0000)
0029F834 000f:
	??_C@_0P@IMNLCIED@frustum_bounds?$AA@ (0000)
0029F844 0027:
	??_C@_0CH@MLEDHCGL@frustum?9?$DOprojection_matrix?$FL3?$FN?$FL3?$FN@ (0000)
0029F86C 0028:
	??_C@_0CI@LBJFINLO@frustum?9?$DOprojection_matrix?$FL2?$FN?$FL3?$FN@ (0000)
0029F894 0027:
	??_C@_0CH@CHHIOMPE@frustum?9?$DOprojection_matrix?$FL1?$FN?$FL3?$FN@ (0000)
0029F8BC 0027:
	??_C@_0CH@LMNNKAJL@frustum?9?$DOprojection_matrix?$FL0?$FN?$FL3?$FN@ (0000)
0029F8E4 0027:
	??_C@_0CH@OFLFFKON@frustum?9?$DOprojection_matrix?$FL3?$FN?$FL1?$FN@ (0000)
0029F90C 0027:
	??_C@_0CH@JCCLIIBN@frustum?9?$DOprojection_matrix?$FL0?$FN?$FL1?$FN@ (0000)
0029F934 0027:
	??_C@_0CH@PCMOEOKO@frustum?9?$DOprojection_matrix?$FL3?$FN?$FL0?$FN@ (0000)
0029F95C 0027:
	??_C@_0CH@BOPFNADB@frustum?9?$DOprojection_matrix?$FL1?$FN?$FL0?$FN@ (0000)
0029F984 001a:
	??_C@_0BK@KOEOFHLL@frustum?9?$DOprojection_valid?$AA@ (0000)
0029F9A0 0017:
	??_C@_0BH@OICCFDNN@bounds?9?$DOz0?$DM?$DNbounds?9?$DOz1?$AA@ (0000)
0029F9B8 0017:
	??_C@_0BH@PLBJIHPN@bounds?9?$DOy0?$DM?$DNbounds?9?$DOy1?$AA@ (0000)
0029F9D0 0017:
	??_C@_0BH@EDCADGCC@bounds?9?$DOx0?$DM?$DNbounds?9?$DOx1?$AA@ (0000)
0029F9E8 0008:
	??_C@_07HEAHMGFP@frustum?$AA@ (0000)
0029F9F0 000d:
	??_C@_0N@GAIFNDPJ@screen_point?$AA@ (0000)
0029FA00 000b:
	??_C@_0L@JFJFDDCJ@view_point?$AA@ (0000)
0029FA0C 000c:
	??_C@_0M@PIACODLM@view_vector?$AA@ (0000)
0029FA18 000d:
	??_C@_0N@DOBKNIBB@world_vector?$AA@ (0000)
0029FA28 000c:
	??_C@_0M@IBHHCMBD@world_point?$AA@ (0000)
0029FA34 0004:
	__real@3f59999a (0000)
0029FA38 0021:
	??_C@_0CB@HKCALOIB@valid_real_vector3d?$CI?$CGresult?9?$DOup?$CJ@ (0000)
0029FA5C 0026:
	??_C@_0CG@PFGJGAON@valid_real_vector3d?$CI?$CGresult?9?$DOfor@ (0000)
0029FA84 0024:
	??_C@_0CE@KJKFBGON@valid_real_plane3d?$CI?$CGadjusted_pla@ (0000)
0029FAA8 0004:
	__real@3bc00000 (0000)
0029FAB0 0008:
	__real@3f899999a0000000 (0000)
0029FAB8 002e:
	??_C@_0CO@BEMDFKEN@valid_real_matrix4x3?$CI?$CGfrustum?9?$DOv@ (0000)
0029FAE8 002e:
	??_C@_0CO@FKHJEIFF@valid_real_matrix4x3?$CI?$CGfrustum?9?$DOw@ (0000)
0029FB18 0036:
	??_C@_0DG@PFOKCMGI@camera?9?$DOviewport_bounds?4y0?$DMcamer@ (0000)
0029FB50 0036:
	??_C@_0DG@MIEIKFFH@camera?9?$DOviewport_bounds?4x0?$DMcamer@ (0000)
0029FB88 001d:
	??_C@_0BN@MDHKMJIG@camera?9?$DOz_far?$DOcamera?9?$DOz_near?$AA@ (0000)
0029FBA8 0015:
	??_C@_0BF@GIODIKMM@camera?9?$DOz_near?$DO?$DN0?40f?$AA@ (0000)
0029FBC0 0030:
	??_C@_0DA@DCMPADCP@?$CD?$CD?$CD?5FATAL?5ERROR?3?5field?5of?5view?5s@ (0000)
0029FBF0 0033:
	??_C@_0DD@EPCFLHOD@camera?9?$DOvertical_field_of_view?$DM_@ (0000)
0029FC24 0004:
	__real@40490e38 (0000)
004B8B28 0113:
	_bss_004b8b28 (0000)
	_debug_no_frustum_clip (0111)
	_render_camera_debug_this_fucking_frustum (0112)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
