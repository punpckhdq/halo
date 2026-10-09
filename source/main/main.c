/*
MAIN.C
*/

/* ---------- headers */

#include "cseries.h"
#include "main.h"
#include "console.h"
#include "debug_keys.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "actor_definitions.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "game_state.h"
#include "network_messages.h"
#include "collision_usage.h"
#include "sound_manager.h"
#include "text_group.h"
#include "vehicles.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "director.h"
#include "ui_widget.h"
#include "weapons.h"
#include "sound_definitions.h"
#include "bipeds.h"
#include "texture_cache.h"
#include "observer.h"
#include "editor_stubs.h"
#include "hud.h"
#include "network_client_manager.h"
#include "font_group.h"
#include "terminal.h"
#include "network_server_manager.h"
#include "player_ui.h"
#include "strings/resource.h"
#include "cinematics.h"
#include "shell.h"
#include "player_effects.h"
#include "hud_messaging.h"
#include "predicted_resources.h"
#include "hs.h"
#include "build_number.h"
#include "bink_playback.h"
#include "camera_scripting.h"
#include "attract_mode.h"
#include "saved_films.h"
#include "marketing_and_strategic_business_development.h"
#include "telnet_console.h"
#include "dialogs.h"
#include "bitmaps.h"
#include "errors.h"
#include "profile.h"
#include "cheats.h"
#include "game_engine.h"
#include "cache_files.h"

/* ---------- structures */

struct _main_globals
{
	unsigned long last_time_msec;
	__int64 last_vblank_index;
	__int64 last_initial_vblank_index;
	__int64 last_achievable_vblank_index;
	__int64 last_present_vblank_index;
	real seconds_elapsed;
	short connection;
	word screenshot_identifier;
	struct bitmap_data *movie;
	long recording_start_tick;
	long recording_stop_tick;
	long recording_frame_index;
	real recording_dt;
	boolean reset_map;
	boolean rename_map;
	boolean revert_map;
	boolean skip_cinematic;
	boolean save_map;
	boolean save_map_safely;
	boolean save_map_timeout;
	boolean saving_map;
	long ticks_until_next_save_check;
	long ticks_unable_to_save;
	unsigned long map_change_load_timer;
	short safe_intervals;
	boolean won_map;
	boolean lost_map;
	boolean respawn;
	boolean save_core;
	boolean load_core;
	boolean load_core_at_startup;
	short switch_to_structure_bsp_index;
	boolean main_menu_scenario_loaded;
	boolean want_to_be_at_main_menu;
	boolean run_xdemos;
	boolean playback_last_recording;
	boolean halt_time_scale;
	boolean restart_time;
	boolean load_last_solo_level;
	boolean cutscene_skip;
	short skip_ticks;
	short loss_timer;
	short respawn_timer;
	boolean queue_map;
	byte pad0[3];
	boolean solo_try_and_load_from_persistent_storage;
	char soloplayer_map_name[MAXIMUM_FILENAME_LENGTH+1];
	char multiplayer_map_name[MAXIMUM_FILENAME_LENGTH+1];
	char queued_map_name[MAXIMUM_FILENAME_LENGTH+1];
	char core_file_name[64];
	short vblank_interval_current;
	short vblank_interval_minimum;
	boolean vblank_interval_held;
	short vblank_failure_count[6];
	__int64 vblank_last_failure_time[6];
	unsigned long *vblank_flip_counter;
	short vblank_flip_delta_next_index;
	short vblank_flip_deltas[15];
	char time_debug_string[512]; /* fake name */
};

/* ---------- prototypes */

static long sort_desired_local_player_controllers(short const *a, short const *b);
static void create_local_players(void);
static void main_queue_map_private(void);
static void compute_subframe_counts(long num_players, long *out_horizontal_count, long *out_vertical_count);
static void main_game_render(double time_delta_since_tick_sec);
static void main_new_map(struct game_options *options);
static void main_change_map_name(void);
static void main_revert_map_private(void);
static void main_skip_cinematic_private(void);
static void main_skip_private(void);
static void main_saving_map_private(void);
static void main_save_map_private(void);
static void main_switch_to_structure_bsp_private(void);
static void main_lost_map_private(void);
static void main_respawn_private(void);
static void main_won_map_private(void);
static void main_load_core_private(void);
static void main_save_core_private(void);
static void main_reset_map_private(void);
static void main_run_demos_private(void);
static void main_frame_rate_debug(void);
static void main_setup_connection(void);
static void main_exit(void);
static void main_initialize_time(void);
static void main_reset_time(void);
static boolean main_time_is_throttled(void);
static void main_update_time(void);
static void screenshot_render(struct render_window *window);
static void screenshot_record(struct bitmap_data *screen, struct file_reference *file);

/* ---------- globals */

short global_difficulty_level = _game_difficulty_level_normal;
short player_spawn_count = 1;
boolean global_frame_rate_throttle = TRUE;
short global_screenshot_size = 1;

static struct _main_globals main_globals;

boolean debug_force_frame_rate_update = FALSE;
boolean debug_no_drawing = FALSE;
boolean debug_game_save = FALSE;
boolean debug_frame_rate = FALSE;
boolean display_framerate = FALSE;
boolean display_vblank_deltas = FALSE;
boolean display_precache_progress = FALSE;
short global_screenshot_count = 0;

/* ---------- public code */

real main_get_seconds_elapsed(
	void)
{
	return main_globals.seconds_elapsed;
}

boolean gamepad_button_is_down(
	short button_index)
{
	short gamepad_index;
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\main\\main.c", 245, button_index>=0 && button_index<NUMBER_OF_GAMEPAD_BUTTONS);

	for (gamepad_index = 0; gamepad_index<MAXIMUM_GAMEPADS; gamepad_index++)
	{
		if (input_has_gamepad(gamepad_index))
		{
			break;
		}
	}

	if (gamepad_index<MAXIMUM_GAMEPADS)
	{
		struct gamepad_state const *state = input_get_gamepad_state(gamepad_index);

		result = state->buttons[button_index]>0;
	}

	return result;
}

void main_loop(
	void)
{
	if (!game_in_editor())
	{
		strncpy(main_globals.soloplayer_map_name, "levels\\b30\\b30", NUMBEROF(main_globals.soloplayer_map_name)-1);
		main_globals.soloplayer_map_name[NUMBEROF(main_globals.soloplayer_map_name)-1] = '\0';
	}

	main_globals.want_to_be_at_main_menu = !game_in_editor();
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.halt_time_scale = TRUE;

	console_initialize();
	debug_keys_initialize();
	game_initialize();
	console_startup();
	main_setup_connection();
	main_initialize_time();

	while (TRUE)
	{
		if (!game_in_editor())
		{
			if (main_globals.switch_to_structure_bsp_index!=NONE)
			{
				main_switch_to_structure_bsp_private();
			}

			if (main_globals.lost_map)
			{
				main_lost_map_private();
			}

			if (main_globals.won_map)
			{
				main_won_map_private();
			}

			if (main_globals.respawn)
			{
				main_respawn_private();
			}

			if (main_globals.saving_map)
			{
				main_saving_map_private();
			}

			if (main_globals.rename_map)
			{
				main_change_map_name();
			}

			if (main_globals.revert_map)
			{
				main_revert_map_private();
			}

			if (main_globals.skip_cinematic)
			{
				main_skip_cinematic_private();
			}

			if (main_globals.reset_map)
			{
				main_reset_map_private();
			}

			if (main_globals.save_core)
			{
				main_save_core_private();
			}

			if (main_globals.load_core)
			{
				main_load_core_private();
			}

			if (main_globals.want_to_be_at_main_menu)
			{
				main_menu_load();
			}

			if (main_globals.load_last_solo_level)
			{
				main_load_last_solo_map();
			}

			if (main_globals.run_xdemos)
			{
				main_run_demos_private();
			}

			if (main_globals.cutscene_skip)
			{
				main_skip_private();
			}

			if (main_globals.queue_map)
			{
				main_queue_map_private();
			}
		}
		else
		{
			if (main_globals.reset_map)
			{
				main_reset_map_private();
			}
		}

		profile_frame_start();
		input_frame_begin();
		input_update();
		input_abstraction_update();
		shell_idle();
		event_manager_update();
		telnet_console_process();

		if (!shell_application_is_paused())
		{
			boolean render = TRUE;
			long connection = game_connection();

			if (connection==_game_connection_network_client)
			{
				if (!network_game_client_start_frame())
				{
					display_error_when_main_menu_loaded(_error_network_connection_lost);
					error(_error_silent, "the game host went down");
					network_game_abort();
				}
			}
			else if (connection==_game_connection_network_server)
			{
				if (!network_game_client_start_frame())
				{
					display_error_when_main_menu_loaded(_error_network_generic);
					error(_error_silent, "the game host went down");
					network_game_abort();
				}
				else if (!network_game_server_start_frame())
				{
					display_error_when_main_menu_loaded(_error_network_generic);
					error(_error_silent, "the game host went down");
					network_game_abort();
				}
			}
			else if (connection==_game_connection_film_playback)
			{
				error(_error_silent, "end of saved film");
				break;
			}

			main_update_time();
			process_ui_widgets();
			bink_playback_update();

			if ((!game_in_editor() && (input_key_is_down(_key_end) || input_key_is_down(_key_escape))) || editor_should_exit())
			{
				main_movie_stop();

				if (!game_engine_running())
				{
					main_reset_map();
				}
			}

			if (game_in_progress())
			{
				terminal_update();

				if (!console_update() || main_globals.connection!=_game_connection_local)
				{
					debug_keys_update();
					cheats_update();
					player_control_update(main_globals.halt_time_scale*main_globals.seconds_elapsed);

					switch (main_globals.connection)
					{
					case _game_connection_network_client:
					case _game_connection_network_server:
						if (!network_game_client_end_frame())
						{
							display_error_when_main_menu_loaded(_error_network_generic);
							network_game_abort();
						}
						break;
					}

					game_time_update(main_globals.halt_time_scale*main_globals.seconds_elapsed);

					render = main_globals.main_menu_scenario_loaded ||
						(main_globals.halt_time_scale && (game_time_get_paused() || game_time_get_elapsed()>0 || game_time_get_speed()<1.f));
					render &= !game_engine_running() || game_time_get()>=3;

					collision_log_continue_period(_collision_period_render_frame);
					director_update(main_globals.halt_time_scale*main_globals.seconds_elapsed);
					observer_update(main_globals.halt_time_scale*main_globals.seconds_elapsed);
					collision_log_end_period();
					game_engine_update_non_deterministic(main_globals.halt_time_scale*main_globals.seconds_elapsed);
				}

				if (main_globals.save_map)
				{
					main_save_map_private();
				}

				if (render && !debug_no_drawing)
				{
					profile_render_start();
					main_game_render(main_globals.seconds_elapsed);
					profile_render_end();
				}
			}
			else
			{
				profile_render_start();
				main_pregame_render();
				profile_render_end();
			}

			main_rasterizer_throttle();

			if (render && !debug_no_drawing)
			{
				main_present_frame();
			}
		}

		input_frame_end();
		profile_frame_end();
		main_frame_rate_debug();

		if (main_globals.restart_time)
		{
			main_globals.restart_time = FALSE;
			main_reset_time();
			main_globals.halt_time_scale = TRUE;
		}
	}

	main_exit();

	return;
}

void main_loop_of_death(
	void)
{
	while (TRUE)
	{
		input_frame_begin();
		input_update();
		shell_idle();
		event_manager_update();
		telnet_console_process();
		process_ui_widgets();
		main_pregame_render();
		main_rasterizer_throttle();
		main_present_frame();
		input_frame_end();
	}

	return;
}

void game_connection_set(
	short new_connection)
{
	main_globals.connection = new_connection;

	return;
}

short game_connection(
	void)
{
	return main_globals.connection;
}

void main_disallow_persistent_storage(
	void)
{
	main_globals.solo_try_and_load_from_persistent_storage = FALSE;

	return;
}

void main_set_map_name(
	char const *name)
{
	main_globals.want_to_be_at_main_menu = FALSE;
	strncpy(main_globals.soloplayer_map_name, name, NUMBEROF(main_globals.soloplayer_map_name)-1);
	main_globals.soloplayer_map_name[NUMBEROF(main_globals.soloplayer_map_name)-1] = '\0';
	main_globals.solo_try_and_load_from_persistent_storage = TRUE;

	if ((game_in_editor() || game_in_progress()) && game_connection()==_game_connection_local)
	{
		main_globals.rename_map = TRUE;
	}

	return;
}

void main_defer_map_map_change(
	void)
{
	main_globals.rename_map = FALSE;

	return;
}

void main_set_multiplayer_map_name(
	char const *name)
{
	strncpy(main_globals.multiplayer_map_name, name, NUMBEROF(main_globals.multiplayer_map_name)-1);
	main_globals.multiplayer_map_name[NUMBEROF(main_globals.multiplayer_map_name)-1] = '\0';
	cache_files_give_time_to_precache(main_globals.multiplayer_map_name);

	return;
}

char *main_get_map_name(
	void)
{
	return main_globals.soloplayer_map_name;
}

char *main_get_multiplayer_map_name(
	void)
{
	return main_globals.multiplayer_map_name;
}

void main_set_difficulty(
	short difficulty)
{
	if (difficulty>=0 && difficulty<NUMBER_OF_GAME_DIFFICULTY_LEVELS)
	{
		global_difficulty_level = difficulty;
	}

	return;
}

short main_get_difficulty(
	void)
{
	return global_difficulty_level;
}

void main_save_current_solo_map(
	char const *map_name)
{
	if (main_get_solo_level_from_name(map_name)!=NONE)
	{
		FILE *file = fopen("z:\\last_solo.txt", "w");

		if (file)
		{
			fwrite(map_name, 1, strlen(map_name)+1, file);
			fclose(file);
		}
		else
		{
			error(_error_silent, "Couldn't create a file to write the current solo map to");
		}
	}

	return;
}

void main_load_last_solo_map(
	void)
{
	if (main_globals.load_last_solo_level && !bink_playback_active())
	{
		char map_name[MAXIMUM_FILENAME_LENGTH+1];
		long count;
		boolean loaded = FALSE;
		FILE *file = fopen("z:\\last_solo.txt", "r");

		if (file)
		{
			count = fread(map_name, 1, NUMBEROF(map_name)-1, file);
			fclose(file);
			map_name[MIN(count, MAXIMUM_FILENAME_LENGTH)] = '\0';
			loaded = main_get_solo_level_from_name(map_name)!=NONE;
		}

		if (loaded)
		{
			main_set_map_name(map_name);
		}
		else
		{
			main_set_map_name(main_get_solo_level_name(_single_player_map_a10));
		}

		main_globals.rename_map = FALSE;
		main_globals.load_last_solo_level = FALSE;
	}

	return;
}

static long sort_desired_local_player_controllers(
	short const *a,
	short const *b)
{
	long result;
	short controller_a = *a;
	short controller_b = *b;

	if (controller_a==NONE && controller_b!=NONE)
	{
		result = 1;
	}
	else if (controller_b==NONE && controller_a!=NONE)
	{
		result = -1;
	}
	else if (controller_a>controller_b)
	{
		result = 1;
	}
	else
	{
		result = controller_a<controller_b ? -1 : 0;
	}

	return result;
}

static void create_local_players(
	void)
{
	if (main_globals.main_menu_scenario_loaded)
	{
		local_player_set_player_index(0, player_new(0, NONE, 0, NULL));
	}
	else
	{
		short controllers_used[MAXIMUM_GAMEPADS];
		short desired_controllers[MAXIMUM_GAMEPADS];
		short default_controllers[MAXIMUM_GAMEPADS];
		long i;

		memset(controllers_used, NONE, sizeof(controllers_used));
		memset(desired_controllers, NONE, sizeof(desired_controllers));

		for (i = 0; i<MAXIMUM_GAMEPADS; i++)
		{
			default_controllers[i] = (short)i;
		}

		match_assert("c:\\halo\\SOURCE\\main\\main.c", 741, game_connection() == _game_connection_local);

		for (i = 0; i<player_spawn_count; i++)
		{
			desired_controllers[i] = player_ui_get_single_player_local_player_controller(i);

			if (desired_controllers[i]==NONE)
			{
				desired_controllers[i] = default_controllers[i];
			}

			match_assert("c:\\halo\\SOURCE\\main\\main.c", 755, (desired_controllers[i]>=0) && (desired_controllers[i]<MAXIMUM_GAMEPADS));

			if (controllers_used[desired_controllers[i]]!=NONE)
			{
				long j;

				for (j = 0; j<MAXIMUM_GAMEPADS; j++)
				{
					if (controllers_used[j]==NONE)
					{
						desired_controllers[i] = j;
						controllers_used[desired_controllers[i]] = desired_controllers[i];
						break;
					}
				}

				match_assert("c:\\halo\\SOURCE\\main\\main.c", 768, j<MAXIMUM_GAMEPADS);
			}
			else
			{
				controllers_used[desired_controllers[i]] = desired_controllers[i];
			}
		}

		qsort(desired_controllers, NUMBEROF(desired_controllers), sizeof(desired_controllers[0]), (int(__cdecl *)(const void *, const void *))sort_desired_local_player_controllers);

		for (i = 0; i<player_spawn_count; i++)
		{
			short gamepad_index = desired_controllers[i];

			match_assert("c:\\halo\\SOURCE\\main\\main.c", 784, (gamepad_index>=0) && (gamepad_index<MAXIMUM_GAMEPADS));

			local_player_set_player_index(gamepad_index, player_new(0, NONE, gamepad_index, NULL));
		}
	}

	return;
}

void main_reset_map(
	void)
{
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.save_map = FALSE;
	main_globals.reset_map = TRUE;
	main_globals.lost_map = FALSE;

	return;
}

void main_revert_map(
	void)
{
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.save_map = FALSE;
	main_globals.revert_map = TRUE;
	main_globals.lost_map = FALSE;

	return;
}

void main_skip_cinematic(
	void)
{
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.save_map = FALSE;
	main_globals.skip_cinematic = TRUE;

	return;
}

void main_save_map_nonsafe(
	void)
{
	main_globals.save_map = TRUE;
	main_globals.save_map_safely = FALSE;

	return;
}

boolean main_saving_map(
	void)
{
	return main_globals.save_map;
}

void main_save_cancel(
	void)
{
	main_globals.save_map = FALSE;

	return;
}

void main_save_map_no_timeout(
	void)
{
	main_save_map_safe();
	main_globals.save_map_timeout = FALSE;

	return;
}

void main_save_map_safe(
	void)
{
	if (!main_globals.save_map || main_globals.save_map_timeout)
	{
		main_globals.save_map = TRUE;
		main_globals.save_map_safely = TRUE;
		main_globals.save_map_timeout = TRUE;
		main_globals.ticks_until_next_save_check = 0;
		main_globals.ticks_unable_to_save = 0;
		main_globals.safe_intervals = 0;
	}

	return;
}

void main_won_map(
	void)
{
	main_globals.save_map = FALSE;
	main_globals.won_map = TRUE;

	return;
}

void main_lost_map(
	void)
{
	main_globals.save_map = FALSE;
	main_globals.lost_map = TRUE;

	return;
}

void main_respawn(
	boolean force_respawn)
{
	main_globals.respawn = TRUE;

	if (force_respawn)
	{
		main_globals.respawn_timer = 3*TICKS_PER_SECOND+1;
	}

	return;
}

void main_save_core(
	void)
{
	main_globals.save_core = TRUE;
	strcpy(main_globals.core_file_name, "core.bin");

	return;
}

void main_save_core_name(
	char const *core_name)
{
	match_vwarn("c:\\halo\\SOURCE\\main\\main.c", 933, strlen(core_name)<NUMBEROF(main_globals.core_file_name), "warning, core file name will be truncated to 63 characters");
	strncpy(main_globals.core_file_name, core_name, NUMBEROF(main_globals.core_file_name)-1);
	main_globals.save_core = TRUE;

	return;
}

void main_load_core(
	void)
{
	main_globals.load_core = TRUE;
	strcpy(main_globals.core_file_name, "core.bin");

	return;
}

void main_load_core_at_startup(
	void)
{
	main_globals.load_core_at_startup = TRUE;
	strcpy(main_globals.core_file_name, "core.bin");

	return;
}

void main_load_core_name(
	char const *core_name)
{
	match_vwarn("c:\\halo\\SOURCE\\main\\main.c", 969, strlen(core_name)<NUMBEROF(main_globals.core_file_name), "warning, core file name will be truncated to 63 characters");
	strncpy(main_globals.core_file_name, core_name, NUMBEROF(main_globals.core_file_name)-1);
	main_globals.load_core = TRUE;

	return;
}

void main_load_core_name_at_startup(
	char const *core_name)
{
	match_vwarn("c:\\halo\\SOURCE\\main\\main.c", 983, strlen(core_name)<NUMBEROF(main_globals.core_file_name), "warning, core file name will be truncated to 63 characters");
	strncpy(main_globals.core_file_name, core_name, NUMBEROF(main_globals.core_file_name)-1);
	main_globals.load_core_at_startup = TRUE;

	return;
}

void main_switch_structure_bsp(
	short new_structure_bsp_index)
{
	struct scenario *scenario = global_scenario_get();

	if (new_structure_bsp_index>=0 && new_structure_bsp_index<scenario->structure_bsp_references.count)
	{
		if (new_structure_bsp_index==global_structure_bsp_index)
		{
			console_warning("tried to switch to current structure-bsp %d", new_structure_bsp_index);
		}
		else
		{
			main_globals.switch_to_structure_bsp_index = new_structure_bsp_index;
			hud_load(TRUE);
		}
	}
	else
	{
		console_warning("tried to switch to invalid structure-bsp %d", new_structure_bsp_index);
	}

	return;
}

void main_skip(
	short ticks)
{
	if (ticks<=15)
	{
		main_globals.skip_ticks = ticks;
		main_globals.cutscene_skip = TRUE;
	}
	else
	{
		error(_error_silent, "cannot skip more than 15 frames (half a second)");
	}

	return;
}

void main_queue_map_name(
	char const *new_name)
{
	if (new_name)
	{
		strncpy(main_globals.queued_map_name, new_name, NUMBEROF(main_globals.queued_map_name)-1);
		main_globals.queue_map = TRUE;
	}
	else
	{
		main_globals.queued_map_name[0] = '\0';
		main_globals.queue_map = FALSE;
	}

	return;
}

static void main_queue_map_private(
	void)
{
	real progress;

	if (cache_files_precache_in_progress() && cache_files_precache_map_status(&progress)==_cached_map_file_success)
	{
		cache_files_precache_map_end();
	}

	if (!cache_files_precache_in_progress())
	{
		cache_files_precache_map_begin(main_globals.queued_map_name, FALSE);
		main_globals.queue_map = FALSE;
	}

	return;
}

void main_goto_main_menu(
	void)
{
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.save_map = FALSE;
	main_globals.want_to_be_at_main_menu = TRUE;

	return;
}

void main_load_ui_scenario(
	boolean precache_resources)
{
	struct game_options options;

	game_precache_new_map("levels\\ui\\ui", TRUE);
	match_assert("c:\\halo\\SOURCE\\main\\main.c", 1092, !main_globals.main_menu_scenario_loaded);

	game_options_new(&options);
	strncpy(options.map_name, "levels\\ui\\ui", NUMBEROF(options.map_name)-1);
	options.map_name[NUMBEROF(options.map_name)-1] = '\0';
	game_precache_new_map(options.map_name, TRUE);
	game_dispose_from_old_map();
	game_unload();
	game_engine_dispose();
	game_set_game_variant(NULL);
	main_globals.main_menu_scenario_loaded = TRUE;
	main_new_map(&options);
	director_script_camera(TRUE);
	scripted_camera_set(0, 0, NONE);
	main_menu_active(TRUE);
	main_globals.load_last_solo_level = TRUE;

	if (precache_resources)
	{
		main_menu_precache_resources();
	}

	return;
}

void main_menu_precache_resources(
	void)
{
	struct scenario *scenario = global_scenario_get();

	if (scenario)
	{
		match_assert("c:\\halo\\SOURCE\\main\\main.c", 1133, scenario->type==_scenario_type_main_menu);
		predicted_resources_precache(&scenario->predicted_ui_resources);
	}

	return;
}

void main_menu_load(
	void)
{
	if (!main_globals.main_menu_scenario_loaded)
	{
		main_load_ui_scenario(FALSE);
	}

	main_screen_shell_load();
	main_menu_precache_resources();
	main_menu_ensure_player_queues_exist();
	game_time_dispose_from_old_map();
	game_time_initialize_for_new_map();
	game_time_start();
	hs_runtime_dispose_from_old_map();
	hs_runtime_initialize_for_new_map();
	main_globals.want_to_be_at_main_menu = FALSE;

	return;
}

void main_menu_unload(
	void)
{
	ui_stop_main_menu_music();
	main_menu_active(FALSE);
	main_globals.main_menu_scenario_loaded = FALSE;

	return;
}

void main_menu_ensure_player_queues_exist(
	void)
{
	update_server_delete();
	update_server_new();
	update_server_start();

	return;
}

boolean main_menu_fade_active(
	void)
{
	return main_globals.map_change_load_timer!=0;
}

void main_menu_switch_to_single_player(
	void)
{
	main_globals.rename_map = TRUE;

	return;
}

void main_set_game_connection_to_film_playback(
	void)
{
	main_globals.playback_last_recording = TRUE;

	return;
}

short main_get_solo_level_from_name(
	char const *name)
{
	short level;
	char current_map[128] = "";

	strncpy(current_map, name, NUMBEROF(current_map)-1);
	current_map[NUMBEROF(current_map)-1] = '\0';
	strlwr(current_map);

	if (strstr(current_map, "a10"))
	{
		level = _single_player_map_a10;
	}
	else if (strstr(current_map, "a30"))
	{
		level = _single_player_map_a30;
	}
	else if (strstr(current_map, "a50"))
	{
		level = _single_player_map_a50;
	}
	else if (strstr(current_map, "b30"))
	{
		level = _single_player_map_b30;
	}
	else if (strstr(current_map, "b40"))
	{
		level = _single_player_map_b40;
	}
	else if (strstr(current_map, "c10"))
	{
		level = _single_player_map_c10;
	}
	else if (strstr(current_map, "c20"))
	{
		level = _single_player_map_c20;
	}
	else if (strstr(current_map, "c40"))
	{
		level = _single_player_map_c40;
	}
	else if (strstr(current_map, "d20"))
	{
		level = _single_player_map_d20;
	}
	else if (strstr(current_map, "d40"))
	{
		level = _single_player_map_d40;
	}
	else
	{
		level = NONE;
	}

	return level;
}

short main_get_current_solo_level(
	void)
{
	return main_get_solo_level_from_name(main_globals.soloplayer_map_name);
}

char const *main_get_solo_level_name(
	short level)
{
	static char const *scenario_paths[] =
	{
		"levels\\a10\\a10",
		"levels\\a30\\a30",
		"levels\\a50\\a50",
		"levels\\b30\\b30",
		"levels\\b40\\b40",
		"levels\\c10\\c10",
		"levels\\c20\\c20",
		"levels\\c40\\c40",
		"levels\\d20\\d20",
		"levels\\d40\\d40",
	};
	char const *result = NULL;

	if (level>=0 && level<NUMBEROF(scenario_paths))
	{
		result = scenario_paths[level];
	}

	return result;
}

void main_run_demos(
	void)
{
	main_globals.run_xdemos = TRUE;

	return;
}

void main_roll_credits(
	void)
{
	error(_error_silent, "congratulations, you won the game!");
	main_menu_load();
	game_end_credits_start();

	return;
}

static void compute_subframe_counts(
	long num_players,
	long *out_horizontal_count,
	long *out_vertical_count)
{
	long horizontal_count = 1;
	long vertical_count = 1;
	boolean favor_horizontal = TRUE;

	match_assert("c:\\halo\\SOURCE\\main\\main.c", 1308, num_players>0);

	while (horizontal_count*vertical_count<num_players)
	{
		boolean grow_horizontal = favor_horizontal ? horizontal_count<vertical_count : horizontal_count<=vertical_count;

		if (grow_horizontal)
		{
			horizontal_count++;
		}
		else
		{
			horizontal_count = 1;
			vertical_count++;
		}
	}

	*out_horizontal_count = horizontal_count;
	*out_vertical_count = vertical_count;

	return;
}

void compute_window_bounds(
	long player_index,
	long num_players,
	rectangle2d *pixel_bounds,
	rectangle2d *safe_frame_bounds)
{
	long horizontal_count;
	long vertical_count;
	long horizontal_index;
	long vertical_index;
	short horizontal_border;
	short vertical_border;
	boolean double_wide;
	long frame_width;
	long frame_height;
	long window_width;
	long window_height;

	match_assert("c:\\halo\\SOURCE\\main\\main.c", 1359, player_index<num_players);

	horizontal_border = num_players>1 ? 4 : 0;
	vertical_border = num_players>1 ? 4 : 0;
	double_wide = FALSE;
	compute_subframe_counts(num_players, &horizontal_count, &vertical_count);

	if (horizontal_count*vertical_count>num_players)
	{
		if (player_index==0)
		{
			double_wide = TRUE;
		}
		else
		{
			player_index++;
		}
	}

	vertical_index = player_index/horizontal_count;
	horizontal_index = player_index-horizontal_count*vertical_index;
	match_assert("c:\\halo\\SOURCE\\main\\main.c", 1390, vertical_index>=0 && vertical_index<vertical_count);
	match_assert("c:\\halo\\SOURCE\\main\\main.c", 1391, horizontal_index>=0 && horizontal_index<horizontal_count);

	frame_width = rasterizer_globals.frame_bounds.x1-rasterizer_globals.frame_bounds.x0;
	frame_height = rasterizer_globals.frame_bounds.y1-rasterizer_globals.frame_bounds.y0;
	window_width = frame_width/horizontal_count;
	window_height = frame_height/vertical_count;
	window_width *= double_wide ? 2 : 1;

	safe_frame_bounds->x0 = rasterizer_globals.frame_bounds.x0+window_width*horizontal_index;
	safe_frame_bounds->x1 = rasterizer_globals.frame_bounds.x0+(horizontal_index+1)*window_width;
	safe_frame_bounds->y0 = rasterizer_globals.frame_bounds.y0+window_height*vertical_index;
	safe_frame_bounds->y1 = rasterizer_globals.frame_bounds.y0+(vertical_index+1)*window_height;
	*pixel_bounds = *safe_frame_bounds;

	safe_frame_bounds->x0 += vertical_border*horizontal_index;
	safe_frame_bounds->x1 -= vertical_border*(horizontal_index==0);
	safe_frame_bounds->y0 += horizontal_border*vertical_index;
	safe_frame_bounds->y1 -= horizontal_border*(vertical_index==0);

	if (horizontal_index==0)
	{
		pixel_bounds->x0 = rasterizer_globals.screen_bounds.x0;
	}

	if (horizontal_index+(double_wide ? 1 : 0)+1==horizontal_count)
	{
		pixel_bounds->x1 = rasterizer_globals.screen_bounds.x1;
	}

	if (vertical_index==0)
	{
		pixel_bounds->y0 = rasterizer_globals.screen_bounds.y0;
	}

	if (vertical_index+1==vertical_count)
	{
		pixel_bounds->y1 = rasterizer_globals.screen_bounds.y1;
	}

	return;
}

void main_pregame_render(
	void)
{
	static struct render_window window;
	real_point3d camera_position;
	real_vector3d forward;
	real_vector3d up;

	collision_log_continue_period(_collision_period_render_frame);
	sound_render();

	set_real_point3d(&camera_position, 0.f, 0.f, 0.f);
	set_real_vector3d(&forward, 0.f, 0.f, 1.f);
	set_real_vector3d(&up, 0.f, 1.f, 0.f);
	window.local_player_index = NONE;
	window.console_window = TRUE;
	window.rasterizer_camera.position = camera_position;
	window.rasterizer_camera.forward = forward;
	window.rasterizer_camera.up = up;
	window.rasterizer_camera.mirrored = FALSE;
	window.rasterizer_camera.vertical_field_of_view = 2.f*arctangent(render_camera_get_adjusted_field_of_view_tangent(DEGREES_TO_RADIANS(80.f))*0.75f, 1.f);
	compute_window_bounds(0, 1, &window.rasterizer_camera.viewport_bounds, &window.rasterizer_camera.window_bounds);
	window.rasterizer_camera.z_near = 0.01f;
	window.rasterizer_camera.z_far = 1.f;
	window.render_camera = window.rasterizer_camera;
	render_frame_pregame(&window, main_globals.movie);
	collision_log_end_period();

	return;
}

void set_window_camera_values(
	struct render_window *current_window,
	struct observer_result const *observer)
{
	if (observer)
	{
		current_window->rasterizer_camera.position = observer->position;
		current_window->rasterizer_camera.forward = observer->forward;
		current_window->rasterizer_camera.up = observer->up;
		current_window->rasterizer_camera.vertical_field_of_view = 2.f*arctangent(render_camera_get_adjusted_field_of_view_tangent(observer->field_of_view)*0.75f, 1.f);

		if (current_window->local_player_index!=NONE && !console_is_active() && !game_time_get_paused() &&
			director_get_perspective(current_window->local_player_index)!=_director_perspective_neutral)
		{
			real_matrix4x3 effect_matrix;
			real_matrix4x3 observer_matrix;

			player_effect_get_camera_effect_matrix(current_window->local_player_index, &effect_matrix);
			matrix4x3_from_point_and_vectors(&observer_matrix, &observer->position, &observer->forward, &observer->up);
			matrix4x3_multiply(&observer_matrix, &effect_matrix, &observer_matrix);
			matrix4x3_to_point_and_vectors(&observer_matrix, &current_window->rasterizer_camera.position, &current_window->rasterizer_camera.forward, &current_window->rasterizer_camera.up);
		}
	}
	else
	{
		current_window->rasterizer_camera.position = *global_origin3d;
		current_window->rasterizer_camera.forward = *global_forward3d;
		current_window->rasterizer_camera.up = *global_up3d;
		current_window->rasterizer_camera.vertical_field_of_view = 2.f*arctangent(render_camera_get_adjusted_field_of_view_tangent(DEGREES_TO_RADIANS(80.f))*0.75f, 1.f);
	}

	current_window->rasterizer_camera.mirrored = FALSE;
	current_window->rasterizer_camera.z_near = rasterizer_globals.z_near;
	current_window->rasterizer_camera.z_far = rasterizer_globals.z_far;

	if (!debug_render_freeze)
	{
		current_window->render_camera = current_window->rasterizer_camera;
	}

	return;
}

short main_get_window_count(
	void)
{
	boolean single_screen = game_engine_force_single_screen() || cinematic_in_progress();

	return single_screen ? 1 : PIN(local_player_count(), 1, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
}

static void main_game_render(
	double time_delta_since_tick_sec)
{
	static struct render_window window[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS+1] = {0};
	long window_index;
	boolean single_screen;
	short local_player_index;
	long active_window_count;
	long window_count;
	struct render_window *current_window;

	lock_global_random_seed();
	collision_log_continue_period(_collision_period_render_frame);
	sound_render();

	single_screen = game_engine_force_single_screen();
	local_player_index = NONE;
	active_window_count = PIN(local_player_count(), 1, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
	window_count = active_window_count;

	if (single_screen || cinematic_in_progress())
	{
		active_window_count = 1;
		window_count = 1;
	}

	for (window_index = 0; window_index<window_count; window_index++)
	{
		struct observer_result const *observer = NULL;

		current_window = &window[window_index];

		compute_window_bounds(window_index, window_count, &current_window->rasterizer_camera.viewport_bounds, &current_window->rasterizer_camera.window_bounds);

		if (single_screen)
		{
			current_window->local_player_index = NONE;
		}
		else if (window_index<active_window_count)
		{
			if (!rasterizer_debug_options.force_all_player_views_to_default_player || local_player_index==NONE)
			{
				if (game_connection()==_game_connection_film_playback)
				{
					local_player_index = 0;
				}
				else
				{
					local_player_index = local_player_get_next(local_player_index);
				}
			}

			current_window->local_player_index = local_player_index;
			observer = observer_get_camera(current_window->local_player_index);
		}
		else
		{
			current_window->local_player_index = NONE;
		}

		set_window_camera_values(current_window, observer);
		current_window->console_window = FALSE;
	}

	current_window = &window[window_count];
	compute_window_bounds(0, 1, &current_window->rasterizer_camera.viewport_bounds, &current_window->rasterizer_camera.window_bounds);
	current_window->local_player_index = NONE;
	current_window->console_window = TRUE;
	set_window_camera_values(current_window, NULL);

	if (global_screenshot_count<=0)
	{
		render_frame(window, window_count+1, NULL, NULL, main_globals.movie, time_delta_since_tick_sec);
	}
	else
	{
		screenshot_render(window);
	}

	collision_log_end_period();
	unlock_global_random_seed();

	return;
}

void main_present_frame(
	void)
{
	char filename[512];
	struct file_reference file;

	render_frame_present(NULL, main_globals.movie);

	if (global_screenshot_count<=0 && main_globals.movie)
	{
		_snprintf(filename, NUMBEROF(filename), "movie\\frame%06d.tga", main_globals.recording_frame_index++);
		file_reference_create_from_path(&file, filename, FALSE);
		screenshot_record(main_globals.movie, &file);
	}

	return;
}

/* ---------- private code */

static void main_new_map(
	struct game_options *options)
{
	input_flush();

	if (game_load(options))
	{
		game_initialize_for_new_map();
	}
	else
	{
		error(_error_immediate, "game_load() failed.");
	}

	if (!errors_handle())
	{
		create_local_players();
		game_time_start();
	}
	else
	{
		error(_error_immediate, "main_new_map() failed.");
	}

	game_initial_pulse();
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.reset_map = FALSE;
	main_globals.rename_map = FALSE;
	main_globals.revert_map = FALSE;
	main_globals.skip_cinematic = FALSE;
	main_globals.save_map = FALSE;
	main_globals.won_map = FALSE;
	main_globals.lost_map = FALSE;
	main_globals.respawn = FALSE;
	main_globals.save_core = FALSE;
	main_globals.load_core = main_globals.load_core_at_startup;
	main_globals.load_core_at_startup = FALSE;

	if (main_globals.solo_try_and_load_from_persistent_storage)
	{
		game_state_try_and_load_from_persistent_storage();
	}

	ui_widgets_disable_pause_game(TICKS_PER_SECOND);

	return;
}

static void main_change_map_name(
	void)
{
	if (main_globals.main_menu_scenario_loaded==TRUE)
	{
		if (main_globals.map_change_load_timer==0)
		{
			if (ui_main_menu_music_active()==TRUE)
			{
				main_globals.map_change_load_timer = main_globals.last_time_msec+1000;
				main_screen_shell_begin_fade(1000);
				ui_widgets_inhibit_processing(TRUE);
				ui_widgets_set_fade_value(0.f);
			}
		}
		else
		{
			ui_widgets_set_fade_value(1.f-(main_globals.map_change_load_timer-main_globals.last_time_msec)*0.001f);
		}
	}
	else
	{
		main_globals.map_change_load_timer = 0;
	}

	if (main_globals.last_time_msec>=main_globals.map_change_load_timer)
	{
		ui_widgets_set_fade_value(-1.f);
		main_menu_unload();
		ui_widgets_inhibit_processing(FALSE);

		if (game_in_progress() && game_connection()==_game_connection_local)
		{
			struct game_options options;
			short local_player_index;

			game_options_new(&options);
			strncpy(options.map_name, main_globals.soloplayer_map_name, NUMBEROF(options.map_name)-1);
			options.map_name[NUMBEROF(options.map_name)-1] = '\0';
			options.difficulty = global_difficulty_level;
			game_dispose_from_old_map();
			game_precache_new_map(options.map_name, TRUE);
			game_unload();
			main_new_map(&options);

			for (local_player_index = 0; local_player_index<player_spawn_count; local_player_index++)
			{
				player_profile_save_last_level_played(local_player_index);
			}
		}

		main_globals.map_change_load_timer = 0;
	}

	return;
}

static void main_revert_map_private(
	void)
{
	game_state_revert();
	ui_widgets_disable_pause_game(TICKS_PER_SECOND);
	main_globals.revert_map = FALSE;

	return;
}

static void main_skip_cinematic_private(
	void)
{
	if (cinematic_can_be_skipped())
	{
		main_revert_map_private();
	}

	main_globals.skip_cinematic = FALSE;

	return;
}

static void main_skip_private(
	void)
{
	if (main_globals.skip_ticks && cinematic_in_progress())
	{
		real speed = game_time_get_speed();

		game_time_set_speed(1.f);

		while (main_globals.skip_ticks-->0)
		{
			game_time_update(SECONDS_PER_TICK);
		}

		game_time_set_speed(speed);
	}
	else
	{
		error(_error_silent, "manual skipping doesn't work outside of cinemtatic start/stop...");
	}

	main_globals.skip_ticks = 0;
	main_globals.cutscene_skip = FALSE;

	return;
}

static void main_saving_map_private(
	void)
{
	game_state_save();
	hud_autosave(FALSE);
	main_globals.saving_map = FALSE;

	return;
}

static void main_save_map_private(
	void)
{
	if (!game_time_get_paused())
	{
		boolean save = FALSE;

		if (main_globals.save_map_safely)
		{
			if (main_globals.ticks_unable_to_save++<8*TICKS_PER_SECOND || !main_globals.save_map_timeout)
			{
				if (main_globals.ticks_until_next_save_check--<=0)
				{
					if (game_safe_to_save())
					{
						if (main_globals.safe_intervals++>=3)
						{
							save = TRUE;
						}
					}
					else
					{
						main_globals.safe_intervals = 0;
					}

					main_globals.ticks_until_next_save_check = TICKS_PER_SECOND/3;
				}
			}
			else
			{
				if (debug_game_save)
				{
					console_printf(FALSE, "gave up trying to save");
				}

				main_globals.save_map = FALSE;
			}
		}
		else
		{
			if (debug_game_save)
			{
				console_printf(FALSE, "unsafe save");
			}

			save = TRUE;
		}

		if (save)
		{
			hud_autosave(TRUE);
			main_globals.saving_map = TRUE;
			main_globals.save_map = FALSE;
		}
	}

	return;
}

static void main_switch_to_structure_bsp_private(
	void)
{
	scenario_switch_structure_bsp(main_globals.switch_to_structure_bsp_index);
	main_globals.switch_to_structure_bsp_index = NONE;
	hud_load(FALSE);

	return;
}

static void main_lost_map_private(
	void)
{
	if (!game_time_get_paused())
	{
		if (main_globals.loss_timer++>3*TICKS_PER_SECOND)
		{
			main_globals.lost_map = FALSE;
			main_globals.loss_timer = 0;
			game_state_revert();
		}
	}

	return;
}

static void main_respawn_private(
	void)
{
	if (!game_time_get_paused() && !cinematic_in_progress())
	{
		if (main_globals.respawn_timer++>3*TICKS_PER_SECOND && players_respawn_coop())
		{
			main_globals.respawn = FALSE;
			main_globals.respawn_timer = 0;
		}
	}

	return;
}

static void main_won_map_private(
	void)
{
	short next_level;
	short local_player_index;

	main_globals.want_to_be_at_main_menu = TRUE;
	main_globals.won_map = FALSE;

	next_level = main_get_solo_level_from_name(main_globals.soloplayer_map_name)+1;

	if (next_level>=NUMBER_OF_SINGLE_PLAYER_LEVELS)
	{
		next_level = NONE;
	}

	for (local_player_index = 0; local_player_index<player_spawn_count; local_player_index++)
	{
		player_profile_save_level_completed(local_player_index);
	}

	ui_set_next_level(next_level);

	return;
}

static void main_load_core_private(
	void)
{
	game_state_load_core(main_globals.core_file_name);
	main_globals.load_core = FALSE;

	return;
}

static void main_save_core_private(
	void)
{
	game_state_save_core(main_globals.core_file_name);
	main_globals.save_core = FALSE;

	return;
}

static void main_reset_map_private(
	void)
{
	if (!game_time_get_paused())
	{
		scenario_switch_structure_bsp(0);
		game_dispose_from_old_map();
		input_flush();
		game_initialize_for_new_map();
		create_local_players();
		game_time_start();
		game_initial_pulse();
		ui_widgets_disable_pause_game(TICKS_PER_SECOND);
		main_globals.reset_map = FALSE;
	}

	return;
}

static void main_run_demos_private(
	void)
{
	main_globals.run_xdemos = FALSE;
	xbox_demos_launch();

	return;
}

static void main_frame_rate_debug(
	void)
{
	enum
	{
		NUMBER_OF_FRAME_SAMPLES = 8,
		RUNS_BEFORE_RESET = 60,
	};

	static real last_spf[NUMBER_OF_FRAME_SAMPLES] = {0};
	static word bad_frame_flags = 0;
	static char current_spf_index = 0;
	static boolean wait_for_good_framerate = FALSE;
	static char good_framerate_count = 0;
	static boolean need_to_initialize = FALSE;

	if (need_to_initialize && !debug_frame_rate)
	{
		need_to_initialize = FALSE;
		memset(last_spf, 0, sizeof(last_spf));
		bad_frame_flags = 0;
		current_spf_index = 0;
		wait_for_good_framerate = FALSE;
		good_framerate_count = 0;
		need_to_initialize = FALSE;
	}

	if (debug_frame_rate)
	{
		last_spf[current_spf_index] = main_globals.seconds_elapsed;

		SET_FLAG(bad_frame_flags, current_spf_index, main_globals.seconds_elapsed>36*0.001);

		current_spf_index++;
		current_spf_index %= NUMBER_OF_FRAME_SAMPLES;
		need_to_initialize = TRUE;

		if (wait_for_good_framerate)
		{
			if (current_spf_index==0)
			{
				if (bad_frame_flags==0)
				{
					if (++good_framerate_count>=RUNS_BEFORE_RESET)
					{
						good_framerate_count = 0;
						wait_for_good_framerate = FALSE;
					}
				}
				else
				{
					good_framerate_count = 0;
				}
			}
		}
		else if (bad_frame_flags==MASK(NUMBER_OF_FRAME_SAMPLES))
		{
			char core_name[256];
			char file_name[MAX_PATH];
			SYSTEMTIME time;
			FILE *f;
			char const *map_name = tag_name_strip_path(tag_get_name(global_scenario_index));

			GetSystemTime(&time);
			sprintf(core_name, "%s_slow_%d_%d_%d_%d_%d_%d.bin", map_name, time.wMonth, time.wDay, time.wYear, time.wHour, time.wMinute, time.wSecond);
			game_state_save_core(core_name);
			sprintf(file_name, "d:\\%s_init.txt", map_name);
			f = fopen(file_name, "r");

			if (!f)
			{
				f = fopen(file_name, "wt");
				fprintf(f, "map_name %s\n", map_name);
			}
			else
			{
				fclose(f);
				f = fopen(file_name, "a+t");
			}

			fprintf(f, ";core_load_name_at_startup %s\n", core_name);
			fflush(f);
			fclose(f);
			wait_for_good_framerate = TRUE;
		}
	}

	return;
}

static void main_setup_connection(
	void)
{
	boolean playback = main_globals.playback_last_recording;

	if (playback)
	{
		main_globals.want_to_be_at_main_menu = FALSE;
	}

	if (main_globals.want_to_be_at_main_menu)
	{
		main_menu_load();
	}
	else if (playback)
	{
		game_connection_set(_game_connection_film_playback);
		error(_error_silent, "error opening saved film");
		main_globals.want_to_be_at_main_menu = TRUE;
		main_menu_load();
	}
	else
	{
		struct game_options options;

		game_connection_set(_game_connection_local);
		game_options_new(&options);
		strncpy(options.map_name, main_globals.soloplayer_map_name, NUMBEROF(options.map_name)-1);
		options.map_name[NUMBEROF(options.map_name)-1] = '\0';
		options.difficulty = global_difficulty_level;
		game_precache_new_map(options.map_name, TRUE);
		game_dispose_from_old_map();
		main_new_map(&options);
	}

	return;
}

static void main_exit(
	void)
{
	switch (main_globals.connection)
	{
	case _game_connection_network_client:
		dispose_global_network_game_client();
		break;
	case _game_connection_network_server:
		dispose_global_network_game_client();
		dispose_global_network_game_server();
		break;
	}

	game_dispose_from_old_map();
	game_dispose();
	debug_keys_dispose();
	console_dispose();

	return;
}

static void main_initialize_time(
	void)
{
	main_globals.last_time_msec = system_milliseconds();
	main_globals.last_vblank_index = 0;
	rasterizer_set_vblank_callback(main_vertical_blank_interrupt_handler);
	main_globals.vblank_flip_delta_next_index = 0;
	memset(main_globals.vblank_flip_deltas, 0, sizeof(main_globals.vblank_flip_deltas));
	main_globals.vblank_flip_counter = (unsigned long *)d3d_find_flipcount();

	return;
}

static void main_reset_time(
	void)
{
	main_globals.last_time_msec = system_milliseconds();
	main_globals.last_vblank_index = rasterizer_globals.vblank_index;

	return;
}

static boolean main_time_is_throttled(
	void)
{
	return rasterizer_globals.use_rasterizer_frame_rate_throttle;
}

static void main_update_time(
	void)
{
	real seconds_elapsed;
	unsigned long current_milliseconds = system_milliseconds();
	__int64 current_vblank_index = MAX(main_globals.last_vblank_index, main_globals.last_present_vblank_index);
	__int64 target_display_vblank_index = current_vblank_index;
	boolean throttle = main_time_is_throttled();

	if (throttle)
	{
		strcpy(main_globals.time_debug_string, "");

		if (global_frame_rate_throttle && rasterizer_globals.refresh_rate>=0)
		{
			short interval;
			short minimum_interval = VBLANKS_PER_SECOND/(rasterizer_globals.refresh_rate ? rasterizer_globals.refresh_rate : TICKS_PER_SECOND);

			main_globals.vblank_interval_minimum = minimum_interval;

			if (rasterizer_globals.use_rasterizer_frame_rate_stabilization)
			{
				short best_interval;
				short frame_rate;
				short elapsed = game_time_get_elapsed();
				short frame_index = (short)current_vblank_index;

				_snprintf(main_globals.time_debug_string+strlen(main_globals.time_debug_string), NUMBEROF(main_globals.time_debug_string)-strlen(main_globals.time_debug_string), "last%6I64d init%6I64d achv%6I64d pres%6I64d g%d cur%d... ",
					main_globals.last_vblank_index, main_globals.last_initial_vblank_index, main_globals.last_achievable_vblank_index, main_globals.last_present_vblank_index,
					elapsed, main_globals.vblank_interval_current);

				best_interval = 5;

				for (interval = 5; interval>0; interval--)
				{
					boolean ignore = FALSE;
					short failure_count = MIN(99, main_globals.vblank_failure_count[interval]);
					short frames_since_failure = MIN(99, frame_index-main_globals.vblank_last_failure_time[interval]);
					short half_interval = (interval+1)/2;
					short half_current_up = (main_globals.vblank_interval_current+1)/2;
					short half_current = main_globals.vblank_interval_current/2;

					if (half_current_up>half_interval && half_current<=half_interval && elapsed>half_interval)
					{
						ignore = TRUE;
					}

					if (main_globals.last_achievable_vblank_index>main_globals.last_initial_vblank_index+interval)
					{
						if (!ignore)
						{
							main_globals.vblank_failure_count[interval]++;
							main_globals.vblank_last_failure_time[interval] = current_vblank_index;
						}

						_snprintf(main_globals.time_debug_string+strlen(main_globals.time_debug_string), NUMBEROF(main_globals.time_debug_string)-strlen(main_globals.time_debug_string), "(%s%2d) ",
							ignore ? "ignor" : "fail ", failure_count);
					}
					else if (current_vblank_index>=main_globals.vblank_last_failure_time[interval]+15)
					{
						main_globals.vblank_failure_count[interval] = 0;
						_snprintf(main_globals.time_debug_string+strlen(main_globals.time_debug_string), NUMBEROF(main_globals.time_debug_string)-strlen(main_globals.time_debug_string), "(ok   %2d) ",
							frames_since_failure);
					}
					else
					{
						_snprintf(main_globals.time_debug_string+strlen(main_globals.time_debug_string), NUMBEROF(main_globals.time_debug_string)-strlen(main_globals.time_debug_string), "(%s%2d/%2d) ",
							main_globals.vblank_failure_count[interval]>=4 ? "dn" : "wt", failure_count, frames_since_failure);
					}

					if (interval>=main_globals.vblank_interval_minimum && main_globals.vblank_failure_count[interval]<4)
					{
						best_interval = interval;
					}
				}

				frame_rate = best_interval==0 ? 999 : VBLANKS_PER_SECOND/best_interval;

				if (best_interval>main_globals.vblank_interval_current)
				{
					_snprintf(main_globals.time_debug_string+strlen(main_globals.time_debug_string), NUMBEROF(main_globals.time_debug_string)-strlen(main_globals.time_debug_string), " FAILDOWN %d", frame_rate);
				}
				else if (best_interval<main_globals.vblank_interval_current)
				{
					_snprintf(main_globals.time_debug_string+strlen(main_globals.time_debug_string), NUMBEROF(main_globals.time_debug_string)-strlen(main_globals.time_debug_string), " RESTORE  %d", frame_rate);
				}
				else
				{
					_snprintf(main_globals.time_debug_string+strlen(main_globals.time_debug_string), NUMBEROF(main_globals.time_debug_string)-strlen(main_globals.time_debug_string), " MAINTAIN %d", frame_rate);
				}

				_snprintf(main_globals.time_debug_string+strlen(main_globals.time_debug_string), NUMBEROF(main_globals.time_debug_string)-strlen(main_globals.time_debug_string), " des %d targ%6I64d",
					best_interval, current_vblank_index+best_interval);

				main_globals.vblank_interval_current = best_interval;
			}
			else
			{
				main_globals.vblank_interval_current = minimum_interval;
			}

			target_display_vblank_index = current_vblank_index+main_globals.vblank_interval_current;
		}
	}
	else
	{
		long delta_msec = current_milliseconds-main_globals.last_time_msec;
		real elapsed = (real)delta_msec;
		long elapsed_msec = (long)elapsed;

		main_globals.vblank_interval_held = FALSE;

		if (elapsed_msec<33)
		{
			profile_idle_start();

			if (global_frame_rate_throttle)
			{
				Sleep(33-elapsed_msec);
			}

			profile_idle_end();
		}
		else
		{
			profile_lapsed_msec(elapsed_msec-33);
		}
	}

	current_milliseconds = system_milliseconds();
	target_display_vblank_index = MAX(rasterizer_globals.vblank_index, target_display_vblank_index);

	if (throttle)
	{
		seconds_elapsed = (target_display_vblank_index-main_globals.last_vblank_index)*(1.f/VBLANKS_PER_SECOND);
	}
	else
	{
		seconds_elapsed = (current_milliseconds-main_globals.last_time_msec)*0.001f;
	}

	if (main_globals.movie)
	{
		seconds_elapsed = main_globals.recording_dt;
	}
	else
	{
		seconds_elapsed = PIN(seconds_elapsed, 0.f, 1.f);

		if (main_globals.connection==_game_connection_local)
		{
			seconds_elapsed = debug_force_frame_rate_update ? CEILING(seconds_elapsed, SECONDS_PER_TICK) : CEILING(seconds_elapsed, 1.f/15.f);
		}
	}

	main_globals.last_time_msec = current_milliseconds;
	main_globals.last_vblank_index = target_display_vblank_index;
	main_globals.seconds_elapsed = seconds_elapsed;
	profile_seconds_elapsed(seconds_elapsed);
	main_globals.last_initial_vblank_index = rasterizer_globals.vblank_index;

	return;
}

void main_rasterizer_throttle(
	void)
{
	boolean at_minimum;
	short lapsed;
	__int64 vblank_index = rasterizer_globals.vblank_index;
	boolean throttled = FALSE;

	main_globals.last_achievable_vblank_index = rasterizer_globals.vblank_index+1;

	if (main_time_is_throttled())
	{
		__int64 vblank_to_begin_present = main_globals.last_vblank_index-1;

		if (rasterizer_globals.vblank_index<vblank_to_begin_present)
		{
			unsigned long time_ms = system_milliseconds();
			boolean precaching = cache_files_precache_in_progress();

			throttled = TRUE;
			profile_idle_start();

			while (rasterizer_globals.vblank_index<vblank_to_begin_present)
			{
				if (precaching)
				{
					Sleep(1);
				}

				if (system_milliseconds()>time_ms+1000)
				{
					console_warning("stuck waiting for VBLANK callback! disabling rasterizer framerate control");
					rasterizer_globals.use_rasterizer_frame_rate_throttle = FALSE;
					break;
				}
			}

			profile_idle_end();
		}
	}

	main_globals.last_present_vblank_index = rasterizer_globals.vblank_index+1;
	at_minimum = main_globals.vblank_interval_current==main_globals.vblank_interval_minimum;
	lapsed = PIN(main_globals.last_present_vblank_index-main_globals.last_vblank_index, 0, SHRT_MAX);

	_snprintf(main_globals.time_debug_string+strlen(main_globals.time_debug_string), NUMBEROF(main_globals.time_debug_string)-strlen(main_globals.time_debug_string), "%6I64d(targ%6I64d %s%2d)",
		vblank_index, main_globals.last_vblank_index,
		throttled ? "THROTTLE" : (lapsed==0 ? "SYNCED  " : "LAPSED  "),
		throttled ? rasterizer_globals.vblank_index-vblank_index : lapsed);

	main_globals.vblank_interval_held = main_globals.vblank_interval_current>0 && lapsed==0;
	profile_lapsed_frames(lapsed, at_minimum, main_globals.time_debug_string);

	return;
}

static void screenshot_render(
	struct render_window *window)
{
	long width;
	long height;
	struct bitmap_data *screen;

	global_screenshot_size = PIN(global_screenshot_size, 1, 3);
	width = rasterizer_globals.screen_bounds.x1-rasterizer_globals.screen_bounds.x0;
	height = rasterizer_globals.screen_bounds.y1-rasterizer_globals.screen_bounds.y0;
	screen = bitmap_2d_new(global_screenshot_size*width, global_screenshot_size*height, 0, _bitmap_format_x8r8g8b8);

	if (screen && screen->base_address)
	{
		point2d screenshot_page_index;

		console_printf(TRUE, "");
		console_close();

		for (screenshot_page_index.y = 0; screenshot_page_index.y<global_screenshot_count; screenshot_page_index.y++)
		{
			for (screenshot_page_index.x = 0; screenshot_page_index.x<global_screenshot_count; screenshot_page_index.x++)
			{
				point2d screenshot_index;
				char filename[512];
				struct file_reference file;

				for (screenshot_index.y = 0; screenshot_index.y<global_screenshot_size; screenshot_index.y++)
				{
					for (screenshot_index.x = 0; screenshot_index.x<global_screenshot_size; screenshot_index.x++)
					{
						if (global_screenshot_count<=1 && global_screenshot_size<=1)
						{
							render_frame(window, 1, NULL, NULL, screen, 0.f);
							render_frame_present(NULL, screen);
						}
						else
						{
							render_frame(window, 1, &screenshot_page_index, &screenshot_index, screen, 0.f);
							render_frame_present(&screenshot_index, screen);
						}
					}
				}

				sprintf(filename, "%dscreenshot%d%d.tif", main_globals.screenshot_identifier, screenshot_page_index.y, screenshot_page_index.x);
				file_reference_create_from_path(&file, filename, FALSE);
				screenshot_record(screen, &file);
			}
		}

		main_globals.screenshot_identifier++;
		bitmap_delete(screen);
	}

	global_screenshot_count = 0;

	return;
}

static void screenshot_record(
	struct bitmap_data *screen,
	struct file_reference *file)
{
	char *error_string = tiff_export(file, screen);

	if (error_string)
	{
		error(_error_silent, error_string);
	}

	return;
}

boolean main_taking_screenshot(
	void)
{
	return global_screenshot_count>0 || main_globals.movie;
}

void main_movie_start(
	real frames_per_second)
{
	match_assert("c:\\halo\\SOURCE\\main\\main.c", 2715, main_globals.movie==NULL);

	main_globals.movie = bitmap_2d_new(RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH, RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT, 0, _bitmap_format_x8r8g8b8);

	if (main_globals.movie)
	{
		directory_create_or_delete_contents("movie");
		main_globals.recording_frame_index = 0;

		main_globals.recording_dt = frames_per_second>0.0001f ? 1.f/frames_per_second : SECONDS_PER_TICK;
		game_time_set_speed(1.f);
	}

	return;
}

void main_movie_stop(
	void)
{
	if (main_globals.movie)
	{
		bitmap_delete(main_globals.movie);
		main_globals.movie = NULL;
	}

	return;
}

void main_stop_time(
	void)
{
	main_globals.halt_time_scale = FALSE;
	main_globals.restart_time = FALSE;

	return;
}

void main_start_time(
	void)
{
	main_globals.restart_time = TRUE;

	return;
}

void main_framerate_render(
	void)
{
	if (display_framerate)
	{
		long font_index = hud_globals->messaging.single_player_font.index;

		if (font_index!=NONE)
		{
			char str[4];
			rectangle2d bounds = render.camera.window_bounds;
			short frame_rate = fast_ftol(1.f/MAX(main_globals.seconds_elapsed, 0.01f));

			if (main_globals.vblank_interval_held)
			{
				frame_rate = VBLANKS_PER_SECOND/main_globals.vblank_interval_current;
			}

			_snprintf(str, NUMBEROF(str)-1, "%d", frame_rate);
			str[NUMBEROF(str)-1] = '\0';
			bounds.x0 = bounds.x1-50;
			bounds.y0 = bounds.y1-50;
			draw_string_set_format(_text_style_plain, _text_justification_left, 0);
			draw_string_set_color(frame_rate>=30 ? global_real_argb_green : global_real_argb_red);
			draw_string_set_font(font_index);
			rasterizer_draw_string(&bounds, NULL, NULL, 0, str);
		}
	}

	if (display_vblank_deltas)
	{
		long font_index = hud_globals->messaging.single_player_font.index;

		if (font_index!=NONE)
		{
			short delta_index;
			rectangle2d bounds = render.camera.window_bounds;

			bounds.y0 = bounds.y1-50;
			bounds.x0 = bounds.x1-50;

			for (delta_index = (main_globals.vblank_flip_delta_next_index+14)%15; delta_index!=main_globals.vblank_flip_delta_next_index; delta_index = (delta_index+14)%15)
			{
				char str[4];

				bounds.y0 -= 20;
				bounds.y1 -= 20;
				_snprintf(str, NUMBEROF(str)-1, "%d", main_globals.vblank_flip_deltas[delta_index]);
				str[NUMBEROF(str)-1] = '\0';
				draw_string_set_format(_text_style_plain, _text_justification_right, 0);
				draw_string_set_font(font_index);
				draw_string_set_color(main_globals.vblank_flip_deltas[delta_index]==2 ? global_real_argb_white : global_real_argb_red);
				rasterizer_draw_string(&bounds, NULL, NULL, 0, str);
			}
		}
	}

	if (display_precache_progress && cache_files_precache_in_progress())
	{
		long font_index = hud_globals->messaging.single_player_font.index;

		if (font_index!=NONE)
		{
			real progress;

			if (cache_files_precache_map_status(&progress)==_cached_map_file_in_progress)
			{
				char str[4];
				rectangle2d bounds = render.camera.window_bounds;
				short percent = fast_ftol(progress*100.f);

				_snprintf(str, NUMBEROF(str)-1, "%d", percent);
				str[NUMBEROF(str)-1] = '\0';
				bounds.x0 = bounds.x1-50;
				bounds.y0 = bounds.y1-100;
				draw_string_set_format(_text_style_plain, _text_justification_left, 0);
				draw_string_set_color(global_real_argb_purple);
				draw_string_set_font(font_index);
				rasterizer_draw_string(&bounds, NULL, NULL, 0, str);
			}
		}
	}

	return;
}

void halt_and_catch_fire(
	void)
{
	static boolean recursion_lock = FALSE;

	if (!recursion_lock)
	{
		long font_index;
		short gamepad_index;
		struct scenario *scenario = global_scenario_try_and_get();

		recursion_lock = TRUE;

		for (gamepad_index = 0; gamepad_index<MAXIMUM_GAMEPADS; gamepad_index++)
		{
			if (input_has_gamepad(gamepad_index))
			{
				input_set_gamepad_rumbler_state(gamepad_index, 0, 0);
			}
		}

		if (!scenario || (font_index = interface_get_tag_index(_interface_font_terminal))==NONE)
		{
			font_index = tag_loaded(FONT_GROUP_TAG, "old tags\\internal system plain");
		}

		while (TRUE)
		{
			rasterizer_reset_state();

			{
				struct rasterizer_frame_begin_parameters parameters;

				memset(&parameters, 0, sizeof(parameters));
				rasterizer_frame_begin(&parameters);
			}

			rasterizer_windows_begin();

			{
				struct rasterizer_window_begin_parameters parameters;

				memset(&parameters, 0, sizeof(parameters));
				parameters.camera.position = *global_origin3d;
				parameters.camera.forward = *global_forward3d;
				parameters.camera.up = *global_up3d;
				parameters.camera.mirrored = FALSE;
				parameters.camera.vertical_field_of_view = 2.f*arctangent(render_camera_get_adjusted_field_of_view_tangent(DEGREES_TO_RADIANS(80.f))*0.75f, 1.f);
				parameters.camera.viewport_bounds.x0 = 0;
				parameters.camera.viewport_bounds.x1 = RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH;
				parameters.camera.viewport_bounds.y0 = 0;
				parameters.camera.viewport_bounds.y1 = RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT;
				parameters.camera.z_near = rasterizer_globals.z_near;
				parameters.camera.z_far = rasterizer_globals.z_far;
				render_camera_build_frustum(&parameters.camera, NULL, &parameters.frustum, TRUE);
				parameters.rasterizer_target = _rasterizer_target_render_primary;
				parameters.fog.atmospheric_color = *global_real_rgb_blue;
				parameters.fog.atmospheric_maximum_distance = 0.f;
				parameters.fog.atmospheric_minimum_distance = 0.f;
				parameters.fog.planar_mode = _render_planar_fog_mode_off;
				render.camera = parameters.camera;
				rasterizer_window_begin(&parameters);
			}

			if (font_index!=NONE)
			{
				point2d cursor;
				rectangle2d bounds;

				cursor.x = 0;
				cursor.y = 0;
				bounds = rasterizer_globals.frame_bounds;
				draw_string_set_draw_mode(font_index, _text_style_plain, _text_justification_left, 0, global_real_argb_white);
				draw_string_set_tab_stops(NULL, 0);
				draw_string_set_color(global_real_argb_white);
				rasterizer_draw_string(&bounds, NULL, &cursor, -4, BUILD_NAME " " BUILD_STRING " built at: " BUILD_DATE " " BUILD_TIME);
				bounds.y0 = cursor.y-1;
				rasterizer_draw_string(&bounds, NULL, &cursor, -4, error_get());
			}

			rasterizer_transparent_geometry_draw(TRUE);
			rasterizer_transparent_geometry_draw(FALSE);
			rasterizer_debug_draw();
			rasterizer_window_end();
			rasterizer_windows_end();
			rasterizer_frame_end();
			rasterizer_present(NULL, NULL);
			input_update();
		}
	}

	exit(0);

	return;
}

void main_crash(
	char const *str)
{
	*(char **)NULL = "chucky was here!  NULL belongs to me!!!!!";

	return;
}

void main_print_version(
	void)
{
	console_printf(FALSE, BUILD_NAME " " BUILD_STRING " " BUILD_DATE " " BUILD_TIME);

	return;
}

void main_vertical_blank_interrupt_handler(
	unsigned long context)
{
	rasterizer_globals.vblank_index++;

	if (!main_globals.vblank_flip_counter)
	{
		rasterizer_globals.flip_vblank_index = rasterizer_globals.vblank_index;
	}
	else if (*main_globals.vblank_flip_counter!=rasterizer_globals.flip_index)
	{
		main_globals.vblank_flip_deltas[main_globals.vblank_flip_delta_next_index] = rasterizer_globals.vblank_index-rasterizer_globals.flip_vblank_index;
		main_globals.vblank_flip_delta_next_index = (main_globals.vblank_flip_delta_next_index+1)%15;
		rasterizer_globals.flip_vblank_index = rasterizer_globals.vblank_index;
		rasterizer_globals.flip_index = *main_globals.vblank_flip_counter;
	}

	input_vertical_blank_interrupt();

	return;
}
