/*
RASTERIZER_XBOX_HARDWARE_BITMAPS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer_xbox.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "texture_cache.h"
#include "bitmaps/s3tc/s3tc.h"
#include "rasterizer_swizzle.h"

/* ---------- prototypes */

static void rasterizer_bitmap_2d_changed(struct bitmap_data *bitmap);
static void rasterizer_bitmap_3d_changed(struct bitmap_data *bitmap);
static void rasterizer_bitmap_cm_changed(struct bitmap_data *bitmap);

/* ---------- globals */

static const D3DFORMAT rasterizer_bitmap_format_table[NUMBER_OF_BITMAP_FORMATS]=
{
	D3DFMT_A8, // _bitmap_format_a8
	D3DFMT_L8, // _bitmap_format_y8
	D3DFMT_AL8, // _bitmap_format_ay8
	D3DFMT_A8L8, // _bitmap_format_a8y8
	D3DFMT_UNKNOWN, // _bitmap_format_unused1
	D3DFMT_UNKNOWN, // _bitmap_format_unused2
	D3DFMT_R5G6B5, // _bitmap_format_r5g6b5
	D3DFMT_UNKNOWN, // _bitmap_format_unused3
	D3DFMT_A1R5G5B5, // _bitmap_format_a1r5g5b5
	D3DFMT_A4R4G4B4, // _bitmap_format_a4r4g4b4
	D3DFMT_X8R8G8B8, // _bitmap_format_x8r8g8b8
	D3DFMT_A8R8G8B8, // _bitmap_format_a8r8g8b8
	D3DFMT_UNKNOWN, // _bitmap_format_unused4
	D3DFMT_UNKNOWN, // _bitmap_format_unused5
	D3DFMT_DXT1, // _bitmap_format_dxt1
	D3DFMT_DXT3, // _bitmap_format_dxt3
	D3DFMT_DXT5, // _bitmap_format_dxt5
	D3DFMT_P8 // _bitmap_format_p8_bump
};

/* ---------- public code */

boolean rasterizer_bitmap_new(
	struct bitmap_data *bitmap)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_bitmaps.c", 51, bitmap);
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_bitmaps.c", 52, TEST_FLAG(bitmap->flags, _bitmap_has_power_of_two_dimensions_bit));

	bitmap->mipmap_count = rasterizer_xbox_bitmap_get_max_mipmap_count(bitmap);

	if (global_d3d_device)
	{
		switch (bitmap->type)
		{
		case _bitmap_type_2d:
			D3DCALL(success, IDirect3DDevice8_CreateTexture(global_d3d_device,
				bitmap->width,
				bitmap->height,
				bitmap->mipmap_count+1,
				0,
				rasterizer_bitmap_format_table[bitmap->format],
				D3DPOOL_MANAGED,
				&(IDirect3DTexture8*)bitmap->hardware_format));
			break;
		case _bitmap_type_3d:
			D3DCALL(success, IDirect3DDevice8_CreateVolumeTexture(global_d3d_device,
				bitmap->width,
				bitmap->height,
				bitmap->depth,
				bitmap->mipmap_count+1,
				0,
				rasterizer_bitmap_format_table[bitmap->format],
				D3DPOOL_MANAGED,
				&(IDirect3DVolumeTexture8*)bitmap->hardware_format));
			break;
		case _bitmap_type_cube_map:
			D3DCALL(success, IDirect3DDevice8_CreateCubeTexture(global_d3d_device,
				bitmap->width,
				bitmap->mipmap_count+1,
				0,
				rasterizer_bitmap_format_table[bitmap->format],
				D3DPOOL_MANAGED,
				&(IDirect3DCubeTexture8*)bitmap->hardware_format));
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_bitmaps.c", 91, FALSE, "### ERROR unsupported bitmap type");
		}

		if (!bitmap->hardware_format)
		{
			success = FALSE;
		}

		if (!success)
		{
			bitmap->hardware_format = NULL;
		}
	}
	else
	{
		bitmap->hardware_format = NULL;
		success = TRUE;
	}

	if (!success)
	{
		error(_error_silent, "### ERROR failed to create bitmap hardware format");
	}

	return success;
}

void rasterizer_bitmap_changed(
	struct bitmap_data *bitmap)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_bitmaps.c", 112, bitmap);

	rasterizer_globals.current_lock_operation = _rasterizer_lock_texture_changed;

	switch (bitmap->type)
	{
	case _bitmap_type_2d:
		rasterizer_bitmap_2d_changed(bitmap);
		break;
	case _bitmap_type_3d:
		rasterizer_bitmap_3d_changed(bitmap);
		break;
	case _bitmap_type_cube_map:
		rasterizer_bitmap_cm_changed(bitmap);
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_bitmaps.c", 128, FALSE, "### ERROR unsupported bitmap type");
	}

	rasterizer_globals.current_lock_operation = _rasterizer_lock_none;

	return;
}

static void rasterizer_bitmap_2d_changed(
	struct bitmap_data *bitmap)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_bitmaps.c", 141, bitmap);

	if (global_d3d_device && bitmap->base_address && bitmap->hardware_format)
	{
		short mipmap_index;

		for (mipmap_index= 0; success && mipmap_index<=bitmap->mipmap_count; mipmap_index++)
		{
			D3DLOCKED_RECT d3d_locked_rect;

			D3DCALL(success, IDirect3DTexture8_LockRect((IDirect3DTexture8*)bitmap->hardware_format, mipmap_index, &d3d_locked_rect, NULL, D3DLOCK_NOOVERWRITE));

			if (success && d3d_locked_rect.pBits)
			{
				void *source = bitmap_mipmap_address(bitmap, mipmap_index);
				void *destination = d3d_locked_rect.pBits;
				short width = bitmap_mipmap_get_width(bitmap, mipmap_index);
				short height = bitmap_mipmap_get_height(bitmap, mipmap_index);

				if (TEST_FLAG(bitmap->flags, _bitmap_compressed_bit))
				{
					memcpy(destination, source, bitmap_mipmap_get_pixel_data_size(bitmap, mipmap_index));
				}
				else
				{
					switch (bitmap_format_get_bits_per_pixel(bitmap->format)/CHAR_BITS)
					{
					case 1:
						rasterizer_xbox_bitmap_swizzle2d_byte(destination, source, width, height);
						break;
					case 2:
						rasterizer_xbox_bitmap_swizzle2d_word(destination, source, width, height);
						break;
					case 4:
						rasterizer_xbox_bitmap_swizzle2d_long(destination, source, width, height);
						break;
					default:
						match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_bitmaps.c", 177, FALSE, "### ERROR uncompressed bitmap format does not have 1,2 or 4 bytes per pixel");
					}
				}

				D3DCALL(success, IDirect3DTexture8_UnlockRect((IDirect3DTexture8*)bitmap->hardware_format, mipmap_index));
			}
			else
			{
				error(_error_silent, "### ERROR failed to lock surface");
				success = FALSE;
			}
		}

		if (!success)
		{
			error(_error_silent, "### ERROR failed to change bitmap hardware format");
		}
	}

	return;
}

static void rasterizer_bitmap_3d_changed(
	struct bitmap_data *bitmap)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_bitmaps.c", 203, bitmap);

	if (global_d3d_device && bitmap->base_address && bitmap->hardware_format)
	{
		short mipmap_index;

		for (mipmap_index= 0; success && mipmap_index<=bitmap->mipmap_count; mipmap_index++)
		{
			D3DLOCKED_BOX d3d_locked_box;

			D3DCALL(success, IDirect3DVolumeTexture8_LockBox((IDirect3DVolumeTexture8*)bitmap->hardware_format, mipmap_index, &d3d_locked_box, NULL, D3DLOCK_NOOVERWRITE));

			if (success && d3d_locked_box.pBits)
			{
				byte *source = bitmap_mipmap_address(bitmap, mipmap_index);
				byte *destination = d3d_locked_box.pBits;
				short width = bitmap_mipmap_get_width(bitmap, mipmap_index);
				short height = bitmap_mipmap_get_height(bitmap, mipmap_index);
				short depth = bitmap_mipmap_get_depth(bitmap, mipmap_index);

				if (TEST_FLAG(bitmap->flags, _bitmap_compressed_bit))
				{
					short slice_index;

					for (slice_index= 0; slice_index<depth; slice_index++)
					{
						long slice_size = bitmap_mipmap_get_pixel_data_size(bitmap, mipmap_index)/depth;

						memcpy(destination, source, slice_size);
						source+= slice_size;
						destination+= d3d_locked_box.SlicePitch;
					}
				}
				else
				{
					switch (bitmap_format_get_bits_per_pixel(bitmap->format)/CHAR_BITS)
					{
					case 1:
						rasterizer_xbox_bitmap_swizzle3d_byte(destination, source, width, height, depth);
						break;
					case 2:
						rasterizer_xbox_bitmap_swizzle3d_word(destination, source, width, height, depth);
						break;
					case 4:
						rasterizer_xbox_bitmap_swizzle3d_long(destination, source, width, height, depth);
						break;
					default:
						match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_bitmaps.c", 249, FALSE, "### ERROR uncompressed bitmap format does not have 1,2 or 4 bytes per pixel");
					}
				}

				D3DCALL(success, IDirect3DVolumeTexture8_UnlockBox((IDirect3DVolumeTexture8*)bitmap->hardware_format, mipmap_index));
			}
			else
			{
				error(_error_silent, "### ERROR failed to lock surface");
				success = FALSE;
			}
		}

		if (!success)
		{
			error(_error_silent, "### ERROR failed to change bitmap hardware format");
		}
	}

	return;
}

static void rasterizer_bitmap_cm_changed(
	struct bitmap_data *bitmap)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_bitmaps.c", 276, bitmap);

	if (global_d3d_device && bitmap->base_address && bitmap->hardware_format)
	{
		static const short face_mapping_table[] = 
		{
			D3DCUBEMAP_FACE_POSITIVE_X, 
			D3DCUBEMAP_FACE_POSITIVE_Y, 
			D3DCUBEMAP_FACE_NEGATIVE_X, 
			D3DCUBEMAP_FACE_NEGATIVE_Y, 
			D3DCUBEMAP_FACE_POSITIVE_Z, 
			D3DCUBEMAP_FACE_NEGATIVE_Z
		};

		short mipmap_index;

		for (mipmap_index= 0; success && mipmap_index<=bitmap->mipmap_count; mipmap_index++)
		{
			short face_index;

			for (face_index= 0; success && face_index<6; face_index++)
			{
				D3DLOCKED_RECT d3d_locked_rect;

				D3DCALL(success, IDirect3DCubeTexture8_LockRect((IDirect3DCubeTexture8*)bitmap->hardware_format, face_mapping_table[face_index], mipmap_index, &d3d_locked_rect, NULL, D3DLOCK_NOOVERWRITE));

				if (success && d3d_locked_rect.pBits)
				{
					void *source = bitmap_cube_map_address(bitmap, 0, 0, face_index, mipmap_index);
					void *destination = d3d_locked_rect.pBits;
					short width = bitmap_mipmap_get_width(bitmap, mipmap_index);
					short height = bitmap_mipmap_get_height(bitmap, mipmap_index);

					if (TEST_FLAG(bitmap->flags, _bitmap_compressed_bit))
					{
						memcpy(destination, source, bitmap_mipmap_get_pixel_data_size(bitmap, mipmap_index)/6);
					}
					else
					{
						switch (bitmap_format_get_bits_per_pixel(bitmap->format)/CHAR_BITS)
						{
						case 1:
							rasterizer_xbox_bitmap_swizzle2d_byte(destination, source, width, height);
							break;
						case 2:
							rasterizer_xbox_bitmap_swizzle2d_word(destination, source, width, height);
							break;
						case 4:
							rasterizer_xbox_bitmap_swizzle2d_long(destination, source, width, height);
							break;
						default:
							match_vassert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_hardware_bitmaps.c", 319, FALSE, "### ERROR uncompressed bitmap format does not have 1,2 or 4 bytes per pixel");
						}
					}

					D3DCALL(success, IDirect3DCubeTexture8_UnlockRect((IDirect3DCubeTexture8*)bitmap->hardware_format, face_mapping_table[face_index], mipmap_index));
				}
				else
				{
					error(_error_silent, "### ERROR failed to lock surface");
					success = FALSE;
				}
			}
		}

		if (!success)
		{
			error(_error_silent, "### ERROR failed to change bitmap hardware format");
		}
	}

	return;
}

void rasterizer_bitmap_delete(
	struct bitmap_data *bitmap)
{
	texture_cache_bitmap_delete(bitmap);

	if (bitmap && bitmap->hardware_format)
	{
		IDirect3DBaseTexture8_Release((IDirect3DBaseTexture8*)bitmap->hardware_format);
		bitmap->hardware_format = NULL;
	}

	return;
}
