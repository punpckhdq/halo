/*
UI_WIDGET_GROUP.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ui_widget_group.h"
#include "bitmap_group.h"
#include "text_group.h"
#include "sound_definitions.h"
#include "virtual_keyboard.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "multiplayer_scenario_description.h"
#include "game_engine.h"
#include "player_ui.h"
#include "saved_game_files.h"
#include "network_game_ui.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void ui_load_tags_for_scenario(
	long scenario_index)
{
	struct scenario *scenario = scenario_definition_get(scenario_index);

	if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, MULTIPLAYER_GAME_TEXT_TAG_NAME, 0) == NONE)
	{
		error(_error_silent, "failed to load the multiplayer game text string list tag");
	}

	if (tag_load(BITMAP_GROUP_TAG, GENERIC_WHITE_BITMAP_TAG_NAME, 0) == NONE)
	{
		error(_error_silent, "generic white texture bitmap");
	}

	switch (scenario->type)
	{
	case _scenario_type_solo:
		if (tag_load(UI_WIDGET_COLLECTION_TAG, SOLO_WIDGET_COLLECTION_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the solo scenario ui_widget_collection tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, TEMPORARY_STRINGS_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load miscellaneous localized strings tag");
		}
		break;
	case _scenario_type_multiplayer:
		if (tag_load(UI_WIDGET_COLLECTION_TAG, MULTIPLAYER_WIDGET_COLLECTION_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the multiplayer scenario ui_widget_collection tag");
		}
		break;
	case _scenario_type_main_menu:
		if (tag_load(UI_WIDGET_COLLECTION_TAG, MAIN_MENU_WIDGET_COLLECTION_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the main menu scenario ui_widget_collection_tag");
		}

		if (tag_load(VIRTUAL_KEYBOARD_TAG, VIRTUAL_KEYBOARD_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the browser's virtual keyboard tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, RANDOM_PLAYER_NAMES_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load random player names string list tag");
		}

		if (tag_load(MULTIPLAYER_SCENARIO_DESCRIPTION_TAG, MULTIPLAYER_SCENARIO_DESCRIPTION_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the multiplayer scenario description tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, SAVED_GAME_FILE_STRINGS_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the default saved game filename string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, DEFAULT_MULTIPLAYER_GAME_SETTING_NAMES_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the default playlist profile names string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, GAME_VARIANT_DESCRIPTIONS_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the multiplayer variant description string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, DIFFICULTY_NAMES_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the game difficulty name string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, DEFAULT_PLAYER_PROFILE_NAMES_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the default player profile names string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, BUTTON_SET_LONG_DESCRIPTIONS_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the player profile button set long descriptions string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, BUTTON_SET_SHORT_DESCRIPTIONS_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the player profile short descriptions string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, JOYSTICK_SET_DEFAULTS_DESCRIPTIONS_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the default player profile joystick set descriptions string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, JOYSTICK_SET_SHORT_DESCRIPTIONS_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the player profile joystick set short descriptions string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, PROFILE_DESCRIPTION_LABELS_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load the player profile description labels string list tag");
		}

		if (tag_load(SOUND_DEFINITION_TAG, UI_CURSOR_SOUND_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load ui cursor sound tag");
		}

		if (tag_load(SOUND_DEFINITION_TAG, UI_FORWARD_SOUND_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load ui forward sound tag");
		}

		if (tag_load(SOUND_DEFINITION_TAG, UI_BACK_SOUND_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load ui back sound tag");
		}

		if (tag_load(SOUND_DEFINITION_TAG, UI_FAILURE_SOUND_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load ui failure sound tag");
		}

		if (tag_load(LOOPING_SOUND_DEFINITION_TAG, MAIN_MENU_MUSIC_TAG_NAME, 0) == NONE)
		{
			error(_error_silent, "failed to load main menu title music");
		}
		break;
	default:
		match_vhalt("c:\\halo\\SOURCE\\interface\\ui_widget_group.c", MATCH_LINE_NTSC(501, 506), "unknown scenario type");
	}

	return;
}
