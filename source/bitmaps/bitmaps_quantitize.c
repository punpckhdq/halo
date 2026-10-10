/*
BITMAPS_QUANTITIZE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "bitmaps.h"

/* ---------- constants */

enum
{
	CHANNEL_BITS = 8
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void row_copy(short *destination, byte const *source, short width);
static void row_dither(short const *bits_per_channel, short const *thresholds, short width, short *current_row, short *next_row, byte *pixels);

/* ---------- globals */

short bits_per_channel_r5g6b5[4] =
{
	0, // alpha
	5, // red
	6, // green
	5 // blue
};

short bits_per_channel_a1r5g5b5[4] =
{
	8, // alpha
	5, // red
	5, // green
	5 // blue
};

short bits_per_channel_a4r4g4b4[4] =
{
	4, // alpha
	4, // red
	4, // green
	4 // blue
};

static short quantitize_bits_per_channel[4] = {0}; /* fake name */

/* ---------- public code */

void bitmap_quantitize(
	struct bitmap_data *bitmap,
	short const *bits_per_channel)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps_quantitize.c", 48, bitmap_verify(bitmap, TRUE));

	if (bits_per_channel && bitmap->type==_bitmap_type_2d)
	{
		short thresholds[4];
		short *current_row = match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmaps_quantitize.c", 52, bitmap->width*4*sizeof(short));
		short *next_row = match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmaps_quantitize.c", 53, bitmap->width*4*sizeof(short));

		{
			short channel_index;

			for (channel_index = 0; channel_index<4; channel_index++)
			{
				match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmaps_quantitize.c", 63, bits_per_channel[channel_index]>=0 && bits_per_channel[channel_index]<=CHANNEL_BITS);
			}
		}

		{
			short channel_index;

			for (channel_index = 0; channel_index<4; channel_index++)
			{
				quantitize_bits_per_channel[3-channel_index] = bits_per_channel[channel_index];
			}
		}

		{
			short channel_index;

			for (channel_index = 0; channel_index<4; channel_index++)
			{
				thresholds[channel_index] = (short)((real)FLAG(CHANNEL_BITS-quantitize_bits_per_channel[channel_index])*0.25f);
			}
		}

		if (current_row && next_row)
		{
			short row;

			row_copy(current_row, bitmap->base_address, bitmap->width);
			for (row = 0; row<bitmap->height-1; row++)
			{
				short *temporary_row;

				row_copy(next_row, bitmap_2d_address(bitmap, 0, row+1, 0), bitmap->width);
				row_dither(quantitize_bits_per_channel, thresholds, bitmap->width, current_row, next_row, bitmap_2d_address(bitmap, 0, row, 0));

				temporary_row = current_row;
				current_row = next_row;
				next_row = temporary_row;
			}
			row_dither(quantitize_bits_per_channel, thresholds, bitmap->width, current_row, NULL, bitmap_2d_address(bitmap, 0, bitmap->height-1, 0));

			match_free("c:\\halo\\SOURCE\\bitmaps\\bitmaps_quantitize.c", 115, current_row);
			match_free("c:\\halo\\SOURCE\\bitmaps\\bitmaps_quantitize.c", 116, next_row);
		}
	}

	return;
}

/* ---------- private code */

static void row_copy(
	short *destination,
	byte const *source,
	short width)
{
	long count;

	for (count = width*4; count>0; count--)
	{
		*destination++ = *source++;
	}

	return;
}

static void row_dither(
	short const *bits_per_channel,
	short const *thresholds,
	short width,
	short *current_row,
	short *next_row,
	byte *pixels)
{
	short x;
	byte quantized_color[4];
	byte color[4];
	short channel_index;
	short error;

	for (x = 0; x<width; x++)
	{
		for (channel_index = 0; channel_index<4; channel_index++)
		{
			color[channel_index] = (byte)PIN(current_row[channel_index], 0, UNSIGNED_CHAR_MAX);
		}

		for (channel_index = 0; channel_index<4; channel_index++)
		{
			quantized_color[channel_index] = bits_per_channel[channel_index] ?
				UNSIGNED_CHAR_MAX*(color[channel_index]>>(CHANNEL_BITS-bits_per_channel[channel_index]))/(FLAG(bits_per_channel[channel_index])-1) :
				0;
			pixels[channel_index] = quantized_color[channel_index];
		}

		for (channel_index = 0; channel_index<4; channel_index++)
		{
			error = color[channel_index]-quantized_color[channel_index];

			if (x<width-1 && current_row[channel_index+4]>thresholds[channel_index])
			{
				current_row[channel_index+4] += 7*error/16;
			}
			if (next_row)
			{
				if (x!=0 && next_row[channel_index-4]>thresholds[channel_index])
				{
					next_row[channel_index-4] += 3*error/16;
				}
				if (next_row[channel_index]>thresholds[channel_index])
				{
					next_row[channel_index] += 5*error/16;
				}
				if (x<width-1 && next_row[channel_index+4]>thresholds[channel_index])
				{
					next_row[channel_index+4] += error/16;
				}
			}
		}

		current_row += 4;
		pixels += 4;
		if (next_row)
		{
			next_row += 4;
		}
	}

	return;
}
