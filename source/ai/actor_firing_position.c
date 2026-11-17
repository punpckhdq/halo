/*
ACTOR_FIRING_POSITION.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "ai_profile.h"
#include "collision_bsp.h"
#include "render.h"
#include "vehicles.h"
#include "meter_definitions.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"

/* ---------- constants */

enum
{
	_firing_position_group_normal = 0,
	_firing_position_group_when_searching,
	_firing_position_group_when_not_searching,
};

enum
{
	_firing_position_attack_vector_friend = 0,
	_firing_position_attack_vector_friend_player,
	_firing_position_attack_vector_dangerous_enemy,
	NUMBER_OF_FIRING_POSITION_ATTACK_VECTOR_TYPES,
};

enum
{
	_firing_position_evaluation_discarded = 0, /* fake name */
	_firing_position_evaluation_pursuit_distance_from_actor, /* fake name */
	_firing_position_evaluation_pursuit_distance_to_target, /* fake name */
	_firing_position_evaluation_pursuit_hint_direction, /* fake name */
	_firing_position_evaluation_pursuit_examined_by_self, /* fake name */
	_firing_position_evaluation_pursuit_examined, /* fake name */
	_firing_position_evaluation_pursuit_actor_count, /* fake name */
	_firing_position_evaluation_pursuit_unexamined, /* fake name */
	_firing_position_evaluation_panic_distance_from_actor, /* fake name */
	_firing_position_evaluation_panic_distance_to_target, /* fake name */
	_firing_position_evaluation_panic_approach_to_target, /* fake name */
	_firing_position_evaluation_vehicle_direction, /* fake name */
	_firing_position_evaluation_attack_firing_range, /* fake name */
	_firing_position_evaluation_attack_combat_range, /* fake name */
	_firing_position_evaluation_attack_line_of_sight, /* fake name */
	_firing_position_evaluation_attack_no_line_of_sight, /* fake name */
	_firing_position_evaluation_hide_occluded, /* fake name */
	_firing_position_evaluation_hide_clear, /* fake name */
	_firing_position_evaluation_hide_line_of_sight, /* fake name */
	_firing_position_evaluation_uncover_too_far, /* fake name */
	_firing_position_evaluation_uncover_line_of_sight, /* fake name */
	_firing_position_evaluation_danger_zone_crossed, /* fake name */
	_firing_position_evaluation_danger_zone_inside, /* fake name */
	_firing_position_evaluation_danger_zone_distance, /* fake name */
	_firing_position_evaluation_no_path_available, /* fake name */
	_firing_position_evaluation_path_available, /* fake name */
	NUMBER_OF_FIRING_POSITION_EVALUATIONS, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

typedef void (*pre_evaluator)(long actor_index, struct firing_position_evaluation_context *evaluation_context, short firing_position_count, struct firing_position *firing_positions);
typedef boolean (*post_evaluator)(long actor_index, struct firing_position_evaluation_context *evaluation_context, struct firing_position *firing_position);

struct pre_evaluator_table_entry
{
	short evaluation_mode_flags; /* fake name */
	pre_evaluator evaluator; /* fake name */
};

struct post_evaluator_table_entry
{
	short evaluation_mode_flags; /* fake name */
	post_evaluator evaluator; /* fake name */
};

/* ---------- prototypes */

static void firing_position_store_evaluation_debug(struct firing_position_evaluation_context *evaluation_context, struct firing_position *firing_position, real evaluation, short evaluation_type);
static boolean firing_position_reject_debug(struct firing_position_evaluation_context *evaluation_context, struct firing_position *firing_position, short rejection_type);
static void pre_evaluator_guard(long actor_index, struct firing_position_evaluation_context *evaluation_context, short firing_position_count, struct firing_position *firing_positions);
static void pre_evaluator_combatmove(long actor_index, struct firing_position_evaluation_context *evaluation_context, short firing_position_count, struct firing_position *firing_positions);
static boolean post_evaluator_global(long actor_index, struct firing_position_evaluation_context *evaluation_context, struct firing_position *firing_position);
static boolean post_evaluator_pursuit(long actor_index, struct firing_position_evaluation_context *evaluation_context, struct firing_position *firing_position);
static boolean post_evaluator_hide(long actor_index, struct firing_position_evaluation_context *evaluation_context, struct firing_position *firing_position);
static boolean post_evaluator_uncover(long actor_index, struct firing_position_evaluation_context *evaluation_context, struct firing_position *firing_position);
static boolean post_evaluator_attack(long actor_index, struct firing_position_evaluation_context *evaluation_context, struct firing_position *firing_position);
static void firing_position_pre_evaluate(long actor_index, struct firing_position_evaluation_context *evaluation_context, short firing_position_count, struct firing_position *firing_positions);
static boolean firing_position_post_evaluate(long actor_index, struct firing_position_evaluation_context *evaluation_context, struct firing_position *firing_position);
static boolean firing_positions_get_post_evaluation_bound(long actor_index, struct firing_position_evaluation_context *evaluation_context);
static boolean firing_position_compare(long index1, long index2);
static void pre_evaluator_global(long actor_index, struct firing_position_evaluation_context *evaluation_context, short firing_position_count, struct firing_position *firing_positions);
static void pre_evaluator_pursuit(long actor_index, struct firing_position_evaluation_context *evaluation_context, short firing_position_count, struct firing_position *firing_positions);
static void pre_evaluator_panic(long actor_index, struct firing_position_evaluation_context *evaluation_context, short firing_position_count, struct firing_position *firing_positions);
static void firing_position_compute_line_of_sight(long actor_index, struct firing_position_evaluation_context *evaluation_context, struct firing_position *firing_position);
static boolean firing_position_forced_evaluation(long actor_index, struct firing_position_evaluation_context *evaluation_context, struct firing_position *firing_position);
static void pre_evaluator_attack(long actor_index, struct firing_position_evaluation_context *evaluation_context, short firing_position_count, struct firing_position *firing_positions);

/* ---------- globals */

struct pre_evaluator_table_entry const global_pre_evaluator_table[] =
{
	{ NONE, pre_evaluator_global },
	{ FLAG(_firing_point_evaluation_mode_fight) | FLAG(_firing_point_evaluation_mode_uncover), pre_evaluator_attack },
	{ FLAG(_firing_point_evaluation_mode_fight) | FLAG(_firing_point_evaluation_mode_cover) | FLAG(_firing_point_evaluation_mode_uncover) | FLAG(_firing_point_evaluation_mode_avoid), pre_evaluator_combatmove },
	{ FLAG(_firing_point_evaluation_mode_guard), pre_evaluator_guard },
	{ FLAG(_firing_point_evaluation_mode_panic), pre_evaluator_panic },
	{ FLAG(_firing_point_evaluation_mode_pursue), pre_evaluator_pursuit },
	{ 0, NULL },
};

struct post_evaluator_table_entry const global_post_evaluator_table[] =
{
	{ NONE, post_evaluator_global },
	{ FLAG(_firing_point_evaluation_mode_fight) | FLAG(_firing_point_evaluation_mode_guard) | FLAG(_firing_point_evaluation_mode_avoid), post_evaluator_attack },
	{ FLAG(_firing_point_evaluation_mode_uncover), post_evaluator_uncover },
	{ FLAG(_firing_point_evaluation_mode_panic) | FLAG(_firing_point_evaluation_mode_cover), post_evaluator_hide },
	{ FLAG(_firing_point_evaluation_mode_pursue), post_evaluator_pursuit },
	{ 0, NULL },
};

short global_temporary_sort_firing_position_count = 0;
struct firing_position const *global_temporary_sort_firing_position_array = NULL;

/* ---------- public code */

static void firing_position_store_evaluation_debug(
	struct firing_position_evaluation_context *evaluation_context,
	struct firing_position *firing_position,
	real evaluation,
	short evaluation_type)
{
	match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 129, (evaluation >= 0.0f) && (evaluation < 1e+03f));
	firing_position->evaluation += evaluation;

	return;
}

static boolean firing_position_reject_debug(
	struct firing_position_evaluation_context *evaluation_context,
	struct firing_position *firing_position,
	short rejection_type)
{
	firing_position->rejected = TRUE;

	return !evaluation_context->allow_rejected_positions;
}

static void pre_evaluator_global(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	short firing_position_count,
	struct firing_position *firing_positions)
{
	short index;
	struct actor_datum *actor = actor_get(actor_index);
	struct firing_position *firing_position;

	for (index = 0, firing_position = firing_positions; index < firing_position_count; index++, firing_position++)
	{
		if (!firing_position->valid)
		{
			continue;
		}

		if (actor_firing_position_discarded(actor_index, firing_position->original_index))
		{
			if (firing_position_reject_debug(evaluation_context, firing_position, _firing_position_evaluation_discarded))
			{
				firing_position->valid = FALSE;
				continue;
			}
		}

		if (evaluation_context->find_path_direction_from_actor)
		{
			real_vector3d danger_vector;

			match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 186, (actor->danger_zone.danger_type > _actor_danger_zone_none) && actor->danger_zone.noticed_danger);

			vector_from_points3d(&actor->danger_zone.position, &actor->danger_zone.predict_danger_position, &danger_vector);

			if (distance_squared3d(&actor->danger_zone.bounding_sphere_center, &firing_position->definition->position) <
				(actor->danger_zone.bounding_sphere_radius + 2.5f) * (actor->danger_zone.bounding_sphere_radius + 2.5f))
			{
				real distance_squared = point_to_line_distance_squared3d(&firing_position->definition->position, &actor->danger_zone.position, &danger_vector);
				real evaluation = 0.0f;

				if (distance_squared < actor->danger_zone.danger_radius * actor->danger_zone.danger_radius)
				{
					if (firing_position_reject_debug(evaluation_context, firing_position, _firing_position_evaluation_danger_zone_inside))
					{
						firing_position->valid = FALSE;
						continue;
					}
				}
				else if (distance_squared < (actor->danger_zone.danger_radius + 2.5f) * (actor->danger_zone.danger_radius + 2.5f))
				{
					real distance = square_root(distance_squared);

					evaluation = (distance - actor->danger_zone.danger_radius) * 8.0f;
				}
				else
				{
					evaluation = 20.0f;
				}

				firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_danger_zone_distance);
			}

			if (distance_squared3d(&actor->danger_zone.bounding_sphere_center, &actor->input.position.body_position) <
				(actor->danger_zone.bounding_sphere_radius + 3.0f) * (actor->danger_zone.bounding_sphere_radius + 3.0f) &&
				actor->danger_zone.current_distance_from_actor > actor->danger_zone.danger_radius)
			{
				real distance_squared = point_to_line_distance_squared3d(&actor->input.position.body_position, &actor->danger_zone.position, &danger_vector);

				if (distance_squared > actor->danger_zone.danger_radius * actor->danger_zone.danger_radius)
				{
					real_vector3d actor_predicted_movement;

					scale_vector3d(&firing_position->path_direction_from_actor, 3.0f, &actor_predicted_movement);
					if (magnitude_squared3d(&actor_predicted_movement) > 0.0001f)
					{
						if (vector_to_line_distance_squared3d(&actor->input.position.body_position, &actor_predicted_movement, &actor->danger_zone.position, &danger_vector) < actor->danger_zone.danger_radius * actor->danger_zone.danger_radius)
						{
							if (firing_position_reject_debug(evaluation_context, firing_position, _firing_position_evaluation_danger_zone_crossed))
							{
								firing_position->valid = FALSE;
								continue;
							}
						}
					}
				}
			}
		}

		if (TEST_FLAG(evaluation_context->preferred_groups, firing_position->definition->group_index))
		{
			real evaluation = evaluation_context->preferred_weight;

			firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_pursuit_distance_to_target);
		}

		if (evaluation_context->avoid_point_count > 0)
		{
			real closest_fraction = 1.0f;
			short point_index;
			real evaluation;

			for (point_index = 0; point_index < evaluation_context->avoid_point_count; point_index++)
			{
				real distance_squared = distance_squared3d(&firing_position->definition->position, &evaluation_context->avoid_point[point_index].point);
				real fraction = distance_squared / (evaluation_context->avoid_point[point_index].radius * evaluation_context->avoid_point[point_index].radius);

				if (fraction < closest_fraction)
				{
					closest_fraction = fraction;
				}
			}

			evaluation = 10.0f;
			if (closest_fraction < 1.0f)
			{
				evaluation = square_root(closest_fraction) * 10.0f;
			}

			firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_hide_occluded);
		}
	}

	if (evaluation_context->directional_driving && actor->input.vehicle_index != NONE)
	{
		struct vehicle_datum *vehicle = vehicle_get(actor->input.vehicle_index);
		real_point3d vehicle_position;

		object_get_origin(actor->input.vehicle_index, &vehicle_position);

		for (index = 0, firing_position = firing_positions; index < firing_position_count; index++, firing_position++)
		{
			if (firing_position->valid)
			{
				real_vector3d vector_to_point;
				real distance_squared;

				vector_from_points3d(&vehicle_position, &firing_position->definition->position, &vector_to_point);
				distance_squared = magnitude_squared3d(&vector_to_point);

				if (!realcmp(distance_squared, 0.0f) && distance_squared < 900.0f)
				{
					real dot = dot_product3d(&vector_to_point, &vehicle->object.forward) / square_root(distance_squared);
					real evaluation;

					if ((evaluation_context->directional_driving_cannot_stop || magnitude_squared3d(&vehicle->object.translational_velocity) > (1.0f / 144.0f)) &&
						distance_squared < 64.0f &&
						dot < _sqrt2 / 2.f)
					{
						if (firing_position_reject_debug(evaluation_context, firing_position, _firing_position_evaluation_panic_approach_to_target))
						{
							firing_position->valid = FALSE;
							continue;
						}
					}

					evaluation = 15.0f;
					if (dot < 0.0f)
					{
						evaluation = 15.0f - dot * -15.0f;
					}
					else if (dot > _cosine30)
					{
						evaluation = (dot - _cosine30) * (15.0f / (1.0f - _cosine30)) + 15.0f;
					}

					firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_vehicle_direction);
				}
			}
		}
	}

	return;
}

static void pre_evaluator_pursuit(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	short firing_position_count,
	struct firing_position *firing_positions)
{
	short index;
	struct firing_position *firing_position;

	for (index = 0, firing_position = firing_positions; index < firing_position_count; index++, firing_position++)
	{
		if (firing_position->valid)
		{
			real evaluation = 0.0f;

			if (firing_position->path_distance_from_actor < evaluation_context->maximum_allowable_range / 2.0f)
			{
				evaluation = 5.0f;
			}
			else if (firing_position->path_distance_from_actor < evaluation_context->maximum_allowable_range)
			{
				evaluation = (evaluation_context->maximum_allowable_range - firing_position->path_distance_from_actor) * (1.0f / (evaluation_context->maximum_allowable_range / 2.0f)) * 5.0f;
			}

			firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_pursuit_distance_from_actor);

			if (evaluation_context->has_target)
			{
				if (firing_position->path_distance_to_target < 20.0f)
				{
					real evaluation = (20.0f - firing_position->path_distance_to_target) * 0.5f;

					firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_pursuit_distance_to_target);
				}

				if (evaluation_context->find_path_direction_from_target && evaluation_context->target_has_hint_vector)
				{
					real dot = dot_product3d(&firing_position->path_direction_from_target, &evaluation_context->target_hint_vector);
					real hint_evaluation;

					if (dot > _sqrt2 / 2.f)
					{
						hint_evaluation = 10.0f;
					}
					else
					{
						hint_evaluation = MAX(0.0f, dot * _sqrt2) * 10.0f;
					}

					firing_position_store_evaluation_debug(evaluation_context, firing_position, hint_evaluation, _firing_position_evaluation_pursuit_hint_direction);
				}
			}
		}
	}

	return;
}

static void pre_evaluator_panic(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	short firing_position_count,
	struct firing_position *firing_positions)
{
	short index;
	struct firing_position *firing_position;

	for (index = 0, firing_position = firing_positions; index < firing_position_count; index++, firing_position++)
	{
		if (firing_position->valid)
		{
			if (firing_position->path_distance_from_actor < 4.0f)
			{
				if (firing_position_reject_debug(evaluation_context, firing_position, _firing_position_evaluation_pursuit_unexamined))
				{
					firing_position->valid = FALSE;
					continue;
				}
			}
			else
			{
				real evaluation = 0.0f;

				if (firing_position->path_distance_from_actor < 8.0f)
				{
					evaluation = (firing_position->path_distance_from_actor - 4.0f) * 2.0f;
				}
				else if (firing_position->path_distance_from_actor < evaluation_context->maximum_allowable_range)
				{
					evaluation = (evaluation_context->maximum_allowable_range - firing_position->path_distance_from_actor) * 8.0f / (evaluation_context->maximum_allowable_range - 8.0f);
				}

				firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_panic_distance_from_actor);
			}

			if (evaluation_context->has_target)
			{
				real evaluation;

				if (firing_position->linear_distance_squared_to_target < 16.0f)
				{
					if (firing_position_reject_debug(evaluation_context, firing_position, _firing_position_evaluation_panic_distance_to_target))
					{
						firing_position->valid = FALSE;
						continue;
					}
				}

				if (firing_position->linear_distance_squared_to_target < 16.0f)
				{
					evaluation = 0.0f;
				}
				else if (firing_position->linear_distance_squared_to_target < 49.0f)
				{
					real distance = square_root(firing_position->linear_distance_squared_to_target);

					evaluation = (distance - 4.0f) * (10.0f / 3.0f);
				}
				else
				{
					evaluation = 10.0f;
				}

				firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_panic_distance_to_target);

				if (firing_position->path_closest_approach_to_target < REAL_MAX &&
					evaluation_context->target_current_distance > 0.0f &&
					evaluation_context->target_current_distance < REAL_MAX)
				{
					real fraction = 1.0f - firing_position->path_closest_approach_to_target / (evaluation_context->target_current_distance * 0.8f);
					real evaluation;

					if (fraction > 0.5f)
					{
						if (firing_position_reject_debug(evaluation_context, firing_position, _firing_position_evaluation_panic_approach_to_target))
						{
							firing_position->valid = FALSE;
							continue;
						}
					}

					evaluation = (1.0f - PIN(fraction, 0.0f, 1.0f)) * 8.0f;
					firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_panic_approach_to_target);
				}
			}
		}
	}

	return;
}

static void pre_evaluator_guard(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	short firing_position_count,
	struct firing_position *firing_positions)
{
	short index;
	struct firing_position *firing_position;

	for (index = 0, firing_position = firing_positions; index < firing_position_count; index++, firing_position++)
	{
		if (firing_position->valid)
		{
			real evaluation = 0.0f;

			if (firing_position->path_distance_from_actor < evaluation_context->maximum_allowable_range / 2.0f)
			{
				evaluation = 8.0f;
			}
			else if (firing_position->path_distance_from_actor < evaluation_context->maximum_allowable_range)
			{
				evaluation = (evaluation_context->maximum_allowable_range - firing_position->path_distance_from_actor) * (1.0f / (evaluation_context->maximum_allowable_range / 2.0f)) * 8.0f;
			}

			firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_pursuit_distance_from_actor);
		}
	}

	return;
}

static void pre_evaluator_attack(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	short firing_position_count,
	struct firing_position *firing_positions)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *actor_variant_definition = actor_combat_get_firing_variant_definition(actor_index);
	short index;
	struct firing_position *firing_position;

	for (index = 0, firing_position = firing_positions; index < firing_position_count; index++, firing_position++)
	{
		if (firing_position->valid)
		{
			if (evaluation_context->has_target)
			{
				real distance = square_root(firing_position->linear_distance_squared_to_target);

				if (actor_variant_definition->ranged_combat.maximum_firing_range > 0.0f)
				{
					real evaluation;

					if (distance < 0.8f * actor_variant_definition->ranged_combat.maximum_firing_range)
					{
						evaluation = 10.0f;
					}
					else
					{
						evaluation = 0.8f * actor_variant_definition->ranged_combat.maximum_firing_range / distance * 10.0f;
					}

					firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_attack_firing_range);
				}

				if (actor_variant_definition->ranged_combat.combat_range_upper_bound > 0.0f &&
					distance < actor_variant_definition->ranged_combat.combat_range_upper_bound)
				{
					real distance_inside_range;
					real range_lower_bound;
					real range_upper_bound;
					struct weapon_definition *weapon_definition;
					real evaluation;

					if (actor->emotions.berserk)
					{
						range_lower_bound = actor_variant_definition->ranged_combat.berserk_firing_range_lower_bound;
						range_upper_bound = actor_variant_definition->ranged_combat.berserk_firing_range_upper_bound;
					}
					else
					{
						range_lower_bound = actor_variant_definition->ranged_combat.combat_range_lower_bound;
						range_upper_bound = actor_variant_definition->ranged_combat.combat_range_upper_bound;
					}

					range_lower_bound = MAX(range_lower_bound, evaluation_context->target_danger_radius);

					weapon_definition = actor_get_weapon_definition(actor_index);
					if (weapon_definition && weapon_definition->weapon.ai_minimum_target_range > 0.0f)
					{
						range_lower_bound = MAX(range_lower_bound, weapon_definition->weapon.ai_minimum_target_range);
					}

					distance_inside_range = range_upper_bound - distance;
					if (range_lower_bound > 0.0f)
					{
						distance_inside_range = MIN(distance_inside_range, distance - range_lower_bound);
					}

					evaluation = 0.0f;
					if (distance_inside_range > 2.0f)
					{
						evaluation = 20.0f;
					}
					else if (distance_inside_range > 0.0f)
					{
						evaluation = 20.0f * (distance_inside_range * 0.5f);
					}

					firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_attack_combat_range);
				}
			}

			if (evaluation_context->dangerous_enemy_attack_vector_count > 0)
			{
				real closest_distance_squared = REAL_MAX;
				short vector_index;

				for (vector_index = 0; vector_index < evaluation_context->attack_vector_count; vector_index++)
				{
					if (evaluation_context->attack_vectors[vector_index].type == _firing_position_attack_vector_dangerous_enemy)
					{
						real_vector3d vector_to_firing_position;
						real dot;

						match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 657, &evaluation_context->attack_vectors[vector_index].vector);

						vector_from_points3d(&evaluation_context->attack_vectors[vector_index].point, &firing_position->definition->position, &vector_to_firing_position);
						dot = dot_product3d(&vector_to_firing_position, &evaluation_context->attack_vectors[vector_index].vector);
						if (dot > 0.0f)
						{
							real distance_squared;

							vector_from_line3d(&vector_to_firing_position, &evaluation_context->attack_vectors[vector_index].vector, -dot, &vector_to_firing_position);
							distance_squared = magnitude_squared3d(&vector_to_firing_position);
							if (distance_squared < closest_distance_squared)
							{
								closest_distance_squared = distance_squared;
							}
						}
					}
				}

				{
					real evaluation = 6.0f;

					if (closest_distance_squared < 12.25f)
					{
						real distance = square_root(closest_distance_squared);

						evaluation = distance * 0.25f;
					}

					firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_attack_no_line_of_sight);
				}
			}
		}
	}

	return;
}

static void pre_evaluator_combatmove(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	short firing_position_count,
	struct firing_position *firing_positions)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *actor_variant_definition = actor_combat_get_firing_variant_definition(actor_index);
	real weight = 8.0f;

	if (evaluation_context->has_target)
	{
		if (evaluation_context->target_current_distance > actor_variant_definition->ranged_combat.maximum_firing_range)
		{
			weight = 0.0f;
		}
		else
		{
			weight = (1.0f - evaluation_context->target_current_distance / actor_variant_definition->ranged_combat.maximum_firing_range) * 8.0f;
		}
	}

	if (weight > 0.0f)
	{
		short index;
		struct firing_position *firing_position;

		for (index = 0, firing_position = firing_positions; index < firing_position_count; index++, firing_position++)
		{
			if (firing_position->valid && firing_position->path_distance_from_actor < evaluation_context->maximum_allowable_range)
			{
				real fraction = firing_position->path_distance_from_actor / evaluation_context->maximum_allowable_range;
				real evaluation = weight * MAX(0.0f, 1.0f - fraction);

				firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_panic_distance_from_actor);
			}
		}
	}

	if (evaluation_context->friend_attack_vector_count > 0)
	{
		short index;
		struct firing_position *firing_position;

		for (index = 0, firing_position = firing_positions; index < firing_position_count; index++, firing_position++)
		{
			short player_blockage = 0;
			short friend_blockage = 0;

			if (firing_position->valid)
			{
				short vector_index;

				for (vector_index = 0; vector_index < evaluation_context->attack_vector_count; vector_index++)
				{
					if (evaluation_context->attack_vectors[vector_index].type == _firing_position_attack_vector_friend || evaluation_context->attack_vectors[vector_index].type == _firing_position_attack_vector_friend_player)
					{
						short blockage = actor_perception_aiming_vector_test_blockage(&evaluation_context->attack_vectors[vector_index].point, &evaluation_context->attack_vectors[vector_index].vector, &firing_position->definition->position, NULL);

						if (evaluation_context->attack_vectors[vector_index].type == _firing_position_attack_vector_friend)
						{
							friend_blockage = MAX(friend_blockage, blockage);
						}
						else if (evaluation_context->attack_vectors[vector_index].type == _firing_position_attack_vector_friend_player)
						{
							player_blockage = MAX(player_blockage, blockage);
						}
					}
				}

				{
					real evaluation;

					if (player_blockage >= 2)
					{
						evaluation = 0.0f;
					}
					else if (player_blockage >= 1)
					{
						evaluation = 1.5f;
					}
					else if (friend_blockage >= 2)
					{
						evaluation = 6.0f;
					}
					else if (friend_blockage >= 1)
					{
						evaluation = 8.5f;
					}
					else
					{
						evaluation = 10.0f;
					}

					firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_panic_distance_to_target);
				}
			}
		}
	}

	return;
}

static void firing_position_compute_line_of_sight(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	struct firing_position *firing_position)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 828, evaluation_context->has_target);

	if (evaluation_context->evaluation_mode == _firing_point_evaluation_mode_pursue)
	{
		if (firing_position->path_distance_from_actor < 6.0f)
		{
			real_point3d estimated_target_head_position;

			unit_estimate_position(actor->meta.unit_index, _unit_estimate_head_standing, &firing_position->definition->position, NULL, NULL, &estimated_target_head_position);
			firing_position->line_of_sight = ai_test_line_of_sight(
				&actor->input.position.head_position,
				actor->input.position.body_location.cluster_index,
				&estimated_target_head_position,
				firing_position->definition->cluster_index,
				0,
				FALSE,
				NONE,
				actor->input.vehicle_index != NONE);
		}
		else
		{
			firing_position->line_of_sight = _ai_line_of_sight_obstructed;
		}
	}
	else
	{
		real_point3d line_of_sight_origin_at_firing_position;
		real_vector3d computed_facing;
		real_vector3d *facing;
		real_vector3d *gun_offset;
		short line_of_sight_mode = 0;
		short estimate_mode;

		if (evaluation_context->evaluation_mode == _firing_point_evaluation_mode_panic ||
			evaluation_context->evaluation_mode == _firing_point_evaluation_mode_cover)
		{
			estimate_mode = _unit_estimate_head_crouching;
			facing = NULL;
			gun_offset = NULL;
		}
		else if (evaluation_context->has_gun_offset_stand)
		{
			estimate_mode = _unit_estimate_gun_position;
			gun_offset = &evaluation_context->gun_offset_stand;

			vector_from_points3d(&firing_position->definition->position, &evaluation_context->target_head_position, &computed_facing);
			if (normalize2d((real_vector2d *)&computed_facing) > 0.0f)
			{
				computed_facing.k = 0.0f;
				facing = &computed_facing;
			}
			else
			{
				facing = &actor->input.facing_vector;
			}
		}
		else
		{
			estimate_mode = _unit_estimate_head_standing;
			facing = NULL;
			gun_offset = NULL;
		}

		unit_estimate_position(actor->meta.unit_index, estimate_mode, &firing_position->definition->position, facing, gun_offset, &line_of_sight_origin_at_firing_position);

		switch (evaluation_context->evaluation_mode)
		{
		case _firing_point_evaluation_mode_panic:
		case _firing_point_evaluation_mode_cover:
		case _firing_point_evaluation_mode_uncover:
			line_of_sight_mode = 1;
			break;
		}

		firing_position->line_of_sight = ai_test_line_of_sight(
			&line_of_sight_origin_at_firing_position,
			firing_position->definition->cluster_index,
			&evaluation_context->target_line_of_sight_position,
			evaluation_context->target_cluster_index,
			line_of_sight_mode,
			TRUE,
			evaluation_context->target_vehicle_index,
			actor->input.vehicle_index != NONE);
	}

	return;
}

static boolean post_evaluator_global(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	struct firing_position *firing_position)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	if (evaluation_context->flying)
	{
		if (!firing_position)
		{
			evaluation_context->post_evaluation_bound += 15.0f;
		}
		else
		{
			real avoidance_distance = 0.0f;

			boolean path_available = actor_path_3d_available(actor_index, &firing_position->definition->position, &avoidance_distance);

			if (path_available)
			{
				path_available = path_3d_available(global_structure_bsp_get(), &actor->input.position.body_position, avoidance_distance, &firing_position->definition->position, NULL, NULL);
			}

			if (path_available)
			{
				firing_position_store_evaluation_debug(evaluation_context, firing_position, 15.0f, _firing_position_evaluation_path_available);
			}
			else if (firing_position_reject_debug(evaluation_context, firing_position, _firing_position_evaluation_no_path_available))
			{
				firing_position->valid = FALSE;
			}
		}
	}

	return !firing_position ? TRUE : firing_position->valid;
}

static boolean post_evaluator_pursuit(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	struct firing_position *firing_position)
{
	struct actor_datum *actor = actor_get(actor_index);
	long current_time = game_time_get();
	long position_last_examined_time = NONE;
	short actor_count = 0;

	if (firing_position)
	{
		boolean unexamined = TRUE;
		short rejection_type = NONE;

		if (firing_position->line_of_sight == _ai_line_of_sight_clear && firing_position->path_distance_from_actor < 6.0f)
		{
			encounter_mark_examined_pursuit_position(actor->meta.encounter_index, actor_index, firing_position->original_index, evaluation_context->evaluation_data.pursue.last_perceived_time);
			position_last_examined_time = current_time;
			actor_count = 7;
			unexamined = FALSE;
			rejection_type = _firing_position_evaluation_pursuit_examined_by_self;
		}
		else if (encounter_pursuit_position_already_examined(actor->meta.encounter_index, actor_index, firing_position->original_index, evaluation_context->evaluation_data.pursue.last_perceived_time, &actor_count, &position_last_examined_time))
		{
			unexamined = FALSE;
			rejection_type = _firing_position_evaluation_pursuit_examined;
		}

		if (evaluation_context->evaluation_data.pursue.tenacious)
		{
			if (unexamined)
			{
				real evaluation = 15.0f;

				firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_pursuit_unexamined);
			}
		}
		else if (!unexamined)
		{
			if (firing_position_reject_debug(evaluation_context, firing_position, rejection_type))
			{
				firing_position->valid = FALSE;
			}
		}

		if (firing_position->valid)
		{
			real evaluation = 0.0f;
			real actor_evaluation;

			if (position_last_examined_time == NONE || position_last_examined_time + 300 < current_time)
			{
				evaluation = 10.0f;
			}
			else if (position_last_examined_time < current_time)
			{
				evaluation = (current_time - position_last_examined_time) * (10.0f / 300.0f);
			}
			firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_pursuit_examined);

			actor_evaluation = 0.0f;
			if (actor_count < 4)
			{
				actor_evaluation = (4 - actor_count) * 5.0f;
			}
			firing_position_store_evaluation_debug(evaluation_context, firing_position, actor_evaluation, _firing_position_evaluation_pursuit_actor_count);
		}
	}

	return !firing_position ? FALSE : firing_position->valid;
}

static boolean post_evaluator_hide(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	struct firing_position *firing_position)
{
	if (evaluation_context->has_target)
	{
		if (!firing_position)
		{
			evaluation_context->post_evaluation_bound += 12.0f;
		}
		else
		{
			real evaluation = 0.0f;

			switch (firing_position->line_of_sight)
			{
			case _ai_line_of_sight_from_cover:
				evaluation = 12.0f;
				break;
			case _ai_line_of_sight_obstructed:
				evaluation = 10.0f;
				break;
			case _ai_line_of_sight_occluded:
				if (evaluation_context->evaluation_data.cover.allow_occluded_points)
				{
					evaluation = 6.0f;
				}
				else if (firing_position_reject_debug(evaluation_context, firing_position, _firing_position_evaluation_hide_occluded))
				{
					firing_position->valid = FALSE;
				}
				break;
			case _ai_line_of_sight_to_cover:
				evaluation = 4.0f;
				break;
			case _ai_line_of_sight_clear:
				if (firing_position_reject_debug(evaluation_context, firing_position, _firing_position_evaluation_hide_clear))
				{
					firing_position->valid = FALSE;
				}
				break;
			default:
				match_halt("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 1117);
				break;
			}

			firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_hide_line_of_sight);
		}
	}

	return !firing_position ? TRUE : firing_position->valid;
}

static boolean post_evaluator_uncover(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	struct firing_position *firing_position)
{
	if (evaluation_context->has_target)
	{
		if (!firing_position)
		{
			evaluation_context->post_evaluation_bound += 20.0f;
		}
		else
		{
			real evaluation = 0.0f;

			switch (firing_position->line_of_sight)
			{
			case _ai_line_of_sight_clear:
				evaluation = 20.0f;
				break;
			case _ai_line_of_sight_occluded:
				evaluation = 10.0f;
				break;
			default:
				{
					real distance = evaluation_context->target_current_distance - 7.5f;

					if (distance < 0.0f || firing_position->linear_distance_squared_to_target > distance * distance)
					{
						if (firing_position_reject_debug(evaluation_context, firing_position, _firing_position_evaluation_uncover_too_far))
						{
							firing_position->valid = FALSE;
						}
					}
				}
				break;
			}

			firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_uncover_line_of_sight);
		}
	}

	return !firing_position ? TRUE : firing_position->valid;
}

static boolean post_evaluator_attack(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	struct firing_position *firing_position)
{
	if (evaluation_context->has_target)
	{
		if (!firing_position)
		{
			evaluation_context->post_evaluation_bound += evaluation_context->target_line_of_sight_optional ? 6.0f : 15.0f;
		}
		else
		{
			real evaluation = 0.0f;

			switch (firing_position->line_of_sight)
			{
			case _ai_line_of_sight_clear:
				evaluation = evaluation_context->target_line_of_sight_optional ? 6.0f : 15.0f;
				break;
			case _ai_line_of_sight_occluded:
				evaluation = evaluation_context->target_line_of_sight_optional ? 2.5f : 5.0f;
				break;
			default:
				if (!evaluation_context->target_line_of_sight_optional)
				{
					if (firing_position_reject_debug(evaluation_context, firing_position, _firing_position_evaluation_attack_no_line_of_sight))
					{
						firing_position->valid = FALSE;
					}
				}
				break;
			}

			firing_position_store_evaluation_debug(evaluation_context, firing_position, evaluation, _firing_position_evaluation_attack_line_of_sight);
		}
	}

	return !firing_position ? TRUE : firing_position->valid;
}

static void firing_position_pre_evaluate(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	short firing_position_count,
	struct firing_position *firing_positions)
{
	struct pre_evaluator_table_entry const *entry;

	for (entry = global_pre_evaluator_table; entry->evaluator; entry++)
	{
		if (TEST_FLAG(entry->evaluation_mode_flags, evaluation_context->evaluation_mode))
		{
			entry->evaluator(actor_index, evaluation_context, firing_position_count, firing_positions);
		}
	}

	return;
}

static boolean firing_position_post_evaluate(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	struct firing_position *firing_position)
{
	struct post_evaluator_table_entry const *entry;
	boolean valid = TRUE;

	for (entry = global_post_evaluator_table; valid && entry->evaluator; entry++)
	{
		if (TEST_FLAG(entry->evaluation_mode_flags, evaluation_context->evaluation_mode))
		{
			valid = entry->evaluator(actor_index, evaluation_context, firing_position);
		}
	}

	match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 1262, firing_position->valid == valid);

	return valid;
}

static boolean firing_positions_get_post_evaluation_bound(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context)
{
	struct post_evaluator_table_entry const *entry;
	boolean valid = TRUE;

	evaluation_context->post_evaluation_bound = 0.0f;
	for (entry = global_post_evaluator_table; valid && entry->evaluator; entry++)
	{
		if (TEST_FLAG(entry->evaluation_mode_flags, evaluation_context->evaluation_mode))
		{
			valid = entry->evaluator(actor_index, evaluation_context, NULL);
		}
	}

	return valid;
}

static boolean firing_position_compare(
	long index1,
	long index2)
{
	struct firing_position const *firing_position1 = &global_temporary_sort_firing_position_array[index1];
	struct firing_position const *firing_position2 = &global_temporary_sort_firing_position_array[index2];

	match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 1295, global_temporary_sort_firing_position_array);
	match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 1296, (index1 >= 0) && (index1 < global_temporary_sort_firing_position_count));
	match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 1297, (index2 >= 0) && (index2 < global_temporary_sort_firing_position_count));

	if (firing_position1->valid != firing_position2->valid)
	{
		return (firing_position1->valid ? -1 : 1) > 0;
	}
	else if (firing_position1->rejected != firing_position2->rejected)
	{
		return (firing_position1->rejected ? 1 : -1) > 0;
	}
	else if (firing_position1->evaluation > firing_position2->evaluation)
	{
		return FALSE;
	}
	else if (firing_position1->evaluation < firing_position2->evaluation)
	{
		return TRUE;
	}

	return FALSE;
}

static boolean firing_position_forced_evaluation(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	struct firing_position *firing_position)
{
	firing_position->evaluation = 0.0f;
	firing_position->pre_evaluation = 0.0f;
	firing_position->valid = TRUE;
	firing_position->rejected = FALSE;

	firing_position_pre_evaluate(actor_index, evaluation_context, 1, firing_position);

	if (firing_position->valid)
	{
		evaluation_context->valid_count++;
	}
	if (!firing_position->rejected)
	{
		evaluation_context->nonrejected_count++;
	}

	if (firing_position->valid)
	{
		if (evaluation_context->has_target)
		{
			firing_position_compute_line_of_sight(actor_index, evaluation_context, firing_position);
		}

		firing_position->pre_evaluation = firing_position->evaluation;
		firing_position->valid = firing_position_post_evaluate(actor_index, evaluation_context, firing_position);
		evaluation_context->post_evaluation_count++;
	}

	return firing_position->valid;
}

unsigned long actor_get_firing_position_group(
	long actor_index,
	short evaluation_mode,
	short group_selection_mode)
{
	struct actor_datum *actor = actor_get(actor_index);
	unsigned long result = 0;

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
		boolean searching = actor->state.searching;
		short index;

		switch (group_selection_mode)
		{
		case _firing_position_group_when_searching:
			searching = TRUE;
			break;
		case _firing_position_group_when_not_searching:
			searching = FALSE;
			break;
		}

		if (evaluation_mode == _firing_point_evaluation_mode_panic)
		{
			index = _firing_position_group_defending_guard;
		}
		else if (evaluation_mode == _firing_point_evaluation_mode_guard)
		{
			index = actor->emotions.currently_defending ? _firing_position_group_defending_guard : _firing_position_group_attacking_guard;
		}
		else if (evaluation_mode == _firing_point_evaluation_mode_pursue)
		{
			index = _firing_position_group_pursuing;
		}
		else if (actor->emotions.currently_defending)
		{
			index = searching ? _firing_position_group_defending_search : _firing_position_group_defending;
		}
		else
		{
			index = searching ? _firing_position_group_attacking_search : _firing_position_group_attacking;
		}

		match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 1412, (index >= 0) && (index < NUMBER_OF_FIRING_POSITION_GROUPS));
		result = squad_definition->firing_position_groups[index];
	}

	return result;
}

boolean actor_nearby_firing_positions(
	long actor_index,
	real_point3d *test_point,
	long test_surface_index,
	short group_selection_mode)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean result = FALSE;

	if (!actor->state.flying && test_surface_index != NONE)
	{
		struct collision_bsp *collision_bsp = global_collision_bsp_get();

		match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 1434, (test_surface_index >= 0) && (test_surface_index < collision_bsp->surfaces.count));
	}

	if ((actor->state.flying || test_surface_index != NONE) && actor->meta.encounter_index != NONE)
	{
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
			struct encounter_definition);
		struct path_state nearby_path_state;
		unsigned long firing_position_group = actor_get_firing_position_group(actor_index, _firing_point_evaluation_mode_fight, group_selection_mode);
		short index;

		if (!actor->state.flying)
		{
			struct path_input input;

			path_input_new(&input, actor_definition->moving.pathfinding_radius, TRUE, NONE);
			path_input_set_start(&input, test_point, test_surface_index);
			path_input_set_search_bounds(&input, 4.0f);
			path_state_new(&input, &nearby_path_state, NULL);
			path_state_find(&nearby_path_state);
		}

		for (index = 0; index < encounter_definition->firing_positions.count; index++)
		{
			struct firing_position_definition *firing_position_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->firing_positions,
				index,
				struct firing_position_definition);

			if (TEST_FLAG(firing_position_group, firing_position_definition->group_index) &&
				distance_squared3d(test_point, &firing_position_definition->position) < 16.0f)
			{
				if (actor->state.flying)
				{
					if (path_3d_available(global_structure_bsp_get(), test_point, 0.0f, &firing_position_definition->position, NULL, NULL))
					{
						result = TRUE;
						break;
					}
				}
				else
				{
					real path_distance;

					path_state_estimated_distance(&nearby_path_state, &firing_position_definition->position, firing_position_definition->surface_index, &path_distance, NULL, NULL);
					if (path_distance < 4.0f)
					{
						result = TRUE;
						break;
					}
				}
			}
		}
	}

	return result;
}

short actor_select_firing_position(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	struct firing_position *best_firing_position,
	long *current_owner,
	struct path_state *area_path_state,
	boolean *area_path_state_valid)
{
	struct actor_datum *actor = actor_get(actor_index);
	short best_firing_position_index = NONE;
	real best_evaluation = 0.0f;
	boolean debug = FALSE;

#ifdef DEBUG
	if (actor->meta.encounter_index == ai_debug.selected_encounter_index &&
		(ai_debug.selected_actor_index == NONE || ai_debug.selected_actor_index == actor_index))
	{
		if (ai_debug.render_evaluations && evaluation_context->evaluation_mode != _firing_point_evaluation_mode_pursue)
		{
			debug = TRUE;
		}
		if (ai_debug.render_pursuit && evaluation_context->evaluation_mode == _firing_point_evaluation_mode_pursue)
		{
			debug = TRUE;
		}
	}
#endif

	if (actor->meta.encounter_index != NONE)
	{
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
		struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
			struct encounter_definition);
		long firing_position_owner_actor_indices[MAXIMUM_FIRING_POSITIONS_PER_ENCOUNTER];
		struct firing_position firing_positions[MAXIMUM_FIRING_POSITIONS_PER_ENCOUNTER];
		short firing_position_count = 0;
		boolean any_position_in_range = FALSE;

		match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 1552, !actor->input.vehicle_passenger);

#ifdef DEBUG
		ai_profile.meters[_ai_meter_firing_point_eval].current_count++;
#endif

		encounter_build_firing_position_owner_actor_indices(actor->meta.encounter_index, firing_position_owner_actor_indices);
		if (actor->firing_positions.current_position_index != NONE)
		{
			firing_position_owner_actor_indices[actor->firing_positions.current_position_index] = NONE;
		}

		evaluation_context->maximum_allowable_range = actor->input.vehicle_driver_type == _actor_vehicle_driver_none ? 15.0f : 80.0f;
		if (evaluation_context->maximum_search_range == 0.0f)
		{
			evaluation_context->maximum_search_range = evaluation_context->maximum_allowable_range;
		}

		evaluation_context->find_path_distance_to_target = evaluation_context->evaluation_mode == _firing_point_evaluation_mode_pursue;
		evaluation_context->has_target = FALSE;

		if (evaluation_context->specific_target_enable)
		{
			evaluation_context->target_point = evaluation_context->specific_target_point;
			evaluation_context->target_pathfinding_point = evaluation_context->specific_target_point;
			evaluation_context->has_target = TRUE;
			evaluation_context->target_pathfinding_surface_index = evaluation_context->specific_target_surface_index;
			evaluation_context->target_cluster_index = evaluation_context->specific_target_cluster_index;
			evaluation_context->target_current_distance = distance3d(&actor->input.position.body_position, &evaluation_context->target_point);
			evaluation_context->target_prop_index = NONE;
			evaluation_context->target_vehicle_index = NONE;
			evaluation_context->target_danger_radius = 0.0f;
			unit_estimate_position(actor->meta.unit_index, _unit_estimate_head_standing, &evaluation_context->target_point, NULL, NULL, &evaluation_context->target_head_position);
			evaluation_context->target_line_of_sight_position = evaluation_context->target_head_position;
		}
		else
		{
			long prop_index = NONE;

			if (actor->state.action == _actor_action_flee && actor->state.action_data.flee.flee_prop_index != NONE)
			{
				prop_index = actor->state.action_data.flee.flee_prop_index;
			}
			else if (actor->target.target_prop_index != NONE)
			{
				prop_index = actor->target.target_prop_index;
			}

			if (prop_index != NONE)
			{
				struct prop_datum *prop = prop_get(prop_index);

				if (evaluation_context->find_path_distance_to_target &&
					prop_acknowledged(prop))
				{
					actor_perception_find_prop_pathfinding_location(actor_index, prop_index);
				}

				evaluation_context->has_target = TRUE;
				evaluation_context->target_point = prop->body_position;
				evaluation_context->target_pathfinding_point = prop->pathfinding_point;
				evaluation_context->target_pathfinding_surface_index = prop->pathfinding_surface_index;
				evaluation_context->target_cluster_index = prop->body_location.cluster_index;
				evaluation_context->target_current_distance = prop->distance;
				evaluation_context->target_prop_index = prop_index;
				evaluation_context->target_head_position = prop->head_position;
				evaluation_context->target_vehicle_index = prop->vehicle_index;
				evaluation_context->target_danger_radius = prop->suicide_radius;

				if (evaluation_context->use_last_visible_target_position && prop->last_visible_time != NONE)
				{
					evaluation_context->target_line_of_sight_position = prop->last_visible_head_position;
				}
				else
				{
					evaluation_context->target_line_of_sight_position = prop->head_position;
				}

				if (prop_orphaned(prop))
				{
					evaluation_context->target_has_hint_vector = TRUE;
					evaluation_context->target_hint_vector = prop->orphan_hint_vector;
				}

				evaluation_context->target_line_of_sight_optional =
					evaluation_context->evaluation_mode == _firing_point_evaluation_mode_guard ||
					evaluation_context->evaluation_mode == _firing_point_evaluation_mode_avoid;
			}
		}

		if (magnitude_squared3d(&actor_variant_definition->ranged_combat.gun_offset_stand) > 0.0001f)
		{
			evaluation_context->has_gun_offset_stand = TRUE;
			evaluation_context->gun_offset_stand = actor_variant_definition->ranged_combat.gun_offset_stand;
		}
		else if (magnitude_squared3d(&actor_definition->perception.gun_offset_stand) > 0.0001f)
		{
			evaluation_context->has_gun_offset_stand = TRUE;
			evaluation_context->gun_offset_stand = actor_definition->perception.gun_offset_stand;
		}
		else
		{
			evaluation_context->has_gun_offset_stand = FALSE;
		}

		if (magnitude_squared3d(&actor_variant_definition->ranged_combat.gun_offset_crouch) > 0.0001f)
		{
			evaluation_context->has_gun_offset_crouch = TRUE;
			evaluation_context->gun_offset_crouch = actor_variant_definition->ranged_combat.gun_offset_crouch;
		}
		else if (magnitude_squared3d(&actor_definition->perception.gun_offset_crouch) > 0.0001f)
		{
			evaluation_context->has_gun_offset_crouch = TRUE;
			evaluation_context->gun_offset_crouch = actor_definition->perception.gun_offset_crouch;
		}
		else
		{
			evaluation_context->has_gun_offset_crouch = FALSE;
		}

		if (actor->danger_zone.danger_type > _actor_danger_zone_none &&
			actor->danger_zone.noticed_danger &&
			actor->danger_zone.bounding_sphere_radius + 3.0f > actor->danger_zone.current_distance_from_actor)
		{
			evaluation_context->find_path_direction_from_actor = TRUE;
		}

		evaluation_context->flying = actor->state.flying;
		if (actor->input.vehicle_driver_type == _actor_vehicle_driver_directional_flying)
		{
			evaluation_context->directional_driving = TRUE;
			evaluation_context->directional_driving_cannot_stop = TRUE;
		}

		evaluation_context->avoid_point_count = 0;
		if (actor->firing_positions.last_discarded_firing_position_valid)
		{
			evaluation_context->avoid_point[evaluation_context->avoid_point_count].point = actor->firing_positions.last_discarded_firing_position;
			evaluation_context->avoid_point[evaluation_context->avoid_point_count++].radius = actor_definition->firing_position.old_avoidance_radius;
		}

		if (actor_definition->firing_position.friend_avoidance_radius > 0.0f)
		{
			switch (evaluation_context->evaluation_mode)
			{
			case _firing_point_evaluation_mode_fight:
			case _firing_point_evaluation_mode_uncover:
			case _firing_point_evaluation_mode_avoid:
				{
					struct prop_iterator iterator;
					struct prop_datum *prop;

					prop_iterator_new(&iterator, actor_index);
					while (evaluation_context->avoid_point_count < MAXIMUM_FIRING_POSITION_AVOID_POINTS && (prop = prop_iterator_next(&iterator)))
					{
						if (prop_acknowledged(prop) &&
							!prop->enemy && !prop->dead && !prop->player)
						{
							evaluation_context->avoid_point[evaluation_context->avoid_point_count].point = prop->body_position;
							evaluation_context->avoid_point[evaluation_context->avoid_point_count++].radius = actor_definition->firing_position.friend_avoidance_radius;
						}
					}
				}
				break;
			}
		}

		evaluation_context->attack_vector_count = 0;
		evaluation_context->friend_attack_vector_count = 0;
		evaluation_context->dangerous_enemy_attack_vector_count = 0;

		{
			boolean avoid_attack_vectors = FALSE;

			if (TEST_FLAG(actor_definition->flags, _actor_definition_avoid_friend_line_of_fire_bit) && actor->state.combat_status >= _actor_combat_status_definite)
			{
				if (actor->situation.area_friends > 0)
				{
					avoid_attack_vectors = TRUE;
				}
			}
			if (TEST_FLAG(actor_definition->flags2, _actor_definition_flags2_avoid_all_enemy_attack_vectors_bit) && actor->state.combat_status >= _actor_combat_status_definite)
			{
				avoid_attack_vectors = TRUE;
			}

			switch (evaluation_context->evaluation_mode)
			{
			case _firing_point_evaluation_mode_fight:
			case _firing_point_evaluation_mode_cover:
			case _firing_point_evaluation_mode_uncover:
			case _firing_point_evaluation_mode_avoid:
				if (avoid_attack_vectors)
				{
					struct prop_iterator iterator;
					struct prop_datum *prop;

					prop_iterator_new(&iterator, actor_index);
					while (evaluation_context->attack_vector_count < MAXIMUM_FIRING_POSITION_ATTACK_VECTORS && (prop = prop_iterator_next(&iterator)))
					{
						if (prop_acknowledged(prop) && !prop->dead)
						{
							real_vector3d attack_vector;

							if (!prop->enemy &&
								(prop->player || prop->vehicle_index == NONE) &&
								actor_perception_friend_prop_is_attacking(actor_index, iterator.index, &attack_vector))
							{
								evaluation_context->attack_vectors[evaluation_context->attack_vector_count].type = prop->player ? _firing_position_attack_vector_friend_player : _firing_position_attack_vector_friend;
								evaluation_context->attack_vectors[evaluation_context->attack_vector_count].point = prop->body_position;
								evaluation_context->attack_vectors[evaluation_context->attack_vector_count++].vector = attack_vector;
								evaluation_context->friend_attack_vector_count++;
							}

							if (prop->enemy)
							{
								boolean avoid_enemy_attack_vector = TEST_FLAG(actor_definition->flags2, _actor_definition_flags2_avoid_all_enemy_attack_vectors_bit);

								if (prop->unopposable_enemy)
								{
									if (prop->player)
									{
										avoid_enemy_attack_vector = TRUE;
									}
									else if (prop->shooting)
									{
										avoid_enemy_attack_vector = TRUE;
									}
								}

								if (avoid_enemy_attack_vector)
								{
									long unit_index = prop->vehicle_index != NONE ? prop->vehicle_index : prop->unit_index;

									evaluation_context->attack_vectors[evaluation_context->attack_vector_count].type = _firing_position_attack_vector_dangerous_enemy;
									evaluation_context->attack_vectors[evaluation_context->attack_vector_count].point = prop->body_position;
									unit_get_aiming_vector(unit_index, &evaluation_context->attack_vectors[evaluation_context->attack_vector_count].vector);
									evaluation_context->attack_vector_count++;
									evaluation_context->dangerous_enemy_attack_vector_count++;
								}
							}
						}
					}
				}
				break;
			}
		}

		evaluation_context->skipped_count = 0;
		evaluation_context->post_evaluation_count = 0;
		evaluation_context->nonrejected_count = 0;
		evaluation_context->valid_count = 0;
		evaluation_context->consider_count = 0;
		evaluation_context->encounter_count = 0;

		if (debug)
		{
#ifdef DEBUG
			ai_debug.firing_position_context_valid = TRUE;
			ai_debug.firing_position_context = *evaluation_context;
#endif
		}

		match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 1830, area_path_state_valid);
		*area_path_state_valid = FALSE;

		{
			short index;
			boolean too_many_firing_positions_reported = FALSE;

			for (index = 0; index < encounter_definition->firing_positions.count; index++)
			{
				struct firing_position_definition *firing_position_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter_definition->firing_positions,
					index,
					struct firing_position_definition);

				if (debug)
				{
#ifdef DEBUG
					ai_debug.firing_positions[index].evaluated = FALSE;
#endif
				}

				if (!TEST_FLAG(evaluation_context->allowed_position_mask, firing_position_definition->group_index))
				{
					continue;
				}

#ifdef DEBUG
				if (actor->meta.encounter_index == ai_debug.selected_encounter_index)
				{
					ai_debug.firing_positions[index].pursuit_position = evaluation_context->evaluation_mode == _firing_point_evaluation_mode_pursue;
				}
#endif

				if (!evaluation_context->flying && firing_position_definition->surface_index == NONE)
				{
					continue;
				}

				if (evaluation_context->evaluation_mode == _firing_point_evaluation_mode_pursue &&
					!actor_nearby_firing_positions(actor_index, &firing_position_definition->position, firing_position_definition->surface_index, _firing_position_group_when_searching))
				{
					continue;
				}

				if (firing_position_owner_actor_indices[index] != NONE)
				{
					struct actor_datum *owner_actor;
					real owner_distance;
					real distance;

					if (evaluation_context->evaluation_mode == _firing_point_evaluation_mode_guard &&
						evaluation_context->avoid_point_count < MAXIMUM_FIRING_POSITION_AVOID_POINTS)
					{
						evaluation_context->avoid_point[evaluation_context->avoid_point_count].point = firing_position_definition->position;
						evaluation_context->avoid_point[evaluation_context->avoid_point_count++].radius = 4.0f;
					}

					owner_actor = actor_get(firing_position_owner_actor_indices[index]);
					owner_distance = distance3d(&owner_actor->input.position.body_position, &firing_position_definition->position);
					distance = distance3d(&actor->input.position.body_position, &firing_position_definition->position);
					if (owner_distance < 1.0f || owner_distance < 2.0f * distance)
					{
						continue;
					}
				}

				if (firing_position_count < MAXIMUM_FIRING_POSITIONS_PER_ENCOUNTER)
				{
					firing_positions[firing_position_count].original_index = index;
					firing_positions[firing_position_count].definition = firing_position_definition;
					firing_positions[firing_position_count].line_of_sight = _ai_line_of_sight_clear;
					firing_positions[firing_position_count].path_distance_from_actor = REAL_MAX;
					firing_positions[firing_position_count].path_direction_from_actor = *global_zero_vector3d;
					firing_positions[firing_position_count].path_distance_to_target = REAL_MAX;
					firing_positions[firing_position_count].path_closest_approach_to_target = REAL_MAX;
					firing_positions[firing_position_count].path_direction_from_target = *global_zero_vector3d;
					firing_positions[firing_position_count].linear_distance_squared_to_target = 0.0f;
					firing_positions[firing_position_count].pre_evaluation = 0.0f;
					firing_positions[firing_position_count].evaluation = 0.0f;
					firing_positions[firing_position_count].valid = TRUE;
					firing_positions[firing_position_count].rejected = FALSE;
					firing_position_count++;
				}
				else if (!too_many_firing_positions_reported)
				{
					error(_error_silent, "encounter %s has too many firing positions (%d > %d)",
						encounter_definition->name,
						encounter_definition->firing_positions.count,
						MAXIMUM_FIRING_POSITIONS_PER_ENCOUNTER);
					too_many_firing_positions_reported = TRUE;
				}
			}
		}

		evaluation_context->encounter_count = (short)encounter_definition->firing_positions.count;
		evaluation_context->consider_count = firing_position_count;

		if (firing_position_count == 0)
		{
			actor_clear_discarded_firing_positions(actor_index, FALSE);
		}
		else
		{
			if (evaluation_context->has_target && evaluation_context->find_path_distance_to_target)
			{
				if (evaluation_context->flying)
				{
					struct structure_bsp *structure_bsp = global_structure_bsp_get();
					short index;

					for (index = 0; index < firing_position_count; index++)
					{
						struct firing_position *firing_position = &firing_positions[index];
						real_vector3d vector_from_target;
						real distance_squared;

						vector_from_points3d(&evaluation_context->target_point, &firing_position->definition->position, &vector_from_target);
						distance_squared = magnitude_squared3d(&vector_from_target);
						if (distance_squared < 400.0f &&
							path_3d_available(structure_bsp, &evaluation_context->target_point, 0.0f, &firing_position->definition->position, NULL, NULL))
						{
							firing_position->path_distance_to_target = normalize3d(&vector_from_target);
							if (evaluation_context->find_path_direction_from_target)
							{
								firing_position->path_direction_from_target = vector_from_target;
							}
						}
					}
				}
				else if (evaluation_context->target_pathfinding_surface_index != NONE)
				{
					struct path_input input;
					struct path_state target_path_state;
					short index;

					path_input_new(&input, actor_definition->moving.pathfinding_radius, actor->emotions.ignorant_of_broken_surfaces, NONE);
					path_input_set_start(&input, &evaluation_context->target_pathfinding_point, evaluation_context->target_pathfinding_surface_index);
					path_input_set_search_bounds(&input, 20.0f);
					path_state_new(&input, &target_path_state, NULL);
					path_state_find(&target_path_state);

					for (index = 0; index < firing_position_count; index++)
					{
						struct firing_position *firing_position = &firing_positions[index];

						path_state_estimated_distance(
							&target_path_state,
							&firing_position->definition->position,
							firing_position->definition->surface_index,
							&firing_position->path_distance_to_target,
							NULL,
							evaluation_context->find_path_direction_from_target ? &firing_position->path_direction_from_target : NULL);
					}
				}
			}

			if (!evaluation_context->flying)
			{
				struct path_input input;

				actor_path_input_new(actor_index, &input);
				path_input_set_search_bounds(&input, evaluation_context->maximum_search_range);

				if (evaluation_context->attractor_enable && evaluation_context->has_target)
				{
					long attractor_object_index = NONE;

					if (evaluation_context->target_prop_index != NONE)
					{
						struct prop_datum *target_prop = prop_get(evaluation_context->target_prop_index);

						attractor_object_index = target_prop->unit_index;
					}
					path_input_set_attractor(&input, &evaluation_context->target_point, evaluation_context->attractor_radius, attractor_object_index, evaluation_context->attractor_weight);
				}
				else if (actor->danger_zone.danger_type > _actor_danger_zone_none &&
					!TEST_FLAG(actor_definition->flags2, _actor_definition_flags2_pathfinding_ignores_danger_bit))
				{
					path_input_set_attractor(&input, &actor->danger_zone.position, actor->danger_zone.danger_radius, actor->danger_zone.object_index, 10.0f);
				}

				path_state_new(&input, area_path_state, ai_debug_get_path_storage(actor_index));
				if (path_state_find(area_path_state))
				{
					*area_path_state_valid = TRUE;
				}
			}

			{
				short index;

				for (index = 0; index < firing_position_count; index++)
				{
					struct firing_position *firing_position = &firing_positions[index];
					real_vector3d vector_from_actor;
					real distance_squared;

					if (evaluation_context->has_target)
					{
						firing_position->linear_distance_squared_to_target = distance_squared3d(&evaluation_context->target_point, &firing_position->definition->position);
					}

					vector_from_points3d(&actor->input.position.body_position, &firing_position->definition->position, &vector_from_actor);
					distance_squared = magnitude_squared3d(&vector_from_actor);
					if (distance_squared < evaluation_context->maximum_search_range * evaluation_context->maximum_search_range)
					{
						if (evaluation_context->flying)
						{
							firing_position->path_closest_approach_to_target = square_root(point_to_line_distance_squared3d(&evaluation_context->target_point, &actor->input.position.body_position, &vector_from_actor));
							firing_position->path_distance_from_actor = normalize3d(&vector_from_actor);
							if (evaluation_context->find_path_direction_from_actor)
							{
								firing_position->path_direction_from_actor = vector_from_actor;
							}
						}
						else
						{
							path_state_estimated_distance(
								area_path_state,
								&firing_position->definition->position,
								firing_position->definition->surface_index,
								&firing_position->path_distance_from_actor,
								&firing_position->path_closest_approach_to_target,
								evaluation_context->find_path_direction_from_actor ? &firing_position->path_direction_from_actor : NULL);
						}
					}

					if (firing_position->path_distance_from_actor < evaluation_context->maximum_search_range)
					{
						any_position_in_range = TRUE;
					}
					else
					{
						firing_position->valid = FALSE;
					}
				}
			}

			if (evaluation_context->allow_outside_range && !any_position_in_range)
			{
				struct firing_position *firing_position;

				match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 2108, firing_position_count > 0);

				best_firing_position_index = random_range(0, firing_position_count);
				*area_path_state_valid = FALSE;
				actor_clear_discarded_firing_positions(actor_index, FALSE);

				firing_position = &firing_positions[best_firing_position_index];
				if (!firing_position_forced_evaluation(actor_index, evaluation_context, firing_position))
				{
					best_firing_position_index = NONE;
				}

				if (debug)
				{
#ifdef DEBUG
					ai_debug.firing_positions[firing_position->original_index].evaluated = TRUE;
					ai_debug.firing_positions[firing_position->original_index].last_evaluation = *firing_position;
#endif
				}
			}
			else
			{
				short firing_position_index;
				short sorted_index;
				struct firing_position *firing_position;
				long firing_position_indices[MAXIMUM_FIRING_POSITIONS_PER_ENCOUNTER];
				boolean expected_to_discard = FALSE;

				firing_position_pre_evaluate(actor_index, evaluation_context, firing_position_count, firing_positions);

				for (firing_position_index = 0; firing_position_index < firing_position_count; firing_position_index++)
				{
					firing_position_indices[firing_position_index] = firing_position_index;
				}

				global_temporary_sort_firing_position_count = firing_position_count;
				global_temporary_sort_firing_position_array = firing_positions;
				qsort_4byte(firing_position_indices, firing_position_count, firing_position_compare);

				evaluation_context->post_evaluation_bounded = firing_positions_get_post_evaluation_bound(actor_index, evaluation_context);

				for (sorted_index = 0; sorted_index < firing_position_count; sorted_index++)
				{
					firing_position_index = (short)firing_position_indices[sorted_index];
					firing_position = &firing_positions[firing_position_index];

					if (firing_position->valid)
					{
						evaluation_context->valid_count++;
					}
					if (!firing_position->rejected)
					{
						evaluation_context->nonrejected_count++;
					}

					if (!firing_position->valid ||
						(evaluation_context->post_evaluation_bounded && firing_position->evaluation + evaluation_context->post_evaluation_bound <= best_evaluation))
					{
						if (!AI_BEHAVIOR(evaluate_all_positions))
						{
							short index;

							for (index = sorted_index; index < firing_position_count; index++)
							{
								if (firing_positions[firing_position_indices[index]].valid)
								{
									evaluation_context->valid_count++;
									evaluation_context->skipped_count++;
								}
							}
							break;
						}

						expected_to_discard = TRUE;
					}

					if (firing_position->valid)
					{
						if (evaluation_context->has_target)
						{
							firing_position_compute_line_of_sight(actor_index, evaluation_context, firing_position);
						}

						evaluation_context->post_evaluation_count++;
						firing_position->pre_evaluation = firing_position->evaluation;
						if (firing_position_post_evaluate(actor_index, evaluation_context, firing_position) &&
							firing_position->evaluation > best_evaluation)
						{
							match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 2243, !expected_to_discard);
							best_firing_position_index = firing_position_index;
							best_evaluation = firing_position->evaluation;
						}
					}
				}

				if (debug)
				{
					for (firing_position_index = 0, firing_position = firing_positions; firing_position_index < firing_position_count; firing_position_index++, firing_position++)
					{
#ifdef DEBUG
						ai_debug.firing_positions[firing_position->original_index].evaluated = TRUE;
						ai_debug.firing_positions[firing_position->original_index].last_evaluation = *firing_position;
#endif
					}
				}

#ifdef DEBUG
				if (ai_debug.print_evaluation_statistics)
				{
					char *mode_names[] =
					{
						"fight",
						"panic",
						"cover",
						"uncover",
						"guard",
						"pursue"
					};

					error(_error_silent, "fp-eval %s: encounter %3d consider %3d valid %3d nonrejected %3d post-eval %3d skipped %3d",
						mode_names[evaluation_context->evaluation_mode],
						evaluation_context->encounter_count,
						evaluation_context->consider_count,
						evaluation_context->valid_count,
						evaluation_context->nonrejected_count,
						evaluation_context->post_evaluation_count,
						evaluation_context->skipped_count);
				}
#endif
			}
		}

		if (best_firing_position_index != NONE)
		{
			if (best_firing_position)
			{
				*best_firing_position = firing_positions[best_firing_position_index];
			}

			best_firing_position_index = firing_positions[best_firing_position_index].original_index;
			if (current_owner)
			{
				*current_owner = firing_position_owner_actor_indices[best_firing_position_index];
			}
		}
	}

	return best_firing_position_index;
}

short actor_active_select_firing_position(
	long actor_index,
	struct firing_position_evaluation_context *evaluation_context,
	struct firing_position *best_firing_position,
	long *current_owner,
	struct path_state *area_path_state,
	boolean *area_path_state_valid)
{
	struct actor_datum *actor = actor_get(actor_index);
	short result = NONE;

	if (actor->meta.encounter_index != NONE)
	{
		unsigned long currently_allowed_groups = actor_get_firing_position_group(actor_index, evaluation_context->evaluation_mode, _firing_position_group_normal);
		unsigned long not_searching_groups = actor_get_firing_position_group(actor_index, evaluation_context->evaluation_mode, _firing_position_group_when_not_searching);
		unsigned long searching_groups = actor_get_firing_position_group(actor_index, evaluation_context->evaluation_mode, _firing_position_group_when_searching);
		unsigned long total_groups = searching_groups | not_searching_groups;

		if (total_groups > currently_allowed_groups)
		{
			evaluation_context->preferred_groups = currently_allowed_groups;
			evaluation_context->preferred_weight = 8.0f;
			match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 2311, (total_groups & currently_allowed_groups) == currently_allowed_groups);
			evaluation_context->allowed_position_mask = total_groups;
		}
		else
		{
			evaluation_context->allowed_position_mask = currently_allowed_groups;
		}

		if (actor->firing_positions.current_position_index != NONE && actor->firing_positions.current_position_found_outside_range)
		{
			evaluation_context->allow_outside_range = FALSE;
		}
		else
		{
			evaluation_context->allow_outside_range = TRUE;
		}
		evaluation_context->allow_rejected_positions = TRUE;

		result = actor_select_firing_position(actor_index, evaluation_context, best_firing_position, current_owner, area_path_state, area_path_state_valid);
		if (result != NONE && !TEST_FLAG(currently_allowed_groups, best_firing_position->definition->group_index))
		{
			struct actor_datum *searching_actor = actor_get(actor_index);

			searching_actor->state.searching = !searching_actor->state.searching;
		}

		if (result == NONE && actor->firing_positions.current_position_index != NONE && actor->firing_positions.current_position_found_outside_range)
		{
			struct encounter_definition *encounter_definition;
			struct firing_position_definition *firing_position_definition;

			result = actor->firing_positions.current_position_index;
			encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
				struct encounter_definition);
			firing_position_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->firing_positions,
				result,
				struct firing_position_definition);

			best_firing_position->definition = firing_position_definition;
			best_firing_position->original_index = result;
			best_firing_position->line_of_sight = _ai_line_of_sight_clear;
			best_firing_position->path_distance_from_actor = REAL_MAX;
			best_firing_position->path_distance_to_target = REAL_MAX;
			best_firing_position->path_closest_approach_to_target = REAL_MAX;
			best_firing_position->path_direction_from_target = *global_zero_vector3d;
			best_firing_position->path_direction_from_actor = *global_zero_vector3d;
			if (evaluation_context->has_target)
			{
				best_firing_position->linear_distance_squared_to_target = distance_squared3d(&evaluation_context->target_point, &firing_position_definition->position);
			}
			else
			{
				best_firing_position->linear_distance_squared_to_target = 0.0f;
			}

			if (!firing_position_forced_evaluation(actor_index, evaluation_context, best_firing_position))
			{
				result = NONE;
				actor->firing_positions.current_position_index = NONE;
			}

			*current_owner = NONE;
			*area_path_state_valid = FALSE;
		}
	}

	return result;
}

short actor_change_firing_position(
	long actor_index,
	short firing_position_index,
	struct firing_position *firing_position,
	long previous_owner,
	struct path_state *cached_path_state,
	boolean cached_path_available)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (firing_position_index == NONE)
	{
		actor_move_halt(actor_index);
		actor->firing_positions.current_position_index = NONE;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 2427, actor->meta.encounter_index != NONE);

		if (actor->firing_positions.current_position_index != NONE && actor->firing_positions.current_position_index != firing_position_index)
		{
			actor_discard_firing_position(actor_index, actor->firing_positions.current_position_index, TRUE);
		}

		if (previous_owner != NONE)
		{
			struct actor_datum *previous_owner_actor = actor_get(previous_owner);

			match_assert("c:\\halo\\SOURCE\\ai\\actor_firing_position.c", 2440, actor_index != previous_owner);
			actor_move_halt(previous_owner);
			previous_owner_actor->firing_positions.current_position_index = NONE;
		}

		if (actor->firing_positions.current_position_index != firing_position_index)
		{
			actor->firing_positions.current_position_index = firing_position_index;
			actor->firing_positions.current_position_found_outside_range = !cached_path_available;
			actor->firing_positions.moved_away_from_firing_position = FALSE;

			if (!actor_move_to_firing_position(actor_index, firing_position_index, cached_path_available ? cached_path_state : NULL))
			{
				actor->firing_positions.current_position_index = NONE;
			}
		}
	}

	if (actor->meta.encounter_index != NONE)
	{
#ifdef DEBUG
		encounter_verify_firing_position_owner_actor_indices(actor->meta.encounter_index);
#endif
	}

	return actor->firing_positions.current_position_index;
}

void actor_clear_discarded_firing_positions(
	long actor_index,
	boolean clear_temporary_only)
{
	struct actor_datum *actor = actor_get(actor_index);
	short index;

	actor->firing_positions.next_discarded_firing_positions_entry = 0;
	for (index = 0; index < NUMBER_OF_DISCARDED_FIRING_POSITIONS_PER_ACTOR; index++)
	{
		actor->firing_positions.discarded_firing_positions[index].index = NONE;
	}

	if (actor->firing_positions.last_discarded_firing_position_valid &&
		!(clear_temporary_only && !actor->firing_positions.last_discarded_firing_position_temporary))
	{
		actor->firing_positions.last_discarded_firing_position_valid = FALSE;
	}

	return;
}

void actor_discard_firing_position(
	long actor_index,
	short firing_position_index,
	boolean temporary)
{
	if (firing_position_index != NONE)
	{
		struct actor_datum *actor = actor_get(actor_index);
		struct encounter_definition *encounter_definition;
		struct firing_position_definition *firing_position_definition;

		actor->firing_positions.discarded_firing_positions[actor->firing_positions.next_discarded_firing_positions_entry].temporary = temporary;
		actor->firing_positions.discarded_firing_positions[actor->firing_positions.next_discarded_firing_positions_entry].index = firing_position_index;
		actor->firing_positions.next_discarded_firing_positions_entry = (actor->firing_positions.next_discarded_firing_positions_entry + 1) % NUMBER_OF_DISCARDED_FIRING_POSITIONS_PER_ACTOR;

		encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
			struct encounter_definition);
		firing_position_definition = TAG_BLOCK_GET_ELEMENT(
			&encounter_definition->firing_positions,
			firing_position_index,
			struct firing_position_definition);

		actor->firing_positions.last_discarded_firing_position_valid = TRUE;
		actor->firing_positions.last_discarded_firing_position_temporary = temporary;
		actor->firing_positions.last_discarded_firing_position = firing_position_definition->position;
	}

	return;
}

boolean actor_firing_position_discarded(
	long actor_index,
	short firing_position_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (firing_position_index != NONE)
	{
		short index;

		for (index = 0; index < NUMBER_OF_DISCARDED_FIRING_POSITIONS_PER_ACTOR; index++)
		{
			if (firing_position_index == actor->firing_positions.discarded_firing_positions[index].index)
			{
				result = TRUE;
				break;
			}
		}
	}

	return result;
}

/* ---------- private code */
