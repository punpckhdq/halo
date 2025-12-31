/*
REFERENCE_LISTS.H

file has inline function assertions.
*/

#ifndef __REFERENCE_LISTS_H
#define __REFERENCE_LISTS_H
#pragma once

/* ---------- headers */

#include "saved games/game_state.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct data_reference
{
	short identifier;
	unsigned short pad;
	long datum_index;
	long next_reference_index;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

__inline struct data_array *reference_list_new(
	const char *name,
	short maximum_count)
{
	char buffer[256];

	sprintf(buffer, "%s reference", name);
	return game_state_data_new(buffer, maximum_count, sizeof(struct data_reference));
}

__inline long reference_list_get_next_datum_index(
	struct data_array *array,
	long *reference_index)
{
	long result;
	struct data_reference *reference;

	if (*reference_index!=NONE)
	{
		reference= (struct data_reference *)datum_get(array, *reference_index);
		*reference_index= reference->next_reference_index;
		result= reference->datum_index;
	}
	else
	{
		result= NONE;
	}

	return result;
}

__inline void reference_list_add(
	struct data_array *array,
	long *first_reference_index,
	long datum_index)
{
	long reference_index= datum_new(array);

	if (reference_index!=NONE)
	{
		struct data_reference *reference= (struct data_reference*)datum_get(array, reference_index);
		reference->datum_index= datum_index;
		reference->next_reference_index= *first_reference_index;
		*first_reference_index= reference_index;
	}
	else
	{
		error(_error_silent, "WARNING: maximum %ss per map (%d) exceeded.", array->name, array->maximum_count);
	}

	return;
}

__inline void reference_list_delete(
	struct data_array *array,
	long first_reference_index)
{
	long i;
	struct data_reference *reference;

	for (i= first_reference_index; i!=NONE; i= reference->next_reference_index)
	{
		reference= (struct data_reference*)datum_get(array, i);
		datum_delete(array, i);
	}

	return;
}

#endif // __REFERENCE_LISTS_H
