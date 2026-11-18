/*
ARRAY.H

header included in hcex build.
*/

#ifndef __ARRAY_H
#define __ARRAY_H
#pragma once

/* ---------- structures */

struct dynamic_array
{
	long element_size;
	long count;
	void *elements;
};

/* ---------- prototypes/ARRAY.C */

void dynamic_array_new(struct dynamic_array *array, long element_size);
boolean dynamic_array_resize(struct dynamic_array *array, long count);
void dynamic_array_delete(struct dynamic_array *array);
long dynamic_array_add_element(struct dynamic_array *array);
void *dynamic_array_get_element(struct dynamic_array const *array, long index, long element_size);
#ifdef DEBUG
#define dynamic_array_get_element_inline dynamic_array_get_element /* fake name */
#endif
void dynamic_array_delete_element(struct dynamic_array *array, long index);
void static_array_new(byte *count, void *elements, short element_size, short maximum_count);
byte static_array_resize(byte *count, void *elements, short element_size, short maximum_count, short new_count);
short static_array_add_element(byte *count, void *elements, short element_size, short maximum_count);
void *static_array_get_element(byte count, void const *elements, short element_size, short index);
void static_array_delete_element(byte *count, void *elements, short element_size, short index);

/* ---------- public code */

#ifndef DEBUG
__inline void *dynamic_array_get_element_inline( /* fake name */
	struct dynamic_array const *array,
	long index,
	long element_size)
{
	return (byte *)array->elements + element_size*index;
}
#endif

#endif // __ARRAY_H
