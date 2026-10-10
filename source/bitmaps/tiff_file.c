/*
TIFF_FILE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "bitmaps.h"
#include "bitmap_macros.h"
#include "tiffio.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static char tiff_error_string[512]; /* fake name */

/* ---------- public code */

boolean tiff_get_bounds(
	struct file_reference *file,
	long *width,
	long *height)
{
	char name[MAXIMUM_FILENAME_LENGTH+1];
	boolean success = FALSE;
	TIFF *tiff = TIFFOpen(file_reference_get_name(file, FLAG(_name_directory_bit)|FLAG(_name_filename_bit)|FLAG(_name_extension_bit), name), "r");

	if (tiff)
	{
		TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, width);
		TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, height);
		TIFFClose(tiff);
		success = TRUE;
	}

	return success;
}

char *tiff_export(
	struct file_reference *file,
	struct bitmap_data const *bitmap)
{
	short format;
	short samples_per_pixel;
	short bits_per_sample;
	short photometric;
	char *error = NULL;

	switch (bitmap->format)
	{
	case _bitmap_format_a8:
	case _bitmap_format_y8:
	case _bitmap_format_ay8:
		format = _bitmap_format_a8;
		photometric = PHOTOMETRIC_MINISBLACK;
		samples_per_pixel = 1;
		bits_per_sample = 8;
		break;
	case _bitmap_format_r5g6b5:
	case _bitmap_format_a1r5g5b5:
	case _bitmap_format_a4r4g4b4:
	case _bitmap_format_x8r8g8b8:
	case _bitmap_format_a8r8g8b8:
		format = _bitmap_format_a8r8g8b8;
		photometric = PHOTOMETRIC_RGB;
		samples_per_pixel = 4;
		bits_per_sample = 8;
		break;
	default:
		format = NONE;
		photometric = NONE;
		samples_per_pixel = NONE;
		bits_per_sample = NONE;
		error = "invalid bitmap encoding for tiff export.";
		break;
	}

	if (!error)
	{
		char name[MAXIMUM_FILENAME_LENGTH+1];
		TIFF *tiff = TIFFOpen(file_reference_get_name(file, FLAG(_name_directory_bit)|FLAG(_name_filename_bit)|FLAG(_name_extension_bit), name), "w");

		if (tiff)
		{
			short row_size = bitmap_format_get_bits_per_pixel(format)*bitmap->width/8;
			byte *buffer = match_malloc("c:\\halo\\SOURCE\\bitmaps\\tiff_file.c", 107, row_size);

			if (buffer)
			{
				short y;

				TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, bitmap->width);
				TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, bitmap->height);
				TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
				TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, photometric);
				TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
				TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, samples_per_pixel);
				TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, bits_per_sample);
				TIFFSetField(tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

				for (y = 0; y<bitmap->height; y++)
				{
					void *pixels = bitmap_2d_address(bitmap, 0, y, 0);

					switch (bitmap->format)
					{
					case _bitmap_format_a1r5g5b5:
						{
							short x;

							for (x = 0; x<bitmap->width; x++)
							{
								word pixel = ((word *)pixels)[x];

								buffer[4*x+3] = 0xff;
								buffer[4*x+2] = PIXEL16_1555_BLUE(pixel);
								buffer[4*x+1] = PIXEL16_1555_GREEN(pixel);
								buffer[4*x+0] = PIXEL16_1555_RED(pixel);
							}
						}
						break;
					case _bitmap_format_r5g6b5:
						{
							short x;

							for (x = 0; x<bitmap->width; x++)
							{
								word pixel = ((word *)pixels)[x];

								buffer[4*x+3] = 0xff;
								buffer[4*x+2] = PIXEL16_565_BLUE(pixel);
								buffer[4*x+1] = PIXEL16_565_GREEN(pixel);
								buffer[4*x+0] = PIXEL16_565_RED(pixel);
							}
						}
						break;
					case _bitmap_format_a4r4g4b4:
						{
							short x;

							for (x = 0; x<bitmap->width; x++)
							{
								word pixel = ((word *)pixels)[x];

								buffer[4*x+3] = PIXEL16_4444_ALPHA(pixel);
								buffer[4*x+2] = PIXEL16_4444_BLUE(pixel);
								buffer[4*x+1] = PIXEL16_4444_GREEN(pixel);
								buffer[4*x+0] = PIXEL16_4444_RED(pixel);
							}
						}
						break;
					case _bitmap_format_x8r8g8b8:
						{
							short x;

							for (x = 0; x<bitmap->width; x++)
							{
								pixel32 pixel = ((pixel32 *)pixels)[x];

								buffer[4*x+3] = 0xff;
								buffer[4*x+2] = (byte)PIXEL32_BLUE(pixel);
								buffer[4*x+1] = (byte)PIXEL32_GREEN(pixel);
								buffer[4*x+0] = (byte)PIXEL32_RED(pixel);
							}
						}
						break;
					case _bitmap_format_a8r8g8b8:
						{
							short x;

							for (x = 0; x<bitmap->width; x++)
							{
								pixel32 pixel = ((pixel32 *)pixels)[x];

								buffer[4*x+3] = (byte)PIXEL32_ALPHA(pixel);
								buffer[4*x+2] = (byte)PIXEL32_BLUE(pixel);
								buffer[4*x+1] = (byte)PIXEL32_GREEN(pixel);
								buffer[4*x+0] = (byte)PIXEL32_RED(pixel);
							}
						}
						break;
					default:
						memcpy(buffer, pixels, row_size);
						break;
					}

					if (TIFFWriteScanline(tiff, buffer, y, 0)<0)
					{
						error = "failed to write scanline";
						break;
					}
				}

				match_free("c:\\halo\\SOURCE\\bitmaps\\tiff_file.c", 231, buffer);
			}
			else
			{
				error = "out of memory";
			}

			TIFFClose(tiff);
		}
		else
		{
			error = "failed to open tiff";
		}
	}

	return error;
}

char *tiff_import(
	struct file_reference *file,
	struct bitmap_data **bitmap,
	rectangle2d const *bounds,
	short format)
{
	char *error = NULL;

	if (file_exists(file))
	{
		char name[MAXIMUM_FILENAME_LENGTH+1];
		TIFF *tiff = TIFFOpen(file_reference_get_name(file, FLAG(_name_directory_bit)|FLAG(_name_filename_bit)|FLAG(_name_extension_bit), name), "r");

		if (tiff)
		{
			rectangle2d import_bounds;
			unsigned short bits_per_sample;
			unsigned short samples_per_pixel;
			unsigned short planar_configuration;
			unsigned short photometric;
			unsigned short orientation;
			unsigned long width;
			unsigned long height;
			short import_format;
			long scanline_size = TIFFScanlineSize(tiff);

			TIFFGetFieldDefaulted(tiff, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
			TIFFGetFieldDefaulted(tiff, TIFFTAG_ORIENTATION, &orientation);
			TIFFGetFieldDefaulted(tiff, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel);
			TIFFGetField(tiff, TIFFTAG_PLANARCONFIG, &planar_configuration);
			TIFFGetField(tiff, TIFFTAG_PHOTOMETRIC, &photometric);
			TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
			TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);

			if (bounds)
			{
				import_bounds = *bounds;
			}
			else
			{
				import_bounds.y0 = 0;
				import_bounds.x0 = 0;
				import_bounds.x1 = (short)width;
				import_bounds.y1 = (short)height;
			}

			if (orientation==ORIENTATION_TOPLEFT)
			{
				import_format = NONE;
				if (bits_per_sample==8 && samples_per_pixel==4) import_format = _bitmap_format_a8r8g8b8;
				if (bits_per_sample==8 && samples_per_pixel==3) import_format = _bitmap_format_a8r8g8b8;
				if (bits_per_sample==8 && samples_per_pixel==2) import_format = _bitmap_format_a8r8g8b8;
				if (bits_per_sample==8 && samples_per_pixel==1) import_format = _bitmap_format_a8r8g8b8;

				if (import_format!=NONE)
				{
					if (format==NONE || import_format==format)
					{
						if (planar_configuration==PLANARCONFIG_CONTIG)
						{
							short import_width = rectangle2d_width(&import_bounds);
							short import_height = rectangle2d_height(&import_bounds);

							if (import_width<0 || import_width>MAXIMUM_BITMAP_WIDTH || import_height<0 || import_height>MAXIMUM_BITMAP_HEIGHT)
							{
								error = "TIFF too large";
							}
							else
							{
								struct bitmap_data *new_bitmap = bitmap_2d_new(import_width, import_height, 0, import_format);
								byte *buffer = match_malloc("c:\\halo\\SOURCE\\bitmaps\\tiff_file.c", 319, scanline_size);

								if (new_bitmap && buffer)
								{
									short y;

									*bitmap = new_bitmap;
									for (y = import_bounds.y0; y<import_bounds.y1; y++)
									{
										if (TIFFReadScanline(tiff, buffer, PIN(y, 0, height-1), 0)<0)
										{
											error = "failed to read TIFF scan line";
											break;
										}

										switch (samples_per_pixel)
										{
										case 1:
											{
												short x;
												pixel32 *pixels = (pixel32 *)bitmap_2d_address(new_bitmap, 0, y-import_bounds.y0, 0);

												for (x = import_bounds.x0; x<import_bounds.x1; x++)
												{
													short pixel_index = PIN(x, 0, width-1);
													pixel32 value = buffer[pixel_index];

													pixels[x-import_bounds.x0] = PIXEL32_FROM_ARGB(value, value, value, value);
												}
											}
											break;
										case 2:
											{
												short x;
												pixel32 *pixels = (pixel32 *)bitmap_2d_address(new_bitmap, 0, y-import_bounds.y0, 0);

												for (x = import_bounds.x0; x<import_bounds.x1; x++)
												{
													short pixel_index = PIN(x, 0, width-1);

													pixels[x-import_bounds.x0] = PIXEL32_FROM_ARGB(buffer[2*pixel_index+1], buffer[2*pixel_index], buffer[2*pixel_index], buffer[2*pixel_index]);
												}
											}
											break;
										case 3:
											{
												short x;
												pixel32 *pixels = (pixel32 *)bitmap_2d_address(new_bitmap, 0, y-import_bounds.y0, 0);

												for (x = import_bounds.x0; x<import_bounds.x1; x++)
												{
													short pixel_index = PIN(x, 0, width-1);

													pixels[x-import_bounds.x0] = PIXEL32_FROM_ARGB(0xff, buffer[3*pixel_index], buffer[3*pixel_index+1], buffer[3*pixel_index+2]);
												}
											}
											break;
										case 4:
											{
												short x;
												pixel32 *pixels = (pixel32 *)bitmap_2d_address(new_bitmap, 0, y-import_bounds.y0, 0);

												for (x = import_bounds.x0; x<import_bounds.x1; x++)
												{
													short pixel_index = PIN(x, 0, width-1);

													pixels[x-import_bounds.x0] = PIXEL32_FROM_ARGB(buffer[4*pixel_index+3], buffer[4*pixel_index], buffer[4*pixel_index+1], buffer[4*pixel_index+2]);
												}
											}
											break;
										default:
											match_vassert("c:\\halo\\SOURCE\\bitmaps\\tiff_file.c", 406, FALSE, NULL);
											break;
										}
									}
								}
								else
								{
									error = "out of memory";
								}

								if (error && new_bitmap)
								{
									bitmap_delete(new_bitmap);
								}
								if (buffer)
								{
									match_free("c:\\halo\\SOURCE\\bitmaps\\tiff_file.c", 422, buffer);
								}
							}
						}
						else
						{
							error = "unsupported TIFF photometric, planar configuration";
						}
					}
					else
					{
						error = "unsupported format";
					}
				}
				else
				{
					_snprintf(tiff_error_string, sizeof(tiff_error_string), "unsupported bits per sample (%d) or sample count (%d)", bits_per_sample, samples_per_pixel);
					error = tiff_error_string;
				}
			}
			else
			{
				error = "unsupported TIFF orientation (must be top left)";
			}

			TIFFClose(tiff);
		}
		else
		{
			error = "not a TIFF file";
		}
	}
	else
	{
		error = "file does not exist";
	}

	return error;
}

/* ---------- private code */
