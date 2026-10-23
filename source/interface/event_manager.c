/*
EVENT_MANAGER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "event_manager.h"
#include "input.h"

/* ---------- constants */

enum
{
	MAXIMUM_QUEUED_EVENTS_PER_GAMEPAD = 8,			/* fake name */

	STICK_EVENT_THRESHOLD = (9 * SHORT_MAX) / 10,	/* fake name */
	STICK_EVENT_REPEAT_DELAY = 250					/* fake name */
};

/* ---------- structures */

struct event_manager_globals
{
	boolean initialized;
	boolean suppressed;
	unsigned long milliseconds_at_last_event_queue;
	struct event_record event_queue[MAXIMUM_GAMEPADS][MAXIMUM_QUEUED_EVENTS_PER_GAMEPAD];
};

/* ---------- prototypes */

static void queue_event(struct event_record *event, short local_player_index);

/* ---------- globals */

struct event_manager_globals event_manager_globals = {0};

static unsigned long stick_event_milliseconds[NUMBER_OF_GAMEPAD_STICKS][MAXIMUM_GAMEPADS] = {0};	/* fake name */
static long previous_stick_values[NUMBER_OF_GAMEPAD_STICKS][2][MAXIMUM_GAMEPADS] = {0};			/* fake name */

/* ---------- public code */

void event_manager_initialize(
	void)
{
	memset(&event_manager_globals, 0, sizeof(event_manager_globals));
	event_manager_globals.milliseconds_at_last_event_queue = system_milliseconds();
	event_manager_globals.initialized = TRUE;

	return;
}

void event_manager_dispose(
	void)
{
	memset(&event_manager_globals, 0, sizeof(event_manager_globals));

	return;
}

void event_manager_update(
	void)
{
	if (event_manager_globals.initialized)
	{
		short gamepad_index;

		for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
		{
			boolean queued = FALSE;

			if (input_has_gamepad(gamepad_index))
			{
				struct gamepad_state const *state = input_get_gamepad_state(gamepad_index);

				if (state)
				{
					struct event_record event;
					short button_index;

					if (state->sticks[_gamepad_stick_left].x || state->sticks[_gamepad_stick_left].y)
					{
						event.type = _event_type_left_analog_stick;
						event.value.analog_value = state->sticks[_gamepad_stick_left];
						queue_event(&event, gamepad_index);
						queued = TRUE;
					}

					if (state->sticks[_gamepad_stick_right].x || state->sticks[_gamepad_stick_right].y)
					{
						event.type = _event_type_right_analog_stick;
						event.value.analog_value = state->sticks[_gamepad_stick_right];
						queue_event(&event, gamepad_index);
						queued = TRUE;
					}

					for (button_index = 0; button_index < NUMBER_OF_GAMEPAD_BUTTONS; button_index++)
					{
						if (state->buttons[button_index])
						{
							event.type = _event_type_button;
							event.value.button_value.button = (byte)button_index;
							event.value.button_value.value = state->buttons[button_index];
							queue_event(&event, gamepad_index);
							queued = TRUE;
						}
					}
				}
			}

			if (!queued)
			{
				struct event_record null_event = { _event_type_none };

				queue_event(&null_event, gamepad_index);
			}
		}
	}

	return;
}

void event_manager_flush(
	void)
{
	memset(event_manager_globals.event_queue, 0, sizeof(event_manager_globals.event_queue));

	return;
}

void event_manager_suppress(
	boolean suppress)
{
	event_manager_globals.suppressed = suppress;

	return;
}

boolean get_next_event(
	struct event_record *event,
	short local_player_index)
{
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\interface\\event_manager.c", 211, event && ((local_player_index>=0 && local_player_index<MAXIMUM_GAMEPADS) || local_player_index==NONE));

	if (event_manager_globals.initialized)
	{
		if (local_player_index == NONE)
		{
			for (local_player_index = 0; !result && local_player_index < MAXIMUM_GAMEPADS; local_player_index++)
			{
				result = get_next_event(event, local_player_index);
			}
		}
		else
		{
			long event_index;

			for (event_index = MAXIMUM_QUEUED_EVENTS_PER_GAMEPAD - 1; event_index >= 0; event_index--)
			{
				if (event_manager_globals.event_queue[local_player_index][event_index].type != _event_type_none)
				{
					*event = event_manager_globals.event_queue[local_player_index][event_index];
					event_manager_globals.event_queue[local_player_index][event_index].type = _event_type_none;

					return TRUE;
				}
			}
		}
	}

	return result;
}

unsigned long event_manager_time_of_last_event(
	void)
{
	return event_manager_globals.milliseconds_at_last_event_queue;
}

/* ---------- private code */

static void queue_event(
	struct event_record *event,
	short local_player_index)
{
	if (!event_manager_globals.suppressed)
	{
		unsigned long milliseconds = system_milliseconds();
		boolean queue = TRUE;

		if (event->type == _event_type_left_analog_stick)
		{
			long x = event->value.analog_value.x;
			long y = event->value.analog_value.y;

			if (ABS(x) < STICK_EVENT_THRESHOLD && ABS(y) < STICK_EVENT_THRESHOLD)
			{
				queue = FALSE;
			}
			else if ((ABS(x) < STICK_EVENT_THRESHOLD || ABS(previous_stick_values[_gamepad_stick_left][0][local_player_index]) >= STICK_EVENT_THRESHOLD) &&
				(ABS(y) < STICK_EVENT_THRESHOLD || ABS(previous_stick_values[_gamepad_stick_left][1][local_player_index]) >= STICK_EVENT_THRESHOLD) &&
				milliseconds - stick_event_milliseconds[_gamepad_stick_left][local_player_index] < STICK_EVENT_REPEAT_DELAY)
			{
				queue = FALSE;
			}
			else
			{
				stick_event_milliseconds[_gamepad_stick_left][local_player_index] = milliseconds;
				queue = TRUE;

				if (ABS(x) >= STICK_EVENT_THRESHOLD)
				{
					switch ((x >= 0) ? 1 : -1)
					{
					case 1:
						event->value.analog_value.x = SHORT_MAX;
						x = SHORT_MAX;
						break;
					case -1:
						event->value.analog_value.x = SHORT_MIN;
						x = SHORT_MIN;
						break;
					}
				}

				if (ABS(y) >= STICK_EVENT_THRESHOLD)
				{
					switch ((y >= 0) ? 1 : -1)
					{
					case 1:
						event->value.analog_value.y = SHORT_MAX;
						y = SHORT_MAX;
						break;
					case -1:
						event->value.analog_value.y = SHORT_MIN;
						y = SHORT_MIN;
						break;
					}
				}
			}

			previous_stick_values[_gamepad_stick_left][0][local_player_index] = x;
			previous_stick_values[_gamepad_stick_left][1][local_player_index] = y;
		}
		else if (event->type == _event_type_right_analog_stick)
		{
			long x = event->value.analog_value.x;
			long y = event->value.analog_value.y;

			if (ABS(x) < STICK_EVENT_THRESHOLD && ABS(y) < STICK_EVENT_THRESHOLD)
			{
				queue = FALSE;
			}
			else if ((ABS(x) < STICK_EVENT_THRESHOLD || ABS(previous_stick_values[_gamepad_stick_right][0][local_player_index]) >= STICK_EVENT_THRESHOLD) &&
				(ABS(y) < STICK_EVENT_THRESHOLD || ABS(previous_stick_values[_gamepad_stick_right][1][local_player_index]) >= STICK_EVENT_THRESHOLD) &&
				milliseconds - stick_event_milliseconds[_gamepad_stick_right][local_player_index] < STICK_EVENT_REPEAT_DELAY)
			{
				queue = FALSE;
			}
			else
			{
				stick_event_milliseconds[_gamepad_stick_right][local_player_index] = milliseconds;
				queue = TRUE;

				if (ABS(x) >= STICK_EVENT_THRESHOLD)
				{
					switch ((x >= 0) ? 1 : -1)
					{
					case 1:
						event->value.analog_value.x = SHORT_MAX;
						x = SHORT_MAX;
						break;
					case -1:
						event->value.analog_value.x = SHORT_MIN;
						x = SHORT_MIN;
						break;
					}
				}

				if (ABS(y) >= STICK_EVENT_THRESHOLD)
				{
					switch ((y >= 0) ? 1 : -1)
					{
					case 1:
						event->value.analog_value.y = SHORT_MAX;
						y = SHORT_MAX;
						break;
					case -1:
						event->value.analog_value.y = SHORT_MIN;
						y = SHORT_MIN;
						break;
					}
				}
			}

			previous_stick_values[_gamepad_stick_right][0][local_player_index] = x;
			previous_stick_values[_gamepad_stick_right][1][local_player_index] = y;
		}

		if (queue)
		{
			event->controller_index = local_player_index;
			memmove(
				event_manager_globals.event_queue[local_player_index],
				&event_manager_globals.event_queue[local_player_index][1],
				sizeof(event_manager_globals.event_queue[local_player_index]) - sizeof(*event));
			event_manager_globals.event_queue[local_player_index][0] = *event;

			if (event->type != _event_type_none)
			{
				event_manager_globals.milliseconds_at_last_event_queue = milliseconds;
			}
		}
	}

	return;
}
