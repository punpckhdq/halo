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
	BITMAP_SIGNATURE = 'bitm',
	BITMAP_MAXIMUM_SPRITE_PAGE_MIPMAP_COUNT = 2,
	NUMBER_OF_ENTRIES_IN_PALETTE = 256
};

enum
{
	MAXIMUM_BITMAP_WIDTH = 30000,
	MAXIMUM_BITMAP_HEIGHT = 30000,
	MAXIMUM_BITMAP_DEPTH = 256
};

enum
{
	_bitmap_type_2d = 0,
	_bitmap_type_3d,
	_bitmap_type_cube_map,
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
	NUMBER_OF_BITMAP_FORMATS,

	BITMAP_FIRST_COMPRESSED_FORMAT = _bitmap_format_dxt1,
	BITMAP_LAST_COMPRESSED_FORMAT = _bitmap_format_dxt5
};

enum
{
	_bitmap_has_power_of_two_dimensions_bit = 0,
	_bitmap_compressed_bit,
	_bitmap_palettized_bit,
	_bitmap_swizzled_bit,
	_bitmap_linear_bit,
	_bitmap_format_v16u16_bit,
	NUMBER_OF_ON_DISK_BITMAP_FLAGS,

	_bitmap_free_on_delete_bit = NUMBER_OF_ON_DISK_BITMAP_FLAGS,
	_bitmap_cached_bit,
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
	word flags;
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

char *bitmap_type_get_string(short type);
char *bitmap_format_get_string(short format);
short bitmap_format_get_bits_per_pixel(short format);
void bitmap_changed(struct bitmap_data *bitmap);
void bitmap_delete(struct bitmap_data *bitmap);
char *bitmap_2d_address(struct bitmap_data const *bitmap, short x, short y, short mipmap_index);
char *bitmap_3d_address(struct bitmap_data const *bitmap, short x, short y, short z, short mipmap_index);
char *bitmap_cube_map_address(struct bitmap_data const *bitmap, short x, short y, short face_index, short mipmap_index);
void *bitmap_mipmap_address(struct bitmap_data const *bitmap, short mipmap_index);
pixel32 bitmap_format_to_a8r8g8b8(short format, void const *mipmap_address, long pixel_index);
void bitmap_byte_swap_pixels(struct bitmap_data *bitmap);
byte palette_find_closest_match(pixel32 const *palette, pixel32 color);
boolean bitmap_verify(struct bitmap_data const *bitmap, boolean import);
void bitmap_rebuild(struct bitmap_data *bitmap);
short bitmap_get_max_mipmap_count(struct bitmap_data const *bitmap);
short bitmap_mipmap_get_width(struct bitmap_data const *bitmap, short mipmap_index);
short bitmap_mipmap_get_height(struct bitmap_data const *bitmap, short mipmap_index);
short bitmap_mipmap_get_depth(struct bitmap_data const *bitmap, short mipmap_index);
long bitmap_mipmap_get_pixel_count(struct bitmap_data const *bitmap, short mipmap_index);
long bitmap_mipmap_get_pixel_data_size(struct bitmap_data const *bitmap, short mipmap_index);
long bitmap_mipmap_get_row_pitch(struct bitmap_data const *bitmap, short mipmap_index);
pixel32 bitmap_2d_get_pixel(struct bitmap_data const *bitmap, real_point2d const *point, real lod);
long bitmap_get_pixel_count(struct bitmap_data const *bitmap);
long bitmap_get_pixel_data_size(struct bitmap_data const *bitmap);
struct bitmap_data *bitmap_2d_new(short width, short height, short mipmap_count, short format);
struct bitmap_data *bitmap_3d_new(short width, short height, short depth, short mipmap_count, short format);
struct bitmap_data *bitmap_cube_map_new(short width, short mipmap_count, short format);
void bitmap_3d_slice_extract(struct bitmap_data const *source_bitmap, short source_mipmap_index, short source_slice_index, struct bitmap_data *slice_bitmap);
void bitmap_3d_slice_insert(struct bitmap_data const *slice_bitmap, struct bitmap_data *destination_bitmap, short destination_mipmap_index, short destination_slice_index);
void bitmap_cube_map_face_extract(struct bitmap_data const *source_bitmap, short source_mipmap_index, short source_face_index, struct bitmap_data *face_bitmap);
void bitmap_cube_map_face_insert(struct bitmap_data const *face_bitmap, struct bitmap_data *destination_bitmap, short destination_mipmap_index, short destination_face_index);

/* ---------- prototypes/BITMAP_UTILITIES.C */

void bitmap_fill(struct bitmap_data *bitmap, pixel32 color);
void bitmap_alpha_to_rgb(struct bitmap_data *bitmap);
struct bitmap_data *bitmap_clone(struct bitmap_data const *source_bitmap);
struct bitmap_data *bitmap_shrink(struct bitmap_data const *source_bitmap, short scale, short alpha_bias, boolean ignore_zero_alpha);
void bitmap_fade(struct bitmap_data *bitmap, pixel32 fade_color, real fade_amount);
void bitmap_smooth(struct bitmap_data *bitmap, real filter_size);
void bitmap_sharpen(struct bitmap_data *bitmap, real sharpen_amount);
void bitmap_alpha_bleed(struct bitmap_data *bitmap, short passes);
void bitmap_height_map(struct bitmap_data *bitmap, real bump_height);
void bitmap_vector_map(struct bitmap_data *bitmap);
void bitmap_compress_to_mipmap(struct bitmap_data const *source_bitmap, struct bitmap_data *destination_bitmap, short destination_mipmap_index, pixel32 const *transparent_color);
void bitmap_uncompress_from_mipmap(struct bitmap_data const *source_bitmap, struct bitmap_data *destination_bitmap, short source_mipmap_index);
real real_rgb_color_brightness(union real_rgb_color const *color);
union hsv_color *rgb_color_to_hsv_color(union rgb_color const *rgb, union hsv_color *hsv);
union rgb_color *hsv_color_to_rgb_color(union hsv_color const *hsv, union rgb_color *rgb);
union real_hsv_color *real_rgb_color_to_real_hsv_color(union real_rgb_color const *rgb, union real_hsv_color *hsv);
union real_rgb_color *real_hsv_color_to_real_rgb_color(union real_hsv_color const *hsv, union real_rgb_color *rgb);
union real_argb_color *argb_color_to_real_argb_color(union argb_color const *argb, union real_argb_color *real_argb);
union real_rgb_color *rgb_color_to_real_rgb_color(union rgb_color const *rgb, union real_rgb_color *real_rgb);
union real_argb_color *pixel32_to_real_argb_color(pixel32 pixel, union real_argb_color *color);
union real_rgb_color *pixel32_to_real_rgb_color(pixel32 color, union real_rgb_color *result);

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

/* ---------- prototypes/BITMAPS_QUANTITIZE.C */

void bitmap_quantitize(struct bitmap_data *bitmap, short const *bits_per_channel);

/* ---------- prototypes/TARGA_FILE.C */

char *targa_export(struct file_reference *file, struct bitmap_data const *bitmap);

/* ---------- prototypes/TIFF_FILE.C */

boolean tiff_get_bounds(struct file_reference *file, long *width, long *height);
char *tiff_export(struct file_reference *file, struct bitmap_data const *bitmap);
char *tiff_import(struct file_reference *file, struct bitmap_data **bitmap, rectangle2d const *bounds, short format);

/* ---------- globals */

extern pixel32 global_vector_palette[NUMBER_OF_ENTRIES_IN_PALETTE];

extern short bits_per_channel_r5g6b5[];
extern short bits_per_channel_a1r5g5b5[];
extern short bits_per_channel_a4r4g4b4[];

/* ---------- public code */

#endif // __BITMAPS_H
