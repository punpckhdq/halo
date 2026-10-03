/*
BITMAP_DRAWING.C
*/

/* ---------- headers */

#include "cseries.h"
#include "bitmap_drawing.h"
#include "bitmap_macros.h"

/* ---------- constants */

enum
{
	_bitmap_copy_none = 0, /* fake name */
	_bitmap_copy_16bit, /* fake name */
	_bitmap_copy_a1r5g5b5_to_r5g6b5, /* fake name */
	_bitmap_copy_r5g6b5_to_a1r5g5b5, /* fake name */
	_bitmap_copy_r5g6b5_to_a8r8g8b8, /* fake name */
	_bitmap_copy_a4r4g4b4_to_a8r8g8b8, /* fake name */
	_bitmap_copy_a8r8g8b8, /* fake name */
	_bitmap_copy_a8r8g8b8_to_r5g6b5, /* fake name */
	_bitmap_copy_a8r8g8b8_to_a4r4g4b4, /* fake name */
	_bitmap_copy_a8r8g8b8_blend, /* fake name */
	_bitmap_copy_a4r4g4b4_to_a8r8g8b8_blend, /* fake name */
	_bitmap_copy_a8r8g8b8_modulate, /* fake name */
	_bitmap_copy_a4r4g4b4_to_a8r8g8b8_modulate_blend, /* fake name */
	_bitmap_copy_a8r8g8b8_modulate_blend /* fake name */
};

enum
{
	_bitmap_fill_write_16bit = 0, /* fake name */
	_bitmap_fill_blend_a1r5g5b5, /* fake name */
	_bitmap_fill_blend_r5g6b5 /* fake name */
};

enum
{
	_bitmap_pixel_write_8bit = 0, /* fake name */
	_bitmap_pixel_write_16bit, /* fake name */
	_bitmap_pixel_write_32bit, /* fake name */
	_bitmap_pixel_blend_a1r5g5b5, /* fake name */
	_bitmap_pixel_blend_r5g6b5, /* fake name */
	_bitmap_pixel_blend_a8r8g8b8 /* fake name */
};

enum
{
	_bevel_left_bit = 0, /* fake name */
	_bevel_right_bit, /* fake name */
	_bevel_top_bit, /* fake name */
	_bevel_bottom_bit, /* fake name */
	_bevel_corner_bit, /* fake name */
	_bevel_clip_bit, /* fake name */
	_bevel_center_bit /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static short bitmap_drawing_unused[8] = {NONE, NONE, NONE, NONE, NONE}; /* fake name */

short translation_table[NUMBER_OF_BITMAP_FORMATS][NUMBER_OF_BITMAP_FORMATS][1<<NUMBER_OF_BITMAP_COPY_FLAGS] =
{
	{0}, // a8
	{0}, // y8
	{0}, // ay8
	{0}, // a8y8
	{0}, // unused1
	{0}, // unused2
	{ // r5g6b5
		{0}, // a8
		{0}, // y8
		{0}, // ay8
		{0}, // a8y8
		{0}, // unused1
		{0}, // unused2
		{_bitmap_copy_16bit}, // r5g6b5
		{0}, // unused3
		{0}, // a1r5g5b5
		{0}, // a4r4g4b4
		{0}, // x8r8g8b8
		{_bitmap_copy_r5g6b5_to_a8r8g8b8, _bitmap_copy_r5g6b5_to_a8r8g8b8} // a8r8g8b8
	},
	{0}, // unused3
	{0}, // a1r5g5b5
	{ // a4r4g4b4
		{0}, // a8
		{0}, // y8
		{0}, // ay8
		{0}, // a8y8
		{0}, // unused1
		{0}, // unused2
		{0}, // r5g6b5
		{0}, // unused3
		{0}, // a1r5g5b5
		{_bitmap_copy_16bit}, // a4r4g4b4
		{0}, // x8r8g8b8
		{_bitmap_copy_a4r4g4b4_to_a8r8g8b8, _bitmap_copy_a4r4g4b4_to_a8r8g8b8_blend, _bitmap_copy_none, _bitmap_copy_a4r4g4b4_to_a8r8g8b8_modulate_blend} // a8r8g8b8
	},
	{ // x8r8g8b8
		{0}, // a8
		{0}, // y8
		{0}, // ay8
		{0}, // a8y8
		{0}, // unused1
		{0}, // unused2
		{_bitmap_copy_a8r8g8b8_to_r5g6b5}, // r5g6b5
		{0}, // unused3
		{0}, // a1r5g5b5
		{0}, // a4r4g4b4
		{0}, // x8r8g8b8
		{_bitmap_copy_a8r8g8b8, _bitmap_copy_a8r8g8b8, _bitmap_copy_a8r8g8b8_modulate, _bitmap_copy_a8r8g8b8_modulate} // a8r8g8b8
	},
	{ // a8r8g8b8
		{0}, // a8
		{0}, // y8
		{0}, // ay8
		{0}, // a8y8
		{0}, // unused1
		{0}, // unused2
		{_bitmap_copy_a8r8g8b8_to_r5g6b5}, // r5g6b5
		{_bitmap_copy_a8r8g8b8_to_a4r4g4b4}, // unused3
		{0}, // a1r5g5b5
		{0}, // a4r4g4b4
		{0}, // x8r8g8b8
		{_bitmap_copy_a8r8g8b8, _bitmap_copy_a8r8g8b8_blend, _bitmap_copy_a8r8g8b8_modulate, _bitmap_copy_a8r8g8b8_modulate_blend} // a8r8g8b8
	}
};

static unsigned long bevel_piece_flags[] = /* fake name */
{
	FLAG(_bevel_center_bit), // center
	FLAG(_bevel_left_bit)|FLAG(_bevel_top_bit)|FLAG(_bevel_corner_bit)|FLAG(_bevel_clip_bit), // top_left
	FLAG(_bevel_right_bit)|FLAG(_bevel_top_bit)|FLAG(_bevel_corner_bit)|FLAG(_bevel_clip_bit), // top_right
	FLAG(_bevel_left_bit)|FLAG(_bevel_bottom_bit)|FLAG(_bevel_corner_bit)|FLAG(_bevel_clip_bit), // bottom_left
	FLAG(_bevel_right_bit)|FLAG(_bevel_bottom_bit)|FLAG(_bevel_corner_bit)|FLAG(_bevel_clip_bit), // bottom_right
	FLAG(_bevel_top_bit)|FLAG(_bevel_clip_bit), // top
	FLAG(_bevel_left_bit)|FLAG(_bevel_clip_bit), // left
	FLAG(_bevel_bottom_bit)|FLAG(_bevel_clip_bit), // bottom
	FLAG(_bevel_right_bit)|FLAG(_bevel_clip_bit) // right
};

static word const bevel_piece_parts[] = /* fake name */
{
	0, // center
	1, // top_left
	1, // top_right
	1, // bottom_left
	1, // bottom_right
	2, // top
	2, // left
	2, // bottom
	2 // right
};

/* ---------- public code */

void bitmap_draw_line(
	struct bitmap_data *destination,
	pixel32 color,
	rectangle2d const *clip_rectangle,
	real_point2d const *p0,
	real_point2d const *p1)
{
	short mode;
	long pixel;
	point2d point0;
	point2d point1;
	struct bitmap_line line;
	short x0 = 0;
	short x1 = destination->width;
	short y0 = 0;
	short y1 = destination->height;
	short alpha = (short)PIXEL32_ALPHA(color);
	short inverse_alpha = 255-alpha;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_drawing.c", 56, destination->base_address);

	switch (destination->format)
	{
	case _bitmap_format_a8:
	case _bitmap_format_y8:
	case _bitmap_format_ay8:
		pixel = PIXEL32_ALPHA(color);
		mode = _bitmap_pixel_write_8bit;
		break;
	case _bitmap_format_r5g6b5:
		pixel = PIXEL32_TO_PIXEL16_565_COLOR(color);
		mode = (alpha!=255) ? _bitmap_pixel_blend_r5g6b5 : _bitmap_pixel_write_16bit;
		break;
	case _bitmap_format_a8r8g8b8:
		pixel = color;
		mode = (alpha!=255) ? _bitmap_pixel_blend_a8r8g8b8 : _bitmap_pixel_write_32bit;
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_drawing.c", 82, FALSE,
			csprintf(temporary, "bitmap @%p has bad encoding #%d for draw_line_software()", destination, destination->format));
		break;
	}

	if (clip_rectangle)
	{
		if (clip_rectangle->x0>x0)
		{
			x0 = clip_rectangle->x0;
		}
		if (clip_rectangle->x1<x1)
		{
			x1 = clip_rectangle->x1;
		}
		if (clip_rectangle->y0>y0)
		{
			y0 = clip_rectangle->y0;
		}
		if (clip_rectangle->y1<y1)
		{
			y1 = clip_rectangle->y1;
		}
	}

	if (p0->y>p1->y)
	{
		real_point2d const *swap = p0;

		p0 = p1;
		p1 = swap;
	}

	point0.x = (short)p0->x;
	point0.y = (short)p0->y;
	point1.x = (short)p1->x;
	point1.y = (short)p1->y;
	bitmap_initialize_line(&line, &point0, &point1);

	do
	{
		if (line.point.x>=x0 && line.point.x<x1 && line.point.y>=y0 && line.point.y<y1)
		{
			void *address = bitmap_2d_address(destination, line.point.x, line.point.y, 0);

			switch (mode)
			{
			case _bitmap_pixel_write_8bit:
				*(byte *)address = (byte)pixel;
				break;
			case _bitmap_pixel_write_16bit:
				*(word *)address = (word)pixel;
				break;
			case _bitmap_pixel_write_32bit:
				*(pixel32 *)address = (pixel32)pixel;
				break;
			case _bitmap_pixel_blend_a1r5g5b5:
				{
					word destination_pixel = *(word *)address;

					*(word *)address = PIXEL16_1555_BLEND(pixel, destination_pixel, alpha, inverse_alpha);
				}
				break;
			case _bitmap_pixel_blend_r5g6b5:
				{
					word destination_pixel = *(word *)address;

					*(word *)address = PIXEL16_565_BLEND(pixel, destination_pixel, alpha, inverse_alpha);
				}
				break;
			case _bitmap_pixel_blend_a8r8g8b8:
				{
					pixel32 destination_pixel = *(pixel32 *)address;

					*(pixel32 *)address = PIXEL32_BLEND(pixel, destination_pixel, alpha, inverse_alpha);
				}
				break;
			}
		}
	}
	while (!bitmap_step_line(&line, _bitmap_line_step_pixel));

	return;
}

void bitmap_initialize_line(
	struct bitmap_line *line,
	point2d const *p0,
	point2d const *p1)
{
	line->dx = p1->x - p0->x;
	line->dy = p1->y - p0->y;
	line->two_dx = 2*ABS(line->dx);
	line->two_dy = 2*ABS(line->dy);
	line->x_step = line->dx ? (line->dx>=0 ? 1 : -1) : 0;
	line->y_step = line->dy ? (line->dy>=0 ? 1 : -1) : 0;
	line->point = *p0;
	line->end_point = *p1;
	line->error = (line->two_dx>line->two_dy) ?
		(line->two_dy - (line->two_dx>>1)) :
		(line->two_dx - (line->two_dy>>1));

	return;
}

boolean bitmap_step_line(
	struct bitmap_line *line,
	short step_mode)
{
	boolean done = FALSE;

	if (line->two_dx>line->two_dy)
	{
		if (line->point.x==line->end_point.x)
		{
			done = TRUE;
		}
		else
		{
			switch (step_mode)
			{
			case _bitmap_line_step_pixel:
				if (line->error>=0)
				{
					line->point.y += line->y_step;
					line->error -= line->two_dx;
				}
				line->point.x += line->x_step;
				line->error += line->two_dy;
				break;
			case _bitmap_line_step_horizontal:
				while (line->error<0 && line->point.x!=line->end_point.x)
				{
					line->error += line->two_dy;
					line->point.x += line->x_step;
				}
				break;
			}
		}
	}
	else
	{
		if (line->point.y==line->end_point.y)
		{
			done = TRUE;
		}
		else
		{
			switch (step_mode)
			{
			case _bitmap_line_step_pixel:
				if (line->error>=0)
				{
					line->point.x += line->x_step;
					line->error -= line->two_dy;
				}
				line->point.y += line->y_step;
				line->error += line->two_dx;
				break;
			case _bitmap_line_step_vertical:
				while (line->error<0 && line->point.y!=line->end_point.y)
				{
					line->error += line->two_dx;
					line->point.y += line->y_step;
				}
				break;
			}
		}
	}

	return done;
}

void bitmap_fill_rectangle(
	struct bitmap_data *destination,
	pixel32 color,
	rectangle2d const *rectangle,
	rectangle2d const *clip_rectangle)
{
	rectangle2d bounds = *rectangle;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_drawing.c", 275, destination);

	if (!clip_rectangle || intersect_rectangles2d(rectangle, clip_rectangle, &bounds))
	{
		short mode;
		long pixel;
		short x0, x1, y0, y1;
		short width;
		short y;
		short alpha = (short)PIXEL32_ALPHA(color);
		short inverse_alpha = 255-alpha;

		switch (destination->format)
		{
		case _bitmap_format_r5g6b5:
			pixel = PIXEL32_TO_PIXEL16_565_COLOR(color);
			mode = (alpha!=255) ? _bitmap_fill_blend_r5g6b5 : _bitmap_fill_write_16bit;
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_drawing.c", 301, FALSE,
				csprintf(temporary, "bitmap @%p has bad encoding #%d for fill_rectangle()", destination, destination->format));
			break;
		}

		x0 = PIN(bounds.x0, 0, destination->width);
		x1 = PIN(bounds.x1, 0, destination->width);
		y0 = PIN(bounds.y0, 0, destination->height);
		y1 = PIN(bounds.y1, 0, destination->height);
		width = x1-x0;

		for (y = y0; y<y1; y++)
		{
			short x;
			word *address = (word *)bitmap_2d_address(destination, x0, y, 0);

			switch (mode)
			{
			case _bitmap_fill_write_16bit:
				for (x = 0; x<width; x++)
				{
					*address++ = (word)pixel;
				}
				break;
			case _bitmap_fill_blend_a1r5g5b5:
				for (x = 0; x<width; x++)
				{
					word destination_pixel = *address;

					*address = PIXEL16_1555_BLEND(pixel, destination_pixel, alpha, inverse_alpha);
				}
				break;
			case _bitmap_fill_blend_r5g6b5:
				for (x = 0; x<width; x++)
				{
					word destination_pixel = *address;

					*address = PIXEL16_565_BLEND(pixel, destination_pixel, alpha, inverse_alpha);
				}
				break;
			}
		}
	}

	return;
}

void bitmap_copy(
	struct bitmap_data *destination,
	point2d const *destination_point,
	rectangle2d const *destination_clip_rectangle,
	struct bitmap_data const *source,
	rectangle2d const *source_rectangle,
	pixel32 color,
	short flags)
{
	rectangle2d source_bounds;
	rectangle2d destination_bounds;
	point2d source_point;
	point2d adjusted_destination_point;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_drawing.c", 458, destination && source);

	set_rectangle2d(&source_bounds, 0, 0, source->width, source->height);
	if (source_rectangle)
	{
		intersect_rectangles2d(source_rectangle, &source_bounds, &source_bounds);
	}
	set_point2d(&source_point, source_bounds.x0, source_bounds.y0);

	set_rectangle2d(&destination_bounds, 0, 0, destination->width, destination->height);
	if (destination_clip_rectangle)
	{
		intersect_rectangles2d(destination_clip_rectangle, &destination_bounds, &destination_bounds);
	}

	if (destination_point)
	{
		offset_rectangle2d(&source_bounds, destination_point->x, destination_point->y);
	}
	offset_rectangle2d(&source_bounds, -source_point.x, -source_point.y);

	if (intersect_rectangles2d(&source_bounds, &destination_bounds, &source_bounds))
	{
		short translation;
		pixel32 converted_color;
		short y;

		set_point2d(&adjusted_destination_point, source_bounds.x0, source_bounds.y0);
		if (destination_point)
		{
			offset_rectangle2d(&source_bounds, -destination_point->x, -destination_point->y);
		}
		offset_rectangle2d(&source_bounds, source_point.x, source_point.y);

		match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_drawing.c", 480, adjusted_destination_point.x>=0);
		match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_drawing.c", 481, adjusted_destination_point.y>=0);
		match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_drawing.c", 484, adjusted_destination_point.x + rectangle2d_width(&source_bounds)<=destination->width,
			csprintf(temporary, "#%d+#%d(#%d,#%d)<=#%d", adjusted_destination_point.x, rectangle2d_width(&source_bounds), source_bounds.x0, source_bounds.x1, destination->width));
		match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_drawing.c", 487, adjusted_destination_point.y + rectangle2d_height(&source_bounds)<=destination->height,
			csprintf(temporary, "#%d+#%d(#%d,#%d)<=#%d", adjusted_destination_point.y, rectangle2d_height(&source_bounds), source_bounds.y0, source_bounds.y1, destination->height));

		translation = translation_table[source->format][destination->format][flags];
		if (!translation)
		{
			match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_drawing.c", 492, FALSE,
				csprintf(temporary, "no copy bitmap translation exists to copy %s to %s. (flags=%d)", bitmap_format_get_string(source->format), bitmap_format_get_string(destination->format), flags));
		}
		else
		{
			bitmap_format_get_bits_per_pixel(source->format);

			switch (translation)
			{
			case _bitmap_copy_a8r8g8b8_modulate:
			case _bitmap_copy_a8r8g8b8_modulate_blend:
				converted_color = color;
				break;
			case _bitmap_copy_a4r4g4b4_to_a8r8g8b8_modulate_blend:
				converted_color = PIXEL32_TO_PIXEL16_4444_COLOR(color);
				break;
			}

			for (y = source_bounds.y0; y<source_bounds.y1; y++)
			{
				void *source_address = bitmap_2d_address(source, source_bounds.x0, y, 0);
				void *destination_address = bitmap_2d_address(destination, adjusted_destination_point.x, y - source_bounds.y0 + adjusted_destination_point.y, 0);
				short width = rectangle2d_width(&source_bounds);

				switch (translation)
				{
				case _bitmap_copy_16bit:
					while (width-->0)
					{
						*((word *)destination_address)++ = *((word *)source_address)++;
					}
					break;
				case _bitmap_copy_a1r5g5b5_to_r5g6b5:
					while (width-->0)
					{
						word source_pixel = *((word *)source_address)++;

						*((word *)destination_address)++ = PIXEL16_1555_TO_PIXEL16_565(source_pixel);
					}
					break;
				case _bitmap_copy_r5g6b5_to_a1r5g5b5:
					while (width-->0)
					{
						word source_pixel = *((word *)source_address)++;

						*((word *)destination_address)++ = PIXEL16_565_TO_PIXEL16_1555(source_pixel);
					}
					break;
				case _bitmap_copy_r5g6b5_to_a8r8g8b8:
					while (width-->0)
					{
						word source_pixel = *((word *)source_address)++;

						*((pixel32 *)destination_address)++ = PIXEL16_565_TO_PIXEL32(source_pixel);
					}
					break;
				case _bitmap_copy_a4r4g4b4_to_a8r8g8b8:
					while (width-->0)
					{
						word source_pixel = *((word *)source_address)++;

						*((pixel32 *)destination_address)++ = PIXEL16_4444_TO_PIXEL32(source_pixel);
					}
					break;
				case _bitmap_copy_a4r4g4b4_to_a8r8g8b8_blend:
					while (width-->0)
					{
						word pixel = *((word *)source_address)++;
						pixel32 source_pixel = PIXEL16_4444_TO_PIXEL32(pixel);
						byte alpha = (byte)PIXEL32_ALPHA(source_pixel);
						byte inverse_alpha = 255-alpha;

						*(pixel32 *)destination_address = PIXEL32_BLEND(source_pixel, *(pixel32 *)destination_address, alpha, inverse_alpha);
						((pixel32 *)destination_address)++;
					}
					break;
				case _bitmap_copy_a4r4g4b4_to_a8r8g8b8_modulate_blend:
					while (width-->0)
					{
						pixel32 source_pixel;
						byte alpha;
						byte inverse_alpha;
						word pixel = *(word *)source_address;

						source_address = offset_pointer(source_address, sizeof(word));
						source_pixel = PIXEL16_4444_MODULATE(pixel, converted_color);
						alpha = (byte)PIXEL32_ALPHA(source_pixel);
						inverse_alpha = 255-alpha;
						*(pixel32 *)destination_address = PIXEL32_BLEND(source_pixel, *(pixel32 *)destination_address, alpha, inverse_alpha);
						destination_address = offset_pointer(destination_address, sizeof(pixel32));
					}
					break;
				case _bitmap_copy_a8r8g8b8:
					while (width-->0)
					{
						*((pixel32 *)destination_address)++ = *((pixel32 *)source_address)++;
					}
					break;
				case _bitmap_copy_a8r8g8b8_blend:
					while (width-->0)
					{
						pixel32 source_pixel = *((pixel32 *)source_address)++;
						byte alpha = (byte)PIXEL32_ALPHA(source_pixel);
						byte inverse_alpha = 255-alpha;

						*(pixel32 *)destination_address = PIXEL32_BLEND(source_pixel, *(pixel32 *)destination_address, alpha, inverse_alpha);
						((pixel32 *)destination_address)++;
					}
					break;
				case _bitmap_copy_a8r8g8b8_modulate:
					while (width-->0)
					{
						pixel32 pixel = *((pixel32 *)source_address)++;

						*((pixel32 *)destination_address)++ = PIXEL32_MODULATE(pixel, converted_color);
					}
					break;
				case _bitmap_copy_a8r8g8b8_modulate_blend:
					while (width-->0)
					{
						byte alpha;
						byte inverse_alpha;
						pixel32 source_pixel = *((pixel32 *)source_address)++;

						source_pixel = PIXEL32_MODULATE(source_pixel, converted_color);
						alpha = (byte)PIXEL32_ALPHA(source_pixel);
						inverse_alpha = 255-alpha;

						*(pixel32 *)destination_address = PIXEL32_BLEND(source_pixel, *(pixel32 *)destination_address, alpha, inverse_alpha);
						((pixel32 *)destination_address)++;
					}
					break;
				case _bitmap_copy_a8r8g8b8_to_r5g6b5:
					while (width-->0)
					{
						pixel32 source_pixel = *((pixel32 *)source_address)++;

						*((word *)destination_address)++ = PIXEL32_TO_PIXEL16_565(source_pixel);
					}
					break;
				case _bitmap_copy_a8r8g8b8_to_a4r4g4b4:
					while (width-->0)
					{
						pixel32 source_pixel = *((pixel32 *)source_address)++;

						*((word *)destination_address)++ = PIXEL32_TO_PIXEL16_4444(source_pixel);
					}
					break;
				default:
					match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_drawing.c", 640, FALSE, NULL);
					break;
				}
			}
		}
	}

	return;
}

void bitmap_tile_and_bevel_rectangle(
	struct bitmap_data *destination,
	long bitmap_group_index,
	short sequence_index,
	rectangle2d const *rectangle,
	rectangle2d const *clip_rectangle,
	pixel32 color,
	long flags)
{
	rectangle2d destination_rectangle;
	rectangle2d bounds;
	point2d maximum[2];
	point2d minimum[2];

	if (!rectangle)
	{
		destination_rectangle.x0 = 0;
		destination_rectangle.y0 = 0;
		destination_rectangle.x1 = destination->width;
		destination_rectangle.y1 = destination->height;
		rectangle = &destination_rectangle;
	}

	bounds = *rectangle;
	minimum[1].x = rectangle->x0;
	minimum[0].x = rectangle->x0;
	maximum[1].x = rectangle->x1;
	maximum[0].x = rectangle->x1;
	minimum[1].y = rectangle->y0;
	minimum[0].y = rectangle->y0;
	maximum[1].y = rectangle->y1;
	maximum[0].y = rectangle->y1;

	if (bitmap_group_index!=NONE && (!clip_rectangle || intersect_rectangles2d(clip_rectangle, &bounds, &bounds)))
	{
		struct bitmap_group *bitmap_group = tag_get(BITMAP_GROUP_TAG, bitmap_group_index);

		if (sequence_index<bitmap_group->sequences.count)
		{
			short piece_index;
			short bitmap_index;
			struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(&bitmap_group->sequences, sequence_index, struct bitmap_group_sequence);

			for (piece_index = 0, bitmap_index = 0; piece_index<9; piece_index++)
			{
				if (bitmap_index>=sequence->bitmap_count)
				{
					break;
				}

				if (TEST_FLAG(flags, 2*bevel_piece_parts[piece_index]))
				{
					struct bitmap_data *bitmap = bitmap_group_get_bitmap_from_sequence(bitmap_group_index, sequence_index, bitmap_index++);

					if (bitmap)
					{
						rectangle2d piece_rectangle = *rectangle;
						rectangle2d piece_bounds = bounds;
						unsigned long piece_flags = bevel_piece_flags[piece_index];

						if (TEST_FLAG(piece_flags, _bevel_top_bit))
						{
							piece_rectangle.y1 = rectangle->y0 + bitmap->height;
						}
						if (TEST_FLAG(piece_flags, _bevel_bottom_bit))
						{
							piece_rectangle.y0 = rectangle->y1 - bitmap->height;
						}
						if (TEST_FLAG(piece_flags, _bevel_left_bit))
						{
							piece_rectangle.x1 = rectangle->x0 + bitmap->width;
						}
						if (TEST_FLAG(piece_flags, _bevel_right_bit))
						{
							piece_rectangle.x0 = rectangle->x1 - bitmap->width;
						}

						if (TEST_FLAG(piece_flags, _bevel_clip_bit))
						{
							rectangle2d *clip_bounds = TEST_FLAG(piece_flags, _bevel_corner_bit) ? &piece_bounds : &piece_rectangle;

							if (TEST_FLAG(piece_flags, _bevel_top_bit))
							{
								clip_bounds->x0 = MAX(minimum[0].x, clip_bounds->x0);
								clip_bounds->x1 = MIN(maximum[0].x, clip_bounds->x1);
							}
							if (TEST_FLAG(piece_flags, _bevel_bottom_bit))
							{
								clip_bounds->x0 = MAX(minimum[1].x, clip_bounds->x0);
								clip_bounds->x1 = MIN(maximum[1].x, clip_bounds->x1);
							}
							if (TEST_FLAG(piece_flags, _bevel_left_bit))
							{
								clip_bounds->y0 = MAX(minimum[0].y, clip_bounds->y0);
								clip_bounds->y1 = MIN(maximum[0].y, clip_bounds->y1);
							}
							if (TEST_FLAG(piece_flags, _bevel_right_bit))
							{
								clip_bounds->y0 = MAX(minimum[1].y, clip_bounds->y0);
								clip_bounds->y1 = MIN(maximum[1].y, clip_bounds->y1);
							}
						}

						if (TEST_FLAG(piece_flags, _bevel_corner_bit))
						{
							if (TEST_FLAG(piece_flags, _bevel_top_bit) && TEST_FLAG(piece_flags, _bevel_left_bit))
							{
								minimum[0].x = piece_rectangle.x1;
								minimum[0].y = piece_rectangle.y1;
							}
							if (TEST_FLAG(piece_flags, _bevel_bottom_bit) && TEST_FLAG(piece_flags, _bevel_left_bit))
							{
								minimum[1].x = piece_rectangle.x1;
								maximum[0].y = piece_rectangle.y0;
							}
							if (TEST_FLAG(piece_flags, _bevel_top_bit) && TEST_FLAG(piece_flags, _bevel_right_bit))
							{
								maximum[0].x = piece_rectangle.x0;
								minimum[1].y = piece_rectangle.y1;
							}
							if (TEST_FLAG(piece_flags, _bevel_bottom_bit) && TEST_FLAG(piece_flags, _bevel_right_bit))
							{
								maximum[1].x = piece_rectangle.x0;
								maximum[1].y = piece_rectangle.y0;
							}
						}

						if (intersect_rectangles2d(&piece_rectangle, &piece_bounds, &piece_bounds))
						{
							short row;
							short piece_width = rectangle2d_width(&piece_rectangle);
							short piece_height = rectangle2d_height(&piece_rectangle);
							short columns = (piece_width + bitmap->width - 1)/bitmap->width;
							short rows = (piece_height + bitmap->height - 1)/bitmap->height;
							short copy_flags = 0;

							if (TEST_FLAG(flags, 2*bevel_piece_parts[piece_index] + 1))
							{
								SET_FLAG(copy_flags, _bitmap_copy_blend_bit, TRUE);
							}
							if (color)
							{
								SET_FLAG(copy_flags, _bitmap_copy_modulate_bit, TRUE);
							}

							for (row = 0; row<rows; row++)
							{
								short column;

								for (column = 0; column<columns; column++)
								{
									point2d destination_point;

									set_point2d(&destination_point, bitmap->width*column + piece_rectangle.x0, bitmap->height*row + piece_rectangle.y0);
									bitmap_copy(destination, &destination_point, &piece_bounds, bitmap, NULL, color, copy_flags);
								}
							}
						}
					}
				}
			}
		}
	}

	return;
}

void bitmap_frame_rectangle(
	struct bitmap_data *destination,
	pixel32 color,
	real_rectangle2d const *bounds,
	rectangle2d const *clip_rectangle)
{
	real_point2d p0;
	real_point2d p1;

	p0.x = bounds->x0;
	p0.y = bounds->y0;
	p1.x = bounds->x1 - 1.f;
	p1.y = bounds->y0;
	bitmap_draw_line(destination, color, clip_rectangle, &p0, &p1);

	p0.x = bounds->x1 - 1.f;
	p0.y = bounds->y1 - 1.f;
	bitmap_draw_line(destination, color, clip_rectangle, &p1, &p0);

	p1.x = bounds->x0;
	p1.y = bounds->y1 - 1.f;
	bitmap_draw_line(destination, color, clip_rectangle, &p0, &p1);

	p0.x = bounds->x0;
	p0.y = bounds->y0;
	bitmap_draw_line(destination, color, clip_rectangle, &p1, &p0);

	return;
}

/* ---------- private code */
