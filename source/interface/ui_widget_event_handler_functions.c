/*
UI_WIDGET_EVENT_HANDLER_FUNCTIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ui_widget_event_handler_functions.h"
#include "ui_widget.h"
#include "player_ui.h"
#include "virtual_keyboard.h"
#include "attract_mode.h"
#include "marketing_and_strategic_business_development.h"
#include "ui_widget_game_data_input_functions.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "network_game_manager.h"
#include "rasterizer.h"
#include "input.h"
#include "console.h"
#include "game_state.h"
#include "network_messages.h"
#include "text_group.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "draw_string.h"
#include "network_client_manager.h"
#include "game_engine_list.h"
#include "network_server_manager.h"
#include "playlist_profile.h"
#include "network_connection.h"
#include "event_manager.h"
#include "game.h"
#include "game_engine.h"
#include "players.h"
#include "bungie_net/network/transport.h"

/* ---------- constants */

enum
{
	PLAYER1 = 0,
	PLAYER2
};

enum
{
	MINIMUM_SAVED_GAME_FILES_LIST_SIZE = 3,
	LOCAL_PLAYER_TO_GET_PLAYSTAGE_PROFILES_FROM = 0
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean widget_event_function_null(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean solo_level_initialize_list_single_player(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean solo_level_initialize_list_coop(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean solo_level_dispose_list(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean solo_level_set_next_map_name(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean difficulty_set(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean start_new_game(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean pause_game_restart_at_checkpoint(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean pause_game_restart_level(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean pause_game_quit_to_main_menu(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean clear_multiplayer_player_joins(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_wants_to_join_multiplayer_game(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean network_game_server_list_initialize(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean network_game_start_new_server(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean network_game_join_game_from_server_list(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean network_server_list_dispose(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean network_game_cancel(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean split_screen_game_initialize(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean coop_game_initialize(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean main_menu_initialize(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean multiplayer_type_menu_initialize(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean multiplayer_pick_quick_start_play_stage(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean multiplayer_level_list_initialize(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean multiplayer_level_list_dispose(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean multiplayer_level_select(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean multiplayer_profiles_list_initialize(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean multiplayer_profiles_list_dispose(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean multiplayer_profile_set_for_game(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean multiplayer_game_swap_teams(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean netgame_join_player(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profiles_list_initialize(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profiles_list_dispose(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_set_for_game_3wide(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_set_for_game_1wide(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_begin_editing(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_end_editing(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_set_game_engine(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_change_name(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_change_ctf_rules(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_change_koth_rules(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_change_slayer_rules(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_change_oddball_rules(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_change_racing_rules(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_change_player_options(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_change_item_options(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_change_indicator_options(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_initialize_game_engine(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_initialize_name(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_initialize_ctf_rules(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_initialize_koth_rules(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_initialize_slayer_rules(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_initialize_oddball_rules(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_initialize_racing_rules(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_initialize_player_options(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_initialize_item_options(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_initialize_indicator_options(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean playlist_profile_save_changes(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_color_picker_menu_initialize(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_color_picker_menu_dispose(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_color_picker_select_color(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_begin_editing(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_end_editing(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_change_name(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_save_changes(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_initialize_controller_settings(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_initialize_advanced_controller_settings(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_change_controller_settings(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean player_profile_change_advanced_controller_settings(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean network_game_remove_local_player(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean switch_from_main_menu_to_single_player(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean delete_player_profile_request(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean delete_playlist_profile_request(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean delete_player_profile_final(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean delete_playlist_profile_final(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean cancel_profile_delete(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean create_and_begin_editing_new_gametype_profile(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean create_and_begin_editing_new_player_profile(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean network_game_start_faster(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean network_game_start_slower(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean network_game_server_accept_connections(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean network_game_server_defer_game_start(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean network_game_server_allow_game_start(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean disable_widget_if_no_xdemos(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean run_xdemos(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean single_player_reset_controller_choices(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean single_player_set_player1_controller_choice(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean single_player_set_player2_controller_choice(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean display_error_if_no_network_connection(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean start_network_game_if_no_advertised_servers(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean netgame_unjoin_player(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean close_calling_widget_if_not_editing_profile(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean exit_to_xbox_dashboard(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean new_campaign_chosen(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean new_campaign_decision(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean go_back_twice_next_time(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean difficulty_menu_initialize(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean begin_music_fade_out(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
static boolean new_campaign_if_no_custom_player_profiles_exist(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);

/* ---------- globals */

static boolean (*event_handler_function_table[NUMBER_OF_EVENT_HANDLER_FUNCTIONS])(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted) = /* fake name */
{
	widget_event_function_null,
	widget_event_function_list_widget_goto_next_item,
	widget_event_function_list_widget_goto_previous_item,
	widget_event_function_null,
	widget_event_function_null,
	solo_level_initialize_list_single_player,
	solo_level_initialize_list_coop,
	solo_level_dispose_list,
	solo_level_set_next_map_name,
	difficulty_set,
	start_new_game,
	pause_game_restart_at_checkpoint,
	pause_game_restart_level,
	pause_game_quit_to_main_menu,
	clear_multiplayer_player_joins,
	player_wants_to_join_multiplayer_game,
	network_game_server_list_initialize,
	network_game_start_new_server,
	network_server_list_dispose,
	network_game_cancel,
	network_game_join_game_from_server_list,
	split_screen_game_initialize,
	coop_game_initialize,
	main_menu_initialize,
	multiplayer_type_menu_initialize,
	multiplayer_pick_quick_start_play_stage,
	multiplayer_level_list_initialize,
	multiplayer_level_list_dispose,
	multiplayer_level_select,
	multiplayer_profiles_list_initialize,
	multiplayer_profiles_list_dispose,
	multiplayer_profile_set_for_game,
	multiplayer_game_swap_teams,
	netgame_join_player,
	player_profiles_list_initialize,
	player_profiles_list_dispose,
	player_profile_set_for_game_3wide,
	player_profile_set_for_game_1wide,
	playlist_profile_begin_editing,
	playlist_profile_end_editing,
	playlist_profile_set_game_engine,
	playlist_profile_change_name,
	playlist_profile_change_ctf_rules,
	playlist_profile_change_koth_rules,
	playlist_profile_change_slayer_rules,
	playlist_profile_change_oddball_rules,
	playlist_profile_change_racing_rules,
	playlist_profile_change_player_options,
	playlist_profile_change_item_options,
	playlist_profile_change_indicator_options,
	playlist_profile_initialize_game_engine,
	playlist_profile_initialize_name,
	playlist_profile_initialize_ctf_rules,
	playlist_profile_initialize_koth_rules,
	playlist_profile_initialize_slayer_rules,
	playlist_profile_initialize_oddball_rules,
	playlist_profile_initialize_racing_rules,
	playlist_profile_initialize_player_options,
	playlist_profile_initialize_item_options,
	playlist_profile_initialize_indicator_options,
	playlist_profile_save_changes,
	player_profile_color_picker_menu_initialize,
	player_profile_color_picker_menu_dispose,
	player_profile_color_picker_select_color,
	player_profile_begin_editing,
	player_profile_end_editing,
	player_profile_change_name,
	player_profile_save_changes,
	player_profile_initialize_controller_settings,
	player_profile_initialize_advanced_controller_settings,
	player_profile_change_controller_settings,
	player_profile_change_advanced_controller_settings,
	network_game_remove_local_player,
	switch_from_main_menu_to_single_player,
	delete_player_profile_request,
	delete_playlist_profile_request,
	delete_player_profile_final,
	delete_playlist_profile_final,
	cancel_profile_delete,
	create_and_begin_editing_new_gametype_profile,
	create_and_begin_editing_new_player_profile,
	network_game_start_faster,
	network_game_start_slower,
	network_game_server_accept_connections,
	network_game_server_defer_game_start,
	network_game_server_allow_game_start,
	disable_widget_if_no_xdemos,
	run_xdemos,
	single_player_reset_controller_choices,
	single_player_set_player1_controller_choice,
	single_player_set_player2_controller_choice,
	display_error_if_no_network_connection,
	start_network_game_if_no_advertised_servers,
	netgame_unjoin_player,
	close_calling_widget_if_not_editing_profile,
	exit_to_xbox_dashboard,
	new_campaign_chosen,
	new_campaign_decision,
	go_back_twice_next_time,
	difficulty_menu_initialize,
	begin_music_fade_out,
	new_campaign_if_no_custom_player_profiles_exist,
};

static char *event_handler_functions_strings[NUMBER_OF_EVENT_HANDLER_FUNCTIONS] =
{
	"NULL",
	"list goto next item",
	"list goto previous item",
	"unused",
	"unused",
	"initialize sp level list solo",
	"initialize sp level list coop",
	"dispose sp level list",
	"solo level set map",
	"set difficulty",
	"start new game",
	"pause game restart at checkpoint",
	"pause game restart level",
	"pause game return to main menu",
	"clear multiplayer player joins",
	"join controller to mp game",
	"initialize net game server list",
	"start network game server",
	"dispose net game server list",
	"shutdown network game",
	"net game join from server list",
	"split screen game initialize",
	"coop game initialize",
	"main menu intialize",
	"mp type menu initialize",
	"pick play stage for quick start",
	"mp level list initialize",
	"mp level list dispose",
	"mp level select",
	"mp profiles list initialize",
	"mp profiles list dispose",
	"mp profile set for game",
	"swap player team",
	"net game join player",
	"player profile list initialize",
	"player profile list dispose",
	"3wide plyr prof set for game",
	"1wide plyr prof set for game",
	"mp profile begin editing",
	"mp profile end editing",
	"mp profile set game engine",
	"mp profile change name",
	"mp profile set ctf rules",
	"mp profile set koth rules",
	"mp profile set slayer rules",
	"mp profile set oddball rules",
	"mp profile set racing rules",
	"mp profile set player options",
	"mp profile set item options",
	"mp profile set indicator opts",
	"mp profile init game engine",
	"mp profile init name",
	"mp profile init ctf rules",
	"mp profile init koth rules",
	"mp profile init slayer rules",
	"mp profile init oddball rules",
	"mp profile init racing rules",
	"mp profile init player opts",
	"mp profile init item options",
	"mp profile init indicator opts",
	"mp profile save changes",
	"color picker menu initialize",
	"color picker menu dispose",
	"color picker select color",
	"player profile begin editing",
	"player profile end editing",
	"player profile change name",
	"player profile save changes",
	"plyr prf init cntl settings",
	"plyr prf init adv cntl set",
	"plyr prf save cntl settings",
	"plyr prf save adv cntl set",
	"mp game player quit",
	"main menu switch to solo game",
	"request del player profile",
	"request del playlist profile",
	"final del player profile",
	"final del playlist profile",
	"cancel profile delete",
	"create&edit playlist profile",
	"create&edit player profile",
	"net game speed start",
	"net game delay start",
	"net server accept conx",
	"net server defer start",
	"net server allow start",
	"disable if no xdemos",
	"run xdemos",
	"sp reset controller choices",
	"sp set p1 controller choice",
	"sp set p2 controller choice",
	"error if no network connection",
	"start server if none advertised",
	"net game unjoin player",
	"close if not editing profile",
	"exit to xbox dashboard",
	"new campaign chosen",
	"new campaign decision",
	"pop history stack once",
	"difficulty menu init",
	"begin music fade out",
	"new game if no plyr profiles",
};

struct enum_definition event_handler_functions =
{
	NUMBER_OF_EVENT_HANDLER_FUNCTIONS,
	event_handler_functions_strings
};

static long profile_index_to_delete = NONE; /* fake name */

static char *single_player_level_names[NUMBER_OF_SINGLE_PLAYER_LEVELS] = /* fake name */
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

static long last_player1_profile_index = NONE; /* fake name */
static long corrupted_saved_game_profile_index = NONE; /* fake name */

static char *multiplayer_level_names[NUMBER_OF_MULTIPLAYER_LEVELS] = /* fake name */
{
	"levels\\test\\beavercreek\\beavercreek",
	"levels\\test\\sidewinder\\sidewinder",
	"levels\\test\\damnation\\damnation",
	"levels\\test\\ratrace\\ratrace",
	"levels\\test\\prisoner\\prisoner",
	"levels\\test\\hangemhigh\\hangemhigh",
	"levels\\test\\chillout\\chillout",
	"levels\\test\\carousel\\carousel",
	"levels\\test\\boardingaction\\boardingaction",
	"levels\\test\\bloodgulch\\bloodgulch",
	"levels\\test\\wizard\\wizard",
	"levels\\test\\putput\\putput",
	"levels\\test\\longest\\longest",
};

static short new_campaign_controller_index = NONE; /* fake name */

static wchar_t new_campaign_profile_name[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH+1]; /* fake name */
struct _single_player_level_data single_player_level_data[NUMBER_OF_SINGLE_PLAYER_LEVELS] = {0};
struct _persistant_game_data_info persistant_game_data_info = {0};

/* ---------- public code */

boolean ui_widget_event_handler_function_invoke(
	struct widget_instance *widget,
	struct event_record *event,
	short function,
	boolean *widget_deleted)
{
	boolean success;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 478, (widget != NULL) && (widget_deleted != NULL), "(widget != NULL) && (widget_deleted != NULL)");

	if (function >= 0 && function < NUMBEROF(event_handler_function_table))
	{
		success = event_handler_function_table[function](widget, event, widget_deleted);

		if (!success)
		{
			console_warning("event handler '%s' failed", event_handler_functions_strings[function]);
		}
	}
	else
	{
		error(_error_silent, "invalid event_handler_function");
		success = FALSE;
	}

	return success;
}

static boolean widget_event_function_null(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	return TRUE;
}

void reset_last_player1_profile_index(
	void)
{
	last_player1_profile_index = NONE;

	return;
}

static boolean solo_level_initialize_list_single_player(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct player_profile profile;
	short last_level_completed;
	short difficulty;
	long level_index;
	struct ui_widget_definition *definition;

	if (player_spawn_count >= 2)
	{
		memset(&persistant_game_data_info, 0, sizeof(persistant_game_data_info));
		solo_level_initialize_list_coop(widget, event, widget_deleted);
	}
	else
	{
		long profile_index = player_ui_get_active_player_profile_index(PLAYER1);

		memset(single_player_level_data, 0, sizeof(single_player_level_data));

		if (profile_index != last_player1_profile_index)
		{
			memset(&persistant_game_data_info, 0, sizeof(persistant_game_data_info));
			persistant_game_data_info.valid = game_state_test_persistent_storage(
				persistant_game_data_info.map_name,
				&persistant_game_data_info.difficulty,
				&persistant_game_data_info.corrupted);
			last_player1_profile_index = profile_index;
		}

		player_ui_get_active_player_profile(PLAYER1, &profile);
		player_profile_get_highest_completed_solo_level(&profile, &last_level_completed, &difficulty);

		for (level_index = 0; level_index < NUMBER_OF_SINGLE_PLAYER_LEVELS; level_index++)
		{
			single_player_level_data[level_index].scenario_tag_path = single_player_level_names[level_index];

			if (profile.single_player_map_flags[level_index] || level_index == last_level_completed + 1 || level_index == _single_player_map_a10)
			{
				single_player_level_data[level_index].level_completed[_game_difficulty_level_easy] = TRUE;
				single_player_level_data[level_index].level_completed[_game_difficulty_level_normal] = TEST_FLAG(profile.single_player_map_flags[level_index], _game_difficulty_level_normal);
				single_player_level_data[level_index].level_completed[_game_difficulty_level_hard] = TEST_FLAG(profile.single_player_map_flags[level_index], _game_difficulty_level_hard);
				single_player_level_data[level_index].level_completed[_game_difficulty_level_impossible] = TEST_FLAG(profile.single_player_map_flags[level_index], _game_difficulty_level_impossible);
			}
		}

		definition = ui_widget_definition_get(widget->tag_index);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 603, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'solo level list' widget");
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 604, definition->child_widgets.count == 3, "expected 3 list items for 'solo level list' widget");
		widget->list.items = single_player_level_data;
		widget->list.item_count = NUMBER_OF_SINGLE_PLAYER_LEVELS;
		widget->list.item_index = PIN(player_ui_get_last_single_player_level_played(PLAYER1), 0, NUMBER_OF_SINGLE_PLAYER_LEVELS - 1);

		if (persistant_game_data_info.valid == TRUE)
		{
			persistant_game_data_info.map_name[NUMBEROF(persistant_game_data_info.map_name) - 1] = '\0';

			for (level_index = 0; level_index < NUMBER_OF_SINGLE_PLAYER_LEVELS; level_index++)
			{
				if (!_stricmp(persistant_game_data_info.map_name, single_player_level_names[level_index]))
				{
					persistant_game_data_info.map_index = (char)level_index;
					persistant_game_data_info.difficulty = PIN(persistant_game_data_info.difficulty, 0, NUMBER_OF_GAME_DIFFICULTY_LEVELS - 1);
					break;
				}
			}

			if (level_index == NUMBER_OF_SINGLE_PLAYER_LEVELS)
			{
				persistant_game_data_info.valid = FALSE;
			}
		}
		else if (persistant_game_data_info.corrupted == TRUE)
		{
			long active_profile_index = player_ui_get_active_player_profile_index(PLAYER1);

			if (active_profile_index != NONE)
			{
				if (corrupted_saved_game_profile_index == NONE)
				{
					display_error_deferred(_error_saved_game_state_corrupt, NONE, TRUE, FALSE);
					corrupted_saved_game_profile_index = active_profile_index;
				}
				else
				{
					corrupted_saved_game_profile_index = NONE;
				}
			}
		}
	}

	return TRUE;
}

static boolean solo_level_initialize_list_coop(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct player_profile profile[2];
	short last_level_completed[2];
	short difficulty[2];
	long level_index;
	struct ui_widget_definition *definition;

	memset(single_player_level_data, 0, sizeof(single_player_level_data));
	player_ui_get_active_player_profile(PLAYER1, &profile[PLAYER1]);
	player_profile_get_highest_completed_solo_level(&profile[PLAYER1], &last_level_completed[PLAYER1], &difficulty[PLAYER1]);
	player_ui_get_active_player_profile(PLAYER2, &profile[PLAYER2]);
	player_profile_get_highest_completed_solo_level(&profile[PLAYER2], &last_level_completed[PLAYER2], &difficulty[PLAYER2]);

	for (level_index = 0; level_index < NUMBER_OF_SINGLE_PLAYER_LEVELS; level_index++)
	{
		single_player_level_data[level_index].scenario_tag_path = single_player_level_names[level_index];

		if (profile[PLAYER1].single_player_map_flags[level_index] || level_index == last_level_completed[PLAYER1] + 1 ||
			profile[PLAYER2].single_player_map_flags[level_index] || level_index == last_level_completed[PLAYER2] + 1 ||
			level_index == _single_player_map_a10)
		{
			long map_flags = profile[PLAYER1].single_player_map_flags[level_index] | profile[PLAYER2].single_player_map_flags[level_index];

			single_player_level_data[level_index].level_completed[_game_difficulty_level_easy] = TRUE;
			single_player_level_data[level_index].level_completed[_game_difficulty_level_normal] = TEST_FLAG(map_flags, _game_difficulty_level_normal);
			single_player_level_data[level_index].level_completed[_game_difficulty_level_hard] = TEST_FLAG(map_flags, _game_difficulty_level_hard);
			single_player_level_data[level_index].level_completed[_game_difficulty_level_impossible] = TEST_FLAG(map_flags, _game_difficulty_level_impossible);
		}
	}

	definition = ui_widget_definition_get(widget->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 689, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'solo level list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 690, definition->child_widgets.count == 3, "expected 3 list items for 'solo level list' widget");
	widget->list.items = single_player_level_data;
	widget->list.item_count = NUMBER_OF_SINGLE_PLAYER_LEVELS;
	widget->list.item_index = PIN(player_ui_get_last_single_player_level_played(PLAYER1), 0, NUMBER_OF_SINGLE_PLAYER_LEVELS - 1);

	return TRUE;
}

static boolean solo_level_dispose_list(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	memset(single_player_level_data, 0, sizeof(single_player_level_data));
	widget->list.items = NULL;
	widget->list.item_count = 0;

	return TRUE;
}

static boolean solo_level_set_next_map_name(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct player_profile profile;
	short last_level_completed;
	short difficulty;
	short local_player_index;
	boolean level_available = FALSE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 724, (widget->list.item_index>=0) && (widget->list.item_index<NUMBER_OF_SINGLE_PLAYER_LEVELS), "I don't think this is the solo level list widget");

	switch (player_spawn_count)
	{
	case 1:
		player_ui_get_active_player_profile(PLAYER1, &profile);
		player_profile_get_highest_completed_solo_level(&profile, &last_level_completed, &difficulty);

		if (profile.single_player_map_flags[widget->list.item_index] || widget->list.item_index == last_level_completed + 1 || widget->list.item_index == 0)
		{
			level_available = TRUE;
		}

		player_ui_remember_player1_profile(FALSE);
	case 2:
		for (local_player_index = PLAYER1; local_player_index <= PLAYER2; local_player_index++)
		{
			player_ui_get_active_player_profile(local_player_index, &profile);
			player_profile_get_highest_completed_solo_level(&profile, &last_level_completed, &difficulty);

			if (profile.single_player_map_flags[widget->list.item_index] || widget->list.item_index == last_level_completed + 1 || widget->list.item_index == 0)
			{
				level_available = TRUE;
				break;
			}
		}
		break;
	default:
		error(_error_silent, "invalid player count for single player game");
		break;
	}

	if (level_available == TRUE)
	{
		main_set_map_name(single_player_level_names[widget->list.item_index]);
		main_defer_map_map_change();
	}
	else
	{
		error(_error_silent, "this level is unavailable to you!");
		ui_play_audio_feedback_sound(_audio_feedback_failure);
	}

	return level_available;
}

static boolean difficulty_set(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 787, (widget->list.item_index>=0) && (widget->list.item_index<NUMBER_OF_GAME_DIFFICULTY_LEVELS), "I don't think this is the difficulty list widget");
	main_set_difficulty(widget->list.item_index);
	ui_play_audio_feedback_sound(_audio_feedback_forward);

	return TRUE;
}

static boolean start_new_game(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	main_set_difficulty(_game_difficulty_level_normal);
	main_set_map_name(single_player_level_names[_single_player_map_a10]);
	game_connection_set(_game_connection_local);
	main_menu_switch_to_single_player();
	player_ui_remember_player1_profile(FALSE);

	return TRUE;
}

static boolean pause_game_restart_at_checkpoint(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	main_revert_map();

	return TRUE;
}

static boolean pause_game_restart_level(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	main_reset_map();

	return TRUE;
}

static boolean pause_game_quit_to_main_menu(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	game_state_save_to_persistent_storage();
	main_goto_main_menu();

	return TRUE;
}

static boolean clear_multiplayer_player_joins(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	dispose_global_network_game_client();
	dispose_global_network_game_server();
	player_ui_clear_multiplayer_joins();
	player_ui_clear_multiplayer_variant();

	return TRUE;
}

static boolean player_wants_to_join_multiplayer_game(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 873, widget->local_player_index!=NONE, "need a specific local player index when joining a multiplayer game");
	player_ui_local_player_joined_multiplayer_game(widget->local_player_index);

	return TRUE;
}

static boolean network_game_server_list_initialize(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	boolean success = TRUE;

	dispose_global_network_game_client();
	dispose_global_network_game_server();
	player_ui_clear_multiplayer_variant();

	if (create_global_network_game_client())
	{
		game_connection_set(_game_connection_network_client);
	}
	else
	{
		error(_error_silent, "failed to create network client to initiate game search");
		success = FALSE;
	}

	return success;
}

static boolean network_game_start_new_server(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	boolean success = TRUE;

	dispose_global_network_game_client();
	player_ui_clear_multiplayer_variant();
	network_game_accept_remote_connections(TRUE);

	if (!global_network_game_server_get())
	{
		game_engine_playlist_initialize();
		success = create_global_network_game_server();

		if (success == TRUE)
		{
			network_game_server_pause_countdown(global_network_game_server_get(), TRUE);
			game_engine_playlist_begin();
			game_connection_set(_game_connection_network_server);
		}
	}

	if (success && !global_network_game_client_get())
	{
		success = create_global_network_game_client();
	}

	if (!success)
	{
		dispose_global_network_game_server();
		dispose_global_network_game_client();
		network_game_accept_remote_connections(FALSE);
		player_ui_clear_multiplayer_variant();
		error(_error_silent, "failed to initiate a multiplayer game server");
	}

	return success;
}

static boolean network_game_join_game_from_server_list(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	boolean success = FALSE;

	if (widget->focused_child && widget->list.item_index >= 0)
	{
		if (widget->list.item_index < widget->list.item_count && widget->list.items)
		{
			if (widget->list.item_count > 0)
			{
				struct advertised_game_data *game = ((struct advertised_game_data **)widget->list.items)[widget->list.item_index];

				if (game->open == TRUE)
				{
					if (!game->platform)
					{
						struct transport_address address = {0};

						transport_client_start(
							&game->host_address,
							&game->key,
							&game->key_id,
							NETWORK_SERVER_PORT,
							&address);

						if (address.address.ipv4_address && address.port)
						{
							struct network_game_join_parameters join_parameters;

							join_parameters.password[0] = L'\0';
							network_game_generate_join_game_token(join_parameters.join_token);

							if (network_game_client_initiate_join_game(global_network_game_client_get(), game, &join_parameters, &address))
							{
								struct widget_instance *topmost_parent = widget_instance_get_topmost_parent(widget);

								if (!ui_widget_load_by_name_or_tag(
									"ui\\shell\\main_menu\\multiplayer_type_select\\connected\\pregame\\connected_pregame_screen",
									NONE,
									NULL,
									NONE,
									topmost_parent->tag_index,
									widget->parent ? widget->parent->tag_index : NONE,
									(short)widget_instance_get_child_index_from_parent(widget)))
								{
									error(_error_silent, "event handler failed to spawn widget");
								}
								else
								{
									game_connection_set(_game_connection_network_client);
									success = TRUE;
								}

								*widget_deleted = TRUE;
							}
							else
							{
								network_game_abort();
								error(_error_silent, "failed to initiate join game procedures");
							}
						}
						else
						{
							error(_error_silent, "attempted to join a network game with a bogus address");
						}
					}
					else
					{
						error(_error_silent, "attempted to join a network game running on a different platform than the local system");
					}
				}
				else
				{
					error(_error_silent, "attempted to join a closed game");
					ui_play_audio_feedback_sound(_audio_feedback_failure);
				}
			}
			else
			{
				error(_error_silent, "unable to join server: there are no servers in the server list (maybe the server list was disposed?)");
			}
		}
		else
		{
			error(_error_silent, "unable to join server: this doesn't look like a valid server list to me... or the server list has been disposed?");
		}
	}

	return success;
}

static boolean network_server_list_dispose(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	widget->list.items = NULL;
	widget->list.item_count = 0;

	return TRUE;
}

static boolean network_game_cancel(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	dispose_global_network_game_server();
	dispose_global_network_game_client();
	player_ui_clear_multiplayer_variant();

	return TRUE;
}

static boolean split_screen_game_initialize(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	boolean success = TRUE;

	network_game_accept_remote_connections(FALSE);

	if (!global_network_game_server_get())
	{
		game_engine_playlist_initialize();
		success = create_global_network_game_server();

		if (success == TRUE)
		{
			game_engine_playlist_begin();
			game_connection_set(_game_connection_network_server);
		}
	}

	if (success && !global_network_game_client_get())
	{
		success = create_global_network_game_client();
	}

	if (!success)
	{
		dispose_global_network_game_server();
		dispose_global_network_game_client();
		player_ui_clear_multiplayer_variant();
		error(_error_silent, "failed to initiate split screen game networking");
	}

	return success;
}

static boolean coop_game_initialize(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	player_spawn_count = 2;

	return TRUE;
}

static boolean main_menu_initialize(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	player_ui_clear_multiplayer_joins();
	player_ui_clear_multiplayer_variant();
	dispose_global_network_game_client();
	dispose_global_network_game_server();
	network_game_accept_remote_connections(FALSE);
	player_spawn_count = 1;
	player_ui_end_editing_profile();

	if (!ui_main_menu_music_active())
	{
		ui_start_main_menu_music();
	}

	return TRUE;
}

static boolean multiplayer_type_menu_initialize(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	player_spawn_count = 1;

	return TRUE;
}

static boolean multiplayer_pick_quick_start_play_stage(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	game_engine_playlist_initialize();
	game_engine_playlist_next(0, 0, 4);
	network_game_set_quickstart_local();

	return TRUE;
}

static boolean multiplayer_level_list_initialize(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	char last_used_map[MAXIMUM_FILENAME_LENGTH+1];
	struct ui_widget_definition *definition = ui_widget_definition_get(widget->tag_index);

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1228, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'multiplayer level list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1229, definition->child_widgets.count == 3, "expected 3 list items for 'multiplayer level list' widget");
	widget->list.items = multiplayer_level_names;
	widget->list.item_count = NUMBER_OF_MULTIPLAYER_LEVELS;

	if (saved_game_file_retrieve_last_used_multiplayer_map(last_used_map))
	{
		for (widget->list.item_index = 0; widget->list.item_index < NUMBER_OF_MULTIPLAYER_LEVELS; widget->list.item_index++)
		{
			if (!_stricmp(last_used_map, multiplayer_level_names[widget->list.item_index]))
			{
				break;
			}
		}

		if (widget->list.item_index == NUMBER_OF_MULTIPLAYER_LEVELS)
		{
			widget->list.item_index = 0;
		}
	}

	return TRUE;
}

static boolean multiplayer_level_list_dispose(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	widget->list.items = NULL;
	widget->list.item_count = 0;

	return TRUE;
}

static boolean multiplayer_level_select(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	char automation_map_name[64];
	struct widget_instance *child;
	struct widget_instance *level_list;
	struct ui_widget_definition *definition;
	struct network_game_server *server;
	char *map_name;
	FILE *automation_file;
	long level_index;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1280, ui_widget_definition_get(widget->tag_index)->child_widgets.count == 1, "expected a wrapper widget around the multiplayer level select screen");
	child = widget->first_child;
	definition = ui_widget_definition_get(child->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1285, definition->type == _ui_widget_type_container && definition->child_widgets.count == 3, "expected the multiplayer level select screen to be a container w/ 3 children");
	child = child->first_child;
	definition = ui_widget_definition_get(child->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1288, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'multiplayer level list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1289, definition->child_widgets.count == 3, "expected 3 list items for 'multiplayer level list' widget");
	level_list = widget->first_child->first_child;
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1298, (level_list->list.item_index>=0) && (level_list->list.item_index<NUMBER_OF_MULTIPLAYER_LEVELS), "invalid multiplayer level specified from 'multiplayer level list' list widget");
	map_name = multiplayer_level_names[level_list->list.item_index];

	automation_file = fopen("d:\\map_automation.txt", "r");

	if (automation_file)
	{
		fgets(automation_map_name, NUMBEROF(automation_map_name), automation_file);
		automation_map_name[NUMBEROF(automation_map_name) - 1] = '\0';
		strtok(automation_map_name, "\n\r \t");
		map_name = automation_map_name;
		fclose(automation_file);
	}

	main_set_multiplayer_map_name(map_name);
	game_engine_override_map_name(map_name);
	server = global_network_game_server_get();

	if (server)
	{
		network_game_server_change_map_name(server, map_name);
	}

	for (level_index = 0; level_index < NUMBER_OF_MULTIPLAYER_LEVELS; level_index++)
	{
		if (!_stricmp(map_name, multiplayer_level_names[level_index]))
		{
			saved_game_file_remember_last_used_multiplayer_map(multiplayer_level_names[level_index]);
			break;
		}
	}

	return TRUE;
}

static boolean multiplayer_profiles_list_initialize(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	char last_variant_profile_directory_path[MAXIMUM_FILENAME_LENGTH+1];
	struct ui_widget_definition *definition;
	long *profile_indices;
	word profile_index;

	profile_index_to_delete = NONE;
	memset(cached_variant_profile, NONE, sizeof(cached_variant_profile));
	definition = ui_widget_definition_get(widget->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1384, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'multiplayer settings list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1385, definition->child_widgets.count == 3, "expected 3 list items for 'multiplayer settings list' widget");

	profile_indices = ui_widget_realloc(widget->list.items, MAXIMUM_ENUMERATED_SAVED_GAME_FILES_ANY_TYPE_PER_MEMORY_UNIT * sizeof(long), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c"), MATCH_LINE(1390));
	widget->list.items = profile_indices;

	if (profile_indices)
	{
		word number_of_profiles = MAXIMUM_ENUMERATED_SAVED_GAME_FILES_ANY_TYPE_PER_MEMORY_UNIT;

		playlist_profiles_enumerate_available_to_local_player_index(
			LOCAL_PLAYER_TO_GET_PLAYSTAGE_PROFILES_FROM,
			&number_of_profiles,
			profile_indices);

		for (profile_index = number_of_profiles; profile_index < MINIMUM_SAVED_GAME_FILES_LIST_SIZE; profile_index++)
		{
			profile_indices[profile_index] = NONE;
			number_of_profiles++;
		}

		widget->list.item_count = number_of_profiles;

		if (saved_game_file_retrieve_last_used_multiplayer_variant_directory(last_variant_profile_directory_path))
		{
			long last_used_profile_index = saved_game_file_find_profile_index_for_directory_path(last_variant_profile_directory_path, _saved_game_file_type_play_list);

			if (last_used_profile_index != NONE)
			{
				for (profile_index = 0; profile_index < number_of_profiles; profile_index++)
				{
					if (profile_indices[profile_index] == last_used_profile_index)
					{
						widget->list.item_index = profile_index;
						break;
					}
				}
			}
		}
	}

	return TRUE;
}

static boolean multiplayer_profiles_list_dispose(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	if (widget->list.items)
	{
		widget_free(widget->list.items);
		widget->list.items = NULL;
	}

	widget->list.item_count = 0;

	return TRUE;
}

static boolean multiplayer_profile_set_for_game(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	char profile_directory_path[MAXIMUM_FILENAME_LENGTH+1];
	struct game_variant automation_variant;
	struct game_variant empty_variant;
	char automation_variant_name[128];
	struct game_variant variant;
	struct widget_instance *child;
	struct ui_widget_definition *definition;
	struct widget_instance *profile_list;
	long profile_index;
	boolean success;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1465, ui_widget_definition_get(widget->tag_index)->child_widgets.count == 1, "expected a wrapper widget around the multiplayer profile select screen");
	child = widget->first_child;
	definition = ui_widget_definition_get(child->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1470, definition->type == _ui_widget_type_container && definition->child_widgets.count == 3, "expected the multiplayer profile select screen to be a container w/ 3 children");
	child = child->first_child;
	definition = ui_widget_definition_get(child->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1473, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'multiplayer profile list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1474, definition->child_widgets.count == 3, "expected 3 list items for 'multiplayer profile list' widget");
	profile_list = widget->first_child->first_child;
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1483, (profile_list->list.item_index>=0) && (profile_list->list.item_index<profile_list->list.item_count), "invalid multiplayer profile specified from 'multiplayer profile list' list widget");
	profile_index = ((long *)profile_list->list.items)[profile_list->list.item_index];

	if (profile_index == NONE)
	{
		ui_play_audio_feedback_sound(_audio_feedback_failure);
		success = FALSE;
	}
	else if (!TEST_FLAG(profile_index, _profile_checksum_valid_bit31))
	{
		display_error_deferred(_error_warning_saved_game_file_damaged, NONE, TRUE, FALSE);
		ui_play_audio_feedback_sound(_audio_feedback_failure);
		success = FALSE;
	}
	else if (playlist_profile_get(profile_index, &variant))
	{
		FILE *automation_file;
		struct network_game_server *server = global_network_game_server_get();

		if (saved_game_file_get_path_to_enclosing_directory(profile_index, profile_directory_path))
		{
			saved_game_file_remember_last_used_multiplayer_variant_directory(profile_directory_path);
		}

		automation_file = fopen("d:\\variant_automation.txt", "r");

		if (automation_file)
		{
			fgets(automation_variant_name, NUMBEROF(automation_variant_name), automation_file);
			automation_variant_name[NUMBEROF(automation_variant_name) - 1] = '\0';
			strtok(automation_variant_name, "\n\r \t");
			memset(&empty_variant, 0, sizeof(empty_variant));
			automation_variant = game_engine_get_variant_by_name(automation_variant_name);

			if (memcmp(&automation_variant, &empty_variant, sizeof(automation_variant)))
			{
				variant = automation_variant;
			}

			fclose(automation_file);
		}

		player_ui_set_game_variant(&variant);

		if (server)
		{
			network_game_server_change_game_variant(server, &variant);
		}

		success = TRUE;
	}
	else
	{
		error(_error_silent, "failed to retrieve user selected game variant");
		success = FALSE;
	}

	return success;
}

static boolean multiplayer_game_swap_teams(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct network_game_data *game;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1572, event);

	game = network_game_get_game();

	if (game && game->variant.universal_variant.teams == TRUE)
	{
		short local_machine_index = network_game_client_get_local_machine_index();

		if (local_machine_index != NONE)
		{
			long player_index;

			for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
			{
				if (network_player_is_valid(&game->players[player_index]) && game->players[player_index].machine_index == local_machine_index && game->players[player_index].controller_index == event->controller_index)
				{
					struct network_player new_player = game->players[player_index];

					new_player.team_index = !new_player.team_index;

					if (!network_game_client_update_local_player_data(global_network_game_client_get(), &new_player))
					{
						error(_error_silent, "failed to update player's team for multiplayer game");
					}
					break;
				}
			}
		}
	}

	return TRUE;
}

static boolean netgame_join_player(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct network_game_client *client;
	short player_index;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1618, event);

	client = global_network_game_client_get();

	if (client && network_game_client_get_state(client, (word *)&player_index) == _network_game_client_state_pregame)
	{
		struct network_game_data *game = network_game_get_game();
		short local_machine_index = network_game_client_get_local_machine_index();
		boolean player_joined = FALSE;

		match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1627, game);

		if (local_machine_index != NONE)
		{
			for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
			{
				if (network_player_is_valid(&game->players[player_index]) && game->players[player_index].machine_index == local_machine_index && game->players[player_index].controller_index == event->controller_index)
				{
					player_joined = TRUE;
					break;
				}
			}
		}

		if (!player_joined && !network_game_client_add_player(client, event->controller_index))
		{
			network_event("failed to send join request");
		}
	}

	return TRUE;
}

static boolean player_profiles_list_initialize(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct ui_widget_definition *definition = ui_widget_definition_get(widget->tag_index);

	profile_index_to_delete = NONE;
	memset(cached_player_profile, NONE, sizeof(cached_player_profile));
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1686, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'player settings list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1688, definition->child_widgets.count == 0 || definition->child_widgets.count == 3, "expected either 1 or 3 list items for 'player settings list' widget");

	widget->list.items = ui_widget_realloc(widget->list.items, MAXIMUM_ENUMERATED_SAVED_GAME_FILES_ANY_TYPE_PER_MEMORY_UNIT * sizeof(long), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c"), MATCH_LINE(1693));

	if (widget->list.items)
	{
		long last_used_profile_index;
		word number_of_profiles = MAXIMUM_ENUMERATED_SAVED_GAME_FILES_ANY_TYPE_PER_MEMORY_UNIT;
		boolean include_default_profiles = TRUE;

		if (definition->child_widgets.count == 3)
		{
			include_default_profiles = FALSE;
		}

		player_profiles_enumerate_available_to_local_player_index(
			widget->local_player_index,
			&number_of_profiles,
			widget->list.items,
			include_default_profiles);

		if (definition->child_widgets.count == 3)
		{
			word empty_index;

			for (empty_index = number_of_profiles; empty_index < MINIMUM_SAVED_GAME_FILES_LIST_SIZE; empty_index++)
			{
				((long *)widget->list.items)[empty_index] = NONE;
				number_of_profiles++;
			}
		}

		widget->list.item_count = number_of_profiles;
		last_used_profile_index = player_ui_get_player1_last_used_profile_index();

		if (last_used_profile_index != NONE)
		{
			long profile_index;
			long *profile_indices = widget->list.items;

			for (profile_index = 0; profile_index < widget->list.item_count; profile_index++)
			{
				if (profile_indices[profile_index] == last_used_profile_index)
				{
					widget->list.item_index = (short)profile_index;
					break;
				}
			}
		}
	}

	return TRUE;
}

static boolean player_profiles_list_dispose(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	if (widget->list.items)
	{
		widget_free(widget->list.items);
		widget->list.items = NULL;
	}

	widget->list.item_count = 0;

	return TRUE;
}

static boolean player_profile_set_for_game_3wide(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct player_profile profile;
	struct ui_widget_definition *definition;
	struct widget_instance *profile_list;
	long *profile_indices;
	long profile_index;
	boolean success;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1763, event && event->controller_index!=NONE, "setting a player profile requires a valid controller index");
	definition = ui_widget_definition_get(widget->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1772, definition->type == _ui_widget_type_container && definition->child_widgets.count >= 3, "expected the player profile select screen to be a container w/ 3 or more children");
	profile_list = widget->first_child;
	definition = ui_widget_definition_get(profile_list->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1775, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'player profile list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1776, definition->child_widgets.count == 3, "expected 3 list items for 'player profile list' widget");
	profile_list = widget->first_child;
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1784, (profile_list->list.item_index>=0) && (profile_list->list.item_index<profile_list->list.item_count), "invalid multiplayer profile specified from 'player profile list' list widget");
	profile_indices = profile_list->list.items;
	profile_index = profile_indices[profile_list->list.item_index];

	if (profile_index != NONE)
	{
		if (!TEST_FLAG(profile_index, _profile_checksum_valid_bit31))
		{
			display_error_deferred(_error_warning_saved_game_file_damaged, NONE, TRUE, FALSE);
			ui_play_audio_feedback_sound(_audio_feedback_failure);
			*widget_deleted = TRUE;
			success = FALSE;
		}
		else if (player_profile_get(profile_index, &profile))
		{
			short local_player_index = player_ui_get_single_player_local_player_from_controller(event->controller_index);

			player_ui_set_active_player_profile(local_player_index, profile_indices[profile_list->list.item_index], &profile);
			success = TRUE;
		}
		else
		{
			error(_error_silent, "failed to retrieve user selected player profile");
			success = FALSE;
		}
	}
	else
	{
		error(_error_silent, "this is not a selectable player profile");
		ui_play_audio_feedback_sound(_audio_feedback_failure);
		success = FALSE;
	}

	return success;
}

static boolean player_profile_set_for_game_1wide(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct player_profile profile;
	struct widget_instance *profile_list;
	long *profile_indices;
	long profile_index;
	short local_player_index;
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1834, event && event->controller_index!=NONE, "setting a player profile requires a valid controller index");
	local_player_index = event->controller_index;

	for (profile_list = widget->first_child; profile_list && profile_list->type != _ui_widget_type_spinner_list; profile_list = profile_list->next);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1838, profile_list, "failed to find the 1-wide spinner list for player profiles (expected it to be a child of this widget)");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1845, ui_widget_definition_get(profile_list->tag_index)->child_widgets.count == 0, "expected a code-generated 1-wide spinner list for 'mp player profile list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1851, (profile_list->list.item_index>=0) && (profile_list->list.item_index<profile_list->list.item_count), "invalid multiplayer profile specified from 'mp player profile list' list widget");
	profile_indices = profile_list->list.items;
	profile_index = profile_indices[profile_list->list.item_index];

	if (!TEST_FLAG(profile_index, _profile_checksum_valid_bit31))
	{
		display_error_deferred(_error_warning_saved_game_file_damaged, local_player_index, TRUE, FALSE);
		ui_play_audio_feedback_sound(_audio_feedback_failure);
		success = FALSE;
	}
	else if (player_profile_get(profile_index, &profile))
	{
		player_ui_set_active_player_profile(local_player_index, profile_indices[profile_list->list.item_index], &profile);
	}
	else
	{
		error(_error_silent, "failed to retrieve user selected player profile");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_begin_editing(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct ui_widget_definition *definition;
	struct widget_instance *profile_list;
	long profile_index;
	boolean success = FALSE;

	profile_index_to_delete = NONE;
	definition = ui_widget_definition_get(widget->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1896, definition->type == _ui_widget_type_container && definition->child_widgets.count >= 3, "expected the multiplayer profile select screen to be a container w/ 3+ children");
	profile_list = widget->first_child;
	definition = ui_widget_definition_get(profile_list->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1899, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'multiplayer profile list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1900, definition->child_widgets.count == 3, "expected 3 list items for 'multiplayer profile list' widget");
	profile_list = widget->first_child;
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1909, (profile_list->list.item_index>=0) && (profile_list->list.item_index<profile_list->list.item_count), "invalid multiplayer profile specified from 'multiplayer profile list' list widget");
	profile_index = ((long *)profile_list->list.items)[profile_list->list.item_index];

	if (profile_index != NONE)
	{
		if (TEST_FLAG(profile_index, _profile_checksum_valid_bit31))
		{
			player_ui_begin_editing_profile(profile_index);
			success = TRUE;
		}
		else
		{
			display_error_deferred(_error_warning_saved_game_file_damaged, NONE, TRUE, FALSE);
			ui_play_audio_feedback_sound(_audio_feedback_failure);
		}
	}
	else
	{
		ui_play_audio_feedback_sound(_audio_feedback_failure);
	}

	return success;
}

static boolean playlist_profile_end_editing(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	profile_index_to_delete = NONE;
	player_ui_end_editing_profile();

	return TRUE;
}

static boolean playlist_profile_set_game_engine(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	struct widget_instance *game_engine_list = widget->parent;
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 1958, game_engine_list && game_engine_list->type == _ui_widget_type_column_list, "expected column list for game engine type list");

	if (variant)
	{
		long game_engine_index;

		switch (game_engine_list->list.item_index)
		{
		case 0:
			game_engine_index = _game_engine_ctf;
			break;
		case 1:
			game_engine_index = _game_engine_king;
			break;
		case 2:
			game_engine_index = _game_engine_slayer;
			break;
		case 3:
			game_engine_index = _game_engine_oddball;
			break;
		case 4:
			game_engine_index = _game_engine_race;
			break;
		default:
			error(_error_silent, "unknown game engine option selected");
			game_engine_index = variant->game_engine_index;
			break;
		}

		if (game_engine_index != variant->game_engine_index)
		{
			memset(&variant->game_engine_variant, 0, sizeof(variant->game_engine_variant));
		}

		variant->game_engine_index = game_engine_index;
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_change_name(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	if (variant)
	{
		if (!virtual_keyboard_launch(
			variant->human_readable_game_description,
			MAXIMUM_GAME_VARIANT_NAME_LENGTH * sizeof(wchar_t),
			_virtual_keyboard_caption_variant_name))
		{
			error(_error_silent, "failed to invoke virtual keyboard on profile name");
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_change_ctf_rules(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2040, list_item, "expected 'assault' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2042, spinner, "expected 'assault' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.ctf.assault = TRUE; break;
		case 1: variant->game_engine_variant.ctf.assault = FALSE; break;
		default: error(_error_silent, "unknown option selected in 'assault' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2051, list_item, "expected 'single flag' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2053, spinner, "expected 'single flag' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.ctf.single_flag_time = 0; break;
		case 1: variant->game_engine_variant.ctf.single_flag_time = 1 * TICKS_PER_MINUTE; break;
		case 2: variant->game_engine_variant.ctf.single_flag_time = 2 * TICKS_PER_MINUTE; break;
		case 3: variant->game_engine_variant.ctf.single_flag_time = 3 * TICKS_PER_MINUTE; break;
		case 4: variant->game_engine_variant.ctf.single_flag_time = 5 * TICKS_PER_MINUTE; break;
		case 5: variant->game_engine_variant.ctf.single_flag_time = 10 * TICKS_PER_MINUTE; break;
		default: error(_error_silent, "unknown option selected in 'single flag' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2066, list_item, "expected 'flag must reset' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2068, spinner, "expected 'flag must reset' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.ctf.flag_must_reset = TRUE; break;
		case 1: variant->game_engine_variant.ctf.flag_must_reset = FALSE; break;
		default: error(_error_silent, "unknown option selected in 'flag must reset' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2077, list_item, "expected 'flag at home to score' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2079, spinner, "expected 'flag at home to score' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.ctf.flag_at_home_to_score = TRUE; break;
		case 1: variant->game_engine_variant.ctf.flag_at_home_to_score = FALSE; break;
		default: error(_error_silent, "unknown option selected in 'flag at home to score' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2088, list_item, "expected 'captures to win' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2090, spinner, "expected 'captures to win' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.score_to_win = 1; break;
		case 1: variant->universal_variant.score_to_win = 3; break;
		case 2: variant->universal_variant.score_to_win = 5; break;
		case 3: variant->universal_variant.score_to_win = 10; break;
		case 4: variant->universal_variant.score_to_win = 15; break;
		default: error(_error_silent, "unknown option selected in 'captures to win' option spinner list"); break;
		}

		ui_widgets_pop_stack(widget->local_player_index);
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
	}

	return success;
}

static boolean playlist_profile_change_koth_rules(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2126, list_item, "expected 'moving hill' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2128, spinner, "expected 'moving hill' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.king.moving_hill = TRUE; break;
		case 1: variant->game_engine_variant.king.moving_hill = FALSE; break;
		default: error(_error_silent, "unknown option selected in 'moving hill' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2137, list_item, "expected 'score to win' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2139, spinner, "expected 'score to win' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.score_to_win = 1; break;
		case 1: variant->universal_variant.score_to_win = 2; break;
		case 2: variant->universal_variant.score_to_win = 5; break;
		case 3: variant->universal_variant.score_to_win = 10; break;
		case 4: variant->universal_variant.score_to_win = 15; break;
		default: error(_error_silent, "unknown option selected in 'score to win' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2151, list_item, "expected 'teams' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2153, spinner, "expected 'teams' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.teams = TRUE; break;
		case 1: variant->universal_variant.teams = FALSE; break;
		default: error(_error_silent, "unknown option selected in 'teams' option spinner list"); break;
		}

		ui_widgets_pop_stack(widget->local_player_index);
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_change_slayer_rules(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2187, list_item, "expected 'death bonus' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2189, spinner, "expected 'death bonus' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.slayer.no_death_bonus = FALSE; break;
		case 1: variant->game_engine_variant.slayer.no_death_bonus = TRUE; break;
		default: error(_error_silent, "unknown option selected in 'death bonus' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2198, list_item, "expected 'kill in order' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2200, spinner, "expected 'kill in order' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.slayer.kill_in_order = TRUE; break;
		case 1: variant->game_engine_variant.slayer.kill_in_order = FALSE; break;
		default: error(_error_silent, "unknown option selected in 'kill in order' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2209, list_item, "expected 'kill penalty' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2211, spinner, "expected 'kill penalty' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.slayer.no_kill_penalty = FALSE; break;
		case 1: variant->game_engine_variant.slayer.no_kill_penalty = TRUE; break;
		default: error(_error_silent, "unknown option selected in 'kill penalty' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2220, list_item, "expected 'kills to win' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2222, spinner, "expected 'kills to win' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.score_to_win = 5; break;
		case 1: variant->universal_variant.score_to_win = 10; break;
		case 2: variant->universal_variant.score_to_win = 15; break;
		case 3: variant->universal_variant.score_to_win = 25; break;
		case 4: variant->universal_variant.score_to_win = 50; break;
		default: error(_error_silent, "unknown option selected in 'kills to win' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2234, list_item, "expected 'teams' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2236, spinner, "expected 'teams' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.teams = TRUE; break;
		case 1: variant->universal_variant.teams = FALSE; break;
		default: error(_error_silent, "unknown option selected in 'teams' option spinner list"); break;
		}

		ui_widgets_pop_stack(widget->local_player_index);
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_change_oddball_rules(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2270, list_item, "expected 'trait with ball' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2272, spinner, "expected 'trait with ball' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.oddball.trait_with_ball = _game_trait_none; break;
		case 1: variant->game_engine_variant.oddball.trait_with_ball = _game_trait_invisible; break;
		case 2: variant->game_engine_variant.oddball.trait_with_ball = _game_trait_extra_damage; break;
		case 3: variant->game_engine_variant.oddball.trait_with_ball = _game_trait_damage_resistant; break;
		default: error(_error_silent, "unknown option selected in 'trait with ball' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2283, list_item, "expected 'trait without ball' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2285, spinner, "expected 'trait without ball' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.oddball.trait_without_ball = _game_trait_none; break;
		case 1: variant->game_engine_variant.oddball.trait_without_ball = _game_trait_invisible; break;
		case 2: variant->game_engine_variant.oddball.trait_without_ball = _game_trait_extra_damage; break;
		case 3: variant->game_engine_variant.oddball.trait_without_ball = _game_trait_damage_resistant; break;
		default: error(_error_silent, "unknown option selected in 'trait without ball' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2296, list_item, "expected 'speed with ball' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2298, spinner, "expected 'speed with ball' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.oddball.speed_with_ball = _oddball_speed_normal; break;
		case 1: variant->game_engine_variant.oddball.speed_with_ball = _oddball_speed_slow; break;
		case 2: variant->game_engine_variant.oddball.speed_with_ball = _oddball_speed_faster; break;
		default: error(_error_silent, "unknown option selected in 'speed with ball' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2308, list_item, "expected 'ball type' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2310, spinner, "expected 'ball type' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.oddball.oddball_ball_type = _oddball_normal; break;
		case 1: variant->game_engine_variant.oddball.oddball_ball_type = _oddball_magic; break;
		case 2: variant->game_engine_variant.oddball.oddball_ball_type = _oddball_terminator; break;
		default: error(_error_silent, "unknown option selected in 'ball type' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2320, list_item, "expected 'random start' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2322, spinner, "expected 'random start' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.oddball.random_start = TRUE; break;
		case 1: variant->game_engine_variant.oddball.random_start = FALSE; break;
		default: error(_error_silent, "unknown option selected in 'random start' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2331, list_item, "expected 'ball spawn count' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2333, spinner, "expected 'ball spawn count' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			variant->game_engine_variant.oddball.ball_spawn_count = spinner->list.item_index + 1;
			break;
		default:
			error(_error_silent, "unknown option selected in 'ball spawn count' option spinner list");
			break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2358, list_item, "expected 'score to win' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2360, spinner, "expected 'score to win' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.score_to_win = 1; break;
		case 1: variant->universal_variant.score_to_win = 2; break;
		case 2: variant->universal_variant.score_to_win = 5; break;
		case 3: variant->universal_variant.score_to_win = 10; break;
		case 4: variant->universal_variant.score_to_win = 15; break;
		default: error(_error_silent, "unknown option selected in 'score to win' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2372, list_item, "expected 'teams' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2374, spinner, "expected 'teams' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.teams = TRUE; break;
		case 1: variant->universal_variant.teams = FALSE; break;
		default: error(_error_silent, "unknown option selected in 'teams' option spinner list"); break;
		}

		ui_widgets_pop_stack(widget->local_player_index);
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_change_racing_rules(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2408, list_item, "expected 'team scoring' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2410, spinner, "expected 'team scoring' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.race.team_scoring = _race_team_scoring_min; break;
		case 1: variant->game_engine_variant.race.team_scoring = _race_team_scoring_max; break;
		case 2: variant->game_engine_variant.race.team_scoring = _race_team_scoring_accumulate; break;
		default: error(_error_silent, "unknown option selected in 'team scoring' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2420, list_item, "expected 'race type' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2422, spinner, "expected 'race type' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->game_engine_variant.race.race_type = _race_type_normal; break;
		case 1: variant->game_engine_variant.race.race_type = _race_type_any_order; break;
		case 2: variant->game_engine_variant.race.race_type = _race_type_flag_rally; break;
		default: error(_error_silent, "unknown option selected in 'race type' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2432, list_item, "expected 'laps to win' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2434, spinner, "expected 'laps to win' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.score_to_win = 1; break;
		case 1: variant->universal_variant.score_to_win = 3; break;
		case 2: variant->universal_variant.score_to_win = 5; break;
		case 3: variant->universal_variant.score_to_win = 10; break;
		case 4: variant->universal_variant.score_to_win = 15; break;
		case 5: variant->universal_variant.score_to_win = 25; break;
		default: error(_error_silent, "unknown option selected in 'laps to win' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2447, list_item, "expected 'teams' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2449, spinner, "expected 'teams' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.teams = TRUE; break;
		case 1: variant->universal_variant.teams = FALSE; break;
		default: error(_error_silent, "unknown option selected in 'teams' option spinner list"); break;
		}

		ui_widgets_pop_stack(widget->local_player_index);
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_change_player_options(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2483, list_item, "expected 'number of lives' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2485, spinner, "expected 'number of lives' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.lives = 0; break;
		case 1: variant->universal_variant.lives = 1; break;
		case 2: variant->universal_variant.lives = 3; break;
		case 3: variant->universal_variant.lives = 5; break;
		default: error(_error_silent, "unknown option selected in 'number of lives' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2496, list_item, "expected 'maximum health' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2498, spinner, "expected 'maximum health' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.health = 0.5f; break;
		case 1: variant->universal_variant.health = 1.f; break;
		case 2: variant->universal_variant.health = 1.5f; break;
		case 3: variant->universal_variant.health = 2.f; break;
		case 4: variant->universal_variant.health = 3.f; break;
		case 5: variant->universal_variant.health = 4.f; break;
		default: error(_error_silent, "unknown option selected in 'maximum health' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2511, list_item, "expected 'shields' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2513, spinner, "expected 'shields' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: SET_FLAG(variant->universal_variant.flags, _no_shields_bit, FALSE); break;
		case 1: SET_FLAG(variant->universal_variant.flags, _no_shields_bit, TRUE); break;
		default: error(_error_silent, "unknown option selected in 'shields' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2522, list_item, "expected 'respawn time' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2524, spinner, "expected 'respawn time' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.respawn_time = 0; break;
		case 1: variant->universal_variant.respawn_time = 5 * TICKS_PER_SECOND; break;
		case 2: variant->universal_variant.respawn_time = 10 * TICKS_PER_SECOND; break;
		case 3: variant->universal_variant.respawn_time = 15 * TICKS_PER_SECOND; break;
		default: error(_error_silent, "unknown option selected in 'respawn time' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2535, list_item, "expected 'respawn time growth' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2537, spinner, "expected 'respawn time growth' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.respawn_time_growth = 0; break;
		case 1: variant->universal_variant.respawn_time_growth = 5 * TICKS_PER_SECOND; break;
		case 2: variant->universal_variant.respawn_time_growth = 10 * TICKS_PER_SECOND; break;
		case 3: variant->universal_variant.respawn_time_growth = 15 * TICKS_PER_SECOND; break;
		default: error(_error_silent, "unknown option selected in 'respawn time growth' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2548, list_item, "expected 'odd man out' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2550, spinner, "expected 'odd man out' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.odd_man_out = TRUE; break;
		case 1: variant->universal_variant.odd_man_out = FALSE; break;
		default: error(_error_silent, "unknown option selected in 'odd man out' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2559, list_item, "expected 'invisible players' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2561, spinner, "expected 'invisible players' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: SET_FLAG(variant->universal_variant.flags, _always_invisible_bit, TRUE); break;
		case 1: SET_FLAG(variant->universal_variant.flags, _always_invisible_bit, FALSE); break;
		default: error(_error_silent, "unknown option selected in 'invisible players' option spinner list"); break;
		}

		list_item = list_item->next;

		if (list_item)
		{
			for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
			match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2574, spinner, "expected 'suicide penalty' option spinner list");

			switch (spinner->list.item_index)
			{
			case 0: variant->universal_variant.suicide_penalty = 0; break;
			case 1: variant->universal_variant.suicide_penalty = 5 * TICKS_PER_SECOND; break;
			case 2: variant->universal_variant.suicide_penalty = 10 * TICKS_PER_SECOND; break;
			case 3: variant->universal_variant.suicide_penalty = 15 * TICKS_PER_SECOND; break;
			default: error(_error_silent, "unknown option selected in 'suicide penalty' option spinner list"); break;
			}
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_change_item_options(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2608, list_item, "expected 'infinite grenades' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2610, spinner, "expected 'infinite grenades' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: SET_FLAG(variant->universal_variant.flags, _infinite_grenades_bit, TRUE); break;
		case 1: SET_FLAG(variant->universal_variant.flags, _infinite_grenades_bit, FALSE); break;
		default: error(_error_silent, "unknown option selected in 'infinite grenades' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2619, list_item, "expected 'vehicle set' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2621, spinner, "expected 'vehicle set' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.vehicle_set = _game_engine_vehicles_default; break;
		case 1: variant->universal_variant.vehicle_set = _game_engine_vehicles_none; break;
		case 2: variant->universal_variant.vehicle_set = _game_engine_vehicles_warthog; break;
		case 3: variant->universal_variant.vehicle_set = _game_engine_vehicles_ghost; break;
		case 4: variant->universal_variant.vehicle_set = _game_engine_vehicles_tank; break;
		default: error(_error_silent, "unknown option selected in 'vehicle set' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2633, list_item, "expected 'weapon set' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2635, spinner, "expected 'weapon set' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.weapon_set = _game_engine_weapons_normal; break;
		case 1: variant->universal_variant.weapon_set = _game_engine_weapons_pistols; break;
		case 2: variant->universal_variant.weapon_set = _game_engine_weapons_assault_rifles; break;
		case 3: variant->universal_variant.weapon_set = _game_engine_weapons_plasma_weapons; break;
		case 4: variant->universal_variant.weapon_set = _game_engine_weapons_sniping; break;
		case 5: variant->universal_variant.weapon_set = _game_engine_weapons_no_sniping; break;
		case 6: variant->universal_variant.weapon_set = _game_engine_weapons_rocket_launchers; break;
		case 7: variant->universal_variant.weapon_set = _game_engine_weapons_shotguns; break;
		case 8: variant->universal_variant.weapon_set = _game_engine_weapons_short_range; break;
		case 9: variant->universal_variant.weapon_set = _game_engine_weapons_human; break;
		case 10: variant->universal_variant.weapon_set = _game_engine_weapons_no_grenades; break;
		default: error(_error_silent, "unknown option selected in 'weapon set' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2654, list_item, "expected 'starting equipment' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2656, spinner, "expected 'starting equipment' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: SET_FLAG(variant->universal_variant.flags, _generic_starting_equipment_bit, FALSE); break;
		case 1: SET_FLAG(variant->universal_variant.flags, _generic_starting_equipment_bit, TRUE); break;
		default: error(_error_silent, "unknown option selected in 'starting equipment' option spinner list"); break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_change_indicator_options(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2688, list_item, "expected 'radar display' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2690, spinner, "expected 'radar display' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: variant->universal_variant.goal_radar = _radar_motion_tracker; break;
		case 1: variant->universal_variant.goal_radar = _radar_nav_point; break;
		case 2: variant->universal_variant.goal_radar = _radar_none; break;
		default: error(_error_silent, "unknown option selected in 'radar display' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2700, list_item, "expected 'other players on radar' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2702, spinner, "expected 'other players on radar' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: SET_FLAG(variant->universal_variant.flags, _allow_motion_tracker_bit, TRUE); break;
		case 1: SET_FLAG(variant->universal_variant.flags, _allow_motion_tracker_bit, FALSE); break;
		default: error(_error_silent, "unknown option selected in 'other players on radar' option spinner list"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2711, list_item, "expected 'friends on screen' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2713, spinner, "expected 'friends on screen' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: SET_FLAG(variant->universal_variant.flags, _allow_friendly_navpoints_bit, TRUE); break;
		case 1: SET_FLAG(variant->universal_variant.flags, _allow_friendly_navpoints_bit, FALSE); break;
		default: error(_error_silent, "unknown option selected in 'friends on screen' option spinner list"); break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_initialize_game_engine(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2738, widget->type == _ui_widget_type_column_list, "expected a column list for the list of available game engines");

	if (variant)
	{
		switch (variant->game_engine_index)
		{
		case _game_engine_ctf: widget->list.item_index = 0; break;
		case _game_engine_king: widget->list.item_index = 1; break;
		case _game_engine_slayer: widget->list.item_index = 2; break;
		case _game_engine_oddball: widget->list.item_index = 3; break;
		case _game_engine_race: widget->list.item_index = 4; break;
		default: widget->list.item_index = 0; break;
		}

		widget->focused_child = widget_instance_get_nth_child(widget, widget->list.item_index);
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_initialize_name(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2770, widget->type == _ui_widget_type_text_box, "expected text box widget for profile name");

	if (variant)
	{
		widget->text_box.string = ui_widget_realloc(widget->text_box.string, 128 * sizeof(wchar_t), MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c"), MATCH_LINE(2774));

		if (widget->text_box.string)
		{
			ustrncpy(widget->text_box.string, variant->human_readable_game_description, 127);
			widget->text_box.string[127] = L'\0';
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_initialize_ctf_rules(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2797, widget->type == _ui_widget_type_column_list, "expected column list for multiplayer game settings widget");

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2805, list_item, "expected 'assault' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2807, spinner, "expected 'assault' option spinner list");

		switch (variant->game_engine_variant.ctf.assault)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2816, list_item, "expected 'single flag' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2818, spinner, "expected 'single flag' option spinner list");

		switch (variant->game_engine_variant.ctf.single_flag_time)
		{
		case 0: spinner->list.item_index = 0; break;
		case 1 * TICKS_PER_MINUTE: spinner->list.item_index = 1; break;
		case 2 * TICKS_PER_MINUTE: spinner->list.item_index = 2; break;
		case 3 * TICKS_PER_MINUTE: spinner->list.item_index = 3; break;
		case 5 * TICKS_PER_MINUTE: spinner->list.item_index = 4; break;
		case 10 * TICKS_PER_MINUTE: spinner->list.item_index = 5; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2831, list_item, "expected 'flag must reset' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2833, spinner, "expected 'flag must reset' option spinner list");

		switch (variant->game_engine_variant.ctf.flag_must_reset)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2842, list_item, "expected 'flag at home to score' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2844, spinner, "expected 'flag at home to score' option spinner list");

		switch (variant->game_engine_variant.ctf.flag_at_home_to_score)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2853, list_item, "expected 'captures to win' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2855, spinner, "expected 'captures to win' option spinner list");

		switch (variant->universal_variant.score_to_win)
		{
		case 1: spinner->list.item_index = 0; break;
		case 3: spinner->list.item_index = 1; break;
		case 5: spinner->list.item_index = 2; break;
		case 10: spinner->list.item_index = 3; break;
		case 15: spinner->list.item_index = 4; break;
		default: spinner->list.item_index = 0; break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_initialize_koth_rules(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2883, widget->type == _ui_widget_type_column_list, "expected column list for multiplayer game settings widget");

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2891, list_item, "expected 'moving hill' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2893, spinner, "expected 'moving hill' option spinner list");

		switch (variant->game_engine_variant.king.moving_hill)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2902, list_item, "expected 'score to win' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2904, spinner, "expected 'score to win' option spinner list");

		switch (variant->universal_variant.score_to_win)
		{
		case 1: spinner->list.item_index = 0; break;
		case 2: spinner->list.item_index = 1; break;
		case 5: spinner->list.item_index = 2; break;
		case 10: spinner->list.item_index = 3; break;
		case 15: spinner->list.item_index = 4; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2916, list_item, "expected 'teams' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2918, spinner, "expected 'teams' option spinner list");

		switch (variant->universal_variant.teams)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_initialize_slayer_rules(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2943, widget->type == _ui_widget_type_column_list, "expected column list for multiplayer game settings widget");

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2951, list_item, "expected 'death bonus' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2953, spinner, "expected 'death bonus' option spinner list");

		switch (variant->game_engine_variant.slayer.no_death_bonus)
		{
		case FALSE: spinner->list.item_index = 0; break;
		case TRUE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2962, list_item, "expected 'kill in order' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2964, spinner, "expected 'kill in order' option spinner list");

		switch (variant->game_engine_variant.slayer.kill_in_order)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2973, list_item, "expected 'kill penalty' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2975, spinner, "expected 'kill penalty' option spinner list");

		switch (variant->game_engine_variant.slayer.no_kill_penalty)
		{
		case FALSE: spinner->list.item_index = 0; break;
		case TRUE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2984, list_item, "expected 'kills to win' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2986, spinner, "expected 'kills to win' option spinner list");

		switch (variant->universal_variant.score_to_win)
		{
		case 5: spinner->list.item_index = 0; break;
		case 10: spinner->list.item_index = 1; break;
		case 15: spinner->list.item_index = 2; break;
		case 25: spinner->list.item_index = 3; break;
		case 50: spinner->list.item_index = 4; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 2998, list_item, "expected 'teams' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3000, spinner, "expected 'teams' option spinner list");

		switch (variant->universal_variant.teams)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_initialize_oddball_rules(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3025, widget->type == _ui_widget_type_column_list, "expected column list for multiplayer game settings widget");

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3033, list_item, "expected 'trait with ball' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3035, spinner, "expected 'trait with ball' option spinner list");

		switch (variant->game_engine_variant.oddball.trait_with_ball)
		{
		case _game_trait_none: spinner->list.item_index = 0; break;
		case _game_trait_invisible: spinner->list.item_index = 1; break;
		case _game_trait_extra_damage: spinner->list.item_index = 2; break;
		case _game_trait_damage_resistant: spinner->list.item_index = 3; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3046, list_item, "expected 'trait without ball' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3048, spinner, "expected 'trait without ball' option spinner list");

		switch (variant->game_engine_variant.oddball.trait_without_ball)
		{
		case _game_trait_none: spinner->list.item_index = 0; break;
		case _game_trait_invisible: spinner->list.item_index = 1; break;
		case _game_trait_extra_damage: spinner->list.item_index = 2; break;
		case _game_trait_damage_resistant: spinner->list.item_index = 3; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3059, list_item, "expected 'speed with ball' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3061, spinner, "expected 'speed with ball' option spinner list");

		switch (variant->game_engine_variant.oddball.speed_with_ball)
		{
		case _oddball_speed_normal: spinner->list.item_index = 0; break;
		case _oddball_speed_slow: spinner->list.item_index = 1; break;
		case _oddball_speed_faster: spinner->list.item_index = 2; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3071, list_item, "expected 'ball type' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3073, spinner, "expected 'ball type' option spinner list");

		switch (variant->game_engine_variant.oddball.oddball_ball_type)
		{
		case _oddball_normal: spinner->list.item_index = 0; break;
		case _oddball_magic: spinner->list.item_index = 1; break;
		case _oddball_terminator: spinner->list.item_index = 2; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3083, list_item, "expected 'random start' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3085, spinner, "expected 'random start' option spinner list");

		switch (variant->game_engine_variant.oddball.random_start)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3094, list_item, "expected 'ball spawn count' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3096, spinner, "expected 'ball spawn count' option spinner list");

		switch (variant->game_engine_variant.oddball.ball_spawn_count)
		{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
			spinner->list.item_index = (short)(variant->game_engine_variant.oddball.ball_spawn_count - 1);
			break;
		default:
			spinner->list.item_index = 0;
			break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3121, list_item, "expected 'score to win' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3123, spinner, "expected 'score to win' option spinner list");

		switch (variant->universal_variant.score_to_win)
		{
		case 1: spinner->list.item_index = 0; break;
		case 2: spinner->list.item_index = 1; break;
		case 5: spinner->list.item_index = 2; break;
		case 10: spinner->list.item_index = 3; break;
		case 15: spinner->list.item_index = 4; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3135, list_item, "expected 'teams' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3137, spinner, "expected 'teams' option spinner list");

		switch (variant->universal_variant.teams)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_initialize_racing_rules(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3162, widget->type == _ui_widget_type_column_list, "expected column list for multiplayer game settings widget");

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3170, list_item, "expected 'team scoring' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3172, spinner, "expected 'team scoring' option spinner list");

		switch (variant->game_engine_variant.race.team_scoring)
		{
		case _race_team_scoring_min: spinner->list.item_index = 0; break;
		case _race_team_scoring_max: spinner->list.item_index = 1; break;
		case _race_team_scoring_accumulate: spinner->list.item_index = 2; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3182, list_item, "expected 'race type' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3184, spinner, "expected 'race type' option spinner list");

		switch (variant->game_engine_variant.race.race_type)
		{
		case _race_type_normal: spinner->list.item_index = 0; break;
		case _race_type_any_order: spinner->list.item_index = 1; break;
		case _race_type_flag_rally: spinner->list.item_index = 2; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3194, list_item, "expected 'laps to win' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3196, spinner, "expected 'laps to win' option spinner list");

		switch (variant->universal_variant.score_to_win)
		{
		case 1: spinner->list.item_index = 0; break;
		case 3: spinner->list.item_index = 1; break;
		case 5: spinner->list.item_index = 2; break;
		case 10: spinner->list.item_index = 3; break;
		case 15: spinner->list.item_index = 4; break;
		case 25: spinner->list.item_index = 5; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3209, list_item, "expected 'teams' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3211, spinner, "expected 'teams' option spinner list");

		switch (variant->universal_variant.teams)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_initialize_player_options(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3237, widget->type == _ui_widget_type_column_list, "expected column list for multiplayer game settings widget");

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3245, list_item, "expected 'number of lives' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3247, spinner, "expected 'number of lives' option spinner list");

		switch (variant->universal_variant.lives)
		{
		case 0: spinner->list.item_index = 0; break;
		case 1: spinner->list.item_index = 1; break;
		case 3: spinner->list.item_index = 2; break;
		case 5: spinner->list.item_index = 3; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3258, list_item, "expected 'maximum health' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3260, spinner, "expected 'maximum health' option spinner list");

		switch ((long)(10.f * variant->universal_variant.health))
		{
		case 5: spinner->list.item_index = 0; break;
		case 10: spinner->list.item_index = 1; break;
		case 15: spinner->list.item_index = 2; break;
		case 20: spinner->list.item_index = 3; break;
		case 30: spinner->list.item_index = 4; break;
		case 40: spinner->list.item_index = 5; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3273, list_item, "expected 'shields' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3275, spinner, "expected 'shields' option spinner list");

		switch (TEST_FLAG(variant->universal_variant.flags, _no_shields_bit))
		{
		case FALSE: spinner->list.item_index = 0; break;
		case TRUE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3285, list_item, "expected 'respawn time' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3287, spinner, "expected 'respawn time' option spinner list");

		switch (variant->universal_variant.respawn_time)
		{
		case 0: spinner->list.item_index = 0; break;
		case 5 * TICKS_PER_SECOND: spinner->list.item_index = 1; break;
		case 10 * TICKS_PER_SECOND: spinner->list.item_index = 2; break;
		case 15 * TICKS_PER_SECOND: spinner->list.item_index = 3; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3298, list_item, "expected 'respawn time growth' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3300, spinner, "expected 'respawn time growth' option spinner list");

		switch (variant->universal_variant.respawn_time_growth)
		{
		case 0: spinner->list.item_index = 0; break;
		case 5 * TICKS_PER_SECOND: spinner->list.item_index = 1; break;
		case 10 * TICKS_PER_SECOND: spinner->list.item_index = 2; break;
		case 15 * TICKS_PER_SECOND: spinner->list.item_index = 3; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3311, list_item, "expected 'odd man out' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3313, spinner, "expected 'odd man out' option spinner list");

		switch (variant->universal_variant.odd_man_out)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3322, list_item, "expected 'invisible players' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3324, spinner, "expected 'invisible players' option spinner list");

		switch (TEST_FLAG(variant->universal_variant.flags, _always_invisible_bit))
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3335, list_item, "expected 'suicide penalty' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);

		switch (variant->universal_variant.suicide_penalty)
		{
		case 0: spinner->list.item_index = 0; break;
		case 5 * TICKS_PER_SECOND: spinner->list.item_index = 1; break;
		case 10 * TICKS_PER_SECOND: spinner->list.item_index = 2; break;
		case 15 * TICKS_PER_SECOND: spinner->list.item_index = 3; break;
		default: spinner->list.item_index = 0; break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_initialize_item_options(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3366, widget->type == _ui_widget_type_column_list, "expected column list for multiplayer game settings widget");

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3374, list_item, "expected 'infinite grenades' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3376, spinner, "expected 'infinite grenades' option spinner list");

		switch (TEST_FLAG(variant->universal_variant.flags, _infinite_grenades_bit))
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3386, list_item, "expected 'vehicle set' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3388, spinner, "expected 'vehicle set' option spinner list");

		switch (variant->universal_variant.vehicle_set)
		{
		case _game_engine_vehicles_default: spinner->list.item_index = 0; break;
		case _game_engine_vehicles_none: spinner->list.item_index = 1; break;
		case _game_engine_vehicles_warthog: spinner->list.item_index = 2; break;
		case _game_engine_vehicles_ghost: spinner->list.item_index = 3; break;
		case _game_engine_vehicles_tank: spinner->list.item_index = 4; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3400, list_item, "expected 'weapon set' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3402, spinner, "expected 'weapon set' option spinner list");

		switch (variant->universal_variant.weapon_set)
		{
		case _game_engine_weapons_normal: spinner->list.item_index = 0; break;
		case _game_engine_weapons_pistols: spinner->list.item_index = 1; break;
		case _game_engine_weapons_assault_rifles: spinner->list.item_index = 2; break;
		case _game_engine_weapons_plasma_weapons: spinner->list.item_index = 3; break;
		case _game_engine_weapons_sniping: spinner->list.item_index = 4; break;
		case _game_engine_weapons_no_sniping: spinner->list.item_index = 5; break;
		case _game_engine_weapons_rocket_launchers: spinner->list.item_index = 6; break;
		case _game_engine_weapons_shotguns: spinner->list.item_index = 7; break;
		case _game_engine_weapons_short_range: spinner->list.item_index = 8; break;
		case _game_engine_weapons_human: spinner->list.item_index = 9; break;
		case _game_engine_weapons_no_grenades: spinner->list.item_index = 10; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3421, list_item, "expected 'starting equipment' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3423, spinner, "expected 'starting equpiment' option spinner list");

		switch (TEST_FLAG(variant->universal_variant.flags, _generic_starting_equipment_bit))
		{
		case FALSE: spinner->list.item_index = 0; break;
		case TRUE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_initialize_indicator_options(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct game_variant *variant = player_ui_get_edit_playlist_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3450, widget->type == _ui_widget_type_column_list, "expected column list for multiplayer game settings widget");

	if (variant)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3458, list_item, "expected 'radar display' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3460, spinner, "expected 'radar display' option spinner list");

		switch (variant->universal_variant.goal_radar)
		{
		case _radar_motion_tracker: spinner->list.item_index = 0; break;
		case _radar_nav_point: spinner->list.item_index = 1; break;
		case _radar_none: spinner->list.item_index = 2; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3470, list_item, "expected 'other players on radar' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3472, spinner, "expected 'other players on radar' option spinner list");

		switch (TEST_FLAG(variant->universal_variant.flags, _allow_motion_tracker_bit))
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3482, list_item, "expected 'friends on screen' item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3484, spinner, "expected 'friends on screen' option spinner list");

		switch (TEST_FLAG(variant->universal_variant.flags, _allow_friendly_navpoints_bit))
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable game variant");
		success = FALSE;
	}

	return success;
}

static boolean playlist_profile_save_changes(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3510, event);

	if (player_ui_edit_profile_is_dirty())
	{
		if (!player_ui_edit_profile_is_default_profile() || player_ui_edit_profile_name_is_dirty())
		{
			success = player_ui_save_profile();

			if (!success)
			{
				error(_error_silent, "failed to save changes to multiplayer playlist profile");
			}
		}
		else if (!player_ui_prompt_user_to_rename_edit_profile())
		{
			error(_error_silent, "failed to prompt user to rename profile");
		}
	}
	else
	{
		error(_error_silent, "no changes to playlist profile detected; not saving to disk");
		player_ui_end_editing_profile();
		ui_widget_delete(widget_instance_get_topmost_parent(widget));
		*widget_deleted = TRUE;
	}

	return success;
}

static boolean player_profile_color_picker_menu_initialize(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	word number_of_colors = player_profile_number_of_available_primary_colors();
	struct player_profile *profile = player_ui_get_edit_player_profile();
	struct ui_widget_definition *definition = ui_widget_definition_get(widget->tag_index);

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3576, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'player color picker list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3577, definition->child_widgets.count == 3, "expected 3 list items for 'player color picker list' widget");

	widget->list.items = ui_widget_realloc(widget->list.items, number_of_colors, MATCH_FILE("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c"), MATCH_LINE(3581));

	if (widget->list.items)
	{
		long color_index;

		for (color_index = 0; color_index < number_of_colors; color_index++)
		{
			((byte *)widget->list.items)[color_index] = (byte)color_index;
		}

		widget->list.item_count = number_of_colors;
	}

	if (profile)
	{
		profile->primary_color_index = PIN(profile->primary_color_index, 0, number_of_colors - 1);
		widget->list.item_index = profile->primary_color_index;
	}
	else
	{
		error(_error_silent, "failed to find editing player profile");
	}

	return TRUE;
}

static boolean player_profile_color_picker_menu_dispose(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	if (widget->list.items)
	{
		widget_free(widget->list.items);
		widget->list.items = NULL;
	}

	return TRUE;
}

static boolean player_profile_color_picker_select_color(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct ui_widget_definition *definition;
	struct widget_instance *color_list = widget->focused_child;
	struct player_profile *profile = player_ui_get_edit_player_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3630, color_list && color_list->type == _ui_widget_type_spinner_list, "expected the color select screen to contain a spinner list for the color picker");
	definition = ui_widget_definition_get(color_list->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3637, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'player color picker list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3638, definition->child_widgets.count == 3, "expected 3 list items for 'player color picker list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3644, (color_list->list.item_index>=0) && (color_list->list.item_index<player_profile_number_of_available_primary_colors()), "invalid player profile color index specified");

	if (profile)
	{
		profile->primary_color_index = color_list->list.item_index;
	}
	else
	{
		error(_error_silent, "failed to set player profile color because no profile is currently being edited");
		success = FALSE;
	}

	return success;
}

static boolean player_profile_begin_editing(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct ui_widget_definition *definition;
	struct widget_instance *profile_list;
	long profile_index;
	boolean success = FALSE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3667, widget->type == _ui_widget_type_container, "expected the player profile select screen to be a container widget");
	profile_index_to_delete = NONE;
	profile_list = widget->first_child;
	definition = ui_widget_definition_get(profile_list->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3677, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'player profile list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3678, definition->child_widgets.count == 3, "expected 3 list items for 'player profile list' widget");
	profile_list = widget->first_child;
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3687, (profile_list->list.item_index>=0) && (profile_list->list.item_index<profile_list->list.item_count), "invalid player profile specified from 'player profile list' list widget");
	profile_index = ((long *)profile_list->list.items)[profile_list->list.item_index];

	if (profile_index != NONE)
	{
		if (TEST_FLAG(profile_index, _profile_checksum_valid_bit31))
		{
			player_ui_begin_editing_profile(((long *)profile_list->list.items)[profile_list->list.item_index]);
			success = TRUE;
		}
		else
		{
			display_error_deferred(_error_warning_saved_game_file_damaged, NONE, TRUE, FALSE);
			ui_play_audio_feedback_sound(_audio_feedback_failure);
		}
	}
	else
	{
		ui_play_audio_feedback_sound(_audio_feedback_failure);
	}

	return success;
}

static boolean player_profile_end_editing(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	profile_index_to_delete = NONE;
	player_ui_end_editing_profile();

	return TRUE;
}

static boolean player_profile_change_name(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct player_profile *profile = player_ui_get_edit_player_profile();
	boolean success = TRUE;

	if (profile)
	{
		if (!virtual_keyboard_launch(
			profile->player_name,
			(MAXIMUM_PLAYER_PROFILE_NAME_LENGTH + 1) * sizeof(wchar_t),
			_virtual_keyboard_caption_player_profile_name))
		{
			error(_error_silent, "failed to invoke virtual keyboard on player profile name");
			success = FALSE;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable player profile");
		success = FALSE;
	}

	return success;
}

static boolean player_profile_save_changes(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3759, event);

	if (player_ui_edit_profile_is_dirty())
	{
		success = player_ui_save_profile();

		if (!success)
		{
			error(_error_silent, "failed to save changes to player profile");
		}
	}
	else
	{
		error(_error_silent, "no changes to player profile detected; not saving to disk");
	}

	if (!success)
	{
		player_ui_end_editing_profile();
		ui_widget_delete(widget_instance_get_topmost_parent(widget));
		*widget_deleted = TRUE;
	}

	return success;
}

static boolean player_profile_initialize_controller_settings(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct player_profile *profile = player_ui_get_edit_player_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3793, widget->type == _ui_widget_type_column_list, "expected column list for controller settings widget");

	if (profile)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3801, list_item, "expected 'joystick config' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3803, spinner, "expected 'joystick config' option spinner list");

		switch (profile->controller_settings.joystick_preset)
		{
		case _joystick_preset_standard: spinner->list.item_index = 0; break;
		case _joystick_preset_south_paw: spinner->list.item_index = 1; break;
		case _joystick_preset_legacy: spinner->list.item_index = 2; break;
		case _joystick_preset_legacy_south_paw: spinner->list.item_index = 3; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3814, list_item, "expected 'button config' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3816, spinner, "expected 'button config' option spinner list");

		switch (profile->controller_settings.button_preset)
		{
		case _button_preset_standard: spinner->list.item_index = 0; break;
		case _button_preset_southpaw: spinner->list.item_index = 1; break;
		case _button_preset_jumpy: spinner->list.item_index = 2; break;
		case _button_preset_bumperjumper: spinner->list.item_index = 3; break;
		case _button_preset_boxer: spinner->list.item_index = 4; break;
		default: spinner->list.item_index = 0; break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable player profile");
		success = FALSE;
	}

	return success;
}

static boolean player_profile_initialize_advanced_controller_settings(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct player_profile *profile = player_ui_get_edit_player_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3844, widget->type == _ui_widget_type_column_list, "expected column list for advanced controller settings widget");

	if (profile)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3852, list_item, "expected 'invert joystick' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3854, spinner, "expected 'invert joystick' option spinner list");

		switch (profile->controller_settings.invert_look)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3863, list_item, "expected 'look sensitivity' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3865, spinner, "expected 'look sensitivity' option spinner list");

		switch (profile->controller_settings.look_sensitivity)
		{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			spinner->list.item_index = profile->controller_settings.look_sensitivity - 1;
			break;
		default:
			spinner->list.item_index = 0;
			break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3884, list_item, "expected 'controller vibration' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3886, spinner, "expected 'controller vibration' option spinner list");

		switch (profile->controller_settings.vibration_disabled)
		{
		case FALSE: spinner->list.item_index = 0; break;
		case TRUE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3895, list_item, "expected 'flight stick controls' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3897, spinner, "expected 'flight stick controls' option spinner list");

		switch (profile->controller_settings.flight_stick_aircraft_controls)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3906, list_item, "expected 'autocenter' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3908, spinner, "expected 'autocenter' option spinner list");

		switch (profile->controller_settings.autocenter)
		{
		case TRUE: spinner->list.item_index = 0; break;
		case FALSE: spinner->list.item_index = 1; break;
		default: spinner->list.item_index = 0; break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable player profile");
		success = FALSE;
	}

	return success;
}

static boolean player_profile_change_controller_settings(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct player_profile *profile = player_ui_get_edit_player_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3933, widget->type == _ui_widget_type_column_list, "expected column list for controller settings widget");

	if (profile)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3941, list_item, "expected 'joystick config' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3943, spinner, "expected 'joystick config' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: profile->controller_settings.joystick_preset = _joystick_preset_standard; break;
		case 1: profile->controller_settings.joystick_preset = _joystick_preset_south_paw; break;
		case 2: profile->controller_settings.joystick_preset = _joystick_preset_legacy; break;
		case 3: profile->controller_settings.joystick_preset = _joystick_preset_legacy_south_paw; break;
		default: error(_error_silent, "unknown option selected for joystick config"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3954, list_item, "expected 'button config' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3956, spinner, "expected 'button config' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: profile->controller_settings.button_preset = _button_preset_standard; break;
		case 1: profile->controller_settings.button_preset = _button_preset_southpaw; break;
		case 2: profile->controller_settings.button_preset = _button_preset_jumpy; break;
		case 3: profile->controller_settings.button_preset = _button_preset_bumperjumper; break;
		case 4: profile->controller_settings.button_preset = _button_preset_boxer; break;
		default: error(_error_silent, "unknown button config option selected"); break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable player profile");
		success = FALSE;
	}

	return success;
}

static boolean player_profile_change_advanced_controller_settings(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct player_profile *profile = player_ui_get_edit_player_profile();
	boolean success = TRUE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3984, widget->type == _ui_widget_type_column_list, "expected column list for advanced controller settings widget");

	if (profile)
	{
		struct widget_instance *list_item;
		struct widget_instance *spinner;

		list_item = widget->first_child;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3992, list_item, "expected 'invert joystick' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 3994, spinner, "expected 'invert joystick' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: profile->controller_settings.invert_look = TRUE; break;
		case 1: profile->controller_settings.invert_look = FALSE; break;
		default: error(_error_silent, "unknown option selected for invert joystick"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4003, list_item, "expected 'look sensitivity' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4005, spinner, "expected 'look sensitivity' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			profile->controller_settings.look_sensitivity = (byte)(spinner->list.item_index + 1);
			break;
		default:
			error(_error_silent, "unknown option selected for look sensitivity");
			break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4024, list_item, "expected 'controller vibration' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4026, spinner, "expected 'controller vibration' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: profile->controller_settings.vibration_disabled = FALSE; break;
		case 1: profile->controller_settings.vibration_disabled = TRUE; break;
		default: error(_error_silent, "unknown option selected for controller vibration"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4035, list_item, "expected 'flight stick controls' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4037, spinner, "expected 'flight stick controls' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: profile->controller_settings.flight_stick_aircraft_controls = TRUE; break;
		case 1: profile->controller_settings.flight_stick_aircraft_controls = FALSE; break;
		default: error(_error_silent, "unknown option selected for controller flight_stick_aircraft_controls"); break;
		}

		list_item = list_item->next;
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4046, list_item, "expected 'autocenter' list item");

		for (spinner = list_item->first_child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4048, spinner, "expected 'autocenter' option spinner list");

		switch (spinner->list.item_index)
		{
		case 0: profile->controller_settings.autocenter = TRUE; break;
		case 1: profile->controller_settings.autocenter = FALSE; break;
		default: error(_error_silent, "unknown option selected for controller autocenter"); break;
		}
	}
	else
	{
		error(_error_silent, "failed to retrieve editable player profile");
		success = FALSE;
	}

	return success;
}

static boolean network_game_remove_local_player(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4073, event && (event->controller_index>=0) && (event->controller_index<MAXIMUM_GAMEPADS), "valid controller index required to remove player from network game");
	network_game_client_local_player_quit(event->controller_index);

	return TRUE;
}

static boolean switch_from_main_menu_to_single_player(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	game_connection_set(_game_connection_local);
	main_menu_switch_to_single_player();
	player_ui_remember_player1_profile(FALSE);

	return TRUE;
}

static boolean delete_player_profile_request(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct ui_widget_definition *definition;
	struct widget_instance *profile_list;
	boolean success;

	definition = ui_widget_definition_get(widget->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4111, definition->type == _ui_widget_type_container && definition->child_widgets.count >= 3, "expected the multiplayer profile select screen to be a container w/ 3+ children");
	profile_list = widget->first_child;
	definition = ui_widget_definition_get(profile_list->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4114, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'multiplayer profile list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4115, definition->child_widgets.count == 3, "expected 3 list items for 'multiplayer profile list' widget");
	profile_list = widget->first_child;
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4124, (profile_list->list.item_index>=0) && (profile_list->list.item_index<profile_list->list.item_count), "invalid multiplayer profile specified from 'multiplayer profile list' list widget");
	profile_index_to_delete = ((long *)profile_list->list.items)[profile_list->list.item_index];

	if (profile_index_to_delete != NONE)
	{
		success = TRUE;
	}
	else
	{
		ui_play_audio_feedback_sound(_audio_feedback_failure);
		success = FALSE;
	}

	return success;
}

static boolean delete_playlist_profile_request(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct ui_widget_definition *definition;
	struct widget_instance *profile_list;
	boolean success = FALSE;

	definition = ui_widget_definition_get(widget->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4157, definition->type == _ui_widget_type_container && definition->child_widgets.count >= 3, "expected the playlist profile select screen to be a container w/ 3+ children");
	profile_list = widget->first_child;
	definition = ui_widget_definition_get(profile_list->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4160, definition->type == _ui_widget_type_spinner_list, "expected a spinner list widget for 'playlist profile list' widget");
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4161, definition->child_widgets.count == 3, "expected 3 list items for 'playlist profile list' widget");
	profile_list = widget->first_child;
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4169, (profile_list->list.item_index>=0) && (profile_list->list.item_index<profile_list->list.item_count), "invalid multiplayer profile specified from 'multiplayer profile list' list widget");
	profile_index_to_delete = ((long *)profile_list->list.items)[profile_list->list.item_index];

	if (profile_index_to_delete != NONE)
	{
		if (TEST_FLAG(profile_index_to_delete, _default_profile_bit30))
		{
			display_error_deferred(_error_cannot_delete_default_game_settings, NONE, TRUE, FALSE);
			ui_play_audio_feedback_sound(_audio_feedback_failure);
		}
		else
		{
			success = TRUE;
		}
	}
	else
	{
		ui_play_audio_feedback_sound(_audio_feedback_failure);
	}

	return success;
}

static boolean delete_player_profile_final(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	boolean success = FALSE;

	if (!TEST_FLAG(profile_index_to_delete, _default_profile_bit30))
	{
		if ((profile_index_to_delete & SAVED_GAME_FILE_TYPE_MASK) == _saved_game_file_type_player_profile)
		{
			player_profile_delete(profile_index_to_delete);
			success = TRUE;
		}
		else
		{
			error(_error_silent, "#0x%08lX is not a player profile index", profile_index_to_delete);
		}
	}
	else
	{
		error(_error_silent, "sorry, you are not allowed to delete default player profiles");
		success = FALSE;
	}

	return success;
}

static boolean delete_playlist_profile_final(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	boolean success = FALSE;

	if ((profile_index_to_delete & SAVED_GAME_FILE_TYPE_MASK) == _saved_game_file_type_play_list)
	{
		playlist_profile_delete(profile_index_to_delete);
		success = TRUE;
	}
	else
	{
		error(_error_silent, "#0x%08lX is not a playlist profile index", profile_index_to_delete);
	}

	return success;
}

static boolean cancel_profile_delete(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	profile_index_to_delete = NONE;

	return TRUE;
}

static boolean create_and_begin_editing_new_gametype_profile(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	wchar_t new_profile_name[MAXIMUM_SAVED_GAME_NAME_LENGTH];
	char full_path[MAXIMUM_FILENAME_LENGTH+1];
	boolean success = FALSE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4272, (event->controller_index>=0) && (event->controller_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS), "creating a new profile requires a valid local player index");

	saved_game_file_get_useable_untitled_profile_name(new_profile_name);

	if (new_profile_name[0])
	{
		long profile_index = playlist_profile_new(widget->local_player_index, new_profile_name);

		if (profile_index != NONE)
		{
			struct game_variant *variant;

			player_ui_begin_editing_profile(profile_index);
			variant = player_ui_get_edit_playlist_profile();

			if (variant)
			{
				struct game_variant slayer = build_game_variant_slayer();

				memcpy(variant, &slayer, sizeof(*variant));
				variant->flags = 0;
				ustrncpy(variant->human_readable_game_description, new_profile_name, MAXIMUM_GAME_VARIANT_NAME_LENGTH - 1);
				variant->human_readable_game_description[MAXIMUM_GAME_VARIANT_NAME_LENGTH - 1] = L'\0';
				success = virtual_keyboard_launch(
					variant->human_readable_game_description,
					MAXIMUM_GAME_VARIANT_NAME_LENGTH * sizeof(wchar_t),
					_virtual_keyboard_caption_variant_name);

				if (success == TRUE)
				{
					if (saved_game_file_get_path_to_enclosing_directory(profile_index, full_path))
					{
						saved_game_file_remember_last_used_multiplayer_variant_directory(full_path);
					}
				}
			}
			else
			{
				error(_error_silent, "failed to retrieve editable game variant profile!");
				player_ui_end_editing_profile();
			}
		}
		else
		{
			error(_error_silent, "failed to create a new multiplayer game type profile");
		}
	}
	else
	{
		error(_error_silent, "unable to create a new untitled profile");
	}

	if (!success)
	{
		display_error_deferred(_error_unable_to_create_multiplayer_game_file, NONE, TRUE, FALSE);
		ui_play_audio_feedback_sound(_audio_feedback_failure);
	}

	return success;
}

static boolean create_and_begin_editing_new_player_profile(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	wchar_t new_profile_name[MAXIMUM_SAVED_GAME_NAME_LENGTH];
	short local_player_index = event->controller_index;
	boolean success = FALSE;

	if (local_player_index == NONE)
	{
		local_player_index = PLAYER1;
	}

	saved_game_file_get_useable_untitled_profile_name(new_profile_name);

	if (new_profile_name[0])
	{
		long profile_index = player_profile_new(local_player_index, new_profile_name);

		if (profile_index != NONE)
		{
			struct player_profile *profile;

			player_ui_begin_editing_profile(profile_index);
			profile = player_ui_get_edit_player_profile();

			if (profile)
			{
				ustrncpy(profile->player_name, new_profile_name, MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
				profile->player_name[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = L'\0';
				success = virtual_keyboard_launch(
					profile->player_name,
					(MAXIMUM_PLAYER_PROFILE_NAME_LENGTH + 1) * sizeof(wchar_t),
					_virtual_keyboard_caption_player_profile_name);
			}
			else
			{
				error(_error_silent, "failed to retrieve editable player profile!");
				player_ui_end_editing_profile();
			}
		}
		else
		{
			error(_error_silent, "failed to create a new player profile");
		}
	}
	else
	{
		error(_error_silent, "unable to create a new untitled profile");
	}

	if (!success)
	{
		display_error_deferred(_error_unable_to_create_player_profile, NONE, TRUE, FALSE);
		ui_play_audio_feedback_sound(_audio_feedback_failure);
	}

	return success;
}

static boolean network_game_start_faster(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct network_game_client *client = global_network_game_client_get();

	if (client)
	{
		long player_index;
		struct network_game_data *game = network_game_client_get_game(client);
		short machine_index = network_game_client_get_machine_index(client);

		for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
		{
			if (network_player_is_valid(&game->players[player_index]) && game->players[player_index].machine_index == machine_index && game->players[player_index].controller_index == event->controller_index)
			{
				if (!network_game_client_request_start_time_change(client, TRUE))
				{
					error(_error_silent, "network_game_client_request_start_time_change() failed");
				}
				break;
			}
		}
	}

	return TRUE;
}

static boolean network_game_start_slower(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct network_game_client *client = global_network_game_client_get();

	if (client)
	{
		long player_index;
		struct network_game_data *game = network_game_client_get_game(client);
		short machine_index = network_game_client_get_machine_index(client);

		for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
		{
			if (network_player_is_valid(&game->players[player_index]) && game->players[player_index].machine_index == machine_index && game->players[player_index].controller_index == event->controller_index)
			{
				if (!network_game_client_request_start_time_change(client, FALSE))
				{
					error(_error_silent, "network_game_client_request_start_time_change() failed");
				}
				break;
			}
		}
	}

	return TRUE;
}

static boolean network_game_server_accept_connections(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct network_game_server *server = global_network_game_server_get();

	if (server)
	{
		network_game_server_open_game(server);
	}

	return TRUE;
}

static boolean network_game_server_defer_game_start(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct network_game_server *server = global_network_game_server_get();

	if (server)
	{
		network_game_server_pause_countdown(server, TRUE);
	}

	return TRUE;
}

static boolean network_game_server_allow_game_start(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct network_game_server *server = global_network_game_server_get();

	if (server)
	{
		network_game_server_pause_countdown(server, FALSE);
	}

	return TRUE;
}

static boolean disable_widget_if_no_xdemos(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	if (!xbox_demos_available())
	{
		widget->disabled = TRUE;
		widget->visible = FALSE;
	}

	return TRUE;
}

static boolean run_xdemos(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	main_run_demos();

	return TRUE;
}

static boolean single_player_reset_controller_choices(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	player_ui_reset_single_player_local_player_controllers();

	return TRUE;
}

static boolean single_player_set_player1_controller_choice(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4537, event != NULL);
	player_ui_set_single_player_local_player_controller(PLAYER1, event->controller_index);

	return TRUE;
}

static boolean single_player_set_player2_controller_choice(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	short controller_index;
	boolean success;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4551, event != NULL);

	controller_index = event->controller_index;

	if (controller_index == player_ui_get_single_player_local_player_controller(PLAYER1))
	{
		display_error(_error_controller_coop_controller_conflict, NONE, TRUE, FALSE);
		*widget_deleted = TRUE;
		success = FALSE;
	}
	else
	{
		player_ui_set_single_player_local_player_controller(PLAYER2, controller_index);
		success = TRUE;
	}

	return success;
}

static boolean display_error_if_no_network_connection(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	boolean network_available = transport_network_available();

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4575, event);

	if (!network_available)
	{
		display_error(_error_network_connected_play_no_network, event->controller_index, TRUE, TRUE);
	}

	return network_available;
}

static boolean start_network_game_if_no_advertised_servers(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	boolean success = FALSE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4593, widget->type == _ui_widget_type_column_list, "expected a column list for server list");

	if (!widget->list.item_count)
	{
		word client_state_progress;
		struct network_game_client *client = global_network_game_client_get();

		if (client && network_game_client_get_state(client, &client_state_progress) == _network_game_client_state_searching)
		{
			success = network_game_start_new_server(widget, event, widget_deleted);
		}
	}
	else
	{
		error(_error_silent, "not attempting to start a new server; there are other servers available");
	}

	return success;
}

static boolean netgame_unjoin_player(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct network_game_client *client;
	boolean success = TRUE;

	client = global_network_game_client_get();

	if (client)
	{
		word client_state_progress;

		if (network_game_client_get_state(client, &client_state_progress) == _network_game_client_state_pregame)
		{
			struct network_game_data *game = network_game_client_get_game(client);
			struct network_player *removed_player = NULL;
			short local_machine_index = network_game_client_get_local_machine_index();
			long local_player_count = 0;

			if (local_machine_index != NONE)
			{
				long player_index;

				for (player_index = 0; player_index < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; player_index++)
				{
					struct network_player *player = &game->players[player_index];

					if (network_player_is_valid(player) && player->machine_index == local_machine_index)
					{
						local_player_count++;

						if (player->controller_index == event->controller_index)
						{
							match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4646, !removed_player, "duplicate player registered in game");
							removed_player = player;
						}
					}
				}

				if (local_player_count > 0)
				{
					if (removed_player)
					{
						if (!network_game_client_request_remove_player(client, removed_player))
						{
							error(_error_silent, "failed to request player removal");
						}

						player_ui_clear_multiplayer_autojoin_for_local_player(removed_player->controller_index);
					}

					if (local_player_count == 1)
					{
						if (global_network_game_server_get() && network_game_should_accept_remote_connections() == TRUE)
						{
							success = network_game_server_defer_game_start(widget, event, widget_deleted);
						}
						else
						{
							dispose_global_network_game_client();
							dispose_global_network_game_server();
							success = TRUE;
						}

						player_ui_autojoin_players_to_next_multiplayer_game();
					}
					else
					{
						success = FALSE;
					}
				}
			}
		}
		else
		{
			error(_error_silent, "can't request player removal from netgame_unjoin_player() unless we are in pregame");
		}
	}

	return success;
}

static boolean close_calling_widget_if_not_editing_profile(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	boolean success;

	if (!player_ui_get_edit_player_profile() && !player_ui_get_edit_playlist_profile())
	{
		struct widget_instance *topmost_parent = widget_instance_get_topmost_parent(widget);

		error(_error_silent, "closing widget '%s' because no saved game file is being edited", topmost_parent->name);
		topmost_parent->milliseconds_to_auto_close = 1;
		topmost_parent->visible = FALSE;
		success = FALSE;
	}
	else
	{
		success = TRUE;
	}

	return success;
}

static boolean exit_to_xbox_dashboard(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	xbox_dashboard_launch();

	return FALSE;
}

static boolean new_campaign_chosen(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	wchar_t random_name[MAXIMUM_SAVED_GAME_NAME_LENGTH];

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4741, event);

	saved_game_file_get_useable_untitled_profile_name(random_name);
	ustrncpy(new_campaign_profile_name, random_name, MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
	new_campaign_profile_name[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = L'\0';
	new_campaign_controller_index = event->controller_index;

	if (!virtual_keyboard_launch(
		new_campaign_profile_name,
		(MAXIMUM_PLAYER_PROFILE_NAME_LENGTH + 1) * sizeof(wchar_t),
		_virtual_keyboard_caption_player_profile_name))
	{
		error(_error_silent, "failed to invoke the virtual keyboard for a new campaign profile name");
	}

	return TRUE;
}

static boolean new_campaign_decision(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	wchar_t random_name[MAXIMUM_SAVED_GAME_NAME_LENGTH];
	struct player_profile profile;
	boolean success = FALSE;

	if (new_campaign_controller_index != NONE)
	{
		if (virtual_keyboard_last_exit_saved_text())
		{
			if (new_campaign_profile_name[0])
			{
				long profile_index;

				player_ui_set_single_player_local_player_controller(PLAYER1, new_campaign_controller_index);
				profile_index = player_profile_new(new_campaign_controller_index, new_campaign_profile_name);

				if (profile_index == NONE)
				{
					saved_game_file_get_useable_untitled_profile_name(random_name);
					ustrncpy(new_campaign_profile_name, random_name, MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
					new_campaign_profile_name[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = L'\0';
					profile_index = player_profile_new(new_campaign_controller_index, new_campaign_profile_name);
				}

				if (profile_index != NONE)
				{
					if (player_profile_get(profile_index, &profile))
					{
						player_ui_set_active_player_profile(PLAYER1, profile_index, &profile);
						success = TRUE;
					}
					else
					{
						error(_error_silent, "failed to retrieve newly created player profile");
					}
				}
				else
				{
					error(_error_silent, "failed to create new player profile");
				}

				if (success == TRUE)
				{
					main_set_map_name(single_player_level_names[_single_player_map_a10]);
					main_defer_map_map_change();
				}
				else
				{
					main_goto_main_menu();
					display_error_deferred(_error_unable_to_create_player_profile, NONE, TRUE, FALSE);
					ui_play_audio_feedback_sound(_audio_feedback_failure);
				}
			}
			else
			{
				error(_error_silent, "can't create a new profile with an empty name");
				ui_play_audio_feedback_sound(_audio_feedback_failure);
			}
		}

		new_campaign_controller_index = NONE;
	}

	return success;
}

static boolean go_back_twice_next_time(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	ui_widgets_pop_stack(widget->local_player_index);

	return TRUE;
}

static boolean difficulty_menu_initialize(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct widget_instance *difficulty_item;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4860, widget->type == _ui_widget_type_column_list, "expected column list for difficulty menu widget");

	if (persistant_game_data_info.valid == TRUE && !_stricmp(persistant_game_data_info.map_name, main_get_map_name()))
	{
		difficulty_item = widget_instance_get_nth_child(widget, persistant_game_data_info.difficulty);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4865, difficulty_item, "failed to find 'difficulty' menu item");
		widget->list.item_index = persistant_game_data_info.difficulty;
		widget->focused_child = difficulty_item;
	}
	else
	{
		difficulty_item = widget_instance_get_nth_child(widget, _game_difficulty_level_normal);
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget_event_handler_functions.c", 4871, difficulty_item, "failed to find 'difficulty' menu item");
		widget->focused_child = difficulty_item;
		widget->list.item_index = _game_difficulty_level_normal;
	}

	return TRUE;
}

static boolean begin_music_fade_out(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	if (ui_main_menu_music_active())
	{
		ui_stop_main_menu_music();
	}

	return TRUE;
}

static boolean new_campaign_if_no_custom_player_profiles_exist(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	long unused_profile_index;
	boolean success;
	short number_of_custom_profiles = 1;

	player_profiles_enumerate_available_to_local_player_index(NONE, (word *)&number_of_custom_profiles, &unused_profile_index, FALSE);

	if (number_of_custom_profiles > 0)
	{
		success = TRUE;
	}
	else
	{
		success = FALSE;
		new_campaign_chosen(widget, event, widget_deleted);
	}

	return success;
}

/* ---------- private code */
