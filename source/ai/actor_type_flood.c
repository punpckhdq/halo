/*
ACTOR_TYPE_FLOOD.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "props.h"
#include "actor_types.h"
#include "units.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

void flood_decide_action(long actor_index);

/* ---------- globals */

struct actor_type_definition actor_type_flood =
{
	"flood",
	_race_floodcombat,
	_actor_pursuit_always,
	_actor_pursuit_always,
	_actor_pursuit_always,
	TRUE,
	FALSE,
	NULL,
	flood_decide_action,
	NULL,
	NULL
};

/* ---------- public code */

void flood_decide_action(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	actor_action_handle_initial_action(actor_index);
	actor_action_handle_pending_command_list(actor_index);
	actor_action_handle_surprise(actor_index, _actor_surprise_unprepared_grenade);

	if (!actor_action_deny_transition(actor_index))
	{
		actor_action_handle_berserking_from_damage(actor_index);
		actor_action_handle_berserk_transition(actor_index, _actor_berserk_damage);
		actor_action_handle_combat_transition(actor_index);
		actor_action_handle_danger_avoidance(actor_index);
	}

	switch (actor->state.action)
	{
	case _actor_action_fight:
	case _actor_action_charge:
		if (!actor_action_handle_combat_status(actor_index, TRUE, FALSE))
		{
			if (!actor_action_handle_combat_failure(actor_index))
			{
				actor_action_handle_evasion(actor_index);
			}
		}
		break;
	case _actor_action_flee:
		if (actor->state.action_data.flee.unable_to_flee)
		{
			actor_action_handle_combat_status(actor_index, TRUE, TRUE);
		}
		else
		{
			actor_action_handle_done_fleeing(actor_index);
		}
		break;
	case _actor_action_guard:
		actor_action_handle_combat_status(actor_index, actor_action_can_stop_guarding(actor_index, _actor_combat_status_definite, _actor_combat_status_dangerous), FALSE);
		break;
	case _actor_action_uncover:
	case _actor_action_search:
	case _actor_action_wait:
		if (!actor_action_handle_combat_status(actor_index, TRUE, FALSE))
		{
			actor_action_handle_exit_pursuit(actor_index);
		}
		break;
	case _actor_action_obey:
		actor_action_handle_combat_status(actor_index, actor->state.action_data.obey.initiative, actor->state.action_data.obey.finished);
		break;
	case _actor_action_avoid:
		if (actor->danger_zone.danger_type == _actor_danger_zone_none)
		{
			actor_action_handle_combat_status(actor_index, TRUE, TRUE);
		}
		break;
	}

	return;
}

boolean actor_type_flood_desire_shamble(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	struct actor_variant_definition *firing_variant_definition = actor_combat_get_firing_variant_definition(actor_index);
	boolean result = FALSE;
	boolean crouch_switching = FALSE;

	if (!unit_is_busy(actor->meta.unit_index) && !actor_path_has_path(actor_index))
	{
		result = TRUE;
	}
	else if (actor->state.mode < _actor_mode_combat)
	{
		result = TRUE;
	}
	else if (actor->state.combat_status < _actor_combat_status_clear_los)
	{
		result = FALSE;
	}
	else
	{
		struct unit_datum *unit = unit_get(actor->meta.unit_index);
		struct prop_datum *target_prop = actor->target.target_prop_index == NONE ? NULL : prop_get(actor->target.target_prop_index);

		if (unit->unit.animation.state == _unit_state_hard_ping && !actor->emotions.berserk)
		{
			result = TRUE;
		}
		else if (target_prop && target_prop->distance > firing_variant_definition->ranged_combat.maximum_firing_range)
		{
			result = FALSE;
		}
		else if (actor->emotions.berserk && target_prop && target_prop->distance > firing_variant_definition->ranged_combat.berserk_firing_range_upper_bound)
		{
			result = FALSE;
		}
		else if (TEST_FLAG(unit->object.damage_flags, _object_melee_attack_inhibited_bit))
		{
			result = TRUE;
		}
		else if (actor->emotions.berserk ||
			(actor->state.action == _actor_action_charge &&
				(actor->state.action_data.charge.goal == _charge_goal_melee || actor->state.action_data.charge.goal == _charge_goal_melee_leaping)))
		{
			result = FALSE;
		}
		else if (!actor_has_ranged_weapon(actor_index) || actor->input.underwater)
		{
			result = FALSE;
		}
		else if (actor_variant_definition->movement_switching.movement_type == 0)
		{
			result = FALSE;
		}
		else if (actor_variant_definition->movement_switching.movement_type == 1)
		{
			result = TRUE;
		}
		else if (target_prop && target_prop->distance < firing_variant_definition->ranged_combat.combat_range_upper_bound)
		{
			result = TRUE;
		}
		else
		{
			crouch_switching = TRUE;
		}
	}

	if (crouch_switching)
	{
		boolean change = FALSE;

		if (actor->emotions.crouch_switching_active)
		{
			boolean flip = FALSE;

			if (actor->emotions.crouch_switching_stay_timer > 0)
			{
				actor->emotions.crouch_switching_stay_timer--;
			}
			else if (TEST_FLAG(actor_variant_definition->flags, _actor_variant_definition_movement_switching_try_to_stay_with_friends_bit) && actor->situation.close_friends > 0)
			{
				struct prop_iterator iterator;
				struct prop_datum *prop;
				struct prop_datum *target_prop;
				short friends_behind;
				short friends_beside;
				short friends_ahead;

				friends_behind = friends_beside = friends_ahead = 0;

				match_assert("c:\\halo\\SOURCE\\ai\\actor_type_flood.c", 201, actor->target.target_prop_index != NONE);

				target_prop = prop_get(actor->target.target_prop_index);

				prop_iterator_new(&iterator, actor_index);

				while (prop = prop_iterator_next(&iterator))
				{
					if (prop->state >= _prop_state_becoming_unacknowledged && prop->state <= _prop_state_acknowledged &&
						!prop->enemy && !prop->dead && prop->distance < 15.0f && prop->actor_index != NONE)
					{
						struct actor_datum *friend_actor = actor_get(prop->actor_index);

						if (friend_actor->emotions.crouch_switching_active)
						{
							real_vector3d friend_vector;
							real distance;

							vector_from_points3d(&actor->input.position.body_position, &friend_actor->input.position.body_position, &friend_vector);
							distance = dot_product3d(&friend_vector, &target_prop->actor_to_prop);

							if (distance > 1.4f)
							{
								friends_ahead++;
							}
							else if (distance < -1.4f)
							{
								friends_behind++;
							}
							else
							{
								friends_beside++;
							}
						}
					}
				}

				if (actor->emotions.crouch_switching_current_value)
				{
					if (friends_behind == 0 && friends_ahead > friends_beside)
					{
						flip = TRUE;
					}
				}
				else
				{
					if (friends_ahead == 0 && friends_behind > friends_beside)
					{
						flip = TRUE;
					}
				}
			}

			if (!flip)
			{
				match_assert("c:\\halo\\SOURCE\\ai\\actor_type_flood.c", 256, actor->emotions.crouch_switching_change_timer > 0);

				if (--actor->emotions.crouch_switching_change_timer == 0)
				{
					flip = TRUE;
				}
			}

			if (flip)
			{
				actor->emotions.crouch_switching_current_value = !actor->emotions.crouch_switching_current_value;
				change = TRUE;
			}
		}
		else
		{
			real shamble_chance = actor_variant_definition->movement_switching.initial_crouch_chance;

			if (actor->situation.area_friends > 0)
			{
				short shambling_friends, running_friends;
				struct prop_iterator iterator;
				struct prop_datum *prop;
				real adjustment;

				shambling_friends = running_friends = 0;

				prop_iterator_new(&iterator, actor_index);
				while (prop = prop_iterator_next(&iterator))
				{
					if (prop->state >= _prop_state_becoming_unacknowledged && prop->state <= _prop_state_acknowledged &&
						!prop->enemy && !prop->dead && prop->actor_index != NONE)
					{
						struct actor_datum *friend_actor = actor_get(prop->actor_index);

						if (friend_actor->meta.type == actor->meta.type && friend_actor->state.combat_status >= _actor_combat_status_clear_los)
						{
							if (friend_actor->emotions.defensive_crouch)
							{
								shambling_friends++;
							}
							else
							{
								running_friends++;
							}
						}
					}
				}

				adjustment = 0.0f;
				adjustment += shambling_friends * (1.0f - shamble_chance);
				adjustment += running_friends * (-shamble_chance);
				shamble_chance -= 0.5f * adjustment;
			}

			{
				real random_value = real_random();

				actor->emotions.crouch_switching_active = TRUE;
				actor->emotions.crouch_switching_current_value = random_value < shamble_chance;
				change = TRUE;
			}
		}

		if (change)
		{
			real ticks;

			if (actor->emotions.crouch_switching_current_value)
			{
				ticks = real_random_range(actor_variant_definition->movement_switching.crouch_time_lower_bound,
					actor_variant_definition->movement_switching.crouch_time_upper_bound);
			}
			else
			{
				ticks = real_random_range(actor_variant_definition->movement_switching.run_time_lower_bound,
					actor_variant_definition->movement_switching.run_time_upper_bound);
			}

			actor->emotions.crouch_switching_change_timer = (short)MAX(ticks * TICKS_PER_SECOND, 31.0f);
			actor->emotions.crouch_switching_stay_timer = 1 * TICKS_PER_SECOND;
		}

		result = actor->emotions.crouch_switching_current_value;
	}
	else
	{
		actor->emotions.crouch_switching_active = FALSE;
	}

	return result;
}

/* ---------- private code */
