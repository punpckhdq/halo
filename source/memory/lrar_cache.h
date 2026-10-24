/*
LRAR_CACHE.H

header included in hcex build.
*/

#ifndef __LRAR_CACHE_H
#define __LRAR_CACHE_H
#pragma once

/* ---------- structures */

typedef void (*lrar_new_block_proc)(short *reference, short block_index); /* fake name */
typedef void (*lrar_purge_block_proc)(short *reference); /* fake name */

/* ---------- prototypes/LRAR_CACHE.C */

struct lrar_cache *lrar_new(char *name, unsigned long minimum_address, unsigned long maximum_address, short block_count, short alignment_bit, short boundary_bit, lrar_new_block_proc new_block_proc, lrar_purge_block_proc purge_block_proc);
void lrar_dispose(struct lrar_cache *cache);
void lrar_flush(struct lrar_cache *cache);
short lrar_allocate(struct lrar_cache *cache, long size, short *reference);
unsigned long lrar_block_address(struct lrar_cache *cache, short block_index);
void lrar_deallocate(struct lrar_cache *cache, short block_index);

#endif // __LRAR_CACHE_H
