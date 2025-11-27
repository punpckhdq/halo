/*
RASTERIZER_FRAME_STATISTICS.C

symbols in this file:
0016E3A0 0040:
	_rasterizer_frame_statistics_initialize (0000)
0016E3E0 0020:
	_rasterizer_frame_statistics_begin (0000)
0016E400 0180:
	_rasterizer_frame_statistics_get_fps (0000)
0016E580 0040:
	_rasterizer_fps_accumulate (0000)
0016E5C0 0020:
	_code_0016e5c0 (0000)
0016E5E0 0040:
	_rasterizer_frame_statistics_count_static_vertices (0000)
0016E620 0130:
	_rasterizer_frame_statistics_count_dynamic_vertices (0000)
0016E750 1050:
	_rasterizer_frame_statistics_draw (0000)
0016F7A0 0010:
	_rasterizer_frame_statistics_end (0000)
0016F7B0 0020:
	_rasterizer_frame_statistics_dispose (0000)
0029DC30 000e:
	??_C@_0O@JFBHNPBL@d?3?2r?9prof?4txt?$AA@ (0000)
0029DC40 0018:
	??_C@_0BI@GCMBLDJP@?$CD?$CD?$CD?5ERROR?5out?5of?5memory?$AA@ (0000)
0029DC58 0038:
	??_C@_0DI@DAFJELOG@c?3?2halo?2SOURCE?2rasterizer?2raster@ (0000)
0029DC90 0028:
	??_C@_0CI@LGKHNHMC@rasterizer_frame_statistics_temp@ (0000)
0029DCB8 0040:
	??_C@_0EA@PGBNPAHI@triangle_count?$DMRASTERIZER_MAXIMU@ (0000)
0029DCF8 0024:
	??_C@_0CE@KDLIJAEE@average?5total?5pushbuffer?$DN?5?$CFd?5byt@ (0000)
0029DD1C 0026:
	??_C@_0CG@DFFGIJGJ@average?5total?5frame?5time?$DN?5?$CF?42f?5m@ (0000)
0029DD44 0007:
	??_C@_06NBDEAPCB@?5?5?9?9?9?9?$AA@ (0000)
0029DD4C 0006:
	??_C@_05GDJIKCCN@?$CF6?42f?$AA@ (0000)
0029DD54 0002:
	??_C@_01LFCBOECM@?4?$AA@ (0000)
0029DD58 0035:
	??_C@_0DF@GMJOLAAA@?$CD?$CD?$CD?5ERROR?5failed?5to?5open?5rasteri@ (0000)
0029DD90 0019:
	??_C@_0BJ@JJFENEFI@?$HMtsystem?5available?$HMt?$CFdKb?$AA@ (0000)
0029DDAC 0017:
	??_C@_0BH@EJIDKNJD@?$HMn?$HMtsystem?5total?$HMt?$CFdKb?$AA@ (0000)
0029DDC4 0013:
	??_C@_0BD@JJMMIDNN@?$HMn?$HMttotal?$HMt?$CFd?5?$CI?$CFd?$CJ?$AA@ (0000)
0029DDD8 0009:
	??_C@_08NACADNAD@?$HMt?$CFs?$HMt?$CFd?$AA@ (0000)
0029DDE4 0024:
	??_C@_0CE@GPLJMHJA@vertex?5shaders?$HMt?$HO35k?5last?5i?5chec@ (0000)
0029DE08 0010:
	??_C@_0BA@PIMAGLFH@debug?5geometry?$CK?$AA@ (0000)
0029DE18 0017:
	??_C@_0BH@OHBLOMMO@motion?5sensor?5buffers?$CK?$AA@ (0000)
0029DE30 000e:
	??_C@_0O@LPNPGMEL@water?5buffers?$AA@ (0000)
0029DE40 0012:
	??_C@_0BC@MBLMGAF@sun?5glow?5buffers?$CK?$AA@ (0000)
0029DE54 000f:
	??_C@_0P@JGCOFABI@shadow?5buffers?$AA@ (0000)
0029DE64 0024:
	??_C@_0CE@GGJOAJBB@mirror?5buffers?5?$CIincludes?5z?9buffe@ (0000)
0029DE88 0011:
	??_C@_0BB@IAJCMCIG@bump?5map?5palette?$AA@ (0000)
0029DE9C 001c:
	??_C@_0BM@KDCCBLNC@transparent?5geometry?5groups?$AA@ (0000)
0029DEB8 0012:
	??_C@_0BC@BJBCACME@dynamic?5triangles?$AA@ (0000)
0029DECC 0022:
	??_C@_0CC@LDJJCNJE@dynamic?5vertices?5?$CIdetail?5objects@ (0000)
0029DEF0 0019:
	??_C@_0BJ@MNAIOGBM@dynamic?5vertices?5?$CImodel?$CJ?$AA@ (0000)
0029DF0C 001a:
	??_C@_0BK@CDEJLHGG@dynamic?5vertices?5?$CIscreen?$CJ?$AA@ (0000)
0029DF28 0018:
	??_C@_0BI@HPJPBHLO@dynamic?5vertices?5?$CIlit?$CK?$CJ?$AA@ (0000)
0029DF40 0019:
	??_C@_0BJ@DKPGOBKB@dynamic?5vertices?5?$CIunlit?$CJ?$AA@ (0000)
0029DF5C 000c:
	??_C@_0M@DEOKAGLG@memory?5pool?$AA@ (0000)
0029DF68 0023:
	??_C@_0CD@ELLHHMII@?$HMtallocation?$HMtmemory?5usage?5?$CIbyte@ (0000)
0029DF8C 0014:
	??_C@_0BE@ELPADKKO@?$HMttotal?$HMt?$CF?42f?$HMt?$CFd?$HMn?$AA@ (0000)
0029DFA0 000e:
	??_C@_0O@NBNGOGEK@?$HMt?$CFs?$HMt?9?9?9?9?$HMt0?$AA@ (0000)
0029DFB0 000f:
	??_C@_0P@EMPKACCD@?$HMt?$CFs?$HMt?$CF?42f?$HMt?$CFd?$AA@ (0000)
0029DFC0 002a:
	??_C@_0CK@PDOKMEKL@?$HMtGPU?5profile?$HMttime?5?$CImsecs?$CJ?$HMtdat@ (0000)
0029DFEC 0028:
	??_C@_0CI@OJALLPNK@?$HMt?$CFd?5dynamic?5lights?$HMn?$HMt?$CFd?5lens?5f@ (0000)
0029E014 0023:
	??_C@_0CD@DPCNBBDK@?$HMtdynamic?5geometry?$HMt?$CFd?1?$CFd?$HMt?$CFd?1?$CFd@ (0000)
0029E038 0017:
	??_C@_0BH@JLJHFKOG@?$HMtdecals?$HMt?$CFd?$HMt?$CFd?$HMt?$CFd?$HMn?$AA@ (0000)
0029E050 0034:
	??_C@_0DE@DDLABOMO@?$HMtsolid?$HMt?$CFd?$HMt?$CFd?$HMt?$CFd?$HMn?$HMttranspare@ (0000)
0029E084 001a:
	??_C@_0BK@IOKCDMIA@?$HMtmodels?5?$CI?$CFd?$CJ?$HMt?$CFd?$HMt?$CFd?$HMt?$CFd?$AA@ (0000)
0029E0A0 0021:
	??_C@_0CB@EOMGBMNP@?$HMtmodel?5shadows?5?$CI?$CFd?$CJ?$HMt?$CFd?$HMt?$CFd?$HMt?$CFd@ (0000)
0029E0C8 0113:
	??_C@_0BBD@GCJAAPME@?$HMtlightmaps?$HMt?$CFd?$HMt?$CFd?$HMt?$CFd?$HMn?$HMtshado@ (0000)
0029E1DC 001a:
	??_C@_0BK@KOANHEHC@?$HMtenvironment?$HMt?$CFd?$HMt?$CFd?$HMt?$CFd?$AA@ (0000)
0029E1F8 0016:
	??_C@_0BG@GAEDMNLC@?$HMttotal?$HMt?$CFd?$HMt?$CFd?$HMt?$CFd?$HMn?$AA@ (0000)
0029E210 0024:
	??_C@_0CE@EIBEMOMJ@?$HMt?$HMtvertices?$HMttriangles?$HMtprimiti@ (0000)
0029E234 0038:
	??_C@_0DI@JADDMGFO@?$HMtlocal_player_count?$HMt?$CFd?$HMn?$HMtmain@ (0000)
0029E26C 0037:
	??_C@_0DH@JFMJJCIO@?$HMtskinning?$HMt?$CFd?$HMn?$HMtlighting?$HMt?$CFd?$HMn@ (0000)
0029E2A4 0038:
	??_C@_0DI@GOEPEHGF@?$HMtfogged?$HMt?$CFd?$HMn?$HMtnormal?$HMt?$CFd?$HMn?$HMtfa@ (0000)
0029E2DC 001b:
	??_C@_0BL@IBKJHKIF@?$HMt?$CF?40f?$HMt?$CF?40f?$HMt?$CF?40f?$HMt?$CF?40f?$HMn?$AA@ (0000)
0029E2F8 0020:
	??_C@_0CA@DPIIOMPF@?$HMt?$CF?40f?$HMt?$CF?40f?1?$CF?40f?$HMt?$CF?40f?$HMt?$CF?40f?$HMn?$AA@ (0000)
0029E318 0029:
	??_C@_0CJ@FFAAFLA@?$HMn?$HMtframerate?$HMtaverage?5?$CIof?5?$CFd?$CJ?$HMt@ (0000)
0030D4CC 0004:
	_profile_log_path (0000)
00466320 0860:
	_bss_00466320 (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
