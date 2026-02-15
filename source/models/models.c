/*
MODELS.C

symbols in this file:
00112DB0 0430:
	_code_00112db0 (0000)
001131E0 0110:
	_model_interpolate_node_orientations (0000)
001132F0 0090:
	_model_get_node_orientations (0000)
00113380 0140:
	_model_get_node_matrices (0000)
001134C0 0110:
	_model_node_matrices_from_orientations (0000)
001135D0 00a0:
	_model_find_marker (0000)
00113670 0030:
	_model_get_default_inverse_matrix (0000)
001136A0 0070:
	_model_find_node (0000)
00113710 0010:
	_code_00113710 (0000)
00113720 0860:
	_render_model (0000)
00113F80 01d0:
	_model_get_marker_by_name (0000)
00114150 0070:
	_model_build_tangent_matrices (0000)
0027F98C 000d:
	??_C@_0N@PBHIAIOK@render_model?$AA@ (0000)
0027F9A0 0061:
	??_C@_0GB@CAHPCM@part?9?$DOcentroid_secondary_node_in@ (0000)
0027FA08 005d:
	??_C@_0FN@MLJHGCND@part?9?$DOcentroid_primary_node_inde@ (0000)
0027FA68 002c:
	??_C@_0CM@MBIKHGBC@?$CBTEST_FLAG?$CIflags?0?5_render_model_@ (0000)
0027FA94 001f:
	??_C@_0BP@PJEILNMK@c?3?2halo?2SOURCE?2models?2models?4c?$AA@ (0000)
0027FAB4 001e:
	??_C@_0BO@FFDGBNA@node?9?$DOparent_node_index?$CB?$DNNONE?$AA@ (0000)
0027FAD8 0064:
	??_C@_0GE@EAOMEKOC@?$CIactual_detail_level_index?5?$DO?$DN?50?$CJ@ (0000)
0027FB3C 001e:
	??_C@_0BO@INPHBEBB@actual_detail_level_index?5?$DO?50?$AA@ (0000)
0027FB60 0060:
	??_C@_0GA@CEBIGMGH@geometry_detail_level_index?$DO?$DN0?5?$CG@ (0000)
0027FBC0 0009:
	??_C@_08OEDKDIOB@lighting?$AA@ (0000)
0027FBD0 0073:
	??_C@_0HD@NKPBNIND@object_marker?9?$DOnode_index?$DO?$DN0?5?$CG?$CG?5@ (0000)
0027FC44 0008:
	??_C@_07LGFKJAB@markers?$AA@ (0000)
0027FC4C 000e:
	??_C@_0O@BKFGBDDJ@node_matrices?$AA@ (0000)
0030A390 05f8:
	_data_0030a390 (0000)
00456650 0088:
	_bss_00456650 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "models.h"

#include "model_definitions.h"

#include "math/real_math.h"
#include "objects/objects.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

short model_get_marker_by_name(
	long model_index,
	char const *name,
	byte const *region_permutations,
	short const *node_remapping_table,
	short node_count,
	real_matrix4x3 const *node_matrices,
	boolean mirrored_flag,
	struct object_marker *markers,
	short maximum_marker_count)
{
	short result= 0;
	short marker_index= model_find_marker(model_index, name);

	match_assert("c:\\halo\\SOURCE\\models\\models.c", 760, node_matrices);
	match_assert("c:\\halo\\SOURCE\\models\\models.c", 761, markers);

	if (marker_index!=NONE)
	{
		short i;

		struct model *model= model_definition_get(model_index);
		struct model_marker* marker= TAG_BLOCK_GET_ELEMENT(&model->markers, marker_index, struct model_marker);

		for (i=0; i<marker->instances.count; i++)
		{
			struct model_marker_instance* instance= TAG_BLOCK_GET_ELEMENT(&marker->instances, i, struct model_marker_instance);
			if (!region_permutations ||
				region_permutations[instance->region_index]==instance->permutation_index)
			{
				struct object_marker *object_marker;

				if (result>=maximum_marker_count)
				{
					break;
				}

				object_marker= &markers[result++];
				object_marker->node_index= node_remapping_table ? node_remapping_table[instance->node_index] : instance->node_index;
				matrix4x3_from_point_and_quaternion(&object_marker->node_matrix, &instance->translation, &instance->rotation);
				match_assert(
					"c:\\halo\\SOURCE\\models\\models.c",
					785,
					object_marker->node_index>=0 && object_marker->node_index<(node_remapping_table ? node_count : model->nodes.count));
				
				matrix4x3_multiply(&node_matrices[object_marker->node_index], &object_marker->node_matrix, &object_marker->matrix);
				if (mirrored_flag)
				{
					negate_vector3d(&object_marker->matrix.left, &object_marker->matrix.left);
				}
			}
		}
	}

	return result;
}

/* ---------- private code */
