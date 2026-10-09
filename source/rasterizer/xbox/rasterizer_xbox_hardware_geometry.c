/*
RASTERIZER_XBOX_HARDWARE_GEOMETRY.C

symbols in this file:
00158410 0020:
	_IDirect3DDevice8_CreateVertexBuffer@24 (0000)
00158430 0020:
	_IDirect3DDevice8_CreateIndexBuffer@24 (0000)
00158450 0010:
	_D3DResource_MoveResourceMemory@8 (0000)
00158460 0010:
	_D3DVertexBuffer_MoveResourceMemory@8 (0000)
00158470 0010:
	_D3DVertexBuffer_Unlock@4 (0000)
00158480 0010:
	_IDirect3DVertexBuffer8_Release@4 (0000)
00158490 0020:
	_IDirect3DVertexBuffer8_Lock@20 (0000)
001584B0 0010:
	_IDirect3DVertexBuffer8_Unlock@4 (0000)
001584C0 0010:
	_D3DIndexBuffer_Lock@20 (0000)
001584D0 0010:
	_D3DIndexBuffer_Unlock@4 (0000)
001584E0 0010:
	_IDirect3DIndexBuffer8_Release@4 (0000)
001584F0 0020:
	_IDirect3DIndexBuffer8_Lock@20 (0000)
00158510 0010:
	_IDirect3DIndexBuffer8_Unlock@4 (0000)
00158520 0170:
	_rasterizer_vertex_buffer_new (0000)
00158690 0030:
	_rasterizer_vertex_buffer_delete (0000)
001586C0 0160:
	_rasterizer_triangle_buffer_new (0000)
00158820 0030:
	_rasterizer_triangle_buffer_delete (0000)
00290F84 0039:
	??_C@_0DJ@PABNNDOA@?$CD?$CD?$CD?5ERROR?5failed?5to?5create?5verte@ (0000)
00290FC0 009a:
	??_C@_0JK@INEJEDKE@IDirect3DDevice8_CreateVertexBuf@ (0000)
0029105C 002c:
	??_C@_0CM@JPLBMMAK@vertex_size?$CKcount?$DN?$DNbuffer_size?5?$HM@ (0000)
00291088 0043:
	??_C@_0ED@PHIGAHC@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
002910D0 00a5:
	??_C@_0KF@HKLHKIPE@IDirect3DDevice8_CreateIndexBuff@ (0000)
00291178 003b:
	??_C@_0DL@GGBLFLCG@?$CD?$CD?$CD?5ERROR?5failed?5to?5create?5trian@ (0000)
002911B4 000a:
	??_C@_09PNPFALML@triangles?$AA@ (0000)
002911C0 0010:
	??_C@_0BA@LCGOFHFK@triangle_buffer?$AA@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer_xbox.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
