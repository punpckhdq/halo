/*
UI_WIDGET.H

header included in hcex build.
*/

#ifndef __UI_WIDGET_H
#define __UI_WIDGET_H
#pragma once

/* ---------- headers */

#include "ui_widget_group.h"

/* ---------- constants */

enum
{
	_error_unknown = 0,
	_error_network_generic,
	_error_network_join_game_closed,
	_error_network_join_game_generic,
	_error_network_server_shut_down,
	_error_network_connected_play_no_network,
	_error_network_connection_lost,
	_error_network_failed_to_join_game,
	_error_network_out_of_sync_alert,
	_error_network_trouble_is_brewing,
	_error_network_unused10,
	_error_controller_generic,
	_error_controller_unplugged_start_to_continue,
	_error_controller_unplugged,
	_error_controller_memory_card,
	_error_controller_saving_file_warning,
	_error_controller_creating_player_profile_warning,
	_error_controller_creating_game_settings_file_warning,
	_error_controller_coop_controller_conflict,
	_error_controller_coop_requires_two_controllers,
	_error_controller_unused10,
	_error_insert_quarter,
	_error_warning_revert_to_saved_progress_will_be_lost,
	_error_warning_restart_level_progress_will_be_lost,
	_error_warning_delete_player_profile,
	_error_warning_delete_multiplayer_profile,
	_error_cannot_delete_default_game_settings,
	_error_already_a_saved_game_file_with_that_name,
	_error_warning_are_you_sure_you_want_to_discard_changes,
	_error_cannot_create_saved_game_file_with_empty_name,
	_error_warning_saving_checkpoint,
	_error_warning_saved_game_file_damaged,
	_error_warning_unused10,
	_error_hard_drive_not_enough_free_space,
	_error_hard_drive_maximum_saved_game_files,
	_error_media_damaged,
	_error_maximum_game_files_created,
};

enum
{
	_audio_feedback_none = 0,
	_audio_feedback_cursor,
	_audio_feedback_forward,
	_audio_feedback_backward,
	_audio_feedback_failure,
	NUMBER_OF_AUDIO_FEEDBACK_SOUNDS
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/UI_WIDGET.C */

void set_ui_plasma_effect_color(real a, real r, real g, real b);
void ui_widgets_initialize(void);
void ui_widgets_safe_to_load(boolean safe);
void ui_widgets_set_fade_value(real value);
void ui_widget_debug_show_path(boolean show);
int widget_instance_count_children(struct widget_instance *widget);
struct widget_instance *widget_instance_get_nth_child(struct widget_instance *widget, int n);
void *ui_widget_realloc(void *ptr, word size);
void widget_free(void *ptr);
boolean ui_widgets_active(void);
boolean ui_widgets_active_for_local_player(short local_player_index);
void ui_widgets_inhibit_processing(boolean inhibit);
void draw_bitmap_in_rect(struct bitmap_data *bitmap, rectangle2d *rect, rectangle2d *bitmap_rect, rectangle2d *clip_rect, pixel32 argb, struct rasterizer_dynamic_screen_geometry_parameters *multitexture_params, boolean no_plasma);
struct widget_instance *widget_instance_get_topmost_parent(struct widget_instance *widget);
int widget_instance_get_child_index_from_parent(struct widget_instance *widget);
void widget_instance_set_visibility_recursive(struct widget_instance *widget, boolean visible);
void main_menu_active(boolean active);
boolean main_menu_is_active(void);
boolean main_menu_screen_is_active(void);
void ui_set_next_level(short level);
void ui_widget_load_progress_widget(void);
boolean filesystem_check_thread_is_active(void);
void display_error_when_main_menu_loaded(short error_code);
void display_error_deferred(short error_code, short local_player_index, boolean modal, boolean pause_game_time);
void display_error_abort_to_dashboard_deferred(short error_code, boolean optional);
void ui_start_main_menu_music(void);
void ui_stop_main_menu_music(void);
boolean ui_main_menu_music_active(void);
void ui_widgets_disable_pause_game(long duration_ticks);
short remap_sticks_for_local_player(short icon, short local_player_index);
real_rgb_color get_ui_rgb_white(void);
real_argb_color get_ui_argb_white(void);
pixel32 modulate_pixel32_by_real_alpha(pixel32 argb, real alpha);
void ui_widget_delete(struct widget_instance *widget);
void ui_widgets_close_all(void);
void ui_widgets_close_all_for_local_player(short local_player_index);
void ui_widgets_delete_history(void);
void ui_widgets_pop_stack(short local_player_index);
void main_screen_shell_begin_fade(unsigned long fade_duration_milliseconds);
void ui_play_audio_feedback_sound(short audio_feedback);
void draw_string_and_hack_in_icons(rectangle2d *bounds, rectangle2d *clip, point2d *cursor_reference, short height_adjust, wchar_t const *instring, boolean ignore_icon_color);
void ui_widgets_dispose(void);
boolean widget_event_function_list_widget_goto_next_item(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
boolean widget_event_function_list_widget_goto_previous_item(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
void render_ui_widgets_postgame(short local_player_index, rectangle2d *window_bounds);
void render_ui_widgets(short local_player_index, rectangle2d const *window_bounds);
struct widget_instance *ui_widget_load_by_name_or_tag(char *name, long tag_index, struct widget_instance *parent, short local_player_index, long invoking_widget_tag, long focused_child_parent_widget_tag, short focused_child_index);
void main_screen_shell_load(void);
void network_game_reset_to_pregame_ui(void);
void display_error(short error_code, short local_player_index, boolean modal, boolean pause_game_time);
void display_error_abort_to_dashboard(short error_code, boolean optional);
void display_error_damaged_media(void);
void display_errors_deferred_until_cinematic_stop(void);
void display_scenario_help(short string_index);
void process_ui_widgets(void);

/* ---------- globals */

extern short dashboard_abort_error;

/* ---------- public code */

#endif // __UI_WIDGET_H
