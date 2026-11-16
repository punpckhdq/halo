/*
ACTION_GUARD.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "props.h"
#include "actor_types.h"
#include "units.h"
#include "ai_communication.h"
#include "game.h"

/* ---------- constants */

/* ---------- macros */

#define GUARD_POSTCOMBAT_TICKS (4*TICKS_PER_SECOND) /* fake name */

/* ---------- structures */

/* ---------- prototypes */

static boolean action_guard_speak_post_combat(long actor_index);

/* ---------- globals */

/* ---------- public code */

boolean action_guard_setup_from_combat_transition(
	long actor_index,
	struct guard_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 32, state_data);
	memset(state_data, 0, sizeof(*state_data));

	state_data->wait_ticks = (short)actor->stimuli.combat_transition_guard_timer;

	if (actor->input.vehicle_passenger || actor->meta.swarm)
	{
		state_data->guard_location_type = _actor_guard_location_current;
	}
	else
	{
		boolean guard_at_point = FALSE;

		match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 45, actor->stimuli.combat_transition);

		if (actor->stimuli.combat_transition_guard_at_point &&
			actor_nearby_firing_positions(actor_index, &actor->stimuli.combat_transition_guard_point, actor->stimuli.combat_transition_guard_point_surface_index, 1))
		{
			state_data->guard_location_type = _actor_guard_location_point;
			state_data->guard_point.position = actor->stimuli.combat_transition_guard_point;
			state_data->guard_point.surface_index = actor->stimuli.combat_transition_guard_point_surface_index;
			state_data->guard_point.radius = actor->stimuli.combat_transition_guard_point_distance;
			guard_at_point = TRUE;
		}

		if (!guard_at_point)
		{
			if (state_data->wait_ticks > 0)
			{
				state_data->guard_location_type = _actor_guard_location_current;
				state_data->has_guard_direction = actor->stimuli.combat_transition_has_vector;
				state_data->aim_in_guard_direction = FALSE;
				if (state_data->has_guard_direction)
				{
					state_data->guard_direction = actor->stimuli.combat_transition_vector;
					if (normalize3d(&state_data->guard_direction) == 0.0f)
					{
						state_data->has_guard_direction = FALSE;
					}
				}
			}
			else
			{
				state_data->guard_location_type = _actor_guard_location_none;
				state_data->find_new_guard_position = TRUE;
			}
		}
	}

	if (actor->stimuli.combat_transition == 2)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 90, actor->stimuli.combat_transition_prop_index != NONE);
		state_data->shout_about_dead_friend = TRUE;
		state_data->shout_dead_friend_prop_index = actor->stimuli.combat_transition_prop_index;
	}

	state_data->guard_look_prop_index = actor->stimuli.combat_transition_prop_index;
	if (state_data->guard_look_prop_index != NONE)
	{
		state_data->look_ticks = (short)actor->stimuli.combat_transition_prop_look_timer;
		state_data->guard_look_until_reached_point = actor->stimuli.combat_transition_prop_look_while_moving;
	}

	return TRUE;
}

boolean action_guard_setup_current_position(
	long actor_index,
	struct guard_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 114, state_data);
	memset(state_data, 0, sizeof(*state_data));

	state_data->guard_location_type = _actor_guard_location_current;
	state_data->has_guard_direction = TRUE;
	state_data->guard_direction = actor->input.facing_vector;
	state_data->guard_look_prop_index = NONE;

	return TRUE;
}

boolean action_guard_setup_find_position(
	long actor_index,
	long delay_timer,
	struct guard_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 134, state_data);
	memset(state_data, 0, sizeof(*state_data));

	if (actor->input.vehicle_passenger || actor->meta.swarm)
	{
		state_data->guard_location_type = _actor_guard_location_current;
	}
	else
	{
		state_data->wait_ticks = (short)delay_timer;
		if (state_data->wait_ticks == 0)
		{
			state_data->find_new_guard_position = TRUE;
			state_data->guard_location_type = _actor_guard_location_none;
		}
		else
		{
			state_data->guard_location_type = _actor_guard_location_current;
			state_data->has_guard_direction = TRUE;
			state_data->guard_direction = actor->input.facing_vector;
		}
	}
	state_data->guard_look_prop_index = NONE;

	return TRUE;
}

boolean action_guard_setup_postcombat(
	long actor_index,
	struct guard_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = TRUE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 172, state_data);
	memset(state_data, 0, sizeof(*state_data));

	state_data->wait_ticks = GUARD_POSTCOMBAT_TICKS;
	state_data->guard_location_type = _actor_guard_location_current;
	state_data->post_combat = TRUE;
	state_data->guard_look_prop_index = NONE;

	if (actor->input.vehicle_driver_type == _actor_vehicle_driver_directional_flying)
	{
		result = FALSE;
	}
	else if (!actor->input.vehicle_passenger && !actor->meta.swarm && actor->external_orders.postcombat_prop_index != NONE)
	{
		struct prop_datum *prop = prop_get(actor->external_orders.postcombat_prop_index);
		boolean run_to_prop = TRUE;

		state_data->guard_look_prop_index = actor->external_orders.postcombat_prop_index;
		state_data->look_ticks = GUARD_POSTCOMBAT_TICKS;
		state_data->guard_look_until_reached_point = TRUE;

		switch (actor->external_orders.postcombat_type)
		{
		case _actor_postcombat_run_to:
			state_data->guard_point.radius = 2.0f;
			break;
		case _actor_postcombat_check_enemy:
		case _actor_postcombat_check_friend:
			state_data->guard_point.radius = 1.0f;
			break;
		case _actor_postcombat_shoot_corpse:
			state_data->guard_point.radius = 1.5f;
			break;
		default:
			run_to_prop = FALSE;
			break;
		}

		if (run_to_prop)
		{
			actor_perception_find_prop_pathfinding_location(actor_index, actor->external_orders.postcombat_prop_index);
			state_data->guard_location_type = _actor_guard_location_point;
			state_data->guard_point.position = prop->pathfinding_point;
			state_data->guard_point.surface_index = prop->pathfinding_surface_index;
		}
	}

	return result;
}

boolean action_guard_setup_from_fleeing(
	long actor_index,
	struct flee_state_data *flee_state_data,
	struct guard_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 237, state_data);
	memset(state_data, 0, sizeof(*state_data));

	if (actor->input.vehicle_passenger || actor->meta.swarm)
	{
		state_data->guard_location_type = _actor_guard_location_current;
	}
	else
	{
		state_data->wait_ticks = 0;
		state_data->cower = TRUE;
		state_data->cower_ticks = 0;
		state_data->cower_panicked = flee_state_data->panic_type > _actor_panic_none;
		state_data->cower_from_retreat = actor->emotions.unopposable_retreat_timer > 0 && !state_data->cower_panicked;

		if (!state_data->cower_from_retreat)
		{
			real cower_time;

			if (state_data->cower_panicked)
			{
				cower_time = real_random_range(actor_definition->panic.cower_time_lower_bound, actor_definition->panic.cower_time_upper_bound);
			}
			else
			{
				cower_time = real_random_range(actor_definition->defensive.hide_time_lower_bound, actor_definition->defensive.hide_time_upper_bound);
			}
			state_data->cower_ticks = (short)(cower_time * TICKS_PER_SECOND);
		}

		if (flee_state_data->flee_firing_position_index == NONE)
		{
			state_data->find_new_guard_position = TRUE;
			state_data->guard_location_type = _actor_guard_location_none;
		}
		else
		{
			state_data->find_new_guard_position = FALSE;
			state_data->guard_location_type = _actor_guard_location_firing_position;
			state_data->guard_firing_position_index = flee_state_data->flee_firing_position_index;

			if (flee_state_data->has_approach_point)
			{
				state_data->has_guard_direction = TRUE;
				state_data->aim_in_guard_direction = TRUE;
				vector_from_points3d(&actor->input.position.body_position, &flee_state_data->approach_point, &state_data->guard_direction);
				if (normalize3d(&state_data->guard_direction) == 0.0f)
				{
					state_data->has_guard_direction = FALSE;
				}
			}
		}
	}
	state_data->guard_look_prop_index = NONE;

	return TRUE;
}

void action_guard_begin(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	actor_perception_forget_recent_damage(actor_index);
	actor->state.searching = FALSE;
	if (state_data->cower_from_retreat)
	{
		actor_perception_retreat_successful(actor_index);
	}

	return;
}

void action_guard_end(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	if (state_data->post_combat)
	{
		actor->external_orders.postcombat_type = _actor_postcombat_none;
		actor->external_orders.postcombat_prop_index = NONE;
	}

	return;
}

static boolean action_guard_speak_post_combat(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 344, state_data->post_combat);

	if (!state_data->post_combat_vocalized)
	{
		short communication_type = NONE;

		switch (actor->external_orders.postcombat_type)
		{
		case _actor_postcombat_speak_alone:
			communication_type = _ai_communication_postcombat_alone;
			break;
		case _actor_postcombat_speak_unscathed:
			communication_type = _ai_communication_postcombat_unscathed;
			break;
		case _actor_postcombat_speak_wounded:
			communication_type = _ai_communication_postcombat_wounded;
			break;
		case _actor_postcombat_speak_massacre:
			communication_type = _ai_communication_postcombat_massacre;
			break;
		case _actor_postcombat_speak_triumph:
			communication_type = _ai_communication_postcombat_triumph;
			break;
		case _actor_postcombat_check_enemy:
			communication_type = _ai_communication_postcombat_check_enemy;
			break;
		case _actor_postcombat_check_friend:
			communication_type = _ai_communication_postcombat_check_friend;
			break;
		case _actor_postcombat_shoot_corpse:
			communication_type = _ai_communication_postcombat_shoot_corpse;
			break;
		case _actor_postcombat_celebrate:
			communication_type = _ai_communication_postcombat_celebrate;
			break;
		}

		if (communication_type != NONE)
		{
			long cause_unit_index = NONE;
			short hostility = NONE;

			if (state_data->guard_look_prop_index != NONE)
			{
				struct prop_datum *prop = prop_get(state_data->guard_look_prop_index);

				cause_unit_index = prop->unit_index;
				if (prop->enemy)
				{
					hostility = game_team_is_ally(actor->meta.team_index, prop->team_index) ? _comm_hostility_traitor : _comm_hostility_enemy;
				}
				else
				{
					hostility = _comm_hostility_friend;
				}
			}

			ai_communication_event(communication_type, actor->meta.unit_index, cause_unit_index, hostility, NONE, NONE, NULL);
			result = TRUE;
		}

		state_data->post_combat_vocalized = TRUE;
	}

	return result;
}

boolean action_guard_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	if (actor->meta.swarm)
	{
		state_data->guard_location_type = _actor_guard_location_current;
	}
	else if (actor->input.vehicle_passenger)
	{
		state_data->guard_location_type = _actor_guard_location_current;
		state_data->find_new_guard_position = TRUE;
	}
	else
	{
		if (state_data->guard_location_type == _actor_guard_location_firing_position && actor->firing_positions.current_position_index == NONE)
		{
			state_data->guard_location_type = _actor_guard_location_none;
			state_data->find_new_guard_position = TRUE;
		}

		if (actor->meta.timeslice && state_data->find_new_guard_position)
		{
			struct firing_position_evaluation_context evaluation_context;
			struct firing_position new_firing_position;
			struct path_state area_path_state;
			real guard_time;
			boolean area_path_valid;
			long previous_owner;
			short firing_position_index;

			if (state_data->guard_location_type == _actor_guard_location_firing_position && actor->firing_positions.current_position_index != NONE)
			{
				actor_discard_firing_position(actor_index, actor->firing_positions.current_position_index, FALSE);
			}

			memset(&evaluation_context, 0, sizeof(evaluation_context));
			evaluation_context.evaluation_mode = _firing_point_evaluation_mode_guard;
			evaluation_context.allowed_position_mask = actor_get_firing_position_group(actor_index, _firing_point_evaluation_mode_guard, 0);
			evaluation_context.allow_outside_range = TRUE;

			firing_position_index = actor_select_firing_position(actor_index, &evaluation_context, &new_firing_position, &previous_owner, &area_path_state, &area_path_valid);
			firing_position_index = actor_change_firing_position(actor_index, firing_position_index, &new_firing_position, previous_owner, &area_path_state, area_path_valid);

			state_data->find_new_guard_position = FALSE;
			state_data->has_guard_direction = FALSE;
			if (firing_position_index == NONE)
			{
				state_data->guard_location_type = _actor_guard_location_current;
			}
			else
			{
				state_data->guard_location_type = _actor_guard_location_firing_position;
				state_data->guard_firing_position_index = firing_position_index;
			}

			guard_time = real_random_range(actor_definition->firing_position.guard_position_time_lower_bound, actor_definition->firing_position.guard_position_time_upper_bound);
			state_data->wait_ticks = (short)(guard_time * TICKS_PER_SECOND);
		}
	}

	return FALSE;
}

void action_guard_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	if (!actor->meta.dormant && actor->control.path.at_destination && state_data->wait_ticks > 0)
	{
		if (--state_data->wait_ticks == 0 && !actor->input.vehicle_passenger && !actor->meta.swarm)
		{
			if (state_data->post_combat)
			{
				action_guard_speak_post_combat(actor_index);
				actor->external_orders.postcombat_type = _actor_postcombat_none;
				actor->external_orders.postcombat_prop_index = NONE;
				state_data->post_combat = FALSE;
				state_data->post_combat_shooting = FALSE;
				state_data->guard_look_prop_index = NONE;
			}
			state_data->find_new_guard_position = TRUE;
		}
	}

	if (state_data->look_ticks > 0 && (!state_data->guard_look_until_reached_point || actor->control.path.at_destination))
	{
		state_data->look_ticks--;
		if (state_data->look_ticks == 0)
		{
			state_data->guard_look_prop_index = NONE;
		}
	}

	if (state_data->path_begun && actor->control.path.at_destination)
	{
		boolean cower_finished = FALSE;

		if (state_data->cower_from_retreat)
		{
			state_data->cower_from_retreat = actor->emotions.unopposable_retreat_timer > 0;
			cower_finished = !state_data->cower_from_retreat;
		}
		else if (state_data->cower_ticks > 0)
		{
			state_data->cower_ticks--;
			cower_finished = state_data->cower_ticks == 0;
		}

		if (cower_finished)
		{
			actor_set_dormant(actor_index, FALSE);
			state_data->cower = FALSE;
			state_data->cower_panicked = FALSE;
			state_data->cower_from_retreat = FALSE;
			state_data->cower_ticks = 0;

			if (actor->state.combat_status >= _actor_combat_status_investigate && actor->meta.unit_index != NONE)
			{
				ai_communication_event(_ai_communication_hiding_finished, actor->meta.unit_index, actor_target_unit_index(actor_index), NONE, NONE, NONE, NULL);
			}

#if CONFIG_VERSION >= 2342
			if (state_data->guard_location_type == _actor_guard_location_firing_position)
			{
				actor_discard_firing_position(actor_index, state_data->guard_firing_position_index, FALSE);
				state_data->guard_firing_position_index = NONE;
			}
			actor->firing_positions.current_position_index = NONE;
#else
			actor_discard_firing_position(actor_index, state_data->guard_firing_position_index, FALSE);
			actor->firing_positions.current_position_index = NONE;
			state_data->guard_firing_position_index = NONE;
#endif

			if (actor->input.vehicle_passenger)
			{
				state_data->guard_location_type = _actor_guard_location_current;
			}
			else
			{
				state_data->guard_location_type = _actor_guard_location_none;
				state_data->find_new_guard_position = TRUE;
			}
		}
	}

	return;
}

void action_guard_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	if (TEST_FLAG(actor_definition->flags, _actor_definition_crouch_when_noncombat_bit) && actor->state.combat_status == _actor_combat_status_none)
	{
		actor->orders.move.stationary_crouch = TRUE;
		actor->orders.move.moving_crouch = TRUE;
	}
	else
	{
		actor->orders.move.moving_crouch = FALSE;
		if (state_data->cower)
		{
			if (state_data->cower_from_retreat)
			{
				actor->orders.move.stationary_crouch = TEST_FLAG(actor_definition->flags, _actor_definition_crouch_when_hiding_from_unopposable_enemy_bit);
			}
			else
			{
				actor->orders.move.stationary_crouch = TRUE;
			}
		}
		else
		{
			actor->orders.move.stationary_crouch = TEST_FLAG(actor_definition->flags, _actor_definition_crouch_when_guarding_bit) && actor->state.combat_status > _actor_combat_status_none;
		}
	}

	actor->orders.move.panicked = FALSE;
	actor->orders.move.dive_into_cover = FALSE;
	actor->orders.move.emerge_from_cover = FALSE;

	if (actor->meta.timeslice && !actor->meta.swarm)
	{
		boolean at_position = FALSE;

		switch (state_data->guard_location_type)
		{
		case _actor_guard_location_none:
		case _actor_guard_location_current:
			actor_move_halt(actor_index);
			at_position = TRUE;
			break;
		case _actor_guard_location_point:
		{
			real distance_squared = distance_squared3d(&actor->input.position.body_position, &state_data->guard_point.position);

			if (distance_squared < state_data->guard_point.radius * state_data->guard_point.radius)
			{
				actor_move_halt(actor_index);
			}
			else
			{
				actor_move_to_point(actor_index, &state_data->guard_point.position, state_data->guard_point.surface_index, NONE);
			}
			at_position = distance_squared < 9.0f;
			break;
		}
		case _actor_guard_location_firing_position:
			if (state_data->guard_firing_position_index != NONE)
			{
				actor->firing_positions.current_position_index = state_data->guard_firing_position_index;
				actor->firing_positions.current_position_found_outside_range = FALSE;
				if (!actor_move_to_firing_position(actor_index, actor->firing_positions.current_position_index, NULL))
				{
					actor_discard_firing_position(actor_index, state_data->guard_firing_position_index, FALSE);
					actor->firing_positions.current_position_index = NONE;
				}
			}
			at_position = !actor->control.path.path.valid ||
				distance_squared3d(&actor->control.path.path.endpoint.point, &actor->input.position.body_position) < 9.0f;
			break;
		default:
			match_halt("c:\\halo\\SOURCE\\ai\\action_guard.c", MATCH_LINE_NTSC(681, 685));
			break;
		}

		state_data->path_begun = TRUE;

		if (at_position)
		{
			if (state_data->shout_about_dead_friend)
			{
				struct prop_datum *prop = prop_get(state_data->shout_dead_friend_prop_index);

				state_data->shout_about_dead_friend = FALSE;
				state_data->shout_dead_friend_prop_index = NONE;
				actor_stimulus_suspicion(actor_index, _actor_combat_status_investigate, 20*TICKS_PER_SECOND);
				ai_communication_event(_ai_communication_found_dead_friend, actor->meta.unit_index, prop->unit_index, NONE, NONE, _ai_information_combat_stimulus, NULL);
			}

			if (state_data->post_combat)
			{
				action_guard_speak_post_combat(actor_index);
				if (actor->external_orders.postcombat_type == _actor_postcombat_shoot_corpse && state_data->guard_location_type == _actor_guard_location_point)
				{
					state_data->post_combat_shooting = TRUE;
				}
			}
		}
	}

	if (state_data->post_combat_shooting)
	{
		actor->orders.look.primary_priority = _primary_priority_locked_aiming;
		actor->orders.look.primary_direction.type = _direction_specification_target;
		actor->orders.combat.shoot_at_target = TRUE;
		actor->orders.combat.use_manual_target_point = TRUE;
		point_from_line3d(&state_data->guard_point.position, global_up3d, 0.05f, &actor->orders.combat.target_point);
	}
	else if (state_data->guard_look_prop_index != NONE)
	{
		actor->orders.look.primary_priority = _primary_priority_aiming;
		actor->orders.look.primary_direction.type = _direction_specification_prop;
		actor->orders.look.primary_direction.prop_index = state_data->guard_look_prop_index;
	}
	else if (state_data->has_guard_direction)
	{
		actor->orders.look.primary_priority = state_data->aim_in_guard_direction ? _primary_priority_aiming : _primary_priority_facing;
		actor->orders.look.primary_direction.type = _direction_specification_vector;
		actor->orders.look.primary_direction.vector = state_data->guard_direction;
	}
	else if (actor->state.combat_status > _actor_combat_status_none && actor->target.target_prop_index != NONE)
	{
		actor->orders.look.primary_priority = _primary_priority_facing;
		actor->orders.look.primary_direction.type = _direction_specification_prop;
		actor->orders.look.primary_direction.prop_index = actor->target.target_prop_index;
	}
	else
	{
		actor->orders.look.primary_priority = _primary_priority_none;
	}

	actor->orders.look.idle_look_type = actor->state.combat_status >= _actor_combat_status_certain ? _idle_look_combat : _idle_look_guarding;

	return;
}

void action_guard_flush_position_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;
	boolean flush;

	if (state_data->cower && state_data->guard_location_type == _actor_guard_location_firing_position)
	{
		state_data->cower = FALSE;
		state_data->cower_ticks = 0;
		state_data->cower_from_retreat = FALSE;
	}

	flush = FALSE;

	if (state_data->guard_location_type == _actor_guard_location_firing_position)
	{
		flush = TRUE;
	}
	else if (state_data->guard_location_type == _actor_guard_location_current && !actor->input.vehicle_passenger)
	{
		flush = TRUE;
	}

	if (flush)
	{
		state_data->guard_location_type = _actor_guard_location_none;
		state_data->guard_firing_position_index = NONE;
		state_data->find_new_guard_position = TRUE;
	}

	return;
}

void action_guard_flush_structure_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	if (state_data->guard_location_type == _actor_guard_location_point)
	{
		state_data->guard_point.surface_index = NONE;
	}

	return;
}

void action_guard_modify_color(
	long actor_index,
	real_argb_color *color)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	if (!state_data->cower)
	{
		*color = *global_real_argb_salmon;
	}
	else if (state_data->cower_from_retreat)
	{
		*color = *global_real_argb_violet;
	}
	else if (state_data->cower_panicked)
	{
		*color = *global_real_argb_pink;
	}
	else
	{
		*color = *global_real_argb_purple;
	}

	return;
}

void action_guard_replace_prop(
	long actor_index,
	long invalid_prop_index,
	long replacement_prop_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	if (state_data->guard_look_prop_index == invalid_prop_index)
	{
		state_data->guard_look_prop_index = replacement_prop_index;
	}

	if (state_data->shout_dead_friend_prop_index == invalid_prop_index)
	{
		state_data->shout_dead_friend_prop_index = replacement_prop_index;
		if (replacement_prop_index == NONE)
		{
			state_data->shout_about_dead_friend = FALSE;
		}
	}

	return;
}

/* ---------- private code */
