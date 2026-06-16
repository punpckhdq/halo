/*
GAME_STATE.H

header included in hcex build.
*/

#ifndef __GAME_STATE_H
#define __GAME_STATE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct game_state_header
{
	unsigned long allocation_size_checksum; // 0x0
	char map_name[256]; // 0x4
	char build_number[32]; // 0x104 TAG_STRING_LENGTH+1
	short player_count; // 0x124
	short difficulty; // 0x126
	unsigned long cache_file_checksum; // 0x128
	long unused[7]; // 0x12C
	unsigned long checksum; // 0x148
};

/* ---------- prototypes/GAME_STATE.C */

void game_state_dispose(void);
void game_state_initialize_for_new_map(void);
void game_state_dispose_from_old_map(void);
void game_state_save(void);
void game_state_revert(void);
void game_state_save_to_persistent_storage(void);
boolean game_state_test_persistent_storage(char *map_name, short *difficulty, boolean *corrupted);
void game_state_save_core(const char *name);
boolean game_state_reverted(void);
void *game_state_malloc(const char *name, const char *type, long size);
void *game_state_gpu_malloc(const char *name, const char *type, long size);
struct data_array *game_state_data_new(const char *name, short maximum_count, short size);
struct memory_pool *game_state_memory_pool_new(const char *name, long size);
/* needs lruv_cache types? */
struct lruv_cache *game_state_lruv_cache_new(const char *name, long page_count, long page_size_bits, long maximum_block_count, void (*delete_block_proc)(long), boolean (*locked_block_proc)(long));
void game_state_try_and_load_from_persistent_storage(void);
void game_state_load_core(const char *name);
void game_state_initialize(void);

struct data_array *game_state_data_new(const char *name, short maximum_count, short size);

/* ---------- prototypes/GAME_STATE_XBOX.C */

void *game_state_allocate_buffer(unsigned long address, unsigned long cpu_size, unsigned long gpu_size);
void game_state_free_buffer(void);
void game_state_create_or_open_file(void);
void game_state_close_file(void);
boolean game_state_write_to_file(void);
boolean game_state_read_from_file(void);
boolean game_state_write_core(const char *name, void *buffer, long buffer_size);
boolean game_state_read_core_header(const char *name, void *header, long header_size);
void game_state_read_core(const char *name, void *buffer, long buffer_size);
const char *game_state_get_persistent_storage_filename(void);
boolean game_state_read_header_from_persistent_storage(void *header, unsigned long *header_checksum, long header_size, long buffer_size, boolean *corrupted);
void game_state_write_to_persistent_storage(void *buffer, unsigned long *header_checksum, long header_size, long buffer_size);
void game_state_read_from_persistent_storage(void *buffer, long buffer_size);
void game_state_create_persistent_storage(const char *path);

/* ---------- globals */

extern boolean recover_saved_games_hack;

/* ---------- public code */

#endif // __GAME_STATE_H
