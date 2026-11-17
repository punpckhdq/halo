/*
ACTION_CHARGE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "units.h"
#include "collisions.h"
#include "items.h"
#include "meter_definitions.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"
#include "projectiles.h"
#include "ai.h"
#include "ai_communication.h"
#include "game.h"
#include "collision_usage.h"
#include "errors.h"

/* ---------- constants */

enum
{
	CHARGE_LEAP_TICKS = 30, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static real action_charge_find_target_range(long actor_index, short goal, struct charge_state_data *state_data);
static boolean action_charge_valid_melee_destination(long actor_index, short goal, real_point3d *melee_target_point);

/* ---------- globals */

/* ---------- public code */

boolean action_charge_setup(
	long actor_index,
	short goal,
	struct charge_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean success = TRUE;
	struct actor_debug_info *actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];

	actor_debug_info->charge_last_time = game_time_get();

	match_assert("c:\\halo\\SOURCE\\ai\\action_charge.c", 47, state_data);

	memset(state_data, 0, sizeof(struct charge_state_data));
	state_data->charge_start_time = game_time_get();

	if (goal == _charge_goal_vehicle_ramming || goal == _charge_goal_vehicle_strafing)
	{
		success = actor->input.vehicle_driver_type > _actor_vehicle_driver_unknown;
		actor_debug_info->charge_decision = !success;
	}
	else if (goal == _charge_goal_melee)
	{
		if (actor->meta.swarm)
		{
			success = FALSE;
			actor_debug_info->charge_decision = _charge_melee_swarm_cant;
		}
		else
		{
			struct unit_datum *unit = unit_get(actor->meta.unit_index);

			success = FALSE;
			if (TEST_FLAG(unit->object.damage_flags, _object_melee_attack_inhibited_bit))
			{
				success = FALSE;
				actor_debug_info->charge_decision = _charge_melee_inhibited;
			}
			else if (actor->target.target_prop_index == NONE)
			{
				success = FALSE;
				actor_debug_info->charge_decision = _charge_melee_notarget;
			}
			else
			{
				struct prop_datum *prop = prop_get(actor->target.target_prop_index);
				boolean leaping;

				if (actor_definition->berserk.melee_leap_range_upper_bound == 0.f || actor_definition->berserk.melee_leap_chance == 0.f)
				{
					state_data->leap_possible_if_at_range = FALSE;
					leaping = FALSE;
				}
				else if (prop->flying || prop->unreachable_ticks > 0)
				{
					state_data->leap_possible_if_at_range = TRUE;
					leaping = TRUE;
				}
				else
				{
					state_data->leap_possible_if_at_range = real_random() < actor_definition->berserk.melee_leap_chance;
					leaping = state_data->leap_possible_if_at_range;
					if (prop->distance < actor_definition->berserk.melee_leap_range_lower_bound)
					{
						leaping = FALSE;
					}
				}

				if (leaping)
				{
					goal = _charge_goal_melee_leaping;
				}

				actor_debug_info->field_198 = leaping;

				{
					short melee_start_tick;
					short melee_end_tick;
					real melee_start_range;
					real melee_end_range;

					if (unit_get_melee_range_and_ticks(actor->meta.unit_index, leaping, &melee_start_tick, &melee_start_range, &melee_end_tick, &melee_end_range))
					{
						if (TEST_FLAG(actor_definition->flags, _actor_definition_suicidal_melee_attack_bit))
						{
							state_data->melee_ticks_until_dangerous = melee_end_tick;
							state_data->melee_danger_range = 0.f;
							state_data->melee_suicide = TRUE;
						}
						else
						{
							if (melee_start_tick == 0)
							{
								struct actor_variant_definition *variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);

								if (variant_definition->actor_reference.name)
								{
									error(_error_silent, "actor %s melee animation has no damage keyframe", variant_definition->actor_reference.name);
								}

								melee_start_tick = melee_end_tick / 2;
								melee_start_range = melee_end_range * 0.5f;
							}

							state_data->melee_ticks_until_dangerous = melee_start_tick;
							state_data->melee_danger_range = melee_end_range - melee_start_range;
						}

						{
							real minimum_range;
							real accept_radius;
							boolean valid = FALSE;

							state_data->acceptable_target_range = action_charge_find_target_range(actor_index, goal, state_data);

							minimum_range = goal == _charge_goal_melee_leaping ? 4.f : 1.5f;
							accept_radius = MAX(minimum_range, state_data->acceptable_target_range);

							if (actor_move_to_prop(actor_index, actor->target.target_prop_index, accept_radius))
							{
								actor_move_keep_moving_past_destination(actor_index);
								valid = action_charge_valid_melee_destination(actor_index, goal, &prop->center_of_mass);
							}

							if (valid)
							{
								success = TRUE;
								actor_debug_info->charge_decision = _charge_melee_success;
							}
							else
							{
								actor_debug_info->charge_decision = _charge_melee_cannot_move;
								actor_debug_info->field_194 = accept_radius;
							}
						}
					}
					else
					{
						actor_debug_info->charge_decision = _charge_melee_no_animation;
					}
				}
			}
		}
	}
	else
	{
		if (goal == _charge_goal_close_range &&
			TEST_FLAG(actor_definition->flags, _actor_definition_stalking_behavior_bit) &&
			actor->state.combat_status >= _actor_combat_status_clear_los &&
			!actor->emotions.berserk)
		{
			goal = _charge_goal_stalking;
			actor_debug_info->charge_decision = _charge_stalking_success;
		}
		else
		{
			actor_debug_info->charge_decision = _charge_close_success;
		}
	}

	state_data->goal = goal;

	return success;
}

void action_charge_begin(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct charge_state_data *state_data = &actor->state.action_data.charge;

	if (state_data->goal == _charge_goal_vehicle_strafing)
	{
		struct actor_variant_definition *variant_definition = actor_combat_get_firing_variant_definition(actor_index);

		if (variant_definition->ranged_combat.special_fire_situation == _actor_special_fire_situation_strafing && actor->control.special_fire_deny_attempts > 0)
		{
			actor->control.special_fire_deny_attempts--;
		}
	}

	return;
}

boolean action_charge_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct actor_variant_definition *variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	struct actor_variant_definition *firing_variant_definition = actor_combat_get_firing_variant_definition(actor_index);
	struct charge_state_data *state_data = &actor->state.action_data.charge;
	struct prop_datum *prop = NULL;
	boolean result = FALSE;
	long time;

	if (actor->target.target_prop_index != NONE)
	{
		prop = prop_get(actor->target.target_prop_index);

		if (actor->input.delayed_attached_projectile_index != NONE)
		{
			state_data->advancing = TRUE;
		}
		else if (state_data->goal == _charge_goal_vehicle_ramming || state_data->goal == _charge_goal_vehicle_strafing)
		{
			state_data->advancing = TRUE;
		}
		else if (state_data->goal != _charge_goal_melee && state_data->goal != _charge_goal_melee_leaping)
		{
			boolean stalking = TEST_FLAG(actor_definition->flags, _actor_definition_stalking_behavior_bit) &&
				actor->state.combat_status >= _actor_combat_status_clear_los &&
				!actor->emotions.berserk;

			state_data->goal = stalking ? _charge_goal_stalking : _charge_goal_close_range;

			if (state_data->goal == _charge_goal_stalking)
			{
				state_data->stalking_currently_exposed = (prop->line_of_sight == _ai_line_of_sight_clear || prop->line_of_sight == _ai_line_of_sight_occluded) &&
					prop->quantized_facing <= 2;
				if (state_data->stalking_currently_exposed && TEST_FLAG(actor_definition->flags, _actor_definition_stalking_freeze_when_exposed_bit))
				{
					state_data->advancing = FALSE;
				}
				else
				{
					state_data->advancing = TRUE;
				}

				if (state_data->stalking_currently_exposed)
				{
					state_data->stalking_discovery_timer++;
				}

				state_data->stalking_catch_target = FALSE;
				if (!state_data->stalking_currently_exposed && prop->quantized_closing_speed <= 1)
				{
					state_data->stalking_catch_target = TRUE;
				}
				else if (actor_definition->defensive.stalking_max_distance > 0.f && prop->distance >= actor_definition->defensive.stalking_max_distance)
				{
					state_data->stalking_catch_target = TRUE;
				}
			}
			else if (actor_has_ranged_weapon(actor_index) && !actor->input.underwater)
			{
				real range_lower_bound;
				real range_upper_bound;
				struct weapon_definition *weapon_definition;

				if (actor->emotions.berserk)
				{
					range_lower_bound = firing_variant_definition->ranged_combat.berserk_firing_range_lower_bound;
					range_upper_bound = firing_variant_definition->ranged_combat.berserk_firing_range_upper_bound;
				}
				else
				{
					range_lower_bound = firing_variant_definition->ranged_combat.combat_range_lower_bound;
					range_upper_bound = firing_variant_definition->ranged_combat.combat_range_upper_bound;
				}

				weapon_definition = actor_get_weapon_definition(actor_index);
				if (weapon_definition && weapon_definition->weapon.ai_minimum_target_range > 0.f)
				{
					range_lower_bound = MAX(range_lower_bound, weapon_definition->weapon.ai_minimum_target_range);
				}

				if (state_data->advancing)
				{
					if (prop->distance < range_lower_bound)
					{
						state_data->advancing = FALSE;
					}
				}
				else if (prop->distance > range_upper_bound)
				{
					state_data->advancing = TRUE;
				}

				if (prop->distance > 0.7f &&
					prop->line_of_sight != _ai_line_of_sight_clear &&
					prop->line_of_sight != _ai_line_of_sight_occluded)
				{
					state_data->advancing = TRUE;
				}
			}
			else
			{
				state_data->advancing = TRUE;
			}
		}
		else
		{
			real abort_range = REAL_MAX;
			boolean can_abort = TRUE;
			boolean berserk = actor->emotions.berserk;

			if (!actor_has_ranged_weapon(actor_index))
			{
				berserk = TRUE;
			}

			match_assert("c:\\halo\\SOURCE\\ai\\action_charge.c", 288, !actor->meta.swarm);

			if (state_data->launched_melee_attack || state_data->launched_leap || state_data->leap_pending)
			{
				can_abort = FALSE;
			}
			else if (actor->emotions.berserk || !actor_has_ranged_weapon(actor_index))
			{
				abort_range = berserk ? variant_definition->ranged_combat.berserk_melee_abort_range : variant_definition->ranged_combat.melee_abort_range;
			}

			if (actor->external_orders.disable_charging)
			{
				real maximum_range = MAX(0.f, actor_definition->berserk.melee_attack_range) + 0.8f;

				abort_range = MIN(abort_range, maximum_range);
			}

			if (!(can_abort && abort_range < prop->distance))
			{
				actor->emotions.last_melee_attack_time = game_time_get();
				state_data->advancing = TRUE;

				if (can_abort)
				{
					if (state_data->goal == _charge_goal_melee)
					{
						if (actor_definition->berserk.melee_leap_range_upper_bound == 0.f || actor_definition->berserk.melee_leap_chance == 0.f)
						{
							state_data->leap_possible_if_at_range = FALSE;
						}
						else if (prop->flying || prop->unreachable_ticks > 0)
						{
							state_data->leap_possible_if_at_range = TRUE;
						}

						if (state_data->leap_possible_if_at_range &&
							(prop->unreachable_ticks > 0 || actor_definition->berserk.melee_leap_range_lower_bound * 1.5f < prop->distance))
						{
							state_data->goal = _charge_goal_melee_leaping;
						}
					}
					else if (prop->distance < actor_definition->berserk.melee_leap_range_lower_bound)
					{
						state_data->goal = _charge_goal_melee;
						state_data->leap_possible_if_at_range = TRUE;
					}
				}
			}
			else
			{
				state_data->aborted_melee_attack = TRUE;
			}
		}
	}
	else
	{
		state_data->advancing = FALSE;
	}

	if (state_data->launched_melee_attack)
	{
		state_data->finished_melee_attack = actor->meta.unit_index == NONE || !unit_is_busy(actor->meta.unit_index);
	}
	else if (!state_data->leap_pending &&
		(state_data->goal == _charge_goal_melee || state_data->goal == _charge_goal_melee_leaping) &&
		prop)
	{
		boolean melee_attack = FALSE;
		struct actor_debug_info *actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
		real_vector3d vector_to_target;

		actor_debug_info->last_melee_time = game_time_get();
		point_from_line3d(&actor->input.position.body_position, global_up3d, 0.05f, &actor_debug_info->field_108);
		actor_debug_info->field_114 = actor->input.facing_vector;
		point_from_line3d(&prop->body_position, global_up3d, 0.05f, &actor_debug_info->field_120);

		match_assert("c:\\halo\\SOURCE\\ai\\action_charge.c", 487, !actor->meta.swarm);

		if (prop->distance < 0.8f)
		{
			actor_debug_info->field_139 = TRUE;
			vector_to_target = prop->actor_to_prop;
			melee_attack = TRUE;
		}
		else
		{
			struct unit_datum *unit = unit_get(actor->meta.unit_index);
			real distance;
			real_point3d estimated_target_position;
			real lead_fraction;
			real target_speed;

			actor_debug_info->field_139 = FALSE;

			lead_fraction = 0.f;
			target_speed = magnitude3d(&prop->velocity);
			if (target_speed > 0.f)
			{
				lead_fraction = (dot_product3d(&prop->velocity, &prop->actor_to_prop) / target_speed + 1.f) * 0.5f;
			}

			point_from_line3d(&prop->body_position, &prop->velocity, (real)state_data->melee_ticks_until_dangerous * lead_fraction, &estimated_target_position);
			vector_from_points3d(&actor->input.position.body_position, &estimated_target_position, &vector_to_target);
			point_from_line3d(&estimated_target_position, global_up3d, 0.05f, &actor_debug_info->field_13C);

			if (dot_product3d(&vector_to_target, &prop->actor_to_prop) < 0.f)
			{
				distance = 0.f;
				vector_to_target = prop->actor_to_prop;
			}
			else
			{
				distance = normalize3d(&vector_to_target);
				if (distance == 0.f)
				{
					vector_to_target = prop->actor_to_prop;
				}
			}

			if (state_data->goal == _charge_goal_melee_leaping && !state_data->launched_leap)
			{
				actor_debug_info->field_148 = actor_definition->berserk.melee_leap_range_lower_bound;
				actor_debug_info->field_14C = actor_definition->berserk.melee_leap_range_upper_bound;

				if (distance < actor_definition->berserk.melee_leap_range_lower_bound && prop->unreachable_ticks == 0 && !prop->flying)
				{
					state_data->aborted_melee_attack = TRUE;
					actor->emotions.last_melee_attack_time = NONE;
				}
				else if (distance < actor_definition->berserk.melee_leap_range_upper_bound)
				{
					real minimum_velocity = actor_definition->berserk.melee_leap_velocity * 0.3f;
					real_vector3d aim_vector;
					real vertical_velocity;
					real horizontal_velocity;

					if (projectile_aim_ballistic(
						actor_definition->berserk.melee_leap_velocity,
						1.f,
						&actor->input.position.body_position,
						&prop->body_position,
						&minimum_velocity,
						&actor_definition->berserk.melee_leap_ballistic,
						NULL,
						FALSE,
						&aim_vector,
						NULL,
						NULL,
						NULL,
						&vertical_velocity,
						&horizontal_velocity))
					{
						if (normalize2d((real_vector2d *)&aim_vector) == 0.f)
						{
							aim_vector = actor->input.facing_vector;
							if (normalize2d((real_vector2d *)&aim_vector) == 0.f)
							{
								aim_vector = *global_forward3d;
							}
						}

						state_data->leap_pending = TRUE;
						state_data->leap_alignment_vector = *(real_vector2d *)&aim_vector;
						state_data->leap_horizontal_velocity = horizontal_velocity;
						state_data->leap_vertical_velocity = vertical_velocity;
					}
				}
			}
			else if (state_data->melee_suicide)
			{
				actor_debug_info->field_148 = 0.f;
				actor_debug_info->field_14C = actor_definition->berserk.melee_attack_range;

				if (distance < actor_definition->berserk.melee_attack_range)
				{
					melee_attack = TRUE;
				}
				else if (distance < actor_definition->berserk.suicide_sensing_distance)
				{
					real_vector3d target_relative_velocity;

					subtract_vectors3d(&prop->velocity, &unit->object.translational_velocity, &target_relative_velocity);
					if (dot_product3d(&target_relative_velocity, &vector_to_target) > 0.7f / TICKS_PER_SECOND)
					{
						melee_attack = TRUE;
					}
				}
			}
			else
			{
				if (state_data->goal == _charge_goal_melee_leaping && state_data->launched_leap)
				{
					distance -= dot_product3d(&vector_to_target, &unit->object.translational_velocity) * (real)state_data->melee_ticks_until_dangerous;
				}

				actor_debug_info->field_148 = state_data->melee_danger_range;
				actor_debug_info->field_14C = state_data->melee_danger_range + actor_definition->berserk.melee_attack_range;

				if (distance < state_data->melee_danger_range + actor_definition->berserk.melee_attack_range)
				{
					melee_attack = TRUE;
				}
			}
		}

		if (state_data->leap_pending || (melee_attack && !state_data->melee_suicide))
		{
			real_vector2d target_alignment_vector;

			target_alignment_vector = *(real_vector2d *)&vector_to_target;

			if (normalize2d(&target_alignment_vector) > 0.f)
			{
				real minimum_alignment = state_data->launched_leap ? 0.f : _cosine30;

				if (dot_product2d(&target_alignment_vector, (real_vector2d *)&actor->input.facing_vector) < minimum_alignment)
				{
					melee_attack = FALSE;
					state_data->leap_pending = FALSE;
					state_data->alignment_incorrect = TRUE;
				}
			}
		}

		actor_debug_info->field_12C = vector_to_target;
		actor_debug_info->field_138 = melee_attack;

		if (melee_attack)
		{
			real_vector2d alignment_vector;

			alignment_vector = *(real_vector2d *)&vector_to_target;

			if (normalize2d(&alignment_vector) == 0.f)
			{
				alignment_vector = *(real_vector2d *)&actor->input.facing_vector;
			}

			if (unit_melee_attack_begin(actor->meta.unit_index, FALSE, &alignment_vector))
			{
				ai_communication_event(_ai_communication_melee, actor->meta.unit_index, prop->unit_index, _comm_hostility_enemy, NONE, NONE, NULL);
				state_data->launched_melee_attack = TRUE;
			}
		}
	}

	time = game_time_get();

	if ((state_data->goal == _charge_goal_melee || state_data->goal == _charge_goal_melee_leaping) &&
		!state_data->launched_melee_attack &&
		!state_data->leap_pending)
	{
		if (state_data->launched_leap)
		{
			if (state_data->leap_failure_timer > 15)
			{
				state_data->aborted_melee_attack = TRUE;
			}
		}
		else if (actor_definition->berserk.melee_attack_charge_timer > 0.f &&
			(real)time >= actor_definition->berserk.melee_attack_charge_timer * 30.f + (real)state_data->charge_start_time)
		{
			state_data->aborted_melee_attack = TRUE;
		}
	}

	if (state_data->goal == _charge_goal_vehicle_strafing || state_data->goal == _charge_goal_vehicle_ramming)
	{
		actor->emotions.last_vehicle_charge_time = time;
	}

	state_data->acceptable_target_range = action_charge_find_target_range(actor_index, state_data->goal, state_data);

	if (!actor->meta.swarm && actor->meta.timeslice)
	{
		boolean moving = FALSE;

		state_data->unable_to_advance = FALSE;

		if (!state_data->launched_melee_attack &&
			!state_data->launched_leap &&
			!state_data->leap_pending &&
			state_data->advancing)
		{
			real minimum_range = state_data->goal == _charge_goal_melee_leaping ? 4.f : 1.5f;
			real accept_radius = MAX(minimum_range, state_data->acceptable_target_range);

			if (actor_move_to_prop(actor_index, actor->target.target_prop_index, accept_radius))
			{
				actor_move_keep_moving_past_destination(actor_index);
				moving = TRUE;
			}
			else
			{
				state_data->unable_to_advance = TRUE;
				state_data->advancing = FALSE;
			}
		}

		if (!moving)
		{
			actor_move_halt(actor_index);
		}

		if (actor->target.target_type >= _actor_target_acknowledged_enemy)
		{
			struct prop_datum *target_prop = prop_get(actor->target.target_prop_index);
			boolean unreachable = FALSE;
			boolean beyond_range = FALSE;

			if (target_prop->distance > state_data->acceptable_target_range)
			{
				beyond_range = TRUE;
			}

			if ((state_data->goal == _charge_goal_melee || state_data->goal == _charge_goal_melee_leaping) &&
				(state_data->launched_leap || state_data->leap_pending || state_data->launched_melee_attack))
			{
				beyond_range = FALSE;
			}

			if (beyond_range)
			{
				if (state_data->unable_to_advance || !actor_path_has_path(actor_index))
				{
					unreachable = TRUE;
				}
				else if (actor->control.path.path.endpoint.target_radius > state_data->acceptable_target_range)
				{
					unreachable = TRUE;
				}
			}

			actor_perception_unreachable(actor_index, actor->target.target_prop_index, unreachable);
		}
	}

	if (state_data->goal == _charge_goal_melee || state_data->goal == _charge_goal_melee_leaping)
	{
		result = state_data->aborted_melee_attack || state_data->finished_melee_attack || state_data->unable_to_advance;
	}
	else if (state_data->goal == _charge_goal_vehicle_strafing || state_data->goal == _charge_goal_vehicle_ramming)
	{
		result = state_data->unable_to_advance;
	}

	return result;
}

static real action_charge_find_target_range(
	long actor_index,
	short goal,
	struct charge_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	real target_range = 0.f;

	if (goal == _charge_goal_melee || goal == _charge_goal_melee_leaping)
	{
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

		if (goal == _charge_goal_melee_leaping)
		{
			target_range = MAX(target_range, actor_definition->berserk.melee_leap_range_upper_bound);
		}

		if (state_data->melee_suicide)
		{
			target_range = MAX(target_range, actor_definition->berserk.melee_attack_range);
		}
		else
		{
			real range = state_data->melee_danger_range + actor_definition->berserk.melee_attack_range;

			target_range = MAX(target_range, range);
		}
	}
	else if (goal == _charge_goal_vehicle_strafing || goal == _charge_goal_close_range)
	{
		if (actor_has_ranged_weapon(actor_index) && actor->target.target_type >= _actor_target_acknowledged_enemy)
		{
			target_range = MAX(target_range, actor->control.weapon_maximum_range);
		}
	}

	return target_range;
}

static boolean action_charge_valid_melee_destination(
	long actor_index,
	short goal,
	real_point3d *melee_target_point)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean valid = TRUE;
	real_point3d estimated_position;

	if (actor->control.path.at_destination)
	{
		estimated_position = actor->input.position.head_position;
	}
	else if (actor->control.path.path.valid)
	{
		unit_estimate_position(actor->meta.unit_index, _unit_estimate_head_standing, &actor->control.path.path.endpoint.point, NULL, NULL, &estimated_position);
	}
	else
	{
		valid = FALSE;
	}

	if (valid)
	{
		long flags =
			FLAG(_collision_test_front_facing_surfaces_bit) |
			FLAG(_collision_test_back_facing_surfaces_bit) |
			FLAG(_collision_test_ignore_breakable_surfaces_bit) |
			FLAG(_collision_test_structure_bit);
		struct collision_result collision;

		match_collision_log_begin_user("c:\\halo\\SOURCE\\ai\\action_charge.c", 889, _collision_user_ai_melee);
		if (!collision_test_line(flags, &estimated_position, melee_target_point, NONE, &collision))
		{
			valid = TRUE;
		}
		match_collision_log_end_user("c:\\halo\\SOURCE\\ai\\action_charge.c", 897);
	}

	return valid;
}

void action_charge_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct charge_state_data *state_data = &actor->state.action_data.charge;

	if (state_data->goal == _charge_goal_melee_leaping &&
		state_data->launched_leap &&
		!state_data->launched_melee_attack &&
		!actor->input.in_midair)
	{
		state_data->leap_failure_timer++;
	}

	return;
}

void action_charge_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct charge_state_data *state_data = &actor->state.action_data.charge;

	actor->orders.look.primary_direction.type = _direction_specification_target;
	actor->orders.look.idle_look_type = _idle_look_combat;

	if ((state_data->goal == _charge_goal_melee || state_data->goal == _charge_goal_melee_leaping) &&
		state_data->alignment_incorrect &&
		!actor->control.moving &&
		!actor_path_has_path(actor_index))
	{
		actor->orders.look.primary_priority = _primary_priority_exact_facing;
	}
	else if (actor->state.combat_status < _actor_combat_status_clear_los || state_data->goal == _charge_goal_stalking)
	{
		actor->orders.look.primary_priority = _primary_priority_aiming;
	}
	else
	{
		actor->orders.look.primary_priority = _primary_priority_locked_aiming;
	}

	if (state_data->goal == _charge_goal_stalking)
	{
		actor->orders.move.stationary_crouch = !state_data->stalking_catch_target;
		actor->orders.move.moving_crouch = !state_data->stalking_catch_target;
	}
	else if (actor->orders.move.panicked)
	{
		actor->orders.move.stationary_crouch = FALSE;
		actor->orders.move.moving_crouch = FALSE;
	}
	else if (TEST_FLAG(actor_definition->flags, _actor_definition_defensive_crouch_while_charging_bit))
	{
		actor->orders.move.stationary_crouch = actor->emotions.defensive_crouch;
		actor->orders.move.moving_crouch = actor->emotions.defensive_crouch;
	}
	else
	{
		actor->orders.move.stationary_crouch = FALSE;
		actor->orders.move.moving_crouch = FALSE;
	}

	if (state_data->leap_pending)
	{
		actor->orders.move.jump = TRUE;
		actor->orders.move.jump_leap = state_data->leap_horizontal_velocity * 0.7f > state_data->leap_vertical_velocity;
		actor->orders.move.jump_targeted = TRUE;
		actor->orders.move.jump_alignment_vector = state_data->leap_alignment_vector;
		actor->orders.move.jump_target_horizontal_vel = state_data->leap_horizontal_velocity;
		actor->orders.move.jump_target_vertical_vel = state_data->leap_vertical_velocity;

		state_data->launched_leap = TRUE;
		state_data->leap_pending = FALSE;
		state_data->leap_start_time = game_time_get();
		state_data->leap_failure_timer = 0;
	}

	if (TEST_FLAG(actor_definition->flags, _actor_definition_berserk_use_panic_movement_bit))
	{
		if (actor->emotions.berserk || state_data->goal == _charge_goal_melee || state_data->goal == _charge_goal_melee_leaping)
		{
			actor->orders.move.panicked = state_data->advancing && !actor->orders.move.moving_crouch;
		}
	}

	actor->orders.move.move_face_exactly = TRUE;
	actor->orders.move.dive_into_cover = FALSE;
	actor->orders.move.emerge_from_cover = FALSE;
	actor->orders.combat.shoot_at_target = state_data->goal != _charge_goal_stalking;

	return;
}

boolean action_charge_is_leaping(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct charge_state_data *state_data = &actor->state.action_data.charge;
	boolean leaping = FALSE;

	if (state_data->goal == _charge_goal_melee_leaping && state_data->launched_leap)
	{
		leaping = state_data->leap_start_time + CHARGE_LEAP_TICKS >= game_time_get();
	}

	return leaping;
}

/* ---------- private code */
