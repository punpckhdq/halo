/*
DATA.C
*/

/* ---------- headers */

#include "cseries.h"

/* ---------- constants */

enum
{
	DATA_ARRAY_SIGNATURE = 'd@t@',
	DATA_ITERATOR_SIGNATURE = 'iter',
};

/* ---------- prototypes */

static void datum_initialize(struct data_array *data, struct datum_header *header);

/* ---------- public code */

struct data_array *data_new(
	const char *name,
	short maximum_count,
	short size)
{
	struct data_array *data = (struct data_array *)match_malloc("c:\\halo\\SOURCE\\memory\\data.c", 41, data_allocation_size(maximum_count, size));

	if (data)
	{
		data_initialize(data, name, maximum_count, size);
	}

	return data;
}

long data_allocation_size(
	short maximum_count,
	short size)
{
	return maximum_count*size+sizeof(struct data_array);
}

void data_initialize(
	struct data_array *data,
	const char *name,
	short maximum_count,
	short size)
{
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 64, maximum_count>0);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 65, size>0);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 66, name);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 67, data);

	memset(data, 0, sizeof(struct data_array));
	strncpy(data->name, name, TAG_STRING_LENGTH);
	data->maximum_count = maximum_count;
	data->size = size;
	data->signature = DATA_ARRAY_SIGNATURE;
	data->data = data+1;
	data->valid = FALSE;

	return;
}

void data_dispose(
	struct data_array *data)
{
	data_verify(data);
	memset(data, 0, sizeof(struct data_array));
	match_free("c:\\halo\\SOURCE\\memory\\data.c", 89, data);

	return;
}

void data_make_valid(
	struct data_array *data)
{
	data_verify(data);
	data->valid = TRUE;
	data_delete_all(data);

	return;
}

void data_make_invalid(
	struct data_array *data)
{
	data_verify(data);
	data->valid = FALSE;

	return;
}

long datum_new_at_index(
	struct data_array *data,
	long index)
{
	short identifier = DATUM_INDEX_TO_IDENTIFIER(index);
	short absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(index);
	long result = NONE;

	data_verify(data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 123, data->valid);

	if (absolute_index>=0 && absolute_index<data->maximum_count && identifier)
	{
		struct datum_header *header = (struct datum_header *)((byte *)data->data+absolute_index*data->size);

		if (!header->identifier)
		{
			data->actual_count++;
			if (absolute_index>=data->count)
			{
				data->count = absolute_index+1;
			}

			datum_initialize(data, header);
			header->identifier = identifier;

			result = DATUM_INDEX_NEW(absolute_index, identifier);
		}
	}

	return result;
}

long datum_new(
	struct data_array *data)
{
	short absolute_index;
	struct datum_header *header;
	long result = NONE;

	data_verify(data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 163, data->valid);

	for (absolute_index = data->first_free_absolute_index, header = (struct datum_header *)((byte *)data->data+absolute_index*data->size);
		absolute_index<data->maximum_count;
		absolute_index++, header = (struct datum_header *)((byte *)header+data->size))
	{
		if (!header->identifier)
		{
			datum_initialize(data, header);
			data->actual_count++;
			data->first_free_absolute_index = absolute_index+1;
			if (data->count<=absolute_index)
			{
				data->count = absolute_index+1;
			}

			result = DATUM_INDEX_NEW(absolute_index, header->identifier);
			break;
		}
	}

	return result;
}

void datum_delete(
	struct data_array *data,
	long index)
{
	short absolute_index;

	struct datum_header *header = (struct datum_header *)datum_get(data, index);
	header->identifier = 0;

	absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(index);

	if (absolute_index<data->first_free_absolute_index)
	{
		data->first_free_absolute_index = index;
	}

	if (absolute_index+1==data->count)
	{
		do
		{
			header = (struct datum_header *)((byte *)header-data->size);
			data->count--;
		}
		while (data->count > 0 && !header->identifier);
	}
	data->actual_count--;
	
	return;
}

void data_delete_all(
	struct data_array *data)
{
	short absolute_index;

	data_verify(data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 226, data->valid);

	data->count = 0;
	data->actual_count = 0;
	data->first_free_absolute_index = 0;

	/* seed identifier salt with first two characters of the array name */
	strncpy((char *)&data->next_identifier, data->name, sizeof(data->next_identifier));

	data->next_identifier |= SHORT_MIN;

	for (absolute_index = 0; absolute_index<data->maximum_count; absolute_index++)
	{
		struct datum_header *header = (struct datum_header *)((byte *)data->data+absolute_index*data->size);
		header->identifier = 0;
	}

	return;
}

void data_iterator_new(
	struct data_iterator *iterator,
	struct data_array *data)
{
	data_verify(data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 249, data->valid);

	iterator->data = data;
	iterator->signature = (unsigned long)data^DATA_ITERATOR_SIGNATURE;
	iterator->absolute_index = 0;
	iterator->index = NONE;

	return;
}

void *data_iterator_next(
	struct data_iterator *iterator)
{
	void *result = NULL;
	short absolute_index;
	short size;
	struct datum_header *header;

	match_vassert("c:\\halo\\SOURCE\\memory\\data.c", 268, iterator->signature==((unsigned long)iterator->data^DATA_ITERATOR_SIGNATURE), "uninitialized iterator passed to iterator_next()");
	data_verify(iterator->data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 271, iterator->data->valid);

	absolute_index = iterator->absolute_index;
	size = iterator->data->size;
	header = (struct datum_header *)((byte *)iterator->data->data+absolute_index*size);

	while (absolute_index<iterator->data->count)
	{
		long index = DATUM_INDEX_NEW(absolute_index, header->identifier);

		absolute_index++;
		if (header->identifier)
		{
			iterator->index = index;
			result = header;
			break;
		}
		header = (struct datum_header *)((byte *)header+size);
	}
	iterator->absolute_index = absolute_index;

	return result;
}

long data_next_index(
	struct data_array *data,
	long index)
{
	long result = NONE;
	short absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(index)+1;

	data_verify(data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 303, data->valid);

	if (absolute_index>=0 && absolute_index<data->count)
	{
		struct datum_header *header = (struct datum_header *)((byte *)data->data+absolute_index*data->size);

		do
		{
			if (header->identifier)
			{
				result = DATUM_INDEX_NEW(absolute_index, header->identifier);
				break;
			}
			absolute_index++;
			header = (struct datum_header *)((byte *)header+data->size);
		}
		while (absolute_index<data->count);
	}

	return result;
}

long data_prev_index(
	struct data_array *data,
	long index)
{
	short absolute_index;
	long result = NONE;

	data_verify(data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 335, data->valid);

	if (index==NONE)
	{
		absolute_index = data->count-1;
	}
	else
	{
		absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(index)-1;
	}

	if (absolute_index>=0 && absolute_index<data->count)
	{
		struct datum_header *header = (struct datum_header *)((byte *)data->data+absolute_index*data->size);

		do
		{
			if (header->identifier)
			{
				result = DATUM_INDEX_NEW(absolute_index, header->identifier);
				break;
			}
			header = (struct datum_header *)((byte *)header-data->size);
		}
		while (absolute_index-->=0);
	}

	return result;
}

void *datum_try_and_get(
	struct data_array *data,
	long index)
{
	void *result = NULL;

	if (index!=NONE)
	{
		short identifier = DATUM_INDEX_TO_IDENTIFIER(index);
		short absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(index);

		match_assert("c:\\halo\\SOURCE\\memory\\data.c", 371, data->valid);
		match_assert("c:\\halo\\SOURCE\\memory\\data.c", 372, identifier || !data->identifier_zero_invalid);

		if (absolute_index>=0 && absolute_index<data->maximum_count)
		{
			struct datum_header *header = (struct datum_header *)((byte *)data->data+absolute_index*data->size);

			if (header->identifier && (!identifier || header->identifier==identifier))
			{
				result = header;
			}
		}
	}

	return result;
}

void *datum_get(
	struct data_array *data,
	long index)
{
	short identifier = DATUM_INDEX_TO_IDENTIFIER(index);
	short absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(index);
	struct datum_header *header;

	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 396, data->valid);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 397, identifier || !data->identifier_zero_invalid);

	if (absolute_index<0 || absolute_index>=data->count ||
		!(header = (struct datum_header *)((byte *)data->data+absolute_index*data->size))->identifier ||
		(identifier && identifier!=header->identifier))
	{
		match_vassert("c:\\halo\\SOURCE\\memory\\data.c", 412, FALSE, csprintf(temporary, "%s index #%d (0x%x) is unused or changed", data->name, DATUM_INDEX_TO_ABSOLUTE_INDEX(index), index));
		header = NULL;
	}

	return header;
}

void data_compact(
	struct data_array *data)
{
	void *buffer = match_malloc("c:\\halo\\SOURCE\\memory\\data.c", 421, data->size*data->maximum_count);

	data_verify(data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 424, data->valid);

	if (buffer)
	{
		short compacted_count = 0;
		short absolute_index;
		struct datum_header *header;

		for (absolute_index = 0, header = (struct datum_header *)data->data;
			absolute_index<data->count;
			absolute_index++, header = (struct datum_header *)((byte *)header+data->size))
		{
			if (header->identifier)
			{
				memcpy((byte *)buffer+compacted_count*data->size, header, data->size);
				compacted_count++;
			}
		}

		memcpy(data->data, buffer, compacted_count*data->size);
		memset((byte *)data->data+compacted_count*data->size, 0, (data->maximum_count-compacted_count)*data->size);
		data->actual_count = compacted_count;
		data->count = compacted_count;
		data->first_free_absolute_index = compacted_count;

		match_free("c:\\halo\\SOURCE\\memory\\data.c", 447, buffer);
	}

	return;
}

void data_verify(
	struct data_array *data)
{
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 457, data);

	match_vassert(
		"c:\\halo\\SOURCE\\memory\\data.c",
		470,
		data->data &&
		data->signature==DATA_ARRAY_SIGNATURE &&
		data->maximum_count>=0 &&
		data->count>=0 &&
		data->count<=data->maximum_count &&
		data->first_free_absolute_index>=0 &&
		data->first_free_absolute_index<=data->maximum_count &&
		data->actual_count>=0 &&
		data->actual_count<=data->count,
		csprintf(temporary, "%s data array @%p is bad or not allocated", data->name, data));

	return;
}

/* ---------- private code */

static void datum_initialize(
	struct data_array *data,
	struct datum_header *header)
{
	memset(header, 0, data->size);
	header->identifier = data->next_identifier++;
	if (!data->next_identifier)
	{
		data->next_identifier = SHORT_MIN;
	}

	return;
}
