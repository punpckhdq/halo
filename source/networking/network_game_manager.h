/*
NETWORK_GAME_MANAGER.H

header included in hcex build.
*/

#ifndef __NETWORK_GAME_MANAGER_H
#define __NETWORK_GAME_MANAGER_H
#pragma once

/* ---------- headers */

#include "errors.h"
#include "players.h"
#include "data.h"
#include "game_engine.h"

typedef wchar_t wchar;

wchar *ustrncpy(wchar *s1, const wchar *s2, unsigned long size);
long ustrcmp(const wchar *s1, const wchar *s2);
char *wide_to_ascii(const wchar *unicode, char *ascii, unsigned long size);

typedef struct network_player network_player;

enum
{
	MAXIMUM_NUMBER_OF_LOCAL_PLAYERS= 4
};

struct player_action_collection
{
	struct player_action actions[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
}; // 0x80

struct game_update
{
	word number_of_actions;
	word pad;
	struct player_action actions[16]; // NETWORK_GAME_MAXIMUM_PLAYER_COUNT
}; // 0x204

void update_server_next_update(void);
void update_server_build_server_update(long machine_index, struct game_update *update, long *update_number);
void update_server_handle_client_update(long machine_index, struct player_action_collection *actions);
void update_server_add_player(long player_index);
void update_client_handle_server_update(struct game_update *update, long update_number);
void update_client_build_client_update(struct player_action_collection *action_collection);
void update_client_add_player(long player_index);
short local_player_get_next(short local_player_index);
void local_player_set_player_index(short local_player_index, long player_index);

struct player_profile
{
	wchar player_name[12]; // 0x00
	short primary_color_index; // 0x18
	char pad_1A[22]; // 0x1A, the tail player_ui.c owns: solo level bits at 0x1C, last_single_player_level_played at 0x26, the input_abstraction preferences at 0x28..0x2D and autolevel_enabled at 0x2E
}; // 0x30
long player_profile_get_random_color(void);
long player_profile_get_random_good_color(void);

void player_ui_get_active_player_profile(short local_player_index, struct player_profile *profile);

// returns NONE if the player could not be added
long player_new(long machine_index, long player_index, short controller_index, struct network_player *player);

#define REQUIRE_CACHE_FILE // keeps the precache assert in have_all_machines_have_precached
boolean cache_files_give_time_to_precache(char *map_name);
boolean cache_files_precache_map_loaded(char *map_name);

char *main_get_multiplayer_map_name(void);
void main_menu_unload(void);
void main_set_multiplayer_map_name(char *map_name);
short main_get_difficulty(void);

enum // display_error() codes the networking units raise
{
	_error_network_server_shut_down= 4,
	_error_network_connection_lost= 6,
	_error_network_failed_to_join_game= 7,
	_error_network_out_of_sync_alert= 8,
	_error_network_trouble_is_brewing= 9,
};

unsigned long get_random_seed(void);

/* ---------- constants */

enum
{
	MAXIMUM_NETWORK_MACHINE_COUNT= 4,
	NETWORK_GAME_MAXIMUM_PLAYER_COUNT= 16,
	MAXIMUM_NETWORK_GAME_NAME_LENGTH= 16,
	MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH= 32,
	MAXIMUM_NETWORK_MAP_NAME_LENGTH= 128,
};

/* ---------- macros */

#define network_machine_is_valid(machine) ((machine) && ((machine)->machine_index>=0) && ((machine)->machine_index<MAXIMUM_NETWORK_MACHINE_COUNT))

/* ---------- structures */

enum
{
	MAXIMUM_NUMBER_OF_SCORED_TEAMS= 2
};

typedef struct network_machine
{
	wchar name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH]; // 0x00
	char machine_index; // 0x40, NONE when the slot is free
	char pad_41[3]; // 0x41, rounds the record out to 0x44; machines only ever move whole and nothing reads these three
} network_machine; // 0x44

typedef struct network_game_data
{
	wchar name[MAXIMUM_NETWORK_GAME_NAME_LENGTH]; // 0x000
	struct
	{
		long version; // 0x020
		char name[MAXIMUM_NETWORK_MAP_NAME_LENGTH]; // 0x024
	} map;
	struct game_variant variant; // 0x0A4
	char pad_10C; // 0x10C, zero-filled by network_game_invalidate: nothing writes it and nothing reads it
	char minimum_players; // 0x10D
	char maximum_players; // 0x10E
	char maximum_teams; // 0x10F
	short difficulty_level; // 0x110
	short machine_count; // 0x112
	network_machine machines[MAXIMUM_NETWORK_MACHINE_COUNT]; // 0x114
	short player_count; // 0x224
	struct network_player players[NETWORK_GAME_MAXIMUM_PLAYER_COUNT]; // 0x226
	char pad_426[2]; // 0x426, aligns network_game_random_seed; nothing reads or writes it
	long network_game_random_seed; // 0x428
	long number_of_games_played; // 0x42C
	struct
	{
		boolean game_objects_loaded; // 0x430
		char pad_431[3]; // 0x431, rounds network_game_data out to 0x434; nothing reads or writes it
	} local_data; // 0x430
} network_game_data; // 0x434

/* ---------- prototypes/NETWORK_GAME_MANAGER.C */

void network_game_invalidate(network_game_data *game);
void network_game_invalidate_machine(network_game_data *game, word machine_index);
void network_game_invalidate_player(struct network_player *player);
boolean network_game_add_machine(network_game_data *game, network_machine *machine);
boolean network_game_update_machine(network_game_data *game, network_machine *machine);
boolean network_game_remove_machine(network_game_data *game, network_machine *machine);
boolean network_game_add_player(network_game_data *game, struct network_player *player);
boolean network_game_update_player(network_game_data *game, struct network_player *player);
boolean network_game_remove_player(network_game_data *game, struct network_player *player);
boolean network_game_spawn_player(struct network_player *player);
boolean network_player_is_valid(struct network_player *player);
boolean network_game_player_is_valid(struct network_player *player, network_game_data *game);
boolean network_game_create_game_objects(network_game_data *game);
void network_game_reset_for_next_round(network_game_data *game, boolean load_ui);
void network_game_generate_local_machine_name(wchar machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH]);
void network_game_end_and_load_ui(network_game_data *game);
void network_game_assign_players_to_team(void);

/* ---------- prototypes/GAME_ENGINE.C */

boolean game_engine_get_current_stage(struct game_variant *variant, char *map_name);
void game_engine_switch_to_postgame(void);
void game_engine_update(void);

/* ---------- globals */

/* ---------- public code */

#endif // __NETWORK_GAME_MANAGER_H
