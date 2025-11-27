/*
RASTERIZER_XBOX_TRANSPARENT_GEOMETRY.C

symbols in this file:
00163B40 01b0:
	_code_00163b40 (0000)
00163CF0 0050:
	_code_00163cf0 (0000)
00163D40 0020:
	_code_00163d40 (0000)
00163D60 0180:
	_rasterizer_transparent_geometry_group_draw__internal (0000)
00163EE0 0220:
	_code_00163ee0 (0000)
00164100 0060:
	_code_00164100 (0000)
00164160 0010:
	_code_00164160 (0000)
00164170 0010:
	_code_00164170 (0000)
00164180 0010:
	_code_00164180 (0000)
00164190 0010:
	_code_00164190 (0000)
001641A0 0010:
	_code_001641a0 (0000)
001641B0 0010:
	_code_001641b0 (0000)
001641C0 0010:
	_code_001641c0 (0000)
001641D0 0020:
	_code_001641d0 (0000)
001641F0 0010:
	_code_001641f0 (0000)
00164200 01b0:
	_rasterizer_transparent_geometry_groups_end (0000)
001643B0 0030:
	_subtract_vectors4d (0000)
001643E0 0040:
	_offset_vector4d (0000)
00164420 00f0:
	_rasterizer_transparent_geometry_initialize_aux_buffer (0000)
00164510 0020:
	_rasterizer_transparent_geometry_dispose_aux_buffer (0000)
00164530 0030:
	_rasterizer_transparent_geometry_groups_begin (0000)
00164560 3b10:
	_rasterizer_transparent_geometry_group_draw (0000)
00292CF0 0030:
	??_C@_0DA@COBMAJBI@group?9?$DOtriangle_count?$DN?$DNvertices_@ (0000)
00292D20 000e:
	??_C@_0O@MKIGFILJ@?$CBhas_lightmap?$AA@ (0000)
00292D30 0046:
	??_C@_0EG@MGBFGMKK@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
00292D78 003e:
	??_C@_0DO@HNCJICHK@?$CD?$CD?$CD?5ERROR?5rasterizer_transparent@ (0000)
00292DB8 0006:
	??_C@_05IDMOFOMC@?$CF?402f?$AA@ (0000)
00292DC0 0003:
	??_C@_02FLHDKHAB@hr?$AA@ (0000)
00292DC4 003d:
	??_C@_0DN@KFMAMEG@IDirect3DDevice8_EndVisibilityTe@ (0000)
00292E04 002d:
	??_C@_0CN@MKPFLBON@?$CD?$CD?$CD?5ERROR?5failed?5to?5allocate?5tex@ (0000)
00292E38 00bb:
	??_C@_0LL@MPPGICGO@IDirect3DVertexBuffer8_Lock?$CIrast@ (0000)
00292EF8 00f7:
	??_C@_0PH@NENLLNM@IDirect3DDevice8_CreateVertexBuf@ (0000)
00292FF0 003c:
	??_C@_0DM@KCHMKPAC@?$CD?$CD?$CD?5ERROR?5rasterizer_transparent@ (0000)
0029302C 0007:
	??_C@_06GACNCAPN@?$CBdirty?$AA@ (0000)
00293034 0022:
	??_C@_0CC@BMOKPLKC@?$CD?$CD?$CD?5ERROR?5unsupported?5shader?5typ@ (0000)
00293058 005a:
	??_C@_0FK@JJJJNNKB@reflection_type?$DO?$DN0?5?$CG?$CG?5reflection@ (0000)
002930B4 003b:
	??_C@_0DL@EGPBHLCN@type?$DO?$DN0?5?$CG?$CG?5type?$DMNUMBER_OF_SHADER@ (0000)
002930F0 00c2:
	??_C@_0MC@HFAHGKIF@?$CBTEST_FLAG?$CIshader_transparent_ch@ (0000)
002931B4 003b:
	??_C@_0DL@BJELFOGH@constant_color0?4blue?5?$DO?$DN0?40f?5?$CG?$CG?5c@ (0000)
002931F0 003b:
	??_C@_0DL@HNJBBLJI@constant_color0?4green?$DO?$DN0?40f?5?$CG?$CG?5c@ (0000)
0029322C 0039:
	??_C@_0DJ@EMPIAIDC@constant_color0?4red?5?$DO?$DN0?40f?5?$CG?$CG?5co@ (0000)
00293268 002e:
	??_C@_0CO@NNMDDBLI@stage?9?$DOconstant_color0_animation@ (0000)
00293298 002c:
	??_C@_0CM@NLPJEHHH@?$CD?$CD?$CD?5ERROR?5unsupported?5framebuffe@ (0000)
002932C8 0094:
	??_C@_0JE@HDBOEKDA@IDirect3DDevice8_SetVertexShader@ (0000)
00293360 0091:
	??_C@_0JB@BKNCAMGA@IDirect3DDevice8_SetVertexShader@ (0000)
002933F4 003b:
	??_C@_0DL@NHKIEDLC@type?$DO?$DN0?5?$CG?$CG?5type?$DMNUMBER_OF_SHADER@ (0000)
00293430 00c2:
	??_C@_0MC@PDFLODDB@?$CBTEST_FLAG?$CIshader_transparent_ge@ (0000)
002934F8 007d:
	??_C@_0HN@NGBJCHJM@IDirect3DDevice8_SetStreamSource@ (0000)
00293578 0091:
	??_C@_0JB@KCBKPHDD@IDirect3DDevice8_SetVertexShader@ (0000)
0029360C 0004:
	__real@4b7fffff (0000)
00293610 0004:
	__real@7149f2ca (0000)
00293618 0076:
	??_C@_0HG@HBDJNJMC@IDirect3DDevice8_SetVertexShader@ (0000)
00293690 0091:
	??_C@_0JB@JFCPBDMI@IDirect3DDevice8_SetVertexShader@ (0000)
00293724 003d:
	??_C@_0DN@FLJIIOKP@?$CD?$CD?$CD?5ERROR?5unsupported?5model?5effe@ (0000)
00293764 0006:
	??_C@_05ICAMDEPF@?$CF?403f?$AA@ (0000)
0029376C 0027:
	??_C@_0CH@EGNMKNCN@color?4blue?5?$DO?$DN0?40f?5?$CG?$CG?5color?4blue?5@ (0000)
00293794 0027:
	??_C@_0CH@MEAOFMIG@color?4green?$DO?$DN0?40f?5?$CG?$CG?5color?4green@ (0000)
002937BC 0025:
	??_C@_0CF@PKCJOKKO@color?4red?5?$DO?$DN0?40f?5?$CG?$CG?5color?4red?5?$DM?$DN@ (0000)
002937E4 0004:
	__real@3e3851ec (0000)
002937E8 0027:
	??_C@_0CH@FDCPMIOG@vertex_shader_table?$FLvertex_type?$FN@ (0000)
00293810 0040:
	??_C@_0EA@EGBNPDLH@vertex_type?$DO?$DN0?5?$CG?$CG?5vertex_type?$DMNU@ (0000)
004662D8 0010:
	_bss_004662d8 (0000)
*/

/* ---------- headers */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
