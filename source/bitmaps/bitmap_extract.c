/*
BITMAP_EXTRACT.C
*/

/* ---------- headers */

#include "cseries.h"
#include "bitmaps.h"
#include "bitmap_group.h"
#include "bitmap_drawing.h"
#include "bitmap_macros.h"
#include "texture_page.h"

/* ---------- constants */

enum
{
	MAXIMUM_TEMPORARY_BITMAPS = 1024, /* fake name */
	MAXIMUM_SPRITE_TEXTURE_PAGES = 32, /* fake name */
	MINIMUM_SPRITE_TEXTURE_PAGE_DIMENSION = 32, /* fake name */
	MAXIMUM_SPRITE_TEXTURE_PAGE_DIMENSION = 512 /* fake name */
};

enum
{
	_row_state_searching = 0, /* fake name */
	_row_state_in_bitmap, /* fake name */
	_row_state_done /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct temporary_bitmap /* fake name */
{
	struct bitmap_data *bitmap;
	short sequence_index;
	short sprite_index;
	short texture_page_index;
	long texture_index;
};

struct cube_map_face_source /* fake name */
{
	short x_block;
	short y_block;
	short x_edge;
	short y_edge;
	short x_column_delta;
	short y_column_delta;
	short x_row_delta;
	short y_row_delta;
};

struct extract_data /* fake name */
{
	struct temporary_bitmap *temporary_bitmaps;
	short temporary_bitmap_count;
	pixel32 background_color;
	pixel32 sequence_divider_color;
	pixel32 dummy_space_color;
	boolean valid_plate;
	boolean no_sequence_dividers;
	struct bitmap_group *group;
	struct bitmap_data *plate;
	char const *debug_plate_name;
	struct bitmap_group_sequence *sequence;
	short sequence_index;
	short bitmap_index;
};

/* ---------- prototypes */

static void preprocess_plate(void);
static boolean extract_no_plate(void);
static short extract_find_row_bottom(short *top_reference);
static void extract_verify_unbroken_horizontal_border(short bottom);
static boolean extract_bitmaps_in_row(short top, short bottom);
static boolean extract_plate(void);
static boolean extract_plateless_cube_map(struct bitmap_data const *bitmap);
static boolean extract_bitmap(rectangle2d const *bounds);
static boolean extract_adjust_bounds(rectangle2d const *bounds, rectangle2d *adjusted_bounds_reference);
static short extract_get_bitmap_format(struct bitmap_data const *bitmap);
static short extract_bitmap_to_group(struct bitmap_data const *bitmap);
static void extract_mipmaps_to_bitmap(struct bitmap_data const *source_bitmap, struct bitmap_data *destination_bitmap);
static void extract_pixels_to_mipmap(struct bitmap_data const *source_bitmap, struct bitmap_data *destination_bitmap, short destination_mipmap_index);
static void extract_pixels_from_mipmap(struct bitmap_data const *source_bitmap, struct bitmap_data *destination_bitmap, short source_mipmap_index);
static void build_texture_pages_by_sequence(struct texture_page **texture_pages, short *texture_page_count, short page_size, short spacing);
static boolean process_3d_bitmaps(void);
static boolean process_cube_maps(void);
static boolean process_sprites(void);

/* ---------- globals */

static struct extract_data extract_data;

/* ---------- public code */

boolean bitmaps_extract_from_plate(
	struct bitmap_data const *plate,
	struct bitmap_group *group,
	char const *debug_plate_name)
{
	unsigned long compressed_plate_size;
	void *compressed_plate;
	boolean success;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 131, bitmap_verify(plate, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 132, group);

	compressed_plate_size = bitmap_get_pixel_data_size(plate);
	group->import_width = plate->width;
	group->import_height = plate->height;
	group->import_bitmap.address = match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 138, compressed_plate_size);

	if (group->import_bitmap.address)
	{
		if (data_compress(
			bitmap_mipmap_address(plate, 0),
			compressed_plate_size,
			group->import_bitmap.address,
			&compressed_plate_size,
			compressed_plate_size))
		{
			compressed_plate = match_realloc("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 144, group->import_bitmap.address, compressed_plate_size);

			if (compressed_plate)
			{
				group->import_bitmap.address = compressed_plate;
				group->import_bitmap.size = compressed_plate_size;
				success = bitmaps_extract(group, debug_plate_name);
			}
			else
			{
				error(_error_silent, "### ERROR extract: failed to realloc color plate");
				success = FALSE;
			}
		}
		else
		{
			error(_error_silent, "### ERROR extract: failed to compress color plate");
			success = FALSE;
		}
	}
	else
	{
		error(_error_silent, "### ERROR extract: failed to allocate temporary buffer");
		success = FALSE;
	}

	return success;
}

boolean bitmaps_extract(
	struct bitmap_group *group,
	char const *debug_plate_name)
{
	unsigned long decompressed_plate_size;
	boolean valid_plate;
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 180, group);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 181, group->type >=0 && group->type <NUMBER_OF_BITMAP_GROUP_TYPES);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 182, group->format>=0 && group->format<NUMBER_OF_BITMAP_GROUP_FORMATS);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 183, group->usage >=0 && group->usage <NUMBER_OF_BITMAP_GROUP_USAGES);

	extract_data.temporary_bitmap_count = 0;
	extract_data.temporary_bitmaps = match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 186, MAXIMUM_TEMPORARY_BITMAPS * sizeof(*extract_data.temporary_bitmaps));

	if (!extract_data.temporary_bitmaps)
	{
		error(_error_silent, "### ERROR extract: failed to allocate bitmap array");
		success = FALSE;
	}

	if (!success ||
		!tag_block_resize(&group->bitmaps, 0) ||
		!tag_block_resize(&group->sequences, 0) ||
		!tag_data_resize(&group->pixel_data, 0))
	{
		error(_error_silent, "### ERROR extract: failed to resize bitmap group tags to zero");
		success = FALSE;
	}
	else
	{
		extract_data.sequence = NULL;
		extract_data.sequence_index = NONE;
		extract_data.group = group;
		extract_data.plate = bitmap_2d_new(group->import_width, group->import_height, 0, _bitmap_format_a8r8g8b8);

		if (extract_data.plate)
		{
			decompressed_plate_size = data_decompressed_size(group->import_bitmap.address, group->import_bitmap.size);
			match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 260, decompressed_plate_size==sizeof(pixel32)*group->import_width*group->import_height);

			if (data_decompress(
				group->import_bitmap.address,
				group->import_bitmap.size,
				bitmap_mipmap_address(extract_data.plate, 0),
				&decompressed_plate_size,
				decompressed_plate_size))
			{
				preprocess_plate();
				valid_plate = extract_data.valid_plate;
				extract_data.debug_plate_name = debug_plate_name;

				if (valid_plate)
				{
					success = extract_plate();
				}
				else
				{
					success = extract_no_plate();
				}

				bitmap_delete(extract_data.plate);
			}
			else
			{
				error(_error_silent, "### ERROR extract: failed to decompress color plate");
				success = FALSE;
			}
		}
		else
		{
			error(_error_silent, "### ERROR extract: failed to allocate color plate");
			success = FALSE;
		}
	}

	if (success)
	{
		switch (extract_data.group->type)
		{
		case _bitmap_group_type_2d_textures:
		case _bitmap_group_type_interface_bitmaps:
			break;
		case _bitmap_group_type_3d_textures:
			success = process_3d_bitmaps();
			break;
		case _bitmap_group_type_cube_maps:
			success = process_cube_maps();
			break;
		case _bitmap_group_type_sprites:
			success = process_sprites();
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 311, FALSE, "### ERROR unsupported bitmap group type");
			break;
		}
	}

	if (extract_data.temporary_bitmaps)
	{
		match_free("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 317, extract_data.temporary_bitmaps);
	}

	return success;
}

static void preprocess_plate(
	void)
{
	short x;

	extract_data.valid_plate = TRUE;
	extract_data.no_sequence_dividers = FALSE;
	extract_data.background_color = PIXEL32_RGB_BITS(*(pixel32 *)bitmap_2d_address(extract_data.plate, 0, 0, 0));
	extract_data.sequence_divider_color = PIXEL32_RGB_BITS(*(pixel32 *)bitmap_2d_address(extract_data.plate, 1, 0, 0));
	extract_data.dummy_space_color = PIXEL32_RGB_BITS(*(pixel32 *)bitmap_2d_address(extract_data.plate, 2, 0, 0));

	if (extract_data.dummy_space_color == extract_data.sequence_divider_color &&
		extract_data.sequence_divider_color != PIXEL32_FROM_ARGB(0, 0, 0, 255))
	{
		extract_data.valid_plate = FALSE;
	}

	if (extract_data.background_color == extract_data.sequence_divider_color)
	{
		extract_data.dummy_space_color = PIXEL32_FROM_ARGB(0, 0, 255, 255);
		extract_data.no_sequence_dividers = TRUE;
	}

	for (x = 3; x < extract_data.plate->width; x++)
	{
		pixel32 top_color = PIXEL32_RGB_BITS(*(pixel32 *)bitmap_2d_address(extract_data.plate, x, 0, 0));
		pixel32 bottom_color = PIXEL32_RGB_BITS(*(pixel32 *)bitmap_2d_address(extract_data.plate, x, 1, 0));

		if (top_color != extract_data.background_color && bottom_color != extract_data.sequence_divider_color)
		{
			extract_data.valid_plate = FALSE;
		}
	}

	if (!extract_data.valid_plate)
	{
		extract_data.dummy_space_color = PIXEL32_ALPHA_MASK;
		extract_data.sequence_divider_color = PIXEL32_ALPHA_MASK;
		extract_data.background_color = PIXEL32_ALPHA_MASK;
	}

	return;
}

static boolean extract_no_plate(
	void)
{
	rectangle2d bounds;
	struct bitmap_group *group = extract_data.group;
	boolean success = TRUE;

	if (group->format == _bitmap_group_format_compressed_color_key_transparency && extract_data.valid_plate)
	{
		error(_error_silent, "### ERROR extract: compressed color-key transparency format must use a valid plate");
		success = FALSE;
	}
	else
	{
		switch (group->type)
		{
		case _bitmap_group_type_2d_textures:
		case _bitmap_group_type_cube_maps:
		case _bitmap_group_type_interface_bitmaps:
			{
				short plate_width;
				short plate_height;
				short sequence_index;

				bounds.y0 = 0;
				bounds.x0 = 0;
				plate_width = extract_data.plate->width;
				plate_height = extract_data.plate->height;
				bounds.x1 = plate_width;
				bounds.y1 = plate_height;
				sequence_index = (short)tag_block_add_element(&group->sequences);
				extract_data.sequence_index = sequence_index;
				extract_data.sequence = TAG_BLOCK_GET_ELEMENT(&extract_data.group->sequences, sequence_index, struct bitmap_group_sequence);
				extract_data.sequence->first_bitmap_index = NONE;
				extract_bitmap(&bounds);
				break;
			}
		case _bitmap_group_type_3d_textures:
			error(_error_silent, "### ERROR can't extract 3D textures without a valid plate");
			success = FALSE;
			break;
		case _bitmap_group_type_sprites:
			error(_error_silent, "### ERROR can't extract sprites without a valid plate");
			success = FALSE;
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 422, FALSE, "### ERROR unsupported bitmap group type");
			break;
		}
	}

	return success;
}

static short extract_find_row_bottom(
	short *top_reference)
{
	short bottom;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 473, top_reference);

	if (extract_data.no_sequence_dividers)
	{
		boolean found_bitmap_row = FALSE;

		for (bottom = *top_reference; bottom < extract_data.plate->height; bottom++)
		{
			short x;
			boolean row_has_data = FALSE;

			for (x = 0; x < extract_data.plate->width; x++)
			{
				pixel32 color = PIXEL32_RGB_BITS(*(pixel32 *)bitmap_2d_address(extract_data.plate, x, bottom, 0));

				if (color != extract_data.background_color)
				{
					row_has_data = TRUE;
				}
			}

			if (row_has_data)
			{
				found_bitmap_row = TRUE;
			}
			else if (found_bitmap_row)
			{
				break;
			}
			else
			{
				*top_reference = bottom + 1;
			}
		}
	}
	else
	{
		boolean found_background = FALSE;

		for (bottom = *top_reference; bottom < extract_data.plate->height; bottom++)
		{
			pixel32 color = PIXEL32_RGB_BITS(*(pixel32 *)bitmap_2d_address(extract_data.plate, 0, bottom, 0));

			if (color == extract_data.background_color)
			{
				found_background = TRUE;
			}
			else if (color == extract_data.sequence_divider_color && found_background)
			{
				break;
			}
			else
			{
				*top_reference = bottom + 1;
			}
		}
	}

	return bottom;
}

static void extract_verify_unbroken_horizontal_border(
	short bottom)
{
	if (bottom >= 0 && bottom < extract_data.plate->height)
	{
		short x;

		for (x = 0; x < extract_data.plate->width; x++)
		{
			if (PIXEL32_RGB_BITS(*(pixel32 *)bitmap_2d_address(extract_data.plate, x, bottom, 0)) != extract_data.sequence_divider_color)
			{
				fprintf(stdout, "### WARNING horizontal border broken at (#%d,#%d)\r\n", x, bottom);
				fflush(stdout);
				break;
			}
		}
	}

	return;
}

static boolean extract_bitmaps_in_row(
	short top,
	short bottom)
{
	short x;
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 564, top>=0);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 565, bottom>=top);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 566, bottom<=extract_data.plate->height);

	x = 0;

	while (success && x < extract_data.plate->width)
	{
		rectangle2d bounds;
		short state = _row_state_searching;

		bounds.x0 = SHORT_MAX;
		bounds.y0 = top;
		bounds.x1 = SHORT_MIN;
		bounds.y1 = bottom;

		while (x < extract_data.plate->width && state != _row_state_done)
		{
			short y;
			boolean found_bitmap = FALSE;
			boolean found_sequence_divider = FALSE;

			for (y = top; y < bottom; y++)
			{
				pixel32 color = PIXEL32_RGB_BITS(*(pixel32 *)bitmap_2d_address(extract_data.plate, x, y, 0));

				if (color == extract_data.sequence_divider_color)
				{
					found_sequence_divider = TRUE;
				}
				else if (color != extract_data.background_color)
				{
					found_bitmap = TRUE;

					switch (state)
					{
					case _row_state_searching:
						state = _row_state_in_bitmap;
						bounds.x0 = x;
					case _row_state_in_bitmap:
						bounds.y0 = MIN(y, bounds.y0);
						bounds.y1 = MAX(y, bounds.y1);
						bounds.x1 = x;
						break;
					}
				}
			}

			if ((found_sequence_divider || extract_data.no_sequence_dividers) && !found_bitmap)
			{
				if (state == _row_state_in_bitmap)
				{
					state = _row_state_done;
				}
			}

			if (state == _row_state_in_bitmap && !found_bitmap)
			{
				state = _row_state_done;
			}

			x++;
		}

		if (state != _row_state_searching)
		{
			rectangle2d adjusted_bounds;

			bounds.x1++;
			bounds.y1++;
			adjusted_bounds = bounds;

			if (TEST_FLAG(extract_data.group->flags, _bitmap_group_extract_sprites_filthy_bug_fix_bit))
			{
				short trim_y;

				for (trim_y = adjusted_bounds.y0; trim_y < adjusted_bounds.y1; trim_y++)
				{
					short trim_x;

					for (trim_x = adjusted_bounds.x0; trim_x < adjusted_bounds.x1; trim_x++)
					{
						pixel32 color = PIXEL32_RGB_BITS(*(pixel32 *)bitmap_2d_address(extract_data.plate, trim_x, trim_y, 0));

						if (color != extract_data.background_color)
						{
							break;
						}
					}

					if (trim_x < adjusted_bounds.x1)
					{
						break;
					}
				}

				adjusted_bounds.y0 = trim_y;

				for (trim_y = adjusted_bounds.y1 - 2; trim_y >= adjusted_bounds.y0; trim_y--)
				{
					short trim_x;

					for (trim_x = adjusted_bounds.x0; trim_x < adjusted_bounds.x1; trim_x++)
					{
						pixel32 color = PIXEL32_RGB_BITS(*(pixel32 *)bitmap_2d_address(extract_data.plate, trim_x, trim_y, 0));

						if (color != extract_data.background_color)
						{
							break;
						}
					}

					if (trim_x < adjusted_bounds.x1)
					{
						break;
					}
				}

				adjusted_bounds.y1 = trim_y + 1;
			}

			success = extract_bitmap(&adjusted_bounds);
		}
	}

	return success;
}

static boolean extract_plate(
	void)
{
	boolean success = TRUE;
	short top = 1;

	while (success && top < extract_data.plate->height)
	{
		short bottom;
		short sequence_index;

		bottom = extract_find_row_bottom(&top);
		extract_verify_unbroken_horizontal_border(bottom);
		sequence_index = (short)tag_block_add_element(&extract_data.group->sequences);

		if (sequence_index != NONE)
		{
			extract_data.sequence_index = sequence_index;
			extract_data.sequence = TAG_BLOCK_GET_ELEMENT(&extract_data.group->sequences, sequence_index, struct bitmap_group_sequence);
			extract_data.sequence->first_bitmap_index = NONE;
			extract_data.sequence->bitmap_count = 0;
			success = extract_bitmaps_in_row(top, bottom);
			top = bottom + 1;
		}
		else
		{
			error(_error_silent, "### ERROR extract: failed to allocate sequence");
			success = FALSE;
		}
	}

	return success;
}

static boolean extract_plateless_cube_map(
	struct bitmap_data const *bitmap)
{
	short face_size;
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 706, bitmap_verify(bitmap, TRUE));

	if (!(bitmap->width % 4) && bitmap->height >= 3 * (bitmap->width / 4) && !(bitmap->width & (bitmap->width - 1)))
	{
		face_size = bitmap->width / 4;

		if (extract_data.temporary_bitmap_count + NUMBER_OF_FACES_PER_CUBE <= MAXIMUM_TEMPORARY_BITMAPS)
		{
			short face_index;
			struct cube_map_face_source faces[NUMBER_OF_FACES_PER_CUBE] =
			{
				{ 0, 1, 1, 0, 0, 1, -1, 0 },
				{ 1, 1, 1, 1, -1, 0, 0, -1 },
				{ 2, 1, 0, 1, 0, -1, 1, 0 },
				{ 3, 1, 0, 0, 1, 0, 0, 1 },
				{ 0, 0, 1, 0, 0, 1, -1, 0 },
				{ 0, 2, 1, 0, 0, 1, -1, 0 }
			};

			for (face_index = 0; face_index < NUMBEROF(faces); face_index++)
			{
				struct temporary_bitmap *temporary_bitmap = &extract_data.temporary_bitmaps[extract_data.temporary_bitmap_count++];

				temporary_bitmap->bitmap = bitmap_2d_new(face_size, face_size, 0, _bitmap_format_a8r8g8b8);

				if (temporary_bitmap->bitmap)
				{
					short y;

					for (y = 0; y < face_size; y++)
					{
						short x;
						short source_x = faces[face_index].x_block * face_size + faces[face_index].x_edge * (face_size - 1) + y * faces[face_index].x_row_delta;
						short source_y = faces[face_index].y_block * face_size + faces[face_index].y_edge * (face_size - 1) + y * faces[face_index].y_row_delta;

						for (x = 0; x < face_size; x++)
						{
							*(pixel32 *)bitmap_2d_address(temporary_bitmap->bitmap, x, y, 0) = *(pixel32 *)bitmap_2d_address(bitmap, source_x, source_y, 0);
							source_x += faces[face_index].x_column_delta;
							source_y += faces[face_index].y_column_delta;
						}
					}

					temporary_bitmap->sequence_index = extract_data.sequence_index;
					temporary_bitmap->sprite_index = NONE;
					temporary_bitmap->texture_page_index = NONE;
					temporary_bitmap->texture_index = NONE;
				}
				else
				{
					error(_error_silent, "### ERROR extract: failed to allocate temporary bitmap");
					success = FALSE;
				}
			}
		}
		else
		{
			error(_error_silent, "### ERROR extract: can't handle more than (#%d) temporary bitmaps", MAXIMUM_TEMPORARY_BITMAPS);
			success = FALSE;
		}
	}
	else
	{
		error(_error_silent, "### ERROR extract: plateless cube map had invalid dimensions #%dx#%d", bitmap->width, bitmap->height);
		success = FALSE;
	}

	return success;
}

static boolean extract_bitmap(
	rectangle2d const *bounds)
{
	rectangle2d adjusted_bounds;
	struct bitmap_data *bitmap;
	short y;
	boolean success = TRUE;
	boolean warned_about_dxt1_alpha = FALSE;
	boolean warned_about_zero_alpha = FALSE;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 802, bounds);

	if (extract_adjust_bounds(bounds, &adjusted_bounds))
	{
		bitmap = bitmap_2d_new(adjusted_bounds.x1 - adjusted_bounds.x0, adjusted_bounds.y1 - adjusted_bounds.y0, 0, _bitmap_format_a8r8g8b8);

		if (bitmap)
		{
			if (TEST_FLAG(extract_data.group->flags, _bitmap_group_extract_sprites_filthy_bug_fix_bit))
			{
				bitmap->registration_point.x = bounds->x0 + bounds->x1 - 2 * adjusted_bounds.x0;
				bitmap->registration_point.y = bounds->y0 + bounds->y1 - 2 * adjusted_bounds.y0;
			}
			else
			{
				bitmap->registration_point.x = (bounds->x0 + bounds->x1) / 2 - adjusted_bounds.x0;
				bitmap->registration_point.y = (bounds->y0 + bounds->y1) / 2 - adjusted_bounds.y0;
			}

			for (y = adjusted_bounds.y0; y < adjusted_bounds.y1; y++)
			{
				short x;
				pixel32 *destination = (pixel32 *)bitmap_2d_address(bitmap, 0, y - adjusted_bounds.y0, 0);

				for (x = adjusted_bounds.x0; x < adjusted_bounds.x1; x++)
				{
					pixel32 color = *(pixel32 *)bitmap_2d_address(extract_data.plate, x, y, 0);

					if (extract_data.valid_plate)
					{
						pixel32 rgb = PIXEL32_RGB_BITS(color);

						if (rgb == extract_data.background_color ||
							rgb == extract_data.dummy_space_color ||
							rgb == extract_data.sequence_divider_color)
						{
							color = 0;
						}
					}

					if (extract_data.group->usage == _bitmap_group_usage_alpha_blend &&
						!PIXEL32_ALPHA_BITS(color) &&
						PIXEL32_RGB_BITS(color) &&
						!warned_about_zero_alpha)
					{
						fprintf(stdout, "==> !!WARNING!! usage set to alpha; non-zero color overlaps with zero-alpha <==\r\n");
						fflush(stdout);
						warned_about_zero_alpha = TRUE;
					}

					if (extract_data.group->format == _bitmap_group_format_compressed_color_key_transparency)
					{
						if (PIXEL32_ALPHA(color) != 0 && PIXEL32_ALPHA(color) != ALPHA_ONE && !warned_about_dxt1_alpha)
						{
							fprintf(stdout, "==> !!WARNING!! bitmap with greater than 1-bit alpha being compressed as DXT1 <==\r\n");
							fflush(stdout);
							warned_about_dxt1_alpha = TRUE;
						}

						if (!PIXEL32_ALPHA(color) && (PIXEL32_RGB_BITS(color) == extract_data.dummy_space_color || !extract_data.valid_plate))
						{
							color = 0;
						}
						else
						{
							color |= PIXEL32_ALPHA_MASK;
						}
					}

					if ((extract_data.group->format == _bitmap_group_format_compressed_color_key_transparency ||
						extract_data.group->format == _bitmap_group_format_compressed_explicit_alpha ||
						extract_data.group->format == _bitmap_group_format_compressed_interpolated_alpha) &&
						extract_data.group->type == _bitmap_group_type_interface_bitmaps)
					{
						error(_error_immediate, "### ERROR interface/linear bitmap cannot be DXT-compressed");
					}

					destination[x - adjusted_bounds.x0] = color;
				}
			}

			if (extract_data.group->type == _bitmap_group_type_2d_textures ||
				extract_data.group->type == _bitmap_group_type_interface_bitmaps)
			{
				short bitmap_index = extract_bitmap_to_group(bitmap);

				if (bitmap_index != NONE)
				{
					if (extract_data.sequence->first_bitmap_index == NONE)
					{
						extract_data.sequence->first_bitmap_index = bitmap_index;
						extract_data.sequence->bitmap_count = 0;
					}

					extract_data.sequence->bitmap_count++;
				}

				bitmap_delete(bitmap);
			}
			else if (!extract_data.valid_plate)
			{
				if (extract_data.group->type == _bitmap_group_type_cube_maps)
				{
					extract_plateless_cube_map(bitmap);
				}
				else
				{
					error(_error_silent, "### ERROR extract: tried to extract non-2d textures without a valid place but they weren't cube maps and/or EXTRACT_PLATELESS_CUBE_MAPS aren't allowed");
					success = FALSE;
				}

				bitmap_delete(bitmap);
			}
			else if (extract_data.temporary_bitmap_count < MAXIMUM_TEMPORARY_BITMAPS)
			{
				struct temporary_bitmap *temporary_bitmap = &extract_data.temporary_bitmaps[extract_data.temporary_bitmap_count++];

				temporary_bitmap->bitmap = bitmap;
				temporary_bitmap->sequence_index = extract_data.sequence_index;
				temporary_bitmap->sprite_index = NONE;
				temporary_bitmap->texture_page_index = NONE;
				temporary_bitmap->texture_index = NONE;

				if (extract_data.group->type == _bitmap_group_type_sprites)
				{
					short sprite_index = (short)tag_block_add_element(&extract_data.sequence->sprites);

					if (sprite_index != NONE)
					{
						struct bitmap_group_sprite *sprite = TAG_BLOCK_GET_ELEMENT(&extract_data.sequence->sprites, sprite_index, struct bitmap_group_sprite);

						sprite->bitmap_index = NONE;

						if (TEST_FLAG(extract_data.group->flags, _bitmap_group_extract_sprites_filthy_bug_fix_bit))
						{
							sprite->registration_point.x = (real)bitmap->registration_point.x * 0.5f;
							sprite->registration_point.y = (real)bitmap->registration_point.y * 0.5f;
						}
						else
						{
							sprite->registration_point.x = (real)bitmap->registration_point.x;
							sprite->registration_point.y = (real)bitmap->registration_point.y;
						}

						temporary_bitmap->sprite_index = sprite_index;
					}
					else
					{
						error(_error_silent, "### ERROR extract: failed to add sprite to sequence");
						success = FALSE;
					}
				}
			}
			else
			{
				error(_error_silent, "### ERROR extract: can't handle more than (#%d) temporary bitmaps", MAXIMUM_TEMPORARY_BITMAPS);
				success = FALSE;
			}
		}
		else
		{
			error(_error_silent, "### ERROR extract: failed to allocate temporary bitmap");
			success = FALSE;
		}
	}
	else
	{
		fprintf(stdout, "### WARNING skipped a bitmap which contained no data\r\n");
		fflush(stdout);
	}

	return success;
}

static boolean extract_adjust_bounds(
	rectangle2d const *bounds,
	rectangle2d *adjusted_bounds_reference)
{
	short y;
	boolean found_data = FALSE;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1015, bounds);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1016, adjusted_bounds_reference);

	adjusted_bounds_reference->y0 = SHORT_MAX;
	adjusted_bounds_reference->x0 = SHORT_MAX;
	adjusted_bounds_reference->y1 = SHORT_MIN;
	adjusted_bounds_reference->x1 = SHORT_MIN;

	for (y = bounds->y0; y < bounds->y1; y++)
	{
		short x;

		for (x = bounds->x0; x < bounds->x1; x++)
		{
			if (x >= 0 && x < extract_data.plate->width && y >= 0 && y < extract_data.plate->height)
			{
				pixel32 color = *(pixel32 *)bitmap_2d_address(extract_data.plate, x, y, 0);
				pixel32 rgb = PIXEL32_RGB_BITS(color);
				boolean contains_data = TRUE;

				if (extract_data.valid_plate)
				{
					if (rgb == extract_data.background_color ||
						rgb == extract_data.sequence_divider_color ||
						rgb == extract_data.dummy_space_color)
					{
						contains_data = FALSE;
					}
					else if (extract_data.group->usage == _bitmap_group_usage_alpha_blend && !PIXEL32_ALPHA_BITS(color))
					{
						contains_data = FALSE;
					}
				}

				if (contains_data)
				{
					adjusted_bounds_reference->x0 = MIN(x, adjusted_bounds_reference->x0);
					adjusted_bounds_reference->y0 = MIN(y, adjusted_bounds_reference->y0);
					adjusted_bounds_reference->x1 = MAX(x, adjusted_bounds_reference->x1);
					adjusted_bounds_reference->y1 = MAX(y, adjusted_bounds_reference->y1);
					found_data = TRUE;
				}
			}
		}
	}

	adjusted_bounds_reference->x1++;
	adjusted_bounds_reference->y1++;

	return found_data;
}

static short extract_get_bitmap_format(
	struct bitmap_data const *bitmap)
{
	pixel32 *pixels;
	pixel32 first_pixel;
	long pixel_count;
	long pixel_index;
	short format = NONE;
	short alpha_bits = 0;
	short color_bits = 0;
	boolean channels_differ = FALSE;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1065, bitmap_verify(bitmap, TRUE));

	pixels = bitmap_mipmap_address(bitmap, 0);
	first_pixel = *pixels;
	pixel_count = bitmap_get_pixel_count(bitmap);

	for (pixel_index = 0; pixel_index < pixel_count; pixel_index++)
	{
		pixel32 pixel = pixels[pixel_index];

		switch (PIXEL32_ALPHA(pixel))
		{
		case 0:
			if (PIXEL32_ALPHA_BITS(first_pixel) == PIXEL32_ALPHA_MASK)
			{
				alpha_bits = MAX(alpha_bits, 1);
			}
			break;
		case ALPHA_ONE:
			if (!PIXEL32_ALPHA_BITS(first_pixel))
			{
				alpha_bits = MAX(alpha_bits, 1);
			}
			break;
		default:
			alpha_bits = ALPHA_BITS;
			break;
		}

		switch (PIXEL32_RED(pixel))
		{
		case 0:
			if (PIXEL32_RED(first_pixel) == PIXEL32_COMPONENT_MASK)
			{
				color_bits = MAX(color_bits, 1);
			}
			break;
		case PIXEL32_COMPONENT_MASK:
			if (!PIXEL32_RED(first_pixel))
			{
				color_bits = MAX(color_bits, 1);
			}
			break;
		default:
			color_bits = PIXEL32_COMPONENT_BITS;
			break;
		}

		if (PIXEL32_ALPHA(pixel) != PIXEL32_RED(pixel))
		{
			channels_differ = TRUE;
		}
	}

	switch (extract_data.group->format)
	{
	case _bitmap_group_format_compressed_color_key_transparency:
		format = _bitmap_format_dxt1;
		break;
	case _bitmap_group_format_compressed_explicit_alpha:
		format = alpha_bits > 0 ? _bitmap_format_dxt3 : _bitmap_format_dxt1;
		break;
	case _bitmap_group_format_compressed_interpolated_alpha:
		format = alpha_bits > 0 ? _bitmap_format_dxt5 : _bitmap_format_dxt1;
		break;
	case _bitmap_group_format_16bit_color:
		if (alpha_bits == 0)
		{
			format = _bitmap_format_r5g6b5;
		}
		else if (alpha_bits == 1)
		{
			format = _bitmap_format_a1r5g5b5;
		}
		else
		{
			format = _bitmap_format_a4r4g4b4;
		}
		break;
	case _bitmap_group_format_32bit_color:
		format = alpha_bits == 0 ? _bitmap_format_x8r8g8b8 : _bitmap_format_a8r8g8b8;
		break;
	case _bitmap_group_format_monochrome:
		if (alpha_bits == 0)
		{
			format = _bitmap_format_y8;
		}
		else if (color_bits == 0)
		{
			format = _bitmap_format_a8;
		}
		else
		{
			format = channels_differ ? _bitmap_format_a8y8 : _bitmap_format_ay8;
		}
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1126, FALSE, "### ERROR extract: unsupported bitmap group format");
		break;
	}

	if (extract_data.group->type == _bitmap_group_type_interface_bitmaps)
	{
		switch (format)
		{
		case _bitmap_format_a1r5g5b5:
			format = _bitmap_format_a4r4g4b4;
			break;
		case _bitmap_format_x8r8g8b8:
			format = _bitmap_format_a8r8g8b8;
			break;
		}
	}

	if (extract_data.group->usage == _bitmap_group_usage_height_map ||
		extract_data.group->usage == _bitmap_group_usage_vector_map)
	{
		if (!TEST_FLAG(extract_data.group->flags, _bitmap_group_disable_vector_compression_bit))
		{
			format = _bitmap_format_p8_bump;
		}
	}

	return format;
}

static short extract_bitmap_to_group(
	struct bitmap_data const *bitmap)
{
	short format;
	short mipmap_count;
	short bitmap_index;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1159, bitmap_verify(bitmap, TRUE));

	format = extract_get_bitmap_format(bitmap);

	if (extract_data.group->type == _bitmap_group_type_interface_bitmaps ||
		extract_data.group->usage == _bitmap_group_usage_light_map)
	{
		mipmap_count = 0;
	}
	else
	{
		mipmap_count = bitmap_get_max_mipmap_count(bitmap);

		if (extract_data.group->type == _bitmap_group_type_sprites && mipmap_count >= BITMAP_MAXIMUM_SPRITE_PAGE_MIPMAP_COUNT)
		{
			mipmap_count = BITMAP_MAXIMUM_SPRITE_PAGE_MIPMAP_COUNT;
		}

		if (extract_data.group->mipmap_count > 0)
		{
			mipmap_count = MIN(extract_data.group->mipmap_count - 1, mipmap_count);
		}
	}

	bitmap_index = bitmap_group_add_bitmap(
		extract_data.group,
		bitmap->width,
		bitmap->height,
		bitmap->depth,
		bitmap->type,
		format,
		mipmap_count);
	extract_data.bitmap_index = bitmap_index;

	if (bitmap_index != NONE)
	{
		struct bitmap_data *working_bitmap = bitmap_clone(bitmap);
		struct bitmap_data *destination_bitmap = TAG_BLOCK_GET_ELEMENT(&extract_data.group->bitmaps, bitmap_index, struct bitmap_data);

		if (TEST_FLAG(extract_data.group->flags, _bitmap_group_extract_sprites_filthy_bug_fix_bit))
		{
			destination_bitmap->registration_point.x = (bitmap->registration_point.x + 1) / 2;
			destination_bitmap->registration_point.y = (bitmap->registration_point.y + 1) / 2;
		}
		else
		{
			destination_bitmap->registration_point = bitmap->registration_point;
		}

		if (working_bitmap && working_bitmap->base_address)
		{
			if (extract_data.group->smoothing_filter_size > 0.0f)
			{
				switch (extract_data.group->type)
				{
				case _bitmap_group_type_2d_textures:
				case _bitmap_group_type_3d_textures:
				case _bitmap_group_type_cube_maps:
					bitmap_smooth(working_bitmap, extract_data.group->smoothing_filter_size);
					break;
				case _bitmap_group_type_sprites:
					fprintf(stdout, "### WARNING tried to smooth a sprite group", "\r\n");
					fflush(stdout);
					break;
				case _bitmap_group_type_interface_bitmaps:
					fprintf(stdout, "### WARNING tried to smooth an interface-bitmap group", "\r\n");
					fflush(stdout);
					break;
				default:
					match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1232, FALSE, "### ERROR unsupported bitmap group type");
					break;
				}
			}

			extract_mipmaps_to_bitmap(working_bitmap, destination_bitmap);

			if (destination_bitmap->type == _bitmap_type_3d)
			{
				fprintf(
					stdout,
					"bitmap created: #%dx#%dx#%d, %s, %dK-bytes\r\n",
					destination_bitmap->width,
					destination_bitmap->height,
					destination_bitmap->depth,
					bitmap_format_get_string(destination_bitmap->format),
					bitmap_get_pixel_data_size(destination_bitmap) / 1024);
				fflush(stdout);
			}
			else
			{
				fprintf(
					stdout,
					"bitmap created: #%dx#%d, %s, %dK-bytes\r\n",
					destination_bitmap->width,
					destination_bitmap->height,
					bitmap_format_get_string(destination_bitmap->format),
					bitmap_get_pixel_data_size(destination_bitmap) / 1024);
				fflush(stdout);
			}
		}
	}

	return bitmap_index;
}

struct bitmap_data *extract_build_debug_plate(
	struct bitmap_data const *bitmap,
	boolean alpha_to_rgb,
	boolean include_mipmaps,
	boolean border)
{
	short mipmap_index;
	short slice_count;
	struct bitmap_data *converted_bitmap = NULL;
	struct bitmap_data *debug_bitmap = NULL;

	switch (bitmap->type)
	{
	case _bitmap_type_2d:
		converted_bitmap = bitmap_2d_new(bitmap->width, bitmap->height, bitmap->mipmap_count, _bitmap_format_a8r8g8b8);
		break;
	case _bitmap_type_3d:
		converted_bitmap = bitmap_3d_new(bitmap->width, bitmap->height, bitmap->depth, bitmap->mipmap_count, _bitmap_format_a8r8g8b8);
		break;
	case _bitmap_type_cube_map:
		converted_bitmap = bitmap_cube_map_new(bitmap->width, bitmap->mipmap_count, _bitmap_format_a8r8g8b8);
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1304, FALSE, "### ERROR unsupported bitmap type");
		break;
	}

	if (converted_bitmap && converted_bitmap->base_address)
	{
		short debug_width;
		short debug_height;

		for (mipmap_index = 0; mipmap_index <= bitmap->mipmap_count; mipmap_index++)
		{
			struct bitmap_data *mipmap_bitmap = NULL;
			short width = MAX(1, bitmap->width >> mipmap_index);
			short height = MAX(1, bitmap->height >> mipmap_index);
			short depth = MAX(1, bitmap->depth >> mipmap_index);

			switch (bitmap->type)
			{
			case _bitmap_type_2d:
				mipmap_bitmap = bitmap_2d_new(width, height, 0, _bitmap_format_a8r8g8b8);
				break;
			case _bitmap_type_3d:
				mipmap_bitmap = bitmap_3d_new(width, height, depth, 0, _bitmap_format_a8r8g8b8);
				break;
			case _bitmap_type_cube_map:
				mipmap_bitmap = bitmap_cube_map_new(width, 0, _bitmap_format_a8r8g8b8);
				break;
			default:
				match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1346, FALSE, "### ERROR unsupported bitmap type");
				break;
			}

			if (mipmap_bitmap && mipmap_bitmap->base_address)
			{
				extract_pixels_from_mipmap(bitmap, mipmap_bitmap, mipmap_index);
				extract_pixels_to_mipmap(mipmap_bitmap, converted_bitmap, mipmap_index);
			}

			bitmap_delete(mipmap_bitmap);
		}

		switch (converted_bitmap->type)
		{
		case _bitmap_type_2d:
			slice_count = 1;
			break;
		case _bitmap_type_3d:
			slice_count = converted_bitmap->depth;
			break;
		case _bitmap_type_cube_map:
			slice_count = NUMBER_OF_FACES_PER_CUBE;
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1383, FALSE, "### ERROR unsupported bitmap type");
			break;
		}

		if (border)
		{
			debug_width = (converted_bitmap->width + 3) * slice_count + 3;
			debug_height = converted_bitmap->height + 8;
		}
		else
		{
			debug_width = converted_bitmap->width * slice_count;
			debug_height = converted_bitmap->height;
		}

		if (include_mipmaps)
		{
			for (mipmap_index = 1; mipmap_index <= converted_bitmap->mipmap_count; mipmap_index++)
			{
				debug_height += (converted_bitmap->height >> mipmap_index) + (border ? 4 : 0);
			}
		}

		debug_bitmap = bitmap_2d_new(debug_width, debug_height, 0, _bitmap_format_a8r8g8b8);

		if (debug_bitmap && debug_bitmap->base_address)
		{
			long pixel_index;
			short destination_y = border ? 4 : 0;
			pixel32 *pixels = bitmap_mipmap_address(debug_bitmap, 0);
			long pixel_count = bitmap_get_pixel_count(debug_bitmap);

			for (pixel_index = 0; pixel_index < pixel_count; pixel_index++)
			{
				pixels[pixel_index] = PIXEL32_FROM_ARGB(0, 0, 0, 255);
			}

			for (mipmap_index = 0; mipmap_index <= (include_mipmaps ? converted_bitmap->mipmap_count : 0); mipmap_index++)
			{
				short destination_x = border ? 3 : 0;
				struct bitmap_data *slice_bitmap = bitmap_2d_new(
					MAX(1, converted_bitmap->width >> mipmap_index),
					MAX(1, converted_bitmap->height >> mipmap_index),
					0,
					_bitmap_format_a8r8g8b8);

				if (slice_bitmap && slice_bitmap->base_address)
				{
					short mipmap_slice_count;
					short slice_index;

					switch (converted_bitmap->type)
					{
					case _bitmap_type_2d:
						mipmap_slice_count = 1;
						break;
					case _bitmap_type_3d:
						mipmap_slice_count = MAX(1, converted_bitmap->depth >> mipmap_index);
						break;
					case _bitmap_type_cube_map:
						mipmap_slice_count = NUMBER_OF_FACES_PER_CUBE;
						break;
					default:
						match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1452, FALSE, "### ERROR unsupported bitmap type");
						break;
					}

					for (slice_index = 0; slice_index < mipmap_slice_count; slice_index++)
					{
						point2d destination_point;

						switch (converted_bitmap->type)
						{
						case _bitmap_type_2d:
							memcpy(
								bitmap_mipmap_address(slice_bitmap, 0),
								bitmap_mipmap_address(converted_bitmap, mipmap_index),
								bitmap_get_pixel_data_size(slice_bitmap));
							break;
						case _bitmap_type_3d:
							memcpy(
								bitmap_mipmap_address(slice_bitmap, 0),
								bitmap_3d_address(converted_bitmap, 0, 0, slice_index, mipmap_index),
								bitmap_get_pixel_data_size(slice_bitmap));
							break;
						case _bitmap_type_cube_map:
							memcpy(
								bitmap_mipmap_address(slice_bitmap, 0),
								bitmap_cube_map_address(converted_bitmap, 0, 0, slice_index, mipmap_index),
								bitmap_get_pixel_data_size(slice_bitmap));
							break;
						default:
							match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1480, FALSE, "### ERROR unsupported bitmap type");
							break;
						}

						set_point2d(&destination_point, destination_x, destination_y);
						bitmap_copy(debug_bitmap, &destination_point, NULL, slice_bitmap, NULL, PIXEL32_FROM_ARGB(255, 255, 255, 255), 0);
						destination_x += (converted_bitmap->width >> mipmap_index) + (border ? 3 : 0);
					}
				}
				else
				{
					error(_error_silent, "### ERROR failed to allocate debug slice bitmap");
				}

				bitmap_delete(slice_bitmap);
				destination_y += (converted_bitmap->height >> mipmap_index) + (border ? 4 : 0);
			}
		}
		else
		{
			error(_error_silent, "### ERROR failed to allocate debug plate bitmap");
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate debug bitmap");
	}

	if (debug_bitmap && debug_bitmap->base_address && alpha_to_rgb)
	{
		bitmap_alpha_to_rgb(debug_bitmap);
	}

	bitmap_delete(converted_bitmap);

	return debug_bitmap;
}

static void extract_mipmaps_to_bitmap(
	struct bitmap_data const *source_bitmap,
	struct bitmap_data *destination_bitmap)
{
	short mipmap_index;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1529, bitmap_verify(source_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1530, destination_bitmap);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1531, destination_bitmap->base_address);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1532, destination_bitmap->type==source_bitmap->type);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1533, destination_bitmap->width==source_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1534, destination_bitmap->height==source_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1535, destination_bitmap->depth==source_bitmap->depth);

	for (mipmap_index = 0; mipmap_index <= destination_bitmap->mipmap_count; mipmap_index++)
	{
		struct bitmap_data *mipmap_bitmap;
		boolean ignore_zero_alpha = extract_data.group->usage == _bitmap_group_usage_alpha_blend &&
			destination_bitmap->format != _bitmap_format_y8 &&
			destination_bitmap->format != _bitmap_format_r5g6b5 &&
			destination_bitmap->format != _bitmap_format_x8r8g8b8;
		real mipmap_fraction = (real)mipmap_index / destination_bitmap->mipmap_count;
		short alpha_bias = (short)floor(PIN(extract_data.group->alpha_bias, -1.0f, 1.0f) * mipmap_fraction * 255.0f + 0.5f);

		match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1558, alpha_bias>=-255 && alpha_bias<=255);

		mipmap_bitmap = bitmap_shrink(source_bitmap, (short)FLAG(mipmap_index), alpha_bias, ignore_zero_alpha);

		if (mipmap_bitmap && mipmap_bitmap->base_address)
		{
			bitmap_sharpen(mipmap_bitmap, extract_data.group->sharpen_amount);

			if (extract_data.group->usage == _bitmap_group_usage_detail_map)
			{
				real mipmap_count = destination_bitmap->mipmap_count;
				real detail_fade = extract_data.group->detail_fade;
				real fade_amount = PIN((real)mipmap_index / (mipmap_count * (1.0f - detail_fade) + detail_fade), 0.0f, 1.0f);

				bitmap_fade(mipmap_bitmap, PIXEL32_FROM_ARGB(255, 127, 127, 127), fade_amount);
			}

			if (ignore_zero_alpha)
			{
				bitmap_alpha_bleed(mipmap_bitmap, 1);
			}

			if (extract_data.group->usage == _bitmap_group_usage_height_map)
			{
				bitmap_height_map(mipmap_bitmap, extract_data.group->bump_height);
			}

			if (extract_data.group->usage == _bitmap_group_usage_vector_map)
			{
				bitmap_vector_map(mipmap_bitmap);
			}

			if (TEST_FLAG(extract_data.group->flags, _bitmap_group_diffusion_dither_bit) &&
				extract_data.group->usage != _bitmap_group_usage_height_map &&
				extract_data.group->usage != _bitmap_group_usage_light_map &&
				extract_data.group->usage != _bitmap_group_usage_vector_map)
			{
				short const *bits_per_channel;

				switch (destination_bitmap->format)
				{
				case _bitmap_format_r5g6b5:
					bits_per_channel = bits_per_channel_r5g6b5;
					break;
				case _bitmap_format_a1r5g5b5:
					bits_per_channel = bits_per_channel_a1r5g5b5;
					break;
				case _bitmap_format_a4r4g4b4:
					bits_per_channel = bits_per_channel_a4r4g4b4;
					break;
				default:
					bits_per_channel = NULL;
					break;
				}

				bitmap_quantitize(mipmap_bitmap, bits_per_channel);
			}

			extract_pixels_to_mipmap(mipmap_bitmap, destination_bitmap, mipmap_index);
			bitmap_delete(mipmap_bitmap);
		}
		else
		{
			error(_error_silent, "### ERROR extract: failed to allocate temporary bitmap");
		}
	}

	if (extract_data.debug_plate_name)
	{
		bitmap_delete(extract_build_debug_plate(destination_bitmap, FALSE, TRUE, TRUE));
	}

	return;
}

static void extract_pixels_to_mipmap(
	struct bitmap_data const *source_bitmap,
	struct bitmap_data *destination_bitmap,
	short destination_mipmap_index)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1702, bitmap_verify(source_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1703, source_bitmap->width ==MAX(1, destination_bitmap->width >>destination_mipmap_index));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1704, source_bitmap->height==MAX(1, destination_bitmap->height>>destination_mipmap_index));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1705, source_bitmap->depth ==MAX(1, destination_bitmap->depth >>destination_mipmap_index));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1707, bitmap_verify(destination_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1708, destination_bitmap->type==source_bitmap->type);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1709, destination_mipmap_index>=0 && destination_mipmap_index<=destination_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1710, !TEST_FLAG(destination_bitmap->flags, _bitmap_swizzled_bit));

	if (TEST_FLAG(destination_bitmap->flags, _bitmap_compressed_bit))
	{
		bitmap_compress_to_mipmap(
			source_bitmap,
			destination_bitmap,
			destination_mipmap_index,
			extract_data.valid_plate ? &extract_data.dummy_space_color : NULL);
	}
	else
	{
		long pixel_index;
		pixel32 const *source_pixels = bitmap_mipmap_address(source_bitmap, 0);
		void *destination_pixels = bitmap_mipmap_address(destination_bitmap, destination_mipmap_index);
		long pixel_count = bitmap_get_pixel_count(source_bitmap);

		for (pixel_index = 0; pixel_index < pixel_count; pixel_index++)
		{
			pixel32 pixel = source_pixels[pixel_index];

			switch (destination_bitmap->format)
			{
			case _bitmap_format_r5g6b5:
				((word *)destination_pixels)[pixel_index] = PIXEL32_TO_PIXEL16_565(pixel);
				break;
			case _bitmap_format_a1r5g5b5:
				((word *)destination_pixels)[pixel_index] = PIXEL32_TO_PIXEL16_1555(pixel);
				break;
			case _bitmap_format_a4r4g4b4:
				((word *)destination_pixels)[pixel_index] = PIXEL32_TO_PIXEL16_4444(pixel);
				break;
			case _bitmap_format_x8r8g8b8:
				((pixel32 *)destination_pixels)[pixel_index] = pixel | PIXEL32_ALPHA_MASK;
				break;
			case _bitmap_format_a8r8g8b8:
				((pixel32 *)destination_pixels)[pixel_index] = pixel;
				break;
			case _bitmap_format_a8:
				((byte *)destination_pixels)[pixel_index] = (byte)PIXEL32_ALPHA(pixel);
				break;
			case _bitmap_format_y8:
			case _bitmap_format_ay8:
				((byte *)destination_pixels)[pixel_index] = (byte)PIXEL32_RED(pixel);
				break;
			case _bitmap_format_a8y8:
				((word *)destination_pixels)[pixel_index] = PIXEL32_TO_PIXEL16_A8Y8(pixel);
				break;
			case _bitmap_format_p8_bump:
				((byte *)destination_pixels)[pixel_index] = palette_find_closest_match(global_vector_palette, pixel);
				break;
			default:
				match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1772, FALSE, "### ERROR unsupported bitmap format");
				break;
			}
		}
	}

	return;
}

static void extract_pixels_from_mipmap(
	struct bitmap_data const *source_bitmap,
	struct bitmap_data *destination_bitmap,
	short source_mipmap_index)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1785, bitmap_verify(destination_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1786, destination_bitmap->width ==MAX(1, source_bitmap->width >>source_mipmap_index));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1787, destination_bitmap->height==MAX(1, source_bitmap->height>>source_mipmap_index));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1788, destination_bitmap->depth ==MAX(1, source_bitmap->depth >>source_mipmap_index));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1790, bitmap_verify(source_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1791, source_bitmap->type==destination_bitmap->type);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1792, source_mipmap_index>=0 && source_mipmap_index<=source_bitmap->mipmap_count);

	if (TEST_FLAG(source_bitmap->flags, _bitmap_compressed_bit))
	{
		bitmap_uncompress_from_mipmap(source_bitmap, destination_bitmap, source_mipmap_index);
	}
	else
	{
		long pixel_index;
		void const *source_pixels = bitmap_mipmap_address(source_bitmap, source_mipmap_index);
		pixel32 *destination_pixels = bitmap_mipmap_address(destination_bitmap, 0);
		long pixel_count = bitmap_get_pixel_count(destination_bitmap);

		for (pixel_index = 0; pixel_index < pixel_count; pixel_index++)
		{
			destination_pixels[pixel_index] = bitmap_format_to_a8r8g8b8(source_bitmap->format, source_pixels, pixel_index);
		}
	}

	return;
}

static void build_texture_pages_by_sequence(
	struct texture_page **texture_pages,
	short *texture_page_count,
	short page_size,
	short spacing)
{
	short sequence_index;
	short page_count = 0;
	short spanned_page_count = 1;

	for (sequence_index = 0; sequence_index < extract_data.group->sequences.count; sequence_index++)
	{
		boolean page_complete;
		short first_temporary_bitmap_index = 0;
		short page_index = 0;

		do
		{
			struct texture_page *texture_page;
			boolean new_page;
			short temporary_bitmap_index;

			if (page_index < page_count)
			{
				texture_page = texture_pages[page_index];
				new_page = FALSE;
			}
			else if (page_count < MAXIMUM_SPRITE_TEXTURE_PAGES)
			{
				texture_page = NULL;
				new_page = TRUE;
			}
			else
			{
				break;
			}

			if (texture_page)
			{
				texture_page_textures_begin(texture_page);
			}

			page_complete = TRUE;

			for (temporary_bitmap_index = first_temporary_bitmap_index; temporary_bitmap_index < extract_data.temporary_bitmap_count; temporary_bitmap_index++)
			{
				struct temporary_bitmap *temporary_bitmap = &extract_data.temporary_bitmaps[temporary_bitmap_index];

				if (temporary_bitmap->sequence_index == sequence_index)
				{
					long texture_index;

					if (!texture_page)
					{
						short page_width = MAX(page_size, temporary_bitmap->bitmap->width);
						short page_height = MAX(page_size, temporary_bitmap->bitmap->height);

						page_width = (short)MIN(MAXIMUM_SPRITE_TEXTURE_PAGE_DIMENSION, ceiling_power2(page_width));
						page_height = (short)MIN(MAXIMUM_SPRITE_TEXTURE_PAGE_DIMENSION, ceiling_power2(page_height));
						texture_page = texture_page_new(NULL, page_width, page_height, spacing);

						if (!texture_page)
						{
							break;
						}

						texture_page_textures_begin(texture_page);
						texture_pages[page_count++] = texture_page;
					}

					texture_index = texture_page_texture_new(texture_page, temporary_bitmap->bitmap->width, temporary_bitmap->bitmap->height, TRUE);

					if (texture_index != NONE)
					{
						temporary_bitmap->texture_page_index = page_index;
						temporary_bitmap->texture_index = texture_index;
					}
					else
					{
						if (new_page)
						{
							spanned_page_count++;
							first_temporary_bitmap_index = temporary_bitmap_index;
							texture_page_textures_end(texture_page);
						}
						else
						{
							texture_page_textures_cancel(texture_page);
						}

						page_complete = FALSE;
						break;
					}
				}
			}

			if (texture_page && page_complete)
			{
				texture_page_textures_end(texture_page);
			}

			page_index++;
		}
		while (!page_complete);
	}

	for (sequence_index = 0; sequence_index < page_count; sequence_index++)
	{
		short width;
		short height;
		struct texture_page *texture_page = texture_pages[sequence_index];

		do
		{
			width = texture_page->width >> 1;
			height = texture_page->height >> 1;
		}
		while (width >= MINIMUM_SPRITE_TEXTURE_PAGE_DIMENSION &&
			height >= MINIMUM_SPRITE_TEXTURE_PAGE_DIMENSION &&
			texture_page_resize(texture_page, width, height));
	}

	fprintf(stdout, "sequence spanned %d texture pages\r\n", spanned_page_count);
	fflush(stdout);
	*texture_page_count = page_count;

	return;
}

static boolean process_3d_bitmaps(
	void)
{
	boolean success = TRUE;
	short first_temporary_bitmap_index = 0;

	while (success && first_temporary_bitmap_index < extract_data.temporary_bitmap_count)
	{
		struct temporary_bitmap *first_temporary_bitmap = &extract_data.temporary_bitmaps[first_temporary_bitmap_index];
		short sequence_index = first_temporary_bitmap->sequence_index;
		short width = first_temporary_bitmap->bitmap->width;
		short height = first_temporary_bitmap->bitmap->height;
		short slice_count = 0;
		boolean incompatible_slices = FALSE;

		while (!incompatible_slices && extract_data.temporary_bitmaps[first_temporary_bitmap_index + slice_count].sequence_index == sequence_index)
		{
			struct bitmap_data *slice_bitmap = extract_data.temporary_bitmaps[first_temporary_bitmap_index + slice_count].bitmap;

			if (slice_bitmap->width != width || slice_bitmap->height != height)
			{
				incompatible_slices = TRUE;
			}

			slice_count++;
		}

		if (incompatible_slices)
		{
			fprintf(stdout, "skipping 3D texture with incompatible slices\r\n");
			fflush(stdout);
		}
		else if (slice_count & (slice_count - 1))
		{
			fprintf(stdout, "skipping 3D texture with non power-of-two slice count\r\n");
			fflush(stdout);
		}
		else
		{
			struct bitmap_data *bitmap = bitmap_3d_new(width, height, slice_count, 0, _bitmap_format_a8r8g8b8);

			if (bitmap && bitmap->base_address)
			{
				short slice_index;
				short bitmap_index;

				for (slice_index = 0; slice_index < slice_count; slice_index++)
				{
					bitmap_3d_slice_insert(extract_data.temporary_bitmaps[first_temporary_bitmap_index + slice_index].bitmap, bitmap, 0, slice_index);
				}

				extract_data.sequence_index = sequence_index;
				bitmap_index = extract_bitmap_to_group(bitmap);

				if (bitmap_index != NONE)
				{
					struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(&extract_data.group->sequences, sequence_index, struct bitmap_group_sequence);

					if (sequence->first_bitmap_index == NONE)
					{
						sequence->first_bitmap_index = bitmap_index;
						sequence->bitmap_count = 1;
					}
					else
					{
						sequence->bitmap_count++;
					}
				}
			}
			else
			{
				error(_error_silent, "### ERROR extract: failed to allocate temporary bitmap");
				success = FALSE;
			}

			bitmap_delete(bitmap);
		}

		first_temporary_bitmap_index += slice_count;
	}

	return success;
}

static boolean process_cube_maps(
	void)
{
	short sequence_index;
	short temporary_bitmap_index;
	boolean success = TRUE;
	struct bitmap_data *temporary_bitmap = NULL;
	short face_index = 0;

	for (temporary_bitmap_index = 0; success && temporary_bitmap_index < extract_data.temporary_bitmap_count; temporary_bitmap_index++)
	{
		struct temporary_bitmap *face = &extract_data.temporary_bitmaps[temporary_bitmap_index];
		boolean skip_cube_map = FALSE;

		if (face_index == 0)
		{
			match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_extract.c", 1944, !temporary_bitmap);

			if (face->bitmap->width == face->bitmap->height)
			{
				temporary_bitmap = bitmap_cube_map_new(face->bitmap->width, 0, _bitmap_format_a8r8g8b8);
				sequence_index = face->sequence_index;
			}
			else
			{
				fprintf(stdout, "skipping cube map with non-square faces\r\n");
				fflush(stdout);
				skip_cube_map = TRUE;
			}
		}

		if (temporary_bitmap && temporary_bitmap->base_address)
		{
			if (face->sequence_index == sequence_index)
			{
				if (face->bitmap->width == temporary_bitmap->width && face->bitmap->height == temporary_bitmap->height)
				{
					bitmap_cube_map_face_insert(face->bitmap, temporary_bitmap, 0, face_index);
					face_index++;
				}
				else
				{
					fprintf(stdout, "skipping cube map with incompatible-size faces\r\n");
					fflush(stdout);
					skip_cube_map = TRUE;
				}
			}
			else
			{
				fprintf(stdout, "skipping cube map which spanned sequence\r\n");
				fflush(stdout);
				skip_cube_map = TRUE;
			}

			if (skip_cube_map)
			{
				bitmap_delete(temporary_bitmap);
				face_index = 0;
			}
		}
		else if (!skip_cube_map)
		{
			error(_error_silent, "### ERROR extract: failed to create temporary bitmap");
			success = FALSE;
		}

		if (success && face_index == NUMBER_OF_FACES_PER_CUBE)
		{
			short bitmap_index = extract_bitmap_to_group(temporary_bitmap);

			if (bitmap_index != NONE)
			{
				struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(&extract_data.group->sequences, sequence_index, struct bitmap_group_sequence);

				if (sequence->first_bitmap_index == NONE)
				{
					sequence->first_bitmap_index = bitmap_index;
					sequence->bitmap_count = 0;
				}

				sequence->bitmap_count++;
			}

			bitmap_delete(temporary_bitmap);
			temporary_bitmap = NULL;
			face_index = 0;
		}

		bitmap_delete(face->bitmap);
	}

	if (temporary_bitmap)
	{
		if (success)
		{
			fprintf(stdout, "skipping cube map with less than six faces\r\n");
			fflush(stdout);
		}

		bitmap_delete(temporary_bitmap);
	}

	return success;
}

static boolean process_sprites(
	void)
{
	struct texture_page *texture_pages[MAXIMUM_SPRITE_TEXTURE_PAGES];
	short texture_page_count;
	short temporary_bitmap_index;
	short page_index;
	boolean success = TRUE;
	long used_pixel_count = 0;
	short sprite_budget_count = extract_data.group->sprite_budget_count;
	short spacing = extract_data.group->mipmap_count == 1 ? 1 : 4;
	long page_size = MINIMUM_SPRITE_TEXTURE_PAGE_DIMENSION << (sprite_budget_count ? extract_data.group->sprite_budget_size : _bitmap_group_sprite_budget_512);
	long budget_pixel_count = sprite_budget_count * page_size * page_size;
	pixel32 sprite_usage_background_colors[NUMBER_OF_BITMAP_GROUP_SPRITE_USAGES] =
	{
		PIXEL32_FROM_ARGB(0, 0, 0, 0),			// blend_add_sub_max
		PIXEL32_FROM_ARGB(255, 255, 255, 255),	// mul_min
		PIXEL32_FROM_ARGB(127, 127, 127, 127)	// double_multiply
	};
	short maximum_sprite_dimension = (short)(page_size - 2 * spacing);

	for (temporary_bitmap_index = 0; success && temporary_bitmap_index < extract_data.temporary_bitmap_count; temporary_bitmap_index++)
	{
		struct temporary_bitmap *temporary_bitmap = &extract_data.temporary_bitmaps[temporary_bitmap_index];

		if (temporary_bitmap->bitmap &&
			(temporary_bitmap->bitmap->width > maximum_sprite_dimension || temporary_bitmap->bitmap->height > maximum_sprite_dimension))
		{
			error(_error_immediate, "### ERROR one or more sprites do not fit in the requested page size");
			success = FALSE;
		}
	}

	if (success)
	{
		if (TEST_FLAG(extract_data.group->flags, _bitmap_group_uniform_sprite_sequences_bit))
		{
			error(_error_immediate, "### ERROR hey - don't even try it! (uniform sprite sequences)\ndon't swim in that septic tank with your mouth open like that");
		}

		build_texture_pages_by_sequence(texture_pages, &texture_page_count, (short)page_size, spacing);
		extract_data.group->sprite_spacing = spacing;
	}

	for (page_index = 0; success && page_index < texture_page_count; page_index++)
	{
		struct texture_page *texture_page = texture_pages[page_index];
		struct bitmap_data *page_bitmap = bitmap_2d_new(texture_page->width, texture_page->height, 0, _bitmap_format_a8r8g8b8);

		if (page_bitmap && page_bitmap->base_address)
		{
			bitmap_fill(page_bitmap, sprite_usage_background_colors[extract_data.group->sprite_usage]);

			for (temporary_bitmap_index = 0; success && temporary_bitmap_index < extract_data.temporary_bitmap_count; temporary_bitmap_index++)
			{
				struct temporary_bitmap *temporary_bitmap = &extract_data.temporary_bitmaps[temporary_bitmap_index];
				struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(&extract_data.group->sequences, temporary_bitmap->sequence_index, struct bitmap_group_sequence);
				struct bitmap_group_sprite *sprite = TAG_BLOCK_GET_ELEMENT(&sequence->sprites, temporary_bitmap->sprite_index, struct bitmap_group_sprite);

				if (temporary_bitmap->texture_page_index == page_index)
				{
					point2d origin;
					struct texture_page_texture *texture = texture_page_texture_get(texture_page, temporary_bitmap->texture_index);
					short sprite_spacing = spacing;

					if (!TEST_FLAG(extract_data.group->flags, _bitmap_group_extract_sprites_filthy_bug_fix_bit) &&
						texture_page->textures->actual_count == 1)
					{
						sprite_spacing = 0;
					}

					sprite->bitmap_index = page_index;
					sprite->registration_point.x = (sprite_spacing + sprite->registration_point.x) / texture_page->width;
					sprite->registration_point.y = (sprite_spacing + sprite->registration_point.y) / texture_page->height;
					sprite->bounds.x0 = (real)(texture->x - sprite_spacing) / texture_page->width;
					sprite->bounds.y0 = (real)(texture->y - sprite_spacing) / texture_page->height;
					sprite->bounds.x1 = (real)(texture->x + texture->width + sprite_spacing) / texture_page->width;
					sprite->bounds.y1 = (real)(texture->y + texture->height + sprite_spacing) / texture_page->height;

					if (sequence->first_bitmap_index == NONE)
					{
						sequence->first_bitmap_index = page_index;
						sequence->bitmap_count = 1;
					}
					else
					{
						sequence->bitmap_count = page_index - sequence->first_bitmap_index;
					}

					origin.x = texture->x;
					origin.y = texture->y;
					bitmap_copy(page_bitmap, &origin, NULL, temporary_bitmap->bitmap, NULL, PIXEL32_FROM_ARGB(255, 255, 255, 255), 0);
					bitmap_delete(temporary_bitmap->bitmap);
				}
			}

			if (extract_bitmap_to_group(page_bitmap) != NONE)
			{
				used_pixel_count += texture_page->width * texture_page->height;
				bitmap_delete(page_bitmap);
			}
		}
		else
		{
			error(_error_silent, "### ERROR extract_sprite: failed to allocate texture page bitmap");
			success = FALSE;
		}

		fprintf(
			stdout,
			"texture page created #%dx#%d (%3.2f%% used)\r\n",
			texture_page->width,
			texture_page->height,
			texture_page_fraction_used(texture_page, TRUE) * 100.0f);
		fflush(stdout);
		texture_page_delete(texture_page);
	}

	if (success)
	{
		if ((real)budget_pixel_count == 0.0f)
		{
			fprintf(stdout, "### WARNING no sprite budget set\r\n");
			fflush(stdout);
		}
		else
		{
			real budget_fraction = (real)used_pixel_count / (real)budget_pixel_count;

			if (budget_fraction <= 1.0f)
			{
				fprintf(stdout, "sprite budget met (%3.0f%%)\r\n", budget_fraction * 100.0f);
				fflush(stdout);
			}
			else
			{
				error(_error_silent, "### ERROR sprite budget exceeded (%3.0f%%)", budget_fraction * 100.0f);
				success = FALSE;
			}
		}
	}

	return success;
}
