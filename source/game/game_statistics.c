/*
GAME_STATISTICS.C

symbols in this file:
000A49B0 0010:
	_game_statistics_start (0000)
000A49C0 0090:
	_game_statistics_stop (0000)
000A4A50 0010:
	_game_statistics_record_damage (0000)
000A4A60 02a0:
	_game_statistics_record_kill (0000)
0043EE00 0001:
	_bss_0043ee00 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "players.h"
#include "network_game_globals.h"
#include "units.h"
#include "game.h"
#include "game_engine.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static struct
{
	boolean active;
} game_statistics_globals;

/* ---------- public code */

void game_statistics_start(
	void)
{
	game_statistics_globals.active = TRUE;

	return;
}

void game_statistics_stop(
	short winning_team_index)
{
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);

	while (player = (struct player_datum *)data_iterator_next(&iterator))
	{
		player->statistics.seconds_online = game_time_get() / TICKS_PER_SECOND;
		player->statistics.games_played = 1;

		if (player->team_index == winning_team_index)
		{
			player->statistics.games_won++;
		}
	}

	game_statistics_globals.active = FALSE;

	return;
}

void game_statistics_record_damage(
	long object_index,
	real damage,
	long owner_player_index,
	long owner_object_index,
	short owner_team_index)
{
	return;
}

void game_statistics_record_kill(
	long object_index,
	long owner_player_index,
	long owner_object_index,
	short owner_team_index)
{
	if (game_statistics_globals.active)
	{
		long dead_player_index = player_index_from_unit_index(object_index);

		if (dead_player_index != NONE)
		{
			boolean friendly_fire;
			long multiple_kill_time;
			long assist_time;
			struct unit_datum *unit;
			struct unit_attacker *attacker;
			short attacker_index;
			short best_attacker_index;
			short owner_attacker_index;
			real best_damage;
			long killing_player_index;
			real assist_damage;
			boolean betrayal;
			short kill_index = 0;
			long dead_team_index = player_get(dead_player_index)->team_index;
			struct player_datum *dead_player = player_get(dead_player_index);

			if (owner_player_index == dead_player_index)
			{
				dead_player->statistics.suicides++;
			}

			dead_player->statistics.deaths++;
			dead_player->statistics.kills_in_a_row = 0;
			dead_player->statistics.last_kill_time = NONE;
			dead_player->statistics.multiple_kills = 0;

			friendly_fire = (owner_player_index != NONE && game_team_is_enemy((short)dead_team_index, owner_team_index)) ? FALSE : TRUE;
			multiple_kill_time = game_time_get() - 120;
			assist_time = game_time_get() - 180;
			unit = unit_get(object_index);
			best_attacker_index = NONE;
			best_damage = -REAL_MAX;
			owner_attacker_index = NONE;

			for (attacker_index = 0, attacker = unit->unit.attackers; attacker_index < MAXIMUM_ATTACKERS_PER_UNIT; attacker_index++, attacker++)
			{
				if (attacker->player_index == owner_player_index)
				{
					owner_attacker_index = attacker_index;
				}

				if ((owner_attacker_index == attacker_index || (friendly_fire && game_team_is_enemy((short)dead_team_index, (short)attacker->player_index))) &&
					attacker->game_time_stamp > (unsigned long)assist_time &&
					attacker->damage_inflicted > best_damage)
				{
					best_attacker_index = attacker_index;
					best_damage = attacker->damage_inflicted;
				}
			}

			betrayal = FALSE;

			if (best_attacker_index == NONE)
			{
				best_attacker_index = owner_attacker_index;
			}

			if (best_attacker_index != NONE)
			{
				killing_player_index = unit->unit.attackers[best_attacker_index].player_index;
				assist_damage = unit->unit.attackers[best_attacker_index].damage_inflicted * 0.4f;
			}
			else
			{
				killing_player_index = owner_player_index;
				assist_damage = 0.f;
			}

			if (killing_player_index != NONE)
			{
				struct player_datum *killing_player = player_get(killing_player_index);

				if (game_team_is_enemy((short)dead_team_index, (short)killing_player->team_index))
				{
					killing_player->statistics.kills[kill_index]++;

					if (kill_index == 0)
					{
						killing_player->statistics.kills_in_a_row++;

						if (killing_player->statistics.last_kill_time >= multiple_kill_time)
						{
							killing_player->statistics.multiple_kills++;
						}
						else
						{
							killing_player->statistics.multiple_kills = 1;
						}

						killing_player->statistics.last_kill_time = (short)game_time_get();
					}
				}
				else
				{
					killing_player->statistics.friendly_fire_kills++;
					betrayal = TRUE;
				}
			}

			if (assist_damage > 0.f)
			{
				for (attacker_index = 0, attacker = unit->unit.attackers; attacker_index < MAXIMUM_ATTACKERS_PER_UNIT; attacker_index++, attacker++)
				{
					if ((attacker_index == owner_attacker_index || attacker->damage_inflicted > assist_damage) &&
						attacker->player_index != NONE &&
						attacker->player_index != killing_player_index)
					{
						struct player_datum *assisting_player = player_get(attacker->player_index);

						if (game_team_is_enemy((short)dead_team_index, (short)assisting_player->team_index))
						{
							assisting_player->statistics.assists[kill_index]++;
						}
					}
				}
			}

			game_engine_player_killed(killing_player_index, owner_object_index, dead_player_index, betrayal);
		}
	}

	return;
}

/* ---------- private code */
