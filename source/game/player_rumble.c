/*
PLAYER_RUMBLE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "players.h"
#include "game_state.h"
#include "damage_effect_definitions.h"
#include "input.h"
#include "player_ui.h"

/* ---------- constants */

enum
{
	MAXIMUM_NUMBER_OF_RUMBLE_EFFECTS = 8 /* fake name */
};

/* ---------- macros */

#define SECONDS_PER_TICK (1.f/TICKS_PER_SECOND) /* fake name */

/* ---------- structures */

struct player_rumble_datum
{
	struct rumble_definition impulse[MAXIMUM_NUMBER_OF_RUMBLE_EFFECTS];
	real time_elapsed[MAXIMUM_NUMBER_OF_RUMBLE_EFFECTS];
	real left_frequency_continuous;
	real right_frequency_continuous;
};

struct scripted_rumble_datum
{
	real left_frequency;
	real right_frequency;
	real scale;
};

struct rumble_global_data
{
	struct player_rumble_datum player_rumble_data[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	struct scripted_rumble_datum scripted_rumble;
};

/* ---------- prototypes */

static struct rumble_data rumble_calculate(struct player_rumble_datum *datum);

/* ---------- globals */

static struct rumble_global_data *rumble_globals;

/* ---------- public code */

void rumble_initialize(
	void)
{
	rumble_globals = game_state_malloc("rumble", NULL, sizeof(*rumble_globals));

	return;
}

void rumble_dispose(
	void)
{
	return;
}

void rumble_initialize_for_new_map(
	void)
{
	memset(rumble_globals, 0, sizeof(*rumble_globals));

	return;
}

void rumble_dispose_from_old_map(
	void)
{
	short gamepad_index;

	for (gamepad_index = 0; gamepad_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; gamepad_index++)
	{
		input_set_gamepad_rumbler_state(gamepad_index, 0, 0);
	}

	rumble_clear_all_now();

	return;
}

void rumble_player_set_scripted_values(
	real left,
	real right)
{
	rumble_globals->scripted_rumble.left_frequency = left;
	rumble_globals->scripted_rumble.right_frequency = right;

	return;
}

void rumble_player_set_scale(
	real scale)
{
	rumble_globals->scripted_rumble.scale = scale;

	return;
}

void rumble_update(
	void)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		struct player_rumble_datum *rumble_datum = &rumble_globals->player_rumble_data[local_player_index];
		struct rumble_data rumble = rumble_calculate(rumble_datum);
		long player_index;
		long impulse_index;

		for (impulse_index = 0; impulse_index < MAXIMUM_NUMBER_OF_RUMBLE_EFFECTS; impulse_index++)
		{
			rumble_datum->time_elapsed[impulse_index] += SECONDS_PER_TICK;
		}

		player_index = local_player_get_player_index(local_player_index);

		if (player_index != NONE)
		{
			long controller_index = player_get(player_index)->local_player_index;

			if (controller_index != NONE)
			{
				if (!player_ui_rumble_disabled(player_ui_get_single_player_local_player_from_controller((short)controller_index)))
				{
					input_set_gamepad_rumbler_state((short)controller_index, rumble.left_frequency, rumble.right_frequency);
				}
				else
				{
					input_set_gamepad_rumbler_state((short)controller_index, 0, 0);
				}
			}
		}
		else
		{
			input_set_gamepad_rumbler_state(local_player_index, 0, 0);
		}
	}

	return;
}

void rumble_player_impulse(
	short local_player_index,
	struct rumble_definition const *rumble_definition,
	real scale,
	real time_scale)
{
	struct player_rumble_datum *rumble_datum = &rumble_globals->player_rumble_data[local_player_index];
	real largest_timestamp = rumble_datum->time_elapsed[0];
	struct rumble_definition *replaced_impulse = &rumble_datum->impulse[0];
	long impulse_index;
	real frequency_scale;

	match_assert("c:\\halo\\SOURCE\\game\\player_rumble.c", 164, rumble_definition);

	for (impulse_index = 1; impulse_index < MAXIMUM_NUMBER_OF_RUMBLE_EFFECTS; impulse_index++)
	{
		if (rumble_datum->time_elapsed[impulse_index] > largest_timestamp)
		{
			replaced_impulse = &rumble_datum->impulse[impulse_index];
			largest_timestamp = rumble_datum->time_elapsed[impulse_index];
		}
	}

	*replaced_impulse = *rumble_definition;
	frequency_scale = (1.f - rumble_definition->zero_scale_factor) * scale + rumble_definition->zero_scale_factor;
	replaced_impulse->rumble_frequencies[_rumble_frequency_left].frequency *= frequency_scale;
	replaced_impulse->rumble_frequencies[_rumble_frequency_right].frequency *= frequency_scale;
	replaced_impulse->rumble_frequencies[_rumble_frequency_left].duration *= time_scale;
	replaced_impulse->rumble_frequencies[_rumble_frequency_right].duration *= time_scale;
	rumble_datum->time_elapsed[replaced_impulse - rumble_datum->impulse] = 0.f;

	return;
}

void rumble_player_clear(
	short local_player_index)
{
	memset(&rumble_globals->player_rumble_data[local_player_index], 0, sizeof(struct player_rumble_datum));

	return;
}

void rumble_clear_all_now(
	void)
{
	long gamepad_index;

	memset(rumble_globals, 0, sizeof(*rumble_globals));

	for (gamepad_index = 0; gamepad_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; gamepad_index++)
	{
		if (input_has_gamepad((short)gamepad_index))
		{
			input_set_gamepad_rumbler_state((short)gamepad_index, 0, 0);
		}
	}

	return;
}

void rumble_player_continuous(
	short local_player_index,
	real left,
	real right)
{
	struct player_rumble_datum *rumble_datum = &rumble_globals->player_rumble_data[local_player_index];

	rumble_datum->left_frequency_continuous = left;
	rumble_datum->right_frequency_continuous = right;

	return;
}

/* ---------- private code */

static struct rumble_data rumble_calculate(
	struct player_rumble_datum *datum)
{
	struct rumble_data result;
	real magnitudes[NUMBER_OF_RUMBLE_FREQUENCIES];
	short impulse_index;

	magnitudes[_rumble_frequency_left] = datum->left_frequency_continuous;
	magnitudes[_rumble_frequency_right] = datum->right_frequency_continuous;

	for (impulse_index = 0; impulse_index < MAXIMUM_NUMBER_OF_RUMBLE_EFFECTS; impulse_index++)
	{
		real time_elapsed = datum->time_elapsed[impulse_index];
		long frequency_index;

		for (frequency_index = 0; frequency_index < NUMBER_OF_RUMBLE_FREQUENCIES; frequency_index++)
		{
			struct rumble_frequency_definition *frequency = &datum->impulse[impulse_index].rumble_frequencies[frequency_index];

			if (frequency->duration > time_elapsed)
			{
				magnitudes[frequency_index] += frequency->frequency * transition_function_evaluate(frequency->fade_function, PIN(1.f - time_elapsed / frequency->duration, 0.f, 1.f));
			}
		}
	}

	if (rumble_globals->scripted_rumble.scale != 0.f)
	{
		magnitudes[_rumble_frequency_left] += rumble_globals->scripted_rumble.left_frequency * rumble_globals->scripted_rumble.scale;
		magnitudes[_rumble_frequency_right] += rumble_globals->scripted_rumble.right_frequency * rumble_globals->scripted_rumble.scale;
	}

	result.left_frequency = (word)fast_ftol(PIN(magnitudes[_rumble_frequency_left] * UNSIGNED_SHORT_MAX, 0.f, UNSIGNED_SHORT_MAX));
	result.right_frequency = (word)fast_ftol(PIN(magnitudes[_rumble_frequency_right] * UNSIGNED_SHORT_MAX, 0.f, UNSIGNED_SHORT_MAX));

	return result;
}
