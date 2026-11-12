/*
ACTION_UNCOVER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai.h"
#include "ai_communication.h"
#include "ai_debug.h"
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
	UNCOVER_HOLD_POSITION_TICKS = 30, /* fake name */
	UNCOVER_PERSISTENT_TIMEOUT_TICKS = 360, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

boolean action_uncover_setup_target(
	long actor_index,
	boolean able_to_search,
	struct uncover_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_uncover.c", 34, state_data);
	memset(state_data, 0, sizeof(*state_data));

	if (!actor->input.vehicle_passenger && !actor->meta.swarm)
	{
		state_data->pursuit_location.type = _pursuit_location_target;
		state_data->able_to_search = able_to_search;
		result = TRUE;
	}

	return result;
}

boolean action_uncover_setup_pursuit(
	long actor_index,
	short firing_position_index,
	struct uncover_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_uncover.c", 56, state_data);
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

		state_data->pursuit_location.type = _pursuit_location_position;
		state_data->pursuit_location.firing_position_index = firing_position_index;
		state_data->pursuit_location.position = firing_position->position;
		state_data->pursuit_location.surface_index = firing_position->surface_index;
		state_data->pursuit_location.cluster_index = firing_position->cluster_index;
		state_data->pursuit_location_inspected = FALSE;
		state_data->able_to_search = TRUE;
		actor->state.searching = TRUE;
		result = TRUE;
	}

	return result;
}

boolean action_uncover_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct uncover_state_data *state_data = &actor->state.action_data.uncover;

	match_assert("c:\\halo\\SOURCE\\ai\\action_uncover.c", 132, !actor->meta.swarm);

	if (actor->meta.timeslice && !actor->input.vehicle_passenger && !state_data->uncover_done)
	{
		struct firing_position_evaluation_context evaluation_context;
		struct firing_position new_firing_position;
		struct path_state path_state;
		boolean area_path_valid;
		long previous_owner;
		short firing_position_index;

		memset(&evaluation_context, 0, sizeof(evaluation_context));
		evaluation_context.evaluation_mode = _firing_point_evaluation_mode_uncover;
		if (state_data->pursuit_location.type == _pursuit_location_position)
		{
			evaluation_context.specific_target_enable = TRUE;
			evaluation_context.specific_target_point = state_data->pursuit_location.position;
			evaluation_context.specific_target_surface_index = state_data->pursuit_location.surface_index;
			evaluation_context.specific_target_cluster_index = state_data->pursuit_location.cluster_index;
		}
		else
		{
			evaluation_context.use_last_visible_target_position = state_data->no_target_sight_available;
		}

		firing_position_index = actor_active_select_firing_position(
			actor_index,
			&evaluation_context,
			&new_firing_position,
			&previous_owner,
			&path_state,
			&area_path_valid);

		if (firing_position_index != NONE)
		{
			if (state_data->pursuit_location.type == _pursuit_location_target)
			{
				if (new_firing_position.line_of_sight != _ai_line_of_sight_clear && new_firing_position.line_of_sight != _ai_line_of_sight_occluded)
				{
					if (!state_data->no_target_sight_available && ai_debug.print_uncovering)
					{
						char actor_name[256];

						ai_debug_describe_actor(actor_index, NONE, TRUE, actor_name, sizeof(actor_name));
						error(_error_silent, "%s: unable to see target's current location", actor_name);
					}

					state_data->no_target_sight_available = TRUE;
				}
			}
			else if (new_firing_position.line_of_sight == _ai_line_of_sight_clear &&
				actor_destination_tolerance(actor_index) > new_firing_position.path_distance_from_actor)
			{
				if (!state_data->pursuit_location_inspected && ai_debug.print_uncovering)
				{
					char actor_name[256];

					ai_debug_describe_actor(actor_index, NONE, TRUE, actor_name, sizeof(actor_name));
					error(_error_silent, "%s: inspected pursuit location", actor_name);
				}

				state_data->pursuit_location_inspected = TRUE;
			}
		}

		firing_position_index = actor_change_firing_position(
			actor_index,
			firing_position_index,
			&new_firing_position,
			previous_owner,
			&path_state,
			area_path_valid);
		if (firing_position_index == NONE)
		{
			state_data->uncover_exit_failure = TRUE;
		}
	}

	return FALSE;
}

void action_uncover_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct uncover_state_data *state_data = &actor->state.action_data.uncover;

	if (actor->target.target_prop_index != NONE)
	{
		struct prop_datum *prop = prop_get(actor->target.target_prop_index);
		boolean shooting_at_target = FALSE; /* fake name */

		if (state_data->pursuit_location.type == _pursuit_location_target)
		{
			if (actor->input.vehicle_gunner_bombardment)
			{
				actor->orders.combat.shoot_at_target = TRUE;
				actor->orders.combat.bombard_target = TRUE;
				shooting_at_target = TRUE;
			}
			else if (TEST_FLAG(actor_definition->flags, _actor_definition_shoot_at_targets_last_location_bit))
			{
				actor->orders.combat.shoot_at_target = actor->target.target_type >= _actor_target_uninspected_orphan;
			}
			else
			{
				actor->orders.combat.shoot_at_target = actor->target.target_type >= _actor_target_definite_orphan;
			}
		}

		if (actor->orders.combat.shoot_at_target &&
			(prop->line_of_sight == _ai_line_of_sight_clear || prop->line_of_sight == _ai_line_of_sight_occluded))
		{
			shooting_at_target = TRUE;
		}

		if (shooting_at_target)
		{
			actor->orders.look.primary_priority = _primary_priority_locked_aiming;
		}
		else if (actor->target.target_type < _actor_target_uninspected_orphan)
		{
			actor->orders.look.primary_priority = _primary_priority_facing;
		}
		else if (prop->line_of_sight == _ai_line_of_sight_from_cover || prop->line_of_sight == _ai_line_of_sight_obstructed)
		{
			actor->orders.look.primary_priority = _primary_priority_opportunity_aiming;
		}
		else
		{
			actor->orders.look.primary_priority = _primary_priority_aiming;
		}

		if (state_data->pursuit_location.type == _pursuit_location_target)
		{
			actor->orders.look.primary_direction.type = _direction_specification_target;
		}
		else if (state_data->pursuit_location.type == _pursuit_location_position)
		{
			actor->orders.look.primary_direction.type = _direction_specification_point;
			actor->orders.look.primary_direction.point = state_data->pursuit_location.position;
		}
	}

	actor->orders.look.idle_look_type = _idle_look_searching;
	actor->orders.move.stationary_crouch = state_data->sneaking;
	actor->orders.move.moving_crouch = state_data->sneaking;
	actor->orders.move.panicked = FALSE;
	actor->orders.move.dive_into_cover = FALSE;
	actor->orders.move.emerge_from_cover = TRUE;

	return;
}

void action_uncover_flush_position_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct uncover_state_data *state_data = &actor->state.action_data.uncover;

	if (state_data->pursuit_location.type == _pursuit_location_position)
	{
		state_data->pursuit_location.firing_position_index = NONE;
		state_data->uncover_done = TRUE;
	}

	return;
}

void action_uncover_flush_structure_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct uncover_state_data *state_data = &actor->state.action_data.uncover;

	state_data->pursuit_location.cluster_index = NONE;
	state_data->pursuit_location.surface_index = NONE;

	return;
}

void action_uncover_modify_color(
	long actor_index,
	real_argb_color *color)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct uncover_state_data *state_data = &actor->state.action_data.uncover;

	if (state_data->sneaking)
	{
		*color = *global_real_argb_blue;
	}
	else
	{
		*color = *global_real_argb_lightblue;
	}

	return;
}

void action_uncover_begin(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct uncover_state_data *state_data = &actor->state.action_data.uncover;
	real time_lower_bound = actor_definition->pursuit.uncover_delay_time_lower_bound;
	real time_upper_bound = actor_definition->pursuit.uncover_delay_time_upper_bound;
	real uncover_time;

	if (!state_data->able_to_search)
	{
		time_lower_bound = MAX(time_lower_bound, actor_definition->pursuit.target_location_time_lower_bound);
		time_upper_bound = MAX(time_upper_bound, actor_definition->pursuit.target_location_time_upper_bound);
	}

	uncover_time = real_random_range(time_lower_bound, time_upper_bound);
	state_data->uncover_desired_time = (long)(uncover_time * TICKS_PER_SECOND);
	state_data->uncover_remaining_time = state_data->uncover_desired_time;

	if (ai_debug.print_uncovering)
	{
		char actor_name[256];

		ai_debug_describe_actor(actor_index, NONE, TRUE, actor_name, sizeof(actor_name));
		error(
			_error_silent,
			"%s: begin %s uncover for [%.1f-%.1f] = %.1f (%sable to search)",
			actor_name,
			(state_data->pursuit_location.type == _pursuit_location_target) ? "target" : "pursuit",
			time_lower_bound,
			time_upper_bound,
			uncover_time,
			state_data->able_to_search ? "" : "un");
	}

	if (state_data->pursuit_location.type == _pursuit_location_target &&
		actor->target.target_prop_index != NONE &&
		actor->state.combat_status < _actor_combat_status_definite)
	{
		struct prop_datum *prop = prop_get(actor->target.target_prop_index);

		ai_communication_event(_ai_communication_uncover_start, actor->meta.unit_index, prop->unit_index, NONE, NONE, NONE, NULL);
	}

	return;
}

void action_uncover_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct uncover_state_data *state_data = &actor->state.action_data.uncover;

	if (!state_data->uncover_done)
	{
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
		boolean reset_timer = TRUE;
		boolean target_visible = FALSE;
		boolean making_progress; /* fake name */
		boolean done;

		state_data->sneaking = FALSE;
		if (state_data->pursuit_location.type == _pursuit_location_target)
		{
			if (actor_definition->defensive.defensive_crouch_type == _defensive_crouch_any_target)
			{
				state_data->sneaking = actor->target.target_type != _actor_target_definite_orphan;
			}
			else if (TEST_FLAG(actor_definition->flags, _actor_definition_sneak_uncovering_target_bit) &&
				actor->target.target_type == _actor_target_uninspected_orphan)
			{
				struct prop_datum *prop = prop_get(actor->target.target_prop_index);

				if (prop->quantized_distance <= _prop_distance_middle)
				{
					state_data->sneaking = TRUE;
				}
			}
		}
		else if (state_data->pursuit_location.type == _pursuit_location_position)
		{
			if (actor_definition->defensive.defensive_crouch_type == _defensive_crouch_any_target)
			{
				state_data->sneaking = TRUE;
			}
			else if (TEST_FLAG(actor_definition->flags, _actor_definition_sneak_uncovering_pursuit_position_bit))
			{
				real distance_squared = distance_squared3d(&actor->input.position.body_position, &state_data->pursuit_location.position);

				if (distance_squared < 100.f)
				{
					state_data->sneaking = TRUE;
				}
			}
		}

		if (actor->control.moving)
		{
			state_data->current_position_hold_timer = 0;
		}
		else
		{
			state_data->current_position_hold_timer++;
			if (state_data->pursuit_location.type == _pursuit_location_target &&
				state_data->current_position_hold_timer >= UNCOVER_HOLD_POSITION_TICKS)
			{
				actor_discard_firing_position(actor_index, actor->firing_positions.current_position_index, FALSE);
			}
		}

		if (state_data->pursuit_location.type == _pursuit_location_target)
		{
			if (actor->target.target_prop_index != NONE)
			{
				struct prop_datum *prop = prop_get(actor->target.target_prop_index);

				target_visible = prop->visibility > _actor_perception_none;
				reset_timer = !target_visible || actor->target.target_type >= _actor_target_uninspected_orphan;
			}
		}
		else
		{
			reset_timer = !state_data->pursuit_location_inspected;
		}

		making_progress = TRUE;
		if (actor->firing_positions.current_position_index == NONE)
		{
			state_data->uncover_exit_failure = TRUE;
			making_progress = FALSE;
		}
		else if (!reset_timer)
		{
			making_progress = FALSE;
		}
		else if (!actor->input.vehicle_gunner_bombardment && !target_visible && !actor->control.moving)
		{
			making_progress = FALSE;
		}

		if (making_progress)
		{
			state_data->uncover_remaining_time = state_data->uncover_desired_time;
		}
		else
		{
			if (state_data->uncover_remaining_time > 0)
			{
				state_data->uncover_remaining_time--;
			}

			state_data->uncover_exit_persistent_timer++;
		}

		done = state_data->uncover_remaining_time == 0 || state_data->uncover_exit_persistent_timer >= UNCOVER_PERSISTENT_TIMEOUT_TICKS;
		if (state_data->pursuit_location.type == _pursuit_location_position && state_data->pursuit_location_inspected)
		{
			done = TRUE;
		}

		if (done && ai_debug.print_uncovering)
		{
			char actor_name[256];
			char reason[256];

			ai_debug_describe_actor(actor_index, NONE, TRUE, actor_name, sizeof(actor_name));
			if (state_data->uncover_remaining_time == 0)
			{
				sprintf(reason, "timer %d finished", state_data->uncover_desired_time);
			}
			else if (state_data->uncover_exit_persistent_timer >= UNCOVER_PERSISTENT_TIMEOUT_TICKS)
			{
				sprintf(reason, "persistent timer %d", state_data->uncover_exit_persistent_timer);
			}
			else if (state_data->pursuit_location.type == _pursuit_location_position && state_data->pursuit_location_inspected)
			{
				strcpy(reason, "location inspected");
			}
			else
			{
				strcpy(reason, "<unknown reason>");
			}

			error(
				_error_silent,
				"%s: %s uncover done: %s",
				actor_name,
				(state_data->pursuit_location.type == _pursuit_location_target) ? "target" : "pursuit",
				reason);
		}

		state_data->uncover_done = done;
	}

	return;
}

/* ---------- private code */
