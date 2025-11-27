/*
LRUV_CACHE.C

symbols in this file:
0010CCA0 0020:
	_lruv_allocation_size (0000)
0010CCC0 0050:
	_lruv_update_function_pointers (0000)
0010CD10 0040:
	_lruv_has_locked_proc (0000)
0010CD50 0020:
	_code_0010cd50 (0000)
0010CD70 0230:
	_code_0010cd70 (0000)
0010CFA0 0110:
	_lruv_initialize (0000)
0010D0B0 0040:
	_lruv_delete (0000)
0010D0F0 0020:
	_lruv_idle (0000)
0010D110 00e0:
	_lruv_block_delete (0000)
0010D1F0 0030:
	_lruv_block_touch (0000)
0010D220 0030:
	_lruv_block_get_address (0000)
0010D250 0030:
	_lruv_block_touched (0000)
0010D280 00a0:
	_lruv_cache_get_page_usage (0000)
0010D320 0090:
	_lruv_resize (0000)
0010D3B0 01d0:
	_lruv_debug_to_file (0000)
0010D580 0060:
	_lruv_new (0000)
0010D5E0 0050:
	_lruv_flush (0000)
0010D630 0520:
	_lruv_block_new (0000)
0027DFC0 0023:
	??_C@_0CD@NCBKDBJP@c?3?2halo?2SOURCE?2memory?2lruv_cache@ (0000)
0027DFE8 0048:
	??_C@_0EI@HEIFIJJL@block?9?$DOfirst_page_index?$CLblock?9?$DOp@ (0000)
0027E030 0035:
	??_C@_0DF@MDNEJLAH@next_block?9?$DOfirst_page_index?$DOblo@ (0000)
0027E068 002e:
	??_C@_0CO@LEEGCGLF@next_block?9?$DOprevious_block_index@ (0000)
0027E098 0025:
	??_C@_0CF@NBCPOKGO@cache?9?$DOlast_block_index?$DN?$DNblock_i@ (0000)
0027E0C0 0055:
	??_C@_0FF@HOIJOPMD@previous_block?9?$DOfirst_page_index@ (0000)
0027E118 0039:
	??_C@_0DJ@EDMBFAJL@previous_block?9?$DOfirst_page_index@ (0000)
0027E154 002e:
	??_C@_0CO@DBDNOGDK@previous_block?9?$DOnext_block_index@ (0000)
0027E184 0026:
	??_C@_0CG@BJMLDEPL@cache?9?$DOfirst_block_index?$DN?$DNblock_@ (0000)
0027E1AC 0027:
	??_C@_0CH@NAJJPKBF@cache?9?$DOsignature?$DN?$DNLRUV_CACHE_SIG@ (0000)
0027E1D4 0016:
	??_C@_0BG@DHPHNPNL@maximum_block_count?$DO0?$AA@ (0000)
0027E1EC 002e:
	??_C@_0CO@MICADOHD@page_size_bits?$DO0?5?$CG?$CG?5page_size_bi@ (0000)
0027E21C 000d:
	??_C@_0N@FBNLCABO@page_count?$DO0?$AA@ (0000)
0027E22C 0011:
	??_C@_0BB@KFLMCPF@new_page_count?$DO0?$AA@ (0000)
0027E240 0010:
	??_C@_0BA@DBGNJGAO@?$CFs?5?$CF?55d?$CF?55d?5?$CFs?6?$AA@ (0000)
0027E250 0002:
	??_C@_01OCOKONAJ@L?$AA@ (0000)
0027E258 006f:
	??_C@_0GP@KEGBJJID@?6?$CD?$CFd?5pages?0?5each?5?$CD?$CFd?5bytes?6?$CD?$CFd?5b@ (0000)
0027E2C8 0031:
	??_C@_0DB@MNKBOABM@?$CFs?5?$CIv1?3?5only?5blocks?5used?5this?5fr@ (0000)
0027E2FC 0003:
	??_C@_02CHHAKFFB@w?$CL?$AA@ (0000)
0027E300 0027:
	??_C@_0CH@CKDODDDO@next_block?9?$DOprevious_block_index@ (0000)
0027E328 001e:
	??_C@_0BO@JHCOBGKL@cache?9?$DOlast_block_index?$DN?$DNNONE?$AA@ (0000)
0027E348 0054:
	??_C@_0FE@PMNMONNB@?$CBcache?9?$DOlocked_block_proc?5?$HM?$HM?5?$CBca@ (0000)
0027E39C 0039:
	??_C@_0DJ@OEGLJEMH@lruv_cache_block_get?$CIcache?0?5olde@ (0000)
0027E3D8 0047:
	??_C@_0EH@JOOPPGKG@?$CBcache?9?$DOlocked_block_proc?5?$HM?$HM?5?$CBca@ (0000)
0027E420 001c:
	??_C@_0BM@BFNMIBPB@hole_read_index?$DN?$DNhole_index?$AA@ (0000)
0027E43C 0015:
	??_C@_0BF@OEOIEJGL@desired_page_count?$DO0?$AA@ (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
