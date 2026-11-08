/*
GAME_STATE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "game_state.h"
#include "game_state_procs.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "console.h"
#include "actors.h"
#include "sound_manager.h"
#include "ai_debug.h"
#include "main.h"
#include "director.h"
#include "game_sound.h"
#include "structures.h"
#include "observer.h"
#include "hud.h"
#include "hud_messaging.h"
#include "physical_memory_map.h"
#include "build_number.h"
#include "recorded_animations.h"
#include "saved_films.h"
#include "rasterizer_hardware_format_utilities.h"

/* ---------- constants */

enum
{
	GAME_STATE_CPU_SIZE = 0x305000,
	GAME_STATE_GPU_SIZE = 0x40000,
	GAME_STATE_SIZE = GAME_STATE_CPU_SIZE + GAME_STATE_GPU_SIZE,	/* fake name */
};

/* ---------- macros */

#define GAME_STATE_ALLOCATION_FILE_PATH "d:\\gamestate.txt"	/* fake name */

/* ---------- prototypes */

static void game_state_call_before_save_procs(void);
static void game_state_call_before_load_procs(void);
static void game_state_call_after_load_procs(void);
static boolean game_state_header_valid(struct game_state_header const *header, boolean fatal);
static void game_state_allocation_record(char const *name, char const *type, long size, boolean gpu);
static void game_state_set_revert_time(void);

/* ---------- globals */

boolean recover_saved_games_hack;

static struct
{
	void *base_address;
	long cpu_allocation_size;
	long gpu_allocation_size;
	unsigned long allocation_size_checksum;
	boolean locked;
	boolean saved_game_valid;
	long revert_time;
	struct game_state_header *header;
} game_state_globals;

static FILE *game_state_allocation_file; /* fake name */

static void (*before_save_procs[])(void) =
{
	dummy
};

static void (*before_load_procs[])(void) =
{
	game_sound_clear
};

static void (*after_load_procs[])(void) =
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

static void game_state_call_before_save_procs(
	void)
{
	short proc_index;

	for (proc_index = 0; proc_index < NUMBEROF(before_save_procs); proc_index++)
	{
		before_save_procs[proc_index]();
	}

	return;
}

static void game_state_call_before_load_procs(
	void)
{
	short proc_index;

	for (proc_index = 0; proc_index < NUMBEROF(before_load_procs); proc_index++)
	{
		before_load_procs[proc_index]();
	}

	return;
}

static void game_state_call_after_load_procs(
	void)
{
	short proc_index;

	for (proc_index = 0; proc_index < NUMBEROF(after_load_procs); proc_index++)
	{
		after_load_procs[proc_index]();
	}

	return;
}

void game_state_initialize(
	void)
{
	crc_new(&game_state_globals.allocation_size_checksum);
	game_state_globals.base_address = game_state_allocate_buffer(
		GAME_STATE_BASE_ADDRESS,
		GAME_STATE_CPU_SIZE,
		GAME_STATE_GPU_SIZE);
	game_state_create_or_open_file();
	game_state_globals.header = game_state_malloc("header", NULL, sizeof(struct game_state_header));

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
	game_state_globals.locked = TRUE;
	game_state_globals.saved_game_valid = FALSE;
	game_state_globals.revert_time = NONE;

	memset(game_state_globals.header, 0, sizeof(struct game_state_header));
	strcpy(game_state_globals.header->map_name, tag_get_name(global_scenario_index));
	strcpy(game_state_globals.header->build_number, BUILD_NUMBER);
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

void *game_state_malloc(
	char const *name,
	char const *type,
	long size)
{
	void *result;

	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 153, !(size&3));
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 156, !game_state_globals.locked);
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 159, game_state_globals.cpu_allocation_size+size<=GAME_STATE_CPU_SIZE);

	game_state_allocation_record(name, type, size, FALSE);
	result = (byte *)game_state_globals.base_address + game_state_globals.cpu_allocation_size;
	game_state_globals.cpu_allocation_size += size;
	crc_checksum_buffer(&game_state_globals.allocation_size_checksum, &size, sizeof(size));

	return result;
}

void *game_state_gpu_malloc(
	char const *name,
	char const *type,
	long size)
{
	void *result;

	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 182, !(size&3));
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 185, !game_state_globals.locked);
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 188, game_state_globals.gpu_allocation_size+size<=GAME_STATE_GPU_SIZE);

	game_state_allocation_record(name, type, size, TRUE);
	game_state_globals.gpu_allocation_size += size;
	result = (byte *)game_state_globals.base_address + GAME_STATE_SIZE - game_state_globals.gpu_allocation_size;
	crc_checksum_buffer(&game_state_globals.allocation_size_checksum, &size, sizeof(size));

	return result;
}

struct data_array *game_state_data_new(
	char const *name,
	short maximum_count,
	short size)
{
	long allocation_size = data_allocation_size(maximum_count, size);
	struct data_array *data = game_state_malloc(name, "data array", allocation_size);

	data_initialize(data, name, maximum_count, size);

	return data;
}

struct memory_pool *game_state_memory_pool_new(
	char const *name,
	long size)
{
	long allocation_size = memory_pool_allocation_size(size);
	struct memory_pool *pool = game_state_malloc(name, "memory pool", allocation_size);

	memory_pool_initialize(pool, name, size);

	return pool;
}

struct lruv_cache *game_state_lruv_cache_new(
	char const *name,
	long page_count,
	long page_size_bits,
	long maximum_block_count,
	void (*delete_block_proc)(long),
	boolean (*locked_block_proc)(long))
{
	long allocation_size = lruv_allocation_size(maximum_block_count);
	struct lruv_cache *cache = game_state_malloc(name, "lruv cache", allocation_size);

	lruv_initialize(
		cache,
		name,
		page_count,
		page_size_bits,
		maximum_block_count,
		delete_block_proc,
		locked_block_proc);

	return cache;
}

void game_state_save(
	void)
{
	game_state_call_before_save_procs();
	main_stop_time();
	game_state_globals.saved_game_valid = game_state_write_to_file() != FALSE;
	main_start_time();

	return;
}

void game_state_revert(
	void)
{
	if (game_state_globals.saved_game_valid || recover_saved_games_hack)
	{
		game_state_call_before_load_procs();
		game_state_read_from_file();
		game_state_call_after_load_procs();
	}
	else
	{
		main_reset_map();
	}

	return;
}

void game_state_try_and_load_from_persistent_storage(
	void)
{
	struct game_state_header header;

	if (game_state_read_header_from_persistent_storage(
			&header,
			&header.checksum,
			sizeof(header),
			GAME_STATE_SIZE,
			NULL) &&
		game_state_header_valid(&header, FALSE) &&
		main_get_difficulty() == header.difficulty)
	{
		game_state_call_before_load_procs();
		game_state_read_from_persistent_storage(game_state_globals.base_address, GAME_STATE_SIZE);
		game_difficulty_level_set(main_get_difficulty());
		game_state_call_after_load_procs();
		game_state_save();
	}

	return;
}

void game_state_save_to_persistent_storage(
	void)
{
	if (player_spawn_count == 1)
	{
		game_state_revert();
		game_state_write_to_persistent_storage(
			game_state_globals.base_address,
			&game_state_globals.header->checksum,
			sizeof(struct game_state_header),
			GAME_STATE_SIZE);
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
		sizeof(header),
		GAME_STATE_SIZE,
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
	char const *name)
{
	if (game_state_write_core(name, game_state_globals.base_address, GAME_STATE_SIZE))
	{
		console_printf(FALSE, "saved '%s'", name);
	}
	else
	{
		console_printf(FALSE, "error writing '%s'", name);
	}

	return;
}

void game_state_load_core(
	char const *name)
{
	struct game_state_header header;

	if (game_state_read_core_header(name, &header, sizeof(header)) &&
		game_state_header_valid(&header, TRUE))
	{
		game_state_call_before_load_procs();
		game_state_read_core(name, game_state_globals.base_address, GAME_STATE_SIZE);
		console_printf(FALSE, "loaded '%s'", name);
		game_state_call_after_load_procs();
	}
	else
	{
		console_printf(FALSE, "couldn't open '%s'", name);
	}

	return;
}

boolean game_state_reverted(
	void)
{
	return game_state_globals.revert_time == game_time_get();
}

/* ---------- private code */

static boolean game_state_header_valid(
	struct game_state_header const *header,
	boolean fatal)
{
	boolean valid = FALSE;

	if (strcmp(header->build_number, BUILD_NUMBER))
	{
		if (fatal)
		{
			match_vhalt(
				"c:\\halo\\SOURCE\\saved games\\game_state.c",
				405,
				csprintf(
					temporary,
					"expected build #%d but got #%d",
					BUILD_NUMBER,
					header->build_number));
		}
	}
	else if (strcmp(header->map_name, tag_get_name(global_scenario_index)))
	{
		if (fatal)
		{
			match_vhalt(
				"c:\\halo\\SOURCE\\saved games\\game_state.c",
				409,
				csprintf(
					temporary,
					"expected \"%s\" but got \"%s\"",
					tag_get_name(global_scenario_index),
					header->map_name));
		}
	}
	else if (header->allocation_size_checksum != game_state_globals.allocation_size_checksum)
	{
		if (fatal)
		{
			match_vhalt(
				"c:\\halo\\SOURCE\\saved games\\game_state.c",
				413,
				csprintf(
					temporary,
					"allocation checksum mismatch"));
		}
	}
	else if (header->player_count != player_spawn_count)
	{
		if (fatal)
		{
			match_vhalt(
				"c:\\halo\\SOURCE\\saved games\\game_state.c",
				417,
				csprintf(
					temporary,
					"expected #%d players but got #%d",
					player_spawn_count,
					header->player_count));
		}
	}
	else if (header->cache_file_checksum != cache_files_get_checksum())
	{
		if (fatal)
		{
			match_vhalt(
				"c:\\halo\\SOURCE\\saved games\\game_state.c",
				422,
				csprintf(
					temporary,
					"checksum from map file doesn't match"));
		}
	}
	else
	{
		valid = TRUE;
	}

	return valid;
}

static void game_state_allocation_record(
	char const *name,
	char const *type,
	long size,
	boolean gpu)
{
	if (!game_state_allocation_file)
	{
		game_state_allocation_file = fopen(GAME_STATE_ALLOCATION_FILE_PATH, "w");
	}

	if (game_state_allocation_file)
	{
		fprintf(
			game_state_allocation_file,
			"% 40s% 20s% 10d%s\n",
			name,
			type,
			size,
			gpu ? "*" : "");
		fflush(game_state_allocation_file);
	}

	return;
}

static void game_state_set_revert_time(
	void)
{
	game_state_globals.revert_time = game_time_get();
	game_time_set_paused(FALSE);

	return;
}
