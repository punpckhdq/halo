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

	if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\multiplayer_game_text", 0) == NONE)
	{
		error(_error_silent, "failed to load the multiplayer game text string list tag");
	}

	if (tag_load(BITMAP_GROUP_TAG, "ui\\shell\\bitmaps\\white", 0) == NONE)
	{
		error(_error_silent, "generic white texture bitmap");
	}

	switch (scenario->type)
	{
	case _scenario_type_solo:
		if (tag_load(UI_WIDGET_COLLECTION_TAG, "ui\\shell\\solo", 0) == NONE)
		{
			error(_error_silent, "failed to load the solo scenario ui_widget_collection tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\shell\\strings\\temp_strings", 0) == NONE)
		{
			error(_error_silent, "failed to load miscellaneous localized strings tag");
		}
		break;
	case _scenario_type_multiplayer:
		if (tag_load(UI_WIDGET_COLLECTION_TAG, "ui\\shell\\multiplayer", 0) == NONE)
		{
			error(_error_silent, "failed to load the multiplayer scenario ui_widget_collection tag");
		}
		break;
	case _scenario_type_main_menu:
		if (tag_load(UI_WIDGET_COLLECTION_TAG, "ui\\shell\\main_menu", 0) == NONE)
		{
			error(_error_silent, "failed to load the main menu scenario ui_widget_collection_tag");
		}

		if (tag_load(VIRTUAL_KEYBOARD_TAG, "ui\\english", 0) == NONE)
		{
			error(_error_silent, "failed to load the browser's virtual keyboard tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\random_player_names", 0) == NONE)
		{
			error(_error_silent, "failed to load random player names string list tag");
		}

		if (tag_load(MULTIPLAYER_SCENARIO_DESCRIPTION_TAG, "ui\\multiplayer_scenarios", 0) == NONE)
		{
			error(_error_silent, "failed to load the multiplayer scenario description tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\saved_game_file_strings", 0) == NONE)
		{
			error(_error_silent, "failed to load the default saved game filename string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\default_multiplayer_game_setting_names", 0) == NONE)
		{
			error(_error_silent, "failed to load the default playlist profile names string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\shell\\strings\\game_variant_descriptions", 0) == NONE)
		{
			error(_error_silent, "failed to load the multiplayer variant description string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\shell\\main_menu\\player_profiles_select\\difficulty_names", 0) == NONE)
		{
			error(_error_silent, "failed to load the game difficulty name string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\shell\\strings\\default_player_profile_names", 0) == NONE)
		{
			error(_error_silent, "failed to load the default player profile names string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\shell\\main_menu\\player_profiles_select\\button_set_long_descriptions", 0) == NONE)
		{
			error(_error_silent, "failed to load the player profile button set long descriptions string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\shell\\main_menu\\player_profiles_select\\button_set_short_descriptions", 0) == NONE)
		{
			error(_error_silent, "failed to load the player profile short descriptions string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\shell\\main_menu\\player_profiles_select\\joystick_set_defaults_descriptions", 0) == NONE)
		{
			error(_error_silent, "failed to load the default player profile joystick set descriptions string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\shell\\main_menu\\player_profiles_select\\joystick_set_short_descriptions", 0) == NONE)
		{
			error(_error_silent, "failed to load the player profile joystick set short descriptions string list tag");
		}

		if (tag_load(UNICODE_STRING_LISTS_GROUP_TAG, "ui\\shell\\main_menu\\player_profiles_select\\profile_description_labels", 0) == NONE)
		{
			error(_error_silent, "failed to load the player profile description labels string list tag");
		}

		if (tag_load(SOUND_DEFINITION_TAG, "sound\\sfx\\ui\\cursor", 0) == NONE)
		{
			error(_error_silent, "failed to load ui cursor sound tag");
		}

		if (tag_load(SOUND_DEFINITION_TAG, "sound\\sfx\\ui\\forward", 0) == NONE)
		{
			error(_error_silent, "failed to load ui forward sound tag");
		}

		if (tag_load(SOUND_DEFINITION_TAG, "sound\\sfx\\ui\\back", 0) == NONE)
		{
			error(_error_silent, "failed to load ui back sound tag");
		}

		if (tag_load(SOUND_DEFINITION_TAG, "sound\\sfx\\ui\\flag_failure", 0) == NONE)
		{
			error(_error_silent, "failed to load ui failure sound tag");
		}

		if (tag_load(LOOPING_SOUND_DEFINITION_TAG, "sound\\music\\title1\\title1", 0) == NONE)
		{
			error(_error_silent, "failed to load main menu title music");
		}
		break;
	default:
		match_vhalt("c:\\halo\\SOURCE\\interface\\ui_widget_group.c", 506, "unknown scenario type");
	}

	return;
}

/* ---------- private code */
