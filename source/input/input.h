/*
INPUT.H
*/

#ifndef __INPUT_H
#define __INPUT_H
#pragma once

/* ---------- constants */

enum
{
	MAXIMUM_GAMEPADS = 4,
	MEMORY_UNIT_SLOTS_PER_GAMEPAD= 2,
	MAXIMUM_GAMEPAD_MEMORY_UNITS= 8,

	_memory_unit_hard_drive= 0,
	_memory_unit_controller0_top,
	_memory_unit_controller0_bottom,
	_memory_unit_controller1_top,
	_memory_unit_controller1_bottom,
	_memory_unit_controller2_top,
	_memory_unit_controller2_bottom,
	_memory_unit_controller3_top,
	_memory_unit_controller3_bottom,
	NUMBER_OF_MEMORY_UNITS
};

enum
{
	_key_escape = 0,
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
	_key_print_screen,
	_key_scroll_lock,
	_key_pause,
	_key_backquote,
	_key_1,
	_key_2,
	_key_3,
	_key_4,
	_key_5,
	_key_6,
	_key_7,
	_key_8,
	_key_9,
	_key_0,
	_key_dash,
	_key_equal,
	_key_backspace,
	_key_tab,
	_key_q,
	_key_w,
	_key_e,
	_key_r,
	_key_t,
	_key_y,
	_key_u,
	_key_i,
	_key_o,
	_key_p,
	_key_left_bracket,
	_key_right_bracket,
	_key_backslash,
	_key_caps_lock,
	_key_a,
	_key_s,
	_key_d,
	_key_f,
	_key_g,
	_key_h,
	_key_j,
	_key_k,
	_key_l,
	_key_semicolon,
	_key_apostrophe,
	_key_return,
	_key_left_shift,
	_key_z,
	_key_x,
	_key_c,
	_key_v,
	_key_b,
	_key_n,
	_key_m,
	_key_comma,
	_key_period,
	_key_forwardslash,
	_key_right_shift,
	_key_left_control,
	_key_left_windows,
	_key_left_alt,
	_key_space,
	_key_right_alt,
	_key_right_windows,
	_key_menu,
	_key_right_control,
	_key_up_arrow,
	_key_down_arrow,
	_key_left_arrow,
	_key_right_arrow,
	_key_insert,
	_key_home,
	_key_page_up,
	_key_delete,
	_key_end,
	_key_page_down,
	_keypad_num_lock,
	_keypad_divide,
	_keypad_multiply,
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
	_keypad_subtract,
	_keypad_add,
	_keypad_enter,
	_keypad_decimal,
	NUMBER_OF_KEYS,

	_key_shift,
	_key_control,
	_key_windows,
	_key_alt,
};

enum
{
	FIRST_GAMEPAD_ANALOG_BUTTON= 0,
	_gamepad_analog_button_a= 0,
	_gamepad_analog_button_b,
	_gamepad_analog_button_x,
	_gamepad_analog_button_y,
	_gamepad_analog_button_black,
	_gamepad_analog_button_white,
	_gamepad_analog_button_left_trigger,
	_gamepad_analog_button_right_trigger,
	NUMBER_OF_GAMEPAD_ANALOG_BUTTONS,

	FIRST_GAMEPAD_BINARY_BUTTON= 8,
	_gamepad_binary_button_dpad_up= 8,
	_gamepad_binary_button_dpad_down,
	_gamepad_binary_button_dpad_left,
	_gamepad_binary_button_dpad_right,
	_gamepad_binary_button_start,
	_gamepad_binary_button_back,
	_gamepad_binary_button_left_thumb,
	_gamepad_binary_button_right_thumb,
	NUMBER_OF_GAMEPAD_BUTTONS,

	NUMBER_OF_GAMEPAD_BINARY_BUTTONS= (NUMBER_OF_GAMEPAD_BUTTONS-FIRST_GAMEPAD_BINARY_BUTTON)
};

enum
{
	_gamepad_stick_left= 0,
	_gamepad_stick_right,
	NUMBER_OF_GAMEPAD_STICKS
};

/* ---------- macros */

/* ---------- structures */

struct key_stroke
{
	unsigned char modifier_flags;
	unsigned char ascii_code;
	short key_code;
};

struct vibrate_data
{
	unsigned short left_frequency;
	unsigned short right_frequency;
};

struct gamepad_state
{
	unsigned char analog_buttons[NUMBER_OF_GAMEPAD_ANALOG_BUTTONS];
	unsigned char analog_button_thresholds[NUMBER_OF_GAMEPAD_ANALOG_BUTTONS];
	byte buttons[NUMBER_OF_GAMEPAD_BUTTONS];
	point2d sticks[NUMBER_OF_GAMEPAD_STICKS];
};

/* ---------- prototypes/INPUT_XBOX.C */

void input_dispose();
void input_initialize();

/* ---------- globals */

/* ---------- public code */

#endif // __INPUT_H
