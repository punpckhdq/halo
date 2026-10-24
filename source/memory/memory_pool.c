/*
MEMORY_POOL.C
*/

/* ---------- headers */

#include "cseries.h"
#include "memory_pool.h"

/* ---------- constants */

enum
{
	BLOCK_HEADER_SIGNATURE = 'head',
	BLOCK_TRAILER_SIGNATURE = 'tail',
	POOL_SIGNATURE = 'pool',
};

/* ---------- structures */

struct memory_pool_block
{
	unsigned long header_signature;
	long size;
	void **reference;
	struct memory_pool_block *next_block;
	struct memory_pool_block *previous_block;
	unsigned long trailer_signature;
};

struct memory_pool
{
	unsigned long signature;
	char name[32];
	void *base_address;
	long size;
	long free_size;
	struct memory_pool_block *first_block;
	struct memory_pool_block *last_block;
};

/* ---------- prototypes */

static long memory_pool_block_compute_actual_size(long size);
static struct memory_pool_block *memory_pool_block_new(struct memory_pool *pool, long size);
static void memory_pool_verify(struct memory_pool *pool);
static struct memory_pool_block *memory_pool_block_get(struct memory_pool *pool, void **reference);

/* ---------- public code */

struct memory_pool *memory_pool_new(
	char const *name,
	long size)
{
	struct memory_pool *pool = (struct memory_pool *)match_malloc("c:\\halo\\SOURCE\\memory\\memory_pool.c", 70, memory_pool_allocation_size(size));

	if (pool)
	{
		memory_pool_initialize(pool, name, size);
	}

	return pool;
}

void memory_pool_delete(
	struct memory_pool *pool)
{
	memory_pool_verify(pool);
	memset(pool, 0, sizeof(*pool));
	match_free("c:\\halo\\SOURCE\\memory\\memory_pool.c", 85, pool);

	return;
}

long memory_pool_allocation_size(
	long size)
{
	return sizeof(struct memory_pool) + size;
}

void memory_pool_initialize(
	struct memory_pool *pool,
	char const *name,
	long size)
{
	memset(pool, 0, sizeof(*pool));
	pool->signature = POOL_SIGNATURE;
	strncpy(pool->name, name, sizeof(pool->name) - 1);
	pool->base_address = &pool[1];
	pool->size = size;
	pool->free_size = size;
	pool->first_block = NULL;
	pool->last_block = NULL;

	return;
}

boolean memory_pool_block_allocate(
	struct memory_pool *pool,
	void **reference,
	long size)
{
	struct memory_pool_block *block;
	long actual_size = memory_pool_block_compute_actual_size(size);

	memory_pool_verify(pool);
	match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 124, size>=0);

	block = memory_pool_block_new(pool, actual_size);

	if (block)
	{
		block->header_signature = BLOCK_HEADER_SIGNATURE;
		block->size = actual_size;
		block->reference = reference;
		block->next_block = NULL;
		block->previous_block = pool->last_block;
		block->trailer_signature = BLOCK_TRAILER_SIGNATURE;

		if (!pool->first_block)
		{
			pool->first_block = block;
		}

		if (pool->last_block)
		{
			pool->last_block->next_block = block;
		}

		pool->last_block = block;
		pool->free_size -= block->size;
		match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 156, pool->free_size>=0);

		*reference = &block[1];

		return TRUE;
	}

	return FALSE;
}

boolean memory_pool_block_reallocate(
	struct memory_pool *pool,
	void **reference,
	long new_size)
{
	struct memory_pool_block *block = memory_pool_block_get(pool, reference);
	long actual_new_size = memory_pool_block_compute_actual_size(new_size);
	byte *maximum_address;

	match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 174, new_size>=0);

	maximum_address = block->next_block ? (byte *)block->next_block : (byte *)pool->base_address + pool->size;

	if ((byte *)block + actual_new_size <= maximum_address)
	{
		pool->free_size += block->size - actual_new_size;
		match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 184, pool->free_size>=0 && pool->free_size<=pool->size);
		block->size = actual_new_size;

		return TRUE;
	}
	else
	{
		void *new_buffer;

		if (memory_pool_block_allocate(pool, &new_buffer, new_size))
		{
			struct memory_pool_block *new_block;

			match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 200, actual_new_size>block->size);
			memcpy(new_buffer, *reference, block->size - sizeof(*block));
			memory_pool_block_free(pool, reference);
			new_block = memory_pool_block_get(pool, &new_buffer);
			new_block->reference = reference;
			*reference = new_buffer;

			return TRUE;
		}
	}

	return FALSE;
}

void memory_pool_block_free(
	struct memory_pool *pool,
	void **reference)
{
	struct memory_pool_block *block = memory_pool_block_get(pool, reference);

	pool->free_size += block->size;
	match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 230, pool->free_size<=pool->size);

	if (block->previous_block)
	{
		block->previous_block->next_block = block->next_block;
	}
	else
	{
		pool->first_block = block->next_block;
	}

	if (block->next_block)
	{
		block->next_block->previous_block = block->previous_block;
	}
	else
	{
		pool->last_block = block->previous_block;
	}

	memset(block, 0, block->size);

	return;
}

void memory_pool_compact(
	struct memory_pool *pool)
{
	if (pool->first_block)
	{
		struct memory_pool_block *block;
		struct memory_pool_block *previous_block;
		byte *next_address;

		next_address = (byte *)pool->base_address;
		previous_block = NULL;

		for (block = pool->first_block; block; next_address = (byte *)block + block->size, previous_block = block, block = block->next_block)
		{
			if ((byte *)block > next_address)
			{
				memmove(next_address, block, block->size);
				block = (struct memory_pool_block *)next_address;
				*block->reference = &block[1];
			}

			block->previous_block = previous_block;

			if (previous_block)
			{
				previous_block->next_block = block;
			}
			else
			{
				pool->first_block = block;
			}
		}

		previous_block->next_block = NULL;
		pool->last_block = previous_block;
	}

	memory_pool_verify(pool);

	return;
}

long memory_pool_get_free_size(
	struct memory_pool *pool)
{
	return pool->free_size;
}

long memory_pool_get_used_size(
	struct memory_pool *pool)
{
	return !pool->last_block ? 0 : (uintptr_t)pool->last_block + pool->last_block->size - (uintptr_t)pool->base_address;
}

long memory_pool_get_contiguous_free_size(
	struct memory_pool *pool)
{
	return pool->size - memory_pool_get_used_size(pool);
}

/* ---------- private code */

static long memory_pool_block_compute_actual_size(
	long size)
{
	size += sizeof(struct memory_pool_block);

	if (size & 3)
	{
		size = (size | 3) + 1;
	}

	return size;
}

static struct memory_pool_block *memory_pool_block_new(
	struct memory_pool *pool,
	long size)
{
	struct memory_pool_block *block = pool->last_block ? (struct memory_pool_block *)((byte *)pool->last_block + pool->last_block->size) : (struct memory_pool_block *)pool->base_address;

	if ((byte *)block + size <= (byte *)pool->base_address + pool->size)
	{
		return block;
	}
	else
	{
		return NULL;
	}
}

static void memory_pool_verify(
	struct memory_pool *pool)
{
	struct memory_pool_block *block;
	struct memory_pool_block *previous_block;

	match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 340, pool->signature==POOL_SIGNATURE);
	match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 341, pool->size>0);

	block = pool->first_block;
	previous_block = NULL;

	while (block)
	{
		match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 352, block->previous_block==previous_block);
		match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 353, block->next_block || pool->last_block==block);

		match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 355, block->header_signature==BLOCK_HEADER_SIGNATURE);
		match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 356, block->trailer_signature==BLOCK_TRAILER_SIGNATURE);

		match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 358, (byte *)block>=(byte *)pool->base_address);
		match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 359, (byte *)block+block->size<=(byte *)pool->base_address+pool->size);

		previous_block = block;
		block = block->next_block;
	}

	return;
}

static struct memory_pool_block *memory_pool_block_get(
	struct memory_pool *pool,
	void **reference)
{
	struct memory_pool_block *block;
	struct memory_pool_block *other_block;

	match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 372, reference && (*reference));
	memory_pool_verify(pool);

	block = (struct memory_pool_block *)*reference - 1;
	match_vassert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 379, block->reference==reference, csprintf(temporary, "expected reference %08x but got %08x", block->reference, reference));

	for (other_block = pool->first_block; other_block && block!=other_block; other_block = other_block->next_block);
	match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 388, other_block);

	return block;
}
