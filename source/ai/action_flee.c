/*
ACTION_FLEE.C
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
	FLEE_FORCED_TICKS = 180, /* fake name */
	FLEE_STATIONARY_TICKS = 45, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void action_flee_find_flee_position(long actor_index, boolean currently_fleeing, struct flee_state_data *flee_state_data);
static boolean action_flee_current_position_exposed(long actor_index, struct flee_state_data *flee_state_data);
static boolean action_flee_at_flee_position(long actor_index);

/* ---------- globals */

/* ---------- public code */

void action_flee_end(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.unit_index != NONE)
	{
		unit_stop_running_blindly(actor->meta.unit_index);
	}

	return;
}

void action_flee_flush_position_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct flee_state_data *state_data = &actor->state.action_data.flee;

	state_data->flee_firing_position_index = NONE;
	state_data->find_new_flee_position = TRUE;

	return;
}

void action_flee_modify_color(
	long actor_index,
	real_argb_color *color)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct flee_state_data *state_data = &actor->state.action_data.flee;

	if (state_data->panic_type > _actor_panic_none)
	{
		*color = *global_real_argb_yellow;
	}
	else
	{
		*color = *global_real_argb_green;
	}

	return;
}

static void action_flee_find_flee_position(
	long actor_index,
	boolean currently_fleeing,
	struct flee_state_data *flee_state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct firing_position_evaluation_context evaluation_context;
	struct firing_position new_firing_position;
	long previous_owner;
	boolean path_state_valid;
	struct path_state path_state;

	match_assert("c:\\halo\\SOURCE\\ai\\action_flee.c", 522, !actor->meta.swarm);

	memset(&evaluation_context, 0, sizeof(evaluation_context));
	evaluation_context.use_last_visible_target_position = flee_state_data->flee_from_last_visible_location;

	if (flee_state_data->panic_type > _actor_panic_none)
	{
		evaluation_context.evaluation_mode = _firing_point_evaluation_mode_panic;
		if (flee_state_data->forced_flee_ticks > 0)
		{
			evaluation_context.allow_rejected_positions = TRUE;
			evaluation_context.allow_outside_range = TRUE;
		}
		evaluation_context.attractor_enable = TRUE;
		evaluation_context.attractor_weight = 10.f;
		evaluation_context.attractor_radius = 6.f;
	}
	else
	{
		evaluation_context.evaluation_mode = _firing_point_evaluation_mode_cover;
		evaluation_context.evaluation_data.cover.allow_occluded_points = flee_state_data->allow_occluded_points;
		if (actor_definition->defensive.cover_max_distance > 0.f)
		{
			evaluation_context.maximum_search_range = actor_definition->defensive.cover_max_distance;
		}
		else
		{
			evaluation_context.maximum_search_range = 6.f;
		}
	}

	evaluation_context.allowed_position_mask = actor_get_firing_position_group(actor_index, evaluation_context.evaluation_mode, 0);

	flee_state_data->flee_firing_position_index = actor_select_firing_position(
		actor_index,
		&evaluation_context,
		&new_firing_position,
		&previous_owner,
		&path_state,
		&path_state_valid);
	flee_state_data->flee_firing_position_index = actor_change_firing_position(
		actor_index,
		flee_state_data->flee_firing_position_index,
		&new_firing_position,
		previous_owner,
		&path_state,
		path_state_valid);
	flee_state_data->flee_firing_position_found_randomly = flee_state_data->flee_firing_position_index != NONE && !path_state_valid;

	flee_state_data->has_approach_point = FALSE;
	if (flee_state_data->flee_firing_position_index != NONE && flee_state_data->flee_prop_index != NONE)
	{
		struct prop_datum *prop = prop_get(flee_state_data->flee_prop_index);
		struct path_input input;
		struct path_state state;
		boolean dummy_straight_line;

		if (prop->state >= _prop_state_becoming_unacknowledged && prop->state <= _prop_state_acknowledged)
		{
			actor_perception_find_prop_pathfinding_location(actor_index, flee_state_data->flee_prop_index);
		}

		path_input_new(
			&input,
			actor_definition->moving.pathfinding_radius,
			FALSE,
			(prop->vehicle_index != NONE) ? prop->vehicle_index : prop->unit_index);
		path_input_set_start(&input, &prop->pathfinding_point, prop->pathfinding_surface_index);
		path_input_set_target_object(&input, actor->meta.unit_index);
		path_state_new(&input, &state, NULL);
		path_state_destination(
			&state,
			&new_firing_position.definition->position,
			new_firing_position.definition->surface_index,
			0.f);

		if (path_state_find(&state))
		{
			flee_state_data->has_approach_point = path_state_approach_point(
				&state,
				&new_firing_position.definition->position,
				new_firing_position.definition->surface_index,
				&dummy_straight_line,
				&flee_state_data->approach_point);
		}
	}

	flee_state_data->find_new_flee_position = FALSE;

	return;
}

static boolean action_flee_current_position_exposed(
	long actor_index,
	struct flee_state_data *flee_state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean exposed = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_flee.c", 613, !actor->meta.swarm);

	if (flee_state_data->flee_prop_index != NONE &&
		actor->meta.encounter_index != NONE &&
		flee_state_data->flee_firing_position_index != NONE)
	{
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
			struct encounter_definition);
		struct prop_datum *prop = prop_get(flee_state_data->flee_prop_index);
		struct firing_position_definition *firing_position = TAG_BLOCK_GET_ELEMENT(
			&encounter_definition->firing_positions,
			flee_state_data->flee_firing_position_index,
			struct firing_position_definition);
		real_point3d actor_head_position_at_firing_position;
		short line_of_sight;

		unit_estimate_position(
			actor->meta.unit_index,
			_unit_estimate_head_crouching,
			&firing_position->position,
			NULL,
			NULL,
			&actor_head_position_at_firing_position);
		line_of_sight = ai_test_line_of_sight(
			&actor_head_position_at_firing_position,
			firing_position->cluster_index,
			&prop->head_position,
			prop->body_location.cluster_index,
			1,
			FALSE,
			prop->vehicle_index,
			actor->input.vehicle_index != NONE);

		if (prop->state >= _prop_state_becoming_unacknowledged && prop->state <= _prop_state_acknowledged)
		{
			exposed = line_of_sight == _ai_line_of_sight_clear;
		}

		if (line_of_sight == _ai_line_of_sight_clear || line_of_sight == _ai_line_of_sight_to_cover)
		{
			flee_state_data->has_approach_point = TRUE;
			flee_state_data->approach_point = prop->body_position;
		}
	}

	return exposed;
}

void action_flee_replace_prop(
	long actor_index,
	long invalid_prop_index,
	long replacement_prop_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct flee_state_data *state_data = &actor->state.action_data.flee;

	if (state_data->flee_prop_index == invalid_prop_index)
	{
		state_data->flee_prop_index = replacement_prop_index;
	}

	return;
}

boolean action_flee_blind_panic(
	short panic_type)
{
	return panic_type >= _actor_panic_grenade_attached_to_us && panic_type <= _actor_panic_burning_to_death;
}

boolean action_flee_setup(
	long actor_index,
	short panic_type,
	long panic_prop_index,
	boolean force_state_entry,
	boolean flee_from_last_visible_location,
	boolean allow_occluded_points,
	struct flee_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (!actor->input.vehicle_passenger)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\action_flee.c", 44, state_data);
		memset(state_data, 0, sizeof(*state_data));

		state_data->flee_firing_position_index = NONE;
		state_data->panic_type = panic_type;
		state_data->forced_flee_ticks = force_state_entry ? FLEE_FORCED_TICKS : 0;
		state_data->flee_from_last_visible_location = flee_from_last_visible_location;
		state_data->allow_occluded_points = allow_occluded_points;
		state_data->flee_prop_index = panic_prop_index;

		if (state_data->flee_prop_index != NONE)
		{
			actor_situation_try_new_target(actor_index, state_data->flee_prop_index);
		}

		if (action_flee_blind_panic(panic_type) && real_random() < 0.4f)
		{
			state_data->flee_stationary_ticks = FLEE_STATIONARY_TICKS;
			result = TRUE;
		}
		else if (!actor->meta.swarm)
		{
			action_flee_find_flee_position(actor_index, FALSE, state_data);
			if (state_data->flee_firing_position_index == NONE)
			{
				state_data->unable_to_flee = FALSE;
			}
			else
			{
				result = TRUE;
			}
		}
	}

	return result;
}

void action_flee_begin(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct flee_state_data *state_data = &actor->state.action_data.flee;

	state_data->flee_timer = 0;
	if (state_data->panic_type > _actor_panic_none)
	{
		actor->state.searching = FALSE;
	}

	if (state_data->flee_stationary_ticks == 0 &&
		actor->meta.unit_index != NONE &&
		action_flee_blind_panic(state_data->panic_type))
	{
		unit_start_running_blindly(actor->meta.unit_index);
	}

	return;
}

void action_flee_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct flee_state_data *state_data = &actor->state.action_data.flee;

	state_data->flee_timer++;
	if (state_data->forced_flee_ticks > 0)
	{
		state_data->forced_flee_ticks--;
	}

	if (state_data->flee_stationary_ticks > 0)
	{
		state_data->flee_stationary_ticks--;
		if (state_data->flee_stationary_ticks == 0 &&
			actor->meta.unit_index != NONE &&
			action_flee_blind_panic(state_data->panic_type))
		{
			unit_start_running_blindly(actor->meta.unit_index);
		}
	}

	if (state_data->panic_type > _actor_panic_none)
	{
		actor->emotions.flee_with_friends_disable_time = game_time_get() + 25 * TICKS_PER_SECOND;
	}

	return;
}

void action_flee_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct flee_state_data *state_data = &actor->state.action_data.flee;

	if (state_data->panic_type > _actor_panic_none)
	{
		actor->orders.look.primary_priority = _primary_priority_locked_facing;
		actor->orders.look.primary_direction.type = _direction_specification_movement;
		actor->orders.combat.abort_burst = TRUE;
	}
	else
	{
		boolean target_visible = FALSE;

		if (actor->target.target_prop_index != NONE)
		{
			struct prop_datum *target_prop = prop_get(actor->target.target_prop_index);

			if (target_prop->visibility > _actor_perception_none)
			{
				target_visible = TRUE;
			}
		}

		if (target_visible)
		{
			actor->orders.look.primary_priority = _primary_priority_locked_aiming;
			actor->orders.look.primary_direction.type = _direction_specification_target;
			actor->orders.combat.shoot_at_target = TRUE;
		}
		else if (state_data->flee_prop_index != NONE)
		{
			actor->orders.look.primary_priority = _primary_priority_facing;
			actor->orders.look.primary_direction.type = _direction_specification_prop;
			actor->orders.look.primary_direction.prop_index = state_data->flee_prop_index;
		}
		else
		{
			actor->orders.look.primary_priority = _primary_priority_none;
		}
	}

	actor->orders.look.idle_look_type = _idle_look_combat;
	actor->orders.move.panicked = state_data->panic_type > _actor_panic_none;
	actor->orders.move.flaming = action_flee_blind_panic(state_data->panic_type);
	actor->orders.move.stationary_crouch = TRUE;
	actor->orders.move.moving_crouch = FALSE;
	actor->orders.move.dive_into_cover = TRUE;
	actor->orders.move.emerge_from_cover = FALSE;

	if (state_data->flee_firing_position_index == NONE)
	{
		actor_move_halt(actor_index);
	}
	else if (actor->meta.timeslice)
	{
		if (actor_move_to_firing_position(actor_index, state_data->flee_firing_position_index, NULL))
		{
			actor->firing_positions.current_position_index = state_data->flee_firing_position_index;
			actor->firing_positions.current_position_found_outside_range = state_data->flee_firing_position_found_randomly;
		}
		else
		{
			if (actor->firing_positions.current_position_index != NONE)
			{
				actor_discard_firing_position(actor_index, actor->firing_positions.current_position_index, FALSE);
				actor_move_halt(actor_index);
				actor->firing_positions.current_position_index = NONE;
			}

			state_data->flee_firing_position_index = NONE;
			state_data->find_new_flee_position = TRUE;
		}
	}

	return;
}

static boolean action_flee_at_flee_position(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct flee_state_data *state_data = &actor->state.action_data.flee;
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_flee.c", 470, !actor->meta.swarm);

	if (actor->meta.encounter_index != NONE && actor->firing_positions.current_position_index != NONE)
	{
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
			struct encounter_definition);
		struct firing_position_definition *firing_position = TAG_BLOCK_GET_ELEMENT(
			&encounter_definition->firing_positions,
			actor->firing_positions.current_position_index,
			struct firing_position_definition);

		if (actor_path_at_destination(actor_index) &&
			actor->control.path.destination_orders.destination_type == 3 &&
			actor->control.path.destination_orders.firing_position_index == actor->firing_positions.current_position_index)
		{
			result = TRUE;
		}
		else
		{
			real tolerance = actor_destination_tolerance(actor_index);

			if (distance_squared3d(&actor->input.position.body_position, &firing_position->position) < tolerance * tolerance)
			{
				boolean exposed = FALSE;

				if (state_data->flee_prop_index != NONE)
				{
					struct prop_datum *prop = prop_get(state_data->flee_prop_index);

					exposed = prop->line_of_sight == _ai_line_of_sight_clear || prop->line_of_sight == _ai_line_of_sight_occluded;
				}

				result = !exposed;
			}
		}
	}

	return result;
}

boolean action_flee_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct flee_state_data *state_data = &actor->state.action_data.flee;

	if (!actor->meta.swarm)
	{
		if (action_flee_blind_panic(state_data->panic_type))
		{
			state_data->forced_flee_ticks = FLEE_FORCED_TICKS;
		}

		if (state_data->flee_stationary_ticks > 0)
		{
			state_data->flee_firing_position_index = NONE;
		}
		else if (state_data->flee_firing_position_index == NONE)
		{
			state_data->find_new_flee_position = TRUE;
		}
		else if (actor->firing_positions.current_position_index == NONE)
		{
			state_data->flee_firing_position_index = NONE;
			state_data->find_new_flee_position = TRUE;
		}
		else if (action_flee_at_flee_position(actor_index))
		{
			match_assert("c:\\halo\\SOURCE\\ai\\action_flee.c", 152, actor->firing_positions.current_position_index != NONE);

			if (state_data->forced_flee_ticks)
			{
				state_data->find_new_flee_position = TRUE;
			}
			else
			{
				state_data->flee_firing_position_index = actor->firing_positions.current_position_index;
				state_data->flee_firing_position_found_randomly = actor->firing_positions.current_position_found_outside_range;
				state_data->done_fleeing = TRUE;
				state_data->find_new_flee_position = FALSE;

				if (state_data->flee_prop_index != NONE)
				{
					struct prop_datum *prop = prop_get(state_data->flee_prop_index);

					prop->visibility = _actor_perception_none;
					prop->perception = MAX(prop->audibility, prop->ineffability);
					prop->currently_damaging_me = FALSE;
					prop->line_of_sight = _ai_line_of_sight_from_cover;
					actor_situation_update_target_status(actor_index);
					actor_situation_combat_status_update(actor_index);
				}
			}
		}

		switch (state_data->panic_type)
		{
		case _actor_panic_grenade_attached_to_us:
		case _actor_panic_delayed_projectile_attached_to_us:
			if (actor->input.delayed_attached_projectile_index == NONE)
			{
				state_data->done_fleeing = TRUE;
			}
			break;
		case _actor_panic_melee_attached_to_us:
			if (!actor->input.melee_attacker_attached)
			{
				state_data->done_fleeing = TRUE;
			}
			break;
		case _actor_panic_burning_to_death:
			if (!actor->input.burning_to_death)
			{
				state_data->done_fleeing = TRUE;
			}
			break;
		}

		if (actor->meta.timeslice && !state_data->done_fleeing)
		{
			if (state_data->flee_firing_position_index != NONE &&
				!state_data->forced_flee_ticks &&
				action_flee_current_position_exposed(actor_index, state_data))
			{
				state_data->flee_firing_position_index = NONE;
				state_data->find_new_flee_position = TRUE;
			}

			if (actor->input.vehicle_passenger)
			{
				state_data->find_new_flee_position = FALSE;
				state_data->unable_to_flee = TRUE;
				actor->emotions.last_flee_failed_time = game_time_get();
			}
			else if (state_data->find_new_flee_position)
			{
				action_flee_find_flee_position(actor_index, TRUE, state_data);
				if (state_data->flee_firing_position_index == NONE)
				{
					state_data->unable_to_flee = TRUE;
					actor->emotions.last_flee_failed_time = game_time_get();
				}
			}
		}
	}

	if (action_flee_blind_panic(state_data->panic_type) &&
		actor->meta.unit_index != NONE &&
		!unit_is_speaking(actor->meta.unit_index))
	{
		state_data->communicated_flee = FALSE;
	}

	if (state_data->panic_type > _actor_panic_none &&
		state_data->flee_firing_position_index != NONE &&
		!state_data->unable_to_flee &&
		actor->meta.unit_index != NONE)
	{
		long current_time = game_time_get();

		if (!state_data->communicated_flee || state_data->last_communication_time + 2 * TICKS_PER_SECOND >= current_time)
		{
			if (state_data->panic_type == _actor_panic_burning_to_death || state_data->panic_type == _actor_panic_melee_attached_to_us)
			{
				unit_scream(actor->meta.unit_index, _unit_scream_burning_to_death);
			}
			else if (state_data->panic_type == _actor_panic_grenade_attached_to_us || state_data->panic_type == _actor_panic_delayed_projectile_attached_to_us)
			{
				unit_scream(actor->meta.unit_index, _unit_scream_grenade_attached_to_us);
			}
			else
			{
				long flee_unit_index = NONE;

				if (state_data->flee_prop_index != NONE)
				{
					flee_unit_index = prop_get(state_data->flee_prop_index)->unit_index;
				}

				if (!state_data->communicated_flee)
				{
					ai_communication_event(
						(state_data->panic_type == _actor_panic_friend_leader_type_killed) ? _ai_communication_flee_leader_died : _ai_communication_flee,
						actor->meta.unit_index,
						flee_unit_index,
						NONE,
						NONE,
						_ai_information_flee,
						NULL);
					state_data->communicated_flee = TRUE;
				}
				else
				{
					ai_communication_event(_ai_communication_flee_idle, actor->meta.unit_index, flee_unit_index, NONE, NONE, NONE, NULL);
				}
			}

			state_data->last_communication_time = current_time;
		}
	}

	if (!actor->meta.swarm)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\action_flee.c", 302, (!actor->meta.timeslice && state_data->find_new_flee_position) || (state_data->flee_stationary_ticks > 0) || (state_data->flee_firing_position_index != NONE) || state_data->unable_to_flee || state_data->done_fleeing);
	}

	return state_data->unable_to_flee || state_data->done_fleeing;
}

/* ---------- private code */
