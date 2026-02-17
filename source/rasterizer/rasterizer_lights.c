/*
RASTERIZER_LIGHTS.C

symbols in this file:
00170840 0030:
	_code_00170840 (0000)
00170870 0040:
	_code_00170870 (0000)
001708B0 00f0:
	_code_001708b0 (0000)
001709A0 0030:
	_rasterizer_lights_reset_for_new_map (0000)
001709D0 0130:
	_rasterizer_lights_begin_for_new_frame (0000)
00170B00 0010:
	_rasterizer_lights_begin (0000)
00170B10 0150:
	_rasterizer_light_submit (0000)
00170C60 0010:
	_rasterizer_lights_end (0000)
00170C70 0250:
	_code_00170c70 (0000)
00170EC0 0290:
	_rasterizer_lens_flare_submit (0000)
00171150 0190:
	_rasterizer_lens_flare_submit_for_cluster (0000)
001712E0 0190:
	_rasterizer_lens_flares_submit_occlusion_tests (0000)
00171470 08c0:
	_rasterizer_lens_flares_draw (0000)
0029E6A4 003f:
	??_C@_0DP@MMPMMAFC@lens_flare_index?$DO?$DN0?5?$CG?$CG?5lens_flar@ (0000)
0029E6E4 002e:
	??_C@_0CO@MGBADAON@c?3?2halo?2SOURCE?2rasterizer?2raster@ (0000)
0029E718 0063:
	??_C@_0GD@JPCGFHAO@lens_flare_parameters?9?$DOlight_ind@ (0000)
0029E780 0082:
	??_C@_0IC@IBFAKJOH@structure_lens_flare_index?$DO?$DN0?5?$CG?$CG@ (0000)
0029E804 0016:
	??_C@_0BG@JFBJFIEK@lens_flare_parameters?$AA@ (0000)
0029E81C 002e:
	??_C@_0CO@FCEOCGFK@?$CD?$CD?$CD?5ERROR?5too?5many?5lights?5submit@ (0000)
0029E84C 003f:
	??_C@_0DP@CJDDDBGK@parameters?9?$DOcolor?4blue?5?$DO?$DN0?40f?5?$CG?$CG@ (0000)
0029E88C 003f:
	??_C@_0DP@IFBCMKNP@parameters?9?$DOcolor?4green?$DO?$DN0?40f?5?$CG?$CG@ (0000)
0029E8CC 003d:
	??_C@_0DN@IFBOCEFD@parameters?9?$DOcolor?4red?5?$DO?$DN0?40f?5?$CG?$CG?5@ (0000)
0029E90C 003a:
	??_C@_0DK@NODNGFKL@?$CD?$CD?$CD?5ERROR?5unsupported?5lens?5flare@ (0000)
0029E948 0032:
	??_C@_0DC@BJJPKDMH@?$CD?$CD?$CD?5ERROR?5too?5many?5lens?5flares?5s@ (0000)
0029E980 004d:
	??_C@_0EN@OMGAKMEG@parameters?9?$DOlight_index?$DO?$DN0?5?$CG?$CG?5pa@ (0000)
0029E9D0 0065:
	??_C@_0GF@CKCJNIMD@structure_lens_flare_index?$DO?$DN0?5?$CG?$CG@ (0000)
0029EA38 005b:
	??_C@_0FL@ECFOOILG@parameters?9?$DOlens_flare_index?$DO?$DN0?5@ (0000)
0029EA98 006b:
	??_C@_0GL@HCLLBKGN@?$CIparameters?9?$DOcompressed_window_i@ (0000)
0029EB04 0017:
	??_C@_0BH@FDNIDAHL@parameters?9?$DOdefinition?$AA@ (0000)
0029EB1C 003c:
	??_C@_0DM@DJHNIOGP@?$CD?$CD?$CD?5ERROR?5unsupported?5lens?5flare@ (0000)
0029EB58 003b:
	??_C@_0DL@EPCOBKHO@animation_color?4blue?5?$DO?$DN0?40f?5?$CG?$CG?5a@ (0000)
0029EB94 003b:
	??_C@_0DL@CLPEFPIB@animation_color?4green?$DO?$DN0?40f?5?$CG?$CG?5a@ (0000)
0029EBD0 0039:
	??_C@_0DJ@NIJDHHFP@animation_color?4red?5?$DO?$DN0?40f?5?$CG?$CG?5an@ (0000)
0029EC0C 003b:
	??_C@_0DL@HIJLAENC@animation_color?4alpha?$DO?$DN0?40f?5?$CG?$CG?5a@ (0000)
0029EC48 0023:
	??_C@_0CD@LGKNCBJE@reflection?9?$DOanimation_period?$CB?$DN0?4@ (0000)
0029EC6C 0004:
	__real@42652ee1 (0000)
00466B80 51725:
	_bss_00466b80 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct lens_flare_occlusion_test_results
{
	short light_identifier;
	byte data[8][4];
};

struct rasterizer_lens_flare_submit_parameters
{
	struct lens_flare_definition *definition;
	real_point3d position;
	unsigned long compressed_direction;
	unsigned long compressed_up;
	unsigned long compressed_light_color;
	short light_identifier;
	short light_index;
	short lens_flare_index;
	byte compressed_window_index;
	byte compressed_light_scale;
	long internal__occlusion_pixels;
};

/* ---------- prototypes */

/* ---------- globals */

static struct lens_flare_occlusion_test_results local_lens_flare_occlusion_test_results[896];
static byte local_lens_flare_occlusion_test_results2[65544][MAXIMUM_WINDOWS];
static struct rasterizer_lens_flare_submit_parameters local_lens_flare_parameters[MAXIMUM_LENS_FLARES_PER_FRAME]= {0};
static long local_lens_flare_count= 0;
static boolean local_lens_flare_error_printed= FALSE;

/* ---------- public code */

void rasterizer_lights_reset_for_new_map(void)
{
	memset(local_lens_flare_occlusion_test_results, 0, sizeof(local_lens_flare_occlusion_test_results));
	memset(local_lens_flare_occlusion_test_results2, 0, sizeof(local_lens_flare_occlusion_test_results2));
	local_lens_flare_count= 0;

	return;
}

static struct rasterizer_lens_flare_submit_parameters *lens_flare_submit_parameter_get(short lens_flare_index)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_lights.c", 67, lens_flare_index>=0 && lens_flare_index<local_lens_flare_count);
	return &local_lens_flare_parameters[lens_flare_index];
}

/* ---------- private code */
