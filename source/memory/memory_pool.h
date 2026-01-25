/*
MEMORY_POOL.H

header included in hcex build.
*/

#ifndef __MEMORY_POOL_H
#define __MEMORY_POOL_H
#pragma once

/* ---------- constants */

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

/* ---------- prototypes/MEMORY_POOL.C */

struct memory_pool *memory_pool_new(char const *name, long size);
void memory_pool_delete(struct memory_pool *pool);
long memory_pool_allocation_size(long size);
void memory_pool_initialize(struct memory_pool *pool, char const *name, long size);
boolean memory_pool_block_allocate(struct memory_pool *pool, void **reference, long size);
boolean memory_pool_block_reallocate(struct memory_pool *pool, void **reference, long new_size);
void memory_pool_block_free(struct memory_pool *pool, void **reference);
void memory_pool_compact(struct memory_pool *pool);
long memory_pool_get_free_size(struct memory_pool *pool);
long memory_pool_get_used_size(struct memory_pool *pool);
long memory_pool_get_contiguous_free_size(struct memory_pool *pool);

/* ---------- globals */

/* ---------- public code */

#endif // __MEMORY_POOL_H
