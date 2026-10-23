/*
BYTE_SWAPPING.C
*/

/* ---------- headers */

#include "cseries.h"
#include "byte_swapping.h"

/* ---------- prototypes */

static void _byte_swap_data(struct byte_swap_definition *definition, void *data, byte_swap_code *codes, long *total_size_in_bytes, long *total_size_in_codes);

/* ---------- globals */

static byte_swap_code byte_bs_codes[] = {_begin_bs_array, 1, sizeof(byte), _end_bs_array};
struct byte_swap_definition byte_bs_definition = {"byte", sizeof(byte), byte_bs_codes, BYTE_SWAP_DEFINITION_SIGNATURE};
static byte_swap_code word_bs_codes[] = {_begin_bs_array, 1, sizeof(word), _end_bs_array};
struct byte_swap_definition word_bs_definition = {"word", sizeof(word), word_bs_codes, BYTE_SWAP_DEFINITION_SIGNATURE};
static byte_swap_code long_bs_codes[] = {_begin_bs_array, 1, sizeof(long), _end_bs_array};
struct byte_swap_definition long_bs_definition = {"long", sizeof(long), long_bs_codes, BYTE_SWAP_DEFINITION_SIGNATURE};
static byte_swap_code int64_bs_codes[] = {_begin_bs_array, 1, sizeof(_uint64), _end_bs_array};
struct byte_swap_definition int64_bs_definition = {"int64", sizeof(_uint64), int64_bs_codes, BYTE_SWAP_DEFINITION_SIGNATURE};

/* ---------- public code */

void byte_swap_data_explicit(
	char *name,
	long size,
	byte_swap_code *codes,
	long data_count,
	void *data)
{
	struct byte_swap_definition byte_swap_definition;

	match_assert("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 40, codes);
	match_assert("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 41, data_count>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 42, size>=0);

	byte_swap_definition.name = name;
	byte_swap_definition.size = size;
	byte_swap_definition.codes = codes;
	byte_swap_definition.signature = BYTE_SWAP_DEFINITION_SIGNATURE;
	byte_swap_definition.verified = data ? TRUE : FALSE;
	byte_swap_data(&byte_swap_definition, data, data_count);

	return;
}

long byte_swap_codes_size(
	char *name,
	byte_swap_code *codes)
{
	long size_in_bytes;
	long size_in_codes;
	struct byte_swap_definition definition;

	definition.name = name;
	definition.size = 0;
	definition.codes = codes;
	definition.signature = BYTE_SWAP_DEFINITION_SIGNATURE;
	_byte_swap_data(&definition, NULL, codes, &size_in_bytes, &size_in_codes);

	return size_in_bytes;
}

void byte_swap_data(
	struct byte_swap_definition *definition,
	void *data,
	long data_count)
{
	match_assert("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 77, definition);

	if (!definition->verified && definition->size>=0)
	{
		long size_in_bytes;
		long size_in_codes;

		_byte_swap_data(definition, NULL, definition->codes, &size_in_bytes, &size_in_codes);
		match_vassert("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 88, size_in_bytes==definition->size, csprintf(temporary, "%s bs data @%p is #%d but should be #%d bytes", definition->name, definition, size_in_bytes, definition->size));
		definition->verified = TRUE;
	}

	if (data)
	{
		long data_index;

		for (data_index = 0; data_index<data_count; data_index++)
		{
			_byte_swap_data(definition, (byte *)data + data_index*definition->size, definition->codes, NULL, NULL);
		}
	}

	return;
}

void byte_swap_memory(
	void *memory,
	long count,
	byte_swap_code code)
{
	match_assert("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 113, memory);
	match_assert("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 114, count>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 115, code==_2byte || code==_4byte || code==_8byte);

	switch (code)
	{
	case _2byte:
		{
			word *data = (word *)memory;

			while (count-->0)
			{
				*data = SWAP2(*data);
				data++;
			}
		}
		break;
	case _4byte:
		{
			unsigned long *data = (unsigned long *)memory;

			while (count-->0)
			{
				*data = SWAP4(*data);
				data++;
			}
		}
		break;
	case _8byte:
		{
			_uint64 *data = (_uint64 *)memory;

			while (count-->0)
			{
				*data = SWAP8(*data);
				data++;
			}
		}
		break;
	}

	return;
}

/* ---------- private code */

static void _byte_swap_data(
	struct byte_swap_definition *definition,
	void *data,
	byte_swap_code *codes,
	long *total_size_in_bytes,
	long *total_size_in_codes)
{
	long size_in_bytes;
	long size_in_codes;
	long array_count;
	long array_index;

	match_vwarn("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 176, definition->signature==BYTE_SWAP_DEFINITION_SIGNATURE, csprintf(temporary, "got bs data with bad signature (assuming name is wrong)"));
	match_vassert("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 178, definition->signature==BYTE_SWAP_DEFINITION_SIGNATURE, csprintf(temporary, "%s bs data has bad signature", definition->name));
	match_vassert("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 183, codes[0]==_begin_bs_array, csprintf(temporary, "%s bs data @%p.#0 has bad start #%d", definition->name, codes, definition->codes[0]));

	array_count = codes[1];
	match_vassert("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 189, array_count>=0, csprintf(temporary, "%s bs data @%p.#1 has invalid array size #%d", definition->name, codes, array_count));

	size_in_bytes = 0;
	for (array_index = 0; array_index<array_count; array_index++)
	{
		boolean done = FALSE;

		size_in_codes = 2;
		while (!done)
		{
			byte_swap_code code = codes[size_in_codes];

			switch (code)
			{
			case _2byte:
				if (data)
				{
					word *value = (word *)((byte *)data + size_in_bytes);

					*value = SWAP2(*value);
				}
				size_in_codes++;
				size_in_bytes += 2;
				break;
			case _4byte:
				if (data)
				{
					unsigned long *value = (unsigned long *)((byte *)data + size_in_bytes);

					*value = SWAP4(*value);
				}
				size_in_codes++;
				size_in_bytes += 4;
				break;
			case _8byte:
				if (data)
				{
					_uint64 *value = (_uint64 *)((byte *)data + size_in_bytes);

					*value = SWAP8(*value);
				}
				size_in_codes++;
				size_in_bytes += 8;
				break;
			case _begin_bs_array:
				{
					long recursive_size_in_bytes;
					long recursive_size_in_codes;

					_byte_swap_data(definition, data ? (byte *)data + size_in_bytes : NULL, &codes[size_in_codes], &recursive_size_in_bytes, &recursive_size_in_codes);
					size_in_codes += recursive_size_in_codes;
					size_in_bytes += recursive_size_in_bytes;
				}
				break;
			case _end_bs_array:
				size_in_codes++;
				done = TRUE;
				break;
			case _extern_bs_definition:
				{
					struct byte_swap_definition *extern_definition = (struct byte_swap_definition *)codes[size_in_codes + 1];
					long recursive_size_in_bytes;

					_byte_swap_data(extern_definition, data ? (byte *)data + size_in_bytes : NULL, extern_definition->codes, &recursive_size_in_bytes, NULL);
					size_in_codes += 2;
					size_in_bytes += recursive_size_in_bytes;
				}
				break;
			default:
				if (code>0)
				{
					size_in_codes++;
					size_in_bytes += code;
				}
				else
				{
					match_vhalt("c:\\halo\\SOURCE\\memory\\byte_swapping.c", 297, csprintf(temporary, "%s bs @%p.#%d has invalid code #%d", definition->name, codes, size_in_codes, code));
				}
				break;
			}
		}
	}

	if (total_size_in_bytes)
	{
		*total_size_in_bytes = size_in_bytes;
	}

	if (total_size_in_codes)
	{
		*total_size_in_codes = size_in_codes;
	}

	return;
}
