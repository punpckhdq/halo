/*
LRUV_CACHE.H

header included in hcex build.
*/

#ifndef __LRUV_CACHE_H
#define __LRUV_CACHE_H
#pragma once

/* ---------- constants */

enum
{
	_lruv_cache_page_usage_allocated_bit = 0,
	_lruv_cache_page_usage_used_this_frame_bit,
	_lruv_cache_page_usage_old_bit,
	_lruv_cache_page_usage_locked_bit,
	NUMBER_OF_LRUV_CACHE_PAGE_USAGE_FLAGS,
};

/* ---------- structures */

typedef void (*lruv_delete_block_proc)(long block_index); /* fake name */
typedef boolean (*lruv_locked_block_proc)(long block_index); /* fake name */
typedef void (*lruv_header_proc)(FILE *file); /* fake name */
typedef char const *(*lruv_name_block_proc)(long block_index); /* fake name */

/* ---------- prototypes/LRUV_CACHE.C */

long lruv_allocation_size(long maximum_block_count);
void lruv_initialize(struct lruv_cache *cache, char const *name, long page_count, long page_size_bits, long maximum_block_count, lruv_delete_block_proc delete_block_proc, lruv_locked_block_proc locked_block_proc);
struct lruv_cache *lruv_new(char const *name, long page_count, long page_size_bits, long maximum_block_count, lruv_delete_block_proc delete_block_proc, lruv_locked_block_proc locked_block_proc);
boolean lruv_has_locked_proc(struct lruv_cache const *cache);
void lruv_update_function_pointers(struct lruv_cache *cache, lruv_delete_block_proc delete_block_proc, lruv_locked_block_proc locked_block_proc);
void lruv_flush(struct lruv_cache *cache);
void lruv_delete(struct lruv_cache *cache);
void lruv_idle(struct lruv_cache *cache);
long lruv_block_new(struct lruv_cache *cache, long size);
unsigned long lruv_block_get_address(struct lruv_cache *cache, long block_index);
void lruv_block_delete(struct lruv_cache *cache, long block_index);
void lruv_block_touch(struct lruv_cache *cache, long block_index);
boolean lruv_block_touched(struct lruv_cache *cache, long block_index);
void lruv_cache_get_page_usage(struct lruv_cache *cache, byte *page_usage);
void lruv_resize(struct lruv_cache *cache, long new_page_count);
void lruv_debug_to_file(char const *path, char const *failed_allocation_name, long failed_allocation_size, struct lruv_cache *cache, lruv_header_proc header_proc, lruv_name_block_proc name_block_proc);

#endif // __LRUV_CACHE_H
