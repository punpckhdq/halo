/*
GAME_ALLEGIANCE.C

symbols in this file:
00095FB0 0030:
	_game_allegiance_initialize (0000)
00095FE0 0010:
	_game_allegiance_dispose (0000)
00095FF0 00a0:
	_game_allegiance_initialize_for_new_map (0000)
00096090 0010:
	_game_allegiance_dispose_from_old_map (0000)
000960A0 0060:
	_game_team_is_enemy (0000)
00096100 0050:
	_game_team_is_ally (0000)
00096150 0060:
	_game_team_ally_status_changed (0000)
000961B0 0080:
	_game_allegiance_get_incidents (0000)
00096230 0070:
	_game_allegiance_provoke (0000)
000962A0 0060:
	_game_allegiance_notify_change (0000)
00096300 01a0:
	_game_allegiance_broken (0000)
000964A0 0080:
	_game_allegiance_update (0000)
00096520 00e0:
	_game_allegiance_create (0000)
00096600 00b0:
	_game_allegiance_remove (0000)
000966B0 00d0:
	_game_allegiance_incident (0000)
0025B0D0 0018:
	??_C@_0BI@FALCEHFN@game?5allegiance?5globals?$AA@ (0000)
0025B0E8 0018:
	??_C@_0BI@MMPFGAEA@game_allegiance_globals?$AA@ (0000)
0025B100 0026:
	??_C@_0CG@MBPCBMLP@c?3?2halo?2SOURCE?2game?2game_allegia@ (0000)
0025B128 0022:
	??_C@_0CC@EJEFDMDC@allegiance?9?$DOcurrent_incidents?5?$DO?5@ (0000)
0025B14C 003d:
	??_C@_0DN@FFLAEGAA@game_allegiance_create?3?5too?5many@ (0000)
002DE3B8 0028:
	_global_game_team_names (0000)
0043E490 0004:
	_bss_0043e490 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "cheats.h"
#include "game.h"
#include "game_engine.h"
#include "ai.h"
#include "network_game_globals.h"
#include "game_state.h"

/* ---------- constants */

enum
{
	MAXIMUM_NUMBER_OF_ALLEGIANCES = 8 /* fake name */
};

/* ---------- macros */

#define TEAM_PAIR_INDEX(team1, team2) ((team1) * NUMBER_OF_SOLO_CAMPAIGN_TEAMS + (team2)) /* fake name */

/* ---------- structures */

struct game_allegiance
{
	short team1_index;
	short team2_index;
	short incident_threshold;
	short incident_decay_time;
	boolean team1_suspicious;
	boolean team2_suspicious;
	boolean currently_broken;
	boolean status_changed;
	boolean requires_communication;
	short current_incidents;
	short current_incident_decay_time;
};

struct game_allegiance_globals /* fake name */
{
	short allegiance_count;
	struct game_allegiance allegiances[MAXIMUM_NUMBER_OF_ALLEGIANCES];
	unsigned long ally_bitvector[BIT_VECTOR_SIZE_IN_LONGS(NUMBER_OF_SOLO_CAMPAIGN_TEAMS * NUMBER_OF_SOLO_CAMPAIGN_TEAMS)];
	unsigned long friendly_bitvector[BIT_VECTOR_SIZE_IN_LONGS(NUMBER_OF_SOLO_CAMPAIGN_TEAMS * NUMBER_OF_SOLO_CAMPAIGN_TEAMS)];
};

/* ---------- prototypes */

static void game_allegiance_broken(struct game_allegiance *allegiance, boolean currently_broken, boolean permanently_broken);

/* ---------- globals */

char const *global_game_team_names[NUMBER_OF_SOLO_CAMPAIGN_TEAMS] =
{
	"default",
	"player",
	"human",
	"covenant",
	"flood",
	"sentinel",
	"unused6",
	"unused7",
	"unused8",
	"unused9"
};

static struct game_allegiance_globals *game_allegiance_globals;

/* ---------- public code */

void game_allegiance_initialize(
	void)
{
	game_allegiance_globals = game_state_malloc("game allegiance globals", NULL, sizeof(*game_allegiance_globals));
	memset(game_allegiance_globals, 0, sizeof(*game_allegiance_globals));

	return;
}

void game_allegiance_dispose(
	void)
{
	return;
}

void game_allegiance_initialize_for_new_map(
	void)
{
	short index;

	match_assert("c:\\halo\\SOURCE\\game\\game_allegiance.c", 87, game_allegiance_globals);

	game_allegiance_globals->allegiance_count = 0;
	memset(game_allegiance_globals->ally_bitvector, 0, sizeof(game_allegiance_globals->ally_bitvector));
	memset(game_allegiance_globals->friendly_bitvector, 0, sizeof(game_allegiance_globals->friendly_bitvector));

	for (index = 0; index < NUMBER_OF_SOLO_CAMPAIGN_TEAMS; index++)
	{
		BIT_VECTOR_SET_FLAG(game_allegiance_globals->friendly_bitvector, TEAM_PAIR_INDEX(index, index), TRUE);
	}

	return;
}

void game_allegiance_dispose_from_old_map(
	void)
{
	return;
}

void game_allegiance_update(
	void)
{
	struct game_allegiance *allegiance;
	short index;

	for (index = 0, allegiance = game_allegiance_globals->allegiances; index < game_allegiance_globals->allegiance_count; index++, allegiance++)
	{
		if (allegiance->current_incident_decay_time > 0 && --allegiance->current_incident_decay_time == 0)
		{
			match_assert("c:\\halo\\SOURCE\\game\\game_allegiance.c", 121, allegiance->current_incidents > 0);

			if (--allegiance->current_incidents == 0)
			{
				game_allegiance_broken(allegiance, FALSE, FALSE);
			}
			else
			{
				allegiance->current_incident_decay_time = allegiance->incident_decay_time;
			}
		}
	}

	return;
}

boolean game_team_is_enemy(
	short our_team,
	short other_team)
{
	boolean enemy = TRUE;

	if (our_team >= 0 && our_team < NUMBER_OF_SOLO_CAMPAIGN_TEAMS && other_team >= 0 && other_team < NUMBER_OF_SOLO_CAMPAIGN_TEAMS)
	{
		enemy = !BIT_VECTOR_TEST_FLAG(game_allegiance_globals->friendly_bitvector, TEAM_PAIR_INDEX(our_team, other_team));
	}

	return enemy;
}

boolean game_team_is_ally(
	short our_team,
	short other_team)
{
	boolean ally = FALSE;

	if (our_team >= 0 && our_team < NUMBER_OF_SOLO_CAMPAIGN_TEAMS && other_team >= 0 && other_team < NUMBER_OF_SOLO_CAMPAIGN_TEAMS)
	{
		ally = BIT_VECTOR_TEST_FLAG(game_allegiance_globals->ally_bitvector, TEAM_PAIR_INDEX(our_team, other_team));
	}

	return ally;
}

boolean game_team_ally_status_changed(
	short our_team_index,
	short other_team_index)
{
	boolean changed = FALSE;
	struct game_allegiance *allegiance;
	short index;

	for (index = 0, allegiance = game_allegiance_globals->allegiances; index < game_allegiance_globals->allegiance_count; index++, allegiance++)
	{
		if ((allegiance->team1_index == our_team_index && allegiance->team2_index == other_team_index) ||
			(allegiance->team2_index == our_team_index && allegiance->team1_index == other_team_index))
		{
			changed = allegiance->status_changed;
			break;
		}
	}

	return changed;
}

short game_allegiance_get_incidents(
	short our_team_index,
	short other_team_index,
	short *incident_threshold)
{
	short count = 0;
	short threshold = NONE;
	struct game_allegiance *allegiance;
	short index;

	for (index = 0, allegiance = game_allegiance_globals->allegiances; index < game_allegiance_globals->allegiance_count; index++, allegiance++)
	{
		if ((allegiance->team1_index == our_team_index && allegiance->team2_index == other_team_index) ||
			(allegiance->team2_index == our_team_index && allegiance->team1_index == other_team_index))
		{
			count = allegiance->current_incidents;
			threshold = allegiance->incident_threshold;
			break;
		}
	}

	if (incident_threshold)
	{
		*incident_threshold = threshold;
	}

	return count;
}

void game_allegiance_create(
	short team1_index,
	boolean team1_suspicious,
	short team2_index,
	boolean team2_suspicious,
	short incident_threshold,
	short incident_decay_time,
	boolean allegiance_requires_communication)
{
	struct game_allegiance *allegiance;
	short index;

	for (index = 0, allegiance = game_allegiance_globals->allegiances; index < game_allegiance_globals->allegiance_count; index++, allegiance++)
	{
		if ((allegiance->team1_index == team1_index && allegiance->team2_index == team2_index) ||
			(allegiance->team2_index == team1_index && allegiance->team1_index == team2_index))
		{
			break;
		}
	}

	if (index >= game_allegiance_globals->allegiance_count)
	{
		if (game_allegiance_globals->allegiance_count < MAXIMUM_NUMBER_OF_ALLEGIANCES)
		{
			index = game_allegiance_globals->allegiance_count++;
		}
		else
		{
			error(_error_silent, "game_allegiance_create: too many allegiances (maximum is %d)", MAXIMUM_NUMBER_OF_ALLEGIANCES);
		}
	}

	if (index < game_allegiance_globals->allegiance_count)
	{
		allegiance = &game_allegiance_globals->allegiances[index];
		allegiance->team1_index = team1_index;
		allegiance->team1_suspicious = team1_suspicious;
		allegiance->team2_index = team2_index;
		allegiance->team2_suspicious = team2_suspicious;
		allegiance->incident_threshold = incident_threshold;
		allegiance->incident_decay_time = incident_decay_time;
		allegiance->current_incidents = 0;
		allegiance->current_incident_decay_time = 0;
		allegiance->requires_communication = allegiance_requires_communication;
		allegiance->currently_broken = TRUE;
		game_allegiance_broken(allegiance, FALSE, FALSE);
		allegiance->status_changed = FALSE;
	}

	return;
}

boolean game_allegiance_remove(
	short team1_index,
	short team2_index)
{
	boolean found = FALSE;
	struct game_allegiance *allegiance;
	short index;

	for (index = 0, allegiance = game_allegiance_globals->allegiances; index < game_allegiance_globals->allegiance_count; index++, allegiance++)
	{
		if ((allegiance->team1_index == team1_index && allegiance->team2_index == team2_index) ||
			(allegiance->team2_index == team1_index && allegiance->team1_index == team2_index))
		{
			game_allegiance_broken(allegiance, TRUE, TRUE);
			game_allegiance_globals->allegiance_count--;

			if (game_allegiance_globals->allegiance_count > index)
			{
				game_allegiance_globals->allegiances[index] = game_allegiance_globals->allegiances[game_allegiance_globals->allegiance_count];
			}

			found = TRUE;
			break;
		}
	}

	return found;
}

boolean game_allegiance_incident(
	short aggressor_team_index,
	short victim_team_index,
	short type,
	boolean *notify_immediately)
{
	boolean broken = FALSE;
	struct game_allegiance *allegiance;
	short index;

	for (index = 0, allegiance = game_allegiance_globals->allegiances; index < game_allegiance_globals->allegiance_count; index++, allegiance++)
	{
		if ((allegiance->team1_index == aggressor_team_index && allegiance->team2_index == victim_team_index && allegiance->team2_suspicious) ||
			(allegiance->team2_index == aggressor_team_index && allegiance->team1_index == victim_team_index && allegiance->team1_suspicious))
		{
			short incident_delta = 0;

			switch (type)
			{
			case _allegiance_incident_accident:
				incident_delta = 1;
				break;
			case _allegiance_incident_betrayal:
				incident_delta = 3;
				break;
			case _allegiance_incident_forgive:
				incident_delta = -1;
				break;
			}

			allegiance->current_incidents += incident_delta;

			if (allegiance->incident_decay_time != NONE)
			{
				allegiance->current_incident_decay_time = allegiance->incident_decay_time;
			}

			if (allegiance->incident_threshold != NONE && allegiance->current_incidents >= allegiance->incident_threshold)
			{
				game_allegiance_broken(allegiance, TRUE, FALSE);
				broken = TRUE;

				if (notify_immediately)
				{
					*notify_immediately = !allegiance->requires_communication;
				}
			}

			break;
		}
	}

	return broken;
}

void game_allegiance_provoke(
	short aggressor_team_index,
	short victim_team_index)
{
	struct game_allegiance *allegiance;
	short index;

	for (index = 0, allegiance = game_allegiance_globals->allegiances; index < game_allegiance_globals->allegiance_count; index++, allegiance++)
	{
		if ((allegiance->team1_index == aggressor_team_index && allegiance->team2_index == victim_team_index && allegiance->team2_suspicious) ||
			(allegiance->team2_index == aggressor_team_index && allegiance->team1_index == victim_team_index && allegiance->team1_suspicious))
		{
			if (allegiance->current_incidents > 0 && allegiance->incident_decay_time != NONE)
			{
				allegiance->current_incident_decay_time = allegiance->incident_decay_time;
			}

			break;
		}
	}

	return;
}

void game_allegiance_notify_change(
	short aggressor_team_index,
	short victim_team_index)
{
	struct game_allegiance *allegiance;
	short index;

	for (index = 0, allegiance = game_allegiance_globals->allegiances; index < game_allegiance_globals->allegiance_count; index++, allegiance++)
	{
		if ((allegiance->team1_index == aggressor_team_index && allegiance->team2_index == victim_team_index) ||
			(allegiance->team2_index == aggressor_team_index && allegiance->team1_index == victim_team_index))
		{
			allegiance->status_changed = FALSE;
			break;
		}
	}

	return;
}

static void game_allegiance_broken(
	struct game_allegiance *allegiance,
	boolean currently_broken,
	boolean permanently_broken)
{
	if (permanently_broken || allegiance->currently_broken != currently_broken)
	{
		allegiance->currently_broken = currently_broken;

		if (allegiance->team1_index < NUMBER_OF_SOLO_CAMPAIGN_TEAMS && allegiance->team2_index < NUMBER_OF_SOLO_CAMPAIGN_TEAMS)
		{
			BIT_VECTOR_SET_FLAG(game_allegiance_globals->ally_bitvector, TEAM_PAIR_INDEX(allegiance->team1_index, allegiance->team2_index), !permanently_broken);
			BIT_VECTOR_SET_FLAG(game_allegiance_globals->ally_bitvector, TEAM_PAIR_INDEX(allegiance->team2_index, allegiance->team1_index), !permanently_broken);
			BIT_VECTOR_SET_FLAG(game_allegiance_globals->friendly_bitvector, TEAM_PAIR_INDEX(allegiance->team1_index, allegiance->team2_index), !currently_broken);
			BIT_VECTOR_SET_FLAG(game_allegiance_globals->friendly_bitvector, TEAM_PAIR_INDEX(allegiance->team2_index, allegiance->team1_index), !currently_broken);
		}

		allegiance->status_changed = TRUE;
		ai_handle_allegiance_status_changed(allegiance->team1_index, allegiance->team2_index, currently_broken, permanently_broken);
	}

	return;
}
