/*
BITMAP_GROUP.C
*/

/* ---------- headers */

#include "cseries.h"
#include "bitmap_group.h"
#include "texture_cache.h"
#include "rasterizer.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean postprocess_bitmap(struct bitmap_data *bitmap, boolean editing);
static void delete_bitmap(struct tag_block *bitmaps_block, long bitmap_index);
static boolean postprocess_bitmap_group(long bitmap_group_index, boolean editing);

/* ---------- globals */

struct tag_reference_definition global_bitmap_reference =
{
	0,
	BITMAP_GROUP_TAG
};

struct tag_reference_definition global_bitmap_reference_optional =
{
	0,
	BITMAP_GROUP_TAG
};

static char *bitmap_types_strings[] =
{
	"2D texture",
	"3D texture",
	"cube map"
};

static struct enum_definition bitmap_types =
{
	NUMBEROF(bitmap_types_strings),
	bitmap_types_strings
};

static char *bitmap_formats_strings[] =
{
	"a8",
	"y8",
	"ay8",
	"a8y8",
	"unused1",
	"unused2",
	"r5g6b5",
	"unused3",
	"a1r5g5b5",
	"a4r4g4b4",
	"x8r8g8b8",
	"a8r8g8b8",
	"unused4",
	"unused5",
	"dxt1",
	"dxt3",
	"dxt5",
	"p8-bump"
};

static struct enum_definition bitmap_formats =
{
	NUMBEROF(bitmap_formats_strings),
	bitmap_formats_strings
};

static char *bitmap_flags_strings[] =
{
	"power of two dimensions",
	"compressed",
	"palettized",
	"swizzled",
	"linear",
	"v16u16"
};

static struct flags_definition bitmap_flags =
{
	NUMBEROF(bitmap_flags_strings),
	bitmap_flags_strings
};

static struct tag_field bitmap_data_block_fields[] =
{
	{ _field_tag, "signature*" },
	{ _field_short_integer, "width*:pixels" },
	{ _field_short_integer, "height*:pixels" },
	{ _field_short_integer, "depth*:pixels#depth is 1 for 2D textures and cube maps" },
	{ _field_enum, "type*#determines bitmap 'geometry'", &bitmap_types },
	{ _field_enum, "format*#determines how pixels are represented internally", &bitmap_formats },
	{ _field_word_flags, "flags*", &bitmap_flags },
	{ _field_point2d, "registration point*" },
	{ _field_short_integer, "mipmap count*" },
	{ _field_pad, NULL, (void *)sizeof(short) },
	{ _field_long_integer, "pixels offset*" },
	{ _field_pad, NULL, (void *)sizeof(long) },
	{ _field_pad, NULL, (void *)sizeof(long) },
	{ _field_pad, NULL, (void *)sizeof(long) },
	{ _field_pad, NULL, (void *)(2*sizeof(void *)) },
	{ _field_terminator }
};

static struct tag_block_definition bitmap_data_block =
{
	"bitmap_data_block",
	0,
	MAXIMUM_BITMAPS_PER_BITMAP_GROUP,
	sizeof(struct bitmap_data),
	NULL,
	bitmap_data_block_fields,
	NULL,
	(postprocess_block_proc)postprocess_bitmap,
	NULL,
	delete_bitmap
};

static struct tag_field bitmap_group_sprite_block_fields[] =
{
	{ _field_short_integer, "bitmap index*" },
	{ _field_pad, NULL, (void *)sizeof(short) },
	{ _field_pad, NULL, (void *)sizeof(long) },
	{ _field_real, "left*" },
	{ _field_real, "right*" },
	{ _field_real, "top*" },
	{ _field_real, "bottom*" },
	{ _field_real_point2d, "registration point*" },
	{ _field_terminator }
};

static struct tag_block_definition bitmap_group_sprite_block =
{
	"bitmap_group_sprite_block",
	0,
	MAXIMUM_SPRITES_PER_SEQUENCE,
	sizeof(struct bitmap_group_sprite),
	NULL,
	bitmap_group_sprite_block_fields
};

static struct tag_field bitmap_group_sequence_block_fields[] =
{
	{ _field_string, "name^" },
	{ _field_short_integer, "first bitmap index*" },
	{ _field_short_integer, "bitmap count*" },
	{ _field_pad, NULL, (void *)(4*sizeof(long)) },
	{ _field_block, "sprites*", &bitmap_group_sprite_block },
	{ _field_terminator }
};

static struct tag_block_definition bitmap_group_sequence_block =
{
	"bitmap_group_sequence_block",
	0,
	MAXIMUM_SEQUENCES_PER_BITMAP_GROUP,
	sizeof(struct bitmap_group_sequence),
	NULL,
	bitmap_group_sequence_block_fields
};

static char *bitmap_group_flags_strings[] =
{
	"enable diffusion dithering",
	"disable height map compression",
	"uniform sprite sequences",
	"filthy sprite bug fix"
};

static struct flags_definition bitmap_group_flags =
{
	NUMBEROF(bitmap_group_flags_strings),
	bitmap_group_flags_strings
};

static char *bitmap_group_types_strings[] =
{
	"2D textures",
	"3D textures",
	"cube maps",
	"sprites",
	"interface bitmaps"
};

static struct enum_definition bitmap_group_types =
{
	NUMBEROF(bitmap_group_types_strings),
	bitmap_group_types_strings
};

static char *bitmap_group_usages_strings[] =
{
	"alpha-blend",
	"default",
	"height map",
	"detail map",
	"light map",
	"vector map"
};

static struct enum_definition bitmap_group_usages =
{
	NUMBEROF(bitmap_group_usages_strings),
	bitmap_group_usages_strings
};

static char *bitmap_group_formats_strings[] =
{
	"compressed with color-key transparency",
	"compressed with explicit alpha",
	"compressed with interpolated alpha",
	"16-bit color",
	"32-bit color",
	"monochrome"
};

static struct enum_definition bitmap_group_formats =
{
	NUMBEROF(bitmap_group_formats_strings),
	bitmap_group_formats_strings
};

static char *bitmap_group_sprite_budgets_strings[] =
{
	"32x32",
	"64x64",
	"128x128",
	"256x256",
	"512x512"
};

static struct enum_definition bitmap_group_sprite_budgets =
{
	NUMBEROF(bitmap_group_sprite_budgets_strings),
	bitmap_group_sprite_budgets_strings
};

static char *bitmap_group_sprite_usages_strings[] =
{
	"blend/add/subtract/max",
	"multiply/min",
	"double multiply"
};

static struct enum_definition bitmap_group_sprite_usages =
{
	NUMBEROF(bitmap_group_sprite_usages_strings),
	bitmap_group_sprite_usages_strings
};

struct tag_data_definition bitmap_pixel_data =
{
	"bitmap_pixel_data",
	FLAG(_tag_data_cached_bit),
	MAXIMUM_PIXEL_DATA_SIZE
};

struct tag_data_definition color_plate_data =
{
	"color_plate_data",
	FLAG(_tag_data_cached_bit),
	MAXIMUM_BITMAP_PIXELS_SIZE
};

static struct tag_field bitmap_fields[] =
{
	{ _field_custom, NULL, (void *)BITMAP_GROUP_SHOW_BITMAP_CUSTOM_ID },
	{ _field_explanation, "type", "Type controls bitmap 'geometry'. All dimensions must be a power of two except for SPRITES and INTERFACE BITMAPS:\n\n* 2D TEXTURES: Ordinary, 2D textures will be generated.\n* 3D TEXTURES: Volume textures will be generated from each sequence of 2D texture 'slices'.\n* CUBE MAPS: Cube maps will be generated from each consecutive set of six 2D textures in each sequence, all faces of a cube map must be square and the same size.\n* SPRITES: Sprite texture pages will be generated.\n* INTERFACE BITMAPS: Similar to 2D TEXTURES, but without mipmaps and without the power of two restriction." },
	{ _field_enum, "type", &bitmap_group_types },
	{ _field_explanation, "format", "Format controls how pixels will be stored internally:\n\n* COMPRESSED WITH COLOR-KEY TRANSPARENCY: DXT1 compression, uses 4 bits per pixel. 4x4 blocks of pixels are reduced to 2 colors and interpolated, alpha channel uses color-key transparency instead of alpha from the plate (all zero-alpha pixels also have zero-color).\n* COMPRESSED WITH EXPLICIT ALPHA: DXT2/3 compression, uses 8 bits per pixel. Same as DXT1 without the color key transparency, alpha channel uses alpha from plate quantized down to 4 bits per pixel.\n* COMPRESSED WITH INTERPOLATED ALPHA: DXT4/5 compression, uses 8 bits per pixel. Same as DXT2/3, except alpha is smoother. Better for smooth alpha gradients, worse for noisy alpha.\n* 16-BIT COLOR: Uses 16 bits per pixel. Depending on the alpha channel, bitmaps are quantized to either r5g6b5 (no alpha), a1r5g5b5 (1-bit alpha), or a4r4g4b4 (>1-bit alpha).\n* 32-BIT COLOR: Uses 32 bits per pixel. Very high quality, can have alpha at no added cost. This format takes up the most memory, however. Bitmap formats are x8r8g8b8 and a8r8g8b.\n* MONOCHROME: Uses either 8 or 16 bits per pixel. Bitmap formats are a8 (alpha), y8 (intensity), ay8 (combined alpha-intensity) and a8y8 (separate alpha-intensity).\n\nNote: Height maps (a.k.a. bump maps) should use 32-bit color; this is internally converted to a palettized format which takes less memory." },
	{ _field_enum, "format", &bitmap_group_formats },
	{ _field_explanation, "usage", "Usage controls how mipmaps are generated:\n\n* ALPHA BLEND: Pixels with zero alpha are ignored in mipmaps, to prevent bleeding the transparent color.\n* DEFAULT: Downsampling works normally, as in Photoshop.\n* HEIGHT MAP: The bitmap (normally grayscale) is a height map which gets converted to a bump map. Uses <bump height> below. Alpha is passed through unmodified.\n* DETAIL MAP: Mipmap color fades to gray, controlled by <detail fade factor> below. Alpha fades to white.\n* LIGHT MAP: Generates no mipmaps. Do not use!\n* VECTOR MAP: Used mostly for special effects; pixels are treated as XYZ vectors and normalized after downsampling. Alpha is passed through unmodified." },
	{ _field_enum, "usage", &bitmap_group_usages },
	{ _field_word_flags, "flags", &bitmap_group_flags },
	{ _field_explanation, "post-processing", "These properties control how mipmaps are post-processed." },
	{ _field_real_fraction, "detail fade factor:[0,1]#0 means fade to gray by last mipmap, 1 means fade to gray by first mipmap" },
	{ _field_real_fraction, "sharpen amount:[0,1]#sharpens mipmap after downsampling" },
	{ _field_real_fraction, "bump height:repeats#the apparent height of the bump map above the triangle it is textured onto, in texture repeats (i.e., 1.0 would be as high as the texture is wide)" },
	{ _field_explanation, "sprite processing", "When creating a sprite group, specify the number and size of textures that the group is allowed to occupy. During importing, you'll receive feedback about how well the alloted space was used." },
	{ _field_enum, "sprite budget size", &bitmap_group_sprite_budgets },
	{ _field_short_integer, "sprite budget count" },
	{ _field_explanation, "color plate", "The original TIFF file used to import the bitmap group." },
	{ _field_short_integer, "color plate width*:pixels" },
	{ _field_short_integer, "color plate height*:pixels" },
	{ _field_data, "compressed color plate data*", &color_plate_data },
	{ _field_explanation, "processed pixel data", "Pixel data after being processed by the tool." },
	{ _field_data, "processed pixel data*", &bitmap_pixel_data },
	{ _field_explanation, "miscellaneous", "" },
	{ _field_real, "blur filter size:[0,10] pixels#blurs the bitmap before generating mipmaps" },
	{ _field_real, "alpha bias:[-1,1]#affects alpha mipmap generation" },
	{ _field_short_integer, "mipmap count:levels#0 defaults to all levels" },
	{ _field_explanation, "...more sprite processing", "Sprite usage controls the background color of sprite plates." },
	{ _field_enum, "sprite usage", &bitmap_group_sprite_usages },
	{ _field_short_integer, "sprite spacing*" },
	{ _field_pad, NULL, (void *)sizeof(short) },
	{ _field_block, "sequences*", &bitmap_group_sequence_block },
	{ _field_block, "bitmaps*", &bitmap_data_block },
	{ _field_terminator }
};

static struct tag_block_definition bitmap_block =
{
	"bitmap",
	0,
	1,
	sizeof(struct bitmap_group),
	NULL,
	bitmap_fields
};

struct tag_group bitmap_group =
{
	"bitmap",
	FLAG(_tag_group_can_be_reloaded_bit),
	BITMAP_GROUP_TAG,
	NONE,
	BITMAP_GROUP_VERSION,
	postprocess_bitmap_group,
	&bitmap_block
};

/* ---------- private code */

static boolean postprocess_bitmap(
	struct bitmap_data *bitmap,
	boolean editing)
{
	return TRUE;
}

static void delete_bitmap(
	struct tag_block *bitmaps_block,
	long bitmap_index)
{
	bitmap_delete(TAG_BLOCK_GET_ELEMENT(bitmaps_block, bitmap_index, struct bitmap_data));

	return;
}

static boolean postprocess_bitmap_group(
	long bitmap_group_index,
	boolean editing)
{
	short bitmap_index;
	short sequence_index;
	struct bitmap_group *group = tag_get(BITMAP_GROUP_TAG, bitmap_group_index);
	boolean success = TRUE;

	for (bitmap_index = 0; bitmap_index<group->bitmaps.count; bitmap_index++)
	{
		struct bitmap_data *bitmap = TAG_BLOCK_GET_ELEMENT(&group->bitmaps, bitmap_index, struct bitmap_data);

		if (group->type==_bitmap_group_type_interface_bitmaps)
		{
			SET_FLAG(bitmap->flags, _bitmap_linear_bit, TRUE);
		}

		if (bitmap_verify(bitmap, FALSE))
		{
			texture_cache_bitmap_new(bitmap_group_index, bitmap);
		}
		else
		{
			success = FALSE;
		}
	}

	for (sequence_index = 0; sequence_index<group->sequences.count; sequence_index++)
	{
		struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(&group->sequences, sequence_index, struct bitmap_group_sequence);
		struct bitmap_group_sequence *next_sequence = sequence_index<group->sequences.count-1 ?
			TAG_BLOCK_GET_ELEMENT(&group->sequences, sequence_index+1, struct bitmap_group_sequence) :
			NULL;

		if (group->type==_bitmap_group_type_sprites)
		{
			if (sequence->first_bitmap_index || sequence->bitmap_count)
			{
				TAG_BLOCK_GET_ELEMENT(&group->sequences, sequence_index, struct bitmap_group_sequence)->first_bitmap_index = 0;
				TAG_BLOCK_GET_ELEMENT(&group->sequences, sequence_index, struct bitmap_group_sequence)->bitmap_count = 0;
			}
		}
	}

	if (group->sequences.count>0)
	{
		struct bitmap_group_sequence *last_sequence =TAG_BLOCK_GET_ELEMENT(&group->sequences, group->sequences.count-1, struct bitmap_group_sequence);

		if (!last_sequence->bitmap_count && !last_sequence->sprites.count)
		{
			if (!tag_block_resize(&group->sequences, group->sequences.count-1))
			{
				error(_error_immediate, "### FATAL_ERROR failed to fix bitmap group '%s'", tag_get_name(bitmap_group_index));
				success = FALSE;
			}
		}
	}

	if (find_all_fucked_up_shit)
	{
		for (bitmap_index = 0; bitmap_index<group->bitmaps.count; bitmap_index++)
		{
			struct bitmap_data *bitmap =TAG_BLOCK_GET_ELEMENT(&group->bitmaps, bitmap_index, struct bitmap_data);

			if (bitmap->format==_bitmap_format_a8y8)
			{
				error(_error_silent, "!!MUST BE FIXED: bitmap #%d of group '%s' has a8y8 format", bitmap_index, tag_get_name(bitmap_group_index));
			}

			if (TEST_FLAG(bitmap->flags, _bitmap_linear_bit) && !(bitmap->width&(bitmap->width-1)) && !(bitmap->height&(bitmap->height-1)))
			{
				error(_error_silent, "!!MUST BE FIXED: bitmap #%d of group '%s' is linear and power-of-two", bitmap_index, tag_get_name(bitmap_group_index));
			}
		}

		if (group->bitmaps.count<1)
		{
			error(_error_silent, "!!MUST BE FIXED: ", "bitmap group '%s' has %d bitmaps", tag_get_name(bitmap_group_index), group->bitmaps.count);
		}

		if (group->sequences.count<1)
		{
			error(_error_silent, "!!MUST BE FIXED: ", "bitmap group '%s' has %d sequences", tag_get_name(bitmap_group_index), group->sequences.count);
		}

		for (sequence_index = 0; sequence_index<group->sequences.count; sequence_index++)
		{
			struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(&group->sequences, sequence_index, struct bitmap_group_sequence);
			struct bitmap_group_sequence *next_sequence =sequence_index<group->sequences.count-1 ?
				TAG_BLOCK_GET_ELEMENT(&group->sequences, sequence_index+1, struct bitmap_group_sequence) :
				NULL;

			if (group->type==_bitmap_group_type_sprites)
			{
				if (sequence->first_bitmap_index || sequence->bitmap_count)
				{
					error(_error_silent, "!!MUST BE FIXED: bitmap group '%s' (type=%d) sequence #%d doesn't know it's a sprite sequence", tag_get_name(bitmap_group_index), group->type, sequence_index);
				}
			}
			else
			{
				if (!(sequence->first_bitmap_index>=0 && sequence->first_bitmap_index<group->bitmaps.count &&
					sequence->bitmap_count>=1 && sequence->first_bitmap_index+sequence->bitmap_count<=group->bitmaps.count &&
					(sequence_index || !sequence->first_bitmap_index) &&
					(!next_sequence || next_sequence->first_bitmap_index==sequence->first_bitmap_index+sequence->bitmap_count)))
				{
					error(_error_silent, "!!MUST BE FIXED: bitmap group '%s' sequence #%d references bitmaps [#%d..#%d]", tag_get_name(bitmap_group_index), sequence_index,
						sequence->first_bitmap_index, sequence->first_bitmap_index+sequence->bitmap_count);
				}
			}

			if (group->type==_bitmap_group_type_sprites)
			{
				if (sequence->sprites.count<1)
				{
					error(_error_silent, "!!MUST BE FIXED: bitmap group '%s' sequence #%d has %d sprites", tag_get_name(bitmap_group_index), sequence_index, sequence->sprites.count);
				}
				else
				{
					short sprite_index;

					for (sprite_index = 0; sprite_index<sequence->sprites.count; sprite_index++)
					{
						struct bitmap_group_sprite *sprite =TAG_BLOCK_GET_ELEMENT(&sequence->sprites, sprite_index, struct bitmap_group_sprite);

						if (!(sprite->bitmap_index>=0 && sprite->bitmap_index<group->bitmaps.count))
						{
							error(_error_silent, "!!MUST BE FIXED: bitmap group '%s' sequence #%d sprite #%d references bitmap #%d", tag_get_name(bitmap_group_index), sequence_index, sprite_index, sprite->bitmap_index);
						}
					}
				}
			}
			else
			{
				if (sequence->sprites.count>0)
				{
					error(_error_silent, "!!MUST BE FIXED: bitmap group '%s' (type=%d) sequence #%d has %d sprites", tag_get_name(bitmap_group_index), group->type, sequence_index, sequence->sprites.count);
				}
			}
		}
	}

	return success;
}

/* ---------- public code */

struct bitmap_data *bitmap_group_try_and_get_bitmap(
	long bitmap_group_index,
	short bitmap_index)
{
	struct bitmap_group *group = tag_get(BITMAP_GROUP_TAG, bitmap_group_index);
	struct bitmap_data *bitmap = NULL;

	if (group && bitmap_index>=0 && bitmap_index<group->bitmaps.count)
	{
		bitmap =TAG_BLOCK_GET_ELEMENT(&group->bitmaps, bitmap_index, struct bitmap_data);
	}

	return bitmap;
}

struct bitmap_data *bitmap_group_get_bitmap_from_sequence(
	long bitmap_group_index,
	short sequence_index,
	short frame_index)
{
	struct bitmap_data *bitmap = NULL;

	if (bitmap_group_index!=NONE)
	{
		struct bitmap_group *group;

		match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_group.c", 678, sequence_index>=0 && frame_index>=0);
		group = tag_get(BITMAP_GROUP_TAG, bitmap_group_index);
		if (group)
		{
			short bitmap_index = NONE;

			if (group->sequences.count>0)
			{
				struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(&group->sequences, sequence_index%group->sequences.count, struct bitmap_group_sequence);

				if (sequence->bitmap_count>0)
				{
					bitmap_index = frame_index%sequence->bitmap_count + sequence->first_bitmap_index;
				}
				else if (sequence->sprites.count)
				{
					bitmap_index = TAG_BLOCK_GET_ELEMENT(&sequence->sprites, frame_index, struct bitmap_group_sprite)->bitmap_index;
				}
			}

			if (bitmap_index==NONE)
			{
				bitmap_index = frame_index;
			}

			if (bitmap_index>=0 && bitmap_index<group->bitmaps.count)
			{
				bitmap =TAG_BLOCK_GET_ELEMENT(&group->bitmaps, bitmap_index, struct bitmap_data);
			}
		}
	}

	return bitmap;
}

short bitmap_group_add_bitmap(
	struct bitmap_group *group,
	short width,
	short height,
	short depth,
	short type,
	short format,
	short mipmap_count)
{
	struct bitmap_data fake_bitmap;
	long pixels_offset = 0;
	short bitmap_index = NONE;
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_group.c", 731, group);

	fake_bitmap.signature = BITMAP_SIGNATURE;
	fake_bitmap.width = width;
	fake_bitmap.height = height;
	fake_bitmap.depth = depth;
	fake_bitmap.type = type;
	fake_bitmap.format = format;
	fake_bitmap.flags = 0;
	fake_bitmap.registration_point.y = 0;
	fake_bitmap.registration_point.x = 0;
	fake_bitmap.mipmap_count = mipmap_count;
	fake_bitmap.pixels_offset = 0;
	fake_bitmap.hardware_format = NULL;
	fake_bitmap.base_address = NULL;

	if (group->type==_bitmap_group_type_interface_bitmaps)
	{
		SET_FLAG(fake_bitmap.flags, _bitmap_linear_bit, TRUE);
	}
	else if ((width&(width-1)) || (height&(height-1)) || (depth&(depth-1)))
	{
		fprintf(stdout, "skipping bitmap with non-power-of-two dimensions (#%dx#%d#%d)\r\n", width, height, depth);
		fflush(stdout);
		success = FALSE;
	}
	else if (group->type==_bitmap_group_type_cube_maps)
	{
		if (width==height)
		{
			SET_FLAG(fake_bitmap.flags, _bitmap_has_power_of_two_dimensions_bit, TRUE);
		}
		else
		{
			fprintf(stdout, "skipping cube map with non-square faces (#%dx#%d)\r\n", width, height);
			fflush(stdout);
			success = FALSE;
		}
	}
	else
	{
		SET_FLAG(fake_bitmap.flags, _bitmap_has_power_of_two_dimensions_bit, TRUE);
	}

	if (success)
	{
		if (format>=BITMAP_FIRST_COMPRESSED_FORMAT && format<=BITMAP_LAST_COMPRESSED_FORMAT)
		{
			SET_FLAG(fake_bitmap.flags, _bitmap_compressed_bit, TRUE);
		}
		if (format==_bitmap_format_p8_bump)
		{
			SET_FLAG(fake_bitmap.flags, _bitmap_palettized_bit, TRUE);
		}

		if (group->type!=_bitmap_group_type_cube_maps || width==height)
		{
			if (TEST_FLAG(fake_bitmap.flags, _bitmap_has_power_of_two_dimensions_bit) || group->type==_bitmap_group_type_interface_bitmaps)
			{
				long bitmap_count = group->bitmaps.count;
				long pixel_data_size = bitmap_get_pixel_data_size(&fake_bitmap);

				if (tag_block_resize(&group->bitmaps, group->bitmaps.count+1) && tag_data_resize(&group->pixel_data, group->pixel_data.size+pixel_data_size))
				{
					struct bitmap_data *new_bitmap;
					short index;
					struct bitmap_data *previous_bitmap = NULL;

					for (index = 0; index<group->bitmaps.count; index++)
					{
						struct bitmap_data *bitmap = TAG_BLOCK_GET_ELEMENT(&group->bitmaps, index, struct bitmap_data);

						if (bitmap->base_address)
						{
							match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_group.c", 845, !bitmap->hardware_format);
							bitmap->base_address = bitmap->pixels_offset + (byte *)tag_data_get_address(&group->pixel_data);
							match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_group.c", 850, (byte*)bitmap->base_address>=(byte*)group->pixel_data.address);
							match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_group.c", 852, (byte*)bitmap->base_address + bitmap_get_pixel_data_size(bitmap) <= (byte*)group->pixel_data.address + group->pixel_data.size);

							if (previous_bitmap)
							{
								long space_between =bitmap->pixels_offset - previous_bitmap->pixels_offset - bitmap_get_pixel_data_size(previous_bitmap);

								match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_group.c", 859, space_between>=0);
								if (space_between)
								{
									error(_error_silent, "### WARNING bitmap group pixel data isn't tight");
								}
							}

							previous_bitmap = bitmap;
							pixels_offset = bitmap_get_pixel_data_size(bitmap) + bitmap->pixels_offset;
						}
					}

					bitmap_index = (short)bitmap_count;
					new_bitmap = TAG_BLOCK_GET_ELEMENT(&group->bitmaps, bitmap_index, struct bitmap_data);
					match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_group.c", 881, new_bitmap);
					memcpy(new_bitmap, &fake_bitmap, sizeof(struct bitmap_data));
					new_bitmap->pixels_offset = pixels_offset;
					new_bitmap->base_address = pixels_offset + (byte *)tag_data_get_address(&group->pixel_data);
					memset(new_bitmap->base_address, 0, pixel_data_size);
				}
				else
				{
					error(_error_silent, "### ERROR failed to add bitmap to group (tag resize failed)");
					tag_block_resize(&group->bitmaps, bitmap_count);
				}
			}
			else
			{
				fprintf(stdout, "skipping bitmap with non power-of-two dimensions (#%dx#%d)\r\n", width, height);
				fflush(stdout);
			}
		}
		else
		{
			fprintf(stdout, "skipping cube map with non-square faces (#%dx#%d)\r\n", width, height);
			fflush(stdout);
		}
	}

	return bitmap_index;
}
