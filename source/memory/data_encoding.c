/*
DATA_ENCODING.C
*/

/* ---------- headers */

#include "cseries.h"
#include "data_encoding.h"

/* ---------- public code */

void data_encode_new(
	struct data_encoding_state *state,
	void *buffer,
	long buffer_size)
{
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 25, buffer);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 26, buffer_size>=0);

	memset(state, 0, sizeof(*state));
	state->buffer = buffer;
	state->buffer_size = buffer_size;

	return;
}

boolean data_encode_memory(
	struct data_encoding_state *state,
	void *buffer,
	short count,
	long code)
{
	long size;

	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 43, state && state->buffer && state->offset>=0 && state->offset<state->buffer_size);

	switch (code)
	{
	case _1byte:
		size = count;
		break;
	case _2byte:
		size = count*sizeof(word);
		break;
	case _4byte:
		size = count*sizeof(long);
		break;
	case _8byte:
		size = count*sizeof(unsigned __int64);
		break;
	default:
		match_halt("c:\\halo\\SOURCE\\memory\\data_encoding.c", 51);
	}

	if (state->offset + size<=state->buffer_size && !state->overflow_flag)
	{
		void *destination = (byte *)state->buffer + state->offset;

		if (buffer)
		{
			memcpy(destination, buffer, size);
		}
		else
		{
			memset(destination, 0, size);
		}

		if (code!=_1byte)
		{
			byte_swap_memory(destination, count, code);
		}

		state->offset += size;
	}
	else
	{
		state->overflow_flag = TRUE;
	}

	return !state->overflow_flag;
}

boolean data_encode_integer(
	struct data_encoding_state *state,
	long value,
	long maximum_value)
{
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 84, maximum_value>0);

	if (maximum_value<=UNSIGNED_CHAR_MAX)
	{
		byte temp = (byte)value;

		data_encode_memory(state, &temp, 1, _1byte);
	}
	else if (maximum_value<=UNSIGNED_SHORT_MAX)
	{
		short temp = (short)value;

		data_encode_memory(state, &temp, 1, _2byte);
	}
	else
	{
		long temp = value;

		data_encode_memory(state, &temp, 1, _4byte);
	}

	return !state->overflow_flag;
}

boolean data_encode_structures(
	struct data_encoding_state *state,
	void *source_structures,
	short structure_count,
	struct byte_swap_definition *bs_definition)
{
	void *destination;
	short size;

	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 110, state && state->buffer && state->offset>=0 && state->offset<state->buffer_size);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 111, source_structures);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 112, bs_definition);

	size = structure_count*bs_definition->size;

	if (size>0)
	{
		if (state->offset + size<=state->buffer_size && !state->overflow_flag)
		{
			destination = (byte *)state->buffer + state->offset;
			memcpy(destination, source_structures, size);
			byte_swap_data(bs_definition, destination, structure_count);
			state->offset += size;
		}
		else
		{
			state->overflow_flag = TRUE;
		}
	}

	return !state->overflow_flag;
}

boolean data_encode_array(
	struct data_encoding_state *state,
	long element_count_size,
	void *source_array,
	long element_count,
	struct byte_swap_definition *bs_definition)
{
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 141, state && state->buffer && state->offset>=0 && state->offset<state->buffer_size);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 142, source_array);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 143, bs_definition);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 144, element_count>=0);

	switch (element_count_size)
	{
	case _1byte:
		{
			byte temp;

			match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 150, element_count<=UNSIGNED_CHAR_MAX);
			temp = (byte)element_count;
			data_encode_memory(state, &temp, 1, _1byte);
		}
		break;
	case _2byte:
		{
			short temp;

			match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 154, element_count<=UNSIGNED_SHORT_MAX);
			temp = (short)element_count;
			data_encode_memory(state, &temp, 1, _2byte);
		}
		break;
	case _4byte:
		{
			long temp = element_count;

			data_encode_memory(state, &temp, 1, _4byte);
		}
		break;
	case _8byte:
		{
			unsigned __int64 temp = element_count;

			data_encode_memory(state, &temp, 1, _8byte);
		}
		break;
	default:
		match_halt("c:\\halo\\SOURCE\\memory\\data_encoding.c", 165);
	}

	data_encode_structures(state, source_array, (short)element_count, bs_definition);

	return !state->overflow_flag;
}

boolean data_encode_string(
	struct data_encoding_state *state,
	char *source_string,
	short maximum_string_length)
{
	short string_length = (short)strnlen(source_string, maximum_string_length);
	char *destination = (char *)state->buffer + state->offset;

	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 182, state->offset+string_length+1<=state->buffer_size);

	if (state->offset + string_length + 1<=state->buffer_size && !state->overflow_flag)
	{
		strncpy(destination, source_string, string_length);
		destination[string_length] = 0;
		state->offset += string_length + 1;
	}
	else
	{
		state->overflow_flag = TRUE;
	}

	return !state->overflow_flag;
}

void data_decode_new(
	struct data_encoding_state *state,
	void *buffer,
	long buffer_size)
{
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 204, buffer);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 205, buffer_size>=0);

	memset(state, 0, sizeof(*state));
	state->buffer = buffer;
	state->buffer_size = buffer_size;

	return;
}

void *data_decode_structures(
	struct data_encoding_state *state,
	short structure_count,
	struct byte_swap_definition *bs_definition)
{
	short size;
	void *result = NULL;
	void *data;

	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 222, state && state->buffer && state->offset>=0 && state->offset<=state->buffer_size);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 223, structure_count>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 224, bs_definition);

	size = structure_count*bs_definition->size;

	if (state->offset + size<=state->buffer_size && !state->overflow_flag)
	{
		data = (byte *)state->buffer + state->offset;

		if (size)
		{
			byte_swap_data(bs_definition, data, structure_count);
			state->offset += size;
		}

		result = data;
	}
	else
	{
		state->overflow_flag = TRUE;
	}

	return result;
}

void *data_decode_memory(
	struct data_encoding_state *state,
	short count,
	long code)
{
	void *result = NULL;
	long size;

	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 256, state && state->buffer && state->offset>=0 && state->offset<=state->buffer_size);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 257, count>=0);

	switch (code)
	{
	case _1byte:
		size = count;
		break;
	case _2byte:
		size = count*sizeof(word);
		break;
	case _4byte:
		size = count*sizeof(long);
		break;
	case _8byte:
		size = count*sizeof(unsigned __int64);
		break;
	default:
		match_halt("c:\\halo\\SOURCE\\memory\\data_encoding.c", 265);
	}

	if (state->offset + size<=state->buffer_size && !state->overflow_flag)
	{
		result = (byte *)state->buffer + state->offset;

		if (code!=_1byte)
		{
			byte_swap_memory(result, count, code);
		}

		state->offset += size;
	}
	else
	{
		state->overflow_flag = TRUE;
	}

	return result;
}

byte data_decode_byte(
	struct data_encoding_state *state)
{
	byte *value = (byte *)data_decode_memory(state, 1, _1byte);

	return value ? *value : 0;
}

short data_decode_short(
	struct data_encoding_state *state)
{
	short *value = (short *)data_decode_memory(state, 1, _2byte);

	return value ? *value : 0;
}

long data_decode_long(
	struct data_encoding_state *state)
{
	long *value = (long *)data_decode_memory(state, 1, _4byte);

	return value ? *value : 0;
}

unsigned __int64 data_decode_int64(
	struct data_encoding_state *state)
{
	unsigned __int64 *value = (unsigned __int64 *)data_decode_memory(state, 1, _8byte);

	return value ? *value : 0;
}

long data_decode_integer(
	struct data_encoding_state *state,
	long maximum_value)
{
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 321, maximum_value>0);

	if (maximum_value<=UNSIGNED_CHAR_MAX)
	{
		return data_decode_byte(state);
	}
	else if (maximum_value<=UNSIGNED_SHORT_MAX)
	{
		return data_decode_short(state);
	}
	else
	{
		return data_decode_long(state);
	}
}

void *data_decode_array(
	struct data_encoding_state *state,
	long element_count_size,
	long *element_count_reference,
	long maximum_element_count,
	struct byte_swap_definition *bs_definition)
{
	long element_count;

	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 348, state && state->buffer && state->offset>=0 && state->offset<state->buffer_size);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 349, element_count_reference);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 350, maximum_element_count>0);
	match_assert("c:\\halo\\SOURCE\\memory\\data_encoding.c", 351, bs_definition);

	switch (element_count_size)
	{
	case _1byte:
		element_count = data_decode_byte(state);
		break;
	case _2byte:
		element_count = data_decode_short(state);
		break;
	case _4byte:
		element_count = data_decode_long(state);
		break;
	case _8byte:
		element_count = (long)data_decode_int64(state);
		break;
	default:
		match_halt("c:\\halo\\SOURCE\\memory\\data_encoding.c", 370);
	}

	if (!state->overflow_flag && element_count>=0 && element_count<=maximum_element_count)
	{
		*element_count_reference = element_count;
		return data_decode_structures(state, (short)element_count, bs_definition);
	}

	return NULL;
}

char *data_decode_string(
	struct data_encoding_state *state,
	short maximum_string_length)
{
	char *string = (char *)state->buffer + state->offset;
	short string_length;
	char *result = NULL;

	for (string_length = 0; TRUE; string_length++)
	{
		if (state->offset + string_length>=state->buffer_size)
		{
			state->overflow_flag = TRUE;
			break;
		}

		if (!string[string_length])
		{
			state->offset += string_length + 1;
			result = string;
			break;
		}
	}

	return result;
}
