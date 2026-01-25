/*
DATA.C
*/

/* ---------- headers */

#include "cseries.h"
#include "data.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void datum_initialize(struct data_array *data, struct datum_header *header);

/* ---------- globals */

/* ---------- public code */

long data_allocation_size(
	short maximum_count, 
	short size)
{
	return maximum_count * size + sizeof(struct data_array);
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
	data->signature = 'd@t@';
	data->data = &data[1];
	data->valid = FALSE;
}

void *datum_try_and_get(
	struct data_array *data, 
	long index)
{
	short identifier = index >> 16;
	short offset = index & 0xFFFF;
	if (index != -1) {
		match_assert("c:\\halo\\SOURCE\\memory\\data.c", 371, data->valid);
		match_assert("c:\\halo\\SOURCE\\memory\\data.c", 372, identifier || !data->identifier_zero_invalid);
		if (offset >= 0 && offset < data->maximum_count) {
		struct datum_header* datum = (struct datum_header*)((byte *)(data->data) + offset * data->size);
			if (datum->identifier) {
				if ((!identifier) || datum->identifier == identifier) {
					return datum;
				}
			}
		}
	}
	return 0;
}

void *datum_get(
	struct data_array *data, 
	long index)
{
	short identifier = index >> 16;
	short offset = index & 0xFFFF;
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 396, data->valid);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 397, identifier || !data->identifier_zero_invalid);
	if (offset >= 0 && offset < data->count) {
		struct datum_header* datum = (struct datum_header*)((byte *)(data->data) + offset * data->size);
		if (datum->identifier) { 
			if ((!identifier) || identifier == datum->identifier) {
				return datum;
			}
		}
	}
	match_vassert("c:\\halo\\SOURCE\\memory\\data.c", 412, FALSE, csprintf(
		temporary,
		"%s index #%d (0x%x) is unused or changed", data->name, index & 0xFFFF, index));
	return 0;
}

void data_verify(
	struct data_array *data) 
{
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 457, data);
	match_vassert("c:\\halo\\SOURCE\\memory\\data.c", 470, 
		data->data 
		&& data->signature == 'd@t@' 
		&& data->maximum_count >= 0
		&& data->count >= 0
		&& data->count <= data->maximum_count
		&& data->first_free_absolute_index >= 0
		&& data->first_free_absolute_index <= data->maximum_count
		&& data->actual_count >= 0
		&& data->actual_count <= data->count,
		csprintf(
			temporary, 
			"%s data array @%p is bad or not allocated", 
			data->name, 
			data));
}

static void datum_initialize(
	struct data_array *data, 
	struct datum_header *header)
{
	memset(header, 0, data->size);
	header->identifier = data->next_identifier;
	data->next_identifier++;
	if (data->next_identifier == 0) {
		data->next_identifier = 0x8000;
	}
}

struct data_array *data_new(
	const char *name, 
	short maximum_count, 
	short size) 
{
	long byte_size = maximum_count * size;
	struct data_array* data = debug_malloc(byte_size + sizeof(struct data_array), FALSE, "c:\\halo\\SOURCE\\memory\\data.c", 41);
	if (data) {
		data_initialize(data, name, maximum_count, size);
	}
	return data;
}

void data_dispose(
	struct data_array *data) 
{
	data_verify(data);
	memset(data, 0, sizeof(struct data_array));
	debug_free(data, "c:\\halo\\SOURCE\\memory\\data.c", 89);
}

void data_make_invalid(
	struct data_array *data) 
{
	data_verify(data);
	data->valid = FALSE;
}

long datum_new_at_index(
	struct data_array *data, 
	long index) 
{
	data_verify(data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 123, data->valid);
	datum_initialize(data, (struct datum_header*)((byte *)data->data + (data->first_free_absolute_index * data->size)));
}

long datum_new(
	struct data_array *data) 
{
	void* datum_position;
	short offset;
	data_verify(data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 163, data->valid);
	datum_position = (void*)((byte *)data->data + (data->first_free_absolute_index * data->size));
	if (data->first_free_absolute_index < data->maximum_count) {
		return -1;
	}
	offset = data->first_free_absolute_index;
	// while (1) {
	// 	if (*(short*)datum_position != 0) break;
	// 	offset++;
	// 	datum_position = (void*)((byte *)datum_position + data->size);
	// 	if (offset >= data->maximum_count) {
	// 		ret_negone:
	// 		return -1;
	// 	}
	// }
	datum_initialize(data, (struct datum_header *)datum_position);
	data->actual_count++;
	data->first_free_absolute_index = offset + 1;
	if (data->count <= offset) {
		data->count = offset + 1;
	}
	return (((struct datum_header*)datum_position)->identifier << 16) | offset;
}

void datum_delete(
	struct data_array *data,
	long index)
{	
	short offset = index & 0xFFFF;
	struct datum_header* ptr = datum_get(data, index);
	ptr->identifier = 0;
	if (offset < data->first_free_absolute_index) {
		data->first_free_absolute_index = offset;
	}
	if (offset + 1  == data->count) {
		do {
			ptr = (struct datum_header*)((byte *)ptr - data->size);
		} while (--data->count > 0 && ptr->identifier == 0);
	}
	data->actual_count--;
}

void data_delete_all(
	struct data_array *data)
{
	short i;
	data_verify(data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 226, data->valid);
	data->count = 0;
	data->actual_count = 0;
	data->first_free_absolute_index = 0;
	strncpy(&data->next_identifier, data->name, 0x2);
	data->next_identifier |= 0x8000;
	for (i = 0; i < data->maximum_count; i++) {
		short* holder;
		holder = (short*)((byte *)data->data + i * data->size);
		*holder = 0;
	}
}

void data_iterator_new(
	struct data_iterator *iterator,
	struct data_array *data)
{
	data_verify(data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 249, data->valid);
	iterator->data = data;
	iterator->signature = 'iter' ^ (unsigned long)data;
	iterator->absolute_index = 0;
	iterator->index = -1;
}

void *data_iterator_next(
	struct data_iterator *iterator)
{
	void* ptr = 0;
	match_vassert("c:\\halo\\SOURCE\\memory\\data.c", 268, (unsigned long)iterator->data == (iterator->signature ^ 'iter'), 
	"uninitialized iterator passed to iterator_next()");
	data_verify(iterator->data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 271, iterator->data->valid);
	ptr = (byte *)iterator->data->data + (iterator->index * iterator->data->size);
	if (iterator->index < iterator->data->count) {

	}
	return ptr;
}

long data_next_index(
	struct data_array *data,
	long index)
{
	long max_index = UNSIGNED_LONG_MAX;
	short offset;
	data_verify(data);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 303, data->valid);
	index++;
	offset = index & 0xFFFF;
	if (offset >= 0 && offset < data->size) {

	}
	return max_index;
}

long data_prev_index(
	struct data_array *data,
	long index)
{

}

void data_compact(
	struct data_array *data)
{
	long unk_ebx = 0;
	struct data_array* saved = data;
	struct data_array* new = debug_malloc(saved->maximum_count * saved->size, unk_ebx, "c:\\halo\\SOURCE\\memory\\data.c", 421);
	data = new;
	data_verify(saved);
	match_assert("c:\\halo\\SOURCE\\memory\\data.c", 424, data->valid);
	if (data != 0) {
		if (data->count > 0) {

		}
		memcpy(saved->data, data, saved->size * unk_ebx);
	}
}

void data_make_valid(
	struct data_array *data)
{
	data_verify(data);
	data->valid = TRUE;
	data_delete_all(data);
}


/* ---------- private code */
