/*
TARGA_FILE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "bitmaps.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct targa_header /* fake name */
{
	unsigned char id_length;
	unsigned char colormap_type;
	unsigned char image_type;
	unsigned char colormap_specification[5];
	unsigned short x_origin;
	unsigned short y_origin;
	unsigned short image_width;
	unsigned short image_height;
	unsigned char pixel_depth;
	unsigned char image_descriptor;
};

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

char *targa_export(
	struct file_reference *file,
	struct bitmap_data const *bitmap)
{
	char *error = NULL;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\targa_file.c", 36, file);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\targa_file.c", 37, bitmap);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\targa_file.c", 38, bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\targa_file.c", 39, bitmap->format==_bitmap_format_x8r8g8b8);

	if (file_create(file) && file_open(file, FLAG(_permission_write_bit)))
	{
		struct targa_header header;

		memset(&header, 0, sizeof(header));
		header.id_length = 0;
		header.colormap_type = 0;
		header.image_type = 2;
		header.x_origin = 0;
		header.y_origin = 0;
		header.image_width = bitmap->width;
		header.image_height = bitmap->height;
		header.pixel_depth = 32;
		header.image_descriptor = 40;

		if (file_write(file, sizeof(header), &header))
		{
			long y;
			long row_size = bitmap->width*sizeof(pixel32);

			for (y = 0; y<bitmap->height; y++)
			{
				pixel32 *pixels = (pixel32 *)bitmap_2d_address(bitmap, 0, (short)y, 0);

				match_assert("c:\\halo\\SOURCE\\bitmaps\\targa_file.c", 67, pixels);
				if (!file_write(file, row_size, pixels))
				{
					error = "couldn't write row";
					break;
				}
			}
		}
		else
		{
			error = "couldn't write header";
		}

		file_close(file);
	}
	else
	{
		error = "couldn't open file";
	}

	return error;
}

/* ---------- private code */
