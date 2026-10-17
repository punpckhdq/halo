/*
GAME_ENGINE.H

header included in hcex build.
*/

#ifndef __GAME_ENGINE_H
#define __GAME_ENGINE_H
#pragma once

/* ---------- headers */

#include "unicode.h"
#include "index_resolution.h"

/* ---------- constants */

enum
{
	_game_engine_none = 0,
	_game_engine_ctf,
	_game_engine_slayer,
	_game_engine_oddball,
	_game_engine_king,
	_game_engine_race,
	_game_engine_terminator,
	_game_engine_stub,
	NUMBER_OF_GAME_ENGINES,
	FIRST_USABLE_GAME_ENGINE_INDEX = _game_engine_ctf,
	LAST_USABLE_GAME_ENGINE_INDEX = _game_engine_race
};

enum get_score_type
{
	_get_score_individual = 0,
	_get_score_team
};

enum
{
	MAXIMUM_GAME_VARIANT_NAME_LENGTH = 12	/* fake name */
};

enum
{
	_variant_is_system_default_bit = 0,
	NUMBER_OF_GAME_VARIANT_FLAGS,

	_default_variant_index_mask = ~MASK(CHAR_BITS)
};

/* ---------- macros */

/* ---------- structures */

struct universal_variant
{
	boolean teams;
	byte pad0;
	byte pad1;
	byte pad2;
	long flags;
	long goal_radar;
	boolean odd_man_out;
	byte pad4;
	byte pad5;
	byte pad6;
	long respawn_time_growth;
	long respawn_time;
	long suicide_penalty;
	long lives;
	real health;
	long score_to_win;
	long weapon_set;
	long vehicle_set;
};

struct ctf_variant
{
	boolean assault;
	boolean reset_on_capture;
	boolean flag_must_reset;
	boolean flag_at_home_to_score;
	long single_flag_time;
};

struct slayer_variant
{
	boolean no_death_bonus;
	boolean no_kill_penalty;
	boolean kill_in_order;
};

struct king_variant
{
	boolean moving_hill;
};

struct oddball_variant
{
	boolean random_start;
	boolean ball_spawn_delay;
	long speed_with_ball;
	long trait_with_ball;
	long trait_without_ball;
	long oddball_ball_type;
	long ball_spawn_count;
};

struct race_variant
{
	long race_type;
	long team_scoring;
};

union game_engine_variant
{
	struct ctf_variant ctf;
	struct slayer_variant slayer;
	struct king_variant king;
	struct oddball_variant oddball;
	struct race_variant race;
};

struct game_variant
{
	wchar_t human_readable_game_description[MAXIMUM_GAME_VARIANT_NAME_LENGTH];
	long game_engine_index;
	struct universal_variant universal_variant;
	union game_engine_variant game_engine_variant;
	word flags;
};

struct game_engine
{
	char const *name;
	unsigned long type;
	void (*dispose)(void);
	boolean (*initialize)(void);
	void (*dispose_from_old_map)(void);
	void (*player_added)(long);
	void (*game_ending)(void);
	void (*game_starting)(void);
	void (*statistics_append)(struct game_statistics *, struct game_statistics *);
	void (*handle_client_message)(long, void *, short);
	void (*handle_server_message)(void *, short);
	void (*pregame_post_rasterize)(void);
	void (*post_rasterize)(void);
	void (*player_update)(long);
	void (*weapon_update)(long, struct weapon_datum *);
	boolean (*weapon_pickup)(long, long);
	void (*weapon_drop)(long);
	void (*update)(void);
	long (*get_score)(long, enum get_score_type);
	long (*get_team_score)(long);
	word *(*get_score_string)(long, word *);
	word *(*get_team_score_string)(long, word *);
	boolean (*allow_pick_up)(long, long);
	void (*player_damaged_player)(long, long, boolean);
	void (*player_killed_player)(long, long, long, boolean);
	boolean (*rasterize_score)(long, long, long, word *, long);
	real (*starting_location_rating)(long, struct scenario_player *);
	void (*prespawn_player_update)(long);
	boolean (*postspawn_player_update)(long);
	long (*game_engine_player_get_team_index)(long);
	boolean (*goal_matches_player)(long, long);
	boolean (*game_engine_test_flag)(long);
	boolean (*game_engine_test_trait)(long, long);
	long (*game_engine_did_player_win)(long);
};

/* ---------- prototypes/GAME_ENGINE.C */

void game_engine_dispose(void);
boolean game_engine_force_single_screen(void);
void game_engine_update_non_deterministic(real seconds_elapsed);

boolean game_engine_running(void);
boolean game_engine_display_team_indicators(void);
boolean game_engine_get_state_message(long player_index, wchar_t *buffer, long buffer_length);
boolean game_engine_has_shield(long player_index);
boolean game_engine_has_teams(void);
boolean game_engine_hud_draw_motion_sensor(long player_index);
boolean game_engine_hud_draw_messages(long player_index);

real_point3d game_engine_get_goal_position(short goal_index);
void game_engine_render_nav_points(short local_player_index);

boolean game_engine_infinite_grenades(long player_index);

long game_engine_remap_object_definition(long definition_index);

long game_engine_remap_vehicle(long vehicle_definition_index);
long game_engine_remap_equipment(long equipment_definition_index);
long game_engine_remap_weapon(long weapon_definition_index);

void game_engine_weapon_fired(long player_index);

boolean game_engine_allow_integrated_lights(long object_index);
boolean game_engine_allow_dynamic_lighting(void);
boolean game_engine_can_score(void);
void game_engine_player_killed(long killing_player_index, long killing_object_index, long dead_player_index, boolean friendly_fire);
real game_engine_get_damage_multiplier(long attacker_index, long defender_index);

void game_engine_playlist_next(long unused1, long unused2, long unused3);
struct game_variant build_game_variant_slayer(void);
struct game_variant build_game_variant_slayer_pro(void);
struct game_variant build_game_variant_elimination(void);
struct game_variant build_game_variant_phantoms(void);
struct game_variant build_game_variant_endurance(void);
struct game_variant build_game_variant_rockets(void);
struct game_variant build_game_variant_snipers(void);
struct game_variant build_game_variant_oddball(void);
struct game_variant build_game_variant_reverse_tag(void);
struct game_variant build_game_variant_accumulation(void);
struct game_variant build_game_variant_juggernaut(void);
struct game_variant build_game_variant_stalker(void);
struct game_variant build_game_variant_king(void);
struct game_variant build_game_variant_king_pro(void);
struct game_variant build_game_variant_crazy_king(void);
struct game_variant build_game_variant_race(void);
struct game_variant build_game_variant_rally(void);
struct game_variant build_game_variant_ctf(void);
struct game_variant build_game_variant_invasion(void);
struct game_variant build_game_variant_iron_ctf(void);
struct game_variant build_game_variant_ctf_pro(void);
struct game_variant build_game_variant_team_race(void);
struct game_variant build_game_variant_team_rally(void);
struct game_variant build_game_variant_team_oddball(void);
struct game_variant build_game_variant_team_king(void);
struct game_variant build_game_variant_team_slayer(void);
void game_engine_variant_cleanup(struct game_variant *variant);
void game_engine_post_rasterize_objects(void);
void game_engine_nonplayer_post_rasterize(void);

/* ---------- prototypes/GAME_ENGINE_MULTIPLAYER_SOUNDS.C */

void game_engine_update_multiplayer_sound(void);
void game_engine_play_multiplayer_sound(long index);
void game_engine_intialize_queued_sounds(void);

/* ---------- globals */

extern struct game_engine *game_engine;

/* ---------- public code */

#endif // __GAME_ENGINE_H
