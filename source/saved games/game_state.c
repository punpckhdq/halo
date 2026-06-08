/*
GAME_STATE.C

symbols in this file:
001AF250 0010:
	_dummy (0000)
001AF260 0010:
	_code_001af260 (0000)
001AF270 0010:
	_code_001af270 (0000)
001AF280 0020:
	_code_001af280 (0000)
001AF2A0 0010:
	_game_state_dispose (0000)
001AF2B0 00b0:
	_game_state_initialize_for_new_map (0000)
001AF360 0010:
	_game_state_dispose_from_old_map (0000)
001AF370 0020:
	_game_state_save (0000)
001AF390 0040:
	_game_state_revert (0000)
001AF3D0 0040:
	_game_state_save_to_persistent_storage (0000)
001AF410 0070:
	_game_state_test_persistent_storage (0000)
001AF480 0050:
	_game_state_save_core (0000)
001AF4D0 0020:
	_game_state_reverted (0000)
001AF4F0 0160:
	_code_001af4f0 (0000)
001AF650 0070:
	_code_001af650 (0000)
001AF6C0 0020:
	_code_001af6c0 (0000)
001AF6E0 0110:
	_game_state_malloc (0000)
001AF7F0 0110:
	_game_state_gpu_malloc (0000)
001AF900 0040:
	_game_state_data_new (0000)
001AF940 0040:
	_game_state_memory_pool_new (0000)
001AF980 0050:
	_game_state_lruv_cache_new (0000)
001AF9D0 00f0:
	_game_state_try_and_load_from_persistent_storage (0000)
001AFAC0 00a0:
	_game_state_load_core (0000)
001AFB60 0050:
	_game_state_initialize (0000)
002A7E7C 0013:
	??_C@_0BD@BHHOKLIG@error?5writing?5?8?$CFs?8?$AA@ (0000)
002A7E90 000b:
	??_C@_0L@MGAFKEJL@saved?5?8?$CFs?8?$AA@ (0000)
002A7E9C 0025:
	??_C@_0CF@NHHJOMMA@checksum?5from?5map?5file?5doesn?8t?5m@ (0000)
002A7EC4 0021:
	??_C@_0CB@ELFCALCI@expected?5?$CD?$CFd?5players?5but?5got?5?$CD?$CFd@ (0000)
002A7EE8 001d:
	??_C@_0BN@CGADBPLM@allocation?5checksum?5mismatch?$AA@ (0000)
002A7F08 001b:
	??_C@_0BL@PFDNBAEN@expected?5?$CC?$CFs?$CC?5but?5got?5?$CC?$CFs?$CC?$AA@ (0000)
002A7F24 001f:
	??_C@_0BP@FJPMMOPC@expected?5build?5?$CD?$CFd?5but?5got?5?$CD?$CFd?$AA@ (0000)
002A7F44 0028:
	??_C@_0CI@KECEBGOP@c?3?2halo?2SOURCE?2saved?5games?2game_@ (0000)
002A7F6C 0013:
	??_C@_0BD@JHLLDANP@?$CF?540s?$CF?520s?$CF?510d?$CFs?6?$AA@ (0000)
002A7F80 0011:
	??_C@_0BB@DCECPIDG@d?3?2gamestate?4txt?$AA@ (0000)
002A7F98 0041:
	??_C@_0EB@JIJEJKFB@game_state_globals?4cpu_allocatio@ (0000)
002A7FDC 001b:
	??_C@_0BL@IDCLCKDD@?$CBgame_state_globals?4locked?$AA@ (0000)
002A7FF8 000a:
	??_C@_09KFGLKCMD@?$CB?$CIsize?$CG3?$CJ?$AA@ (0000)
002A8008 0041:
	??_C@_0EB@OBCPKDFA@game_state_globals?4gpu_allocatio@ (0000)
002A804C 000b:
	??_C@_0L@CIHKLGI@data?5array?$AA@ (0000)
002A8058 000b:
	??_C@_0L@HOFDEGMG@lruv?5cache?$AA@ (0000)
002A8064 0013:
	??_C@_0BD@OOCHHAFI@couldn?8t?5open?5?8?$CFs?8?$AA@ (0000)
002A8078 000c:
	??_C@_0M@NNNFMOGK@loaded?5?8?$CFs?8?$AA@ (0000)
00316840 003c:
	_data_00316840 (0000)
004D27B0 0020:
	_bss_004d27b0 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries_windows.h"
#include "real_math.h"
#include "console.h"
#include "game_state.h"
#include "game.h"
#include "tag_files.h"
#include "cache_files.h"
#include "scenario.h"
#include "main.h"
#include "game_sound.h"
#include "sound_manager.h"
#include "observer.h"
#include "players.h"
#include "rasterizer.h"
#include "recorded_animations.h"
#include "ai_debug.h"
#include "structures.h"
#include "director.h"
#include "hud_messaging.h"
#include "crc.h"

/* ---------- constants */

enum
{
	GAME_STATE_CPU_SIZE = 0x305000
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

void dummy(void);
static void game_state_set_revert_time(void);

/* ---------- globals */

static FILE* bss_004d27b0;

static struct
{
	void *base_address; // 0x0
	long cpu_allocation_size; // 0x4
	long gpu_allocation_size; // 0x8
	long allocation_size_checksum; // 0xC
	boolean locked; // 0x10
	boolean saved_game_valid; // 0x11
	long revert_time; // 0x14
	struct game_state_header *header; // 0x18
} game_state_globals;

typedef void (*game_state_before_load_proc)();
typedef void (*game_state_after_load_proc)();
typedef void (*game_state_before_save_proc)();

static game_state_before_save_proc before_save_procs[] =
{
	dummy,
};

static game_state_before_load_proc before_load_procs[] =
{
	game_sound_clear,
};

static game_state_after_load_proc after_load_procs[] =
{
	scenario_reload_structure_bsp_if_necessary,
	sound_stop_all,
	game_sound_restore,
	observer_initialize_for_new_map,
	update_queues_reset_and_fill_with_lies,
	rasterizer_decals_update_function_pointers,
	recorded_animations_clear_debug_storage,
	ai_debug_initialize_for_new_map,
	structure_detail_objects_flush,
	game_state_set_revert_time,
	player_control_fix_for_loaded_game_state,
	director_initialize_for_saved_game,
	scripted_hud_messages_clear
};

/* ---------- public code */

void dummy(
	void)
{
	return;
}

void game_state_call_before_save_procs(
	void)
{
	game_state_after_load_proc *proc = before_save_procs;
	long i;

	for (i =NUMBEROF(before_save_procs); i>0; i--, proc++)
	{
		(*proc)();
	}

	return;
}

void game_state_call_before_load_procs(
	void)
{
	game_state_after_load_proc *proc = before_load_procs;
	long i;

	for (i =NUMBEROF(before_load_procs); i>0; i--, proc++)
	{
		(*proc)();
	}

	return;
}

void game_state_call_after_load_procs(
	void)
{
	game_state_after_load_proc *proc = after_load_procs;
	long i;

	for (i =NUMBEROF(after_load_procs); i>0; i--, proc++)
	{
		(*proc)();
	}

	return;
}

void game_state_dispose(
	void)
{
	game_state_free_buffer();
	game_state_close_file();

	return;
}

void game_state_initialize_for_new_map(
	void)
{
	const char *name;

	game_state_globals.locked = TRUE;
	game_state_globals.saved_game_valid = FALSE;
	game_state_globals.revert_time = NONE;

	memset(game_state_globals.header, 0, sizeof(*game_state_globals.header));

	name = tag_get_name(global_scenario_index);
	strcpy(game_state_globals.header->map_name, name);
	strcpy(game_state_globals.header->build_number, "01.01.14.2342");

	game_state_globals.header->player_count = player_spawn_count;
	game_state_globals.header->difficulty = game_difficulty_level_get();
	game_state_globals.header->cache_file_checksum = cache_files_get_checksum();
	game_state_globals.header->allocation_size_checksum = game_state_globals.allocation_size_checksum;

	return;
}

void game_state_dispose_from_old_map(
	void)
{
	return;
}

void game_state_save(
	void)
{
	game_state_call_before_save_procs();

	main_stop_time();
	game_state_globals.saved_game_valid = (game_state_write_to_file()!=FALSE);
	main_start_time();

	return;
}

void game_state_revert(
	void)
{
	if (!game_state_globals.saved_game_valid && !recover_saved_games_hack)
	{
		main_reset_map();

		return;
	}

	game_state_call_before_load_procs();
	game_state_read_from_file();
	game_state_call_after_load_procs();

	return;
}

void game_state_save_to_persistent_storage(
	void)
{
	if (player_spawn_count==1)
	{
		game_state_revert();
		game_state_write_to_persistent_storage(
			game_state_globals.base_address,
			&game_state_globals.header->checksum,
			sizeof(*game_state_globals.header),
			0x345000);
	}

	return;
}

boolean game_state_test_persistent_storage(
	char *map_name,
	short *difficulty,
	boolean *corrupted)
{
	struct game_state_header header;
	boolean success;

	if (game_state_read_header_from_persistent_storage(
		&header,
		&header.checksum,
		sizeof(*game_state_globals.header),
		0x345000,
		corrupted))
	{
		*difficulty = header.difficulty;
		strcpy(map_name, header.map_name);

		success = TRUE;
	}
	else
	{
		*difficulty = _game_difficulty_level_normal;
		strcpy(map_name, "");

		success = FALSE;
	}

	return success;
}

void game_state_save_core(
	const char *name)
{
	if (game_state_write_core(name, game_state_globals.base_address, 0x345000))
	{
		console_printf(FALSE, "saved '%s'", name);
	}
	else
	{
		console_printf(FALSE, "error writing '%s'", name);
	}
}

boolean game_state_reverted(
	void)
{
	return (game_state_globals.revert_time==game_time_get());
}

// _code_001af4f0
// _code_001af650

static void game_state_set_revert_time(
	void)
{
	game_state_globals.revert_time = game_time_get();
	game_time_set_paused(FALSE);

	return;
}

void *game_state_malloc(
	const char *name,
	const char *type,
	long size)
{
	byte *pointer;
	FILE *file;

	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 153, !(size&3));
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 156, !game_state_globals.locked);
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 159, game_state_globals.cpu_allocation_size+size<=GAME_STATE_CPU_SIZE);

	file = bss_004d27b0;

	if (!file)
	{
		file = fopen("d:\\gamestate.txt", "w");
		bss_004d27b0 = file;
	}

	if (file)
	{
		fprintf(file, "% 40s% 20s% 10d%s\n", name, type, size, "");
		fflush(bss_004d27b0);
	}

	pointer = (byte *)game_state_globals.base_address+game_state_globals.cpu_allocation_size;
	game_state_globals.cpu_allocation_size+= size;

	crc_checksum_buffer((unsigned long *)&game_state_globals.allocation_size_checksum, &size, sizeof(size));

	return pointer;
}

/* ---------- private code */
