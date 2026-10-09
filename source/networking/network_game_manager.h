/*
NETWORK_GAME_MANAGER.H

header included in hcex build.
*/

#ifndef __NETWORK_GAME_MANAGER_H
#define __NETWORK_GAME_MANAGER_H
#pragma once

/* ---------- headers */

#include "game_engine.h"

/* ---------- constants */

enum
{
	MAXIMUM_NETWORK_MACHINE_COUNT = 4,
	NETWORK_GAME_MAXIMUM_PLAYER_COUNT = 16,
	MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH = 12,
	MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH = 32,
	MAXIMUM_NETWORK_GAME_NAME_LENGTH = 16,
	MAXIMUM_NETWORK_GAME_SERVER_PASSWORD_LENGTH = 8,
	MAXIMUM_NETWORK_MAP_NAME_LENGTH = 128,
	NETWORK_SERVER_PORT = 0x141E,
	NETWORK_CLIENT_PORT = 0x141F,
	MINIMUM_NETWORK_GAME_PLAYER_COUNT = 2, /* fake name */
};

/* ---------- macros */

#define network_machine_is_valid(machine) ((machine) && (machine)->machine_index>=0 && (machine)->machine_index<MAXIMUM_NETWORK_MACHINE_COUNT)

/* ---------- structures */

struct network_player
{
	wchar_t name[MAXIMUM_NETWORK_GAME_PLAYER_NAME_LENGTH];
	short primary_color_index;
	short icon_index;
	char machine_index;
	char controller_index;
	char team_index;
	char player_list_index;
};

struct network_machine
{
	wchar_t name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH];
	char machine_index;
	byte pad[3];
};

struct network_map
{
	unsigned long version;
	char name[MAXIMUM_NETWORK_MAP_NAME_LENGTH];
};

struct network_game_data
{
	wchar_t name[MAXIMUM_NETWORK_GAME_NAME_LENGTH];
	struct network_map map;
	struct game_variant variant;
	char _unused_game_engine;
	char minimum_players; /* fake name */
	char maximum_players;
	char maximum_teams; /* fake name */
	short difficulty_level;
	short machine_count;
	struct network_machine machines[MAXIMUM_NETWORK_MACHINE_COUNT];
	short player_count;
	struct network_player players[NETWORK_GAME_MAXIMUM_PLAYER_COUNT];
	word pad;
	unsigned long network_game_random_seed;
	long number_of_games_played;
	struct
	{
		boolean game_objects_loaded;
		byte pad[3];
	} local_data;
};

/* ---------- prototypes/NETWORK_GAME_MANAGER.C */

void network_game_invalidate(struct network_game_data *game);
void network_game_invalidate_machine(struct network_game_data *game, word machine_index);
void network_game_invalidate_player(struct network_player *player);
boolean network_game_add_machine(struct network_game_data *game, struct network_machine *machine);
boolean network_game_update_machine(struct network_game_data *game, struct network_machine *machine);
boolean network_game_remove_machine(struct network_game_data *game, struct network_machine *machine);
boolean network_game_add_player(struct network_game_data *game, struct network_player *player);
boolean network_game_update_player(struct network_game_data *game, struct network_player *player);
boolean network_game_remove_player(struct network_game_data *game, struct network_player *player);
boolean network_game_create_game_objects(struct network_game_data *game);
boolean network_game_spawn_player(struct network_player *player);
void xbox_set_machine_name(char const *machine_name);
void network_game_generate_local_machine_name(wchar_t *machine_name);
void network_game_end_and_load_ui(struct network_game_data *game);
void network_game_reset_for_next_round(struct network_game_data *game, boolean unload_game_objects);
boolean network_game_player_is_valid(struct network_player *player, struct network_game_data *game);
void network_game_assign_players_to_team(struct network_game_data *game);
boolean network_player_is_valid(struct network_player *player);

/* ---------- globals */

/* ---------- public code */

#endif // __NETWORK_GAME_MANAGER_H
