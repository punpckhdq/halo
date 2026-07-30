/*
ARRAY.C
*/

/* ---------- headers */

#include "cseries.h"
#include "array.h"

#include "cseries/profile.h"

/* ---------- globals */

static struct profile_section memory_dynamic_array_resize_section = {"memory_dynamic_array_resize", NONE, TRUE};
static struct profile_section memory_dynamic_array_add_element_section = {"memory_dynamic_array_add_element", NONE, TRUE};
static struct profile_section memory_dynamic_array_delete_element_section = { "memory_dynamic_array_delete_element", NONE, TRUE };

/* ---------- public code */

void dynamic_array_new(
	struct dynamic_array *array,
	long element_size)
{
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 16, array);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 17, element_size>0);

	array->element_size = element_size;
	array->count = 0;
	array->elements = 0;

	return;
}

boolean dynamic_array_resize(
	struct dynamic_array *array,
	long count)
{
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 33, array);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 34, array->element_size>0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 35, array->count>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 36, (array->count!=0)==(array->elements!=NULL));

	profile_enter(memory_dynamic_array_resize_section);

	if (count>=0 && count<=LONG_MAX)
	{
		if (count!=array->count)
		{
			void *new_elements = match_realloc("c:\\halo\\SOURCE\\memory\\array.c", 44, array->elements, array->element_size*count);

			if ((count!=0)==(new_elements!=NULL))
			{
				if (count>array->count)
				{
					memset((byte*)new_elements + array->element_size*array->count, 0, (count-array->count)*array->element_size);
				}

				array->count = count;
				array->elements = new_elements;

				result = TRUE;
			}
		}
		else
		{
			result = TRUE;
		}
	}

	profile_exit(memory_dynamic_array_resize_section);

	return result;
}

void dynamic_array_delete(
	struct dynamic_array *array)
{
	void *elements;

	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 73, array);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 74, array->count>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 75, (array->count!=0)==(array->elements!=NULL));

	elements = array->elements;

	array->element_size = NONE;
	array->count = NONE;

	if (elements!=NULL)
	{
		array->elements = match_realloc("c:\\halo\\SOURCE\\memory\\array.c", 80, elements, 0);
	}

	return;
}

long dynamic_array_add_element(
	struct dynamic_array *array)
{
	long new_index = NONE;

	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 93, array);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 94, array->element_size>0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 95, array->count>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 96, (array->count!=0)==(array->elements!=NULL));

	profile_enter(memory_dynamic_array_add_element_section);

	
	if (array->count<LONG_MAX)
	{
		long new_count = array->count+1;
		void *new_elements = match_realloc("c:\\halo\\SOURCE\\memory\\array.c", 103, array->elements, array->element_size*(new_count));

		if (new_elements!=NULL)
		{
			new_index = array->count;

			memset((byte*)new_elements + array->element_size*new_index, 0, array->element_size);

			array->count = new_count;
			array->elements = new_elements;
		}
	}

	profile_exit(memory_dynamic_array_add_element_section);

	return new_index;
}

void *dynamic_array_get_element(
	struct dynamic_array const *array,
	long index,
	long element_size)
{
	void* element;

	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 125, array);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 126, array->element_size>0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 127, array->element_size==element_size);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 128, array->count>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 129, (array->count!=0)==(array->elements!=NULL));
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 130, index>=0 && index<array->count);

	element = (byte*)array->elements + array->element_size*index;
	return element;
}

void dynamic_array_delete_element(
	struct dynamic_array *array,
	long index)
{
	long new_count;

	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 139, array);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 140, array->element_size>0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 141, array->count>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 142, (array->count!=0)==(array->elements!=NULL));
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 143, index>=0 && index<array->count);

	profile_enter(memory_dynamic_array_delete_element_section);

	array->count--;

	if (index<array->count)
	{
		byte *element_start = &((byte *)array->elements)[array->element_size*index];

		memmove(element_start, &element_start[array->element_size], array->element_size * (array->count-index));
	}

	array->elements = match_realloc("c:\\halo\\SOURCE\\memory\\array.c", 156, array->elements, array->element_size*array->count);
	
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 158, (array->count!=0)==(array->elements!=NULL));

	profile_exit(memory_dynamic_array_delete_element_section);


	return;
}

void static_array_new(
	byte *count,
	void *elements,
	short element_size,
	short maximum_count)
{
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 171, count);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 172, elements);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 173, element_size>0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 174, maximum_count<=UNSIGNED_CHAR_MAX);

	*count = 0;
	memset(elements, NONE, element_size*maximum_count);

	return;
}

byte static_array_resize(
	byte *count,
	void *elements,
	short element_size,
	short maximum_count,
	short new_count)
{
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 191, count && *count>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 192, elements);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 193, element_size>0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 194, maximum_count<=UNSIGNED_CHAR_MAX);

	if (new_count>=0 && new_count<maximum_count)
	{
		if (*count!=new_count)
		{
			void *old_end = &((byte *)elements)[element_size * *count];
			void *new_end = &((byte *)elements)[element_size * new_count];

			if (new_end>old_end)
			{
				memset(old_end, 0, (uintptr_t)new_end - (uintptr_t)old_end);
			}
			else
			{
				memset(new_end, NONE, (uintptr_t)old_end - (uintptr_t)new_end);
			}

			*count = new_count;
		}

		result = TRUE;
	}

	return result;
}

short static_array_add_element(
	byte *count,
	void *elements,
	short element_size,
	short maximum_count)
{
	short new_index = NONE;

	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 230, count && *count>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 231, elements);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 232, element_size>0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 233, maximum_count<=UNSIGNED_CHAR_MAX);


	if (*count < maximum_count)
	{
		new_index = (*count)++;
		memset(&((byte*)elements)[element_size*new_index], 0, element_size);
	}

	return new_index;
}

void *static_array_get_element(
	byte count,
	void const *elements,
	short element_size,
	short index)
{
	void* element;

	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 251, count>0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 252, elements);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 253, element_size>0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 254, index>=0 && index<count);

	element = (byte*)elements + element_size*index;
	return element;
}

void static_array_delete_element(
	byte *count,
	void *elements,
	short element_size,
	short index)
{
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 265, count && *count>0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 266, elements);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 267, element_size>0);
	match_assert("c:\\halo\\SOURCE\\memory\\array.c", 268, index>=0 && index<*count);

	if (index < (long)(--*count))
	{
		byte *element_start = &((byte *)elements)[element_size*index];
		memmove(
			element_start,
			&element_start[element_size],
			element_size * (*count-index));
	}

	memset(&((byte*)elements)[element_size * *count], NONE, element_size);

	return;
}
