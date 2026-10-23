/*
LRA_CACHE.H

header included in hcex build.
*/

#ifndef __LRA_CACHE_H
#define __LRA_CACHE_H
#pragma once

/* ---------- structures */

typedef void (*lra_new_block_proc)(void **reference, void *data); /* fake name */
typedef void (*lra_purge_block_proc)(void **reference); /* fake name */

/* ---------- prototypes/LRA_CACHE.C */

struct lra_cache *lra_new(char const *name, long size, lra_new_block_proc new_block_proc, lra_purge_block_proc purge_block_proc, void *base_address);
void lra_dispose(struct lra_cache *cache);
void lra_flush(struct lra_cache *cache);
boolean lra_full(struct lra_cache *cache);
void *lra_allocate(struct lra_cache *cache, long size, void **reference);
void lra_free(struct lra_cache *cache, void *pointer);
void lra_lock(struct lra_cache *cache, void *pointer);
void lra_unlock(struct lra_cache *cache, void *pointer);

#endif // __LRA_CACHE_H
