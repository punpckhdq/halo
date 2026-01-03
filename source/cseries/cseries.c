/*
CSERIES.C

symbols in this file:
0007BEA0 0020:
	_cseries_initialize (0000)
0007BEC0 0010:
	_cseries_dispose (0000)
0007BED0 0030:
	_string_to_tag (0000)
0007BF00 0040:
	_tag_to_string (0000)
0007BF40 0020:
	_strnlen (0000)
0007BF60 0040:
	_strnupr (0000)
0007BFA0 0040:
	_strnlwr (0000)
0007BFE0 0030:
	_strupr (0000)
0007C010 0030:
	_strlwr (0000)
0007C040 0020:
	_csprintf (0000)
0007C060 0050:
	_display_assert (0000)
0007C0B0 00a0:
	_csmemcmp (0000)
0007C150 00a0:
	_csmemmove (0000)
0007C1F0 00b0:
	_csmemset (0000)
0007C2A0 0080:
	_csstrcat (0000)
0007C320 0080:
	_csstrcmp (0000)
0007C3A0 00a0:
	_csstrncat (0000)
0007C440 00a0:
	_csstrncmp (0000)
0007C4E0 00a0:
	_csstrncpy (0000)
0007C580 0050:
	_csstrtok (0000)
0007C5D0 0090:
	_csstrlen (0000)
0007C660 00c0:
	_csstrcpy (0000)
0007C720 00e0:
	_csmemcpy (0000)
0007C800 00c0:
	_csstrcasecmp (0000)
0007C8C0 0060:
	_stristr (0000)
0007C920 0030:
	_string_hash (0000)
002572B0 0110:
	_rdata_002572b0 (0000)
002573C0 001b:
	??_C@_0BL@GHNPBCKH@EXCEPTION?5?$CFs?5in?5?$CFs?0?$CD?$CFd?3?5?$CFs?$AA@ (0000)
002573DC 0005:
	??_C@_04FNLOFOEM@warn?$AA@ (0000)
002573E4 0005:
	??_C@_04BJJLCKLO@halt?$AA@ (0000)
002573EC 0012:
	??_C@_0BC@GDIKHIHG@?$DMno?5reason?5given?$DO?$AA@ (0000)
00257400 0025:
	??_C@_0CF@OMIHKKPG@size?$DO?$DN0?5?$CG?$CG?5size?$DM?$DNMAXIMUM_MEMCMP_@ (0000)
00257428 0021:
	??_C@_0CB@DJKCEIPC@c?3?2halo?2SOURCE?2cseries?2cseries?4c@ (0000)
0025744C 0009:
	??_C@_08OJEIONGL@p1?5?$CG?$CG?5p2?$AA@ (0000)
00257458 002d:
	??_C@_0CN@HOKDOGEN@size?$DO?$DN0?5?$CG?$CG?5size?$DM?$DNMAXIMUM_MEMCPY_@ (0000)
00257488 0025:
	??_C@_0CF@GHONNHCB@size?$DO?$DN0?5?$CG?$CG?5size?$DM?$DNMAXIMUM_MEMSET_@ (0000)
002574B0 0007:
	??_C@_06HOHPNJLC@buffer?$AA@ (0000)
002574B8 0009:
	??_C@_08NCIDGPPH@s1?5?$CG?$CG?5s2?$AA@ (0000)
002574C4 0024:
	??_C@_0CE@PKLKNLJL@size?$DO?$DN0?5?$CG?$CG?5size?$DMMAXIMUM_STRING_S@ (0000)
002574E8 0003:
	??_C@_02LLHJKEJF@s2?$AA@ (0000)
002574EC 0003:
	??_C@_02JAFEPHFG@s1?$AA@ (0000)
002574F0 0041:
	??_C@_0EB@GGDKFPCD@source?$CLsource_size?$DMdestination?5?$HM@ (0000)
00257534 0032:
	??_C@_0DC@NKDHFHJJ@source_size?$DO?$DN0?5?$CG?$CG?5source_size?$DMMA@ (0000)
00257568 0055:
	??_C@_0FF@LCNAJGEP@?$CIbyte?5?$CK?$CJsource?$CLsize?$DM?$DN?$CIbyte?5?$CK?$CJdes@ (0000)
002575C0 002c:
	??_C@_0CM@JNDKGHH@size?$DO?$DN0?5?$CG?$CG?5size?$DMMAXIMUM_MEMCPY_M@ (0000)
002DCC84 0088:
	_global_real_argb_white (0000)
	_global_real_argb_grey (0004)
	_global_real_argb_black (0008)
	_global_real_argb_red (000c)
	_global_real_argb_green (0010)
	_global_real_argb_blue (0014)
	_global_real_argb_cyan (0018)
	_global_real_argb_yellow (001c)
	_global_real_argb_magenta (0020)
	_global_real_argb_pink (0024)
	_global_real_argb_lightblue (0028)
	_global_real_argb_orange (002c)
	_global_real_argb_purple (0030)
	_global_real_argb_aqua (0034)
	_global_real_argb_darkgreen (0038)
	_global_real_argb_salmon (003c)
	_global_real_argb_violet (0040)
	_global_real_rgb_white (0044)
	_global_real_rgb_grey (0048)
	_global_real_rgb_black (004c)
	_global_real_rgb_red (0050)
	_global_real_rgb_green (0054)
	_global_real_rgb_blue (0058)
	_global_real_rgb_cyan (005c)
	_global_real_rgb_yellow (0060)
	_global_real_rgb_magenta (0064)
	_global_real_rgb_pink (0068)
	_global_real_rgb_lightblue (006c)
	_global_real_rgb_orange (0070)
	_global_real_rgb_purple (0074)
	_global_real_rgb_aqua (0078)
	_global_real_rgb_darkgreen (007c)
	_global_real_rgb_salmon (0080)
	_global_real_rgb_violet (0084)
*/

/* ---------- headers */

#include "math/real_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static const real_argb_color global_real_argb_color_table[17] =
{
	{ 1.f, 1.f,		1.f,	1.f  },
	{ 1.f, .5f,		.5f,	.5f  },
	{ 1.f, .0f,		.0f,	.0f  },
	{ 1.f, 1.f,		.0f,	.0f  },
	{ 1.f, .0f,		1.f,	.0f  },
	{ 1.f, .0f,		.0f,	1.f  },
	{ 1.f, .0f,		1.f,	1.f  },
	{ 1.f, 1.f,		1.f,	.0f  },
	{ 1.f, 1.f,		.0f,	1.f  },
	{ 1.f, 1.f,		.41f,	.7f  },
	{ 1.f, .39f,	.58f,	.93f },
	{ 1.f, 1.f,		.5f,	.0f },
	{ 1.f, .44f,	.05f,	.43f },
	{ 1.f, .5f,		1.f,	.83f },
	{ 1.f, .0f,		.39f,	.0f },
	{ 1.f, 1.f,		.63f,	.48f },
	{ 1.f, .81f,	.13f,	.56f }
};

const real_argb_color *const global_real_argb_white= &global_real_argb_color_table[0];
const real_argb_color *const global_real_argb_grey= &global_real_argb_color_table[1];
const real_argb_color *const global_real_argb_black= &global_real_argb_color_table[2];
const real_argb_color *const global_real_argb_red= &global_real_argb_color_table[3];
const real_argb_color *const global_real_argb_green= &global_real_argb_color_table[4];
const real_argb_color *const global_real_argb_blue= &global_real_argb_color_table[5];
const real_argb_color *const global_real_argb_cyan= &global_real_argb_color_table[6];
const real_argb_color *const global_real_argb_yellow= &global_real_argb_color_table[7];
const real_argb_color *const global_real_argb_magenta= &global_real_argb_color_table[8];
const real_argb_color *const global_real_argb_pink= &global_real_argb_color_table[9];
const real_argb_color *const global_real_argb_lightblue= &global_real_argb_color_table[10];
const real_argb_color *const global_real_argb_orange= &global_real_argb_color_table[11];
const real_argb_color *const global_real_argb_purple= &global_real_argb_color_table[12];
const real_argb_color *const global_real_argb_aqua= &global_real_argb_color_table[13];
const real_argb_color *const global_real_argb_darkgreen= &global_real_argb_color_table[14];
const real_argb_color *const global_real_argb_salmon= &global_real_argb_color_table[15];
const real_argb_color *const global_real_argb_violet= &global_real_argb_color_table[16];

const real_rgb_color *const global_real_rgb_white= &global_real_argb_color_table[0].rgb;
const real_rgb_color *const global_real_rgb_grey= &global_real_argb_color_table[1].rgb;
const real_rgb_color *const global_real_rgb_black= &global_real_argb_color_table[2].rgb;
const real_rgb_color *const global_real_rgb_red= &global_real_argb_color_table[3].rgb;
const real_rgb_color *const global_real_rgb_green= &global_real_argb_color_table[4].rgb;
const real_rgb_color *const global_real_rgb_blue= &global_real_argb_color_table[5].rgb;
const real_rgb_color *const global_real_rgb_cyan= &global_real_argb_color_table[6].rgb;
const real_rgb_color *const global_real_rgb_yellow= &global_real_argb_color_table[7].rgb;
const real_rgb_color *const global_real_rgb_magenta= &global_real_argb_color_table[8].rgb;
const real_rgb_color *const global_real_rgb_pink= &global_real_argb_color_table[9].rgb;
const real_rgb_color *const global_real_rgb_lightblue= &global_real_argb_color_table[10].rgb;
const real_rgb_color *const global_real_rgb_orange= &global_real_argb_color_table[11].rgb;
const real_rgb_color *const global_real_rgb_purple= &global_real_argb_color_table[12].rgb;
const real_rgb_color *const global_real_rgb_aqua= &global_real_argb_color_table[13].rgb;
const real_rgb_color *const global_real_rgb_darkgreen= &global_real_argb_color_table[14].rgb;
const real_rgb_color *const global_real_rgb_salmon= &global_real_argb_color_table[15].rgb;
const real_rgb_color *const global_real_rgb_violet= &global_real_argb_color_table[16].rgb;

/* ---------- public code */

/* ---------- private code */
