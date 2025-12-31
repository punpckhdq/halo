/*
OBJECT_LISTS.H
*/

#ifndef __OBJECT_LISTS_H
#define __OBJECT_LISTS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

#define object_list_header_get(index)	((struct object_list_header_datum*)(datum_get(object_list_header_data, (index))))

#define object_list_get(index)	((struct data_reference*)(datum_get(object_list_data, (index))))

/* ---------- structures */

struct object_list_header_datum
{
	short identifier;
	unsigned short pad;
	short reference_count;
	short count;
	long first_reference_index;
};

/* ---------- prototypes/OBJECT_LISTS.C */

void object_lists_initialize(void);
void object_lists_dispose(void);
void object_lists_initialize_for_new_map(void);
void object_lists_dispose_from_old_map(void);
long object_list_new(void);
void object_list_delete(long list_index);
void object_list_add(long object_list_index, long object_index);


/* ---------- globals */

struct data_array *object_list_header_data;
struct data_array *object_list_data;

/* ---------- public code */

#endif // __OBJECT_LISTS_H
