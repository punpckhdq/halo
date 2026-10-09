/*
STACK_MEMORY_POOL.C

symbols in this file:
0010E1E0 0090:
	_stack_memory_pool_reset (0000)
0010E270 0040:
	_memory_block_set_size_and_index (0000)
0010E2B0 0030:
	_memory_block_get_real_size (0000)
0010E2E0 0050:
	_memory_block_get_user_size (0000)
0010E330 0030:
	_memory_block_get_pool_index (0000)
0010E360 0080:
	_stack_memory_pool_free_space_at_end_of_pool (0000)
0010E3E0 0050:
	_stack_memory_pool_find_first_unused_memory_block (0000)
0010E430 0060:
	_stack_memory_pool_set_next_block_index (0000)
0010E490 0080:
	_stack_memory_pool_find_space_between_blocks (0000)
0010E510 00a0:
	_memory_block_valid (0000)
0010E5B0 0040:
	_memory_block_lock (0000)
0010E5F0 0040:
	_memory_block_unlock (0000)
0010E630 0040:
	_memory_block_is_locked (0000)
0010E670 0030:
	_memory_block_get_user_address (0000)
0010E6A0 00d0:
	_stack_memory_pool_compact (0000)
0010E770 0080:
	_stack_memory_pool_valid_block (0000)
0010E7F0 00a0:
	_stack_memory_pool_dispose_block (0000)
0010E890 00d0:
	_stack_memory_pool_lock_block (0000)
0010E960 00a0:
	_stack_memory_pool_unlock_block (0000)
0010EA00 0280:
	_stack_memory_pool_allocate (0000)
0010EC80 0080:
	_dispose_handle (0000)
0010ED00 0070:
	_lock_handle (0000)
0010ED70 00d0:
	_unlock_handle (0000)
0010EE40 00b0:
	_dispose_pointer (0000)
0010EEF0 0020:
	_stack_memory_pool_new_block (0000)
0010EF10 0060:
	_stack_memory_pool_new_block_clear (0000)
0010EF70 00c0:
	_stack_memory_pool_resize_block (0000)
0010F030 0070:
	_pool_new_handle (0000)
0010F0A0 00b0:
	_pool_new_handle_clear (0000)
0010F150 0110:
	_pool_resize_handle (0000)
0010F260 00b0:
	_pool_new_pointer (0000)
0010F310 00f0:
	_pool_new_pointer_clear (0000)
0010F400 0130:
	_pool_resize_pointer (0000)
0027E6B4 0005:
	??_C@_04CJIJKHMO@pool?$AA@ (0000)
0027E6BC 002a:
	??_C@_0CK@POCGLLGP@c?3?2halo?2SOURCE?2memory?2stack_memo@ (0000)
0027E6E8 0006:
	??_C@_05PIBOEKAB@block?$AA@ (0000)
0027E6F0 001b:
	??_C@_0BL@DBFCIPGG@pool?5?$CG?$CG?5pool?9?$DObase_address?$AA@ (0000)
0027E70C 0037:
	??_C@_0DH@EDIODODK@?$CB?$CCwrote?5beyond?5the?5valid?5address@ (0000)
0027E744 0022:
	??_C@_0CC@ECLAPKCC@?$CB?$CCthis?5memory?5has?5been?5corrupted@ (0000)
0027E768 001c:
	??_C@_0BM@JILEAMIL@?$CB?$CCpointer?5has?5invalid?5size?$CC?$AA@ (0000)
0027E784 001a:
	??_C@_0BK@LDONEMGA@memory_block_valid?$CIblock?$CJ?$AA@ (0000)
0027E7A0 002b:
	??_C@_0CL@HLEBFHCG@pool?5?$CG?$CG?5pool?9?$DObase_address?5?$CG?$CG?5po@ (0000)
0027E7CC 002f:
	??_C@_0CP@BNFGAJLL@stack_memory_pool_valid_block?$CIpo@ (0000)
0027E800 0055:
	??_C@_0FF@JCHHGMKP@stack_memory_pool_valid_block?$CIpo@ (0000)
0027E858 0054:
	??_C@_0FE@NGEIOJCH@stack_memory_pool_valid_block?$CIpo@ (0000)
0027E8AC 000d:
	??_C@_0N@CECKNFDB@invalid?5size?$AA@ (0000)
0027E8BC 001c:
	??_C@_0BM@OEEFEPIM@free_space_in_pool_previous?$AA@ (0000)
0027E8D8 003c:
	??_C@_0DM@JDDKECPN@?$CIpool?9?$DOlast_block?5?$DN?$DN?5NULL?$CJ?5?$CG?$CG?5?$CIp@ (0000)
0027E914 0011:
	??_C@_0BB@LKLKPKKP@pool?9?$DOlast_block?$AA@ (0000)
0027E928 0053:
	??_C@_0FD@KNOHBMJO@the?5memory?5pool?5has?5no?5more?5unsu@ (0000)
0027E980 0050:
	??_C@_0FA@LKCLAHNE@allocation?5from?5memory?5pool?5fail@ (0000)
0027E9D0 0028:
	??_C@_0CI@IMGJKFFB@not?5a?5valid?5handle?0?5or?5handle?5is@ (0000)
0027E9F8 0030:
	??_C@_0DA@KBONCLF@not?5a?5valid?5handle?0?5or?5handle?5is@ (0000)
0027EA28 0002:
	??_C@_01BDACAMKP@h?$AA@ (0000)
0027EA2C 0029:
	??_C@_0CJ@NEONIHNA@invalid?5handle?0?5or?5handle?5was?5no@ (0000)
0027EA58 0010:
	??_C@_0BA@KKNICBIG@invalid?5pointer?$AA@ (0000)
0027EA68 0038:
	??_C@_0DI@NCMLNKNJ@not?5a?5valid?5handle?0?5or?5trying?5to@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "stack_memory_pool.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

/* ---------- private code */
