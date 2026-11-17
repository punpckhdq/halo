/*
GAME.C
*/

/* ---------- headers */

#include "cseries.h"
#include "game.h"
#include "players.h"
#include "game_engine_list.h"
#include "render.h"
#include "rasterizer/common/rasterizer_common.h"
#include "network_game_globals.h"
#include "input.h"
#include "console.h"
#include "game_state.h"
#include "shaders.h"
#include "network_messages.h"
#include "collisions.h"
#include "sound_manager.h"
#include "text_group.h"
#include "vehicles.h"
#include "encounters.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "director.h"
#include "ui_widget.h"
#include "game_sound.h"
#include "structures.h"
#include "observer.h"
#include "editor_stubs.h"
#include "hud.h"
#include "effects.h"
#include "network_server_message_handler.h"
#include "network_server_manager.h"
#include "object_types.h"
#include "player_ui.h"
#include "damage_effect_definitions.h"
#include "strings/resource.h"
#include "cinematics.h"
#include "devices.h"
#include "shell.h"
#include "projectiles.h"
#include "physics.h"
#include "player_effects.h"
#include "ai_script.h"
#include "first_person_weapons.h"
#include "hs.h"
#include "point_physics.h"
#include "recorded_animations.h"
#include "decals.h"
#include "equipment.h"
#include "widgets.h"
#include "bink_playback.h"
#include "particles.h"
#include "sound_classes.h"
#include "contrails.h"
#include "saved_films.h"
#include "particle_systems.h"
#include "progress_bar.h"
#include "weather_particle_systems.h"
#include "telnet_console.h"
#include "interface.h"
#include "event_manager.h"
#include "cheats.h"
#include "units.h"

/* ---------- constants */

/* ---------- macros */

#define SECONDS_PER_TICK (1.f/TICKS_PER_SECOND) /* fake name */
#define DEFAULT_RANDOM_SEED 0xdeadbeef /* fake name */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static struct
{
	boolean map_loaded;
	boolean active;
	boolean players_are_double_speed;
	boolean map_load_in_progress;
	real loading_progress;
	struct game_options options;
} *game_globals;

struct game_variant game_variant_global;

char const *global_game_difficulty_level_names[NUMBER_OF_GAME_DIFFICULTY_LEVELS] = {"easy", "normal", "hard", "impossible"};

static struct profile_section game_update_section = {"game_update", NONE, TRUE}; /* fake name */

/* ---------- public code */

void game_initialize(
	void)
{
	game_globals = game_state_malloc("game globals", NULL, sizeof(*game_globals));
	memset(game_globals, 0, sizeof(*game_globals));
	memset(&game_variant_global, 0, sizeof(game_variant_global));
	real_math_reset_precision();
	game_time_initialize();
	game_engine_initialize(&game_variant_global);
	game_allegiance_initialize();
	interface_initialize();
	scenario_initialize();
	director_initialize();
	observer_initialize();
	render_initialize();
	objects_initialize();
	structures_initialize();
	breakable_surfaces_initialize();
	decals_initialize();
	collision_log_initialize();
	players_initialize();
	contrails_initialize();
	particles_initialize();
	effects_initialize();
	weather_particle_systems_initialize();
	particle_systems_initialize();
	sound_classes_initialize();
	game_sound_initialize();
	rumble_initialize();
	player_effect_initialize();
	ai_initialize();
	editor_initialize();
	ui_widgets_initialize();
	hs_initialize();
	recorded_animations_initialize();
	cheats_initialize();
	transport_initialize();
	telnet_console_initialize();
	initialize_network_game_packets();
	cinematic_initialize();
	saved_game_files_initialize();
	event_manager_initialize();
	input_abstraction_initialize();
	player_ui_initialize();
	bink_playback_initialize();
	progress_bar_initialize();

	return;
}

void game_dispose(
	void)
{
	recorded_animations_dispose();
	cinematic_dispose();
	hs_dispose();
	cheats_dispose();
	ui_widgets_dispose();
	editor_dispose();
	ai_dispose();
	player_effect_dispose();
	rumble_dispose();
	game_sound_dispose();
	sound_classes_dispose();
	game_engine_dispose();
	particle_systems_dispose();
	weather_particle_systems_dispose();
	effects_dispose();
	particles_dispose();
	contrails_dispose();
	players_dispose();
	decals_dispose();
	breakable_surfaces_dispose();
	structures_dispose();
	render_dispose();
	objects_dispose();
	director_dispose();
	interface_dispose();
	game_allegiance_dispose();
	game_time_dispose();
	saved_game_files_dispose();
	event_manager_dispose();
	input_abstraction_dispose();
	player_ui_dispose();
	game_state_dispose();
	telnet_console_dispose();
	transport_dispose();
	bink_playback_dispose();
	progress_bar_dispose();

	return;
}

void game_precache_new_map(
	char *map_name,
	boolean blocking)
{
	if (!cache_files_precache_map_loaded(map_name))
	{
		if (cache_files_precache_in_progress() && !cache_files_precache_is_copying_map(map_name))
		{
			if (blocking)
			{
				cache_files_precache_map_end();
			}
			else
			{
				cache_files_precache_map_queue_end();
				main_queue_map_name(map_name);
			}
		}

		if (!cache_files_precache_in_progress())
		{
			if (!cache_files_precache_map_begin(map_name, blocking))
			{
				error(_error_silent, "shouldn't be here... map '%s' doesn't exist", map_name);

				if (blocking)
				{
					match_vhalt("c:\\halo\\SOURCE\\game\\game.c", 249, "read the last error message for which map failed to load");
				}
			}
		}

		cache_files_precache_set_priority(blocking);

		if (blocking)
		{
			long status;

			game_globals->map_load_in_progress = TRUE;
			game_globals->loading_progress = 0.f;
			match_assert("c:\\halo\\SOURCE\\game\\game.c", 261, cache_files_precache_in_progress() && cache_files_precache_is_copying_map(map_name));
			ui_widget_load_progress_widget();
			progress_bar_begin(global_scenario_index != NONE);

			do
			{
				status = cache_files_precache_map_status(&game_globals->loading_progress);
				main_pregame_render();
				main_rasterizer_throttle();
				main_present_frame();
			}
			while (status == _cached_map_file_in_progress);

			progress_bar_end();
			ui_widgets_close_all();

			if (status == _cached_map_file_failed)
			{
				display_error_damaged_media();
			}

			cache_files_precache_map_end();
			match_assert("c:\\halo\\SOURCE\\game\\game.c", 282, cache_files_precache_map_loaded(map_name));
			game_globals->map_load_in_progress = FALSE;
			game_globals->loading_progress = 1.f;
		}
	}

	if (blocking)
	{
		match_assert("c:\\halo\\SOURCE\\game\\game.c", 291, cache_files_precache_map_loaded(map_name));
		main_save_current_solo_map(map_name);
		main_queue_map_name(NULL);

		if (cache_files_precache_in_progress())
		{
			cache_files_precache_map_end();
		}

		if (player_spawn_count == 1)
		{
			player_ui_remember_player1_profile(TRUE);
		}
	}

	return;
}

boolean game_map_loading_in_progress(
	real *progress)
{
	if (progress)
	{
		*progress = game_globals->loading_progress;
	}

	return game_globals->map_load_in_progress;
}

boolean game_load(
	struct game_options *options)
{
	match_assert("c:\\halo\\SOURCE\\game\\game.c", 402, !game_globals->active);
	match_assert("c:\\halo\\SOURCE\\game\\game.c", 403, !game_globals->map_loaded);
	match_assert("c:\\halo\\SOURCE\\game\\game.c", 404, game_options_verify(options));

	random_seed_debug_log(TRUE);
	memcpy(&game_globals->options, options, sizeof(*options));

	if (scenario_load(options->map_name))
	{
		game_globals->map_loaded = TRUE;
	}

	return game_globals->map_loaded;
}

void game_unload(
	void)
{
	if (cache_files_precache_in_progress())
	{
		long status;

		game_globals->map_load_in_progress = TRUE;
		ui_widget_load_progress_widget();

		do
		{
			status = cache_files_precache_map_status(&game_globals->loading_progress);
			main_pregame_render();
			main_rasterizer_throttle();
			main_present_frame();
		}
		while (status == _cached_map_file_in_progress);

		ui_widgets_close_all();

		if (status == _cached_map_file_failed)
		{
			display_error_damaged_media();
		}

		cache_files_precache_map_end();
	}

	if (game_globals->map_loaded)
	{
		scenario_unload();
		random_seed_debug_log(FALSE);
		game_globals->map_loaded = FALSE;
	}

	return;
}

void game_initialize_for_new_map(
	void)
{
	match_assert("c:\\halo\\SOURCE\\game\\game.c", 465, game_globals->map_loaded);
	match_assert("c:\\halo\\SOURCE\\game\\game.c", 466, !game_globals->active);

	set_random_seed(game_globals->options.random_seed);
	game_engine_dispose();
	game_engine_initialize(&game_variant_global);
	real_math_reset_precision();
	rasterizer_initialize_for_new_map();
	game_state_initialize_for_new_map();
	game_time_initialize_for_new_map();
	interface_initialize_for_new_map();
	game_allegiance_initialize_for_new_map();
	players_initialize_for_new_map();
	scenario_initialize_for_new_map();
	objects_initialize_for_new_map();
	render_initialize_for_new_map();
	structures_initialize_for_new_map();
	breakable_surfaces_initialize_for_new_map();
	decals_initialize_for_new_map();
	director_initialize_for_new_map();
	observer_initialize_for_new_map();
	contrails_initialize_for_new_map();
	particles_initialize_for_new_map();
	effects_initialize_for_new_map();
	particle_systems_initialize_for_new_map();
	sound_initialize_for_new_map();
	sound_classes_initialize_for_new_map();
	game_sound_initialize_for_new_map();
	weather_particle_systems_initialize_for_new_map();
	point_physics_initialize_for_new_map();
	game_engine_initialize_for_new_map();
	game_statistics_start();
	update_server_new();
	player_control_initialize_for_new_map();
	rumble_initialize_for_new_map();
	player_effect_initialize_for_new_map();
	ai_initialize_for_new_map();
	console_initialize_for_new_map();
	editor_initialize_for_new_map();
	cinematic_initialize_for_new_map();
	hs_initialize_for_new_map();
	recorded_animations_initialize_for_new_map();
	cheats_initialize_for_new_map();
	game_globals->active = TRUE;
	objects_place();

	if (!game_in_editor())
	{
		ai_place();
	}

	ui_widgets_safe_to_load(TRUE);

	return;
}

void game_dispose_from_old_map(
	void)
{
	rasterizer_dispose_from_old_map();
	game_state_dispose_from_old_map();
	cheats_dispose_from_old_map();
	recorded_animations_dispose_from_old_map();
	hs_dispose_from_old_map();
	cinematic_dispose_from_old_map();
	editor_dispose_from_old_map();
	ai_dispose_from_old_map();
	player_effect_dispose_from_old_map();
	rumble_dispose_from_old_map();
	point_physics_dispose_from_old_map();
	particle_systems_dispose_from_old_map();
	weather_particle_systems_dispose_from_old_map();
	decals_dispose_from_old_map();
	breakable_surfaces_dispose_from_old_map();
	structures_dispose_from_old_map();
	render_dispose_from_old_map();
	objects_dispose_from_old_map();
	director_dispose_from_old_map();
	observer_dispose_from_old_map();
	interface_dispose_from_old_map();
	players_dispose_from_old_map();
	contrails_dispose_from_old_map();
	particles_dispose_from_old_map();
	effects_dispose_from_old_map();
	game_sound_dispose_from_old_map();
	sound_classes_dispose_from_old_map();
	sound_dispose_from_old_map();
	game_allegiance_dispose_from_old_map();
	update_server_delete();
	game_engine_dispose_from_old_map();
	scenario_dispose_from_old_map();
	game_time_dispose_from_old_map();
	ui_widgets_close_all();
	ui_widgets_safe_to_load(FALSE);
	game_globals->active = FALSE;

	return;
}

void game_frame(
	real dt)
{
	real game_dt = game_globals->players_are_double_speed ? dt * 0.5f : dt;

	match_assert("c:\\halo\\SOURCE\\game\\game.c", 585, game_globals->active);
	collision_log_begin_period(_collision_period_render_frame);
	particles_update(game_dt);
	contrails_update(game_dt);
	particle_systems_update(game_dt);
	widgets_update(game_dt);
	game_sound_update(game_dt);
	scenario_frame_update(game_dt);
	rasterizer_frame_update(game_dt);
	numeric_countdown_timer_update();
	collision_log_end_period();

	return;
}

void remove_quitting_players_from_game(
	void)
{
	if (game_engine_running())
	{
		long game_time = game_time_get();
		struct data_iterator player_iterator;
		struct player_datum *player;

		data_iterator_new(&player_iterator, player_data);

		while (player = (struct player_datum *)data_iterator_next(&player_iterator))
		{
			if (player->quit_out_of_game_time != NONE && !player->quit_out_of_game)
			{
				if (game_time == player->quit_out_of_game_time)
				{
					player->quit_out_of_game = TRUE;

					if (player->unit_index != NONE)
					{
						unit_get(player->unit_index);
						unit_kill_no_statistics(player->unit_index);
					}
				}
				else if (game_time > player->quit_out_of_game_time)
				{
					error(_error_silent, "player %x failed to quit, wanted %d is %d", player_iterator.index, player->quit_out_of_game_time, game_time);
				}
			}
		}
	}

	return;
}

void game_tick(
	void)
{
	profile_tick_start();
	collision_log_begin_period(_collision_period_game_tick);
	real_math_reset_precision();
	profile_enter(game_update_section);
	match_assert("c:\\halo\\SOURCE\\game\\game.c", 653, game_globals->active);
	remove_quitting_players_from_game();
	game_allegiance_update();
	units_update();
	ai_update();
	players_update_before_game();
	effects_update(game_globals->players_are_double_speed ? SECONDS_PER_TICK / 2 : SECONDS_PER_TICK);
	lock_global_random_seed();
	rumble_update();
	first_person_weapons_update();
	unlock_global_random_seed();
	game_engine_update();
	editor_update();
	hs_update();
	recorded_animations_update();
	objects_update();
	players_update_after_game();
	hud_update();
	player_effect_update();
	profile_exit(game_update_section);
	collision_log_end_period();
	profile_tick_end();

	return;
}

void game_options_new(
	struct game_options *options)
{
	memset(options, 0, sizeof(*options));
	options->code_version = 0;
	options->difficulty = _game_difficulty_level_normal;
	options->random_seed = DEFAULT_RANDOM_SEED;

	return;
}

boolean game_options_verify(
	struct game_options *options)
{
	return (boolean)(options->difficulty >= 0 && options->difficulty < NUMBER_OF_GAME_DIFFICULTY_LEVELS);
}

void game_initial_pulse(
	void)
{
	struct data_iterator player_iterator;

	data_iterator_new(&player_iterator, player_data);

	while (data_iterator_next(&player_iterator))
	{
		game_engine_player_added(player_iterator.index);
	}

	game_engine_game_starting();

	return;
}

void game_set_players_are_double_speed(
	boolean players_are_double_speed)
{
	game_globals->players_are_double_speed = players_are_double_speed;

	return;
}

boolean game_players_are_double_speed(
	void)
{
	return game_globals->players_are_double_speed;
}

void game_difficulty_level_set(
	short difficulty)
{
	game_globals->options.difficulty = difficulty;

	return;
}

short game_difficulty_level_get(
	void)
{
	return game_globals->options.difficulty;
}

short game_difficulty_level_get_ignore_easy(
	void)
{
	return MAX(game_globals->options.difficulty, _game_difficulty_level_normal);
}

void game_set_game_variant_from_name(
	char const *name)
{
	struct game_variant variant = game_engine_get_variant_by_name(name);

	game_set_game_variant(&variant);

	return;
}

void game_set_game_variant(
	struct game_variant *variant)
{
	if (NULL == variant)
	{
		memset(&game_variant_global, 0, sizeof(game_variant_global));
	}
	else
	{
		game_variant_global = *variant;
	}

	return;
}

void game_set_game_engine_index(
	short index)
{
	match_assert("c:\\halo\\SOURCE\\game\\game.c", 785, !"this is broken and should get updated for the variants, ask michael");

	return;
}

boolean game_all_quiet(
	void)
{
	return (boolean)(!dangerous_projectiles_near_player() && !dangerous_items_near_player() && !dangerous_effects_near_player() && !any_unit_is_dangerous() && !ai_enemies_can_see_player());
}

boolean game_safe_to_save(
	void)
{
	boolean safe = TRUE;

	if (ai_enemies_can_see_player())
	{
		if (debug_game_save)
		{
			console_warning("not safe to save: ai_enemies_can_see_player");
		}

		safe = FALSE;
	}
	else if (dangerous_projectiles_near_player())
	{
		if (debug_game_save)
		{
			console_warning("not safe to save: dangerous_projectiles_near_player");
		}

		safe = FALSE;
	}
	else if (dangerous_items_near_player())
	{
		if (debug_game_save)
		{
			console_warning("not safe to save: dangerous_items_near_player");
		}

		safe = FALSE;
	}
	else if (dangerous_effects_near_player())
	{
		if (debug_game_save)
		{
			console_warning("not safe to save: dangerous_effects_near_player");
		}

		safe = FALSE;
	}
	else if (any_unit_is_dangerous())
	{
		if (debug_game_save)
		{
			console_warning("not safe to save: any_unit_is_dangerous");
		}

		safe = FALSE;
	}
	else if (any_player_is_in_the_air())
	{
		if (debug_game_save)
		{
			console_warning("not safe to save: any_player_is_in_the_air");
		}

		safe = FALSE;
	}
	else if (any_player_is_dead())
	{
		if (debug_game_save)
		{
			console_warning("not safe to save: any_player_is_dead");
		}

		safe = FALSE;
	}
	else if (vehicle_moving_near_any_player())
	{
		if (debug_game_save)
		{
			console_warning("not safe to save: vehicle_moving_near_any_player");
		}

		safe = FALSE;
	}

	return safe;
}

boolean game_safe_to_speak(
	void)
{
	return (boolean)(!dangerous_projectiles_near_player() && !any_player_is_dead());
}

boolean game_is_cooperative(
	void)
{
	return player_spawn_count > 1;
}

/* ---------- private code */
