/*
RASTERIZER_XBOX_WIDGETS.C

symbols in this file:
00169EF0 01b0:
	_D3DDevice_SetRenderState (0000)
0016A0A0 0050:
	_D3DDevice_SetTextureStageState (0000)
0016A0F0 01c0:
	_rasterizer_widget_project_billboard (0000)
0016A2B0 0220:
	_IDirect3DDevice8_SetRenderState@12 (0000)
0016A4D0 0060:
	_IDirect3DDevice8_SetTextureStageState@16 (0000)
0016A530 0010:
	_IDirect3DDevice8_SetVertexShaderConstant@16 (0000)
0016A540 0010:
	_IDirect3DDevice8_BeginVisibilityTest@4 (0000)
0016A550 0010:
	_IDirect3DDevice8_EndVisibilityTest@8 (0000)
0016A560 0010:
	_IDirect3DDevice8_GetVisibilityTestResult@16 (0000)
0016A570 0020:
	_IDirect3DDevice8_SetVertexData2f@16 (0000)
0016A590 0030:
	_IDirect3DDevice8_SetVertexData4f@24 (0000)
0016A5C0 0010:
	_IDirect3DDevice8_SetVertexData2s@16 (0000)
0016A5D0 0010:
	_IDirect3DDevice8_SetVertexDataColor@12 (0000)
0016A5E0 0010:
	_IDirect3DDevice8_Begin@8 (0000)
0016A5F0 0010:
	_IDirect3DDevice8_End@4 (0000)
0016A600 0010:
	__rasterizer_widget_end (0000)
0016A610 00d0:
	__rasterizer_widget_get_occlusion_test_result (0000)
0016A6E0 0170:
	__rasterizer_widget_submit (0000)
0016A850 0480:
	__rasterizer_widget_begin (0000)
0016ACD0 00c0:
	__rasterizer_widget_set_texture (0000)
0016AD90 0040:
	__rasterizer_widget_set_tint_factor (0000)
0016ADD0 0040:
	__rasterizer_widget_set_zbuffer_enable (0000)
0016AE10 0210:
	__rasterizer_widget_draw_sprite2d (0000)
0016B020 0240:
	__rasterizer_widget_draw_sprite3d (0000)
0016B260 0280:
	__rasterizer_widget_submit_occlusion_test (0000)
0029CCB8 003d:
	??_C@_0DN@BOAOHHMH@?$CD?$CD?$CD?5ERROR?5rasterizer_widget_get_@ (0000)
0029CCF8 0026:
	??_C@_0CG@BMNNONEP@?$CIocclusion_test_result?$CG0x8000000@ (0000)
0029CD20 0039:
	??_C@_0DJ@CJFIOLEN@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
0029CD5C 0022:
	??_C@_0CC@GCBGNNKG@?$CD?$CD?$CD?5ERROR?5unsupported?5widget?5typ@ (0000)
0029CD80 0045:
	??_C@_0EF@KIEIHHAH@fabs?$CIcos_theta?$CKcos_theta?5?$CL?5sin_t@ (0000)
0029CDC8 0039:
	??_C@_0DJ@DLLMHNIO@?$CD?$CD?$CD?5ERROR?5rasterizer_widget_subm@ (0000)
0029CE04 0004:
	__real@c6fffe00 (0000)
004662EA 0001:
	_bss_004662ea (0000)
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
#include "widgets.h"
#include "rasterizer_widgets.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
