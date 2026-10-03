/*
SOUND_WAVE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "sound_import.h"
#include "files.h"

/* ---------- structures */

struct riff_container_chunk
{
	long chunk_type; /* fake name */
	long chunk_length; /* fake name */
	long container_type; /* fake name */
};

struct riff_chunk
{
	short format_tag; /* fake name */
	short channel_count; /* fake name */
	long samples_per_second; /* fake name */
	long average_bytes_per_second; /* fake name */
	short block_alignment; /* fake name */
	short bits_per_sample; /* fake name */
	short extra_size; /* fake name */
};

/* ---------- prototypes */

extern boolean file_read_from_position(const struct file_reference *file, unsigned long position, unsigned long count, void *buffer);

/* ---------- globals */

static byte_swap_code riff_container_chunk_bs_codes[] = /* fake name */
{
	_begin_bs_array, 1,
		_4byte,
		_4byte,
		_4byte,
	_end_bs_array
};

static struct byte_swap_definition riff_container_chunk_bs_definition = /* fake name */
{
	"riff container chunk",
	sizeof(struct riff_container_chunk),
	riff_container_chunk_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code riff_chunk_type_bs_codes[] = /* fake name */
{
	_begin_bs_array, 1,
		_4byte,
	_end_bs_array
};

static struct byte_swap_definition riff_chunk_type_bs_definition = /* fake name */
{
	"riff chunk type",
	sizeof(long),
	riff_chunk_type_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code riff_chunk_length_bs_codes[] = /* fake name */
{
	_begin_bs_array, 1,
		_4byte,
	_end_bs_array
};

static struct byte_swap_definition riff_chunk_length_bs_definition = /* fake name */
{
	"riff chunk length",
	sizeof(long),
	riff_chunk_length_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code riff_chunk_bs_codes[] = /* fake name */
{
	_begin_bs_array, 1,
		_2byte,
		_2byte,
		_4byte,
		_4byte,
		_2byte,
		_2byte,
		_2byte,
	_end_bs_array
};

static struct byte_swap_definition riff_chunk_bs_definition = /* fake name */
{
	"riff chunk",
	18,
	riff_chunk_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

/* ---------- public code */

boolean sound_file_is_wave(
	struct file_reference *file)
{
	boolean result = FALSE;

	if (file_open(file, FLAG(_permission_read_bit)))
	{
		struct riff_container_chunk chunk;

		if (file_read_from_position(file, 0, sizeof(chunk), &chunk))
		{
			byte_swap_data(&riff_container_chunk_bs_definition, &chunk, 1);
			if (chunk.chunk_type == 'RIFF' && chunk.container_type == 'WAVE')
			{
				result = TRUE;
			}
		}

		file_close(file);
	}

	return result;
}

boolean sound_file_wave_info_get(
	struct file_reference *file,
	struct sound_file_info *info)
{
	boolean success = FALSE;
	long offset = sizeof(struct riff_container_chunk);

	if (file_open(file, FLAG(_permission_read_bit)))
	{
		long chunk_type;
		long chunk_length;

		while (file_read_from_position(file, offset, sizeof(chunk_type), &chunk_type))
		{
			offset += sizeof(chunk_type);
			if (file_read_from_position(file, offset, sizeof(chunk_length), &chunk_length))
			{
				byte_swap_data(&riff_chunk_type_bs_definition, &chunk_type, 1);
				if (chunk_type == 'fmt ')
				{
					struct riff_chunk format_info;

					offset += sizeof(chunk_length);
					if (file_read_from_position(file, offset, 18, &format_info))
					{
						if (format_info.samples_per_second == 11025 ||
							format_info.samples_per_second == 22050 ||
							format_info.samples_per_second == 44100)
						{
							info->samples_per_second = format_info.samples_per_second;
							info->channel_count = format_info.channel_count;
							info->significant_bits_per_sample = format_info.bits_per_sample;
							if (format_info.format_tag == 1)
							{
								success = TRUE;
							}
						}
						else
						{
							info->samples_per_second = NONE;
						}
					}
					break;
				}
				else
				{
					offset += sizeof(chunk_length) + ((chunk_length & 1) ? chunk_length + 1 : chunk_length);
				}
			}
		}

		file_close(file);
	}

	return success;
}

boolean sound_file_wave_raw_data_get(
	struct file_reference *file,
	long *size,
	void *buffer)
{
	boolean success = FALSE;
	long offset = sizeof(struct riff_container_chunk);

	if (file_open(file, FLAG(_permission_read_bit)))
	{
		long chunk_type;
		long chunk_length;

		while (file_read_from_position(file, offset, sizeof(chunk_type), &chunk_type))
		{
			offset += sizeof(chunk_type);
			if (file_read_from_position(file, offset, sizeof(chunk_length), &chunk_length))
			{
				byte_swap_data(&riff_chunk_type_bs_definition, &chunk_type, 1);
				if (chunk_type == 'data')
				{
					*size = chunk_length;
					offset += sizeof(chunk_length);
					if (file_read_from_position(file, offset, chunk_length, buffer))
					{
						success = TRUE;
					}
					break;
				}
				else
				{
					offset += sizeof(chunk_length) + ((chunk_length & 1) ? chunk_length + 1 : chunk_length);
				}
			}
		}

		file_close(file);
	}

	return success;
}

void sound_file_wave_format(
	struct sound_file_info const *info,
	long *size,
	void *buffer)
{
	if (info->significant_bits_per_sample == 8)
	{
		long sample_index = *size - 1;
		byte *source = (byte *)buffer + sample_index;
		short *destination = (short *)buffer + sample_index;

		while (sample_index-- >= 0)
		{
			long sample = (*source << 8) + *source;

			sample -= 0x8080;
			*destination = (short)sample;
			destination--;
			source--;
		}

		*size <<= 1;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\sound\\sound_import\\sound_wave.c", 280, info->significant_bits_per_sample==16);
	}

	return;
}
