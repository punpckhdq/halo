/*
IMA_ADPCM.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ima_adpcm.h"

/* ---------- constants */

enum
{
	MAXIMUM_STEP_SIZE_INDEX = 88, /* fake name */
};

/* ---------- globals */

long const step_size_adjustment_table[16] =
{
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};

long const step_size_table[MAXIMUM_STEP_SIZE_INDEX + 1] =
{
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static byte_swap_code bungie_ima_adpcm_header_bs_codes[] = /* fake name */
{
	_begin_bs_array, 1,
		_4byte,
		_2byte,
		_2byte,
	_end_bs_array
};

static struct byte_swap_definition bungie_ima_adpcm_header_bs_definition = /* fake name */
{
	"bungie ima adpcm header",
	sizeof(struct bungie_ima_adpcm_header),
	bungie_ima_adpcm_header_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

/* ---------- public code */

long compress_ima_adpcm_audio_data(
	short const *samples,
	long sample_count,
	byte *buffer,
	long buffer_size)
{
	long result = sizeof(struct bungie_ima_adpcm_header) + (sample_count >> 1) + (sample_count & 1);

	if (buffer)
	{
		short step_index = 0;
		long predicted_sample = *samples;
		boolean high_nibble = TRUE;
		long original_sample_count = sample_count;
		struct bungie_ima_adpcm_header *header = (struct bungie_ima_adpcm_header *)buffer;

		buffer += sizeof(struct bungie_ima_adpcm_header);
		buffer_size -= sizeof(struct bungie_ima_adpcm_header);
		header->predicted_sample = (short)predicted_sample;

		while (sample_count > 0 && buffer_size)
		{
			long delta;
			char code;
			char mask;
			long difference = *samples - predicted_sample;
			long step_size = step_size_table[step_index];

			if (difference < 0)
			{
				code = 8;
				difference = -difference;
			}
			else
			{
				code = 0;
			}

			for (mask = 4; mask; mask >>= 1, step_size >>= 1)
			{
				if (difference >= step_size)
				{
					code |= mask;
					difference -= step_size;
				}
			}

			step_size = step_size_table[step_index];
			delta = step_size >> 3;
			for (mask = 4; mask; mask >>= 1, step_size >>= 1)
			{
				if (code & mask)
				{
					delta += step_size;
				}
			}
			if (code & 8)
			{
				delta = -delta;
			}

			predicted_sample = PIN(predicted_sample + delta, -32768, 32767);
			step_index = (short)PIN(step_index + step_size_adjustment_table[code], 0, MAXIMUM_STEP_SIZE_INDEX);

			if (high_nibble)
			{
				*buffer = code << 4;
			}
			else
			{
				*buffer |= code;
			}

			high_nibble = !high_nibble;
			if (high_nibble)
			{
				buffer++;
				buffer_size--;
			}

			sample_count--;
			samples++;
		}

		header->sample_count = original_sample_count - sample_count;
		result = sample_count;
	}

	return result;
}

long decompress_ima_adpcm_audio_data(
	void const *buffer,
	long buffer_size,
	short *samples,
	long maximum_sample_count,
	struct ima_adpcm_decompression_state *state)
{
	long result;
	struct bungie_ima_adpcm_header const *header = (struct bungie_ima_adpcm_header const *)buffer;
	char const *data = (char const *)(header + 1);

	buffer_size -= sizeof(struct bungie_ima_adpcm_header);
	result = header->sample_count * sizeof(short);

	if (samples)
	{
		long predicted_sample;
		short step_index;
		boolean high_nibble;
		long remaining_sample_count;

		if (state)
		{
			if (state->sample_count == 0)
			{
				state->sample_count = header->sample_count;
				state->predicted_sample = header->predicted_sample;
				state->step_index = 0;
			}

			predicted_sample = state->predicted_sample;
			step_index = state->step_index;
			buffer_size -= state->sample_index >> 1;
			data += state->sample_index >> 1;
			remaining_sample_count = state->sample_count - state->sample_index;
			high_nibble = (state->sample_index & 1) == 0;
		}
		else
		{
			predicted_sample = header->predicted_sample;
			step_index = 0;
			high_nibble = TRUE;
			remaining_sample_count = header->sample_count;
		}

		while (remaining_sample_count && maximum_sample_count && buffer_size)
		{
			char code;
			long step_size = step_size_table[step_index];
			long delta = step_size >> 3;
			char mask = 4;

			if (high_nibble)
			{
				code = (*data >> 4) & 0xf;
			}
			else
			{
				code = *data & 0xf;
			}

			while (mask)
			{
				if (code & mask)
				{
					delta += step_size;
				}
				mask >>= 1;
				step_size >>= 1;
			}
			if (code & 8)
			{
				delta = -delta;
			}

			predicted_sample = PIN(predicted_sample + delta, -32768, 32767);
			step_index = (short)PIN(step_index + step_size_adjustment_table[code], 0, MAXIMUM_STEP_SIZE_INDEX);
			*samples = (short)predicted_sample;

			high_nibble = !high_nibble;
			if (high_nibble)
			{
				data++;
				buffer_size--;
			}

			samples++;
			remaining_sample_count--;
			maximum_sample_count--;
			if (state)
			{
				state->sample_index++;
			}
		}

		if (state)
		{
			state->predicted_sample = (short)predicted_sample;
			state->step_index = step_index;
		}

		result = remaining_sample_count;
	}

	return result;
}

void byte_swap_bungie_ima_adpcm_header(
	struct bungie_ima_adpcm_header *header)
{
	byte_swap_data(&bungie_ima_adpcm_header_bs_definition, header, 1);

	return;
}
