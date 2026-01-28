/*
MODEL_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __MODEL_DEFINITIONS_H
#define __MODEL_DEFINITIONS_H
#pragma once


/* ---------- headers */

#include "math/real_math.h"
#include "tag_files/tag_files.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

enum
{
	MODELS_GROUP_TAG= 'mode',
	MODELS_GROUP_VERSION= 4,
	MAXIMUM_MARKERS_PER_MODEL= 256,
	MAXIMUM_INSTANCES_PER_MODEL_MARKER= 32,
	MAXIMUM_GEOMETRIES_PER_MODEL= 256,
	MAXIMUM_PARTS_PER_MODEL_GEOMETRY= 32,
	MAXIMUM_VERTICES_PER_MODEL_GEOMETRY_PART= UNSIGNED_SHORT_MAX,
	MAXIMUM_TRIANGLES_PER_MODEL_GEOMETRY_PART= UNSIGNED_SHORT_MAX,
	MAXIMUM_NODES_PER_MODEL_GEOMETRY_PART= 22,
	PADDED_MAXIMUM_NODES_PER_MODEL_GEOMETRY_PART= 24,
	MAXIMUM_REGIONS_PER_MODEL= 32,
	MAXIMUM_PERMUTATIONS_PER_MODEL_REGION= 32,
	MAXIMUM_MARKERS_PER_MODEL_REGION_PERMUTATION= 64,
	MAXIMUM_NODES_PER_MODEL= 64,
	MAXIMUM_SHADERS_PER_MODEL= 32,
	MAXIMUM_FUNCTION_VALUES_PER_MODEL= 4,
	MAXIMUM_CHANGE_COLORS_PER_MODEL= 4,
};

enum
{
	_model_region_permutation_cannot_be_chosen_randomly_bit= 0,
	NUMBER_OF_MODEL_REGION_PERMUTATION_FLAGS,
};

/* ---------- macros */

#define model_definition_get(index) ((struct model *)tag_get(MODELS_GROUP_TAG, index))

/* ---------- structures */

struct model_marker_instance
{
	byte region_index;
	byte permutation_index;
	byte node_index;
	byte pad;
	real_point3d translation;
	real_quaternion rotation;
};

struct model_marker
{
	char name[32];
	long unused[5];
	struct tag_block instances;		// model_marker_instance
};

struct model_node
{
	char name[TAG_STRING_LENGTH+1];
	short next_sibling_node_index;
	short first_child_node_index;
	short parent_node_index;
	word pad;
	real_point3d default_translation;
	real_quaternion default_rotation;
	real node_distance_from_parent;
	long unused[8];
	real_matrix4x3 runtime_default_inverse_matrix;
};

struct model_region_permutation
{
	char name[TAG_STRING_LENGTH+1];
	long flags;
	short variant_number;
	word pad;
	long unused[6];
	short geometry_indices[5];
	word pad2;
	struct tag_block markers;				// object_marker
};

struct model_region
{
	char name[TAG_STRING_LENGTH+1];
	long unused[8];
	struct tag_block permutations;			// model_region_permutation
};

struct model
{
	unsigned long flags;
	unsigned long node_list_checksum;
	real detail_cutoff_pixels[5];
	short node_counts[5];
	word pad;
	long unused1[2];
	real_vector2d base_map_scale;
	long unused2[29];
	struct tag_block markers;			// model_marker
	struct tag_block nodes;				// model_node
	struct tag_block regions;			// model_region
	struct tag_block geometries;
	struct tag_block shaders;
};

/* ---------- prototypes/MODEL_DEFINITIONS.C */

/* ---------- globals */

/* ---------- public code */

#endif // __MODEL_DEFINITIONS_H
