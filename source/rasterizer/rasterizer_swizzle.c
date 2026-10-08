/*
RASTERIZER_SWIZZLE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer_swizzle.h"
#include "bitmaps.h"
#include "integer_math.h"

/* ---------- globals */

static unsigned long ax;
static unsigned long ay;
static unsigned long az;

/* ---------- public code */

static void compute_swizzle_masks(
	short width,
	short height,
	short depth)
{
	unsigned long i = 1;
	unsigned long j = 1;
	unsigned long k;

	az = 0;
	ay = 0;
	ax = 0;
	do
	{
		k = 0;
		if (i<(unsigned long)width)
		{
			ax |= j;
			j <<= 1;
			k = j;
		}

		if (i<(unsigned long)height)
		{
			ay |= j;
			j <<= 1;
			k = j;
		}

		if (i<(unsigned long)depth)
		{
			az |= j;
			j <<= 1;
			k = j;
		}
		i <<= 1;
	}
	while (k);

	return;
}

void bitmap_swizzle_vector2d(
	short dim_x,
	short dim_y,
	short x,
	short y,
	long *result)
{
	static word const swizzle_table[64] =
	{
		0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015,
		0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
		0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115,
		0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
		0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415,
		0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
		0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515,
		0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555
	};
	short log2_x = floor_log2(dim_x);
	short log2_y = floor_log2(dim_y);
	short log2_min = MIN(log2_x, log2_y);
	short mask = (1<<log2_min)-1;
	long result_x;
	long result_y;

	if (mask<=63)
	{
		result_x = swizzle_table[x&mask];
		result_y = swizzle_table[y&mask]<<1;
	}
	else
	{
		long upper_mask = mask>>6;

		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 86, upper_mask<=63);
		result_x = swizzle_table[x&63] | (swizzle_table[(x>>6)&upper_mask]<<12);
		result_y = (swizzle_table[y&63]<<1) | (swizzle_table[(y>>6)&upper_mask]<<13);
	}

	if (log2_x>log2_min)
	{
		result_x |= (x>>log2_min)<<(log2_min*2);
	}
	else if (log2_y>log2_min)
	{
		result_y |= (y>>log2_min)<<(log2_min*2);
	}

	result[0] = result_x;
	result[1] = result_y;

	return;
}

void bitmap_swizzle_vector3d(
	short dim_x,
	short dim_y,
	short dim_z,
	short x,
	short y,
	short z,
	long *result)
{
	long result_x = 0;
	long result_y = 0;
	long result_z = 0;
	short previous_bit;
	short bit = 0;
	short i = 1;

	do
	{
		previous_bit = bit;
		if (i<dim_x)
		{
			result_x |= (x&1)<<bit;
			x >>= 1;
			bit++;
		}

		if (i<dim_y)
		{
			result_y |= (y&1)<<bit;
			y >>= 1;
			bit++;
		}

		if (i<dim_z)
		{
			result_z |= (z&1)<<bit;
			z >>= 1;
			bit++;
		}
		i <<= 1;
	}
	while (previous_bit!=bit);

	result[0] = result_x;
	result[1] = result_y;
	result[2] = result_z;

	return;
}

void rasterizer_xbox_bitmap_swizzle2d_byte(
	void *dst,
	void const *src,
	short width,
	short height)
{
	long offset = 0;
	unsigned long x_offset = 0;
	unsigned long y_offset = 0;
	short x;
	short y;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 147, dst);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 148, src);

	compute_swizzle_masks(width, height, 1);
	for (y = 0; y<height; y++)
	{
		for (x = 0; x<width; x++)
		{
			((byte *)dst)[x_offset | y_offset] = ((byte const *)src)[offset];
			offset++;
			x_offset = (x_offset-ax)&ax;
		}
		y_offset = (y_offset-ay)&ay;
	}

	return;
}

void rasterizer_xbox_bitmap_swizzle2d_word(
	void *dst,
	void const *src,
	short width,
	short height)
{
	long offset = 0;
	unsigned long x_offset = 0;
	unsigned long y_offset = 0;
	short x;
	short y;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 176, dst);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 177, src);

	compute_swizzle_masks(width, height, 1);
	for (y = 0; y<height; y++)
	{
		for (x = 0; x<width; x++)
		{
			((word *)dst)[x_offset | y_offset] = ((word const *)src)[offset];
			offset++;
			x_offset = (x_offset-ax)&ax;
		}
		y_offset = (y_offset-ay)&ay;
	}

	return;
}

void rasterizer_xbox_bitmap_swizzle2d_long(
	void *dst,
	void const *src,
	short width,
	short height)
{
	long offset = 0;
	unsigned long x_offset = 0;
	unsigned long y_offset = 0;
	short x;
	short y;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 205, dst);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 206, src);

	compute_swizzle_masks(width, height, 1);
	for (y = 0; y<height; y++)
	{
		for (x = 0; x<width; x++)
		{
			((unsigned long *)dst)[x_offset | y_offset] = ((unsigned long const *)src)[offset];
			offset++;
			x_offset = (x_offset-ax)&ax;
		}
		y_offset = (y_offset-ay)&ay;
	}

	return;
}

void rasterizer_xbox_bitmap_swizzle3d_byte(
	void *dst,
	void const *src,
	short width,
	short height,
	short depth)
{
	long offset = 0;
	unsigned long x_offset = 0;
	unsigned long y_offset = 0;
	unsigned long z_offset = 0;
	short x;
	short y;
	short z;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 235, dst);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 236, src);

	compute_swizzle_masks(width, height, depth);
	for (z = 0; z<depth; z++)
	{
		for (y = 0; y<height; y++)
		{
			for (x = 0; x<width; x++)
			{
				((byte *)dst)[x_offset | y_offset | z_offset] = ((byte const *)src)[offset];
				offset++;
				x_offset = (x_offset-ax)&ax;
			}
			y_offset = (y_offset-ay)&ay;
		}
		z_offset = (z_offset-az)&az;
	}

	return;
}

void rasterizer_xbox_bitmap_swizzle3d_word(
	void *dst,
	void const *src,
	short width,
	short height,
	short depth)
{
	long offset = 0;
	unsigned long x_offset = 0;
	unsigned long y_offset = 0;
	unsigned long z_offset = 0;
	short x;
	short y;
	short z;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 270, dst);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 271, src);

	compute_swizzle_masks(width, height, depth);
	for (z = 0; z<depth; z++)
	{
		for (y = 0; y<height; y++)
		{
			for (x = 0; x<width; x++)
			{
				((word *)dst)[x_offset | y_offset | z_offset] = ((word const *)src)[offset];
				offset++;
				x_offset = (x_offset-ax)&ax;
			}
			y_offset = (y_offset-ay)&ay;
		}
		z_offset = (z_offset-az)&az;
	}

	return;
}

void rasterizer_xbox_bitmap_swizzle3d_long(
	void *dst,
	void const *src,
	short width,
	short height,
	short depth)
{
	long offset = 0;
	unsigned long x_offset = 0;
	unsigned long y_offset = 0;
	unsigned long z_offset = 0;
	short x;
	short y;
	short z;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 305, dst);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 306, src);

	compute_swizzle_masks(width, height, depth);
	for (z = 0; z<depth; z++)
	{
		for (y = 0; y<height; y++)
		{
			for (x = 0; x<width; x++)
			{
				((unsigned long *)dst)[x_offset | y_offset | z_offset] = ((unsigned long const *)src)[offset];
				offset++;
				x_offset = (x_offset-ax)&ax;
			}
			y_offset = (y_offset-ay)&ay;
		}
		z_offset = (z_offset-az)&az;
	}

	return;
}

void rasterizer_xbox_bitmap_swizzle(
	struct bitmap_data *bitmap)
{
	short mipmap_index;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 334, bitmap);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 335, bitmap->base_address);

	if (!TEST_FLAG(bitmap->flags, _bitmap_compressed_bit) && !TEST_FLAG(bitmap->flags, _bitmap_linear_bit))
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 345, TEST_FLAG(bitmap->flags, _bitmap_has_power_of_two_dimensions_bit));

		for (mipmap_index = 0; mipmap_index<=bitmap->mipmap_count; mipmap_index++)
		{
			long size = bitmap_mipmap_get_pixel_data_size(bitmap, mipmap_index);
			void *source = bitmap_mipmap_address(bitmap, mipmap_index);
			void *buffer = match_malloc("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 351, size);
			short width = bitmap_mipmap_get_width(bitmap, mipmap_index);
			short height = bitmap_mipmap_get_height(bitmap, mipmap_index);
			short depth = bitmap_mipmap_get_depth(bitmap, mipmap_index);

			if (buffer)
			{
				short bytes_per_pixel = bitmap_format_get_bits_per_pixel(bitmap->format)/CHAR_BITS;

				compute_swizzle_masks(width, height, depth);
				switch (bitmap->type)
				{
				case _bitmap_type_2d:
					switch (bytes_per_pixel)
					{
					case 1:
						rasterizer_xbox_bitmap_swizzle2d_byte(buffer, source, width, height);
						break;
					case 2:
						rasterizer_xbox_bitmap_swizzle2d_word(buffer, source, width, height);
						break;
					case 4:
						rasterizer_xbox_bitmap_swizzle2d_long(buffer, source, width, height);
						break;
					default:
						match_vassert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 379, FALSE, "### ERROR unsupported bitmap format (bytes per pixel)");
					}
					break;
				case _bitmap_type_3d:
					switch (bytes_per_pixel)
					{
					case 1:
						rasterizer_xbox_bitmap_swizzle3d_byte(buffer, source, width, height, depth);
						break;
					case 2:
						rasterizer_xbox_bitmap_swizzle3d_word(buffer, source, width, height, depth);
						break;
					case 4:
						rasterizer_xbox_bitmap_swizzle3d_long(buffer, source, width, height, depth);
						break;
					default:
						match_vassert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 399, FALSE, "### ERROR unsupported bitmap format (bytes per pixel)");
					}
					break;
				case _bitmap_type_cube_map:
				{
					short face_index;
					char *face_source = source;
					char *face_buffer = buffer;

					for (face_index = 0; face_index<NUMBER_OF_FACES_PER_CUBE; face_index++)
					{
						switch (bytes_per_pixel)
						{
						case 1:
							rasterizer_xbox_bitmap_swizzle2d_byte(face_buffer, face_source, width, height);
							break;
						case 2:
							rasterizer_xbox_bitmap_swizzle2d_word(face_buffer, face_source, width, height);
							break;
						case 4:
							rasterizer_xbox_bitmap_swizzle2d_long(face_buffer, face_source, width, height);
							break;
						default:
							match_vassert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 425, FALSE, "### ERROR unsupported bitmap format (bytes per pixel)");
						}
						face_buffer += size/NUMBER_OF_FACES_PER_CUBE;
						face_source += size/NUMBER_OF_FACES_PER_CUBE;
					}
					break;
				}
				default:
					match_vassert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 436, FALSE, "### ERROR unsupported bitmap type");
				}

				memcpy(source, buffer, size);
				match_free("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 440, buffer);
				SET_FLAG(bitmap->flags, _bitmap_swizzled_bit, TRUE);
			}
			else
			{
				error(_error_silent, "### ERROR failed to allocate temporary buffer for swizzling");
			}
		}
	}

	return;
}

short rasterizer_xbox_bitmap_get_max_mipmap_count(
	struct bitmap_data const *bitmap)
{
	short max_mipmap_count = 0;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 459, bitmap_verify(bitmap, FALSE));

	if (TEST_FLAG(bitmap->flags, _bitmap_has_power_of_two_dimensions_bit) && !TEST_FLAG(bitmap->flags, _bitmap_linear_bit))
	{
		if (TEST_FLAG(bitmap->flags, _bitmap_compressed_bit))
		{
			max_mipmap_count = MIN(bitmap->mipmap_count, floor_log2(MAX(bitmap->width/4, MAX(bitmap->height/4, bitmap->depth))));
		}
		else
		{
			max_mipmap_count = MIN(bitmap->mipmap_count, floor_log2(MAX(bitmap->width, MAX(bitmap->height, bitmap->depth))));
		}
	}

	return max_mipmap_count;
}

long rasterizer_xbox_bitmap_get_pixel_data_size(
	struct bitmap_data const *bitmap)
{
	long size = 0;
	short max_mipmap_count = rasterizer_xbox_bitmap_get_max_mipmap_count(bitmap);
	short mipmap_index;

	for (mipmap_index = 0; mipmap_index<=max_mipmap_count; mipmap_index++)
	{
		long mipmap_size = bitmap_mipmap_get_pixel_data_size(bitmap, mipmap_index);

		if (TEST_FLAG(bitmap->flags, _bitmap_linear_bit))
		{
			long row_pitch;
			long padding;

			match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 506, mipmap_index==0);
			match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 507, !TEST_FLAG(bitmap->flags, _bitmap_compressed_bit));

			row_pitch = bitmap_mipmap_get_row_pitch(bitmap, mipmap_index);
			padding = (D3DTEXTURE_PITCH_ALIGNMENT-row_pitch)&(D3DTEXTURE_PITCH_ALIGNMENT-1);
			mipmap_size += bitmap_mipmap_get_height(bitmap, mipmap_index)*padding;
		}

		if (bitmap->type==_bitmap_type_cube_map)
		{
			mipmap_size /= NUMBER_OF_FACES_PER_CUBE;
		}
		size += mipmap_size;
	}

	size += (D3DTEXTURE_ALIGNMENT-size)&(D3DTEXTURE_ALIGNMENT-1);
	if (bitmap->type==_bitmap_type_cube_map)
	{
		size *= NUMBER_OF_FACES_PER_CUBE;
	}

	return size;
}

boolean rasterizer_xbox_bitmap_rebuild_hardware_format(
	struct bitmap_data *bitmap)
{
	static short const face_mapping_inverse_table[NUMBER_OF_FACES_PER_CUBE] = { 0, 2, 1, 3, 4, 5 };
	boolean success = TRUE;
	long size = rasterizer_xbox_bitmap_get_pixel_data_size(bitmap);
	long offset = 0;
	char *buffer = NULL;
	short face_count = bitmap->type==_bitmap_type_cube_map ? NUMBER_OF_FACES_PER_CUBE : 1;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 549, bitmap->base_address);

	buffer = match_malloc("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 552, size);
	if (buffer)
	{
		short face_index;

		rasterizer_xbox_bitmap_swizzle(bitmap);
		for (face_index = 0; face_index<face_count; face_index++)
		{
			short max_mipmap_count = rasterizer_xbox_bitmap_get_max_mipmap_count(bitmap);
			short mipmap_index;

			for (mipmap_index = 0; mipmap_index<=max_mipmap_count; mipmap_index++)
			{
				char *mipmap_address = bitmap_mipmap_address(bitmap, mipmap_index);
				long mipmap_size = bitmap_mipmap_get_pixel_data_size(bitmap, mipmap_index);
				short adjusted_face_index = face_mapping_inverse_table[face_index];

				if (bitmap->type==_bitmap_type_cube_map)
				{
					mipmap_size /= NUMBER_OF_FACES_PER_CUBE;
				}

				if (TEST_FLAG(bitmap->flags, _bitmap_linear_bit))
				{
					long row_pitch;
					long row_padding;
					short row;

					match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 588, face_index==0 && adjusted_face_index==0);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 589, mipmap_index==0);
					match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 590, !TEST_FLAG(bitmap->flags, _bitmap_compressed_bit));

					row_pitch = bitmap_mipmap_get_row_pitch(bitmap, mipmap_index);
					row_padding = (D3DTEXTURE_PITCH_ALIGNMENT-row_pitch)&(D3DTEXTURE_PITCH_ALIGNMENT-1);
					for (row = 0; row<bitmap->height; row++)
					{
						memcpy(buffer+offset, mipmap_address, row_pitch);
						offset += row_pitch;
						memset(buffer+offset, 0, row_padding);
						mipmap_address += row_pitch;
						offset += row_padding;
					}
				}
				else
				{
					memcpy(buffer+offset, mipmap_address+adjusted_face_index*mipmap_size, mipmap_size);
					offset += mipmap_size;
				}
			}

			{
				long padding = (D3DTEXTURE_ALIGNMENT-offset)&(D3DTEXTURE_ALIGNMENT-1);

				memset(buffer+offset, 0, padding);
				offset += padding;
			}
		}

		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 625, offset==size);
		memcpy(bitmap->base_address, buffer, size);
		match_free("c:\\halo\\SOURCE\\rasterizer\\rasterizer_swizzle.c", 629, buffer);
	}
	else
	{
		error(_error_silent, "### ERROR rasterizer_xbox_bitmap_rebuild_hardware_format failed (out of memory)");
		success = FALSE;
	}

	return success;
}
