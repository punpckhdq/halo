/*
GAME_ENGINE.H

header included in hcex build.
*/

#ifndef __GAME_ENGINE_H
#define __GAME_ENGINE_H
#pragma once

/* ---------- headers */


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
	_multiplayer_sound_oddball_spawn = 0,
	_multiplayer_sound_game_over,
	_multiplayer_sound_60_seconds,
	_multiplayer_sound_30_seconds,
	_multiplayer_sound_red_60_seconds,
	_multiplayer_sound_red_30_seconds,
	_multiplayer_sound_blue_60_seconds,
	_multiplayer_sound_blue_30_seconds,
	_multiplayer_sound_ctf_blue_took_flag,
	_multiplayer_sound_ctf_blue_returned_flag,
	_multiplayer_sound_ctf_blue_captured_flag,
	_multiplayer_sound_ctf_red_took_flag,
	_multiplayer_sound_ctf_red_returned_flag,
	_multiplayer_sound_ctf_red_captured_flag,
	_multiplayer_sound_double_kill,
	_multiplayer_sound_triple_kill,
	_multiplayer_sound_killtacular_kill,
	_multiplayer_sound_running_riot,
	_multiplayer_sound_killing_spree,
	_multiplayer_sound_oddball,
	_multiplayer_sound_race,
	_multiplayer_sound_slayer,
	_multiplayer_sound_ctf,
	_multiplayer_sound_warthog,
	_multiplayer_sound_ghost,
	_multiplayer_sound_scorpion,
	_multiplayer_sound_countdown_timer,
	_multiplayer_sound_teleporter_activate,
	_multiplayer_sound_flag_failure,
	_multiplayer_sound_countdown_for_respawn,
	_multiplayer_sound_hill_move,
	_multiplayer_sound_respawn,
	_multiplayer_sound_team_king,
	_multiplayer_sound_team_oddball,
	_multiplayer_sound_team_race,
	_multiplayer_sound_team_slayer,
	_multiplayer_sound_king,
	_multiplayer_sound_blue_team_ctf,
	_multiplayer_sound_red_team_ctf,
	_multiplayer_sound_hill_contested,
	_multiplayer_sound_hill_controlled,
	_multiplayer_sound_hill_occupied,
	_multiplayer_sound_countdown_timer_end,
	NUMBER_OF_MULTIPLAYER_SOUNDS,
};

/* ---------- macros */

/* ---------- structures */

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
	unsigned short *(*get_score_string)(long, unsigned short *);
	unsigned short *(*get_team_score_string)(long, unsigned short *);
	boolean (*allow_pick_up)(long, long);
	void (*player_damaged_player)(long, long, boolean);
	void (*player_killed_player)(long, long, long, boolean);
	boolean (*rasterize_score)(long, long, long, unsigned short *, long);
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

boolean game_engine_running(void);
boolean game_engine_has_shield(long player_index);
boolean game_engine_has_teams(void);
boolean game_engine_hud_draw_motion_sensor(long player_index);

real_point3d game_engine_get_goal_position(short goal_index);
void game_engine_render_nav_points(short local_player_index);

boolean game_engine_infinite_grenades(long player_index);

long game_engine_remap_object_definition(long definition_index);

long game_engine_remap_vehicle(long vehicle_definition_index);
long game_engine_remap_equipment(long equipment_definition_index);
long game_engine_remap_weapon(long weapon_definition_index);


boolean game_engine_allow_integrated_lights(long object_index);

/* ---------- prototypes/GAME_ENGINE_MULTIPLAYER_SOUNDS.C */

void game_engine_update_multiplayer_sound(void);
void game_engine_play_multiplayer_sound(long index);
void game_engine_intialize_queued_sounds(void);

/* ---------- globals */

extern struct game_engine *game_engine;

/* ---------- public code */

#endif // __GAME_ENGINE_H
