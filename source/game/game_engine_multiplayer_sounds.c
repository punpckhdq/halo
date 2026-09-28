/*
GAME_ENGINE_MULTIPLAYER_SOUNDS.C

*/

/* ---------- headers */

#include "cseries.h"
#include "players.h"
#include "network_game_globals.h"
#include "units.h"
#include "network_messages.h"
#include "sound_manager.h"
#include "game_sound.h"
#include "sound_definitions.h"
#include "network_server_message_handler.h"
#include "network_server_manager.h"

/* ---------- structures */

struct queued_mp_sound
{
	long index;
	long ticks_left;
};

/* ---------- prototypes */

static void _game_engine_play_multiplayer_sound(long index);
static void push_queued_sound(long index, long length);
static long get_sound_length_in_ticks(long index);

/* ---------- globals */

static boolean sound_is_queueable[NUMBER_OF_MULTIPLAYER_SOUNDS] =
{
	TRUE,  // _multiplayer_sound_oddball_spawn
	TRUE,  // _multiplayer_sound_game_over
	TRUE,  // _multiplayer_sound_60_seconds
	TRUE,  // _multiplayer_sound_30_seconds
	TRUE,  // _multiplayer_sound_red_60_seconds
	TRUE,  // _multiplayer_sound_red_30_seconds
	TRUE,  // _multiplayer_sound_blue_60_seconds
	TRUE,  // _multiplayer_sound_blue_30_seconds
	TRUE,  // _multiplayer_sound_ctf_blue_took_flag
	TRUE,  // _multiplayer_sound_ctf_blue_returned_flag
	TRUE,  // _multiplayer_sound_ctf_blue_captured_flag
	TRUE,  // _multiplayer_sound_ctf_red_took_flag
	TRUE,  // _multiplayer_sound_ctf_red_returned_flag
	TRUE,  // _multiplayer_sound_ctf_red_captured_flag
	TRUE,  // _multiplayer_sound_double_kill
	TRUE,  // _multiplayer_sound_triple_kill
	TRUE,  // _multiplayer_sound_killtacular_kill
	TRUE,  // _multiplayer_sound_running_riot
	TRUE,  // _multiplayer_sound_killing_spree
	TRUE,  // _multiplayer_sound_oddball
	TRUE,  // _multiplayer_sound_race
	TRUE,  // _multiplayer_sound_slayer
	TRUE,  // _multiplayer_sound_ctf
	TRUE,  // _multiplayer_sound_warthog
	TRUE,  // _multiplayer_sound_ghost
	TRUE,  // _multiplayer_sound_scorpion
	FALSE, // _multiplayer_sound_countdown_timer
	FALSE, // _multiplayer_sound_teleporter_activate
	FALSE, // _multiplayer_sound_flag_failure
	FALSE, // _multiplayer_sound_countdown_for_respawn
	FALSE, // _multiplayer_sound_hill_move
	FALSE, // _multiplayer_sound_respawn
	TRUE,  // _multiplayer_sound_team_king
	TRUE,  // _multiplayer_sound_team_oddball
	TRUE,  // _multiplayer_sound_team_race
	TRUE,  // _multiplayer_sound_team_slayer
	TRUE,  // _multiplayer_sound_king
	TRUE,  // _multiplayer_sound_blue_team_ctf
	TRUE,  // _multiplayer_sound_red_team_ctf
	TRUE,  // _multiplayer_sound_hill_contested
	TRUE,  // _multiplayer_sound_hill_controlled
	TRUE,  // _multiplayer_sound_hill_occupied
	FALSE  // _multiplayer_sound_countdown_timer_end
};

static long mp_sound_queue_count = 0;
static struct queued_mp_sound mp_sound_queue[5] = {0};

/* ---------- public code */

void game_engine_update_multiplayer_sound(
	void)
{
	if (mp_sound_queue_count)
	{
		if (!--mp_sound_queue[0].ticks_left)
		{
			long i;

			for (i = 1; i < mp_sound_queue_count; i++)
			{
				mp_sound_queue[i - 1] = mp_sound_queue[i];
			}
			mp_sound_queue_count--;
			if (mp_sound_queue_count)
			{
				_game_engine_play_multiplayer_sound(mp_sound_queue[0].index);
			}
		}
	}

	return;
}

void game_engine_play_multiplayer_sound(
	long index)
{
	if (sound_is_queueable[index])
	{
		push_queued_sound(index, get_sound_length_in_ticks(index) + 5);
		if (mp_sound_queue_count == 1)
		{
			_game_engine_play_multiplayer_sound(index);
		}
	}
	else
	{
		_game_engine_play_multiplayer_sound(index);
	}

	return;
}

void game_engine_intialize_queued_sounds(
	void)
{
	memset(mp_sound_queue, 0, sizeof(mp_sound_queue));
	mp_sound_queue_count = 1;
	mp_sound_queue[0].index = NONE;
	mp_sound_queue[0].ticks_left = 2 * TICKS_PER_SECOND;

	return;
}

/* ---------- private code */

static void _game_engine_play_multiplayer_sound(
	long index)
{
	struct scenario *scenario = global_scenario_get();
	struct game_globals *globals = scenario_get_game_globals();
	struct game_globals_multiplayer_information *multiplayer = TAG_BLOCK_GET_ELEMENT(&globals->multiplayer_information, 0, struct game_globals_multiplayer_information);

	if (multiplayer)
	{
		if (index < multiplayer->sounds.count)
		{
			struct tag_reference *sound = TAG_BLOCK_GET_ELEMENT(&multiplayer->sounds, index, struct tag_reference);

			if (sound)
			{
				if (sound->index != NONE)
				{
					unspatialized_impulse_sound_new(sound->index, 1.f);
				}
			}
		}
	}

	return;
}

static void push_queued_sound(
	long index,
	long length)
{
	if (mp_sound_queue_count < (long)NUMBEROF(mp_sound_queue))
	{
		mp_sound_queue[mp_sound_queue_count].index = index;
		mp_sound_queue[mp_sound_queue_count].ticks_left = length;
		mp_sound_queue_count++;
	}

	return;
}

static long get_sound_length_in_ticks(
	long index)
{
	struct scenario *scenario = global_scenario_get();
	struct game_globals *globals = scenario_get_game_globals();
	struct game_globals_multiplayer_information *multiplayer = TAG_BLOCK_GET_ELEMENT(&globals->multiplayer_information, 0, struct game_globals_multiplayer_information);

	struct tag_reference *sound;

	if (!multiplayer)
	{
		return 0;
	}

	if (index >= multiplayer->sounds.count)
	{
		return 0;
	}

	sound = TAG_BLOCK_GET_ELEMENT(&multiplayer->sounds, index, struct tag_reference);

	if (!sound)
	{
		return 0;
	}

	if (sound->index == NONE)
	{
		return 0;
	}

	return TICKS_PER_SECOND * sound_definition_get(sound->index)->runtime_maximum_play_time / MILLISECONDS_PER_SECOND;
}
