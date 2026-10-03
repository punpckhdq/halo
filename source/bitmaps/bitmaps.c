/*
BITMAPS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "bitmaps.h"
#include "bitmap_macros.h"
#include "rasterizer_swizzle.h"
#include "s3tc/s3tc.h"
#include "rasterizer_hardware_format_utilities.h"

/* ---------- constants */

enum
{
	COMPRESSED_BLOCK_DIMENSION = 4 /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean bitmap_format_type_valid_width(short format, short type, short width);
static boolean bitmap_format_type_valid_height(short format, short type, short height);
static boolean bitmap_format_type_valid_depth(short format, short type, short depth);

/* ---------- globals */

pixel32 global_vector_palette[PIXEL8_MAXIMUM_COLORS] =
{
	PIXEL32_FROM_ARGB(255, 122, 25, 204), PIXEL32_FROM_ARGB(255, 126, 25, 204), PIXEL32_FROM_ARGB(255, 128, 25, 204), PIXEL32_FROM_ARGB(255, 129, 25, 204),
	PIXEL32_FROM_ARGB(255, 133, 25, 204), PIXEL32_FROM_ARGB(255, 116, 47, 226), PIXEL32_FROM_ARGB(255, 122, 47, 226), PIXEL32_FROM_ARGB(255, 126, 47, 226),
	PIXEL32_FROM_ARGB(255, 128, 47, 226), PIXEL32_FROM_ARGB(255, 129, 47, 226), PIXEL32_FROM_ARGB(255, 133, 47, 226), PIXEL32_FROM_ARGB(255, 139, 47, 226),
	PIXEL32_FROM_ARGB(255, 107, 66, 237), PIXEL32_FROM_ARGB(255, 116, 66, 238), PIXEL32_FROM_ARGB(255, 122, 66, 239), PIXEL32_FROM_ARGB(255, 126, 66, 239),
	PIXEL32_FROM_ARGB(255, 128, 66, 239), PIXEL32_FROM_ARGB(255, 129, 66, 239), PIXEL32_FROM_ARGB(255, 133, 66, 239), PIXEL32_FROM_ARGB(255, 139, 66, 238),
	PIXEL32_FROM_ARGB(255, 148, 66, 237), PIXEL32_FROM_ARGB(255, 96, 82, 242), PIXEL32_FROM_ARGB(255, 107, 82, 245), PIXEL32_FROM_ARGB(255, 116, 82, 246),
	PIXEL32_FROM_ARGB(255, 122, 82, 247), PIXEL32_FROM_ARGB(255, 126, 82, 247), PIXEL32_FROM_ARGB(255, 128, 82, 247), PIXEL32_FROM_ARGB(255, 129, 82, 247),
	PIXEL32_FROM_ARGB(255, 133, 82, 247), PIXEL32_FROM_ARGB(255, 139, 82, 246), PIXEL32_FROM_ARGB(255, 148, 82, 245), PIXEL32_FROM_ARGB(255, 159, 82, 242),
	PIXEL32_FROM_ARGB(255, 82, 96, 242), PIXEL32_FROM_ARGB(255, 96, 96, 247), PIXEL32_FROM_ARGB(255, 107, 96, 249), PIXEL32_FROM_ARGB(255, 116, 96, 251),
	PIXEL32_FROM_ARGB(255, 122, 96, 251), PIXEL32_FROM_ARGB(255, 126, 96, 251), PIXEL32_FROM_ARGB(255, 128, 96, 251), PIXEL32_FROM_ARGB(255, 129, 96, 251),
	PIXEL32_FROM_ARGB(255, 133, 96, 251), PIXEL32_FROM_ARGB(255, 139, 96, 251), PIXEL32_FROM_ARGB(255, 148, 96, 249), PIXEL32_FROM_ARGB(255, 159, 96, 247),
	PIXEL32_FROM_ARGB(255, 173, 96, 242), PIXEL32_FROM_ARGB(255, 66, 107, 237), PIXEL32_FROM_ARGB(255, 82, 107, 245), PIXEL32_FROM_ARGB(255, 96, 107, 249),
	PIXEL32_FROM_ARGB(255, 107, 107, 252), PIXEL32_FROM_ARGB(255, 116, 107, 253), PIXEL32_FROM_ARGB(255, 122, 107, 253), PIXEL32_FROM_ARGB(255, 126, 107, 253),
	PIXEL32_FROM_ARGB(255, 128, 107, 253), PIXEL32_FROM_ARGB(255, 129, 107, 253), PIXEL32_FROM_ARGB(255, 133, 107, 253), PIXEL32_FROM_ARGB(255, 139, 107, 253),
	PIXEL32_FROM_ARGB(255, 148, 107, 252), PIXEL32_FROM_ARGB(255, 159, 107, 249), PIXEL32_FROM_ARGB(255, 173, 107, 245), PIXEL32_FROM_ARGB(255, 189, 107, 237),
	PIXEL32_FROM_ARGB(255, 47, 116, 226), PIXEL32_FROM_ARGB(255, 66, 116, 238), PIXEL32_FROM_ARGB(255, 82, 116, 246), PIXEL32_FROM_ARGB(255, 96, 116, 251),
	PIXEL32_FROM_ARGB(255, 107, 116, 253), PIXEL32_FROM_ARGB(255, 116, 116, 254), PIXEL32_FROM_ARGB(255, 122, 116, 254), PIXEL32_FROM_ARGB(255, 126, 116, 254),
	PIXEL32_FROM_ARGB(255, 128, 116, 254), PIXEL32_FROM_ARGB(255, 129, 116, 254), PIXEL32_FROM_ARGB(255, 133, 116, 254), PIXEL32_FROM_ARGB(255, 139, 116, 254),
	PIXEL32_FROM_ARGB(255, 148, 116, 253), PIXEL32_FROM_ARGB(255, 159, 116, 251), PIXEL32_FROM_ARGB(255, 173, 116, 246), PIXEL32_FROM_ARGB(255, 189, 116, 238),
	PIXEL32_FROM_ARGB(255, 208, 116, 226), PIXEL32_FROM_ARGB(255, 25, 122, 204), PIXEL32_FROM_ARGB(255, 47, 122, 226), PIXEL32_FROM_ARGB(255, 66, 122, 239),
	PIXEL32_FROM_ARGB(255, 82, 122, 247), PIXEL32_FROM_ARGB(255, 96, 122, 251), PIXEL32_FROM_ARGB(255, 107, 122, 253), PIXEL32_FROM_ARGB(255, 116, 122, 254),
	PIXEL32_FROM_ARGB(255, 122, 122, 255), PIXEL32_FROM_ARGB(255, 126, 122, 255), PIXEL32_FROM_ARGB(255, 128, 122, 255), PIXEL32_FROM_ARGB(255, 129, 122, 255),
	PIXEL32_FROM_ARGB(255, 133, 122, 255), PIXEL32_FROM_ARGB(255, 139, 122, 254), PIXEL32_FROM_ARGB(255, 148, 122, 253), PIXEL32_FROM_ARGB(255, 159, 122, 251),
	PIXEL32_FROM_ARGB(255, 173, 122, 247), PIXEL32_FROM_ARGB(255, 189, 122, 239), PIXEL32_FROM_ARGB(255, 208, 122, 226), PIXEL32_FROM_ARGB(255, 229, 122, 204),
	PIXEL32_FROM_ARGB(255, 25, 126, 204), PIXEL32_FROM_ARGB(255, 47, 126, 226), PIXEL32_FROM_ARGB(255, 66, 126, 239), PIXEL32_FROM_ARGB(255, 82, 126, 247),
	PIXEL32_FROM_ARGB(255, 96, 126, 251), PIXEL32_FROM_ARGB(255, 107, 126, 253), PIXEL32_FROM_ARGB(255, 116, 126, 254), PIXEL32_FROM_ARGB(255, 122, 126, 255),
	PIXEL32_FROM_ARGB(255, 126, 126, 255), PIXEL32_FROM_ARGB(255, 128, 126, 255), PIXEL32_FROM_ARGB(255, 129, 126, 255), PIXEL32_FROM_ARGB(255, 133, 126, 255),
	PIXEL32_FROM_ARGB(255, 139, 126, 254), PIXEL32_FROM_ARGB(255, 148, 126, 253), PIXEL32_FROM_ARGB(255, 159, 126, 251), PIXEL32_FROM_ARGB(255, 173, 126, 247),
	PIXEL32_FROM_ARGB(255, 189, 126, 239), PIXEL32_FROM_ARGB(255, 208, 126, 226), PIXEL32_FROM_ARGB(255, 229, 126, 204), PIXEL32_FROM_ARGB(255, 25, 128, 204),
	PIXEL32_FROM_ARGB(255, 47, 128, 226), PIXEL32_FROM_ARGB(255, 66, 128, 239), PIXEL32_FROM_ARGB(255, 82, 128, 247), PIXEL32_FROM_ARGB(255, 96, 128, 251),
	PIXEL32_FROM_ARGB(255, 107, 128, 253), PIXEL32_FROM_ARGB(255, 116, 128, 254), PIXEL32_FROM_ARGB(255, 122, 128, 255), PIXEL32_FROM_ARGB(255, 126, 128, 255),
	PIXEL32_FROM_ARGB(255, 128, 128, 255), PIXEL32_FROM_ARGB(255, 129, 128, 255), PIXEL32_FROM_ARGB(255, 133, 128, 255), PIXEL32_FROM_ARGB(255, 139, 128, 254),
	PIXEL32_FROM_ARGB(255, 148, 128, 253), PIXEL32_FROM_ARGB(255, 159, 128, 251), PIXEL32_FROM_ARGB(255, 173, 128, 247), PIXEL32_FROM_ARGB(255, 189, 128, 239),
	PIXEL32_FROM_ARGB(255, 208, 128, 226), PIXEL32_FROM_ARGB(255, 229, 128, 204), PIXEL32_FROM_ARGB(255, 25, 129, 204), PIXEL32_FROM_ARGB(255, 47, 129, 226),
	PIXEL32_FROM_ARGB(255, 66, 129, 239), PIXEL32_FROM_ARGB(255, 82, 129, 247), PIXEL32_FROM_ARGB(255, 96, 129, 251), PIXEL32_FROM_ARGB(255, 107, 129, 253),
	PIXEL32_FROM_ARGB(255, 116, 129, 254), PIXEL32_FROM_ARGB(255, 122, 129, 255), PIXEL32_FROM_ARGB(255, 126, 129, 255), PIXEL32_FROM_ARGB(255, 128, 129, 255),
	PIXEL32_FROM_ARGB(255, 129, 129, 255), PIXEL32_FROM_ARGB(255, 133, 129, 255), PIXEL32_FROM_ARGB(255, 139, 129, 254), PIXEL32_FROM_ARGB(255, 148, 129, 253),
	PIXEL32_FROM_ARGB(255, 159, 129, 251), PIXEL32_FROM_ARGB(255, 173, 129, 247), PIXEL32_FROM_ARGB(255, 189, 129, 239), PIXEL32_FROM_ARGB(255, 208, 129, 226),
	PIXEL32_FROM_ARGB(255, 229, 129, 204), PIXEL32_FROM_ARGB(255, 25, 133, 204), PIXEL32_FROM_ARGB(255, 47, 133, 226), PIXEL32_FROM_ARGB(255, 66, 133, 239),
	PIXEL32_FROM_ARGB(255, 82, 133, 247), PIXEL32_FROM_ARGB(255, 96, 133, 251), PIXEL32_FROM_ARGB(255, 107, 133, 253), PIXEL32_FROM_ARGB(255, 116, 133, 254),
	PIXEL32_FROM_ARGB(255, 122, 133, 255), PIXEL32_FROM_ARGB(255, 126, 133, 255), PIXEL32_FROM_ARGB(255, 128, 133, 255), PIXEL32_FROM_ARGB(255, 129, 133, 255),
	PIXEL32_FROM_ARGB(255, 133, 133, 255), PIXEL32_FROM_ARGB(255, 139, 133, 254), PIXEL32_FROM_ARGB(255, 148, 133, 253), PIXEL32_FROM_ARGB(255, 159, 133, 251),
	PIXEL32_FROM_ARGB(255, 173, 133, 247), PIXEL32_FROM_ARGB(255, 189, 133, 239), PIXEL32_FROM_ARGB(255, 208, 133, 226), PIXEL32_FROM_ARGB(255, 229, 133, 204),
	PIXEL32_FROM_ARGB(255, 47, 139, 226), PIXEL32_FROM_ARGB(255, 66, 139, 238), PIXEL32_FROM_ARGB(255, 82, 139, 246), PIXEL32_FROM_ARGB(255, 96, 139, 251),
	PIXEL32_FROM_ARGB(255, 107, 139, 253), PIXEL32_FROM_ARGB(255, 116, 139, 254), PIXEL32_FROM_ARGB(255, 122, 139, 254), PIXEL32_FROM_ARGB(255, 126, 139, 254),
	PIXEL32_FROM_ARGB(255, 128, 139, 254), PIXEL32_FROM_ARGB(255, 129, 139, 254), PIXEL32_FROM_ARGB(255, 133, 139, 254), PIXEL32_FROM_ARGB(255, 139, 139, 254),
	PIXEL32_FROM_ARGB(255, 148, 139, 253), PIXEL32_FROM_ARGB(255, 159, 139, 251), PIXEL32_FROM_ARGB(255, 173, 139, 246), PIXEL32_FROM_ARGB(255, 189, 139, 238),
	PIXEL32_FROM_ARGB(255, 208, 139, 226), PIXEL32_FROM_ARGB(255, 66, 148, 237), PIXEL32_FROM_ARGB(255, 82, 148, 245), PIXEL32_FROM_ARGB(255, 96, 148, 249),
	PIXEL32_FROM_ARGB(255, 107, 148, 252), PIXEL32_FROM_ARGB(255, 116, 148, 253), PIXEL32_FROM_ARGB(255, 122, 148, 253), PIXEL32_FROM_ARGB(255, 126, 148, 253),
	PIXEL32_FROM_ARGB(255, 128, 148, 253), PIXEL32_FROM_ARGB(255, 129, 148, 253), PIXEL32_FROM_ARGB(255, 133, 148, 253), PIXEL32_FROM_ARGB(255, 139, 148, 253),
	PIXEL32_FROM_ARGB(255, 148, 148, 252), PIXEL32_FROM_ARGB(255, 159, 148, 249), PIXEL32_FROM_ARGB(255, 173, 148, 245), PIXEL32_FROM_ARGB(255, 189, 148, 237),
	PIXEL32_FROM_ARGB(255, 82, 159, 242), PIXEL32_FROM_ARGB(255, 96, 159, 247), PIXEL32_FROM_ARGB(255, 107, 159, 249), PIXEL32_FROM_ARGB(255, 116, 159, 251),
	PIXEL32_FROM_ARGB(255, 122, 159, 251), PIXEL32_FROM_ARGB(255, 126, 159, 251), PIXEL32_FROM_ARGB(255, 128, 159, 251), PIXEL32_FROM_ARGB(255, 129, 159, 251),
	PIXEL32_FROM_ARGB(255, 133, 159, 251), PIXEL32_FROM_ARGB(255, 139, 159, 251), PIXEL32_FROM_ARGB(255, 148, 159, 249), PIXEL32_FROM_ARGB(255, 159, 159, 247),
	PIXEL32_FROM_ARGB(255, 173, 159, 242), PIXEL32_FROM_ARGB(255, 96, 173, 242), PIXEL32_FROM_ARGB(255, 107, 173, 245), PIXEL32_FROM_ARGB(255, 116, 173, 246),
	PIXEL32_FROM_ARGB(255, 122, 173, 247), PIXEL32_FROM_ARGB(255, 126, 173, 247), PIXEL32_FROM_ARGB(255, 128, 173, 247), PIXEL32_FROM_ARGB(255, 129, 173, 247),
	PIXEL32_FROM_ARGB(255, 133, 173, 247), PIXEL32_FROM_ARGB(255, 139, 173, 246), PIXEL32_FROM_ARGB(255, 148, 173, 245), PIXEL32_FROM_ARGB(255, 159, 173, 242),
	PIXEL32_FROM_ARGB(255, 107, 189, 237), PIXEL32_FROM_ARGB(255, 116, 189, 238), PIXEL32_FROM_ARGB(255, 122, 189, 239), PIXEL32_FROM_ARGB(255, 126, 189, 239),
	PIXEL32_FROM_ARGB(255, 128, 189, 239), PIXEL32_FROM_ARGB(255, 129, 189, 239), PIXEL32_FROM_ARGB(255, 133, 189, 239), PIXEL32_FROM_ARGB(255, 139, 189, 238),
	PIXEL32_FROM_ARGB(255, 148, 189, 237), PIXEL32_FROM_ARGB(255, 116, 208, 226), PIXEL32_FROM_ARGB(255, 122, 208, 226), PIXEL32_FROM_ARGB(255, 126, 208, 226),
	PIXEL32_FROM_ARGB(255, 128, 208, 226), PIXEL32_FROM_ARGB(255, 129, 208, 226), PIXEL32_FROM_ARGB(255, 133, 208, 226), PIXEL32_FROM_ARGB(255, 139, 208, 226),
	PIXEL32_FROM_ARGB(255, 122, 229, 204), PIXEL32_FROM_ARGB(255, 126, 229, 204), PIXEL32_FROM_ARGB(255, 128, 229, 204), PIXEL32_FROM_ARGB(255, 129, 229, 204),
	PIXEL32_FROM_ARGB(255, 133, 229, 204), PIXEL32_FROM_ARGB(0, 0, 0, 0), PIXEL32_FROM_ARGB(0, 0, 0, 0), PIXEL32_FROM_ARGB(0, 0, 0, 0),
	PIXEL32_FROM_ARGB(0, 0, 0, 0), PIXEL32_FROM_ARGB(0, 0, 0, 0), PIXEL32_FROM_ARGB(0, 0, 0, 0), PIXEL32_FROM_ARGB(0, 128, 128, 255)
};

static char *bitmap_type_string_table[NUMBER_OF_BITMAP_TYPES+1] =
{
	"2d texture", // 2d
	"3d texture", // 3d
	"cube map", // cube_map
	NULL
};

static char *bitmap_format_string_table[NUMBER_OF_BITMAP_FORMATS+1] =
{
	"alpha", // a8
	"intensity", // y8
	"combined alpha-intensity", // ay8
	"separate alpha-intensity", // a8y8
	"", // unused1
	"", // unused2
	"high-color", // r5g6b5
	"r6g5b5", // unused3
	"high-color with 1-bit alpha", // a1r5g5b5
	"high-color with alpha", // a4r4g4b4
	"true-color", // x8r8g8b8
	"true-color with alpha", // a8r8g8b8
	"", // unused4
	"", // unused5
	"compressed with color-key transparency", // dxt1
	"compressed with explicit alpha", // dxt3
	"compressed with interpolated alpha", // dxt5
	"palettized bump map", // p8_bump
	NULL
};

static char const bitmap_format_bits_per_pixel_table[NUMBER_OF_BITMAP_FORMATS+1] =
{
	8, // a8
	8, // y8
	8, // ay8
	16, // a8y8
	0, // unused1
	0, // unused2
	16, // r5g6b5
	0, // unused3
	16, // a1r5g5b5
	16, // a4r4g4b4
	32, // x8r8g8b8
	32, // a8r8g8b8
	0, // unused4
	0, // unused5
	4, // dxt1
	8, // dxt3
	8, // dxt5
	8, // p8_bump
	NONE
};

/* ---------- public code */

char *bitmap_type_get_string(
	short type)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 80, type>=0 && type<NUMBER_OF_BITMAP_TYPES);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 81, bitmap_type_string_table[NUMBER_OF_BITMAP_TYPES]==NULL);

	return bitmap_type_string_table[type];
}

char *bitmap_format_get_string(
	short format)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 134, format>=0 && format<NUMBER_OF_BITMAP_FORMATS);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 135, bitmap_format_string_table[NUMBER_OF_BITMAP_FORMATS]==NULL);

	return bitmap_format_string_table[format];
}

short bitmap_format_get_bits_per_pixel(
	short format)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 166, format>=0 && format<NUMBER_OF_BITMAP_FORMATS);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 167, bitmap_format_bits_per_pixel_table[format]!=0);

	return bitmap_format_bits_per_pixel_table[format];
}

struct bitmap_data *bitmap_2d_new(
	short width,
	short height,
	short mipmap_count,
	short format)
{
	struct bitmap_data *bitmap;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 181, bitmap_format_type_valid_width (format, _bitmap_type_2d, width));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 182, bitmap_format_type_valid_height(format, _bitmap_type_2d, height));

	bitmap = match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 184, sizeof(struct bitmap_data));
	if (bitmap)
	{
		memset(bitmap, 0, sizeof(struct bitmap_data));
		bitmap->signature = BITMAP_SIGNATURE;
		bitmap->width = width;
		bitmap->height = height;
		bitmap->depth = 1;
		bitmap->type = _bitmap_type_2d;
		bitmap->format = format;
		bitmap->flags = FLAG(_bitmap_free_on_delete_bit);
		bitmap->mipmap_count = mipmap_count;

		if ((width&(width-1))==0 && (height&(height-1))==0)
		{
			bitmap->flags = FLAG(_bitmap_free_on_delete_bit) | FLAG(_bitmap_has_power_of_two_dimensions_bit);
		}

		if (format>=BITMAP_FIRST_COMPRESSED_FORMAT && format<=BITMAP_LAST_COMPRESSED_FORMAT)
		{
			bitmap->flags |= FLAG(_bitmap_compressed_bit);
		}

		if (format==_bitmap_format_p8_bump)
		{
			bitmap->flags |= FLAG(_bitmap_palettized_bit);
		}

		bitmap->base_address = match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 213, bitmap_get_pixel_data_size(bitmap));
		if (bitmap->base_address)
		{
			match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 217, bitmap_verify(bitmap, FALSE));
		}
		else
		{
			error(_error_silent, "### ERROR failed to allocate bitmap->base_address");
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate bitmap");
	}

	return bitmap;
}

struct bitmap_data *bitmap_3d_new(
	short width,
	short height,
	short depth,
	short mipmap_count,
	short format)
{
	struct bitmap_data *bitmap;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 241, bitmap_format_type_valid_width (format, _bitmap_type_3d, width));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 242, bitmap_format_type_valid_height(format, _bitmap_type_3d, height));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 243, bitmap_format_type_valid_depth (format, _bitmap_type_3d, depth));

	bitmap = match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 245, sizeof(struct bitmap_data));
	if (bitmap)
	{
		memset(bitmap, 0, sizeof(struct bitmap_data));
		bitmap->signature = BITMAP_SIGNATURE;
		bitmap->width = width;
		bitmap->height = height;
		bitmap->depth = depth;
		bitmap->type = _bitmap_type_3d;
		bitmap->format = format;
		bitmap->flags = FLAG(_bitmap_free_on_delete_bit);
		bitmap->mipmap_count = mipmap_count;

		if ((width&(width-1))==0 && (height&(height-1))==0 && (depth&(depth-1))==0)
		{
			bitmap->flags = FLAG(_bitmap_free_on_delete_bit) | FLAG(_bitmap_has_power_of_two_dimensions_bit);
		}

		if (format>=BITMAP_FIRST_COMPRESSED_FORMAT && format<=BITMAP_LAST_COMPRESSED_FORMAT)
		{
			bitmap->flags |= FLAG(_bitmap_compressed_bit);
		}

		if (format==_bitmap_format_p8_bump)
		{
			bitmap->flags |= FLAG(_bitmap_palettized_bit);
		}

		bitmap->base_address = match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 274, bitmap_get_pixel_data_size(bitmap));
		if (bitmap->base_address)
		{
			match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 278, bitmap_verify(bitmap, FALSE));
		}
		else
		{
			error(_error_silent, "### ERROR failed to allocate bitmap->base_address");
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate bitmap");
	}

	return bitmap;
}

struct bitmap_data *bitmap_cube_map_new(
	short width,
	short mipmap_count,
	short format)
{
	struct bitmap_data *bitmap;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 300, bitmap_format_type_valid_width(format, _bitmap_type_cube_map, width));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 301, (width&(width-1))==0);

	bitmap = match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 303, sizeof(struct bitmap_data));
	if (bitmap)
	{
		memset(bitmap, 0, sizeof(struct bitmap_data));
		bitmap->signature = BITMAP_SIGNATURE;
		bitmap->width = width;
		bitmap->height = width;
		bitmap->depth = 1;
		bitmap->type = _bitmap_type_cube_map;
		bitmap->format = format;
		bitmap->mipmap_count = mipmap_count;
		bitmap->hardware_format = NULL;
		bitmap->flags = FLAG(_bitmap_free_on_delete_bit) | FLAG(_bitmap_has_power_of_two_dimensions_bit);

		if (format>=BITMAP_FIRST_COMPRESSED_FORMAT && format<=BITMAP_LAST_COMPRESSED_FORMAT)
		{
			bitmap->flags = FLAG(_bitmap_free_on_delete_bit) | FLAG(_bitmap_has_power_of_two_dimensions_bit) | FLAG(_bitmap_compressed_bit);
		}

		if (format==_bitmap_format_p8_bump)
		{
			bitmap->flags |= FLAG(_bitmap_palettized_bit);
		}

		bitmap->base_address = match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 333, bitmap_get_pixel_data_size(bitmap));
		if (bitmap->base_address)
		{
			match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 337, bitmap_verify(bitmap, FALSE));
		}
		else
		{
			error(_error_silent, "### ERROR failed to allocate bitmap->base_address");
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate bitmap");
	}

	return bitmap;
}

void bitmap_rebuild(
	struct bitmap_data *bitmap)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 355, bitmap);

	if (!bitmap->hardware_format)
	{
		rasterizer_bitmap_new(bitmap);
	}
	rasterizer_bitmap_changed(bitmap);

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 369, bitmap_verify(bitmap, FALSE));

	return;
}

void bitmap_changed(
	struct bitmap_data *bitmap)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 377, bitmap);

	rasterizer_bitmap_changed(bitmap);

	return;
}

void bitmap_delete(
	struct bitmap_data *bitmap)
{
	if (bitmap)
	{
		rasterizer_bitmap_delete(bitmap);

		if (TEST_FLAG(bitmap->flags, _bitmap_free_on_delete_bit))
		{
			if (bitmap->base_address)
			{
				match_free("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 395, bitmap->base_address);
			}
			match_free("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 398, bitmap);
		}
	}

	return;
}

char *bitmap_2d_address(
	struct bitmap_data const *bitmap,
	short x,
	short y,
	short mipmap_index)
{
	short width, height;
	short minimum_dimension;
	long bits_per_pixel;
	short index;
	long offset = 0;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 417, bitmap);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 418, bitmap->base_address);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 419, bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 420, x>=0 && x<bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 421, y>=0 && y<bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 422, mipmap_index>=0 && mipmap_index<=bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 423, !TEST_FLAG(bitmap->flags, _bitmap_compressed_bit) || (x==0 && y==0));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 424, !TEST_FLAG(bitmap->flags, _bitmap_swizzled_bit) || (x==0 && y==0));

	width = bitmap->width;
	height = bitmap->height;
	minimum_dimension = TEST_FLAG(bitmap->flags, _bitmap_compressed_bit) ? COMPRESSED_BLOCK_DIMENSION : 1;
	bits_per_pixel = bitmap_format_get_bits_per_pixel(bitmap->format);

	for (index = 0; index<mipmap_index; index++)
	{
		offset += width*height;
		width = MAX(minimum_dimension, width>>1);
		height = MAX(minimum_dimension, height>>1);
	}

	return (char *)bitmap->base_address + ((offset + y*width + x)*bits_per_pixel)/8;
}

char *bitmap_3d_address(
	struct bitmap_data const *bitmap,
	short x,
	short y,
	short z,
	short mipmap_index)
{
	short width, height, depth;
	short minimum_dimension;
	long bits_per_pixel;
	short index;
	long offset = 0;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 455, bitmap);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 456, bitmap->base_address);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 457, bitmap->type==_bitmap_type_3d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 458, x>=0 && x<bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 459, y>=0 && y<bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 460, z>=0 && z<bitmap->depth);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 461, mipmap_index>=0 && mipmap_index<=bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 462, !TEST_FLAG(bitmap->flags, _bitmap_compressed_bit) || (x==0 && y==0 && z==0));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 463, !TEST_FLAG(bitmap->flags, _bitmap_swizzled_bit) || (x==0 && y==0 && z==0));

	width = bitmap->width;
	height = bitmap->height;
	depth = bitmap->depth;
	minimum_dimension = TEST_FLAG(bitmap->flags, _bitmap_compressed_bit) ? COMPRESSED_BLOCK_DIMENSION : 1;
	bits_per_pixel = bitmap_format_get_bits_per_pixel(bitmap->format);

	for (index = 0; index<mipmap_index; index++)
	{
		offset += width*height*depth;
		width = MAX(minimum_dimension, width>>1);
		height = MAX(minimum_dimension, height>>1);
		depth = MAX(1, depth>>1);
	}

	return (char *)bitmap->base_address + ((offset + (z*height + y)*width + x)*bits_per_pixel)/8;
}

char *bitmap_cube_map_address(
	struct bitmap_data const *bitmap,
	short x,
	short y,
	short face_index,
	short mipmap_index)
{
	short width;
	short minimum_dimension;
	long bits_per_pixel;
	short index;
	long offset = 0;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 496, bitmap);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 497, bitmap->base_address);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 498, bitmap->type==_bitmap_type_cube_map);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 499, x>=0 && x<bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 500, y>=0 && y<bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 501, mipmap_index>=0 && mipmap_index<=bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 502, !TEST_FLAG(bitmap->flags, _bitmap_compressed_bit) || (x==0 && y==0));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 503, !TEST_FLAG(bitmap->flags, _bitmap_swizzled_bit) || (x==0 && y==0));

	width = bitmap->width;
	minimum_dimension = TEST_FLAG(bitmap->flags, _bitmap_compressed_bit) ? COMPRESSED_BLOCK_DIMENSION : 1;
	bits_per_pixel = bitmap_format_get_bits_per_pixel(bitmap->format);

	for (index = 0; index<mipmap_index; index++)
	{
		offset += width*width*NUMBER_OF_FACES_PER_CUBE;
		width = MAX(minimum_dimension, width>>1);
	}

	return (char *)bitmap->base_address + ((offset + (face_index*width + y)*width + x)*bits_per_pixel)/8;
}

void *bitmap_mipmap_address(
	struct bitmap_data const *bitmap,
	short mipmap_index)
{
	void *address;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 525, bitmap);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 526, bitmap->base_address);

	switch (bitmap->type)
	{
	case _bitmap_type_2d:
		address = bitmap_2d_address(bitmap, 0, 0, mipmap_index);
		break;
	case _bitmap_type_3d:
		address = bitmap_3d_address(bitmap, 0, 0, 0, mipmap_index);
		break;
	case _bitmap_type_cube_map:
		address = bitmap_cube_map_address(bitmap, 0, 0, 0, mipmap_index);
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 540, FALSE, "### ERROR unsupported bitmap type");
	}

	return address;
}

pixel32 bitmap_format_to_a8r8g8b8(
	short format,
	void const *mipmap_address,
	long pixel_index)
{
	pixel32 color;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 555, mipmap_address);

	switch (format)
	{
	case _bitmap_format_r5g6b5:
	{
		word pixel = ((word const *)mipmap_address)[pixel_index];

		color = PIXEL16_565_TO_PIXEL32(pixel);
		break;
	}
	case _bitmap_format_a1r5g5b5:
	{
		word pixel = ((word const *)mipmap_address)[pixel_index];

		color = PIXEL16_1555_TO_PIXEL32(pixel);
		break;
	}
	case _bitmap_format_a4r4g4b4:
	{
		word pixel = ((word const *)mipmap_address)[pixel_index];

		color = PIXEL16_4444_TO_PIXEL32(pixel);
		break;
	}
	case _bitmap_format_x8r8g8b8:
		color = ((pixel32 const *)mipmap_address)[pixel_index];
		break;
	case _bitmap_format_a8r8g8b8:
		color = ((pixel32 const *)mipmap_address)[pixel_index];
		break;
	case _bitmap_format_a8:
		color = PIXEL32_FROM_ARGB(((byte const *)mipmap_address)[pixel_index], 0, 0, 0);
		break;
	case _bitmap_format_y8:
	{
		byte intensity = ((byte const *)mipmap_address)[pixel_index];

		color = PIXEL32_FROM_ARGB(0xff, intensity, intensity, intensity);
		break;
	}
	case _bitmap_format_ay8:
	{
		byte intensity = ((byte const *)mipmap_address)[pixel_index];

		color = PIXEL32_FROM_ARGB(intensity, intensity, intensity, intensity);
		break;
	}
	case _bitmap_format_a8y8:
	{
		word pixel = ((word const *)mipmap_address)[pixel_index];
		byte intensity = (byte)pixel;

		color = PIXEL32_FROM_ARGB(PIXEL16_A8Y8_ALPHA(pixel), intensity, intensity, intensity);
		break;
	}
	case _bitmap_format_p8_bump:
		color = global_vector_palette[((byte const *)mipmap_address)[pixel_index]];
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 596, FALSE, "### ERROR unsupported bitmap format");
	}

	return color;
}

pixel32 bitmap_2d_get_pixel(
	struct bitmap_data const *bitmap,
	real_point2d const *point,
	real lod)
{
	pixel32 color;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 609, bitmap);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 610, bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 611, !TEST_FLAG(bitmap->flags, _bitmap_linear_bit));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 612, point);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 613, lod>=0.0f && lod<=1.0f);

	if (bitmap->base_address)
	{
		short mipmap_index;
		short width, height;
		short x, y;
		void *address;

		if (lod<1.0f && bitmap->mipmap_count>0)
		{
			mipmap_index = (short)fast_ftol((1.0f-lod)*bitmap->mipmap_count);
			match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 623, mipmap_index>=0 && mipmap_index<=bitmap->mipmap_count);
		}
		else
		{
			mipmap_index = 0;
		}

		width = bitmap_mipmap_get_width(bitmap, mipmap_index);
		height = bitmap_mipmap_get_height(bitmap, mipmap_index);

		x = (width&(width-1))==0 ? fast_ftol(width*point->x - 0.5f)&(width-1) : (fast_ftol(width*point->x - 0.5f)%width + width)%width;
		y = (height&(height-1))==0 ? fast_ftol(height*point->y - 0.5f)&(height-1) : (fast_ftol(height*point->y - 0.5f)%height + height)%height;

		address = bitmap_mipmap_address(bitmap, mipmap_index);

		if (TEST_FLAG(bitmap->flags, _bitmap_compressed_bit))
		{
			short block_size = (16*bitmap_format_get_bits_per_pixel(bitmap->format))/8;
			short block_x = x/COMPRESSED_BLOCK_DIMENSION;
			short block_y = y/COMPRESSED_BLOCK_DIMENSION;
			byte *block = (byte *)address + block_size*(width*block_y/COMPRESSED_BLOCK_DIMENSION + block_x);

			x &= COMPRESSED_BLOCK_DIMENSION-1;
			y &= COMPRESSED_BLOCK_DIMENSION-1;

			match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 672, block>=(byte *)bitmap->base_address,
				csprintf(temporary, "bitmap_2d_get_pixel tried to access compressed block @ -%d bytes from address start (w=%d, h=%d, m=%d, x=%d, y=%d, lod=%f)",
					(byte *)bitmap->base_address-block, bitmap->width, bitmap->height, bitmap->mipmap_count,
					fast_ftol(width*point->x - 0.5f)%width, fast_ftol(height*point->y - 0.5f)%height, mipmap_index));
			match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 681, block<(byte *)bitmap->base_address+bitmap->pixels_size,
				csprintf(temporary, "bitmap_2d_get_pixel tried to access compressed block @ -%d bytes from address end (w=%d, h=%d, m=%d, x=%d, y=%d, lod=%f)",
					block-(byte *)bitmap->base_address-bitmap->pixels_size, bitmap->width, bitmap->height, bitmap->mipmap_count,
					fast_ftol(width*point->x - 0.5f)%width, fast_ftol(height*point->y - 0.5f)%height, mipmap_index));

			switch (bitmap->format)
			{
			case _bitmap_format_dxt1:
				DecodeBlockRGB__single_pixel((struct S3TCBlockRGB const *)block, (struct S3TC_COLOR *)&color, x, y);
				break;
			case _bitmap_format_dxt3:
				DecodeBlockAlpha4__single_pixel((struct S3TCBlockAlpha4 const *)block, (struct S3TC_COLOR *)&color, x, y);
				break;
			case _bitmap_format_dxt5:
				DecodeBlockAlpha3__single_pixel((struct S3TCBlockAlpha3 const *)block, (struct S3TC_COLOR *)&color, x, y);
				break;
			default:
				match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 695, FALSE, "### ERROR unsupported bitmap format");
			}
		}
		else
		{
			long pixel_index;

			if (TEST_FLAG(bitmap->flags, _bitmap_swizzled_bit))
			{
				long offsets[2];

				match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 705, x>=0 && x<4096);
				match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 706, y>=0 && y<4096);

				bitmap_swizzle_vector2d(width, height, x, y, offsets);
				pixel_index = offsets[0] | offsets[1];
			}
			else
			{
				pixel_index = y*width + x;
			}

			color = bitmap_format_to_a8r8g8b8(bitmap->format, address, pixel_index);
		}
	}
	else
	{
		color = NONE;
	}

	return color;
}

void bitmap_3d_slice_extract(
	struct bitmap_data const *source_bitmap,
	short source_mipmap_index,
	short source_slice_index,
	struct bitmap_data *slice_bitmap)
{
	long pixel_data_size;
	void const *source_address;
	void *destination_address;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 739, bitmap_verify(source_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 740, source_bitmap->type==_bitmap_type_3d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 741, source_mipmap_index>=0 && source_mipmap_index<=source_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 742, source_slice_index>=0 && source_slice_index<source_bitmap->depth);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 743, MAX(1, source_bitmap->width >>source_mipmap_index)==slice_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 744, MAX(1, source_bitmap->height>>source_mipmap_index)==slice_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 745, !TEST_FLAG(source_bitmap->flags, _bitmap_swizzled_bit));

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 747, bitmap_verify(slice_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 748, slice_bitmap->mipmap_count==0);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 749, slice_bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 750, slice_bitmap->format==source_bitmap->format);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 751, !TEST_FLAG(slice_bitmap->flags, _bitmap_swizzled_bit));

	pixel_data_size = bitmap_get_pixel_data_size(slice_bitmap);
	source_address = bitmap_3d_address(source_bitmap, 0, 0, source_slice_index, source_mipmap_index);
	destination_address = bitmap_mipmap_address(slice_bitmap, 0);
	memcpy(destination_address, source_address, pixel_data_size);

	return;
}

void bitmap_3d_slice_insert(
	struct bitmap_data const *slice_bitmap,
	struct bitmap_data *destination_bitmap,
	short destination_mipmap_index,
	short destination_slice_index)
{
	long pixel_data_size;
	void const *source_address;
	void *destination_address;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 774, bitmap_verify(slice_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 775, slice_bitmap->mipmap_count==0);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 776, slice_bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 777, slice_bitmap->format==destination_bitmap->format);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 778, !TEST_FLAG(slice_bitmap->flags, _bitmap_swizzled_bit));

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 780, bitmap_verify(destination_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 781, destination_bitmap->type==_bitmap_type_3d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 782, destination_mipmap_index>=0 && destination_mipmap_index<=destination_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 783, destination_slice_index>=0 && destination_slice_index<destination_bitmap->depth);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 784, MAX(1, destination_bitmap->width >>destination_mipmap_index)==slice_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 785, MAX(1, destination_bitmap->height>>destination_mipmap_index)==slice_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 786, !TEST_FLAG(destination_bitmap->flags, _bitmap_swizzled_bit));

	pixel_data_size = bitmap_get_pixel_data_size(slice_bitmap);
	destination_address = bitmap_3d_address(destination_bitmap, 0, 0, destination_slice_index, destination_mipmap_index);
	source_address = bitmap_mipmap_address(slice_bitmap, 0);
	memcpy(destination_address, source_address, pixel_data_size);

	return;
}

void bitmap_cube_map_face_extract(
	struct bitmap_data const *source_bitmap,
	short source_mipmap_index,
	short source_face_index,
	struct bitmap_data *face_bitmap)
{
	long pixel_data_size;
	void const *source_address;
	void *destination_address;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 809, bitmap_verify(source_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 810, source_bitmap->type==_bitmap_type_cube_map);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 811, source_mipmap_index>=0 && source_mipmap_index<=source_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 812, source_face_index>=0 && source_face_index<NUMBER_OF_FACES_PER_CUBE);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 813, MAX(1, source_bitmap->width >>source_mipmap_index)==face_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 814, MAX(1, source_bitmap->height>>source_mipmap_index)==face_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 815, !TEST_FLAG(source_bitmap->flags, _bitmap_swizzled_bit));

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 817, bitmap_verify(face_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 818, face_bitmap->mipmap_count==0);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 819, face_bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 820, face_bitmap->format==source_bitmap->format);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 821, !TEST_FLAG(face_bitmap->flags, _bitmap_swizzled_bit));

	pixel_data_size = bitmap_get_pixel_data_size(face_bitmap);
	source_address = bitmap_cube_map_address(source_bitmap, 0, 0, source_face_index, source_mipmap_index);
	destination_address = bitmap_mipmap_address(face_bitmap, 0);
	memcpy(destination_address, source_address, pixel_data_size);

	return;
}

void bitmap_cube_map_face_insert(
	struct bitmap_data const *face_bitmap,
	struct bitmap_data *destination_bitmap,
	short destination_mipmap_index,
	short destination_face_index)
{
	long pixel_data_size;
	void const *source_address;
	void *destination_address;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 844, bitmap_verify(face_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 845, face_bitmap->mipmap_count==0);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 846, face_bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 847, face_bitmap->format==destination_bitmap->format);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 848, !TEST_FLAG(face_bitmap->flags, _bitmap_swizzled_bit));

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 850, bitmap_verify(destination_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 851, destination_bitmap->type==_bitmap_type_cube_map);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 852, destination_mipmap_index>=0 && destination_mipmap_index<=destination_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 853, destination_face_index>=0 && destination_face_index<NUMBER_OF_FACES_PER_CUBE);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 854, MAX(1, destination_bitmap->width >>destination_mipmap_index)==face_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 855, MAX(1, destination_bitmap->height>>destination_mipmap_index)==face_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 856, !TEST_FLAG(destination_bitmap->flags, _bitmap_swizzled_bit));

	pixel_data_size = bitmap_get_pixel_data_size(face_bitmap);
	destination_address = bitmap_cube_map_address(destination_bitmap, 0, 0, destination_face_index, destination_mipmap_index);
	source_address = bitmap_mipmap_address(face_bitmap, 0);
	memcpy(destination_address, source_address, pixel_data_size);

	return;
}

short bitmap_get_max_mipmap_count(
	struct bitmap_data const *bitmap)
{
	short mipmap_count = 0;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 872, bitmap_verify(bitmap, FALSE));

	if (TEST_FLAG(bitmap->flags, _bitmap_has_power_of_two_dimensions_bit))
	{
		mipmap_count = floor_log2(MAX(bitmap->width, MAX(bitmap->height, bitmap->depth)));
	}

	return mipmap_count;
}

long bitmap_get_pixel_count(
	struct bitmap_data const *bitmap)
{
	short mipmap_index;
	long pixel_count = 0;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 888, bitmap_verify(bitmap, FALSE));

	for (mipmap_index = 0; mipmap_index<=bitmap->mipmap_count; mipmap_index++)
	{
		pixel_count +=bitmap_mipmap_get_pixel_count(bitmap, mipmap_index);
	}

	return pixel_count;
}

long bitmap_get_pixel_data_size(
	struct bitmap_data const *bitmap)
{
	long pixel_count;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 906, bitmap_verify(bitmap, FALSE));

	pixel_count = bitmap_get_pixel_count(bitmap);

	return (pixel_count*bitmap_format_get_bits_per_pixel(bitmap->format))/8;
}

short bitmap_mipmap_get_width(
	struct bitmap_data const *bitmap,
	short mipmap_index)
{
	short width;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 923, bitmap_verify(bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 924, mipmap_index>=0 && mipmap_index<=bitmap->mipmap_count);

	width = MAX(bitmap->width>>mipmap_index, 1);
	if (TEST_FLAG(bitmap->flags, _bitmap_compressed_bit))
	{
		width +=(-width)&(COMPRESSED_BLOCK_DIMENSION-1);
	}

	return width;
}

short bitmap_mipmap_get_height(
	struct bitmap_data const *bitmap,
	short mipmap_index)
{
	short height;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 942, bitmap_verify(bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 943, mipmap_index>=0 && mipmap_index<=bitmap->mipmap_count);

	height = MAX(bitmap->height>>mipmap_index, 1);
	if (TEST_FLAG(bitmap->flags, _bitmap_compressed_bit))
	{
		height +=(-height)&(COMPRESSED_BLOCK_DIMENSION-1);
	}

	return height;
}

short bitmap_mipmap_get_depth(
	struct bitmap_data const *bitmap,
	short mipmap_index)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 959, bitmap_verify(bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 960, mipmap_index>=0 && mipmap_index<=bitmap->mipmap_count);

	return MAX(bitmap->depth>>mipmap_index, 1);
}

long bitmap_mipmap_get_pixel_count(
	struct bitmap_data const *bitmap,
	short mipmap_index)
{
	short width, height, depth;
	long pixel_count;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 971, bitmap_verify(bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 972, mipmap_index>=0 && mipmap_index<=bitmap->mipmap_count);

	width = bitmap_mipmap_get_width(bitmap, mipmap_index);
	height = bitmap_mipmap_get_height(bitmap, mipmap_index);
	depth = bitmap_mipmap_get_depth(bitmap, mipmap_index);
	pixel_count = width*height*depth;
	if (bitmap->type==_bitmap_type_cube_map)
	{
		pixel_count *=NUMBER_OF_FACES_PER_CUBE;
	}

	return pixel_count;
}

long bitmap_mipmap_get_pixel_data_size(
	struct bitmap_data const *bitmap,
	short mipmap_index)
{
	long pixel_count;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 995, bitmap_verify(bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 996, mipmap_index>=0 && mipmap_index<=bitmap->mipmap_count);

	pixel_count = bitmap_mipmap_get_pixel_count(bitmap, mipmap_index);

	return (pixel_count*bitmap_format_get_bits_per_pixel(bitmap->format))/8;
}

long bitmap_mipmap_get_row_pitch(
	struct bitmap_data const *bitmap,
	short mipmap_index)
{
	short width;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 1013, bitmap_verify(bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 1014, mipmap_index>=0 && mipmap_index<=bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 1015, !TEST_FLAG(bitmap->flags, _bitmap_compressed_bit));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 1016, !TEST_FLAG(bitmap->flags, _bitmap_swizzled_bit));

	width = bitmap_mipmap_get_width(bitmap, mipmap_index);

	return (width*bitmap_format_get_bits_per_pixel(bitmap->format))/8;
}

void bitmap_byte_swap_pixels(
	struct bitmap_data *bitmap)
{
	return;
}

boolean bitmap_verify(
	struct bitmap_data const *bitmap,
	boolean import)
{
	boolean valid = TRUE;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 1039, bitmap);

	if (bitmap->signature==BITMAP_SIGNATURE &&
		!(bitmap->flags & ~MASK(NUMBER_OF_BITMAP_FLAGS)) &&
		bitmap->type>=0 && bitmap->type<NUMBER_OF_BITMAP_TYPES &&
		bitmap->format>=0 && bitmap->format<NUMBER_OF_BITMAP_FORMATS &&
		bitmap_format_type_valid_width(bitmap->format, bitmap->type, bitmap->width) &&
		bitmap_format_type_valid_height(bitmap->format, bitmap->type, bitmap->height) &&
		bitmap_format_type_valid_depth(bitmap->format, bitmap->type, bitmap->depth) &&
		bitmap->mipmap_count>=0 &&
		bitmap->mipmap_count<=floor_log2(MAX(bitmap->width, MAX(bitmap->height, bitmap->depth))))
	{
		if (import &&
			!(bitmap->format==_bitmap_format_a8r8g8b8 &&
			bitmap->base_address &&
			bitmap->mipmap_count==0 &&
			!TEST_FLAG(bitmap->flags, _bitmap_compressed_bit) &&
			!TEST_FLAG(bitmap->flags, _bitmap_palettized_bit) &&
			!TEST_FLAG(bitmap->flags, _bitmap_swizzled_bit)))
		{
			error(_error_silent, "### ERROR bitmap @%p (#%dx#%d) appears to be invalid for import", bitmap, bitmap->width, bitmap->height);
			valid = FALSE;
		}
	}
	else
	{
		error(_error_silent, "### ERROR bitmap @%p (#%dx#%d) appears to be invalid", bitmap, bitmap->width, bitmap->height);
		valid = FALSE;
	}

	return valid;
}

byte palette_find_closest_match(
	pixel32 const *palette,
	pixel32 color)
{
	short closest_match_index = NONE;
	long closest_match_distance = 0;

	if (PIXEL32_ALPHA_BITS(color)<=0x80000000)
	{
		closest_match_index = PIXEL8_MAXIMUM_COLORS-1;
	}
	else
	{
		short palette_index;

		for (palette_index = 0; palette_index<PIXEL8_MAXIMUM_COLORS; palette_index++)
		{
			long red_difference, green_difference, blue_difference;
			long distance;
			pixel32 const *palette_color = &palette[palette_index];

			if (!*palette_color)
			{
				break;
			}

			red_difference = ABS((long)PIXEL32_RED(*palette_color) - (long)PIXEL32_RED(color));
			green_difference = ABS((long)PIXEL32_GREEN(*palette_color) - (long)PIXEL32_GREEN(color));
			blue_difference = ABS((long)PIXEL32_BLUE(*palette_color) - (long)PIXEL32_BLUE(color));

			distance = blue_difference*blue_difference + green_difference*green_difference + red_difference*red_difference;
			if (palette_index==0 || closest_match_distance>distance)
			{
				closest_match_distance = distance;
				closest_match_index = palette_index;
			}
		}

		match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps.c", 1101, closest_match_index!=NONE);
	}

	return (byte)closest_match_index;
}

/* ---------- private code */

static boolean bitmap_format_type_valid_width(
	short format,
	short type,
	short width)
{
	return width>0 && width<=MAXIMUM_BITMAP_WIDTH;
}

static boolean bitmap_format_type_valid_height(
	short format,
	short type,
	short height)
{
	return height>0 && height<=MAXIMUM_BITMAP_HEIGHT;
}

static boolean bitmap_format_type_valid_depth(
	short format,
	short type,
	short depth)
{
	return depth>0 && depth<=MAXIMUM_BITMAP_DEPTH && (depth==1 || type==_bitmap_type_3d);
}
