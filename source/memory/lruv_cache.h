/*
LRUV_CACHE.H

header included in hcex build.
*/

#ifndef __LRUV_CACHE_H
#define __LRUV_CACHE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/LRUV_CACHE.C */

boolean lruv_has_locked_proc(struct lruv_cache const *cache);
void lruv_update_function_pointers(struct lruv_cache *cache, void (*delete_block_proc)(long), boolean (*locked_block_proc)(long));
void lruv_flush(struct lruv_cache *cache);
void lruv_delete(struct lruv_cache *cache);
void lruv_idle(struct lruv_cache *cache);
long lruv_block_new(struct lruv_cache *cache, long size);
unsigned long lruv_block_get_address(struct lruv_cache const *cache, long block_index);
void lruv_block_delete(struct lruv_cache *cache, long block_index);

/* ---------- globals */

/* ---------- public code */

#endif // __LRUV_CACHE_H
