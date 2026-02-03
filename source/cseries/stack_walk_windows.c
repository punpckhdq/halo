/*
STACK_WALK_WINDOWS.C

symbols in this file:
000806A0 0020:
	_stack_walk_global_function_offset (0000)
000806C0 0010:
	_stack_walk_disregard_symbol_names (0000)
000806D0 0030:
	_code_000806d0 (0000)
00080700 0080:
	_free_symbol_table (0000)
00080780 00b0:
	_symbol_name_from_address (0000)
00080830 0060:
	_base_address_from_symbol_name (0000)
00080890 0020:
	_code_00080890 (0000)
000808B0 0040:
	_code_000808b0 (0000)
000808F0 0020:
	_code_000808f0 (0000)
00080910 00d0:
	_code_00080910 (0000)
000809E0 00d0:
	_code_000809e0 (0000)
00080AB0 0020:
	_stack_walk_dispose (0000)
00080AD0 02b0:
	_stack_walk_with_context (0000)
00080D80 0620:
	_load_symbol_table (0000)
000813A0 0030:
	_stack_walk_initialize (0000)
000813D0 0020:
	_stack_walk (0000)
002586B4 000d:
	??_C@_0N@CNIOPMPL@symbol_table?$AA@ (0000)
002586C4 002c:
	??_C@_0CM@LOAPPLNB@c?3?2halo?2SOURCE?2cseries?2stack_wal@ (0000)
002586F0 0010:
	??_C@_0BA@OFOBHDCE@?$CFs?5?$CL?5?$CF04lX?5?3?5?$CFs?$AA@ (0000)
00258700 000a:
	??_C@_09JLKGDFIH@?$CF08lX?5?$CFs?6?$AA@ (0000)
0025870C 0029:
	??_C@_0CJ@POGHEDC@EIP?3?50x?$CF08lX?0?5?$CF02lX?5?$CF02lX?5?$CF02lX?5@ (0000)
00258738 000d:
	??_C@_0N@DEGIIKCL@ESP?3?50x?$CF08lX?$AA@ (0000)
00258748 000d:
	??_C@_0N@KDNFCLLI@EBP?3?50x?$CF08lX?$AA@ (0000)
00258758 000d:
	??_C@_0N@OBNDHAGF@ESI?3?50x?$CF08lX?$AA@ (0000)
00258768 000d:
	??_C@_0N@JJFDHEBG@EDI?3?50x?$CF08lX?$AA@ (0000)
00258778 000d:
	??_C@_0N@PGCLGPBN@EDX?3?50x?$CF08lX?$AA@ (0000)
00258788 000d:
	??_C@_0N@ICLDIGJC@ECX?3?50x?$CF08lX?$AA@ (0000)
00258798 000d:
	??_C@_0N@BJBGMKPN@EBX?3?50x?$CF08lX?$AA@ (0000)
002587A8 000d:
	??_C@_0N@GOIIBIAN@EAX?3?50x?$CF08lX?$AA@ (0000)
002587B8 0009:
	??_C@_08CCCCNONO@?$CF08lX?5?$CFs?$AA@ (0000)
002587C4 0006:
	??_C@_05EHMBKEBF@?$DP?$DP?$DP?$DP?$DP?$AA@ (0000)
002587CC 0025:
	??_C@_0CF@JNHFBDOE@Printing?5stuff?5for?5Mat?8s?5edifica@ (0000)
002587F4 002e:
	??_C@_0CO@DKHIBCJD@could?5not?5allocate?5enough?5memory@ (0000)
00258828 0051:
	??_C@_0FB@FDLBMJMP@string_storage_used?5?$CL?5strlen?$CIlib@ (0000)
00258880 0044:
	??_C@_0EE@HFHPDHDJ@string_storage_used?5?$CL?5strlen?$CIsym@ (0000)
002588C8 006b:
	??_C@_0GL@HCCMEKIO@string_storage_used?5?$CL?5strlen?$CIsym@ (0000)
00258934 0013:
	??_C@_0BD@IFIGKEIG@_load_symbol_table?$AA@ (0000)
00258948 000f:
	??_C@_0P@JLHKGDG@Static?5symbols?$AA@ (0000)
00258958 000f:
	??_C@_0P@BNEDFJAM@entry?5point?5at?$AA@ (0000)
00258968 0005:
	??_C@_04DNCNJDPH@?5?7?6?$AN?$AA@ (0000)
00258970 0002:
	??_C@_01JLIPDDHJ@?3?$AA@ (0000)
00258974 0008:
	??_C@_07DFGIHOIL@nothing?$AA@ (0000)
0025897C 0019:
	??_C@_0BJ@BLEJAHCK@map?5file?5appears?5corrupt?$AA@ (0000)
00258998 000a:
	??_C@_09KDGENIOP@Timestamp?$AA@ (0000)
002589A4 000b:
	??_C@_0L@CKIHAGIB@Lib?3Object?$AA@ (0000)
002589B0 001c:
	??_C@_0BM@LBIFOOIP@Couldn?8t?5read?5map?5file?5?8?$CFs?8?$AA@ (0000)
002589CC 0011:
	??_C@_0BB@LJHACKGC@d?3?2cachebeta?4map?$AA@ (0000)
002589E0 0019:
	??_C@_0BJ@NNIPHMJN@Mon?5Dec?517?512?349?336?52001?$AA@ (0000)
002DCD40 0014:
	_stack_walk_globals (0000)
00431C98 4008:
	_bss_00431c98 (0000)
*/

/* ---------- headers */

#include "cseries.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct debug_symbol_table
{
	long number_of_symbols;
	char *string_storage;
	struct debug_symbol *symbols;
};

struct debug_symbol
{
	unsigned long address;
	unsigned long rva_base;
	unsigned long name_string_offset;
	unsigned long library_object_string_offset;
};

struct _stack_walk_globals
{
	long fixup;
	boolean disregard_symbol_names;
	struct debug_symbol_table symbol_table;
};

/* ---------- prototypes */

/* ---------- globals */

struct _stack_walk_globals stack_walk_globals=
{
	NONE,
	FALSE
};

/* ---------- public code */

void stack_walk_disregard_symbol_names(
	boolean disregard)
{
	stack_walk_globals.disregard_symbol_names= disregard;
}

void free_symbol_table(
	struct debug_symbol_table *symbol_table)
{
	match_assert("c:\\halo\\SOURCE\\cseries\\stack_walk_windows.c", 549, symbol_table);

	if (symbol_table->string_storage)
	{
		debug_free(symbol_table->string_storage, "c:\\halo\\SOURCE\\cseries\\stack_walk_windows.c", 551);
	}

	if (symbol_table->symbols)
	{
		debug_free(symbol_table->symbols, "c:\\halo\\SOURCE\\cseries\\stack_walk_windows.c", 552);
	}

	symbol_table->number_of_symbols= 0;
	symbol_table->string_storage= NULL;
	symbol_table->symbols= NULL;
}

void stack_walk_dispose(
	void)
{
	stack_walk_globals.fixup= NONE;
	stack_walk_globals.disregard_symbol_names= FALSE;
	free_symbol_table(&stack_walk_globals.symbol_table);
}

/* ---------- private code */
