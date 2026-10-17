/*
NETWORK_GAME_MANAGER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "network_game_manager.h"
#include "network_game_globals.h"
#include "network_messages.h"
#include "network_client_manager.h"
#include "network_server_message_handler.h"
#include "network_server_manager.h"
#include "network_game_ui.h"
#include "units.h"
#include "players.h"
#include "main.h"
#include "game_engine_list.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static long sort_network_players(struct network_player *p1, struct network_player *p2);

/* ---------- globals */

/* ---------- public code */

void network_game_invalidate(
	struct network_game_data *game)
{
	long machine_index;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 35, game);

	memset(game, 0, sizeof(*game));
	memset(&game->map, 0, sizeof(game->map));
	game->machine_count = 0;
	game->player_count = 0;

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		network_game_invalidate_machine(game, machine_index);
	}

	memset(game->players, NONE, sizeof(game->players));
	game->minimum_players = MINIMUM_NETWORK_GAME_PLAYER_COUNT;
	game->maximum_players = NETWORK_GAME_MAXIMUM_PLAYER_COUNT;
	game->local_data.game_objects_loaded = FALSE;

	return;
}

void network_game_invalidate_machine(
	struct network_game_data *game,
	word machine_index)
{
	long player_index;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 64, game && (machine_index<MAXIMUM_NETWORK_MACHINE_COUNT));

	game->machines[machine_index].machine_index = NONE;
	game->machines[machine_index].name[0] = 0;

	for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
	{
		if (game->players[player_index].machine_index == machine_index)
		{
			network_game_invalidate_player(&game->players[player_index]);
		}
	}

	return;
}

void network_game_invalidate_player(
	struct network_player *player)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 88, player);

	player->machine_index = NONE;
	player->controller_index = NONE;
	player->team_index = NONE;
	player->player_list_index = NONE;
	player->name[0] = 0;

	return;
}

boolean network_game_add_machine(
	struct network_game_data *game,
	struct network_machine *machine)
{
	long machine_index;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 106, game && machine && network_machine_is_valid(machine));

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		if (!network_machine_is_valid(&game->machines[machine_index]))
		{
			memcpy(&game->machines[machine_index], machine, sizeof(*machine));
			game->machine_count++;
			success = TRUE;
			break;
		}
	}

	return success;
}

boolean network_game_update_machine(
	struct network_game_data *game,
	struct network_machine *machine)
{
	long machine_index;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 129, game && machine && network_machine_is_valid(machine));

	for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		if (game->machines[machine_index].machine_index == machine->machine_index)
		{
			memcpy(&game->machines[machine_index], machine, sizeof(*machine));
			success = TRUE;
			break;
		}
	}

	return success;
}

boolean network_game_remove_machine(
	struct network_game_data *game,
	struct network_machine *machine)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 151, game && machine);

	if (network_machine_is_valid(machine))
	{
		long machine_index;

		for (machine_index = 0; machine_index < MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
		{
			if (game->machines[machine_index].machine_index == machine->machine_index)
			{
				long player_index;

				for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
				{
					if (network_player_is_valid(&game->players[player_index]) &&
						game->players[player_index].machine_index == machine->machine_index &&
						!network_game_remove_player(game, &game->players[player_index]))
					{
						error(_error_silent, "failed to remove a machine's player");
					}
				}

				network_game_invalidate_machine(game, machine->machine_index);
				game->machine_count--;
				success = TRUE;
				break;
			}
		}
	}

	return success;
}

boolean network_game_add_player(
	struct network_game_data *game,
	struct network_player *player)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 187, game && player);

	if (game->player_count < game->maximum_players)
	{
		if (player->machine_index >= 0 && player->machine_index < MAXIMUM_NETWORK_MACHINE_COUNT &&
			player->controller_index >= 0 && player->controller_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
		{
			long player_index;

			for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
			{
				if (game->players[player_index].machine_index == player->machine_index &&
					game->players[player_index].controller_index == player->controller_index)
				{
					break;
				}
			}

			if (player_index == NETWORK_GAME_MAXIMUM_PLAYER_COUNT && network_player_is_valid(player))
			{
				long new_player_index = NONE;

				for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
				{
					if (game->players[player_index].player_list_index == NONE)
					{
						new_player_index = player_index;
						break;
					}
				}

				if ((player->player_list_index == NONE || new_player_index == player->player_list_index) && new_player_index != NONE)
				{
					player->player_list_index = (char)new_player_index;
					memcpy(&game->players[new_player_index], player, sizeof(*player));
					game->player_count++;
					success = TRUE;
				}
			}
		}
	}
	else
	{
		error(_error_silent, "game is already at maximum players; can't add new player");
	}

	return success;
}

boolean network_game_update_player(
	struct network_game_data *game,
	struct network_player *player)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 257, game && player);

	if (network_game_player_is_valid(player, game))
	{
		struct network_player *game_player = &game->players[player->player_list_index];

		if (game_player->controller_index == player->controller_index &&
			game_player->machine_index == player->machine_index)
		{
			memcpy(game_player, player, sizeof(*game_player));
			success = TRUE;
		}
	}

	if (!success)
	{
		error(_error_silent, "tried to update a player with indvalid data");
	}

	return success;
}

boolean network_game_remove_player(
	struct network_game_data *game,
	struct network_player *player)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 288, game && player);

	if (network_game_player_is_valid(player, game))
	{
		long player_index;

		for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
		{
			if (network_player_is_valid(&game->players[player_index]) &&
				game->players[player_index].machine_index == player->machine_index &&
				game->players[player_index].controller_index == player->controller_index)
			{
				network_game_invalidate_player(&game->players[player_index]);
				game->player_count--;
				success = TRUE;
				break;
			}
		}
	}
	else
	{
		error(_error_silent, "tried to remove a player with indvalid data");
	}

	return success;
}

static long sort_network_players(
	struct network_player *p1,
	struct network_player *p2)
{
	long result = 0;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 322, p1 && p2);

	if (!network_player_is_valid(p1) && !network_player_is_valid(p2))
	{
		result = 0;
	}
	else if (!network_player_is_valid(p1) && network_player_is_valid(p2))
	{
		result = 1;
	}
	else if (network_player_is_valid(p1) && !network_player_is_valid(p2))
	{
		result = -1;
	}
	else if (p1->machine_index > p2->machine_index)
	{
		result = 1;
	}
	else if (p1->machine_index < p2->machine_index)
	{
		result = -1;
	}
	else if (p1->controller_index > p2->controller_index)
	{
		result = 1;
	}
	else if (p1->controller_index < p2->controller_index)
	{
		result = -1;
	}
	else
	{
		match_vhalt("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 357, "multiple players on the same machine cannot have the same controller index");
	}

	return result;
}

boolean network_game_create_game_objects(
	struct network_game_data *game)
{
	struct game_options options;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 368, game);

	game_options_new(&options);
	strncpy(options.map_name, game->map.name, NUMBEROF(game->map.name) - 1);
	options.difficulty = game->difficulty_level;

	switch (game_connection())
	{
	case _game_connection_network_client:
	case _game_connection_network_server:
		options.random_seed = network_game_get_random_seed();
		break;
	case _game_connection_film_playback:
		options.random_seed = game->network_game_random_seed;
		break;
	default:
		match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 383, !"bad game connection");
		break;
	}

	game_precache_new_map(options.map_name, TRUE);
	main_menu_unload();

	if (game_in_progress())
	{
		game_dispose_from_old_map();
		game_unload();
	}

	if (game->variant.game_engine_index)
	{
		game_set_game_variant(&game->variant);
	}

	if (game_load(&options))
	{
		long player_index;

		game->local_data.game_objects_loaded = TRUE;
		game_initialize_for_new_map();
		qsort(
			game->players,
			NUMBEROF(game->players),
			sizeof(struct network_player),
			(int(__cdecl *)(const void *, const void *))sort_network_players);

		for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
		{
			if (!network_player_is_valid(&game->players[player_index]))
			{
				break;
			}

			if (!network_game_spawn_player(&game->players[player_index]))
			{
				game->local_data.game_objects_loaded = FALSE;
				break;
			}
		}
	}
	else
	{
		error(_error_immediate, "game_load() failed.");
	}

	return game->local_data.game_objects_loaded;
}

boolean network_game_spawn_player(
	struct network_player *player)
{
	long player_index;
	short local_player_index;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 444, network_player_is_valid(player));

	local_player_index = network_game_player_is_local(player) ? player->controller_index : NONE;
	player_index = player_new(player->machine_index, NONE, local_player_index, player);

	if (player_index != NONE)
	{
		player->player_list_index = (char)player_index;

		return TRUE;
	}

	return FALSE;
}

void xbox_set_machine_name(
	char const *machine_name)
{
	wchar_t wide_machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH];

	if (machine_name && machine_name[0])
	{
		if (ascii_to_wide(machine_name, wide_machine_name, sizeof(wide_machine_name)))
		{
			wide_machine_name[NUMBEROF(wide_machine_name) - 1] = 0;

			if (!XSetNicknameW(wide_machine_name, TRUE))
			{
				error(_error_silent, "XSetNickname() failed");
			}
		}
		else
		{
			error(
				_error_silent,
				"'%s' is not a valid machine name (max. name length= %d characters)",
				wide_machine_name,
				NUMBEROF(wide_machine_name) - 1);
		}
	}

	return;
}

void network_game_generate_local_machine_name(
	wchar_t *machine_name)
{
	char ascii_machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH];
	HANDLE find_handle = XFindFirstNicknameW(FALSE, machine_name, MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH);

	if (find_handle == INVALID_HANDLE_VALUE)
	{
		ustrncpy(machine_name, network_game_get_random_player_name(), MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH);
		machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH - 1] = 0;

		if (XSetNicknameW(machine_name, TRUE))
		{
			error(
				_error_silent,
				"system nickname set to '%s'",
				wide_to_ascii(machine_name, ascii_machine_name, NUMBEROF(ascii_machine_name)));
		}
		else
		{
			error(_error_silent, "XSetNickname() failed to set system nickname");
		}
	}
	else
	{
		XFindClose(find_handle);
	}

	machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH - 1] = 0;

	return;
}

void network_game_end_and_load_ui(
	struct network_game_data *game)
{
	if (game->local_data.game_objects_loaded)
	{
		main_load_ui_scenario(TRUE);
	}

	memset(&game->local_data, 0, sizeof(game->local_data));

	return;
}

void network_game_reset_for_next_round(
	struct network_game_data *game,
	boolean unload_game_objects)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 557, game);

	if (unload_game_objects && game->local_data.game_objects_loaded)
	{
		main_load_ui_scenario(TRUE);
		memset(&game->local_data, 0, sizeof(game->local_data));

		if (global_network_game_server_get())
		{
			game_connection_set(_game_connection_network_server);
		}
		else if (global_network_game_client_get())
		{
			game_connection_set(_game_connection_network_client);
		}
	}
	else
	{
		memset(&game->local_data, 0, sizeof(game->local_data));
	}

	game_time_end();

	return;
}

boolean network_game_player_is_valid(
	struct network_player *player,
	struct network_game_data *game)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 583, player && game);

	if (network_player_is_valid(player))
	{
		long player_index;

		for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
		{
			if (game->players[player_index].machine_index == player->machine_index &&
				game->players[player_index].controller_index == player->controller_index)
			{
				success = TRUE;
				break;
			}
		}
	}

	return success;
}

void network_game_assign_players_to_team(
	struct network_game_data *game)
{
	return;
}

boolean network_player_is_valid(
	struct network_player *player)
{
	if (player &&
		player->controller_index >= 0 && player->controller_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS &&
		player->machine_index >= 0 && player->machine_index < MAXIMUM_NETWORK_MACHINE_COUNT)
	{
		return TRUE;
	}

	return FALSE;
}

/* ---------- private code */
