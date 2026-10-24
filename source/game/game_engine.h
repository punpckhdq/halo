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

enum
{
	_game_engine_ignored1 = NUMBER_OF_GAME_ENGINES,
	_game_engine_ignored2,
	_game_engine_ignored3,
	_game_engine_ignored4,
	_game_engine_all,
	_game_engine_all_non_team,
	_game_engine_all_normal,
	NUMBER_OF_GAME_MATCHING_OPTIONS
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
	MAXIMUM_NETGAME_GOALS = 32	/* fake name */
};

enum goal_radar
{
	_radar_motion_tracker = 0,
	_radar_nav_point,
	_radar_none
};

enum game_engine_weapons
{
	_game_engine_weapons_normal = 0,
	_game_engine_weapons_pistols,
	_game_engine_weapons_assault_rifles,
	_game_engine_weapons_plasma_weapons,
	_game_engine_weapons_sniping,
	_game_engine_weapons_no_sniping,
	_game_engine_weapons_rocket_launchers,
	_game_engine_weapons_shotguns,
	_game_engine_weapons_short_range,
	_game_engine_weapons_human,
	_game_engine_weapons_no_grenades,
	NUMBER_OF_GAME_ENGINE_WEAPON_SETS
};

enum
{
	_weapon_list_assault_rifle = 0,
	_weapon_list_flamethrower,
	_weapon_list_gravity_rifle,
	_weapon_list_needler,
	_weapon_list_pistol,
	_weapon_list_plasma_pistol,
	_weapon_list_plasma_rifle,
	_weapon_list_rocket_launcher,
	_weapon_list_shotgun,
	_weapon_list_sniper_rifle,
	_weapon_list_ball,
	_weapon_list_flag,
	_weapon_list_frag_grenade,
	_weapon_list_plasma_grenade
};

enum game_engine_vehicles
{
	_game_engine_vehicles_default = 0,
	_game_engine_vehicles_none,
	_game_engine_vehicles_warthog,
	_game_engine_vehicles_ghost,
	_game_engine_vehicles_tank,
	NUMBER_OF_GAME_ENGINE_VEHICLE_SETS
};

enum game_engine_universal_variant_flags
{
	_allow_motion_tracker_bit = 0,
	_allow_friendly_navpoints_bit,
	_infinite_grenades_bit,
	_no_shields_bit,
	_always_invisible_bit,
	_generic_starting_equipment_bit,
	_motion_tracker_no_enemies,
	NUMBER_OF_GAME_ENGINE_UNIVERSAL_VARIANT_FLAGS
};

enum game_engine_mode
{
	game_engine_mode_active = 0,
	game_engine_mode_postgame_delay,
	game_engine_mode_postgame_rasterize_delay,
	game_engine_mode_postgame_rasterize
};

enum
{
	_game_engine_disable_dynamic_light_bit = 0,
	_game_engine_disable_integrated_lights_bit,
	_game_engine_5_or_more_players_bit,
	_game_engine_9_or_more_players_bit,
	_game_engine_game_over_sound_disabled_bit
};

enum
{
	_game_engine_spawn_point_obeys_teams_bit = 0,
	_game_engine_show_kills_with_score_bit,
	NUMBER_OF_GAME_ENGINE_FLAGS
};

enum game_engine_trait
{
	_game_trait_none = 0,
	_game_trait_invisible,
	_game_trait_extra_damage,
	_game_trait_damage_resistant
};

enum game_engine_place_flags
{
	_place_tied = 0,
	_place_all_tied,
	_place_two_groups,
	_place_team
};

enum postgame_statistic
{
	_postgame_statistic_ranking = 0,
	_postgame_statistic_custom,
	_postgame_statistic_kills,
	_postgame_statistic_assists,
	_postgame_statistic_deaths
};

enum
{
	game_engine_message_welcome = 0,
	game_engine_message_killed_by_unknown,
	game_engine_message_killed_by_biped,
	game_engine_message_killed_by_vehicle,
	game_engine_message_killed_by_player,
	game_engine_message_killed_by_friendly_fire,
	game_engine_message_killed_by_self,
	game_engine_message_double_kill,
	game_engine_message_killed_enemy,
	game_engine_message_triple_kill,
	game_engine_message_multi_kill,
	game_engine_message_five_kills_in_row,
	game_engine_message_ten_kills_in_a_row,
	game_engine_message_killed_friendly,
	game_engine_message_multi_kill_with_score,
	game_engine_message_triple_kill_with_score,
	game_engine_message_double_kill_with_score,
	game_engine_message_ten_kills_in_a_row_with_score,
	game_engine_message_five_kills_in_row_with_score,
	game_engine_message_killed_enemy_with_score,
	game_engine_message_winner,
	game_engine_message_team_winner,
	game_engine_message_show_score,
	game_engine_message_odd_man_out,
	game_engine_message_out_of_lives,
	game_engine_message_respawn_timer,
	game_engine_message_waiting_for_space_to_clear,
	game_engine_message_player_quit_self,
	game_engine_message_quit,
	game_engine_message_press_back_for_score,
	game_engine_message_time_left,
	game_engine_message_count
};

enum oddball_speed
{
	_oddball_speed_slow = 0,
	_oddball_speed_normal,
	_oddball_speed_faster,
	NUMBER_OF_ODDBALL_SPEEDS
};

enum oddball_ball_type
{
	_oddball_normal = 0,
	_oddball_magic,
	_oddball_terminator
};

enum race_type
{
	_race_type_normal = 0,
	_race_type_any_order,
	_race_type_flag_rally
};

enum race_team_scoring
{
	_race_team_scoring_min = 0,
	_race_team_scoring_max,
	_race_team_scoring_accumulate
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
	word *(*get_score_string)(long, word *);
	word *(*get_score_header_string)(word *);
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

struct _game_engine_globals
{
	long flags;
	long current_team_to_add;
	real game_ending_timer;
	real post_game_options_fade;
	enum game_engine_mode mode;
	real score_alpha[2];
	boolean map_cycle_countdown_enabled;
	real time_until_map_cycle_timeout_expires_seconds;
};

struct netgame_goal
{
	real_point3d position;
	boolean in_use;
	long player_index;
	short team_index;
	long ignore_player_index;
	short nav_index;
};

struct play_stage
{
	char map_name[64];
	struct game_variant game_variant;
};

struct statistic_buffer
{
	long player_index;
	long score;
	long custom;
	long kills;
	long deaths;
	long assists;
	long place;
};

struct game_engine_place
{
	short flags;
	short place;
};

/* ---------- prototypes/GAME_ENGINE.C */

long game_engine_get_team_score(long team_index);
real linear_to_non_linear_alpha(real linear_alpha);
void game_engine_initialize(struct game_variant *variant);
void game_engine_dispose(void);
void game_engine_initialize_for_new_map(void);
void game_engine_dispose_from_old_map(void);
void game_engine_player_added(long player_index);
void game_engine_game_ending(void);
void game_engine_game_starting(void);
long players_in_game(void);
boolean multiple_teams_alive(void);
boolean team_has_players(long team_index);
boolean game_engine_should_end_game(void);
void game_engine_statistics_append(struct game_statistics *permanent_statistics, struct game_statistics *game_statistics);
void game_engine_handle_client_message(long player_index, void *encoded_message, short encoded_message_size);
void game_engine_handle_server_message(void *encoded_message, short encoded_message_size);
int sort_statistic_buffer(struct statistic_buffer const *buffer_1, struct statistic_buffer const *buffer_2);
int sort_statistic_buffer_ranking(struct statistic_buffer const *buffer_1, struct statistic_buffer const *buffer_2);
long populate_statistic_buffer(struct statistic_buffer *statistic_buffer, enum postgame_statistic statistic, boolean inverse);
long postgame_statistic_get_rating(long player_index, enum postgame_statistic statistic, boolean inverse);
void get_postgame_hilite_colors(real_argb_color *normal, real_argb_color *column_hilite, real_argb_color *winner);
void game_engine_post_rasterize_post_game(void);
void game_engine_post_rasterize_objects(void);
void game_engine_post_rasterize(void);
boolean match_game_type(long game_engine_type, long count, short const *game_type);
void game_engine_load_stage(char const *map_name);
void game_engine_playlist_begin(void);
boolean game_engine_get_current_stage(struct game_variant *variant, char *map_name);
void game_engine_update_player_always_invis(long player_index);
void game_engine_end_game(void);
void game_engine_update_non_deterministic(real seconds_elapsed);
void game_engine_update(void);
boolean game_engine_allow_pick_up(long unit_index, long item_index);
void game_engine_player_damaged_player(long killing_player_index, long dead_player_index, boolean friendly_fire);
void game_engine_player_killed(long killing_player_index, long killing_object_index, long dead_player_index, boolean friendly_fire);
void game_show_score_team(long team_index, long message);
void game_show_score_you_ally_enemy(long player_index, long you_message, long ally_message, long enemy_message, long message_data);
void game_show_score_extended(long player_index, long message, long message_data);
void game_show_score(long player_index, long message);
boolean game_engine_player_is_out_of_lives(long player_index);
boolean game_engine_should_spawn_player(long player_index);
long game_engine_player_get_team_index(long player_index);
long find_netgame_flags(real_point3d const *location, real distance, real height_delta, short type, short team, long max, long *flags);
long find_netgame_flag(real_point3d const *location, real distance, real height_delta, short type, short team);
void game_engine_prespawn_player_update(long player_index);
boolean game_engine_running(void);
void game_engine_postspawn_player_update(long player_index);
real game_engine_get_damage_multiplier(long attacker_index, long defender_index);
boolean game_engine_can_score(void);
boolean game_engine_force_single_screen(void);
real get_blink_alpha(void);
void game_engine_nonplayer_post_rasterize(void);
real game_engine_get_distance_rating_for_spawn(long player_index, struct scenario_player *starting_location);
real game_engine_get_starting_location_rating(long player_index, struct scenario_player *starting_location);
void game_engine_rasterize_message(wchar_t const *string, real alpha);
boolean game_engine_picking_up(long unit_index, long item_index);
short game_engine_player_get_custom_motion_sensor_positions(long player_index, real_point2d *points, char *goal_indices, short max_points);
long get_flag_definition_index(void);
long get_ball_definition_index(void);
void game_engine_switch_to_postgame(void);
struct game_variant *game_engine_get_variant(void);
boolean game_engine_get_goal_in_use(short index);
real_point3d game_engine_get_goal_position(short index);
void game_engine_set_goal_position(short index, real_point3d const *position, real vertical_offset, char const *descriptor, long player_index, short team_index, long ignore_player_index);
void game_engine_clear_goal_position(short index);
struct game_variant game_engine_get_variant_by_name(char const *name);
void game_engine_render_nav_points(short local_player_index);
boolean game_engine_infinite_grenades(long player_index);
boolean game_engine_has_teams(void);
boolean game_engine_display_team_indicators(void);
boolean game_engine_has_shield(long player_index);
long weapon_definition_index_to_list_index(long definition_index);
long list_index_to_weapon_definition_index(long list_index);
long game_engine_remap_vehicle(long vehicle_definition_index);
long game_engine_remap_equipment(long equipment_definition_index);
long game_engine_remap_weapon(long weapon_definition_index);
long game_engine_remap_object_definition(long definition_index);
boolean game_engine_man_out(long player_index);
boolean game_engine_get_state_message(long player_index, wchar_t *buffer, long buffer_length);
void game_engine_state_message(long player_index, long message, long message_data);
real_rgb_color game_engine_player_get_change_color(long player_index);
boolean game_engine_player_has_flag(long player_index);
void game_engine_player_depower_active_camo(long player_index);
wchar_t *get_place_name(struct game_engine_place place);
struct game_engine_place game_engine_get_place(long player_index, enum get_score_type get_score_type);
boolean game_engine_hud_draw_motion_sensor(long player_index);
boolean game_engine_test_flag(long flag);
boolean game_engine_test_trait(long player_index, long trait);
long game_engine_did_player_win_default(long player_index);
long game_engine_did_player_win(long player_index);
void game_engine_playlist_initialize(void);
void game_engine_playlist_next(long unused1, long unused2, long unused3);
struct game_variant build_game_variant_slayer(void);
struct game_variant build_game_variant_slayer_pro(void);
struct game_variant build_game_variant_elimination(void);
struct game_variant build_game_variant_phantoms(void);
struct game_variant build_game_variant_endurance(void);
struct game_variant build_game_variant_rockets(void);
struct game_variant build_game_variant_snipers(void);
struct game_variant build_game_variant_team_slayer(void);
struct game_variant build_game_variant_oddball(void);
struct game_variant build_game_variant_team_oddball(void);
struct game_variant build_game_variant_reverse_tag(void);
struct game_variant build_game_variant_accumulation(void);
struct game_variant build_game_variant_juggernaut(void);
struct game_variant build_game_variant_stalker(void);
struct game_variant build_game_variant_king(void);
struct game_variant build_game_variant_king_pro(void);
struct game_variant build_game_variant_crazy_king(void);
struct game_variant build_game_variant_team_king(void);
struct game_variant build_game_variant_ctf(void);
struct game_variant build_game_variant_ctf_pro(void);
struct game_variant build_game_variant_invasion(void);
struct game_variant build_game_variant_iron_ctf(void);
struct game_variant build_game_variant_race(void);
struct game_variant build_game_variant_rally(void);
struct game_variant build_game_variant_team_race(void);
struct game_variant build_game_variant_team_rally(void);
void game_engine_override_map_name(char const *map_name);
void game_engine_override_game_variant(struct game_variant const *variant);
boolean game_engine_hud_draw_messages(long player_index);
boolean game_engine_player_has_stealth_weapon(long player_index);
void game_engine_weapon_fired(long player_index);
void ticks_to_unicode_time_string(long ticks, long count, wchar_t *buffer);
void game_engine_flag_reset(long weapon_index, real_point3d const *position);
void game_engine_variant_cleanup(struct game_variant *variant);
boolean game_engine_allow_pause(void);
boolean game_engine_draw_object_in_motion_sensor(long unit_index);
boolean game_engine_allow_dynamic_lighting(void);
boolean game_engine_allow_integrated_lights(long object_index);
boolean game_engine_force_autopickup(long unit_index, long weapon_index);

/* ---------- prototypes/GAME_ENGINE_MULTIPLAYER_SOUNDS.C */

void game_engine_update_multiplayer_sound(void);
void game_engine_play_multiplayer_sound(long index);
void game_engine_intialize_queued_sounds(void);

/* ---------- globals */

extern struct _game_engine_globals game_engine_globals;
extern struct play_stage global_stage;
extern struct netgame_goal global_goal[MAXIMUM_NETGAME_GOALS];
extern struct game_variant global_variant;
extern struct game_engine *game_engine;
extern short debug_player_color;

extern long timeout_for_endgame_sound;

/* ---------- public code */

#endif // __GAME_ENGINE_H
