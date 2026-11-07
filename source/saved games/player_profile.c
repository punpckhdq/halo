/*
PLAYER_PROFILE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "players.h"
#include "rasterizer.h"
#include "input.h"
#include "console.h"
#include "text_group.h"
#include "main.h"
#include "ui_widget.h"
#include "player_ui.h"
#include "strings/resource.h"
#include "shell.h"
#include "bungie_net/common/thread.h"
#include "input_device_defaults.h"
#include "bitmap_macros.h"

/* ---------- constants */

enum
{
	NUMBER_OF_PRIMARY_COLORS = 18,		/* fake name */
	NUMBER_OF_GOOD_PRIMARY_COLORS = 3,	/* fake name */
	DEFAULT_LOOK_SENSITIVITY = 3,		/* fake name */
};

/* ---------- structures */

struct player_profile_internal
{
	struct player_profile profile;
	XCALCSIG_SIGNATURE checksum;
};

union player_profile_block
{
	byte data[SAVED_GAME_FILE_BLOCK_SIZE];
	struct player_profile_internal profile_block;
};

struct player_profile_write_internal_input
{
	long index;
	struct player_profile profile;
};

struct player_profile_globals
{
	struct player_profile default_profile;
	struct player_profile_write_internal_input thread_input;
	struct thread *thread;
	boolean initialized;
};

/* ---------- prototypes */

static void player_profile_create_default_profiles_on_disk(void);
static void build_default_profile(struct player_profile *profile, long i);
static boolean player_profile_read(long index, struct player_profile *profile);
static void player_profile_write(long index, struct player_profile *profile);
static unsigned long __stdcall player_profile_write_thread_proc(void *input);

/* ---------- globals */

static pixel32 profile_color_table[NUMBER_OF_PRIMARY_COLORS] =
{
	PIXEL32_FROM_ARGB(0, 255, 255, 255),
	PIXEL32_FROM_ARGB(0, 0, 0, 0),
	PIXEL32_FROM_ARGB(0, 254, 0, 0),
	PIXEL32_FROM_ARGB(0, 2, 1, 227),
	PIXEL32_FROM_ARGB(0, 112, 126, 113),
	PIXEL32_FROM_ARGB(0, 255, 255, 1),
	PIXEL32_FROM_ARGB(0, 0, 255, 1),
	PIXEL32_FROM_ARGB(0, 255, 86, 185),
	PIXEL32_FROM_ARGB(0, 171, 16, 244),
	PIXEL32_FROM_ARGB(0, 1, 255, 255),
	PIXEL32_FROM_ARGB(0, 100, 147, 237),
	PIXEL32_FROM_ARGB(0, 255, 127, 0),
	PIXEL32_FROM_ARGB(0, 30, 204, 145),
	PIXEL32_FROM_ARGB(0, 0, 100, 1),
	PIXEL32_FROM_ARGB(0, 96, 56, 20),
	PIXEL32_FROM_ARGB(0, 198, 156, 108),
	PIXEL32_FROM_ARGB(0, 157, 11, 14),
	PIXEL32_FROM_ARGB(0, 245, 153, 158)
};

struct player_profile_globals player_profile_globals = {0};

/* ---------- public code */

void player_profiles_initialize(
	void)
{
	memset(&player_profile_globals, 0, sizeof(player_profile_globals));
	player_profile_globals.initialized = TRUE;
	player_profile_create_default_profiles_on_disk();

	return;
}

void player_profiles_dispose(
	void)
{
	if (player_profile_globals.thread)
	{
		error(_error_silent, "waiting for asynchronous player profile writes to finish...");

		while (!thread_has_exited(player_profile_globals.thread))
		{
		}

		dispose_thread(player_profile_globals.thread);
		player_profile_globals.thread = NULL;
	}

	memset(&player_profile_globals, 0, sizeof(player_profile_globals));

	return;
}

void player_profiles_enumerate_available_to_local_player_index(
	short local_player_index,
	word *number_of_profiles,
	long *player_profile_indices,
	boolean include_default_profiles)
{
	saved_game_files_enumerate_available_to_local_player_index(
		local_player_index,
		_saved_game_file_type_player_profile,
		number_of_profiles,
		player_profile_indices,
		include_default_profiles);

	return;
}

void player_profile_delete(
	long player_profile_index)
{
	if (player_profile_index != NONE &&
		!delete_enumerated_saved_game_file(player_profile_index))
	{
		error(
			_error_silent,
			"player_profile_delete() failed (profile index= #0x%lX)",
			player_profile_index);
	}

	return;
}

boolean player_profile_get(
	long player_profile_index,
	struct player_profile *profile)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\saved games\\player_profile.c", 194, profile);

	if (player_profile_index == NONE)
	{
		memcpy(profile, &player_profile_globals.default_profile, sizeof(*profile));
	}
	else
	{
		success = player_profile_read(player_profile_index, profile);
	}

	return success;
}

boolean player_profile_get_from_path(
	char *full_path,
	struct player_profile *profile)
{
	union player_profile_block block;
	struct file_reference profile_file;
	XCALCSIG_SIGNATURE checksum;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\saved games\\player_profile.c", 216, full_path && profile);

	if (file_reference_create_from_path(&profile_file, full_path, FALSE) &&
		file_open(&profile_file, FLAG(_permission_read_bit)))
	{
		if (file_read(&profile_file, sizeof(block), &block))
		{
			saved_game_file_generate_checksum(
				&block.profile_block.profile,
				sizeof(struct player_profile),
				&checksum);

			if (!memcmp(&checksum, &block.profile_block.checksum, sizeof(checksum)))
			{
				memcpy(profile, &block.profile_block.profile, sizeof(*profile));
				success = TRUE;
			}
			else
			{
				error(_error_silent, "checksum failed on player profile file");
			}
		}
		else
		{
			error(_error_silent, "failed to read player profile");
		}

		file_close(&profile_file);
	}
	else
	{
		error(_error_silent, "failed to open player profile file");
	}

	return success;
}

void player_profile_save(
	long player_profile_index,
	struct player_profile *profile)
{
	match_assert("c:\\halo\\SOURCE\\saved games\\player_profile.c", 256, profile);

	if (player_profile_index != NONE)
	{
		player_profile_write(player_profile_index, profile);
	}

	return;
}

real_argb_color player_profile_get_argb_color(
	long index)
{
	real_argb_color color;

	color.rgb = player_profile_get_rgb_color(index);
	color.alpha = 1.f;

	return color;
}

word player_profile_number_of_available_primary_colors(
	void)
{
	return NUMBER_OF_PRIMARY_COLORS;
}

real_rgb_color player_profile_get_rgb_color(
	long index)
{
	real_rgb_color color;
	long color_value;

	index = MIN(NUMBER_OF_PRIMARY_COLORS - 1, index);
	index = MAX(0, index);
	color_value = profile_color_table[index];

	color.red = PIXEL32_RED(color_value) / 255.f;
	color.green = PIXEL32_GREEN(color_value) / 255.f;
	color.blue = PIXEL32_BLUE(color_value) / 255.f;

	return color;
}

long player_profile_get_random_good_color(
	void)
{
	return local_random_range(0, NUMBER_OF_GOOD_PRIMARY_COLORS);
}

long player_profile_get_random_color(
	void)
{
	return local_random_range(0, NUMBER_OF_PRIMARY_COLORS - 1);
}

void player_profile_save_last_level_played(
	short local_player_index)
{
	struct player_profile profile;
	long player_profile_index;
	short level = main_get_current_solo_level();

	match_assert("c:\\halo\\SOURCE\\saved games\\player_profile.c", 378, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	if (level != NONE)
	{
		match_assert("c:\\halo\\SOURCE\\saved games\\player_profile.c", 381, (level>=0) && (level<NUMBER_OF_SINGLE_PLAYER_LEVELS));

		player_profile_index = player_ui_get_active_player_profile_index(local_player_index);

		if (player_profile_index != NONE)
		{
			player_ui_get_active_player_profile(local_player_index, &profile);

			if (profile.last_single_player_map_played != level)
			{
				profile.last_single_player_map_played = level;
				player_profile_save(player_profile_index, &profile);
			}

			player_ui_set_active_player_profile(local_player_index, player_profile_index, &profile);
		}
	}

	return;
}

void player_profile_save_level_completed(
	short local_player_index)
{
	struct player_profile profile;
	short level;
	short difficulty;
	long player_profile_index;

	match_assert("c:\\halo\\SOURCE\\saved games\\player_profile.c", 407, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	level = main_get_current_solo_level();
	difficulty = game_difficulty_level_get();

	match_assert("c:\\halo\\SOURCE\\saved games\\player_profile.c", 413, (level>=0) && (level<NUMBER_OF_SINGLE_PLAYER_LEVELS) && (difficulty >= 0) && (difficulty < NUMBER_OF_GAME_DIFFICULTY_LEVELS));

	player_profile_index = player_ui_get_active_player_profile_index(local_player_index);

	if (player_profile_index != NONE)
	{
		player_ui_get_active_player_profile(local_player_index, &profile);
		SET_FLAG(profile.single_player_map_flags[level], difficulty, TRUE);
		player_profile_write(player_profile_index, &profile);
		player_ui_set_active_player_profile(local_player_index, player_profile_index, &profile);
	}
	else
	{
		error(_error_silent, "failed to save player's current level as being completed because the player profile was not found");
	}

	return;
}

void player_profile_get_highest_completed_solo_level(
	struct player_profile const *profile,
	short *level,
	short *difficulty)
{
	long level_index;

	match_assert("c:\\halo\\SOURCE\\saved games\\player_profile.c", 440, profile && level && difficulty);

	*level = NONE;
	*difficulty = _game_difficulty_level_normal;

	for (level_index = 0; level_index < NUMBER_OF_SINGLE_PLAYER_LEVELS; level_index++)
	{
		if (profile->single_player_map_flags[level_index])
		{
			if (TEST_FLAG(profile->single_player_map_flags[level_index], _game_difficulty_level_impossible))
			{
				*level = level_index;
				*difficulty = _game_difficulty_level_impossible;
			}
			else if (TEST_FLAG(profile->single_player_map_flags[level_index], _game_difficulty_level_hard))
			{
				*level = level_index;
				*difficulty = _game_difficulty_level_hard;
			}
			else if (TEST_FLAG(profile->single_player_map_flags[level_index], _game_difficulty_level_normal))
			{
				*level = level_index;
				*difficulty = _game_difficulty_level_normal;
			}
			else if (TEST_FLAG(profile->single_player_map_flags[level_index], _game_difficulty_level_easy))
			{
				*level = level_index;
				*difficulty = _game_difficulty_level_easy;
			}
		}
	}

	return;
}

boolean player_profile_get_enclosing_directory_path(
	long player_profile_index,
	char *full_path)
{
	return saved_game_file_get_path_to_enclosing_directory(player_profile_index, full_path);
}

static void player_profile_create_default_profiles_on_disk(
	void)
{
	long i;

	for (i = 0; i < NUMBER_OF_DEFAULT_PROFILES; i++)
	{
		union player_profile_block block;
		char full_path[MAXIMUM_FILENAME_LENGTH+1];
		struct file_reference file;
		boolean success = FALSE;

		build_default_profile(&block.profile_block.profile, i);
		_snprintf(
			full_path,
			MAXIMUM_FILENAME_LENGTH,
			DEFAULT_PLAYER_PROFILE_PATH_FORMAT,
			i);

		if (file_reference_create_from_path(&file, full_path, FALSE))
		{
			saved_game_file_generate_checksum(
				&block.profile_block.profile,
				sizeof(struct player_profile),
				&block.profile_block.checksum);

			if (file_create(&file) &&
				file_open(&file, FLAG(_permission_write_bit)) &&
				file_set_position(&file, 0))
			{
				success = file_write(&file, sizeof(block), &block);

				if (!file_close(&file))
				{
					error(
						_error_silent,
						"failed to close default player profile file '%s'",
						full_path);
				}
			}
		}

		if (!success)
		{
			error(
				_error_silent,
				"failed to create/update default player profile file '%s' on disk",
				full_path);
		}
	}

	return;
}

static void build_default_profile(
	struct player_profile *profile,
	long i)
{
	match_assert("c:\\halo\\SOURCE\\saved games\\player_profile.c", 567, (profile != NULL) && (i>=0) && (i<NUMBER_OF_DEFAULT_PROFILES));

	memset(profile, 0, sizeof(*profile));
	profile->primary_color_index = NONE;
	profile->controller_settings.look_sensitivity = DEFAULT_LOOK_SENSITIVITY;
	profile->controller_settings.invert_look = FALSE;
	profile->controller_settings.flight_stick_aircraft_controls = FALSE;
	profile->controller_settings.ingame_help_disabled = FALSE;
	SET_FLAG(profile->flags, _profile_is_default_bit, TRUE);
	profile->flags |= i << CHAR_BITS;
	profile->controller_settings.vibration_disabled = FALSE;
	profile->last_single_player_map_played = 0;

	switch (i)
	{
	case 0:
		profile->controller_settings.button_preset = _button_preset_standard;
		profile->controller_settings.joystick_preset = _joystick_preset_standard;
		break;

	case 1:
		profile->controller_settings.invert_look = TRUE;
		profile->controller_settings.button_preset = _button_preset_standard;
		profile->controller_settings.joystick_preset = _joystick_preset_standard;
		break;

	default:
		match_vhalt(
			"c:\\halo\\SOURCE\\saved games\\player_profile.c",
			594,
			"unknown default profile configuration requested");
		break;
	}

	return;
}

static boolean player_profile_read(
	long index,
	struct player_profile *profile)
{
	struct file_reference profile_file;
	union player_profile_block block;
	XCALCSIG_SIGNATURE checksum;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\saved games\\player_profile.c", 609, profile);

	if (player_profile_globals.thread)
	{
		error(_error_silent, "waiting for asynchronous player profile io to finish...");

		while (!thread_has_exited(player_profile_globals.thread))
		{
		}

		dispose_thread(player_profile_globals.thread);
		player_profile_globals.thread = NULL;
	}

	if (TEST_FLAG(index, _profile_checksum_valid_bit31))
	{
		if (saved_game_files_take_mutex())
		{
			if (saved_game_file_open(&profile_file, index))
			{
				if (file_read(&profile_file, sizeof(block), &block))
				{
					saved_game_file_generate_checksum(
						&block.profile_block.profile,
						sizeof(struct player_profile),
						&checksum);

					if (!memcmp(&checksum, &block.profile_block.checksum, sizeof(checksum)))
					{
						memcpy(profile, &block.profile_block.profile, sizeof(*profile));
					}
					else
					{
						struct player_profile sanitized_profile;

						error(_error_silent, "checksum failed on player profile file, sanitizing memory resident version...");

						memset(&sanitized_profile, 0, sizeof(sanitized_profile));
						sanitized_profile.primary_color_index = NONE;
						sanitized_profile.controller_settings.look_sensitivity = DEFAULT_LOOK_SENSITIVITY;
						sanitized_profile.controller_settings.invert_look = FALSE;
						sanitized_profile.controller_settings.flight_stick_aircraft_controls = FALSE;
						sanitized_profile.controller_settings.ingame_help_disabled = FALSE;
						sanitized_profile.controller_settings.vibration_disabled = FALSE;
						sanitized_profile.last_single_player_map_played = 0;
						sanitized_profile.controller_settings.button_preset = _button_preset_standard;
						sanitized_profile.controller_settings.joystick_preset = _joystick_preset_standard;
						sanitized_profile.flags = 0;
						ustrncpy(
							sanitized_profile.player_name,
							saved_game_file_get_display_name(index),
							MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
						sanitized_profile.player_name[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = '\0';
						memcpy(profile, &sanitized_profile, sizeof(*profile));
					}

					success = TRUE;
				}
				else
				{
					error(_error_silent, "failed to read player profile from file");
				}

				saved_game_file_close(&profile_file, index);
			}
			else
			{
				error(_error_silent, "failed to open player profile file");
			}

			saved_game_files_release_mutex();
		}
		else
		{
			error(_error_silent, "failed to get saved game files mutex; perhaps another operation is in progress?");
		}
	}
	else
	{
		struct player_profile sanitized_profile;

		error(_error_silent, "checksum failed on player profile file, sanitizing memory resident version...");

		memset(&sanitized_profile, 0, sizeof(sanitized_profile));
		sanitized_profile.primary_color_index = NONE;
		sanitized_profile.controller_settings.look_sensitivity = DEFAULT_LOOK_SENSITIVITY;
		sanitized_profile.controller_settings.invert_look = FALSE;
		sanitized_profile.controller_settings.flight_stick_aircraft_controls = FALSE;
		sanitized_profile.controller_settings.ingame_help_disabled = FALSE;
		sanitized_profile.controller_settings.vibration_disabled = FALSE;
		sanitized_profile.last_single_player_map_played = 0;
		sanitized_profile.controller_settings.button_preset = _button_preset_standard;
		sanitized_profile.controller_settings.joystick_preset = _joystick_preset_standard;
		sanitized_profile.flags = 0;
		ustrncpy(
			sanitized_profile.player_name,
			saved_game_file_get_display_name(index),
			MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
		sanitized_profile.player_name[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = '\0';
		memcpy(profile, &sanitized_profile, sizeof(*profile));

		success = TRUE;
	}

	return success;
}

static void player_profile_write(
	long index,
	struct player_profile *profile)
{
	match_assert("c:\\halo\\SOURCE\\saved games\\player_profile.c", 704, profile);

	if (player_profile_globals.thread)
	{
		error(_error_silent, "waiting for asynchronous player profile io to finish...");

		while (!thread_has_exited(player_profile_globals.thread))
		{
		}

		dispose_thread(player_profile_globals.thread);
		player_profile_globals.thread = NULL;
	}

	player_profile_globals.thread_input.index = index;
	memcpy(&player_profile_globals.thread_input.profile, profile, sizeof(*profile));
	create_thread(
		0,
		player_profile_write_thread_proc,
		&player_profile_globals.thread_input,
		&player_profile_globals.thread);

	return;
}

static unsigned long __stdcall player_profile_write_thread_proc(
	void *input)
{
	struct player_profile_write_internal_input *thread_input = (struct player_profile_write_internal_input *)input;

	match_assert("c:\\halo\\SOURCE\\saved games\\player_profile.c", 727, input);

	error(_error_silent, "begin player profile write");

	if (saved_game_files_take_mutex())
	{
		struct file_reference profile_file;
		union player_profile_block block;
		long index = thread_input->index;
		boolean failed = FALSE;
		struct player_profile *profile = &thread_input->profile;

		if (saved_game_file_open(&profile_file, index))
		{
			memcpy(&block.profile_block.profile, profile, sizeof(*profile));
			saved_game_file_generate_checksum(
				&block.profile_block.profile,
				sizeof(struct player_profile),
				&block.profile_block.checksum);

			if (!file_set_position(&profile_file, 0) ||
				!file_write(&profile_file, sizeof(block), &block))
			{
				error(_error_silent, "failed to write player profile to file");
				failed = TRUE;
			}

			if (saved_game_file_close(&profile_file, index) &&
				!synchronize_metadata_display_name_with_profile_name(index, profile->player_name))
			{
				error(_error_silent, "metadata name may not match game display name");
			}
		}
		else
		{
			error(_error_silent, "failed to open player profile file");
		}

		if (failed)
		{
			delete_enumerated_saved_game_file(index);
		}

		saved_game_files_release_mutex();
	}
	else
	{
		error(_error_silent, "failed to get saved game files mutex; perhaps another operation is in progress?");
	}

	error(_error_silent, "end player profile write");

	return 0;
}

long player_profile_new(
	short local_player_index,
	wchar_t *name)
{
	struct file_reference profile_file;
	long player_profile_index = create_enumerated_saved_game_file(
		_saved_game_file_type_player_profile,
		local_player_index,
		name);

	if (player_profile_index != NONE)
	{
		if (saved_game_file_open(&profile_file, player_profile_index))
		{
			long level;
			union player_profile_block block = {0};
			struct player_profile *profile = &block.profile_block.profile;

			memset(profile, 0, sizeof(*profile));
			profile->primary_color_index = NONE;
			profile->controller_settings.look_sensitivity = DEFAULT_LOOK_SENSITIVITY;
			profile->controller_settings.invert_look = FALSE;
			profile->controller_settings.flight_stick_aircraft_controls = FALSE;
			profile->controller_settings.ingame_help_disabled = FALSE;
			profile->controller_settings.vibration_disabled = FALSE;
			profile->last_single_player_map_played = 0;
			profile->controller_settings.button_preset = _button_preset_standard;
			profile->controller_settings.joystick_preset = _joystick_preset_standard;
			profile->flags = 0;
			ustrncpy(profile->player_name, name, MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
			profile->player_name[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = '\0';

			error(_error_silent, "### DEBUG unlocking all solo levels for newly created profile");

			for (level = 0; level < NUMBER_OF_SINGLE_PLAYER_LEVELS; level++)
			{
				long difficulty = 0;

				do
				{
					SET_FLAG(profile->single_player_map_flags[level], difficulty, TRUE);
					difficulty++;
				}
				while (difficulty < NUMBER_OF_GAME_DIFFICULTY_LEVELS);
			}

			saved_game_file_generate_checksum(profile, sizeof(*profile), &block.profile_block.checksum);

			if (!file_set_position(&profile_file, 0) ||
				!file_write(&profile_file, sizeof(block), &block))
			{
				error(_error_silent, "failed to initialize newly created player profile");
				delete_enumerated_saved_game_file(player_profile_index);
				player_profile_index = NONE;
			}

			saved_game_file_close(&profile_file, player_profile_index);
		}
		else
		{
			error(_error_silent, "failed to open newly created player profile");
			delete_enumerated_saved_game_file(player_profile_index);
			player_profile_index = NONE;
		}
	}
	else
	{
		error(_error_silent, "failed to create new player profile");
	}

	return player_profile_index;
}
