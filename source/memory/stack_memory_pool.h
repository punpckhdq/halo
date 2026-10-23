/*
STACK_MEMORY_POOL.H

header included in hcex build.
*/

#ifndef __STACK_MEMORY_POOL_H
#define __STACK_MEMORY_POOL_H
#pragma once

/* ---------- structures */

typedef void **handle;

struct stack_memory_pool
{
	char const *name;
	byte *base_address;
	unsigned long pool_size;
	long number_of_blocks;
	long next_block_index;
	struct
	{
		long total_size; /* fake name */
		long maximum_total_size; /* fake name */
		unsigned long block_count; /* fake name */
		unsigned long maximum_block_count; /* fake name */
		unsigned long largest_block_size; /* fake name */
	} statistics;
	boolean locked;
	struct memory_block *first_block;
	struct memory_block *last_block;
	struct memory_block *blocks[];
};

/* ---------- prototypes/STACK_MEMORY_POOL.C */

handle pool_new_handle(struct stack_memory_pool *pool, unsigned long size, char const *file, long line);
handle pool_new_handle_clear(struct stack_memory_pool *pool, unsigned long size, char const *file, long line);
boolean pool_resize_handle(struct stack_memory_pool *pool, handle *h, unsigned long new_size, char const *file, long line);
void dispose_handle(struct stack_memory_pool *pool, handle h);
void lock_handle(struct stack_memory_pool *pool, handle *h);
void unlock_handle(struct stack_memory_pool *pool, handle h);
void *pool_new_pointer(struct stack_memory_pool *pool, unsigned long size, char const *file, long line);
void *pool_new_pointer_clear(struct stack_memory_pool *pool, unsigned long size, char const *file, long line);
void *pool_resize_pointer(struct stack_memory_pool *pool, void *p, unsigned long new_size, char const *file, long line);
void dispose_pointer(struct stack_memory_pool *pool, void *p);
void stack_memory_pool_reset(struct stack_memory_pool *pool);

#endif // __STACK_MEMORY_POOL_H
