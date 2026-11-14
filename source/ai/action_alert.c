/*
ACTION_ALERT.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "actions.h"
#include "encounters.h"
#include "props.h"
#include "units.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static short action_alert_next_position(long actor_index, short move_position_order, short current_position_index, boolean *direction_increasing);

/* ---------- globals */

/* ---------- public code */

boolean action_alert_setup(
	long actor_index,
	short move_position_order,
	short initial_move_position_index,
	struct alert_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\action_alert.c", 35, state_data);
	memset(state_data, 0, sizeof(*state_data));

	if (actor->meta.swarm)
	{
		move_position_order = _move_position_order_none;
	}
	state_data->move_position_order = move_position_order;
	state_data->target_move_position_index = NONE;
	state_data->pending_move_position_index = initial_move_position_index;
	state_data->move_position_increasing = TRUE;
	state_data->wait_ticks = 0;
	state_data->must_play_animation = FALSE;

	return TRUE;
}

void action_alert_begin(
	long actor_index)
{
	return;
}

boolean action_alert_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct alert_state_data *state_data = &actor->state.action_data.alert;

	if (state_data->move_position_order != _move_position_order_none && state_data->pending_move_position_index == NONE)
	{
		boolean arrived = TRUE;

		match_assert("c:\\halo\\SOURCE\\ai\\action_alert.c", 71, !actor->meta.swarm);
		if (state_data->target_move_position_index != NONE && actor_path_has_path(actor_index))
		{
			real distance_squared = distance_squared3d(&actor->input.position.body_position, &state_data->target_position_definition.position);
			real tolerance = actor_destination_tolerance(actor_index);

			tolerance = MAX(tolerance, 0.5f);
			if (distance_squared > tolerance * tolerance)
			{
				arrived = FALSE;
			}
		}

		if (arrived)
		{
			if (state_data->wait_ticks > 0)
			{
				arrived = FALSE;
			}
			else if (state_data->must_play_animation)
			{
				arrived = FALSE;
			}
			else
			{
				struct unit_datum *unit = unit_get(actor->meta.unit_index);

				if (unit->unit.animation.state == _unit_state_user_animation)
				{
					arrived = FALSE;
				}
			}
		}

		if (arrived)
		{
			state_data->pending_move_position_index = action_alert_next_position(
				actor_index,
				state_data->move_position_order,
				state_data->target_move_position_index,
				&state_data->move_position_increasing);
		}
	}

	if (actor->meta.timeslice && !actor->meta.dormant && state_data->pending_move_position_index != NONE)
	{
		boolean success = FALSE;

		match_assert("c:\\halo\\SOURCE\\ai\\action_alert.c", 120, !actor->meta.swarm);
		if (actor->meta.encounter_index != NONE)
		{
			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
				struct encounter_definition);
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->squads,
				actor->meta.squad_index,
				struct squad_definition);

			if (state_data->pending_move_position_index >= 0 &&
				state_data->pending_move_position_index < squad_definition->move_positions.count)
			{
				struct move_position_definition *move_position = TAG_BLOCK_GET_ELEMENT(
					&squad_definition->move_positions,
					state_data->pending_move_position_index,
					struct move_position_definition);
				real wait_time = real_random_range(move_position->time_lower_bound, move_position->time_upper_bound);

				state_data->target_move_position_index = state_data->pending_move_position_index;
				state_data->pending_move_position_index = NONE;
				state_data->target_position_definition = *move_position;
				state_data->wait_ticks = (short)(wait_time * 30.f);
				state_data->must_play_animation = TRUE;
				if (actor_move_to_move_position(actor_index, state_data->target_move_position_index))
				{
					success = TRUE;
				}
			}
		}

		if (!success)
		{
			state_data->target_move_position_index = state_data->pending_move_position_index;
			state_data->pending_move_position_index = NONE;
			state_data->wait_ticks = 0;
			state_data->must_play_animation = FALSE;
		}
	}

	return FALSE;
}

void action_alert_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct alert_state_data *state_data = &actor->state.action_data.alert;

	if (!actor->meta.dormant && state_data->target_move_position_index != NONE)
	{
		real distance_squared = distance_squared3d(&actor->input.position.body_position, &state_data->target_position_definition.position);

		match_assert("c:\\halo\\SOURCE\\ai\\action_alert.c", 174, !actor->meta.swarm);
		if (actor_path_at_destination(actor_index) || distance_squared < 0.25f)
		{
			if (state_data->wait_ticks > 0)
			{
				state_data->wait_ticks--;
			}

			if (state_data->must_play_animation)
			{
				if (state_data->target_position_definition.animation_reference_index != NONE)
				{
					struct ai_animation_reference_definition *animation_reference = TAG_BLOCK_GET_ELEMENT(
						&global_scenario_get()->ai_animation_references,
						state_data->target_position_definition.animation_reference_index,
						struct ai_animation_reference_definition);
					long animation_graph_index = animation_reference->animation_graph.index;

					if (animation_graph_index == NONE)
					{
						struct unit_datum *unit = unit_get(actor->meta.unit_index);
						struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

						animation_graph_index = unit_definition->object.animation_graph.index;
					}

					unit_start_user_animation(actor->meta.unit_index, animation_graph_index, animation_reference->animation_name, TRUE);
				}

				state_data->must_play_animation = FALSE;
			}
		}
	}

	return;
}

void action_alert_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	actor->orders.look.idle_look_type = _idle_look_noncombat;
	if (TEST_FLAG(actor_definition->flags, _actor_definition_crouch_when_noncombat_bit))
	{
		actor->orders.move.stationary_crouch = TRUE;
		actor->orders.move.moving_crouch = TRUE;
	}

	return;
}

void action_alert_flush_position_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct alert_state_data *state_data = &actor->state.action_data.alert;

	state_data->target_move_position_index = NONE;
	state_data->pending_move_position_index = NONE;

	return;
}

void action_alert_flush_structure_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct alert_state_data *state_data = &actor->state.action_data.alert;

	state_data->target_position_definition.cluster_index = NONE;
	state_data->target_position_definition.surface_index = NONE;

	return;
}

static short action_alert_next_position(
	long actor_index,
	short move_position_order,
	short current_position_index,
	boolean *direction_increasing)
{
	struct actor_datum *actor = actor_get(actor_index);
	short result = NONE;

	if (actor->input.vehicle_passenger || move_position_order == _move_position_order_none)
	{
		result = NONE;
	}
	else if (actor->meta.encounter_index != NONE)
	{
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
			struct encounter_definition);
		struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
			&encounter_definition->squads,
			actor->meta.squad_index,
			struct squad_definition);

		match_assert("c:\\halo\\SOURCE\\ai\\action_alert.c", 275, !actor->meta.swarm);
		if (move_position_order == _move_position_order_repeat && current_position_index != NONE)
		{
			result = current_position_index;
		}
		else
		{
			unsigned long invalid_move_positions[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_MOVE_POSITIONS_PER_SQUAD)];
			boolean valid_position_found = FALSE;
			short move_position_index;

			memset(invalid_move_positions, 0, sizeof(invalid_move_positions));
			for (move_position_index = 0; move_position_index < squad_definition->move_positions.count; move_position_index++)
			{
				struct move_position_definition *move_position = TAG_BLOCK_GET_ELEMENT(
					&squad_definition->move_positions,
					move_position_index,
					struct move_position_definition);
				boolean valid = TRUE;
				struct prop_iterator iterator;
				struct prop_datum *prop;

				if (move_position_index == current_position_index)
				{
					valid = FALSE;
				}

				if (current_position_index != NONE &&
					distance_squared3d(&actor->input.position.body_position, &move_position->position) < 0.25f)
				{
					valid = FALSE;
				}

				if (move_position->sequence_id && move_position->sequence_id != actor->state.noncombat_sequence_id)
				{
					valid = FALSE;
				}

				prop_iterator_new(&iterator, actor_index);
				while (prop = prop_iterator_next(&iterator))
				{
					if (prop->state >= _prop_state_becoming_unacknowledged && prop->state <= _prop_state_acknowledged &&
						distance_squared3d(&prop->body_position, &move_position->position) < 0.25f)
					{
						valid = FALSE;
						break;
					}
				}

				if (valid)
				{
					valid_position_found = TRUE;
				}
				else
				{
					BIT_VECTOR_SET_FLAG(invalid_move_positions, move_position_index, TRUE);
				}
			}

			if (!valid_position_found)
			{
				result = NONE;
			}
			else if (move_position_order == _move_position_order_random)
			{
				result = choose_random_array_element(
					squad_definition->move_positions.address,
					sizeof(struct move_position_definition),
					(short)squad_definition->move_positions.count,
					offsetof(struct move_position_definition, weight),
					invalid_move_positions);
			}
			else
			{
				if (current_position_index >= 0 && current_position_index < squad_definition->move_positions.count)
				{
					result = current_position_index;
				}
				else
				{
					result = 0;
				}

				do
				{
					boolean increasing = TRUE;

					switch (move_position_order)
					{
					case _move_position_order_loop:
						increasing = TRUE;
						break;
					case _move_position_order_loop_back_and_forth:
						if (result == 0)
						{
							increasing = TRUE;
						}
						else if (result == squad_definition->move_positions.count - 1)
						{
							increasing = FALSE;
						}
						else
						{
							increasing = direction_increasing ? *direction_increasing : TRUE;
						}
						break;
					case _move_position_order_loop_randomly:
						increasing = game_time_get() & 1;
						break;
					}

					if (direction_increasing)
					{
						*direction_increasing = increasing;
					}

					if (increasing)
					{
						result++;
						if (result >= squad_definition->move_positions.count)
						{
							result = 0;
						}
					}
					else
					{
						result--;
						if (result < 0)
						{
							result = (short)squad_definition->move_positions.count - 1;
						}
					}
				} while (BIT_VECTOR_TEST_FLAG(invalid_move_positions, result));
			}
		}
	}

	return result;
}

/* ---------- private code */
