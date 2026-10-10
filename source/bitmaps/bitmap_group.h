/*
BITMAP_GROUP.H

header included in hcex build.
*/

#ifndef __BITMAP_GROUP_H
#define __BITMAP_GROUP_H
#pragma once

/* ---------- constants */

enum
{
	BITMAP_GROUP_TAG = 'bitm',
	BITMAP_GROUP_VERSION = 7,
	BITMAP_GROUP_SHOW_BITMAP_CUSTOM_ID = 'bshw',
	MAXIMUM_BITMAP_PIXELS_SIZE = 0x1000000
};

enum
{
	MAXIMUM_BITMAPS_PER_BITMAP_GROUP = 2048,
	MAXIMUM_SEQUENCES_PER_BITMAP_GROUP = 256,
	MAXIMUM_SPRITES_PER_SEQUENCE = 64,
	MAXIMUM_PIXEL_DATA_SIZE = 0x1000000
};

enum
{
	_bitmap_group_diffusion_dither_bit = 0,
	_bitmap_group_disable_vector_compression_bit,
	_bitmap_group_uniform_sprite_sequences_bit,
	_bitmap_group_extract_sprites_filthy_bug_fix_bit,
	NUMBER_OF_BITMAP_GROUP_FLAGS
};

enum
{
	_bitmap_group_type_2d_textures = 0,
	_bitmap_group_type_3d_textures,
	_bitmap_group_type_cube_maps,
	_bitmap_group_type_sprites,
	_bitmap_group_type_interface_bitmaps,
	NUMBER_OF_BITMAP_GROUP_TYPES
};

enum
{
	_bitmap_group_format_compressed_color_key_transparency = 0,
	_bitmap_group_format_compressed_explicit_alpha,
	_bitmap_group_format_compressed_interpolated_alpha,
	_bitmap_group_format_16bit_color,
	_bitmap_group_format_32bit_color,
	_bitmap_group_format_monochrome,
	NUMBER_OF_BITMAP_GROUP_FORMATS
};

enum
{
	_bitmap_group_usage_alpha_blend = 0,
	_bitmap_group_usage_default,
	_bitmap_group_usage_height_map,
	_bitmap_group_usage_detail_map,
	_bitmap_group_usage_light_map,
	_bitmap_group_usage_vector_map,
	NUMBER_OF_BITMAP_GROUP_USAGES
};

enum
{
	_bitmap_group_sprite_budget_32 = 0,
	_bitmap_group_sprite_budget_64,
	_bitmap_group_sprite_budget_128,
	_bitmap_group_sprite_budget_256,
	_bitmap_group_sprite_budget_512,
	NUMBER_OF_BITMAP_GROUP_SPRITE_BUDGETS
};

enum
{
	_bitmap_group_sprite_usage_blend_add_sub_max = 0,
	_bitmap_group_sprite_usage_mul_min,
	_bitmap_group_sprite_usage_double_multiply,
	NUMBER_OF_BITMAP_GROUP_SPRITE_USAGES
};

/* ---------- macros */

#define bitmap_group_get(index) ((struct bitmap_group *)tag_get(BITMAP_GROUP_TAG, (index))) /* fake name */

/* ---------- structures */

struct bitmap_group_sprite
{
	short bitmap_index;
	short bitmap_pad;
	long unused;
	real_rectangle2d bounds;
	real_point2d registration_point;
};

struct bitmap_group_sequence
{
	char name[TAG_STRING_LENGTH+1];
	short first_bitmap_index;
	short bitmap_count;
	long unused[4];
	struct tag_block sprites;		// bitmap_group_sprite
};

struct bitmap_group
{
	short type;
	short format;
	short usage;
	word flags;
	real detail_fade;
	real sharpen_amount;
	real bump_height;
	short sprite_budget_size;
	short sprite_budget_count;
	short import_width;
	short import_height;
	struct tag_data import_bitmap;
	struct tag_data pixel_data;
	real smoothing_filter_size;
	real alpha_bias;
	short mipmap_count;
	short sprite_usage;
	short sprite_spacing;
	word pad;
	struct tag_block sequences;		// bitmap_group_sequence
	struct tag_block bitmaps;		// bitmap_data
};

/* ---------- prototypes/BITMAP_GROUP.C */

struct bitmap_data *bitmap_group_try_and_get_bitmap(long bitmap_group_index, short bitmap_index);
struct bitmap_data *bitmap_group_get_bitmap_from_sequence(long bitmap_group_index, short sequence_index, short frame_index);
short bitmap_group_add_bitmap(struct bitmap_group *group, short width, short height, short depth, short type, short format, short mipmap_count);

/* ---------- prototypes/BITMAP_EXTRACT.C */

struct bitmap_data *extract_build_debug_plate(struct bitmap_data const *bitmap, boolean alpha_to_rgb, boolean include_mipmaps, boolean border);
boolean bitmaps_extract(struct bitmap_group *group, char const *debug_plate_name);
boolean bitmaps_extract_from_plate(struct bitmap_data const *plate, struct bitmap_group *group, char const *debug_plate_name);

/* ---------- globals */

extern struct tag_reference_definition global_bitmap_reference;
extern struct tag_reference_definition global_bitmap_reference_optional;
extern struct tag_data_definition bitmap_pixel_data;
extern struct tag_data_definition color_plate_data;
extern struct tag_group bitmap_group;

/* ---------- public code */

#endif // __BITMAP_GROUP_H
