/*
PROGRESS_BAR.C

symbols in this file:
000D0D10 01b0:
	_D3DDevice_SetRenderState (0000)
000D0EC0 0050:
	_D3DDevice_SetTextureStageState (0000)
000D0F10 0010:
	_IDirect3DDevice8_GetBackBuffer@16 (0000)
000D0F20 0020:
	_IDirect3DDevice8_CreateTexture@32 (0000)
000D0F40 0010:
	_IDirect3DDevice8_SetRenderTarget@12 (0000)
000D0F50 00d0:
	_tgaLoadHeader (0000)
000D1020 0010:
	_IDirect3DDevice8_GetDepthStencilSurface@8 (0000)
000D1030 0020:
	_IDirect3DDevice8_Clear@28 (0000)
000D1050 0090:
	_tgaLoadImageData (0000)
000D10E0 0010:
	_IDirect3DDevice8_SetTransform@12 (0000)
000D10F0 0010:
	_IDirect3DDevice8_GetTransform@12 (0000)
000D1100 0050:
	_tgaLoad (0000)
000D1150 0010:
	_progress_bar_is_stuff_ready (0000)
000D1160 0220:
	_IDirect3DDevice8_SetRenderState@12 (0000)
000D1380 0010:
	_IDirect3DDevice8_SetTexture@12 (0000)
000D1390 0060:
	_IDirect3DDevice8_SetTextureStageState@16 (0000)
000D13F0 0090:
	_progress_bar_initialize (0000)
000D1480 0010:
	_progress_bar_dispose (0000)
000D1490 0030:
	_progress_bar_begin (0000)
000D14C0 0010:
	_IDirect3DDevice8_SetVertexShader@8 (0000)
000D14D0 0020:
	_progress_bar_end (0000)
000D14F0 0010:
	_progress_bar_is_active (0000)
000D1500 0050:
	_to_screen (0000)
000D1550 0110:
	_generate_gravy_rect (0000)
000D1660 0010:
	_IDirect3DDevice8_SetPixelShaderProgram@8 (0000)
000D1670 0010:
	_IDirect3DDevice8_BlockUntilVerticalBlank@4 (0000)
000D1680 0020:
	_IDirect3DDevice8_SetVertexData2f@16 (0000)
000D16A0 0030:
	_IDirect3DDevice8_SetVertexData4f@24 (0000)
000D16D0 0010:
	_progress_bar_create_noise_texture (0000)
000D16E0 0010:
	_IDirect3DDevice8_Begin@8 (0000)
000D16F0 0010:
	_IDirect3DDevice8_End@4 (0000)
000D1700 0030:
	_gen_cloud_coord (0000)
000D1730 0040:
	_gen_mask_coord (0000)
000D1770 0160:
	_draw_layer_int (0000)
000D18D0 0060:
	_draw_layer (0000)
000D1930 0060:
	_draw_initial_layer (0000)
000D1990 0030:
	_this_is_awful (0000)
000D19C0 00a0:
	_do_convoluation_coords (0000)
000D1A60 0130:
	_draw_fade_layer (0000)
000D1B90 0010:
	_progress_bar_enable (0000)
000D1BA0 0010:
	_D3DTexture_UnlockRect@8 (0000)
000D1BB0 0010:
	_IDirect3DTexture8_Release@4 (0000)
000D1BC0 0010:
	_IDirect3DTexture8_GetLevelDesc@12 (0000)
000D1BD0 0020:
	_IDirect3DTexture8_LockRect@20 (0000)
000D1BF0 0010:
	_IDirect3DTexture8_UnlockRect@8 (0000)
000D1C00 0040:
	_D3DXMatrixIdentity (0000)
000D1C40 0250:
	_SetRenderStateSmart (0000)
000D1E90 0070:
	_SetTextureStageStateSmart (0000)
000D1F00 04b0:
	_progress_bar_render (0000)
000D23B0 00b0:
	_code_000d23b0 (0000)
000D2460 00d0:
	_progress_bar_create_mask_texture (0000)
000D2530 00c0:
	_progress_bar_eachframe (0000)
000D25F0 0490:
	_progress_bar_make_stuff_ready (0000)
000D2A80 00a0:
	_progress_bar_display (0000)
00271088 0148:
	_D3DSIMPLERENDERSTATEENCODE (0000)
002711D0 0003:
	??_C@_02JDPG@rb?$AA@ (0000)
002711D4 0004:
	??_C@_03OAKNMOCF@z?$CFs?$AA@ (0000)
002711D8 000f:
	??_C@_0P@HMFGDDOL@z?$CFsloading?4tga?$AA@ (0000)
002711E8 000f:
	??_C@_0P@NFEHLENB@d?$CFsloading?4tga?$AA@ (0000)
002711F8 0014:
	??_C@_0BE@GLEHOOPO@rect?9?$DOy0?5?$DM?5rect?9?$DOy1?$AA@ (0000)
0027120C 0014:
	??_C@_0BE@PHIKGFLP@rect?9?$DOx0?5?$DM?5rect?9?$DOx1?$AA@ (0000)
00271220 0028:
	??_C@_0CI@MFDDAKJL@c?3?2halo?2SOURCE?2interface?2progres@ (0000)
00271248 0004:
	__real@42000000 (0000)
0027124C 0004:
	__real@44400000 (0000)
00271250 0004:
	__real@44200000 (0000)
00271258 0008:
	__real@406e000000000000 (0000)
00271260 0008:
	__real@4074000000000000 (0000)
00271268 0008:
	__real@3fd3333340000000 (0000)
00271270 0008:
	__real@400921cac083126f (0000)
00271278 0008:
	__real@3fecccccc0000000 (0000)
00271280 0008:
	__real@400921f9f01b866e (0000)
00271288 0008:
	__real@3fc999999999999a (0000)
00271290 0008:
	__real@3fa999999999999a (0000)
00271298 0004:
	__real@3ca0a0a1 (0000)
0027129C 0004:
	__real@3f7afafb (0000)
002712A0 0008:
	__real@3fe8000000000000 (0000)
002712A8 0004:
	__real@40490e56 (0000)
002712AC 0023:
	??_C@_0CD@KJFNFKIK@?$CIprogress?$DO?$DN0?4f?$CJ?5?$CG?$CG?5?$CIprogress?$DM?$DN1?4@ (0000)
002FD5A8 47f1:
	_data_002fd5a8 (0000)
	_compressed_monk_tone (0008)
	_delta_z (47e4)
	_num_layers (47e8)
00454030 01f4:
	_bss_00454030 (0000)
	_progress_bar_mode (01d8)
	_last_t (01ec)
*/

/* ---------- headers */

#include "cseries.h"
#include "progress_bar.h"
#include "ui_widget.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "unicode.h"
#include "rasterizer.h"
#include "input.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "ima_adpcm.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
