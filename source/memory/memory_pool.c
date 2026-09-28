/*
MEMORY_POOL.C

symbols in this file:
0010DB50 0010:
	_memory_pool_allocation_size (0000)
0010DB60 0050:
	_memory_pool_initialize (0000)
0010DBB0 0010:
	_memory_pool_get_free_size (0000)
0010DBC0 0020:
	_memory_pool_get_used_size (0000)
0010DBE0 0030:
	_memory_pool_get_contiguous_free_size (0000)
0010DC10 0010:
	_code_0010dc10 (0000)
0010DC20 0030:
	_code_0010dc20 (0000)
0010DC50 0170:
	_code_0010dc50 (0000)
0010DDC0 00b0:
	_code_0010ddc0 (0000)
0010DE70 0040:
	_memory_pool_new (0000)
0010DEB0 0030:
	_memory_pool_delete (0000)
0010DEE0 00e0:
	_memory_pool_block_allocate (0000)
0010DFC0 00a0:
	_memory_pool_block_free (0000)
0010E060 0060:
	_memory_pool_compact (0000)
0010E0C0 0120:
	_memory_pool_block_reallocate (0000)
0027E458 0041:
	??_C@_0EB@NADHALE@?$CIbyte?5?$CK?$CJblock?$CLblock?9?$DOsize?$DM?$DN?$CIbyte@ (0000)
0027E49C 002a:
	??_C@_0CK@LLMKELFI@?$CIbyte?5?$CK?$CJblock?$DO?$DN?$CIbyte?5?$CK?$CJpool?9?$DObas@ (0000)
0027E4C8 0032:
	??_C@_0DC@HIOFDAGC@block?9?$DOtrailer_signature?$DN?$DNBLOCK_@ (0000)
0027E4FC 0030:
	??_C@_0DA@IHOELNEF@block?9?$DOheader_signature?$DN?$DNBLOCK_H@ (0000)
0027E52C 002d:
	??_C@_0CN@BLDKBJCH@block?9?$DOnext_block?5?$HM?$HM?5pool?9?$DOlast_@ (0000)
0027E55C 0026:
	??_C@_0CG@CJPANJAP@block?9?$DOprevious_block?$DN?$DNprevious_@ (0000)
0027E584 000d:
	??_C@_0N@EHOFABCJ@pool?9?$DOsize?$DO0?$AA@ (0000)
0027E594 0020:
	??_C@_0CA@HGIACAOK@pool?9?$DOsignature?$DN?$DNPOOL_SIGNATURE?$AA@ (0000)
0027E5B4 0024:
	??_C@_0CE@POLAKNJH@c?3?2halo?2SOURCE?2memory?2memory_poo@ (0000)
0027E5D8 000c:
	??_C@_0M@MJBIHABJ@other_block?$AA@ (0000)
0027E5E4 0025:
	??_C@_0CF@IJNEMAOG@expected?5reference?5?$CF08x?5but?5got?5@ (0000)
0027E60C 001a:
	??_C@_0BK@MFLHGBFC@reference?5?$CG?$CG?5?$CI?$CKreference?$CJ?$AA@ (0000)
0027E628 0013:
	??_C@_0BD@IKFFHLNA@pool?9?$DOfree_size?$DO?$DN0?$AA@ (0000)
0027E63C 001c:
	??_C@_0BM@JADHFLPK@pool?9?$DOfree_size?$DM?$DNpool?9?$DOsize?$AA@ (0000)
0027E658 001c:
	??_C@_0BM@HDOIAFHN@actual_new_size?$DOblock?9?$DOsize?$AA@ (0000)
0027E674 0032:
	??_C@_0DC@FAPCJHLD@pool?9?$DOfree_size?$DO?$DN0?5?$CG?$CG?5pool?9?$DOfree@ (0000)
0027E6A8 000c:
	??_C@_0M@PJCPCKMH@new_size?$DO?$DN0?$AA@ (0000)
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

enum
{
	_pointer_locked_bit = -2147483648,
	_pointer_size_mask = 2147483647,
	MAXIMUM_BLOCK_SIZE = 2147483647,
	ALIGNMENT_BYTE_BOUNDARY = 4,
};

enum
{
	INTERNAL_POINTER_OVERHEAD = 16,
};

/* ---------- macros */

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

struct memory_block
{
	unsigned long bits;
	unsigned long index;
	struct memory_block *previous_block;
	struct memory_block *next_block;
};

/* ---------- prototypes */

static void memory_pool_verify(struct memory_pool *pool);
static struct memory_pool_block* memory_pool_block_get(struct memory_pool *pool, void **reference);

/* ---------- globals */

/* ---------- public code */

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

struct memory_pool *memory_pool_new(
	char const *name,
	long size)
{
	struct memory_pool* pool = (struct memory_pool *)match_malloc("c:\\halo\\SOURCE\\memory\\memory_pool.c", 70, memory_pool_allocation_size(size));

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
}

boolean memory_pool_block_allocate(
	struct memory_pool *pool,
	void **reference,
	long size)
{
	boolean result = FALSE;
	struct memory_pool_block *base_address;
	long new_size;

	new_size = size + sizeof(struct memory_pool_block);
	// round up macro?
	if ( ((size + sizeof(struct memory_pool_block)) & 3) != 0 )
	{
		new_size = (new_size | 3) + 1;
	}

	memory_pool_verify(pool);
	match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 124, size>=0);

	if (pool->last_block)
	{
		base_address = (struct memory_pool_block *)((byte *)pool->last_block + pool->last_block->size);
	}
	else
	{
		base_address = (struct memory_pool_block *)pool->base_address;
	}

	if ((byte *)base_address + new_size <= (byte *)pool->base_address + pool->size && base_address)
	{
		base_address->size = new_size;
		base_address->header_signature = BLOCK_HEADER_SIGNATURE;
		base_address->reference = reference;
		base_address->next_block = NULL;
		base_address->previous_block = pool->last_block;
		base_address->trailer_signature = BLOCK_TRAILER_SIGNATURE;

		if ( !pool->first_block )
		{
			pool->first_block = base_address;
		}
		
		if ( pool->last_block )
		{
			pool->last_block->next_block = base_address;
		}

		pool->last_block = base_address;
		pool->free_size -= base_address->size;

		match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 156, pool->free_size>=0);

		*reference = &base_address[1];
		result = TRUE;
	}

	return result;
}

void memory_pool_block_free(
	struct memory_pool *pool,
	void **reference)
{
	struct memory_pool_block* block = memory_pool_block_get(pool, reference);

	match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 230, pool->free_size<=pool->size);
}

void memory_pool_compact(
	struct memory_pool *pool)
{
	memory_pool_verify(pool);
}

boolean memory_pool_block_reallocate(
	struct memory_pool *pool,
	void **reference,
	long new_size)
{
	memory_pool_block_get(pool, reference);
}

/* ---------- private code */

static void memory_pool_verify(
	struct memory_pool *pool)
{
	struct memory_pool_block* block;
	struct memory_pool_block* previous_block;

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
}

static struct memory_pool_block* memory_pool_block_get(
	struct memory_pool *pool,
	void **reference)
{
	void* other_reference;
	struct memory_pool_block* block;
	struct memory_pool_block* other_block;

	match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 372, reference && (*reference));
	memory_pool_verify(pool);

	other_reference = *(void **)((byte *)*reference - INTERNAL_POINTER_OVERHEAD);
	match_vassert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 379, other_reference==reference, csprintf(temporary, "expected reference %08x but got %08x", other_reference, reference));

	block = (*(struct memory_pool_block**)reference) - 1;
	other_block = block;

	do
	{
		other_block = block->next_block;
		match_assert("c:\\halo\\SOURCE\\memory\\memory_pool.c", 388, other_block);
	} while (block != other_block);

	return block;
}
