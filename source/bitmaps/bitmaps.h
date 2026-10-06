/*
BITMAPS.H

header included in hcex build.
*/

#ifndef __BITMAPS_H
#define __BITMAPS_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	_bitmap_type_2d = 0,
	_bitmap_type_3d,
	_bitmap_type_cube_map,
	_bitmap_type_white,
	NUMBER_OF_BITMAP_TYPES
};

enum
{
	_bitmap_format_a8 = 0,
	_bitmap_format_y8,
	_bitmap_format_ay8,
	_bitmap_format_a8y8,
	_bitmap_format_unused1,
	_bitmap_format_unused2,
	_bitmap_format_r5g6b5,
	_bitmap_format_unused3,
	_bitmap_format_a1r5g5b5,
	_bitmap_format_a4r4g4b4,
	_bitmap_format_x8r8g8b8,
	_bitmap_format_a8r8g8b8,
	_bitmap_format_unused4,
	_bitmap_format_unused5,
	_bitmap_format_dxt1,
	_bitmap_format_dxt3,
	_bitmap_format_dxt5,
	_bitmap_format_p8_bump,
	NUMBER_OF_BITMAP_FORMATS
};

enum
{
	_bitmap_has_power_of_two_dimensions_bit = 0,
	_bitmap_compressed_bit,
	_bitmap_palettized_bit,
	_bitmap_swizzled_bit,
	_bitmap_linear_bit,
	_bitmap_format_v16u16_bit,
	_bitmap_free_on_delete_bit,
	_bitmap_cached_bit,
	_bitmap_data_file_cache_bit,
	NUMBER_OF_BITMAP_FLAGS
};

enum
{
	_bitmap_usage_additive = 0,
	_bitmap_usage_multiplicative,
	_bitmap_usage_detail,
	_bitmap_usage_vector,
	NUMBER_OF_BITMAP_USAGES
};

/* ---------- macros */

/* ---------- structures */

struct bitmap_data
{
	unsigned long signature;
	short width;
	short height;
	short depth;
	short type;
	short format;
	unsigned short flags;
	point2d registration_point;
	short mipmap_count;
	short mipmap_pad;
	long pixels_offset;
	long pixels_size;
	long tag_index;
	long cache_block_index;
	void *hardware_format;
	void *base_address;
};

/* ---------- prototypes/BITMAPS.C */

struct bitmap_data *bitmap_2d_new(short width, short height, short mipmap_count, short format);
void bitmap_delete(struct bitmap_data *bitmap);
char *tiff_export(struct file_reference *file, struct bitmap_data *in_bitmap);

void *bitmap_2d_address(struct bitmap_data const *bitmap, short x, short y, short mipmap_index);
short bitmap_format_get_bits_per_pixel(short format);
char *bitmap_cube_map_address(struct bitmap_data const *bitmap, short x, short y, short face_index, short mipmap_index);
void *bitmap_mipmap_address(struct bitmap_data const *bitmap, short mipmap_index);
short bitmap_mipmap_get_width(struct bitmap_data const *bitmap, short mipmap_index);
short bitmap_mipmap_get_height(struct bitmap_data const *bitmap, short mipmap_index);
short bitmap_mipmap_get_depth(struct bitmap_data const *bitmap, short mipmap_index);
long bitmap_mipmap_get_pixel_data_size(struct bitmap_data const *bitmap, short mipmap_index);
long bitmap_mipmap_get_row_pitch(struct bitmap_data const *bitmap, short mipmap_index);
boolean bitmap_verify(struct bitmap_data const *bitmap, boolean verify_data);

/* ---------- prototypes/BITMAP_UTILITIES.C */

real real_rgb_color_brightness(union real_rgb_color const *color);
union real_argb_color *pixel32_to_real_argb_color(pixel32 pixel, union real_argb_color *color);

union real_rgb_color *rgb_colors_interpolate(
	union real_rgb_color *rgb_result,
	unsigned long flags,
	union real_rgb_color const *rgb_lower_bound,
	union real_rgb_color const *rgb_upper_bound,
	real u);
union real_rgb_color *rgb_colors_interpolate_and_scale(
	union real_rgb_color *rgb_result,
	unsigned long flags,
	union real_argb_color const *argb_lower_bound,
	union real_argb_color const *argb_upper_bound,
	union real_rgb_color const *rgb_scale,
	real u);

/* ---------- prototypes/BITMAP_UTILITIES.C */

union real_rgb_color *pixel32_to_real_rgb_color(pixel32 color, union real_rgb_color *result);

/* ---------- globals */

/* ---------- public code */

#endif // __BITMAPS_H
