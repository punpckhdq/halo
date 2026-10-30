/*
PLAYER_CONTROL.C

symbols in this file:
000A56F0 0050:
	_player_control_get (0000)
000A5740 0020:
	_player_control_initialize (0000)
000A5760 0010:
	_player_control_dispose (0000)
000A5770 0010:
	_player_control_dispose_from_old_map (0000)
000A5780 0020:
	_player_control_camera_control_is_active (0000)
000A57A0 0030:
	_scripted_player_control_set_camera_control (0000)
000A57D0 0060:
	_player_control_inhibit_buttons (0000)
000A5830 00f0:
	_evaluate_piecewise_linear_function (0000)
000A5920 0010:
	_clear_input_blob (0000)
000A5930 0060:
	_player_control_get_aiming_unit_index (0000)
000A5990 0070:
	_player_control_get_target_object_index (0000)
000A5A00 00b0:
	_player_control_get_field_of_view (0000)
000A5AB0 0130:
	_player_control_get_unit_camera_info (0000)
000A5BE0 0050:
	_player_control_get_unit_index (0000)
000A5C30 0080:
	_player_control_get_desired_weapon (0000)
000A5CB0 0050:
	_player_control_get_autoaim_level (0000)
000A5D00 0040:
	_unit_get_local_player_index (0000)
000A5D40 0050:
	_players_unzoom_all (0000)
000A5D90 0050:
	_player_control_unzoom (0000)
000A5DE0 0020:
	_player_control_get_zoom_level (0000)
000A5E00 0020:
	_player_control_action_test_reset (0000)
000A5E20 0020:
	_player_control_action_test_accept (0000)
000A5E40 0020:
	_player_control_action_test_back (0000)
000A5E60 0020:
	_player_control_action_test_action (0000)
000A5E80 0010:
	_player_control_action_test_jump (0000)
000A5E90 0010:
	_player_control_action_test_primary_trigger (0000)
000A5EA0 0010:
	_player_control_action_test_grenade_trigger (0000)
000A5EB0 0010:
	_player_control_action_test_zoom (0000)
000A5EC0 0020:
	_player_control_action_test_move_relative_all_directions (0000)
000A5EE0 0020:
	_player_control_action_test_look_relative_all_directions (0000)
000A5F00 0010:
	_player_control_action_test_look_relative_left (0000)
000A5F10 0010:
	_player_control_action_test_look_relative_right (0000)
000A5F20 0010:
	_player_control_action_test_look_relative_up (0000)
000A5F30 0010:
	_player_control_action_test_look_relative_down (0000)
000A5F40 0200:
	_player_control_action_test_check_reset_input_blob (0000)
000A6140 0040:
	_signed_angular_difference (0000)
000A6180 0050:
	_limit2d (0000)
000A61D0 0040:
	_interpolate_scalar (0000)
000A6210 0120:
	_player_control_set_facing (0000)
000A6330 00f0:
	_player_control_new_unit (0000)
000A6420 0d80:
	_get_local_player_input_blob (0000)
000A71A0 00e0:
	_player_control_get_facing_angles (0000)
000A7280 0030:
	_player_control_get_facing_direction (0000)
000A72B0 0050:
	_player_control_set_desired_weapon (0000)
000A7300 0630:
	_player_control_modify_desired_angles (0000)
000A7930 0090:
	_player_control_initialize_for_new_map (0000)
000A79C0 0620:
	_handle_one_player_input (0000)
000A7FE0 0080:
	_player_control_update (0000)
000A8060 0040:
	_player_control_permanent_impulse (0000)
0025C614 0016:
	??_C@_0BG@KLOMMKO@player_control_update?$AA@ (0000)
0025C62C 000c:
	_MOUSE_YAW_SCALE (0000)
	_MOUSE_PITCH_SCALE (0004)
0025C638 0025:
	??_C@_0CF@CDJDDDIC@c?3?2halo?2SOURCE?2game?2player_contr@ (0000)
0025C660 0017:
	??_C@_0BH@GDMLDNGJ@player?5control?5globals?$AA@ (0000)
0025C678 0046:
	??_C@_0EG@FFIEHMMD@function?5?$CG?$CG?5low_index?$DO?$DN0?5?$CG?$CG?5low_@ (0000)
0025C6C0 0004:
	__real@3f9c61aa (0000)
0025C6C4 000c:
	??_C@_0M@KHNJLLND@camera_info?$AA@ (0000)
0025C6D0 0004:
	__real@c0490fdb (0000)
0025C6D4 0023:
	??_C@_0CD@PHGHILFD@player_control?9?$DOdesired_angles?4y@ (0000)
0025C6F8 0025:
	??_C@_0CF@JIDICGJ@player_control?9?$DOdesired_angles?4p@ (0000)
0025C720 0017:
	??_C@_0BH@BJMHFMPG@input?9?$DOprimary_trigger?$AA@ (0000)
0025C738 0004:
	__real@3f75dcc7 (0000)
0025C73C 0004:
	__real@3d567750 (0000)
0025C740 0004:
	__real@bd567750 (0000)
0025C744 0004:
	__real@3dd67750 (0000)
0025C748 0004:
	__real@bdd67750 (0000)
0025C74C 0027:
	??_C@_0CH@OIFLGJJD@constants?9?$DOlook_acceleration_tim@ (0000)
0025C774 0021:
	??_C@_0CB@KGNPCEEG@constants?9?$DOlook_function?4count?$DO1@ (0000)
0025C798 002e:
	??_C@_0CO@HHADHJPE@valid_euler_angles2d?$CI?$CGplayer?9?$DOde@ (0000)
0025C7C8 0004:
	__real@bfbf0243 (0000)
0025C7CC 0004:
	__real@3fbf0243 (0000)
0025C7D0 0004:
	__real@3c490fdb (0000)
0025C7D4 0004:
	__real@bc490fdb (0000)
0025C7D8 0004:
	__real@3da3d70a (0000)
0025C7DC 001d:
	??_C@_0BN@JFNDAOJI@player?9?$DOdesired_angles?4pitch?$AA@ (0000)
0025C800 0008:
	__real@3fe45f3060000000 (0000)
0025C808 001a:
	??_C@_0BK@GKNPAGDI@action?4desired_facing?4yaw?$AA@ (0000)
0025C824 001c:
	??_C@_0BM@BMIDFCCA@action?4desired_facing?4pitch?$AA@ (0000)
0025C840 0018:
	??_C@_0BI@OPEALALJ@player?9?$DOprimary_trigger?$AA@ (0000)
0025C858 001b:
	??_C@_0BL@MOACFICP@player?9?$DOdesired_angles?4yaw?$AA@ (0000)
0025C874 0006:
	??_C@_05NCGEDJPM@delta?$AA@ (0000)
002DE808 0600:
	_player_autoaim_flag (0000)
	_player_magnetism_flag (0001)
	_controls_swapped (0002)
	_player_look_zoomed_scale (0004)
0043EE30 002b:
	_bss_0043ee30 (0000)
	_debug_input_target (0004)
	_player_look_yaw_rate (0008)
	_player_look_pitch_rate (0018)
	_controls_enable_crouch (0028)
	_controls_enable_doubled_spin (0029)
	_controls_swap_doubled_spin_state (002a)
*/

/* ---------- headers */

#include "cseries.h"
#include "players.h"
#include "network_game_globals.h"
#include "actor_definitions.h"
#include "input.h"
#include "console.h"
#include "game_state.h"
#include "network_messages.h"
#include "collisions.h"
#include "input_abstraction.h"
#include "text_group.h"
#include "vehicles.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "main.h"
#include "director.h"
#include "weapons.h"
#include "bipeds.h"
#include "observer.h"
#include "physics_constants.h"
#include "hud.h"
#include "player_ui.h"
#include "damage_effect_definitions.h"
#include "cinematics.h"
#include "static_camera.h"
#include "aim_assist.h"
#include "game.h"
#include "game_globals.h"
#include "units.h"
#include "unit_definitions.h"
#include "objects.h"
#include "real_math.h"
#include "profile.h"
#include "collision_usage.h"
#include "scenario.h"
#include "tag_groups.h"

/* ---------- constants */

enum
{
	_player_control_rotate_weapons_bit = 0,
	_player_control_rotate_grenades_bit,
	_player_control_zoom_bit,
	_player_control_debug_rotate_units_bit,
	_player_control_debug_rotate_all_units_bit,
	_player_control_debug_ninja_rope_bit,
	NUMBER_OF_PLAYER_CONTROL_FLAGS
};

enum
{
	_player_control_action_test_action_bit = 0,
	_player_control_action_test_jump_bit,
	_player_control_action_test_accept_bit,
	_player_control_action_test_back_bit,
	_player_control_action_test_primary_trigger_bit,
	_player_control_action_test_grenade_trigger_bit,
	_player_control_action_test_zoom_bit,
	_player_control_action_test_look_positive_pitch_bit,
	_player_control_action_test_look_negative_pitch_bit,
	_player_control_action_test_look_positive_yaw_bit,
	_player_control_action_test_look_negative_yaw_bit,
	_player_control_action_test_move_forward_bit,
	_player_control_action_test_move_backward_bit,
	_player_control_action_test_move_right_bit,
	_player_control_action_test_move_left_bit,
	NUMBER_OF_PLAYER_CONTROL_ACTION_TEST_FLAGS
};

enum
{
	_player_control_camera_off_bit = 0,
	NUMBER_OF_GLOBAL_PLAYER_CONTROL_FLAGS
};

/* ---------- macros */

#define MAXIMUM_PLAYER_PITCH DEGREES_TO_RADIANS(85.5f) /* fake name */

#define valid_euler_angles2d(angles) \
	(valid_real((angles)->pitch) && (angles)->pitch <= MAXIMUM_PLAYER_PITCH && (angles)->pitch >= -MAXIMUM_PLAYER_PITCH && \
	valid_real((angles)->yaw) && (angles)->yaw <= (_pi*2.f) && (angles)->yaw >= 0.f)

/* ---------- structures */

struct player_control
{
	long unit_index;
	unsigned long control_flags;
	word inhibited_button_bit_vector;
	word reset_button_when_released_bit_vector;
	real_euler_angles2d desired_angles;
	real_vector2d throttle;
	real primary_trigger;
	short desired_weapon_index;
	short desired_grenade_index;
	short desired_zoom_level;
	boolean use_autolevel;
	char autolevel_ticks;
	long target_object_index;
	real autoaim_level;
	real magnetism_level;
	real look_acceleration_time;
	real pitch_minimum;
	real pitch_maximum;
};

struct player_control_globals
{
	unsigned long action_test_flags;
	unsigned long testing_for_action_flags;
	unsigned long inhibit_button_flags;
	unsigned long control_flags;
	struct player_control players[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
};

struct input_blob
{
	real_vector2d throttle;
	real primary_trigger;
	real_euler_angles2d facing_delta;
	boolean accept;
	boolean back;
	unsigned long unit_control_flags;
	unsigned long player_control_flags;
};

/* ---------- prototypes */

__inline struct player_control *player_control_get(short local_player_index);
static void clear_input_blob(struct input_blob *input);
static void player_control_action_test_check_reset_input_blob(struct input_blob *input);
static void get_local_player_input_blob(short local_player_index, real seconds_elapsed, struct input_blob *input);
static void player_control_modify_desired_angles(short local_player_index, real delta_yaw, real delta_pitch);
static void handle_one_player_input(short local_player_index, real seconds_elapsed);

/* ---------- globals */

boolean player_autoaim_flag = TRUE;
boolean player_magnetism_flag = TRUE;
boolean controls_swapped = TRUE;
real player_look_zoomed_scale = 0.5f;

static struct profile_section player_control_update_section = {"player_control_update", NONE, TRUE}; /* fake name */

real const MOUSE_YAW_SCALE = 0.0031415927f;
real const MOUSE_PITCH_SCALE = 0.0031415927f;

static struct player_control_globals *player_control_globals = NULL;
short debug_input_target = 0;
real player_look_yaw_rate[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS] = { 0.f };
real player_look_pitch_rate[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS] = { 0.f };
boolean controls_enable_crouch = FALSE;
boolean controls_enable_doubled_spin = FALSE;
boolean controls_swap_doubled_spin_state = FALSE;

/* ---------- code */

__inline struct player_control *player_control_get(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\game\\player_control.c", 177, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);

	return &player_control_globals->players[local_player_index];
}

void player_control_set_facing(
	short local_player_index,
	real_vector3d const *forward)
{
	struct player_control *player_control = player_control_get(local_player_index);

	euler_angles2d_from_vector3d(&player_control->desired_angles, forward);
	match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 187, player_control->desired_angles.pitch);
	match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 188, player_control->desired_angles.yaw);

	if (player_control->desired_angles.yaw < 0.f)
	{
		player_control->desired_angles.yaw += (_pi*2.f);
	}

	return;
}

void player_control_initialize(
	void)
{
	player_control_globals = (struct player_control_globals *)game_state_malloc("player control globals", NULL, sizeof(*player_control_globals));

	return;
}

void player_control_dispose(
	void)
{
	return;
}

void player_control_initialize_for_new_map(
	void)
{
	short local_player_index;

	player_control_globals->testing_for_action_flags = 0;
	player_control_globals->action_test_flags = 0;
	player_control_globals->inhibit_button_flags = 0;
	player_control_globals->control_flags = 0;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		struct game_globals_player_control *constants = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_control, 0, struct game_globals_player_control);

		player_control_new_unit(local_player_index, NONE);

		if (player_look_pitch_rate[local_player_index] == 0.f)
		{
			player_look_pitch_rate[local_player_index] = constants->look_default_pitch_rate;
		}

		if (player_look_yaw_rate[local_player_index] == 0.f)
		{
			player_look_yaw_rate[local_player_index] = constants->look_default_yaw_rate;
		}
	}

	return;
}

void player_control_dispose_from_old_map(
	void)
{
	return;
}

void player_control_new_unit(
	short local_player_index,
	long unit_index)
{
	struct player_control *player_control = player_control_get(local_player_index);

	memset(player_control, 0, sizeof(*player_control));
	player_control->unit_index = unit_index;
	player_control->desired_weapon_index = NONE;
	player_control->desired_grenade_index = NONE;
	player_control->desired_zoom_level = NONE;
	player_control->use_autolevel = FALSE;
	player_control->target_object_index = NONE;
	player_control->pitch_maximum = MAXIMUM_PLAYER_PITCH;
	player_control->pitch_minimum = -MAXIMUM_PLAYER_PITCH;
	player_control->inhibited_button_bit_vector = 0;
	player_control->reset_button_when_released_bit_vector = 0;

	if (unit_index != NONE)
	{
		struct unit_datum *unit = (struct unit_datum *)object_get_and_verify_type(unit_index, _object_mask_unit);

		euler_angles2d_from_vector3d(&player_control->desired_angles, &unit->unit.desired_facing_vector);
		if (player_control->desired_angles.yaw < 0.f)
		{
			player_control->desired_angles.yaw += (_pi*2.f);
		}

		player_control->desired_weapon_index = unit->unit.desired_weapon_index;
		player_control->desired_grenade_index = unit->unit.desired_grenade_index;
		player_control->desired_zoom_level = unit->unit.desired_zoom_level;
		valid_euler_angles2d(&player_control->desired_angles);
	}

	return;
}

boolean player_control_camera_control_is_active(
	void)
{
	if (TEST_FLAG(player_control_globals->control_flags, _player_control_camera_off_bit) || game_time_get_paused())
	{
		return FALSE;
	}

	return TRUE;
}

boolean scripted_player_control_set_camera_control(
	boolean enabled)
{
	SET_FLAG(player_control_globals->control_flags, _player_control_camera_off_bit, !enabled);

	return enabled;
}

void player_control_inhibit_buttons(
	short local_player_index,
	word button_flags,
	boolean uninhibit_when_released)
{
	struct player_control *player_control = player_control_get(local_player_index);

	player_control->inhibited_button_bit_vector |= button_flags;
	if (uninhibit_when_released)
	{
		player_control->reset_button_when_released_bit_vector |= button_flags;
	}

	return;
}

real evaluate_piecewise_linear_function(
	short count,
	real *function,
	real x)
{
	boolean negative = x < 0.f;
	short low_index;
	short high_index;
	real result;

	x = (real)PIN(fabs(x) * (count - 1), 0, (real)count - 1.f);
	low_index = (short)x;
	low_index = PIN(low_index, 0, count - 1);
	high_index = MIN(low_index + 1, count - 1);
	match_assert("c:\\halo\\SOURCE\\game\\player_control.c", 331, function && low_index>=0 && low_index<=high_index && high_index<count);

	result = function[low_index] + (function[high_index] - function[low_index]) * (x - low_index);
	if (negative)
	{
		result = -result;
	}

	return result;
}

static void clear_input_blob(
	struct input_blob *input)
{
	memset(input, 0, sizeof(*input));

	return;
}

static void get_local_player_input_blob(
	short local_player_index,
	real seconds_elapsed,
	struct input_blob *input)
{
	static real const one_over_255 = 1.f / 255.f;
	long player_index = local_player_get_player_index(local_player_index);

	clear_input_blob(input);

	if (player_index != NONE)
	{
		struct player_control *player_control = player_control_get(local_player_index);
		struct player_datum *player = player_get(player_index);
		short gamepad_index = player->local_player_index;
		boolean debug_input = gamepad_index == debug_input_target;

		if (gamepad_index != NONE && input_has_gamepad(gamepad_index))
		{
			struct game_globals_player_control *constants = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_control, 0, struct game_globals_player_control);
			struct gamepad_state const *gamepad = input_get_gamepad_state(gamepad_index);
			struct game_input_state *state = input_abstraction_get_input_state(gamepad_index);
			real yaw_rate = 0.f;
			real pitch_rate = 0.f;
			real yaw;
			real pitch;

			if (player->unit_index != NONE)
			{
				struct unit_datum *unit = (struct unit_datum *)object_get_and_verify_type(player->unit_index, _object_mask_unit);

				yaw_rate = (real)(player_look_yaw_rate[local_player_index] * (_pi / 180.f)) / TICKS_PER_SECOND;
				pitch_rate = (real)(player_look_pitch_rate[local_player_index] * (_pi / 180.f)) / TICKS_PER_SECOND;

				if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
				{
					struct unit_datum *parent = (struct unit_datum *)object_get_and_verify_type(unit->object.parent_object_index, _object_mask_unit);
					struct unit_definition *definition = (struct unit_definition *)tag_get(UNIT_DEFINITION_TAG, parent->definition_index);
					struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

					if (seat->yaw_rate > 0.f)
					{
						yaw_rate = (real)(seat->yaw_rate * (_pi / 180.f)) / TICKS_PER_SECOND;
					}

					if (seat->pitch_rate > 0.f)
					{
						pitch_rate = (real)(seat->pitch_rate * (_pi / 180.f)) / TICKS_PER_SECOND;
					}
				}
			}

			input->throttle.i = state->forward_movement;
			input->throttle.j = state->strafe;

			{
				real abs_pitch = (real)fabs(state->pitch);
				real abs_yaw = (real)fabs(state->yaw);
				real look_scale = 1.f;

				if (abs_pitch > 0.1f && abs_yaw > 0.1f)
				{
					if (abs_pitch > abs_yaw)
					{
						abs_yaw /= abs_pitch;
						abs_pitch = 1.f;
					}
					else
					{
						abs_pitch /= abs_yaw;
						abs_yaw = 1.f;
					}

					look_scale = square_root(abs_pitch * abs_pitch + abs_yaw * abs_yaw);
				}

				yaw = PIN(look_scale * state->yaw, -1.f, 1.f);
				pitch = PIN(look_scale * state->pitch, -1.f, 1.f);
			}

			if (player_control_camera_control_is_active())
			{
				real_euler_angles2d angular_velocity;

				{
					real pitch_spin = (real)((state->buttons[_button_scope_zoom] && controls_enable_doubled_spin ? !controls_swap_doubled_spin_state : controls_swap_doubled_spin_state) + 1);
					real yaw_spin = (real)((state->buttons[_button_scope_zoom] && controls_enable_doubled_spin ? !controls_swap_doubled_spin_state : controls_swap_doubled_spin_state) + 1);

					match_assert("c:\\halo\\SOURCE\\game\\player_control.c", 456, constants->look_function.count>1);
					angular_velocity.yaw = evaluate_piecewise_linear_function((short)constants->look_function.count, (real *)constants->look_function.address, yaw) * yaw_spin * yaw_rate;
					angular_velocity.pitch = evaluate_piecewise_linear_function((short)constants->look_function.count, (real *)constants->look_function.address, pitch) * pitch_spin * pitch_rate;
				}

				if (player->unit_index != NONE && player_control->desired_zoom_level != NONE)
				{
					real magnification = unit_get_zoom_magnification(player->unit_index, player_control->desired_zoom_level);
					real zoom_scale = 1.f / magnification;

					angular_velocity.yaw *= zoom_scale;
					angular_velocity.pitch *= zoom_scale;
				}

				if (player->unit_index != NONE)
				{
					struct game_globals_player_information *player_information = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_information, 0, struct game_globals_player_information);
					struct unit_datum *unit = (struct unit_datum *)object_get_and_verify_type(player->unit_index, _object_mask_unit);
					real stun_scale = 1.f - unit->unit.body_stun * player_information->stun_turning_penalty;

					angular_velocity.yaw *= stun_scale;
					angular_velocity.pitch *= stun_scale;
				}

				match_assert("c:\\halo\\SOURCE\\game\\player_control.c", 483, constants->look_acceleration_time>0.0f);
				if (fabs(yaw) >= constants->look_pegging_threshold)
				{
					real acceleration = player_control->look_acceleration_time / constants->look_acceleration_time;

					angular_velocity.yaw *= (constants->look_acceleration_scale - 1.f) * PIN(acceleration, 0.f, 1.f) + 1.f;
					player_control->look_acceleration_time += seconds_elapsed;
				}
				else
				{
					player_control->look_acceleration_time = 0.f;
				}

				{
					real_euler_angles2d target_angular_position;
					real_euler_angles2d target_angular_velocity;

					player_control->target_object_index = local_player_aim_assist(local_player_index, &player_control->autoaim_level, &player_control->magnetism_level, &target_angular_position, &target_angular_velocity);
					if (player_magnetism_flag && player_control->magnetism_level > 0.f &&
						(fabs(yaw) > 0.0001f || fabs(pitch) > 0.0001f || fabs(input->throttle.i) > 0.0001f || fabs(input->throttle.j) > 0.0001f))
					{
						real game_speed = game_time_get_speed();
						real friction = 1.f - player_control->magnetism_level * PIN(constants->magnetism_friction, 0.f, 1.f);
						real adhesion = player_control->magnetism_level * PIN(constants->magnetism_adhesion, 0.f, 1.f);

						if (game_players_are_double_speed())
						{
							game_speed *= 0.5f;
						}

						target_angular_velocity.yaw *= game_speed;
						target_angular_velocity.pitch *= game_speed;
						target_angular_velocity.yaw = PIN(target_angular_velocity.yaw, -_pi / 30.f, _pi / 30.f);
						target_angular_velocity.pitch = PIN(target_angular_velocity.pitch, -_pi / 60.f, _pi / 60.f);
						angular_velocity.yaw = friction * angular_velocity.yaw + adhesion * target_angular_velocity.yaw;
						angular_velocity.pitch = friction * angular_velocity.pitch + adhesion * target_angular_velocity.pitch;
					}
				}

				input->facing_delta.yaw = angular_velocity.yaw * (seconds_elapsed * 30.f);
				input->facing_delta.pitch = angular_velocity.pitch * (seconds_elapsed * 30.f);
			}
			else
			{
				input->facing_delta.yaw = 0.f;
				input->facing_delta.pitch = 0.f;
			}

			{
				byte buttons[NUMBER_OF_ACTION_CONTROL_BUTTONS] = { 0 };
				word reset_buttons;
				long button_index;

				reset_buttons = player_control->inhibited_button_bit_vector & player_control->reset_button_when_released_bit_vector;
				if (reset_buttons)
				{
					long button_index;

					for (button_index = 0; button_index < NUMBER_OF_ACTION_CONTROL_BUTTONS; button_index++)
					{
						if (TEST_FLAG(reset_buttons, button_index) && !state->buttons[button_index])
						{
							SET_FLAG(player_control->inhibited_button_bit_vector, button_index, FALSE);
							SET_FLAG(player_control->reset_button_when_released_bit_vector, button_index, FALSE);
						}
					}
				}

				for (button_index = 0; button_index < NUMBER_OF_ACTION_CONTROL_BUTTONS; button_index++)
				{
					if (!TEST_FLAG(player_control->inhibited_button_bit_vector, button_index))
					{
						buttons[button_index] = state->buttons[button_index];
					}
				}

				if (player->unit_index != NONE)
				{
					struct biped_datum *biped = (struct biped_datum *)object_try_and_get_and_verify_type(player->unit_index, _object_mask_biped);

					if (biped && (controls_enable_crouch || TEST_FLAG(biped->biped.flags, _biped_airborne_bit) || magnitude_squared2d(&input->throttle) < 0.98f * 0.98f))
					{
						SET_FLAG(input->unit_control_flags, _unit_control_crouch_modifier_bit, buttons[_button_crouch]);
					}
				}

				input->primary_trigger = state->buttons[_button_fire] * one_over_255;
				SET_FLAG(input->unit_control_flags, _unit_control_weapon_primary_trigger_bit, buttons[_button_fire]);
				SET_FLAG(input->unit_control_flags, _unit_control_throw_grenade_bit, buttons[_button_throw_grenade]);
				SET_FLAG(input->unit_control_flags, _unit_control_weapon_secondary_trigger_bit, buttons[_button_throw_grenade]);
				SET_FLAG(input->player_control_flags, _player_control_zoom_bit, buttons[_button_scope_zoom] == 1);
				SET_FLAG(input->unit_control_flags, _unit_control_action_bit, buttons[_button_action_reload]);
				SET_FLAG(input->unit_control_flags, _unit_control_swap_weapons_bit, buttons[_button_action_reload] >= constants->minimum_weapon_swap_ticks);
				SET_FLAG(input->unit_control_flags, _unit_control_integrated_light_bit, buttons[_button_flashlight]);
				SET_FLAG(input->unit_control_flags, _unit_control_jump_bit, buttons[_button_jump]);
				SET_FLAG(input->unit_control_flags, _unit_control_use_equipment_bit, buttons[_button_melee_attack]);
				SET_FLAG(input->player_control_flags, _player_control_rotate_weapons_bit, buttons[_button_switch_weapon] == 1);
				SET_FLAG(input->player_control_flags, _player_control_rotate_grenades_bit, buttons[_button_switch_grenade] == 1);

				if (!TEST_FLAG(player_control->inhibited_button_bit_vector, _button_back))
				{
					input->back = gamepad->buttons[_gamepad_analog_button_b];
				}

				if (!TEST_FLAG(player_control->inhibited_button_bit_vector, _button_action_reload))
				{
					input->accept = gamepad->buttons[_gamepad_analog_button_a];
				}
			}
		}
		else if (input_get_mouse_state() && debug_input)
		{
			struct mouse_state const *mouse = input_get_mouse_state();

			input->throttle.i = (real)((input_key_is_down(_key_w) ? 1 : 0) - (input_key_is_down(_key_s) ? 1 : 0));
			input->throttle.j = (real)((input_key_is_down(_key_a) ? 1 : 0) - (input_key_is_down(_key_d) ? 1 : 0));

			if (player_control_camera_control_is_active())
			{
				input->facing_delta.yaw = -(mouse->dx * MOUSE_YAW_SCALE);
				input->facing_delta.pitch = -(mouse->dy * MOUSE_PITCH_SCALE);
			}
			else
			{
				input->facing_delta.yaw = 0.f;
				input->facing_delta.pitch = 0.f;
			}

			SET_FLAG(input->unit_control_flags, _unit_control_look_dont_turn_bit, input_key_is_down(_key_shift));
			SET_FLAG(input->unit_control_flags, _unit_control_force_alert_bit, input_key_is_down(_key_alt));
			SET_FLAG(input->unit_control_flags, _unit_control_crouch_modifier_bit, input_key_is_down(_key_control));
			SET_FLAG(input->unit_control_flags, _unit_control_jump_bit, input_key_is_down(_key_space));
			SET_FLAG(input->unit_control_flags, _unit_control_action_bit, input_key_is_down(_key_q));
			SET_FLAG(input->unit_control_flags, _unit_control_integrated_light_bit, input_key_is_down(_key_x));
			SET_FLAG(input->unit_control_flags, _unit_control_weapon_reload_bit, input_key_is_down(_key_r));
			SET_FLAG(input->unit_control_flags, _unit_control_weapon_primary_trigger_bit, mouse->button_frames[_mouse_button_left]);
			SET_FLAG(input->unit_control_flags, _unit_control_throw_grenade_bit, mouse->button_frames[_mouse_button_right]);
			SET_FLAG(input->player_control_flags, _player_control_zoom_bit, input_key_is_down(_key_z) == TRUE);
			SET_FLAG(input->player_control_flags, _player_control_rotate_weapons_bit, input_key_is_down(_key_e) == TRUE);
			input->primary_trigger = TEST_FLAG(input->unit_control_flags, _unit_control_weapon_primary_trigger_bit) ? 1.f : 0.f;
		}

		SET_FLAG(input->player_control_flags, _player_control_debug_rotate_units_bit, input_key_is_down(_key_backslash) == TRUE);
		SET_FLAG(input->player_control_flags, _player_control_debug_rotate_all_units_bit, input_key_is_down(_key_right_bracket) == TRUE);
		SET_FLAG(input->player_control_flags, _player_control_debug_ninja_rope_bit, input_key_is_down(_key_left_bracket) == TRUE);
		SET_FLAG(input->unit_control_flags, _unit_control_user_animation1_bit, input_key_is_down(_key_1) == TRUE);
		SET_FLAG(input->unit_control_flags, _unit_control_user_animation2_bit, input_key_is_down(_key_2) == TRUE);
		limit2d(&input->throttle, 1.f);
	}

	player_control_action_test_check_reset_input_blob(input);
	match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 694, input->primary_trigger);

	return;
}

static void handle_one_player_input(
	short local_player_index,
	real seconds_elapsed)
{
	struct player_control *player = player_control_get(local_player_index);
	struct game_globals_player_control *constants = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_control, 0, struct game_globals_player_control);
	struct input_blob input;
	long weapon_index;

	memset(&input, 0xFA, sizeof(input));
	get_local_player_input_blob(local_player_index, seconds_elapsed, &input);

	if (local_player_get_player_index(local_player_index) != NONE)
	{
		match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 718, player->desired_angles.pitch);
		match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 719, player->desired_angles.yaw);
	}

	if (director_inhibited_input(local_player_index))
	{
		clear_input_blob(&input);
	}

	if (game_connection() == _game_connection_local)
	{
		if (input.player_control_flags & (FLAG(_player_control_debug_rotate_units_bit) | FLAG(_player_control_debug_rotate_all_units_bit)))
		{
			long unit_index = TEST_FLAG(input.player_control_flags, _player_control_debug_rotate_all_units_bit) ? units_debug_get_next_unit(player->unit_index) : units_debug_get_closest_unit(player->unit_index);

			if (unit_index != NONE)
			{
				players_set_local_player_unit(local_player_index, unit_index);
			}
		}

		if (TEST_FLAG(input.player_control_flags, _player_control_debug_ninja_rope_bit) && player->unit_index != NONE)
		{
			unit_debug_ninja_rope(player->unit_index);
		}
	}

	if (player->unit_index != NONE)
	{
		struct unit_datum *unit = (struct unit_datum *)object_get_and_verify_type(player->unit_index, _object_mask_unit);
		struct unit_definition *definition = (struct unit_definition *)tag_get(UNIT_DEFINITION_TAG, unit->definition_index);
		short must_be_readied_weapon_index;

		weapon_index = unit_inventory_get_weapon(player->unit_index, unit->unit.current_weapon_index);

		if (player->desired_weapon_index == NONE || unit_inventory_get_weapon(player->unit_index, player->desired_weapon_index) == NONE)
		{
			player->desired_weapon_index = unit->unit.desired_weapon_index;
		}

		if (TEST_FLAG(input.player_control_flags, _player_control_rotate_weapons_bit) || unit_inventory_get_weapon(player->unit_index, player->desired_weapon_index) == NONE || player->desired_weapon_index == NONE)
		{
			player->desired_weapon_index = unit_inventory_next_weapon(player->unit_index, player->desired_weapon_index, TEST_FLAG(input.player_control_flags, _player_control_rotate_weapons_bit));
			player->desired_zoom_level = NONE;
		}

		must_be_readied_weapon_index = unit_inventory_get_must_be_readied_weapon(player->unit_index);
		if (must_be_readied_weapon_index != NONE && player->desired_weapon_index != must_be_readied_weapon_index)
		{
			player->desired_weapon_index = must_be_readied_weapon_index;
			player->desired_zoom_level = NONE;
		}

		if (player->desired_grenade_index == NONE || !unit_get_grenade_count(player->unit_index, player->desired_grenade_index))
		{
			player->desired_grenade_index = unit->unit.desired_grenade_index;
		}

		if (TEST_FLAG(input.player_control_flags, _player_control_rotate_grenades_bit) || !unit_get_grenade_count(player->unit_index, player->desired_grenade_index) || player->desired_grenade_index == NONE)
		{
			player->desired_grenade_index = unit_inventory_next_grenade(player->unit_index, player->desired_grenade_index, TRUE);
		}

		if (TEST_FLAG(input.player_control_flags, _player_control_zoom_bit) && player_control_camera_control_is_active() && weapon_index != NONE && !cinematic_in_progress())
		{
			player->desired_zoom_level = weapon_rotate_zoom_level(weapon_index, player->desired_zoom_level);
		}

		if (!director_inhibited_facing(local_player_index))
		{
			player_control_modify_desired_angles(local_player_index, input.facing_delta.yaw, input.facing_delta.pitch);
		}

		if (unit->object.parent_object_index == NONE)
		{
			if (player_ui_autolevel_enabled(local_player_index) && fabs(player->throttle.i) > 0.5f && input.facing_delta.pitch < 0.0001f && player->magnetism_level < 0.0001f)
			{
				player->autolevel_ticks = (char)PIN(player->autolevel_ticks + 1, 0, CHAR_MAX);
				player->use_autolevel = player->autolevel_ticks > constants->minimum_autolevel_enabled_ticks;
			}
			else
			{
				player->autolevel_ticks = 0;
				player->use_autolevel = FALSE;
			}
		}
		else
		{
			player->use_autolevel = FALSE;
		}
	}

	player->control_flags = input.unit_control_flags;
	player->throttle = input.throttle;
	player->primary_trigger = input.primary_trigger;
	match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 849, player->primary_trigger);

	if (local_player_get_player_index(local_player_index) != NONE)
	{
		struct player_action action;

		match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 861, player->desired_angles.pitch);
		match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 862, player->desired_angles.yaw);
		match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 863, player->primary_trigger);

		action.desired_facing = player->desired_angles;
		action.control_flags = player->control_flags;
		action.desired_weapon_index = player->desired_weapon_index;
		action.desired_grenade_index = player->desired_grenade_index;
		action.desired_zoom_level = player->desired_zoom_level;
		action.throttle = player->throttle;
		action.primary_trigger = player->primary_trigger;

		match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 873, action.desired_facing.pitch);
		match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 874, action.desired_facing.yaw);
		update_client_queue(&action);
	}

	return;
}

boolean player_control_update(
	real seconds_elapsed)
{
	short local_player_index;

	profile_enter(player_control_update_section);
	collision_log_begin_period(_collision_period_player_input);
	update_client_queue_push();

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		handle_one_player_input(local_player_index, seconds_elapsed);
	}

	collision_log_end_period();
	profile_exit(player_control_update_section);

	return FALSE;
}

real_euler_angles2d const *player_control_get_facing_angles(
	short local_player_index)
{
	struct player_control *player = player_control_get(local_player_index);

	match_assert("c:\\halo\\SOURCE\\game\\player_control.c", 960, valid_euler_angles2d(&player->desired_angles));

	return &player->desired_angles;
}

real_vector3d *player_control_get_facing_direction(
	short local_player_index,
	real_vector3d *direction)
{
	player_aiming_vector_from_facing(local_player_get_player_index(local_player_index), direction, player_control_get_facing_angles(local_player_index));

	return direction;
}

long player_control_get_aiming_unit_index(
	short local_player_index)
{
	struct player_control *player_control = player_control_get(local_player_index);

	return unit_get_aiming_unit_index(player_control->unit_index);
}

long player_control_get_target_object_index(
	short local_player_index)
{
	struct player_control *player_control = player_control_get(local_player_index);

	return object_try_and_get_and_verify_type(player_control->target_object_index, NONE) ? player_control->target_object_index : NONE;
}

real player_control_get_field_of_view(
	short local_player_index)
{
	struct player_control *player_control = player_control_get(local_player_index);
	real field_of_view = DEGREES_TO_RADIANS(70.f);

	if (player_control->unit_index != NONE)
	{
		struct unit_datum *unit = (struct unit_datum *)object_get_and_verify_type(player_control->unit_index, _object_mask_unit);
		struct unit_definition *definition = (struct unit_definition *)tag_get(UNIT_DEFINITION_TAG, unit->definition_index);
		long weapon_index = unit_inventory_get_weapon(player_control->unit_index, unit->unit.current_weapon_index);

		if (weapon_index != NONE)
		{
			field_of_view = weapon_get_field_of_view(weapon_index, definition->unit.camera_field_of_view, player_control->desired_zoom_level);
		}
		else
		{
			field_of_view = definition->unit.camera_field_of_view;
		}
	}

	return field_of_view;
}

void player_control_get_unit_camera_info(
	short local_player_index,
	struct unit_camera_info *camera_info)
{
	struct player_control *player_control;

	match_assert("c:\\halo\\SOURCE\\game\\player_control.c", 1026, camera_info);
	camera_info->unit_camera = NULL;
	player_control = player_control_get(local_player_index);
	camera_info->unit_index = player_control->unit_index;
	camera_info->seat_index = NONE;

	if (camera_info->unit_index != NONE)
	{
		struct unit_datum *unit = (struct unit_datum *)object_get_and_verify_type(camera_info->unit_index, _object_mask_unit);

		unit_get_camera_position(camera_info->unit_index, &camera_info->unit_origin);
		if (unit->object.parent_object_index != NONE)
		{
			struct vehicle_datum *vehicle = (struct vehicle_datum *)object_try_and_get_and_verify_type(unit->object.parent_object_index, _object_mask_vehicle);

			if (vehicle)
			{
				struct vehicle_definition *definition = (struct vehicle_definition *)tag_get(VEHICLE_DEFINITION_TAG, vehicle->definition_index);
				struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

				camera_info->unit_index = unit->object.parent_object_index;
				camera_info->unit_camera = &seat->camera;
				camera_info->seat_index = unit->unit.parent_seat_index;
				unit = (struct unit_datum *)object_get_and_verify_type(camera_info->unit_index, _object_mask_unit);
			}
		}

		if (camera_info->seat_index == NONE)
		{
			camera_info->unit_camera = &((struct unit_definition *)tag_get(UNIT_DEFINITION_TAG, unit->definition_index))->unit.camera;
		}
	}

	return;
}

long player_control_get_unit_index(
	short local_player_index)
{
	return player_control_get(local_player_index)->unit_index;
}

long player_control_get_desired_weapon(
	short local_player_index,
	long unit_index)
{
	struct player_control *player_control = player_control_get(local_player_index);

	if (player_control->unit_index == unit_index)
	{
		long weapon_index = unit_inventory_get_weapon(unit_index, player_control->desired_weapon_index);

		if (weapon_index != NONE)
		{
			return weapon_index;
		}
	}

	return unit_inventory_get_weapon(unit_index, ((struct unit_datum *)object_get_and_verify_type(unit_index, _object_mask_unit))->unit.current_weapon_index);
}

void player_control_set_desired_weapon(
	long unit_index,
	short desired_weapon_index)
{
	short local_player_index = unit_get_local_player_index(unit_index);

	if (local_player_index != NONE)
	{
		player_control_get(local_player_index)->desired_weapon_index = desired_weapon_index;
	}

	return;
}

real player_control_get_autoaim_level(
	short local_player_index)
{
	return player_control_get(local_player_index)->autoaim_level;
}

void player_control_permanent_impulse(
	short local_player_index,
	real_euler_angles2d const *delta)
{
	match_assert("c:\\halo\\SOURCE\\game\\player_control.c", 1127, delta);
	player_control_modify_desired_angles(local_player_index, delta->yaw, delta->pitch);

	return;
}

static void player_control_modify_desired_angles(
	short local_player_index,
	real delta_yaw,
	real delta_pitch)
{
	struct player_control *player = player_control_get(local_player_index);
	struct game_globals_player_control *constants = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->player_control, 0, struct game_globals_player_control);
	real pitch_minimum = -MAXIMUM_PLAYER_PITCH;
	real pitch_maximum = MAXIMUM_PLAYER_PITCH;
	struct unit_camera_info camera_info;

	match_assert("c:\\halo\\SOURCE\\game\\player_control.c", 1172, valid_euler_angles2d(&player->desired_angles));
	player_control_get_unit_camera_info(local_player_index, &camera_info);
	player->desired_angles.yaw += delta_yaw;

	if (camera_info.seat_index != NONE)
	{
		struct unit_definition *definition = (struct unit_definition *)tag_get(UNIT_DEFINITION_TAG, ((struct object_datum *)object_get_and_verify_type(camera_info.unit_index, _object_mask_unit))->definition_index);
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&definition->unit.seats, camera_info.seat_index, struct unit_seat);

		if (seat->yaw_minimum != 0.f || seat->yaw_maximum != 0.f)
		{
			struct object_marker seat_marker;
			real_euler_angles2d seat_facing;
			real yaw_minimum;
			real yaw_maximum;
			real yaw_range;
			real maximum_difference;
			real minimum_difference;

			object_get_marker_by_name(camera_info.unit_index, seat->marker_name, &seat_marker, 1);
			euler_angles2d_from_vector3d(&seat_facing, &seat_marker.matrix.forward);
			yaw_minimum = seat_facing.yaw + seat->yaw_minimum;
			yaw_maximum = seat_facing.yaw + seat->yaw_maximum;
			yaw_range = signed_angular_difference(yaw_minimum, yaw_maximum);
			maximum_difference = signed_angular_difference(player->desired_angles.yaw, yaw_maximum);
			minimum_difference = signed_angular_difference(yaw_minimum, player->desired_angles.yaw);

			if (yaw_range < 0.f)
			{
				yaw_range += (_pi*2.f);
			}

			if (!(maximum_difference >= 0.f && maximum_difference < yaw_range) && !(minimum_difference >= 0.f && minimum_difference < yaw_range))
			{
				if (fabs(minimum_difference) < fabs(maximum_difference))
				{
					player->desired_angles.yaw = yaw_minimum;
				}
				else
				{
					player->desired_angles.yaw = yaw_maximum;
				}
			}
		}
	}

	while (player->desired_angles.yaw < 0.f)
	{
		player->desired_angles.yaw += (_pi*2.f);
	}

	while (player->desired_angles.yaw > (_pi*2.f))
	{
		player->desired_angles.yaw -= (_pi*2.f);
	}

	if (camera_info.unit_camera)
	{
		struct unit_datum *unit = (struct unit_datum *)object_get_and_verify_type(camera_info.unit_index, _object_mask_unit);
		real autolevel_pitch = camera_info.unit_camera->pitch_autolevel;

		if (camera_info.unit_camera->pitch_maximum != 0.f || camera_info.unit_camera->pitch_minimum != 0.f)
		{
			pitch_minimum = camera_info.unit_camera->pitch_minimum;
			pitch_maximum = camera_info.unit_camera->pitch_maximum;

			if (camera_info.seat_index != NONE && unit->object.up.k > 0.2f)
			{
				real_euler_angles2d facing_angle;
				real_vector3d facing;
				real tilt;

				facing_angle.yaw = player->desired_angles.yaw;
				facing_angle.pitch = 0.f;
				vector3d_from_euler_angles2d(&facing, &facing_angle);
				tilt = (_pi/2.f) - angle_between_vectors3d(&facing, &unit->object.up);
				pitch_minimum -= tilt;
				pitch_maximum -= tilt;
				autolevel_pitch -= tilt;
			}

			pitch_minimum = PIN(pitch_minimum, -MAXIMUM_PLAYER_PITCH, MAXIMUM_PLAYER_PITCH);
			pitch_maximum = PIN(pitch_maximum, -MAXIMUM_PLAYER_PITCH, MAXIMUM_PLAYER_PITCH);
		}

		if (autolevel_pitch != 0.f || player->use_autolevel)
		{
			real const pitch_error_scale = 2.f / _pi;
			real pitch_error = (real)(fabs(player->desired_angles.pitch - autolevel_pitch) * pitch_error_scale);

			match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 1266, player->desired_angles.pitch);

			if (autolevel_pitch != 0.f)
			{
				real const autolevel_scale = 0.08f;

				interpolate_scalar(&player->desired_angles.pitch, autolevel_pitch, pitch_error * autolevel_scale * magnitude3d(&unit->object.translational_velocity));
			}
			else
			{
				interpolate_scalar(&player->desired_angles.pitch, autolevel_pitch, pitch_error * constants->look_autolevel_scale * magnitude3d(&unit->object.translational_velocity));
			}

			match_assert_valid_real("c:\\halo\\SOURCE\\game\\player_control.c", 1277, player->desired_angles.pitch);
		}
	}

	interpolate_scalar(&player->pitch_minimum, pitch_minimum, _pi / 256.f);
	interpolate_scalar(&player->pitch_maximum, pitch_maximum, _pi / 256.f);
	player->desired_angles.pitch += delta_pitch;
	player->desired_angles.pitch = PIN(player->desired_angles.pitch, player->pitch_minimum, player->pitch_maximum);

	return;
}

short unit_get_local_player_index(
	long unit_index)
{
	struct unit_datum *unit = (struct unit_datum *)object_get_and_verify_type(unit_index, _object_mask_unit);

	if (unit->unit.player_index != NONE)
	{
		return player_get(unit->unit.player_index)->local_player_index;
	}

	return NONE;
}

void players_unzoom_all(
	void)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		player_control_get(local_player_index)->desired_zoom_level = NONE;
	}

	return;
}

void player_control_unzoom(
	long unit_index)
{
	short local_player_index = unit_get_local_player_index(unit_index);

	if (local_player_index != NONE)
	{
		player_control_get(local_player_index)->desired_zoom_level = NONE;
	}

	return;
}

short player_control_get_zoom_level(
	short local_player_index)
{
	short zoom_level = NONE;

	if (local_player_index != NONE)
	{
		zoom_level = player_control_get(local_player_index)->desired_zoom_level;
	}

	return zoom_level;
}

void player_control_action_test_reset(
	void)
{
	player_control_globals->action_test_flags = 0;
	player_control_globals->testing_for_action_flags = 0;

	return;
}

boolean player_control_action_test_accept(
	void)
{
	SET_FLAG(player_control_globals->testing_for_action_flags, _player_control_action_test_accept_bit, TRUE);
	SET_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_accept_bit, TRUE);

	return TEST_FLAG(player_control_globals->action_test_flags, _player_control_action_test_accept_bit);
}

boolean player_control_action_test_back(
	void)
{
	SET_FLAG(player_control_globals->testing_for_action_flags, _player_control_action_test_back_bit, TRUE);
	SET_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_back_bit, TRUE);

	return TEST_FLAG(player_control_globals->action_test_flags, _player_control_action_test_back_bit);
}

boolean player_control_action_test_action(
	void)
{
	SET_FLAG(player_control_globals->testing_for_action_flags, _player_control_action_test_action_bit, TRUE);
	SET_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_action_bit, TRUE);

	return TEST_FLAG(player_control_globals->action_test_flags, _player_control_action_test_action_bit);
}

boolean player_control_action_test_jump(
	void)
{
	return TEST_FLAG(player_control_globals->action_test_flags, _player_control_action_test_jump_bit);
}

boolean player_control_action_test_primary_trigger(
	void)
{
	return TEST_FLAG(player_control_globals->action_test_flags, _player_control_action_test_primary_trigger_bit);
}

boolean player_control_action_test_grenade_trigger(
	void)
{
	return TEST_FLAG(player_control_globals->action_test_flags, _player_control_action_test_grenade_trigger_bit);
}

boolean player_control_action_test_zoom(
	void)
{
	return TEST_FLAG(player_control_globals->action_test_flags, _player_control_action_test_zoom_bit);
}

boolean player_control_action_test_move_relative_all_directions(
	void)
{
	unsigned long mask = FLAG(_player_control_action_test_move_forward_bit) | FLAG(_player_control_action_test_move_backward_bit) | FLAG(_player_control_action_test_move_right_bit) | FLAG(_player_control_action_test_move_left_bit);

	return !(~player_control_globals->action_test_flags & mask);
}

boolean player_control_action_test_look_relative_all_directions(
	void)
{
	unsigned long mask = FLAG(_player_control_action_test_look_positive_pitch_bit) | FLAG(_player_control_action_test_look_negative_pitch_bit) | FLAG(_player_control_action_test_look_positive_yaw_bit) | FLAG(_player_control_action_test_look_negative_yaw_bit);

	return !(~player_control_globals->action_test_flags & mask);
}

boolean player_control_action_test_look_relative_left(
	void)
{
	return TEST_FLAG(player_control_globals->action_test_flags, _player_control_action_test_look_positive_yaw_bit);
}

boolean player_control_action_test_look_relative_right(
	void)
{
	return TEST_FLAG(player_control_globals->action_test_flags, _player_control_action_test_look_negative_yaw_bit);
}

boolean player_control_action_test_look_relative_up(
	void)
{
	return TEST_FLAG(player_control_globals->action_test_flags, _player_control_action_test_look_positive_pitch_bit);
}

boolean player_control_action_test_look_relative_down(
	void)
{
	return TEST_FLAG(player_control_globals->action_test_flags, _player_control_action_test_look_negative_pitch_bit);
}

static void player_control_action_test_check_reset_input_blob(
	struct input_blob *input)
{
	if (input->accept && cinematic_can_be_skipped())
	{
		main_skip_cinematic();
	}

	if (TEST_FLAG(input->unit_control_flags, _unit_control_action_bit))
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_action_bit, TRUE);
	}

	if (TEST_FLAG(input->unit_control_flags, _unit_control_jump_bit))
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_jump_bit, TRUE);
	}

	if (input->accept)
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_accept_bit, TRUE);
	}

	if (input->back)
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_back_bit, TRUE);
	}

	if (input->primary_trigger > 0.f)
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_primary_trigger_bit, TRUE);
	}

	if (TEST_FLAG(input->unit_control_flags, _unit_control_throw_grenade_bit))
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_grenade_trigger_bit, TRUE);
	}

	if (TEST_FLAG(input->player_control_flags, _player_control_zoom_bit))
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_zoom_bit, TRUE);
	}

	if (input->facing_delta.pitch > 0.f)
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_look_positive_pitch_bit, TRUE);
	}
	else if (input->facing_delta.pitch < 0.f)
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_look_negative_pitch_bit, TRUE);
	}

	if (input->facing_delta.yaw > 0.f)
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_look_positive_yaw_bit, TRUE);
	}
	else if (input->facing_delta.yaw < 0.f)
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_look_negative_yaw_bit, TRUE);
	}

	if (input->throttle.i > 0.f)
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_move_forward_bit, TRUE);
	}
	else if (input->throttle.i < 0.f)
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_move_backward_bit, TRUE);
	}

	if (input->throttle.j > 0.f)
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_move_right_bit, TRUE);
	}
	else if (input->throttle.j < 0.f)
	{
		SET_FLAG(player_control_globals->action_test_flags, _player_control_action_test_move_left_bit, TRUE);
	}

	if (TEST_FLAG(player_control_globals->testing_for_action_flags, _player_control_action_test_action_bit))
	{
		SET_FLAG(input->unit_control_flags, _unit_control_action_bit, FALSE);
	}
	else if (TEST_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_action_bit))
	{
		SET_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_action_bit, TEST_FLAG(input->unit_control_flags, _unit_control_action_bit));
		SET_FLAG(input->unit_control_flags, _unit_control_action_bit, FALSE);
	}

	if (!controls_swapped)
	{
		if (TEST_FLAG(player_control_globals->testing_for_action_flags, _player_control_action_test_accept_bit))
		{
			SET_FLAG(input->unit_control_flags, _unit_control_action_bit, FALSE);
		}
		else if (TEST_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_accept_bit))
		{
			SET_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_accept_bit, TEST_FLAG(input->unit_control_flags, _unit_control_action_bit));
			SET_FLAG(input->unit_control_flags, _unit_control_action_bit, FALSE);
		}

		if (TEST_FLAG(player_control_globals->testing_for_action_flags, _player_control_action_test_back_bit))
		{
			SET_FLAG(input->player_control_flags, _player_control_rotate_weapons_bit, FALSE);
		}
		else if (TEST_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_back_bit))
		{
			SET_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_back_bit, TEST_FLAG(input->player_control_flags, _player_control_rotate_weapons_bit));
			SET_FLAG(input->player_control_flags, _player_control_rotate_weapons_bit, FALSE);
		}
	}
	else
	{
		if (TEST_FLAG(player_control_globals->testing_for_action_flags, _player_control_action_test_accept_bit))
		{
			SET_FLAG(input->unit_control_flags, _unit_control_jump_bit, FALSE);
		}
		else if (TEST_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_accept_bit))
		{
			SET_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_accept_bit, TEST_FLAG(input->unit_control_flags, _unit_control_jump_bit));
			SET_FLAG(input->unit_control_flags, _unit_control_jump_bit, FALSE);
		}

		if (TEST_FLAG(player_control_globals->testing_for_action_flags, _player_control_action_test_back_bit))
		{
			SET_FLAG(input->player_control_flags, _player_control_rotate_grenades_bit, FALSE);
		}
		else if (TEST_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_accept_bit))
		{
			SET_FLAG(player_control_globals->inhibit_button_flags, _player_control_action_test_accept_bit, TEST_FLAG(input->player_control_flags, _player_control_rotate_grenades_bit));
			SET_FLAG(input->player_control_flags, _player_control_rotate_grenades_bit, FALSE);
		}
	}

	return;
}

