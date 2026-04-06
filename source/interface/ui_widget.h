/*
UI_WIDGET.H

header included in hcex build.
*/

#ifndef __UI_WIDGET_H
#define __UI_WIDGET_H
#pragma once

/* ---------- headers */

#include "integer_math.h"
#include "real_math.h"

/* ---------- constants */

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
