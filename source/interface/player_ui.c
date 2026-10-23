/*
PLAYER_UI.C
*/

/* ---------- headers */

#include "cseries.h"
#include "player_ui.h"
#include "ui_widget.h"
#include "hud.h"
#include "hud_messaging.h"
#include "virtual_keyboard.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "input.h"
#include "console.h"
#include "rasterizer_console_vars.h"
#include "sound_manager.h"
#include "text_group.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "texture_cache.h"
#include "strings/resource.h"
#include "playlist_profile.h"

/* ---------- constants */

enum
{
	MAXIMUM_LOOK_SENSITIVITY_SETTINGS = 10,			/* fake name */
	TEMPORARY_STRINGS_INVERT_LOOK_STRING_INDEX = 1,	/* fake name */
	PROFILE_NAME_CAPTION_INDEX = 10					/* fake name */
};

/* ---------- macros */

#define PREGAME_SCREEN_WIDGET_NAME "ui\\shell\\main_menu\\multiplayer_type_select\\connected\\pregame\\connected_pregame_screen"	/* fake name */
#define TEMPORARY_STRINGS_TAG_NAME "ui\\shell\\strings\\temp_strings"																/* fake name */

/* ---------- structures */

struct _player_data
{
	struct player_profile profile;
	long profile_index;
	boolean prejoined_multiplayer;
};

struct _edit_profile_data
{
	long profile_index;

	union
	{
		struct player_profile player_profile;	/* fake name */
		struct game_variant playlist_profile;	/* fake name */
	} edit_profile;

	union
	{
		struct player_profile player_profile;	/* fake name */
		struct game_variant playlist_profile;	/* fake name */
	} original_profile;
};

struct _player_ui_globals
{
	struct _player_data player_data[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	boolean join_players_to_next_multiplayer_game[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	short single_player_controller_index[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	struct game_variant variant;
	boolean user_selected_variant;
	struct _edit_profile_data edit_profile_data;
	boolean initialized;
};

/* ---------- prototypes */

static void hud_message_to_all(wchar_t const *string);
static void generate_default_player_profile(struct player_profile *profile);
static void set_local_player_controls_from_player_profile(short local_player_index);
static void clear_profile_edit_data(void);
static void reset_local_player_profile(short local_player_index);

/* ---------- globals */

struct _player_ui_globals player_ui_globals = {0};

static char player1_profile_path[MAXIMUM_FILENAME_LENGTH+1] = {0};
static long player1_last_used_profile_index = NONE;

/* ---------- public code */

void player_ui_initialize(
	void)
{
	long local_player_index;

	memset(&player_ui_globals, 0, sizeof(player_ui_globals));

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		reset_local_player_profile(local_player_index);
	}

	clear_profile_edit_data();
	player_ui_globals.initialized = TRUE;

	return;
}

void player_ui_dispose(
	void)
{
	memset(&player_ui_globals, 0, sizeof(player_ui_globals));

	return;
}

void player_ui_reset_single_player_local_player_controllers(
	void)
{
	memset(player_ui_globals.single_player_controller_index, NONE, sizeof(player_ui_globals.single_player_controller_index));

	return;
}

void player_ui_set_single_player_local_player_controller(
	short local_player_index,
	short controller_index)
{
	match_vassert("c:\\halo\\SOURCE\\interface\\player_ui.c", 119, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS), "invalid local player index");
	match_vassert("c:\\halo\\SOURCE\\interface\\player_ui.c", 121, (controller_index>=0) && (controller_index<MAXIMUM_GAMEPADS), "invalid controller index");

	player_ui_globals.single_player_controller_index[local_player_index] = controller_index;

	return;
}

short player_ui_get_single_player_local_player_controller(
	short local_player_index)
{
	match_vassert("c:\\halo\\SOURCE\\interface\\player_ui.c", 132, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS), "invalid local player index");

	return player_ui_globals.single_player_controller_index[local_player_index];
}

short player_ui_get_single_player_local_player_from_controller(
	short controller_index)
{
	short local_player_index;
	short result = NONE;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		if (player_ui_globals.single_player_controller_index[local_player_index] == controller_index)
		{
			result = local_player_index;
			break;
		}
	}

	return result;
}

void player_ui_local_player_joined_multiplayer_game(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 157, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	player_ui_globals.player_data[local_player_index].prejoined_multiplayer = TRUE;
	player_ui_globals.join_players_to_next_multiplayer_game[local_player_index] = TRUE;

	return;
}

boolean player_ui_local_player_wants_to_play_multiplayer(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 167, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	return player_ui_globals.player_data[local_player_index].prejoined_multiplayer;
}

void player_ui_clear_multiplayer_autojoin_for_local_player(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 175, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	player_ui_globals.player_data[local_player_index].prejoined_multiplayer = FALSE;

	return;
}

void player_ui_autojoin_players_to_next_multiplayer_game(
	void)
{
	long local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		player_ui_globals.player_data[local_player_index].prejoined_multiplayer = player_ui_globals.join_players_to_next_multiplayer_game[local_player_index];
	}

	return;
}

void player_ui_clear_multiplayer_joins(
	void)
{
	long local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		reset_local_player_profile((short)local_player_index);
		player_ui_globals.player_data[local_player_index].prejoined_multiplayer = FALSE;
		player_ui_globals.join_players_to_next_multiplayer_game[local_player_index] = FALSE;
	}

	return;
}

void player_ui_clear_multiplayer_variant(
	void)
{
	player_ui_globals.user_selected_variant = FALSE;
	game_connection_set(_game_connection_local);
	game_engine_dispose();
	game_set_game_variant(NULL);

	return;
}

void player_ui_set_active_player_profile(
	short local_player_index,
	long profile_index,
	struct player_profile const *profile)
{
	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 226, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS) && (profile != NULL));

	player_ui_globals.player_data[local_player_index].profile_index = profile_index;
	memcpy(&player_ui_globals.player_data[local_player_index].profile, profile, sizeof(*profile));
	set_local_player_controls_from_player_profile(local_player_index);

	return;
}

void player_ui_get_active_player_profile(
	short local_player_index,
	struct player_profile *profile)
{
	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 238, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS) && (profile != NULL));

	memcpy(profile, &player_ui_globals.player_data[local_player_index].profile, sizeof(*profile));

	return;
}

long player_ui_get_active_player_profile_index(
	short local_player_index)
{
	long profile_index;

	if (local_player_index >= 0 && local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
	{
		profile_index = player_ui_globals.player_data[local_player_index].profile_index;
	}
	else
	{
		profile_index = NONE;
	}

	return profile_index;
}

short player_ui_get_last_single_player_level_played(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 265, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	return player_ui_globals.player_data[local_player_index].profile.last_single_player_map_played;
}

void player_ui_set_game_variant(
	struct game_variant const *variant)
{
	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 273, variant);

	memcpy(&player_ui_globals.variant, variant, sizeof(player_ui_globals.variant));
	player_ui_globals.user_selected_variant = TRUE;

	return;
}

boolean player_ui_game_variant_specified(
	struct game_variant *variant)
{
	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 284, variant);

	if (player_ui_globals.user_selected_variant)
	{
		memcpy(variant, &player_ui_globals.variant, sizeof(*variant));
	}

	return player_ui_globals.user_selected_variant;
}

boolean player_ui_rumble_disabled(
	short local_player_index)
{
	if (local_player_index == NONE)
	{
		return FALSE;
	}

	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 305, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	return player_ui_globals.player_data[local_player_index].profile.controller_settings.vibration_disabled;
}

boolean player_ui_autolevel_enabled(
	short controller_index)
{
	short local_player_index = controller_index;

	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 320, (controller_index>=0) && (controller_index<MAXIMUM_GAMEPADS));

	if (!network_game_is_active())
	{
		local_player_index = player_ui_get_single_player_local_player_from_controller(controller_index);
	}

	if (local_player_index == NONE)
	{
		return FALSE;
	}

	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 340, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	return player_ui_globals.player_data[local_player_index].profile.controller_settings.autocenter;
}

boolean player_ui_get_path_to_local_player_profile_directory(
	short local_player_index,
	char *full_path)
{
	boolean success;

	if (local_player_index >= 0 && local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
	{
		success = player_profile_get_enclosing_directory_path(player_ui_globals.player_data[local_player_index].profile_index, full_path);
	}
	else
	{
		success = FALSE;
	}

	return success;
}

void player_ui_remember_player1_profile(
	boolean write_to_disk)
{
	if (player1_last_used_profile_index != player_ui_globals.player_data[0].profile_index)
	{
		if (player_ui_globals.player_data[0].profile_index == NONE ||
			!player_profile_get_enclosing_directory_path(player_ui_globals.player_data[0].profile_index, player1_profile_path))
		{
			error(_error_silent, "player 1 has no active player profile assigned");
		}

		player1_last_used_profile_index = player_ui_globals.player_data[0].profile_index;
	}

	if (write_to_disk && player1_profile_path[0] != '\0')
	{
		saved_game_file_remember_player1_last_used_profile_directory(player1_profile_path);
	}

	return;
}

long player_ui_get_player1_last_used_profile_index(
	void)
{
	if (player1_profile_path[0] == '\0' && saved_game_file_retrieve_player1_last_used_profile_directory(player1_profile_path))
	{
		player1_last_used_profile_index = saved_game_file_find_profile_index_for_directory_path(player1_profile_path, _saved_game_file_type_player_profile);
	}

	return player1_last_used_profile_index;
}

void player_ui_fast_setup_network_server(
	void)
{
	ui_widgets_close_all();
	dispose_global_network_game_server();
	dispose_global_network_game_client();
	game_connection_set(_game_connection_local);
	main_set_multiplayer_map_name("");
	player_ui_globals.user_selected_variant = FALSE;

	if (ui_widget_load_by_name_or_tag(PREGAME_SCREEN_WIDGET_NAME, NONE, NULL, NONE, NONE, NONE, NONE))
	{
		game_engine_playlist_initialize();
		network_game_accept_remote_connections(TRUE);

		if (create_global_network_game_server() && create_global_network_game_client())
		{
			game_engine_playlist_begin();
			game_connection_set(_game_connection_network_server);
		}
		else
		{
			dispose_global_network_game_server();
			dispose_global_network_game_client();
			network_game_accept_remote_connections(FALSE);
			error(_error_silent, "failed to initiate a multiplayer game server");
			main_goto_main_menu();
		}
	}
	else
	{
		error(_error_silent, "failed to load network pregame screen... maybe you ran this from some place other than the game shell UI?");
		main_goto_main_menu();
	}

	return;
}

boolean player_ui_edit_profile_is_default_profile(
	void)
{
	boolean is_default = FALSE;

	if (player_ui_globals.edit_profile_data.profile_index != NONE)
	{
		switch (saved_game_file_get_type(player_ui_globals.edit_profile_data.profile_index))
		{
		case _saved_game_file_type_player_profile:
		case _saved_game_file_type_play_list:
			is_default = TEST_FLAG(player_ui_globals.edit_profile_data.profile_index, _default_profile_bit30);
			break;
		default:
			error(_error_silent, "unknown saved game file type being edited");
			break;
		}
	}

	return is_default;
}

boolean player_ui_edit_profile_name_is_dirty(
	void)
{
	boolean dirty = FALSE;

	if (player_ui_globals.edit_profile_data.profile_index != NONE)
	{
		switch (saved_game_file_get_type(player_ui_globals.edit_profile_data.profile_index))
		{
		case _saved_game_file_type_player_profile:
			if (ustrncmp(
				player_ui_globals.edit_profile_data.edit_profile.player_profile.player_name,
				player_ui_globals.edit_profile_data.original_profile.player_profile.player_name,
				MAXIMUM_PLAYER_PROFILE_NAME_LENGTH+1))
			{
				dirty = TRUE;
			}
			break;
		case _saved_game_file_type_play_list:
			if (ustrncmp(
				player_ui_globals.edit_profile_data.edit_profile.playlist_profile.human_readable_game_description,
				player_ui_globals.edit_profile_data.original_profile.playlist_profile.human_readable_game_description,
				MAXIMUM_GAME_VARIANT_NAME_LENGTH))
			{
				dirty = TRUE;
			}
			break;
		default:
			error(_error_silent, "unknown saved game file type being edited");
			break;
		}
	}
	else
	{
		error(_error_silent, "not currently editing a saved game file");
	}

	return dirty;
}

boolean player_ui_prompt_user_to_rename_edit_profile(
	void)
{
	boolean success = FALSE;

	if (player_ui_globals.edit_profile_data.profile_index != NONE)
	{
		switch (saved_game_file_get_type(player_ui_globals.edit_profile_data.profile_index))
		{
		case _saved_game_file_type_player_profile:
			success = virtual_keyboard_launch(
				player_ui_globals.edit_profile_data.edit_profile.player_profile.player_name,
				sizeof(player_ui_globals.edit_profile_data.edit_profile.player_profile.player_name),
				PROFILE_NAME_CAPTION_INDEX);
			break;
		case _saved_game_file_type_play_list:
			success = virtual_keyboard_launch(
				player_ui_globals.edit_profile_data.edit_profile.playlist_profile.human_readable_game_description,
				sizeof(player_ui_globals.edit_profile_data.edit_profile.playlist_profile.human_readable_game_description),
				PROFILE_NAME_CAPTION_INDEX);
			break;
		default:
			error(_error_silent, "unknown saved game file type being edited");
			break;
		}
	}
	else
	{
		error(_error_silent, "not currently editing a saved game file");
	}

	return success;
}

struct player_profile *player_ui_get_edit_player_profile(
	void)
{
	struct player_profile *profile;

	if (saved_game_file_get_type(player_ui_globals.edit_profile_data.profile_index) == _saved_game_file_type_player_profile)
	{
		profile = &player_ui_globals.edit_profile_data.edit_profile.player_profile;
	}
	else
	{
		profile = NULL;
	}

	return profile;
}

struct game_variant *player_ui_get_edit_playlist_profile(
	void)
{
	struct game_variant *variant;

	if (saved_game_file_get_type(player_ui_globals.edit_profile_data.profile_index) == _saved_game_file_type_play_list)
	{
		variant = &player_ui_globals.edit_profile_data.edit_profile.playlist_profile;
	}
	else
	{
		variant = NULL;
	}

	return variant;
}

void player_ui_begin_editing_profile(
	long profile_index)
{
	clear_profile_edit_data();

	switch (saved_game_file_get_type(profile_index))
	{
	case _saved_game_file_type_player_profile:
		if (player_profile_get(profile_index, &player_ui_globals.edit_profile_data.original_profile.player_profile))
		{
			memcpy(
				&player_ui_globals.edit_profile_data.edit_profile.player_profile,
				&player_ui_globals.edit_profile_data.original_profile.player_profile,
				sizeof(struct player_profile));
			player_ui_globals.edit_profile_data.profile_index = profile_index;
		}
		else
		{
			error(_error_silent, "failed to retrieve player profile #%08lX for editing", profile_index);
		}
		break;
	case _saved_game_file_type_play_list:
		if (playlist_profile_get(profile_index, &player_ui_globals.edit_profile_data.original_profile.playlist_profile))
		{
			memcpy(
				&player_ui_globals.edit_profile_data.edit_profile.playlist_profile,
				&player_ui_globals.edit_profile_data.original_profile.playlist_profile,
				sizeof(struct game_variant));
			player_ui_globals.edit_profile_data.profile_index = profile_index;
		}
		else
		{
			error(_error_silent, "failed to retrieve playlist profile #%08lX for editing", profile_index);
		}
		break;
	default:
		error(_error_silent, "invalid profile index (#%08lX)", profile_index);
		break;
	}

	return;
}

boolean player_ui_save_profile(
	void)
{
	boolean success = FALSE;

	switch (saved_game_file_get_type(player_ui_globals.edit_profile_data.profile_index))
	{
	case _saved_game_file_type_player_profile:
		if (TEST_FLAG(player_ui_globals.edit_profile_data.profile_index, _default_profile_bit30))
		{
			error(_error_silent, "### WARNING: saving over a default player profile");
		}

		if (!player_ui_edit_profile_is_dirty())
		{
			error(_error_silent, "### WARNING: saving player profile even though it hasn't been changed");
		}

		player_profile_save(player_ui_globals.edit_profile_data.profile_index, &player_ui_globals.edit_profile_data.edit_profile.player_profile);
		success = TRUE;
		break;
	case _saved_game_file_type_play_list:
		if (!player_ui_edit_profile_is_dirty())
		{
			error(_error_silent, "### WARNING: saving player profile even though it hasn't been changed");
		}

		if (TEST_FLAG(player_ui_globals.edit_profile_data.profile_index, _default_profile_bit30))
		{
			if (ustrncmp(
				player_ui_globals.edit_profile_data.edit_profile.playlist_profile.human_readable_game_description,
				player_ui_globals.edit_profile_data.original_profile.playlist_profile.human_readable_game_description,
				MAXIMUM_GAME_VARIANT_NAME_LENGTH))
			{
				long new_profile_index;

				SET_FLAG(player_ui_globals.edit_profile_data.edit_profile.playlist_profile.flags, _variant_is_system_default_bit, FALSE);
				new_profile_index = playlist_profile_new(0, player_ui_globals.edit_profile_data.edit_profile.playlist_profile.human_readable_game_description);

				if (new_profile_index != NONE)
				{
					char full_path[MAXIMUM_FILENAME_LENGTH+1];

					playlist_profile_save(new_profile_index, &player_ui_globals.edit_profile_data.edit_profile.playlist_profile);
					player_ui_globals.edit_profile_data.profile_index = new_profile_index;

					if (saved_game_file_get_path_to_enclosing_directory(new_profile_index, full_path))
					{
						saved_game_file_remember_last_used_multiplayer_variant_directory(full_path);
					}

					success = TRUE;
				}
				else
				{
					error(_error_silent, "failed to save renamed profile to disk");
				}
			}
			else
			{
				error(_error_silent, "cannot save over default profiles; must rename and save-as a new profile");
			}
		}
		else
		{
			char full_path[MAXIMUM_FILENAME_LENGTH+1];

			playlist_profile_save(player_ui_globals.edit_profile_data.profile_index, &player_ui_globals.edit_profile_data.edit_profile.playlist_profile);

			if (saved_game_file_get_path_to_enclosing_directory(player_ui_globals.edit_profile_data.profile_index, full_path))
			{
				saved_game_file_remember_last_used_multiplayer_variant_directory(full_path);
			}

			success = TRUE;
		}
		break;
	default:
		error(_error_silent, "failed to save profile because we are not editing one");
		break;
	}

	clear_profile_edit_data();

	return success;
}

void player_ui_end_editing_profile(
	void)
{
	clear_profile_edit_data();

	return;
}

boolean player_ui_edit_profile_is_dirty(
	void)
{
	word original_flags;
	word edit_flags;
	boolean dirty = FALSE;

	if (player_ui_globals.edit_profile_data.profile_index != NONE)
	{
		switch (saved_game_file_get_type(player_ui_globals.edit_profile_data.profile_index))
		{
		case _saved_game_file_type_player_profile:
			original_flags = player_ui_globals.edit_profile_data.original_profile.player_profile.flags;
			edit_flags = player_ui_globals.edit_profile_data.edit_profile.player_profile.flags;
			player_ui_globals.edit_profile_data.original_profile.player_profile.flags = 0;
			player_ui_globals.edit_profile_data.edit_profile.player_profile.flags = 0;

			if (memcmp(
				&player_ui_globals.edit_profile_data.original_profile.player_profile,
				&player_ui_globals.edit_profile_data.edit_profile.player_profile,
				sizeof(struct player_profile)))
			{
				dirty = TRUE;
			}

			player_ui_globals.edit_profile_data.original_profile.player_profile.flags = original_flags;
			player_ui_globals.edit_profile_data.edit_profile.player_profile.flags = edit_flags;
			break;
		case _saved_game_file_type_play_list:
			original_flags = player_ui_globals.edit_profile_data.original_profile.playlist_profile.flags;
			edit_flags = player_ui_globals.edit_profile_data.edit_profile.playlist_profile.flags;
			player_ui_globals.edit_profile_data.original_profile.playlist_profile.flags = 0;
			player_ui_globals.edit_profile_data.edit_profile.playlist_profile.flags = 0;

			if (memcmp(
				&player_ui_globals.edit_profile_data.original_profile.playlist_profile,
				&player_ui_globals.edit_profile_data.edit_profile.playlist_profile,
				sizeof(struct game_variant)))
			{
				dirty = TRUE;
			}

			player_ui_globals.edit_profile_data.original_profile.playlist_profile.flags = original_flags;
			player_ui_globals.edit_profile_data.edit_profile.playlist_profile.flags = edit_flags;
			break;
		default:
			error(_error_silent, "unknown profile type being edited");
			break;
		}
	}

	return dirty;
}

void player_ui_activate_all_solo_levels(
	void)
{
	long level_index;

	for (level_index = 0; level_index < NUMBER_OF_SINGLE_PLAYER_LEVELS; level_index++)
	{
		long difficulty;

		for (difficulty = 0; difficulty < NUMBER_OF_GAME_DIFFICULTY_LEVELS; difficulty++)
		{
			SET_FLAG(player_ui_globals.player_data[0].profile.single_player_map_flags[level_index], difficulty, TRUE);
		}
	}

	if (player_ui_globals.player_data[0].profile_index != NONE)
	{
		player_profile_save(player_ui_globals.player_data[0].profile_index, &player_ui_globals.player_data[0].profile);
	}

	return;
}

static void hud_message_to_all(
	wchar_t const *string)
{
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);

	while (player = data_iterator_next(&iterator))
	{
		if (player->local_player_index != NONE)
		{
			hud_print_message(player->local_player_index, string);
		}
	}

	return;
}

void player0_look_invert_pitch(
	boolean invert)
{
	player_ui_globals.player_data[0].profile.controller_settings.invert_look = invert;

	if (player_ui_globals.player_data[0].profile_index != NONE)
	{
		wchar_t *string;
		long string_list_index = tag_loaded(UNICODE_STRING_LISTS_GROUP_TAG, TEMPORARY_STRINGS_TAG_NAME);

		if (string_list_index != NONE)
		{
			string = unicode_string_list_get_string(string_list_index, TEMPORARY_STRINGS_INVERT_LOOK_STRING_INDEX);
		}
		else
		{
			string = L"";
		}

		hud_message_to_all(string);
		player_profile_save(player_ui_globals.player_data[0].profile_index, &player_ui_globals.player_data[0].profile);
	}

	set_local_player_controls_from_player_profile(0);

	return;
}

boolean player0_look_pitch_is_inverted(
	void)
{
	return player_ui_globals.player_data[0].profile.controller_settings.invert_look;
}

boolean player0_joystick_set_is_normal(
	void)
{
	return player_ui_globals.player_data[0].profile.controller_settings.joystick_preset == _joystick_preset_standard ||
		player_ui_globals.player_data[0].profile.controller_settings.joystick_preset == _joystick_preset_south_paw;
}

/* ---------- private code */

static void generate_default_player_profile(
	struct player_profile *profile)
{
	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 873, profile);

	memset(profile, 0, sizeof(*profile));
	profile->primary_color_index = NONE;
	profile->controller_settings.button_preset = _button_preset_standard;
	profile->controller_settings.joystick_preset = _joystick_preset_standard;

	return;
}

static void set_local_player_controls_from_player_profile(
	short local_player_index)
{
	struct _controller_settings *controller_settings;
	short pitch_index;
	short yaw_index;
	struct game_input_preferences controls = {0};
	real const pitch_rate[MAXIMUM_LOOK_SENSITIVITY_SETTINGS] = { 40.f, 50.f, 60.f, 70.f, 80.f, 90.f, 100.f, 110.f, 120.f, 130.f };
	real const yaw_rate[MAXIMUM_LOOK_SENSITIVITY_SETTINGS] = { 80.f, 100.f, 120.f, 140.f, 160.f, 180.f, 200.f, 220.f, 240.f, 260.f };

	match_assert("c:\\halo\\SOURCE\\interface\\player_ui.c", 918, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	controller_settings = &player_ui_globals.player_data[local_player_index].profile.controller_settings;
	pitch_index = PIN(controller_settings->look_sensitivity - 1, 0, MAXIMUM_LOOK_SENSITIVITY_SETTINGS - 1);
	yaw_index = PIN(controller_settings->look_sensitivity - 1, 0, MAXIMUM_LOOK_SENSITIVITY_SETTINGS - 1);
	controls.pitch_rate = pitch_rate[pitch_index];
	controls.yaw_rate = yaw_rate[yaw_index];
	controls.joystick_controls = MIN(controller_settings->joystick_preset, _joystick_preset_legacy_south_paw);

	switch (controller_settings->button_preset)
	{
	case _button_preset_standard:
		controls.game_control_to_xbox_buttons[_button_jump] = _gamepad_analog_button_a;
		controls.game_control_to_xbox_buttons[_button_switch_grenade] = _gamepad_analog_button_black;
		controls.game_control_to_xbox_buttons[_button_action_reload] = _gamepad_analog_button_x;
		controls.game_control_to_xbox_buttons[_button_switch_weapon] = _gamepad_analog_button_y;
		controls.game_control_to_xbox_buttons[_button_melee_attack] = _gamepad_analog_button_b;
		controls.game_control_to_xbox_buttons[_button_flashlight] = _gamepad_analog_button_white;
		controls.game_control_to_xbox_buttons[_button_throw_grenade] = _gamepad_analog_button_left_trigger;
		controls.game_control_to_xbox_buttons[_button_fire] = _gamepad_analog_button_right_trigger;
		controls.game_control_to_xbox_buttons[_button_start] = _gamepad_binary_button_start;
		controls.game_control_to_xbox_buttons[_button_back] = _gamepad_binary_button_back;
		controls.game_control_to_xbox_buttons[_button_crouch] = _gamepad_binary_button_left_thumb;
		controls.game_control_to_xbox_buttons[_button_scope_zoom] = _gamepad_binary_button_right_thumb;
		break;
	case _button_preset_southpaw:
		controls.game_control_to_xbox_buttons[_button_jump] = _gamepad_analog_button_a;
		controls.game_control_to_xbox_buttons[_button_switch_grenade] = _gamepad_analog_button_black;
		controls.game_control_to_xbox_buttons[_button_action_reload] = _gamepad_analog_button_x;
		controls.game_control_to_xbox_buttons[_button_switch_weapon] = _gamepad_analog_button_y;
		controls.game_control_to_xbox_buttons[_button_melee_attack] = _gamepad_analog_button_b;
		controls.game_control_to_xbox_buttons[_button_flashlight] = _gamepad_analog_button_white;
		controls.game_control_to_xbox_buttons[_button_throw_grenade] = _gamepad_analog_button_right_trigger;
		controls.game_control_to_xbox_buttons[_button_fire] = _gamepad_analog_button_left_trigger;
		controls.game_control_to_xbox_buttons[_button_start] = _gamepad_binary_button_start;
		controls.game_control_to_xbox_buttons[_button_back] = _gamepad_binary_button_back;
		controls.game_control_to_xbox_buttons[_button_crouch] = _gamepad_binary_button_left_thumb;
		controls.game_control_to_xbox_buttons[_button_scope_zoom] = _gamepad_binary_button_right_thumb;
		break;
	case _button_preset_jumpy:
		controls.game_control_to_xbox_buttons[_button_jump] = _gamepad_analog_button_left_trigger;
		controls.game_control_to_xbox_buttons[_button_switch_grenade] = _gamepad_analog_button_black;
		controls.game_control_to_xbox_buttons[_button_action_reload] = _gamepad_analog_button_x;
		controls.game_control_to_xbox_buttons[_button_switch_weapon] = _gamepad_analog_button_y;
		controls.game_control_to_xbox_buttons[_button_melee_attack] = _gamepad_analog_button_b;
		controls.game_control_to_xbox_buttons[_button_flashlight] = _gamepad_analog_button_white;
		controls.game_control_to_xbox_buttons[_button_throw_grenade] = _gamepad_analog_button_a;
		controls.game_control_to_xbox_buttons[_button_fire] = _gamepad_analog_button_right_trigger;
		controls.game_control_to_xbox_buttons[_button_start] = _gamepad_binary_button_start;
		controls.game_control_to_xbox_buttons[_button_back] = _gamepad_binary_button_back;
		controls.game_control_to_xbox_buttons[_button_crouch] = _gamepad_binary_button_left_thumb;
		controls.game_control_to_xbox_buttons[_button_scope_zoom] = _gamepad_binary_button_right_thumb;
		break;
	case _button_preset_bumperjumper:
		controls.game_control_to_xbox_buttons[_button_jump] = _gamepad_analog_button_a;
		controls.game_control_to_xbox_buttons[_button_switch_grenade] = _gamepad_analog_button_black;
		controls.game_control_to_xbox_buttons[_button_action_reload] = _gamepad_analog_button_x;
		controls.game_control_to_xbox_buttons[_button_switch_weapon] = _gamepad_analog_button_y;
		controls.game_control_to_xbox_buttons[_button_melee_attack] = _gamepad_analog_button_left_trigger;
		controls.game_control_to_xbox_buttons[_button_flashlight] = _gamepad_analog_button_white;
		controls.game_control_to_xbox_buttons[_button_throw_grenade] = _gamepad_analog_button_b;
		controls.game_control_to_xbox_buttons[_button_fire] = _gamepad_analog_button_right_trigger;
		controls.game_control_to_xbox_buttons[_button_start] = _gamepad_binary_button_start;
		controls.game_control_to_xbox_buttons[_button_back] = _gamepad_binary_button_back;
		controls.game_control_to_xbox_buttons[_button_crouch] = _gamepad_binary_button_left_thumb;
		controls.game_control_to_xbox_buttons[_button_scope_zoom] = _gamepad_binary_button_right_thumb;
		break;
	case _button_preset_boxer:
		controls.game_control_to_xbox_buttons[_button_jump] = _gamepad_analog_button_a;
		controls.game_control_to_xbox_buttons[_button_switch_grenade] = _gamepad_analog_button_black;
		controls.game_control_to_xbox_buttons[_button_action_reload] = _gamepad_analog_button_x;
		controls.game_control_to_xbox_buttons[_button_switch_weapon] = _gamepad_analog_button_y;
		controls.game_control_to_xbox_buttons[_button_melee_attack] = _gamepad_binary_button_right_thumb;
		controls.game_control_to_xbox_buttons[_button_flashlight] = _gamepad_analog_button_white;
		controls.game_control_to_xbox_buttons[_button_throw_grenade] = _gamepad_analog_button_left_trigger;
		controls.game_control_to_xbox_buttons[_button_fire] = _gamepad_analog_button_right_trigger;
		controls.game_control_to_xbox_buttons[_button_start] = _gamepad_binary_button_start;
		controls.game_control_to_xbox_buttons[_button_back] = _gamepad_binary_button_back;
		controls.game_control_to_xbox_buttons[_button_crouch] = _gamepad_binary_button_left_thumb;
		controls.game_control_to_xbox_buttons[_button_scope_zoom] = _gamepad_analog_button_b;
		break;
	}

	controls.invert_look = controller_settings->invert_look;
	controls.invert_look_aircraft_control = controller_settings->flight_stick_aircraft_controls;

	if (player_ui_globals.single_player_controller_index[local_player_index] != NONE)
	{
		input_abstraction_update_local_player_preferences(player_ui_globals.single_player_controller_index[local_player_index], &controls);
	}
	else
	{
		input_abstraction_update_local_player_preferences(local_player_index, &controls);
	}

	return;
}

static void clear_profile_edit_data(
	void)
{
	player_ui_globals.edit_profile_data.profile_index = NONE;

	return;
}

static void reset_local_player_profile(
	short local_player_index)
{
	generate_default_player_profile(&player_ui_globals.player_data[local_player_index].profile);
	player_ui_globals.player_data[local_player_index].profile_index = NONE;
	player_ui_globals.single_player_controller_index[local_player_index] = NONE;

	return;
}
