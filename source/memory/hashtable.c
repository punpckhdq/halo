/*
HASHTABLE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "hashtable.h"

/* ---------- macros */

#define hashtable_valid(table) ((table) && (table)->key_size>0 && (table)->element_size>0 && (table)->load_factor>0 && (table)->load_factor<=1 && ((table)->capacity_bits==NONE || FLAG((table)->capacity_bits)==(table)->elements.count))
#define hashtable_wrap_index(table, index) ((index) & ((table)->elements.count - 1)) /* fake name */
#define hashtable_hash(table, key) ((table)->hash_function ? (table)->hash_function((table)->user_data, (key)) : default_hash_function((key), (table)->key_size)) /* fake name */
#define hashtable_element_get(table, index) dynamic_array_get_element(&(table)->elements, (index), (table)->element_size) /* fake name */
#define hashtable_element_value(table, element) ((byte *)(element) + (table)->key_size) /* fake name */

/* ---------- prototypes */

static boolean hashtable_search(struct hashtable *table, void const *key, short *index);
static void *hashtable_capacious_put(struct hashtable *table, void const *key);

/* ---------- globals */

short default_hash_polynomial[] = {3, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59};

/* ---------- public code */

void hashtable_new(
	struct hashtable *table,
	short key_size,
	short element_size,
	real load_factor,
	hashtable_hash_function hash_function,
	hashtable_compare_function compare_function)
{
	match_assert("c:\\halo\\SOURCE\\memory\\hashtable.c", 41, table);
	match_assert("c:\\halo\\SOURCE\\memory\\hashtable.c", 42, key_size>0);
	match_assert("c:\\halo\\SOURCE\\memory\\hashtable.c", 43, element_size>0);
	match_assert("c:\\halo\\SOURCE\\memory\\hashtable.c", 44, load_factor>0 && load_factor<=1);

	table->key_size = key_size;
	table->element_size = element_size;
	table->count = 0;
	table->capacity_bits = NONE;
	table->load_factor = load_factor;
	table->hash_function = hash_function;
	table->compare_function = compare_function;
	dynamic_array_new(&table->elements, key_size + element_size);
	table->used = NULL;

	return;
}

void hashtable_set_user_data(
	struct hashtable *table,
	void *user_data)
{
	table->user_data = user_data;

	return;
}

void *hashtable_get(
	struct hashtable *table,
	void const *key)
{
	void *result = NULL;
	short index;

	match_assert("c:\\halo\\SOURCE\\memory\\hashtable.c", 77, hashtable_valid(table));

	if (table->count && hashtable_search(table, key, &index))
	{
		result = hashtable_element_value(table, hashtable_element_get(table, index));
	}

	return result;
}

void *hashtable_put(
	struct hashtable *table,
	void const *key)
{
	match_assert("c:\\halo\\SOURCE\\memory\\hashtable.c", 93, hashtable_valid(table));

	if (table->capacity_bits==NONE || table->count>=table->elements.count*table->load_factor)
	{
		if (!hashtable_grow(table, table->capacity_bits==NONE ? 2 : 1))
		{
			return NULL;
		}
	}

	return hashtable_capacious_put(table, key);
}

void hashtable_delete(
	struct hashtable *table)
{
	match_assert("c:\\halo\\SOURCE\\memory\\hashtable.c", 110, hashtable_valid(table));

	dynamic_array_delete(&table->elements);

	if (table->used)
	{
		match_free("c:\\halo\\SOURCE\\memory\\hashtable.c", 116, table->used);
	}

	return;
}

boolean hashtable_grow(
	struct hashtable *table,
	short growth_bits)
{
	short old_capacity_bits = table->capacity_bits;
	short old_count = table->count;
	unsigned long *old_used = table->used;
	struct dynamic_array old_elements = table->elements;
	short capacity;

	match_assert("c:\\halo\\SOURCE\\memory\\hashtable.c", 134, hashtable_valid(table));
	match_assert("c:\\halo\\SOURCE\\memory\\hashtable.c", 135, growth_bits>0);
	match_assert("c:\\halo\\SOURCE\\memory\\hashtable.c", 136, table->capacity_bits+growth_bits<SHORT_BITS);

	table->capacity_bits += growth_bits;
	capacity = FLAG(table->capacity_bits);
	table->count = 0;
	table->used = (unsigned long *)match_malloc("c:\\halo\\SOURCE\\memory\\hashtable.c", 143, BIT_VECTOR_SIZE_IN_BYTES(capacity));

	if (table->used)
	{
		dynamic_array_new(&table->elements, table->elements.element_size);

		if (dynamic_array_resize(&table->elements, capacity))
		{
			short index;

			memset(table->used, 0, BIT_VECTOR_SIZE_IN_BYTES(capacity));

			for (index = 0; index<old_elements.count; index++)
			{
				if (BIT_VECTOR_TEST_FLAG(old_used, index))
				{
					void *element = dynamic_array_get_element(&old_elements, index, old_elements.element_size);

					memcpy(hashtable_capacious_put(table, element), hashtable_element_value(table, element), table->element_size);
				}
			}

			if (old_used)
			{
				match_free("c:\\halo\\SOURCE\\memory\\hashtable.c", 168, old_used);
			}

			dynamic_array_delete(&old_elements);

			return TRUE;
		}

		match_free("c:\\halo\\SOURCE\\memory\\hashtable.c", 176, table->used);
	}

	table->capacity_bits = old_capacity_bits;
	table->count = old_count;
	table->used = old_used;
	table->elements = old_elements;

	return FALSE;
}

void hashtable_remove(
	struct hashtable *table,
	void const *key)
{
	short index;

	match_assert("c:\\halo\\SOURCE\\memory\\hashtable.c", 195, hashtable_valid(table));
	match_assert("c:\\halo\\SOURCE\\memory\\hashtable.c", 196, key);

	if (hashtable_search(table, key, &index))
	{
		short next_index = index;

		while (TRUE)
		{
			void *element;
			short hash_index;

			next_index = hashtable_wrap_index(table, next_index + 1);

			if (!BIT_VECTOR_TEST_FLAG(table->used, next_index))
			{
				break;
			}

			element = hashtable_element_get(table, next_index);
			hash_index = hashtable_wrap_index(table, hashtable_hash(table, element));

			if ((hash_index<next_index && index>=hash_index && index<next_index) || (hash_index>next_index && (index>=hash_index || index<next_index)))
			{
				memcpy(hashtable_element_get(table, index), hashtable_element_get(table, next_index), table->elements.element_size);
				index = next_index;
			}
		}

		BIT_VECTOR_SET_FLAG(table->used, index, FALSE);
	}
	else
	{
		match_vhalt("c:\\halo\\SOURCE\\memory\\hashtable.c", 225, "removing key not in hashtable");
	}

	return;
}

short default_hash_function(
	byte const *key,
	unsigned long key_size)
{
	short hash = 0;
	short index;

	for (index = 0; index<key_size; index++, key++)
	{
		if (index==NUMBEROF(default_hash_polynomial))
		{
			index = 0;
			key_size -= NUMBEROF(default_hash_polynomial);
		}

		hash += default_hash_polynomial[index] * *key;
	}

	return hash;
}

/* ---------- private code */

static void *hashtable_capacious_put(
	struct hashtable *table,
	void const *key)
{
	void *result = NULL;
	short index;

	if (hashtable_search(table, key, &index))
	{
		match_vhalt("c:\\halo\\SOURCE\\memory\\hashtable.c", 241, "putting key already in hashtable");
	}
	else
	{
		void *element = hashtable_element_get(table, index);

		memcpy(element, key, table->key_size);
		BIT_VECTOR_SET_FLAG(table->used, index, TRUE);
		result = hashtable_element_value(table, element);
		table->count++;
	}

	return result;
}

static boolean hashtable_search(
	struct hashtable *table,
	void const *key,
	short *index)
{
	boolean found = FALSE;
	short probe_count = 0;
	short element_index = hashtable_wrap_index(table, hashtable_hash(table, key));

	while (BIT_VECTOR_TEST_FLAG(table->used, element_index) && probe_count<table->count)
	{
		if (table->compare_function ?
			table->compare_function(table->user_data, hashtable_element_get(table, element_index), key) :
			!memcmp(hashtable_element_get(table, element_index), key, table->key_size))
		{
			found = TRUE;
			break;
		}

		element_index = hashtable_wrap_index(table, element_index + 1);
		probe_count++;
	}

	*index = element_index;
	return found;
}
