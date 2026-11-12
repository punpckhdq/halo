/*
ACTION_SEARCH.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai.h"
#include "ai_communication.h"
#include "ai_scenario_definitions.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "units.h"
#include "scenario.h"
#include "game.h"

/* ---------- constants */

enum
{
	SEARCH_FAILURE_TICKS = 120, /* fake name */
	SEARCH_LOST_CONTACT_TICKS = 90, /* fake name */
	SEARCH_MINIMUM_LOOK_TICKS = 90, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

boolean action_search_setup_target(
	long actor_index,
	boolean must_charge,
	struct search_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_search.c", 33, state_data);
	memset(state_data, 0, sizeof(*state_data));

	if (!actor->input.vehicle_passenger)
	{
		state_data->pursuit_location.type = _pursuit_location_target;
		state_data->charging = must_charge;
		actor->state.searching = TRUE;
		result = TRUE;
	}

	return result;
}

boolean action_search_setup_pursuit(
	long actor_index,
	short firing_position_index,
	boolean tenacious,
	struct search_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_search.c", 56, state_data);
	memset(state_data, 0, sizeof(*state_data));

	if (!actor->input.vehicle_passenger &&
		!actor->meta.swarm &&
		actor->meta.encounter_index != NONE &&
		firing_position_index != NONE)
	{
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
			struct encounter_definition);
		struct firing_position_definition *firing_position = TAG_BLOCK_GET_ELEMENT(
			&encounter_definition->firing_positions,
			firing_position_index,
			struct firing_position_definition);

		state_data->tenacious = tenacious;
		state_data->pursuit_location.type = _pursuit_location_position;
		state_data->pursuit_location.firing_position_index = firing_position_index;
		state_data->pursuit_location.position = firing_position->position;
		state_data->pursuit_location.surface_index = firing_position->surface_index;
		state_data->pursuit_location.cluster_index = firing_position->cluster_index;
		actor->state.searching = TRUE;
		result = TRUE;
	}

	return result;
}

boolean action_search_setup_undirected(
	long actor_index,
	boolean tenacious,
	struct search_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_search.c", 87, state_data);
	memset(state_data, 0, sizeof(*state_data));

	if (actor->meta.swarm)
	{
		state_data->pursuit_location.type = _pursuit_location_undirected;
		actor->state.searching = TRUE;
		result = TRUE;
	}

	return result;
}

void action_search_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct search_state_data *state_data = &actor->state.action_data.search;

	if (!state_data->search_done)
	{
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

		if (actor_definition->defensive.defensive_crouch_type == _defensive_crouch_any_target)
		{
			state_data->sneaking = TRUE;
		}
		else
		{
			state_data->sneaking = FALSE;
			if (TEST_FLAG(actor_definition->flags, _actor_definition_sneak_uncovering_target_bit) &&
				state_data->pursuit_location.type == _pursuit_location_target &&
				actor->target.target_type == _actor_target_uninspected_orphan)
			{
				struct prop_datum *prop = prop_get(actor->target.target_prop_index);

				if (prop->quantized_distance <= _prop_distance_middle)
				{
					state_data->sneaking = TRUE;
				}
			}
		}

		if (state_data->at_destination)
		{
			if (state_data->search_remaining_time > 0)
			{
				state_data->search_remaining_time--;
			}

			if (state_data->search_remaining_time == 0)
			{
				state_data->search_done = TRUE;
			}

			if (actor->meta.unit_index != NONE)
			{
				if (state_data->pursuit_location.type == _pursuit_location_target)
				{
					if (!actor->firing_positions.pursuit_communicated_lost_contact &&
						(state_data->search_done || state_data->search_remaining_time + SEARCH_LOST_CONTACT_TICKS < state_data->search_desired_time))
					{
						ai_communication_event(
							_ai_communication_lost_contact,
							actor->meta.unit_index,
							actor_target_unit_index(actor_index),
							NONE,
							NONE,
							NONE,
							NULL);
						actor->firing_positions.pursuit_communicated_lost_contact = TRUE;
					}
				}
				else if (state_data->search_remaining_time == 0)
				{
					ai_communication_event(
						_ai_communication_search_report,
						actor->meta.unit_index,
						actor_target_unit_index(actor_index),
						NONE,
						NONE,
						NONE,
						NULL);
				}
			}
		}
		else if (!actor->control.moving && !actor->meta.swarm)
		{
			if (++state_data->search_failure_timer > SEARCH_FAILURE_TICKS)
			{
				state_data->search_failed = TRUE;
				state_data->search_done = TRUE;
			}
		}
	}

	return;
}

void action_search_flush_position_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct search_state_data *state_data = &actor->state.action_data.search;

	if (state_data->pursuit_location.type == _pursuit_location_position)
	{
		state_data->pursuit_location.firing_position_index = NONE;
		state_data->search_done = TRUE;
	}

	return;
}

void action_search_flush_structure_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct search_state_data *state_data = &actor->state.action_data.search;

	state_data->pursuit_location.cluster_index = NONE;
	state_data->pursuit_location.surface_index = NONE;

	return;
}

void action_search_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct search_state_data *state_data = &actor->state.action_data.search;

	if (actor->control.moving)
	{
		actor->orders.look.primary_priority = _primary_priority_facing;
		actor->orders.look.primary_direction.type = _direction_specification_movement;
	}
	else
	{
		long look_ticks = MAX(state_data->search_desired_time / 3, SEARCH_MINIMUM_LOOK_TICKS);
		boolean looking = FALSE;

		if (state_data->search_desired_time - state_data->search_remaining_time < look_ticks)
		{
			if (state_data->pursuit_location.type == _pursuit_location_target)
			{
				actor->orders.look.primary_priority = _primary_priority_facing;
				actor->orders.look.primary_direction.type = _direction_specification_target;
				looking = TRUE;
			}
			else if (state_data->pursuit_location.type == _pursuit_location_position)
			{
				actor->orders.look.primary_priority = _primary_priority_facing;
				actor->orders.look.primary_direction.type = _direction_specification_point;
				actor->orders.look.primary_direction.point = state_data->pursuit_location.position;
				looking = TRUE;
			}
		}

		if (!looking)
		{
			actor->orders.look.primary_priority = _primary_priority_face_360;
		}
	}

	actor->orders.look.idle_look_type = _idle_look_searching;
	if (state_data->pursuit_location.type == _pursuit_location_target)
	{
		if (TEST_FLAG(actor_definition->flags, _actor_definition_shoot_at_targets_last_location_bit))
		{
			actor->orders.combat.shoot_at_target = actor->target.target_type >= _actor_target_uninspected_orphan;
		}
		else
		{
			actor->orders.combat.shoot_at_target = actor->target.target_type >= _actor_target_definite_orphan;
		}
	}

	actor->orders.move.stationary_crouch = state_data->sneaking;
	actor->orders.move.moving_crouch = state_data->sneaking;
	actor->orders.move.panicked = FALSE;
	actor->orders.move.dive_into_cover = FALSE;
	actor->orders.move.emerge_from_cover = TRUE;

	return;
}

void action_search_begin(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct search_state_data *state_data = &actor->state.action_data.search;
	real search_time;

	if (state_data->pursuit_location.type == _pursuit_location_target)
	{
		search_time = real_random_range(
			actor_definition->pursuit.target_location_time_lower_bound,
			actor_definition->pursuit.target_location_time_upper_bound);
	}
	else
	{
		search_time = real_random_range(
			actor_definition->pursuit.pursuit_position_time_lower_bound,
			actor_definition->pursuit.pursuit_position_time_upper_bound);
	}

	state_data->search_desired_time = (long)(search_time * TICKS_PER_SECOND);
	state_data->search_remaining_time = state_data->search_desired_time;

	return;
}

boolean action_search_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct search_state_data *state_data = &actor->state.action_data.search;

	if (!actor->meta.swarm && actor->meta.timeslice && !state_data->search_done)
	{
		state_data->at_destination = TRUE;
		if (state_data->pursuit_location.type == _pursuit_location_target && actor->target.target_prop_index != NONE)
		{
			struct prop_datum *prop = prop_get(actor->target.target_prop_index);
			real range = (prop->line_of_sight == _ai_line_of_sight_clear) ? 1.7f : 0.7f;

			state_data->at_destination = distance_squared3d(&prop->body_position, &actor->input.position.body_position) < range * range;
		}
		else if (state_data->pursuit_location.type == _pursuit_location_position &&
			state_data->pursuit_location.firing_position_index != NONE)
		{
			real distance_squared = distance_squared3d(&state_data->pursuit_location.position, &actor->input.position.body_position);

			if (distance_squared < 0.7f * 0.7f)
			{
				state_data->at_destination = TRUE;
			}
			else if (distance_squared > 2.5f * 2.5f)
			{
				state_data->at_destination = FALSE;
			}
			else
			{
				real_point3d calculated_head_position;
				short line_of_sight;

				unit_estimate_position(
					actor->meta.unit_index,
					_unit_estimate_head_standing,
					&state_data->pursuit_location.position,
					NULL,
					NULL,
					&calculated_head_position);
				line_of_sight = ai_test_line_of_sight(
					&actor->input.position.head_position,
					actor->input.position.body_location.cluster_index,
					&calculated_head_position,
					state_data->pursuit_location.cluster_index,
					0,
					FALSE,
					NONE,
					actor->input.vehicle_index != NONE);
				state_data->at_destination = line_of_sight == _ai_line_of_sight_clear;
			}
		}

		if (!state_data->at_destination && !state_data->charging)
		{
			short searching_actor_count = 0;
			short nearby_stationary_actor_count = 0;
			short required_actor_count;
			struct prop_iterator iterator;
			struct prop_datum *prop;

			prop_iterator_new(&iterator, actor_index);
			while (prop = prop_iterator_next(&iterator))
			{
				if (prop->state >= _prop_state_becoming_unacknowledged &&
					prop->state <= _prop_state_acknowledged &&
					!prop->enemy &&
					!prop->dead &&
					prop->actor_index != NONE &&
					actors_searching_same_position(actor_index, prop->actor_index))
				{
					struct actor_datum *friend_actor = actor_get(prop->actor_index);

					searching_actor_count++;
					if (!friend_actor->meta.swarm &&
						!friend_actor->control.moving &&
						distance_squared3d(&actor->input.position.body_position, &friend_actor->input.position.body_position) < 0.8f * 0.8f)
					{
						nearby_stationary_actor_count++;
					}
				}
			}

			required_actor_count = (state_data->pursuit_location.type == _pursuit_location_position) ? 2 : 4;
			if (!state_data->tenacious && searching_actor_count >= required_actor_count)
			{
				long history_start_time = NONE;

				state_data->search_done = TRUE;
				if (actor->target.target_prop_index != NONE)
				{
					struct prop_datum *target_prop = prop_get(actor->target.target_prop_index);

					history_start_time = target_prop->last_perceived_time;
				}

				if (actor->meta.encounter_index != NONE)
				{
					encounter_mark_examined_pursuit_position(
						actor->meta.encounter_index,
						actor_index,
						state_data->pursuit_location.firing_position_index,
						history_start_time);
				}
			}
			else if (nearby_stationary_actor_count > 0)
			{
				state_data->at_destination = TRUE;
			}
		}

		if (state_data->at_destination)
		{
			actor_move_halt(actor_index);
		}
		else if (state_data->pursuit_location.type == _pursuit_location_target)
		{
			if (!actor_move_to_prop(actor_index, actor->target.target_prop_index, 2.5f))
			{
				state_data->search_failed = TRUE;
				state_data->search_done = TRUE;
			}
		}
		else if (state_data->pursuit_location.type == _pursuit_location_position)
		{
			actor->firing_positions.current_position_index = NONE;
			if (!actor_move_to_firing_position(actor_index, state_data->pursuit_location.firing_position_index, NULL))
			{
				state_data->search_failed = TRUE;
				state_data->search_done = TRUE;
			}
		}
		else
		{
			actor_move_halt(actor_index);
		}
	}

	return state_data->search_done;
}

/* ---------- private code */
