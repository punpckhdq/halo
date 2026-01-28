/*
GAME.H

header included in hcex build.
*/

#ifndef __GAME_H
#define __GAME_H
#pragma once

/* ---------- constants */

enum
{
	_game_connection_local= 0,
	_game_connection_network_client,
	_game_connection_network_server,
	_game_connection_film_playback,
	NUMBER_OF_GAME_CONNECTIONS,
};

enum
{
	_game_difficulty_level_easy= 0,
	_game_difficulty_level_normal,
	_game_difficulty_level_hard,
	_game_difficulty_level_impossible,
	NUMBER_OF_GAME_DIFFICULTY_LEVELS
};

/* ---------- macros */

/* ---------- structures */

struct slayer_statistics
{
	short ignored;
};

struct ctf_statistics
{
	short flag_grabs;
	short flag_returns;
	short flag_scores;
};

struct oddball_statistics
{
	short time_with_the_ball;
	short ball_carrier_kills;
	short kills_as_ball_carrier;
};

struct king_statistics
{
	short time_on_hill;
};

struct race_statistics
{
	short last_lap_time;
	short laps;
	short best_lap_time;
};

union multiplayer_statistics
{
	struct slayer_statistics slayer_statistics;
	struct ctf_statistics ctf_statistics;
	struct oddball_statistics oddball_statistics;
	struct king_statistics king_statistics;
	struct race_statistics race_statistics;
};

struct game_statistics
{
	short sort_key;
	short games_played;
	short games_won;
	short kills_in_a_row;
	short multiple_kills;
	short last_kill_time;
	short kills[4];
	short assists[4];
	short friendly_fire_kills;
	short deaths;
	short suicides;
	long shots_fired;
	long shots_hit;
	long seconds_online;
	short killed_teammate_since_last_death;
	short custom_data_size;
	union multiplayer_statistics multiplayer_statistics;
};


/* ---------- prototypes/GAME.C */

void game_initialize(void);
void game_dispose(void);
void game_precache_new_map(char *map_name, boolean blocking);
boolean game_map_loading_in_progress(real *progress);
void game_unload(void);
void game_dispose_from_old_map(void);
void game_frame(real dt);
void remove_quitting_players_from_game(void);
void game_tick(long ticks_remaining_this_frame);
void game_options_new(struct game_options *options);
boolean game_options_verify(struct game_options *options);
void game_initial_pulse(void);
void game_set_players_are_double_speed(boolean players_are_double_speed);
boolean game_players_are_double_speed(void);
void game_difficulty_level_set(short difficulty);
short game_difficulty_level_get(void);
short game_difficulty_level_get_ignore_easy(void);
void game_set_game_variant(struct game_variant *variant);
void game_set_game_engine_index(short index);
boolean game_all_quiet(void);
boolean game_safe_to_save(void);
boolean game_safe_to_speak(void);
boolean game_is_cooperative(void);
boolean game_load(struct game_options *options);
void game_initialize_for_new_map(void);
void game_set_game_variant_from_name(const char *name);

/* ---------- prototypes/GAME_TIME.C */

boolean game_time_initialized(void);
void game_time_initialize(void);
void game_time_initialize_for_new_map(void);
void game_time_dispose_from_old_map(void);
void game_time_dispose(void);
void game_time_end(void);
long game_time_get(void);
short game_time_get_elapsed(void);
long local_time_get(void);
short local_time_get_elapsed(void);
boolean game_predicting(void);
boolean game_in_progress(void);
boolean game_time_get_paused(void);
void game_time_set_paused(boolean paused);
real game_time_get_speed(void);
void game_time_set_speed(real speed);
void game_time_start(void);
void game_time_update(real time_delta_sec);

/* ---------- prototypes/MAIN.C */

short game_connection(void);

/* ---------- globals */

/* ---------- public code */

#endif // __GAME_H
