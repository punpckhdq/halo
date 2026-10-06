/*
MAIN.H
*/

#ifndef __MAIN_H
#define __MAIN_H
#pragma once

/* ---------- constants */

enum
{
	_single_player_map_a10 = 0,
	_single_player_map_a30,
	_single_player_map_a50,
	_single_player_map_b30,
	_single_player_map_b40,
	_single_player_map_c10,
	_single_player_map_c20,
	_single_player_map_c40,
	_single_player_map_d20,
	_single_player_map_d40,
	NUMBER_OF_SINGLE_PLAYER_LEVELS,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/MAIN.C */

real main_get_seconds_elapsed(void);
boolean gamepad_button_is_down(short button_index);
void main_loop(void);
void main_loop_of_death(void);
void game_connection_set(short new_connection);
short game_connection(void);
void main_disallow_persistent_storage(void);
void main_set_map_name(char const *name);
void main_defer_map_map_change(void);
void main_set_multiplayer_map_name(char const *name);
char *main_get_map_name(void);
char *main_get_multiplayer_map_name(void);
void main_set_difficulty(short difficulty);
short main_get_difficulty(void);
void main_save_current_solo_map(char const *map_name);
void main_load_last_solo_map(void);
void main_reset_map(void);
void main_revert_map(void);
void main_skip_cinematic(void);
void main_save_map_nonsafe(void);
boolean main_saving_map(void);
void main_save_cancel(void);
void main_save_map_no_timeout(void);
void main_save_map_safe(void);
void main_won_map(void);
void main_lost_map(void);
void main_respawn(boolean force_respawn);
void main_save_core(void);
void main_save_core_name(char const *core_name);
void main_load_core(void);
void main_load_core_at_startup(void);
void main_load_core_name(char const *core_name);
void main_load_core_name_at_startup(char const *core_name);
void main_switch_structure_bsp(short new_structure_bsp_index);
void main_skip(short ticks);
void main_queue_map_name(char const *new_name);
void main_goto_main_menu(void);
void main_load_ui_scenario(boolean precache_resources);
void main_menu_precache_resources(void);
void main_menu_load(void);
void main_menu_unload(void);
void main_menu_ensure_player_queues_exist(void);
boolean main_menu_fade_active(void);
void main_menu_switch_to_single_player(void);
void main_set_game_connection_to_film_playback(void);
short main_get_solo_level_from_name(char const *name);
short main_get_current_solo_level(void);
char const *main_get_solo_level_name(short level);
void main_run_demos(void);
void main_roll_credits(void);
void compute_window_bounds(long player_index, long num_players, rectangle2d *pixel_bounds, rectangle2d *safe_frame_bounds);
void main_pregame_render(void);
void set_window_camera_values(struct render_window *current_window, struct observer_result const *observer);
short main_get_window_count(void);
void main_present_frame(void);
void main_rasterizer_throttle(void);
boolean main_taking_screenshot(void);
void main_movie_start(real frames_per_second);
void main_movie_stop(void);
void main_stop_time(void);
void main_start_time(void);
void main_framerate_render(void);
void main_crash(char const *str);
void main_print_version(void);
void main_vertical_blank_interrupt_handler(unsigned long context);

/* ---------- globals */

extern short global_difficulty_level;
extern short player_spawn_count;
extern boolean global_frame_rate_throttle;
extern short global_screenshot_size;

extern boolean debug_force_frame_rate_update;
extern boolean debug_no_drawing;
extern boolean debug_game_save;
extern boolean debug_frame_rate;
extern boolean display_framerate;
extern boolean display_vblank_deltas;
extern boolean display_precache_progress;
extern short global_screenshot_count;

/* ---------- public code */

#endif // __MAIN_H
