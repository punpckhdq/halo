/*
OBJECT_LISTS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "object_lists.h"

#include "data.h"
#include "reference_lists.h"

/* ---------- constants */

enum
{
	MAXIMUM_OBJECT_LISTS_PER_MAP = 48,
	MAXIMUM_LISTED_OBJECTS_PER_MAP = 128,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void object_lists_initialize(
	void)
{
	object_list_header_data = game_state_data_new("object list header", MAXIMUM_OBJECT_LISTS_PER_MAP, sizeof(struct object_list_header_datum));
	object_list_data = reference_list_new("list object", MAXIMUM_LISTED_OBJECTS_PER_MAP);

	return;
}

void object_lists_dispose(
	void)
{
	return;
}

void object_lists_initialize_for_new_map(
	void)
{
	data_make_valid(object_list_header_data);
	data_make_valid(object_list_data);
	return;
}

void object_lists_dispose_from_old_map(
	void)
{
	data_make_invalid(object_list_header_data);
	data_make_invalid(object_list_data);
	return;
}

long object_list_new(
	void)
{
	long list_index = datum_new(object_list_header_data);

	if (list_index != NONE)
	{
		struct object_list_header_datum* list = object_list_header_get(list_index);

		list->count = 0;
		list->first_reference_index = NONE;
	}

	return list_index;
}

void object_list_delete(
	long list_index)
{
	if (list_index != NONE)
	{
		struct object_list_header_datum *list = object_list_header_get(list_index);

		match_assert("c:\\halo\\SOURCE\\hs\\object_lists.c", 100, list->reference_count==0);
		reference_list_delete(object_list_data, list->first_reference_index);
		datum_delete(object_list_header_data, list_index);
	}

	return;
}

void object_list_add(
	long object_list_index,
	long object_index)
{
	struct object_list_header_datum *list = object_list_header_get(object_list_index);

	reference_list_add(object_list_data, &list->first_reference_index, object_index);
	++list->count;
	return;
}

long object_list_get_next(
	long object_list_index,
	long *reference_index)
{
	return reference_list_get_next_datum_index(object_list_data, reference_index);
}

void object_list_add_reference(
	long object_list_index)
{
	if (object_list_index!=NONE)
	{
		struct object_list_header_datum *list = object_list_header_get(object_list_index);
		++list->reference_count;
	}

	return;
}

void object_list_remove_reference(
	long object_list_index)
{
	if (object_list_index!=NONE)
	{
		struct object_list_header_datum *list = object_list_header_get(object_list_index);
		
		match_assert("c:\\halo\\SOURCE\\hs\\object_lists.c", 165, list->reference_count>0);
		--list->reference_count;
	}

	return;
}

void object_list_gc(
	void)
{
	long object_list_index;

	for (object_list_index = data_next_index(object_list_header_data, NONE);
		object_list_index!=NONE; 
		object_list_index = data_next_index(object_list_header_data, object_list_index))
	{
		struct object_list_header_datum const *list = object_list_header_get(object_list_index);

		if (list->reference_count==0)
		{
			object_list_delete(object_list_index);
		}
	}

	return;
}

short object_list_count(
	long object_list_index)
{
	short count = 0;

	if (object_list_index!=NONE)
	{
		struct object_list_header_datum const *list = object_list_header_get(object_list_index);

		count = list->count;
	}

	return count;
}

long object_list_get_first(
	long object_list_index,
	long *reference_index)
{
	long first_object_index = NONE;
	
	if (object_list_index != NONE)
	{
		struct object_list_header_datum const *list = object_list_header_get(object_list_index);

		*reference_index = list->first_reference_index;
		first_object_index = object_list_get_next(object_list_index, reference_index);
	}

	return first_object_index;
}

/* ---------- private code */
