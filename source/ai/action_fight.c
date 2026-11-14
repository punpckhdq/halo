/*
ACTION_FIGHT.C








*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "vehicles.h"
#include "vehicle_definitions.h"
#include "game.h"

/* ---------- public code */

boolean action_fight_setup(
	long actor_index,
	struct fight_state_data *state_data)
{
	match_assert("c:\\halo\\SOURCE\\ai\\action_fight.c", 30, state_data);

	memset(state_data, 0, sizeof(struct fight_state_data));

	return TRUE;
}

void action_fight_begin(
	long actor_index)
{
	return;
}

void action_fight_end(
	long actor_index)
{
	return;
}

boolean action_fight_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct fight_state_data *state_data = &actor->state.action_data.fight;

	match_assert("c:\\halo\\SOURCE\\ai\\action_fight.c", 55, !actor->meta.swarm);

	if (actor->meta.timeslice)
	{
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
		struct actor_variant_definition *variant_definition = actor_combat_get_firing_variant_definition(actor_index);
		boolean find_new_position = TRUE;

		if (actor->input.vehicle_passenger)
		{
			find_new_position = FALSE;
		}
		else if (actor->emotions.defensive_crouch && TEST_FLAG(actor_definition->flags, _actor_definition_crouch_try_not_to_move_bit))
		{
			actor_find_pathfinding_location(actor_index);

			if (actor_nearby_firing_positions(actor_index, &actor->input.pathfinding_point, actor->input.pathfinding_surface_index, 0))
			{
				boolean at_firing_position = FALSE;

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
					real tolerance = actor_destination_tolerance(actor_index);

					if (distance_squared3d(&actor->input.position.body_position, &firing_position->position) < tolerance * tolerance)
					{
						at_firing_position = TRUE;
					}
				}

				if (actor->control.moving && !actor->situation.cumulative_threats[_actor_threat_shooting_near_me])
				{
					at_firing_position = TRUE;

					if (actor->target.target_prop_index != NONE)
					{
						struct prop_datum *prop = prop_get(actor->target.target_prop_index);

						if (prop->distance < variant_definition->ranged_combat.combat_range_upper_bound)
						{
							at_firing_position = FALSE;
						}
					}
				}

				if (!at_firing_position)
				{
					actor->firing_positions.current_position_index = NONE;
					actor_move_halt(actor_index);
				}

				find_new_position = FALSE;
			}
		}

		if (find_new_position)
		{
			struct firing_position_evaluation_context evaluation_context;
			struct firing_position new_firing_position;
			struct path_state path_state;
			boolean area_path_valid;
			long previous_owner;
			short old_firing_position_index = actor->firing_positions.current_position_index;
			short firing_position_index;

			memset(&evaluation_context, 0, sizeof(evaluation_context));
			evaluation_context.evaluation_mode = _firing_point_evaluation_mode_fight;

			firing_position_index = actor_active_select_firing_position(actor_index, &evaluation_context, &new_firing_position, &previous_owner, &path_state, &area_path_valid);
			firing_position_index = actor_change_firing_position(actor_index, firing_position_index, &new_firing_position, previous_owner, &path_state, area_path_valid);

			if (firing_position_index == NONE)
			{
				state_data->firing_position_timer = 0;
			}
			else if (firing_position_index != old_firing_position_index)
			{
				real time = real_random_range(actor_definition->firing_position.combat_position_time_lower_bound, actor_definition->firing_position.combat_position_time_upper_bound);

				if (actor->input.vehicle_driver_type > _actor_vehicle_driver_none)
				{
					struct vehicle_datum *vehicle = vehicle_get(actor->input.vehicle_index);
					struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

					if (vehicle_definition->vehicle.ai_movement_max_time > 0.f)
					{
						time = MIN(time, vehicle_definition->vehicle.ai_movement_max_time);
					}
				}

				state_data->firing_position_timer = (short)(time * TICKS_PER_SECOND);
			}
		}

		if (actor->target.target_type >= _actor_target_acknowledged_enemy)
		{
			struct prop_datum *prop = prop_get(actor->target.target_prop_index);
			boolean unreachable = TRUE;

			if (actor_has_ranged_weapon(actor_index))
			{
				if (prop->distance < actor->control.weapon_maximum_range)
				{
					unreachable = FALSE;
				}
				else if (actor->meta.encounter_index != NONE && actor->firing_positions.current_position_index != NONE)
				{
					struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
						&global_scenario_get()->ai_encounters,
						DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
						struct encounter_definition);
					struct firing_position_definition *firing_position = TAG_BLOCK_GET_ELEMENT(
						&encounter_definition->firing_positions,
						actor->firing_positions.current_position_index,
						struct firing_position_definition);
					real tolerance = actor_destination_tolerance(actor_index);

					if (distance_squared3d(&actor->input.position.body_position, &firing_position->position) > tolerance * tolerance)
					{
						if (distance_squared3d(&firing_position->position, &prop->body_position) < actor->control.weapon_maximum_range * actor->control.weapon_maximum_range)
						{
							unreachable = FALSE;
						}
					}
				}
			}

			actor_perception_unreachable(actor_index, actor->target.target_prop_index, unreachable);
		}
	}

	return FALSE;
}

void action_fight_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct fight_state_data *state_data = &actor->state.action_data.fight;

	if (state_data->firing_position_timer > 0 && actor->control.path.at_destination)
	{
		state_data->firing_position_timer--;

		if (state_data->firing_position_timer == 0)
		{
			if (actor->firing_positions.current_position_index != NONE && !actor->firing_positions.current_position_found_outside_range)
			{
				actor_discard_firing_position(actor_index, actor->firing_positions.current_position_index, FALSE);
			}
		}
	}

	return;
}

void action_fight_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->orders.look.primary_priority = _primary_priority_aiming;
	actor->orders.look.primary_direction.type = _direction_specification_target;
	actor->orders.look.idle_look_type = _idle_look_combat;
	actor->orders.move.stationary_crouch = actor->emotions.defensive_crouch;
	actor->orders.move.moving_crouch = FALSE;
	actor->orders.move.panicked = FALSE;
	actor->orders.move.dive_into_cover = FALSE;
	actor->orders.move.emerge_from_cover = FALSE;

	if (actor->input.vehicle_driver_type == _actor_vehicle_driver_directional_flying)
	{
	}
	else if (actor->state.combat_status >= _actor_combat_status_clear_los)
	{
		actor->orders.combat.shoot_at_target = TRUE;
		actor->orders.look.primary_priority = _primary_priority_locked_aiming;
	}

	return;
}
