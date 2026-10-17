/*
HUD_MESSAGING.C
*/

/* ---------- headers */

#include "cseries.h"
#include "hud_messaging.h"
#include "meter_definitions.h"
#include "ui_widget.h"
#include "hud.h"
#include "hud_messaging_definitions.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "actor_definitions.h"
#include "input.h"
#include "bitmaps_inlines.h"
#include "game_state.h"
#include "network_messages.h"
#include "sound_manager.h"
#include "input_abstraction.h"
#include "text_group.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"
#include "render_debug.h"
#include "game_sound.h"
#include "bipeds.h"
#include "texture_cache.h"
#include "font_group.h"
#include "network_server_message_handler.h"
#include "network_server_manager.h"
#include "cinematics.h"
#include "equipment_definitions.h"

/* ---------- constants */

enum
{
	_hud_icon_remapped_action_button_end = 28 /* fake name */
};

/* ---------- structures */

struct hud_message
{
	long time;
	wchar_t msg[MAX_MESSAGE_LENGTH];
	boolean valid;
	byte magic_number;
	long item_definition_index;
	short quantity;
	char message_offset;
};

union state_info
{
	struct icon_hud_element_definition const *icon;
	struct
	{
		short string_index;
		boolean uses_scenario_names;
	} text;
};

struct hud_state_message
{
	wchar_t message_buffer[HUD_CUSTOM_MESSAGE_LENGTH + 1];
	union state_info text;
	union state_info info[MAX_STATE_TEXT_PANELS];
	struct hud_state_message_definition *state_message;
	boolean valid;
	byte is_text_flags;
	byte pad[2];
};

struct hud_messaging_datum
{
	struct hud_message messages[MAX_TEXT_MESSAGES];
	struct hud_state_message state_message;
	short first_message;
	boolean leave_first_line_blank;
	boolean custom_message;
};

struct hud_objective_message
{
	struct hud_state_message_definition *message;
	short uptime;
	short pad;
};

struct hud_timer_data_definition
{
	long reference_time;
	short ticks;
	short flash_cutoff;
	point2d position;
	short corner;
	boolean paused;
	boolean enabled;
};

struct hud_messaging_globals_definition
{
	struct hud_messaging_datum message_data[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	long flash_start_time;
	boolean use_flash;
	byte magic_number;
	byte pad[3];
	struct hud_state_message_definition *help_message;
	struct hud_objective_message objective;
	struct hud_timer_data_definition timer;
};

/* ---------- prototypes */

static void render_state_text(rectangle2d *bounds, rectangle2d *cursor_bounds, wchar_t const *text, boolean custom);
static void render_state_bitmap(rectangle2d *bounds, rectangle2d *cursor_bounds, unsigned long color, struct icon_hud_element_definition const *icon);
static struct hud_message *find_free_hud_message(struct hud_messaging_datum *datum, long item_definition_index, char message_offset);
static int __cdecl compare_messages(struct hud_message const *lhs, struct hud_message const *rhs);

/* ---------- globals */

struct hud_messaging_parameters_definition *hud_msg_def;

static char button_mappings[] = /* fake name */
{
	_button_action_reload,
	_button_throw_grenade,
	_button_fire,
	_button_flashlight,
	_button_jump,
	_button_melee_attack,
	_button_switch_weapon,
	_button_switch_grenade,
	_button_scope_zoom,
	_button_crouch,
	_button_jump,
	1,
	16,
	17
};

long time_code_time = NONE;
long time_code_stop_time = NONE;

static long split_screen_offset = 17; /* fake name */

static struct hud_messaging_globals_definition *hud_messaging_globals;

/* ---------- public code */

static void render_state_text(
	rectangle2d *bounds,
	rectangle2d *cursor_bounds,
	wchar_t const *text,
	boolean custom)
{
	rectangle2d message_bounds;

	draw_string_set_indents(cursor_bounds->x0 - bounds->x0, 0);
	draw_unicode_string_compute_bounds(bounds, text, &message_bounds, cursor_bounds);
	cursor_bounds->x0 -= 3;
	message_bounds.x0 = bounds->x0;

	if (custom && game_engine_running())
	{
		draw_string_and_hack_in_icons(&message_bounds, NULL, NULL, 0, text, TRUE);
	}
	else
	{
		rasterizer_draw_unicode_string(&message_bounds, NULL, NULL, 0, text);
	}

	bounds->y0 = cursor_bounds->y0;

	return;
}

static void render_state_bitmap(
	rectangle2d *bounds,
	rectangle2d *cursor_bounds,
	unsigned long color,
	struct icon_hud_element_definition const *icon)
{
	long bitmap_group_index = hud_globals->messaging.messaging_icons.index;
	struct bitmap_data const *bitmap = NULL;
	real_rectangle2d const *clip = NULL;
	short frame_index = 0;

	if (icon->frame_rate)
	{
		frame_index = (short)(game_time_get() / icon->frame_rate);
	}

	hud_retrieve_bitmap_and_bounding_rect(bitmap_group_index, icon->sequence_index, frame_index, &bitmap, &clip);

	if (bitmap && _texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
	{
		point2d corner;
		real scale = local_player_count() > 1 ? 0.75f : 1.f;

		corner.x = (short)(icon->offset.x * scale + cursor_bounds->x0);
		corner.y = (short)(cursor_bounds->y1 - icon->offset.y * scale);
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
			cursor_bounds->x0 = (short)(icon->width_offset * scale + corner.x);
		}
		else if (clip)
		{
			cursor_bounds->x0 = (short)(((clip->x1 - clip->x0) * bitmap->width + icon->width_offset) * scale + corner.x);
		}
		else
		{
			cursor_bounds->x0 = (short)((bitmap->width + icon->width_offset) * scale + corner.x);
		}
	}

	return;
}

void hud_messaging_initialize(
	void)
{
	hud_messaging_globals = game_state_malloc("hud messaging", NULL, sizeof(*hud_messaging_globals));

	return;
}

void hud_messaging_initialize_for_new_map(
	void)
{
	hud_msg_def = &hud_globals->messaging;
	memset(hud_messaging_globals, 0, sizeof(*hud_messaging_globals));

	return;
}

void hud_messaging_dispose_from_old_map(
	void)
{
	return;
}

void hud_messaging_dispose(
	void)
{
	return;
}

void scripted_hud_set_state_message(
	short message_index)
{
	struct scenario *scenario = global_scenario_get();

	if (hud_scripted_globals->show_hud_help_text && scenario->hud_messages.index != NONE)
	{
		struct hud_state_messages *messages = hud_state_messages_get(scenario->hud_messages.index);

		hud_messaging_globals->help_message = TAG_BLOCK_GET_ELEMENT(&messages->messages, message_index, struct hud_state_message_definition);
	}

	return;
}

void scripted_hud_set_flashing_state(
	boolean flash)
{
	if (flash && !hud_messaging_globals->use_flash)
	{
		hud_messaging_globals->flash_start_time = game_time_get();
	}

	hud_messaging_globals->use_flash = flash;

	return;
}

void scripted_hud_restart_flashing(
	void)
{
	if (hud_messaging_globals->use_flash)
	{
		hud_messaging_globals->flash_start_time = game_time_get();
	}
	else
	{
		error(_error_silent, "trying to restart help text flashing when flashing is disabled");
	}

	return;
}

void scripted_hud_set_objective(
	short message_index)
{
	struct scenario *scenario = global_scenario_get();

	if (scenario->hud_messages.index != NONE)
	{
		struct hud_state_messages *messages = hud_state_messages_get(scenario->hud_messages.index);
		struct hud_state_message_definition *message = TAG_BLOCK_GET_ELEMENT(&messages->messages, message_index, struct hud_state_message_definition);
		struct hud_state_message_element *element = TAG_BLOCK_GET_ELEMENT(&messages->elements, message->element_start_index, struct hud_state_message_element);

		if (message->element_count == 1 && element->type == _hud_message_type_text)
		{
			struct hud_color_definition *objective_color = &hud_globals->messaging.objective_color;

			hud_messaging_globals->objective.message = message;
			hud_messaging_globals->objective.uptime = objective_color->custom.objective.up_ticks + objective_color->custom.objective.fade_ticks;
		}
		else
		{
			error(_error_silent, "objective text MUST only be text, no icons");
		}
	}

	return;
}

void scripted_hud_set_timer_time(
	short minutes,
	short seconds)
{
	hud_messaging_globals->timer.ticks = (SECONDS_PER_MINUTE * minutes + seconds) * TICKS_PER_SECOND;
	hud_messaging_globals->timer.paused = FALSE;
	hud_messaging_globals->timer.enabled = TRUE;
	hud_messaging_globals->timer.reference_time = game_time_get();
	hud_messaging_globals->timer.corner = PIN(hud_messaging_globals->timer.corner, _hud_top_left, _hud_center);

	return;
}

void scripted_hud_set_timer_warning_cutoff(
	short minutes,
	short seconds)
{
	hud_messaging_globals->timer.flash_cutoff = (SECONDS_PER_MINUTE * minutes + seconds) * TICKS_PER_SECOND;

	return;
}

void scripted_hud_set_timer_position(
	short x,
	short y,
	short corner)
{
	hud_messaging_globals->timer.position.x = x;
	hud_messaging_globals->timer.position.y = y;
	hud_messaging_globals->timer.corner = PIN(corner, _hud_top_left, _hud_center);

	return;
}

void scripted_hud_show_timer(
	boolean enabled)
{
	hud_messaging_globals->timer.enabled = enabled;

	return;
}

void scripted_hud_pause_timer(
	boolean paused)
{
	struct hud_timer_data_definition *timer = &hud_messaging_globals->timer;

	timer->paused = paused;

	if (timer->ticks > 0)
	{
		if (paused)
		{
			timer->ticks += timer->reference_time - game_time_get();
		}
		else
		{
			timer->ticks += game_time_get() - timer->reference_time;
		}
	}

	return;
}

short scripted_hud_get_timer_ticks(
	void)
{
	struct hud_timer_data_definition *timer = &hud_messaging_globals->timer;
	short ticks = 0;

	if (timer->enabled)
	{
		if (timer->ticks == NONE)
		{
			ticks = NONE;
		}
		else if (timer->paused)
		{
			ticks = timer->ticks;
		}
		else
		{
			ticks = (short)(timer->ticks + timer->reference_time - game_time_get());
		}
	}

	return ticks;
}

void scripted_hud_time_code_show(
	boolean show)
{
	if (show)
	{
		time_code_time = game_time_get();
		time_code_stop_time = time_code_time;
	}
	else
	{
		time_code_time = NONE;
	}

	return;
}

void scripted_hud_time_code_start(
	boolean start)
{
	if (start)
	{
		time_code_time += game_time_get() - time_code_stop_time;
		time_code_stop_time = NONE;
	}
	else
	{
		time_code_stop_time = game_time_get();
	}

	return;
}

void scripted_hud_time_code_reset(
	void)
{
	long time = game_time_get();

	time_code_time = time;

	if (time_code_stop_time != NONE)
	{
		time_code_stop_time = time;
	}

	return;
}

void hud_render_timer(
	void)
{
	struct hud_timer_data_definition *timer = &hud_messaging_globals->timer;

	if (timer->enabled)
	{
		struct number_hud_element_definition numbers;
		long hud_digits_index;
		short seconds;
		struct hud_absolute_placement_definition absolute_placement = { timer->corner };
		long time = game_time_get();
		short draw_flags = 0;
		short digit_width = 0;
		short ticks = scripted_hud_get_timer_ticks();

		numbers.digits = 2;
		numbers.fractional_digits = 4;
		numbers.number_flags = FLAG(_hud_number_show_all_leading_zeros_bit);
		numbers.placement.offset = timer->position;
		numbers.placement.scale.i = 1.f;
		numbers.placement.scale.j = 1.f;
		hud_digits_index = interface_get_tag_index(_interface_hud_digits);

		if (hud_digits_index != NONE)
		{
			digit_width = (short)(hud_number_definition_get(hud_digits_index)->screen_width * 2.f);
		}

		switch (timer->corner)
		{
		case _hud_top_left:
		case _hud_bottom_left:
			break;
		case _hud_top_right:
		case _hud_bottom_right:
			numbers.placement.offset.x += 5 * digit_width;
			digit_width = -digit_width;
			break;
		case _hud_center:
			numbers.placement.offset.x -= 3 * digit_width;
			break;
		default:
			match_unreachable("c:\\halo\\SOURCE\\interface\\hud_messaging.c", 504);
			break;
		}

		if (ticks > 0)
		{
			numbers.colors = hud_globals->timer_definition.color;

			if (ticks <= timer->flash_cutoff)
			{
				draw_flags = FLAG(_hud_draw_flashing_bit);

				if (timer->ticks > timer->flash_cutoff)
				{
					timer->ticks = timer->flash_cutoff;
					timer->reference_time = timer->flash_cutoff - ticks + time;
				}
			}
		}
		else
		{
			timer->ticks = NONE;
			numbers.colors = hud_globals->timer_definition.time_up_color;
			draw_flags = FLAG(_hud_draw_flashing_bit);

			if (timer->reference_time == NONE)
			{
				timer->reference_time = game_time_get();
			}
		}

		ticks = MAX(ticks, 0);
		seconds = ticks / TICKS_PER_SECOND;
		hud_draw_numbers(
			render.local_player_index,
			&absolute_placement,
			&numbers,
			seconds / SECONDS_PER_MINUTE,
			NONE,
			draw_flags,
			timer->reference_time,
			2.f);
		numbers.placement.offset.x += digit_width * 2.5;
		hud_draw_numbers(
			render.local_player_index,
			&absolute_placement,
			&numbers,
			seconds % SECONDS_PER_MINUTE,
			NONE,
			draw_flags,
			timer->reference_time,
			2.f);
		numbers.placement.offset.x += digit_width * 2.5;
		hud_draw_numbers(
			render.local_player_index,
			&absolute_placement,
			&numbers,
			100 * (ticks % TICKS_PER_SECOND) / TICKS_PER_SECOND,
			NONE,
			draw_flags,
			timer->reference_time,
			2.f);
	}

	if (time_code_time != NONE)
	{
		sprintf(temporary, "%d", (time_code_stop_time == NONE ? game_time_get() : time_code_stop_time) - time_code_time);
		render_debug_string(TRUE, temporary);
	}

	return;
}

void hud_set_state_message(
	short local_player_index,
	short message_index)
{
	if (!hud_scripted_globals->show_hud_help_text)
	{
		long hud_messages_index = hud_globals->messaging.hud_messages.index;

		if (hud_messages_index != NONE)
		{
			struct hud_messaging_datum *datum = &hud_messaging_globals->message_data[local_player_index];

			if (message_index != NONE)
			{
				struct hud_state_messages *messages = hud_state_messages_get(hud_messages_index);

				if (message_index < messages->messages.count)
				{
					datum->state_message.state_message = TAG_BLOCK_GET_ELEMENT(&messages->messages, message_index, struct hud_state_message_definition);
					datum->state_message.is_text_flags = 0;
				}
				else
				{
					message_index = NONE;
				}
			}

			datum->state_message.valid = message_index != NONE;
		}
	}

	return;
}

void hud_set_state_message_icon(
	short local_player_index,
	short custom_icon_index,
	struct icon_hud_element_definition const *icon)
{
	struct hud_messaging_datum *datum = &hud_messaging_globals->message_data[local_player_index];

	if (datum->state_message.valid && !hud_scripted_globals->show_hud_help_text && datum->state_message.state_message)
	{
		datum->state_message.info[custom_icon_index].icon = icon;
		SET_FLAG(datum->state_message.is_text_flags, custom_icon_index, FALSE);
	}

	return;
}

void hud_set_state_message_text(
	short local_player_index,
	short custom_icon_index,
	short icon_string_index,
	boolean uses_scenario_names)
{
	struct hud_messaging_datum *datum = &hud_messaging_globals->message_data[local_player_index];

	if (datum->state_message.valid && !hud_scripted_globals->show_hud_help_text && datum->state_message.state_message)
	{
		datum->state_message.info[custom_icon_index].text.string_index = icon_string_index;
		datum->state_message.info[custom_icon_index].text.uses_scenario_names = uses_scenario_names;
		SET_FLAG(datum->state_message.is_text_flags, custom_icon_index, TRUE);
	}

	return;
}

void hud_enable_custom_state_message(
	short local_player_index,
	boolean enabled)
{
	struct hud_messaging_datum *datum = &hud_messaging_globals->message_data[local_player_index];

	datum->leave_first_line_blank |= datum->state_message.valid != enabled;
	datum->state_message.valid = enabled;
	datum->state_message.state_message = NULL;

	if (enabled)
	{
		datum->state_message.state_message = NULL;
		ustrncpy(datum->state_message.message_buffer, L"", HUD_CUSTOM_MESSAGE_LENGTH);
	}

	datum->custom_message = enabled;

	return;
}

void hud_set_state_text(
	short local_player_index,
	wchar_t const *message)
{
	struct hud_messaging_datum *datum = &hud_messaging_globals->message_data[local_player_index];

	ustrncpy(datum->state_message.message_buffer, message, HUD_CUSTOM_MESSAGE_LENGTH);
	datum->state_message.message_buffer[HUD_CUSTOM_MESSAGE_LENGTH] = 0;

	return;
}

wchar_t const *hud_messaging_get_objective(
	void)
{
	wchar_t const *result = NULL;

	if (hud_messaging_globals->objective.message)
	{
		struct hud_state_messages *messages = hud_state_messages_get(global_scenario_get()->hud_messages.index);
		struct hud_state_message_definition *message = hud_messaging_globals->objective.message;
		struct hud_state_message_element *element = TAG_BLOCK_GET_ELEMENT(&messages->elements, message->element_start_index, struct hud_state_message_element);

		match_assert("c:\\halo\\SOURCE\\interface\\hud_messaging.c", 674, message->element_count==1);
		match_assert("c:\\halo\\SOURCE\\interface\\hud_messaging.c", 675, element->type==_hud_message_type_text);
		result = tag_data_get_pointer(&messages->text_data, message->text_start_index * sizeof(wchar_t), element->data * sizeof(wchar_t));
	}

	return result;
}

void hud_print_message(
	short local_player_index,
	wchar_t const *text)
{
	if (local_player_index != NONE)
	{
		struct hud_messaging_datum *datum = &hud_messaging_globals->message_data[local_player_index];
		struct hud_message *message = find_free_hud_message(datum, NONE, 0);

		ustrncpy(message->msg, text, MAX_MESSAGE_LENGTH);
		message->item_definition_index = NONE;
		message->time = game_time_get();
		message->valid = TRUE;
		message->magic_number = hud_messaging_globals->magic_number++;
		datum->leave_first_line_blank = FALSE;
	}

	return;
}

void hud_add_item_message(
	short local_player_index,
	long item_definition_index,
	short quantity,
	char message_offset)
{
	if (local_player_index != NONE)
	{
		struct hud_messaging_datum *datum = &hud_messaging_globals->message_data[local_player_index];
		struct hud_message *message = find_free_hud_message(datum, item_definition_index, message_offset);

		if (!message->valid)
		{
			message->quantity = 0;
		}

		message->quantity += quantity;
		message->item_definition_index = item_definition_index;
		message->message_offset = message_offset;
		message->time = game_time_get();
		message->valid = TRUE;
		message->magic_number = hud_messaging_globals->magic_number++;
		datum->leave_first_line_blank = FALSE;
	}

	return;
}

static struct hud_message *find_free_hud_message(
	struct hud_messaging_datum *datum,
	long item_definition_index,
	char message_offset)
{
	short message_index;
	struct hud_message *result = NULL;
	long oldest_time = LONG_MAX;
	short oldest_message_index = 0;

	for (message_index = 0; message_index < NUMBEROF(datum->messages); message_index++)
	{
		struct hud_message *message = &datum->messages[message_index];

		if ((item_definition_index != NONE && item_definition_index == message->item_definition_index && message_offset == message->message_offset) || !message->valid)
		{
			result = message;

			if (item_definition_index == NONE || item_definition_index == message->item_definition_index)
			{
				break;
			}
		}
		else if (oldest_time > message->time)
		{
			oldest_time = message->time;
			oldest_message_index = message_index;
		}
	}

	if (!result)
	{
		result = &datum->messages[oldest_message_index];
	}

	return result;
}

static int __cdecl compare_messages(
	struct hud_message const *lhs,
	struct hud_message const *rhs)
{
	int result = rhs->time - lhs->time;

	if (!result)
	{
		result = rhs->item_definition_index - lhs->item_definition_index;

		if (!result)
		{
			result = rhs->magic_number - lhs->magic_number;
		}
	}

	return result;
}

void hud_broadcast_team_message(
	long victim_player_index,
	wchar_t const *message)
{
	if (game_engine_running())
	{
		short local_player_index;

		for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
		{
			long player_index = local_player_get_player_index(local_player_index);

			if (player_index != NONE && player_get(player_index)->team_index == player_get(victim_player_index)->team_index)
			{
				hud_print_message(local_player_index, message);
			}
		}
	}

	return;
}

void scripted_hud_messages_clear(
	void)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		short message_index;

		for (message_index = 0; message_index < MAX_TEXT_MESSAGES; message_index++)
		{
			hud_messaging_globals->message_data[local_player_index].messages[message_index].valid = FALSE;
		}
	}

	return;
}

long hud_get_font_index(
	void)
{
	return local_player_count() > 1 && hud_msg_def->multi_player_font.index != NONE ? hud_msg_def->multi_player_font.index : hud_msg_def->single_player_font.index;
}

real_argb_color *hud_get_text_color(
	real_argb_color *result)
{
	*result = hud_msg_def->state_color;

	return result;
}

void hud_messaging_globals_update(
	void)
{
	hud_messaging_globals->magic_number = 0;

	return;
}

void hud_messaging_update(
	short local_player_index)
{
	if (!cinematic_in_progress() && local_player_index != NONE && game_engine_hud_draw_messages(local_player_get_player_index(local_player_index)))
	{
		short line_height;
		point2d upper_left;
		short offset;
		struct font_header *font;
		struct hud_messaging_datum *datum;
		short line_max;
		boolean show_objective;
		boolean show_help_text;
		boolean show_state_message;
		short message_index;
		long font_index = hud_get_font_index();
		boolean split_screen = local_player_count() > 1;

		hud_calculate_point(local_player_index, &hud_msg_def->absolute_placement, &hud_msg_def->placement, NULL, local_player_count() > 1, 0.f, &upper_left);
		offset = upper_left.y;
		font = font_definition_get(font_index);

		if (split_screen)
		{
			line_height = font->leading_height + font->ascending_height;
			offset -= split_screen_offset;
		}
		else
		{
			line_height = font->leading_height + font->descending_height + font->ascending_height;
		}

		datum = &hud_messaging_globals->message_data[render.local_player_index];
		line_max = MAX_TEXT_MESSAGES - (local_player_count() > 1);
		show_objective = hud_messaging_globals->objective.message && hud_messaging_globals->objective.uptime;
		show_help_text = hud_scripted_globals->show_hud_help_text && hud_messaging_globals->help_message;
		show_state_message = datum->state_message.valid && (datum->state_message.state_message || datum->state_message.message_buffer[0]);

		if (show_objective || show_help_text || show_state_message)
		{
			struct game_input_preferences input_preferences;
			real_argb_color color;
			unsigned long pixel_color;
			rectangle2d bounds;
			rectangle2d cursor_bounds;
			struct hud_state_message *state_message = &datum->state_message;

			input_abstraction_get_local_player_preferences(local_player_index, &input_preferences);

			if (show_objective)
			{
				struct hud_color_definition *objective_color = &hud_globals->messaging.objective_color;
				long reference_time = game_time_get() + hud_messaging_globals->objective.uptime - objective_color->custom.objective.up_ticks - objective_color->custom.objective.fade_ticks;

				pixel32_to_real_argb_color(get_flash_color(objective_color, reference_time), &color);
				color.alpha *= MIN((real)hud_messaging_globals->objective.uptime / objective_color->custom.objective.fade_ticks, 1.f);
				pixel_color = real_argb_color_to_pixel32(&color);
			}
			else if (show_help_text)
			{
				if (hud_messaging_globals->use_flash)
				{
					pixel_color = get_flash_color(&hud_globals->messaging.color, hud_messaging_globals->flash_start_time);
				}
				else if (!TEST_FLAG(hud_globals->messaging.color.flash_flags, _hud_flash_reverse_colors_bit))
				{
					pixel_color = hud_globals->messaging.color.color;
				}
				else
				{
					pixel_color = hud_globals->messaging.color.flash_color;
				}

				pixel32_to_real_argb_color(pixel_color, &color);
			}
			else
			{
				color = hud_msg_def->state_color;
				pixel_color = real_argb_color_to_pixel32(&color);
			}

			bounds.x0 = upper_left.x;
			bounds.x1 = render.camera.window_bounds.x1 - render.camera.viewport_bounds.x0;
			bounds.y0 = offset;
			bounds.y1 = offset + 5 * line_height;
			cursor_bounds = bounds;
			draw_string_set_draw_mode(font_index, _text_style_plain, _text_justification_left, 0, &color);

			if (!show_objective && !show_help_text && !datum->state_message.state_message)
			{
				if (state_message->message_buffer[0])
				{
					render_state_text(&bounds, &cursor_bounds, state_message->message_buffer, TRUE);
				}
			}
			else
			{
				struct hud_state_messages *messages;
				struct hud_state_message_definition *message;
				word text_index;
				short element_index;

				if (show_objective)
				{
					match_assert("c:\\halo\\SOURCE\\interface\\hud_messaging.c", 1056, hud_messaging_globals->objective.message && hud_messaging_globals->objective.uptime);
					hud_messaging_globals->objective.uptime = MAX(hud_messaging_globals->objective.uptime - game_time_get_elapsed(), 0);
					messages = hud_state_messages_get(global_scenario_get()->hud_messages.index);
					message = hud_messaging_globals->objective.message;
				}
				else if (show_help_text)
				{
					messages = hud_state_messages_get(global_scenario_get()->hud_messages.index);
					message = hud_messaging_globals->help_message;
				}
				else
				{
					match_assert("c:\\halo\\SOURCE\\interface\\hud_messaging.c", 1068, show_state_message);
					messages = hud_state_messages_get(hud_globals->messaging.hud_messages.index);
					message = datum->state_message.state_message;
				}

				text_index = message->text_start_index;

				for (element_index = 0; element_index < message->element_count; element_index++)
				{
					struct hud_state_message_element *element = TAG_BLOCK_GET_ELEMENT(&messages->elements, message->element_start_index + element_index, struct hud_state_message_element);

					switch (element->type)
					{
					case _hud_message_type_text:
						render_state_text(
							&bounds,
							&cursor_bounds,
							tag_data_get_pointer(&messages->text_data, text_index * sizeof(wchar_t), element->data * sizeof(wchar_t)),
							FALSE);
						text_index += element->data;
						break;
					case _hud_message_type_icon:
					{
						short icon_index = NONE;

						if (element->data > _hud_icon_specific_button_end)
						{
							if (element->data <= _hud_icon_remapped_button_end)
							{
								if (element->data <= _hud_icon_remapped_action_button_end)
								{
									icon_index = input_preferences.game_control_to_xbox_buttons[button_mappings[element->data - _hud_icon_remapped_button_start]];
								}
								else
								{
									icon_index = button_mappings[element->data - _hud_icon_remapped_button_start];
								}
							}
							else if (!hud_scripted_globals->show_hud_help_text)
							{
								short custom_index = element->data - _hud_icon_misc_start;

								match_assert("c:\\halo\\SOURCE\\interface\\hud_messaging.c", 1111, custom_index<NUMBER_OF_HUD_CUSTOM_ICONS);

								if (TEST_FLAG(state_message->is_text_flags, custom_index))
								{
									short string_index = state_message->info[custom_index].text.string_index;

									if (string_index != NONE)
									{
										if (state_message->info[custom_index].text.uses_scenario_names)
										{
											match_assert("c:\\halo\\SOURCE\\interface\\hud_messaging.c", 1120, global_scenario_get()->custom_object_names.index!=NONE);
											render_state_text(
												&bounds,
												&cursor_bounds,
												unicode_string_list_get_string(global_scenario_get()->custom_object_names.index, string_index),
												FALSE);
										}
										else
										{
											render_state_text(
												&bounds,
												&cursor_bounds,
												unicode_string_list_get_string(hud_globals->messaging.alternate_icon_text.index, string_index),
												FALSE);
										}
									}
									else
									{
										render_state_text(&bounds, &cursor_bounds, L"<unknown>", FALSE);
									}
								}
								else
								{
									struct icon_hud_element_definition const *icon = state_message->info[custom_index].icon;

									if (icon)
									{
										render_state_bitmap(&bounds, &cursor_bounds, pixel_color, icon);
									}
									else
									{
										error(_error_silent, "help message using old code. get latest code and tags.");
									}
								}
								break;
							}
							else
							{
								error(_error_silent, "help text cannot use custom icons");
							}
						}
						else
						{
							icon_index = element->data;
						}

						if (icon_index < hud_globals->messaging.button_icons.count)
						{
							struct icon_hud_element_definition *icon;

							icon_index = remap_sticks_for_local_player(icon_index, local_player_index);
							icon = TAG_BLOCK_GET_ELEMENT(&hud_globals->messaging.button_icons, icon_index, struct icon_hud_element_definition);

							if (TEST_FLAG(icon->flags, _hud_icon_use_text_bit))
							{
								if (TEST_FLAG(icon->flags, _hud_icon_use_color_bit))
								{
									real_argb_color custom_color;

									pixel32_to_real_argb_color(pixel_color, &custom_color);
									draw_string_set_draw_mode(font_index, _text_style_plain, _text_justification_left, 0, &custom_color);
								}

								{
									rectangle2d text_bounds;
									wchar_t const *text = unicode_string_list_get_string(hud_globals->messaging.alternate_icon_text.index, icon->text_index);

									draw_string_set_indents(cursor_bounds.x0 - bounds.x0, 0);
									draw_unicode_string_compute_bounds(&bounds, text, &text_bounds, &cursor_bounds);
									cursor_bounds.x0 -= 3;
									text_bounds.x0 = bounds.x0;
									rasterizer_draw_unicode_string(&text_bounds, NULL, NULL, 0, text);
									bounds.y0 = cursor_bounds.y0;
								}

								draw_string_set_draw_mode(font_index, _text_style_plain, _text_justification_left, 0, &color);
							}
							else
							{
								render_state_bitmap(&bounds, &cursor_bounds, pixel_color, icon);
							}
						}
						else
						{
							rectangle2d text_bounds;

							draw_string_set_indents(cursor_bounds.x0 - bounds.x0, 0);
							draw_unicode_string_compute_bounds(&bounds, L"<no button icon>", &text_bounds, &cursor_bounds);
							cursor_bounds.x0 -= 3;
							text_bounds.x0 = bounds.x0;
							rasterizer_draw_unicode_string(&text_bounds, NULL, NULL, 0, L"<no button icon>");
							bounds.y0 = cursor_bounds.y0;
						}
						break;
					}
					default:
						match_unreachable("c:\\halo\\SOURCE\\interface\\hud_messaging.c", 1191);
						break;
					}
				}
			}

			draw_string_set_indents(0, 0);
			offset = cursor_bounds.y1;
		}

		if (!show_objective && !show_help_text && (datum->state_message.valid || datum->leave_first_line_blank))
		{
			if (split_screen)
			{
				offset = (short)(upper_left.y - split_screen_offset + line_height * hud_msg_def->spacing);
			}
			else
			{
				offset = (short)(upper_left.y + line_height * hud_msg_def->spacing);
			}

			line_max--;
		}

		qsort(datum->messages, NUMBEROF(datum->messages), sizeof(datum->messages[0]), (int (__cdecl *)(void const *, void const *))compare_messages);

		for (message_index = 0; message_index < line_max; message_index++)
		{
			long time;
			real_argb_color text_color;
			rectangle2d bounds;
			struct hud_message *message = &datum->messages[message_index];

			if (!message->valid)
			{
				break;
			}

			time = game_time_get();
			text_color = hud_msg_def->text_color;

			if ((real)(time - message->time) > hud_msg_def->up_time * TICKS_PER_SECOND)
			{
				real fade = PIN(1.f - ((real)(time - message->time) - hud_msg_def->up_time * TICKS_PER_SECOND) / (hud_msg_def->fade_time * TICKS_PER_SECOND), 0.f, 1.f);

				text_color.alpha *= (real)pow(fade, 1.9f);
			}

			bounds = render.camera.window_bounds;
			bounds.x0 = upper_left.x;
			bounds.x1 = render.camera.window_bounds.x1 - render.camera.viewport_bounds.x0;
			bounds.y0 = offset;
			bounds.y1 = offset + line_height;
			offset = (short)(line_height * hud_msg_def->spacing + offset);
			draw_string_set_draw_mode(font_index, _text_style_plain, _text_justification_left, 0, &text_color);

			if (message->item_definition_index != NONE)
			{
				char message_offset = message->message_offset == NONE ? message->quantity > 1 : message->message_offset;
				struct item_definition *item_definition = item_definition_get(message->item_definition_index);
				short string_index = item_definition->item.hud_message_index + message_offset;
				wchar_t const *string = hud_get_item_string(string_index);

				if ((message->message_offset == NONE && message_offset) || message->quantity)
				{
					wchar_t buffer[256];
					short value_scale = MAX(item_definition->item.hud_message_value_scale, 1);

					usprintf(buffer, string, message->quantity / value_scale);
					rasterizer_draw_unicode_string(&bounds, NULL, NULL, 0, buffer);
				}
				else
				{
					rasterizer_draw_unicode_string(&bounds, NULL, NULL, 0, string);
				}
			}
			else
			{
				rasterizer_draw_unicode_string(&bounds, NULL, NULL, 0, message->msg);
			}

			message->valid = (real)(time - message->time) < (hud_msg_def->fade_time + hud_msg_def->up_time) * TICKS_PER_SECOND;

			if (!message->valid)
			{
				message->time = NONE;
			}
		}
	}

	return;
}
