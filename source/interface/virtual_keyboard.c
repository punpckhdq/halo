/*
VIRTUAL_KEYBOARD.C
*/

/* ---------- headers */

#include "cseries.h"
#include "virtual_keyboard.h"
#include "ui_widget.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "input.h"
#include "text_group.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "font_group.h"
#include "event_manager.h"
#include "unicode.h"

/* ---------- constants */

enum
{
	_vkey_1 = 0,
	_vkey_2,
	_vkey_3,
	_vkey_4,
	_vkey_5,
	_vkey_6,
	_vkey_7,
	_vkey_8,
	_vkey_9,
	_vkey_0,
	_vkey_a,
	_vkey_b,
	_vkey_c,
	_vkey_d,
	_vkey_e,
	_vkey_f,
	_vkey_g,
	_vkey_h,
	_vkey_i,
	_vkey_j,
	_vkey_k,
	_vkey_l,
	_vkey_m,
	_vkey_n,
	_vkey_o,
	_vkey_p,
	_vkey_q,
	_vkey_r,
	_vkey_s,
	_vkey_t,
	_vkey_u,
	_vkey_v,
	_vkey_w,
	_vkey_x,
	_vkey_y,
	_vkey_z,
	NUMBER_OF_CONFIGURABLE_VIRTUAL_KEYS,

	_vkey_done = NUMBER_OF_CONFIGURABLE_VIRTUAL_KEYS,
	_vkey_shift,
	_vkey_caps,
	_vkey_symbols,
	_vkey_backspace,
	_vkey_left,
	_vkey_right,
	_vkey_space,
	NUMBER_OF_VIRTUAL_KEYS
};

enum
{
	_event_tab_left = 0,
	_event_tab_right,
	_event_tab_up,
	_event_tab_down,
	_event_key_select,
	_event_cancel,
	NUMBER_OF_VIRTUAL_KEYBOARD_EVENTS
};

enum
{
	_virtual_keyboard_caption_player_profile_name = 8,
	_virtual_keyboard_caption_variant_name,
	_virtual_keyboard_caption_edited_default_variant_name,
	NUMBER_OF_VIRTUAL_KEYBOARD_STRINGS,

	FIRST_VIRTUAL_KEYBOARD_CAPTION_STRING_INDEX = _virtual_keyboard_caption_player_profile_name
};

enum
{
	FIRST_VIRTUAL_KEYBOARD_FORTUNE_STRING_INDEX = 11,	/* fake name */
	NUMBER_OF_VIRTUAL_KEYBOARD_FORTUNES = 10,			/* fake name */
	VIRTUAL_KEYBOARD_TAB_REPEAT_MILLISECONDS = 250,		/* fake name */
	MINIMUM_FREE_SPACE_FOR_CHARACTER = sizeof(wchar_t)	/* fake name */
};

enum
{
	RENDER_AS_NULL_CHARACTER = 0x7F,
	NUMBER_OF_VIRTUAL_KEYBOARD_ROWS = 5,
	NUMBER_OF_VIRTUAL_KEYBOARD_COLUMNS = 11,
	MAXIMUM_KEY_LABEL_STRING_LENGTH = 24,
	MAXIMUM_VIRTUAL_KEYBOARD_EDIT_TEXT_FIELD_CHARACTERS = 32,
	DISPLAY_WIDTH = 640,
	DISPLAY_HEIGHT = 480,
	KEY_HEIGHT = 32,
	KEY_WIDTH = 32,
	EDIT_TEXT_FIELD_TOP = 118,
	EDIT_TEXT_FIELD_HEIGHT = 25,
	EDIT_TEXT_FIELD_WIDTH = 200,
	EDIT_TEXT_FIELD_LEFT = 220,
	EDIT_TEXT_CARET_TOP = 120,	/* fake name */
	CAPTION_TEXT_FIELD_LEFT = 114,
	CAPTION_TEXT_FIELD_RIGHT = 640,
	CAPTION_TEXT_FIELD_TOP = 78,
	CAPTION_TEXT_FIELD_BOTTOM = 110,
	KEYS_HOFFSET = 112,
	KEYS_VOFFSET = 158,
	KEY_LABEL_TEXT_VOFFSET = 5,
	KEY_LABEL_TEXT_HOFFSET = 2,
	EDIT_TEXT_CARET_WIDTH = 1
};

/* ---------- macros */

#define VIRTUAL_KEYBOARD_TAG_NAME "ui\\english"								/* fake name */
#define VIRTUAL_KEYBOARD_CARET_BITMAP_TAG_NAME "ui\\shell\\bitmaps\\white"	/* fake name */
#define VIRTUAL_KEYBOARD_FORTUNE_TEXT L".fortune"							/* fake name */

#define KEY_RECTANGLE(row, column, width) {\
	KEYS_VOFFSET + (row) * KEY_HEIGHT,\
	KEYS_HOFFSET + (column) * KEY_WIDTH,\
	KEYS_VOFFSET + ((row) + 1) * KEY_HEIGHT,\
	KEYS_HOFFSET + ((column) + (width)) * KEY_WIDTH }	/* fake name */

/* ---------- structures */

struct _virtual_keyboard_globals
{
	boolean active;
	boolean shift_active;
	boolean caps_active;
	boolean symbols_active;
	struct virtual_keyboard *keyboard;
	short row;
	short column;
	word buffer_size;
	short last_event;
	short last_key;
	word number_of_event_repeats;
	short caption_index;
	boolean buffer_saved;
	boolean first_key_replaces_buffer;
	wchar_t *text_buffer;
	wchar_t *insertion_point;
	unsigned long time_of_last_event;
	long caret_bitmap_tag;
	wchar_t original_buffer[MAXIMUM_VIRTUAL_KEYBOARD_EDIT_TEXT_FIELD_CHARACTERS];
};

/* ---------- prototypes */

static boolean virtual_keyboard_tab_left(void);
static boolean virtual_keyboard_tab_right(void);
static boolean virtual_keyboard_tab_up(void);
static boolean virtual_keyboard_tab_down(void);
static boolean virtual_keyboard_cancel(void);
static void virtual_keyboard_process_internal(void);
static boolean virtual_keyboard_select(void);
static wchar_t virtual_keyboard_get_character(word keycode);
static wchar_t virtual_keyboard_get_current_character(void);
static void virtual_keyboard_render_internal(void);
static long virtual_keyboard_free_space_in_text_buffer(void);
static void virtual_keyboard_backspace(void);

/* ---------- globals */

static char const virtual_keyboard_layout_table[NUMBER_OF_VIRTUAL_KEYBOARD_ROWS][NUMBER_OF_VIRTUAL_KEYBOARD_COLUMNS] =
{
	{ _vkey_done, _vkey_1, _vkey_2, _vkey_3, _vkey_4, _vkey_5, _vkey_6, _vkey_7, _vkey_8, _vkey_9, _vkey_0 },
	{ _vkey_shift, _vkey_a, _vkey_b, _vkey_c, _vkey_d, _vkey_e, _vkey_f, _vkey_g, _vkey_h, _vkey_i, _vkey_j },
	{ _vkey_caps, _vkey_k, _vkey_l, _vkey_m, _vkey_n, _vkey_o, _vkey_p, _vkey_q, _vkey_r, _vkey_s, _vkey_t },
	{ _vkey_symbols, _vkey_u, _vkey_v, _vkey_w, _vkey_x, _vkey_y, _vkey_z, _vkey_backspace, _vkey_backspace, _vkey_backspace, _vkey_backspace },
	{ _vkey_space, _vkey_space, _vkey_space, _vkey_space, _vkey_space, _vkey_space, _vkey_space, _vkey_left, _vkey_left, _vkey_right, _vkey_right }
};

static rectangle2d keyboard_rect[NUMBER_OF_VIRTUAL_KEYS] =
{
	KEY_RECTANGLE(0, 3, 1),
	KEY_RECTANGLE(0, 4, 1),
	KEY_RECTANGLE(0, 5, 1),
	KEY_RECTANGLE(0, 6, 1),
	KEY_RECTANGLE(0, 7, 1),
	KEY_RECTANGLE(0, 8, 1),
	KEY_RECTANGLE(0, 9, 1),
	KEY_RECTANGLE(0, 10, 1),
	KEY_RECTANGLE(0, 11, 1),
	KEY_RECTANGLE(0, 12, 1),
	KEY_RECTANGLE(1, 3, 1),
	KEY_RECTANGLE(1, 4, 1),
	KEY_RECTANGLE(1, 5, 1),
	KEY_RECTANGLE(1, 6, 1),
	KEY_RECTANGLE(1, 7, 1),
	KEY_RECTANGLE(1, 8, 1),
	KEY_RECTANGLE(1, 9, 1),
	KEY_RECTANGLE(1, 10, 1),
	KEY_RECTANGLE(1, 11, 1),
	KEY_RECTANGLE(1, 12, 1),
	KEY_RECTANGLE(2, 3, 1),
	KEY_RECTANGLE(2, 4, 1),
	KEY_RECTANGLE(2, 5, 1),
	KEY_RECTANGLE(2, 6, 1),
	KEY_RECTANGLE(2, 7, 1),
	KEY_RECTANGLE(2, 8, 1),
	KEY_RECTANGLE(2, 9, 1),
	KEY_RECTANGLE(2, 10, 1),
	KEY_RECTANGLE(2, 11, 1),
	KEY_RECTANGLE(2, 12, 1),
	KEY_RECTANGLE(3, 3, 1),
	KEY_RECTANGLE(3, 4, 1),
	KEY_RECTANGLE(3, 5, 1),
	KEY_RECTANGLE(3, 6, 1),
	KEY_RECTANGLE(3, 7, 1),
	KEY_RECTANGLE(3, 8, 1),
	KEY_RECTANGLE(0, 0, 3),
	KEY_RECTANGLE(1, 0, 3),
	KEY_RECTANGLE(2, 0, 3),
	KEY_RECTANGLE(3, 0, 3),
	KEY_RECTANGLE(3, 9, 4),
	KEY_RECTANGLE(4, 9, 2),
	KEY_RECTANGLE(4, 11, 2),
	KEY_RECTANGLE(4, 3, 6)
};

static struct _virtual_keyboard_globals virtual_keyboard_globals;

/* ---------- public code */

boolean virtual_keyboard_initialize(
	void)
{
	long keyboard_tag_index;

	virtual_keyboard_globals.active = FALSE;
	virtual_keyboard_globals.shift_active = FALSE;
	virtual_keyboard_globals.caps_active = FALSE;
	virtual_keyboard_globals.symbols_active = FALSE;
	keyboard_tag_index = tag_loaded(VIRTUAL_KEYBOARD_TAG, VIRTUAL_KEYBOARD_TAG_NAME);

	if (keyboard_tag_index != NONE)
	{
		virtual_keyboard_globals.keyboard = virtual_keyboard_definition_get(keyboard_tag_index);
		match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 364, virtual_keyboard_globals.keyboard);

		virtual_keyboard_globals.row = 0;
		virtual_keyboard_globals.column = 0;
		virtual_keyboard_globals.buffer_size = 0;
		virtual_keyboard_globals.last_event = NONE;
		virtual_keyboard_globals.last_key = NONE;
		virtual_keyboard_globals.number_of_event_repeats = 0;
		virtual_keyboard_globals.text_buffer = NULL;
		virtual_keyboard_globals.insertion_point = NULL;
		virtual_keyboard_globals.time_of_last_event = 0;
	}
	else
	{
		error(_error_silent, "failed to load virtual keyboard for '%s' language", "<unknown>");
	}

	virtual_keyboard_globals.caret_bitmap_tag = tag_loaded(BITMAP_GROUP_TAG, VIRTUAL_KEYBOARD_CARET_BITMAP_TAG_NAME);

	if (virtual_keyboard_globals.caret_bitmap_tag == NONE)
	{
		error(_error_silent, "failed to load virtual keyboard caret bitmap '%s'", VIRTUAL_KEYBOARD_CARET_BITMAP_TAG_NAME);
	}

	return virtual_keyboard_globals.keyboard != NULL;
}

void virtual_keyboard_dispose(
	void)
{
	virtual_keyboard_globals.active = FALSE;
	virtual_keyboard_globals.shift_active = FALSE;
	virtual_keyboard_globals.caps_active = FALSE;
	virtual_keyboard_globals.symbols_active = FALSE;
	virtual_keyboard_globals.keyboard = NULL;
	virtual_keyboard_globals.row = 0;
	virtual_keyboard_globals.column = 0;
	virtual_keyboard_globals.buffer_size = 0;
	virtual_keyboard_globals.last_event = NONE;
	virtual_keyboard_globals.last_key = NONE;
	virtual_keyboard_globals.number_of_event_repeats = 0;
	virtual_keyboard_globals.text_buffer = NULL;
	virtual_keyboard_globals.insertion_point = NULL;
	virtual_keyboard_globals.time_of_last_event = 0;
	event_manager_flush();

	return;
}

boolean virtual_keyboard_launch(
	wchar_t *text_buffer,
	word buffer_size,
	short caption_index)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 418, text_buffer && buffer_size && !(buffer_size&1) && !virtual_keyboard_globals.active);
	match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 419, (caption_index>=FIRST_VIRTUAL_KEYBOARD_CAPTION_STRING_INDEX) && (caption_index<NUMBER_OF_VIRTUAL_KEYBOARD_STRINGS));

	if (!virtual_keyboard_globals.active && virtual_keyboard_globals.keyboard)
	{
		event_manager_flush();
		virtual_keyboard_globals.row = 0;
		virtual_keyboard_globals.column = 0;
		virtual_keyboard_globals.active = TRUE;
		virtual_keyboard_globals.text_buffer = text_buffer;
		virtual_keyboard_globals.insertion_point = text_buffer + ustrlen(text_buffer);
		virtual_keyboard_globals.buffer_size = MIN(sizeof(virtual_keyboard_globals.original_buffer), buffer_size);
		virtual_keyboard_globals.last_event = NONE;
		virtual_keyboard_globals.time_of_last_event = system_milliseconds();
		virtual_keyboard_globals.caption_index = caption_index;
		virtual_keyboard_globals.shift_active = FALSE;
		virtual_keyboard_globals.caps_active = FALSE;
		virtual_keyboard_globals.symbols_active = FALSE;
		virtual_keyboard_globals.first_key_replaces_buffer = TRUE;
		ustrncpy(virtual_keyboard_globals.original_buffer, text_buffer, NUMBEROF(virtual_keyboard_globals.original_buffer));
		virtual_keyboard_globals.original_buffer[NUMBEROF(virtual_keyboard_globals.original_buffer) - 1] = '\0';
		virtual_keyboard_globals.buffer_saved = FALSE;
		ui_play_audio_feedback_sound(_audio_feedback_forward);
		success = TRUE;
	}

	return success;
}

boolean virtual_keyboard_close(
	void)
{
	return virtual_keyboard_cancel();
}

boolean virtual_keyboard_active(
	void)
{
	return virtual_keyboard_globals.active;
}

void virtual_keyboard_process(
	void)
{
	if (virtual_keyboard_globals.active)
	{
		virtual_keyboard_process_internal();
	}

	return;
}

void virtual_keyboard_render(
	void)
{
	if (virtual_keyboard_globals.active)
	{
		virtual_keyboard_render_internal();
	}

	return;
}

boolean virtual_keyboard_last_exit_saved_text(
	void)
{
	return virtual_keyboard_globals.buffer_saved;
}

/* ---------- private code */

static boolean virtual_keyboard_tab_left(
	void)
{
	char key = virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column];

	do
	{
		if (--virtual_keyboard_globals.column < 0)
		{
			virtual_keyboard_globals.column = NUMBER_OF_VIRTUAL_KEYBOARD_COLUMNS - 1;
		}
	}
	while (virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column] == key);

	ui_play_audio_feedback_sound(_audio_feedback_cursor);

	return TRUE;
}

static boolean virtual_keyboard_tab_right(
	void)
{
	char key = virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column];

	do
	{
		if (++virtual_keyboard_globals.column == NUMBER_OF_VIRTUAL_KEYBOARD_COLUMNS)
		{
			virtual_keyboard_globals.column = 0;
		}
	}
	while (virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column] == key);

	ui_play_audio_feedback_sound(_audio_feedback_cursor);

	return TRUE;
}

static boolean virtual_keyboard_tab_up(
	void)
{
	char key = virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column];

	do
	{
		if (--virtual_keyboard_globals.row < 0)
		{
			virtual_keyboard_globals.row = NUMBER_OF_VIRTUAL_KEYBOARD_ROWS - 1;
		}
	}
	while (virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column] == key);

	ui_play_audio_feedback_sound(_audio_feedback_cursor);

	return TRUE;
}

static boolean virtual_keyboard_tab_down(
	void)
{
	char key = virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column];

	do
	{
		if (++virtual_keyboard_globals.row == NUMBER_OF_VIRTUAL_KEYBOARD_ROWS)
		{
			virtual_keyboard_globals.row = 0;
		}
	}
	while (virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column] == key);

	ui_play_audio_feedback_sound(_audio_feedback_cursor);

	return TRUE;
}

static boolean virtual_keyboard_cancel(
	void)
{
	virtual_keyboard_globals.active = FALSE;

	if (virtual_keyboard_globals.text_buffer)
	{
		ustrncpy(
			virtual_keyboard_globals.text_buffer,
			virtual_keyboard_globals.original_buffer,
			virtual_keyboard_globals.buffer_size / sizeof(wchar_t));
		virtual_keyboard_globals.text_buffer[virtual_keyboard_globals.buffer_size / sizeof(wchar_t) - 1] = '\0';
	}

	virtual_keyboard_globals.text_buffer = NULL;
	virtual_keyboard_globals.original_buffer[0] = '\0';
	virtual_keyboard_globals.buffer_saved = FALSE;
	ui_play_audio_feedback_sound(_audio_feedback_backward);

	return TRUE;
}

static void virtual_keyboard_process_internal(
	void)
{
	struct event_record event;
	static unsigned long time_of_last_tab = 0;
	unsigned long current_time = system_milliseconds();
	long virtual_keyboard_event = NONE;
	boolean success = FALSE;

	while (get_next_event(&event, NONE))
	{
		switch (event.type)
		{
		case _event_type_left_analog_stick:
			if (event.value.analog_value.y == SHORT_MAX)
			{
				virtual_keyboard_event = _event_tab_up;
			}
			else if (event.value.analog_value.y == SHORT_MIN)
			{
				virtual_keyboard_event = _event_tab_down;
			}
			else if (event.value.analog_value.x == SHORT_MIN)
			{
				virtual_keyboard_event = _event_tab_left;
			}
			else if (event.value.analog_value.x == SHORT_MAX)
			{
				virtual_keyboard_event = _event_tab_right;
			}
			break;
		case _event_type_button:
			switch (event.value.button_value.button)
			{
			case _gamepad_analog_button_a:
				if (event.value.button_value.value == 1)
				{
					virtual_keyboard_event = _event_key_select;
				}
				break;
			case _gamepad_analog_button_b:
			case _gamepad_binary_button_back:
				if (event.value.button_value.value == 1)
				{
					virtual_keyboard_event = _event_cancel;
				}
				break;
			case _gamepad_binary_button_dpad_up:
				if (virtual_keyboard_globals.last_event != _event_tab_up ||
					current_time - time_of_last_tab >= VIRTUAL_KEYBOARD_TAB_REPEAT_MILLISECONDS ||
					event.value.button_value.value == 1)
				{
					virtual_keyboard_event = _event_tab_up;
					time_of_last_tab = current_time;
				}
				break;
			case _gamepad_binary_button_dpad_left:
				if (virtual_keyboard_globals.last_event != _event_tab_left ||
					current_time - time_of_last_tab >= VIRTUAL_KEYBOARD_TAB_REPEAT_MILLISECONDS ||
					event.value.button_value.value == 1)
				{
					virtual_keyboard_event = _event_tab_left;
					time_of_last_tab = current_time;
				}
				break;
			case _gamepad_binary_button_dpad_down:
				if (virtual_keyboard_globals.last_event != _event_tab_down ||
					current_time - time_of_last_tab >= VIRTUAL_KEYBOARD_TAB_REPEAT_MILLISECONDS ||
					event.value.button_value.value == 1)
				{
					virtual_keyboard_event = _event_tab_down;
					time_of_last_tab = current_time;
				}
				break;
			case _gamepad_binary_button_dpad_right:
				if (virtual_keyboard_globals.last_event != _event_tab_right ||
					current_time - time_of_last_tab >= VIRTUAL_KEYBOARD_TAB_REPEAT_MILLISECONDS ||
					event.value.button_value.value == 1)
				{
					virtual_keyboard_event = _event_tab_right;
					time_of_last_tab = current_time;
				}
				break;
			case _gamepad_binary_button_start:
				if (event.value.button_value.value == 1)
				{
					virtual_keyboard_globals.row = 0;
					virtual_keyboard_globals.column = 0;
					virtual_keyboard_event = _event_key_select;
				}
				break;
			case _gamepad_analog_button_x:
				if (event.value.button_value.value == 1)
				{
					if (virtual_keyboard_globals.first_key_replaces_buffer == TRUE)
					{
						match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 635, virtual_keyboard_globals.buffer_size>0);

						memset(virtual_keyboard_globals.text_buffer, 0, virtual_keyboard_globals.buffer_size);
						virtual_keyboard_globals.insertion_point = virtual_keyboard_globals.text_buffer;
						virtual_keyboard_globals.first_key_replaces_buffer = FALSE;
						ui_play_audio_feedback_sound(_audio_feedback_cursor);
					}
					else
					{
						virtual_keyboard_backspace();
					}

					success = TRUE;
				}
				break;
			case _gamepad_analog_button_left_trigger:
				if (event.value.button_value.value == 1)
				{
					virtual_keyboard_globals.first_key_replaces_buffer = FALSE;

					if (virtual_keyboard_globals.insertion_point > virtual_keyboard_globals.text_buffer)
					{
						virtual_keyboard_globals.insertion_point--;
					}

					ui_play_audio_feedback_sound(_audio_feedback_cursor);
					success = TRUE;
				}
				break;
			case _gamepad_analog_button_right_trigger:
				if (event.value.button_value.value == 1)
				{
					virtual_keyboard_globals.first_key_replaces_buffer = FALSE;

					if (*virtual_keyboard_globals.insertion_point)
					{
						virtual_keyboard_globals.insertion_point++;
					}

					ui_play_audio_feedback_sound(_audio_feedback_cursor);
					success = TRUE;
				}
				break;
			}
			break;
		}
	}

	if (virtual_keyboard_event != NONE)
	{
		virtual_keyboard_globals.last_key = virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column];

		switch (virtual_keyboard_event)
		{
		case _event_tab_left:
			success = virtual_keyboard_tab_left();
			break;
		case _event_tab_right:
			success = virtual_keyboard_tab_right();
			break;
		case _event_tab_up:
			success = virtual_keyboard_tab_up();
			break;
		case _event_tab_down:
			success = virtual_keyboard_tab_down();
			break;
		case _event_key_select:
			success = virtual_keyboard_select();
			break;
		case _event_cancel:
			success = virtual_keyboard_cancel();
			break;
		}

		if (success == TRUE)
		{
			virtual_keyboard_globals.time_of_last_event = current_time;
			virtual_keyboard_globals.last_event = virtual_keyboard_event;
		}
	}

	return;
}

static boolean virtual_keyboard_select(
	void)
{
	switch (virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column])
	{
	case _vkey_done:
		if (ustrcmp(virtual_keyboard_globals.original_buffer, virtual_keyboard_globals.text_buffer))
		{
			if (virtual_keyboard_globals.text_buffer[0] != '\0')
			{
				if (saved_game_file_name_unique(virtual_keyboard_globals.text_buffer))
				{
					virtual_keyboard_globals.buffer_saved = TRUE;
				}
				else
				{
					display_error(_error_already_a_saved_game_file_with_that_name, NONE, TRUE, FALSE);
					virtual_keyboard_cancel();
				}
			}
			else
			{
				display_error(_error_cannot_create_saved_game_file_with_empty_name, NONE, TRUE, FALSE);
				virtual_keyboard_cancel();
			}
		}
		else
		{
			virtual_keyboard_globals.buffer_saved = TRUE;
		}

		ui_play_audio_feedback_sound(_audio_feedback_backward);
		virtual_keyboard_globals.active = FALSE;
		event_manager_flush();
		break;
	case _vkey_shift:
		ui_play_audio_feedback_sound(_audio_feedback_cursor);
		virtual_keyboard_globals.shift_active = !virtual_keyboard_globals.shift_active;
		break;
	case _vkey_caps:
		ui_play_audio_feedback_sound(_audio_feedback_cursor);
		virtual_keyboard_globals.caps_active = !virtual_keyboard_globals.caps_active;
		break;
	case _vkey_symbols:
		ui_play_audio_feedback_sound(_audio_feedback_cursor);
		virtual_keyboard_globals.symbols_active = !virtual_keyboard_globals.symbols_active;
		break;
	case _vkey_backspace:
		if (virtual_keyboard_globals.first_key_replaces_buffer == TRUE)
		{
			match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 854, virtual_keyboard_globals.buffer_size>0);

			memset(virtual_keyboard_globals.text_buffer, 0, virtual_keyboard_globals.buffer_size);
			virtual_keyboard_globals.insertion_point = virtual_keyboard_globals.text_buffer;
			virtual_keyboard_globals.first_key_replaces_buffer = FALSE;
		}
		else
		{
			virtual_keyboard_backspace();
		}
		break;
	case _vkey_left:
		if (virtual_keyboard_globals.insertion_point > virtual_keyboard_globals.text_buffer)
		{
			virtual_keyboard_globals.insertion_point--;
		}

		virtual_keyboard_globals.first_key_replaces_buffer = FALSE;
		ui_play_audio_feedback_sound(_audio_feedback_cursor);
		break;
	case _vkey_right:
		if (*virtual_keyboard_globals.insertion_point)
		{
			virtual_keyboard_globals.insertion_point++;
		}

		virtual_keyboard_globals.first_key_replaces_buffer = FALSE;
		ui_play_audio_feedback_sound(_audio_feedback_cursor);
		break;
	case _vkey_space:
		if (virtual_keyboard_globals.first_key_replaces_buffer == TRUE)
		{
			match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 889, virtual_keyboard_globals.buffer_size>0);

			memset(virtual_keyboard_globals.text_buffer, 0, virtual_keyboard_globals.buffer_size);
			virtual_keyboard_globals.insertion_point = virtual_keyboard_globals.text_buffer;
			virtual_keyboard_globals.first_key_replaces_buffer = FALSE;
		}

		if (virtual_keyboard_free_space_in_text_buffer() >= MINIMUM_FREE_SPACE_FOR_CHARACTER)
		{
			memmove(
				virtual_keyboard_globals.insertion_point + 1,
				virtual_keyboard_globals.insertion_point,
				virtual_keyboard_globals.buffer_size - ((byte *)virtual_keyboard_globals.insertion_point - (byte *)virtual_keyboard_globals.text_buffer) - sizeof(wchar_t));
			*virtual_keyboard_globals.insertion_point = ' ';
			virtual_keyboard_globals.insertion_point++;
			ui_play_audio_feedback_sound(_audio_feedback_forward);
		}
		else
		{
			ui_play_audio_feedback_sound(_audio_feedback_failure);
		}
		break;
	default:
		if (virtual_keyboard_globals.first_key_replaces_buffer == TRUE)
		{
			match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 914, virtual_keyboard_globals.buffer_size>0);

			memset(virtual_keyboard_globals.text_buffer, 0, virtual_keyboard_globals.buffer_size);
			virtual_keyboard_globals.insertion_point = virtual_keyboard_globals.text_buffer;
			virtual_keyboard_globals.first_key_replaces_buffer = FALSE;
		}

		if (virtual_keyboard_free_space_in_text_buffer() >= MINIMUM_FREE_SPACE_FOR_CHARACTER)
		{
			memmove(
				virtual_keyboard_globals.insertion_point + 1,
				virtual_keyboard_globals.insertion_point,
				virtual_keyboard_globals.buffer_size - ((byte *)virtual_keyboard_globals.insertion_point - (byte *)virtual_keyboard_globals.text_buffer) - sizeof(wchar_t));
			*virtual_keyboard_globals.insertion_point = virtual_keyboard_get_current_character();
			virtual_keyboard_globals.insertion_point++;

			if (!ustrcmp(virtual_keyboard_globals.text_buffer, VIRTUAL_KEYBOARD_FORTUNE_TEXT))
			{
				unsigned long fortune_index = system_milliseconds() % NUMBER_OF_VIRTUAL_KEYBOARD_FORTUNES;

				if (fortune_index > NUMBER_OF_VIRTUAL_KEYBOARD_FORTUNES - 1)
				{
					fortune_index = NUMBER_OF_VIRTUAL_KEYBOARD_FORTUNES - 1;
				}

				virtual_keyboard_globals.caption_index = fortune_index + FIRST_VIRTUAL_KEYBOARD_FORTUNE_STRING_INDEX;

				if (virtual_keyboard_globals.original_buffer[0])
				{
					ustrcpy(virtual_keyboard_globals.text_buffer, virtual_keyboard_globals.original_buffer);
					virtual_keyboard_globals.insertion_point = virtual_keyboard_globals.text_buffer + ustrlen(virtual_keyboard_globals.text_buffer);
				}
				else
				{
					memset(virtual_keyboard_globals.text_buffer, 0, virtual_keyboard_globals.buffer_size);
					virtual_keyboard_globals.insertion_point = virtual_keyboard_globals.text_buffer;
				}
			}

			ui_play_audio_feedback_sound(_audio_feedback_forward);
		}
		else
		{
			ui_play_audio_feedback_sound(_audio_feedback_failure);
		}
	}

	if (virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column] != _vkey_shift)
	{
		virtual_keyboard_globals.shift_active = FALSE;
	}

	return TRUE;
}

static wchar_t virtual_keyboard_get_character(
	word keycode)
{
	struct virtual_key *key;
	wchar_t character;

	match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 987, virtual_keyboard_globals.keyboard != NULL);
	match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 988, keycode < NUMBER_OF_CONFIGURABLE_VIRTUAL_KEYS);

	key = (struct virtual_key *)virtual_keyboard_globals.keyboard->virtual_keys.address + keycode;

	if (virtual_keyboard_globals.shift_active)
	{
		if (virtual_keyboard_globals.caps_active)
		{
			character = key->shift_caps_character;
		}
		else if (virtual_keyboard_globals.symbols_active)
		{
			character = key->shift_symbols_character;
		}
		else
		{
			character = key->shift_character;
		}
	}
	else if (virtual_keyboard_globals.caps_active)
	{
		if (virtual_keyboard_globals.symbols_active)
		{
			character = key->caps_symbols_character;
		}
		else
		{
			character = key->caps_character;
		}
	}
	else if (virtual_keyboard_globals.symbols_active)
	{
		character = key->symbols_character;
	}
	else
	{
		character = key->lowercase_character;
	}

	if (!character)
	{
		character = RENDER_AS_NULL_CHARACTER;
	}

	return character;
}

static wchar_t virtual_keyboard_get_current_character(
	void)
{
	return virtual_keyboard_get_character(virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column]);
}

static void virtual_keyboard_render_internal(
	void)
{
	struct font_header *keyboard_font_header;
	real_argb_color text_argb = { 1.f, .9f, .9f, .9f };
	real_argb_color caption_color = text_argb;

	match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 1036, (virtual_keyboard_globals.keyboard != NULL) && (virtual_keyboard_globals.keyboard->font_tag.index != NONE));

	keyboard_font_header = font_definition_get(virtual_keyboard_globals.keyboard->font_tag.index);
	match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 1038, keyboard_font_header);

	if (virtual_keyboard_globals.keyboard->background_bitmap_tag.index != NONE)
	{
		struct bitmap_data *bitmap;
		rectangle2d rect;

		rect.y0 = 0;
		rect.x0 = 0;
		rect.y1 = DISPLAY_HEIGHT;
		rect.x1 = DISPLAY_WIDTH;
		bitmap = bitmap_group_get_bitmap_from_sequence(virtual_keyboard_globals.keyboard->background_bitmap_tag.index, 0, 0);
		match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 1046, bitmap);

		draw_bitmap_in_rect(bitmap, &rect, &rect, NULL, NONE, NULL, FALSE);
	}

	draw_string_set_draw_mode(
		virtual_keyboard_globals.keyboard->font_tag.index,
		_text_style_plain,
		_text_justification_left,
		0,
		&caption_color);

	if (virtual_keyboard_globals.keyboard->special_key_labels_string_list_tag.index != NONE)
	{
		rectangle2d rect;
		wchar_t *caption = unicode_string_list_get_string(
			virtual_keyboard_globals.keyboard->special_key_labels_string_list_tag.index,
			virtual_keyboard_globals.caption_index);

		rect.y0 = CAPTION_TEXT_FIELD_TOP;
		rect.x0 = CAPTION_TEXT_FIELD_LEFT;
		rect.y1 = CAPTION_TEXT_FIELD_BOTTOM;
		rect.x1 = CAPTION_TEXT_FIELD_RIGHT;
		rasterizer_draw_unicode_string(&rect, &rect, NULL, 0, caption);
	}

	draw_string_set_draw_mode(
		virtual_keyboard_globals.keyboard->font_tag.index,
		_text_style_plain,
		_text_justification_center,
		0,
		&text_argb);

	{
		rectangle2d rect;

		rect.y0 = EDIT_TEXT_FIELD_TOP;
		rect.x0 = EDIT_TEXT_FIELD_LEFT;
		rect.y1 = EDIT_TEXT_FIELD_TOP + EDIT_TEXT_FIELD_HEIGHT;
		rect.x1 = EDIT_TEXT_FIELD_LEFT + EDIT_TEXT_FIELD_WIDTH;

		if (virtual_keyboard_globals.first_key_replaces_buffer == TRUE)
		{
			struct bitmap_data *bitmap = bitmap_group_get_bitmap_from_sequence(virtual_keyboard_globals.caret_bitmap_tag, 0, 0);

			if (bitmap)
			{
				rectangle2d text_bounds;
				rectangle2d cursor_bounds;

				draw_unicode_string_compute_bounds(&rect, virtual_keyboard_globals.text_buffer, &text_bounds, &cursor_bounds);
				text_bounds.x0 -= 2;
				text_bounds.x1 += 2;
				draw_bitmap_in_rect(bitmap, &text_bounds, &rect, NULL, 0x7F7F7F7F, NULL, FALSE);
			}
		}

		rasterizer_draw_unicode_string(&rect, &rect, NULL, 0, virtual_keyboard_globals.text_buffer);
	}

	if (!virtual_keyboard_globals.first_key_replaces_buffer &&
		virtual_keyboard_globals.caret_bitmap_tag != NONE &&
		TEST_FLAG(system_milliseconds() / 1000, 0))
	{
		rectangle2d caret_rect;
		wchar_t *character = virtual_keyboard_globals.text_buffer;
		short caret_offset = 0;
		short text_width = 0;
		short caret_height = keyboard_font_header->descending_height + keyboard_font_header->ascending_height;
		struct bitmap_data *bitmap = bitmap_group_get_bitmap_from_sequence(virtual_keyboard_globals.caret_bitmap_tag, 0, 0);

		if (bitmap)
		{
			while (character)
			{
				struct font_character *font_character = font_get_character_by_ascii_code(keyboard_font_header, *character);

				if (!font_character)
				{
					break;
				}

				if (character < virtual_keyboard_globals.insertion_point)
				{
					caret_offset += font_character->character_width;
				}

				text_width += font_character->character_width;
				character++;
			}

			caret_rect.y0 = EDIT_TEXT_CARET_TOP;
			caret_rect.x0 = caret_offset - (text_width >> 1) + EDIT_TEXT_FIELD_LEFT + EDIT_TEXT_FIELD_WIDTH / 2;
			caret_rect.x1 = caret_rect.x0 + EDIT_TEXT_CARET_WIDTH;
			caret_rect.y1 = caret_height + EDIT_TEXT_CARET_TOP;
			draw_bitmap_in_rect(bitmap, &caret_rect, NULL, NULL, NONE, NULL, FALSE);
		}
	}

	draw_string_set_draw_mode(
		virtual_keyboard_globals.keyboard->font_tag.index,
		_text_style_plain,
		_text_justification_center,
		0,
		&text_argb);

	{
		long key_index;
		struct virtual_key *virtual_keys = (struct virtual_key *)virtual_keyboard_globals.keyboard->virtual_keys.address;
		wchar_t key_string[MAXIMUM_KEY_LABEL_STRING_LENGTH] = { 0 };

		for (key_index = 0; key_index < NUMBER_OF_CONFIGURABLE_VIRTUAL_KEYS; key_index++)
		{
			long bitmap_tag_index;

			key_string[0] = virtual_keyboard_get_character((word)key_index);

			if (font_get_character_by_ascii_code(keyboard_font_header, key_string[0]) ||
				(key_string[0] = RENDER_AS_NULL_CHARACTER, font_get_character_by_ascii_code(keyboard_font_header, RENDER_AS_NULL_CHARACTER)))
			{
				rectangle2d text_rect = keyboard_rect[key_index];

				text_rect.x0 += KEY_LABEL_TEXT_HOFFSET;
				text_rect.x1 += KEY_LABEL_TEXT_HOFFSET;
				text_rect.y0 += KEY_LABEL_TEXT_VOFFSET;
				text_rect.y1 += KEY_LABEL_TEXT_VOFFSET;
				rasterizer_draw_unicode_string(&text_rect, NULL, NULL, 0, key_string);
			}

			if (virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column] == key_index)
			{
				if (virtual_keyboard_globals.last_event == _event_key_select)
				{
					bitmap_tag_index = virtual_keys[key_index].active_background_bitmap_tag.index;
				}
				else
				{
					bitmap_tag_index = virtual_keys[key_index].selected_background_bitmap_tag.index;
				}
			}
			else
			{
				bitmap_tag_index = virtual_keys[key_index].unselected_background_bitmap_tag.index;
			}

			if (bitmap_tag_index != NONE)
			{
				struct bitmap_data *bitmap = bitmap_group_get_bitmap_from_sequence(bitmap_tag_index, 0, 0);

				if (bitmap)
				{
					draw_bitmap_in_rect(bitmap, &keyboard_rect[key_index], NULL, NULL, NONE, NULL, FALSE);
				}
			}
		}

		if (virtual_keyboard_globals.keyboard->special_key_labels_string_list_tag.index != NONE)
		{
			for (; key_index < NUMBER_OF_VIRTUAL_KEYS; key_index++)
			{
				wchar_t *label = unicode_string_list_get_string(
					virtual_keyboard_globals.keyboard->special_key_labels_string_list_tag.index,
					(short)(key_index - NUMBER_OF_CONFIGURABLE_VIRTUAL_KEYS));
				struct bitmap_data *bitmap = NULL;

				switch (key_index)
				{
				case _vkey_done:
					if (virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column] == key_index)
					{
						if (virtual_keyboard_globals.last_event == _event_key_select)
						{
							if (virtual_keys[key_index].active_background_bitmap_tag.index != NONE)
							{
								bitmap = bitmap_group_get_bitmap_from_sequence(
									virtual_keys[key_index].active_background_bitmap_tag.index,
									0,
									0);
							}
						}
						else
						{
							if (virtual_keys[key_index].selected_background_bitmap_tag.index != NONE)
							{
								bitmap = bitmap_group_get_bitmap_from_sequence(
									virtual_keys[key_index].selected_background_bitmap_tag.index,
									0,
									0);
							}
						}
					}
					else
					{
						if (virtual_keys[key_index].unselected_background_bitmap_tag.index != NONE)
						{
							bitmap = bitmap_group_get_bitmap_from_sequence(
								virtual_keys[key_index].unselected_background_bitmap_tag.index,
								0,
								0);
						}
					}
					break;
				case _vkey_shift:
					if (virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column] == key_index)
					{
						if (virtual_keyboard_globals.last_event == _event_key_select)
						{
							if (virtual_keys[key_index].active_background_bitmap_tag.index != NONE)
							{
								bitmap = bitmap_group_get_bitmap_from_sequence(
									virtual_keys[key_index].active_background_bitmap_tag.index,
									0,
									0);
							}
						}
						else
						{
							if (virtual_keys[key_index].selected_background_bitmap_tag.index != NONE)
							{
								bitmap = bitmap_group_get_bitmap_from_sequence(
									virtual_keys[key_index].selected_background_bitmap_tag.index,
									0,
									0);
							}
						}
					}
					else if (virtual_keyboard_globals.shift_active)
					{
						if (virtual_keys[key_index].sticky_background_bitmap_tag.index != NONE)
						{
							bitmap = bitmap_group_get_bitmap_from_sequence(
								virtual_keys[key_index].sticky_background_bitmap_tag.index,
								0,
								0);
						}
					}
					else
					{
						if (virtual_keys[key_index].unselected_background_bitmap_tag.index != NONE)
						{
							bitmap = bitmap_group_get_bitmap_from_sequence(
								virtual_keys[key_index].unselected_background_bitmap_tag.index,
								0,
								0);
						}
					}
					break;
				case _vkey_caps:
					if (virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column] == key_index)
					{
						if (virtual_keyboard_globals.last_event == _event_key_select)
						{
							if (virtual_keys[key_index].active_background_bitmap_tag.index != NONE)
							{
								bitmap = bitmap_group_get_bitmap_from_sequence(
									virtual_keys[key_index].active_background_bitmap_tag.index,
									0,
									0);
							}
						}
						else
						{
							if (virtual_keys[key_index].selected_background_bitmap_tag.index != NONE)
							{
								bitmap = bitmap_group_get_bitmap_from_sequence(
									virtual_keys[key_index].selected_background_bitmap_tag.index,
									0,
									0);
							}
						}
					}
					else if (virtual_keyboard_globals.caps_active)
					{
						if (virtual_keys[key_index].sticky_background_bitmap_tag.index != NONE)
						{
							bitmap = bitmap_group_get_bitmap_from_sequence(
								virtual_keys[key_index].sticky_background_bitmap_tag.index,
								0,
								0);
						}
					}
					else
					{
						if (virtual_keys[key_index].unselected_background_bitmap_tag.index != NONE)
						{
							bitmap = bitmap_group_get_bitmap_from_sequence(
								virtual_keys[key_index].unselected_background_bitmap_tag.index,
								0,
								0);
						}
					}
					break;
				case _vkey_symbols:
					if (virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column] == key_index)
					{
						if (virtual_keyboard_globals.last_event == _event_key_select)
						{
							if (virtual_keys[key_index].active_background_bitmap_tag.index != NONE)
							{
								bitmap = bitmap_group_get_bitmap_from_sequence(
									virtual_keys[key_index].active_background_bitmap_tag.index,
									0,
									0);
							}
						}
						else
						{
							if (virtual_keys[key_index].selected_background_bitmap_tag.index != NONE)
							{
								bitmap = bitmap_group_get_bitmap_from_sequence(
									virtual_keys[key_index].selected_background_bitmap_tag.index,
									0,
									0);
							}
						}
					}
					else if (virtual_keyboard_globals.symbols_active)
					{
						if (virtual_keys[key_index].sticky_background_bitmap_tag.index != NONE)
						{
							bitmap = bitmap_group_get_bitmap_from_sequence(
								virtual_keys[key_index].sticky_background_bitmap_tag.index,
								0,
								0);
						}
					}
					else
					{
						if (virtual_keys[key_index].unselected_background_bitmap_tag.index != NONE)
						{
							bitmap = bitmap_group_get_bitmap_from_sequence(
								virtual_keys[key_index].unselected_background_bitmap_tag.index,
								0,
								0);
						}
					}
					break;
				case _vkey_backspace:
				case _vkey_left:
				case _vkey_right:
				case _vkey_space:
					if (virtual_keyboard_layout_table[virtual_keyboard_globals.row][virtual_keyboard_globals.column] == key_index)
					{
						if (virtual_keyboard_globals.last_event == _event_key_select)
						{
							if (virtual_keys[key_index].active_background_bitmap_tag.index != NONE)
							{
								bitmap = bitmap_group_get_bitmap_from_sequence(
									virtual_keys[key_index].active_background_bitmap_tag.index,
									0,
									0);
							}
						}
						else
						{
							if (virtual_keys[key_index].selected_background_bitmap_tag.index != NONE)
							{
								bitmap = bitmap_group_get_bitmap_from_sequence(
									virtual_keys[key_index].selected_background_bitmap_tag.index,
									0,
									0);
							}
						}
					}
					else
					{
						if (virtual_keys[key_index].unselected_background_bitmap_tag.index != NONE)
						{
							bitmap = bitmap_group_get_bitmap_from_sequence(
								virtual_keys[key_index].unselected_background_bitmap_tag.index,
								0,
								0);
						}
					}
					break;
				default:
					match_assert("c:\\halo\\SOURCE\\interface\\virtual_keyboard.c", 1408, !"what key is this?");
					break;
				}

				if (label)
				{
					rectangle2d rect = keyboard_rect[key_index];

					rect.x0 += KEY_LABEL_TEXT_HOFFSET;
					rect.x1 += KEY_LABEL_TEXT_HOFFSET;
					rect.y0 += KEY_LABEL_TEXT_VOFFSET;
					rect.y1 += KEY_LABEL_TEXT_VOFFSET;
					rasterizer_draw_unicode_string(&rect, NULL, NULL, 0, label);
				}

				if (bitmap)
				{
					draw_bitmap_in_rect(bitmap, &keyboard_rect[key_index], &keyboard_rect[key_index], NULL, NONE, NULL, FALSE);
				}
			}
		}
	}

	return;
}

static long virtual_keyboard_free_space_in_text_buffer(
	void)
{
	return virtual_keyboard_globals.buffer_size - (ustrlen(virtual_keyboard_globals.text_buffer) + 1) * sizeof(wchar_t);
}

static void virtual_keyboard_backspace(
	void)
{
	if (virtual_keyboard_globals.insertion_point > virtual_keyboard_globals.text_buffer)
	{
		long size = virtual_keyboard_globals.buffer_size - ((byte *)virtual_keyboard_globals.insertion_point - (byte *)virtual_keyboard_globals.text_buffer);

		if (size >= 0)
		{
			memmove(virtual_keyboard_globals.insertion_point - 1, virtual_keyboard_globals.insertion_point, size);
			virtual_keyboard_globals.text_buffer[virtual_keyboard_globals.buffer_size / sizeof(wchar_t) - 1] = '\0';
			virtual_keyboard_globals.insertion_point--;
		}
	}

	ui_play_audio_feedback_sound(_audio_feedback_cursor);

	return;
}
