/*
UI_WIDGET_GROUP.H

header included in hcex build.
*/

#ifndef __UI_WIDGET_GROUP_H
#define __UI_WIDGET_GROUP_H
#pragma once

/* ---------- constants */

enum
{
	UI_WIDGET_DEFINITION_TAG = 'DeLa',
	UI_WIDGET_DEFINITION_VERSION = 1,
	UI_WIDGET_COLLECTION_TAG = 'Soul',
	UI_WIDGET_COLLECTION_VERSION = 1,
};

enum
{
	_ui_widget_type_container = 0,
	_ui_widget_type_text_box,
	_ui_widget_type_spinner_list,
	_ui_widget_type_column_list,
	_ui_widget_type_game_model,
	_ui_widget_type_movie,
	_ui_widget_type_custom,
	NUMBER_OF_UI_WIDGET_TYPES,
};

enum
{
	_widget_controller0 = 0,
	_widget_controller1,
	_widget_controller2,
	_widget_controller3,
	_widget_controller_any,
	NUMBER_OF_WIDGET_CONTROLLER_OPTIONS,
};

enum
{
	_widget_pass_unhandled_events_to_children_bit = 0,
	_widget_pause_game_time_bit,
	_widget_flash_background_bitmap_bit,
	_widget_dpad_updown_tabs_thru_children_bit,
	_widget_dpad_leftright_tabs_thru_children_bit,
	_widget_dpad_updown_tabs_thru_list_items_bit,
	_widget_dpad_leftright_tabs_thru_list_items_bit,
	_widget_dont_focus_a_specific_child_bit,
	_widget_pass_unhandled_events_to_all_children_bit,
	_widget_render_regardless_of_controller_index_bit,
	_widget_pass_handled_events_to_all_children_bit,
	_widget_return_to_main_menu_if_no_history_bit,
	_widget_always_use_tag_controller_index_bit,
	_widget_always_render_with_nifty_fx_bit,
	_widget_dont_push_history_data_bit,
	NUMBER_OF_WIDGET_FLAGS,
};

enum
{
	_game_data_input_function_null = 0, /* fake name */
	_game_data_input_function_player_settings_menu_update_description, /* fake name */
	_game_data_input_function_unused, /* fake name */
	_game_data_input_function_playlist_settings_menu_update_description, /* fake name */
	_game_data_input_function_gametype_select_menu_update_description, /* fake name */
	_game_data_input_function_multiplayer_type_menu_update_description, /* fake name */
	_game_data_input_function_solo_level_select_update, /* fake name */
	_game_data_input_function_difficulty_menu_update_description, /* fake name */
	_game_data_input_function_build_number, /* fake name */
	_game_data_input_function_server_list_update, /* fake name */
	_game_data_input_function_network_pregame_status_update, /* fake name */
	_game_data_input_function_splitscreen_pregame_status_update, /* fake name */
	_game_data_input_function_network_splitscreen_prejoin_players, /* fake name */
	_game_data_input_function_multiplayer_profile_list_update, /* fake name */
	_game_data_input_function_player_profile_list_3wide_update, /* fake name */
	_game_data_input_function_player_profile_edit_select_menu_update, /* fake name */
	_game_data_input_function_player_profile_small_menu_update, /* fake name */
	_game_data_input_function_game_settings_lists_text_update, /* fake name */
	_game_data_input_function_solo_game_objective_text, /* fake name */
	_game_data_input_function_color_picker_update, /* fake name */
	_game_data_input_function_game_settings_lists_picture_update, /* fake name */
	_game_data_input_function_main_menu_fake_animate, /* fake name */
	_game_data_input_function_multiplayer_level_select_update, /* fake name */
	_game_data_input_function_get_active_player_profile_name, /* fake name */
	_game_data_input_function_get_edit_player_profile_name, /* fake name */
	_game_data_input_function_get_edit_game_settings_name, /* fake name */
	_game_data_input_function_get_active_player_profile_color, /* fake name */
	_game_data_input_function_multiplayer_set_textbox_map_name, /* fake name */
	_game_data_input_function_multiplayer_set_textbox_game_ruleset, /* fake name */
	_game_data_input_function_multiplayer_set_textbox_teams_noteams, /* fake name */
	_game_data_input_function_multiplayer_set_textbox_score_limit, /* fake name */
	_game_data_input_function_multiplayer_set_textbox_score_limit_type, /* fake name */
	_game_data_input_function_multiplayer_set_bitmap_for_map, /* fake name */
	_game_data_input_function_multiplayer_set_bitmap_for_ruleset, /* fake name */
	_game_data_input_function_multiplayer_set_textbox_number_of_players, /* fake name */
	_game_data_input_function_multiplayer_edit_profile_set_rule_text, /* fake name */
	_game_data_input_function_system_link_status_check, /* fake name */
	_game_data_input_function_multiplayer_game_directions, /* fake name */
	_game_data_input_function_teams_no_teams_bitmap_update, /* fake name */
	_game_data_input_function_warn_if_difficulty_will_delete_saved_game, /* fake name */
	_game_data_input_function_dim_if_no_network_cable, /* fake name */
	NUMBER_OF_GAME_DATA_INPUT_FUNCTIONS, /* fake name */
};

enum
{
	_event_handler_close_current_widget_bit = 0,
	_event_handler_close_other_widget_bit,
	_event_handler_close_all_widgets_bit,
	_event_handler_open_widget_bit,
	_event_handler_reload_self_bit,
	_event_handler_reload_widget_bit,
	_event_handler_give_focus_to_widget,
	_event_handler_run_function_bit,
	_event_handler_replace_with_other_widget_bit,
	_event_handler_go_back_to_previous_widget_bit,
	_event_handler_run_scenario_script_bit,
	_event_handler_look_for_conditional_widget_on_failure_bit,
	NUMBER_OF_EVENT_HANDLER_FLAGS,
};

enum
{
	_widget_event_button_a = 0,
	_widget_event_button_b,
	_widget_event_button_x,
	_widget_event_button_y,
	_widget_event_button_black,
	_widget_event_button_white,
	_widget_event_button_left_trigger,
	_widget_event_button_right_trigger,
	_widget_event_button_dpad_up,
	_widget_event_button_dpad_down,
	_widget_event_button_dpad_left,
	_widget_event_button_dpad_right,
	_widget_event_button_start,
	_widget_event_button_back,
	_widget_event_button_left_thumb,
	_widget_event_button_right_thumb,
	NUMBER_OF_WIDGET_EVENT_BUTTONS,

	_widget_event_left_analog_stick_up = NUMBER_OF_WIDGET_EVENT_BUTTONS,
	_widget_event_left_analog_stick_down,
	_widget_event_left_analog_stick_left,
	_widget_event_left_analog_stick_right,
	_widget_event_right_analog_stick_up,
	_widget_event_right_analog_stick_down,
	_widget_event_right_analog_stick_left,
	_widget_event_right_analog_stick_right,
	_widget_event_created,
	_widget_event_deleted,
	_widget_event_get_focus,
	_widget_event_lose_focus,
	NUMBER_OF_WIDGET_EVENTS,
};

enum
{
	_event_handler_function_null = 0, /* fake name */
	_event_handler_function_list_goto_next_item, /* fake name */
	_event_handler_function_list_goto_previous_item, /* fake name */
	_event_handler_function_unused1, /* fake name */
	_event_handler_function_unused2, /* fake name */
	_event_handler_function_initialize_single_player_level_list_solo, /* fake name */
	_event_handler_function_initialize_single_player_level_list_coop, /* fake name */
	_event_handler_function_dispose_single_player_level_list, /* fake name */
	_event_handler_function_solo_level_set_map, /* fake name */
	_event_handler_function_set_difficulty, /* fake name */
	_event_handler_function_start_new_game, /* fake name */
	_event_handler_function_pause_game_restart_at_checkpoint, /* fake name */
	_event_handler_function_pause_game_restart_level, /* fake name */
	_event_handler_function_pause_game_return_to_main_menu, /* fake name */
	_event_handler_function_clear_multiplayer_player_joins, /* fake name */
	_event_handler_function_join_controller_to_multiplayer_game, /* fake name */
	_event_handler_function_initialize_network_game_server_list, /* fake name */
	_event_handler_function_start_network_game_server, /* fake name */
	_event_handler_function_dispose_network_game_server_list, /* fake name */
	_event_handler_function_shutdown_network_game, /* fake name */
	_event_handler_function_network_game_join_from_server_list, /* fake name */
	_event_handler_function_split_screen_game_initialize, /* fake name */
	_event_handler_function_coop_game_initialize, /* fake name */
	_event_handler_function_main_menu_initialize, /* fake name */
	_event_handler_function_multiplayer_type_menu_initialize, /* fake name */
	_event_handler_function_pick_play_stage_for_quick_start, /* fake name */
	_event_handler_function_multiplayer_level_list_initialize, /* fake name */
	_event_handler_function_multiplayer_level_list_dispose, /* fake name */
	_event_handler_function_multiplayer_level_select, /* fake name */
	_event_handler_function_multiplayer_profiles_list_initialize, /* fake name */
	_event_handler_function_multiplayer_profiles_list_dispose, /* fake name */
	_event_handler_function_multiplayer_profile_set_for_game, /* fake name */
	_event_handler_function_swap_player_team, /* fake name */
	_event_handler_function_network_game_join_player, /* fake name */
	_event_handler_function_player_profile_list_initialize, /* fake name */
	_event_handler_function_player_profile_list_dispose, /* fake name */
	_event_handler_function_player_profile_set_for_game_3wide, /* fake name */
	_event_handler_function_player_profile_set_for_game_1wide, /* fake name */
	_event_handler_function_multiplayer_profile_begin_editing, /* fake name */
	_event_handler_function_multiplayer_profile_end_editing, /* fake name */
	_event_handler_function_multiplayer_profile_set_game_engine, /* fake name */
	_event_handler_function_multiplayer_profile_change_name, /* fake name */
	_event_handler_function_multiplayer_profile_set_ctf_rules, /* fake name */
	_event_handler_function_multiplayer_profile_set_koth_rules, /* fake name */
	_event_handler_function_multiplayer_profile_set_slayer_rules, /* fake name */
	_event_handler_function_multiplayer_profile_set_oddball_rules, /* fake name */
	_event_handler_function_multiplayer_profile_set_racing_rules, /* fake name */
	_event_handler_function_multiplayer_profile_set_player_options, /* fake name */
	_event_handler_function_multiplayer_profile_set_item_options, /* fake name */
	_event_handler_function_multiplayer_profile_set_indicator_options, /* fake name */
	_event_handler_function_multiplayer_profile_initialize_game_engine, /* fake name */
	_event_handler_function_multiplayer_profile_initialize_name, /* fake name */
	_event_handler_function_multiplayer_profile_initialize_ctf_rules, /* fake name */
	_event_handler_function_multiplayer_profile_initialize_koth_rules, /* fake name */
	_event_handler_function_multiplayer_profile_initialize_slayer_rules, /* fake name */
	_event_handler_function_multiplayer_profile_initialize_oddball_rules, /* fake name */
	_event_handler_function_multiplayer_profile_initialize_racing_rules, /* fake name */
	_event_handler_function_multiplayer_profile_initialize_player_options, /* fake name */
	_event_handler_function_multiplayer_profile_initialize_item_options, /* fake name */
	_event_handler_function_multiplayer_profile_initialize_indicator_options, /* fake name */
	_event_handler_function_multiplayer_profile_save_changes, /* fake name */
	_event_handler_function_color_picker_menu_initialize, /* fake name */
	_event_handler_function_color_picker_menu_dispose, /* fake name */
	_event_handler_function_color_picker_select_color, /* fake name */
	_event_handler_function_player_profile_begin_editing, /* fake name */
	_event_handler_function_player_profile_end_editing, /* fake name */
	_event_handler_function_player_profile_change_name, /* fake name */
	_event_handler_function_player_profile_save_changes, /* fake name */
	_event_handler_function_player_profile_initialize_controller_settings, /* fake name */
	_event_handler_function_player_profile_initialize_advanced_controller_settings, /* fake name */
	_event_handler_function_player_profile_save_controller_settings, /* fake name */
	_event_handler_function_player_profile_save_advanced_controller_settings, /* fake name */
	_event_handler_function_multiplayer_game_player_quit, /* fake name */
	_event_handler_function_main_menu_switch_to_solo_game, /* fake name */
	_event_handler_function_request_delete_player_profile, /* fake name */
	_event_handler_function_request_delete_playlist_profile, /* fake name */
	_event_handler_function_final_delete_player_profile, /* fake name */
	_event_handler_function_final_delete_playlist_profile, /* fake name */
	_event_handler_function_cancel_profile_delete, /* fake name */
	_event_handler_function_create_and_edit_playlist_profile, /* fake name */
	_event_handler_function_create_and_edit_player_profile, /* fake name */
	_event_handler_function_network_game_speed_start, /* fake name */
	_event_handler_function_network_game_delay_start, /* fake name */
	_event_handler_function_network_server_accept_connections, /* fake name */
	_event_handler_function_network_server_defer_start, /* fake name */
	_event_handler_function_network_server_allow_start, /* fake name */
	_event_handler_function_disable_if_no_xdemos, /* fake name */
	_event_handler_function_run_xdemos, /* fake name */
	_event_handler_function_single_player_reset_controller_choices, /* fake name */
	_event_handler_function_single_player_set_player1_controller_choice, /* fake name */
	_event_handler_function_single_player_set_player2_controller_choice, /* fake name */
	_event_handler_function_error_if_no_network_connection, /* fake name */
	_event_handler_function_start_server_if_none_advertised, /* fake name */
	_event_handler_function_network_game_unjoin_player, /* fake name */
	_event_handler_function_close_if_not_editing_profile, /* fake name */
	_event_handler_function_exit_to_xbox_dashboard, /* fake name */
	_event_handler_function_new_campaign_chosen, /* fake name */
	_event_handler_function_new_campaign_decision, /* fake name */
	_event_handler_function_pop_history_stack_once, /* fake name */
	_event_handler_function_difficulty_menu_initialize, /* fake name */
	_event_handler_function_begin_music_fade_out, /* fake name */
	_event_handler_function_new_game_if_no_player_profiles, /* fake name */
	NUMBER_OF_EVENT_HANDLER_FUNCTIONS, /* fake name */
};

enum
{
	_search_and_replace_function_null = 0, /* fake name */
	_search_and_replace_function_widget_controller, /* fake name */
	NUMBER_OF_SEARCH_AND_REPLACE_FUNCTIONS, /* fake name */
};

enum
{
	_text_box_editable_bit = 0,
	_text_box_password_bit,
	_text_box_flashing_text_bit,
	NUMBER_OF_TEXT_BOX_FLAGS,
};

enum
{
	_list_items_generated_in_code = 0,
	_list_items_generated_from_string_list_tag,
	NUMBER_OF_LIST_FLAGS,
};

enum
{
	_conditional_widget_load_if_event_handler_function_fails_bit = 0,
	NUMBER_OF_CONDITIONAL_WIDGET_FLAGS,
};

enum
{
	_child_widget_use_custom_controller_index_bit = 0,
	NUMBER_OF_CHILD_WIDGET_FLAGS,
};

/* ---------- macros */

#define GENERIC_WHITE_BITMAP_TAG_NAME "ui\\shell\\bitmaps\\white"																		/* fake name */
#define SOLO_WIDGET_COLLECTION_TAG_NAME "ui\\shell\\solo"																				/* fake name */
#define MULTIPLAYER_WIDGET_COLLECTION_TAG_NAME "ui\\shell\\multiplayer"																	/* fake name */
#define MAIN_MENU_WIDGET_COLLECTION_TAG_NAME "ui\\shell\\main_menu"																		/* fake name */
#define GAME_VARIANT_DESCRIPTIONS_TAG_NAME "ui\\shell\\strings\\game_variant_descriptions"												/* fake name */
#define DIFFICULTY_NAMES_TAG_NAME "ui\\shell\\main_menu\\player_profiles_select\\difficulty_names"										/* fake name */
#define BUTTON_SET_LONG_DESCRIPTIONS_TAG_NAME "ui\\shell\\main_menu\\player_profiles_select\\button_set_long_descriptions"				/* fake name */
#define BUTTON_SET_SHORT_DESCRIPTIONS_TAG_NAME "ui\\shell\\main_menu\\player_profiles_select\\button_set_short_descriptions"			/* fake name */
#define JOYSTICK_SET_DEFAULTS_DESCRIPTIONS_TAG_NAME "ui\\shell\\main_menu\\player_profiles_select\\joystick_set_defaults_descriptions"	/* fake name */
#define JOYSTICK_SET_SHORT_DESCRIPTIONS_TAG_NAME "ui\\shell\\main_menu\\player_profiles_select\\joystick_set_short_descriptions"		/* fake name */
#define PROFILE_DESCRIPTION_LABELS_TAG_NAME "ui\\shell\\main_menu\\player_profiles_select\\profile_description_labels"					/* fake name */
#define UI_CURSOR_SOUND_TAG_NAME "sound\\sfx\\ui\\cursor"																				/* fake name */
#define UI_FORWARD_SOUND_TAG_NAME "sound\\sfx\\ui\\forward"																				/* fake name */
#define UI_BACK_SOUND_TAG_NAME "sound\\sfx\\ui\\back"																					/* fake name */
#define UI_FAILURE_SOUND_TAG_NAME "sound\\sfx\\ui\\flag_failure"																		/* fake name */
#define MAIN_MENU_MUSIC_TAG_NAME "sound\\music\\title1\\title1"																			/* fake name */

#define ui_widget_definition_get(index) ((struct ui_widget_definition *)tag_get(UI_WIDGET_DEFINITION_TAG, (index))) /* fake name */

/* ---------- structures */

struct game_data_input_reference
{
	short function;
	short unused_flags;
	long unused[8];
};

struct event_handler_reference
{
	unsigned long flags;
	short event_type;
	short function;
	struct tag_reference ui_widget_tag;
	struct tag_reference sound_effect_tag;
	char script_command[TAG_STRING_LENGTH+1];
};

struct search_and_replace_reference
{
	char search_string[TAG_STRING_LENGTH+1];
	short replace_function;
};

struct conditional_widget_reference
{
	struct tag_reference ui_widget_tag;
	char name_unused[TAG_STRING_LENGTH+1];
	unsigned long flags;
	short custom_controller_index;
	word pad;
	long unused[6];
};

struct child_widget_reference
{
	struct tag_reference ui_widget_tag;
	char name_unused[TAG_STRING_LENGTH+1];
	unsigned long flags;
	short custom_controller_index;
	short vertical_offset;
	short horizontal_offset;
	word pad;
	long unused[5];
};

struct ui_widget_definition
{
	short type;
	short controller_index;
	char name[TAG_STRING_LENGTH+1];
	rectangle2d bounds;
	unsigned long flags;
	long millisecond_to_auto_close;
	long millisecond_auto_close_fade_time;
	struct tag_reference background_bitmap;
	struct tag_block game_data_input_references;		// game_data_input_reference
	struct tag_block event_handler_references;			// event_handler_reference
	struct tag_block search_and_replace_references;		// search_and_replace_reference
	long unused[32];

	// text box
	struct tag_reference text_label_unicode_string_list_tag;
	struct tag_reference font_tag;
	real_argb_color text_color;
	short justification;
	word text_box_flags; // the tag field is 32 bits wide, cachebeta reads only the low bits
	word pad1[7];
	short string_list_index;
	short hoffset;
	short voffset;
	long widget_type_text_box_unused[6];
	word pad2[2];

	// list items
	unsigned long list_flags;

	// spinner list
	struct tag_reference list_header_bitmap;
	struct tag_reference list_footer_bitmap;
	rectangle2d list_header_bounds;
	rectangle2d list_footer_bounds;
	long widget_type_spinner_list_unused[8];

	// column list
	struct tag_reference extended_description_widget_tag;
	long widget_type_column_list_unused[8];
	long widget_type_game_model_unused[64];

	struct tag_block conditional_widget_references;		// conditional_widget_reference
	long widget_type_movie_unused[32];
	long widget_type_custom_unused[32];
	struct tag_block child_widgets;						// child_widget_reference
};

struct ui_widget_reference
{
	struct tag_reference ui_widget_definition;
};

struct ui_widget_collection
{
	struct tag_block ui_widget_definitions;			// ui_widget_reference
};

/* ---------- prototypes/UI_WIDGET_GROUP.C */

void ui_load_tags_for_scenario(long scenario_index);

/* ---------- globals */

/* ---------- public code */

#endif // __UI_WIDGET_GROUP_H
