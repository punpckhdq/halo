/*
INPUT_ABSTRACTION.C
*/

/* ---------- headers */

#include "cseries.h"
#include "input_abstraction.h"
#include "network_game_globals.h"
#include "players.h"
#include "console.h"
#include "network_messages.h"
#include "text_group.h"
#include "vehicles.h"
#include "main.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "ui_widget.h"
#include "network_client_manager.h"
#include "terminal.h"
#include "player_ui.h"
#include "shell.h"
#include "bink_playback.h"
#include "virtual_keyboard.h"

/* ---------- constants */

#define DEFAULT_PITCH_RATE 60.f  // [fakename]
#define DEFAULT_YAW_RATE   120.f // [fakename]

/* ---------- macros */

/* ---------- structures */

struct _input_abstraction_globals
{
	struct game_input_preferences player_control_settings[MAXIMUM_GAMEPADS];
	struct game_input_state input_state[MAXIMUM_GAMEPADS];
	unsigned long device_enumeration_startup_timer;
	boolean controller_available[MAXIMUM_GAMEPADS];
	boolean initialized;
};

/* ---------- prototypes */

static boolean local_player_is_piloting_aircraft(short controller_index);
static void input_abstraction_initialize_player_controller_settings_to_default(struct game_input_preferences *preferences);

/* ---------- globals */

struct _input_abstraction_globals input_abstraction_globals = {0};

/* ---------- public code */

void input_abstraction_dispose(
	void)
{
	memset(&input_abstraction_globals, 0, sizeof(input_abstraction_globals));

	return;
}

void input_abstraction_reset_controller_detection_timer(
	void)
{
	input_abstraction_globals.device_enumeration_startup_timer = system_milliseconds();

	return;
}

void input_abstraction_get_local_player_preferences(
	short local_player_index,
	struct game_input_preferences *preferences)
{
	match_assert("c:\\halo\\SOURCE\\input\\input_abstraction.c", 495, (local_player_index>=0) && (local_player_index<MAXIMUM_GAMEPADS));
	match_assert("c:\\halo\\SOURCE\\input\\input_abstraction.c", 496, preferences);

	memcpy(preferences, &input_abstraction_globals.player_control_settings[local_player_index], sizeof(struct game_input_preferences));

	return;
}

void input_abstraction_update_local_player_preferences(
	short controller_index,
	struct game_input_preferences *preferences)
{
	match_assert("c:\\halo\\SOURCE\\input\\input_abstraction.c", 507, (controller_index>=0) && (controller_index<MAXIMUM_GAMEPADS));
	match_assert("c:\\halo\\SOURCE\\input\\input_abstraction.c", 508, preferences);
	match_vassert("c:\\halo\\SOURCE\\input\\input_abstraction.c", 511,
		preferences->game_control_to_xbox_buttons[_button_start]==_gamepad_binary_button_start && preferences->game_control_to_xbox_buttons[_button_back]==_gamepad_binary_button_back,
		"invalid controller preferences; can't remap start & back buttons");

	memcpy(&input_abstraction_globals.player_control_settings[controller_index], preferences, sizeof(struct game_input_preferences));

	return;
}

struct game_input_state *input_abstraction_get_input_state(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\input\\input_abstraction.c", 521, (local_player_index>=0) && (local_player_index<MAXIMUM_GAMEPADS));

	return &input_abstraction_globals.input_state[local_player_index];
}

void input_abstraction_update_device_changes(
	unsigned long device_change_flags)
{
	static unsigned long time_of_first_device_insertion = 0;

	if (input_abstraction_globals.initialized)
	{
		if (device_change_flags &&
			(system_milliseconds() - input_abstraction_globals.device_enumeration_startup_timer >= 2*MILLISECONDS_PER_SECOND ||
			(time_of_first_device_insertion && system_milliseconds() - time_of_first_device_insertion >= 2*MILLISECONDS_PER_SECOND)))
		{
			error(_error_silent, "stopping bink playback to due to change in input devices");
			bink_playback_stop();
		}

		if ((device_change_flags & DEVICE_CHANGE_INSERTED_MASK) && !time_of_first_device_insertion)
		{
			time_of_first_device_insertion = system_milliseconds();
		}
	}

	return;
}

static boolean local_player_is_piloting_aircraft(
	short controller_index)
{
	boolean result = FALSE;
	long player_index;

	match_assert("c:\\halo\\SOURCE\\input\\input_abstraction.c", 569, (controller_index>=0) && (controller_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	player_index = local_player_get_player_index(controller_index);

	if (player_index!=NONE)
	{
		struct player_datum *player = player_try_and_get(player_index);

		if (player)
		{
			struct unit_datum *unit = unit_try_and_get(player->unit_index);

			if (unit && unit->object.parent_object_index!=NONE && unit->unit.parent_seat_index!=NONE)
			{
				struct object_datum *vehicle = (struct object_datum *)object_get_and_verify_type(unit->object.parent_object_index, _object_mask_vehicle);
				struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);

				if (definition->vehicle.type==_vehicle_human_plane || definition->vehicle.type==_vehicle_alien_fighter)
				{
					struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

					if (TEST_FLAG(seat->flags, _unit_seat_is_driver_bit))
					{
						result = TRUE;
					}
				}
			}
		}
	}

	return result;
}

static void input_abstraction_initialize_player_controller_settings_to_default(
	struct game_input_preferences *preferences)
{
	preferences->pitch_rate = DEFAULT_PITCH_RATE;
	preferences->yaw_rate = DEFAULT_YAW_RATE;
	preferences->game_control_to_xbox_buttons[_button_jump] = _gamepad_analog_button_a;
	preferences->game_control_to_xbox_buttons[_button_switch_grenade] = _gamepad_analog_button_black;
	preferences->game_control_to_xbox_buttons[_button_action_reload] = _gamepad_analog_button_x;
	preferences->game_control_to_xbox_buttons[_button_switch_weapon] = _gamepad_analog_button_y;
	preferences->game_control_to_xbox_buttons[_button_melee_attack] = _gamepad_analog_button_b;
	preferences->game_control_to_xbox_buttons[_button_flashlight] = _gamepad_analog_button_white;
	preferences->game_control_to_xbox_buttons[_button_throw_grenade] = _gamepad_analog_button_left_trigger;
	preferences->game_control_to_xbox_buttons[_button_fire] = _gamepad_analog_button_right_trigger;
	preferences->game_control_to_xbox_buttons[_button_start] = _gamepad_binary_button_start;
	preferences->game_control_to_xbox_buttons[_button_back] = _gamepad_binary_button_back;
	preferences->game_control_to_xbox_buttons[_button_crouch] = _gamepad_binary_button_left_thumb;
	preferences->game_control_to_xbox_buttons[_button_scope_zoom] = _gamepad_binary_button_right_thumb;
	preferences->joystick_controls = _joystick_preset_standard;
	preferences->invert_look = FALSE;
	preferences->invert_look_aircraft_control = FALSE;

	return;
}

void input_abstraction_initialize(
	void)
{
	long controller_index;

	memset(&input_abstraction_globals, 0, sizeof(input_abstraction_globals));

	for (controller_index = 0; controller_index < MAXIMUM_GAMEPADS; controller_index++)
	{
		input_abstraction_initialize_player_controller_settings_to_default(&input_abstraction_globals.player_control_settings[controller_index]);
		input_abstraction_globals.controller_available[controller_index] = input_has_gamepad((short)controller_index);
	}

	input_abstraction_reset_controller_detection_timer();
	input_abstraction_globals.initialized = TRUE;

	return;
}

void input_abstraction_update(
	void)
{
	real const stick_diagonal_angle = DEGREES_TO_RADIANS(45);
	real const stick_second_quadrant_diagonal_angle = DEGREES_TO_RADIANS(135);
	real const right_stick_diagonal_snap_angle = DEGREES_TO_RADIANS(10);
	real const left_stick_diagonal_snap_angle = stick_diagonal_angle - right_stick_diagonal_snap_angle;
	real const stick_diagonal_blend_scale = 1.f / left_stick_diagonal_snap_angle;
	long controller_index;

	TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_control, 0, struct game_globals_player_control);

	for (controller_index = 0; controller_index < MAXIMUM_GAMEPADS; controller_index++)
	{
		struct gamepad_state const *gamepad = input_get_gamepad_state((short)controller_index);

		if (gamepad)
		{
			static real const one_over_short_max = 1.f/SHORT_MAX;
			static real const reference_values[4] = { _pi/4.f, 3.f*_pi/4.f, -_pi/4.f, -3.f*_pi/4.f };
			struct game_input_state *state = &input_abstraction_globals.input_state[controller_index];
			real left_angle;
			real right_angle;
			real_point2d left_stick;
			real_point2d right_stick;
			long control_index;
			boolean invert_look;

			player_look_yaw_rate[controller_index] = input_abstraction_globals.player_control_settings[controller_index].yaw_rate;
			player_look_pitch_rate[controller_index] = input_abstraction_globals.player_control_settings[controller_index].pitch_rate;

			{
				real scale;

				left_angle = arctangent(gamepad->sticks[_gamepad_stick_left].y, gamepad->sticks[_gamepad_stick_left].x);
				scale = 1.0 / MAX(fabs(sine(left_angle)), fabs(cosine(left_angle)));
				left_stick.x = PIN(gamepad->sticks[_gamepad_stick_left].x * one_over_short_max * scale, -1.f, 1.f);
				left_stick.y = PIN(gamepad->sticks[_gamepad_stick_left].y * one_over_short_max * scale, -1.f, 1.f);
			}

			{
				real scale;

				right_angle = arctangent(gamepad->sticks[_gamepad_stick_right].y, gamepad->sticks[_gamepad_stick_right].x);
				scale = 1.0 / MAX(fabs(sine(right_angle)), fabs(cosine(right_angle)));
				right_stick.x = PIN(gamepad->sticks[_gamepad_stick_right].x * one_over_short_max * scale, -1.f, 1.f);
				right_stick.y = PIN(gamepad->sticks[_gamepad_stick_right].y * one_over_short_max * scale, -1.f, 1.f);
			}

			for (control_index = 0; control_index < NUMBER_OF_ACTION_CONTROL_BUTTONS; control_index++)
			{
				state->buttons[control_index] =
					gamepad->buttons[input_abstraction_globals.player_control_settings[controller_index].game_control_to_xbox_buttons[control_index]];
			}

			if (input_abstraction_globals.player_control_settings[controller_index].joystick_controls == _joystick_preset_legacy ||
				input_abstraction_globals.player_control_settings[controller_index].joystick_controls == _joystick_preset_legacy_south_paw)
			{
				short left_quadrant = (left_stick.x < 0.f ? 1 : 0) | (left_stick.y < 0.f ? 2 : 0);
				short right_quadrant = (right_stick.x < 0.f ? 1 : 0) | (right_stick.y < 0.f ? 2 : 0);
				real left_difference = left_angle - reference_values[left_quadrant];
				real right_difference = right_angle - reference_values[right_quadrant];
				real left_magnitude = square_root(left_stick.x * left_stick.x + left_stick.y * left_stick.y);
				real right_magnitude = square_root(right_stick.x * right_stick.x + right_stick.y * right_stick.y);

				if (fabs(left_difference) < left_stick_diagonal_snap_angle)
				{
					real absolute_angle = fabs(left_angle);

					if (absolute_angle < stick_diagonal_angle ||
						absolute_angle > stick_second_quadrant_diagonal_angle)
					{
						left_stick.x = (left_stick.x < 0.f ? -1 : 1) * left_magnitude;
						left_stick.y = (left_stick.y < 0.f ? -1 : 1) * left_magnitude *
							(1.0 - fabs(left_difference) * stick_diagonal_blend_scale);
					}
					else
					{
						left_stick.y = (left_stick.y < 0.f ? -1 : 1) * left_magnitude;
						left_stick.x = (left_stick.x < 0.f ? -1 : 1) * left_magnitude *
							(1.0 - fabs(left_difference) * stick_diagonal_blend_scale);
					}
				}
				else
				{
					if (fabs(left_stick.x) > fabs(left_stick.y))
					{
						left_stick.x = (left_stick.x < 0.f ? -1 : 1) * left_magnitude;
						left_stick.y = 0.f;
					}
					else
					{
						left_stick.y = (left_stick.y < 0.f ? -1 : 1) * left_magnitude;
						left_stick.x = 0.f;
					}
				}

				if (fabs(right_difference) < right_stick_diagonal_snap_angle)
				{
					real absolute_angle = fabs(right_angle);

					if (absolute_angle < stick_diagonal_angle ||
						absolute_angle > stick_second_quadrant_diagonal_angle)
					{
						right_stick.x = (right_stick.x < 0.f ? -1 : 1) * right_magnitude;
						right_stick.y = (right_stick.y < 0.f ? -1 : 1) * right_magnitude *
							(1.0 - fabs(right_difference) * stick_diagonal_blend_scale);
					}
					else
					{
						right_stick.y = (right_stick.y < 0.f ? -1 : 1) * right_magnitude;
						right_stick.x = (right_stick.x < 0.f ? -1 : 1) * right_magnitude *
							(1.0 - fabs(right_difference) * stick_diagonal_blend_scale);
					}
				}
				else
				{
					if (fabs(right_stick.x) > fabs(right_stick.y))
					{
						right_stick.x = (right_stick.x < 0.f ? -1 : 1) * right_magnitude;
						right_stick.y = 0.f;
					}
					else
					{
						right_stick.y = (right_stick.y < 0.f ? -1 : 1) * right_magnitude;
						right_stick.x = 0.f;
					}
				}
			}

			invert_look = input_abstraction_globals.player_control_settings[controller_index].invert_look;

			if (!invert_look && input_abstraction_globals.player_control_settings[controller_index].invert_look_aircraft_control)
			{
				invert_look = local_player_is_piloting_aircraft((short)controller_index);
			}

			switch (input_abstraction_globals.player_control_settings[controller_index].joystick_controls)
			{
			case _joystick_preset_standard:
				if (gamepad->buttons[_gamepad_binary_button_dpad_left])
				{
					state->strafe = 1.f;
				}
				else if (gamepad->buttons[_gamepad_binary_button_dpad_right])
				{
					state->strafe = -1.f;
				}
				else
				{
					state->strafe = -left_stick.x;
				}

				if (gamepad->buttons[_gamepad_binary_button_dpad_up])
				{
					state->forward_movement = 1.f;
				}
				else if (gamepad->buttons[_gamepad_binary_button_dpad_down])
				{
					state->forward_movement = -1.f;
				}
				else
				{
					state->forward_movement = left_stick.y;
				}

				state->yaw = -right_stick.x;
				state->pitch = (invert_look ? -1.f : 1.f) * right_stick.y;
				break;
			case _joystick_preset_south_paw:
				if (gamepad->buttons[_gamepad_binary_button_dpad_left])
				{
					state->yaw = 1.f;
				}
				else if (gamepad->buttons[_gamepad_binary_button_dpad_right])
				{
					state->yaw = -1.f;
				}
				else
				{
					state->yaw = -left_stick.x;
				}

				if (gamepad->buttons[_gamepad_binary_button_dpad_up])
				{
					state->pitch = invert_look ? -1.f : 1.f;
				}
				else if (gamepad->buttons[_gamepad_binary_button_dpad_down])
				{
					state->pitch = invert_look ? 1.f : -1.f;
				}
				else
				{
					state->pitch = (invert_look ? -1.f : 1.f) * left_stick.y;
				}

				state->forward_movement = right_stick.y;
				state->strafe = -right_stick.x;
				break;
			case _joystick_preset_legacy:
				if (gamepad->buttons[_gamepad_binary_button_dpad_left])
				{
					state->yaw = 1.f;
				}
				else if (gamepad->buttons[_gamepad_binary_button_dpad_right])
				{
					state->yaw = -1.f;
				}
				else
				{
					state->yaw = -left_stick.x;
				}

				if (gamepad->buttons[_gamepad_binary_button_dpad_up])
				{
					state->forward_movement = 1.f;
				}
				else if (gamepad->buttons[_gamepad_binary_button_dpad_down])
				{
					state->forward_movement = -1.f;
				}
				else
				{
					state->forward_movement = left_stick.y;
				}

				state->strafe = -right_stick.x;
				state->pitch = (invert_look ? -1.f : 1.f) * right_stick.y;
				break;
			case _joystick_preset_legacy_south_paw:
				if (gamepad->buttons[_gamepad_binary_button_dpad_left])
				{
					state->strafe = 1.f;
				}
				else if (gamepad->buttons[_gamepad_binary_button_dpad_right])
				{
					state->strafe = -1.f;
				}
				else
				{
					state->strafe = -left_stick.x;
				}

				if (gamepad->buttons[_gamepad_binary_button_dpad_up])
				{
					state->pitch = invert_look ? -1.f : 1.f;
				}
				else if (gamepad->buttons[_gamepad_binary_button_dpad_down])
				{
					state->pitch = invert_look ? 1.f : -1.f;
				}
				else
				{
					state->pitch = (invert_look ? -1.f : 1.f) * left_stick.y;
				}

				state->forward_movement = right_stick.y;
				state->yaw = -right_stick.x;
				break;
			default:
				error(_error_silent, "unknown joystick preset");
				break;
			}

			input_abstraction_globals.controller_available[controller_index] = TRUE;
		}
		else
		{
			if (input_abstraction_globals.controller_available[controller_index])
			{
				short error_controller = (short)controller_index;
				short error_code;
				boolean pause_game;
				boolean show_error = TRUE;

				if (main_menu_is_active())
				{
					long available_controllers = 0;
					long index = 0;

					do
					{
						if (input_abstraction_globals.controller_available[index])
						{
							available_controllers++;
						}

						index++;
					}
					while (index < MAXIMUM_GAMEPADS);

					pause_game = FALSE;
					error_code = _error_controller_unplugged;

					if (available_controllers >= 2)
					{
						if (player_ui_get_single_player_local_player_controller(0) != controller_index &&
							player_ui_get_single_player_local_player_controller(1) != controller_index &&
							player_ui_get_single_player_local_player_controller(2) != controller_index &&
							player_ui_get_single_player_local_player_controller(3) != controller_index &&
							!player_ui_local_player_wants_to_play_multiplayer((short)controller_index))
						{
							show_error = FALSE;
						}
					}
					else
					{
						if (player_ui_get_single_player_local_player_controller(0) != controller_index &&
							player_ui_get_single_player_local_player_controller(1) != controller_index &&
							player_ui_get_single_player_local_player_controller(2) != controller_index &&
							player_ui_get_single_player_local_player_controller(3) != controller_index &&
							!player_ui_local_player_wants_to_play_multiplayer((short)controller_index))
						{
							error_controller = NONE;
						}
					}
				}
				else if (global_network_game_client_get())
				{
					pause_game = FALSE;
					error_code = _error_controller_unplugged;
					show_error = local_player_exists(controller_index);
				}
				else
				{
					pause_game = TRUE;
					error_code = _error_controller_unplugged_start_to_continue;
					show_error = local_player_exists(controller_index);
				}

				if (show_error == TRUE)
				{
					if (virtual_keyboard_active())
					{
						virtual_keyboard_close();
					}
					display_error_deferred(error_code, error_controller, pause_game, pause_game);
				}
			}

			input_abstraction_globals.controller_available[controller_index] = FALSE;
		}
	}

	return;
}
