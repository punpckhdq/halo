/*
ACTION_WAIT.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "props.h"
#include "ai_communication.h"
#include "game.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

boolean action_wait_setup(
	long actor_index,
	boolean actively_searching,
	struct wait_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_wait.c", 33, state_data);

	memset(state_data, 0, sizeof(struct wait_state_data));

	if (!actor->input.vehicle_passenger)
	{
		state_data->waiting_as_coordinator = actor->external_orders.pursuit_is_coordinator;
		state_data->was_actively_searching = actively_searching;
		state_data->entry_time = game_time_get();
		state_data->exit_timer = 0;
		state_data->look_timer = 4 * TICKS_PER_SECOND;
		state_data->desire_move = TRUE;
		state_data->query_timer = random_range(10 * TICKS_PER_SECOND, 20 * TICKS_PER_SECOND);
		success = TRUE;
	}

	return success;
}

void action_wait_begin(
	long actor_index)
{
	return;
}

boolean action_wait_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct wait_state_data *state_data = &actor->state.action_data.wait;

	if (actor->meta.timeslice)
	{
		state_data->desire_move = FALSE;
		actor_pursuit_find_nearby_actors(actor_index, actor->external_orders.pursuit_is_coordinator);

		if (state_data->waiting_as_coordinator)
		{
			if (actor->external_orders.pursuit_group_prop_index == NONE)
			{
				if (state_data->exit_timer == 0)
				{
					state_data->exit_timer = 5 * TICKS_PER_SECOND;
				}
			}
			else if (game_time_get() >= state_data->entry_time + 90 * TICKS_PER_SECOND)
			{
				state_data->wait_done = TRUE;
			}
		}
		else
		{
			state_data->wait_done = TRUE;

			if (actor->external_orders.pursuit_group_prop_index != NONE)
			{
				struct prop_datum *prop = prop_get(actor->external_orders.pursuit_group_prop_index);

				if (state_data->was_actively_searching && !state_data->move_failed)
				{
					state_data->desire_move = prop->distance > 3.5f;
					state_data->wait_done = FALSE;
				}
				else if (prop->visibility >= _actor_perception_full && prop->distance < 8.f)
				{
					state_data->desire_move = !state_data->move_failed && prop->distance > 3.5f;
					state_data->wait_done = FALSE;
				}
				else
				{
					state_data->wait_done = TRUE;
				}
			}
		}

		if (!actor->meta.swarm)
		{
			if (state_data->desire_move)
			{
				if (!actor_move_to_prop(actor_index, actor->external_orders.pursuit_group_prop_index, 8.f))
				{
					state_data->move_failed = TRUE;
				}
			}
			else
			{
				actor_move_halt(actor_index);
			}
		}
	}

	return state_data->wait_done;
}

void action_wait_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct wait_state_data *state_data = &actor->state.action_data.wait;

	if (state_data->query_timer > 0)
	{
		state_data->query_timer--;

		if (state_data->query_timer == 0)
		{
			if (actor->meta.unit_index != NONE)
			{
				ai_communication_event(_ai_communication_search_query, actor->meta.unit_index, NONE, NONE, NONE, NONE, NULL);
			}

			state_data->query_timer = random_range(10 * TICKS_PER_SECOND, 20 * TICKS_PER_SECOND);
		}
	}

	if (state_data->exit_timer > 0)
	{
		state_data->exit_timer--;

		if (state_data->exit_timer == 0)
		{
			if (state_data->waiting_as_coordinator && actor->meta.unit_index != NONE)
			{
				ai_communication_event(_ai_communication_search_group_abandon, actor->meta.unit_index, NONE, NONE, NONE, NONE, NULL);
			}

			state_data->wait_done = TRUE;
		}
	}

	if (!state_data->desire_move && state_data->look_timer > 0)
	{
		state_data->look_timer--;
	}

	return;
}

void action_wait_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct wait_state_data *state_data = &actor->state.action_data.wait;

	if (actor->control.moving)
	{
		actor->orders.look.primary_priority = _primary_priority_facing;
		actor->orders.look.primary_direction.type = _direction_specification_movement;
	}
	else if (!actor->external_orders.pursuit_is_coordinator && actor->external_orders.pursuit_group_prop_index != NONE && state_data->look_timer > 0)
	{
		actor->orders.look.primary_priority = _primary_priority_aiming;
		actor->orders.look.primary_direction.type = _direction_specification_prop;
		actor->orders.look.primary_direction.prop_index = actor->external_orders.pursuit_group_prop_index;
	}
	else
	{
		actor->orders.look.primary_priority = _primary_priority_face_360;
	}

	actor->orders.look.idle_look_type = _idle_look_searching;
	actor->orders.combat.shoot_at_target = FALSE;
	actor->orders.move.stationary_crouch = FALSE;
	actor->orders.move.moving_crouch = FALSE;
	actor->orders.move.panicked = FALSE;
	actor->orders.move.dive_into_cover = FALSE;
	actor->orders.move.emerge_from_cover = FALSE;

	return;
}

/* ---------- private code */
