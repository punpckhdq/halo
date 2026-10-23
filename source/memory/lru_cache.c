/*
LRU_CACHE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "lru_cache.h"

/* ---------- constants */

enum
{
	LRU_CACHE_SIGNATURE = 'lruc', /* fake name */
	LRU_BLOCK_SIGNATURE = 'Ublj', /* fake name */
};

enum
{
	_lru_block_locked_bit = 0, /* fake name */
};

/* ---------- structures */

struct lru_block /* fake name */
{
	void **reference; /* fake name */
	unsigned long signature; /* fake name */
	unsigned long age; /* fake name */
	unsigned long unused; /* fake name */
};

struct lru_cache
{
	char name[32];
	long block_count; /* fake name */
	long block_size; /* fake name */
	long total_size;
	lru_new_block_proc new_block_proc; /* fake name */
	lru_purge_block_proc purge_block_proc; /* fake name */
	byte *blocks; /* fake name */
	boolean allocated_blocks; /* fake name */
	unsigned long age; /* fake name */
	long used_block_count; /* fake name */
	unsigned long signature; /* fake name */
};

/* ---------- prototypes */

static void lru_default_new_block_proc(void **reference, void *data);
static void lru_default_purge_block_proc(void **reference);
static long get_lru_cache_block_offset(struct lru_cache *cache, struct lru_block *block);
static void verify_lru_cache_block(struct lru_cache *cache, struct lru_block *block);
static void verify_lru_cache(struct lru_cache *cache);

/* ---------- public code */

struct lru_cache *lru_new(
	char const *name,
	long total_size,
	long block_size,
	lru_new_block_proc new_block_proc,
	lru_purge_block_proc purge_block_proc,
	void *buffer)
{
	struct lru_cache *cache;
	long block_count;

	match_assert("c:\\halo\\SOURCE\\memory\\lru_cache.c", 94, block_size>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\lru_cache.c", 95, total_size>=block_size);

	if (!new_block_proc || !purge_block_proc)
	{
		new_block_proc = lru_default_new_block_proc;
		purge_block_proc = lru_default_purge_block_proc;
	}

	block_size += sizeof(struct lru_block);

	if (block_size & 3)
	{
		block_size = (block_size | 3) + 1;
	}

	block_count = total_size / block_size;
	cache = (struct lru_cache *)match_malloc("c:\\halo\\SOURCE\\memory\\lru_cache.c", 110, sizeof(struct lru_cache));

	if (cache)
	{
		boolean allocated_blocks = FALSE;

		if (!buffer)
		{
			buffer = match_malloc("c:\\halo\\SOURCE\\memory\\lru_cache.c", 117, block_count*block_size);
			allocated_blocks = TRUE;
		}

		if (buffer)
		{
			memset(cache, 0, sizeof(struct lru_cache));
			cache->block_count = block_count;
			cache->block_size = block_size;
			cache->total_size = block_size*block_count;
			cache->signature = LRU_CACHE_SIGNATURE;
			cache->blocks = (byte *)buffer;
			cache->new_block_proc = new_block_proc;
			cache->age = 0;
			cache->used_block_count = 0;
			cache->purge_block_proc = purge_block_proc;
			cache->allocated_blocks = allocated_blocks;
			strncpy(cache->name, name, sizeof(cache->name) - 1);
			cache->name[sizeof(cache->name) - 1] = 0;
			verify_lru_cache(cache);
		}
		else
		{
			match_free("c:\\halo\\SOURCE\\memory\\lru_cache.c", 142, cache);
			cache = NULL;
		}
	}

	return cache;
}

void lru_dispose(
	struct lru_cache *cache)
{
	verify_lru_cache(cache);

	if (cache->allocated_blocks)
	{
		match_free("c:\\halo\\SOURCE\\memory\\lru_cache.c", 156, cache->blocks);
	}

	match_free("c:\\halo\\SOURCE\\memory\\lru_cache.c", 157, cache);

	return;
}

void lru_flush(
	struct lru_cache *cache)
{
	long block_index;
	byte *block;

	verify_lru_cache(cache);

	for (block_index = 0, block = cache->blocks; block_index<cache->used_block_count; block_index++, block += cache->block_size)
	{
		verify_lru_cache_block(cache, (struct lru_block *)block);
		cache->purge_block_proc(((struct lru_block *)block)->reference);
	}

	cache->used_block_count = 0;

	return;
}

long lru_free_blocks(
	struct lru_cache *cache)
{
	verify_lru_cache(cache);

	return cache->block_count - cache->used_block_count;
}

void *lru_allocate(
	struct lru_cache *cache,
	void **reference)
{
	struct lru_block *block = NULL;
	void *result = NULL;

	verify_lru_cache(cache);

	if (cache->used_block_count==cache->block_count)
	{
		unsigned long oldest_age;
		long block_index;
		struct lru_block *current_block;

		for (block_index = 0, current_block = (struct lru_block *)cache->blocks; block_index<cache->used_block_count; block_index++, current_block = (struct lru_block *)((byte *)current_block + cache->block_size))
		{
			verify_lru_cache_block(cache, current_block);

			if (!TEST_FLAG(current_block->signature, _lru_block_locked_bit) && (!block || oldest_age>current_block->age))
			{
				block = current_block;
				oldest_age = current_block->age;
			}
		}

		if (block)
		{
			cache->purge_block_proc(block->reference);
		}
	}
	else
	{
		block = (struct lru_block *)(cache->blocks + cache->used_block_count*cache->block_size);
		cache->used_block_count++;
	}

	if (block)
	{
		block->signature = LRU_BLOCK_SIGNATURE;
		block->reference = reference;
		block->age = cache->age++;
		block->unused = 0;
		result = block + 1;
		cache->new_block_proc(block->reference, result);
	}

	return result;
}

void lru_lock(
	struct lru_cache *cache,
	void *data)
{
	struct lru_block *block = (struct lru_block *)data - 1;

	verify_lru_cache(cache);
	verify_lru_cache_block(cache, block);
	SET_FLAG(block->signature, _lru_block_locked_bit, TRUE);

	return;
}

void lru_unlock(
	struct lru_cache *cache,
	void *data)
{
	struct lru_block *block = (struct lru_block *)data - 1;

	verify_lru_cache(cache);
	verify_lru_cache_block(cache, block);
	SET_FLAG(block->signature, _lru_block_locked_bit, FALSE);

	return;
}

void lru_touch(
	struct lru_cache *cache,
	void *data)
{
	struct lru_block *block = (struct lru_block *)data - 1;

	verify_lru_cache(cache);
	verify_lru_cache_block(cache, block);
	block->age = cache->age++;

	return;
}

/* ---------- private code */

static void lru_default_new_block_proc(
	void **reference,
	void *data)
{
	*reference = data;

	return;
}

static void lru_default_purge_block_proc(
	void **reference)
{
	*reference = NULL;

	return;
}

static long get_lru_cache_block_offset(
	struct lru_cache *cache,
	struct lru_block *block)
{
	verify_lru_cache_block(cache, block);

	return (byte *)block - cache->blocks;
}

static void verify_lru_cache_block(
	struct lru_cache *cache,
	struct lru_block *block)
{
	boolean valid = FALSE;

	if ((block->signature & ~FLAG(_lru_block_locked_bit))==LRU_BLOCK_SIGNATURE && !block->unused)
	{
		long offset = get_lru_cache_block_offset(cache, block);

		if (offset>=0 && offset + cache->block_size<=cache->total_size && block->age<cache->age)
		{
			valid = TRUE;
		}
	}

	match_vassert("c:\\halo\\SOURCE\\memory\\lru_cache.c", 342, valid, csprintf(temporary, "lru cache %s @%p block @%p appears to be corrupt", cache->name, cache, block));

	return;
}

static void verify_lru_cache(
	struct lru_cache *cache)
{
	boolean valid = FALSE;

	if (cache->signature==LRU_CACHE_SIGNATURE &&
		cache->blocks &&
		cache->new_block_proc &&
		cache->purge_block_proc &&
		cache->total_size==cache->block_count*cache->block_size &&
		cache->block_size>=sizeof(struct lru_block) &&
		cache->block_count>=0 &&
		cache->used_block_count>=0 &&
		cache->used_block_count<=cache->block_count)
	{
		valid = TRUE;
	}

	match_vassert("c:\\halo\\SOURCE\\memory\\lru_cache.c", 363, valid, csprintf(temporary, "lru cache %s @%p appears to be corrupt", cache->name, cache));

	return;
}
