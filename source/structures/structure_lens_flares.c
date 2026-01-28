/*
STRUCTURE_LENS_FLARES.C

symbols in this file:
00183BB0 0020:
	_code_00183bb0 (0000)
00183BD0 0060:
	_cluster_index_from_point (0000)
00183C30 10f0:
	_build_structure_lens_flares (0000)
00184D20 0010:
	_structure_lens_flares_place (0000)
002A1370 0048:
	??_C@_0EI@BOLLHBJG@?$CD?$CD?$CD?5ERROR?5failed?5to?5clear?5lens?5f@ (0000)
002A13B8 0007:
	??_C@_06DMAFEADM@done?$AN?6?$AA@ (0000)
002A13C0 0048:
	??_C@_0EI@OPILBFAL@?$CD?$CD?$CD?5ERROR?5failed?5to?5allocate?5tem@ (0000)
002A1408 0047:
	??_C@_0EH@EENPIBPH@?$CD?$CD?$CD?5ERROR?5failed?5to?5sort?5lens?5fl@ (0000)
002A1450 0033:
	??_C@_0DD@CDKGJELE@?$CD?$CD?$CD?5ERROR?5failed?5to?5resize?5lens?5@ (0000)
002A1488 0047:
	??_C@_0EH@GEEEEHH@?$CD?$CD?$CD?5WARNING?5failed?5to?5add?5lens?5f@ (0000)
002A14D0 0066:
	??_C@_0GG@JDHLIDMF@point_count?$CL2?$DMMAXIMUM_TRIANGLES_@ (0000)
002A1538 0071:
	??_C@_0HB@PCJNLAFB@cluster?9?$DOfirst_lens_flare_marker@ (0000)
002A15B0 004f:
	??_C@_0EP@HDCOMOMJ@cluster?9?$DOfirst_lens_flare_marker@ (0000)
002A1600 0037:
	??_C@_0DH@MGKKDPBA@cluster_index?$DMtemp_markers?$FLmarke@ (0000)
002A1638 0040:
	??_C@_0EA@NCAOCOMC@?$CD?$CD?$CD?5WARNING?5failed?5to?5add?5lens?5f@ (0000)
002A1678 0055:
	??_C@_0FF@CDJGMDIB@surface?9?$DOvertex_indices?$FL2?$FN?$DO?$DN0?5?$CG?$CG@ (0000)
002A16D0 0055:
	??_C@_0FF@KHBPBDOH@surface?9?$DOvertex_indices?$FL1?$FN?$DO?$DN0?5?$CG?$CG@ (0000)
002A1728 0055:
	??_C@_0FF@NLGHKDMF@surface?9?$DOvertex_indices?$FL0?$FN?$DO?$DN0?5?$CG?$CG@ (0000)
002A1780 0022:
	??_C@_0CC@BLKMGOAC@building?5structure?5lens?5flares?4?4@ (0000)
002A17A4 000e:
	??_C@_0O@IHAGMCLG@structure_bsp?$AA@ (0000)
002A17B4 0032:
	??_C@_0DC@KJLLBBH@c?3?2halo?2SOURCE?2structures?2struct@ (0000)
*/

/* ---------- headers */

#include "cseries.h"

#include "structure_bsp_definitions.h"
#include "physics/collision_bsp_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

long cluster_index_from_point(
	struct structure_bsp const *structure_bsp,
	real_point3d const *point)
{
	long test_result= bsp3d_test_point(&TAG_BLOCK_GET_ELEMENT(&structure_bsp->collision_bsp, 0, struct collision_bsp)->bsp3d, 0, point);
	long result= NONE;
	
	if (test_result!=NONE)
	{
		result= TAG_BLOCK_GET_ELEMENT(&structure_bsp->leaves, test_result&LONG_MAX, struct structure_leaf)->cluster_index;
	}

	return result;
}

/* ---------- private code */
