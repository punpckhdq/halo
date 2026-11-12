/*
ACTION_CONVERSE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_communication.h"
#include "ai_scenario_definitions.h"
#include "encounters.h"
#include "props.h"
#include "units.h"
#include "scenario.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

boolean action_converse_setup(
	long actor_index,
	long conversation_index,
	struct converse_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct conversation_datum *conversation = conversation_get(conversation_index);
	struct ai_conversation *conversation_definition = ai_conversation_definition_get(conversation->conversation_definition_index);

	match_assert("c:\\halo\\SOURCE\\ai\\action_converse.c", 33, state_data);
	memset(state_data, 0, sizeof(*state_data));

	state_data->conversation_index = conversation_index;
	state_data->run_to_distance = conversation_definition->run_to_player_dist;
	state_data->run_to_unit_index = (state_data->run_to_distance == 0.f) ? NONE : conversation->triggering_player_unit_index;
	state_data->run_to_prop_index = NONE;
	state_data->in_range = FALSE;

	return TRUE;
}

void action_converse_begin(
	long actor_index)
{
	return;
}

boolean action_converse_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct converse_state_data *state_data = &actor->state.action_data.converse;

	if (actor->meta.timeslice)
	{
		if (state_data->run_to_prop_index == NONE && state_data->run_to_unit_index != NONE)
		{
			state_data->run_to_prop_index = prop_get_base_by_unit_index(actor_index, state_data->run_to_unit_index, TRUE, TRUE);
		}

		if (state_data->run_to_prop_index == NONE)
		{
			state_data->failed = TRUE;
		}
		else
		{
			if (!state_data->in_range)
			{
				struct prop_datum *prop = prop_get(state_data->run_to_prop_index);

				if (prop->visibility >= _actor_perception_full && prop->distance < state_data->run_to_distance)
				{
					state_data->in_range = TRUE;
				}
				else if (prop->distance < 0.7f)
				{
					state_data->in_range = TRUE;
				}
			}

			if (state_data->in_range)
			{
				actor_move_halt(actor_index);
			}
			else if (!actor_move_to_prop(actor_index, state_data->run_to_prop_index, state_data->run_to_distance))
			{
				state_data->failed = TRUE;
			}
		}
	}

	return state_data->failed;
}

void action_converse_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct converse_state_data *state_data = &actor->state.action_data.converse;

	return;
}

void action_converse_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct converse_state_data *state_data = &actor->state.action_data.converse;
	struct conversation_datum *conversation = NULL;
	long prop_index = NONE;

	if (state_data->conversation_index != NONE)
	{
		conversation = conversation_get(state_data->conversation_index);
	}

	if (state_data->run_to_prop_index != NONE)
	{
		prop_index = state_data->run_to_prop_index;
	}
	else if (conversation && conversation->triggering_player_unit_index != NONE)
	{
		prop_index = prop_get_active_by_unit_index(actor_index, conversation->triggering_player_unit_index);
	}

	actor->orders.look.idle_look_type = _idle_look_noncombat;
	if (prop_index != NONE)
	{
		actor->orders.look.primary_priority = _primary_priority_facing;
		actor->orders.look.primary_direction.type = _direction_specification_prop;
		actor->orders.look.primary_direction.prop_index = prop_index;
	}

	return;
}

void action_converse_replace_prop(
	long actor_index,
	long invalid_prop_index,
	long replacement_prop_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct converse_state_data *state_data = &actor->state.action_data.converse;

	if (state_data->run_to_prop_index == invalid_prop_index)
	{
		state_data->run_to_prop_index = replacement_prop_index;
	}

	return;
}

void actor_conversation_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->external_orders.conversation_index != NONE && actor->external_orders.conversation_attention_unit_index != NONE)
	{
		struct direction_specification direction;
		long prop_index = prop_get_active_by_unit_index(actor_index, actor->external_orders.conversation_attention_unit_index);

		if (prop_index != NONE)
		{
			direction.type = _direction_specification_prop;
			direction.prop_index = prop_index;
		}
		else
		{
			direction.type = _direction_specification_point;
			unit_get_head_position(actor->external_orders.conversation_attention_unit_index, &direction.point);
		}

		actor_look_secondary(actor_index, _secondary_look_communicating_prop, _secondary_look_priority_turn_and_aim, &direction);
	}

	return;
}

void actor_conversation_end(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->external_orders.conversation_index != NONE)
	{
		ai_conversation_finish(actor->external_orders.conversation_index, FALSE, FALSE);
	}

	return;
}

void action_converse_end(
	long actor_index)
{
	actor_conversation_end(actor_index);

	return;
}

/* ---------- private code */
