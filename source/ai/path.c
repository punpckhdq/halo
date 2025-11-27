/*
PATH.C

symbols in this file:
0004C5F0 0010:
	_paths_initialize (0000)
0004C600 0010:
	_paths_dispose (0000)
0004C610 0010:
	_paths_initialize_for_new_map (0000)
0004C620 0010:
	_paths_dispose_from_old_map (0000)
0004C630 0030:
	_path_input_new (0000)
0004C660 0010:
	_path_input_set_target_object (0000)
0004C670 0030:
	_path_input_set_start (0000)
0004C6A0 0040:
	_path_input_set_attractor (0000)
0004C6E0 0020:
	_path_input_set_search_bounds (0000)
0004C700 0040:
	_path_state_new (0000)
0004C740 0030:
	_path_state_destination (0000)
0004C770 0040:
	_code_0004c770 (0000)
0004C7B0 0010:
	_code_0004c7b0 (0000)
0004C7C0 01e0:
	_code_0004c7c0 (0000)
0004C9A0 0230:
	_code_0004c9a0 (0000)
0004CBD0 0120:
	_code_0004cbd0 (0000)
0004CCF0 0080:
	_code_0004ccf0 (0000)
0004CD70 0060:
	_code_0004cd70 (0000)
0004CDD0 0080:
	_path_get_node (0000)
0004CE50 0050:
	_path_node_from_hash_table (0000)
0004CEA0 00f0:
	_path_3d_available (0000)
0004CF90 0090:
	_path_3d_build_path (0000)
0004D020 0130:
	_path_state_approach_point (0000)
0004D150 04a0:
	_path_state_build_path (0000)
0004D5F0 0250:
	_code_0004d5f0 (0000)
0004D840 0070:
	_code_0004d840 (0000)
0004D8B0 0180:
	_code_0004d8b0 (0000)
0004DA30 00d0:
	_closest_point_to_attractor (0000)
0004DB00 00c0:
	_path_attractor_weight (0000)
0004DBC0 01f0:
	_path_state_estimated_distance (0000)
0004DDB0 0830:
	_code_0004ddb0 (0000)
0004E5E0 0100:
	_path_state_find (0000)
0024DAB8 0054:
	??_C@_0FE@EPMGNKHK@state?9?$DOnode_list?$FLparent_node_ind@ (0000)
0024DB10 0045:
	??_C@_0EF@LADBNEPJ@state?9?$DOnode_list?$FLparent_node_ind@ (0000)
0024DB58 0046:
	??_C@_0EG@NGIKIECC@?$CIparent_node_index?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIpar@ (0000)
0024DBA0 0042:
	??_C@_0EC@FLANNGD@state?9?$DOnode_list?$FLnode_index?$FN?4qua@ (0000)
0024DBE4 0038:
	??_C@_0DI@LCDBFPAM@?$CInode_index?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CInode_index@ (0000)
0024DC1C 003f:
	??_C@_0DP@ONCFDIOO@?$CIheap_location?5?$DO?$DN?51?$CJ?5?$CG?$CG?5?$CIheap_lo@ (0000)
0024DC5C 0019:
	??_C@_0BJ@IILBBPFP@c?3?2halo?2SOURCE?2ai?2path?4c?$AA@ (0000)
0024DC78 0049:
	??_C@_0EJ@NPIFJABE@state?9?$DOnode_list?$FLchild_node_inde@ (0000)
0024DCC8 0048:
	??_C@_0EI@JGENJHLI@state?9?$DOnode_list?$FLchild_node_inde@ (0000)
0024DD10 0044:
	??_C@_0EE@LMKECELL@?$CIchild_node_index?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIchil@ (0000)
0024DD58 005f:
	??_C@_0FP@HHDGGKGA@state?9?$DOnode_list?$FLnode_index?$FN?4qua@ (0000)
0024DDB8 0030:
	??_C@_0DA@HHBOKDMG@state?9?$DOnode_list?$FLnode_index?$FN?4hea@ (0000)
0024DDE8 0017:
	??_C@_0BH@BOHECKL@state?9?$DOheap_count?5?$DO?$DN?51?$AA@ (0000)
0024DE00 002e:
	??_C@_0CO@NBKNGICB@path_heap_insert?3?5overflowed?5sta@ (0000)
0024DE30 0036:
	??_C@_0DG@MBNMAAEM@?$CInode_index?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CInode_index@ (0000)
0024DE68 0013:
	??_C@_0BD@MBAKELBN@node_index?5?$CB?$DN?5NONE?$AA@ (0000)
0024DE7C 0018:
	??_C@_0BI@PECADNDG@straight_line_reference?$AA@ (0000)
0024DE94 0019:
	??_C@_0BJ@GLKEJCKE@approach_point_reference?$AA@ (0000)
0024DEB0 003b:
	??_C@_0DL@MMNOICOM@state?9?$DOdebug?9?$DOpath_build_result?5@ (0000)
0024DEEC 0017:
	??_C@_0BH@ICOJEHHO@child_node?9?$DOdepth?5?$DN?$DN?50?$AA@ (0000)
0024DF04 0019:
	??_C@_0BJ@CFIDHICE@child_node_index?5?$CB?$DN?5NONE?$AA@ (0000)
0024DF20 0025:
	??_C@_0CF@EODIHIDD@node?9?$DOdepth?5?$DN?$DN?5child_node?9?$DOdepth@ (0000)
0024DF48 0035:
	??_C@_0DF@MJJJFNJI@?$CInode?9?$DOdepth?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CInode?9?$DOdep@ (0000)
0024DF80 005a:
	??_C@_0FK@GBFFKFKH@?$CIinitial_node?9?$DOsurface_index?5?$DO?$DN?5@ (0000)
0024DFDC 0017:
	??_C@_0BH@EGEJGGGM@state?9?$DOnode_count?5?$DN?$DN?50?$AA@ (0000)
0024DFF8 0078:
	??_C@_0HI@IDMLJPKG@pathfinding?3?5attempted?5to?5build?5@ (0000)
0024E070 0064:
	??_C@_0GE@LNDFFALH@?$CIstate?9?$DOinput?4start_surface_inde@ (0000)
0024E0D8 005c:
	??_C@_0FM@NOKAIMOC@?$CIedge?9?$DOadjacent_surface_index?5?$DO?$DN@ (0000)
0024E134 003e:
	??_C@_0DO@PCGCNINJ@?$CIsurface_index?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIsurface@ (0000)
0024E174 0013:
	??_C@_0BD@GFDHLODO@distance_reference?$AA@ (0000)
0024E188 0058:
	??_C@_0FI@OHAECMC@quantized_cost_estimate?5?$DM?$DN?5state@ (0000)
0024E1E0 0050:
	??_C@_0FA@CJCEFIEA@?$CInew_node?9?$DOheap_location?5?$DO?$DN?51?$CJ?5?$CG@ (0000)
0024E230 0054:
	??_C@_0FE@CHKDPLDB@state?9?$DOheap?$FLheap_location?$FN?4quant@ (0000)
0024E284 0034:
	??_C@_0DE@MIONFOJA@state?9?$DOheap?$FLheap_location?$FN?4node_@ (0000)
0024E2B8 003c:
	??_C@_0DM@DFJMOLG@?$CIheap_location?5?$DO?$DN?51?$CJ?5?$CG?$CG?5?$CIheap_lo@ (0000)
0024E2F4 003c:
	??_C@_0DM@PHFOIMMP@path_state_traverse?3?5found?5a?5?8be@ (0000)
0024E330 0052:
	??_C@_0FC@GGCMLBII@?$CInew_node?9?$DOsurface_index?5?$DO?$DN?50?$CJ?5?$CG@ (0000)
0024E388 004e:
	??_C@_0EO@KKHPGEKA@path_state_traverse?3?5cost?5?$CF?41f?5e@ (0000)
0024E3D8 005c:
	??_C@_0FM@KGGDAHPH@total_cost_estimate?5?9?5cheapest_n@ (0000)
0024E434 0004:
	__real@bdcccccd (0000)
0024E438 005c:
	??_C@_0FM@PLCDNLNA@?$CIcheapest_node?9?$DOsurface_index?5?$DO?$DN@ (0000)
0024E498 0041:
	??_C@_0EB@OLCMDDAG@state?9?$DOdebug?9?$DOpath_traverse_resu@ (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
