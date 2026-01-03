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
	_code_000a50c0 (0000)
000A50D0 01e0:
	_code_000a50d0 (0000)
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
#include "cseries_windows.h"
#include "real_math.h"
#include "game.h"

/* ---------- constants */

enum
{
	SOME_LARGE_NUMBER_OF_TICKS= 1000, // 0x03E8
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

extern void update_client_start(void);
extern void update_server_start(void);
extern void *game_state_malloc(char const *, char const *, long);

/* ---------- globals */

static struct game_time_statistics game_time_statistics;
static struct game_time_globals_struct *game_time_globals;

/* ---------- public code */

boolean game_time_initialized(
	void)
{
	return (game_time_globals && game_time_globals->initialized);
}

void game_time_initialize(
	void)
{
	game_time_globals= (game_time_globals_struct *)game_state_malloc("game time globals", NULL, sizeof(*game_time_globals));
	memset(game_time_globals, 0, sizeof(*game_time_globals));

	return;
}

void game_time_initialize_for_new_map(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 131, game_time_globals && !game_time_globals->initialized);
	memset(game_time_globals, 0, sizeof(*game_time_globals));
	game_time_globals->initialized= TRUE;

	return;
}

void game_time_dispose_from_old_map(
	void)
{
	if (game_time_globals)
	{
		game_time_globals->initialized= FALSE;
		game_time_globals->active= FALSE;
	}

	return;
}

void game_time_dispose(
	void)
{
	return;
}

void game_time_end(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 196, game_time_globals);
	game_time_globals->active= FALSE;

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
		game_time_globals->active= !paused;
	}

	game_time_globals->paused= paused;

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

	game_time_globals->speed= speed;

	return;
}

static void code_000a50c0(
	void)
{
	game_time_statistics.first_line= TRUE;
	game_time_statistics.active= FALSE;

	return;
}

//code_000a50d0

void game_time_start(
	void)
{
	short connection;

	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 162, game_time_globals && game_time_globals->initialized);
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 163, !game_time_globals->active);

	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 562, game_time_globals);

	game_time_globals->speed= 1.f;
	game_time_globals->leftover_dt= 0;
	game_time_globals->active= TRUE;
	
	game_time_statistics.first_line= TRUE;
	game_time_statistics.active= FALSE;

	connection= game_connection();

	switch (connection)
	{
	case _game_connection_local:
	case _game_connection_network_server:
	{
		update_server_start();
		break;
	}
	case _game_connection_network_client:
	case _game_connection_film_playback:
	{
		update_client_start();
		break;
	}
	}

	return;
}

void game_time_update(
	real time_delta_sec)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_time.c", 205, game_time_globals);

	if (game_time_globals->active)
	{
		real v31= game_time_globals->speed*30.;

		if (v31>.0f)
		{
			switch (game_connection())
			{
			case _game_connection_network_client:
			{
				break;
			}
			case _game_connection_network_server:
			{
				break;
			}
			case _game_connection_film_playback:
			{
				break;
			}
			default:
			{
				break;
			}
			}
		}
	}
	else
	{
		game_time_globals->last_local_time_elapsed= 0;
	}

	return;
}

/* ---------- private code */
