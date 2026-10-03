/*
DIRECTOR.C
*/

/* ---------- headers */

#include "cseries.h"
#include "director.h"
#include "observer.h"
#include "camera_scripting.h"
#include "dead_camera.h"
#include "first_person_camera.h"
#include "flying_camera.h"
#include "orbiting_camera.h"
#include "editor_flying_camera.h"
#include "following_camera.h"
#include "static_camera.h"
#include "bored_camera.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "input.h"
#include "console.h"
#include "game_state.h"
#include "input_abstraction.h"
#include "main.h"
#include "editor_stubs.h"

/* ---------- prototypes */

static struct director *director_get(
	short local_player_index);
static void director_set_camera(
	short local_player_index,
	void (*camera_proc)(void *, struct camera_control const *, struct observer_command *),
	boolean interpolate);
static void director_initialize_variables(
	short local_player_index);
static void director_process_variables(
	short local_player_index,
	long control_bits,
	real speed_delta);
static void director_rotate_cameras(
	short local_player_index,
	short const *cameras,
	short camera_count);
static void director_choose_game_perspective(
	short local_player_index,
	boolean force);
static void director_choose_camera_game(
	short local_player_index,
	boolean initialize,
	boolean key);
static void director_choose_camera_editor(
	short local_player_index,
	boolean initialize,
	boolean key);
static void director_choose_camera_script_camera_record(
	short local_player_index,
	boolean initialize,
	boolean key);
static boolean director_update_controls(
	short local_player_index,
	struct camera_control *controls);
static void director_choose_camera(
	short local_player_index,
	boolean initialize,
	boolean key);

/* ---------- globals */

static struct
{
	real dtime;
	short game_mode;
	boolean initialize_camera;
	struct director local_players[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
} director_globals;
boolean director_camera_switch_fast = FALSE;
static boolean hyper_key_down = FALSE;

static char const *director_camera_mode_names[NUMBER_OF_DIRECTOR_CAMERA_MODES + 1] =
{
	"following",
	"orbiting",
	"flying",
	"editor",
	"first person",
};

static struct director_variable_definition variables[NUMBER_OF_DIRECTOR_VARIABLES] =
{
	{ _camera_control_down_bit, _camera_control_up_bit, NONE, 0.15f, 0.f, -REAL_MAX, REAL_MAX, TRUE },
	{ _camera_control_roll_left_bit, _camera_control_roll_right_bit, NONE, 0.075f, 0.f, -REAL_MAX, REAL_MAX, FALSE },
	{ _camera_control_reverse_bit, _camera_control_forward_bit, NONE, 0.075f, 0.f, -REAL_MAX, REAL_MAX, TRUE },
	{ _camera_control_right_bit, _camera_control_left_bit, NONE, 0.075f, 0.f, -REAL_MAX, REAL_MAX, TRUE },
};

static short const director_game_camera_modes[3] =
{
	_camera_first_person,
	_camera_flying,
	_camera_following,
};

static short const director_script_camera_record_camera_modes[4] =
{
	_camera_first_person,
	_camera_flying,
	_camera_following,
	_camera_orbiting,
};

static real const ticks_per_millisecond = 0.03f;
static real const friction = 5.f;
static real const acceleration_scale = 25.f;
static real const genius_boy = 1.3f;

/* ---------- public code */

void director_initialize(
	void)
{
	director_camera_scripted = game_state_malloc("director scripting", NULL, 4);
	*director_camera_scripted = FALSE;

	return;
}

void director_dispose(
	void)
{
	return;
}

void director_inhibit_facing(
	short local_player_index)
{
	director_get(local_player_index)->inhibited_facing = TRUE;

	return;
}

void director_inhibit_input(
	short local_player_index)
{
	director_get(local_player_index)->inhibited_input = TRUE;

	return;
}

boolean director_inhibited_facing(
	short local_player_index)
{
	return director_get(local_player_index)->inhibited_facing;
}

boolean director_inhibited_input(
	short local_player_index)
{
	return director_get(local_player_index)->inhibited_input;
}

void director_set_mode(
	short mode)
{
	match_assert("c:\\halo\\SOURCE\\camera\\director.c", 384, mode>=0 && mode<NUMBER_OF_DIRECTOR_GAME_MODES);

	if (director_globals.game_mode != mode)
	{
		director_globals.game_mode = mode;
		director_globals.initialize_camera = TRUE;
	}

	return;
}

void director_save_camera(
	void)
{
	FILE *file = fopen("d:\\camera.txt", "w");

	if (file)
	{
		struct observer_result const *camera = observer_get_camera(0);

		fprintf(file, "%f %f %f\n", camera->position.x, camera->position.y, camera->position.z);
		fprintf(file, "%f %f %f\n", camera->forward.i, camera->forward.j, camera->forward.k);
		fprintf(file, "%f %f %f\n", camera->up.i, camera->up.j, camera->up.k);
		fprintf(file, "%f\n", camera->field_of_view);
		fclose(file);
	}

	return;
}

short director_get_perspective(
	short local_player_index)
{
	struct director *director = director_get(local_player_index);

	if (director->camera_proc == first_person_camera_update)
	{
		if (director->camera_change_pause == 0.f)
		{
			director->perspective = _director_perspective_first_person;
		}
	}
	else if (director->camera_proc == following_camera_update)
	{
		director->perspective = _director_perspective_third_person;
	}
	else if (director->camera_proc == scripted_camera_update)
	{
		director->perspective = _director_perspective_scripted;
	}
	else
	{
		director->perspective = _director_perspective_neutral;
	}

	return director->perspective;
}

short director_desired_perspective(
	long unit_index,
	short *seat_state)
{
	short perspective = _director_perspective_first_person;

	*seat_state = _not_in_seat;
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->object.parent_object_index != NONE)
		{
			struct object_datum *parent = object_get_and_verify_type(unit->object.parent_object_index, NONE);
			boolean third_person_on_enter = FALSE;

			if (TEST_FLAG(_object_mask_unit, parent->object.type))
			{
				struct unit_seat const *seat = TAG_BLOCK_GET_ELEMENT(&unit_definition_get(parent->definition_index)->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

				third_person_on_enter = TEST_FLAG(seat->flags, _unit_seat_third_person_on_enter_bit);
				if (TEST_FLAG(seat->flags, _unit_seat_has_third_person_camera_bit))
				{
					perspective = _director_perspective_third_person;
				}
			}

			if (third_person_on_enter && unit->unit.animation.state == 0x1a)
			{
				*seat_state = _entering_seat;
			}
			else if (third_person_on_enter && unit->unit.animation.state == 0x1b)
			{
				*seat_state = _exiting_seat;
			}
			else
			{
				*seat_state = _seat_idle;
			}
		}
	}

	if (*seat_state == _entering_seat || *seat_state == _exiting_seat)
	{
		perspective = _director_perspective_third_person;
	}

	return perspective;
}

void director_dispose_from_old_map(
	void)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		director_set_camera(local_player_index, NULL, FALSE);
	}
	*director_camera_scripted = FALSE;

	return;
}

void director_load_camera(
	void)
{
	FILE *file = fopen("d:\\camera.txt", "r");

	if (file)
	{
		real_point3d position;
		real_vector3d forward;
		real_vector3d up;
		real_vector3d default_up;
		real_vector3d cross;
		real field_of_view;
		struct director *director = director_get(0);
		struct flying_camera *camera = (struct flying_camera *)director->camera_data;

		fscanf(file, "%f %f %f\n", &position.x, &position.y, &position.z);
		fscanf(file, "%f %f %f\n", &forward.i, &forward.j, &forward.k);
		fscanf(file, "%f %f %f\n", &up.i, &up.j, &up.k);
		fscanf(file, "%f\n", &field_of_view);
		fclose(file);

		flying_camera_new_from_point_and_vector(camera, &position, &forward);
		observer_up_from_forward(&forward, &default_up);
		camera->roll = angle_between_vectors3d(&up, &default_up);
		cross_product3d(&up, &default_up, &cross);
		if (dot_product3d(&cross, &forward) > 0.f)
		{
			camera->roll = -camera->roll;
		}
		camera->field_of_view = field_of_view;
		director_set_camera(0, flying_camera_update, FALSE);
		director->camera_mode_index = _camera_flying;
	}

	return;
}

short director_camera_deterministic(
	long unit_index,
	real_point3d *position,
	real_vector3d *forward)
{
	short seat_state;
	short perspective = director_desired_perspective(unit_index, &seat_state);

	if (perspective == _director_perspective_first_person)
	{
		first_person_camera_deterministic(unit_index, position, forward);
	}
	else
	{
		following_camera_deterministic(unit_index, position, forward);
	}

	return perspective;
}

void director_script_camera(
	boolean enabled)
{
	short local_player_index;

	*director_camera_scripted = enabled;
	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		director_get(local_player_index);
		if (enabled)
		{
			director_set_camera(local_player_index, scripted_camera_update, FALSE);
		}
		else
		{
			director_choose_game_perspective(local_player_index, TRUE);
		}
		scripted_camera_enable(enabled);
	}

	return;
}

void director_initialize_for_new_map(
	void)
{
	short local_player_index;

	director_globals.game_mode = game_in_editor() ? _director_mode_editor : _director_mode_game;
	director_globals.initialize_camera = FALSE;
	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		struct director *director = director_get(local_player_index);

		director->bored = FALSE;
		director->bored_time = 0;
		director->camera_change_pause = 0.f;
		director_choose_camera(local_player_index, TRUE, FALSE);
		director_initialize_variables(local_player_index);
	}

	return;
}

void director_update(
	real dt)
{
	short local_player_index;

	director_globals.dtime = dt;
	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		if (local_player_get_player_index(local_player_index) != NONE)
		{
			struct camera_control controls;
			struct observer_command command;
			struct director *director = director_get(local_player_index);
			boolean key;

			director->inhibited_facing = FALSE;
			director->inhibited_input = FALSE;
			key = director_update_controls(local_player_index, &controls);
			director_choose_camera(local_player_index, director_globals.initialize_camera, key);
			director_globals.initialize_camera = FALSE;
			memset(&command, 0, sizeof(command));
			if (director->camera_proc && (director->camera_proc != scripted_camera_update || local_player_index == local_player_get_next(NONE)))
			{
				director->camera_proc(director->camera_data, &controls, &command);
			}

			if (TEST_FLAG(command.flags, _observer_command_valid_bit))
			{
				if (director->camera_change_pause != 0.f)
				{
					if (director->camera_change_pause < 0.2f && director->camera_proc == first_person_camera_update)
					{
						director->camera_change_pause = 0.f;
						command.position_timer = 0.f;
						command.position_flags = FLAG(_observer_time_valid_bit) | FLAG(_observer_time_force_bit);
						command.distance_timer = 0.f;
						command.distance_flags = FLAG(_observer_time_valid_bit) | FLAG(_observer_time_force_bit);
					}
					else
					{
						command.timer = MAX(command.timer, director->camera_change_pause);
					}
					director->camera_change_pause = MAX(0.f, director->camera_change_pause - dt);
				}

				director->command = command;
			}
			else
			{
				director->command.flags &= ~FLAG(_observer_command_valid_bit);
			}
			observer_set_camera(local_player_index, &director->command);
		}
	}

	return;
}

void director_initialize_for_saved_game(
	void)
{
	director_initialize_for_new_map();
	director_script_camera(*director_camera_scripted);

	return;
}

/* ---------- private code */

static struct director *director_get(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\camera\\director.c", 179, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);

	return &director_globals.local_players[local_player_index];
}

static void director_set_camera(
	short local_player_index,
	void (*camera_proc)(void *, struct camera_control const *, struct observer_command *),
	boolean interpolate)
{
	struct director *director = director_get(local_player_index);

	director->camera_proc = camera_proc;
	director->debug_input_scale = 1.f;
	director->debug_controls = FALSE;
	if (interpolate)
	{
		director->camera_change_pause = 1.f;
	}

	return;
}

static void director_initialize_variables(
	short local_player_index)
{
	struct director *director = director_get(local_player_index);
	short variable_index;

	for (variable_index = 0; variable_index < NUMBER_OF_DIRECTOR_VARIABLES; variable_index++)
	{
		director->debug_variables[variable_index].value = variables[variable_index].initial_value;
		director->debug_variables[variable_index].delta = 0.f;
		director->debug_variables[variable_index].velocity = 0.f;
	}

	return;
}

static void director_process_variables(
	short local_player_index,
	long control_bits,
	real speed_delta)
{
	struct director *director = director_get(local_player_index);
	short variable_index;

	director->debug_input_scale *= pow(genius_boy, speed_delta);
	director->debug_input_scale = PIN(director->debug_input_scale, 0.01f, 50.f);
	for (variable_index = 0; variable_index < NUMBER_OF_DIRECTOR_VARIABLES; variable_index++)
	{
		struct director_variable_definition const *definition = variables + variable_index;
		struct director_variable_instance *instance = director->debug_variables + variable_index;
		real scale = variables[0].has_hyper_scale ? director->debug_input_scale : 1.f;
		real friction_scale = 1.f - PIN(friction * director_globals.dtime, 0.f, 1.f);
		boolean negative = definition->negative_bit != NONE && TEST_FLAG(control_bits, definition->negative_bit);
		boolean positive = definition->positive_bit != NONE && TEST_FLAG(control_bits, definition->positive_bit);
		boolean reset = definition->reset_bit != NONE && TEST_FLAG(control_bits, definition->reset_bit);

		instance->velocity *= friction_scale;
		if (negative && !positive)
		{
			instance->velocity -= acceleration_scale * definition->scale * director_globals.dtime * scale;
		}
		else if (positive && !negative)
		{
			instance->velocity += acceleration_scale * definition->scale * director_globals.dtime * scale;
		}
		else if (game_in_editor())
		{
			instance->velocity = 0.f;
		}

		instance->delta = director_globals.dtime * instance->velocity;
		if (reset)
		{
			instance->value = definition->initial_value;
		}
		else
		{
			instance->value += instance->delta;
		}
		instance->value = PIN(instance->value, definition->minimum, definition->maximum);
	}

	return;
}

static void director_rotate_cameras(
	short local_player_index,
	short const *cameras,
	short camera_count)
{
	struct director *director = director_get(local_player_index);

	director->camera_mode_index = (director->camera_mode_index + 1) % camera_count;
	switch (cameras[director->camera_mode_index])
	{
		case _camera_following:
			following_camera_new((struct following_camera *)director->camera_data);
			director_set_camera(local_player_index, following_camera_update, TRUE);
			break;

		case _camera_orbiting:
			orbiting_camera_new((struct orbiting_camera *)director->camera_data, director->command.focus_distance, &director->command.forward);
			director_set_camera(local_player_index, orbiting_camera_update, TRUE);
			break;

		case _camera_flying:
			flying_camera_new_from_point_and_vector((struct flying_camera *)director->camera_data, &director->command.focus_position, &director->command.forward);
			director_set_camera(local_player_index, flying_camera_update, TRUE);
			break;

		case _camera_editor:
			break;

		case _camera_first_person:
			first_person_camera_new((struct first_person_camera *)director->camera_data);
			director_set_camera(local_player_index, first_person_camera_update, TRUE);
			break;

		default:
			match_vassert("c:\\halo\\SOURCE\\camera\\director.c", 512, FALSE, NULL);
			break;
	}
	console_printf(FALSE, "%s camera", director_camera_mode_names[cameras[director->camera_mode_index]]);

	return;
}

static void director_choose_game_perspective(
	short local_player_index,
	boolean force)
{
	struct director *director = director_get(local_player_index);
	long unit_index = player_control_get_unit_index(local_player_index);
	short seat_state;
	short perspective = director_desired_perspective(unit_index, &seat_state);

	if (force || director->seat_state != seat_state)
	{
		if (perspective == _director_perspective_third_person)
		{
			if (force || director->camera_proc == first_person_camera_update)
			{
				following_camera_new((struct following_camera *)director->camera_data);
				director_set_camera(local_player_index, following_camera_update, !force);
			}
		}
		else if (force || director->camera_proc == following_camera_update)
		{
			first_person_camera_new((struct first_person_camera *)director->camera_data);
			director_set_camera(local_player_index, first_person_camera_update, !force);
		}
		director->seat_state = seat_state;
	}

	return;
}

static void director_choose_camera_game(
	short local_player_index,
	boolean initialize,
	boolean key)
{
	struct director *director = director_get(local_player_index);

	if (initialize)
	{
		first_person_camera_new((struct first_person_camera *)director->camera_data);
		director_set_camera(local_player_index, first_person_camera_update, FALSE);
	}
	else
	{
		struct player_datum *player = player_get(local_player_get_player_index(local_player_index));
		boolean dead = player->unit_index == NONE && player->statistics.deaths > 0;

		if (key)
		{
			director_rotate_cameras(local_player_index, director_game_camera_modes, NUMBEROF(director_game_camera_modes));
		}

		if (!*director_camera_scripted)
		{
			director_choose_game_perspective(local_player_index, initialize);
			if (dead)
			{
				if (director->camera_proc != dead_camera_update)
				{
					dead_camera_new((struct dead_camera *)director->camera_data, local_player_index, NONE);
					director_set_camera(local_player_index, dead_camera_update, TRUE);
				}
			}
			else if (director->camera_proc == dead_camera_update)
			{
				director_choose_game_perspective(local_player_index, TRUE);
			}
		}
	}

	return;
}

static void director_choose_camera_editor(
	short local_player_index,
	boolean initialize,
	boolean key)
{
	struct director *director = director_get(local_player_index);

	if (initialize || director->camera_proc != editor_camera_update)
	{
		editor_camera_new((struct flying_camera *)director->camera_data, local_player_index);
		director_set_camera(local_player_index, editor_camera_update, FALSE);
	}

	return;
}

static void director_choose_camera_script_camera_record(
	short local_player_index,
	boolean initialize,
	boolean key)
{
	if (initialize)
	{
		first_person_camera_new((struct first_person_camera *)director_get(local_player_index)->camera_data);
		director_set_camera(local_player_index, first_person_camera_update, FALSE);
	}
	else if (key)
	{
		director_rotate_cameras(local_player_index, director_script_camera_record_camera_modes, NUMBEROF(director_script_camera_record_camera_modes));
	}

	return;
}

static boolean director_update_controls(
	short local_player_index,
	struct camera_control *controls)
{
	struct director *director = director_get(local_player_index);
	boolean key = FALSE;
	struct player_datum *player;
	long gamepad_index;

	memset(controls, 0, sizeof(struct camera_control));
	controls->local_player_index = local_player_index;
	controls->seconds_elapsed = director_globals.dtime;
	player = player_get(local_player_get_player_index(local_player_index));
	gamepad_index = player->local_player_index;
	if (gamepad_index != NONE && input_has_gamepad(gamepad_index))
	{
		struct gamepad_state const *gamepad = input_get_gamepad_state(gamepad_index);

		if (director_camera_switch_fast)
		{
			key = gamepad->buttons[_gamepad_analog_button_black] == 1;
		}
		else
		{
			key = gamepad->buttons[_gamepad_analog_button_black] > 0 && gamepad->buttons[_gamepad_analog_button_black] % TICKS_PER_SECOND == 0;
		}

		if (director->camera_proc != first_person_camera_update && director->camera_proc != following_camera_update)
		{
			if (gamepad->buttons[_gamepad_binary_button_right_thumb] == 1)
			{
				director->debug_controls = !director->debug_controls;
			}

			if (director->debug_controls)
			{
				real const yaw_scale = -(_pi / 80000.f);
				real const pitch_scale = _pi / 160000.f;
				real const forward_scale = 5.0e-5f;
				real const side_scale = -5.0e-5f;
				long control_bits = 0;

				SET_FLAG(control_bits, _camera_control_up_bit, gamepad->buttons[_gamepad_analog_button_right_trigger]);
				SET_FLAG(control_bits, _camera_control_down_bit, gamepad->buttons[_gamepad_analog_button_left_trigger]);
				controls->wheel_delta = ((gamepad->buttons[_gamepad_binary_button_dpad_up] > 1) - (gamepad->buttons[_gamepad_binary_button_dpad_down] > 1)) * 0.4f;
				director_process_variables(local_player_index, control_bits, controls->wheel_delta);
				controls->facing_delta.yaw = gamepad->sticks[1].x * director_globals.dtime * yaw_scale;
				controls->facing_delta.pitch = gamepad->sticks[1].y * director_globals.dtime * pitch_scale;
				controls->position_delta.i = gamepad->sticks[0].y * director->debug_input_scale * director_globals.dtime * forward_scale;
				controls->position_delta.j = gamepad->sticks[0].x * director->debug_input_scale * director_globals.dtime * side_scale;
				controls->position_delta.k += director->debug_variables[_variable_height].delta;
				controls->active = TRUE;
				director_inhibit_input(local_player_index);
				director_inhibit_facing(local_player_index);
			}
		}
	}
	else if (input_get_mouse_state())
	{
		struct mouse_state const *mouse = input_get_mouse_state();

		key = input_key_is_down(_key_backspace) == 1;
		if ((director->camera_proc != first_person_camera_update && mouse->button_frames[_mouse_button_middle]) || input_key_is_down(_key_tab))
		{
			long control_bits = 0;

			SET_FLAG(control_bits, _camera_control_forward_bit, input_key_is_down(_key_w));
			SET_FLAG(control_bits, _camera_control_reverse_bit, input_key_is_down(_key_s));
			SET_FLAG(control_bits, _camera_control_left_bit, input_key_is_down(_key_a));
			SET_FLAG(control_bits, _camera_control_right_bit, input_key_is_down(_key_d));
			SET_FLAG(control_bits, _camera_control_up_bit, input_key_is_down(_key_r));
			SET_FLAG(control_bits, _camera_control_down_bit, input_key_is_down(_key_f));
			SET_FLAG(control_bits, _camera_control_roll_left_bit, input_key_is_down(_key_t));
			SET_FLAG(control_bits, _camera_control_roll_right_bit, input_key_is_down(_key_g));
			director_process_variables(local_player_index, control_bits, mouse->dw);
			controls->facing_delta.yaw = mouse->dx * -(_pi / 1000.f);
			controls->facing_delta.pitch = mouse->dy * -(_pi / 1000.f);
			controls->facing_delta.roll += director->debug_variables[_variable_roll].delta;
			controls->wheel_delta = mouse->dw;
			controls->position_delta.i += director->debug_variables[_variable_forward].delta;
			controls->position_delta.j += director->debug_variables[_variable_right].delta;
			controls->position_delta.k += director->debug_variables[_variable_height].delta;
			controls->active = TRUE;
			director_inhibit_input(local_player_index);
			director_inhibit_facing(local_player_index);
		}
	}

	return key;
}

static void director_choose_camera(
	short local_player_index,
	boolean initialize,
	boolean key)
{
	switch (director_globals.game_mode)
	{
		case _director_mode_game:
		case _director_mode_netgame:
			director_choose_camera_game(local_player_index, initialize, key);
			break;

		case _director_mode_editor:
			director_choose_camera_editor(local_player_index, initialize, key);
			break;

		case _director_mode_script_camera_record:
			director_choose_camera_script_camera_record(local_player_index, initialize, key);
			break;
	}

	return;
}
