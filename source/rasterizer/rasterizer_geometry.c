/*
RASTERIZER_GEOMETRY.C

symbols in this file:
0016F7D0 0020:
	_uncompress_int8_to_real (0000)
0016F7F0 0020:
	_uncompress_int16_to_real (0000)
0016F810 0090:
	_uncompress_int32_to_real_vector3d (0000)
0016F8A0 0050:
	_rasterizer_geometry_get_vertex_size (0000)
0016F8F0 0010:
	_rasterizer_geometry_byte_swap_vertices (0000)
0016F900 0440:
	_rasterizer_geometry_uncompress_vertices (0000)
0016FD40 0010:
	_rasterizer_geometry_stripify (0000)
0016FD50 0070:
	_environment_vertex_compressed_get_point (0000)
0016FDC0 0080:
	_environment_vertex_compressed_get_normal (0000)
0016FE40 0070:
	_environment_vertex_compressed_get_texcoord (0000)
0016FEB0 0080:
	_environment_lightmap_vertex_compressed_get_incident_radiosity (0000)
0016FF30 0090:
	_environment_lightmap_vertex_compressed_get_texcoord (0000)
0016FFC0 0060:
	_compress_real_to_int8 (0000)
00170020 0050:
	_compress_real_to_int8_clamp (0000)
00170070 0070:
	_compress_real_to_int16 (0000)
001700E0 0060:
	_compress_real_to_int16_clamp (0000)
00170140 0220:
	_compress_real_vector3d_to_int32 (0000)
00170360 0200:
	_compress_real_vector3d_to_int32_clamp (0000)
00170560 02e0:
	_rasterizer_geometry_compress_vertices (0000)
0029E344 001a:
	_rdata_0029e344 (0000)
0029E360 0004:
	__real@35000000 (0000)
0029E364 0004:
	__real@3a001002 (0000)
0029E368 0030:
	??_C@_0DA@GFMENBNG@c?3?2halo?2SOURCE?2rasterizer?2raster@ (0000)
0029E398 0044:
	??_C@_0EE@PNJAPPJM@count?$CKsizeof?$CIstruct?5environment_@ (0000)
0029E3E0 0048:
	??_C@_0EI@DONGBPFL@count?$CKsizeof?$CIstruct?5environment_@ (0000)
0029E428 004d:
	??_C@_0EN@HJINING@count?$CKsizeof?$CIstruct?5environment_@ (0000)
0029E478 0051:
	??_C@_0FB@GCMMGGGN@count?$CKsizeof?$CIstruct?5environment_@ (0000)
0029E4CC 0013:
	??_C@_0BD@OPFHNGDK@src?9?$DOnodes?$FL1?$FN?$CF3?$DN?$DN0?$AA@ (0000)
0029E4E0 0013:
	??_C@_0BD@CDPNNGKE@src?9?$DOnodes?$FL0?$FN?$CF3?$DN?$DN0?$AA@ (0000)
0029E4F4 003e:
	??_C@_0DO@MIHGPCNE@count?$CKsizeof?$CIstruct?5model_vertex@ (0000)
0029E538 0042:
	??_C@_0EC@DGDJGMJD@count?$CKsizeof?$CIstruct?5model_vertex@ (0000)
0029E57C 0036:
	??_C@_0DG@CBADPEAN@?$CD?$CD?$CD?5ERROR?5can?8t?5uncompress?5this?5@ (0000)
0029E5B4 000d:
	??_C@_0N@NFCOPMLF@uncompressed?$AA@ (0000)
0029E5C4 0009:
	??_C@_08FDJGCBLL@texcoord?$AA@ (0000)
0029E5D0 0013:
	??_C@_0BD@GHLLAJAG@z?$DO?$DN0?40f?5?$CG?$CG?5z?$DM?$DN1?40f?$AA@ (0000)
0029E5E4 0004:
	__real@46ffff00 (0000)
0029E5E8 0014:
	??_C@_0BE@NNGOJEKG@z?$DO?$DN?91?40f?5?$CG?$CG?5z?$DM?$DN1?40f?$AA@ (0000)
0029E5FC 0018:
	??_C@_0BI@PFBMHOJF@fabs?$CIv2?4k?5?9?5v?9?$DOk?$CJ?$DM0?401f?$AA@ (0000)
0029E614 0018:
	??_C@_0BI@EMAPPLEH@fabs?$CIv2?4j?5?9?5v?9?$DOj?$CJ?$DM0?401f?$AA@ (0000)
0029E62C 0018:
	??_C@_0BI@FMEKHDHA@fabs?$CIv2?4i?5?9?5v?9?$DOi?$CJ?$DM0?401f?$AA@ (0000)
0029E644 0004:
	__real@43ffc000 (0000)
0029E648 0004:
	__real@447fe000 (0000)
0029E64C 0024:
	??_C@_0CE@BAFANFAL@invalid?5vector?$DN?5?$FL?$CFf?5?$CFf?5?$CFf?$FN?50x?$CFx?$CF@ (0000)
0029E670 0034:
	??_C@_0DE@MCFDJBBN@?$CD?$CD?$CD?5ERROR?5can?8t?5compress?5this?5ty@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer_geometry.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
