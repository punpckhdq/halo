/*
TAG_GROUPS.H

header included in hcex build.
*/

#ifndef __TAG_GROUPS_H
#define __TAG_GROUPS_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	_field_string = 0,
	_field_char_integer,
	_field_short_integer,
	_field_long_integer,
	_field_angle,
	_field_tag,
	_field_enum,
	_field_flags,
	_field_word_flags,
	_field_byte_flags,
	_field_point2d,
	_field_rectangle2d,
	_field_rgb_pixel32,
	_field_argb_pixel32,
	_field_real,
	_field_real_fraction,
	_field_real_point2d,
	_field_real_point3d,
	_field_real_vector2d,
	_field_real_vector3d,
	_field_real_quaternion,
	_field_real_euler_angles2d,
	_field_real_euler_angles3d,
	_field_real_plane2d,
	_field_real_plane3d,
	_field_real_rgb_color,
	_field_real_argb_color,
	_field_real_hsv_color,
	_field_real_ahsv_color,
	_field_short_integer_bounds,
	_field_angle_bounds,
	_field_real_bounds,
	_field_real_fraction_bounds,
	_field_tag_reference,
	_field_block,
	_field_short_block_index,
	_field_long_block_index,
	_field_data,
	_field_start_array,
	_field_end_array,
	_field_pad,
	_field_skip,
	_field_explanation,
	_field_custom,
	_field_terminator,
	NUMBER_OF_TAG_FIELD_TYPES
};

enum
{
	_tag_data_cached_bit = 0,
	_tag_data_string_bit,
	_tag_data_not_added_to_monolithic_file_bit,
	NUMBER_OF_TAG_DATA_DEFINITION_FLAGS
};

enum
{
	_tag_group_is_game_critical_bit = 0,
	_tag_group_is_local_bit,
	_tag_group_is_not_editable_bit,
	_tag_group_can_be_reloaded_bit,
	NUMBER_OF_TAG_GROUP_FLAGS
};

/* ---------- macros */

#define TAG_BLOCK_GET_ELEMENT(block_address, index, type) ((type *)tag_block_get_element_with_size((block_address), (index), sizeof(type)))

/* ---------- structures */

typedef void (*byte_swap_block_proc)(void *);
typedef boolean (*postprocess_block_proc)(void *, boolean);
typedef byte *(*format_block_proc)(long, struct tag_block *, long, byte *);
typedef void (*delete_block_proc)(struct tag_block *, long);

struct tag_field
{
	short type;
	char *name;
	void *definition;
};

struct flags_definition
{
	short count;
	char **strings;
};

struct enum_definition
{
	short count;
	char **strings;
	union real_rgb_color *colors;
};

struct tag_block_definition
{
	char *name;
	unsigned long flags;
	long maximum_element_count;
	long element_size;
	void *default_element;
	struct tag_field *fields;
	byte_swap_block_proc byte_swap_block;
	postprocess_block_proc postprocess_block;
	format_block_proc format_block;
	delete_block_proc delete_block;
	byte_swap_code *byte_swap_codes;
};

struct tag_block
{
	long count;
	void *address;
	struct tag_block_definition *definition;
};

struct tag_reference
{
	unsigned long group_tag;
	char *name;
	long name_length;
	long index;
};

struct tag_reference_definition
{
    unsigned long flags;
    unsigned long group_tag;
    unsigned long *group_tags;
};

struct tag_data
{
	long size;
	unsigned long pad;
	long file_offset;
	void *address;
	struct tag_data_definition *definition;
};

typedef void (*byte_swap_data_proc)(struct tag_data *, void *); /* fake name */
typedef boolean (*postprocess_tag_proc)(long, boolean); /* fake name */

struct tag_data_definition
{
	char *name;
	unsigned long flags;
	long maximum_size;
	byte_swap_data_proc byte_swap_data;
};

struct tag_group
{
	char *name;
	unsigned long flags;
	unsigned long group_tag;
	unsigned long parent_tag;
	short version;
	postprocess_tag_proc postprocess_tag;
	struct tag_block_definition *header_block_definition;
	unsigned long child_tags[16];
	short child_count;
};

/* ---------- prototypes/TAG_GROUPS.C */

void *tag_data_get_pointer(struct tag_data const *data, long offset, long size);
void *tag_block_get_element_with_size(struct tag_block const *block, long index, long element_size);
long tag_block_add_element(struct tag_block *block);
boolean tag_block_resize(struct tag_block *block, long element_count);
boolean tag_data_resize(struct tag_data *data, long size);

/* ---------- prototypes/CACHE_FILES.C */

long tag_loaded(long group_tag, const char *name);

void *tag_get(long group_tag, long tag_index);

/* ---------- globals */

/* ---------- public code */

__inline void *tag_data_get_address(
	struct tag_data const *data)
{
	return data->address;
}

#endif // __TAG_GROUPS_H
