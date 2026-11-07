/*
UI_WIDGET.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ui_widget.h"
#include "hud.h"
#include "player_ui.h"
#include "hud_messaging.h"
#include "virtual_keyboard.h"
#include "attract_mode.h"
#include "hud_messaging_definitions.h"
#include "progress_bar.h"
#include "ui_widget_event_handler_functions.h"
#include "ui_widget_game_data_input_functions.h"
#include "ui_widget_text_search_and_replace_functions.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "rasterizer.h"
#include "players.h"
#include "input.h"
#include "bitmaps_inlines.h"
#include "network_messages.h"
#include "sound_manager.h"
#include "text_group.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "game_sound.h"
#include "sound_definitions.h"
#include "texture_cache.h"
#include "network_client_manager.h"
#include "font_group.h"
#include "network_server_manager.h"
#include "cinematics.h"
#include "shell.h"
#include "hs.h"
#include "bink_playback.h"
#include "playlist_profile.h"
#include "bungie_net/common/thread.h"
#include "bungie_net/common/memory_manager.h"
#include "stack_memory_pool.h"

/* ---------- constants */

enum
{
	_fscheck_result_no_error = 0,
	_fscheck_result_not_enough_free_space,
	_fscheck_result_maximum_saved_games,
	NUMBER_OF_FILE_SYSTEM_CHECK_RESULTS
};

enum
{
	PLAYER1 = 0,
	PLAYER2,
	PLAYER3,
	PLAYER4
};

enum
{
	LIST_ARROW_ILLUMINATION_DURATION_TICKS = 15
};

enum
{
	_screen_multitexture_blend_func_add = 0,
	_screen_multitexture_blend_func_multiply,
	_screen_multitexture_blend_func_subtract,
	_screen_multitexture_blend_func_multiply2X,
	_screen_multitexture_blend_func_dot,
	_screen_multitexture_blend_func_phatty_plasma
};

enum
{
	WIDGET_MEMORY_POOL_BUFFER_SIZE = 0x4000,
	MAXIMUM_WIDGET_MEMORY_POOL_BLOCKS = 0x1000 /* fake name */
};

enum
{
	WIDGET_DELETED_PLAYER_CONTROL_INHIBIT_FLAGS = 0xFFF
};

enum
{
	CINEMATIC_START_GRACE_PERIOD_TICKS = 30
};

enum
{
	NUM_ICONS = 18
};

/* ---------- macros */

/* ---------- structures */

struct _widget_stack_node_data
{
	long previous_widget_tag;
	long focused_child_parent_widget_tag;
	short focused_child_index;
	short local_player_index;
};

struct widget_stack_node
{
	struct _widget_stack_node_data data;
	struct widget_stack_node *next;
};

struct _deferred_error
{
	short error_code;
	short local_player_index;
	boolean modal;
	boolean pause_game_time;
};

struct _deferred_dashboard_error
{
	short error_code;
	boolean allow_user_to_continue;
};

struct _deferred_cinematic_error
{
	short error_code;
	boolean modal;
	boolean pause_game_time;
};

struct _widget_globals
{
	struct widget_instance *active_widgets[MAXIMUM_GAMEPADS];
	struct widget_stack_node *widget_stack[MAXIMUM_GAMEPADS];
	unsigned long current_system_milliseconds;
	long pause_disabled_ticks;
	short main_menu_deferred_error_code;
	short pause_game_time_count;
	real fade_to_black;
	struct _deferred_error deferred_error[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	struct _deferred_dashboard_error deferred_dashboard_error;
	struct _deferred_cinematic_error deferred_cinematic_error[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	struct thread *initialization_thread;
	short filesystem_check_result;
	boolean initialized;
	boolean dont_load_children_recursive;
	boolean debug_show_path;
	boolean processing_inhibited;
	boolean main_menu_music_active;
	boolean sound_paused;
};

/* ---------- prototypes */

static void *pool_alloc(unsigned long size);
static void pool_free(void *pointer);
static real compute_offset_coordinate(long milliseconds, real speed);
static boolean transition_to_game_in_progress(void);
static void push_widget(struct widget_stack_node **top, struct _widget_stack_node_data const *data);
static void pop_widget(struct widget_stack_node **top, struct _widget_stack_node_data *data);
static void dispose_widget_stack(struct widget_stack_node **top);
static boolean ui_widget_load_children_recursive(struct widget_instance *widget, struct ui_widget_definition *definition);
static struct widget_instance *widget_instance_find_by_tag_index(long tag_index);
static void ui_widget_add_child(struct widget_instance *child, struct widget_instance *parent);
static void ui_widget_delete_children_recursive(struct widget_instance *widget);
static struct widget_instance *widget_instance_find_by_tag_index_recursive(struct widget_instance *widget, long tag_index);
static real widget_instance_get_cumulative_alpha_modifier(struct widget_instance const *widget);
static boolean widget_instance_can_receive_events(struct widget_instance const *widget);
static boolean widget_instance_can_handle_events(struct widget_instance const *widget);
static void widget_instance_initialize(struct widget_instance *widget, struct widget_instance *parent, long tag_index, short local_player_index, short widget_stack, struct ui_widget_definition *definition);
static void widget_instance_process_one_event_recursive(struct widget_instance *widget, struct ui_widget_definition *definition, struct event_record *event, boolean *return_widget_deleted);
static void event_handler_dispatch(struct widget_instance *widget, struct ui_widget_definition *definition, struct event_record *event, struct event_handler_reference *event_handler, boolean *return_widget_deleted);
static void widget_instance_render_recursive(struct widget_instance *widget, rectangle2d *clip_rect, point2d offset, boolean use_white_text, boolean plasma);
static void widget_instance_render_column_list(struct widget_instance *widget, struct ui_widget_definition *definition, rectangle2d *clip_rect, point2d offset, boolean use_white_text);
static boolean widget_instance_text_box_is_focused(struct widget_instance const *widget);
static short get_icon_type(wchar_t const *string);
static void render_state_bitmap(rectangle2d *bounds, rectangle2d *cursor_bounds, unsigned long color, struct icon_hud_element_definition const *icon);
static void render_state_text(rectangle2d *bounds, rectangle2d *cursor_bounds, wchar_t const *string);
static boolean string_has_icons_to_draw(wchar_t const *string);
static boolean should_flip_sticks_for_local_player(short local_player_index);
static void widget_instance_render_text_box(struct widget_instance *widget, struct ui_widget_definition *definition, rectangle2d *clip_rect, point2d offset, boolean use_white_text);
static void widget_instance_render_spinner_list(struct ui_widget_definition *definition, struct widget_instance *widget, rectangle2d *clip_rect, point2d offset, boolean use_white_text);
static void widget_instance_give_focus_by_tag(struct widget_instance *widget, long tag_index, short local_player_index);
static void widget_instance_go_back_to_previous(struct widget_instance *widget);
static void widget_instance_update_animation_parameters(struct widget_instance *widget);
static void widget_instance_reload_recursive(struct widget_instance *widget);
static void ui_widget_reload_by_tag(long tag_index);
static void widget_instance_give_focus_directly(struct widget_instance *widget, struct widget_instance *focus_widget);
static void widget_instance_set_focused_child_by_index(long tag_index, struct widget_instance *widget, short child_index);
static void play_sound_tag(long tag_index);
static long search_and_replace(wchar_t const *search_string, wchar_t const *replace_string, wchar_t **string);
static void spinner_list_update(struct widget_instance *widget);
static void column_list_update(struct widget_instance *widget, struct ui_widget_definition *definition);
static struct widget_instance *widget_instance_get_tail_child_widget(struct widget_instance const *widget);
static void widget_instance_tab_to_next_valid_widget(struct widget_instance *widget);
static void widget_instance_tab_to_previous_valid_widget(struct widget_instance *widget);
static boolean ui_check_for_pause_game(void);
static struct widget_instance *ui_widget_launch_widget(struct widget_instance *widget, long tag_index);
static void perform_filesystem_initialization(void);
static unsigned long __stdcall filesystem_initialization_thread_proc(void *parameter);

/* ---------- globals */

real const _one_over_255 = 1.f / 255.f;
real const SECONDS_PER_MILLISECOND = 0.001f;

static union
{
	struct stack_memory_pool pool;
	byte storage[sizeof(struct stack_memory_pool) + MAXIMUM_WIDGET_MEMORY_POOL_BLOCKS * sizeof(struct memory_block *)];
} widget_memory_pool_storage = /* fake name */
{
	{ "widget_memory_pool", NULL, 0, MAXIMUM_WIDGET_MEMORY_POOL_BLOCKS }
};

struct stack_memory_pool *widget_memory_pool = &widget_memory_pool_storage.pool;
static boolean main_screen_shell_first_load = TRUE; /* fake name */
short dashboard_abort_error = NONE;

static char const *solo_level_scenario_paths[NUMBER_OF_SINGLE_PLAYER_LEVELS] = /* fake name */
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

static boolean icon_is_special[NUM_ICONS] =
{
	FALSE,	// a-button
	FALSE,	// b-button
	FALSE,	// x-button
	FALSE,	// y-button
	FALSE,	// black-button
	FALSE,	// white-button
	TRUE,	// left-trigger
	TRUE,	// right-trigger
	FALSE,	// dpad-up
	FALSE,	// dpad-down
	FALSE,	// dpad-left
	FALSE,	// dpad-right
	FALSE,	// start-button
	FALSE,	// back-button
	TRUE,	// left-thumb
	TRUE,	// right-thumb
	TRUE,	// left-stick
	TRUE,	// right-stick
};

static wchar_t const *icon_names[NUMBER_OF_HUD_ICON_TYPES] =
{
	L"a-button",
	L"b-button",
	L"x-button",
	L"y-button",
	L"black-button",
	L"white-button",
	L"left-trigger",
	L"right-trigger",
	L"dpad-up",
	L"dpad-down",
	L"dpad-left",
	L"dpad-right",
	L"start-button",
	L"back-button",
	L"left-thumb",
	L"right-thumb",
	L"left-stick",
	L"right-stick",
	L"action",
	L"throw-grenade",
	L"primary-trigger",
	L"integrated-light",
	L"jump",
	L"use-equipment",
	L"rotate-weapons",
	L"rotate-grenades",
	L"crouch",
	L"zoom",
	L"accept",
	L"back",
	L"move",
	L"look",
	L"custom-1",
	L"custom-2",
	L"custom-3",
	L"custom-4",
	L"custom-5",
	L"custom-6",
	L"custom-7",
	L"custom-8",
};

static char remapped_icon_button_indices[_hud_icon_misc_start - _hud_icon_remapped_button_start] = /* fake name */
{
	_button_action_reload,		// action
	_button_throw_grenade,		// throw-grenade
	_button_fire,				// primary-trigger
	_button_flashlight,			// integrated-light
	_button_jump,				// jump
	_button_melee_attack,		// use-equipment
	_button_switch_weapon,		// rotate-weapons
	_button_switch_grenade,		// rotate-grenades
	_button_scope_zoom,			// crouch
	_button_crouch,				// zoom
	_gamepad_analog_button_a,	// accept
	_gamepad_analog_button_b,	// back
	_gamepad_stick_left_icon,	// move
	_gamepad_stick_right_icon,	// look
};

static real_rgb_color ui_rgb_white = { 0.8f, 0.8f, 0.8f }; /* fake name */

static unsigned long dpad_event_times[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS][_widget_event_button_dpad_right - _widget_event_button_dpad_up + 1]; /* fake name */
static boolean main_menu_active_state; /* fake name */
static struct _widget_globals widget_globals;

/* ---------- public code */

static void *pool_alloc(
	unsigned long size)
{
	return match_malloc("c:\\halo\\SOURCE\\interface\\ui_widget.c", 117, size);
}

static void pool_free(
	void *pointer)
{
	match_free("c:\\halo\\SOURCE\\interface\\ui_widget.c", 118, pointer);

	return;
}

boolean event_controller_index_compatible_with_widget(
	struct event_record const *event,
	struct widget_instance const *widget)
{
	return widget->local_player_index == NONE || widget->local_player_index == event->controller_index;
}

void set_ui_plasma_effect_color(
	real a,
	real r,
	real g,
	real b)
{
	ui_plasma_effect_color.alpha = a;
	ui_plasma_effect_color.red = r;
	ui_plasma_effect_color.green = g;
	ui_plasma_effect_color.blue = b;

	return;
}

void ui_widgets_initialize(
	void)
{
	long local_player_index;
	boolean success = TRUE;
	void *pool_memory = pool_alloc(WIDGET_MEMORY_POOL_BUFFER_SIZE);

	if (pool_memory)
	{
		widget_memory_pool->base_address = pool_memory;
		widget_memory_pool->pool_size = WIDGET_MEMORY_POOL_BUFFER_SIZE;
	}
	else
	{
		success = FALSE;
	}

	stack_memory_pool_reset(widget_memory_pool);

	memset(&widget_globals, 0, sizeof(widget_globals));
	widget_globals.main_menu_deferred_error_code = NONE;
	widget_globals.deferred_dashboard_error.error_code = NONE;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		widget_globals.deferred_error[local_player_index].error_code = NONE;
		widget_globals.deferred_cinematic_error[local_player_index].error_code = NONE;
	}

	widget_globals.initialized = success;
	widget_globals.fade_to_black = -1.f;

	return;
}

void ui_widgets_dispose(
	void)
{
	ui_widgets_close_all();

	if (widget_memory_pool->base_address)
	{
		pool_free(widget_memory_pool->base_address);
	}

	widget_memory_pool->base_address = NULL;
	widget_memory_pool->pool_size = 0;
	memset(&widget_globals, 0, sizeof(widget_globals));

	return;
}

void ui_widgets_safe_to_load(
	boolean safe)
{
	return;
}

void ui_widgets_set_fade_value(
	real value)
{
	widget_globals.fade_to_black = value;

	return;
}

void ui_widget_debug_show_path(
	boolean show)
{
	widget_globals.debug_show_path = show;

	return;
}

pixel32 modulate_pixel32_by_real_alpha(
	pixel32 argb,
	real alpha)
{
	return PIXEL32_RGB_BITS(argb) | PIXEL32_FROM_ALPHA(fast_ftol(PIXEL32_ALPHA(argb) * alpha));
}

struct widget_instance *ui_widget_load_by_name_or_tag(
	char const *name,
	long tag_index,
	struct widget_instance *parent,
	short local_player_index,
	long invoking_widget_tag,
	long focused_child_parent_widget_tag,
	short focused_child_index)
{
	struct widget_instance *widget = NULL;
	short widget_stack = local_player_index == NONE ? 0 : local_player_index;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 377, widget_globals.initialized);
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 378, (name != NULL) || (tag_index != NONE));
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 379, (widget_stack>=0) && (widget_stack<MAXIMUM_GAMEPADS));

	if (tag_index == NONE)
	{
		tag_index = tag_loaded(UI_WIDGET_DEFINITION_TAG, name);
	}

	if (tag_index != NONE)
	{
		struct ui_widget_definition *definition = ui_widget_definition_get(tag_index);

		widget = pool_new_pointer(widget_memory_pool, sizeof(*widget), "c:\\halo\\SOURCE\\interface\\ui_widget.c", 395);

		if (widget)
		{
			if (!parent)
			{
				short previous_local_player_index;

				if (widget_globals.active_widgets[widget_stack])
				{
					previous_local_player_index = widget_globals.active_widgets[widget_stack]->local_player_index;
					ui_widget_delete(widget_globals.active_widgets[widget_stack]);
				}
				else
				{
					previous_local_player_index = NONE;
				}

				widget_globals.active_widgets[widget_stack] = widget;

				if (invoking_widget_tag != NONE && !TEST_FLAG(ui_widget_definition_get(invoking_widget_tag)->flags, _widget_dont_push_history_data_bit))
				{
					struct _widget_stack_node_data history_data;

					history_data.previous_widget_tag = invoking_widget_tag;
					history_data.focused_child_parent_widget_tag = focused_child_parent_widget_tag;
					history_data.focused_child_index = focused_child_index;
					history_data.local_player_index = previous_local_player_index;
					push_widget(&widget_globals.widget_stack[widget_stack], &history_data);
				}
			}

			if (local_player_index == NONE)
			{
				switch (definition->controller_index)
				{
				case _widget_controller0:
					local_player_index = PLAYER1;
					break;
				case _widget_controller1:
					local_player_index = PLAYER2;
					break;
				case _widget_controller2:
					local_player_index = PLAYER3;
					break;
				case _widget_controller3:
					local_player_index = PLAYER4;
					break;
				case _widget_controller_any:
					local_player_index = NONE;
					break;
				}
			}

			widget_instance_initialize(widget, parent, tag_index, local_player_index, widget_stack, definition);
		}
		else
		{
			error(_error_silent, "failed to create new widget; out of memory!");
		}
	}
	else
	{
		error(_error_silent, "ui_widget_definition tag '%s'/%d not loaded", name, tag_index);
	}

	return widget;
}

void ui_widget_delete(
	struct widget_instance *widget)
{
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 469, widget && widget_globals.initialized);

	if (!widget->delete_recursion_lock)
	{
		struct ui_widget_definition *definition;
		long event_handler_index;
		long widget_stack;

		widget->delete_recursion_lock = TRUE;

		if (widget->local_player_index != NONE && !widget->parent)
		{
			player_control_inhibit_buttons(widget->local_player_index, WIDGET_DELETED_PLAYER_CONTROL_INHIBIT_FLAGS, TRUE);
		}

		definition = ui_widget_definition_get(widget->tag_index);

		for (event_handler_index = 0; event_handler_index < definition->event_handler_references.count; event_handler_index++)
		{
			struct event_handler_reference *event_handler = &((struct event_handler_reference *)definition->event_handler_references.address)[event_handler_index];

			if (event_handler->event_type == _widget_event_deleted && TEST_FLAG(event_handler->flags, _event_handler_run_function_bit))
			{
				boolean widget_deleted = FALSE;
				boolean success = ui_widget_event_handler_function_invoke(widget, NULL, event_handler->function, &widget_deleted);

				match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 520, !widget_deleted, "a 'widget deleted' event handler tried to delete the widget being deleted!");

				if (success == TRUE &&
					TEST_FLAG(event_handler->flags, _event_handler_open_widget_bit) &&
					event_handler->ui_widget_tag.index != NONE &&
					!ui_widget_launch_widget(widget, event_handler->ui_widget_tag.index))
				{
					error(_error_silent, "event handler failed to spawn widget");
				}
			}
		}

		if (widget->pause_game_time == TRUE)
		{
			match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 544, widget_globals.pause_game_time_count > 0, "widget pause counter out of whack");

			if (!--widget_globals.pause_game_time_count)
			{
				if (game_time_get_paused())
				{
					game_time_set_paused(FALSE);

					if (main_menu_active_state)
					{
						main_menu_ensure_player_queues_exist();
						game_time_dispose_from_old_map();
						game_time_initialize_for_new_map();
						game_time_start();
					}
				}

				if (widget_globals.sound_paused == TRUE)
				{
					sound_pause(FALSE);
					widget_globals.sound_paused = FALSE;
				}
			}
		}

		ui_widget_delete_children_recursive(widget);

		if (widget->previous)
		{
			widget->previous->next = widget->next;
		}

		if (widget->next)
		{
			widget->next->previous = widget->previous;
		}

		if (widget->parent && widget->parent->first_child == widget)
		{
			widget->parent->first_child = widget->next;
		}

		switch (widget->type)
		{
		case _ui_widget_type_text_box:
			if (widget->text_box.string)
			{
				dispose_pointer(widget_memory_pool, widget->text_box.string);
			}
			break;
		case _ui_widget_type_spinner_list:
		case _ui_widget_type_column_list:
			if (widget->list.items)
			{
				error(_error_silent, "###WARNING: possible memory leak disposing of a list widget (%s)", definition->name ? definition->name : "<unknown>");
			}

			if (widget->list.string)
			{
				dispose_pointer(widget_memory_pool, widget->list.string);
			}

			if (widget->list.extended_description_widget)
			{
				ui_widget_delete(widget->list.extended_description_widget);
			}
			break;
		}

		dispose_pointer(widget_memory_pool, widget);

		for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
		{
			if (widget_globals.active_widgets[widget_stack] == widget)
			{
				widget_globals.active_widgets[widget_stack] = NULL;
				break;
			}
		}
	}

	return;
}

void process_ui_widgets(
	void)
{
	boolean events_processed = FALSE;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 644, widget_globals.initialized);

	widget_globals.current_system_milliseconds = system_milliseconds();

	if (widget_globals.initialization_thread)
	{
		if (thread_has_exited(widget_globals.initialization_thread))
		{
			dispose_thread(widget_globals.initialization_thread);
			widget_globals.initialization_thread = NULL;
			ui_widgets_inhibit_processing(FALSE);

			switch (widget_globals.filesystem_check_result)
			{
			case _fscheck_result_not_enough_free_space:
				if (bink_playback_in_progress())
				{
					bink_playback_stop();
				}

				display_error_abort_to_dashboard(_error_hard_drive_not_enough_free_space, TRUE);
				break;
			case _fscheck_result_maximum_saved_games:
				if (bink_playback_in_progress())
				{
					bink_playback_stop();
				}

				display_error_abort_to_dashboard(_error_hard_drive_maximum_saved_game_files, TRUE);
				break;
			}
		}
	}
	else if (!progress_bar_is_active())
	{
		if (virtual_keyboard_active())
		{
			virtual_keyboard_process();
			event_manager_flush();
		}
		else if (attract_mode_should_start())
		{
			attract_mode_start();
		}
		else if (widget_globals.deferred_dashboard_error.error_code != NONE)
		{
			display_error_abort_to_dashboard(widget_globals.deferred_dashboard_error.error_code, widget_globals.deferred_dashboard_error.allow_user_to_continue);
			widget_globals.deferred_dashboard_error.error_code = NONE;
		}
		else if (widget_globals.deferred_error[0].error_code == NONE &&
			widget_globals.deferred_error[1].error_code == NONE &&
			widget_globals.deferred_error[2].error_code == NONE &&
			widget_globals.deferred_error[3].error_code == NONE)
		{
			boolean error_displayed[MAXIMUM_GAMEPADS];
			long widget_stack;
			boolean game_paused = ui_check_for_pause_game();
			boolean any_error_displayed = FALSE;

			for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
			{
				error_displayed[widget_stack] = widget_globals.active_widgets[widget_stack] && widget_globals.active_widgets[widget_stack]->widget_is_error_dialog == TRUE;
				any_error_displayed |= error_displayed[widget_stack];
			}

			for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
			{
				boolean process_widget;
				struct widget_instance *widget = widget_globals.active_widgets[widget_stack];

				if (error_displayed[widget_stack] == TRUE)
				{
					process_widget = widget && widget->widget_is_error_dialog == TRUE;
				}
				else if (main_menu_active_state)
				{
					process_widget = widget && !any_error_displayed;
				}
				else
				{
					process_widget = widget != NULL;
				}

				if (process_widget == TRUE)
				{
					boolean widget_deleted;
					struct ui_widget_definition *definition = ui_widget_definition_get(widget->tag_index);
					struct event_record event = { _event_type_none };

					if (!widget_globals.processing_inhibited && get_next_event(&event, widget->local_player_index))
					{
						do
						{
							if (!game_paused)
							{
								widget_instance_process_one_event_recursive(widget, definition, &event, &widget_deleted);

								if (widget_deleted == TRUE)
								{
									break;
								}
							}
						}
						while (widget == widget_globals.active_widgets[widget_stack] && get_next_event(&event, widget->local_player_index));
					}
					else if (!game_paused)
					{
						event.controller_index = widget->local_player_index;
						widget_instance_process_one_event_recursive(widget, definition, &event, &widget_deleted);
					}

					events_processed = TRUE;

					if (!widget_globals.active_widgets[widget_stack] && widget_globals.widget_stack[widget_stack])
					{
						struct _widget_stack_node_data data;

						pop_widget(&widget_globals.widget_stack[widget_stack], &data);

						if (data.previous_widget_tag != NONE)
						{
							struct widget_instance *previous_widget = ui_widget_load_by_name_or_tag(
								NULL,
								data.previous_widget_tag,
								NULL,
								data.local_player_index,
								NONE,
								NONE,
								NONE);

							if (previous_widget)
							{
								widget_instance_set_focused_child_by_index(data.focused_child_parent_widget_tag, previous_widget, data.focused_child_index);
							}
						}
					}
				}
			}

			if (events_processed)
			{
				event_manager_flush();
			}
		}
		else
		{
			long local_player_index;

			for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
			{
				struct _deferred_error *deferred_error = &widget_globals.deferred_error[local_player_index];

				if (deferred_error->error_code != NONE)
				{
					if (main_menu_active_state || network_game_is_active() || game_time_get() >= CINEMATIC_START_GRACE_PERIOD_TICKS)
					{
						display_error(deferred_error->error_code, deferred_error->local_player_index, deferred_error->modal, deferred_error->pause_game_time);
						deferred_error->error_code = NONE;
					}
					else
					{
						error(_error_silent, "waiting for %d ticks before displaying deferred errors", CINEMATIC_START_GRACE_PERIOD_TICKS);
					}
				}
			}
		}
	}

	return;
}

void render_ui_widgets_postgame(
	short local_player_index,
	rectangle2d const *window_bounds)
{
	point2d window_offsets[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS][MAXIMUM_NUMBER_OF_LOCAL_PLAYERS] =
	{
		{ { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
		{ { 0, 0 }, { 0, RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT / 2 }, { 0, 0 }, { 0, 0 } },
		{ { 0, 0 }, { 0, RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT / 2 }, { RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH / 2, RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT / 2 }, { 0, 0 } },
		{ { 0, 0 }, { RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH / 2, 0 }, { 0, RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT / 2 }, { RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH / 2, RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT / 2 } },
	};

	if (!virtual_keyboard_active())
	{
		long widget_stack;

		if (local_player_index < 0)
		{
			local_player_index = 0;
		}
		else
		{
			local_player_index = MIN(local_player_index, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS - 1);
		}

		for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
		{
			boolean render = FALSE;

			if (widget_globals.active_widgets[widget_stack])
			{
				if (widget_globals.active_widgets[widget_stack]->render_regardless_of_controller_index == TRUE)
				{
					render = TRUE;
				}
				else if (widget_globals.active_widgets[widget_stack]->widget_is_error_dialog == TRUE)
				{
					if (widget_globals.active_widgets[widget_stack]->local_player_index == local_player_index || widget_globals.active_widgets[widget_stack]->local_player_index == NONE || local_player_index == NONE || main_menu_active_state)
					{
						render = TRUE;
					}
				}
				else if (widget_globals.active_widgets[widget_stack]->local_player_index != NONE || widget_stack)
				{
					if (widget_globals.active_widgets[widget_stack]->local_player_index == local_player_index)
					{
						render = TRUE;
					}
				}
				else
				{
					render = TRUE;
				}
			}

			if (render == TRUE)
			{
				rectangle2d bounds;

				bounds.x0 = 0;
				bounds.y0 = 0;
				bounds.x1 = window_bounds->x1 - window_bounds->x0;
				bounds.y1 = window_bounds->y1 - window_bounds->y0;
				widget_instance_render_recursive(
					widget_globals.active_widgets[widget_stack],
					&bounds,
					window_offsets[local_player_count() - 1][local_player_index],
					TRUE,
					FALSE);
			}
		}
	}

	return;
}

void render_ui_widgets(
	short local_player_index,
	rectangle2d const *window_bounds)
{
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 911, window_bounds != NULL);

	local_player_index_for_draw_string_and_hack_in_icons = local_player_index == NONE ? 0 : local_player_index;

	if (!bink_playback_ui_rendering_inhibited())
	{
		if (!virtual_keyboard_active())
		{
			long widget_stack;

			if (local_player_index < 0)
			{
				local_player_index = 0;
			}
			else
			{
				local_player_index = MIN(local_player_index, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS - 1);
			}

			for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
			{
				boolean render = FALSE;

				if (widget_globals.active_widgets[widget_stack])
				{
					if (widget_globals.active_widgets[widget_stack]->render_regardless_of_controller_index == TRUE)
					{
						render = TRUE;
					}
					else if (widget_globals.active_widgets[widget_stack]->widget_is_error_dialog == TRUE)
					{
						if (widget_globals.active_widgets[widget_stack]->local_player_index == local_player_index || widget_globals.active_widgets[widget_stack]->local_player_index == NONE || local_player_index == NONE || main_menu_active_state)
						{
							render = TRUE;
						}
					}
					else if (widget_globals.active_widgets[widget_stack]->local_player_index != NONE || widget_stack)
					{
						if (widget_globals.active_widgets[widget_stack]->local_player_index == local_player_index)
						{
							render = TRUE;
						}
					}
					else
					{
						render = TRUE;
					}
				}

				if (render == TRUE)
				{
					rectangle2d bounds;
					point2d offset;

					bounds.x1 = window_bounds->x1 - window_bounds->x0;
					bounds.y1 = window_bounds->y1 - window_bounds->y0;
					offset.x = 0;
					offset.y = 0;
					bounds.x0 = 0;
					bounds.y0 = 0;
					widget_instance_render_recursive(widget_globals.active_widgets[widget_stack], &bounds, offset, TRUE, FALSE);

					if (widget_globals.debug_show_path)
					{
						real_argb_color color = { 1.f, 1.f, 1.f, 1.f };

						bounds.x0 += 32;
						bounds.x1 += 32;
						bounds.y0 += 32;
						bounds.y1 += 32;
						draw_string_set_draw_mode(tag_loaded(FONT_GROUP_TAG, "ui\\small_ui"), NONE, _text_justification_left, 0, &color);
						rasterizer_draw_string(&bounds, NULL, NULL, 0, tag_get_name(widget_globals.active_widgets[widget_stack]->tag_index));
					}
				}
			}

			if (widget_globals.fade_to_black >= 0.f && widget_globals.fade_to_black <= 1.f)
			{
				rectangle2d bounds;

				bounds.x0 = 0;
				bounds.x1 = RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH;
				bounds.y0 = 0;
				bounds.y1 = RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT;

				if (widget_globals.fade_to_black >= 0.95f)
				{
					widget_globals.fade_to_black = 1.f;
				}

				draw_quad(&bounds, PIXEL32_FROM_ALPHA(fast_ftol(widget_globals.fade_to_black * 255.f)));
			}
		}
		else
		{
			virtual_keyboard_render();
		}
	}

	return;
}

int widget_instance_count_children(
	struct widget_instance const *widget)
{
	int count = 0;

	if (widget)
	{
		struct widget_instance *child;

		for (child = widget->first_child; child; child = child->next)
		{
			count++;
		}
	}

	return count;
}

struct widget_instance *widget_instance_get_nth_child(
	struct widget_instance const *widget,
	int n)
{
	struct widget_instance *child;
	int child_index;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1050, widget);

	child = widget->first_child;

	for (child_index = 0; child_index < n && child; child_index++)
	{
		child = child->next;
	}

	return child;
}

void *ui_widget_realloc(
	void *pointer,
	word size,
	char const *file,
	long line)
{
	return pool_resize_pointer(widget_memory_pool, pointer, size, file, line);
}

void widget_free(
	void *pointer)
{
	dispose_pointer(widget_memory_pool, pointer);

	return;
}

boolean ui_widgets_active(
	void)
{
	boolean active = FALSE;

	if (widget_globals.initialized)
	{
		long widget_stack;

		for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
		{
			if (widget_globals.active_widgets[widget_stack])
			{
				active = TRUE;
				break;
			}
		}
	}

	return active;
}

boolean ui_widgets_active_for_local_player(
	short local_player_index)
{
	boolean active = FALSE;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1110, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS), "expected a valid local_player_index");

	if (widget_globals.initialized)
	{
		long widget_stack;

		for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
		{
			struct widget_instance *widget = widget_globals.active_widgets[widget_stack];

			if (widget && widget->local_player_index == local_player_index)
			{
				active = TRUE;
				break;
			}
		}
	}

	return active;
}

void ui_widgets_close_all(
	void)
{
	long widget_stack;

	for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
	{
		if (widget_globals.active_widgets[widget_stack])
		{
			ui_widget_delete(widget_globals.active_widgets[widget_stack]);
		}

		if (widget_globals.widget_stack[widget_stack])
		{
			dispose_widget_stack(&widget_globals.widget_stack[widget_stack]);
		}
	}

	return;
}

void ui_widgets_close_all_for_local_player(
	short local_player_index)
{
	long widget_stack;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1154, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS), "expected a valid local_player_index");

	for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
	{
		if (widget_globals.active_widgets[widget_stack] && widget_globals.active_widgets[widget_stack]->local_player_index == local_player_index)
		{
			ui_widget_delete(widget_globals.active_widgets[widget_stack]);

			if (widget_globals.widget_stack[widget_stack])
			{
				dispose_widget_stack(&widget_globals.widget_stack[widget_stack]);
			}
		}
	}

	return;
}

void ui_widgets_inhibit_processing(
	boolean inhibit)
{
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1174, widget_globals.initialized);

	widget_globals.processing_inhibited = inhibit;

	return;
}

void ui_widgets_delete_history(
	void)
{
	long widget_stack;

	for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
	{
		if (widget_globals.widget_stack[widget_stack])
		{
			dispose_widget_stack(&widget_globals.widget_stack[widget_stack]);
		}
	}

	return;
}

void ui_widgets_pop_stack(
	short local_player_index)
{
	if (local_player_index == NONE)
	{
		local_player_index = 0;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1204, (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));
	}

	if (widget_globals.widget_stack[local_player_index])
	{
		struct _widget_stack_node_data data;

		pop_widget(&widget_globals.widget_stack[local_player_index], &data);
	}

	return;
}

boolean widget_event_function_list_widget_goto_next_item(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct ui_widget_definition *definition;
	struct widget_instance *child;
	long item_index;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1230, widget);

	definition = ui_widget_definition_get(widget->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1233, widget->type == _ui_widget_type_spinner_list || widget->type == _ui_widget_type_column_list, "calling a list widget function on a non-list widget");

	if (widget->list.items && widget->list.item_count > 0)
	{
		item_index = widget->list.item_index + 1;

		if (item_index >= widget->list.item_count)
		{
			item_index = 0;
		}

		if (widget->type == _ui_widget_type_column_list)
		{
			child = widget_instance_get_nth_child(widget, item_index);

			if (!child)
			{
				error(_error_silent, "failed to set focus to the #%d list item of a column list widget", item_index);
				return FALSE;
			}

			widget_instance_give_focus_by_tag(widget, child->tag_index, widget->local_player_index);
			widget->list.item_index = (short)item_index;
			widget->list.last_list_tab_direction = LIST_ARROW_ILLUMINATION_DURATION_TICKS;

			return TRUE;
		}
		else if (widget->type == _ui_widget_type_spinner_list)
		{
			if (definition->child_widgets.count > 1)
			{
				match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1268, definition->child_widgets.count == 3, "spinner lists must be either 1- or 3-wide... sorry");

				if (widget->focused_child == widget->first_child || widget->focused_child == widget->first_child->next)
				{
					if (widget->focused_child->next)
					{
						widget_instance_give_focus_directly(widget, widget->focused_child->next);
					}
				}
			}

			widget->list.item_index = (short)item_index;
		}
	}
	else
	{
		if (widget->type == _ui_widget_type_spinner_list)
		{
			match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1288, definition->child_widgets.count <= 1, "spinner lists with more that 1 visible item need to have code-generated lists associated with them... sorry.");
		}

		if (widget->type == _ui_widget_type_spinner_list &&
			TEST_FLAG(definition->list_flags, _list_items_generated_from_string_list_tag) &&
			!definition->child_widgets.count)
		{
			if (++widget->list.item_index != widget->list.item_count)
			{
				widget->list.last_list_tab_direction = LIST_ARROW_ILLUMINATION_DURATION_TICKS;

				return TRUE;
			}

			widget->list.item_index = 0;
		}
		else
		{
			child = NULL;

			if (widget->focused_child)
			{
				child = widget->focused_child->next;
				item_index = widget->list.item_index + 1;

				if (item_index == widget->list.item_count)
				{
					child = NULL;
				}
			}

			if (!child)
			{
				child = widget->first_child;
				item_index = 0;
			}

			if (child)
			{
				widget_instance_give_focus_by_tag(widget, child->tag_index, widget->local_player_index);
				widget->list.item_index = (short)item_index;
			}
			else
			{
				error(_error_silent, "failed to set focus to the next list item of a column widget");

				return FALSE;
			}
		}

		widget->list.last_list_tab_direction = LIST_ARROW_ILLUMINATION_DURATION_TICKS;

		return TRUE;
	}

	widget->list.last_list_tab_direction = LIST_ARROW_ILLUMINATION_DURATION_TICKS;

	return TRUE;
}

boolean widget_event_function_list_widget_goto_previous_item(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct ui_widget_definition *definition;
	struct widget_instance *child;
	long item_index;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1359, widget);

	definition = ui_widget_definition_get(widget->tag_index);
	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1362, widget->type == _ui_widget_type_spinner_list || widget->type == _ui_widget_type_column_list, "calling a list widget function on a non-list widget");

	if (widget->list.items && widget->list.item_count > 0)
	{
		item_index = widget->list.item_index - 1;

		if (item_index < 0)
		{
			item_index = widget->list.item_count - 1;
		}

		if (widget->type == _ui_widget_type_column_list)
		{
			child = widget_instance_get_nth_child(widget, item_index);

			if (child)
			{
				widget_instance_give_focus_by_tag(widget, child->tag_index, widget->local_player_index);
			}
			else
			{
				error(_error_silent, "failed to set focus to the #%d list item of a column list widget", item_index);
				return FALSE;
			}

			widget->list.item_index = (short)item_index;
		}
		else if (widget->type == _ui_widget_type_spinner_list)
		{
			if (definition->child_widgets.count > 1)
			{
				match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1396, definition->child_widgets.count == 3, "spinner lists must be either 1- or 3-wide... sorry");

				if (widget->focused_child != widget->first_child)
				{
					match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1405, widget->focused_child);

					if (widget->focused_child->previous)
					{
						widget_instance_give_focus_directly(widget, widget->focused_child->previous);
					}
				}
			}

			widget->list.item_index = (short)item_index;
		}

		widget->list.last_list_tab_direction = -LIST_ARROW_ILLUMINATION_DURATION_TICKS;

		return TRUE;
	}
	else
	{
		if (widget->type == _ui_widget_type_spinner_list)
		{
			match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1420, definition->child_widgets.count <= 1, "spinner lists with more that 1 visible item need to have code-generated lists associated with them... sorry.");
		}

		if (widget->type == _ui_widget_type_spinner_list &&
			TEST_FLAG(definition->list_flags, _list_items_generated_from_string_list_tag) &&
			!definition->child_widgets.count)
		{
			if (--widget->list.item_index < 0)
			{
				widget->list.item_index = widget->list.item_count - 1;
			}

			widget->list.last_list_tab_direction = -LIST_ARROW_ILLUMINATION_DURATION_TICKS;

			return TRUE;
		}

		child = NULL;

		if (widget->focused_child)
		{
			item_index = widget->list.item_index - 1;
			child = widget->focused_child->previous;
		}

		if (!child)
		{
			child = widget->first_child;
			item_index = 0;

			while (child->next)
			{
				child = child->next;
				item_index++;
			}
		}
	}

	widget_instance_give_focus_by_tag(widget, child->tag_index, widget->local_player_index);
	widget->list.item_index = (short)item_index;
	widget->list.last_list_tab_direction = -LIST_ARROW_ILLUMINATION_DURATION_TICKS;

	return TRUE;
}

void main_screen_shell_begin_fade(
	unsigned long fade_duration_milliseconds)
{
	long widget_stack;

	ui_stop_main_menu_music();

	for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
	{
		if (widget_globals.active_widgets[widget_stack] && !widget_globals.active_widgets[widget_stack]->widget_is_error_dialog)
		{
			widget_globals.active_widgets[widget_stack]->auto_close_fade_time = fade_duration_milliseconds;
			widget_globals.active_widgets[widget_stack]->milliseconds_to_auto_close = widget_globals.current_system_milliseconds - widget_globals.active_widgets[widget_stack]->creation_time + 100;

			if (widget_globals.widget_stack[widget_stack])
			{
				dispose_widget_stack(&widget_globals.widget_stack[widget_stack]);
			}
		}
	}

	return;
}

void ui_play_audio_feedback_sound(
	short audio_feedback)
{
	switch (audio_feedback)
	{
	case _audio_feedback_cursor:
		play_sound_tag(tag_loaded(SOUND_DEFINITION_TAG, "sound\\sfx\\ui\\cursor"));
		break;
	case _audio_feedback_forward:
		play_sound_tag(tag_loaded(SOUND_DEFINITION_TAG, "sound\\sfx\\ui\\forward"));
		break;
	case _audio_feedback_backward:
		play_sound_tag(tag_loaded(SOUND_DEFINITION_TAG, "sound\\sfx\\ui\\back"));
		break;
	case _audio_feedback_failure:
		play_sound_tag(tag_loaded(SOUND_DEFINITION_TAG, "sound\\sfx\\ui\\flag_failure"));
		break;
	}

	return;
}

static real compute_offset_coordinate(
	long milliseconds,
	real speed)
{
	real offset = milliseconds * (speed / MILLISECONDS_PER_SECOND);

	return (real)fmod(offset, 1.0);
}

void draw_bitmap_in_rect(
	struct bitmap_data *bitmap,
	rectangle2d const *rect,
	rectangle2d const *bitmap_rect,
	rectangle2d const *clip_rect,
	pixel32 argb,
	struct rasterizer_dynamic_screen_geometry_parameters *multitexture_params,
	boolean no_plasma)
{
	if (bitmap && rect)
	{
		struct dynamic_screen_vertex vertices[NUMBER_OF_POINTS_PER_RECTANGLE];
		struct rasterizer_dynamic_screen_geometry_parameters parameters;
		real_point2d points[NUMBER_OF_POINTS_PER_RECTANGLE];
		real_point2d plasma_map1_offset;
		real_point2d plasma_map0_offset;
		rectangle2d default_bitmap_rect;
		short width;
		short height;
		short bitmap_width;
		short bitmap_height;
		real u;
		real v;
		short vertex_index;
		real_rgb_color plasma_tint = { 0.9f, 0.9f, 0.9f };
		real_argb_color plasma_fade = ui_plasma_effect_color;
		real plasma_map_fade = 0.9f;

		if (!bitmap_rect)
		{
			default_bitmap_rect.x1 = bitmap->width;
			default_bitmap_rect.x0 = 0;
			default_bitmap_rect.y0 = 0;
			default_bitmap_rect.y1 = bitmap->height;
			bitmap_rect = &default_bitmap_rect;
		}

		width = rect->x1 - rect->x0;
		height = rect->y1 - rect->y0;
		bitmap_width = bitmap_rect->x1 - bitmap_rect->x0;
		bitmap_height = bitmap_rect->y1 - bitmap_rect->y0;

		points[0].x = rect->x0;
		points[0].y = rect->y0;
		points[1].x = rect->x0 + width;
		points[1].y = rect->y0;
		points[2].x = rect->x0 + width;
		points[2].y = rect->y0 + height;
		points[3].x = rect->x0;
		points[3].y = rect->y0 + height;

		if (clip_rect)
		{
			if (clip_rect->x0 > rect->x0)
			{
				points[3].x = clip_rect->x0;
				points[0].x = clip_rect->x0;
			}

			if (clip_rect->x1 < rect->x1)
			{
				points[2].x = clip_rect->x1;
				points[1].x = clip_rect->x1;
			}

			if (clip_rect->y0 > rect->y0)
			{
				points[1].y = clip_rect->y0;
				points[0].y = clip_rect->y0;
			}

			if (clip_rect->y1 < rect->y1)
			{
				points[3].y = clip_rect->y1;
				points[2].y = clip_rect->y1;
			}
		}

		u = bitmap->width;

		if (1.f > u)
		{
			u = 1.f;
		}

		u = bitmap_width / u;

		if (u > 1.f)
		{
			u = 1.f;
		}

		v = bitmap->height;

		if (1.f > v)
		{
			v = 1.f;
		}

		v = bitmap_height / v;

		if (v > 1.f)
		{
			v = 1.f;
		}

		for (vertex_index = 0; vertex_index < NUMBER_OF_POINTS_PER_RECTANGLE; vertex_index++)
		{
			vertices[vertex_index].color = argb;
			vertices[vertex_index].texcoord.x = (vertex_index % 3) ? u : 0.f;
			vertices[vertex_index].texcoord.y = (vertex_index > 1) ? v : 0.f;
			vertices[vertex_index].position = points[vertex_index];
		}

		memset(&parameters, 0, sizeof(parameters));

		if (no_plasma)
		{
			parameters.map_texture_scale[0].j = 1.f;
			parameters.map_texture_scale[0].i = 1.f;
			parameters.map_scale[0].j = 1.f;
			parameters.map_scale[0].i = 1.f;
			parameters.map[0] = bitmap;
		}
		else
		{
			struct bitmap_data *plasma_bitmap = TAG_BLOCK_GET_ELEMENT(&bitmap_group_get(interface_get_tag_index(_interface_bitmap_iface_map3))->bitmaps, 0, struct bitmap_data);
			long milliseconds = system_milliseconds();

			plasma_map0_offset.x = (compute_offset_coordinate(milliseconds, 1000.f / (100.f * 311.f))) * 311.f;
			plasma_map0_offset.y = (compute_offset_coordinate(milliseconds, 1000.f / (120.f * 311.f))) * 311.f;
			plasma_map1_offset.x = (-compute_offset_coordinate(milliseconds, 1000.f / (140.f * 201.f))) * 201.f;
			plasma_map1_offset.y = (-compute_offset_coordinate(milliseconds, 1000.f / (160.f * 201.f))) * 201.f;

			parameters.map_tint[0] = &plasma_tint;
			parameters.map_fade[0] = &plasma_map_fade;
			parameters.map_offset[0] = &plasma_map0_offset;
			parameters.map[0] = plasma_bitmap;
			parameters.map0_to_1_blend_function = _screen_multitexture_blend_func_phatty_plasma;
			parameters.map_scale[0].i = 1.f;
			parameters.map_scale[0].j = 1.f;
			parameters.map_wrapped[0] = TRUE;
			parameters.map_anchor_screen[0] = TRUE;
			parameters.map_texture_scale[0].i = 1.f / 311.f;
			parameters.map_texture_scale[0].j = 1.f / 311.f;

			parameters.map_tint[1] = &plasma_tint;
			parameters.map_fade[1] = &plasma_map_fade;
			parameters.map[1] = plasma_bitmap;
			parameters.map_offset[1] = &plasma_map1_offset;
			parameters.map1_to_2_blend_function = _screen_multitexture_blend_func_add;
			parameters.map_scale[1].i = 1.f;
			parameters.map_scale[1].j = 1.f;
			parameters.map_wrapped[1] = TRUE;
			parameters.map_anchor_screen[1] = TRUE;
			parameters.map_texture_scale[1].i = 1.f / 201.f;
			parameters.map_texture_scale[1].j = 1.f / 201.f;

			parameters.plasma_fade = plasma_fade;
			parameters.map_fade[2] = NULL;
			parameters.map_texture_scale[2].j = 1.f;
			parameters.map_texture_scale[2].i = 1.f;
			parameters.map_scale[2].j = 1.f;
			parameters.map_scale[2].i = 1.f;
			parameters.map[2] = bitmap;
			parameters.doing_plasma_effect = TRUE;
		}

		parameters.meter_parameters = NULL;
		parameters.point_sampled = FALSE;
		parameters.framebuffer_blend_function = _shader_framebuffer_blend_function_alpha_blend;
		rasterizer_psuedo_dynamic_screen_quad_draw(&parameters, vertices);
	}

	return;
}

struct widget_instance *widget_instance_get_topmost_parent(
	struct widget_instance *widget)
{
	while (widget->parent)
	{
		widget = widget->parent;
	}

	return widget;
}

int widget_instance_get_child_index_from_parent(
	struct widget_instance const *widget)
{
	int child_index = NONE;

	if (widget->parent)
	{
		struct widget_instance *child = widget->parent->first_child;
		int index = 0;

		while (child)
		{
			if (child == widget)
			{
				child_index = index;
				break;
			}

			child = child->next;
			index++;
		}
	}

	return child_index;
}

void widget_instance_set_visibility_recursive(
	struct widget_instance *widget,
	boolean visible)
{
	struct widget_instance *child;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 1859, widget);

	widget->visible = visible;

	for (child = widget->first_child; child; child = child->next)
	{
		widget_instance_set_visibility_recursive(child, visible);
	}

	return;
}

void main_menu_active(
	boolean active)
{
	main_menu_active_state = active;

	return;
}

boolean main_menu_is_active(
	void)
{
	return main_menu_active_state;
}

boolean main_menu_screen_is_active(
	void)
{
	if (main_menu_active_state == TRUE && widget_globals.active_widgets[0] && !strcmp(widget_globals.active_widgets[0]->name, "the_main_menu"))
	{
		return TRUE;
	}

	return FALSE;
}

void ui_set_next_level(
	short level)
{
	switch (level)
	{
	case NONE:
		main_roll_credits();
		break;
	case _single_player_map_a10:
	case _single_player_map_a30:
	case _single_player_map_a50:
	case _single_player_map_b30:
	case _single_player_map_b40:
	case _single_player_map_c10:
	case _single_player_map_c20:
	case _single_player_map_c40:
	case _single_player_map_d20:
	case _single_player_map_d40:
		main_set_map_name(main_get_solo_level_name(level));
		main_disallow_persistent_storage();
		break;
	default:
		error(_error_silent, "unknown level");
		main_goto_main_menu();
		break;
	}

	return;
}

void ui_widget_load_progress_widget(
	void)
{
	error(_error_silent, "the old loading progress screen has been replaced with glowy halo gravy");

	return;
}

boolean filesystem_check_thread_is_active(
	void)
{
	return widget_globals.initialization_thread != NULL;
}

void main_screen_shell_load(
	void)
{
	boolean load_main_menu = TRUE;

	ui_widgets_inhibit_processing(FALSE);

	if (main_screen_shell_first_load == TRUE)
	{
		char const *command_line = shell_get_command_line();

		if (command_line && !_stricmp(command_line, "xdemo"))
		{
			error(_error_silent, "xbox command line= '%s'", command_line);
		}
		else
		{
			unsigned long flags = 0;

			SET_FLAG(flags, _bink_playback_button_click_stops_movie_bit, TRUE);
			SET_FLAG(flags, _bink_playback_prevent_events_to_ui_bit, TRUE);
			SET_FLAG(flags, _bink_playback_return_to_main_menu_when_finished_bit, TRUE);
			SET_FLAG(flags, _bink_playback_dont_allow_skipping_if_filesystem_check_thread_is_active_bit, TRUE);
			SET_FLAG(flags, _bink_playback_eat_up_memory_like_a_goddamn_beaver_bit, TRUE);
			bink_playback_start(attract_mode_get_localized_movie_path(_bink_intro_movie), flags);
			load_main_menu = FALSE;

			if (!bink_playback_active())
			{
				load_main_menu = TRUE;
			}
		}

		perform_filesystem_initialization();
		input_abstraction_reset_controller_detection_timer();
	}

	if (load_main_menu)
	{
		attract_mode_reset_timer();
		ui_widgets_close_all();

		if (!ui_widget_load_by_name_or_tag("ui\\shell\\main_menu\\main_menu", NONE, NULL, NONE, NONE, NONE, NONE))
		{
			error(_error_silent, "failed to load main screen shell window");
		}

		if (widget_globals.main_menu_deferred_error_code != NONE)
		{
			display_error(widget_globals.main_menu_deferred_error_code, NONE, TRUE, FALSE);
			widget_globals.main_menu_deferred_error_code = NONE;
		}

		if (!widget_globals.main_menu_music_active)
		{
			ui_start_main_menu_music();
		}

		reset_last_player1_profile_index();
	}

	if (!virtual_keyboard_initialize())
	{
		error(_error_silent, "failed to initialize the virtual keyboard");
	}

	main_screen_shell_first_load = FALSE;

	return;
}

void network_game_reset_to_pregame_ui(
	void)
{
	ui_widgets_close_all();

	if (network_game_is_splitscreen_local())
	{
		if (network_game_is_quickstart_local())
		{
			if (!ui_widget_load_by_name_or_tag("ui\\shell\\main_menu\\multiplayer_type_select\\split_screen\\pregame\\splitscreen_pregame_wrapper_normal", NONE, NULL, NONE, NONE, NONE, NONE))
			{
				error(_error_silent, "failed to load pregame screen after quickstart match");
			}
		}
		else if (!ui_widget_load_by_name_or_tag("ui\\shell\\main_menu\\multiplayer_type_select\\split_screen\\splitscreen_map_select_postgame_wrapper", NONE, NULL, NONE, NONE, NONE, NONE))
		{
			error(_error_silent, "failed to load map select postgame screen");
		}
	}
	else if (global_network_game_server_get())
	{
		network_game_server_pause_countdown(global_network_game_server_get(), TRUE);

		if (!ui_widget_load_by_name_or_tag("ui\\shell\\main_menu\\multiplayer_type_select\\connected\\connected_map_select_postgame_wrapper", NONE, NULL, NONE, NONE, NONE, NONE))
		{
			error(_error_silent, "failed to load map select postgame screen");
		}
	}
	else if (!ui_widget_load_by_name_or_tag("ui\\shell\\main_menu\\multiplayer_type_select\\connected\\pregame\\connected_pregame_screen", NONE, NULL, NONE, NONE, NONE, NONE))
	{
		error(_error_silent, "failed to load networked pregame status screen");
	}

	return;
}

static boolean transition_to_game_in_progress(
	void)
{
	if (main_menu_active_state && widget_globals.fade_to_black <= 1.f && widget_globals.fade_to_black >= 0.f)
	{
		return TRUE;
	}

	return FALSE;
}

void display_error(
	short error_code,
	short local_player_index,
	boolean modal,
	boolean pause_game_time)
{
	if (cinematic_in_progress())
	{
		if (local_player_index == NONE)
		{
			local_player_index = 0;
		}
		else
		{
			match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2077, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
		}

		if (widget_globals.deferred_cinematic_error[local_player_index].error_code == NONE)
		{
			widget_globals.deferred_cinematic_error[local_player_index].error_code = error_code;
			widget_globals.deferred_cinematic_error[local_player_index].modal = modal;
			widget_globals.deferred_cinematic_error[local_player_index].pause_game_time = pause_game_time;
		}
		else
		{
			error(_error_silent, "there is already a deferred-for-cinematic error queued for player #%d; ignoring this one", local_player_index);
		}
	}
	else
	{
		short local_player_count;
		short found_local_player_index;
		boolean first_local_player = TRUE;
		char const *widget_name = NULL;

		if (local_player_index == NONE)
		{
			found_local_player_index = NONE;
			local_player_count = 0;
		}
		else
		{
			short player_index;

			local_player_count = 0;
			found_local_player_index = NONE;

			for (player_index = local_player_get_next(NONE); player_index != NONE; player_index = local_player_get_next(player_index))
			{
				if (player_index == local_player_index)
				{
					found_local_player_index = local_player_index;

					if (local_player_count > 0)
					{
						first_local_player = FALSE;
					}
				}

				local_player_count++;
			}
		}

		if (found_local_player_index == NONE && !main_menu_active_state)
		{
			local_player_index = NONE;
		}

		switch (local_player_count)
		{
		case 0:
		case 1:
			widget_name = modal ? "ui\\shell\\error\\error_modal_fullscreen" : "ui\\shell\\error\\error_nonmodal_fullscreen";
			break;
		case 2:
			widget_name = modal ? "ui\\shell\\error\\error_modal_halfscreen" : "ui\\shell\\error\\error_nonmodal_halfscreen";
			break;
		case 3:
			if (first_local_player == TRUE)
			{
				widget_name = modal ? "ui\\shell\\error\\error_modal_halfscreen" : "ui\\shell\\error\\error_nonmodal_halfscreen";
			}
			else
			{
				widget_name = modal ? "ui\\shell\\error\\error_modal_qtrscreen" : "ui\\shell\\error\\error_nonmodal_qtrscreen";
			}
			break;
		case 4:
			widget_name = modal ? "ui\\shell\\error\\error_modal_qtrscreen" : "ui\\shell\\error\\error_nonmodal_qtrscreen";
			break;
		default:
			match_vhalt("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2161, "invalid local player count");
		}

		if (widget_name)
		{
			struct widget_instance *invoking_widget;
			long invoking_widget_tag;
			struct widget_instance *error_widget;
			short widget_stack = local_player_index == NONE ? 0 : local_player_index;

			match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2168, (widget_stack>=0) && (widget_stack<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

			if (transition_to_game_in_progress())
			{
				error(_error_silent, "aborting to the main menu root, for safety's sake");
				main_screen_shell_load();
				main_defer_map_map_change();
				widget_globals.fade_to_black = -1.f;
			}

			invoking_widget = widget_globals.active_widgets[widget_stack];
			invoking_widget_tag = invoking_widget ? invoking_widget->tag_index : NONE;

			if (invoking_widget && invoking_widget->widget_is_error_dialog == TRUE)
			{
				error(_error_silent, "there is already an error message displayed for this local player index");
				error_widget = NULL;
			}
			else
			{
				error_widget = ui_widget_load_by_name_or_tag(widget_name, NONE, NULL, local_player_index, invoking_widget_tag, NONE, NONE);
			}

			if (error_widget)
			{
				struct widget_instance *text_box;

				match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2206, error_widget->first_child && error_widget->first_child->first_child, "error screen widget tag not layed out as expected");
				text_box = error_widget->first_child->first_child;
				match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2208, text_box->type == _ui_widget_type_text_box, "expected a text box widget in the error widget");
				text_box->text_box.string_list_index = error_code < 0 ? 0 : MIN(error_code, NUMBER_OF_ERROR_CODES - 1);
				error_widget->widget_is_error_dialog = TRUE;

				if (!error_widget->pause_game_time)
				{
					error_widget->pause_game_time = pause_game_time;

					if (error_widget->pause_game_time == TRUE)
					{
						match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2217, widget_globals.pause_game_time_count >= 0, "widget pause counter is out of whack");
						widget_globals.pause_game_time_count++;

						if (!game_time_get_paused())
						{
							game_time_set_paused(TRUE);
						}

						if (!widget_globals.sound_paused && !main_menu_active_state)
						{
							sound_pause(TRUE);
							widget_globals.sound_paused = TRUE;
						}
					}
				}

				switch (error_code)
				{
				case _error_controller_unplugged_start_to_continue:
					error_widget->milliseconds_to_auto_close = 0;
					error_widget->auto_close_fade_time = 0;
					break;
				case _error_controller_unplugged:
					error_widget->close_if_local_player_controller_present = TRUE;
					error_widget->milliseconds_to_auto_close = 0;
					error_widget->auto_close_fade_time = 0;
					break;
				default:
					error_widget->close_if_local_player_controller_present = FALSE;
					break;
				}
			}
			else
			{
				error(_error_silent, "failed to display error message");
			}
		}
	}

	return;
}

void display_error_when_main_menu_loaded(
	short error_code)
{
	if (widget_globals.main_menu_deferred_error_code == NONE)
	{
		widget_globals.main_menu_deferred_error_code = error_code;
	}
	else
	{
		error(_error_silent, "there is already an error message queued for display at the main menu; ignoring this one");
	}

	return;
}

void display_error_deferred(
	short error_code,
	short local_player_index,
	boolean modal,
	boolean pause_game_time)
{
	long index;

	if (local_player_index == NONE)
	{
		index = 0;
	}
	else
	{
		index = local_player_index;
		match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2288, (index>=0) && (index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));
	}

	if (widget_globals.deferred_error[index].error_code == NONE)
	{
		widget_globals.deferred_error[index].error_code = error_code;
		widget_globals.deferred_error[index].local_player_index = local_player_index;
		widget_globals.deferred_error[index].modal = modal;
		widget_globals.deferred_error[index].pause_game_time = pause_game_time;
	}
	else
	{
		error(_error_silent, "there is already a deferred error message for local player %d; ignoring this one", index);
	}

	return;
}

void display_error_abort_to_dashboard(
	short error_code,
	boolean optional)
{
	struct widget_instance *widget;
	char const *widget_name = optional == TRUE ? "ui\\shell\\error\\error_abort_to_dashboard" : "ui\\shell\\error\\error_abort_to_dashboard_you_have_no_choice";

	if (!optional)
	{
		ui_widgets_close_all();
	}

	widget = ui_widget_load_by_name_or_tag(widget_name, NONE, NULL, NONE, NONE, NONE, NONE);

	if (widget)
	{
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2319, widget->type == _ui_widget_type_text_box, "expected a text box widget");
		widget->text_box.string_list_index = error_code;
		widget->widget_is_error_dialog = TRUE;
		dashboard_abort_error = error_code;
	}
	else
	{
		error(_error_silent, "failed to load '%s' widget", widget_name);
	}

	return;
}

void display_error_damaged_media(
	void)
{
	display_error_abort_to_dashboard(_error_media_damaged, FALSE);
	input_frame_end();
	main_loop_of_death();

	return;
}

void display_error_abort_to_dashboard_deferred(
	short error_code,
	boolean optional)
{
	if (widget_globals.deferred_dashboard_error.error_code == NONE)
	{
		widget_globals.deferred_dashboard_error.error_code = error_code;
		widget_globals.deferred_dashboard_error.allow_user_to_continue = optional;
	}
	else
	{
		error(_error_silent, "there is already a deferred dashbaord error queued; ignoring this one!");
	}

	return;
}

void display_errors_deferred_until_cinematic_stop(
	void)
{
	short local_player_index;

	match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2367, !cinematic_in_progress(), "Noooooooooooooooooo!!!");

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		struct _deferred_cinematic_error *deferred_error = &widget_globals.deferred_cinematic_error[local_player_index];

		if (deferred_error->error_code >= 0 && deferred_error->error_code < NUMBER_OF_ERROR_CODES)
		{
			display_error(deferred_error->error_code, local_player_index, deferred_error->modal, deferred_error->pause_game_time);
		}

		deferred_error->error_code = NONE;
	}

	return;
}

void display_scenario_help(
	short string_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2407, string_index>=0);

	if (global_scenario_index != NONE)
	{
		char scenario_name[256];
		char const *widget_name;
		short local_player_index;
		struct widget_instance *widget;

		strncpy(scenario_name, tag_get_name(global_scenario_index), sizeof(scenario_name) - 1);
		scenario_name[sizeof(scenario_name) - 1] = '\0';
		strlwr(scenario_name);

		if (strstr(scenario_name, "a10"))
		{
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_a10";
		}
		else if (strstr(scenario_name, "a30"))
		{
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_a30";
		}
		else if (strstr(scenario_name, "a50"))
		{
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_a50";
		}
		else if (strstr(scenario_name, "b30"))
		{
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_b30";
		}
		else if (strstr(scenario_name, "b40"))
		{
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_b40";
		}
		else if (strstr(scenario_name, "c10"))
		{
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_c10";
		}
		else if (strstr(scenario_name, "c20"))
		{
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_c20";
		}
		else if (strstr(scenario_name, "c40"))
		{
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_c40";
		}
		else if (strstr(scenario_name, "d20"))
		{
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_d20";
		}
		else if (strstr(scenario_name, "d40"))
		{
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_d40";
		}
		else
		{
			error(_error_silent, "can't display scenario help; unknown scenario is active '%s'", scenario_name);
			return;
		}

		local_player_index = player_ui_get_single_player_local_player_controller(0);
		widget = ui_widget_load_by_name_or_tag(widget_name, NONE, NULL, local_player_index, NONE, NONE, NONE);

		if (widget)
		{
			struct widget_instance *child;

			for (child = widget->first_child; child && child->type != _ui_widget_type_text_box; child = child->next);
			match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2438, child && child->type == _ui_widget_type_text_box, "expected text box widget in player help screen");
			child->text_box.string_list_index = string_index;
		}
		else
		{
			error(_error_silent, "failed to load in-game help dialog");
		}
	}
	else
	{
		error(_error_silent, "can't display scenario help because no scenario is loaded");
	}

	return;
}

void ui_start_main_menu_music(
	void)
{
	if (!widget_globals.main_menu_music_active && !main_menu_fade_active())
	{
		long music_tag_index = tag_loaded(LOOPING_SOUND_DEFINITION_TAG, "sound\\music\\title1\\title1");

		if (music_tag_index != NONE)
		{
			error(_error_silent, "starting main menu music");
			scripted_looping_sound_start(music_tag_index, NONE, 1.f);
			widget_globals.main_menu_music_active = TRUE;
		}
		else
		{
			error(_error_silent, "title music tag not found");
		}
	}

	return;
}

void ui_stop_main_menu_music(
	void)
{
	if (widget_globals.main_menu_music_active == TRUE)
	{
		long music_tag_index = tag_loaded(LOOPING_SOUND_DEFINITION_TAG, "sound\\music\\title1\\title1");

		if (music_tag_index != NONE)
		{
			error(_error_silent, "stopping main menu music");
			scripted_looping_sound_stop(music_tag_index);
		}
		else
		{
			error(_error_silent, "title music tag not found");
		}

		widget_globals.main_menu_music_active = FALSE;
	}

	return;
}

boolean ui_main_menu_music_active(
	void)
{
	return widget_globals.main_menu_music_active;
}

void ui_widgets_disable_pause_game(
	long duration_ticks)
{
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2519, duration_ticks>=0);

	widget_globals.pause_disabled_ticks = duration_ticks;

	return;
}

static void push_widget(
	struct widget_stack_node **top,
	struct _widget_stack_node_data const *data)
{
	struct widget_stack_node *node = pool_new_pointer(widget_memory_pool, sizeof(*node), "c:\\halo\\SOURCE\\interface\\ui_widget.c", 2532);

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2534, top && data);

	if (node)
	{
		node->data = *data;
		node->next = *top;
		*top = node;
	}
	else
	{
		match_vwarn("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2544, FALSE, "out of memory! the UI screen history will be hosed.");
	}

	return;
}

static void pop_widget(
	struct widget_stack_node **top,
	struct _widget_stack_node_data *data)
{
	struct widget_stack_node *node;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2556, top && data);

	node = *top;
	*data = node->data;
	*top = node->next;
	dispose_pointer(widget_memory_pool, node);

	return;
}

static void dispose_widget_stack(
	struct widget_stack_node **top)
{
	while (*top)
	{
		struct widget_stack_node *node = *top;

		*top = node->next;
		dispose_pointer(widget_memory_pool, node);
	}

	return;
}

static boolean ui_widget_load_children_recursive(
	struct widget_instance *widget,
	struct ui_widget_definition *definition)
{
	long child_index;
	boolean success = TRUE;

	if (TEST_FLAG(definition->list_flags, _list_items_generated_from_string_list_tag))
	{
		struct unicode_string_list_group_header *string_list;

		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2592, widget->type == _ui_widget_type_spinner_list, "_list_items_generated_from_string_list_tag flag should only be set for 1-wide spinner list widgets");
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2594, !definition->child_widgets.count, "no child widget references are needed to define list items when generating a list from a string list tag");
		match_vassert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2596, definition->text_label_unicode_string_list_tag.index != NONE, "_list_items_generated_from_string_list_tag flag was set but no string list tag was specified");

		string_list = unicode_string_list_definition_get(definition->text_label_unicode_string_list_tag.index);
		widget_globals.dont_load_children_recursive = TRUE;

		for (child_index = 0; child_index < string_list->string_references.count; child_index++)
		{
			struct widget_instance *child = ui_widget_load_by_name_or_tag(
				NULL,
				widget->tag_index,
				widget,
				widget->local_player_index,
				NONE,
				NONE,
				NONE);

			if (!child)
			{
				success = FALSE;
				break;
			}

			ui_widget_add_child(child, widget);
			widget->list.item_count++;
		}

		widget_globals.dont_load_children_recursive = FALSE;
	}

	for (child_index = 0; child_index < definition->child_widgets.count; child_index++)
	{
		struct child_widget_reference *child_reference = &((struct child_widget_reference *)definition->child_widgets.address)[child_index];
		short local_player_index = widget->local_player_index;

		if (TEST_FLAG(child_reference->flags, _child_widget_use_custom_controller_index_bit))
		{
			if (child_reference->custom_controller_index >= 0 && child_reference->custom_controller_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
			{
				local_player_index = child_reference->custom_controller_index;
			}
			else
			{
				error(_error_silent, "invalid controller index specified for child widget (#%d)", child_reference->custom_controller_index);
			}
		}

		if (child_reference->ui_widget_tag.index != NONE)
		{
			struct widget_instance *child = ui_widget_load_by_name_or_tag(
				NULL,
				child_reference->ui_widget_tag.index,
				widget,
				local_player_index,
				NONE,
				NONE,
				NONE);

			if (!child)
			{
				success = FALSE;
				break;
			}

			child->position.x = child_reference->horizontal_offset + widget->position.x;
			child->position.y = child_reference->vertical_offset + widget->position.y;
			ui_widget_add_child(child, widget);
		}
	}

	if (widget->type == _ui_widget_type_column_list && definition->extended_description_widget_tag.index != NONE)
	{
		widget->list.extended_description_widget = ui_widget_load_by_name_or_tag(
			NULL,
			definition->extended_description_widget_tag.index,
			widget,
			widget->local_player_index,
			NONE,
			NONE,
			NONE);

		if (widget->list.extended_description_widget)
		{
			if (widget->list.extended_description_widget->previous)
			{
				widget->list.extended_description_widget->previous->next = NULL;
			}

			widget->list.extended_description_widget->previous = NULL;
			widget->list.extended_description_widget->parent = NULL;
		}
	}

	if (!TEST_FLAG(definition->flags, _widget_dont_focus_a_specific_child_bit))
	{
		struct widget_instance *child = NULL;

		if (widget->type == _ui_widget_type_spinner_list || widget->type == _ui_widget_type_column_list)
		{
			widget->list.item_index = 0;
			widget->list.last_list_tab_direction = 0;
			child = widget->first_child;
		}
		else if (TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit))
		{
			child = widget->first_child;
		}

		for (; child; child = child->next)
		{
			if (widget->type == _ui_widget_type_spinner_list ||
				widget->type == _ui_widget_type_column_list ||
				widget_instance_can_handle_events(child))
			{
				widget->focused_child = child;
				break;
			}
		}
	}

	return success;
}

static struct widget_instance *widget_instance_find_by_tag_index(
	long tag_index)
{
	long widget_stack;
	struct widget_instance *result = NULL;

	for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS && !result; widget_stack++)
	{
		if (widget_globals.active_widgets[widget_stack])
		{
			result = widget_instance_find_by_tag_index_recursive(widget_globals.active_widgets[widget_stack], tag_index);
		}
	}

	return result;
}

static void ui_widget_add_child(
	struct widget_instance *child,
	struct widget_instance *parent)
{
	struct widget_instance *next_child = parent->first_child;
	struct widget_instance *tail_child = NULL;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2719, (child->previous == NULL) && (child->next == NULL));

	while (next_child)
	{
		tail_child = next_child;
		next_child = next_child->next;
	}

	if (tail_child)
	{
		match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2729, tail_child->next == NULL);

		tail_child->next = child;
		child->previous = tail_child;
	}
	else
	{
		parent->first_child = child;
	}

	return;
}

static void ui_widget_delete_children_recursive(
	struct widget_instance *widget)
{
	struct widget_instance *child = widget->first_child;

	while (child)
	{
		struct widget_instance *next = child->next;

		match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2750, child->previous == NULL);

		if (next)
		{
			match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 2754, next->previous == child);
		}

		ui_widget_delete(child);

		if (next)
		{
			next->previous = NULL;
		}

		child = next;
	}

	return;
}

static struct widget_instance *widget_instance_find_by_tag_index_recursive(
	struct widget_instance *widget,
	long tag_index)
{
	struct widget_instance *result = NULL;

	if (widget->tag_index == tag_index)
	{
		result = widget;
	}
	else
	{
		struct widget_instance *child;

		for (child = widget->first_child; child && !result; child = child->next)
		{
			result = (child->tag_index == tag_index) ? child : widget_instance_find_by_tag_index_recursive(child, tag_index);
		}
	}

	return result;
}

static real widget_instance_get_cumulative_alpha_modifier(
	struct widget_instance const *widget)
{
	struct widget_instance *parent;
	real alpha_modifier = widget->alpha_modifier;

	for (parent = widget->parent; parent; parent = parent->parent)
	{
		alpha_modifier *= parent->alpha_modifier;
	}

	return alpha_modifier;
}

static boolean widget_instance_can_receive_events(
	struct widget_instance const *widget)
{
	boolean result;

	if (widget->disabled)
	{
		result = FALSE;
	}
	else if (widget->parent)
	{
		struct widget_instance *parent;
		struct ui_widget_definition *definition = ui_widget_definition_get(widget->parent->tag_index);

		for (parent = widget->parent, result = TRUE; parent && result; parent = parent->parent)
		{
			struct ui_widget_definition *parent_definition = ui_widget_definition_get(parent->tag_index);

			result = TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit) || parent->type == _ui_widget_type_spinner_list || parent->type == _ui_widget_type_column_list;
			definition = parent_definition;
		}
	}
	else
	{
		result = TRUE;
	}

	return result;
}

static boolean widget_instance_can_handle_events(
	struct widget_instance const *widget)
{
	struct ui_widget_definition *definition = ui_widget_definition_get(widget->tag_index);

	if (!widget->disabled && (definition->event_handler_references.count > 0 || widget->type == _ui_widget_type_spinner_list || widget->type == _ui_widget_type_column_list))
	{
		return TRUE;
	}

	return FALSE;
}

static void widget_instance_initialize(
	struct widget_instance *widget,
	struct widget_instance *parent,
	long tag_index,
	short local_player_index,
	short widget_stack,
	struct ui_widget_definition *definition)
{
	long event_handler_index;

	memset(widget, 0, sizeof(*widget));

	if (TEST_FLAG(definition->list_flags, _list_items_generated_from_string_list_tag) && parent && tag_index == parent->tag_index)
	{
		widget->type = _ui_widget_type_text_box;
	}

	widget->tag_index = tag_index;
	widget->name = definition->name;
	widget->local_player_index = local_player_index;
	widget->type = definition->type;
	widget->visible = TRUE;
	widget->render_regardless_of_controller_index = TEST_FLAG(definition->flags, _widget_render_regardless_of_controller_index_bit);
	widget->pause_game_time = TEST_FLAG(definition->flags, _widget_pause_game_time_bit);
	widget->creation_time = widget_globals.current_system_milliseconds;
	widget->milliseconds_to_auto_close = definition->millisecond_to_auto_close <= 0 ? 0 : definition->millisecond_to_auto_close;
	widget->auto_close_fade_time = definition->millisecond_auto_close_fade_time <= 0 ? 0 : definition->millisecond_auto_close_fade_time;
	widget->alpha_modifier = 1.f;
	widget->parent = parent;

	switch (widget->type)
	{
	case _ui_widget_type_text_box:
		widget->text_box.string_list_index = NONE;
		break;
	}

	if (definition->background_bitmap.index != NONE)
	{
		widget->bitmap_frame_count = TAG_BLOCK_GET_ELEMENT(&bitmap_group_get(definition->background_bitmap.index)->sequences, 0, struct bitmap_group_sequence)->bitmap_count;
	}

	if (!widget_globals.dont_load_children_recursive && !ui_widget_load_children_recursive(widget, definition))
	{
		error(_error_silent, "failed to load widget children");
	}

	for (event_handler_index = 0; event_handler_index < definition->event_handler_references.count; event_handler_index++)
	{
		struct event_handler_reference *event_handler = &((struct event_handler_reference *)definition->event_handler_references.address)[event_handler_index];

		if (event_handler->event_type == _widget_event_created)
		{
			boolean widget_deleted;
			struct event_record event = {0};

			event.controller_index = widget->local_player_index;
			event_handler_dispatch(widget, definition, &event, event_handler, &widget_deleted);
		}
	}

	if (!widget->focused_child)
	{
		struct widget_instance *child;

		for (child = widget->first_child; child; child = child->next)
		{
			if (widget_instance_can_handle_events(child))
			{
				widget_instance_give_focus_directly(widget, child);
			}
		}
	}

	if (widget->pause_game_time == TRUE)
	{
		widget_globals.pause_game_time_count++;

		if (!game_time_get_paused())
		{
			game_time_set_paused(TRUE);
		}

		if (!widget_globals.sound_paused && !main_menu_active_state)
		{
			sound_pause(TRUE);
			widget_globals.sound_paused = TRUE;
		}
	}

	return;
}

static void widget_instance_process_one_event_recursive(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	struct event_record *event,
	boolean *return_widget_deleted)
{
	boolean handled = FALSE;
	boolean widget_deleted = FALSE;
	boolean event_is_for_widget = event_controller_index_compatible_with_widget(event, widget);
	long audio_feedback = _audio_feedback_none;

	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 3067, widget && definition && event && return_widget_deleted);

	if (event->type == _event_type_button &&
		event->value.button_value.value > 1 &&
		event->controller_index >= 0 && event->controller_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS &&
		event->value.button_value.button >= _widget_event_button_dpad_up && event->value.button_value.button <= _widget_event_button_dpad_right &&
		widget_globals.current_system_milliseconds - dpad_event_times[event->controller_index][event->value.button_value.button - _widget_event_button_dpad_up] >= 250)
	{
		event->value.button_value.value = 1;
	}

	if (widget->close_if_local_player_controller_present == TRUE)
	{
		if (widget->local_player_index >= 0 && widget->local_player_index < MAXIMUM_GAMEPADS)
		{
			if (input_has_gamepad(widget->local_player_index))
			{
				ui_widget_delete(widget_instance_get_topmost_parent(widget));
				widget_deleted = TRUE;
			}
		}
		else
		{
			long gamepad_index;

			match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 3107, widget->local_player_index==NONE);

			for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
			{
				if (input_has_gamepad((short)gamepad_index))
				{
					ui_widget_delete(widget_instance_get_topmost_parent(widget));
					widget_deleted = TRUE;
					break;
				}
			}
		}
	}

	if (event_is_for_widget && !widget_deleted && event->type == _event_type_button && event->value.button_value.value == 1)
	{
		long event_handler_index;
		boolean go_back = FALSE;

		if (event->value.button_value.button == _widget_event_button_back)
		{
			go_back = TRUE;

			for (event_handler_index = 0; event_handler_index < definition->event_handler_references.count; event_handler_index++)
			{
				if (((struct event_handler_reference *)definition->event_handler_references.address)[event_handler_index].event_type == _widget_event_button_back)
				{
					go_back = FALSE;
					break;
				}
			}
		}
		else if (event->value.button_value.button == _widget_event_button_b)
		{
			go_back = TRUE;

			for (event_handler_index = 0; event_handler_index < definition->event_handler_references.count; event_handler_index++)
			{
				if (((struct event_handler_reference *)definition->event_handler_references.address)[event_handler_index].event_type == _widget_event_button_b)
				{
					go_back = FALSE;
					break;
				}
			}
		}

		if (go_back)
		{
			widget_instance_go_back_to_previous(widget);
			audio_feedback = _audio_feedback_backward;
			widget_deleted = TRUE;
			handled = TRUE;
		}
	}

	if (!widget_deleted && widget->milliseconds_to_auto_close > 0)
	{
		if (widget_globals.current_system_milliseconds - widget->creation_time >= widget->milliseconds_to_auto_close + widget->auto_close_fade_time)
		{
			ui_widget_delete(widget_instance_get_topmost_parent(widget));
			widget_deleted = TRUE;
		}
		else if (widget->auto_close_fade_time > 0)
		{
			long fade_milliseconds = widget_globals.current_system_milliseconds - widget->creation_time - widget->milliseconds_to_auto_close;

			if (fade_milliseconds > 0)
			{
				widget->alpha_modifier = 1.f - (real)fade_milliseconds / widget->auto_close_fade_time;
			}
		}
	}

	if (!widget_deleted)
	{
		widget_instance_update_animation_parameters(widget);

		if (widget->type == _ui_widget_type_spinner_list)
		{
			spinner_list_update(widget);
		}
		else if (widget->type == _ui_widget_type_column_list)
		{
			column_list_update(widget, definition);
		}

		if (event_is_for_widget && !handled && TEST_FLAG(definition->flags, _widget_dpad_updown_tabs_thru_children_bit) && widget->focused_child && !widget_deleted)
		{
			if (event->type == _event_type_button && event->value.button_value.value == 1)
			{
				switch (event->value.button_value.button)
				{
				case _widget_event_button_dpad_up:
					widget_instance_tab_to_previous_valid_widget(widget);
					audio_feedback = _audio_feedback_cursor;
					handled = TRUE;
					break;
				case _widget_event_button_dpad_down:
					widget_instance_tab_to_next_valid_widget(widget);
					audio_feedback = _audio_feedback_cursor;
					handled = TRUE;
					break;
				}
			}
			else if (event->type == _event_type_left_analog_stick)
			{
				switch (event->value.analog_value.y)
				{
				case SHORT_MIN:
					widget_instance_tab_to_next_valid_widget(widget);
					audio_feedback = _audio_feedback_cursor;
					handled = TRUE;
					break;
				case SHORT_MAX:
					widget_instance_tab_to_previous_valid_widget(widget);
					audio_feedback = _audio_feedback_cursor;
					handled = TRUE;
					break;
				}
			}
		}

		if (event_is_for_widget && !handled && TEST_FLAG(definition->flags, _widget_dpad_leftright_tabs_thru_children_bit) && widget->focused_child && !widget_deleted)
		{
			if (event->type == _event_type_button && event->value.button_value.value == 1)
			{
				switch (event->value.button_value.button)
				{
				case _widget_event_button_dpad_left:
					widget_instance_tab_to_previous_valid_widget(widget);

					if (!audio_feedback)
					{
						audio_feedback = _audio_feedback_cursor;
					}

					handled = TRUE;
					break;
				case _widget_event_button_dpad_right:
					widget_instance_tab_to_next_valid_widget(widget);

					if (!audio_feedback)
					{
						audio_feedback = _audio_feedback_cursor;
					}

					handled = TRUE;
					break;
				}
			}
			else if (event->type == _event_type_left_analog_stick)
			{
				switch (event->value.analog_value.x)
				{
				case SHORT_MIN:
					widget_instance_tab_to_previous_valid_widget(widget);

					if (!audio_feedback)
					{
						audio_feedback = _audio_feedback_cursor;
					}

					handled = TRUE;
					break;
				case SHORT_MAX:
					widget_instance_tab_to_next_valid_widget(widget);

					if (!audio_feedback)
					{
						audio_feedback = _audio_feedback_cursor;
					}

					handled = TRUE;
					break;
				}
			}
		}

		if (event_is_for_widget &&
			TEST_FLAG(definition->flags, _widget_dpad_updown_tabs_thru_list_items_bit) &&
			(widget->type == _ui_widget_type_spinner_list || widget->type == _ui_widget_type_column_list) &&
			!handled && !widget_deleted)
		{
			if (event->type == _event_type_button && event->value.button_value.value == 1)
			{
				switch (event->value.button_value.button)
				{
				case _widget_event_button_dpad_up:
					widget_event_function_list_widget_goto_previous_item(widget, event, &widget_deleted);

					if (!audio_feedback)
					{
						audio_feedback = _audio_feedback_cursor;
					}

					handled = TRUE;
					break;
				case _widget_event_button_dpad_down:
					widget_event_function_list_widget_goto_next_item(widget, event, &widget_deleted);

					if (!audio_feedback)
					{
						audio_feedback = _audio_feedback_cursor;
					}

					handled = TRUE;
					break;
				}
			}
			else if (event->type == _event_type_left_analog_stick)
			{
				switch (event->value.analog_value.y)
				{
				case SHORT_MIN:
					widget_event_function_list_widget_goto_next_item(widget, event, &widget_deleted);

					if (!audio_feedback)
					{
						audio_feedback = _audio_feedback_cursor;
					}

					handled = TRUE;
					break;
				case SHORT_MAX:
					widget_event_function_list_widget_goto_previous_item(widget, event, &widget_deleted);

					if (!audio_feedback)
					{
						audio_feedback = _audio_feedback_cursor;
					}

					handled = TRUE;
					break;
				}
			}
		}

		if (event_is_for_widget &&
			TEST_FLAG(definition->flags, _widget_dpad_leftright_tabs_thru_list_items_bit) &&
			(widget->type == _ui_widget_type_spinner_list || widget->type == _ui_widget_type_column_list) &&
			!handled && !widget_deleted)
		{
			if (event->type == _event_type_button && event->value.button_value.value == 1)
			{
				switch (event->value.button_value.button)
				{
				case _widget_event_button_dpad_left:
					widget_event_function_list_widget_goto_previous_item(widget, event, &widget_deleted);

					if (!audio_feedback)
					{
						audio_feedback = _audio_feedback_cursor;
					}

					handled = TRUE;
					break;
				case _widget_event_button_dpad_right:
					widget_event_function_list_widget_goto_next_item(widget, event, &widget_deleted);

					if (!audio_feedback)
					{
						audio_feedback = _audio_feedback_cursor;
					}

					handled = TRUE;
					break;
				}
			}
			else if (event->type == _event_type_left_analog_stick)
			{
				switch (event->value.analog_value.x)
				{
				case SHORT_MIN:
					widget_event_function_list_widget_goto_previous_item(widget, event, &widget_deleted);

					if (!audio_feedback)
					{
						audio_feedback = _audio_feedback_cursor;
					}

					handled = TRUE;
					break;
				case SHORT_MAX:
					widget_event_function_list_widget_goto_next_item(widget, event, &widget_deleted);

					if (!audio_feedback)
					{
						audio_feedback = _audio_feedback_cursor;
					}

					handled = TRUE;
					break;
				}
			}
		}
	}

	if (event_is_for_widget)
	{
		long event_handler_index;

		for (event_handler_index = 0; event_handler_index < definition->event_handler_references.count && !widget_deleted; event_handler_index++)
		{
			struct event_handler_reference *event_handler = &((struct event_handler_reference *)definition->event_handler_references.address)[event_handler_index];
			boolean matches = FALSE;

			switch (event->type)
			{
			case _event_type_left_analog_stick:
				switch (event_handler->event_type)
				{
				case _widget_event_left_analog_stick_up:
					if (event->value.analog_value.y == SHORT_MAX)
					{
						matches = TRUE;
					}
					break;
				case _widget_event_left_analog_stick_down:
					if (event->value.analog_value.y == SHORT_MIN)
					{
						matches = TRUE;
					}
					break;
				case _widget_event_left_analog_stick_left:
					if (event->value.analog_value.x == SHORT_MIN)
					{
						matches = TRUE;
					}
					break;
				case _widget_event_left_analog_stick_right:
					if (event->value.analog_value.x == SHORT_MAX)
					{
						matches = TRUE;
					}
					break;
				}
				break;
			case _event_type_right_analog_stick:
				switch (event_handler->event_type)
				{
				case _widget_event_right_analog_stick_up:
					if (event->value.analog_value.y == SHORT_MAX)
					{
						matches = TRUE;
					}
					break;
				case _widget_event_right_analog_stick_down:
					if (event->value.analog_value.y == SHORT_MIN)
					{
						matches = TRUE;
					}
					break;
				case _widget_event_right_analog_stick_left:
					if (event->value.analog_value.x == SHORT_MIN)
					{
						matches = TRUE;
					}
					break;
				case _widget_event_right_analog_stick_right:
					if (event->value.analog_value.x == SHORT_MAX)
					{
						matches = TRUE;
					}
					break;
				}
				break;
			case _event_type_button:
				if (event_handler->event_type == event->value.button_value.button)
				{
					if (event->value.button_value.value == 1)
					{
						matches = TRUE;
					}
				}
				break;
			}

			if (matches == TRUE)
			{
				handled = TRUE;
				event_handler_dispatch(widget, definition, event, event_handler, &widget_deleted);
			}
		}
	}

	if (TEST_FLAG(definition->flags, _widget_pass_handled_events_to_all_children_bit) && !TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit))
	{
		match_vwarn("c:\\halo\\SOURCE\\interface\\ui_widget.c", 3477, FALSE, "if the _widget_pass_handled_events_to_all_children_bit flag is checked, _widget_pass_unhandled_events_to_children_bit must also be checked for it to work");
	}

	if ((TEST_FLAG(definition->flags, _widget_pass_handled_events_to_all_children_bit) || !handled) &&
		(TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit) || TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_all_children_bit)) &&
		!widget_deleted)
	{
		if (TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_all_children_bit))
		{
			struct widget_instance *child;

			for (child = widget->first_child; child; child = child->next)
			{
				if (event_controller_index_compatible_with_widget(event, child))
				{
					widget_instance_process_one_event_recursive(child, ui_widget_definition_get(child->tag_index), event, &widget_deleted);

					if (widget_deleted == TRUE)
					{
						break;
					}
				}
			}
		}
		else if (widget->focused_child && event_controller_index_compatible_with_widget(event, widget->focused_child))
		{
			widget_instance_process_one_event_recursive(widget->focused_child, ui_widget_definition_get(widget->focused_child->tag_index), event, &widget_deleted);
		}
	}

	if (widget_deleted == TRUE && TEST_FLAG(definition->flags, _widget_return_to_main_menu_if_no_history_bit))
	{
		long widget_stack;

		for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
		{
			if (widget_globals.active_widgets[widget_stack])
			{
				break;
			}
		}

		if (widget_stack == MAXIMUM_GAMEPADS)
		{
			main_goto_main_menu();
		}
	}

	if (event->type == _event_type_button &&
		event->value.button_value.value == 1 &&
		event->controller_index >= 0 && event->controller_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS &&
		event->value.button_value.button >= _widget_event_button_dpad_up && event->value.button_value.button <= _widget_event_button_dpad_right)
	{
		dpad_event_times[event->controller_index][event->value.button_value.button - _widget_event_button_dpad_up] = widget_globals.current_system_milliseconds;
	}

	ui_play_audio_feedback_sound(audio_feedback);
	*return_widget_deleted = widget_deleted;

	return;
}

static void event_handler_dispatch(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	struct event_record *event,
	struct event_handler_reference *event_handler,
	boolean *return_widget_deleted)
{
	boolean widget_deleted = FALSE;
	boolean success = TRUE;
	boolean function_failed = FALSE;
	long audio_feedback = _audio_feedback_none;
	boolean close_current_widget = FALSE;
	boolean delete_widget = FALSE;
	boolean close_all_widgets = FALSE;

	if (TEST_FLAG(event_handler->flags, _event_handler_run_scenario_script_bit) &&
		event_handler->script_command[0] &&
		!hs_evaluate_by_name(event_handler->script_command))
	{
		error(_error_silent, "failed to run ui widget event script '%s'", event_handler->script_command);
	}

	if (TEST_FLAG(event_handler->flags, _event_handler_run_function_bit) &&
		!widget_deleted &&
		!ui_widget_event_handler_function_invoke(widget, event, event_handler->function, &widget_deleted))
	{
		error(_error_silent, "event handler function failed");
		function_failed = TRUE;
	}
	else
	{
		if (TEST_FLAG(event_handler->flags, _event_handler_give_focus_to_widget) && !widget_deleted)
		{
			if (event_handler->ui_widget_tag.index != NONE)
			{
				widget_instance_give_focus_by_tag(widget, event_handler->ui_widget_tag.index, widget->local_player_index);
				audio_feedback = _audio_feedback_cursor;
			}
			else
			{
				error(_error_silent, "failed to give focus to a widget because event_handler->ui_widget_tag == NONE");
				success = FALSE;
			}
		}

		if (TEST_FLAG(event_handler->flags, _event_handler_reload_self_bit) && !widget_deleted)
		{
			widget_instance_reload_recursive(widget);
		}

		if (TEST_FLAG(event_handler->flags, _event_handler_reload_widget_bit) && !widget_deleted)
		{
			if (event_handler->ui_widget_tag.index != NONE)
			{
				ui_widget_reload_by_tag(event_handler->ui_widget_tag.index);
			}
			else
			{
				error(_error_silent, "failed to reload widget because event_handler->ui_widget_tag == NONE");
				success = FALSE;
			}
		}

		if (TEST_FLAG(event_handler->flags, _event_handler_close_current_widget_bit) && !widget_deleted)
		{
			close_current_widget = TRUE;
		}

		if (TEST_FLAG(event_handler->flags, _event_handler_close_other_widget_bit) && !widget_deleted && event_handler->ui_widget_tag.index != NONE)
		{
			struct widget_instance *other_widget = widget_instance_find_by_tag_index(event_handler->ui_widget_tag.index);

			if (other_widget)
			{
				if (other_widget == widget)
				{
					delete_widget = TRUE;
				}
				else
				{
					ui_widget_delete(other_widget);
				}
			}
			else
			{
				error(_error_silent, "failed to close widget because event_handler->ui_widget_tag == NONE");
				success = FALSE;
			}
		}

		if (TEST_FLAG(event_handler->flags, _event_handler_close_all_widgets_bit) && !widget_deleted)
		{
			close_all_widgets = TRUE;
		}

		if (TEST_FLAG(event_handler->flags, _event_handler_open_widget_bit) && event_handler->ui_widget_tag.index != NONE)
		{
			if (!ui_widget_launch_widget(widget, event_handler->ui_widget_tag.index))
			{
				error(_error_silent, "event handler failed to spawn widget");
				success = FALSE;
			}
			else
			{
				if (!audio_feedback)
				{
					audio_feedback = _audio_feedback_forward;
				}

				widget_deleted = TRUE;
			}
		}

		if (TEST_FLAG(event_handler->flags, _event_handler_replace_with_other_widget_bit) && !widget_deleted && event_handler->ui_widget_tag.index != NONE)
		{
			struct widget_instance *new_widget = ui_widget_load_by_name_or_tag(
				NULL,
				event_handler->ui_widget_tag.index,
				widget,
				widget->local_player_index,
				NONE,
				NONE,
				NONE);

			if (new_widget)
			{
				long widget_stack;
				struct widget_instance *parent = widget->parent;
				struct widget_instance *next = widget->next;
				struct widget_instance *previous = widget->previous;

				if (new_widget->previous)
				{
					new_widget->previous->next = NULL;
				}

				new_widget->previous = NULL;
				new_widget->parent = NULL;
				new_widget->position.x += widget->position.x;
				new_widget->position.y += widget->position.y;

				if (parent)
				{
					new_widget->parent = parent;

					if (parent->first_child == widget)
					{
						parent->first_child = new_widget;
					}

					if (parent->focused_child == widget)
					{
						parent->focused_child = new_widget;
					}
				}

				if (next)
				{
					match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 3721, next->previous == widget);
					next->previous = new_widget;
				}

				new_widget->next = next;

				if (previous)
				{
					match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 3728, previous->next == widget);
					previous->next = new_widget;
				}

				new_widget->previous = previous;

				for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
				{
					if (widget_globals.active_widgets[widget_stack] == new_widget)
					{
						widget_globals.active_widgets[widget_stack] = NULL;
						break;
					}
				}

				if (!audio_feedback)
				{
					audio_feedback = _audio_feedback_forward;
				}

				widget->previous = NULL;
				widget->next = NULL;
				widget->parent = NULL;
				delete_widget = TRUE;
			}
			else
			{
				error(_error_silent, "failed to open widget because the specified widget tag was not found");
				success = FALSE;
			}
		}

		if (TEST_FLAG(event_handler->flags, _event_handler_go_back_to_previous_widget_bit))
		{
			widget_instance_go_back_to_previous(widget);

			if (!audio_feedback)
			{
				audio_feedback = _audio_feedback_backward;
			}

			widget_deleted = TRUE;
		}

		play_sound_tag(event_handler->sound_effect_tag.index);

		if (close_all_widgets)
		{
			long widget_stack;

			for (widget_stack = 0; widget_stack < MAXIMUM_GAMEPADS; widget_stack++)
			{
				if (widget_globals.active_widgets[widget_stack])
				{
					ui_widget_delete(widget_globals.active_widgets[widget_stack]);
				}

				while (widget_globals.widget_stack[widget_stack])
				{
					struct _widget_stack_node_data data;

					pop_widget(&widget_globals.widget_stack[widget_stack], &data);
				}
			}

			widget_deleted = TRUE;
		}
		else if (close_current_widget)
		{
			ui_widget_delete(widget_instance_get_topmost_parent(widget));
			widget_deleted = TRUE;
		}
		else if (delete_widget)
		{
			ui_widget_delete(widget);
			widget_deleted = TRUE;
		}
	}

	if ((!success || function_failed) && TEST_FLAG(event_handler->flags, _event_handler_look_for_conditional_widget_on_failure_bit))
	{
		long conditional_widget_index;

		for (conditional_widget_index = 0; conditional_widget_index < definition->conditional_widget_references.count; conditional_widget_index++)
		{
			struct conditional_widget_reference *conditional_widget = &((struct conditional_widget_reference *)definition->conditional_widget_references.address)[conditional_widget_index];

			if (function_failed == TRUE && TEST_FLAG(conditional_widget->flags, _conditional_widget_load_if_event_handler_function_fails_bit))
			{
				if (!widget_deleted)
				{
					if (conditional_widget->ui_widget_tag.index != NONE)
					{
						if (!ui_widget_launch_widget(widget, conditional_widget->ui_widget_tag.index))
						{
							error(_error_silent, "condition handler failed to spawn widget");
						}
						else
						{
							widget_deleted = TRUE;
						}
					}
				}
				else
				{
					error(_error_silent, "couldn't load conditional widget because the calling widget was deleted");
				}
			}
		}
	}

	ui_play_audio_feedback_sound(audio_feedback);
	*return_widget_deleted = widget_deleted;

	return;
}

static void widget_instance_render_recursive(
	struct widget_instance *widget,
	rectangle2d *clip_rect,
	point2d offset,
	boolean use_white_text,
	boolean plasma)
{
	long game_data_input_index;
	struct ui_widget_definition *definition = ui_widget_definition_get(widget->tag_index);
	real alpha_modifier = widget_instance_get_cumulative_alpha_modifier(widget);

	if (!plasma && TEST_FLAG(definition->flags, _widget_always_render_with_nifty_fx_bit))
	{
		plasma = TRUE;
	}

	offset.x += widget->position.x;
	offset.y += widget->position.y;

	for (game_data_input_index = 0; game_data_input_index < definition->game_data_input_references.count; game_data_input_index++)
	{
		ui_widget_game_data_function_invoke(widget, ((struct game_data_input_reference *)definition->game_data_input_references.address)[game_data_input_index].function);
	}

	if (widget->visible)
	{
		struct widget_instance *child;
		struct bitmap_data *bitmap = bitmap_group_get_bitmap_from_sequence(definition->background_bitmap.index, 0, widget->bitmap_frame_index);
		boolean render_children = TRUE;

		if (bitmap)
		{
			struct rasterizer_dynamic_screen_geometry_parameters parameters;
			rectangle2d clip;
			real alpha = alpha_modifier;
			rectangle2d bounds = definition->bounds;
			rectangle2d *bitmap_clip_rect = clip_rect;
			struct bitmap_group *bitmap_group = bitmap_group_get(definition->background_bitmap.index);

			TAG_BLOCK_GET_ELEMENT(&bitmap_group->sequences, 0, struct bitmap_group_sequence);

			if (plasma)
			{
				set_ui_plasma_effect_color(0.f, 0.05f, 0.05f, 0.05f);
			}

			bounds.x0 += offset.x;
			bounds.x1 += offset.x;
			bounds.y0 += offset.y;
			bounds.y1 += offset.y;

			if (bitmap_clip_rect)
			{
				clip = *bitmap_clip_rect;
				bitmap_clip_rect = &clip;
				bitmap_clip_rect->x0 += offset.x;
				bitmap_clip_rect->x1 += offset.x;
				bitmap_clip_rect->y0 += offset.y;
				bitmap_clip_rect->y1 += offset.y;
			}

			if (TEST_FLAG(definition->flags, _widget_flash_background_bitmap_bit))
			{
				alpha = alpha_modifier * ((cosine(widget_globals.current_system_milliseconds * SECONDS_PER_MILLISECOND * 3.f) + 1.f) * 0.5f);
			}

			draw_bitmap_in_rect(bitmap, &bounds, &bounds, bitmap_clip_rect, PIXEL32_FROM_ALPHA(fast_ftol(alpha * 255.f)) | PIXEL32_RGB_MASK, &parameters, FALSE);

			if (plasma)
			{
				set_ui_plasma_effect_color(0.f, 0.f, 0.f, 0.f);
			}
		}

		switch (widget->type)
		{
		case _ui_widget_type_text_box:
			widget_instance_render_text_box(widget, definition, clip_rect, offset, widget_instance_text_box_is_focused(widget));
			break;
		case _ui_widget_type_spinner_list:
			widget_instance_render_spinner_list(definition, widget, clip_rect, offset, use_white_text);

			if (TEST_FLAG(definition->list_flags, _list_items_generated_from_string_list_tag) && !definition->child_widgets.count)
			{
				render_children = FALSE;
			}
			break;
		case _ui_widget_type_column_list:
			widget_instance_render_column_list(widget, definition, clip_rect, offset, use_white_text);
			render_children = !TEST_FLAG(definition->list_flags, _list_items_generated_in_code);
			break;
		}

		if (render_children)
		{
			for (child = widget->first_child; child; child = child->next)
			{
				use_white_text = child == widget->focused_child;
				plasma = use_white_text && (widget->type == _ui_widget_type_spinner_list || widget->type == _ui_widget_type_column_list);
				widget_instance_render_recursive(child, clip_rect, offset, use_white_text, plasma);
			}
		}
	}

	return;
}

static void widget_instance_render_column_list(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	rectangle2d *clip_rect,
	point2d offset,
	boolean use_white_text)
{
	if (widget->list.extended_description_widget)
	{
		widget->list.extended_description_widget->alpha_modifier = widget_instance_get_cumulative_alpha_modifier(widget);
		widget_instance_render_recursive(widget->list.extended_description_widget, clip_rect, offset, FALSE, TRUE);
	}

	if (TEST_FLAG(definition->list_flags, _list_items_generated_in_code))
	{
		struct widget_instance *child;
		long item_index;

		for (child = widget->first_child, item_index = 0; child && item_index < widget->list.item_count; child = child->next, item_index++)
		{
			widget_instance_render_recursive(child, clip_rect, offset, use_white_text, item_index == widget->list.item_index);
		}
	}

	widget->list.last_list_tab_direction = 0;

	return;
}

static boolean widget_instance_text_box_is_focused(
	struct widget_instance const *widget)
{
	struct widget_instance *parent = widget->parent;
	boolean focused = parent ? parent->focused_child == widget : TRUE;

	if (!focused && parent)
	{
		while (parent)
		{
			struct widget_instance *grandparent = parent->parent;

			if (grandparent)
			{
				if (grandparent->focused_child != parent)
				{
					break;
				}

				focused = (grandparent->type == _ui_widget_type_spinner_list || grandparent->type == _ui_widget_type_column_list) && grandparent->focused_child == parent;
			}

			parent = grandparent;
		}
	}

	return focused;
}

static short get_icon_type(
	wchar_t const *string)
{
	short icon_type;

	for (icon_type = 0; icon_type < NUMBEROF(icon_names); icon_type++)
	{
		if (!_wcsnicmp(string, icon_names[icon_type], wcslen(icon_names[icon_type])))
		{
			break;
		}
	}

	if (icon_type == NUMBEROF(icon_names))
	{
		icon_type = NONE;
	}

	return icon_type;
}

static void render_state_bitmap(
	rectangle2d *bounds,
	rectangle2d *cursor_bounds,
	unsigned long color,
	struct icon_hud_element_definition const *icon)
{
	struct scenario *scenario = global_scenario_get();
	struct game_globals *game_globals = scenario_get_game_globals();
	struct game_globals_interface_tag_references *interface_tag_references = game_globals->interface_tag_references.count ?
		TAG_BLOCK_GET_ELEMENT(&game_globals->interface_tag_references, 0, struct game_globals_interface_tag_references) :
		NULL;
	long bitmap_group_index = interface_tag_references->interface_tag_references[_interface_bitmap_iface_map2].index;
	struct bitmap_data const *bitmap = NULL;
	real_rectangle2d const *clip = NULL;
	short frame_index = 0;

	if (icon->frame_rate)
	{
		frame_index = (short)(system_milliseconds() * TICKS_PER_SECOND / MILLISECONDS_PER_SECOND / icon->frame_rate);
	}

	hud_retrieve_bitmap_and_bounding_rect(bitmap_group_index, icon->sequence_index, frame_index, &bitmap, &clip);

	if (bitmap && _texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
	{
		point2d corner;
		real scale = hud_globals_get_scale(local_player_count() > 1);

		corner.x = (short)(icon->offset.x * scale + cursor_bounds->x0 + 1.f);
		corner.y = (short)(cursor_bounds->y1 - icon->offset.y * scale - 2.f);
		hud_draw_bitmap_direct(
			bitmap,
			_hud_bottom_left,
			&corner,
			clip,
			scale,
			0.f,
			TEST_FLAG(icon->flags, _hud_icon_use_color_bit) ? icon->color : color,
			FALSE);

		if (TEST_FLAG(icon->flags, _hud_icon_absolute_width_bit))
		{
			cursor_bounds->x0 = icon->width_offset + corner.x;
		}
		else if (clip)
		{
			cursor_bounds->x0 = (short)((clip->x1 - clip->x0) * bitmap->width + icon->width_offset + corner.x);
		}
		else
		{
			cursor_bounds->x0 = bitmap->width + icon->width_offset + corner.x;
		}
	}

	return;
}

static void render_state_text(
	rectangle2d *bounds,
	rectangle2d *cursor_bounds,
	wchar_t const *string)
{
	rectangle2d text_bounds;
	short initial_indent = cursor_bounds->x0 - bounds->x0;

	if (initial_indent < 0)
	{
		error(_error_silent, "initial_indent<0 in render_state_text() and was about to explode");
	}

	initial_indent = MAX(0, initial_indent);

	draw_string_set_indents(initial_indent, 0);
	draw_unicode_string_compute_bounds(bounds, string, &text_bounds, cursor_bounds);
	cursor_bounds->x0 -= 3;
	text_bounds.x0 = bounds->x0;
	rasterizer_draw_unicode_string(&text_bounds, NULL, NULL, 0, string);
	bounds->y0 = cursor_bounds->y0;

	return;
}

static boolean string_has_icons_to_draw(
	wchar_t const *string)
{
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 4181, string);

	while (string)
	{
		wchar_t const *icon_string = wcschr(string, L'%');

		if (!icon_string)
		{
			break;
		}

		icon_string++;

		if (get_icon_type(icon_string) != NONE)
		{
			return TRUE;
		}

		string = icon_string;
	}

	return FALSE;
}

static boolean should_flip_sticks_for_local_player(
	short local_player_index)
{
	struct game_input_preferences preferences;

	if (local_player_index == NONE)
	{
		local_player_index = local_player_get_next(NONE);
	}

	memset(&preferences, 0, sizeof(preferences));

	if (local_player_index != NONE)
	{
		input_abstraction_get_local_player_preferences(local_player_index, &preferences);
	}

	switch (preferences.joystick_controls)
	{
	case _joystick_preset_south_paw:
	case _joystick_preset_legacy_south_paw:
		return TRUE;
	}

	return FALSE;
}

short remap_sticks_for_local_player(
	short icon,
	short local_player_index)
{
	switch (icon)
	{
	case _gamepad_stick_left_icon:
	case _hud_icon_move:
		match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 4243, 16 == get_icon_type(L"left-stick"));
		match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 4244, 30 == get_icon_type(L"move"));
		icon = should_flip_sticks_for_local_player(local_player_index) ? _gamepad_stick_right_icon : _gamepad_stick_left_icon;
		break;
	case _gamepad_stick_right_icon:
	case _hud_icon_look:
		match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 4250, 17 == get_icon_type(L"right-stick"));
		match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 4251, 31 == get_icon_type(L"look"));
		icon = should_flip_sticks_for_local_player(local_player_index) ? _gamepad_stick_left_icon : _gamepad_stick_right_icon;
		break;
	}

	return icon;
}

void draw_string_and_hack_in_icons(
	rectangle2d *bounds,
	rectangle2d *clip,
	point2d *cursor_reference,
	short height_adjust,
	wchar_t const *instring,
	boolean ignore_icon_color)
{
	static wchar_t buffer[1024];
	wchar_t *string = buffer;
	rectangle2d cursor_bounds = *bounds;

	wcscpy(buffer, instring);

	while (string)
	{
		short icon_type;
		wchar_t *icon_string = wcschr(string, L'%');

		if (!icon_string)
		{
			break;
		}

		*icon_string = L'\0';
		icon_string++;
		render_state_text(bounds, &cursor_bounds, string);
		string = icon_string;

		icon_type = get_icon_type(icon_string);

		if (icon_type == NONE)
		{
			render_state_text(bounds, &cursor_bounds, L"%");
		}
		else
		{
			struct icon_hud_element_definition *icon;
			real_argb_color icon_color;
			real_argb_color text_color;
			pixel32 alpha;
			short icon_index;
			short width_offset;
			byte flags;

			string = icon_string + wcslen(icon_names[icon_type]);
			icon_type = remap_sticks_for_local_player(icon_type, local_player_index_for_draw_string_and_hack_in_icons);
			icon_index = NONE;

			if (icon_type > _hud_icon_specific_button_end)
			{
				if (icon_type <= _hud_icon_remapped_button_end)
				{
					if (icon_type <= _hud_icon_remapped_action_button_end)
					{
						struct game_input_preferences preferences;

						input_abstraction_get_local_player_preferences(local_player_index_for_draw_string_and_hack_in_icons, &preferences);
						icon_index = preferences.game_control_to_xbox_buttons[remapped_icon_button_indices[icon_type - _hud_icon_remapped_button_start]];
					}
					else
					{
						icon_index = remapped_icon_button_indices[icon_type - _hud_icon_remapped_button_start];

						switch (icon_type)
						{
						case _hud_icon_accept:
							icon_index = _gamepad_binary_button_start;
							break;
						case _hud_icon_back:
							icon_index = _gamepad_binary_button_back;
							break;
						case _hud_icon_move:
							icon_index = _gamepad_stick_left_icon;
							break;
						case _hud_icon_look:
							icon_index = _gamepad_stick_right_icon;
							break;
						}
					}
				}
				else
				{
					match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 4341, FALSE);
				}
			}
			else
			{
				icon_index = icon_type;
			}

			icon = TAG_BLOCK_GET_ELEMENT(&hud_globals->messaging.button_icons, icon_index, struct icon_hud_element_definition);
			flags = icon->flags;
			width_offset = icon->width_offset;
			pixel32_to_real_argb_color(icon->color, &icon_color);
			SET_FLAG(icon->flags, _hud_icon_use_color_bit, FALSE);
			match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 4362, icon_index>=0 && icon_index<NUM_ICONS);

			if (icon_is_special[icon_index])
			{
				SET_FLAG(icon->flags, _hud_icon_absolute_width_bit, FALSE);
				icon->width_offset = -5;
			}

			draw_string_get_color(&text_color);
			alpha = PIXEL32_FROM_ALPHA((pixel32)(text_color.alpha * 255.f));

			if (!icon->color || ignore_icon_color)
			{
				icon_color = text_color;
			}

			icon_color.red *= text_color.alpha;
			icon_color.green *= text_color.alpha;
			icon_color.blue *= text_color.alpha;
			render_state_bitmap(bounds, &cursor_bounds, alpha | PIXEL32_RGB_BITS(real_argb_color_to_pixel32(&icon_color)), icon);
			bounds->x0++;
			icon->flags = flags;
			icon->width_offset = width_offset;
		}
	}

	if (string)
	{
		render_state_text(bounds, &cursor_bounds, string);
	}

	draw_string_set_indents(0, 0);

	return;
}

static void widget_instance_render_text_box(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	rectangle2d *clip_rect,
	point2d offset,
	boolean use_white_text)
{
	long font_index;
	short justification;

	if (definition->text_label_unicode_string_list_tag.index != NONE)
	{
		short string_index;
		wchar_t *string;
		long string_length;

		if (widget->text_box.string_list_index == NONE)
		{
			string_index = definition->string_list_index;
		}
		else
		{
			string_index = widget->text_box.string_list_index;
		}

		string = unicode_string_list_get_string(definition->text_label_unicode_string_list_tag.index, string_index);
		string_length = ustrlen(string);
		widget->text_box.string = pool_resize_pointer(widget_memory_pool, widget->text_box.string, (string_length + 1) * sizeof(wchar_t), "c:\\halo\\SOURCE\\interface\\ui_widget.c", 4421);

		if (widget->text_box.string)
		{
			memcpy(widget->text_box.string, string, string_length * sizeof(wchar_t));
			widget->text_box.string[string_length] = L'\0';
		}
		else
		{
			widget->text_box.string = L"<out of memory>";
		}
	}

	if (widget->text_box.string && *widget->text_box.string)
	{
		long search_and_replace_index;

		for (search_and_replace_index = 0; search_and_replace_index < definition->search_and_replace_references.count; search_and_replace_index++)
		{
			struct search_and_replace_reference *reference = &((struct search_and_replace_reference *)definition->search_and_replace_references.address)[search_and_replace_index];

			if (reference && reference->search_string[0])
			{
				wchar_t search_string[TAG_STRING_LENGTH+1];
				wchar_t const *replace_string = ui_widget_search_and_replace_invoke(widget, reference->replace_function);

				search_and_replace(ascii_to_wide(reference->search_string, search_string, sizeof(search_string)), replace_string, &widget->text_box.string);
			}
		}

		font_index = definition->font_tag.index;

		if (font_index == NONE)
		{
			error(_error_silent, "failed to render text box widget because the font tag was invalid");
		}
		else
		{
			justification = definition->justification;

			if (justification >= 0 && justification < NUMBER_OF_TEXT_JUSTIFICATIONS)
			{
				if (widget->visible)
				{
					real_argb_color color;
					real alpha_modifier = widget_instance_get_cumulative_alpha_modifier(widget);
					rectangle2d bounds = definition->bounds;
					rectangle2d clip = clip_rect ? *clip_rect : definition->bounds;

					bounds.y1 += offset.y;
					bounds.x1 += offset.x;
					bounds.x0 += offset.x;
					bounds.y0 += offset.y;
					bounds.x0 += definition->hoffset;
					bounds.y0 += definition->voffset;

					if (use_white_text)
					{
						color = get_ui_argb_white();
						color.alpha = definition->text_color.alpha;
					}
					else
					{
						color = definition->text_color;

						if (1.f == color.red && 1.f == color.green && 1.f == color.blue)
						{
							color = get_ui_argb_white();
							color.alpha = definition->text_color.alpha;
						}
					}

					color.alpha *= alpha_modifier;

					if (TEST_FLAG(definition->text_box_flags, _text_box_flashing_text_bit))
					{
						color.alpha = color.alpha * ((cosine(widget_globals.current_system_milliseconds * SECONDS_PER_MILLISECOND * 3.f) + 1.5f) * 0.4f);
					}

					draw_string_set_draw_mode(font_index, NONE, justification, 0, &color);

					if (string_has_icons_to_draw(widget->text_box.string))
					{
						draw_string_and_hack_in_icons(&bounds, &clip, NULL, 0, widget->text_box.string, FALSE);
					}
					else
					{
						rasterizer_draw_unicode_string(&bounds, &clip, NULL, 0, widget->text_box.string);
					}
				}
			}
			else
			{
				error(_error_silent, "failed to render text box widget because the justification was invalid");
			}
		}
	}

	return;
}

static void widget_instance_render_spinner_list(
	struct ui_widget_definition *definition,
	struct widget_instance *widget,
	rectangle2d *clip_rect,
	point2d offset,
	boolean use_white_text)
{
	short header_frame_index = 0;
	short footer_frame_index = 0;
	real alpha_modifier = widget_instance_get_cumulative_alpha_modifier(widget);

	if (widget->visible)
	{
		struct rasterizer_dynamic_screen_geometry_parameters parameters;
		rectangle2d bounds;
		struct bitmap_data *bitmap;
		wchar_t *string;

		if (widget->list.last_list_tab_direction)
		{
			switch (widget->list.last_list_tab_direction >= 0 ? 1 : -1)
			{
			case -1:
				widget->list.last_list_tab_direction++;
				header_frame_index = 1;
				break;
			case 1:
				widget->list.last_list_tab_direction--;
				footer_frame_index = 1;
				break;
			}
		}

		bitmap = bitmap_group_get_bitmap_from_sequence(definition->list_header_bitmap.index, 0, header_frame_index);

		if (bitmap)
		{
			pixel32 alpha = fast_ftol(alpha_modifier * 255.f);

			memset(&parameters, 0, sizeof(parameters));
			bounds = definition->list_header_bounds;
			bounds.x0 += offset.x;
			bounds.y0 += offset.y;
			bounds.x1 += offset.x;
			bounds.y1 += offset.y;
			draw_bitmap_in_rect(bitmap, &bounds, &bounds, clip_rect, PIXEL32_FROM_ALPHA(alpha) | PIXEL32_RGB_MASK, &parameters, FALSE);
		}

		bitmap = bitmap_group_get_bitmap_from_sequence(definition->list_footer_bitmap.index, 0, footer_frame_index);

		if (bitmap)
		{
			pixel32 alpha = fast_ftol(alpha_modifier * 255.f);

			memset(&parameters, 0, sizeof(parameters));
			bounds = definition->list_footer_bounds;
			bounds.x0 += offset.x;
			bounds.y0 += offset.y;
			bounds.x1 += offset.x;
			bounds.y1 += offset.y;
			draw_bitmap_in_rect(bitmap, &bounds, &bounds, clip_rect, PIXEL32_FROM_ALPHA(alpha) | PIXEL32_RGB_MASK, &parameters, FALSE);
		}

		if (!definition->child_widgets.count)
		{
			if (definition->text_label_unicode_string_list_tag.index != NONE)
			{
				short item_index = widget->list.item_index;
				wchar_t *list_string = unicode_string_list_get_string(definition->text_label_unicode_string_list_tag.index, item_index);
				long string_length = ustrlen(list_string);

				string = pool_new_pointer(widget_memory_pool, (string_length + 1) * sizeof(wchar_t), "c:\\halo\\SOURCE\\interface\\ui_widget.c", 4610);

				if (string)
				{
					long search_and_replace_index;

					memcpy(string, list_string, string_length * sizeof(wchar_t));
					string[string_length] = L'\0';

					for (search_and_replace_index = 0; search_and_replace_index < definition->search_and_replace_references.count; search_and_replace_index++)
					{
						struct search_and_replace_reference *reference = &((struct search_and_replace_reference *)definition->search_and_replace_references.address)[search_and_replace_index];

						if (reference && reference->search_string[0])
						{
							wchar_t search_string[TAG_STRING_LENGTH+1];
							wchar_t const *replace_string = ui_widget_search_and_replace_invoke(widget, reference->replace_function);

							search_and_replace(ascii_to_wide(reference->search_string, search_string, sizeof(search_string)), replace_string, &string);
						}
					}
				}
			}
			else
			{
				string = widget->list.string;
			}

			if (string)
			{
				if (definition->font_tag.index == NONE)
				{
					error(_error_silent, "failed to render spinner list item because the font tag was invalid");
				}
				else if (definition->justification >= 0 && definition->justification < NUMBER_OF_TEXT_JUSTIFICATIONS)
				{
					real_argb_color color;
					real text_alpha_modifier = widget_instance_get_cumulative_alpha_modifier(widget);
					rectangle2d clip = clip_rect ? *clip_rect : definition->bounds;
					rectangle2d bounds = definition->bounds;

					bounds.x1 += offset.x;
					bounds.y1 += offset.y;
					bounds.x0 += offset.x;
					bounds.y0 += offset.y;

					if (use_white_text)
					{
						color.alpha = definition->text_color.alpha;
						color.rgb = get_ui_rgb_white();
					}
					else
					{
						color = definition->text_color;

						if (1.f == color.red && 1.f == color.green && 1.f == color.blue)
						{
							color.alpha = definition->text_color.alpha;
						}
					}

					color.alpha *= text_alpha_modifier;

					if (TEST_FLAG(definition->text_box_flags, _text_box_flashing_text_bit))
					{
						color.alpha = color.alpha * ((sine(widget_globals.current_system_milliseconds * SECONDS_PER_MILLISECOND * 3.f) + 1.f) * 0.5f);
					}

					draw_string_set_draw_mode(definition->font_tag.index, NONE, definition->justification, 0, &color);
					rasterizer_draw_unicode_string(&bounds, &clip, NULL, 0, string);
				}
				else
				{
					error(_error_silent, "failed to render spinner list item because the justification was invalid");
				}
			}

			if (definition->text_label_unicode_string_list_tag.index != NONE)
			{
				dispose_pointer(widget_memory_pool, string);
			}
		}
	}

	return;
}

static void widget_instance_give_focus_by_tag(
	struct widget_instance *widget,
	long tag_index,
	short local_player_index)
{
	struct widget_instance *topmost_parent = widget_instance_get_topmost_parent(widget);
	struct widget_instance *focus_widget = widget_instance_find_by_tag_index_recursive(topmost_parent, tag_index);

	if (focus_widget)
	{
		widget_instance_give_focus_directly(topmost_parent, focus_widget);
	}
	else
	{
		error(_error_silent, "failed to find event focus target widget");
	}

	return;
}

static void widget_instance_go_back_to_previous(
	struct widget_instance *widget)
{
	struct _widget_stack_node_data data;
	long previous_widget_tag_index;
	short widget_stack = widget->local_player_index == NONE ? 0 : widget->local_player_index;
	short previous_local_player_index = NONE;

	if (widget_globals.widget_stack[widget_stack])
	{
		pop_widget(&widget_globals.widget_stack[widget_stack], &data);
		previous_local_player_index = data.local_player_index;
		previous_widget_tag_index = data.previous_widget_tag;
	}
	else
	{
		previous_widget_tag_index = NONE;
	}

	ui_widget_delete(widget_instance_get_topmost_parent(widget));

	if (previous_widget_tag_index != NONE)
	{
		struct widget_instance *previous_widget = ui_widget_load_by_name_or_tag(
			NULL,
			previous_widget_tag_index,
			NULL,
			previous_local_player_index,
			NONE,
			NONE,
			NONE);

		if (previous_widget)
		{
			widget_instance_set_focused_child_by_index(data.focused_child_parent_widget_tag, previous_widget, data.focused_child_index);
		}
	}

	return;
}

static void widget_instance_update_animation_parameters(
	struct widget_instance *widget)
{
	widget->bitmap_frame_minimum = FLOOR(widget->bitmap_frame_minimum, 0);
	widget->bitmap_frame_maximum = FLOOR(widget->bitmap_frame_maximum, 0);

	return;
}

static void widget_instance_reload_recursive(
	struct widget_instance *widget)
{
	return;
}

static void ui_widget_reload_by_tag(
	long tag_index)
{
	return;
}

static void widget_instance_give_focus_directly(
	struct widget_instance *widget,
	struct widget_instance *focus_widget)
{
	struct widget_instance *old_focused_child = widget_instance_get_topmost_parent(widget)->focused_child;

	if (focus_widget->disabled == TRUE)
	{
		struct widget_instance *candidate;

		for (candidate = focus_widget->next; candidate; candidate = candidate->next)
		{
			if (widget_instance_can_handle_events(candidate))
			{
				break;
			}
		}

		if (!candidate && focus_widget->parent)
		{
			for (candidate = focus_widget->parent->first_child; candidate; candidate = candidate->next)
			{
				if (widget_instance_can_handle_events(candidate))
				{
					break;
				}
			}

			if (candidate == focus_widget->parent->focused_child)
			{
				for (candidate = focus_widget->previous; candidate; candidate = candidate->previous)
				{
					if (widget_instance_can_handle_events(candidate))
					{
						break;
					}
				}
			}
		}

		if (candidate)
		{
			focus_widget = candidate;
		}
	}

	if (old_focused_child && focus_widget && old_focused_child->parent == focus_widget->parent && old_focused_child->parent)
	{
		focus_widget->parent->focused_child = focus_widget;
	}
	else
	{
		if (old_focused_child)
		{
			do
			{
				old_focused_child->parent->focused_child = NULL;
				old_focused_child = old_focused_child->focused_child;
			}
			while (old_focused_child);
		}

		while (focus_widget->parent)
		{
			focus_widget->parent->focused_child = focus_widget;
			focus_widget = focus_widget->parent;
		}
	}

	return;
}

static void widget_instance_set_focused_child_by_index(
	long tag_index,
	struct widget_instance *widget,
	short child_index)
{
	if (tag_index != NONE)
	{
		struct widget_instance *parent = widget_instance_find_by_tag_index_recursive(widget, tag_index);

		if (parent)
		{
			if (child_index >= 0)
			{
				struct widget_instance *child;
				long index;

				for (child = parent->first_child, index = 0; child; child = child->next, index++)
				{
					if (child->type == _ui_widget_type_spinner_list && ui_widget_definition_get(child->tag_index)->child_widgets.count > 1)
					{
						break;
					}

					if (index == child_index)
					{
						widget_instance_give_focus_directly(widget, child);

						if (child->parent && (child->parent->type == _ui_widget_type_spinner_list || child->parent->type == _ui_widget_type_column_list))
						{
							child->parent->list.item_index = (short)index;
						}
						break;
					}
				}
			}
			else
			{
				if (widget_instance_can_receive_events(parent) &&
					(parent->type != _ui_widget_type_spinner_list || ui_widget_definition_get(parent->tag_index)->child_widgets.count <= 1))
				{
					widget_instance_give_focus_directly(widget, parent);
				}
			}
		}
	}

	return;
}

static void play_sound_tag(
	long tag_index)
{
	if (tag_index != NONE)
	{
		unspatialized_impulse_sound_new(tag_index, 1.f);
	}

	return;
}

static long search_and_replace(
	wchar_t const *search_string,
	wchar_t const *replace_string,
	wchar_t **string)
{
	long replace_count = 0;

	if (string && *string)
	{
		long difference;
		long replace_length;
		long search_length;
		long string_length;
		wchar_t *match;
		wchar_t *result = *string;

		search_length = ustrlen(search_string);
		replace_length = ustrlen(replace_string);
		string_length = ustrlen(result) + 1;

		if (replace_length <= search_length)
		{
			difference = search_length - replace_length;

			for (match = ustrstr(result, search_string); match; match = ustrstr(result, search_string))
			{
				replace_count++;
				memcpy(match, replace_string, sizeof(wchar_t) * replace_length);

				if (difference > 0)
				{
					memmove(match + replace_length, match + search_length, sizeof(wchar_t) * (string_length - (match - result + replace_length)));
					string_length -= difference;
				}
			}
		}
		else
		{
			difference = replace_length - search_length;

			for (match = ustrstr(result, search_string); match; match = ustrstr(match + search_length, search_string))
			{
				replace_count++;
			}

			if (replace_count > 0)
			{
				result = pool_resize_pointer(widget_memory_pool, result, sizeof(wchar_t) * (string_length + difference * replace_count), "c:\\halo\\SOURCE\\interface\\ui_widget.c", 4994);

				if (!result)
				{
					replace_count = NONE;
				}
				else
				{
					for (match = ustrstr(result, search_string); match; match = ustrstr(result, search_string))
					{
						memmove(match + replace_length, match + search_length, sizeof(wchar_t) * (string_length - (match - result + search_length)));
						memcpy(match, replace_string, sizeof(wchar_t) * replace_length);
						string_length += difference;
					}

					*string = result;
				}
			}
		}
	}

	return replace_count;
}

static void spinner_list_update(
	struct widget_instance *widget)
{
	struct widget_instance *child;

	for (child = widget->first_child; child; child = child->next)
	{
		child->bitmap_frame_index = 0;

		if (child == widget->focused_child && child->bitmap_frame_count == 2)
		{
			child->bitmap_frame_index = 1;
		}
	}

	return;
}

static void column_list_update(
	struct widget_instance *widget,
	struct ui_widget_definition *definition)
{
	struct widget_instance *child;

	for (child = widget->first_child; child; child = child->next)
	{
		if (child == widget->focused_child)
		{
			if (child->bitmap_frame_count == 2)
			{
				child->bitmap_frame_index = 1;
			}
		}
		else if (child->bitmap_frame_count == 2)
		{
			child->bitmap_frame_index = 0;
		}
	}

	return;
}

static struct widget_instance *widget_instance_get_tail_child_widget(
	struct widget_instance const *widget)
{
	struct widget_instance *child = widget->first_child;

	if (child)
	{
		while (child->next)
		{
			child = child->next;
		}
	}

	return child;
}

static void widget_instance_tab_to_next_valid_widget(
	struct widget_instance *widget)
{
	struct widget_instance *child;

	if (widget->focused_child && widget->focused_child->next)
	{
		child = widget->focused_child->next;
	}
	else
	{
		child = widget->first_child;
	}

	while (child && child != widget->focused_child)
	{
		struct ui_widget_definition *definition = ui_widget_definition_get(child->tag_index);

		if (definition->event_handler_references.count > 0 ||
			TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit) ||
			widget->type == _ui_widget_type_spinner_list ||
			widget->type == _ui_widget_type_column_list)
		{
			widget->focused_child = child;
			break;
		}

		child = child->next;

		if (!child)
		{
			child = widget->first_child;
		}
	}

	return;
}

static void widget_instance_tab_to_previous_valid_widget(
	struct widget_instance *widget)
{
	struct widget_instance *child;

	if (widget->focused_child)
	{
		if (widget->focused_child->previous)
		{
			child = widget->focused_child->previous;
		}
		else
		{
			child = widget_instance_get_tail_child_widget(widget);
		}
	}
	else if (widget->first_child->previous)
	{
		child = widget->first_child->previous;
	}
	else
	{
		child = widget->first_child;
	}

	while (child && child != widget->focused_child)
	{
		struct ui_widget_definition *definition = ui_widget_definition_get(child->tag_index);

		if (definition->event_handler_references.count > 0 ||
			TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit) ||
			widget->type == _ui_widget_type_spinner_list ||
			widget->type == _ui_widget_type_column_list)
		{
			widget->focused_child = child;
			break;
		}

		if (child->previous)
		{
			child = child->previous;
		}
		else
		{
			child = widget_instance_get_tail_child_widget(widget);
		}
	}

	return;
}

real_rgb_color get_ui_rgb_white(
	void)
{
	real_rgb_color color = *global_real_rgb_white;

	color.red = ui_rgb_white.red;
	color.green = ui_rgb_white.green;
	color.blue = ui_rgb_white.blue;

	return color;
}

real_argb_color get_ui_argb_white(
	void)
{
	real_argb_color color = *global_real_argb_white;

	color.red = ui_rgb_white.red;
	color.green = ui_rgb_white.green;
	color.blue = ui_rgb_white.blue;

	return color;
}

static boolean ui_check_for_pause_game(
	void)
{
	long gamepad_index;
	boolean start_pressed = FALSE;
	boolean network_game = network_game_is_active();

	if (game_in_progress() &&
		!cinematic_in_progress() &&
		game_connection() != _game_connection_film_playback &&
		!main_menu_active_state &&
		!widget_globals.pause_disabled_ticks)
	{
		for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
		{
			if (input_has_gamepad((short)gamepad_index) && local_player_exists(gamepad_index))
			{
				struct gamepad_state const *gamepad_state = input_get_gamepad_state((short)gamepad_index);

				if (gamepad_state->buttons[_gamepad_binary_button_start] == 1)
				{
					start_pressed = TRUE;
					break;
				}
			}
		}
	}

	if (start_pressed)
	{
		short local_player_index;
		boolean first_local_player = TRUE;
		short local_player_count = 0;
		short found_local_player_index = NONE;

		for (local_player_index = local_player_get_next(NONE); local_player_index != NONE; local_player_index = local_player_get_next(local_player_index))
		{
			if (local_player_index == (short)gamepad_index)
			{
				found_local_player_index = (short)gamepad_index;

				if (local_player_count >= 1)
				{
					first_local_player = FALSE;
				}
			}

			local_player_count++;
		}

		if (network_game)
		{
			if (game_engine_allow_pause() && found_local_player_index == (short)gamepad_index)
			{
				if (!widget_globals.active_widgets[(short)gamepad_index])
				{
					struct network_game_client *client = global_network_game_client_get();
					char const *widget_name = NULL;

					network_game_client_get_game(client);
					network_game_client_get_machine_index(client);

					switch (local_player_count)
					{
					case 1:
						widget_name = "ui\\shell\\multiplayer_game\\pause_game\\1p_pause_game";
						break;
					case 2:
						widget_name = "ui\\shell\\multiplayer_game\\pause_game\\2p_pause_game";
						break;
					case 3:
						if (first_local_player == TRUE)
						{
							widget_name = "ui\\shell\\multiplayer_game\\pause_game\\2p_pause_game";
						}
						else
						{
							widget_name = "ui\\shell\\multiplayer_game\\pause_game\\4p_pause_game";
						}
						break;
					case 4:
						widget_name = "ui\\shell\\multiplayer_game\\pause_game\\4p_pause_game";
						break;
					default:
						error(_error_silent, "invalid local player count for multiplayer game");
						break;
					}

					if (widget_name)
					{
						if (!ui_widget_load_by_name_or_tag(widget_name, NONE, NULL, (short)gamepad_index, NONE, NONE, NONE))
						{
							error(_error_silent, "failed to load multiplayer pause game window");
						}
					}
				}
				else
				{
					ui_widget_delete(widget_globals.active_widgets[(short)gamepad_index]);
				}
			}
		}
		else
		{
			switch (local_player_count)
			{
			case 0:
			case 1:
				if (widget_globals.active_widgets[(short)gamepad_index])
				{
					if (game_time_get_paused() == TRUE)
					{
						ui_widgets_close_all();
					}
				}
				else if (!ui_widget_load_by_name_or_tag("ui\\shell\\solo_game\\pause_game\\pause_game", NONE, NULL, (short)gamepad_index, NONE, NONE, NONE))
				{
					error(_error_silent, "failed to load full screen pause game window");
				}
				break;
			case 2:
				if (widget_globals.active_widgets[(short)gamepad_index])
				{
					if (game_time_get_paused() == TRUE)
					{
						ui_widgets_close_all();
					}
				}
				else if (!game_time_get_paused() &&
					!ui_widget_load_by_name_or_tag("ui\\shell\\solo_game\\pause_game\\pause_game_split_screen", NONE, NULL, (short)gamepad_index, NONE, NONE, NONE))
				{
					error(_error_silent, "failed to load split screen pause game window");
				}
				break;
			default:
				error(_error_silent, "the ui seems to be confused... assuming you are playing full-screen single player?");

				if (!ui_widgets_active())
				{
					if (!ui_widget_load_by_name_or_tag("ui\\shell\\solo_game\\pause_game\\pause_game", NONE, NULL, (short)gamepad_index, NONE, NONE, NONE))
					{
						error(_error_silent, "failed to load full screen pause game window");
					}
				}
				else
				{
					ui_widgets_close_all();
				}
				break;
			}
		}
	}

	widget_globals.pause_disabled_ticks = FLOOR(widget_globals.pause_disabled_ticks - 1, 0);

	return start_pressed;
}

static struct widget_instance *ui_widget_launch_widget(
	struct widget_instance *widget,
	long tag_index)
{
	struct widget_instance *topmost_parent;
	long parent_tag_index;
	short child_index;
	struct widget_instance *result;
	short local_player_index;
	struct ui_widget_definition *definition = ui_widget_definition_get(tag_index);

	if (TEST_FLAG(definition->flags, _widget_always_use_tag_controller_index_bit))
	{
		switch (definition->controller_index)
		{
		case _widget_controller0:
			local_player_index = PLAYER1;
			break;
		case _widget_controller1:
			local_player_index = PLAYER2;
			break;
		case _widget_controller2:
			local_player_index = PLAYER3;
			break;
		case _widget_controller3:
			local_player_index = PLAYER4;
			break;
		case _widget_controller_any:
			local_player_index = NONE;
			break;
		default:
			match_vhalt("c:\\halo\\SOURCE\\interface\\ui_widget.c", 5380, "invalid widget controller index specified");
		}
	}
	else
	{
		switch (definition->controller_index)
		{
		case _widget_controller0:
			local_player_index = PLAYER1;
			break;
		case _widget_controller1:
			local_player_index = PLAYER2;
			break;
		case _widget_controller2:
			local_player_index = PLAYER3;
			break;
		case _widget_controller3:
			local_player_index = PLAYER4;
			break;
		case _widget_controller_any:
			local_player_index = widget->local_player_index;
			break;
		default:
			match_vhalt("c:\\halo\\SOURCE\\interface\\ui_widget.c", 5392, "invalid widget controller index specified");
		}
	}

	topmost_parent = widget_instance_get_topmost_parent(widget);
	parent_tag_index = widget->parent ? widget->parent->tag_index : NONE;
	child_index = (short)widget_instance_get_child_index_from_parent(widget);
	result = ui_widget_load_by_name_or_tag(
		NULL,
		tag_index,
		NULL,
		local_player_index,
		topmost_parent->tag_index,
		parent_tag_index,
		child_index);

	if (!result)
	{
		error(_error_silent, "event handler failed to spawn widget");
	}

	return result;
}

static void perform_filesystem_initialization(
	void)
{
	match_assert("c:\\halo\\SOURCE\\interface\\ui_widget.c", 5439, widget_globals.initialization_thread==NULL);

	error(_error_silent, "begining filesystem checks & saved game file enumeration...");
	ui_widgets_inhibit_processing(TRUE);
	widget_globals.filesystem_check_result = _fscheck_result_no_error;

	if (!create_thread(0, filesystem_initialization_thread_proc, NULL, &widget_globals.initialization_thread))
	{
		error(_error_silent, "failed to spawn thread for filesystem checks - running synchronously!");
		widget_globals.initialization_thread = NULL;
		filesystem_initialization_thread_proc(NULL);
		ui_widgets_inhibit_processing(FALSE);
	}

	return;
}

static unsigned long __stdcall filesystem_initialization_thread_proc(
	void *parameter)
{
	widget_globals.filesystem_check_result = saved_game_perform_file_system_checks();

	if (widget_globals.filesystem_check_result == _fscheck_result_no_error)
	{
		long profile_index;
		word number_of_profiles = 1;

		playlist_profiles_enumerate_available_to_local_player_index(NONE, &number_of_profiles, &profile_index);
		player_profiles_enumerate_available_to_local_player_index(NONE, &number_of_profiles, &profile_index, TRUE);
		player_ui_get_player1_last_used_profile_index();
	}

	return 0;
}

/* ---------- private code */
