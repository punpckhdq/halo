/*
ACTOR_COMBAT.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai.h"
#include "ai_debug.h"
#include "ai_communication.h"
#include "encounters.h"
#include "props.h"
#include "network_game_globals.h"
#include "players.h"
#include "console.h"
#include "network_messages.h"
#include "collisions.h"
#include "units.h"
#include "vehicles.h"
#include "vehicle_definitions.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
#include "weapon_definitions.h"
#include "projectiles.h"
#include "projectile_definitions.h"
#include "game.h"
#include "game_globals.h"
#include "scenario.h"
#include "tag_files.h"

/* ---------- constants */

enum
{
	ACTOR_COMBAT_MANUAL_TARGET_LINE_OF_SIGHT_TICKS = 10, /* fake name */
	ACTOR_COMBAT_BLOCKED_COMMUNICATION_TICKS = 45, /* fake name */
	ACTOR_COMBAT_MINIMUM_TRIGGER_DELAY_TICKS = 2, /* fake name */
	ACTOR_COMBAT_SPECIAL_FIRE_DENY_ATTEMPTS = 3, /* fake name */
	ACTOR_COMBAT_STRAFING_MINIMUM_ENEMY_COUNT = 3, /* fake name */
	ACTOR_COMBAT_OUT_OF_SIGHT_TICKS = 75, /* fake name */
	MAXIMUM_COLLATERAL_DAMAGE_ENEMY_ACTORS = 32, /* fake name */
};

#define ACTOR_COMBAT_FIRE_TARGET_MOVED_DISTANCE 0.5f /* fake name */
#define ACTOR_COMBAT_STRAFING_ENEMY_RADIUS 6.0f /* fake name */
#define ACTOR_COMBAT_GRENADE_TARGET_HEIGHT 0.2f /* fake name */
#define ACTOR_COMBAT_PLAYFIGHTING_MISS_DISTANCE 1.5f /* fake name */
#define ACTOR_COMBAT_GUN_OFFSET_EPSILON 0.0001f /* fake name */

/* 30 degrees */
#define ACTOR_COMBAT_AIM_ROTATION_SINE 0.5f /* fake name */
#define ACTOR_COMBAT_AIM_ROTATION_COSINE 0.86602539f /* fake name */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean actor_combat_enable_special_fire_situation(long actor_index, short special_fire_situation);
static void actor_get_weapon_vector(long actor_index, real_vector3d *weapon_vector);
static void actor_combat_get_firing_parameters(long actor_index, struct actor_variant_definition *firing_variant_definition, struct actor_burst_geometry **burst_geometry_reference, struct actor_firing_pattern **firing_pattern_reference);
static struct projectile_definition *actor_get_grenade_definition(short grenade_type);
static void actor_combat_find_nearby_target(real_point3d *target_point, real miss_distance);
static void actor_start_pause(long actor_index);
static boolean actor_start_first_burst_delay(long actor_index, struct actor_variant_definition *firing_variant_definition);
static boolean actor_combat_reaim_grenade(long actor_index, real_point3d const *grenade_origin);
static boolean actor_combat_build_grenade_trajectory(short grenade_type, real_point3d const *grenade_origin, real velocity_max, real_point3d const *desired_impact_point, real *optional_ballistic_fraction_min, boolean lob, real_vector3d *aim_vector, real *aim_speed, real *arc_time, real_vector3d *arc_initial_velocity, real *arc_acceleration);
static boolean actor_combat_allow_special_fire_situation(long actor_index, short special_fire_situation);
static void actor_start_burst(long actor_index);
static boolean actor_combat_retarget_grenade(long actor_index, real_point3d const *desired_grenade_target);

/* ---------- globals */

/* ---------- public code */

static boolean actor_combat_enable_special_fire_situation(
	long actor_index,
	short special_fire_situation)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	switch (special_fire_situation)
	{
	case _actor_special_fire_situation_enemy_visible:
		result = actor->control.current_fire_target_type == _actor_fire_target_prop &&
			actor->target.target_type >= _actor_target_clear_line_of_sight_enemy;
		break;

	case _actor_special_fire_situation_enemy_out_of_sight:
		result = actor->control.current_fire_target_type == _actor_fire_target_none &&
			actor->target.target_type >= _actor_target_uninspected_orphan &&
			actor->target.target_really_alive &&
			actor->target.since_any_target_visible_timer >= ACTOR_COMBAT_OUT_OF_SIGHT_TICKS;
		break;

	case _actor_special_fire_situation_strafing:
		result = actor->control.current_fire_target_type == _actor_fire_target_prop &&
			actor->target.target_type >= _actor_target_clear_line_of_sight_enemy &&
			actor->input.vehicle_gunner;
		break;
	}

	return result;
}

void actor_combat_fire_wildly(
	long actor_index,
	long fire_ticks)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->control.fire_state = _actor_fire_state_wild;
	actor->control.fire_state_timer = (short)fire_ticks;

	return;
}

void actor_combat_disable_bursts(
	long actor_index,
	long disable_timer)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->control.burst_disable_timer = (short)MAX(actor->control.burst_disable_timer, disable_timer);

	return;
}

boolean actor_firing_blindly(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	return actor->control.fire_state == _actor_fire_state_wild;
}

boolean actor_combat_currently_firing_burst(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->control.current_fire_target_type > _actor_fire_target_none)
	{
		result = actor->control.fire_state == _actor_fire_state_bursting;
	}

	return result;
}

struct weapon_definition *actor_get_weapon_definition(
	long actor_index)
{
	struct weapon_definition *weapon_definition = NULL;
	long weapon_index = actor_get_weapon(actor_index);

	if (weapon_index != NONE)
	{
		struct weapon_datum *weapon = weapon_get(weapon_index);

		weapon_definition = weapon_definition_get(weapon->definition_index);
	}

	return weapon_definition;
}

static void actor_get_weapon_vector(
	long actor_index,
	real_vector3d *weapon_vector)
{
	struct actor_datum *actor = actor_get(actor_index);
	long unit_index = actor->meta.unit_index;
	boolean done /* fake name */ = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 1166, weapon_vector);

	if (actor->input.vehicle_gunner)
	{
		struct vehicle_datum *vehicle = vehicle_get(actor->input.vehicle_index);
		struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

		unit_index = actor->input.vehicle_index;

		if (TEST_FLAG(vehicle_definition->vehicle.flags, _vehicle_ai_weapon_cannot_rotate_bit))
		{
			*weapon_vector = vehicle->object.forward;
			done = TRUE;
		}
	}

	if (!done)
	{
		unit_get(unit_index);
		unit_get_aiming_vector(unit_index, weapon_vector);
		unit_clip_to_aiming_bounds(unit_index, weapon_vector, TRUE);
	}

	return;
}

struct actor_variant_definition *actor_combat_get_firing_variant_definition(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *firing_variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	struct weapon_definition *weapon_definition = actor_get_weapon_definition(actor_index);

	if (weapon_definition && weapon_definition->weapon.ai_firing_parameters.index != NONE)
	{
		firing_variant_definition = actor_variant_definition_get(weapon_definition->weapon.ai_firing_parameters.index);
	}

	return firing_variant_definition;
}

static void actor_combat_get_firing_parameters(
	long actor_index,
	struct actor_variant_definition *firing_variant_definition,
	struct actor_burst_geometry **burst_geometry_reference,
	struct actor_firing_pattern **firing_pattern_reference)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_burst_geometry *burst_geometry = &firing_variant_definition->ranged_combat.burst_geometry;
	struct actor_firing_pattern *firing_pattern = NULL;

	if (actor->emotions.berserk)
	{
		firing_pattern = &firing_variant_definition->ranged_combat.berserk_pattern;
	}
	else if (actor->control.firing_at_new_target)
	{
		firing_pattern = &firing_variant_definition->ranged_combat.new_target_pattern;
	}
	else if (actor->control.firing_while_moving)
	{
		firing_pattern = &firing_variant_definition->ranged_combat.moving_pattern;
	}

	match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 1677, burst_geometry_reference && firing_pattern_reference);

	*burst_geometry_reference = burst_geometry;
	*firing_pattern_reference = firing_pattern;

	return;
}

static struct projectile_definition *actor_get_grenade_definition(
	short grenade_type)
{
	struct game_globals_grenade *grenade = TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->grenades, grenade_type, struct game_globals_grenade);
	struct projectile_definition *projectile_definition = NULL;

	if (grenade && grenade->projectile.index != NONE)
	{
		projectile_definition = projectile_definition_get(grenade->projectile.index);
	}

	return projectile_definition;
}

static void actor_combat_find_nearby_target(
	real_point3d *target_point,
	real miss_distance)
{
	real_vector3d offset_vector;
	real_point3d cast_from_point;
	real_point3d new_target;
	struct collision_result collision;

	point_from_line3d(target_point, global_up3d, 1.5f, &cast_from_point);
	vector3d_from_angle(&offset_vector, real_random_range(-_pi, _pi));
	point_from_line3d(&cast_from_point, &offset_vector, miss_distance, &new_target);

	if (collision_test_line(
		FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_back_facing_surfaces_bit) | FLAG(_collision_test_structure_bit),
		target_point,
		&cast_from_point,
		NONE,
		&collision))
	{
		cast_from_point = *target_point;
	}

	if (collision_test_line(
		FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_back_facing_surfaces_bit) | FLAG(_collision_test_structure_bit),
		&cast_from_point,
		&new_target,
		NONE,
		&collision))
	{
		real distance = collision.t * miss_distance - 0.1f;

		distance = MAX(0.0f, distance);
		point_from_line3d(&cast_from_point, &offset_vector, distance, &new_target);
	}

	*target_point = new_target;

	return;
}

static void actor_start_pause(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_burst_geometry *geometry;
	struct actor_firing_pattern *pattern;
	real pause_time;

	actor_combat_get_firing_parameters(actor_index, actor_combat_get_firing_variant_definition(actor_index), &geometry, &pattern);

	pause_time = real_random_range(geometry->burst_separation_lower_bound, geometry->burst_separation_upper_bound);
	pause_time *= game_difficulty_get_team_value(_game_difficulty_burst_separation_delay_scale, actor->meta.team_index);

	if (pattern && pattern->burst_separation_modifier != 0.0f)
	{
		pause_time *= pattern->burst_separation_modifier;
	}

	if (actor->external_orders.playfighting)
	{
		pause_time *= 1.7f;
	}

	actor->control.fire_state_timer = (short)(pause_time * TICKS_PER_SECOND);

	return;
}

static boolean actor_start_first_burst_delay(
	long actor_index,
	struct actor_variant_definition *firing_variant_definition)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean fire_immediately = actor->orders.combat.override_firing_restrictions;

	if (actor->control.current_fire_target_type == _actor_fire_target_prop)
	{
		struct prop_datum *prop = prop_get(actor->control.current_fire_target_prop_index);

		if (prop->state >= _prop_state_uninspected_orphan && prop->state <= _prop_state_inspected_orphan)
		{
			actor->firing_positions.pursuit_fired_at_orphan = TRUE;
			fire_immediately = TRUE;
		}
	}

	if (fire_immediately)
	{
		actor->control.fire_state_timer = 0;
	}
	else
	{
		actor->control.fire_state_timer = (short)(TICKS_PER_SECOND * real_random_range(
			firing_variant_definition->ranged_combat.first_burst_delay_lower_bound,
			firing_variant_definition->ranged_combat.first_burst_delay_upper_bound));
	}

	return !fire_immediately;
}

static boolean actor_combat_reaim_grenade(
	long actor_index,
	real_point3d const *grenade_origin)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	struct projectile_definition *projectile_definition;
	real_vector3d aim_vector;
	real aim_speed;
	real arc_time;
	boolean linear;
	boolean result;

	projectile_definition = actor_get_grenade_definition(variant_definition->grenade_combat.grenade_type);
	result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 1731, projectile_definition);

	if (projectile_aim(
		projectile_definition,
		grenade_origin,
		&actor->control.grenade_current_target,
		NULL,
		NULL,
		NULL,
		&actor->control.grenade_current_aim_speed,
		actor->control.grenade_current_lob,
		&aim_vector,
		&aim_speed,
		&arc_time,
		NULL,
		&linear))
	{
		real_vector2d horizontal_aiming_direction;

		horizontal_aiming_direction.i = aim_vector.i;
		horizontal_aiming_direction.j = aim_vector.j;

		if (normalize2d(&horizontal_aiming_direction) > 0.0f &&
			dot_product2d(&horizontal_aiming_direction, (real_vector2d *)&actor->input.facing_vector) > ACTOR_COMBAT_AIM_ROTATION_COSINE)
		{
			real_vector3d arc_initial_velocity;
			real arc_acceleration;

			scale_vector3d(&aim_vector, aim_speed, &arc_initial_velocity);
			arc_acceleration = linear ? 0.0f : projectile_get_ballistic_acceleration(projectile_definition);

			if (ai_test_ballistic_line_of_fire(
				actor_index,
				grenade_origin,
				arc_time,
				&arc_initial_velocity,
				arc_acceleration,
				actor->control.grenade_current_ignore_object_index,
				actor->input.vehicle_index != NONE))
			{
				actor->control.grenade_current_aim_vector = aim_vector;
				actor->control.grenade_current_aim_speed = aim_speed;
				result = TRUE;
			}
		}
	}

	return result;
}

static boolean actor_combat_build_grenade_trajectory(
	short grenade_type,
	real_point3d const *grenade_origin,
	real velocity_max,
	real_point3d const *desired_impact_point,
	real *optional_ballistic_fraction_min,
	boolean lob,
	real_vector3d *aim_vector,
	real *aim_speed,
	real *arc_time,
	real_vector3d *arc_initial_velocity,
	real *arc_acceleration)
{
	struct projectile_definition *projectile_definition;
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 1780, grenade_origin && desired_impact_point && aim_vector && aim_speed);

	projectile_definition = actor_get_grenade_definition(grenade_type);

	if (projectile_definition)
	{
		boolean linear = FALSE;

		if (projectile_aim(
			projectile_definition,
			grenade_origin,
			desired_impact_point,
			&velocity_max,
			NULL,
			optional_ballistic_fraction_min,
			NULL,
			FALSE,
			aim_vector,
			aim_speed,
			arc_time,
			NULL,
			&linear))
		{
			result = TRUE;

			if (arc_initial_velocity)
			{
				scale_vector3d(aim_vector, *aim_speed, arc_initial_velocity);
			}

			if (arc_acceleration)
			{
				*arc_acceleration = linear ? 0.0f : projectile_get_ballistic_acceleration(projectile_definition);
			}
		}
	}

	return result;
}

boolean actor_combat_find_grenade_target(
	long actor_index,
	real_point3d *grenade_target,
	long *grenade_target_prop_index,
	long *grenade_ignore_object_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	boolean result = FALSE;

	if (actor->target.target_prop_index != NONE)
	{
		struct prop_datum *prop = prop_get(actor->target.target_prop_index);

		if (prop->enemy &&
			!prop->dead &&
			((prop->state >= _prop_state_becoming_unacknowledged && prop->state <= _prop_state_acknowledged) || prop->state == _prop_state_uninspected_orphan) &&
			prop->distance > variant_definition->grenade_combat.grenade_range_lower_bound &&
			prop->distance < variant_definition->grenade_combat.grenade_range_upper_bound)
		{
			*grenade_target = prop->body_position;
			grenade_target->z += ACTOR_COMBAT_GRENADE_TARGET_HEIGHT;
			*grenade_target_prop_index = actor->target.target_prop_index;
			*grenade_ignore_object_index = prop->vehicle_index;
			result = TRUE;
		}
	}

	if (result && actor->external_orders.playfighting)
	{
		actor_combat_find_nearby_target(grenade_target, ACTOR_COMBAT_PLAYFIGHTING_MISS_DISTANCE);
	}

	return result;
}

boolean actor_combat_check_collateral_damage(
	long actor_index,
	real enemy_radius,
	real collateral_damage_radius,
	real_point3d const *target_point,
	short *enemy_count_reference)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	boolean result = TRUE;
	short enemy_count = 0;
	long enemy_actor_indices[MAXIMUM_COLLATERAL_DAMAGE_ENEMY_ACTORS];
	short enemy_actor_count = 0;

	{
		struct prop_iterator iterator;
		struct prop_datum *prop;

		prop_iterator_new(&iterator, actor_index);

		while (prop = prop_iterator_next(&iterator))
		{
			if (prop->state >= _prop_state_becoming_unacknowledged && prop->state <= _prop_state_acknowledged && !prop->dead)
			{
				if (prop->enemy)
				{
					if (distance_squared3d(&prop->body_position, target_point) < enemy_radius * enemy_radius)
					{
						if (prop->player)
						{
							enemy_count += 10;
						}
						else if (prop->vehicle_index != NONE)
						{
							enemy_count += 5;
						}
						else if (prop->actor_index != NONE)
						{
							if ((unsigned short)enemy_actor_count < MAXIMUM_COLLATERAL_DAMAGE_ENEMY_ACTORS)
							{
								enemy_actor_indices[enemy_actor_count++] = prop->actor_index;
							}

							if (prop->swarm)
							{
								struct actor_datum *prop_actor = actor_get(prop->actor_index);

								enemy_count += prop_actor->meta.swarm_unit_count;
							}
							else
							{
								enemy_count++;
							}
						}
					}
				}
				else if (collateral_damage_radius > 0.0f &&
					distance_squared3d(&prop->body_position, target_point) < collateral_damage_radius * collateral_damage_radius)
				{
					result = FALSE;
					break;
				}
			}
		}
	}

	if (enemy_radius > 0.0f && actor->target.target_prop_index != NONE)
	{
		struct prop_datum *target_prop = prop_get(actor->target.target_prop_index);

		if (target_prop->actor_index != NONE)
		{
			struct actor_datum *target_actor = actor_get(target_prop->actor_index);

			if (target_actor->meta.encounter_index != NONE)
			{
				struct encounter_actor_iterator iterator;
				struct actor_datum *enemy_actor;

				encounter_actor_iterator_new(&iterator, target_actor->meta.encounter_index);

				while (enemy_actor = encounter_actor_iterator_next(&iterator))
				{
					boolean already_counted = FALSE;
					short index;

					for (index = 0; index < enemy_actor_count; index++)
					{
						if (enemy_actor_indices[index] == iterator.index)
						{
							already_counted = TRUE;
							break;
						}
					}

					if (!already_counted &&
						distance_squared3d(&enemy_actor->input.position.body_position, target_point) < enemy_radius * enemy_radius)
					{
						if (enemy_actor->meta.swarm)
						{
							enemy_count += enemy_actor->meta.swarm_unit_count;
						}
						else
						{
							enemy_count++;
						}
					}
				}
			}
		}
	}

	if (result && actor->meta.encounter_index != NONE && collateral_damage_radius > 0.0f)
	{
		struct encounter_actor_iterator iterator;
		struct actor_datum *friend_actor;

		encounter_actor_iterator_new(&iterator, actor->meta.encounter_index);

		while (friend_actor = encounter_actor_iterator_next(&iterator))
		{
			if (distance_squared3d(&friend_actor->input.position.body_position, target_point) < collateral_damage_radius * collateral_damage_radius)
			{
				result = FALSE;
				break;
			}
		}
	}

	if (enemy_count_reference)
	{
		*enemy_count_reference = enemy_count;
	}

	return result;
}

boolean actor_combat_plan_grenade_trajectory(
	long actor_index,
	short trajectory_type,
	real_point3d const *grenade_target,
	long grenade_target_prop_index,
	long grenade_ignore_object_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	real_point3d const *target = grenade_target;
	boolean result = FALSE;
	real_point3d origin = actor->input.position.head_position;

	{
		real_vector3d aim_vector;
		real aim_speed;
		real arc_time;
		real_vector3d arc_initial_velocity;
		real arc_acceleration;

		if (actor_combat_build_grenade_trajectory(
				variant_definition->grenade_combat.grenade_type,
				&origin,
				variant_definition->grenade_combat.grenade_maximum_velocity,
				target,
				NULL,
				FALSE,
				&aim_vector,
				&aim_speed,
				&arc_time,
				&arc_initial_velocity,
				&arc_acceleration) &&
			ai_test_ballistic_line_of_fire(
				actor_index,
				&origin,
				arc_time,
				&arc_initial_velocity,
				arc_acceleration,
				grenade_ignore_object_index,
				actor->input.vehicle_index != NONE))
		{
			actor->control.grenade_current_target = *target;
			actor->control.grenade_current_prop_index = grenade_target_prop_index;
			actor->control.grenade_current_ignore_object_index = grenade_ignore_object_index;
			actor->control.grenade_current_aim_vector = aim_vector;
			actor->control.grenade_current_aim_speed = aim_speed;
			actor->control.grenade_current_lob = FALSE;
			result = TRUE;
		}
	}

	return result;
}

static boolean actor_combat_allow_special_fire_situation(
	long actor_index,
	short special_fire_situation)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = TRUE;

	if (special_fire_situation == _actor_special_fire_situation_strafing)
	{
		struct prop_datum *prop;

		match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 970, actor->control.current_fire_target_type == _actor_fire_target_prop);

		prop = prop_get(actor->control.current_fire_target_prop_index);

		if (prop->vehicle_index != NONE)
		{
			result = TRUE;
		}
		else if (prop->player)
		{
			result = FALSE;
		}
		else
		{
			short enemies_nearby = 0;

			actor_combat_check_collateral_damage(actor_index, ACTOR_COMBAT_STRAFING_ENEMY_RADIUS, 0.0f, &prop->body_position, &enemies_nearby);
			result = enemies_nearby >= ACTOR_COMBAT_STRAFING_MINIMUM_ENEMY_COUNT;
		}
	}

	return result;
}

long actor_aim_projectile(
	long actor_index,
	real_point3d const *origin,
	real_vector3d *vector,
	real *error_reference)
{
	struct actor_datum *actor = actor_get(actor_index);
	long target_unit_index = NONE;

	if (actor->control.fire_state == _actor_fire_state_bursting)
	{
		struct actor_debug_info *debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];

		debug_info->last_projectile_aiming_time = game_time_get();

		if (actor->control.current_fire_target_type == _actor_fire_target_prop && actor->control.current_fire_target_prop_index != NONE)
		{
			struct prop_datum *prop = prop_get(actor->control.current_fire_target_prop_index);

			if (prop->state >= _prop_state_becoming_unacknowledged && prop->state <= _prop_state_acknowledged)
			{
				target_unit_index = prop->unit_index;
			}
		}

		match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 1071, vector);

		debug_info->aim_last_by_vector = actor->control.burst_aim_by_vector;
		debug_info->aim_last_origin = *origin;

		if (actor->control.burst_aim_by_vector)
		{
			*vector = actor->control.burst_aim_vector;
		}
		else
		{
			debug_info->aim_last_target = actor->control.burst_target;
			vector_from_points3d(origin, &actor->control.burst_target, vector);
			normalize3d(vector);
		}

		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_combat.c", 1090, vector);

		debug_info->aim_last_vector = *vector;

		{
			real_vector3d weapon_vector;

			actor_get_weapon_vector(actor_index, &weapon_vector);

			if (dot_product3d(vector, &weapon_vector) < ACTOR_COMBAT_AIM_ROTATION_COSINE)
			{
				real_vector3d rotation_axis;
				boolean rotate = TRUE;

				cross_product3d(&weapon_vector, vector, &rotation_axis);

				if (normalize3d(&rotation_axis) == 0.0f)
				{
					perpendicular3d(&weapon_vector, &rotation_axis);

					if (normalize3d(&rotation_axis) == 0.0f)
					{
						rotate = FALSE;
					}
				}

				*vector = weapon_vector;

				if (rotate)
				{
					rotate_vector_about_axis(vector, &rotation_axis, ACTOR_COMBAT_AIM_ROTATION_SINE, ACTOR_COMBAT_AIM_ROTATION_COSINE);
				}

				debug_info->aim_last_rotated = TRUE;
				debug_info->aim_last_rotated_original_vector = weapon_vector;
				debug_info->aim_last_rotated_vector = *vector;
			}
			else
			{
				debug_info->aim_last_rotated = FALSE;
			}
		}

		*error_reference = actor->control.burst_error;
	}

	return target_unit_index;
}

static void actor_start_burst(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *firing_variant_definition = actor_combat_get_firing_variant_definition(actor_index);
	struct actor_burst_geometry *geometry;
	struct actor_firing_pattern *pattern;
	boolean bombardment = FALSE;
	real new_target_time;
	real burst_duration;
	real error;

	if (actor->control.next_burst_secondary &&
		!actor_combat_allow_special_fire_situation(actor_index, firing_variant_definition->ranged_combat.special_fire_situation))
	{
		actor->control.next_burst_secondary = FALSE;
	}

	actor->control.fire_burst_secondary = actor->control.next_burst_secondary;
	actor->control.next_burst_secondary = FALSE;

	if (actor->input.vehicle_index == NONE)
	{
		actor->control.firing_while_moving = actor->input.in_midair || actor->control.moving;
	}
	else
	{
		struct vehicle_datum *vehicle = vehicle_get(actor->input.vehicle_index);

		actor->control.firing_while_moving = magnitude_squared3d(&vehicle->object.translational_velocity) > 1.0f;
	}

	new_target_time = firing_variant_definition->ranged_combat.new_target_pattern_time;
	new_target_time *= game_difficulty_get_team_value(_game_difficulty_new_target_delay_scale, actor->meta.team_index);
	actor->control.firing_at_new_target = (real)actor->control.current_fire_target_timer < new_target_time * TICKS_PER_SECOND;

	actor_combat_get_firing_parameters(actor_index, firing_variant_definition, &geometry, &pattern);

	if (actor->orders.combat.override_burst_duration > 0.0f)
	{
		burst_duration = actor->orders.combat.override_burst_duration;
	}
	else
	{
		burst_duration = real_random_range(geometry->burst_duration_lower_bound, geometry->burst_duration_upper_bound);

		if (pattern && pattern->burst_duration_modifier != 0.0f)
		{
			burst_duration *= pattern->burst_duration_modifier;
		}

		if (actor->external_orders.playfighting)
		{
			burst_duration *= 0.6f;
		}
	}

	actor->control.fire_state_timer = (short)(burst_duration * TICKS_PER_SECOND);

	error = firing_variant_definition->ranged_combat.projectile_error_angle;
	error *= game_difficulty_get_team_value(_game_difficulty_fire_projectile_error_scale, actor->meta.team_index);

	if (pattern && pattern->error_angle_modifier != 0.0f)
	{
		error *= pattern->error_angle_modifier;
	}

	if (actor->external_orders.playfighting)
	{
		error = error * 2.0f + DEGREES_TO_RADIANS(1.0f);
	}

	actor->control.burst_error = error;
	actor->control.burst_damage_modifier = 0.0f;

	if (firing_variant_definition->ranged_combat.weapon_damage_modifier > 0.0f)
	{
		actor->control.burst_damage_modifier = firing_variant_definition->ranged_combat.weapon_damage_modifier;

		if (ai_debug.print_damage_modifiers)
		{
			console_printf(
				FALSE,
				"%s: manual damage modifier %.2f",
				tag_name_strip_path(tag_get_name(actor->meta.variant_definition_index)),
				actor->control.burst_damage_modifier);
		}
	}
	else if (firing_variant_definition->ranged_combat.damage_per_second > 0.0f)
	{
		long weapon_index = actor_get_weapon(actor_index);

		if (weapon_index != NONE)
		{
			struct weapon_datum *weapon = weapon_get(weapon_index);
			real rate_of_fire;
			real damage = weapon_definition_get_damage_potential(weapon->definition_index, &rate_of_fire);
			real damage_per_second;

			if (firing_variant_definition->ranged_combat.rate_of_fire > 0.0f)
			{
				rate_of_fire = MIN(rate_of_fire, firing_variant_definition->ranged_combat.rate_of_fire);
			}

			damage_per_second = rate_of_fire * damage;

			if (damage_per_second > 0.0f)
			{
				actor->control.burst_damage_modifier = firing_variant_definition->ranged_combat.damage_per_second / damage_per_second;

				if (ai_debug.print_damage_modifiers)
				{
					console_printf(
						FALSE,
						"%s: proj %.1f rof %.1f dmg/s %.1f -> to get %.1f mod= %.2f",
						tag_name_strip_path(tag_get_name(actor->meta.variant_definition_index)),
						damage,
						rate_of_fire,
						damage_per_second,
						firing_variant_definition->ranged_combat.damage_per_second,
						actor->control.burst_damage_modifier);
				}
			}
		}
	}

	if (actor->control.fire_burst_secondary || actor->control.overcharging_weapon)
	{
		if (firing_variant_definition->ranged_combat.special_damage_modifier > 0.0f)
		{
			actor->control.burst_damage_modifier *= firing_variant_definition->ranged_combat.special_damage_modifier;
		}

		actor->control.burst_error += firing_variant_definition->ranged_combat.special_projectile_error;
	}

	if (firing_variant_definition->ranged_combat.weapon_bombardment_range > 0.0f &&
		actor->control.current_fire_target_type == _actor_fire_target_prop)
	{
		struct prop_datum *prop = prop_get(actor->control.current_fire_target_prop_index);

		bombardment = !(prop->state >= _prop_state_becoming_unacknowledged &&
			prop->state <= _prop_state_acknowledged &&
			prop->visibility != _actor_perception_none);
	}

	{
		real_point3d origin;
		real_vector3d offset;
		real_vector3d adjustment;
		real_vector3d side;
		real_vector3d up;
		real_vector3d forward;
		real origin_angle;
		real return_angle;
		real origin_radius;
		real return_length;
		real origin_cosine;
		real origin_sine;
		real return_cosine;
		real return_sine;

		origin = actor->control.current_fire_target_position;

		if (bombardment)
		{
			actor_combat_find_nearby_target(&origin, firing_variant_definition->ranged_combat.weapon_bombardment_range);
		}

		vector_from_points3d(&actor->input.position.head_position, &origin, &forward);
		set_real_vector3d(&up, 0.0f, 0.0f, 1.0f);
		cross_product3d(&forward, &up, &side);
		normalize3d(&side);

		if (random_boolean())
		{
			negate_vector3d(&side, &side);
		}

		origin_angle = real_random_range(-geometry->burst_origin_angle, geometry->burst_origin_angle);
		return_angle = real_random_range(-geometry->burst_return_angle, geometry->burst_return_angle) + origin_angle;
		origin_radius = game_difficulty_get_team_value(_game_difficulty_burst_error_scale, actor->meta.team_index) * geometry->burst_origin_radius;
		return_length = real_random_range(geometry->burst_return_length_lower_bound, geometry->burst_return_length_upper_bound);
		return_length *= game_difficulty_get_team_value(_game_difficulty_burst_error_scale, actor->meta.team_index);

		if (actor->external_orders.playfighting)
		{
			origin_radius *= 2.0f;
			return_length *= 2.0f;
		}

		if (actor->control.fire_state_timer > 0 && geometry->burst_maximum_angular_velocity > 0.0f)
		{
			real maximum_angle = (real)actor->control.fire_state_timer * geometry->burst_maximum_angular_velocity * (1.0f / TICKS_PER_SECOND);
			real maximum_radius;

			maximum_angle = MIN(maximum_angle, _pi / 4);
			maximum_radius = tangent(maximum_angle) * actor->control.current_fire_target_range;

			if (origin_radius > maximum_radius)
			{
				real scale; /* fake name */

				if (origin_radius < maximum_radius * 1.5f)
				{
					scale = origin_radius / maximum_radius;
					actor->control.fire_state_timer = (short)fast_ftol((real)actor->control.fire_state_timer * scale);
				}
				else
				{
					scale = 1.5f;
					actor->control.fire_state_timer = (short)fast_ftol((real)actor->control.fire_state_timer * scale);
					maximum_radius *= scale;
					return_length *= maximum_radius / origin_radius;
					origin_radius = maximum_radius;
				}
			}
		}

		origin_cosine = cosine(origin_angle);
		origin_sine = sine(origin_angle);
		return_cosine = cosine(return_angle);
		return_sine = sine(return_angle);

		offset.i = (side.i * origin_cosine + up.i * origin_sine) * origin_radius;
		offset.j = (side.j * origin_cosine + up.j * origin_sine) * origin_radius;
		offset.k = (side.k * origin_cosine + up.k * origin_sine) * origin_radius;

		adjustment.i = -return_length * (side.i * return_cosine + up.i * return_sine);
		adjustment.j = -return_length * (side.j * return_cosine + up.j * return_sine);
		adjustment.k = -return_length * (side.k * return_cosine + up.k * return_sine);

		if (actor->control.fire_state_timer > 0)
		{
			scale_vector3d(&adjustment, 1.0f / (real)actor->control.fire_state_timer, &adjustment);
		}

		actor->control.burst_initial_position = origin;
		actor->control.burst_relative_position = offset;
		actor->control.burst_adjustment = adjustment;
		point_from_line3d(&actor->control.burst_initial_position, &actor->control.burst_relative_position, 1.0f, &actor->control.burst_target);
	}

	if (actor->state.combat_status >= _actor_combat_status_visible)
	{
		short communication_type;
		boolean ally = FALSE;
		long target_unit_index = NONE;

		if (actor->control.current_fire_target_type == _actor_fire_target_prop)
		{
			struct prop_datum *prop = prop_get(actor->control.current_fire_target_prop_index);

			ally = prop->ally;
			target_unit_index = prop->unit_index;
		}

		if (actor->emotions.berserk)
		{
			communication_type = _ai_communication_shooting_berserk;
		}
		else if (ally)
		{
			communication_type = _ai_communication_shooting_traitor;
		}
		else if (actor->situation.cumulative_threats[_actor_threat_visible] >= 5)
		{
			communication_type = _ai_communication_shooting_group;
		}
		else if (actor->input.vehicle_gunner)
		{
			communication_type = _ai_communication_shooting_vehicle;
		}
		else
		{
			communication_type = _ai_communication_shooting;
		}

		ai_communication_event(communication_type, actor->meta.unit_index, target_unit_index, _comm_hostility_enemy, NONE, NONE, NULL);
	}

	return;
}

static boolean actor_combat_retarget_grenade(
	long actor_index,
	real_point3d const *desired_grenade_target)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	boolean result = FALSE;

	if (actor_combat_check_collateral_damage(
		actor_index,
		variant_definition->grenade_combat.enemy_radius,
		variant_definition->grenade_combat.collateral_damage_radius,
		desired_grenade_target,
		NULL))
	{
		actor->control.grenade_current_target = *desired_grenade_target;
		result = TRUE;
	}

	return result;
}

long actor_aim_grenade(
	long actor_index,
	real_point3d const *origin,
	real_vector3d *vector)
{
	struct actor_datum *actor = actor_get(actor_index);
	long target_unit_index = NONE;
	real_vector3d aim_vector;

	if (actor->control.grenade_current_prop_index != NONE)
	{
		struct prop_datum *prop = prop_get(actor->control.grenade_current_prop_index);

		if (prop->state >= _prop_state_becoming_unacknowledged && prop->state <= _prop_state_acknowledged)
		{
			target_unit_index = prop->unit_index;
		}

		if (prop->state < _prop_state_unacknowledged || prop->state > _prop_state_becoming_acknowledged)
		{
			real_point3d new_desired_target = prop->body_position;

			new_desired_target.z += ACTOR_COMBAT_GRENADE_TARGET_HEIGHT;
			actor_combat_retarget_grenade(actor_index, &new_desired_target);
		}
	}

	actor_combat_reaim_grenade(actor_index, origin);

	/* aim_vector is left uninitialized when the actor is in a vehicle (original behaviour) */
	if (actor->input.vehicle_index == NONE)
	{
		real_vector2d horizontal_aim_vector;

		aim_vector = actor->control.grenade_current_aim_vector;
		horizontal_aim_vector = *(real_vector2d *)&aim_vector;

		if (normalize2d(&horizontal_aim_vector) > 0.0f &&
			dot_product2d(&horizontal_aim_vector, (real_vector2d *)&actor->input.facing_vector) < ACTOR_COMBAT_AIM_ROTATION_COSINE)
		{
			real_vector3d new_aim_vector = actor->input.facing_vector;
			boolean rotate_left = cross_product2d((real_vector2d *)&actor->input.facing_vector, &horizontal_aim_vector) > 0.0f;

			rotate_vector_about_axis(&new_aim_vector, global_up3d, (rotate_left ? 1 : -1) * ACTOR_COMBAT_AIM_ROTATION_SINE, ACTOR_COMBAT_AIM_ROTATION_COSINE);
			scale_vector2d((real_vector2d *)&new_aim_vector, magnitude2d((real_vector2d *)&aim_vector), (real_vector2d *)&new_aim_vector);
			new_aim_vector.k = aim_vector.k;

			match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_combat.c", 1865, &new_aim_vector);

			aim_vector = new_aim_vector;
		}
	}

	scale_vector3d(&aim_vector, actor->control.grenade_current_aim_speed, vector);

	return target_unit_index;
}

void actor_combat_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct actor_variant_definition *variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	struct actor_variant_definition *firing_variant_definition = actor_combat_get_firing_variant_definition(actor_index);
	struct weapon_definition *weapon_definition = actor_get_weapon_definition(actor_index);
	struct unit_datum *unit = unit_get(actor->meta.unit_index);
	short new_state = NONE;
	boolean fire_at_target = FALSE;
	boolean hold_burst_start = FALSE;
	boolean unknown /* fake name; set but never used (also present in Sapien) */ = FALSE;
	long weapon_index = actor_get_weapon(actor_index);
	struct actor_debug_info *debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
	short firing_decision = NONE;
	boolean fire_primary;
	boolean fire_secondary;
	boolean primary_trigger;
	boolean secondary_trigger;
	real analog_primary_trigger;

	if (actor->control.fire_state_timer > 0)
	{
		actor->control.fire_state_timer--;
	}

	if (actor->control.burst_disable_timer > 0)
	{
		actor->control.burst_disable_timer--;
	}

	if (actor->control.trigger_delay_timer > 0)
	{
		actor->control.trigger_delay_timer--;
	}

	if (actor->control.special_fire_delay > 0)
	{
		actor->control.special_fire_delay--;
	}

	if (actor->control.current_fire_target_type > _actor_fire_target_none)
	{
		actor->control.current_fire_target_timer++;
	}

	if (actor->control.fire_state != _actor_fire_state_bursting)
	{
		short new_fire_target_type = _actor_fire_target_none;
		boolean new_target = FALSE;

		if (actor->orders.combat.shoot_at_target)
		{
			if (actor->orders.combat.use_manual_target_point)
			{
				new_fire_target_type = _actor_fire_target_manual_point;
			}
			else if (actor->target.target_prop_index != NONE)
			{
				new_fire_target_type = _actor_fire_target_prop;
			}
		}

		if (new_fire_target_type != actor->control.current_fire_target_type)
		{
			new_target = TRUE;
		}
		else if (new_fire_target_type == _actor_fire_target_prop)
		{
			new_target = actor->target.target_prop_index != actor->control.current_fire_target_prop_index;
		}
		else if (new_fire_target_type == _actor_fire_target_manual_point)
		{
			new_target = distance_squared3d(&actor->orders.combat.target_point, &actor->control.current_fire_target_manual_point) >
				ACTOR_COMBAT_FIRE_TARGET_MOVED_DISTANCE * ACTOR_COMBAT_FIRE_TARGET_MOVED_DISTANCE;
		}

		if (new_target)
		{
			actor->control.current_fire_target_timer = 0;
		}

		actor->control.current_fire_target_type = new_fire_target_type;

		if (new_fire_target_type == _actor_fire_target_prop)
		{
			actor->control.current_fire_target_prop_index = actor->target.target_prop_index;
			match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 133, actor->control.current_fire_target_prop_index != NONE);
		}
		else if (new_fire_target_type == _actor_fire_target_manual_point)
		{
			actor->control.current_fire_target_manual_point = actor->orders.combat.target_point;
		}
	}

	actor->control.aiming_at_fire_target = FALSE;
	actor->control.weapon_maximum_range = actor_has_ranged_weapon(actor_index) ? firing_variant_definition->ranged_combat.maximum_firing_range : 0.0f;

	if (actor->orders.combat.throw_grenade)
	{
		if (variant_definition->grenade_combat.grenade_type != NONE &&
			unit_get_grenade_count(actor->meta.unit_index, variant_definition->grenade_combat.grenade_type) == 0)
		{
			unit_add_grenade_type_to_inventory(actor->meta.unit_index, variant_definition->grenade_combat.grenade_type, 1);
		}

		actor_unit_control_throw_grenade(actor_index);
		ai_communication_event(_ai_communication_grenade_throwing, actor->meta.unit_index, NONE, NONE, NONE, NONE, NULL);
		fire_at_target = FALSE;
	}
	else if (!actor_has_ranged_weapon(actor_index))
	{
		fire_at_target = FALSE;
	}
	else if (actor->control.fire_state == _actor_fire_state_wild)
	{
		fire_at_target = TRUE;
	}
	else
	{
		boolean weapon_linear = TRUE;

		match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 173, weapon_index != NONE);

		if (firing_variant_definition->ranged_combat.special_fire_mode > _actor_special_fire_mode_none)
		{
			real special_fire_chance = firing_variant_definition->ranged_combat.special_fire_chance;
			real special_fire_delay = firing_variant_definition->ranged_combat.special_fire_delay;
			boolean special_fire_possible = TRUE;

			if (actor->control.fire_state == _actor_fire_state_bursting || actor->control.fire_state == _actor_fire_state_wild)
			{
				special_fire_possible = FALSE;
			}
			else if (actor->control.special_fire_delay > 0 || actor->control.special_fire_deny_attempts > 0)
			{
				special_fire_possible = FALSE;
			}

			if (special_fire_possible)
			{
				struct weapon_datum *weapon = weapon_get(weapon_index);
				struct weapon_definition *special_weapon_definition = weapon_definition_get(weapon->definition_index);

				special_fire_delay *= game_difficulty_get_team_value(_game_difficulty_special_fire_delay_scale, actor->meta.team_index);

				switch (firing_variant_definition->ranged_combat.special_fire_mode)
				{
				case _actor_special_fire_mode_overcharge:
					special_fire_chance *= game_difficulty_get_team_value(_game_difficulty_overcharge_chance_scale, actor->meta.team_index);

					if (special_weapon_definition->weapon.triggers.count > 0)
					{
						struct weapon_trigger_definition *trigger = TAG_BLOCK_GET_ELEMENT(&special_weapon_definition->weapon.triggers, 0, struct weapon_trigger_definition);
					}
					else
					{
						special_fire_possible = FALSE;
					}
					break;

				case _actor_special_fire_mode_secondary:
					special_fire_possible = special_weapon_definition->weapon.triggers.count > 1;
					break;
				}
			}

			if (special_fire_possible)
			{
				special_fire_possible = actor_combat_enable_special_fire_situation(actor_index, firing_variant_definition->ranged_combat.special_fire_situation);
			}

			if (special_fire_possible)
			{
				real delay /* fake name */ = real_random_range(0.0f, 1.5f) + firing_variant_definition->ranged_combat.special_fire_delay;
				real chance /* fake name */ = real_random();

				actor->control.special_fire_delay = (short)(delay * TICKS_PER_SECOND);
				special_fire_possible = chance < firing_variant_definition->ranged_combat.special_fire_chance;
			}

			if (special_fire_possible)
			{
				special_fire_possible = actor_combat_allow_special_fire_situation(actor_index, firing_variant_definition->ranged_combat.special_fire_situation);
			}

			if (special_fire_possible)
			{
				if (firing_variant_definition->ranged_combat.special_fire_situation == _actor_special_fire_situation_strafing)
				{
					actor->control.special_fire_deny_attempts = ACTOR_COMBAT_SPECIAL_FIRE_DENY_ATTEMPTS;
				}

				switch (firing_variant_definition->ranged_combat.special_fire_mode)
				{
				case _actor_special_fire_mode_overcharge:
					actor->control.overcharging_weapon = TRUE;
					break;

				case _actor_special_fire_mode_secondary:
					actor->control.next_burst_secondary = TRUE;
					break;
				}
			}
		}

		if (actor->control.current_fire_target_type > _actor_fire_target_none)
		{
			if (actor->control.current_fire_target_type == _actor_fire_target_prop)
			{
				struct prop_datum *prop = prop_get(actor->control.current_fire_target_prop_index);
				unsigned long const *combined_pvs;

				actor->control.current_fire_target_range = prop->distance;
				actor->control.current_fire_target_position = prop->center_of_mass;
				actor->control.current_fire_target_line_of_sight = prop->line_of_sight;
				actor->control.current_fire_target_underwater = prop->underwater;
				combined_pvs = players_get_combined_pvs();
				actor->control.current_fire_target_outside_active_area = TRUE;

				if (prop->body_location.cluster_index != NONE)
				{
					actor->control.current_fire_target_outside_active_area = !BIT_VECTOR_TEST_FLAG(combined_pvs, prop->body_location.cluster_index);
				}
			}
			else
			{
				match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 296, actor->control.current_fire_target_type == _actor_fire_target_manual_point);

				actor->control.current_fire_target_position = actor->control.current_fire_target_manual_point;
				actor->control.current_fire_target_range = distance3d(&actor->input.position.head_position, &actor->control.current_fire_target_manual_point);
				actor->control.current_fire_target_underwater = FALSE;
				actor->control.current_fire_target_outside_active_area = FALSE;

				if (actor->control.current_fire_target_timer % ACTOR_COMBAT_MANUAL_TARGET_LINE_OF_SIGHT_TICKS == 0)
				{
					actor->control.current_fire_target_line_of_sight = ai_test_line_of_sight(
						&actor->input.position.head_position,
						actor->input.position.body_location.cluster_index,
						&actor->control.current_fire_target_position,
						NONE,
						0,
						FALSE,
						NONE,
						actor->input.vehicle_index != NONE);
				}
			}

			actor->control.current_fire_target_superballistic = FALSE;

			if (firing_variant_definition->ranged_combat.weapon_super_ballistic_range > 0.0f &&
				actor->control.current_fire_target_range > firing_variant_definition->ranged_combat.weapon_super_ballistic_range)
			{
				actor->control.current_fire_target_superballistic = TRUE;
			}

			actor->control.current_fire_target_bombardment = actor->orders.combat.bombard_target &&
				firing_variant_definition->ranged_combat.weapon_bombardment_range > 0.0f;

			if (!weapon_aim(
				weapon_index,
				0,
				&actor->input.position.head_position,
				&actor->control.current_fire_target_position,
				actor->control.current_fire_target_superballistic,
				&actor->control.current_fire_target_aim_vector,
				NULL,
				&actor->control.current_fire_target_distance,
				&weapon_linear))
			{
				actor->control.current_fire_target_type = _actor_fire_target_none;
			}
		}

		if (actor->control.current_fire_target_type == _actor_fire_target_none)
		{
			fire_at_target = FALSE;
			firing_decision = _firing_no_target;
		}
		else if (actor->control.current_fire_target_outside_active_area)
		{
			fire_at_target = FALSE;
			firing_decision = _firing_outside_active_area;
		}
		else if (!actor->orders.combat.override_firing_restrictions && actor->control.burst_disable_timer > 0)
		{
			fire_at_target = FALSE;
			firing_decision = _firing_disabled;
		}
		else if (actor_move_animation_busy(actor_index))
		{
			fire_at_target = FALSE;
			firing_decision = _firing_busy;
		}
		else if (!actor->orders.combat.override_firing_restrictions &&
			actor->input.in_midair &&
			!actor->state.flying &&
			!TEST_FLAG(variant_definition->flags, _actor_variant_definition_can_shoot_while_flying_bit))
		{
			fire_at_target = FALSE;
			firing_decision = _firing_not_in_midair;
		}
		else if (!actor->orders.combat.override_firing_restrictions &&
			TEST_FLAG(actor_definition->flags, _actor_definition_must_crouch_to_fire_bit) &&
			!actor->control.crouching)
		{
			fire_at_target = FALSE;
			firing_decision = _firing_not_crouching;
		}
		else if (!actor->orders.combat.override_firing_restrictions &&
			TEST_FLAG(actor_definition->flags2, _actor_definition_flags2_must_stand_to_fire_bit) &&
			actor->control.crouching)
		{
			fire_at_target = FALSE;
			firing_decision = _firing_not_standing;
		}
		else if (!actor->orders.combat.override_firing_restrictions &&
			TEST_FLAG(actor_definition->flags2, _actor_definition_flags2_must_stop_to_fire_bit) &&
			actor->control.moving)
		{
			fire_at_target = FALSE;
			firing_decision = _firing_not_stationary;
		}
		else if (actor->control.current_fire_target_underwater || actor->input.underwater)
		{
			fire_at_target = FALSE;
			firing_decision = _firing_underwater;
		}
		else if (weapon_definition &&
			weapon_definition->weapon.ai_minimum_target_range > 0.0f &&
			actor->control.current_fire_target_range < weapon_definition->weapon.ai_minimum_target_range)
		{
			fire_at_target = FALSE;
			firing_decision = _firing_min_range;
		}
		else if (actor->orders.look.primary_priority == _primary_priority_none ||
			actor->orders.look.primary_direction.type != _direction_specification_target ||
			actor->control.aiming_away_from_primary)
		{
			fire_at_target = FALSE;
			firing_decision = _firing_wrong_target;
		}
		else if (actor->control.fire_state == _actor_fire_state_bursting)
		{
			fire_at_target = TRUE;
		}
		else
		{
			actor->control.current_fire_target_visible =
				actor->control.current_fire_target_line_of_sight == _ai_line_of_sight_clear ||
				actor->control.current_fire_target_line_of_sight == _ai_line_of_sight_occluded;

			if (!actor->control.current_fire_target_visible && !actor->control.current_fire_target_bombardment)
			{
				fire_at_target = FALSE;
				firing_decision = _firing_not_visible;
			}
			else if (!actor->orders.combat.override_firing_restrictions &&
				actor->control.current_fire_target_range >= actor->control.weapon_maximum_range)
			{
				fire_at_target = FALSE;
				firing_decision = _firing_outside_range;
			}
			else
			{
				fire_at_target = TRUE;
				actor->control.aiming_at_fire_target = TRUE;
				debug_info->fire_alignment_time = game_time_get();

				if (TEST_FLAG(actor_definition->flags, _actor_definition_start_firing_before_aligned_bit))
				{
					debug_info->fire_alignment_aligned = TRUE;
					debug_info->fire_alignment_ignored = TRUE;
				}
				else
				{
					real_vector3d weapon_vector;
					real alignment_threshold;
					real alignment;

					if (actor->control.current_fire_target_range < 1.5f)
					{
						alignment_threshold = actor->control.current_fire_target_range * 0.17526217f + 0.70710677f;
					}
					else
					{
						alignment_threshold = 0.97f;
					}

					actor_get_weapon_vector(actor_index, &weapon_vector);
					alignment = dot_product3d(&actor->control.current_fire_target_aim_vector, &weapon_vector);

					if (alignment < alignment_threshold)
					{
						hold_burst_start = TRUE;
					}

					debug_info->fire_alignment_weapon_vector = weapon_vector;
					debug_info->fire_alignment_aim_vector = actor->control.current_fire_target_aim_vector;
					debug_info->fire_alignment_dot_product = alignment;
					debug_info->fire_alignment_threshold = alignment_threshold;

					debug_info->fire_alignment_aligned = alignment >= alignment_threshold;
					debug_info->fire_alignment_ignored = FALSE;
				}
			}
		}
	}

	if (!fire_at_target)
	{
		actor->control.fire_state = _actor_fire_state_none;
	}

	if (actor->control.fire_state != _actor_fire_state_none)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 517, firing_decision == NONE);

		switch (actor->control.fire_state)
		{
		case _actor_fire_state_wild:
			firing_decision = _firing_wild;
			break;

		case _actor_fire_state_holding:
			firing_decision = actor->control.fire_state_timer ? _firing_holding : _firing_holding_for_line;
			break;

		case _actor_fire_state_bursting:
			firing_decision = _firing_burst;
			break;

		case _actor_fire_state_pausing:
			firing_decision = actor->control.fire_state_timer ? _firing_pausing : _firing_pausing_for_line;
			break;

		default:
			match_halt("c:\\halo\\SOURCE\\ai\\actor_combat.c", 538);
			break;
		}
	}

	switch (actor->control.fire_state)
	{
	case _actor_fire_state_none:
		if (fire_at_target)
		{
			new_state = _actor_fire_state_holding;
		}
		break;

	case _actor_fire_state_holding:
		if (!hold_burst_start && actor->control.fire_state_timer == 0)
		{
			new_state = _actor_fire_state_bursting;
		}
		break;

	case _actor_fire_state_bursting:
		if (actor->control.fire_state_timer == 0)
		{
			new_state = _actor_fire_state_pausing;
		}
		break;

	case _actor_fire_state_pausing:
		if (!hold_burst_start && actor->control.fire_state_timer == 0)
		{
			new_state = _actor_fire_state_bursting;
		}
		break;

	case _actor_fire_state_wild:
		if (actor->control.fire_state_timer == 0)
		{
			new_state = _actor_fire_state_none;
		}
		break;

	default:
		match_halt("c:\\halo\\SOURCE\\ai\\actor_combat.c", 582);
		break;
	}

	if (new_state != NONE)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 588, new_state!=actor->control.fire_state);

		switch (new_state)
		{
		case _actor_fire_state_holding:
		{
			boolean delayed /* fake name */ = actor_start_first_burst_delay(actor_index, firing_variant_definition);

			if (!delayed && !hold_burst_start)
			{
				new_state = _actor_fire_state_bursting;
				actor_start_burst(actor_index);
			}
			break;
		}

		case _actor_fire_state_bursting:
			match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 611, !hold_burst_start);
			actor_start_burst(actor_index);
			break;

		case _actor_fire_state_pausing:
			actor_start_pause(actor_index);
			break;
		}

		actor->control.fire_state = new_state;
	}

	fire_primary = FALSE;
	fire_secondary = FALSE;
	actor->control.burst_aim_by_vector = FALSE;

	if (actor->control.fire_state == _actor_fire_state_wild)
	{
		fire_primary = TRUE;
	}
	else if (actor->control.fire_state == _actor_fire_state_bursting)
	{
		long ignore_unit_index = NONE;
		long blocking_prop_index;
		real_point3d gun_position;
		real_vector3d fire_vector;

		match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 640, weapon_index != NONE);

		actor->control.burst_origin = actor->control.burst_initial_position;
		debug_info->burst_tracked_position = actor->control.burst_origin;
		debug_info->burst_lead_vector = *global_zero_vector3d;

		if (actor->control.current_fire_target_type == _actor_fire_target_prop)
		{
			struct prop_datum *prop = prop_get(actor->control.current_fire_target_prop_index);
			real track_fraction;
			real_vector3d vector;
			real lead_fraction;

			debug_info->burst_last_known_position = prop->center_of_mass;
			ignore_unit_index = prop->attached_to_unit_index;

			track_fraction = firing_variant_definition->ranged_combat.target_track_fraction;
			track_fraction += game_difficulty_get_team_value(_game_difficulty_target_tracking_bonus, actor->meta.team_index);
			track_fraction = MIN(1.0f, track_fraction);

			if (track_fraction > 0.0f && !actor->control.current_fire_target_bombardment)
			{
				vector_from_points3d(&actor->control.burst_initial_position, &prop->center_of_mass, &vector);
				point_from_line3d(&actor->control.burst_origin, &vector, firing_variant_definition->ranged_combat.target_track_fraction, &actor->control.burst_origin);
			}

			debug_info->burst_tracked_position = actor->control.burst_origin;

			lead_fraction = firing_variant_definition->ranged_combat.target_lead_fraction;
			lead_fraction += game_difficulty_get_team_value(_game_difficulty_target_leading_bonus, actor->meta.team_index);
			lead_fraction = MIN(1.0f, lead_fraction);

			if (lead_fraction > 0.0f)
			{
				real time_to_target = weapon_estimate_time_to_target(weapon_index, actor->control.fire_burst_secondary ? 1 : 0, actor->control.current_fire_target_distance);
				real_vector3d lead_vector;

				scale_vector3d(&prop->velocity, time_to_target, &lead_vector);
				debug_info->burst_lead_vector = lead_vector;
				point_from_line3d(&actor->control.burst_origin, &lead_vector, firing_variant_definition->ranged_combat.target_lead_fraction, &actor->control.burst_origin);
			}
		}
		else
		{
			match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 707, actor->control.current_fire_target_type == _actor_fire_target_manual_point);
			debug_info->burst_last_known_position = actor->control.burst_origin;
		}

		add_vectors3d(&actor->control.burst_adjustment, &actor->control.burst_relative_position, &actor->control.burst_relative_position);
		point_from_line3d(&actor->control.burst_origin, &actor->control.burst_relative_position, 1.0f, &actor->control.burst_target);

		if (actor->input.vehicle_index != NONE)
		{
			unit_get_camera_position(actor->meta.unit_index, &gun_position);
		}
		else
		{
			real_vector3d *gun_offset = NULL;

			if (actor->control.crouching)
			{
				if (magnitude_squared3d(&firing_variant_definition->ranged_combat.gun_offset_crouch) > ACTOR_COMBAT_GUN_OFFSET_EPSILON)
				{
					gun_offset = &firing_variant_definition->ranged_combat.gun_offset_crouch;
				}
				else if (magnitude_squared3d(&actor_definition->perception.gun_offset_crouch) > ACTOR_COMBAT_GUN_OFFSET_EPSILON)
				{
					gun_offset = &actor_definition->perception.gun_offset_crouch;
				}
			}
			else
			{
				if (magnitude_squared3d(&firing_variant_definition->ranged_combat.gun_offset_stand) > ACTOR_COMBAT_GUN_OFFSET_EPSILON)
				{
					gun_offset = &firing_variant_definition->ranged_combat.gun_offset_stand;
				}
				else if (magnitude_squared3d(&actor_definition->perception.gun_offset_stand) > ACTOR_COMBAT_GUN_OFFSET_EPSILON)
				{
					gun_offset = &actor_definition->perception.gun_offset_stand;
				}
			}

			if (!gun_offset)
			{
				gun_position = actor->input.position.head_position;
			}
			else
			{
				real_vector3d horizontal_aiming_vector;

				vector_from_points3d(&actor->control.burst_target, &actor->input.position.body_position, &horizontal_aiming_vector);

				if (normalize2d((real_vector2d *)&horizontal_aiming_vector) > 0.0f)
				{
					horizontal_aiming_vector.k = 0.0f;
				}
				else
				{
					horizontal_aiming_vector = actor->input.facing_vector;
				}

				unit_estimate_position(
					actor->meta.unit_index,
					_unit_estimate_gun_position,
					&actor->input.position.body_position,
					&horizontal_aiming_vector,
					gun_offset,
					&gun_position);
			}
		}

		{
		boolean aim_linear;
		boolean aiming_success;

			aiming_success = weapon_aim(
				weapon_index,
				actor->control.fire_burst_secondary ? 1 : 0,
				&gun_position,
				&actor->control.burst_target,
				actor->control.current_fire_target_superballistic,
				&actor->control.burst_aim_vector,
				NULL,
				NULL,
				&aim_linear);
			match_assert("c:\\halo\\SOURCE\\ai\\actor_combat.c", 784, aiming_success);
			match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_combat.c", 785, &actor->control.burst_aim_vector);

			actor->control.burst_aim_by_vector = !aim_linear;
			vector_from_points3d(&gun_position, &actor->control.burst_target, &fire_vector);

			if (ai_test_line_of_fire(actor_index, ignore_unit_index, &gun_position, &fire_vector, &blocking_prop_index))
			{
				actor->control.blocked_communication_timer = 0;

				if (actor->control.fire_burst_secondary)
				{
					fire_secondary = TRUE;
				}
				else
				{
					fire_primary = TRUE;
				}
			}
			else
			{
				actor->control.blocked_communication_timer++;
				actor->control.fire_state_timer++;
				firing_decision = _firing_blocked;

				if (actor->control.blocked_communication_timer >= ACTOR_COMBAT_BLOCKED_COMMUNICATION_TICKS &&
					actor->target.target_type >= _actor_target_acknowledged_enemy)
				{
					long blocking_unit_index = NONE;

					if (blocking_prop_index != NONE)
					{
						struct prop_datum *prop = prop_get(blocking_prop_index);

						blocking_unit_index = prop->unit_index;
					}

					ai_communication_event(_ai_communication_blocked, actor->meta.unit_index, blocking_unit_index, _comm_hostility_friend, NONE, NONE, NULL);
					actor->control.blocked_communication_timer = 0;
				}
			}
		}
	}

	primary_trigger = FALSE;
	secondary_trigger = FALSE;
	analog_primary_trigger = 0.0f;

	if (fire_primary)
	{
		real rate_of_fire = firing_variant_definition->ranged_combat.rate_of_fire;

		if (actor->control.overcharging_weapon)
		{
			actor->control.overcharging_weapon = FALSE;
		}
		else if (rate_of_fire == 0.0f)
		{
			primary_trigger = TRUE;
			analog_primary_trigger = 1.0f;
		}
		else if (actor->control.trigger_delay_timer == 0)
		{
			struct actor_burst_geometry *burst_geometry;
			struct actor_firing_pattern *firing_pattern;
			short trigger_delay;

			primary_trigger = TRUE;
			analog_primary_trigger = 1.0f;
			rate_of_fire *= game_difficulty_get_team_value(_game_difficulty_rate_of_fire_scale, actor->meta.team_index);
			actor_combat_get_firing_parameters(actor_index, firing_variant_definition, &burst_geometry, &firing_pattern);

			if (firing_pattern && firing_pattern->rate_of_fire_modifier > 0.0f)
			{
				rate_of_fire *= firing_pattern->rate_of_fire_modifier;
			}

			debug_info->shooting_rof = rate_of_fire;
			trigger_delay = (short)fast_ftol(TICKS_PER_SECOND / rate_of_fire);
			actor->control.trigger_delay_timer = MAX(ACTOR_COMBAT_MINIMUM_TRIGGER_DELAY_TICKS, trigger_delay);
		}
	}
	else if (fire_secondary)
	{
		if (actor->control.overcharging_weapon)
		{
			actor->control.overcharging_weapon = FALSE;
		}
		else
		{
			secondary_trigger = TRUE;
		}
	}
	else if (actor->control.overcharging_weapon)
	{
		primary_trigger = TRUE;
		analog_primary_trigger = 1.0f;
	}

	actor_unit_control_primary_trigger(actor_index, primary_trigger, analog_primary_trigger);
	actor_unit_control_secondary_trigger(actor_index, secondary_trigger);
	debug_info->firing_decision = firing_decision;

	return;
}

/* ---------- private code */
