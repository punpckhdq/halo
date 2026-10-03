/*
SOUND_AIFF.C
*/

/* ---------- headers */

#include "cseries.h"
#include "sound_import.h"
#include "files.h"

/* ---------- structures */

struct aiff_container_chunk
{
	long chunk_type; /* fake name */
	long chunk_length; /* fake name */
	long container_type; /* fake name */
};

struct aiff_chunk
{
	long chunk_type; /* fake name */
	long chunk_length; /* fake name */
};

#pragma pack(push, 2)
struct aiff_format_info_p1
{
	short channel_count; /* fake name */
	long sample_frame_count; /* fake name */
	short sample_size; /* fake name */
	byte sample_rate[10]; /* fake name */
	long compression_type; /* fake name */
};
#pragma pack(pop)

/* ---------- prototypes */

extern boolean file_read_from_position(const struct file_reference *file, unsigned long position, unsigned long count, void *buffer);

/* ---------- globals */

static byte_swap_code aiff_container_chunk_bs_codes[] = /* fake name */
{
	_begin_bs_array, 1,
		_4byte,
		_4byte,
		_4byte,
	_end_bs_array
};

static struct byte_swap_definition aiff_container_chunk_bs_definition = /* fake name */
{
	"aiff container chunk",
	sizeof(struct aiff_container_chunk),
	aiff_container_chunk_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code aiff_chunk_bs_codes[] = /* fake name */
{
	_begin_bs_array, 1,
		_4byte,
		_4byte,
	_end_bs_array
};

static struct byte_swap_definition aiff_chunk_bs_definition = /* fake name */
{
	"aiff chunk",
	sizeof(struct aiff_chunk),
	aiff_chunk_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code aiff_format_info_p1_bs_codes[] = /* fake name */
{
	_begin_bs_array, 1,
		_2byte,
		_4byte,
		_2byte,
		10,
		_4byte,
	_end_bs_array
};

static struct byte_swap_definition aiff_format_info_p1_bs_definition = /* fake name */
{
	"aiff format info p1",
	sizeof(struct aiff_format_info_p1),
	aiff_format_info_p1_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

/* ---------- public code */

boolean sound_file_is_aiff(
	struct file_reference *file)
{
	boolean result = FALSE;

	if (file_open(file, FLAG(_permission_read_bit)))
	{
		struct aiff_container_chunk chunk;

		if (file_read_from_position(file, 0, sizeof(chunk), &chunk))
		{
			byte_swap_data(&aiff_container_chunk_bs_definition, &chunk, 1);
			if (chunk.chunk_type == 'FORM' &&
				(chunk.container_type == 'AIFF' || chunk.container_type == 'AIFC'))
			{
				result = TRUE;
			}
		}

		file_close(file);
	}

	return result;
}

boolean sound_file_aiff_info_get(
	struct file_reference *file,
	struct sound_file_info *info)
{
	boolean success = FALSE;
	long offset = sizeof(struct aiff_container_chunk);

	if (file_open(file, FLAG(_permission_read_bit)))
	{
		struct aiff_chunk chunk;

		while (file_read_from_position(file, offset, sizeof(chunk), &chunk))
		{
			byte_swap_data(&aiff_chunk_bs_definition, &chunk, 1);
			if (chunk.chunk_type == 'COMM')
			{
				struct aiff_format_info_p1 format_info;

				offset += sizeof(chunk);
				if (file_read_from_position(file, offset, sizeof(format_info), &format_info))
				{
					byte rate_11025[10] = { 0x40, 0x0c, 0xac, 0x44, 0, 0, 0, 0, 0, 0 };
					byte rate_22050[10] = { 0x40, 0x0d, 0xac, 0x44, 0, 0, 0, 0, 0, 0 };
					byte rate_44100[10] = { 0x40, 0x0e, 0xac, 0x44, 0, 0, 0, 0, 0, 0 };

					byte_swap_data(&aiff_format_info_p1_bs_definition, &format_info, 1);
					if (!memcmp(rate_11025, format_info.sample_rate, sizeof(format_info.sample_rate)))
					{
						info->samples_per_second = 11025;
						info->significant_bits_per_sample = format_info.sample_size;
						info->channel_count = format_info.channel_count;
						if (chunk.chunk_length == 18 || format_info.compression_type == 'NONE')
						{
							success = TRUE;
						}
					}
					else if (!memcmp(rate_22050, format_info.sample_rate, sizeof(format_info.sample_rate)))
					{
						info->samples_per_second = 22050;
						info->significant_bits_per_sample = format_info.sample_size;
						info->channel_count = format_info.channel_count;
						if (chunk.chunk_length == 18 || format_info.compression_type == 'NONE')
						{
							success = TRUE;
						}
					}
					else if (!memcmp(rate_44100, format_info.sample_rate, sizeof(format_info.sample_rate)))
					{
						info->samples_per_second = 44100;
						info->significant_bits_per_sample = format_info.sample_size;
						info->channel_count = format_info.channel_count;
						if (chunk.chunk_length == 18 || format_info.compression_type == 'NONE')
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

			offset += sizeof(chunk) + ((chunk.chunk_length & 1) ? chunk.chunk_length + 1 : chunk.chunk_length);
		}

		file_close(file);
	}

	return success;
}

boolean sound_file_aiff_raw_data_get(
	struct file_reference *file,
	long *size,
	void *buffer)
{
	boolean success = FALSE;
	long offset = sizeof(struct aiff_container_chunk);

	if (file_open(file, FLAG(_permission_read_bit)))
	{
		struct aiff_chunk chunk;

		while (file_read_from_position(file, offset, sizeof(chunk), &chunk))
		{
			byte_swap_data(&aiff_chunk_bs_definition, &chunk, 1);
			if (chunk.chunk_type == 'SSND')
			{
				*size = chunk.chunk_length - 8;
				offset += sizeof(chunk) + 8;
				if (file_read_from_position(file, offset, *size, buffer))
				{
					success = TRUE;
				}
				break;
			}

			offset += sizeof(chunk) + ((chunk.chunk_length & 1) ? chunk.chunk_length + 1 : chunk.chunk_length);
		}

		file_close(file);

		if (success)
		{
			byte_swap_memory(buffer, *size >> 1, _2byte);
		}
	}

	return success;
}

void sound_file_aiff_format(
	struct sound_file_info const *info,
	long *size,
	void *buffer)
{
	return;
}
