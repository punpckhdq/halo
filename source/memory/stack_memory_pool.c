/*
STACK_MEMORY_POOL.C
*/

/* ---------- headers */

#include "cseries.h"
#include "stack_memory_pool.h"

/* ---------- constants */

enum
{
	_pointer_locked_bit = -2147483648,
	_pointer_size_mask = 2147483647,
	MAXIMUM_BLOCK_SIZE = 2147483647,
	ALIGNMENT_BYTE_BOUNDARY = 4,
};

enum
{
	INTERNAL_POINTER_OVERHEAD = 32,
};

enum
{
	MEMORY_BLOCK_HEADER_SIGNATURE = 'fryd', /* fake name */
	MEMORY_BLOCK_TRAILER_SIGNATURE = 'chkn', /* fake name */
};

/* ---------- structures */

struct memory_block
{
	unsigned long bits;
	unsigned long index;
	struct memory_block *previous_block;
	struct memory_block *next_block;
	char const *file; /* fake name */
	long line; /* fake name */
	unsigned long signature; /* fake name */
};

/* ---------- prototypes */

__inline static boolean memory_block_valid(struct memory_block *block);
__inline static void memory_block_lock(struct memory_block *block);
__inline static void memory_block_unlock(struct memory_block *block);
__inline static boolean memory_block_is_locked(struct memory_block *block);
__inline static void memory_block_set_size_and_index(struct memory_block *block, unsigned long new_size, unsigned long index);
__inline static unsigned long memory_block_get_real_size(struct memory_block *block);
__inline static unsigned long memory_block_get_user_size(struct memory_block *block);
__inline static void *memory_block_get_user_address(struct memory_block *block);
__inline static unsigned long memory_block_get_pool_index(struct memory_block *block);
static struct memory_block *stack_memory_pool_new_block(struct stack_memory_pool *pool, unsigned long size, char const *file, long line);
static struct memory_block *stack_memory_pool_new_block_clear(struct stack_memory_pool *pool, unsigned long size, char const *file, long line);
static struct memory_block *stack_memory_pool_resize_block(struct stack_memory_pool *pool, struct memory_block *reference, unsigned long new_size, char const *file, long line);
static void stack_memory_pool_dispose_block(struct stack_memory_pool *pool, struct memory_block *reference);
static void *stack_memory_pool_lock_block(struct stack_memory_pool *pool, struct memory_block *reference);
static void stack_memory_pool_unlock_block(struct stack_memory_pool *pool, struct memory_block *reference);
static unsigned long stack_memory_pool_free_space_at_end_of_pool(struct stack_memory_pool *pool);
static long stack_memory_pool_find_first_unused_memory_block(struct stack_memory_pool *pool);
static void stack_memory_pool_set_next_block_index(struct stack_memory_pool *pool);
static struct memory_block *stack_memory_pool_allocate(struct stack_memory_pool *pool, unsigned long size, char const *file, long line);
static void stack_memory_pool_compact(struct stack_memory_pool *pool);
static boolean stack_memory_pool_valid_block(struct stack_memory_pool *pool, struct memory_block *block);
static void *stack_memory_pool_find_space_between_blocks(struct stack_memory_pool *pool, unsigned long size, struct memory_block **free_space_in_pool_previous);

/* ---------- public code */

handle pool_new_handle(
	struct stack_memory_pool *pool,
	unsigned long size,
	char const *file,
	long line)
{
	struct memory_block *block = stack_memory_pool_new_block(pool, size, file, line);
	handle h;

	if (block)
	{
		h = (handle)block;
		pool->statistics.bytes_allocated += memory_block_get_real_size(block);
		pool->statistics.blocks_used++;

		if (pool->statistics.bytes_allocated>pool->statistics.peak_bytes_allocated)
		{
			pool->statistics.peak_bytes_allocated = pool->statistics.bytes_allocated;
		}

		if (pool->statistics.blocks_used>pool->statistics.peak_blocks_used)
		{
			pool->statistics.peak_blocks_used = pool->statistics.blocks_used;
		}

		if (memory_block_get_real_size(block)>pool->statistics.largest_block_allocated)
		{
			pool->statistics.largest_block_allocated = memory_block_get_real_size(block);
		}
	}
	else
	{
		h = NULL;
	}

	return h;
}

handle pool_new_handle_clear(
	struct stack_memory_pool *pool,
	unsigned long size,
	char const *file,
	long line)
{
	struct memory_block *block = stack_memory_pool_new_block_clear(pool, size, file, line);
	handle h;

	if (block)
	{
		h = (handle)block;
		pool->statistics.bytes_allocated += memory_block_get_real_size(block);
		pool->statistics.blocks_used++;

		if (pool->statistics.bytes_allocated>pool->statistics.peak_bytes_allocated)
		{
			pool->statistics.peak_bytes_allocated = pool->statistics.bytes_allocated;
		}

		if (pool->statistics.blocks_used>pool->statistics.peak_blocks_used)
		{
			pool->statistics.peak_blocks_used = pool->statistics.blocks_used;
		}

		if (memory_block_get_real_size(block)>pool->statistics.largest_block_allocated)
		{
			pool->statistics.largest_block_allocated = memory_block_get_real_size(block);
		}
	}
	else
	{
		h = NULL;
	}

	return h;
}

boolean pool_resize_handle(
	struct stack_memory_pool *pool,
	handle *h,
	unsigned long new_size,
	char const *file,
	long line)
{
	boolean success;
	struct memory_block *block;
	unsigned long old_size;

	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 193, h);

	block = (struct memory_block *)*h;
	success = stack_memory_pool_valid_block(pool, block);
	match_vassert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 197, success, "not a valid handle, or trying to resize a locked handle");

	if (success)
	{
		old_size = memory_block_get_real_size(block);
		block = stack_memory_pool_resize_block(pool, block, new_size, file, line);

		if (block)
		{
			*h = (handle)block;
			pool->statistics.bytes_allocated += memory_block_get_real_size(block) - old_size;

			if (pool->statistics.bytes_allocated>pool->statistics.peak_bytes_allocated)
			{
				pool->statistics.peak_bytes_allocated = pool->statistics.bytes_allocated;
			}

			if (memory_block_get_real_size(block)>pool->statistics.largest_block_allocated)
			{
				pool->statistics.largest_block_allocated = memory_block_get_real_size(block);
			}
		}
		else
		{
			success = FALSE;
		}
	}

	return success;
}

void dispose_handle(
	struct stack_memory_pool *pool,
	handle h)
{
	struct memory_block *block = (struct memory_block *)h;
	unsigned long size;

	match_vassert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 232, stack_memory_pool_valid_block(pool, block), "not a valid handle, or handle is locked");

	size = memory_block_get_real_size(block);
	stack_memory_pool_dispose_block(pool, block);
	pool->statistics.bytes_allocated -= size;
	pool->statistics.blocks_used--;

	return;
}

void lock_handle(
	struct stack_memory_pool *pool,
	handle *h)
{
	struct memory_block *block;

	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 247, h);

	block = (struct memory_block *)*h;
	match_vassert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 250, stack_memory_pool_valid_block(pool, block), "not a valid handle, or handle is already locked");
	*h = (handle)stack_memory_pool_lock_block(pool, block);

	return;
}

void unlock_handle(
	struct stack_memory_pool *pool,
	handle h)
{
	struct memory_block *block = NULL;
	unsigned long index;

	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 263, h);

	for (index = 0; index<pool->number_of_blocks; index++)
	{
		if (pool->blocks[index] && memory_block_get_user_address(pool->blocks[index])==(void *)h)
		{
			block = pool->blocks[index];
			break;
		}
	}

	match_vassert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 273, block, "invalid handle, or handle was not locked");
	stack_memory_pool_unlock_block(pool, block);

	return;
}

void *pool_new_pointer(
	struct stack_memory_pool *pool,
	unsigned long size,
	char const *file,
	long line)
{
	void *p = NULL;
	struct memory_block *block = stack_memory_pool_new_block(pool, size, file, line);

	if (block)
	{
		stack_memory_pool_lock_block(pool, block);
		p = memory_block_get_user_address(block);
		pool->statistics.bytes_allocated += memory_block_get_real_size(block);
		pool->statistics.blocks_used++;

		if (pool->statistics.bytes_allocated>pool->statistics.peak_bytes_allocated)
		{
			pool->statistics.peak_bytes_allocated = pool->statistics.bytes_allocated;
		}

		if (pool->statistics.blocks_used>pool->statistics.peak_blocks_used)
		{
			pool->statistics.peak_blocks_used = pool->statistics.blocks_used;
		}

		if (memory_block_get_real_size(block)>pool->statistics.largest_block_allocated)
		{
			pool->statistics.largest_block_allocated = memory_block_get_real_size(block);
		}
	}

	return p;
}

void *pool_new_pointer_clear(
	struct stack_memory_pool *pool,
	unsigned long size,
	char const *file,
	long line)
{
	void *p = NULL;
	struct memory_block *block = stack_memory_pool_new_block_clear(pool, size, file, line);

	if (block)
	{
		stack_memory_pool_lock_block(pool, block);
		p = memory_block_get_user_address(block);
		pool->statistics.bytes_allocated += memory_block_get_real_size(block);
		pool->statistics.blocks_used++;

		if (pool->statistics.bytes_allocated>pool->statistics.peak_bytes_allocated)
		{
			pool->statistics.peak_bytes_allocated = pool->statistics.bytes_allocated;
		}

		if (pool->statistics.blocks_used>pool->statistics.peak_blocks_used)
		{
			pool->statistics.peak_blocks_used = pool->statistics.blocks_used;
		}

		if (memory_block_get_real_size(block)>pool->statistics.largest_block_allocated)
		{
			pool->statistics.largest_block_allocated = memory_block_get_real_size(block);
		}
	}

	return p;
}

void *pool_resize_pointer(
	struct stack_memory_pool *pool,
	void *p,
	unsigned long new_size,
	char const *file,
	long line)
{
	struct memory_block *block = p ? (struct memory_block *)((byte *)p - 24) : NULL;
	long old_size = 0;

	if (block)
	{
		block = (struct memory_block *)((byte *)block - 4);
		old_size = memory_block_get_real_size(block);
	}

	block = stack_memory_pool_resize_block(pool, block, new_size, file, line);

	if (block)
	{
		if (!memory_block_is_locked(block))
		{
			stack_memory_pool_lock_block(pool, block);
		}

		p = memory_block_get_user_address(block);
		pool->statistics.bytes_allocated += memory_block_get_real_size(block) - old_size;
		pool->statistics.blocks_used += old_size>0 ? 0 : 1;

		if (pool->statistics.bytes_allocated>pool->statistics.peak_bytes_allocated)
		{
			pool->statistics.peak_bytes_allocated = pool->statistics.bytes_allocated;
		}

		if (pool->statistics.blocks_used>pool->statistics.peak_blocks_used)
		{
			pool->statistics.peak_blocks_used = pool->statistics.blocks_used;
		}

		if (memory_block_get_real_size(block)>pool->statistics.largest_block_allocated)
		{
			pool->statistics.largest_block_allocated = memory_block_get_real_size(block);
		}
	}
	else
	{
		p = NULL;
	}

	return p;
}

void dispose_pointer(
	struct stack_memory_pool *pool,
	void *p)
{
	struct memory_block *block;
	unsigned long size;

	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 407, p);

	block = (struct memory_block *)((byte *)p - 24);
	block = (struct memory_block *)((byte *)block - 4);
	match_vassert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 413, stack_memory_pool_valid_block(pool, block), "invalid pointer");

	size = memory_block_get_real_size(block);
	stack_memory_pool_dispose_block(pool, block);
	pool->statistics.bytes_allocated -= size;
	pool->statistics.blocks_used--;

	return;
}

void stack_memory_pool_reset(
	struct stack_memory_pool *pool)
{
	char *name;
	byte *base_address;
	struct memory_block **blocks_array;
	long number_of_blocks;
	unsigned long pool_size;

	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 432, pool);

	name = pool->name;
	base_address = pool->base_address;
	number_of_blocks = pool->number_of_blocks;
	blocks_array = pool->blocks;
	pool_size = pool->pool_size;
	memset(pool->blocks, 0, pool->number_of_blocks*sizeof(struct memory_block *));
	memset(pool, 0, sizeof(struct stack_memory_pool));
	pool->name = name;
	pool->base_address = base_address;
	pool->number_of_blocks = number_of_blocks;
	pool->pool_size = pool_size;
	memcpy(pool->blocks, &blocks_array, sizeof(blocks_array));

	return;
}

/* ---------- private code */

__inline static boolean memory_block_valid(
	struct memory_block *block)
{
	boolean valid;
	unsigned long real_size;

	if (!block)
	{
		valid = FALSE;
	}
	else
	{
		real_size = memory_block_get_real_size(block);

		if (real_size - INTERNAL_POINTER_OVERHEAD<=0)
		{
			match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 484, !"pointer has invalid size");
			valid = FALSE;
		}
		else if (block->signature!=MEMORY_BLOCK_HEADER_SIGNATURE)
		{
			match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 489, !"this memory has been corrupted");
			valid = FALSE;
		}
		else if (*(unsigned long *)((byte *)block + real_size - sizeof(unsigned long))!=MEMORY_BLOCK_TRAILER_SIGNATURE)
		{
			match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 494, !"wrote beyond the valid address space for this block");
			valid = FALSE;
		}
		else
		{
			valid = TRUE;
		}
	}

	return valid;
}

__inline static void memory_block_lock(
	struct memory_block *block)
{
	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 515, memory_block_valid(block));
	block->bits |= _pointer_locked_bit;

	return;
}

__inline static void memory_block_unlock(
	struct memory_block *block)
{
	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 524, memory_block_valid(block));
	block->bits &= _pointer_size_mask;

	return;
}

__inline static boolean memory_block_is_locked(
	struct memory_block *block)
{
	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 533, memory_block_valid(block));

	return (block->bits & _pointer_locked_bit) ? TRUE : FALSE;
}

__inline static void memory_block_set_size_and_index(
	struct memory_block *block,
	unsigned long new_size,
	unsigned long index)
{
	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 543, block);

	block->bits = new_size;
	block->index = index;
	block->signature = MEMORY_BLOCK_HEADER_SIGNATURE;
	*(unsigned long *)((byte *)block + new_size - sizeof(unsigned long)) = MEMORY_BLOCK_TRAILER_SIGNATURE;

	return;
}

__inline static unsigned long memory_block_get_real_size(
	struct memory_block *block)
{
	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 559, block);

	return block->bits & _pointer_size_mask;
}

__inline static unsigned long memory_block_get_user_size(
	struct memory_block *block)
{
	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 567, block);

	return memory_block_get_real_size(block) - INTERNAL_POINTER_OVERHEAD;
}

__inline static void *memory_block_get_user_address(
	struct memory_block *block)
{
	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 575, memory_block_valid(block));

	return block + 1;
}

__inline static unsigned long memory_block_get_pool_index(
	struct memory_block *block)
{
	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 586, block);

	return block->index;
}

static struct memory_block *stack_memory_pool_new_block(
	struct stack_memory_pool *pool,
	unsigned long size,
	char const *file,
	long line)
{
	return stack_memory_pool_allocate(pool, size, file, line);
}

static struct memory_block *stack_memory_pool_new_block_clear(
	struct stack_memory_pool *pool,
	unsigned long size,
	char const *file,
	long line)
{
	struct memory_block *block = stack_memory_pool_allocate(pool, size, file, line);

	if (block)
	{
		memset(memory_block_get_user_address(block), 0, size);
	}

	return block;
}

static struct memory_block *stack_memory_pool_resize_block(
	struct stack_memory_pool *pool,
	struct memory_block *reference,
	unsigned long new_size,
	char const *file,
	long line)
{
	struct memory_block *new_block = NULL;

	if (new_size>0)
	{
		if (!reference)
		{
			new_block = stack_memory_pool_allocate(pool, new_size, file, line);
		}
		else
		{
			match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 692, stack_memory_pool_valid_block(pool, reference));

			if (memory_block_get_user_size(reference)>=new_size)
			{
				new_block = reference;
			}
			else
			{
				new_block = stack_memory_pool_allocate(pool, new_size, file, line);

				if (new_block)
				{
					memcpy(memory_block_get_user_address(new_block), memory_block_get_user_address(reference), memory_block_get_user_size(reference));
					stack_memory_pool_dispose_block(pool, reference);
				}
			}
		}
	}

	return new_block;
}

static void stack_memory_pool_dispose_block(
	struct stack_memory_pool *pool,
	struct memory_block *reference)
{
	unsigned long index;

	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 723, stack_memory_pool_valid_block(pool, reference));

	index = memory_block_get_pool_index(reference);

	if (reference->previous_block)
	{
		reference->previous_block->next_block = reference->next_block;
	}

	if (reference->next_block)
	{
		reference->next_block->previous_block = reference->previous_block;
	}

	if (reference==pool->first_block)
	{
		pool->first_block = reference->next_block;
	}

	if (reference==pool->last_block)
	{
		pool->last_block = reference->previous_block;
	}

	pool->blocks[index] = NULL;
	pool->next_block_index = !pool->first_block ? 0 : index;

	return;
}

static void *stack_memory_pool_lock_block(
	struct stack_memory_pool *pool,
	struct memory_block *reference)
{
	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 741, stack_memory_pool_valid_block(pool, reference) && !memory_block_is_locked(reference));
	memory_block_lock(reference);

	return memory_block_get_user_address(reference);
}

static void stack_memory_pool_unlock_block(
	struct stack_memory_pool *pool,
	struct memory_block *reference)
{
	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 752, stack_memory_pool_valid_block(pool, reference) && memory_block_is_locked(reference));
	memory_block_unlock(reference);

	return;
}

static unsigned long stack_memory_pool_free_space_at_end_of_pool(
	struct stack_memory_pool *pool)
{
	unsigned long free_space;

	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 764, pool && pool->base_address);

	if (!pool->first_block)
	{
		free_space = pool->pool_size;
	}
	else
	{
		free_space = pool->pool_size - ((byte *)pool->last_block + memory_block_get_real_size(pool->last_block) - pool->base_address);
	}

	return free_space;
}

static long stack_memory_pool_find_first_unused_memory_block(
	struct stack_memory_pool *pool)
{
	unsigned long index;
	long result;

	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 784, pool);

	for (index = 0, result = NONE; index<pool->number_of_blocks; index++)
	{
		if (!pool->blocks[index])
		{
			result = index;
			break;
		}
	}

	return result;
}

static void stack_memory_pool_set_next_block_index(
	struct stack_memory_pool *pool)
{
	long index;

	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 801, pool);

	if (pool->next_block_index!=NONE)
	{
		index = pool->next_block_index + 1;
		pool->next_block_index = NONE;

		for (; index<pool->number_of_blocks; index++)
		{
			if (!pool->blocks[index])
			{
				pool->next_block_index = index;
				break;
			}
		}
	}

	return;
}

static struct memory_block *stack_memory_pool_allocate(
	struct stack_memory_pool *pool,
	unsigned long size,
	char const *file,
	long line)
{
	struct memory_block *block = NULL;

	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 834, pool && pool->base_address);

	if (size>0 && size<=MAXIMUM_BLOCK_SIZE && size<pool->pool_size)
	{
		unsigned long free_space;
		struct memory_block *free_space_in_pool_previous = NULL;
		void *new_block_address = NULL;

		size += INTERNAL_POINTER_OVERHEAD;

		while (size % ALIGNMENT_BYTE_BOUNDARY)
		{
			size++;
		}

		free_space = stack_memory_pool_free_space_at_end_of_pool(pool);

		if (free_space<size)
		{
			stack_memory_pool_compact(pool);
			free_space = stack_memory_pool_free_space_at_end_of_pool(pool);
		}

		if (free_space<size)
		{
			new_block_address = stack_memory_pool_find_space_between_blocks(pool, size, &free_space_in_pool_previous);
		}

		if (free_space>=size || new_block_address)
		{
			if (pool->next_block_index==NONE)
			{
				pool->next_block_index = stack_memory_pool_find_first_unused_memory_block(pool);
				match_vwarn("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 863, pool->next_block_index!=NONE, "the memory pool has no more unsused master pointers; you need to use a bigger pool");
			}

			if (pool->next_block_index!=NONE)
			{
				if (new_block_address)
				{
					pool->blocks[pool->next_block_index] = (struct memory_block *)new_block_address;
				}
				else if (!pool->first_block)
				{
					pool->blocks[pool->next_block_index] = (struct memory_block *)pool->base_address;
				}
				else
				{
					match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 880, pool->last_block);
					pool->blocks[pool->next_block_index] = (struct memory_block *)((byte *)pool->last_block + memory_block_get_real_size(pool->last_block));
				}

				memory_block_set_size_and_index(pool->blocks[pool->next_block_index], size, pool->next_block_index);
				block = pool->blocks[pool->next_block_index];
				block->file = file;
				block->line = line;

				if (!pool->first_block)
				{
					match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 893, (pool->last_block == NULL) && (pool->next_block_index == 0));
					pool->last_block = block;
					pool->first_block = block;
					block->previous_block = NULL;
					block->next_block = NULL;
				}
				else if (block<pool->first_block)
				{
					block->previous_block = NULL;
					block->next_block = pool->first_block;
					pool->first_block->previous_block = block;
					pool->first_block = block;
				}
				else if (block>pool->last_block)
				{
					block->previous_block = pool->last_block;
					block->next_block = NULL;
					pool->last_block->next_block = block;
					pool->last_block = block;
				}
				else
				{
					match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 916, free_space_in_pool_previous);
					block->previous_block = free_space_in_pool_previous;
					block->next_block = free_space_in_pool_previous->next_block;
					free_space_in_pool_previous->next_block = block;

					if (block->next_block)
					{
						block->next_block->previous_block = block;
					}
				}

				stack_memory_pool_set_next_block_index(pool);
			}
		}
		else
		{
			match_vwarn("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 927, FALSE, "allocation from memory pool failed; unable to find sufficient space in the pool");
		}
	}
	else
	{
		match_vwarn("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 932, FALSE, "invalid size");
	}

	return block;
}

static void stack_memory_pool_compact(
	struct stack_memory_pool *pool)
{
	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 941, pool && pool->base_address && pool->blocks);

	if (pool->first_block && !pool->locked)
	{
		byte *previous_block = pool->base_address;
		struct memory_block *block = pool->first_block;
		unsigned long previous_size = 0;

		while (block)
		{
			if (!memory_block_is_locked(block))
			{
				long gap = (byte *)block - (previous_block + previous_size);

				if (gap>0)
				{
					memmove(previous_block + previous_size, block, memory_block_get_real_size(block));
					block = (struct memory_block *)(previous_block + previous_size);

					if (block->previous_block)
					{
						block->previous_block->next_block = block;
					}
				}
			}

			previous_size = memory_block_get_real_size(block);
			previous_block = (byte *)block;
			block = block->next_block;
		}
	}

	return;
}

static boolean stack_memory_pool_valid_block(
	struct stack_memory_pool *pool,
	struct memory_block *block)
{
	boolean valid = FALSE;

	match_assert("c:\\halo\\SOURCE\\memory\\stack_memory_pool.c", 983, pool && pool->base_address);

	if ((byte *)block>=pool->base_address && (byte *)block<pool->base_address + pool->pool_size && memory_block_valid(block))
	{
		unsigned long index = memory_block_get_pool_index(block);

		if (index<pool->number_of_blocks &&
			pool->blocks[index] &&
			pool->blocks[index]->bits==block->bits &&
			pool->blocks[index]->previous_block==block->previous_block &&
			pool->blocks[index]->next_block==block->next_block)
		{
			valid = TRUE;
		}
	}

	return valid;
}

static void *stack_memory_pool_find_space_between_blocks(
	struct stack_memory_pool *pool,
	unsigned long size,
	struct memory_block **free_space_in_pool_previous)
{
	void *result = NULL;

	if (pool->first_block && (unsigned long)((byte *)pool->first_block - pool->base_address)>=size)
	{
		result = pool->base_address;
	}
	else
	{
		struct memory_block *block = pool->first_block;
		struct memory_block *next_block = pool->first_block ? pool->first_block->next_block : NULL;

		while (next_block)
		{
			unsigned long free_space = (byte *)next_block - ((byte *)block + memory_block_get_real_size(block));

			if (free_space>=size)
			{
				result = (byte *)block + memory_block_get_real_size(block);
				*free_space_in_pool_previous = block;
				break;
			}

			block = next_block;
			next_block = next_block->next_block;
		}
	}

	return result;
}
