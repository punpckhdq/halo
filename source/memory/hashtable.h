/*
HASHTABLE.H

header included in hcex build.
*/

#ifndef __HASHTABLE_H
#define __HASHTABLE_H
#pragma once

/* ---------- structures */

typedef short (*hashtable_hash_function)(void *user_data, void const *key); /* fake name */
typedef boolean (*hashtable_compare_function)(void *user_data, void const *element, void const *key); /* fake name */

struct hashtable
{
	short key_size;
	short element_size;
	short count; /* fake name */
	short capacity_bits;
	real load_factor;
	void *user_data;
	hashtable_hash_function hash_function;
	hashtable_compare_function compare_function;
	unsigned long *used; /* fake name */
	struct dynamic_array elements; /* fake name */
};

/* ---------- prototypes/HASHTABLE.C */

void hashtable_new(struct hashtable *table, short key_size, short element_size, real load_factor, hashtable_hash_function hash_function, hashtable_compare_function compare_function);
void hashtable_set_user_data(struct hashtable *table, void *user_data);
void *hashtable_get(struct hashtable *table, void const *key);
void *hashtable_put(struct hashtable *table, void const *key);
void hashtable_delete(struct hashtable *table);
boolean hashtable_grow(struct hashtable *table, short growth_bits);
void hashtable_remove(struct hashtable *table, void const *key);
short default_hash_function(byte const *key, unsigned long key_size);

#endif // __HASHTABLE_H
