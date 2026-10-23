/*
LRA_CACHE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "lra_cache.h"

/* ---------- constants */

enum
{
	LRA_CACHE_SIGNATURE = 'lrac', /* fake name */
	LRA_BLOCK_SIGNATURE = 'Ablh', /* fake name */
};

enum
{
	_lra_block_locked_bit = 0, /* fake name */
	_lra_block_purged_bit, /* fake name */
	NUMBER_OF_LRA_BLOCK_FLAGS /* fake name */
};

/* ---------- structures */

struct lra_block /* fake name */
{
	void **reference; /* fake name */
	unsigned long signature; /* fake name */
	long size; /* fake name */
	struct lra_block *next_block; /* fake name */
};

struct lra_cache
{
	char name[32];
	long size;
	byte *base_address;
	boolean allocated_base_address; /* fake name */
	struct lra_block *last_block; /* fake name */
	lra_new_block_proc new_block_proc; /* fake name */
	lra_purge_block_proc purge_block_proc; /* fake name */
	unsigned long signature; /* fake name */
};

/* ---------- prototypes */

static void lra_default_new_block_proc(void **reference, void *data);
static void lra_default_purge_block_proc(void **reference);
static void lra_purge_block(struct lra_cache *cache, struct lra_block *block);
static long get_cache_block_offset(struct lra_cache *cache, struct lra_block *block);
static void verify_lra_cache_block(struct lra_cache *cache, struct lra_block *block);
static void verify_lra_cache(struct lra_cache *cache);

/* ---------- public code */

struct lra_cache *lra_new(
	char *name,
	long size,
	lra_new_block_proc new_block_proc,
	lra_purge_block_proc purge_block_proc,
	void *base_address)
{
	struct lra_cache *cache = (struct lra_cache *)match_malloc("c:\\halo\\SOURCE\\memory\\lra_cache.c", 86, sizeof(struct lra_cache));

	match_assert("c:\\halo\\SOURCE\\memory\\lra_cache.c", 88, size>=0);

	if (!new_block_proc || !purge_block_proc)
	{
		new_block_proc = lra_default_new_block_proc;
		purge_block_proc = lra_default_purge_block_proc;
	}

	if (cache)
	{
		boolean allocated_base_address = FALSE;

		if (!base_address)
		{
			base_address = match_malloc("c:\\halo\\SOURCE\\memory\\lra_cache.c", 102, size);
			allocated_base_address = TRUE;
		}

		if (base_address)
		{
			match_assert("c:\\halo\\SOURCE\\memory\\lra_cache.c", 107, !((long)base_address&3));

			memset(cache, 0, sizeof(struct lra_cache));
			strncpy(cache->name, name, sizeof(cache->name) - 1);
			cache->name[sizeof(cache->name) - 1] = 0;
			cache->size = size;
			cache->base_address = (byte *)base_address;
			cache->last_block = NULL;
			cache->signature = LRA_CACHE_SIGNATURE;
			cache->allocated_base_address = allocated_base_address;
			cache->purge_block_proc = purge_block_proc;
			cache->new_block_proc = new_block_proc;
			verify_lra_cache(cache);
		}
		else
		{
			match_free("c:\\halo\\SOURCE\\memory\\lra_cache.c", 126, cache);
			cache = NULL;
		}
	}

	return cache;
}

void lra_dispose(
	struct lra_cache *cache)
{
	verify_lra_cache(cache);

	if (cache->allocated_base_address)
	{
		match_free("c:\\halo\\SOURCE\\memory\\lra_cache.c", 140, cache->base_address);
	}

	match_free("c:\\halo\\SOURCE\\memory\\lra_cache.c", 141, cache);

	return;
}

void lra_flush(
	struct lra_cache *cache)
{
	verify_lra_cache(cache);

	if (cache->last_block)
	{
		struct lra_block *block;

		for (block = (struct lra_block *)cache->base_address; block; block = block->next_block)
		{
			lra_purge_block(cache, block);
		}
	}

	cache->last_block = NULL;

	return;
}

boolean lra_full(
	struct lra_cache *cache)
{
	return cache->last_block && cache->last_block->next_block;
}

void *lra_allocate(
	struct lra_cache *cache,
	long size,
	void **reference)
{
	void *result = NULL;

	verify_lra_cache(cache);

	size += sizeof(struct lra_block);

	if (size & 3)
	{
		size = (size | 3) + 1;
	}

	if (size>=0 && size<=cache->size)
	{
		struct lra_block *previous_block = cache->last_block;
		struct lra_block *next_block = previous_block ? previous_block->next_block : NULL;
		struct lra_block *first_purge_block = NULL;
		short wrap_count = 0;

		do
		{
			long free_offset = previous_block ? get_cache_block_offset(cache, previous_block) + previous_block->size : 0;

			if (next_block)
			{
				verify_lra_cache_block(cache, next_block);
			}

			if (next_block && free_offset + size>get_cache_block_offset(cache, next_block))
			{
				if (TEST_FLAG(next_block->signature, _lra_block_locked_bit))
				{
					first_purge_block = NULL;
					previous_block = next_block;
				}
				else if (!first_purge_block)
				{
					first_purge_block = next_block;
				}

				next_block = next_block->next_block;
			}
			else if (free_offset + size>cache->size)
			{
				next_block = (struct lra_block *)cache->base_address;
				first_purge_block = previous_block = NULL;

				if (wrap_count++)
				{
					break;
				}
			}
			else
			{
				struct lra_block *purge_block;
				struct lra_block *block;

				for (purge_block = first_purge_block; purge_block && purge_block!=next_block; purge_block = purge_block->next_block)
				{
					lra_purge_block(cache, purge_block);
				}

				block = (struct lra_block *)(cache->base_address + free_offset);
				block->signature = LRA_BLOCK_SIGNATURE;
				block->size = size;
				block->reference = reference;
				block->next_block = next_block;
				result = block + 1;
				cache->new_block_proc(reference, result);

				if (previous_block)
				{
					previous_block->next_block = block;
				}

				cache->last_block = block;
			}
		}
		while (!result);
	}

	return result;
}

void lra_free(
	struct lra_cache *cache,
	void *pointer)
{
	struct lra_block *block = (struct lra_block *)pointer - 1;

	match_assert("c:\\halo\\SOURCE\\memory\\lra_cache.c", 282, pointer);

	verify_lra_cache(cache);
	verify_lra_cache_block(cache, block);
	lra_purge_block(cache, block);

	return;
}

void lra_lock(
	struct lra_cache *cache,
	void *pointer)
{
	struct lra_block *block = (struct lra_block *)pointer - 1;

	match_assert("c:\\halo\\SOURCE\\memory\\lra_cache.c", 298, pointer);

	verify_lra_cache(cache);
	verify_lra_cache_block(cache, block);
	SET_FLAG(block->signature, _lra_block_locked_bit, TRUE);

	return;
}

void lra_unlock(
	struct lra_cache *cache,
	void *pointer)
{
	struct lra_block *block = (struct lra_block *)pointer - 1;

	match_assert("c:\\halo\\SOURCE\\memory\\lra_cache.c", 314, pointer);

	verify_lra_cache(cache);
	verify_lra_cache_block(cache, block);
	SET_FLAG(block->signature, _lra_block_locked_bit, FALSE);

	return;
}

/* ---------- private code */

static void lra_default_new_block_proc(
	void **reference,
	void *data)
{
	*reference = data;

	return;
}

static void lra_default_purge_block_proc(
	void **reference)
{
	*reference = NULL;

	return;
}

static void lra_purge_block(
	struct lra_cache *cache,
	struct lra_block *block)
{
	if (!TEST_FLAG(block->signature, _lra_block_purged_bit))
	{
		cache->purge_block_proc(block->reference);
		SET_FLAG(block->signature, _lra_block_locked_bit, FALSE);
		SET_FLAG(block->signature, _lra_block_purged_bit, TRUE);
	}

	return;
}

static long get_cache_block_offset(
	struct lra_cache *cache,
	struct lra_block *block)
{
	verify_lra_cache_block(cache, block);

	return (byte *)block - cache->base_address;
}

static void verify_lra_cache_block(
	struct lra_cache *cache,
	struct lra_block *block)
{
	boolean valid = FALSE;

	if ((block->signature & ~MASK(NUMBER_OF_LRA_BLOCK_FLAGS))==LRA_BLOCK_SIGNATURE && block->size>=0 && block->size<cache->size)
	{
		long offset = (byte *)block - cache->base_address;

		if (offset>=0 && offset + block->size<=cache->size)
		{
			long next_offset = block->next_block ? (byte *)block->next_block - cache->base_address : 0;

			if (next_offset>=0 && next_offset + sizeof(struct lra_block)<=cache->size)
			{
				valid = TRUE;
			}
		}
	}

	match_vassert("c:\\halo\\SOURCE\\memory\\lra_cache.c", 398, valid, csprintf(temporary, "lra cache %s @%p block @%p appears to be corrupt", cache->name, cache, block));

	return;
}

static void verify_lra_cache(
	struct lra_cache *cache)
{
	boolean valid = FALSE;

	match_assert("c:\\halo\\SOURCE\\memory\\lra_cache.c", 408, cache);

	if (cache->signature==LRA_CACHE_SIGNATURE && cache->base_address && cache->size>=0)
	{
		valid = TRUE;
	}

	match_vassert("c:\\halo\\SOURCE\\memory\\lra_cache.c", 418, valid, csprintf(temporary, "lra cache %s @%p appears to be corrupt", cache->name, cache));

	if (cache->last_block)
	{
		verify_lra_cache_block(cache, cache->last_block);
	}

	return;
}
