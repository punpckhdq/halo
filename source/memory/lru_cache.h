/*
LRU_CACHE.H

header included in hcex build.
*/

#ifndef __LRU_CACHE_H
#define __LRU_CACHE_H
#pragma once

/* ---------- structures */

typedef void (*lru_new_block_proc)(void **reference, void *data); /* fake name */
typedef void (*lru_purge_block_proc)(void **reference); /* fake name */

/* ---------- prototypes/LRU_CACHE.C */

struct lru_cache *lru_new(char *name, long total_size, long block_size, lru_new_block_proc new_block_proc, lru_purge_block_proc purge_block_proc, void *buffer);
void lru_dispose(struct lru_cache *cache);
void lru_flush(struct lru_cache *cache);
long lru_free_blocks(struct lru_cache *cache);
void *lru_allocate(struct lru_cache *cache, void **reference);
void lru_lock(struct lru_cache *cache, void *data);
void lru_unlock(struct lru_cache *cache, void *data);
void lru_touch(struct lru_cache *cache, void *data);

#endif // __LRU_CACHE_H
