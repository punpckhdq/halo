/*
UI_WIDGET_GAME_DATA_INPUT_FUNCTIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ui_widget_game_data_input_functions.h"
#include "ui_widget.h"
#include "hud.h"
#include "player_ui.h"
#include "hud_messaging.h"
#include "marketing_and_strategic_business_development.h"
#include "network_game_globals.h"
#include "units.h"
#include "network_game_manager.h"
#include "input.h"
#include "network_messages.h"
#include "text_group.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "network_client_manager.h"
#include "game_engine_list.h"
#include "build_number.h"
#include "playlist_profile.h"
#include "ui_widget_event_handler_functions.h"
#include "players.h"
#include "bungie_net/network/transport.h"

/* ---------- constants */

enum
{
	_controller_inactive_bitmap = 0,
	_controller_active_bitmap,
	_controller_unknown_bitmap,
	_controller_player1_active_bitmap,
	_controller_player1_blue_active_bitmap,
	_controller_player1_red_active_bitmap,
	_controller_player2_active_bitmap,
	_controller_player2_blue_active_bitmap,
	_controller_player2_red_active_bitmap,
	_controller_player3_active_bitmap,
	_controller_player3_blue_active_bitmap,
	_controller_player3_red_active_bitmap,
	_controller_player4_active_bitmap,
	_controller_player4_blue_active_bitmap,
	_controller_player4_red_active_bitmap,
	NUMBER_OF_CONTROLLER_SPRITE_FRAMES
};

enum
{
	_console_inactive_bitmap = 0,
	_console_active_bitmap
};

enum
{
	_multiplayer_game_type_ctf_bitmap = 0,
	_multiplayer_game_type_koth_bitmap,
	_multiplayer_game_type_slayer_bitmap,
	_multiplayer_game_type_oddball_bitmap,
	_multiplayer_game_type_race_bitmap,
	_multiplayer_game_type_unknown_bitmap,
	NUMBER_OF_MULTIPLAYER_GAME_TYPE_SPRITE_FRAMES
};

enum
{
	_mp_map_beavercreek = 0,
	_mp_map_sidewinder,
	_mp_map_damnation,
	_mp_map_ratrace,
	_mp_map_prisoner,
	_mp_map_hangemhigh,
	_mp_map_chillout,
	_mp_map_carousel,
	_mp_map_boardingaction,
	_mp_map_bloodgulch,
	_mp_map_wizard,
	_mp_map_putput,
	_mp_map_longest,
	_mp_map_unknown,
	NUMBER_OF_MULTIPLAYER_MAP_TAG_INDICES,

	_mp_map_name_empty_string = NUMBER_OF_MULTIPLAYER_MAP_TAG_INDICES
};

enum
{
	_description_ctf_ffa = 0,
	_description_ctf_teams,
	_description_slayer_ffa,
	_description_slayer_teams,
	_description_oddball_ffa,
	_description_oddball_teams,
	_description_koth_ffa,
	_description_koth_teams,
	_description_racing_ffa,
	_description_racing_teams,
	FIRST_DEFAULT_VARIANT_DESCRIPTION
};

enum
{
	MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH = 256
};

enum
{
	MAXIMUM_PLAYER_PROFILE_DESCRIPTION_LENGTH = 256
};

enum
{
	AUTO_START_STRING_LENGTH = 16
};

enum
{
	_string_label_game_start = 0,
	_string_label_loading
};

enum
{
	_string_looking_for_servers = 0,
	_string_still_looking_for_servers,
	_string_looking_for_servers_empty_string,
	NUMBER_OF_LOOKING_FOR_SERVER_TEXTBOX_STRINGS
};

enum
{
	NUMBER_OF_PLAYERS_STRING_LENGTH = 4,
	SCORE_LIMIT_STRING_LENGTH = 4
};

enum
{
	NUMBER_OF_PLAYERS_STRING_SIZE = 4
};

enum
{
	_spinner_list_3wide_left = 0,		/* fake name */
	_spinner_list_3wide_center,			/* fake name */
	_spinner_list_3wide_right,			/* fake name */
	NUMBER_OF_SPINNER_LIST_3WIDE_ITEMS	/* fake name */
};

enum
{
	_network_team_bitmap_none = 0,	/* fake name */
	_network_team_bitmap_blue,		/* fake name */
	_network_team_bitmap_red,		/* fake name */
	NUMBER_OF_NETWORK_TEAM_BITMAPS	/* fake name */
};

enum
{
	_splitscreen_team_bitmap_none = 0,	/* fake name */
	_splitscreen_team_bitmap_red,		/* fake name */
	_splitscreen_team_bitmap_blue,		/* fake name */
	NUMBER_OF_SPLITSCREEN_TEAM_BITMAPS	/* fake name */
};

enum
{
	_mp_pregame_header_teams_bitmap = 0,	/* fake name */
	_mp_pregame_header_no_teams_bitmap,		/* fake name */
	NUMBER_OF_MP_PREGAME_HEADER_BITMAPS		/* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void widget_function_null(struct widget_instance *widget);
static void settings_menu_update_extended_description(struct widget_instance *widget);
static void playlist_settings_menu_update_extended_description(struct widget_instance *widget);
static void playlist_gametype_select_menu_update_extended_description(struct widget_instance *widget);
static void multiplayer_type_menu_update_extended_description(struct widget_instance *widget);
static void solo_level_select_list_update_displayed_items(struct widget_instance *widget);
static void difficulty_select_menu_update_extended_description(struct widget_instance *widget);
static void set_textbox_to_build_number(struct widget_instance *widget);
static void server_list_menu_update(struct widget_instance *widget);
static void network_pregame_status_screen_update(struct widget_instance *widget);
static void splitscreen_pregame_status_screen_update(struct widget_instance *widget);
static void netgame_prejoin_players(struct widget_instance *widget);
static void mutliplayer_settings_select_list_update_displayed_items(struct widget_instance *widget);
static void player_profile_3wide_list_update(struct widget_instance *widget);
static void player_profile_edit_select_menu_update_extended_description(struct widget_instance *widget);
static void player_profile_1wide_list_update(struct widget_instance *widget);
static void game_options_menu_update_text_desc(struct widget_instance *widget);
static void solo_game_objective_text(struct widget_instance *widget);
static void player_profile_color_picker_update(struct widget_instance *widget);
static void game_options_menu_update_pic_desc(struct widget_instance *widget);
static void main_menu_animation_fakery(struct widget_instance *widget);
static void mp_level_select_list_update_displayed_items(struct widget_instance *widget);
static void get_active_player_profile_display_name(struct widget_instance *widget);
static void get_editable_player_profile_display_name(struct widget_instance *widget);
static void get_editable_playlist_profile_display_name(struct widget_instance *widget);
static void get_active_player_profile_color_index(struct widget_instance *widget);
static void multiplayer_game_set_text_box_for_map_name(struct widget_instance *widget);
static void multiplayer_game_set_text_box_for_game_ruleset(struct widget_instance *widget);
static void multiplayer_game_set_text_box_for_teams_noteams(struct widget_instance *widget);
static void multiplayer_game_set_text_box_for_score_limit(struct widget_instance *widget);
static void multiplayer_game_set_text_box_for_score_limit_type(struct widget_instance *widget);
static void multiplayer_game_set_bitmap_for_map(struct widget_instance *widget);
static void multiplayer_game_set_bitmap_for_ruleset(struct widget_instance *widget);
static void multiplayer_game_set_text_box_for_number_of_players(struct widget_instance *widget);
static void multiplayer_edit_profile_set_ruleset_textbox_string_index(struct widget_instance *widget);
static void system_link_status_check(struct widget_instance *widget);
static void multiplayer_game_directions(struct widget_instance *widget);
static void teams_no_teams_mp_game_bitmap_update(struct widget_instance *widget);
static void warn_if_difficulty_will_nuke_saved_game(struct widget_instance *widget);
static void dim_if_no_system_link_cable(struct widget_instance *widget);
static void spinner_list_3wide_determine_displayed_item_indices(struct widget_instance *widget, long *displayed_item_index);
static void player_profile_update_cache_for_nwide_list(long *profile_indices, long profile_count);
static void variant_profile_update_cache_for_nwide_list(long *profile_indices, long profile_count);
static int list_indices_sort_proc(void const *a, void const *b);
static long filter_invalid_list_indices(long *list_indices, long count);

/* ---------- globals */

static void (*game_data_input_function_table[NUMBER_OF_GAME_DATA_INPUT_FUNCTIONS])(struct widget_instance *widget) = /* fake name */
{
	widget_function_null,
	settings_menu_update_extended_description,
	widget_function_null,
	playlist_settings_menu_update_extended_description,
	playlist_gametype_select_menu_update_extended_description,
	multiplayer_type_menu_update_extended_description,
	solo_level_select_list_update_displayed_items,
	difficulty_select_menu_update_extended_description,
	set_textbox_to_build_number,
	server_list_menu_update,
	network_pregame_status_screen_update,
	splitscreen_pregame_status_screen_update,
	netgame_prejoin_players,
	mutliplayer_settings_select_list_update_displayed_items,
	player_profile_3wide_list_update,
	player_profile_edit_select_menu_update_extended_description,
	player_profile_1wide_list_update,
	game_options_menu_update_text_desc,
	solo_game_objective_text,
	player_profile_color_picker_update,
	game_options_menu_update_pic_desc,
	main_menu_animation_fakery,
	mp_level_select_list_update_displayed_items,
	get_active_player_profile_display_name,
	get_editable_player_profile_display_name,
	get_editable_playlist_profile_display_name,
	get_active_player_profile_color_index,
	multiplayer_game_set_text_box_for_map_name,
	multiplayer_game_set_text_box_for_game_ruleset,
	multiplayer_game_set_text_box_for_teams_noteams,
	multiplayer_game_set_text_box_for_score_limit,
	multiplayer_game_set_text_box_for_score_limit_type,
	multiplayer_game_set_bitmap_for_map,
	multiplayer_game_set_bitmap_for_ruleset,
	multiplayer_game_set_text_box_for_number_of_players,
	multiplayer_edit_profile_set_ruleset_textbox_string_index,
	system_link_status_check,
	multiplayer_game_directions,
	teams_no_teams_mp_game_bitmap_update,
	warn_if_difficulty_will_nuke_saved_game,
	dim_if_no_system_link_cable,
};

struct _cached_player_profile cached_player_profile[NUMBER_OF_CACHED_PROFILES];
struct _cached_variant_profile cached_variant_profile[NUMBER_OF_CACHED_PROFILES];

/* ---------- public code */

void ui_widget_game_data_function_invoke(
	struct widget_instance *widget,
	short function)
{
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 266, widget);

	if (function >= 0 && function < NUMBEROF(game_data_input_function_table))
	{
		game_data_input_function_table[function](widget);
	}
	else
	{
		error(_error_silent, "invalid game_data_input_reference_function");
	}

	return;
}

static void widget_function_null(
	struct widget_instance *widget)
{
	return;
}

static void settings_menu_update_extended_description(
	struct widget_instance *widget)
{
	short index;
	struct widget_instance *child;
	struct widget_instance *picture;
	struct widget_instance *text;
	struct ui_widget_definition *definition = ui_widget_definition_get(widget->list.extended_description_widget->tag_index);

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 300, widget && widget->focused_child && widget->list.extended_description_widget, "invalid widget trying to update its extended list description");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 301, definition->child_widgets.count == 2, "this doesn't look like the settings select widget to me");
	picture = widget->list.extended_description_widget->first_child;
	text = picture->next;
	index = 0;
	child = widget->first_child;

	while (child && child != widget->focused_child)
	{
		child = child->next;
		index++;
	}

	if (index != NONE)
	{
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 312, picture->type == _ui_widget_type_container, "expected a container widget for the settings select list extended description pic");
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 313, text->type == _ui_widget_type_text_box, "expected a text box widget for the settings select list extended description text");
		picture->bitmap_frame_index = index;
		text->text_box.string_list_index = index;
	}

	return;
}

static void playlist_settings_menu_update_extended_description(
	struct widget_instance *widget)
{
	short index;
	struct widget_instance *child;
	struct widget_instance *text;
	struct widget_instance *picture;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 333, widget && widget->focused_child && widget->list.extended_description_widget, "invalid widget trying to update its extended list description");
	index = 0;
	child = widget->first_child;

	while (child && child != widget->focused_child)
	{
		child = child->next;
		index++;
	}

	if (index != NONE)
	{
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 347, ui_widget_definition_get(widget->list.extended_description_widget->tag_index)->child_widgets.count == 2, "expected a container widget w/ 2 children for the playlist settings list extended description");
		text = widget->list.extended_description_widget->first_child;
		picture = text->next;
		text->text_box.string_list_index = index;
		picture->bitmap_frame_index = index;
	}

	return;
}

static void playlist_gametype_select_menu_update_extended_description(
	struct widget_instance *widget)
{
	short index;
	struct widget_instance *child;
	struct widget_instance *text;
	struct widget_instance *picture;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 371, widget && widget->focused_child && widget->list.extended_description_widget, "invalid widget trying to update its extended list description");
	index = 0;
	child = widget->first_child;

	while (child && child != widget->focused_child)
	{
		child = child->next;
		index++;
	}

	if (index != NONE)
	{
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 385, ui_widget_definition_get(widget->list.extended_description_widget->tag_index)->child_widgets.count == 2, "expected a container widget w/ 2 children for the playlist settings list extended description");
		text = widget->list.extended_description_widget->first_child;
		picture = text->next;
		text->text_box.string_list_index = index;
		picture->bitmap_frame_index = index;
	}

	return;
}

static void multiplayer_type_menu_update_extended_description(
	struct widget_instance *widget)
{
	short index;
	struct widget_instance *child;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 422, widget && widget->focused_child && widget->list.extended_description_widget && widget->list.extended_description_widget->first_child && widget->list.extended_description_widget->first_child->next, "invalid widget trying to update its extended list description");
	index = 0;
	child = widget->first_child;

	while (child && child != widget->focused_child)
	{
		child = child->next;
		index++;
	}

	if (index != NONE)
	{
		widget->list.extended_description_widget->first_child->bitmap_frame_index = index;
		widget->list.extended_description_widget->first_child->next->text_box.string_list_index = index;
	}

	return;
}

static void solo_level_select_list_update_displayed_items(
	struct widget_instance *widget)
{
	struct player_profile profile;
	short last_level_completed;
	short difficulty;
	long displayed_item_index[NUMBER_OF_SPINNER_LIST_3WIDE_ITEMS];
	struct ui_widget_definition *definition;
	long i;

	player_ui_get_active_player_profile(0, &profile);
	player_profile_get_highest_completed_solo_level(&profile, &last_level_completed, &difficulty);
	definition = ui_widget_definition_get(widget->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 480, definition->type == _ui_widget_type_spinner_list, "expected a spinner list for 'level select' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 481, definition->child_widgets.count == 3, "expected 3 children (list items) for 'level select' widget");
	spinner_list_3wide_determine_displayed_item_indices(widget, displayed_item_index);

	for (i = 0; i < NUMBER_OF_SPINNER_LIST_3WIDE_ITEMS && displayed_item_index[i] != NONE; i++)
	{
		struct widget_instance *list_item = widget_instance_get_nth_child(widget, i);
		struct widget_instance *map_name = list_item->first_child;
		struct widget_instance *map_picture = map_name->next;
		struct widget_instance *map_description = map_picture->next;
		struct widget_instance *normal_difficulty = map_description->next;
		struct widget_instance *hard_difficulty = normal_difficulty->next;
		struct widget_instance *impossible_difficulty = hard_difficulty->next;

		definition = ui_widget_definition_get(list_item->tag_index);







		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 502, definition->child_widgets.count, "expected 3 children in solo level list item (name, pic, desc)");
		definition = ui_widget_definition_get(map_name->tag_index);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 504, definition->type == _ui_widget_type_text_box, "expected a text box widget for the list item's first child (map name)");
		definition = ui_widget_definition_get(map_picture->tag_index);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 506, definition->type == _ui_widget_type_container, "expected a container widget for the list item's second child (map pic)");
		definition = ui_widget_definition_get(map_description->tag_index);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 508, definition->type == _ui_widget_type_text_box, "expected a text box widget for the list item's third child (map description)");
		normal_difficulty->bitmap_frame_index = _game_difficulty_level_normal;
		hard_difficulty->bitmap_frame_index = _game_difficulty_level_hard;
		impossible_difficulty->bitmap_frame_index = _game_difficulty_level_impossible;




		match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 516, (displayed_item_index[i]>=0) && (displayed_item_index[i]<NUMBER_OF_SINGLE_PLAYER_LEVELS));

		if (single_player_level_data[displayed_item_index[i]].level_completed[_game_difficulty_level_easy] ||
			single_player_level_data[displayed_item_index[i]].level_completed[_game_difficulty_level_normal] ||
			single_player_level_data[displayed_item_index[i]].level_completed[_game_difficulty_level_hard] ||
			single_player_level_data[displayed_item_index[i]].level_completed[_game_difficulty_level_impossible])
		{
			map_name->text_box.string_list_index = (short)displayed_item_index[i];
			map_picture->bitmap_frame_index = (short)displayed_item_index[i];
			map_description->text_box.string_list_index = (short)displayed_item_index[i];

			if (persistant_game_data_info.valid == TRUE && displayed_item_index[i] == persistant_game_data_info.map_index)
			{
				map_description->text_box.string_list_index = 11;
			}

			normal_difficulty->visible = single_player_level_data[displayed_item_index[i]].level_completed[_game_difficulty_level_normal];
			hard_difficulty->visible = single_player_level_data[displayed_item_index[i]].level_completed[_game_difficulty_level_hard];
			impossible_difficulty->visible = single_player_level_data[displayed_item_index[i]].level_completed[_game_difficulty_level_impossible];
		}
		else
		{
			map_name->text_box.string_list_index = 10;
			map_picture->bitmap_frame_index = 10;
			map_description->text_box.string_list_index = 10;
			normal_difficulty->visible = FALSE;
			hard_difficulty->visible = FALSE;
			impossible_difficulty->visible = FALSE;
		}
	}

	return;
}

static void difficulty_select_menu_update_extended_description(
	struct widget_instance *widget)
{
	short index;
	struct widget_instance *child;
	struct widget_instance *picture;
	struct widget_instance *text;
	struct ui_widget_definition *definition = ui_widget_definition_get(widget->list.extended_description_widget->tag_index);

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 567, widget && widget->focused_child && widget->list.extended_description_widget, "invalid widget trying to update its extended list description");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 568, definition->child_widgets.count == 2, "this doesn't look like the difficulty select widget to me");
	picture = widget->list.extended_description_widget->first_child;
	text = picture->next;
	index = 0;
	child = widget->first_child;

	while (child && child != widget->focused_child)
	{
		child = child->next;
		index++;
	}

	if (index != NONE)
	{
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 579, picture->type == _ui_widget_type_container, "expected a container widget for the difficulty list extended description pic");
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 580, text->type == _ui_widget_type_text_box, "expected a text box widget for the difficulty list extended description text");
		picture->bitmap_frame_index = index;
		text->text_box.string_list_index = index;
	}

	return;
}

static void set_textbox_to_build_number(
	struct widget_instance *widget)
{
	static wchar_t build_number[64];

	if (!build_number[0])
	{
		ascii_to_wide(BUILD_STRING, build_number, sizeof(build_number));
	}

	if (!widget->text_box.string)
	{
		widget->text_box.string = ui_widget_realloc(widget->text_box.string, sizeof(build_number), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(606));

		if (widget->text_box.string)
		{
			memset(widget->text_box.string, 0, sizeof(build_number));
		}
	}

	if (widget->text_box.string)
	{
		ustrncpy(widget->text_box.string, build_number, NUMBEROF(build_number) - 1);
		widget->text_box.string[NUMBEROF(build_number) - 1] = L'\0';
	}

	return;
}

static void server_list_menu_update(
	struct widget_instance *widget)
{
	static struct advertised_game_data *advertised_games[MAXIMUM_NETWORK_ADVERTISED_GAMES];
	struct network_game_client *client = global_network_game_client_get();
	long advertised_game_count = 0;

	memset(advertised_games, 0, sizeof(advertised_games));

	if (client)
	{
		struct widget_instance *child;
		long game_index;
		unsigned long elapsed_milliseconds;
		struct widget_instance *gametype_picture;
		struct widget_instance *map_picture;
		struct widget_instance *text_container;
		struct widget_instance *status_text;
		struct widget_instance *open_closed_text;
		struct widget_instance *map_name_text;
		struct widget_instance *ruleset_text;
		struct widget_instance *teams_text;
		struct widget_instance *number_of_players_text;
		struct widget_instance *score_limit_text;
		struct widget_instance *score_limit_type_text;
		struct ui_widget_definition *definition = ui_widget_definition_get(widget->tag_index);
		struct advertised_game_data *available_games = network_game_client_get_available_games(client);

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 663, definition->type == _ui_widget_type_column_list && definition->child_widgets.count == MAXIMUM_NETWORK_ADVERTISED_GAMES, "this doesn't look like the net game server list widget");

		for (game_index = 0; game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES; game_index++)
		{
			struct advertised_game_data *game = &available_games[game_index];

			if (network_game_client_advertised_game_is_valid(game) && game->platform == _game_platform_xbox && game->open)
			{
				advertised_games[advertised_game_count++] = game;
			}
		}

		for (game_index = 0; game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES; game_index++)
		{
			struct advertised_game_data *game = &available_games[game_index];

			if (network_game_client_advertised_game_is_valid(game) && game->platform == _game_platform_xbox && !game->open)
			{
				advertised_games[advertised_game_count++] = game;
			}
		}

		widget->list.items = advertised_games;
		widget->list.item_count = (word)advertised_game_count;
		widget->list.item_index = MIN(widget->list.item_index, advertised_game_count - 1);

		for (child = widget->first_child, game_index = 0; child && game_index < advertised_game_count; child = child->next, game_index++)
		{
			child->text_box.string = ui_widget_realloc(child->text_box.string, 32 * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(709));

			if (child->text_box.string)
			{
				if (advertised_games[game_index]->open == TRUE)
				{
					ustrncpy(child->text_box.string, advertised_games[game_index]->name, 31);
				}
				else
				{
					long string_list_tag_index = tag_loaded(UNICODE_STRING_LISTS_GROUP_TAG, MULTIPLAYER_GAME_TEXT_TAG_NAME);
					wchar_t *string = unicode_string_list_get_string(string_list_tag_index, _string_closed_game_prefix);

					usnprintf(
						child->text_box.string,
						31,
						L"%s %s",
						string,
						advertised_games[game_index]->name);
				}

				child->text_box.string[31] = L'\0';
			}
		}

		if (advertised_game_count > 0 && widget->list.item_index < 0)
		{
			widget->list.item_index = 0;
		}

		elapsed_milliseconds = system_milliseconds() - widget->creation_time;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 759, ui_widget_definition_get(widget->list.extended_description_widget->tag_index)->child_widgets.count == 5, "this doesn't look like the server list extended description widget");
		gametype_picture = widget->list.extended_description_widget->first_child;
		map_picture = gametype_picture->next;
		text_container = map_picture->next;
		status_text = text_container->next;
		open_closed_text = text_container->first_child;


		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 767, open_closed_text && open_closed_text->type == _ui_widget_type_text_box, "expected 'open/closed game' textbox");
		map_name_text = open_closed_text->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 769, map_name_text && map_name_text->type == _ui_widget_type_text_box, "expected 'map name' textbox");
		ruleset_text = map_name_text->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 771, ruleset_text && ruleset_text->type == _ui_widget_type_text_box, "expected 'ruleset' textbox");
		teams_text = ruleset_text->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 773, teams_text && teams_text->type == _ui_widget_type_text_box, "expected 'teams on/off' textbox");
		number_of_players_text = teams_text->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 775, number_of_players_text && number_of_players_text->type == _ui_widget_type_text_box, "expected 'number of players' textbox");
		score_limit_text = number_of_players_text->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 777, score_limit_text && score_limit_text->type == _ui_widget_type_text_box, "expected 'score limit' textbox");
		score_limit_type_text = score_limit_text->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 779, score_limit_type_text && score_limit_type_text->type == _ui_widget_type_text_box, "expected 'score limit type' textbox");

		if (widget->list.item_index >= 0)
		{
			struct advertised_game_data *game = advertised_games[widget->list.item_index];

			switch (game->game_engine)
			{
			case _game_engine_ctf:
				gametype_picture->bitmap_frame_index = _multiplayer_game_type_ctf_bitmap;
				break;
			case _game_engine_slayer:
				gametype_picture->bitmap_frame_index = _multiplayer_game_type_slayer_bitmap;
				break;
			case _game_engine_oddball:
				gametype_picture->bitmap_frame_index = _multiplayer_game_type_oddball_bitmap;
				break;
			case _game_engine_king:
				gametype_picture->bitmap_frame_index = _multiplayer_game_type_koth_bitmap;
				break;
			case _game_engine_race:
				gametype_picture->bitmap_frame_index = _multiplayer_game_type_race_bitmap;
				break;
			default:
				gametype_picture->bitmap_frame_index = _multiplayer_game_type_unknown_bitmap;
				break;
			}

			if (strstr(game->map.name, "beavercreek"))
			{
				map_picture->bitmap_frame_index = _mp_map_beavercreek;
			}
			else if (strstr(game->map.name, "sidewinder"))
			{
				map_picture->bitmap_frame_index = _mp_map_sidewinder;
			}
			else if (strstr(game->map.name, "damnation"))
			{
				map_picture->bitmap_frame_index = _mp_map_damnation;
			}
			else if (strstr(game->map.name, "ratrace"))
			{
				map_picture->bitmap_frame_index = _mp_map_ratrace;
			}
			else if (strstr(game->map.name, "prisoner"))
			{
				map_picture->bitmap_frame_index = _mp_map_prisoner;
			}
			else if (strstr(game->map.name, "hangemhigh"))
			{
				map_picture->bitmap_frame_index = _mp_map_hangemhigh;
			}
			else if (strstr(game->map.name, "chillout"))
			{
				map_picture->bitmap_frame_index = _mp_map_chillout;
			}
			else if (strstr(game->map.name, "carousel"))
			{
				map_picture->bitmap_frame_index = _mp_map_carousel;
			}
			else if (strstr(game->map.name, "boardingaction"))
			{
				map_picture->bitmap_frame_index = _mp_map_boardingaction;
			}
			else if (strstr(game->map.name, "bloodgulch"))
			{
				map_picture->bitmap_frame_index = _mp_map_bloodgulch;
			}
			else if (strstr(game->map.name, "wizard"))
			{
				map_picture->bitmap_frame_index = _mp_map_wizard;
			}
			else if (strstr(game->map.name, "putput"))
			{
				map_picture->bitmap_frame_index = _mp_map_putput;
			}
			else if (strstr(game->map.name, "longest"))
			{
				map_picture->bitmap_frame_index = _mp_map_longest;
			}
			else
			{
				map_picture->bitmap_frame_index = _mp_map_unknown;
			}

			if (game->open == TRUE)
			{
				open_closed_text->text_box.string_list_index = _string_game_accepting_players;
			}
			else
			{
				open_closed_text->text_box.string_list_index = _string_game_in_progress;
			}

			map_name_text->text_box.string_list_index = map_picture->bitmap_frame_index;

			switch (game->game_engine)
			{
			case _game_engine_ctf:
				ruleset_text->text_box.string_list_index = _string_capture_the_flag;
				break;
			case _game_engine_slayer:
				ruleset_text->text_box.string_list_index = _string_slayer;
				break;
			case _game_engine_oddball:
				ruleset_text->text_box.string_list_index = _string_oddball;
				break;
			case _game_engine_king:
				ruleset_text->text_box.string_list_index = _string_king_of_the_hill;
				break;
			case _game_engine_race:
				ruleset_text->text_box.string_list_index = _string_race;
				break;
			default:
				ruleset_text->text_box.string_list_index = _string_unknown_game_type;
				break;
			}

			if (game->teams_enabled == TRUE)
			{
				teams_text->text_box.string_list_index = _string_team_game;
			}
			else
			{
				teams_text->text_box.string_list_index = _string_free_for_all;
			}

			number_of_players_text->text_box.string = ui_widget_realloc(number_of_players_text->text_box.string, NUMBER_OF_PLAYERS_STRING_LENGTH * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(868));

			if (number_of_players_text->text_box.string)
			{
				usnprintf(number_of_players_text->text_box.string, NUMBER_OF_PLAYERS_STRING_LENGTH - 1, L"%d", game->current_number_of_players);
				number_of_players_text->text_box.string[NUMBER_OF_PLAYERS_STRING_LENGTH - 1] = L'\0';
			}

			score_limit_text->text_box.string = ui_widget_realloc(score_limit_text->text_box.string, SCORE_LIMIT_STRING_LENGTH * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(873));

			if (score_limit_text->text_box.string)
			{
				usnprintf(score_limit_text->text_box.string, SCORE_LIMIT_STRING_LENGTH - 1, L"%d", game->score_to_win);
				score_limit_text->text_box.string[SCORE_LIMIT_STRING_LENGTH - 1] = L'\0';
			}

			switch (game->game_engine)
			{
			case _game_engine_ctf:
				score_limit_type_text->text_box.string_list_index = _string_captures;
				break;
			case _game_engine_slayer:
				score_limit_type_text->text_box.string_list_index = _string_frags;
				break;
			case _game_engine_oddball:
				if (game->terminator == TRUE)
				{
					score_limit_type_text->text_box.string_list_index = _string_frags;
				}
				else
				{
					score_limit_type_text->text_box.string_list_index = _string_minutes;
				}
				break;
			case _game_engine_king:
				score_limit_type_text->text_box.string_list_index = _string_minutes;
				break;
			case _game_engine_race:
				score_limit_type_text->text_box.string_list_index = _string_laps;
				break;
			default:
				score_limit_type_text->text_box.string_list_index = _string_empty_string;
				break;
			}

			status_text->text_box.string_list_index = _string_looking_for_servers_empty_string;
			status_text->visible = FALSE;

			if (!widget->focused_child)
			{
				widget->list.item_index = 0;
				widget->focused_child = widget->first_child;
			}
		}
		else
		{
			gametype_picture->bitmap_frame_index = _multiplayer_game_type_unknown_bitmap;
			map_picture->bitmap_frame_index = _mp_map_unknown;
			open_closed_text->text_box.string_list_index = _string_empty_string;
			map_name_text->text_box.string_list_index = _mp_map_name_empty_string;
			ruleset_text->text_box.string_list_index = _string_empty_string;
			teams_text->text_box.string_list_index = _string_empty_string;




			number_of_players_text->text_box.string = ui_widget_realloc(number_of_players_text->text_box.string, NUMBER_OF_PLAYERS_STRING_LENGTH * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(931));

			if (number_of_players_text->text_box.string)
			{
				number_of_players_text->text_box.string[0] = L'\0';
			}

			score_limit_text->text_box.string = ui_widget_realloc(score_limit_text->text_box.string, SCORE_LIMIT_STRING_LENGTH * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(935));

			if (score_limit_text->text_box.string)
			{
				score_limit_text->text_box.string[0] = L'\0';
			}

			score_limit_type_text->text_box.string_list_index = _string_empty_string;
			status_text->text_box.string_list_index = elapsed_milliseconds >= MILLISECONDS_PER_SECOND;
			status_text->visible = TRUE;
		}
	}

	return;
}

static void network_pregame_status_screen_update(
	struct widget_instance *widget)
{
	static byte const team_bitmap_indices[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS][NUMBER_OF_NETWORK_TEAM_BITMAPS] =
	{
		{
			_controller_player1_active_bitmap,
			_controller_player1_blue_active_bitmap,
			_controller_player1_red_active_bitmap
		},
		{
			_controller_player2_active_bitmap,
			_controller_player2_blue_active_bitmap,
			_controller_player2_red_active_bitmap
		},
		{
			_controller_player3_active_bitmap,
			_controller_player3_blue_active_bitmap,
			_controller_player3_red_active_bitmap
		},
		{
			_controller_player4_active_bitmap,
			_controller_player4_blue_active_bitmap,
			_controller_player4_red_active_bitmap
		},
	};
	struct network_game_data *game = network_game_get_game();

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1020, ui_widget_definition_get(widget->tag_index)->child_widgets.count == 6, "this doesn't look like the net pregame status screen to me");

	if (game)
	{
		long player_indices[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
		wchar_t const *machine_name;
		boolean found_local_machine;
		long i;
		long j;
		short machine_index = network_game_client_get_machine_index(global_network_game_client_get());
		struct widget_instance *local_machine_widget = widget->first_child;
		struct widget_instance *remote_machine_widget0 = local_machine_widget->next;
		struct widget_instance *remote_machine_widget1 = remote_machine_widget0->next;
		struct widget_instance *remote_machine_widget2 = remote_machine_widget1->next;
		struct widget_instance *countdown_label = remote_machine_widget2->next;
		struct widget_instance *countdown_text = countdown_label->next;

		countdown_text->text_box.string = ui_widget_realloc(countdown_text->text_box.string, AUTO_START_STRING_LENGTH * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1035));

		if (countdown_text->text_box.string)
		{
			short seconds_to_game_start = network_game_client_get_seconds_to_game_start(global_network_game_client_get());
			boolean only_machine = global_network_game_server_get() && game->machine_count < 2;

			ustrncpy(countdown_text->text_box.string, L"-:--", AUTO_START_STRING_LENGTH - 1);
			countdown_label->visible = TRUE;
			countdown_label->text_box.string_list_index = _string_label_game_start;
			countdown_text->visible = TRUE;

			if (seconds_to_game_start == 0)
			{
				countdown_label->text_box.string_list_index = _string_label_loading;
				countdown_text->visible = FALSE;
			}
			else if (seconds_to_game_start > 0)
			{
				if (seconds_to_game_start < SECONDS_PER_MINUTE)
				{
					usnprintf(countdown_text->text_box.string, AUTO_START_STRING_LENGTH - 1, L"0:%02d", seconds_to_game_start);
				}
				else if (seconds_to_game_start < SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
				{
					long minutes = seconds_to_game_start / SECONDS_PER_MINUTE;
					long seconds = seconds_to_game_start - minutes * SECONDS_PER_MINUTE;

					usnprintf(countdown_text->text_box.string, AUTO_START_STRING_LENGTH - 1, L"%02d:%02d", minutes, seconds);
				}
				else
				{
					long hours = seconds_to_game_start / (SECONDS_PER_MINUTE * MINUTES_PER_HOUR);
					long minutes = (seconds_to_game_start - hours * (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)) / SECONDS_PER_MINUTE;
					long seconds = seconds_to_game_start - hours * (SECONDS_PER_MINUTE * MINUTES_PER_HOUR) - minutes * SECONDS_PER_MINUTE;

					usnprintf(countdown_text->text_box.string, AUTO_START_STRING_LENGTH - 1, L"%d:%02d:%02d", hours, minutes, seconds);
				}
			}
			else if (only_machine || game->variant.universal_variant.teams == TRUE)
			{
				countdown_label->visible = FALSE;
				countdown_text->visible = FALSE;
			}

			countdown_text->text_box.string[AUTO_START_STRING_LENGTH - 1] = L'\0';
		}

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1100, ui_widget_definition_get(local_machine_widget->tag_index)->child_widgets.count == 6, "this doesn't look like the net pregame status screen to me");
		{
			struct widget_instance *machine_name_text;
			struct widget_instance *machine_bitmap;
			struct widget_instance *local_player_widgets[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
			long local_player_count;
			size_t length;

			found_local_machine = FALSE;
			machine_name = L"?";
			machine_name_text = local_machine_widget->first_child;
			machine_bitmap = machine_name_text->next;
			local_player_widgets[0] = machine_bitmap->next;
			local_player_widgets[1] = local_player_widgets[0]->next;
			local_player_widgets[2] = local_player_widgets[1]->next;
			local_player_widgets[3] = local_player_widgets[2]->next;

			for (i = 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
			{
				struct network_machine *machine = &game->machines[i];

				if (machine && machine->machine_index >= 0 && machine->machine_index < MAXIMUM_NETWORK_MACHINE_COUNT && machine->machine_index == machine_index)
				{
					struct network_machine *local_machine = network_game_client_get_machine(global_network_game_client_get());

					if (local_machine && local_machine->name[0])
					{
						machine_name = local_machine->name;
					}

					found_local_machine = TRUE;
				}
			}

			memset(player_indices, NONE, sizeof(player_indices));
			local_player_count = 0;

			for (i = 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i++)
			{
				if (network_player_is_valid(&game->players[i]) && game->players[i].machine_index == machine_index)
				{
					local_player_count++;
					player_indices[game->players[i].controller_index] = i;

					if (local_player_count == MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
					{
						break;
					}
				}
			}

			length = ustrlen(machine_name);
			machine_name_text->text_box.string = ui_widget_realloc(machine_name_text->text_box.string, (word)(length * sizeof(wchar_t) + sizeof(wchar_t)), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1147));

			if (machine_name_text->text_box.string)
			{
				ustrncpy(machine_name_text->text_box.string, machine_name, length);
				machine_name_text->text_box.string[length] = L'\0';
			}

			machine_bitmap->bitmap_frame_index = found_local_machine != FALSE;

			for (i = 0; i < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; i++)
			{
				struct widget_instance *controller_bitmap;
				struct widget_instance *player_name_text;
				struct widget_instance *team_spinner;
				struct widget_instance *local_player_widget = local_player_widgets[i];

				match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1157, ui_widget_definition_get(local_player_widget->tag_index)->child_widgets.count == 3, "this doesn't look like the net pregame status screen to me");
				controller_bitmap = local_player_widget->first_child;




				match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1163, controller_bitmap && controller_bitmap->type == _ui_widget_type_container, "expected container widget for local player controller bitmap");
				player_name_text = controller_bitmap->next;
				match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1165, player_name_text && player_name_text->type == _ui_widget_type_text_box, "expected a text box for local player name field");
				team_spinner = player_name_text->next;
				match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1167, team_spinner && team_spinner->type == _ui_widget_type_spinner_list, "expected a spinner list for local player team display");

				if (!game->variant.universal_variant.teams)
				{
					widget_instance_set_visibility_recursive(team_spinner, FALSE);
				}
				else
				{
					widget_instance_set_visibility_recursive(team_spinner, TRUE);
				}

				if (player_indices[i] == NONE)
				{
					controller_bitmap->bitmap_frame_index = _controller_inactive_bitmap;
					player_name_text->text_box.string = ui_widget_realloc(player_name_text->text_box.string, sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1181));

					if (player_name_text->text_box.string)
					{
						player_name_text->text_box.string[0] = L'\0';
					}

					team_spinner->list.item_index = 2;
				}
				else
				{
					length = ustrlen(game->players[player_indices[i]].name);
					player_name_text->text_box.string = ui_widget_realloc(player_name_text->text_box.string, (word)(length * sizeof(wchar_t) + sizeof(wchar_t)), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1190));

					if (player_name_text->text_box.string)
					{
						ustrncpy(player_name_text->text_box.string, game->players[player_indices[i]].name, length);
						player_name_text->text_box.string[length] = L'\0';
					}

					if (!game->variant.universal_variant.teams)
					{
						controller_bitmap->bitmap_frame_index = _controller_active_bitmap;
					}
					else
					{
						switch (game->players[player_indices[i]].team_index)
						{
						case _team_red:
							controller_bitmap->bitmap_frame_index = team_bitmap_indices[i][_network_team_bitmap_red];
							team_spinner->list.item_index = 0;
							break;
						case _team_blue:
							controller_bitmap->bitmap_frame_index = team_bitmap_indices[i][_network_team_bitmap_blue];
							team_spinner->list.item_index = 1;
							break;
						default:
							controller_bitmap->bitmap_frame_index = team_bitmap_indices[i][_network_team_bitmap_none];
							team_spinner->list.item_index = 2;
							break;
						}
					}
				}
			}
		}

		{
			struct widget_instance *remote_machine_widgets[MAXIMUM_NETWORK_MACHINE_COUNT - 1];
			long machine_indices[MAXIMUM_NETWORK_MACHINE_COUNT - 1];

			remote_machine_widgets[0] = remote_machine_widget0;
			remote_machine_widgets[1] = remote_machine_widget1;
			remote_machine_widgets[2] = remote_machine_widget2;
			memset(machine_indices, NONE, sizeof(machine_indices));

			for (i = 0, j = 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
			{
				struct network_machine *machine = &game->machines[i];

				if (machine && machine->machine_index >= 0 && machine->machine_index < MAXIMUM_NETWORK_MACHINE_COUNT && machine->machine_index != machine_index)
				{
					match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1236, j<(MAXIMUM_NETWORK_MACHINE_COUNT-1));
					machine_indices[j++] = i;
				}
			}

			for (i = 0; i < MAXIMUM_NETWORK_MACHINE_COUNT - 1; i++)
			{
				struct widget_instance *remote_machine_bitmap;
				struct widget_instance *remote_machine_name_text;
				struct widget_instance *remote_machine_widget = remote_machine_widgets[i];

				match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1245, ui_widget_definition_get(remote_machine_widget->tag_index)->child_widgets.count == 6, "this doesn't look like the net pregame status screen to me");
				remote_machine_bitmap = remote_machine_widget->first_child;
				remote_machine_name_text = remote_machine_bitmap->next;

				if (machine_indices[i] == NONE)
				{
					struct widget_instance *child;

					remote_machine_bitmap->bitmap_frame_index = _console_inactive_bitmap;

					remote_machine_name_text->text_box.string = ui_widget_realloc(remote_machine_name_text->text_box.string, sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1255));

					if (remote_machine_name_text->text_box.string)
					{
						remote_machine_name_text->text_box.string[0] = L'\0';
					}

					for (child = remote_machine_name_text->next, j = 0; j < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS && child; j++, child = child->next)
					{
						child->bitmap_frame_index = _controller_unknown_bitmap;
					}
				}
				else
				{
					struct widget_instance *child;
					size_t length;

					remote_machine_bitmap->bitmap_frame_index = _console_active_bitmap;
					length = ustrlen(game->machines[machine_indices[i]].name);
					remote_machine_name_text->text_box.string = ui_widget_realloc(remote_machine_name_text->text_box.string, (word)(length * sizeof(wchar_t) + sizeof(wchar_t)), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1268));

					if (remote_machine_name_text->text_box.string)
					{
						ustrncpy(remote_machine_name_text->text_box.string, game->machines[machine_indices[i]].name, length);
						remote_machine_name_text->text_box.string[length] = L'\0';
					}

					memset(player_indices, NONE, sizeof(player_indices));

					for (j = 0; j < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; j++)
					{
						if (network_player_is_valid(&game->players[j]) && game->players[j].machine_index == game->machines[machine_indices[i]].machine_index)
						{
							player_indices[game->players[j].controller_index] = j;
						}
					}

					for (child = remote_machine_name_text->next, j = 0; j < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS && child; j++, child = child->next)
					{
						if (player_indices[j] == NONE)
						{
							child->bitmap_frame_index = _controller_unknown_bitmap;
						}
						else if (!game->variant.universal_variant.teams)
						{
							child->bitmap_frame_index = team_bitmap_indices[j][_network_team_bitmap_none];
						}
						else
						{
							switch (game->players[player_indices[j]].team_index)
							{
							case _team_red:
								child->bitmap_frame_index = team_bitmap_indices[j][_network_team_bitmap_red];
								break;
							case _team_blue:
								child->bitmap_frame_index = team_bitmap_indices[j][_network_team_bitmap_blue];
								break;
							default:
								child->bitmap_frame_index = team_bitmap_indices[j][_network_team_bitmap_none];
								break;
							}
						}
					}
				}
			}
		}
	}

	return;
}

static void splitscreen_pregame_status_screen_update(
	struct widget_instance *widget)
{
	static byte const team_bitmap_indices[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS][NUMBER_OF_SPLITSCREEN_TEAM_BITMAPS] =
	{
		{
			_controller_player1_active_bitmap,
			_controller_player1_red_active_bitmap,
			_controller_player1_blue_active_bitmap
		},
		{
			_controller_player2_active_bitmap,
			_controller_player2_red_active_bitmap,
			_controller_player2_blue_active_bitmap
		},
		{
			_controller_player3_active_bitmap,
			_controller_player3_red_active_bitmap,
			_controller_player3_blue_active_bitmap
		},
		{
			_controller_player4_active_bitmap,
			_controller_player4_red_active_bitmap,
			_controller_player4_blue_active_bitmap
		},
	};
	struct network_game_data *game = network_game_get_game();

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1360, ui_widget_definition_get(widget->tag_index)->child_widgets.count == 3, "this doesn't look like the net pregame status screen to me");

	if (game)
	{
		struct widget_instance *machine_name_text;
		struct widget_instance *machine_bitmap;
		struct widget_instance *local_player_widgets[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
		long player_indices[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
		wchar_t const *machine_name;
		boolean found_local_machine;
		long local_player_count;
		size_t length;
		long i;
		short machine_index = network_game_client_get_machine_index(global_network_game_client_get());
		struct widget_instance *local_machine_widget = widget->first_child;
		struct widget_instance *countdown_label = local_machine_widget->next;
		struct widget_instance *countdown_text = countdown_label->next;

		countdown_text->text_box.string = ui_widget_realloc(countdown_text->text_box.string, AUTO_START_STRING_LENGTH * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1372));

		if (countdown_text->text_box.string)
		{
			short seconds_to_game_start = network_game_client_get_seconds_to_game_start(global_network_game_client_get());

			ustrncpy(countdown_text->text_box.string, L"-:--", AUTO_START_STRING_LENGTH - 1);
			countdown_label->visible = TRUE;
			countdown_label->text_box.string_list_index = _string_label_game_start;
			countdown_text->visible = TRUE;

			if (seconds_to_game_start == 0)
			{
				countdown_label->text_box.string_list_index = _string_label_loading;
				countdown_text->visible = FALSE;
			}
			else if (seconds_to_game_start > 0)
			{
				if (seconds_to_game_start < SECONDS_PER_MINUTE)
				{
					usnprintf(countdown_text->text_box.string, AUTO_START_STRING_LENGTH - 1, L"0:%02d", seconds_to_game_start);
				}
				else if (seconds_to_game_start < SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
				{
					long minutes = seconds_to_game_start / SECONDS_PER_MINUTE;
					long seconds = seconds_to_game_start - minutes * SECONDS_PER_MINUTE;

					usnprintf(countdown_text->text_box.string, AUTO_START_STRING_LENGTH - 1, L"%02d:%02d", minutes, seconds);
				}
				else
				{
					long hours = seconds_to_game_start / (SECONDS_PER_MINUTE * MINUTES_PER_HOUR);
					long minutes = (seconds_to_game_start - hours * (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)) / SECONDS_PER_MINUTE;
					long seconds = seconds_to_game_start - hours * (SECONDS_PER_MINUTE * MINUTES_PER_HOUR) - minutes * SECONDS_PER_MINUTE;

					usnprintf(countdown_text->text_box.string, AUTO_START_STRING_LENGTH - 1, L"%d:%02d:%02d", hours, minutes, seconds);
				}
			}
			else if (game->player_count < 2 || game->variant.universal_variant.teams == TRUE)
			{
				countdown_label->visible = FALSE;
				countdown_text->visible = FALSE;
			}

			countdown_text->text_box.string[AUTO_START_STRING_LENGTH - 1] = L'\0';
		}








		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1426, ui_widget_definition_get(local_machine_widget->tag_index)->child_widgets.count == 6, "this doesn't look like the net pregame status screen to me");
		found_local_machine = FALSE;
		machine_name = L"?";
		machine_name_text = local_machine_widget->first_child;
		machine_bitmap = machine_name_text->next;
		local_player_widgets[0] = machine_bitmap->next;
		local_player_widgets[1] = local_player_widgets[0]->next;
		local_player_widgets[2] = local_player_widgets[1]->next;
		local_player_widgets[3] = local_player_widgets[2]->next;

		for (i = 0; i < MAXIMUM_NETWORK_MACHINE_COUNT; i++)
		{
			struct network_machine *machine = &game->machines[i];

			if (machine && machine->machine_index >= 0 && machine->machine_index < MAXIMUM_NETWORK_MACHINE_COUNT && machine->machine_index == machine_index)
			{
				struct network_machine *local_machine = network_game_client_get_machine(global_network_game_client_get());

				if (local_machine && local_machine->name[0])
				{
					machine_name = local_machine->name;
				}

				found_local_machine = TRUE;
				break;
			}
		}

		memset(player_indices, NONE, sizeof(player_indices));
		local_player_count = 0;

		for (i = 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i++)
		{
			if (network_player_is_valid(&game->players[i]) && game->players[i].machine_index == machine_index)
			{
				local_player_count++;
				player_indices[game->players[i].controller_index] = i;

				if (local_player_count == MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
				{
					break;
				}
			}
		}

		length = ustrlen(machine_name);


		machine_name_text->text_box.string = ui_widget_realloc(machine_name_text->text_box.string, (word)(length * sizeof(wchar_t) + sizeof(wchar_t)), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1474));

		if (machine_name_text->text_box.string)
		{
			ustrncpy(machine_name_text->text_box.string, machine_name, length);
			machine_name_text->text_box.string[length] = L'\0';
		}

		machine_bitmap->bitmap_frame_index = found_local_machine != FALSE;

		for (i = 0; i < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; i++)
		{
			struct widget_instance *controller_bitmap;
			struct widget_instance *player_name_text;
			struct widget_instance *team_spinner;
			struct widget_instance *local_player_widget = local_player_widgets[i];

			match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1484, ui_widget_definition_get(local_player_widget->tag_index)->child_widgets.count == 3, "this doesn't look like the net pregame status screen to me");
			controller_bitmap = local_player_widget->first_child;
			player_name_text = controller_bitmap->next;
			team_spinner = player_name_text->next;

			if (!game->variant.universal_variant.teams)
			{
				widget_instance_set_visibility_recursive(team_spinner, FALSE);
			}

			if (player_indices[i] == NONE)
			{
				controller_bitmap->bitmap_frame_index = _controller_inactive_bitmap;


				player_name_text->text_box.string = ui_widget_realloc(player_name_text->text_box.string, sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1499));

				if (player_name_text->text_box.string)
				{
					player_name_text->text_box.string[0] = L'\0';
				}

				team_spinner->list.item_index = 0;
			}
			else
			{
				length = ustrlen(game->players[player_indices[i]].name);
				player_name_text->text_box.string = ui_widget_realloc(player_name_text->text_box.string, (word)(length * sizeof(wchar_t) + sizeof(wchar_t)), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1508));

				if (player_name_text->text_box.string)
				{
					ustrncpy(player_name_text->text_box.string, game->players[player_indices[i]].name, length);
					player_name_text->text_box.string[length] = L'\0';
				}

				if (!game->variant.universal_variant.teams)
				{
					controller_bitmap->bitmap_frame_index = _controller_active_bitmap;
				}
				else
				{
					switch (game->players[player_indices[i]].team_index)
					{
					case _team_red:
						controller_bitmap->bitmap_frame_index = team_bitmap_indices[i][_splitscreen_team_bitmap_red];
						team_spinner->list.item_index = 0;
						break;
					case _team_blue:
						controller_bitmap->bitmap_frame_index = team_bitmap_indices[i][_splitscreen_team_bitmap_blue];
						team_spinner->list.item_index = 1;
						break;
					default:
						controller_bitmap->bitmap_frame_index = team_bitmap_indices[i][_splitscreen_team_bitmap_none];
						team_spinner->list.item_index = 0;
						break;
					}
				}
			}
		}
	}

	return;
}

static void netgame_prejoin_players(
	struct widget_instance *widget)
{
	struct network_game_client *client = global_network_game_client_get();

	if (client)
	{
		short i;

		if (network_game_client_get_state(client, (word *)&i) == _network_game_client_state_pregame)
		{
			boolean join_player[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
			struct network_game_data *game = network_game_get_game();

			match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1556, game);

			for (i = 0; i < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; i++)
			{
				join_player[i] = player_ui_local_player_wants_to_play_multiplayer(i);
			}

			for (i = 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i++)
			{
				if (network_player_is_valid(&game->players[i]) && network_game_player_is_local(&game->players[i]))
				{
					join_player[game->players[i].controller_index] = FALSE;
				}
			}

			for (i = 0; i < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; i++)
			{
				if (join_player[i] && !network_game_client_add_player(client, i))
				{
					network_event("failed to send join request");
				}
			}
		}
	}

	return;
}

static void mutliplayer_settings_select_list_update_displayed_items(
	struct widget_instance *widget)
{
	long displayed_item_index[NUMBER_OF_SPINNER_LIST_3WIDE_ITEMS];
	long profile_indices[NUMBER_OF_CACHED_PROFILES];
	long i;
	long string_list_tag_index = tag_loaded(UNICODE_STRING_LISTS_GROUP_TAG, GAME_VARIANT_DESCRIPTIONS_TAG_NAME);
	struct ui_widget_definition *definition = ui_widget_definition_get(widget->tag_index);

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1661, definition->type == _ui_widget_type_spinner_list, "expected a spinner list for 'multiplayer settings select' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1662, definition->child_widgets.count == 3, "expected 3 children (list items) for 'multiplayer settings select' widget");
	spinner_list_3wide_determine_displayed_item_indices(widget, displayed_item_index);

	for (i = 0; i < NUMBER_OF_CACHED_PROFILES; i++)
	{
		if (displayed_item_index[i] != NONE)
		{
			profile_indices[i] = ((long *)widget->list.items)[displayed_item_index[i]];
		}
		else
		{
			profile_indices[i] = NONE;
		}
	}

	variant_profile_update_cache_for_nwide_list(profile_indices, NUMBER_OF_CACHED_PROFILES);

	for (i = 0; i < NUMBER_OF_SPINNER_LIST_3WIDE_ITEMS && displayed_item_index[i] != NONE; i++)
	{
		long profile_index;
		struct widget_instance *list_item = widget_instance_get_nth_child(widget, i);
		struct widget_instance *name_text = list_item->first_child;
		struct widget_instance *picture = name_text->next;
		struct widget_instance *description_text = picture->next;
		struct widget_instance *default_label = description_text->next;
		struct game_variant *variant = NULL;






		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1694, ui_widget_definition_get(list_item->tag_index)->child_widgets.count, "expected 3 children in multiplayer settings list item (name, pic, desc)");

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1696, ui_widget_definition_get(name_text->tag_index)->type == _ui_widget_type_text_box, "expected a text box widget for the list item's first child (map name)");

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1698, ui_widget_definition_get(picture->tag_index)->type == _ui_widget_type_container, "expected a container widget for the list item's second child (map pic)");

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1700, ui_widget_definition_get(description_text->tag_index)->type == _ui_widget_type_text_box, "expected a text box widget for the list item's third child (map description)");
		profile_index = ((long *)widget->list.items)[displayed_item_index[i]];

		if (profile_index != NONE)
		{
			long cache_index;

			for (cache_index = 0; cache_index < NUMBER_OF_CACHED_PROFILES; cache_index++)
			{
				if (cached_variant_profile[cache_index].profile_index == profile_index)
				{
					variant = &cached_variant_profile[cache_index].variant;
					break;
				}
			}
		}

		default_label->visible = FALSE;

		if (variant)
		{
			name_text->text_box.string = ui_widget_realloc(name_text->text_box.string, 128 * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1722));

			if (name_text->text_box.string)
			{
				ustrncpy(name_text->text_box.string, variant->human_readable_game_description, 127);
				name_text->text_box.string[127] = L'\0';
			}

			picture->bitmap_frame_index = _multiplayer_game_type_unknown_bitmap;
			description_text->text_box.string = ui_widget_realloc(description_text->text_box.string, MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1729));

			if (description_text->text_box.string)
			{
				description_text->text_box.string[0] = L'\0';
			}

			if (TEST_FLAG(variant->flags, _variant_is_system_default_bit))
			{
				long string_index = ((variant->flags & _default_variant_index_mask) >> CHAR_BITS) + FIRST_DEFAULT_VARIANT_DESCRIPTION;

				switch (variant->game_engine_index)
				{
				case _game_engine_ctf:
					picture->bitmap_frame_index = _multiplayer_game_type_ctf_bitmap;
					break;
				case _game_engine_slayer:
					picture->bitmap_frame_index = _multiplayer_game_type_slayer_bitmap;
					break;
				case _game_engine_oddball:
					picture->bitmap_frame_index = _multiplayer_game_type_oddball_bitmap;
					break;
				case _game_engine_king:
					picture->bitmap_frame_index = _multiplayer_game_type_koth_bitmap;
					break;
				case _game_engine_race:
					picture->bitmap_frame_index = _multiplayer_game_type_race_bitmap;
					break;
				}

				if (string_list_tag_index != NONE && description_text->text_box.string)
				{
					ustrncpy(
						description_text->text_box.string,
						unicode_string_list_get_string(string_list_tag_index, string_index),
						MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1);
					description_text->text_box.string[MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1] = L'\0';
				}

				default_label->visible = TRUE;
			}
			else
			{
				switch (variant->game_engine_index)
				{
				case _game_engine_ctf:
					picture->bitmap_frame_index = _multiplayer_game_type_ctf_bitmap;

					if (string_list_tag_index != NONE && description_text->text_box.string)
					{
						if (variant->universal_variant.teams == TRUE)
						{
							ustrncpy(
								description_text->text_box.string,
								unicode_string_list_get_string(string_list_tag_index, _description_ctf_teams),
								MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1);
						}
						else
						{
							ustrncpy(
								description_text->text_box.string,
								unicode_string_list_get_string(string_list_tag_index, _description_ctf_ffa),
								MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1);
						}

						description_text->text_box.string[MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1] = L'\0';
					}
					break;
				case _game_engine_slayer:
					picture->bitmap_frame_index = _multiplayer_game_type_slayer_bitmap;

					if (string_list_tag_index != NONE && description_text->text_box.string)
					{
						if (variant->universal_variant.teams == TRUE)
						{
							ustrncpy(
								description_text->text_box.string,
								unicode_string_list_get_string(string_list_tag_index, _description_slayer_teams),
								MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1);
						}
						else
						{
							ustrncpy(
								description_text->text_box.string,
								unicode_string_list_get_string(string_list_tag_index, _description_slayer_ffa),
								MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1);
						}

						description_text->text_box.string[MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1] = L'\0';
					}
					break;
				case _game_engine_oddball:
					picture->bitmap_frame_index = _multiplayer_game_type_oddball_bitmap;

					if (string_list_tag_index != NONE && description_text->text_box.string)
					{
						if (variant->universal_variant.teams == TRUE)
						{
							ustrncpy(
								description_text->text_box.string,
								unicode_string_list_get_string(string_list_tag_index, _description_oddball_teams),
								MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1);
						}
						else
						{
							ustrncpy(
								description_text->text_box.string,
								unicode_string_list_get_string(string_list_tag_index, _description_oddball_ffa),
								MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1);
						}

						description_text->text_box.string[MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1] = L'\0';
					}
					break;
				case _game_engine_king:
					picture->bitmap_frame_index = _multiplayer_game_type_koth_bitmap;

					if (string_list_tag_index != NONE && description_text->text_box.string)
					{
						if (variant->universal_variant.teams == TRUE)
						{
							ustrncpy(
								description_text->text_box.string,
								unicode_string_list_get_string(string_list_tag_index, _description_koth_teams),
								MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1);
						}
						else
						{
							ustrncpy(
								description_text->text_box.string,
								unicode_string_list_get_string(string_list_tag_index, _description_koth_ffa),
								MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1);
						}

						description_text->text_box.string[MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1] = L'\0';
					}
					break;
				case _game_engine_race:
					picture->bitmap_frame_index = _multiplayer_game_type_race_bitmap;

					if (string_list_tag_index != NONE && description_text->text_box.string)
					{
						if (variant->universal_variant.teams == TRUE)
						{
							ustrncpy(
								description_text->text_box.string,
								unicode_string_list_get_string(string_list_tag_index, _description_racing_teams),
								MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1);
						}
						else
						{
							ustrncpy(
								description_text->text_box.string,
								unicode_string_list_get_string(string_list_tag_index, _description_racing_ffa),
								MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1);
						}

						description_text->text_box.string[MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1] = L'\0';
					}
					break;
				}
			}
		}
		else
		{
			name_text->text_box.string = ui_widget_realloc(name_text->text_box.string, 128 * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1873));

			if (name_text->text_box.string)
			{
				name_text->text_box.string[0] = L'\0';
			}

			picture->bitmap_frame_index = _multiplayer_game_type_unknown_bitmap;
			description_text->text_box.string = ui_widget_realloc(description_text->text_box.string, MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(1878));

			if (description_text->text_box.string)
			{
				long labels_tag_index = tag_loaded(UNICODE_STRING_LISTS_GROUP_TAG, PROFILE_DESCRIPTION_LABELS_TAG_NAME);

				description_text->text_box.string[0] = L'\0';

				if (labels_tag_index != NONE)
				{
					wchar_t *string = unicode_string_list_get_string(labels_tag_index, 5);

					ustrncpy(description_text->text_box.string, string, MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1);
					description_text->text_box.string[MAXIMUM_MULTIPLAYER_PROFILE_DESCRIPTION_LENGTH - 1] = L'\0';
				}
			}
		}
	}

	return;
}

static void player_profile_3wide_list_update(
	struct widget_instance *widget)
{
	struct ui_widget_definition *definition;
	long profile_indices[NUMBER_OF_CACHED_PROFILES];
	long i;
	long displayed_item_index[NUMBER_OF_SPINNER_LIST_3WIDE_ITEMS] = { NONE, NONE, NONE };

	definition = ui_widget_definition_get(widget->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1930, definition->type == _ui_widget_type_spinner_list, "expected a spinner list for 'player settings select' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1931, definition->child_widgets.count == 3, "expected 3 children (list items) for 'player settings select' widget");
	spinner_list_3wide_determine_displayed_item_indices(widget, displayed_item_index);

	for (i = 0; i < NUMBER_OF_CACHED_PROFILES; i++)
	{
		if (displayed_item_index[i] != NONE)
		{
			profile_indices[i] = ((long *)widget->list.items)[displayed_item_index[i]];
		}
		else
		{
			profile_indices[i] = NONE;
		}
	}

	player_profile_update_cache_for_nwide_list(profile_indices, NUMBER_OF_CACHED_PROFILES);

	for (i = 0; i < NUMBER_OF_SPINNER_LIST_3WIDE_ITEMS && displayed_item_index[i] != NONE; i++)
	{
		struct widget_instance *name_text;
		struct widget_instance *color_picture;
		struct widget_instance *description;
		struct widget_instance *empty_profile_label;
		struct widget_instance *current_level_label;
		struct widget_instance *current_level_text;
		struct widget_instance *skill_level_label;
		struct widget_instance *skill_level_text;
		struct widget_instance *controls_label;
		struct widget_instance *controls_text;
		long profile_index;
		struct widget_instance *list_item = widget_instance_get_nth_child(widget, i);
		struct player_profile *profile = NULL;




		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1967, list_item && list_item->type == _ui_widget_type_container, "expected profile item description container widget");
		name_text = list_item->first_child;

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1970, name_text && name_text->type == _ui_widget_type_text_box, "expected a text box widget for profile name");
		color_picture = name_text->next;

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1973, color_picture && color_picture->type == _ui_widget_type_container, "expected a container widget for the profile color picture");
		description = color_picture->next;

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1976, description && description->type == _ui_widget_type_container, "expected a container widget for the profile description fields");
		empty_profile_label = description->first_child;

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1979, empty_profile_label && empty_profile_label->type == _ui_widget_type_text_box, "expected a text box widget for the 'empty profile' label");
		current_level_label = empty_profile_label->next;

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1982, current_level_label && current_level_label->type == _ui_widget_type_text_box, "expected a text box widget for the current level label");
		current_level_text = current_level_label->next;

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1985, current_level_text && current_level_text->type == _ui_widget_type_text_box, "expected a text box widget for the current level text field");
		skill_level_label = current_level_text->next;

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1988, skill_level_label && skill_level_label->type == _ui_widget_type_text_box, "expected a text box widget for the current skill level label");
		skill_level_text = skill_level_label->next;

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1991, skill_level_text && skill_level_text->type == _ui_widget_type_text_box, "expected a text box widget for the profile's current skill level text field");
		controls_label = skill_level_text->next;

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1994, controls_label && controls_label->type == _ui_widget_type_text_box, "expected a text box widget for the controls label");
		controls_text = controls_label->next;

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 1997, controls_text && controls_text->type == _ui_widget_type_text_box, "expected a text box widget for the controls text field");
		profile_index = ((long *)widget->list.items)[displayed_item_index[i]];

		if (profile_index != NONE)
		{
			long cache_index;

			for (cache_index = 0; cache_index < NUMBER_OF_CACHED_PROFILES; cache_index++)
			{
				if (cached_player_profile[cache_index].profile_index == profile_index)
				{
					profile = &cached_player_profile[cache_index].profile;
					break;
				}
			}
		}

		if (profile)
		{
			name_text->visible = TRUE;
			empty_profile_label->visible = FALSE;
			current_level_label->visible = TRUE;
			current_level_text->visible = TRUE;
			skill_level_label->visible = TRUE;
			skill_level_text->visible = TRUE;
			controls_label->visible = TRUE;
			controls_text->visible = TRUE;
			name_text->text_box.string = ui_widget_realloc(name_text->text_box.string, (MAXIMUM_PLAYER_PROFILE_NAME_LENGTH + 1) * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(2023));

			if (name_text->text_box.string)
			{
				if (TEST_FLAG(profile->flags, _profile_is_default_bit))
				{
					wchar_t *string;
					long string_index = (profile->flags & _default_profile_index_mask) >> CHAR_BITS;
					long names_tag_index = tag_loaded(UNICODE_STRING_LISTS_GROUP_TAG, DEFAULT_PLAYER_PROFILE_NAMES_TAG_NAME);

					if (names_tag_index != NONE)
					{
						string = unicode_string_list_get_string(names_tag_index, string_index);
					}
					else
					{
						string = L"<unknown>";
					}

					ustrncpy(name_text->text_box.string, string, MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
					name_text->text_box.string[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = L'\0';
				}
				else
				{
					ustrncpy(name_text->text_box.string, profile->player_name, MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
					name_text->text_box.string[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = L'\0';
				}
			}

			color_picture->bitmap_frame_index = PIN(profile->primary_color_index, 0, player_profile_number_of_available_primary_colors() - 1);

			if (TEST_FLAG(profile->flags, _profile_is_default_bit))
			{
				current_level_text->visible = FALSE;
				skill_level_text->visible = FALSE;
			}
			else
			{
				short level;
				short difficulty;

				player_profile_get_highest_completed_solo_level(profile, &level, &difficulty);
				level = MIN(level + 1, NUMBER_OF_SINGLE_PLAYER_LEVELS - 1);
				current_level_text->text_box.string_list_index = level;
				skill_level_text->text_box.string_list_index = difficulty;
				controls_text->text_box.string_list_index = profile->controller_settings.invert_look == TRUE;
			}
		}
		else
		{
			name_text->visible = FALSE;
			color_picture->bitmap_frame_index = player_profile_number_of_available_primary_colors();
			empty_profile_label->visible = TRUE;
			current_level_label->visible = FALSE;
			current_level_text->visible = FALSE;
			skill_level_label->visible = FALSE;
			skill_level_text->visible = FALSE;
			controls_label->visible = FALSE;
			controls_text->visible = FALSE;
		}
	}

	return;
}

static void player_profile_edit_select_menu_update_extended_description(
	struct widget_instance *widget)
{
	short index;
	struct widget_instance *child;
	struct widget_instance *text;
	struct widget_instance *picture;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2102, widget && widget->focused_child && widget->list.extended_description_widget, "invalid widget trying to update its extended list description");
	index = 0;
	child = widget->first_child;

	while (child && child != widget->focused_child)
	{
		child = child->next;
		index++;
	}

	if (index != NONE)
	{
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2116, ui_widget_definition_get(widget->list.extended_description_widget->tag_index)->child_widgets.count == 2, "expected a container widget w/ 2 children for the player profile edit settings list extended description");
		text = widget->list.extended_description_widget->first_child;
		picture = text->next;
		text->text_box.string_list_index = index;
		picture->bitmap_frame_index = index;
	}

	return;
}

static void player_profile_1wide_list_update(
	struct widget_instance *widget)
{
	struct ui_widget_definition *definition;
	struct widget_instance *color_picture;
	struct widget_instance *description_text;
	long profile_index;
	struct player_profile *profile = NULL;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2142, widget->parent, "expected qtr-screen profile select list to be wrapped in a container widget");
	definition = ui_widget_definition_get(widget->tag_index);




	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2148, definition->type == _ui_widget_type_spinner_list, "expected a spinner list for 'mp player settings select' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2149, definition->child_widgets.count == 0, "expected 0 children (1-wide spinner) for 'mp player settings select' widget");
	definition = ui_widget_definition_get(widget->parent->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2151, definition->child_widgets.count == 3, "expected qtr-screen profile select wrapper screen to have 3 child widgets (pic, description, list... in that order)");
	color_picture = widget->parent->first_child;
	description_text = color_picture->next;



	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2157, widget == description_text->next, "expected qtr-screen profile select wrapper screen to have 3 child widgets (pic, description, list... in that order)");

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2159, widget->list.item_index >= 0 && widget->list.item_index < widget->list.item_count, "qtr-screen profile list has invalid list item index");


	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2162, widget->list.item_index >= 0 && widget->list.item_index < widget->list.item_count, "invalid list item index");
	profile_index = ((long *)widget->list.items)[widget->list.item_index];
	player_profile_update_cache_for_nwide_list(&profile_index, 1);

	if (profile_index != NONE)
	{
		long cache_index;

		for (cache_index = 0; cache_index < NUMBER_OF_CACHED_PROFILES; cache_index++)
		{
			if (cached_player_profile[cache_index].profile_index == profile_index)
			{
				profile = &cached_player_profile[cache_index].profile;
				break;
			}
		}
	}

	if (profile)
	{
		widget->list.string = ui_widget_realloc(widget->list.string, (MAXIMUM_PLAYER_PROFILE_NAME_LENGTH + 1) * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(2182));

		if (widget->list.string)
		{
			if (TEST_FLAG(profile->flags, _profile_is_default_bit))
			{
				long string_index = (profile->flags & _default_profile_index_mask) >> CHAR_BITS;
				long names_tag_index = tag_loaded(UNICODE_STRING_LISTS_GROUP_TAG, DEFAULT_PLAYER_PROFILE_NAMES_TAG_NAME);
				wchar_t *string = L"";

				if (names_tag_index != NONE)
				{
					string = unicode_string_list_get_string(names_tag_index, string_index);
				}

				ustrncpy(widget->list.string, string, MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
				widget->list.string[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = L'\0';
			}
			else
			{
				ustrncpy(widget->list.string, profile->player_name, MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
				widget->list.string[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = L'\0';
			}

			color_picture->bitmap_frame_index = PIN(profile->primary_color_index, 0, player_profile_number_of_available_primary_colors() - 1);
			description_text->text_box.string = ui_widget_realloc(description_text->text_box.string, MAXIMUM_PLAYER_PROFILE_DESCRIPTION_LENGTH * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(2204));

			if (description_text->text_box.string)
			{
				if (TEST_FLAG(profile->flags, _profile_is_default_bit))
				{
					long joystick_tag_index = tag_loaded(UNICODE_STRING_LISTS_GROUP_TAG, JOYSTICK_SET_DEFAULTS_DESCRIPTIONS_TAG_NAME);
					long button_tag_index = tag_loaded(UNICODE_STRING_LISTS_GROUP_TAG, BUTTON_SET_LONG_DESCRIPTIONS_TAG_NAME);

					if (joystick_tag_index == NONE || button_tag_index == NONE)
					{
						description_text->text_box.string[0] = L'\0';
					}
					else
					{
						wchar_t *joystick_string = unicode_string_list_get_string(joystick_tag_index, profile->controller_settings.joystick_preset);
						wchar_t *button_string = unicode_string_list_get_string(button_tag_index, profile->controller_settings.button_preset);

						usnprintf(
							description_text->text_box.string,
							MAXIMUM_PLAYER_PROFILE_DESCRIPTION_LENGTH - 1,
							L"%s%hs%s",
							joystick_string,
							"\r\n",
							button_string);
						description_text->text_box.string[MAXIMUM_PLAYER_PROFILE_DESCRIPTION_LENGTH - 1] = L'\0';
					}
				}
				else
				{
					long joystick_tag_index = tag_loaded(UNICODE_STRING_LISTS_GROUP_TAG, JOYSTICK_SET_SHORT_DESCRIPTIONS_TAG_NAME);
					long button_tag_index = tag_loaded(UNICODE_STRING_LISTS_GROUP_TAG, BUTTON_SET_SHORT_DESCRIPTIONS_TAG_NAME);

					if (joystick_tag_index != NONE && button_tag_index != NONE)
					{
						wchar_t *joystick_string = unicode_string_list_get_string(joystick_tag_index, profile->controller_settings.joystick_preset);
						wchar_t *button_string = unicode_string_list_get_string(button_tag_index, profile->controller_settings.button_preset);

						usnprintf(
							description_text->text_box.string,
							MAXIMUM_PLAYER_PROFILE_DESCRIPTION_LENGTH - 1,
							L"%s%hs%s",
							joystick_string,
							"\r\n",
							button_string);
					}
				}

				description_text->text_box.string[MAXIMUM_PLAYER_PROFILE_DESCRIPTION_LENGTH - 1] = L'\0';
			}
		}
	}
	else if (widget->list.item_count > 0)
	{
		widget->list.item_count = (word)filter_invalid_list_indices(widget->list.items, widget->list.item_count);
		widget->list.item_index = PIN(widget->list.item_index, 0, widget->list.item_count - 1);
		player_profile_1wide_list_update(widget);
	}
	else
	{
		widget->list.string = ui_widget_realloc(widget->list.string, 2 * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(2258));

		if (widget->list.string)
		{
			widget->list.string[0] = L'\0';
		}

		color_picture->bitmap_frame_index = 0;
		description_text->text_box.string = ui_widget_realloc(description_text->text_box.string, 2 * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(2263));

		if (description_text->text_box.string)
		{
			description_text->text_box.string[0] = L'\0';
		}
	}

	return;
}

static void game_options_menu_update_text_desc(
	struct widget_instance *widget)
{
	long index;
	struct widget_instance *description;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2293, widget->type == _ui_widget_type_column_list, "expected column list for multiplayer game options list");


	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2296, widget->list.extended_description_widget && widget->list.extended_description_widget->type == _ui_widget_type_text_box, "expected a text box for multiplayer game options list extended description");





	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2302, ui_widget_definition_get(widget->tag_index)->child_widgets.count > 0, "expected some list items for multiplayer game settings list");
	description = widget->list.extended_description_widget;

	if (widget->focused_child)
	{
		struct widget_instance *child;

		index = 0;

		for (child = widget->first_child; child; child = child->next)
		{
			struct widget_instance *spinner;

			for (spinner = child->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
			match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2316, spinner, "expected a spinner list somewhere in the multiplayer game options settings list item makeup");

			if (child == widget->focused_child)
			{
				index += spinner->list.item_index;
				break;
			}

			index += spinner->list.item_count;
		}
	}

	description->text_box.string_list_index = index;

	return;
}

static void solo_game_objective_text(
	struct widget_instance *widget)
{
	long length;
	wchar_t const *objective = hud_messaging_get_objective();

	if (objective && objective[0])
	{
		length = ustrlen(objective);
	}
	else
	{
		length = 0;
	}

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2340, widget->type == _ui_widget_type_text_box, "expected a text box for objective text");

	if (length > 0)
	{
		widget->text_box.string = ui_widget_realloc(widget->text_box.string, (word)(length * sizeof(wchar_t) + sizeof(wchar_t)), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(2344));

		if (widget->text_box.string)
		{
			ustrncpy(widget->text_box.string, objective, length);
			widget->text_box.string[length] = L'\0';
		}
	}

	return;
}

static void player_profile_color_picker_update(
	struct widget_instance *widget)
{
	struct ui_widget_definition *definition;
	long displayed_item_index[NUMBER_OF_SPINNER_LIST_3WIDE_ITEMS];
	long i;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2372, widget->list.items, "no list items associated with color picker!");
	definition = ui_widget_definition_get(widget->tag_index);



	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2377, definition->type == _ui_widget_type_spinner_list, "expected a spinner list for 'player color picker' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2378, definition->child_widgets.count == 3, "expected 3 children (list items) for 'player color picker' widget");
	spinner_list_3wide_determine_displayed_item_indices(widget, displayed_item_index);

	for (i = 0; i < NUMBER_OF_SPINNER_LIST_3WIDE_ITEMS && displayed_item_index[i] != NONE; i++)
	{
		short color_index;
		struct widget_instance *list_item = widget_instance_get_nth_child(widget, i);
		struct widget_instance *color_name = list_item->first_child;
		struct widget_instance *color_picture = color_name->next;

		definition = ui_widget_definition_get(list_item->tag_index);






		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2395, definition->child_widgets.count == 2, "expected 2 children in player color picker list item (name, pic)");
		definition = ui_widget_definition_get(color_name->tag_index);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2397, definition->type == _ui_widget_type_text_box, "expected a text box widget for the list item's first child (color name)");
		definition = ui_widget_definition_get(color_picture->tag_index);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2399, definition->type == _ui_widget_type_container, "expected a container widget for the list item's second child (color pic)");
		color_index = ((byte *)widget->list.items)[displayed_item_index[i]];
		color_name->text_box.string_list_index = color_index;
		color_picture->bitmap_frame_index = color_index;
	}

	return;
}

static void game_options_menu_update_pic_desc(
	struct widget_instance *widget)
{
	long index;
	struct widget_instance *picture;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2432, widget->type == _ui_widget_type_column_list, "expected column list for game options list");


	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2435, widget->list.extended_description_widget && widget->list.extended_description_widget->type == _ui_widget_type_container, "expected a picture (container) for game options list extended description");




	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2440, ui_widget_definition_get(widget->tag_index)->child_widgets.count > 0, "expected some list items for game settings list");
	picture = widget->list.extended_description_widget;

	if (widget->focused_child)
	{
		struct widget_instance *child;

		index = 0;

		for (child = widget->first_child; child; child = child->next)
		{
			struct widget_instance *spinner;

			for (spinner = child->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
			match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2454, spinner, "expected a spinner list somewhere in the multiplayer game options settings list item makeup");

			if (child == widget->focused_child)
			{
				index += spinner->list.item_index;
				break;
			}

			index += spinner->list.item_count;
		}
	}

	picture->bitmap_frame_index = index;

	return;
}

static void main_menu_animation_fakery(
	struct widget_instance *widget)
{
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2475, widget->type == _ui_widget_type_column_list, "expected column list for main menu options list");


	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2478, widget->list.extended_description_widget && widget->list.extended_description_widget->type == _ui_widget_type_container, "expected a picture (container) for main menu options list extended description");
	widget->list.extended_description_widget->bitmap_frame_index = widget->list.item_index;
	widget->list.extended_description_widget->bitmap_frame_index = (widget->list.extended_description_widget->bitmap_frame_index < 0) ? 0 : widget->list.extended_description_widget->bitmap_frame_index;

	return;
}

static void mp_level_select_list_update_displayed_items(
	struct widget_instance *widget)
{
	long displayed_item_index[NUMBER_OF_SPINNER_LIST_3WIDE_ITEMS];
	long i;
	struct ui_widget_definition *definition = ui_widget_definition_get(widget->tag_index);

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2495, definition->type == _ui_widget_type_spinner_list, "expected a spinner list for 'level select' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2496, definition->child_widgets.count == 3, "expected 3 children (list items) for 'level select' widget");
	spinner_list_3wide_determine_displayed_item_indices(widget, displayed_item_index);

	for (i = 0; i < NUMBER_OF_SPINNER_LIST_3WIDE_ITEMS && displayed_item_index[i] != NONE; i++)
	{
		struct widget_instance *list_item = widget_instance_get_nth_child(widget, i);
		struct widget_instance *map_name = list_item->first_child;
		struct widget_instance *map_picture = map_name->next;
		struct widget_instance *map_description = map_picture->next;









		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2514, ui_widget_definition_get(list_item->tag_index)->child_widgets.count, "expected 3 children in solo level list item (name, pic, desc)");

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2516, ui_widget_definition_get(map_name->tag_index)->type == _ui_widget_type_text_box, "expected a text box widget for the list item's first child (map name)");

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2518, ui_widget_definition_get(map_picture->tag_index)->type == _ui_widget_type_container, "expected a container widget for the list item's second child (map pic)");

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2520, ui_widget_definition_get(map_description->tag_index)->type == _ui_widget_type_text_box, "expected a text box widget for the list item's third child (map description)");
		map_name->text_box.string_list_index = (short)displayed_item_index[i];
		map_picture->bitmap_frame_index = (short)displayed_item_index[i];
		map_description->text_box.string_list_index = (short)displayed_item_index[i];
	}

	return;
}

static void get_active_player_profile_display_name(
	struct widget_instance *widget)
{
	struct player_profile profile;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2540, widget->type == _ui_widget_type_text_box, "expected a text box widget for profile display name");

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2542, widget->local_player_index >= 0 && widget->local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS, "profile display name requires a valid local player index");
	player_ui_get_active_player_profile(widget->local_player_index, &profile);
	widget->text_box.string = ui_widget_realloc(widget->text_box.string, (MAXIMUM_PLAYER_PROFILE_NAME_LENGTH + 1) * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(2544));

	if (widget->text_box.string)
	{
		ustrncpy(widget->text_box.string, profile.player_name, MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
		widget->text_box.string[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = L'\0';
	}

	return;
}

static void get_editable_player_profile_display_name(
	struct widget_instance *widget)
{
	struct player_profile *profile;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2558, widget->type == _ui_widget_type_text_box, "expected a text box widget for profile display name");

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2560, widget->local_player_index >= 0 && widget->local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS, "profile display name requires a valid local player index");
	profile = player_ui_get_edit_player_profile();

	if (profile)
	{
		widget->text_box.string = ui_widget_realloc(widget->text_box.string, (MAXIMUM_PLAYER_PROFILE_NAME_LENGTH + 1) * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(2564));

		if (widget->text_box.string)
		{
			ustrncpy(widget->text_box.string, profile->player_name, MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
			widget->text_box.string[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = L'\0';
		}
	}

	return;
}

static void get_editable_playlist_profile_display_name(
	struct widget_instance *widget)
{
	struct game_variant *variant;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2579, widget->type == _ui_widget_type_text_box, "expected a text box widget for profile display name");

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2581, widget->local_player_index >= 0 && widget->local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS, "profile display name requires a valid local player index");
	variant = player_ui_get_edit_playlist_profile();

	if (variant)
	{
		widget->text_box.string = ui_widget_realloc(widget->text_box.string, (MAXIMUM_PLAYER_PROFILE_NAME_LENGTH + 1) * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(2585));

		if (widget->text_box.string)
		{
			ustrncpy(widget->text_box.string, variant->human_readable_game_description, MAXIMUM_GAME_VARIANT_NAME_LENGTH - 1);
			widget->text_box.string[MAXIMUM_GAME_VARIANT_NAME_LENGTH - 1] = L'\0';
		}
	}
	else
	{
		error(_error_silent, "not currently editing a game variant");
	}

	return;
}

static void get_active_player_profile_color_index(
	struct widget_instance *widget)
{
	struct player_profile profile;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2605, widget->local_player_index >= 0 && widget->local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS, "profile color picture requires a valid local player index");
	player_ui_get_active_player_profile(widget->local_player_index, &profile);
	widget->bitmap_frame_index = PIN(profile.primary_color_index, 0, player_profile_number_of_available_primary_colors() - 1);

	return;
}

static void multiplayer_game_set_text_box_for_map_name(
	struct widget_instance *widget)
{
	struct network_game_data *game;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2617, widget->type == _ui_widget_type_text_box, "expected text box widget for mp game settings text");
	game = network_game_get_game();

	if (game)
	{
		if (strstr(game->map.name, "beavercreek"))
		{
			widget->text_box.string_list_index = _mp_map_beavercreek;
		}
		else if (strstr(game->map.name, "sidewinder"))
		{
			widget->text_box.string_list_index = _mp_map_sidewinder;
		}
		else if (strstr(game->map.name, "damnation"))
		{
			widget->text_box.string_list_index = _mp_map_damnation;
		}
		else if (strstr(game->map.name, "ratrace"))
		{
			widget->text_box.string_list_index = _mp_map_ratrace;
		}
		else if (strstr(game->map.name, "prisoner"))
		{
			widget->text_box.string_list_index = _mp_map_prisoner;
		}
		else if (strstr(game->map.name, "hangemhigh"))
		{
			widget->text_box.string_list_index = _mp_map_hangemhigh;
		}
		else if (strstr(game->map.name, "chillout"))
		{
			widget->text_box.string_list_index = _mp_map_chillout;
		}
		else if (strstr(game->map.name, "carousel"))
		{
			widget->text_box.string_list_index = _mp_map_carousel;
		}
		else if (strstr(game->map.name, "boardingaction"))
		{
			widget->text_box.string_list_index = _mp_map_boardingaction;
		}
		else if (strstr(game->map.name, "bloodgulch"))
		{
			widget->text_box.string_list_index = _mp_map_bloodgulch;
		}
		else if (strstr(game->map.name, "wizard"))
		{
			widget->text_box.string_list_index = _mp_map_wizard;
		}
		else if (strstr(game->map.name, "putput"))
		{
			widget->text_box.string_list_index = _mp_map_putput;
		}
		else if (strstr(game->map.name, "longest"))
		{
			widget->text_box.string_list_index = _mp_map_longest;
		}
		else
		{
			widget->text_box.string_list_index = _mp_map_unknown;
		}
	}
	else
	{
		error(_error_silent, "no network game");
	}

	return;
}

static void multiplayer_game_set_text_box_for_game_ruleset(
	struct widget_instance *widget)
{
	struct network_game_data *game;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2649, widget->type == _ui_widget_type_text_box, "expected text box widget for mp game settings text");
	game = network_game_get_game();

	if (game)
	{
		switch (game->variant.game_engine_index)
		{
		case _game_engine_ctf:
			if (game->variant.game_engine_variant.ctf.assault == TRUE)
			{
				if (game->variant.game_engine_variant.ctf.single_flag_time)
				{
					widget->text_box.string_list_index = _string_single_flag_assault;
				}
				else
				{
					widget->text_box.string_list_index = _string_assault;
				}
			}
			else
			{
				if (game->variant.game_engine_variant.ctf.single_flag_time)
				{
					widget->text_box.string_list_index = _string_single_flag_ctf;
				}
				else
				{
					widget->text_box.string_list_index = _string_capture_the_flag;
				}
			}
			break;
		case _game_engine_slayer:
			widget->text_box.string_list_index = _string_slayer;
			break;
		case _game_engine_oddball:
			switch (game->variant.game_engine_variant.oddball.oddball_ball_type)
			{
			case _oddball_magic:
				widget->text_box.string_list_index = _string_reverse_tag;
				break;
			case _oddball_terminator:
				widget->text_box.string_list_index = _string_juggernaut;
				break;
			default:
				widget->text_box.string_list_index = _string_oddball;
				break;
			}
			break;
		case _game_engine_king:
			widget->text_box.string_list_index = _string_king_of_the_hill;
			break;
		case _game_engine_race:
			switch (game->variant.game_engine_variant.race.race_type)
			{
			case _race_type_flag_rally:
				widget->text_box.string_list_index = _string_rally;
				break;
			default:
				widget->text_box.string_list_index = _string_race;
				break;
			}
			break;
		default:
			widget->text_box.string_list_index = _string_unknown_game_type;
			break;
		}
	}
	else
	{
		error(_error_silent, "no network game");
	}

	return;
}

static void multiplayer_game_set_text_box_for_teams_noteams(
	struct widget_instance *widget)
{
	struct network_game_data *game;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2730, widget->type == _ui_widget_type_text_box, "expected text box widget for mp game settings text");
	game = network_game_get_game();

	if (game)
	{
		if (game->variant.universal_variant.teams == TRUE)
		{
			widget->text_box.string_list_index = _string_team_game;
		}
		else
		{
			widget->text_box.string_list_index = _string_free_for_all;
		}
	}
	else
	{
		error(_error_silent, "no network game");
	}

	return;
}

static void multiplayer_game_set_text_box_for_score_limit(
	struct widget_instance *widget)
{
	struct network_game_data *game;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2755, widget->type == _ui_widget_type_text_box, "expected text box widget for mp game settings text");
	game = network_game_get_game();

	if (game)
	{
		widget->text_box.string = ui_widget_realloc(widget->text_box.string, 8 * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(2760));

		if (widget->text_box.string)
		{
			usnprintf(widget->text_box.string, 7, L"%d", game->variant.universal_variant.score_to_win);
			widget->text_box.string[7] = L'\0';
		}
	}
	else
	{
		error(_error_silent, "no network game");
	}

	return;
}

static void multiplayer_game_set_text_box_for_score_limit_type(
	struct widget_instance *widget)
{
	struct network_game_data *game;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2780, widget->type == _ui_widget_type_text_box, "expected text box widget for mp game settings text");
	game = network_game_get_game();

	if (game)
	{
		switch (game->variant.game_engine_index)
		{
		case _game_engine_ctf:
			widget->text_box.string_list_index = _string_captures;
			break;
		case _game_engine_slayer:
			widget->text_box.string_list_index = _string_frags;
			break;
		case _game_engine_oddball:
			if (game->variant.game_engine_variant.oddball.oddball_ball_type == _oddball_terminator)
			{
				widget->text_box.string_list_index = _string_frags;
			}
			else
			{
				widget->text_box.string_list_index = _string_minutes;
			}
			break;
		case _game_engine_king:
			widget->text_box.string_list_index = _string_minutes;
			break;
		case _game_engine_race:
			widget->text_box.string_list_index = _string_laps;
			break;
		default:
			widget->text_box.string_list_index = _string_frags;
			break;
		}
	}
	else
	{
		error(_error_silent, "no network game");
	}

	return;
}

static void multiplayer_game_set_bitmap_for_map(
	struct widget_instance *widget)
{
	struct network_game_data *game;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2825, widget->type == _ui_widget_type_container, "expected container widget for mp game settings bitmap");
	game = network_game_get_game();

	if (game)
	{
		if (strstr(game->map.name, "beavercreek"))
		{
			widget->bitmap_frame_index = _mp_map_beavercreek;
		}
		else if (strstr(game->map.name, "sidewinder"))
		{
			widget->bitmap_frame_index = _mp_map_sidewinder;
		}
		else if (strstr(game->map.name, "damnation"))
		{
			widget->bitmap_frame_index = _mp_map_damnation;
		}
		else if (strstr(game->map.name, "ratrace"))
		{
			widget->bitmap_frame_index = _mp_map_ratrace;
		}
		else if (strstr(game->map.name, "prisoner"))
		{
			widget->bitmap_frame_index = _mp_map_prisoner;
		}
		else if (strstr(game->map.name, "hangemhigh"))
		{
			widget->bitmap_frame_index = _mp_map_hangemhigh;
		}
		else if (strstr(game->map.name, "chillout"))
		{
			widget->bitmap_frame_index = _mp_map_chillout;
		}
		else if (strstr(game->map.name, "carousel"))
		{
			widget->bitmap_frame_index = _mp_map_carousel;
		}
		else if (strstr(game->map.name, "boardingaction"))
		{
			widget->bitmap_frame_index = _mp_map_boardingaction;
		}
		else if (strstr(game->map.name, "bloodgulch"))
		{
			widget->bitmap_frame_index = _mp_map_bloodgulch;
		}
		else if (strstr(game->map.name, "wizard"))
		{
			widget->bitmap_frame_index = _mp_map_wizard;
		}
		else if (strstr(game->map.name, "putput"))
		{
			widget->bitmap_frame_index = _mp_map_putput;
		}
		else if (strstr(game->map.name, "longest"))
		{
			widget->bitmap_frame_index = _mp_map_longest;
		}
		else
		{
			widget->bitmap_frame_index = _mp_map_unknown;
		}
	}
	else
	{
		error(_error_silent, "no network game");
	}

	return;
}

static void multiplayer_game_set_bitmap_for_ruleset(
	struct widget_instance *widget)
{
	struct network_game_data *game;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2856, widget->type == _ui_widget_type_container, "expected container widget for mp game settings bitmap");
	game = network_game_get_game();

	if (game)
	{
		switch (game->variant.game_engine_index)
		{
		case _game_engine_ctf:
			widget->bitmap_frame_index = _multiplayer_game_type_ctf_bitmap;
			break;
		case _game_engine_slayer:
			widget->bitmap_frame_index = _multiplayer_game_type_slayer_bitmap;
			break;
		case _game_engine_oddball:
			widget->bitmap_frame_index = _multiplayer_game_type_oddball_bitmap;
			break;
		case _game_engine_king:
			widget->bitmap_frame_index = _multiplayer_game_type_koth_bitmap;
			break;
		case _game_engine_race:
			widget->bitmap_frame_index = _multiplayer_game_type_race_bitmap;
			break;
		default:
			widget->bitmap_frame_index = _multiplayer_game_type_unknown_bitmap;
			break;
		}
	}
	else
	{
		error(_error_silent, "no network game");
	}

	return;
}

static void multiplayer_game_set_text_box_for_number_of_players(
	struct widget_instance *widget)
{
	struct network_game_data *game;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2894, widget->type == _ui_widget_type_text_box, "expected text box widget for mp game settings text");
	game = network_game_get_game();

	if (game)
	{
		widget->text_box.string = ui_widget_realloc(widget->text_box.string, NUMBER_OF_PLAYERS_STRING_SIZE * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c"), MATCH_LINE(2902));

		if (widget->text_box.string)
		{
			usnprintf(widget->text_box.string, NUMBER_OF_PLAYERS_STRING_SIZE - 1, L"%d", game->player_count);
			widget->text_box.string[NUMBER_OF_PLAYERS_STRING_SIZE - 1] = L'\0';
		}
	}
	else
	{
		error(_error_silent, "no network game");
	}

	return;
}

static void multiplayer_edit_profile_set_ruleset_textbox_string_index(
	struct widget_instance *widget)
{
	struct game_variant *variant;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2921, widget->type == _ui_widget_type_text_box, "expected a text box widget for mp profile edit ruleset text widget");
	variant = player_ui_get_edit_playlist_profile();

	if (variant)
	{
		switch (variant->game_engine_index)
		{
		case _game_engine_ctf:
			widget->text_box.string_list_index = _string_capture_the_flag;
			break;
		case _game_engine_slayer:
			widget->text_box.string_list_index = _string_slayer;
			break;
		case _game_engine_oddball:
			widget->text_box.string_list_index = _string_oddball;
			break;
		case _game_engine_king:
			widget->text_box.string_list_index = _string_king_of_the_hill;
			break;
		case _game_engine_race:
			widget->text_box.string_list_index = _string_race;
			break;
		default:
			widget->text_box.string_list_index = _string_unknown_game_type;
			break;
		}
	}
	else
	{
		error(_error_silent, "not currently editing a game variant");
	}

	return;
}

static void system_link_status_check(
	struct widget_instance *widget)
{
	if (!transport_network_available() && !network_game_is_splitscreen_local())
	{
		display_error_when_main_menu_loaded(_error_network_connection_lost);
		main_goto_main_menu();
		error(_error_silent, "network connection went down!");
	}

	return;
}

static void multiplayer_game_directions(
	struct widget_instance *widget)
{
	struct network_game_data *game = network_game_get_game();
	struct network_game_server *server = global_network_game_server_get();
	boolean waiting_for_another_xbox = FALSE;
	boolean waiting_for_another_player = FALSE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 2989, widget->type == _ui_widget_type_text_box, "expected text box widget for team game directions");

	if (server && !network_game_is_splitscreen_local() && game && game->machine_count < 2)
	{
		waiting_for_another_xbox = TRUE;
	}
	else if (server && network_game_is_splitscreen_local() && game && game->player_count < 2)
	{
		waiting_for_another_player = TRUE;
	}

	if (waiting_for_another_xbox)
	{
		widget->text_box.string_list_index = _string_waiting_for_another_xbox;
		widget->visible = TRUE;
	}
	else if (waiting_for_another_player)
	{
		widget->text_box.string_list_index = _string_waiting_for_another_player;
		widget->visible = TRUE;
	}
	else if (game && game->variant.universal_variant.teams == TRUE && network_game_client_get_seconds_to_game_start(global_network_game_client_get()) < 0)
	{
		long player_index;
		long red_team_count = 0;
		long blue_team_count = 0;

		for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
		{
			if (network_player_is_valid(&game->players[player_index]))
			{
				switch (game->players[player_index].team_index)
				{
				case _team_red:
					red_team_count++;
					break;
				case _team_blue:
					blue_team_count++;
					break;
				}
			}
		}

		if (red_team_count && blue_team_count)
		{
			widget->text_box.string_list_index = _string_choose_teams_using_dpad;
		}
		else
		{
			widget->text_box.string_list_index = _string_need_two_teams;
		}

		widget->visible = TRUE;
	}
	else
	{
		widget->visible = FALSE;
	}

	return;
}

static void teams_no_teams_mp_game_bitmap_update(
	struct widget_instance *widget)
{
	struct network_game_data *game = network_game_get_game();

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 3068, widget->type == _ui_widget_type_container, "expected a container bitmap for mp pregame header widget");

	if (game)
	{
		if (game->variant.universal_variant.teams == TRUE)
		{
			widget->bitmap_frame_index = _mp_pregame_header_teams_bitmap;
		}
		else
		{
			widget->bitmap_frame_index = _mp_pregame_header_no_teams_bitmap;
		}
	}

	return;
}

static void warn_if_difficulty_will_nuke_saved_game(
	struct widget_instance *widget)
{
	struct widget_instance *warning_text;
	struct widget_instance *difficulty_list = widget->first_child;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 3103, difficulty_list && difficulty_list->type == _ui_widget_type_column_list, "this doesn't look like the difficulty list widget");
	warning_text = difficulty_list->next;


	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 3107, warning_text && warning_text->type == _ui_widget_type_text_box, "this doesn't look like the difficulty list widget (expected warning text box)");

	if (persistant_game_data_info.valid == TRUE && !_stricmp(persistant_game_data_info.map_name, main_get_map_name()))
	{
		warning_text->visible = difficulty_list->list.item_index != persistant_game_data_info.difficulty;
	}
	else
	{
		warning_text->visible = FALSE;
	}

	return;
}

static void dim_if_no_system_link_cable(
	struct widget_instance *widget)
{
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 3132, widget->type == _ui_widget_type_text_box, "expected a text box widget for system link menu text item");

	if (transport_network_available())
	{
		widget->alpha_modifier = 1.f;
	}
	else
	{
		widget->alpha_modifier = 0.333f;
	}

	return;
}

/* ---------- private code */

static void spinner_list_3wide_determine_displayed_item_indices(
	struct widget_instance *widget,
	long *displayed_item_index)
{
	if (widget->focused_child == widget->first_child)
	{
		displayed_item_index[_spinner_list_3wide_left] = widget->list.item_index;
		displayed_item_index[_spinner_list_3wide_center] = displayed_item_index[_spinner_list_3wide_left] + 1;

		if (displayed_item_index[_spinner_list_3wide_center] == widget->list.item_count)
		{
			displayed_item_index[_spinner_list_3wide_center] = 0;
		}

		displayed_item_index[_spinner_list_3wide_right] = displayed_item_index[_spinner_list_3wide_center] + 1;

		if (displayed_item_index[_spinner_list_3wide_right] == widget->list.item_count)
		{
			displayed_item_index[_spinner_list_3wide_right] = 0;
		}
	}
	else if (widget->focused_child == widget->first_child->next)
	{
		displayed_item_index[_spinner_list_3wide_center] = widget->list.item_index;
		displayed_item_index[_spinner_list_3wide_left] = displayed_item_index[_spinner_list_3wide_center] - 1;

		if (displayed_item_index[_spinner_list_3wide_left] < 0)
		{
			displayed_item_index[_spinner_list_3wide_left] = widget->list.item_count - 1;
		}

		displayed_item_index[_spinner_list_3wide_right] = displayed_item_index[_spinner_list_3wide_center] + 1;

		if (displayed_item_index[_spinner_list_3wide_right] == widget->list.item_count)
		{
			displayed_item_index[_spinner_list_3wide_right] = 0;
		}
	}
	else
	{
		displayed_item_index[_spinner_list_3wide_right] = widget->list.item_index;
		displayed_item_index[_spinner_list_3wide_center] = displayed_item_index[_spinner_list_3wide_right] - 1;

		if (displayed_item_index[_spinner_list_3wide_center] < 0)
		{
			displayed_item_index[_spinner_list_3wide_center] = widget->list.item_count - 1;
		}

		displayed_item_index[_spinner_list_3wide_left] = displayed_item_index[_spinner_list_3wide_center] - 1;

		if (displayed_item_index[_spinner_list_3wide_left] < 0)
		{
			displayed_item_index[_spinner_list_3wide_left] = widget->list.item_count - 1;
		}
	}

	if (displayed_item_index[_spinner_list_3wide_left] >= widget->list.item_count)
	{
		displayed_item_index[_spinner_list_3wide_left] = NONE;
	}

	if (displayed_item_index[_spinner_list_3wide_center] >= widget->list.item_count)
	{
		displayed_item_index[_spinner_list_3wide_center] = NONE;
	}

	if (displayed_item_index[_spinner_list_3wide_right] >= widget->list.item_count)
	{
		displayed_item_index[_spinner_list_3wide_right] = NONE;
	}

	return;
}

static void player_profile_update_cache_for_nwide_list(
	long *profile_indices,
	long profile_count)
{
	long i;
	long j;
	boolean cached_profiles_still_in_use[NUMBER_OF_CACHED_PROFILES] = { FALSE };

	for (i = 0; i < NUMBER_OF_CACHED_PROFILES; i++)
	{
		if (cached_player_profile[i].profile_index != NONE)
		{
			for (j = 0; j < profile_count; j++)
			{
				if (cached_player_profile[i].profile_index == profile_indices[j])
				{
					cached_profiles_still_in_use[i] = TRUE;
					break;
				}
			}
		}
	}

	for (i = 0; i < profile_count; i++)
	{
		if (profile_indices[i] != NONE)
		{
			for (j = 0; j < NUMBER_OF_CACHED_PROFILES && profile_indices[i] != cached_player_profile[j].profile_index; j++);

			if (j == NUMBER_OF_CACHED_PROFILES)
			{
				for (j = 0; j < NUMBER_OF_CACHED_PROFILES && cached_profiles_still_in_use[j] == TRUE; j++)
				{
					match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 3234, j < NUMBER_OF_CACHED_PROFILES, "not enough cache profiles");
				}

				if (player_profile_get(profile_indices[i], &cached_player_profile[j].profile))
				{
					cached_player_profile[j].profile_index = profile_indices[i];
					cached_profiles_still_in_use[j] = TRUE;
				}
				else
				{
					error(_error_silent, "failed to cache player profile");
				}
			}
		}
	}

	return;
}

static void variant_profile_update_cache_for_nwide_list(
	long *profile_indices,
	long profile_count)
{
	long i;
	long j;
	boolean cached_profiles_still_in_use[NUMBER_OF_CACHED_PROFILES] = { FALSE };

	for (i = 0; i < NUMBER_OF_CACHED_PROFILES; i++)
	{
		if (cached_variant_profile[i].profile_index != NONE)
		{
			for (j = 0; j < profile_count; j++)
			{
				if (cached_variant_profile[i].profile_index == profile_indices[j])
				{
					cached_profiles_still_in_use[i] = TRUE;
					break;
				}
			}
		}
	}

	for (i = 0; i < profile_count; i++)
	{
		if (profile_indices[i] != NONE)
		{
			for (j = 0; j < NUMBER_OF_CACHED_PROFILES && profile_indices[i] != cached_variant_profile[j].profile_index; j++);

			if (j == NUMBER_OF_CACHED_PROFILES)
			{
				for (j = 0; j < NUMBER_OF_CACHED_PROFILES && cached_profiles_still_in_use[j] == TRUE; j++)
				{
					match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_game_data_input_functions.c", 3285, j < NUMBER_OF_CACHED_PROFILES, "not enough cache profiles");
				}

				if (playlist_profile_get(profile_indices[i], &cached_variant_profile[j].variant))
				{
					cached_variant_profile[j].profile_index = profile_indices[i];
					cached_profiles_still_in_use[j] = TRUE;
				}
				else
				{
					error(_error_silent, "failed to cache playlist profile");
				}
			}
		}
	}

	return;
}

static int list_indices_sort_proc(
	void const *a,
	void const *b)
{
	int result;
	long index_a = *(long const *)a;
	long index_b = *(long const *)b;

	if (index_a == NONE && index_b != NONE)
	{
		result = 1;
	}
	else if (index_a != NONE && index_b == NONE)
	{
		result = -1;
	}
	else
	{
		result = 0;
	}

	return result;
}

static long filter_invalid_list_indices(
	long *list_indices,
	long count)
{
	long i;

	qsort(list_indices, count, sizeof(*list_indices), list_indices_sort_proc);

	for (i = 0; i < count && list_indices[i] != NONE; i++);

	return i;
}
