/*
LRUV_CACHE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "lruv_cache.h"

/* ---------- constants */

enum
{
	LRUV_CACHE_SIGNATURE = 'weee',
};

enum
{
	MAXIMUM_LRUV_CACHE_HOLES = 256, /* fake name */
};

/* ---------- macros */

#define lruv_cache_block_get(cache, block_index) ((struct lruv_cache_block *)datum_get((cache)->blocks, (block_index)))

/* ---------- structures */

struct lruv_cache
{
	char name[32];
	lruv_delete_block_proc delete_block_proc;
	lruv_locked_block_proc locked_block_proc;
	long page_count;
	long page_size_bits;
	unsigned long frame_index;
	long first_block_index;
	long last_block_index;
	struct data_array *blocks;
	unsigned long signature;
};

struct lruv_cache_block
{
	short identifier;
	byte unused_flags[2];
	long page_count;
	long first_page_index;
	long next_block_index;
	long previous_block_index;
	unsigned long last_used_frame_index;
	unsigned long unused[1];
};

struct lruv_cache_hole
{
	long previous_block_index;
	unsigned long last_used_frame_index;
	long first_page_index;
	long page_count;
};

/* ---------- prototypes */

static long lruv_cache_bytes_to_pages(struct lruv_cache *cache, long size_in_bytes);
static void lruv_cache_verify(struct lruv_cache *cache, boolean verify_blocks);

/* ---------- public code */

struct lruv_cache *lruv_new(
	char const *name,
	long page_count,
	long page_size_bits,
	long maximum_block_count,
	lruv_delete_block_proc delete_block_proc,
	lruv_locked_block_proc locked_block_proc)
{
	struct lruv_cache *cache = (struct lruv_cache *)match_malloc("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 82, lruv_allocation_size(maximum_block_count));

	if (cache)
	{
		lruv_initialize(cache, name, page_count, page_size_bits, maximum_block_count, delete_block_proc, locked_block_proc);
	}

	return cache;
}

long lruv_allocation_size(
	long maximum_block_count)
{
	return sizeof(struct lruv_cache) + data_allocation_size(maximum_block_count, sizeof(struct lruv_cache_block));
}

void lruv_initialize(
	struct lruv_cache *cache,
	char const *name,
	long page_count,
	long page_size_bits,
	long maximum_block_count,
	lruv_delete_block_proc delete_block_proc,
	lruv_locked_block_proc locked_block_proc)
{
	struct data_array *blocks = (struct data_array *)(cache + 1);

	match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 109, name);
	match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 110, page_count>0);
	match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 111, page_size_bits>0 && page_size_bits<SHORT_BITS);
	match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 112, maximum_block_count>0);

	data_initialize(blocks, name, maximum_block_count, sizeof(struct lruv_cache_block));
	data_make_valid(blocks);

	memset(cache, 0, sizeof(*cache));
	strncpy(cache->name, name, sizeof(cache->name) - 1);
	cache->delete_block_proc = delete_block_proc;
	cache->locked_block_proc = locked_block_proc;
	cache->page_count = page_count;
	cache->page_size_bits = page_size_bits;
	cache->blocks = blocks;
	cache->signature = LRUV_CACHE_SIGNATURE;
	cache->first_block_index = NONE;
	cache->last_block_index = NONE;
	cache->frame_index = 1;
	lruv_cache_verify(cache, TRUE);

	return;
}

void lruv_update_function_pointers(
	struct lruv_cache *cache,
	lruv_delete_block_proc delete_block_proc,
	lruv_locked_block_proc locked_block_proc)
{
	match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 148, cache);

	cache->delete_block_proc = delete_block_proc;
	cache->locked_block_proc = locked_block_proc;

	return;
}

void lruv_delete(
	struct lruv_cache *cache)
{
	lruv_cache_verify(cache, TRUE);
	data_dispose(cache->blocks);
	memset(cache, 0, sizeof(*cache));
	match_free("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 163, cache);

	return;
}

void lruv_idle(
	struct lruv_cache *cache)
{
	lruv_cache_verify(cache, FALSE);
	cache->frame_index++;

	return;
}

void lruv_flush(
	struct lruv_cache *cache)
{
	struct data_iterator iterator;

	lruv_cache_verify(cache, TRUE);

	data_iterator_new(&iterator, cache->blocks);

	while (data_iterator_next(&iterator))
	{
		lruv_block_delete(cache, iterator.index);
	}

	return;
}

long lruv_block_new(
	struct lruv_cache *cache,
	long size)
{
	long desired_page_count = lruv_cache_bytes_to_pages(cache, size);
	long block_index = NONE;
	struct lruv_cache_hole best_hole;
	boolean found_hole = FALSE;
	long oldest_unlocked_block_index = NONE;
	unsigned long oldest_block_last_used_frame_index;
	struct lruv_cache_hole holes[MAXIMUM_LRUV_CACHE_HOLES];
	short hole_read_index;
	short hole_write_index;
	long next_block_index;
	long previous_block_index;
	long page_index;

	match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 225, desired_page_count>0);

	hole_read_index = 0;
	hole_write_index = 0;
	next_block_index = cache->first_block_index;
	previous_block_index = NONE;
	page_index = 0;

	while (page_index<cache->page_count)
	{
		unsigned long last_used_frame_index;
		long page_count;
		boolean locked;

		if ((hole_write_index==MAXIMUM_LRUV_CACHE_HOLES - 1 ? 0 : hole_write_index + 1)!=hole_read_index)
		{
			struct lruv_cache_hole *hole = &holes[hole_write_index];

			hole->previous_block_index = previous_block_index;
			hole->first_page_index = page_index;
			hole->last_used_frame_index = 0;
			hole->page_count = 0;
			hole_write_index = hole_write_index==MAXIMUM_LRUV_CACHE_HOLES - 1 ? 0 : hole_write_index + 1;
		}

		if (next_block_index==NONE)
		{
			last_used_frame_index = 0;
			page_count = cache->page_count - page_index;
			locked = FALSE;
			page_index = cache->page_count;
		}
		else
		{
			struct lruv_cache_block *block = lruv_cache_block_get(cache, next_block_index);

			if (page_index==block->first_page_index)
			{
				last_used_frame_index = block->last_used_frame_index;
				page_count = block->page_count;
				locked = cache->locked_block_proc && cache->locked_block_proc(next_block_index) ? TRUE : FALSE;

				if (block->last_used_frame_index==cache->frame_index)
				{
					locked = TRUE;
				}

				if (!locked && (oldest_unlocked_block_index==NONE || block->last_used_frame_index<oldest_block_last_used_frame_index))
				{
					oldest_unlocked_block_index = next_block_index;
					oldest_block_last_used_frame_index = block->last_used_frame_index;
				}

				page_index = block->first_page_index + block->page_count;
				previous_block_index = next_block_index;
				next_block_index = block->next_block_index;
			}
			else
			{
				last_used_frame_index = 0;
				page_count = block->first_page_index - page_index;
				locked = FALSE;
				match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 311, page_count>0);
				page_index = block->first_page_index;
			}
		}

		if (locked)
		{
			hole_read_index = hole_write_index;
		}
		else
		{
			short hole_index;

			for (hole_index = hole_read_index; hole_index!=hole_write_index; hole_index = hole_index==MAXIMUM_LRUV_CACHE_HOLES - 1 ? 0 : hole_index + 1)
			{
				struct lruv_cache_hole *hole = &holes[hole_index];

				if (last_used_frame_index>hole->last_used_frame_index)
				{
					hole->last_used_frame_index = last_used_frame_index;
				}

				hole->page_count += page_count;

				if (hole->page_count>=desired_page_count)
				{
					if (!found_hole ||
						hole->last_used_frame_index<best_hole.last_used_frame_index ||
						(hole->last_used_frame_index==best_hole.last_used_frame_index && hole->page_count<best_hole.page_count))
					{
						best_hole = *hole;
						found_hole = TRUE;
					}

					match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 351, hole_read_index==hole_index);
					hole_read_index = hole_read_index==MAXIMUM_LRUV_CACHE_HOLES - 1 ? 0 : hole_read_index + 1;
				}
			}
		}
	}

	if (found_hole)
	{
		struct data_iterator iterator;
		struct lruv_cache_block *block;

		data_iterator_new(&iterator, cache->blocks);

		while (block = (struct lruv_cache_block *)data_iterator_next(&iterator))
		{
			if (block->first_page_index<best_hole.first_page_index + desired_page_count && block->first_page_index + block->page_count>best_hole.first_page_index)
			{
				match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 375, !cache->locked_block_proc || !cache->locked_block_proc(iterator.index));
				lruv_block_delete(cache, iterator.index);
			}
		}

		if (cache->blocks->actual_count==cache->blocks->maximum_count && oldest_unlocked_block_index!=NONE)
		{
			if (best_hole.previous_block_index==oldest_unlocked_block_index)
			{
				best_hole.previous_block_index = lruv_cache_block_get(cache, oldest_unlocked_block_index)->previous_block_index;
			}

			match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 392, lruv_cache_block_get(cache, oldest_unlocked_block_index));
			match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 393, !cache->locked_block_proc || !cache->locked_block_proc(oldest_unlocked_block_index));
			lruv_block_delete(cache, oldest_unlocked_block_index);
		}

		block_index = datum_new(cache->blocks);

		if (block_index!=NONE)
		{
			block = lruv_cache_block_get(cache, block_index);

			if (best_hole.previous_block_index==NONE)
			{
				if (cache->first_block_index==NONE)
				{
					match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 408, cache->last_block_index==NONE);
					block->previous_block_index = NONE;
					cache->last_block_index = block_index;
				}
				else
				{
					struct lruv_cache_block *next_block = lruv_cache_block_get(cache, cache->first_block_index);

					match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 416, next_block->previous_block_index==NONE);
					block->previous_block_index = NONE;
					next_block->previous_block_index = block_index;
				}
			}
			else
			{
				struct lruv_cache_block *previous_block = lruv_cache_block_get(cache, best_hole.previous_block_index);

				if (previous_block->next_block_index==NONE)
				{
					block->previous_block_index = cache->last_block_index;
					cache->last_block_index = block_index;
				}
				else
				{
					struct lruv_cache_block *next_block = lruv_cache_block_get(cache, previous_block->next_block_index);

					block->previous_block_index = next_block->previous_block_index;
					next_block->previous_block_index = block_index;
				}
			}

			if (best_hole.previous_block_index==NONE)
			{
				block->next_block_index = cache->first_block_index;
				cache->first_block_index = block_index;
			}
			else
			{
				struct lruv_cache_block *previous_block = lruv_cache_block_get(cache, best_hole.previous_block_index);

				block->next_block_index = previous_block->next_block_index;
				previous_block->next_block_index = block_index;
			}

			block->first_page_index = best_hole.first_page_index;
			block->page_count = desired_page_count;
			block->last_used_frame_index = cache->frame_index;
			lruv_cache_verify(cache, TRUE);
		}
	}

	return block_index;
}

void lruv_block_delete(
	struct lruv_cache *cache,
	long block_index)
{
	struct lruv_cache_block *block = lruv_cache_block_get(cache, block_index);

	lruv_cache_verify(cache, TRUE);

	if (cache->delete_block_proc)
	{
		cache->delete_block_proc(block_index);
	}

	if (block->previous_block_index!=NONE)
	{
		struct lruv_cache_block *previous_block = lruv_cache_block_get(cache, block->previous_block_index);

		previous_block->next_block_index = block->next_block_index;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 488, cache->first_block_index==block_index);
		cache->first_block_index = block->next_block_index;
	}

	if (block->next_block_index!=NONE)
	{
		struct lruv_cache_block *next_block = lruv_cache_block_get(cache, block->next_block_index);

		next_block->previous_block_index = block->previous_block_index;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 501, cache->last_block_index==block_index);
		cache->last_block_index = block->previous_block_index;
	}

	datum_delete(cache->blocks, block_index);
	lruv_cache_verify(cache, TRUE);

	return;
}

void lruv_block_touch(
	struct lruv_cache *cache,
	long block_index)
{
	struct lruv_cache_block *block;

	lruv_cache_verify(cache, FALSE);
	block = lruv_cache_block_get(cache, block_index);
	block->last_used_frame_index = cache->frame_index;

	return;
}

unsigned long lruv_block_get_address(
	struct lruv_cache *cache,
	long block_index)
{
	struct lruv_cache_block *block;

	lruv_cache_verify(cache, FALSE);
	block = lruv_cache_block_get(cache, block_index);

	return block->first_page_index<<cache->page_size_bits;
}

boolean lruv_block_touched(
	struct lruv_cache *cache,
	long block_index)
{
	struct lruv_cache_block *block;

	lruv_cache_verify(cache, FALSE);
	block = lruv_cache_block_get(cache, block_index);

	return block->last_used_frame_index==cache->frame_index;
}

void lruv_cache_get_page_usage(
	struct lruv_cache *cache,
	byte *page_usage)
{
	struct data_iterator iterator;
	struct lruv_cache_block *block;

	lruv_cache_verify(cache, TRUE);
	memset(page_usage, 0, cache->page_count);

	data_iterator_new(&iterator, cache->blocks);

	while (block = (struct lruv_cache_block *)data_iterator_next(&iterator))
	{
		byte flags = FLAG(_lruv_cache_page_usage_allocated_bit);

		if (cache->locked_block_proc && cache->locked_block_proc(iterator.index))
		{
			SET_FLAG(flags, _lruv_cache_page_usage_locked_bit, TRUE);
		}

		if (block->last_used_frame_index==cache->frame_index)
		{
			SET_FLAG(flags, _lruv_cache_page_usage_used_this_frame_bit, TRUE);
		}

		if (block->last_used_frame_index + 30<cache->frame_index)
		{
			SET_FLAG(flags, _lruv_cache_page_usage_old_bit, TRUE);
		}

		memset(page_usage + block->first_page_index, flags, block->page_count);
	}

	return;
}

void lruv_resize(
	struct lruv_cache *cache,
	long new_page_count)
{
	struct data_iterator iterator;
	struct lruv_cache_block *block;

	match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 603, new_page_count>0);
	lruv_cache_verify(cache, TRUE);

	data_iterator_new(&iterator, cache->blocks);

	while (block = (struct lruv_cache_block *)data_iterator_next(&iterator))
	{
		if (block->first_page_index + block->page_count>new_page_count)
		{
			lruv_block_delete(cache, iterator.index);
		}
	}

	cache->page_count = new_page_count;

	return;
}

boolean lruv_has_locked_proc(
	struct lruv_cache const *cache)
{
	match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 631, cache);

	return cache->locked_block_proc!=NULL;
}

void lruv_debug_to_file(
	char const *path,
	char const *failed_allocation_name,
	long failed_allocation_size,
	struct lruv_cache *cache,
	lruv_header_proc header_proc,
	lruv_name_block_proc name_block_proc)
{
	FILE *file;

	lruv_cache_verify(cache, TRUE);

	file = fopen(path, "w+");

	if (file)
	{
		long block_index;
		long page_index;

		fprintf(file, "%s (v1: only blocks used this frame are locked)\n", cache->name);
		header_proc(file);
		fprintf(file, "\n#%d pages, each #%d bytes\n#%d blocks at frame index #%d\nfailed allocation of \"%s\" was #%d bytes (#%d pages)\n\n",
			cache->page_count,
			FLAG(cache->page_size_bits),
			cache->blocks->actual_count,
			cache->frame_index,
			failed_allocation_name,
			failed_allocation_size,
			lruv_cache_bytes_to_pages(cache, failed_allocation_size));

		block_index = cache->first_block_index;
		page_index = 0;

		while (page_index<cache->page_count)
		{
			unsigned long age = 0;
			char const *block_name = NULL;
			boolean locked = FALSE;
			long page_count;

			if (block_index==NONE)
			{
				page_count = cache->page_count - page_index;
				page_index = cache->page_count;
			}
			else
			{
				struct lruv_cache_block *block = lruv_cache_block_get(cache, block_index);

				if (page_index==block->first_page_index)
				{
					age = cache->frame_index - block->last_used_frame_index;
					page_count = block->page_count;
					locked = cache->locked_block_proc && cache->locked_block_proc(block_index) ? TRUE : FALSE;

					if (block->last_used_frame_index + 1>=cache->frame_index)
					{
						locked = TRUE;
					}

					page_index = block->first_page_index + block->page_count;
					block_name = name_block_proc(block_index);
					block_index = block->next_block_index;
				}
				else
				{
					page_count = block->first_page_index - page_index;
					match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 716, page_count>0);
					page_index = block->first_page_index;
				}
			}

			fprintf(file, "%s % 5d% 5d %s\n", locked ? "L" : " ", page_count, age>=0 && age<9999 ? age : 9999, block_name ? block_name : "");
		}

		fprintf(file, "\n");
		fclose(file);
	}

	return;
}

/* ---------- private code */

static long lruv_cache_bytes_to_pages(
	struct lruv_cache *cache,
	long size_in_bytes)
{
	long page_count = size_in_bytes>>cache->page_size_bits;

	if (size_in_bytes & MASK(cache->page_size_bits))
	{
		page_count++;
	}

	return page_count;
}

static void lruv_cache_verify(
	struct lruv_cache *cache,
	boolean verify_blocks)
{
	match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 754, cache);
	match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 755, cache->signature==LRUV_CACHE_SIGNATURE);
	data_verify(cache->blocks);

	if (verify_blocks)
	{
		long block_index;

		for (block_index = cache->first_block_index; block_index!=NONE; )
		{
			struct lruv_cache_block *block = lruv_cache_block_get(cache, block_index);

			if (block->previous_block_index==NONE)
			{
				match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 770, cache->first_block_index==block_index);
			}
			else
			{
				struct lruv_cache_block *previous_block = lruv_cache_block_get(cache, block->previous_block_index);

				match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 776, previous_block->next_block_index==block_index);
				match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 777, previous_block->first_page_index<block->first_page_index);
				match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 778, previous_block->first_page_index+previous_block->page_count<=block->first_page_index);
			}

			if (block->next_block_index==NONE)
			{
				match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 784, cache->last_block_index==block_index);
			}
			else
			{
				struct lruv_cache_block *next_block = lruv_cache_block_get(cache, block->next_block_index);

				match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 790, next_block->previous_block_index==block_index);
				match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 791, next_block->first_page_index>block->first_page_index);
				match_assert("c:\\halo\\SOURCE\\memory\\lruv_cache.c", 792, block->first_page_index+block->page_count<=next_block->first_page_index);
			}

			block_index = block->next_block_index;
		}
	}

	return;
}
