/*
INPUT_XBOX.C
*/

/* ---------- headers */

#include "cseries.h"
#include "input.h"
#include "input_abstraction.h"
#include "HaloAutoTest.h"
#include "console.h"
#include "game.h"
#include "player_ui.h"

/* ---------- constants */

enum
{
	NUMBER_OF_VIRTUAL_CODES = 256,
	NUMBER_OF_ASCII_CODES = 128,
	MAXIMUM_BUFFERED_KEYSTROKES = 64,
	KEY_REPEAT_DELAY = 500,
	KEY_REPEAT_RATE = 100,
	ANALOG_BUTTON_DOWN = 64,
	ANALOG_BUTTON_UP = 32,
};

enum
{
	GAMEPAD_STICK_DEAD_ZONE = 9000, /* fake name */
	INPUT_RUMBLE_THREAD_STACK_SIZE = 16*1024, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct input_globals_xbox
{
	boolean suppressed; /* fake name */
	boolean rumble_thread_resumed; /* fake name */
	HANDLE gamepad_handles[MAXIMUM_GAMEPADS];
	struct gamepad_state gamepads[MAXIMUM_GAMEPADS];
	struct gamepad_state suppressed_gamepad; /* fake name */
	struct vibrate_data vibrations[MAXIMUM_GAMEPADS]; /* fake name */
	HANDLE rumble_thread; /* fake name */
	HANDLE rumble_event; /* fake name */
	boolean rumble_this_vertical_blank; /* fake name */
	boolean in_frame; /* fake name */
	long unused; /* fake name */
	HANDLE keyboard_handle;
	byte key_ticks[NUMBER_OF_KEYS];
	byte key_latches[NUMBER_OF_KEYS];
	short buffered_key_read_index;
	short buffered_key_write_index;
	struct key_stroke buffered_keys[MAXIMUM_BUFFERED_KEYSTROKES];
};

/* ---------- prototypes */

static void update_threshold(byte *threshold, boolean down, byte value);
static void acquire_input_mutex(void);
static void release_input_mutex(void);
static void input_update_gamepads(void);
static void input_update_gamepads_rumble(void);
static void input_update_keyboard(void);
static DWORD WINAPI input_update_thread_proc(LPVOID parameter);

/* ---------- globals */

static byte const gamepad_analog_button_table[NUMBER_OF_GAMEPAD_ANALOG_BUTTONS]= /* fake name */
{
	XINPUT_GAMEPAD_A,
	XINPUT_GAMEPAD_B,
	XINPUT_GAMEPAD_X,
	XINPUT_GAMEPAD_Y,
	XINPUT_GAMEPAD_BLACK,
	XINPUT_GAMEPAD_WHITE,
	XINPUT_GAMEPAD_LEFT_TRIGGER,
	XINPUT_GAMEPAD_RIGHT_TRIGGER
};

static byte const gamepad_binary_button_table[NUMBER_OF_GAMEPAD_BINARY_BUTTONS]= /* fake name */
{
	XINPUT_GAMEPAD_DPAD_UP,
	XINPUT_GAMEPAD_DPAD_DOWN,
	XINPUT_GAMEPAD_DPAD_LEFT,
	XINPUT_GAMEPAD_DPAD_RIGHT,
	XINPUT_GAMEPAD_START,
	XINPUT_GAMEPAD_BACK,
	XINPUT_GAMEPAD_LEFT_THUMB,
	XINPUT_GAMEPAD_RIGHT_THUMB
};

static short const virtual_to_key_table[NUMBER_OF_VIRTUAL_CODES]=
{
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	_key_backspace,
	_key_tab,
	NONE,
	NONE,
	NONE,
	_key_return,
	NONE,
	NONE,
	_key_shift,
	_key_control,
	_key_left_alt,
	_key_pause,
	_key_caps_lock,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	_key_escape,
	NONE,
	NONE,
	NONE,
	NONE,
	_key_space,
	_key_page_up,
	_key_page_down,
	_key_end,
	_key_home,
	_key_left_arrow,
	_key_up_arrow,
	_key_right_arrow,
	_key_down_arrow,
	NONE,
	NONE,
	NONE,
	NONE,
	_key_insert,
	_key_delete,
	NONE,
	_key_0,
	_key_1,
	_key_2,
	_key_3,
	_key_4,
	_key_5,
	_key_6,
	_key_7,
	_key_8,
	_key_9,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	_key_a,
	_key_b,
	_key_c,
	_key_d,
	_key_e,
	_key_f,
	_key_g,
	_key_h,
	_key_i,
	_key_j,
	_key_k,
	_key_l,
	_key_m,
	_key_n,
	_key_o,
	_key_p,
	_key_q,
	_key_r,
	_key_s,
	_key_t,
	_key_u,
	_key_v,
	_key_w,
	_key_x,
	_key_y,
	_key_z,
	_key_left_windows,
	_key_right_windows,
	_key_menu,
	NONE,
	NONE,
	_keypad_0,
	_keypad_1,
	_keypad_2,
	_keypad_3,
	_keypad_4,
	_keypad_5,
	_keypad_6,
	_keypad_7,
	_keypad_8,
	_keypad_9,
	_keypad_multiply,
	_keypad_add,
	NONE,
	_keypad_subtract,
	_keypad_decimal,
	_keypad_divide,
	_key_f1,
	_key_f2,
	_key_f3,
	_key_f4,
	_key_f5,
	_key_f6,
	_key_f7,
	_key_f8,
	_key_f9,
	_key_f10,
	_key_f11,
	_key_f12,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	_keypad_num_lock,
	_key_scroll_lock,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	_key_semicolon,
	_key_equal,
	_key_comma,
	_key_dash,
	_key_period,
	_key_forwardslash,
	_key_backquote,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	_key_left_bracket,
	_key_backslash,
	_key_right_bracket,
	_key_apostrophe,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE
};

static short const ascii_to_key_table[NUMBER_OF_ASCII_CODES]=
{
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	_key_space,
	_key_1,
	_key_apostrophe,
	_key_3,
	_key_4,
	_key_5,
	_key_7,
	_key_apostrophe,
	_key_9,
	_key_0,
	_key_8,
	_key_equal,
	_key_comma,
	_key_dash,
	_key_period,
	_key_forwardslash,
	_key_0,
	_key_1,
	_key_2,
	_key_3,
	_key_4,
	_key_5,
	_key_6,
	_key_7,
	_key_8,
	_key_9,
	_key_semicolon,
	_key_semicolon,
	_key_comma,
	_key_equal,
	_key_period,
	_key_forwardslash,
	_key_2,
	_key_a,
	_key_b,
	_key_c,
	_key_d,
	_key_e,
	_key_f,
	_key_g,
	_key_h,
	_key_i,
	_key_j,
	_key_k,
	_key_l,
	_key_m,
	_key_n,
	_key_o,
	_key_p,
	_key_q,
	_key_r,
	_key_s,
	_key_t,
	_key_u,
	_key_v,
	_key_w,
	_key_x,
	_key_y,
	_key_z,
	_key_left_bracket,
	_key_backslash,
	_key_right_bracket,
	_key_6,
	_key_dash,
	_key_backquote,
	_key_a,
	_key_b,
	_key_c,
	_key_d,
	_key_e,
	_key_f,
	_key_g,
	_key_h,
	_key_i,
	_key_j,
	_key_k,
	_key_l,
	_key_m,
	_key_n,
	_key_o,
	_key_p,
	_key_q,
	_key_r,
	_key_s,
	_key_t,
	_key_u,
	_key_v,
	_key_w,
	_key_x,
	_key_y,
	_key_z,
	_key_left_bracket,
	_key_backslash,
	_key_right_bracket,
	_key_backquote,
	_key_delete
};

static XINPUT_FEEDBACK gamepad_feedback[MAXIMUM_GAMEPADS] = {0}; /* fake name */
static point2d gamepad_raw_sticks[MAXIMUM_GAMEPADS][NUMBER_OF_GAMEPAD_STICKS] = {0}; /* fake name */
static struct input_globals_xbox input_globals = {0};

/* ---------- public code */

short fix_dead_zone(
	short value,
	short dead_range)
{
	if (value > dead_range)
	{
		return (value - dead_range) * SHORT_MAX / (SHORT_MAX - dead_range);
	}
	else if (value < -dead_range)
	{
		return (value + dead_range) * SHORT_MIN / (SHORT_MIN + dead_range);
	}
	else
	{
		return 0;
	}
}

void update_ticks(
	byte *ticks,
	boolean down)
{
	*ticks = down ? MIN(*ticks + 1, UNSIGNED_CHAR_MAX) : 0;

	return;
}

static void update_threshold(
	byte *threshold,
	boolean down,
	byte value)
{
	if (down)
	{
		byte new_threshold = (value < ANALOG_BUTTON_UP) ? 0 : value - ANALOG_BUTTON_UP;

		if (new_threshold > *threshold)
		{
			*threshold = new_threshold;
		}
	}
	else
	{
		byte new_threshold = (value > UNSIGNED_CHAR_MAX - ANALOG_BUTTON_DOWN) ? UNSIGNED_CHAR_MAX : value + ANALOG_BUTTON_DOWN;

		if (new_threshold < *threshold)
		{
			*threshold = new_threshold;
		}
	}

	return;
}

boolean input_initialize(
	void)
{
	XDEVICE_PREALLOC_TYPE device_types[]=
	{
		{ XDEVICE_TYPE_GAMEPAD, MAXIMUM_GAMEPADS },
		{ XDEVICE_TYPE_DEBUG_KEYBOARD, 1 },
		{ XDEVICE_TYPE_MEMORY_UNIT, MAXIMUM_GAMEPAD_MEMORY_UNITS },
	};
	XINPUT_DEBUG_KEYQUEUE_PARAMETERS keyboard_parameters;
	HRESULT result;

	memset(&input_globals, 0, sizeof(input_globals));
	XInitDevices(NUMBEROF(device_types), device_types);

	input_globals.rumble_this_vertical_blank = TRUE;
	input_globals.rumble_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	input_globals.rumble_thread = CreateThread(NULL, INPUT_RUMBLE_THREAD_STACK_SIZE, input_update_thread_proc, NULL, CREATE_SUSPENDED, NULL);
	SetThreadPriority(input_globals.rumble_thread, THREAD_PRIORITY_HIGHEST);
	input_globals.rumble_thread_resumed = TRUE;
	input_globals.unused = 0;

	keyboard_parameters.dwFlags = XINPUT_DEBUG_KEYQUEUE_FLAG_KEYDOWN | XINPUT_DEBUG_KEYQUEUE_FLAG_KEYREPEAT | XINPUT_DEBUG_KEYQUEUE_FLAG_KEYUP;
	keyboard_parameters.dwQueueSize = MAXIMUM_BUFFERED_KEYSTROKES;
	keyboard_parameters.dwRepeatDelay = KEY_REPEAT_DELAY;
	keyboard_parameters.dwRepeatInterval = KEY_REPEAT_RATE;
	result = XInputDebugInitKeyboardQueue(&keyboard_parameters);

	if (FAILED(result))
	{
		error(_error_silent, "XInputDebugInitKeyboardQueue failed (#%d) during input_initialize()", result);
	}

	HATInit();
	input_update_gamepads();
	input_update();
	input_globals.rumble_thread_resumed = FALSE;

	return TRUE;
}

void input_dispose(
	void)
{
	short gamepad_index;

	HATCleanup();

	if (input_globals.keyboard_handle)
	{
		XInputClose(input_globals.keyboard_handle);
		input_globals.keyboard_handle = NULL;
	}

	for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
	{
		if (input_globals.gamepad_handles[gamepad_index])
		{
			XInputClose(input_globals.gamepad_handles[gamepad_index]);
			input_globals.gamepad_handles[gamepad_index] = NULL;
		}
	}

	return;
}

void input_activate(
	void)
{
	return;
}

void input_deactivate(
	void)
{
	return;
}

void input_flush(
	void)
{
	memset(input_globals.gamepads, 0, sizeof(input_globals.gamepads));
	memset(input_globals.key_ticks, 0, sizeof(input_globals.key_ticks));
	memset(input_globals.key_latches, 0, sizeof(input_globals.key_latches));
	input_globals.buffered_key_read_index = 0;
	input_globals.buffered_key_write_index = 0;
	memset(input_globals.buffered_keys, 0, sizeof(input_globals.buffered_keys));

	return;
}

boolean input_key_is_down(
	short key_code)
{
	boolean result = FALSE;

	if (!input_globals.suppressed)
	{
		switch (key_code)
		{
		case _key_shift:
			result = MAX(input_globals.key_ticks[_key_left_shift], input_globals.key_ticks[_key_right_shift]);
			break;
		case _key_control:
			result = MAX(input_globals.key_ticks[_key_left_control], input_globals.key_ticks[_key_right_control]);
			break;
		case _key_windows:
			result = MAX(input_globals.key_ticks[_key_left_windows], input_globals.key_ticks[_key_right_windows]);
			break;
		case _key_alt:
			result = MAX(input_globals.key_ticks[_key_left_alt], input_globals.key_ticks[_key_right_alt]);
			break;
		default:
			match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 314, key_code>=0 && key_code<NUMBER_OF_KEYS);
			result = input_globals.key_ticks[key_code];
			break;
		}
	}

	return result;
}

boolean input_get_key(
	struct key_stroke *key)
{
	boolean result = FALSE;

	if (input_globals.buffered_key_read_index < input_globals.buffered_key_write_index)
	{
		match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 334, input_globals.buffered_key_read_index>=0 && input_globals.buffered_key_read_index<MAXIMUM_BUFFERED_KEYSTROKES);
		*key = input_globals.buffered_keys[input_globals.buffered_key_read_index++];
		result = TRUE;
	}

	return result;
}

const struct mouse_state *input_get_mouse_state(
	void)
{
	return NULL;
}

void input_suppress(
	void)
{
	input_globals.suppressed = TRUE;

	return;
}

boolean input_mouse_button_is_down(
	short button_index)
{
	return FALSE;
}

boolean input_has_gamepad(
	short gamepad_index)
{
	match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 369, gamepad_index>=0 && gamepad_index<MAXIMUM_GAMEPADS);

	return input_globals.gamepad_handles[gamepad_index] != NULL;
}

const struct gamepad_state *input_get_gamepad_state(
	short gamepad_index)
{
	const struct gamepad_state *result = NULL;

	match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 379, gamepad_index>=0 && gamepad_index<MAXIMUM_GAMEPADS);

	if (input_globals.gamepad_handles[gamepad_index])
	{
		if (input_globals.suppressed)
		{
			result = &input_globals.suppressed_gamepad;
		}
		else
		{
			result = &input_globals.gamepads[gamepad_index];
		}
	}

	return result;
}

void input_set_gamepad_rumbler_state(
	short gamepad_index,
	word left_speed,
	word right_speed)
{
	match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 408, gamepad_index>=0 && gamepad_index<MAXIMUM_GAMEPADS);

	if (!player_ui_rumble_disabled(gamepad_index))
	{
		input_globals.vibrations[gamepad_index].left_frequency = left_speed;
		input_globals.vibrations[gamepad_index].right_frequency = right_speed;
	}

	return;
}

void input_vertical_blank_interrupt(
	void)
{
	if (input_globals.rumble_this_vertical_blank)
	{
		SetEvent(input_globals.rumble_event);
	}

	input_globals.rumble_this_vertical_blank = !input_globals.rumble_this_vertical_blank;

	return;
}

static void acquire_input_mutex(
	void)
{
	return;
}

static void release_input_mutex(
	void)
{
	return;
}

static void input_update_gamepads(
	void)
{
	DWORD memory_unit_insertions, memory_unit_removals;
	unsigned long device_changes = 0;
	short gamepad_index;

	{
		DWORD gamepad_insertions, gamepad_removals;

		if (XGetDeviceChanges(XDEVICE_TYPE_GAMEPAD, &gamepad_insertions, &gamepad_removals))
		{
			for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
			{
				if (gamepad_removals & FLAG(gamepad_index))
				{
					match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 535, input_globals.gamepad_handles[gamepad_index]);
					XInputClose(input_globals.gamepad_handles[gamepad_index]);
					input_globals.gamepad_handles[gamepad_index] = NULL;
				}

				if (gamepad_insertions & FLAG(gamepad_index))
				{
					match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 542, input_globals.gamepad_handles[gamepad_index]==NULL);
					input_globals.gamepad_handles[gamepad_index] = XInputOpen(XDEVICE_TYPE_GAMEPAD, gamepad_index, XDEVICE_NO_SLOT, NULL);

					if (!input_globals.gamepad_handles[gamepad_index])
					{
						error(_error_silent, "XInputOpen (gamepad) failed (#%d) during input_update()", GetLastError());
					}
				}
			}

			if (gamepad_removals & XDEVICE_PORT0_MASK)
			{
				SET_FLAG(device_changes, _device_change_gamepad0_removed_bit, TRUE);
			}

			if (gamepad_removals & XDEVICE_PORT1_MASK)
			{
				SET_FLAG(device_changes, _device_change_gamepad1_removed_bit, TRUE);
			}

			if (gamepad_removals & XDEVICE_PORT2_MASK)
			{
				SET_FLAG(device_changes, _device_change_gamepad2_removed_bit, TRUE);
			}

			if (gamepad_removals & XDEVICE_PORT3_MASK)
			{
				SET_FLAG(device_changes, _device_change_gamepad3_removed_bit, TRUE);
			}

			if (gamepad_insertions & XDEVICE_PORT0_MASK)
			{
				SET_FLAG(device_changes, _device_change_gamepad0_inserted_bit, TRUE);
			}

			if (gamepad_insertions & XDEVICE_PORT1_MASK)
			{
				SET_FLAG(device_changes, _device_change_gamepad1_inserted_bit, TRUE);
			}

			if (gamepad_insertions & XDEVICE_PORT2_MASK)
			{
				SET_FLAG(device_changes, _device_change_gamepad2_inserted_bit, TRUE);
			}

			if (gamepad_insertions & XDEVICE_PORT3_MASK)
			{
				SET_FLAG(device_changes, _device_change_gamepad3_inserted_bit, TRUE);
			}
		}
	}

	if (XGetDeviceChanges(XDEVICE_TYPE_MEMORY_UNIT, &memory_unit_insertions, &memory_unit_removals))
	{
		if (memory_unit_removals & XDEVICE_PORT0_TOP_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller0_top_removed_bit, TRUE);
		}

		if (memory_unit_removals & XDEVICE_PORT0_BOTTOM_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller0_bottom_removed_bit, TRUE);
		}

		if (memory_unit_removals & XDEVICE_PORT1_TOP_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller1_top_removed_bit, TRUE);
		}

		if (memory_unit_removals & XDEVICE_PORT1_BOTTOM_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller1_bottom_removed_bit, TRUE);
		}

		if (memory_unit_removals & XDEVICE_PORT2_TOP_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller2_top_removed_bit, TRUE);
		}

		if (memory_unit_removals & XDEVICE_PORT2_BOTTOM_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller2_bottom_removed_bit, TRUE);
		}

		if (memory_unit_removals & XDEVICE_PORT3_TOP_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller3_top_removed_bit, TRUE);
		}

		if (memory_unit_removals & XDEVICE_PORT3_BOTTOM_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller3_bottom_removed_bit, TRUE);
		}

		if (memory_unit_insertions & XDEVICE_PORT0_TOP_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller0_top_inserted_bit, TRUE);
		}

		if (memory_unit_insertions & XDEVICE_PORT0_BOTTOM_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller0_bottom_inserted_bit, TRUE);
		}

		if (memory_unit_insertions & XDEVICE_PORT1_TOP_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller1_top_inserted_bit, TRUE);
		}

		if (memory_unit_insertions & XDEVICE_PORT1_BOTTOM_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller1_bottom_inserted_bit, TRUE);
		}

		if (memory_unit_insertions & XDEVICE_PORT2_TOP_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller2_top_inserted_bit, TRUE);
		}

		if (memory_unit_insertions & XDEVICE_PORT2_BOTTOM_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller2_bottom_inserted_bit, TRUE);
		}

		if (memory_unit_insertions & XDEVICE_PORT3_TOP_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller3_top_inserted_bit, TRUE);
		}

		if (memory_unit_insertions & XDEVICE_PORT3_BOTTOM_MASK)
		{
			SET_FLAG(device_changes, _device_change_controller3_bottom_inserted_bit, TRUE);
		}
	}

	input_abstraction_update_device_changes(device_changes);

	for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
	{
		if (input_globals.gamepad_handles[gamepad_index])
		{
			XINPUT_STATE state;
			HRESULT result = XInputGetState(input_globals.gamepad_handles[gamepad_index], &state);

			if (SUCCEEDED(result))
			{
				struct gamepad_state *gamepad = &input_globals.gamepads[gamepad_index];
				short button_index;

				for (button_index = 0; button_index < NUMBER_OF_GAMEPAD_ANALOG_BUTTONS; button_index++)
				{
					gamepad->analog_buttons[button_index] = state.Gamepad.bAnalogButtons[gamepad_analog_button_table[button_index]];
					update_ticks(&gamepad->buttons[button_index], gamepad->analog_buttons[button_index] > gamepad->analog_button_thresholds[button_index]);
					update_threshold(&gamepad->analog_button_thresholds[button_index], gamepad->buttons[button_index], gamepad->analog_buttons[button_index]);
				}

				for (button_index = 0; button_index < NUMBER_OF_GAMEPAD_BINARY_BUTTONS; button_index++)
				{
					update_ticks(&gamepad->buttons[FIRST_GAMEPAD_BINARY_BUTTON + button_index], state.Gamepad.wButtons & gamepad_binary_button_table[button_index]);
				}

				gamepad_raw_sticks[gamepad_index][_gamepad_stick_left].x = state.Gamepad.sThumbLX;
				gamepad_raw_sticks[gamepad_index][_gamepad_stick_left].y = state.Gamepad.sThumbLY;
				gamepad_raw_sticks[gamepad_index][_gamepad_stick_right].x = state.Gamepad.sThumbRX;
				gamepad_raw_sticks[gamepad_index][_gamepad_stick_right].y = state.Gamepad.sThumbRY;

				gamepad->sticks[_gamepad_stick_left].x = fix_dead_zone(state.Gamepad.sThumbLX, GAMEPAD_STICK_DEAD_ZONE);
				gamepad->sticks[_gamepad_stick_left].y = fix_dead_zone(state.Gamepad.sThumbLY, GAMEPAD_STICK_DEAD_ZONE);
				gamepad->sticks[_gamepad_stick_right].x = fix_dead_zone(state.Gamepad.sThumbRX, GAMEPAD_STICK_DEAD_ZONE);
				gamepad->sticks[_gamepad_stick_right].y = fix_dead_zone(state.Gamepad.sThumbRY, GAMEPAD_STICK_DEAD_ZONE);
			}
			else
			{
				error(_error_silent, "XGetState (gamepad) failed (#%d) during input_update()", result);
			}
		}
	}

	return;
}

static void input_update_gamepads_rumble(
	void)
{
	boolean suppress_rumble;
	short gamepad_index;

	if (input_globals.suppressed || console_is_active() || game_time_get_paused() || !game_in_progress())
	{
		suppress_rumble = TRUE;
	}
	else
	{
		suppress_rumble = FALSE;
	}

	for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
	{
		if (input_globals.gamepad_handles[gamepad_index])
		{
			if (gamepad_feedback[gamepad_index].Header.dwStatus == ERROR_SUCCESS)
			{
				gamepad_feedback[gamepad_index].Rumble.wLeftMotorSpeed = suppress_rumble ? 0 : input_globals.vibrations[gamepad_index].left_frequency;
				gamepad_feedback[gamepad_index].Rumble.wRightMotorSpeed = suppress_rumble ? 0 : input_globals.vibrations[gamepad_index].right_frequency;
				XInputSetState(input_globals.gamepad_handles[gamepad_index], &gamepad_feedback[gamepad_index]);
			}
			else if (gamepad_feedback[gamepad_index].Header.dwStatus != ERROR_IO_PENDING)
			{
				gamepad_feedback[gamepad_index].Header.dwStatus = ERROR_SUCCESS;
			}
		}
	}

	return;
}

static void input_update_keyboard(
	void)
{
	short key_index;

	{
		DWORD insertions, removals;

		if (XGetDeviceChanges(XDEVICE_TYPE_DEBUG_KEYBOARD, &insertions, &removals))
		{
			short port;

			for (port = 0; port < LONG_BITS; port++)
			{
				if (removals & FLAG(port))
				{
					match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 725, input_globals.keyboard_handle);
					XInputClose(input_globals.keyboard_handle);
					input_globals.keyboard_handle = NULL;
				}

				if (insertions & FLAG(port))
				{
					match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 732, input_globals.keyboard_handle==NULL);
					input_globals.keyboard_handle = XInputOpen(XDEVICE_TYPE_DEBUG_KEYBOARD, port, XDEVICE_NO_SLOT, NULL);

					if (!input_globals.keyboard_handle)
					{
						error(_error_silent, "XInputOpen (keyboard) failed (#%d) during input_update()", GetLastError());
					}
				}
			}
		}
	}

	for (key_index = 0; key_index < NUMBER_OF_KEYS; key_index++)
	{
		update_ticks(&input_globals.key_ticks[key_index], input_globals.key_latches[key_index]);
	}

	input_globals.buffered_key_read_index = 0;
	input_globals.buffered_key_write_index = 0;

	{
		XINPUT_DEBUG_KEYSTROKE keystroke;

		while (XInputDebugGetKeystroke(&keystroke) == ERROR_SUCCESS)
		{
			struct key_stroke key;

			key.modifier_flags = 0;
			SET_FLAG(key.modifier_flags, _key_modifier_shift_bit, keystroke.Flags & XINPUT_DEBUG_KEYSTROKE_FLAG_SHIFT);
			SET_FLAG(key.modifier_flags, _key_modifier_control_bit, keystroke.Flags & XINPUT_DEBUG_KEYSTROKE_FLAG_CTRL);
			SET_FLAG(key.modifier_flags, _key_modifier_alt_bit, keystroke.Flags & XINPUT_DEBUG_KEYSTROKE_FLAG_ALT);

			match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 773, keystroke.Ascii>=0 && keystroke.Ascii<NUMBER_OF_ASCII_CODES);
			key.ascii_code = (ascii_to_key_table[keystroke.Ascii]!=NONE) ? keystroke.Ascii : NONE;

			match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 776, keystroke.VirtualKey>=0 && keystroke.VirtualKey<NUMBER_OF_VIRTUAL_CODES);
			key.key_code = virtual_to_key_table[keystroke.VirtualKey];

			if (key.key_code!=NONE)
			{
				match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 782, key.key_code>=0 && key.key_code<NUMBER_OF_KEYS);

				if (keystroke.Flags & XINPUT_DEBUG_KEYSTROKE_FLAG_KEYUP)
				{
					input_globals.key_latches[key.key_code] = FALSE;

					if (input_globals.key_ticks[key.key_code] > 1)
					{
						input_globals.key_ticks[key.key_code] = 0;
					}
				}
				else
				{
					if (input_globals.buffered_key_write_index < MAXIMUM_BUFFERED_KEYSTROKES)
					{
						input_globals.buffered_keys[input_globals.buffered_key_write_index++] = key;
					}

					input_globals.key_latches[key.key_code] = TRUE;

					if (!input_globals.key_ticks[key.key_code])
					{
						input_globals.key_ticks[key.key_code] = 1;
					}
				}
			}
		}
	}

	return;
}

void input_get_raw_data_string(
	char *buffer,
	short size)
{
	match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 811, buffer);
	match_assert("c:\\halo\\SOURCE\\input\\input_xbox.c", 812, size>0);

	if (buffer && size > 0)
	{
		short length = _snprintf(buffer, size, "|n|n|n|ngamepad|tleft stick|tright stick|t|n");
		short gamepad_index;

		for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
		{
			if (input_globals.gamepad_handles[gamepad_index])
			{
				length += _snprintf(&buffer[length], size - length, "gamepad %d|t(%d, %d)|t(%d, %d)|n",
					gamepad_index,
					gamepad_raw_sticks[gamepad_index][_gamepad_stick_left].x,
					gamepad_raw_sticks[gamepad_index][_gamepad_stick_left].y,
					gamepad_raw_sticks[gamepad_index][_gamepad_stick_right].x,
					gamepad_raw_sticks[gamepad_index][_gamepad_stick_right].y);
			}
		}
	}

	return;
}

void input_update(
	void)
{
	input_globals.suppressed = FALSE;

	if (!input_globals.rumble_thread_resumed)
	{
		ResumeThread(input_globals.rumble_thread);
		input_globals.rumble_thread_resumed = TRUE;
	}

	input_update_keyboard();
	acquire_input_mutex();

	HATRun(&input_globals.gamepads[0]);
	HATRun(&input_globals.gamepads[1]);
	HATRun(&input_globals.gamepads[2]);
	HATRun(&input_globals.gamepads[3]);

	return;
}

void input_frame_begin(
	void)
{
	release_input_mutex();
	input_update_gamepads();
	input_globals.in_frame = TRUE;

	return;
}

void input_frame_end(
	void)
{
	input_globals.in_frame = FALSE;

	return;
}

static DWORD WINAPI input_update_thread_proc(
	LPVOID parameter)
{
	for (;;)
	{
		WaitForSingleObject(input_globals.rumble_event, INFINITE);
		input_update_gamepads_rumble();
	}
}
