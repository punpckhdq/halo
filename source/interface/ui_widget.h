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
	_error_unable_to_create_player_profile,
	_error_unable_to_create_multiplayer_game_file,
	_error_saved_game_state_corrupt,
	NUMBER_OF_ERROR_CODES
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

struct _text_box_parameters
{
	wchar_t *text;
	short string_list_index;
};

struct _list_parameters
{
	short current_list_item_index;
	short last_list_tab_direction;
	void *list_items;
	word number_of_items;
	struct widget_instance *extended_description;
	wchar_t *item_text;
};

struct _animation_data
{
	short current_frame_index;
	short first_frame_index;
	short last_frame_index;
	short number_of_sprite_frames;
};

struct widget_instance
{
	long ui_widget_tag_index;
	char const *name;
	short local_player_index;
	short horizontal_offset;
	short vertical_offset;
	short widget_type;
	boolean visible;
	boolean render_regardless_of_controller_index;
	boolean never_receive_events;
	boolean pause_game_time;
	boolean delete_recursion_lock;
	boolean widget_is_error_dialog;
	boolean close_if_local_player_controller_present;
	unsigned long creation_time;
	unsigned long milliseconds_to_auto_close;
	unsigned long auto_close_fade_time;
	real alpha_modifier;
	struct widget_instance *previous;
	struct widget_instance *next;
	struct widget_instance *parent;
	struct widget_instance *children;
	struct widget_instance *focused_child;
	union
	{
		struct _text_box_parameters text_box_parameters;
		struct _list_parameters list_parameters;
	} parameters;
	struct _animation_data animation_data;
};

/* ---------- prototypes/UI_WIDGET.C */

boolean event_controller_index_compatible_with_widget(struct event_record const *event, struct widget_instance const *widget);
void set_ui_plasma_effect_color(real a, real r, real g, real b);
void ui_widgets_initialize(void);
void ui_widgets_dispose(void);
void ui_widgets_safe_to_load(boolean safe);
void ui_widgets_set_fade_value(real value);
void ui_widget_debug_show_path(boolean show);
pixel32 modulate_pixel32_by_real_alpha(pixel32 argb, real alpha);
struct widget_instance *ui_widget_load_by_name_or_tag(char const *name, long tag_index, struct widget_instance *parent, short local_player_index, long invoking_widget_tag, long focused_child_parent_widget_tag, short focused_child_index);
void ui_widget_delete(struct widget_instance *widget);
void process_ui_widgets(void);
void render_ui_widgets_postgame(short local_player_index, rectangle2d const *window_bounds);
void render_ui_widgets(short local_player_index, rectangle2d const *window_bounds);
int widget_instance_count_children(struct widget_instance const *widget);
struct widget_instance *widget_instance_get_nth_child(struct widget_instance const *widget, int n);
void *ui_widget_realloc(void *ptr, word size, char const *file, long line);
void widget_free(void *ptr);
boolean ui_widgets_active(void);
boolean ui_widgets_active_for_local_player(short local_player_index);
void ui_widgets_close_all(void);
void ui_widgets_close_all_for_local_player(short local_player_index);
void ui_widgets_inhibit_processing(boolean inhibit);
void ui_widgets_delete_history(void);
void ui_widgets_pop_stack(short local_player_index);
boolean widget_event_function_list_widget_goto_next_item(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
boolean widget_event_function_list_widget_goto_previous_item(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
void main_screen_shell_begin_fade(unsigned long fade_duration_milliseconds);
void ui_play_audio_feedback_sound(short audio_feedback);
void draw_bitmap_in_rect(struct bitmap_data *bitmap, rectangle2d const *rect, rectangle2d const *bitmap_rect, rectangle2d const *clip_rect, pixel32 argb, struct rasterizer_dynamic_screen_geometry_parameters *multitexture_params, boolean no_plasma);
struct widget_instance *widget_instance_get_topmost_parent(struct widget_instance *widget);
int widget_instance_get_child_index_from_parent(struct widget_instance const *widget);
void widget_instance_set_visibility_recursive(struct widget_instance *widget, boolean visible);
void main_menu_active(boolean active);
boolean main_menu_is_active(void);
boolean main_menu_screen_is_active(void);
void ui_set_next_level(short level);
void ui_widget_load_progress_widget(void);
boolean filesystem_check_thread_is_active(void);
void main_screen_shell_load(void);
void network_game_reset_to_pregame_ui(void);
void display_error(short error_code, short local_player_index, boolean modal, boolean pause_game_time);
void display_error_when_main_menu_loaded(short error_code);
void display_error_deferred(short error_code, short local_player_index, boolean modal, boolean pause_game_time);
void display_error_abort_to_dashboard(short error_code, boolean optional);
void display_error_damaged_media(void);
void display_error_abort_to_dashboard_deferred(short error_code, boolean optional);
void display_errors_deferred_until_cinematic_stop(void);
void display_scenario_help(short string_index);
void ui_start_main_menu_music(void);
void ui_stop_main_menu_music(void);
boolean ui_main_menu_music_active(void);
void ui_widgets_disable_pause_game(long duration_ticks);
short remap_sticks_for_local_player(short icon, short local_player_index);
void draw_string_and_hack_in_icons(rectangle2d *bounds, rectangle2d *clip, point2d *cursor_reference, short height_adjust, wchar_t const *instring, boolean ignore_icon_color);
real_rgb_color get_ui_rgb_white(void);
real_argb_color get_ui_argb_white(void);

/* ---------- globals */

extern struct stack_memory_pool *widget_memory_pool;
extern short dashboard_abort_error;

extern real_argb_color ui_plasma_effect_color;
extern short local_player_index_for_draw_string_and_hack_in_icons;

/* ---------- public code */

#endif // __UI_WIDGET_H
