/*
STRUCTURE_BSP_DEFINITIONS.C

symbols in this file:
00182DA0 00a0:
	_structure_bsp_get_cluster_pvs (0000)
00182E40 01b0:
	_structure_bsp_find_material_for_surface (0000)
00182FF0 0030:
	_vertex_type_from_shader_tag (0000)
00183020 00a0:
	_structure_bsp_get_cluster_encoded_sound_data (0000)
001830C0 00a0:
	_structure_bsp_get_cluster_encoded_sound_distance (0000)
002A1078 006c:
	??_C@_0GM@IEBDGEJL@?$CIcluster_index?$CL1?$CJ?$CKBIT_VECTOR_SIZ@ (0000)
002A10E8 0040:
	??_C@_0EA@EIICKPGK@cluster_index?$DO?$DN0?5?$CG?$CG?5cluster_inde@ (0000)
002A1128 0036:
	??_C@_0DG@NELJLNPJ@c?3?2halo?2SOURCE?2structures?2struct@ (0000)
002A1160 0044:
	??_C@_0EE@GCNBGNLI@surface_index?$DMmaterial?9?$DOfirst_su@ (0000)
002A11A4 002d:
	??_C@_0CN@DMFHEBIB@surface_index?$DO?$DNmaterial?9?$DOfirst_s@ (0000)
002A11D4 003b:
	??_C@_0DL@PINMBEBG@offset?$DO?$DN0?5?$CG?$CG?5offset?$DMstructure_bs@ (0000)
002A1210 0017:
	??_C@_0BH@FKLIMEJG@row_index?$DMcolumn_index?$AA@ (0000)
002A1228 0046:
	??_C@_0EG@FIKJDFNH@to_cluster_index?$DO?$DN0?5?$CG?$CG?5to_cluste@ (0000)
002A1270 004a:
	??_C@_0EK@GPLJLDED@from_cluster_index?$DO?$DN0?5?$CG?$CG?5from_cl@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "structure_bsp_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

unsigned long *structure_bsp_get_cluster_pvs(
	struct structure_bsp *structure_bsp,
	short cluster_index)
{
	match_assert("c:\\halo\\SOURCE\\structures\\structure_bsp_definitions.c", 36, cluster_index>=0 && cluster_index<structure_bsp->clusters.count);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_bsp_definitions.c",
		37,
		(cluster_index+1)*BIT_VECTOR_SIZE_IN_LONGS(structure_bsp->clusters.count)<=structure_bsp->cluster_data.size
	);

	// Get pointer to bitvector starting at the cluster index
	return (unsigned long *)(
		(byte *)structure_bsp->cluster_data.address +
		sizeof(unsigned long) * cluster_index *
		BIT_VECTOR_SIZE_IN_LONGS(structure_bsp->clusters.count)	
	);
}

/* ---------- private code */
