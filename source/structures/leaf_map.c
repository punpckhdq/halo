/*
LEAF_MAP.C

symbols in this file:
00181320 0050:
	_code_00181320 (0000)
00181370 0040:
	_code_00181370 (0000)
001813B0 0040:
	_code_001813b0 (0000)
001813F0 0030:
	_leaf_map_delete (0000)
00181420 00a0:
	_code_00181420 (0000)
001814C0 0040:
	_code_001814c0 (0000)
00181500 00d0:
	_leaf_map_close_portal (0000)
001815D0 0060:
	_leaf_map_leaf_is_closed (0000)
00181630 00b0:
	_code_00181630 (0000)
001816E0 0160:
	_render_debug_leaf_portal (0000)
00181840 0060:
	_render_debug_leaf_portals (0000)
001818A0 0340:
	_code_001818a0 (0000)
00181BE0 0310:
	_leaf_map_get_leaf_bounds (0000)
00181EF0 0070:
	_leaf_map_closure (0000)
00181F60 0290:
	_leaf_map_leaf_spans_polygon (0000)
001821F0 00b0:
	_code_001821f0 (0000)
001822A0 0160:
	_render_debug_leaf_faces (0000)
00182400 0080:
	_code_00182400 (0000)
00182480 0170:
	_code_00182480 (0000)
001825F0 01e0:
	_code_001825f0 (0000)
001827D0 0260:
	_code_001827d0 (0000)
00182A30 00f0:
	_code_00182a30 (0000)
00182B20 0070:
	_code_00182b20 (0000)
00182B90 00e0:
	_code_00182b90 (0000)
00182C70 0130:
	_leaf_map_initialize_from_bsp (0000)
002A0C84 0012:
	??_C@_0BC@EGCHAPG@front?5leaf?5index?$CK?$AA@ (0000)
002A0C98 0011:
	??_C@_0BB@LINKKPJD@back?5leaf?5index?$CK?$AA@ (0000)
002A0CAC 000d:
	??_C@_0N@KPMCHLJG@plane?5index?$CK?$AA@ (0000)
002A0CBC 0019:
	??_C@_0BJ@FMOPLLFD@global_leaf_portal_block?$AA@ (0000)
002A0CD8 0019:
	??_C@_0BJ@IBJPLNPO@leaf_portal_vertex_block?$AA@ (0000)
002A0CF4 0010:
	??_C@_0BA@IDKJGEOA@portal?5indices?$CK?$AA@ (0000)
002A0D04 0007:
	??_C@_06KCOLJFCM@faces?$CK?$AA@ (0000)
002A0D0C 0016:
	??_C@_0BG@MIFDMMKE@global_map_leaf_block?$AA@ (0000)
002A0D24 000c:
	??_C@_0M@MIPIKCKP@node?5index?$CK?$AA@ (0000)
002A0D30 0014:
	??_C@_0BE@JGACHFLM@map_leaf_face_block?$AA@ (0000)
002A0D44 000e:
	??_C@_0O@GPDFDPME@portal?5index?$CK?$AA@ (0000)
002A0D54 001c:
	??_C@_0BM@OMCNINBM@map_leaf_portal_index_block?$AA@ (0000)
002A0D70 0008:
	??_C@_07FKKFMDPH@vertex?$CK?$AA@ (0000)
002A0D78 001b:
	??_C@_0BL@MDBPGNIK@map_leaf_face_vertex_block?$AA@ (0000)
002A0D94 0014:
	??_C@_0BE@CCPHNOCB@leaf_map_initialize?$AA@ (0000)
002A0DA8 003b:
	??_C@_0DL@KLHAKAHD@leaf_map_globals?4node_stack_coun@ (0000)
002A0DE4 0025:
	??_C@_0CF@JDBJPHEO@c?3?2halo?2SOURCE?2structures?2leaf_m@ (0000)
002A0E0C 0024:
	??_C@_0CE@INFHLEED@leaf_map_globals?4node_stack_coun@ (0000)
002A0E30 003c:
	??_C@_0DM@JGHBMKCF@levels_up?$DO?$DN0?5?$CG?$CG?5levels_up?$DMleaf_m@ (0000)
002A0E6C 0038:
	??_C@_0DI@KHGJOGO@portal_designator_index?$CB?$DNleaf?9?$DOp@ (0000)
002A0EA4 002e:
	??_C@_0CO@EGKEMBBF@couldn?8t?5allocate?5leaf?5map?5porta@ (0000)
002A0ED4 0022:
	??_C@_0CC@GKACLMKE@couldn?8t?5allocate?5leaf?5map?5porta@ (0000)
002A0EF8 0004:
	__real@3b23d70a (0000)
002A0EFC 001c:
	??_C@_0BM@NDHGCOBM@too?5many?5vertices?5in?5portal?$AA@ (0000)
002A0F18 0019:
	??_C@_0BJ@NHOJIIIJ@leaf_index0?$CB?$DNleaf_index1?$AA@ (0000)
002A0F34 0009:
	??_C@_08JHHHNNE@map?9?$DObsp?$AA@ (0000)
002A0F40 0026:
	??_C@_0CG@HMEMFMPL@reference_plane?9?$DOn?4n?$FLprojection?$FN@ (0000)
002A0F68 004f:
	??_C@_0EP@LAJCFBMA@ancestor_node_index?$CB?$DNNONE?5?$HM?$HM?5ind@ (0000)
002A0FB8 001d:
	??_C@_0BN@LDAFHDBA@couldn?8t?5allocate?5leaf?5face?4?$AA@ (0000)
002A0FD8 0021:
	??_C@_0CB@LPMMAOJD@couldn?8t?5allocate?5leaf?5vertices?4@ (0000)
002A0FFC 001a:
	??_C@_0BK@OEBNDAAE@result?4vertex_count?$CB?$DNNONE?$AA@ (0000)
002A1018 0023:
	??_C@_0CD@KPKMOOLF@couldn?8t?5allocate?5leaf_map?5leave@ (0000)
002A103C 0025:
	??_C@_0CF@CKHDENNA@leaf_map_globals?4node_stack_coun@ (0000)
002A1064 0004:
	??_C@_03DGBMLAJE@bsp?$AA@ (0000)
002A1068 0009:
	??_C@_08NNODDDAN@leaf_map?$AA@ (0000)
0030E970 09d0:
	_data_0030e970 (0000)
	_global_map_leaf_block (08f8)
	_global_leaf_portal_block (09a4)
004C08B0 040a:
	_bss_004c08b0 (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
