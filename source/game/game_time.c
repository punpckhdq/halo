/*
GAME_TIME.C

symbols in this file:
000A4D00 0020:
	_game_time_initialized (0000)
000A4D20 0030:
	_game_time_initialize (0000)
000A4D50 0050:
	_game_time_initialize_for_new_map (0000)
000A4DA0 0020:
	_game_time_dispose_from_old_map (0000)
000A4DC0 0010:
	_game_time_dispose (0000)
000A4DD0 0040:
	_game_time_end (0000)
000A4E10 0040:
	_game_time_get (0000)
000A4E50 0040:
	_game_time_get_elapsed (0000)
000A4E90 0040:
	_local_time_get (0000)
000A4ED0 0040:
	_local_time_get_elapsed (0000)
000A4F10 0040:
	_game_predicting (0000)
000A4F50 0050:
	_game_in_progress (0000)
000A4FA0 0040:
	_game_time_get_paused (0000)
000A4FE0 0050:
	_game_time_set_paused (0000)
000A5030 0040:
	_game_time_get_speed (0000)
000A5070 0050:
	_game_time_set_speed (0000)
000A50C0 0010:
	_game_time_statistics_new (0000)
000A50D0 01e0:
	_game_time_statistics_frame (0000)
000A52B0 00e0:
	_game_time_start (0000)
000A5390 0360:
	_game_time_update (0000)
0025C480 0012:
	??_C@_0BC@ENHAGKAA@game?5time?5globals?$AA@ (0000)
0025C494 0035:
	??_C@_0DF@KKMNPDPK@game_time_globals?5?$CG?$CG?5?$CBgame_time_@ (0000)
0025C4CC 0020:
	??_C@_0CA@INLHHDGN@c?3?2halo?2SOURCE?2game?2game_time?4c?$AA@ (0000)
0025C4EC 0012:
	??_C@_0BC@NOJBGBLH@game_time_globals?$AA@ (0000)
0025C500 0034:
	??_C@_0DE@PPGKJIJK@game_time_globals?5?$CG?$CG?5game_time_g@ (0000)
0025C534 001b:
	??_C@_0BL@DECMFGMD@?$CBgame_time_globals?9?$DOactive?$AA@ (0000)
0025C550 0021:
	??_C@_0CB@OIFBECDL@ticks_elapsed?5?$DM?$DN?5maximum_actions@ (0000)
0025C578 004c:
	??_C@_0EM@FOLKNDLG@game_time_globals?9?$DOleftover_dt?$DO?$DN@ (0000)
0025C5C8 004c:
	??_C@_0EM@FBBKPKLD@update?5server?5is?5too?5far?5ahead?5o@ (0000)
0043EE04 002c:
	_bss_0043ee04 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "players.h"
#include "network_game_globals.h"
#include "units.h"
#include "input.h"
#include "console.h"
#include "game_state.h"
#include "network_messages.h"
#include "network_server_manager.h"
#include "game.h"

/* ---------- constants */

enum
{
	SOME_LARGE_NUMBER_OF_TICKS = 1000, // 0x03E8
};

/* ---------- macros */

/* ---------- structures */

struct game_time_statistics
{
	FILE *file; // 0x0
	boolean active; // 0x4
	boolean first_line; // 0x5
	unsigned long last_milliseconds; // 0x8
	short frame_count; // 0xC
	short total_milliseconds_elapsed; // 0xE
	short minimum_milliseconds_per_frame; // 0x10
	short maximum_milliseconds_per_frame; // 0x12
	short total_latency; // 0x14
	short minimum_latency; // 0x16
	short maximum_latency; // 0x18
	short server_updates; // 0x1A
	short minimum_server_updates; // 0x1C
	short maximum_server_updates; // 0x1E
	short predicted_updates; // 0x20
	short minimum_predicted_updates; // 0x22
	short maximum_predicted_updates; // 0x24
};

struct game_time_globals_struct
{
	boolean initialized;
	boolean active;
	boolean paused;
	short monitor_state;
	short monitor_counter;
	short monitor_latency;
	long local_time;
	short last_local_time_elapsed;
	long server_time;
	real speed;
	real leftover_dt;
};

/* ---------- prototypes */

static void game_time_statistics_new(void);
static void game_time_statistics_frame(short latency, short server_updates, short predicted_updates, boolean unknown);

/* ---------- globals */

static struct game_time_statistics game_time_statistics = {0};
static struct game_time_globals_struct *game_time_globals = NULL;

/* ---------- public code */

boolean game_time_initialized(
	void)
{
	return (game_time_globals && game_time_globals->initialized);
}

void game_time_initialize(
	void)
{
	game_time_globals = (struct game_time_globals_struct *)game_state_malloc("game time globals", NULL, sizeof(*game_time_globals));
	memset(game_time_globals, 0, sizeof(*game_time_globals));

	return;
}

void game_time_initialize_for_new_map(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 131, game_time_globals && !game_time_globals->initialized);
	memset(game_time_globals, 0, sizeof(*game_time_globals));
	game_time_globals->initialized = TRUE;

	return;
}

void game_time_dispose_from_old_map(
	void)
{
	if (game_time_globals)
	{
		game_time_globals->initialized = FALSE;
		game_time_globals->active = FALSE;
	}

	return;
}

void game_time_dispose(
	void)
{
	return;
}

void game_time_start(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 162, game_time_globals && game_time_globals->initialized);
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 163, !game_time_globals->active);

	game_time_set_speed(1.f);
	game_time_globals->leftover_dt = 0.f;
	game_time_globals->active = TRUE;
	game_time_statistics_new();

	switch (game_connection())
	{
	case _game_connection_local:
	case _game_connection_network_server:
		update_server_start();
		break;
	case _game_connection_network_client:
	case _game_connection_film_playback:
		update_client_start();
		break;
	}

	return;
}

void game_time_end(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 196, game_time_globals);
	game_time_globals->active = FALSE;

	return;
}

void game_time_update(
	real time_delta_sec)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 205, game_time_globals);

	if (game_time_globals->active)
	{
		real ticks_per_second = game_time_globals->speed * TICKS_PER_SECOND;

		if (ticks_per_second > 0.f)
		{
			long maximum_ticks;
			boolean discard_leftover_time;
			real dt;
			real ticks;
			long ticks_elapsed;

			switch (game_connection())
			{
			case _game_connection_film_playback:
				maximum_ticks = TICKS_PER_SECOND;
				discard_leftover_time = FALSE;
				break;
			case _game_connection_network_client:
				maximum_ticks = TICKS_PER_SECOND;
				discard_leftover_time = TRUE;
				break;
			case _game_connection_network_server:
			{
				struct network_game_server *server = global_network_game_server_get();
				unsigned long oldest_update = network_game_server_get_oldest_client_update_received(server);
				unsigned long game_time = game_time_get();

				match_vassert("c:\\halo\\SOURCE\\game\\game_time.c", 243, game_time - oldest_update <= 128, "update server is too far ahead of a client for the client to ever catch up!");

				if (game_time > 0)
				{
					long ticks_ahead = oldest_update - game_time + 128;

					maximum_ticks = MIN(TICKS_PER_SECOND, ticks_ahead);

					if (maximum_ticks <= 0)
					{
						network_game_server_stalled_on_client(server, TRUE);
					}
					else
					{
						network_game_server_stalled_on_client(server, FALSE);
					}
				}
				else
				{
					maximum_ticks = 1;
				}

				discard_leftover_time = TRUE;
				break;
			}
			case _game_connection_local:
				maximum_ticks = 7;
				discard_leftover_time = TRUE;
				break;
			default:
				maximum_ticks = 7;
				discard_leftover_time = TRUE;
				break;
			}

			dt = time_delta_sec + game_time_globals->leftover_dt;
			ticks = (real)floor(dt * ticks_per_second);
			ticks_elapsed = (long)(ticks <= SOME_LARGE_NUMBER_OF_TICKS ? ticks : SOME_LARGE_NUMBER_OF_TICKS);

			if (ticks_elapsed > maximum_ticks)
			{
				ticks_elapsed = maximum_ticks;

				if (discard_leftover_time)
				{
					dt = ticks / ticks_per_second;
				}
			}

			game_time_globals->leftover_dt = dt - ticks / ticks_per_second;

			if (game_time_globals->leftover_dt < 0.f)
			{
				game_time_globals->leftover_dt = 0.f;
			}

			match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 306, game_time_globals->leftover_dt>=0.f && game_time_globals->leftover_dt<100.f);

			if (game_connection() == _game_connection_network_client)
			{
				long maximum_actions = update_client_get_maximum_actions();

				if (ticks_elapsed > maximum_actions)
				{
					ticks_elapsed = FLOOR(maximum_actions - 1, 0);
				}
				else if (ticks_elapsed + 1 + 6 < maximum_actions)
				{
					ticks_elapsed = FLOOR(maximum_actions - 1, 0);
				}
				else if (ticks_elapsed + 1 < maximum_actions)
				{
					ticks_elapsed = ticks_elapsed + 1;
				}

				if (ticks_elapsed > maximum_actions)
				{
					match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 339, ticks_elapsed <= maximum_actions);
					ticks_elapsed = maximum_actions;
				}
			}

			if (ticks_elapsed > 0)
			{
				long target_time;
				long maximum_server_time;
				long server_ticks;

				while (ticks_elapsed > 0 && game_time_globals->local_time < game_time_globals->server_time)
				{
					game_time_globals->local_time++;
					ticks_elapsed--;
				}

				target_time = game_time_globals->local_time + ticks_elapsed;

				switch (game_connection())
				{
				case _game_connection_local:
					update_client_local_ticks((short)ticks_elapsed);
					break;
				case _game_connection_network_server:
					network_game_server_update_ticks(global_network_game_server_get(), (short)ticks_elapsed);
					break;
				}

				maximum_server_time = update_client_get_maximum_possible_server_time();

				if (maximum_server_time > game_time_globals->server_time)
				{
					long tick_index;

					server_ticks = MIN(maximum_server_time, target_time) - game_time_globals->server_time;

					for (tick_index = 0; tick_index < server_ticks; tick_index++)
					{
						game_tick();
						game_time_globals->server_time++;
						game_time_globals->local_time++;
					}
				}
				else
				{
					server_ticks = 0;
				}

				game_time_statistics_frame((short)(maximum_server_time - game_time_globals->local_time), (short)server_ticks, 0, FALSE);
				game_time_globals->last_local_time_elapsed = (short)ticks_elapsed;
			}
		}

		game_frame(game_time_get_speed() * time_delta_sec);
	}
	else
	{
		game_time_globals->last_local_time_elapsed = 0;
	}

	return;
}

long game_time_get(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 463, game_time_globals && game_time_globals->initialized);

	return game_time_globals->local_time;
}

short game_time_get_elapsed(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 471, game_time_globals && game_time_globals->initialized);

	return game_time_globals->last_local_time_elapsed;
}

long local_time_get(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 479, game_time_globals && game_time_globals->initialized);

	return game_time_globals->local_time;
}

short local_time_get_elapsed(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 487, game_time_globals && game_time_globals->initialized);

	return game_time_globals->last_local_time_elapsed;
}

boolean game_predicting(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 495, game_time_globals && game_time_globals->initialized);

	return FALSE;
}

boolean game_in_progress(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 505, game_time_globals);

	if (game_time_globals->initialized)
	{
		if (game_time_globals->active)
		{
			return TRUE;
		}

		/* bug? */
		if (game_time_globals->paused)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	return FALSE;
}

boolean game_time_get_paused(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 533, game_time_globals);

	return game_time_globals->paused;
}

void game_time_set_paused(
	boolean paused)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 541, game_time_globals);

	if (game_time_globals->initialized)
	{
		game_time_globals->active = !paused;
	}

	game_time_globals->paused = paused;

	return;
}

real game_time_get_speed(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 555, game_time_globals);

	return game_time_globals->speed;
}

void game_time_set_speed(
	real speed)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 562, game_time_globals);

	game_time_globals->speed = speed;

	return;
}

static void game_time_statistics_new(
	void)
{
	game_time_statistics.first_line = TRUE;
	game_time_statistics.active = FALSE;

	return;
}

static void game_time_statistics_frame(
	short latency,
	short server_updates,
	short predicted_updates,
	boolean unknown) /* fake name */
{
	if (!game_time_statistics.active)
	{
		game_time_statistics.frame_count = 0;
		game_time_statistics.total_milliseconds_elapsed = 0;
		game_time_statistics.minimum_milliseconds_per_frame = SHORT_MAX;
		game_time_statistics.maximum_milliseconds_per_frame = SHORT_MIN;
		game_time_statistics.total_latency = 0;
		game_time_statistics.minimum_latency = SHORT_MAX;
		game_time_statistics.maximum_latency = SHORT_MIN;
		game_time_statistics.server_updates = 0;
		game_time_statistics.minimum_server_updates = SHORT_MAX;
		game_time_statistics.maximum_server_updates = SHORT_MIN;
		game_time_statistics.predicted_updates = 0;
		game_time_statistics.minimum_predicted_updates = SHORT_MAX;
		game_time_statistics.maximum_predicted_updates = SHORT_MIN;
		game_time_statistics.last_milliseconds = system_milliseconds();
		game_time_statistics.active = TRUE;
	}
	else
	{
		unsigned long milliseconds = system_milliseconds();
		short milliseconds_elapsed = (short)(milliseconds - game_time_statistics.last_milliseconds);

		game_time_statistics.frame_count++;
		game_time_statistics.total_milliseconds_elapsed += milliseconds_elapsed;
		game_time_statistics.last_milliseconds = milliseconds;

		if (milliseconds_elapsed > game_time_statistics.maximum_milliseconds_per_frame)
		{
			game_time_statistics.maximum_milliseconds_per_frame = milliseconds_elapsed;
		}

		if (milliseconds_elapsed < game_time_statistics.minimum_milliseconds_per_frame)
		{
			game_time_statistics.minimum_milliseconds_per_frame = milliseconds_elapsed;
		}

		game_time_statistics.total_latency += latency;

		if (latency > game_time_statistics.maximum_latency)
		{
			game_time_statistics.maximum_latency = latency;
		}

		if (latency < game_time_statistics.minimum_latency)
		{
			game_time_statistics.minimum_latency = latency;
		}

		game_time_statistics.server_updates += server_updates;

		if (server_updates > game_time_statistics.maximum_server_updates)
		{
			game_time_statistics.maximum_server_updates = server_updates;
		}

		if (server_updates < game_time_statistics.minimum_server_updates)
		{
			game_time_statistics.minimum_server_updates = server_updates;
		}

		game_time_statistics.predicted_updates += predicted_updates;

		if (predicted_updates > game_time_statistics.maximum_predicted_updates)
		{
			game_time_statistics.maximum_predicted_updates = predicted_updates;
		}

		if (predicted_updates < game_time_statistics.minimum_predicted_updates)
		{
			game_time_statistics.minimum_predicted_updates = predicted_updates;
		}

		if (game_time_statistics.total_milliseconds_elapsed >= 1000 && game_time_statistics.frame_count > 0)
		{
			if (game_time_statistics.minimum_latency)
			{
				short monitor_state = game_time_statistics.minimum_latency >= 0;

				if (monitor_state != game_time_globals->monitor_state)
				{
					game_time_globals->monitor_state = monitor_state;
					game_time_globals->monitor_counter = 0;
					game_time_globals->monitor_latency = monitor_state ? SHORT_MAX : SHORT_MIN;
				}

				switch (game_time_globals->monitor_state)
				{
				case 0:
					if (game_time_statistics.minimum_latency > game_time_globals->monitor_latency)
					{
						game_time_globals->monitor_latency = game_time_statistics.minimum_latency;
					}
					break;
				case 1:
					if (game_time_statistics.minimum_latency < game_time_globals->monitor_latency)
					{
						game_time_globals->monitor_latency = game_time_statistics.minimum_latency;
					}
					break;
				}

				if (++game_time_globals->monitor_counter == 5)
				{
					game_time_globals->monitor_state = NONE;
				}
			}
			else
			{
				game_time_globals->monitor_state = NONE;
			}

			game_time_statistics.active = FALSE;
		}
	}

	return;
}
