/*
DATA.H

header included in hcex build.
*/

#ifndef __DATA_H
#define __DATA_H
#pragma once

/* ---------- headers */

#include "tag_files.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct data_array
{
	char name[TAG_STRING_LENGTH+1];
	short maximum_count;
	short size;
	boolean valid;
	boolean identifier_zero_invalid;
	unsigned long signature;
	short first_free_absolute_index;
	short count;
	short actual_count;
	short next_identifier;
	void *data;
};

struct datum_header
{
	short identifier;
};

struct data_iterator
{
	struct data_array *data;
    short absolute_index;
    long index;
    unsigned long signature;
};

/* ---------- prototypes/DATA.C */

long data_allocation_size(short maximum_count, short size);
void data_initialize(struct data_array *data, const char *name, short maximum_count, short size);
void *datum_try_and_get(struct data_array *data, long index);
void data_verify(struct data_array *data);
void *datum_get(struct data_array *data, long index);
struct data_array *data_new(const char *name, short maximum_count, short size);
void data_dispose(struct data_array *data);
void data_make_invalid(struct data_array *data);
long datum_new_at_index(struct data_array *data, long index);
long datum_new(struct data_array *data);
void datum_delete(struct data_array *data, long index);
void data_delete_all(struct data_array *data);
void data_iterator_new(struct data_iterator *iterator, struct data_array *data);
void *data_iterator_next(struct data_iterator *iterator);
long data_next_index(struct data_array *data, long index);
long data_prev_index(struct data_array *data, long index);
void data_compact(struct data_array *data);
void data_make_valid(struct data_array *data);


/* ---------- globals */

/* ---------- public code */

#endif // __DATA_H
