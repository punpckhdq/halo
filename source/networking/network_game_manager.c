/*
NETWORK_GAME_MANAGER.C

- owns the network_game_data record: the machine and player tables the server and client managers replicate
- a client builds its game objects out of that record when the game starts
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries_windows.h"
#include "errors.h"
#include "network_game_manager.h"
#include "network_game_globals.h"
#include "network_game_ui.h"
#include "game.h"
#include "game_engine.h"
#include "main.h"
#include "players.h"
#include "unicode.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static long sort_network_players(struct network_player *p1, struct network_player *p2);

/* ---------- globals */

/* ---------- public code */

void network_game_invalidate_player(
	struct network_player *player)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 88, player);

	player->machine_index= NONE;
	player->controller_index= NONE;
	player->team_index= NONE;
	player->player_list_index= NONE;
	player->name[0]= 0;

	return;
}

boolean network_game_add_machine(
	network_game_data *game,
	network_machine *machine)
{
	boolean result= FALSE;
	long machine_index;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 106,
		game && machine && network_machine_is_valid(machine));

	for (machine_index= 0; machine_index<MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		if (!network_machine_is_valid(&game->machines[machine_index]))
		{
			csmemcpy(&game->machines[machine_index], machine, sizeof(network_machine));
			game->machine_count++;
			result= TRUE;
			break;
		}
	}

	return result;
}

boolean network_game_update_machine(
	network_game_data *game,
	network_machine *machine)
{
	boolean result= FALSE;
	long machine_index;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 129,
		game && machine && network_machine_is_valid(machine));

	for (machine_index= 0; machine_index<MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		if (game->machines[machine_index].machine_index==machine->machine_index)
		{
			csmemcpy(&game->machines[machine_index], machine, sizeof(network_machine));
			result= TRUE;
			break;
		}
	}

	return result;
}

void xbox_set_machine_name(
	char *machine_name)
{
	wchar unicode_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH];

	if (machine_name && machine_name[0])
	{
		if (ascii_to_wide(machine_name, unicode_name, sizeof(unicode_name)))
		{
			unicode_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH-1]= 0;

			if (!XSetNicknameW(unicode_name, TRUE))
			{
				error(_error_silent, "XSetNickname() failed");
			}
		}
		else
		{
			error(_error_silent, "'%s' is not a valid machine name (max. name length= %d characters)",
				unicode_name, MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH-1);
		}
	}

	return;
}

void network_game_generate_local_machine_name(
	wchar machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH])
{
	HANDLE nickname_handle= XFindFirstNicknameW(FALSE, machine_name, MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH);

	if (nickname_handle==INVALID_HANDLE_VALUE)
	{
		char ascii_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH];

		ustrncpy(machine_name, network_game_get_random_player_name(), MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH);
		machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH-1]= 0;

		if (XSetNicknameW(machine_name, TRUE))
		{
			error(_error_silent, "system nickname set to '%s'",
				wide_to_ascii(machine_name, ascii_name, MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH));
		}
		else
		{
			error(_error_silent, "XSetNickname() failed to set system nickname");
		}
	}
	else
	{
		XFindClose(nickname_handle);
	}

	machine_name[MAXIMUM_NETWORK_GAME_MACHINE_NAME_LENGTH-1]= 0;

	return;
}

void network_game_end_and_load_ui(
	network_game_data *game)
{
	if (game->local_data.game_objects_loaded)
	{
		main_load_ui_scenario(TRUE);
	}

	csmemset(&game->local_data, 0, sizeof(game->local_data));

	return;
}

void network_game_reset_for_next_round(
	network_game_data *game,
	boolean load_ui)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 557, game);

	if (load_ui && game->local_data.game_objects_loaded)
	{
		network_game_end_and_load_ui(game);

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
		csmemset(&game->local_data, 0, sizeof(game->local_data));
	}

	game_time_end();

	return;
}

// nothing calls this any more
void network_game_assign_players_to_team(
	void)
{
	return;
}

boolean network_player_is_valid(
	struct network_player *player)
{
	return (boolean)(player &&
		VALID_INDEX(player->controller_index, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS) &&
		VALID_INDEX(player->machine_index, MAXIMUM_NETWORK_MACHINE_COUNT));
}

void network_game_invalidate_machine(
	network_game_data *game,
	word machine_index)
{
	long player_index;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 64,
		game && (machine_index<MAXIMUM_NETWORK_MACHINE_COUNT));

	game->machines[machine_index].machine_index= NONE;
	game->machines[machine_index].name[0]= 0;

	for (player_index= 0; player_index<NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
	{
		if (game->players[player_index].machine_index==machine_index)
		{
			network_game_invalidate_player(&game->players[player_index]);
		}
	}

	return;
}

boolean network_game_add_player(
	network_game_data *game,
	struct network_player *player)
{
	boolean result= FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 187, game && player);

	if (game->player_count<game->maximum_players)
	{
		if (VALID_INDEX(player->machine_index, MAXIMUM_NETWORK_MACHINE_COUNT) &&
			VALID_INDEX(player->controller_index, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS))
		{
			long player_index;

			for (player_index= 0; player_index<NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
			{
				if ((game->players[player_index].machine_index==player->machine_index) &&
					(game->players[player_index].controller_index==player->controller_index))
				{
					break;
				}
			}

			if (player_index==NETWORK_GAME_MAXIMUM_PLAYER_COUNT && network_player_is_valid(player))
			{
				long free_index= NONE;
				long index;

				for (index= 0; index<NETWORK_GAME_MAXIMUM_PLAYER_COUNT; index++)
				{
					if (game->players[index].player_list_index==NONE)
					{
						free_index= index;
						break;
					}
				}

				if ((player->player_list_index==NONE || free_index==player->player_list_index) &&
					free_index!=NONE)
				{
					player->player_list_index= (char)free_index;
					csmemcpy(&game->players[free_index], player, sizeof(struct network_player));
					game->player_count++;
					result= TRUE;
				}
			}
		}
	}
	else
	{
		error(_error_silent, "game is already at maximum players; can't add new player");
	}

	return result;
}

/* ---------- private code */

static long sort_network_players(
	struct network_player *p1,
	struct network_player *p2)
{
	long result= 0;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 322, p1 && p2);

	if (!network_player_is_valid(p1) && !network_player_is_valid(p2))
	{
		result= 0;
	}
	else if (!network_player_is_valid(p1) && network_player_is_valid(p2))
	{
		result= 1;
	}
	else if (network_player_is_valid(p1) && !network_player_is_valid(p2))
	{
		result= -1;
	}
	else if (p1->machine_index>p2->machine_index)
	{
		result= 1;
	}
	else if (p1->machine_index<p2->machine_index)
	{
		result= -1;
	}
	else if (p1->controller_index>p2->controller_index)
	{
		result= 1;
	}
	else if (p1->controller_index<p2->controller_index)
	{
		result= -1;
	}
	else
	{
		match_vhalt("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 357,
			"multiple players on the same machine cannot have the same controller index");
	}

	return result;
}

/* ---------- public code */

boolean network_game_spawn_player(
	struct network_player *player)
{
	short controller_index;
	long player_index;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 444,
		network_player_is_valid(player));

	if (network_game_player_is_local(player))
	{
		controller_index= player->controller_index;
	}
	else
	{
		controller_index= NONE;
	}

	player_index= player_new(player->machine_index, NONE, controller_index, player);

	if (player_index!=NONE)
	{
		player->player_list_index= (char)player_index;

		return TRUE;
	}

	return FALSE;
}

boolean network_game_player_is_valid(
	struct network_player *player,
	network_game_data *game)
{
	boolean result= FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 583, player && game);

	if (network_player_is_valid(player))
	{
		long player_index;

		for (player_index= 0; player_index<NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
		{
			if ((game->players[player_index].machine_index==player->machine_index) &&
				(game->players[player_index].controller_index==player->controller_index))
			{
				result= TRUE;
				break;
			}
		}
	}

	return result;
}

void network_game_invalidate(
	network_game_data *game)
{
	long machine_index;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 35, game);

	csmemset(game, 0, sizeof(network_game_data));
	csmemset(&game->map, 0, sizeof(game->map));

	game->machine_count= 0;
	game->player_count= 0;

	for (machine_index= 0; machine_index<MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
	{
		network_game_invalidate_machine(game, (word)machine_index);
	}

	csmemset(game->players, NONE, sizeof(game->players));

	game->minimum_players= MAXIMUM_NUMBER_OF_SCORED_TEAMS;
	game->maximum_players= NETWORK_GAME_MAXIMUM_PLAYER_COUNT;
	game->local_data.game_objects_loaded= FALSE;

	return;
}

boolean network_game_update_player(
	network_game_data *game,
	struct network_player *player)
{
	boolean result= FALSE;
	struct network_player *game_player;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 257, game && player);

	if (network_game_player_is_valid(player, game) &&
		((game_player= &game->players[player->player_list_index])->controller_index==player->controller_index) &&
		(game_player->machine_index==player->machine_index))
	{
		csmemcpy(game_player, player, sizeof(struct network_player));

		result= TRUE;
	}
	else
	{
		error(_error_silent, "tried to update a player with indvalid data");
	}

	return result;
}

boolean network_game_remove_player(
	network_game_data *game,
	struct network_player *player)
{
	boolean result= FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 288, game && player);

	if (network_game_player_is_valid(player, game))
	{
		long player_index;

		for (player_index= 0; player_index<NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
		{
			if (network_player_is_valid(&game->players[player_index]) &&
				(game->players[player_index].machine_index==player->machine_index) &&
				(game->players[player_index].controller_index==player->controller_index))
			{
				network_game_invalidate_player(&game->players[player_index]);
				game->player_count--;

				result= TRUE;
				break;
			}
		}
	}
	else
	{
		error(_error_silent, "tried to remove a player with indvalid data");
	}

	return result;
}

boolean network_game_create_game_objects(
	network_game_data *game)
{
	struct game_options options;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 368, game);

	game_options_new(&options);
	csstrncpy(options.map_name, game->map.name, MAXIMUM_NETWORK_MAP_NAME_LENGTH-1);
	options.difficulty_level= game->difficulty_level;

	switch (game_connection())
	{
		case _game_connection_network_client:
		case _game_connection_network_server:
			options.random_seed= network_game_get_random_seed();
			break;

		case _game_connection_film_playback:
			options.random_seed= game->network_game_random_seed;
			break;

		default:
			match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 383,
				!"bad game connection");
			break;
	}

	game_precache_new_map(options.map_name, TRUE);

	main_menu_unload();

	if (game_in_progress())
	{
		game_dispose_from_old_map();
		game_unload();
	}

	if (game->variant.engine_index)
	{
		game_set_game_variant(&game->variant);
	}

	if (game_load(&options))
	{
		long player_index;

		game->local_data.game_objects_loaded= TRUE;

		game_initialize_for_new_map();

		qsort(game->players, NETWORK_GAME_MAXIMUM_PLAYER_COUNT, sizeof(struct network_player),
			sort_network_players);

		for (player_index= 0; player_index<NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
		{
			if (!network_player_is_valid(&game->players[player_index])) break;

			if (!network_game_spawn_player(&game->players[player_index]))
			{
				game->local_data.game_objects_loaded= FALSE;
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

boolean network_game_remove_machine(
	network_game_data *game,
	network_machine *machine)
{
	boolean result= FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_game_manager.c", 151, game && machine);

	if (network_machine_is_valid(machine))
	{
		long machine_index;

		for (machine_index= 0; machine_index<MAXIMUM_NETWORK_MACHINE_COUNT; machine_index++)
		{
			if (game->machines[machine_index].machine_index==machine->machine_index)
			{
				long player_index;

				for (player_index= 0; player_index<NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
				{
					if (network_player_is_valid(&game->players[player_index]) &&
						(game->players[player_index].machine_index==machine->machine_index))
					{
						if (!network_game_remove_player(game, &game->players[player_index]))
						{
							error(_error_silent, "failed to remove a machine's player");
						}
					}
				}

				network_game_invalidate_machine(game, machine->machine_index);
				game->machine_count--;
				result= TRUE;
				break;
			}
		}
	}

	return result;
}
