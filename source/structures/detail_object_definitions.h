/*
DETAIL_OBJECT_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __DETAIL_OBJECT_DEFINITIONS_H
#define __DETAIL_OBJECT_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	DETAIL_OBJECT_COLLECTION_DEFINITION_TAG = 'dobc' /* fake name */
};

enum
{
	DETAIL_OBJECT_CELL_SIZE = 8,
	DETAIL_OBJECT_CELL_GRANULARITY = 255,
	MAXIMUM_DETAIL_OBJECT_CELLS_PER_AXIS = 3,
	MAXIMUM_DETAIL_OBJECT_CELLS_PER_FRAME = 27,
	MAXIMUM_DETAIL_OBJECT_LAYERS_PER_STRUCTURE = 32,
	MAXIMUM_DETAIL_OBJECT_LOCAL_PLAYERS = 2,	/* fake name */
};

enum
{
	MAXIMUM_DETAIL_OBJECT_TYPES_PER_COLLECTION = 16,
	MAXIMUM_DETAIL_OBJECT_SPRITES_PER_COLLECTION = 128
};

enum
{
	_structure_detail_object_data_valid_bit = 0,	/* fake name */
	_structure_detail_object_data_dirty_bit,		/* fake name */
	NUMBER_OF_STRUCTURE_DETAIL_OBJECT_DATA_FLAGS	/* fake name */
};

/* ---------- macros */

#define detail_object_collection_definition_get(index) ((struct detail_object_collection_definition *)tag_get(DETAIL_OBJECT_COLLECTION_DEFINITION_TAG, (index))) /* fake name */

/* ---------- structures */

struct detail_object_type_definition
{
	char name[TAG_STRING_LENGTH+1];
	byte sequence_index;
	byte flags;
	byte first_frame_index;
	byte frame_count;
	real color_override_factor;
	long unused1[2];
	real near_fade_distance;
	real far_fade_distance;
	real size_min;
	real size_max;
	real_rgb_color color_min;
	real_rgb_color color_max;
	pixel32 color_ambient;
	long unused2[1];
};

struct detail_object_collection_definition
{
	short collection_type;
	word pad;
	real global_z_offset;
	long unused1[11];
	struct tag_reference map;				// bitmap_group
	struct tag_block type_definitions;		// detail_object_type_definition
	long unused2[12];
};

struct detail_object_cell_definition
{
	short cell_x;
	short cell_y;
	short cell_z;
	short offset_z;
	unsigned long valid_layers;
	long start_index;
	long count_index;
	long unused[3];
};

struct detail_object
{
	byte position[3];
	byte data;
	word color;
};

struct structure_detail_object_data
{
	struct tag_block cells;									// detail_object_cell_definition
	struct tag_block detail_objects;						// detail_object
	struct tag_block detail_objects_counts;					// short
	struct tag_block detail_object_z_reference_vectors;		// real_vector4d
	byte valid;
	byte pad[3];
	long unused[3];
};

struct detail_object_cell_data
{
	long first_detail_object_index;
	long detail_object_count;
	short cell_x;
	short cell_y;
	real cell_z;
	long internal__first_vertex_index;
	real_vector4d const *z_reference_vector;
};

struct detail_object_layer_data
{
	struct detail_object_cell_data *cells;
	short cell_count;
	short collection_definition_index;
};

struct detail_object_view_data
{
	struct detail_object_layer_data *layers;
	short layer_count;
	short pad;
};

struct detail_object_cell_coord
{
	short x;
	short y;
	short z;
	boolean initialized;
};

struct detail_object_runtime_data
{
	struct detail_object_cell_data cells[MAXIMUM_DETAIL_OBJECT_LAYERS_PER_STRUCTURE][MAXIMUM_DETAIL_OBJECT_CELLS_PER_FRAME];
	struct detail_object_layer_data layers[MAXIMUM_DETAIL_OBJECT_LAYERS_PER_STRUCTURE];
	struct detail_object_view_data view_data;
	struct detail_object_cell_coord last_cell_coord;
};

struct detail_object_global_runtime_data
{
	struct detail_object_runtime_data local_player_data[MAXIMUM_DETAIL_OBJECT_LOCAL_PLAYERS];
	real_vector4d default_z_reference_vector;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __DETAIL_OBJECT_DEFINITIONS_H
