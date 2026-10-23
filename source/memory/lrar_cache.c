/*
LRAR_CACHE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "lrar_cache.h"

/* ---------- constants */

enum
{
	LRAR_CACHE_SIGNATURE = 'lrar', /* fake name */
	LRAR_BLOCK_SIGNATURE = 'Rblk', /* fake name */
};

/* ---------- structures */

struct lrar_block /* fake name */
{
	short *reference; /* fake name */
	unsigned long signature; /* fake name */
	unsigned long address; /* fake name */
	long size; /* fake name */
};

struct lrar_cache
{
	char name[32];
	short alignment_bit;
	short boundary_bit;
	unsigned long minimum_address;
	unsigned long maximum_address;
	long size; /* fake name */
	struct lrar_block *blocks; /* fake name */
	short first_block_index; /* fake name */
	short last_block_index; /* fake name */
	short block_count;
	lrar_new_block_proc new_block_proc; /* fake name */
	lrar_purge_block_proc purge_block_proc; /* fake name */
	unsigned long signature; /* fake name */
};

/* ---------- prototypes */

static void lrar_default_new_block_proc(short *reference, short block_index);
static void lrar_default_purge_block_proc(short *reference);
static void lrar_purge_block(struct lrar_cache *cache, struct lrar_block *block);
static struct lrar_block *get_lrar_cache_block(struct lrar_cache *cache, short block_index);
static void verify_lrar_cache_block(struct lrar_cache *cache, struct lrar_block *block);
static void verify_lrar_cache(struct lrar_cache *cache);

/* ---------- public code */

struct lrar_cache *lrar_new(
	char const *name,
	unsigned long minimum_address,
	unsigned long maximum_address,
	short block_count,
	short alignment_bit,
	short boundary_bit,
	lrar_new_block_proc new_block_proc,
	lrar_purge_block_proc purge_block_proc)
{
	struct lrar_cache *cache = (struct lrar_cache *)match_malloc("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 86, sizeof(struct lrar_cache));

	match_assert("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 88, minimum_address<maximum_address);

	if (minimum_address & MASK(alignment_bit))
	{
		minimum_address = (minimum_address | MASK(alignment_bit)) + 1;
	}

	if (!new_block_proc || !purge_block_proc)
	{
		new_block_proc = lrar_default_new_block_proc;
		purge_block_proc = lrar_default_purge_block_proc;
	}

	match_assert("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 102, alignment_bit>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 103, boundary_bit==NONE || boundary_bit>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 104, block_count>0);

	if (cache)
	{
		struct lrar_block *blocks = (struct lrar_block *)match_malloc("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 108, block_count*sizeof(struct lrar_block));

		if (blocks)
		{
			memset(cache, 0, sizeof(struct lrar_cache));
			memset(blocks, 0, block_count*sizeof(struct lrar_block));
			strncpy(cache->name, name, sizeof(cache->name) - 1);
			cache->name[sizeof(cache->name) - 1] = 0;
			cache->alignment_bit = alignment_bit;
			cache->boundary_bit = boundary_bit;
			cache->minimum_address = minimum_address;
			cache->maximum_address = maximum_address;
			cache->size = maximum_address - minimum_address;
			cache->blocks = blocks;
			cache->first_block_index = NONE;
			cache->last_block_index = NONE;
			cache->block_count = block_count;
			cache->new_block_proc = new_block_proc;
			cache->purge_block_proc = purge_block_proc;
			cache->signature = LRAR_CACHE_SIGNATURE;
			verify_lrar_cache(cache);
		}
		else
		{
			match_free("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 138, cache);
			return NULL;
		}
	}

	return cache;
}

void lrar_dispose(
	struct lrar_cache *cache)
{
	verify_lrar_cache(cache);
	match_free("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 152, cache->blocks);
	match_free("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 153, cache);

	return;
}

void lrar_flush(
	struct lrar_cache *cache)
{
	short block_index;

	verify_lrar_cache(cache);

	block_index = cache->first_block_index;
	while (block_index!=NONE)
	{
		lrar_purge_block(cache, get_lrar_cache_block(cache, block_index));
		if (block_index==cache->last_block_index)
		{
			block_index = NONE;
		}
		else if (++block_index==cache->block_count)
		{
			block_index = 0;
		}
	}

	cache->first_block_index = NONE;
	cache->last_block_index = NONE;

	return;
}

short lrar_allocate(
	struct lrar_cache *cache,
	long size,
	short *reference)
{
	short block_index = NONE;

	verify_lrar_cache(cache);

	if (size & MASK(cache->alignment_bit))
	{
		size = (size | MASK(cache->alignment_bit)) + 1;
	}

	if (size>=0 && size<=cache->size)
	{
		short first_block_index = cache->first_block_index;
		short new_block_index = cache->last_block_index==NONE ? 0 : cache->last_block_index + 1;

		if (new_block_index>=cache->block_count)
		{
			new_block_index = 0;
		}

		for (;;)
		{
			unsigned long new_block_address;
			unsigned long adjusted_new_block_address;

			if (cache->last_block_index==NONE)
			{
				new_block_address = cache->minimum_address;
			}
			else
			{
				struct lrar_block *last_block = get_lrar_cache_block(cache, cache->last_block_index);

				new_block_address = last_block->address + last_block->size;
			}

			adjusted_new_block_address = new_block_address;

			if (cache->boundary_bit!=NONE)
			{
				unsigned long boundary_mask = ~MASK(cache->boundary_bit);
				unsigned long start_boundary = new_block_address & boundary_mask;

				if (start_boundary!=((new_block_address + size) & boundary_mask))
				{
					adjusted_new_block_address = start_boundary + FLAG(cache->boundary_bit);
				}
			}

			if (first_block_index!=NONE)
			{
				struct lrar_block *block;

				block = get_lrar_cache_block(cache, first_block_index);
				while (first_block_index==new_block_index || (new_block_address<=block->address && adjusted_new_block_address + size>block->address))
				{
					lrar_purge_block(cache, block);
					block->signature = NONE;
					if (++first_block_index>=cache->block_count)
					{
						first_block_index = 0;
					}

					block = get_lrar_cache_block(cache, first_block_index);
				}
			}

			if (adjusted_new_block_address + size>cache->maximum_address)
			{
				cache->last_block_index = NONE;
			}
			else
			{
				struct lrar_block *block = &cache->blocks[new_block_index];
				short test_block_index;

				match_assert("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 273, adjusted_new_block_address>=cache->minimum_address && adjusted_new_block_address+size<=cache->maximum_address);
				for (test_block_index = 0; test_block_index<cache->block_count; test_block_index++)
				{
					struct lrar_block *test_block = &cache->blocks[test_block_index];

					if (test_block->signature==LRAR_BLOCK_SIGNATURE)
					{
						match_assert("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 284, adjusted_new_block_address>=test_block->address+test_block->size || adjusted_new_block_address+size<=test_block->address);
					}
				}

				block->size = size;
				block->signature = LRAR_BLOCK_SIGNATURE;
				block->address = adjusted_new_block_address;
				block->reference = reference;
				cache->new_block_proc(reference, new_block_index);
				cache->last_block_index = new_block_index;
				if (first_block_index==NONE)
				{
					first_block_index = new_block_index;
				}
				cache->first_block_index = first_block_index;
				block_index = new_block_index;
				break;
			}
		}
	}

	return block_index;
}

unsigned long lrar_block_address(
	struct lrar_cache *cache,
	short block_index)
{
	return get_lrar_cache_block(cache, block_index)->address;
}

void lrar_deallocate(
	struct lrar_cache *cache,
	short block_index)
{
	verify_lrar_cache(cache);
	lrar_purge_block(cache, get_lrar_cache_block(cache, block_index));

	return;
}

/* ---------- private code */

static void lrar_default_new_block_proc(
	short *reference,
	short block_index)
{
	*reference = block_index;

	return;
}

static void lrar_default_purge_block_proc(
	short *reference)
{
	*reference = NONE;

	return;
}

static void lrar_purge_block(
	struct lrar_cache *cache,
	struct lrar_block *block)
{
	if (block->reference)
	{
		cache->purge_block_proc(block->reference);
		block->reference = NULL;
	}

	return;
}

static struct lrar_block *get_lrar_cache_block(
	struct lrar_cache *cache,
	short block_index)
{
	struct lrar_block *block;

	verify_lrar_cache(cache);
	match_assert("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 366, block_index>=0 && block_index<cache->block_count);

	block = &cache->blocks[block_index];
	verify_lrar_cache_block(cache, block);

	return block;
}

static void verify_lrar_cache_block(
	struct lrar_cache *cache,
	struct lrar_block *block)
{
	match_vassert("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 390,
		block->signature==LRAR_BLOCK_SIGNATURE &&
		block->size>=0 && block->size<cache->size &&
		block->address>=cache->minimum_address && block->address + block->size<=cache->maximum_address,
		csprintf(temporary, "lrar cache %s @%p block @%p appears to be corrupt", cache->name, cache, block));

	return;
}

static void verify_lrar_cache(
	struct lrar_cache *cache)
{
	match_vassert("c:\\halo\\SOURCE\\memory\\lrar_cache.c", 409,
		cache->signature==LRAR_CACHE_SIGNATURE &&
		cache->minimum_address<cache->maximum_address &&
		cache->size>0 &&
		cache->block_count>0,
		csprintf(temporary, "lrar cache %s @%p appears to be corrupt", cache->name, cache));

	return;
}
