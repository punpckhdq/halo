/*
OBJECT_LISTS.C

symbols in this file:
000BD3F0 0040:
	_reference_list_new (0000)
000BD430 0030:
	_reference_list_get_next_datum_index (0000)
000BD460 0050:
	_reference_list_add (0000)
000BD4B0 0040:
	_reference_list_delete (0000)
000BD4F0 0060:
	_object_lists_initialize (0000)
000BD550 0010:
	_object_lists_dispose (0000)
000BD560 0020:
	_object_lists_initialize_for_new_map (0000)
000BD580 0020:
	_object_lists_dispose_from_old_map (0000)
000BD5A0 0040:
	_object_list_new (0000)
000BD5E0 0070:
	_object_list_delete (0000)
000BD650 0070:
	_object_list_add (0000)
000BD6C0 0030:
	_object_list_get_next (0000)
000BD6F0 0020:
	_object_list_add_reference (0000)
000BD710 0050:
	_object_list_remove_reference (0000)
000BD760 0060:
	_object_list_gc (0000)
000BD7C0 0030:
	_object_list_count (0000)
000BD7F0 0050:
	_object_list_get_first (0000)
0026F2B4 000d:
	??_C@_0N@DOAKPICE@?$CFs?5reference?$AA@ (0000)
0026F2C4 002c:
	??_C@_0CM@PKPEAIHK@WARNING?3?5maximum?5?$CFss?5per?5map?5?$CI?$CFd@ (0000)
0026F2F0 000c:
	??_C@_0M@CNNGENLL@list?5object?$AA@ (0000)
0026F2FC 0013:
	??_C@_0BD@BCDBOOBL@object?5list?5header?$AA@ (0000)
0026F310 0019:
	??_C@_0BJ@LLENHKLF@list?9?$DOreference_count?$DN?$DN0?$AA@ (0000)
0026F32C 0021:
	??_C@_0CB@EEGJCDAK@c?3?2halo?2SOURCE?2hs?2object_lists?4c@ (0000)
0026F350 0018:
	??_C@_0BI@NHAMEGAN@list?9?$DOreference_count?$DO0?$AA@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "object_lists.h"

#include "cseries/errors.h"
#include "memory/data.h"
#include "objects/reference_lists.h"

/* ---------- constants */

enum
{
	MAXIMUM_OBJECT_LISTS_PER_MAP= 48,
	MAXIMUM_LISTED_OBJECTS_PER_MAP= 128,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void object_lists_initialize(
	void)
{
	object_list_header_data= game_state_data_new("object list header", MAXIMUM_OBJECT_LISTS_PER_MAP, sizeof(struct object_list_header_datum));
	object_list_data= reference_list_new("list object", sizeof(struct data_reference));

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
	long list_index= datum_new(object_list_header_data);

	if (list_index != NONE)
	{
		struct object_list_header_datum* list= object_list_header_get(list_index);
		list->count= 0;
		list->first_reference_index= NONE;
	}

	return list_index;
}

void object_list_delete(
	long list_index)
{
	if (list_index != NONE)
	{
		struct object_list_header_datum *list= object_list_header_get(list_index);
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
	struct object_list_header_datum *list= object_list_header_get(object_list_index);
	reference_list_add(object_list_header_data, &list->first_reference_index, object_index);
	list->count++;

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
		struct object_list_header_datum *list= object_list_header_get(object_list_index);
		list->reference_count++;
	}

	return;
}

void object_list_remove_reference(
	long object_list_index)
{
	if (object_list_index!=NONE)
	{
		struct object_list_header_datum *list= object_list_header_get(object_list_index);
		match_assert("c:\\halo\\SOURCE\\hs\\object_lists.c", 165, list->reference_count>0);
		list->reference_count--;
	}

	return;
}

void object_list_gc(
	void)
{
	long i;

	for (
		i= data_next_index(object_list_header_data, NONE);
		i!=NONE; 
		i= data_next_index(object_list_header_data, i))
	{
		if (object_list_header_get(i)->reference_count==0)
		{
			object_list_delete(i);
		}
	}

	return;
}

short object_list_count(
	long object_list_index)
{
	short result= 0;

	if (object_list_index!=NONE)
	{
		result= object_list_header_get(object_list_index)->count;
	}

	return result;
}

long object_list_get_first(
	long object_list_index,
	long *reference_index)
{
	long result= NONE;

	if (object_list_index!=NONE)
	{
		struct object_list_header_datum *list= object_list_header_get(object_list_index);

		*reference_index= list->first_reference_index;
		if (list->first_reference_index!=NONE)
		{
			struct data_reference *reference= object_list_get(list->first_reference_index);
			*reference_index= reference->next_reference_index;
			result= reference->datum_index;
		}
	}

	return result;
}

/* ---------- private code */
