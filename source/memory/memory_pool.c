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

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

long memory_pool_get_free_size(
	struct memory_pool *pool)
{
	return pool->free_size;
}

long memory_pool_get_used_size(
	struct memory_pool *pool)
{
	return !pool->last_block ? 0 : (unsigned long)pool->last_block + pool->last_block->size - (unsigned long)pool->base_address;
}

long memory_pool_get_contiguous_free_size(
	struct memory_pool *pool)
{
	return pool->size - memory_pool_get_used_size(pool);
}

/* ---------- private code */
