/*
RASTERIZER_XBOX_DYNAVOBGEOM.C

symbols in this file:
0014E840 01b0:
	_D3DDevice_SetRenderState (0000)
0014E9F0 0050:
	_D3DDevice_SetTextureStageState (0000)
0014EA40 0010:
	__rasterizer_hud_begin (0000)
0014EA50 0010:
	__rasterizer_hud_end (0000)
0014EA60 0010:
	__rasterizer_dynamic_lit_geometry_draw (0000)
0014EA70 0100:
	__rasterizer_dynamic_screen_geometry_add_multitexture_params_to_base (0000)
0014EB70 0220:
	_IDirect3DDevice8_SetRenderState@12 (0000)
0014ED90 0030:
	__rasterizer_dynamic_screen_geometry_draw (0000)
0014EDC0 0060:
	_IDirect3DDevice8_SetTextureStageState@16 (0000)
0014EE20 0010:
	_IDirect3DDevice8_SetVertexShaderConstant@16 (0000)
0014EE30 0020:
	_IDirect3DDevice8_SetVertexData2f@16 (0000)
0014EE50 0010:
	_IDirect3DDevice8_SetVertexDataColor@12 (0000)
0014EE60 0010:
	_IDirect3DDevice8_Begin@8 (0000)
0014EE70 0010:
	_IDirect3DDevice8_End@4 (0000)
0014EE80 0270:
	__rasterizer_dynamic_unlit_geometry_draw (0000)
0014F0F0 0040:
	_submit_screen_vertex (0000)
0014F130 0b90:
	__rasterizer_psuedo_dynamic_screen_quad_draw (0000)
0028FBDC 0010:
	??_C@_0BA@KKBHHCGF@multitex_params?$AA@ (0000)
0028FBEC 0005:
	??_C@_04BHIIPFEC@base?$AA@ (0000)
0028FBF4 003d:
	??_C@_0DN@KLLFPFFH@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
0028FC34 003e:
	??_C@_0DO@GMEFDNIN@_rasterizer_dynamic_screen_geome@ (0000)
0028FC74 002f:
	??_C@_0CP@PIDBCAKH@?$CD?$CD?$CD?5ERROR?5too?5many?5transparent?5g@ (0000)
0028FCA4 0009:
	??_C@_08JOJIKHG@centroid?$AA@ (0000)
0028FCB0 0027:
	??_C@_0CH@DLHDBKLI@shader?9?$DObase?4type?$DN?$DN_shader_type_@ (0000)
0028FCD8 0069:
	??_C@_0GJ@OKAOIPGM@?$CBTEST_FLAG?$CIgeometry_flags?0?5_rast@ (0000)
0028FD44 0016:
	??_C@_0BG@MMAGMAFI@meter?9?$DOgradient?$DN?$DN1?40f?$AA@ (0000)
0028FD5C 0013:
	??_C@_0BD@BMLFIAIE@meter?9?$DOtint_mode_2?$AA@ (0000)
0028FD70 0035:
	??_C@_0DF@MDAHAMMD@?$CBparameters?9?$DOmap?$FL1?$FN?5?$HM?$HM?5?$CBparamete@ (0000)
0028FDA8 002a:
	??_C@_0CK@NCHEDINN@?$CBparameters?9?$DOmap?$FL2?$FN?5?$HM?$HM?5parameter@ (0000)
0028FDD4 0013:
	??_C@_0BD@LBFDNODJ@parameters?9?$DOmap?$FL0?$FN?$AA@ (0000)
00465A16 0001:
	_bss_00465a16 (0000)
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
