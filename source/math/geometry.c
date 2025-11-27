/*
GEOMETRY.C

symbols in this file:
000F4F00 00b0:
	_geosphere_dispose (0000)
000F4FB0 0010:
	_convex_hull3d_verify (0000)
000F4FC0 0020:
	_plane2d_distance_to_point (0000)
000F4FE0 0030:
	_vector_intersect_plane2d (0000)
000F5010 0040:
	_vector_intersect_plane3d (0000)
000F5050 0150:
	_code_000f5050 (0000)
000F51A0 0300:
	_build_torus (0000)
000F54A0 00a0:
	_code_000f54a0 (0000)
000F5540 0310:
	_convex_hull2d (0000)
000F5850 0100:
	_convex_hull2d_verify (0000)
000F5950 00d0:
	_convex_hull2d_test_circle (0000)
000F5A20 0090:
	_convex_hull2d_test_point (0000)
000F5AB0 00a0:
	_convex_hull2d_test_point_indexed (0000)
000F5B50 0060:
	_convex_hull2d_area (0000)
000F5BB0 0060:
	_convex_hull2d_perimeter (0000)
000F5C10 0120:
	_convex_hull2d_test_vector (0000)
000F5D30 03f0:
	_convex_polygon2d_clip_to_plane (0000)
000F6120 0060:
	_convex_polygon2d_verify (0000)
000F6180 0460:
	_convex_polygon3d_clip_to_plane (0000)
000F65E0 0190:
	_convex_polygon3d_verify (0000)
000F6770 05d0:
	_convex_hull3d_begin (0000)
000F6D40 0710:
	_convex_hull3d_expand (0000)
000F7450 0110:
	_convex_hull3d (0000)
000F7560 0070:
	_convex_hull3d_test_point (0000)
000F75D0 0110:
	_convex_hull3d_test_vector (0000)
000F76E0 01a0:
	_code_000f76e0 (0000)
000F7880 0210:
	_convex_hull2d_intersect (0000)
000F7A90 0190:
	_code_000f7a90 (0000)
000F7C20 03b0:
	_code_000f7c20 (0000)
000F7FD0 01f0:
	_geosphere_new (0000)
0027A2B8 0078:
	_rdata_0027a2b8 (0000)
0027A330 0026:
	??_C@_0CG@DDHFMONE@sphere?9?$DOtriangle_strip_vertex_in@ (0000)
0027A358 0011:
	??_C@_0BB@NLDBGHK@sphere?9?$DOvertices?$AA@ (0000)
0027A36C 0007:
	??_C@_06FOHEPPJH@sphere?$AA@ (0000)
0027A374 001f:
	??_C@_0BP@BJAGONOE@c?3?2halo?2SOURCE?2math?2geometry?4c?$AA@ (0000)
0027A394 0035:
	??_C@_0DF@KKNLNKMO@new_vertex?5?$DO?$DN0?5?$CG?$CG?5new_vertex?5?$DM?$DN?5@ (0000)
0027A3CC 002f:
	??_C@_0CP@CNADNFO@parent2?5?$DO?$DN0?5?$CG?$CG?5parent2?5?$DM?$DN?5sphere@ (0000)
0027A3FC 002f:
	??_C@_0CP@BDMOGHAE@parent1?5?$DO?$DN0?5?$CG?$CG?5parent1?5?$DM?$DN?5sphere@ (0000)
0027A42C 003f:
	??_C@_0DP@FFEHBALB@subdivision_index?5?$DO?50?5?$CG?$CG?5subdivi@ (0000)
0027A46C 0019:
	??_C@_0BJ@ENKKOHHL@cylinder_segment_count?$DO2?$AA@ (0000)
0027A488 0015:
	??_C@_0BF@GOKGNGOE@ring_segment_count?$DO2?$AA@ (0000)
0027A4A0 0015:
	??_C@_0BF@HMLCHDNC@start_vertex_index?$DO0?$AA@ (0000)
0027A4B8 002d:
	??_C@_0CN@HIIJMMI@start_vertex_index?$CLvertex_index?$DM@ (0000)
0027A4E8 001a:
	??_C@_0BK@MBDLNMGM@vertex_index?$DMvertex_count?$AA@ (0000)
0027A504 0021:
	??_C@_0CB@IMGLMJJD@count?$DO?$DN0?5?$CG?$CG?5count?$DM?$DNmaximum_count@ (0000)
0027A528 0018:
	??_C@_0BI@ONCCMPIK@count?$DM?$DNCLIP_BUFFER_SIZE?$AA@ (0000)
0027A540 0027:
	??_C@_0CH@GFCOCAKD@count?$DO?$DNNUMBER_OF_VERTICES_PER_TR@ (0000)
0027A568 0004:
	__real@b58637bd (0000)
0027A56C 0009:
	??_C@_08FGPNIDBO@surfaces?$AA@ (0000)
0027A578 0006:
	??_C@_05JGEJPNHM@edges?$AA@ (0000)
0027A580 0009:
	??_C@_08BOLPBPCL@vertices?$AA@ (0000)
0027A58C 003c:
	??_C@_0DM@LMGOJFMO@vertex2?9?$DOpoint_index?$DO?$DN0?5?$CG?$CG?5verte@ (0000)
0027A5C8 0010:
	??_C@_0BA@FMIEEKIK@vertex2?9?$DOextant?$AA@ (0000)
0027A5D8 003c:
	??_C@_0DM@NBEECNH@vertex1?9?$DOpoint_index?$DO?$DN0?5?$CG?$CG?5verte@ (0000)
0027A614 0010:
	??_C@_0BA@LHLDPBIJ@vertex1?9?$DOextant?$AA@ (0000)
0027A628 0043:
	??_C@_0ED@EPDBMIEH@?$CIedge?9?$DOsurface_indices?$FL0?$FN?$DN?$DNNONE?$CJ@ (0000)
0027A670 0043:
	??_C@_0ED@EMHNLENG@edge?9?$DOvertex_indices?$FL1?$FN?$DO?$DN0?5?$CG?$CG?5ed@ (0000)
0027A6B8 0043:
	??_C@_0ED@JNNMLONE@edge?9?$DOvertex_indices?$FL0?$FN?$DO?$DN0?5?$CG?$CG?5ed@ (0000)
0027A6FC 000d:
	??_C@_0N@NNPGIPCM@edge?9?$DOextant?$AA@ (0000)
0027A710 004f:
	??_C@_0EP@NMDNFEOD@edge?9?$DOvertex_indices?$FL0?$FN?$DN?$DNvertex_@ (0000)
0027A760 0027:
	??_C@_0CH@PNIAGAON@edge_index?$DO?$DN0?5?$CG?$CG?5edge_index?$DMedge@ (0000)
0027A788 0020:
	??_C@_0CA@OJBIMDIM@first_boundary_edge_index?$CB?$DNNONE?$AA@ (0000)
0027A7A8 0009:
	??_C@_08NONFIEMK@vb?5?$CB?$DN?5va?$AA@ (0000)
0027A7B4 0025:
	??_C@_0CF@PLABFBEI@vb?5?$DO?$DN?50?5?$CG?$CG?5vb?5?$DM?5sphere?9?$DOvertex_c@ (0000)
0027A7DC 0025:
	??_C@_0CF@KKAMJMOF@va?5?$DO?$DN?50?5?$CG?$CG?5va?5?$DM?5sphere?9?$DOvertex_c@ (0000)
0027A804 002f:
	??_C@_0CP@IEJEPOMO@result_count?$DO?$DN0?5?$CG?$CG?5result_count?$DM@ (0000)
0027A834 0017:
	??_C@_0BH@GHJAIPGF@p?$CB?$DNresult?5?$CG?$CG?5q?$CB?$DNresult?$AA@ (0000)
0027A84C 0008:
	??_C@_07OPEDLPNI@q_count?$AA@ (0000)
0027A854 0002:
	??_C@_01IIACKFLH@q?$AA@ (0000)
0027A858 0008:
	??_C@_07CDOJLPEG@p_count?$AA@ (0000)
0027A860 0020:
	??_C@_0CA@GNDAHJOD@maximum_count?$DM?$DNCLIP_BUFFER_SIZE?$AA@ (0000)
0027A880 0025:
	??_C@_0CF@POILEFLF@v3?5?$DO?$DN0?5?$CG?$CG?5v3?5?$DM?$DN?5sphere?9?$DOvertex_c@ (0000)
0027A8A8 0025:
	??_C@_0CF@IMOGDHLH@v2?5?$DO?$DN0?5?$CG?$CG?5v2?5?$DM?$DN?5sphere?9?$DOvertex_c@ (0000)
0027A8D0 0025:
	??_C@_0CF@BKFBKBLB@v1?5?$DO?$DN0?5?$CG?$CG?5v1?5?$DM?$DN?5sphere?9?$DOvertex_c@ (0000)
0027A8F8 0040:
	??_C@_0EA@DLEJKPCO@topright_vertex?5?$DO?$DN?50?5?$CG?$CG?5topright@ (0000)
0027A938 003a:
	??_C@_0DK@CHGNILGK@right_vertex?5?$DO?$DN?50?5?$CG?$CG?5right_verte@ (0000)
0027A974 0038:
	??_C@_0DI@CELCELPM@left_vertex?5?$DO?$DN?50?5?$CG?$CG?5left_vertex?5@ (0000)
0027A9AC 0036:
	??_C@_0DG@HPBCBMGH@top_vertex?5?$DO?$DN?50?5?$CG?$CG?5top_vertex?5?$DM?$DN@ (0000)
0027A9E8 0066:
	??_C@_0GG@CNBBDAKE@?$CKtriangle_strip_vertex_indices_i@ (0000)
0027AA50 0025:
	??_C@_0CF@PKGGEOOK@v3?5?$DO?$DN?50?5?$CG?$CG?5v3?5?$DM?5sphere?9?$DOvertex_c@ (0000)
0027AA78 0025:
	??_C@_0CF@MKJNAKHB@v2?5?$DO?$DN?50?5?$CG?$CG?5v2?5?$DM?5sphere?9?$DOvertex_c@ (0000)
0027AAA0 0025:
	??_C@_0CF@JLJAMHNM@v1?5?$DO?$DN?50?5?$CG?$CG?5v1?5?$DM?5sphere?9?$DOvertex_c@ (0000)
0027AAC8 001b:
	??_C@_0BL@OKBFAGJI@vertex_subdivision_indices?$AA@ (0000)
0027AAE4 0024:
	??_C@_0CE@LEDAAIBG@triangle_strip_vertex_indices_in@ (0000)
0027AB08 000d:
	??_C@_0N@KLBMBFKD@vertex_index?$AA@ (0000)
0027AB18 0025:
	??_C@_0CF@JFAFJJEF@vertex_index?5?$DN?$DN?5result?9?$DOvertex_c@ (0000)
0027AB40 0065:
	??_C@_0GF@PMJOGKBA@triangle_strip_vertex_indices_in@ (0000)
003078C4 0008:
	_global_convex_hull3d_delta (0000)
	_global_convex_hull3d_epsilon (0004)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
