/*
ACTOR_MOVING.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "console.h"
#include "collisions.h"
#include "vehicles.h"
#include "bipeds.h"
#include "physics_constants.h"
#include "collision_model_definitions.h"

/* ---------- constants */

enum
{
	VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS = 8,
	VECTOR_AVOIDANCE_NUMBER_OF_SENSE_RAYS = 9, /* fake name */
	VECTOR_AVOIDANCE_NUMBER_OF_AVOID_RAYS = 2, /* fake name */
};

enum
{
	_vector_avoidance_decision_none = 0,				/* fake name */
	_vector_avoidance_decision_turn,					/* fake name */
	_vector_avoidance_decision_sign_no_danger,			/* fake name */
	_vector_avoidance_decision_sign_too_far,			/* fake name */
	_vector_avoidance_decision_sign_rotated,			/* fake name */
	_vector_avoidance_decision_sharp_turn_stationary,	/* fake name */
	_vector_avoidance_decision_sharp_turn_moving,		/* fake name */
	_vector_avoidance_decision_continue_sharp_turn,		/* fake name */
};

#define _full_circle (_pi * 2.f)

/* ---------- structures */

struct vector_avoidance_ray
{
	real length;
	real_vector3d offset;
	real_vector3d divergence;
};

/* ---------- prototypes */

static void actor_move_calculate_movement(long actor_index, boolean move_in_3d, short override_facing, boolean free_movement, real free_movement_dist_sq, boolean allow_all_moving_turns, real steering_maximum_angle, real oversteer_minimum_angle, real oversteer_maximum_angle, real rotation_emergency_amount, real maximum_throttle, real_vector3d const *desired_movement_vector, real_vector3d *desired_facing_vector, short *desired_facing_direction, real_vector3d *desired_throttle, boolean *movement_thwarted, boolean *movement_complete);
static void actor_move_calculate_controlled_by_aiming(boolean move_in_3d, real_vector3d const *movement_vector, real_vector3d const *forced_aim_direction, real_vector3d const *current_facing_vector, real_vector3d *desired_facing_vector, short *desired_facing_direction);
static void actor_move_calculate_free(boolean move_in_3d, real_vector3d const *movement_direction, real_vector3d const *facing_direction, real_vector3d *throttle);
static void actor_move_avoidance_setup(struct vector_avoidance_data *avoidance_data);
static short actor_move_test_avoidance_vector(struct vector_avoidance_data *avoidance_data, struct vector_avoidance_ray const *avoidance_ray, real_point3d *ray_origin, real_vector3d *ray_direction, real *collision_t, byte *collision_timer);
static void actor_move_vector_avoidance(long actor_index, real_vector3d const *movement_direction, real_vector3d *avoidance_rotation, real *emergency_amount);
static boolean actor_move_vector_avoidance_find_direction(short direction_count, real_vector3d const *avoidance_directions, real const *avoidance_weights, real_vector3d const *direction_vector, real *approximate_direction_reference, real *approximate_weight_reference);

/* ---------- globals */

const real sense_ray_length = 1.f;
const real sense_ray_offset = 0.7f;
const real sense_ray_divergence = DEGREES_TO_RADIANS(3.f);

const real sense_ray_offsets[VECTOR_AVOIDANCE_NUMBER_OF_SENSE_RAYS] = { 0.f, 0.5f, 0.5f, 0.5f, 0.5f, 1.f, 1.f, 1.f, 1.f };
const real sense_ray_divergences[VECTOR_AVOIDANCE_NUMBER_OF_SENSE_RAYS] = { 0.f, 0.3f, 0.3f, 0.3f, 0.3f, 1.f, 1.f, 1.f, 1.f };
const real sense_ray_angles[VECTOR_AVOIDANCE_NUMBER_OF_SENSE_RAYS] = { 0.f, 0.f, _pi / 2.f, _pi, 3.f * _pi / 2.f, 0.f, _pi / 2.f, _pi, 3.f * _pi / 2.f };

const real avoidance_ray_length = 0.7f;
const real avoidance_ray_angles[VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS] = { 0.f, _pi / 4.f, _pi / 2.f, 3.f * _pi / 4.f, _pi, 5.f * _pi / 4.f, 3.f * _pi / 2.f, 7.f * _pi / 4.f };
const real avoidance_ray_offsets[VECTOR_AVOIDANCE_NUMBER_OF_AVOID_RAYS] = { 0.7f, 1.f };
const real avoidance_ray_divergence[3] = { DEGREES_TO_RADIANS(30.f), DEGREES_TO_RADIANS(55.f), 0.f };

const real sense_ray_avoidance_weights[VECTOR_AVOIDANCE_NUMBER_OF_SENSE_RAYS][VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS] =
{
	{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
	{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
	{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
	{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
	{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
	{ -0.5f, -0.5f, 0.f, 0.7f, 1.f, 0.7f, 0.f, -0.5f },
	{ 0.f, -0.5f, -0.5f, -0.5f, 0.f, 0.7f, 1.f, 0.7f },
	{ 1.f, 0.7f, 0.f, -0.5f, -0.5f, -0.5f, 0.f, 0.7f },
	{ 0.f, 0.7f, 1.f, 0.7f, 0.f, -0.5f, -0.5f, -0.5f },
};

const real avoid_ray_avoidance_weights[VECTOR_AVOIDANCE_NUMBER_OF_AVOID_RAYS] = { 0.8f, 1.2f };
const short avoid_ray_clear_bias_time = 75;
const real avoid_ray_adjacent_fractions[2] = { 0.8f, 0.5f };
const real avoid_ray_fully_obstructed_t = 0.5f;

struct vector_avoidance_ray sense_rays[VECTOR_AVOIDANCE_NUMBER_OF_SENSE_RAYS];
real_vector3d avoidance_directions[VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS];
struct vector_avoidance_ray avoidance_rays[VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS][VECTOR_AVOIDANCE_NUMBER_OF_AVOID_RAYS];

/* ---------- public code */

void actor_destination_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.timeslice && !actor->control.path.refreshed_this_tick && !actor->meta.dormant)
	{
		actor_path_refresh(actor_index, FALSE, NULL);
	}

	actor_test_destination(actor_index);

	if (actor->control.path.path.valid)
	{
		boolean advance;
		struct path_result *path = &actor->control.path.path;
		boolean final_step = FALSE;

		do
		{
			real_vector2d actor_to_point;
			real_vector2d next_step;

			if (path->step_index + 1>=path->step_count)
			{
				final_step = TRUE;
				break;
			}

			vector_from_points2d(
				(real_point2d *)&actor->input.position.body_position,
				(real_point2d *)&path->steps[path->step_index].point,
				&actor_to_point);
			vector_from_points2d(
				(real_point2d *)&path->steps[path->step_index].point,
				(real_point2d *)&path->steps[path->step_index + 1].point,
				&next_step);
			advance = FALSE;

			if (actor->control.movement_complete)
			{
				advance = TRUE;
			}
			else if (actor->control.moving && actor->control.movement_thwarted)
			{
				real facing_dot = dot_product2d((real_vector2d *)&actor->input.facing_vector, &next_step);
				real offset_dot = dot_product2d(&actor_to_point, &next_step);

				if (facing_dot>0.f && offset_dot<0.f)
				{
					point_from_line2d((real_point2d *)&actor_to_point, &next_step, -offset_dot, (real_point2d *)&actor_to_point);

					if (magnitude_squared2d(&actor_to_point)<0.0625f)
					{
						advance = TRUE;
					}
				}
			}
			else if (magnitude_squared2d(&actor_to_point)<0.0225f)
			{
				advance = TRUE;
			}

			if (advance)
			{
				path->step_index++;
				actor->control.movement_complete = FALSE;
			}
		} while (advance);

		if (actor->control.movement_complete)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 180, final_step);

			if (actor->control.path.path.steps_finish_path)
			{
				actor_path_clear(actor_index);
			}
#ifdef DEBUG
			else if (ai_debug.print_unfinished_paths)
			{
				char actorbuf[512];

				ai_debug_describe_actor(actor_index, NONE, TRUE, actorbuf, sizeof(actorbuf));
				error(_error_silent, "%s: fell off end of unfinished path %d/%d", actorbuf, actor->control.path.path.step_count, MAXIMUM_SMOOTHED_PATH_STEPS);
			}
#endif
		}
	}

	if (actor->control.path.path.valid && (actor->control.moving || !actor->control.path.at_destination))
	{
		actor->control.moving = TRUE;
		actor->control.moving_towards_point = actor->control.path.path.steps[actor->control.path.path.step_index].point;
		vector_from_points3d(&actor->input.position.body_position, &actor->control.moving_towards_point, &actor->control.moving_towards_vector);

		if (magnitude3d(&actor->control.moving_towards_vector)>=1000000.f)
		{
			error(_error_silent, "pathfinding is attempting to walk to tau ceti");
			actor->control.path.path.valid = FALSE;
		}
	}
	else if (actor->input.vehicle_driver_type==_actor_vehicle_driver_directional_flying)
	{
		boolean reverse = actor->control.vector_avoidance_rotation_emergency_instantaneous>0.9f;

		actor->control.moving = TRUE;
		actor->control.movement_complete = FALSE;
		scale_vector3d(&actor->input.facing_vector, (reverse ? -1 : 1) * 3.f, &actor->control.moving_towards_vector);
		point_from_line3d(&actor->input.position.body_position, &actor->control.moving_towards_vector, 1.f, &actor->control.moving_towards_point);
	}
	else
	{
		actor->control.moving = FALSE;
		actor->control.movement_complete = FALSE;
		actor->control.path.at_destination = TRUE;
		actor_path_clear(actor_index);
	}

	return;
}

boolean actor_test_destination(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->control.path.destination_orders.destination_type==_destination_none ||
		actor->control.path.destination_orders.destination_type==_destination_halt)
	{
		actor->control.path.at_destination = TRUE;
	}
	else
	{
		real tolerance = actor_destination_tolerance(actor_index);

		if (distance_squared3d(&actor->input.position.body_position, &actor->control.path.destination.point)<tolerance * tolerance)
		{
			actor->control.path.at_destination = TRUE;
		}
	}

	return actor->control.path.at_destination;
}

void actor_move_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean crouch = FALSE;
	boolean free_movement = FALSE;
	boolean move_in_3d = FALSE;
	boolean allow_all_moving_turns = FALSE;
	boolean normal_movement = FALSE;
	boolean face_current_direction = FALSE;
	boolean jump = FALSE;
	short override_facing;
	real free_movement_dist_sq = 0.f;
	real steering_maximum_angle = 0.f;
	real oversteer_maximum_angle = 0.f;
	real oversteer_minimum_angle = 0.f;
	real rotation_emergency_amount = 0.f;
	real maximum_throttle = 1.f;

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_moving.c", 286, &actor->input.facing_vector);

	actor->control.face_exactly = FALSE;
	actor->control.free_facing_vector = TRUE;
	actor->control.optional_facing_vector = TRUE;
	actor->control.desired_facing_vector = actor->input.facing_vector;

	if (actor->orders.move.override_movement_direction)
	{
		actor->control.moving_towards_vector = actor->orders.move.override_movement_direction_vector;
		actor->control.moving = TRUE;
		actor->control.free_facing_vector = FALSE;
		actor->control.vector_avoidance_rotation = *global_zero_vector3d;
		actor->control.vector_avoidance_rotation_emergency = 0.f;
		actor->control.vector_avoidance_rotation_emergency_instantaneous = 0.f;
	}
	else if (actor->input.vehicle_driver_type==_actor_vehicle_driver_directional_flying)
	{
		real_vector3d const *movement_vector;
		real_vector3d fake_movement_vector;
		real_vector3d instantaneous_rotation_vector;
		real instantaneous_rotation_emergency;
		real blend;

		if (actor->control.moving)
		{
			movement_vector = &actor->control.moving_towards_vector;
		}
		else
		{
			scale_vector3d(&actor->input.facing_vector, 3.f, &fake_movement_vector);
			movement_vector = &fake_movement_vector;
		}

		actor_move_vector_avoidance(actor_index, movement_vector, &instantaneous_rotation_vector, &instantaneous_rotation_emergency);

		if (magnitude_squared3d(&instantaneous_rotation_vector)>magnitude_squared3d(&actor->control.vector_avoidance_rotation))
		{
			blend = 0.3f;
		}
		else
		{
			blend = 0.05f;
		}

		scale_vector3d(&actor->control.vector_avoidance_rotation, 1.f - blend, &actor->control.vector_avoidance_rotation);
		vector_from_line3d(&actor->control.vector_avoidance_rotation, &instantaneous_rotation_vector, blend, &actor->control.vector_avoidance_rotation);

		if (magnitude_squared3d(&actor->control.vector_avoidance_rotation)<_real_epsilon)
		{
			actor->control.vector_avoidance_rotation = *global_zero_vector3d;
		}

		actor->control.vector_avoidance_rotation_emergency_instantaneous = instantaneous_rotation_emergency;
		actor->control.vector_avoidance_rotation_emergency = (1.f - blend) * actor->control.vector_avoidance_rotation_emergency + instantaneous_rotation_emergency * blend;

		if (actor->control.vector_avoidance_rotation_emergency<0.001f)
		{
			actor->control.vector_avoidance_rotation_emergency = 0.f;
		}

		if (actor->control.moving)
		{
			real_vector3d rotation_vector = actor->control.vector_avoidance_rotation;
			real rotation_angle_squared = magnitude_squared3d(&rotation_vector);

			if (rotation_angle_squared>_real_epsilon)
			{
				real rotation_angle = square_root(rotation_angle_squared);

				scale_vector3d(&rotation_vector, 1.f / rotation_angle, &rotation_vector);
				rotate_vector_about_axis(&actor->control.moving_towards_vector, &rotation_vector, sine(rotation_angle), cosine(rotation_angle));
			}

			rotation_emergency_amount = actor->control.vector_avoidance_rotation_emergency;
		}
	}

	if (actor->orders.move.override_movement_type==NONE)
	{
		short movement_type = _actor_movement_type_combat;

		if (actor->orders.move.flaming)
		{
			movement_type = _actor_movement_type_flaming;
		}
		else if (actor->orders.move.panicked)
		{
			movement_type = _actor_movement_type_panic;
		}
		else
		{
			switch (actor->state.mode)
			{
			case _actor_mode_asleep:
				movement_type = _actor_movement_type_asleep;
				break;

			case _actor_mode_alert:
				movement_type = _actor_movement_type_alert;
				break;

			case _actor_mode_combat:
				movement_type = _actor_movement_type_combat;
				break;
			}
		}

		actor->output.movement_type = movement_type;
	}
	else
	{
		actor->output.movement_type = actor->orders.move.override_movement_type;
	}

	override_facing = actor->orders.move.override_movement_facing;

	if (actor_path_has_path(actor_index) && actor->control.path.destination_original_distance>=actor_definition->moving.stationary_moving_distance)
	{
		crouch = actor->orders.move.moving_crouch;
	}
	else
	{
		crouch = actor->orders.move.stationary_crouch;
	}

	if (actor->input.vehicle_driver_type>_actor_vehicle_driver_none)
	{
		struct vehicle_datum *vehicle = vehicle_get(actor->input.vehicle_index);
		struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

		steering_maximum_angle = vehicle_definition->vehicle.ai_steering_max_angle;

		if (vehicle_definition->vehicle.ai_steering_max_throttle>0.f)
		{
			maximum_throttle = vehicle_definition->vehicle.ai_steering_max_throttle;
		}

		oversteer_minimum_angle = vehicle_definition->vehicle.ai_oversteer_angle_lower_bound;
		oversteer_maximum_angle = vehicle_definition->vehicle.ai_oversteer_angle_upper_bound;

		switch (actor->input.vehicle_driver_type)
		{
		case _actor_vehicle_driver_hovering_ground:
			if (vehicle->vehicle.airborne_ticks>0)
			{
				jump = TRUE;
				actor->control.moving = FALSE;
				actor->control.free_facing_vector = TRUE;
				break;
			}

			if (vehicle->vehicle.hover<0.7f)
			{
				jump = TRUE;

				if (vehicle->object.up.k<0.8f)
				{
					real_vector3d righting_direction;

					righting_direction = vehicle->object.up;
					righting_direction.k = 0.f;

					if (normalize3d(&righting_direction)>0.f)
					{
						actor->control.moving = TRUE;
						scale_vector3d(&righting_direction, 3.f, &actor->control.moving_towards_vector);
					}
					else
					{
						actor->control.moving = FALSE;
					}

					break;
				}
			}
			// fall through

		case _actor_vehicle_driver_nondirectional_ground:
			free_movement_dist_sq = (vehicle_definition->vehicle.ai_sideslip_distance) * (vehicle_definition->vehicle.ai_sideslip_distance);
			free_movement = TRUE;
			break;

		case _actor_vehicle_driver_directional_flying:
		{
			real_vector3d stuck_direction;

			if (vehicle_stuck(actor->input.vehicle_index, &stuck_direction))
			{
				actor->control.moving = FALSE;
				actor->control.free_facing_vector = FALSE;
				actor->control.optional_facing_vector = FALSE;
				negate_vector3d(&stuck_direction, &actor->control.desired_facing_vector);
			}
			else
			{
				override_facing = _actor_facing_forward;
				move_in_3d = TRUE;
				allow_all_moving_turns = TRUE;
			}

			break;
		}

		default:
			actor->control.moving = FALSE;
			actor->control.moving_facing_direction = _actor_facing_forward;
			actor->control.free_facing_vector = actor->meta.type==_actor_mounted_weapon || actor->input.vehicle_gunner;
			break;
		}

		crouch = FALSE;
	}
	else if (actor->input.vehicle_passenger)
	{
		actor->control.moving = FALSE;
		actor->control.moving_facing_direction = _actor_facing_forward;
		actor->control.free_facing_vector = actor->meta.type==_actor_mounted_weapon || actor->input.vehicle_gunner;
		actor->control.optional_facing_vector = FALSE;
		crouch = FALSE;
	}
	else if (actor->orders.move.animation.impulse!=NONE)
	{
		actor->control.moving = FALSE;
		actor->control.free_facing_vector = FALSE;
		actor->control.optional_facing_vector = FALSE;
		crouch = FALSE;
	}
	else if (actor->output.movement_type==_actor_movement_type_asleep)
	{
		actor->control.moving = FALSE;
		actor->control.free_facing_vector = FALSE;
		actor->control.optional_facing_vector = FALSE;
		face_current_direction = TRUE;
		crouch = FALSE;
	}
	else if (actor->input.in_midair && !actor->state.flying)
	{
		actor->control.moving = FALSE;
		actor->control.free_facing_vector = TRUE;
		crouch = FALSE;
	}
	else if (actor->control.grenade_trying_to_throw)
	{
		real_vector3d grenade_direction;

		actor->control.moving = FALSE;
		vector_from_points3d(&actor->input.position.body_position, &actor->control.grenade_current_target, &grenade_direction);
		crouch = FALSE;

		if (normalize3d(&grenade_direction)!=0.f)
		{
			actor->control.desired_facing_vector = grenade_direction;
			actor->control.free_facing_vector = FALSE;
			actor->control.optional_facing_vector = FALSE;
			actor->control.face_exactly = TRUE;
		}
		else
		{
			actor->control.free_facing_vector = TRUE;
		}
	}
	else if (actor->emotions.moving_into_fire_timer>0)
	{
		actor->control.moving = FALSE;
		actor->control.free_facing_vector = TRUE;
		crouch = TEST_FLAG(actor_definition->flags, _actor_definition_crouch_in_line_of_fire_bit);
	}
	else
	{
		normal_movement = TRUE;

		if (actor->output.movement_type==_actor_movement_type_combat)
		{
			if (crouch)
			{
				if (TEST_FLAG(actor_definition->flags, _actor_definition_crouching_must_move_forward_bit))
				{
					actor->control.moving_forced_by_aiming = FALSE;
				}
			}
			else if (TEST_FLAG(actor_definition->flags, _actor_definition_standing_must_move_forward_bit))
			{
				actor->control.moving_forced_by_aiming = FALSE;
			}
		}
		else
		{
			actor->control.moving_forced_by_aiming = FALSE;
		}

		if (actor->output.movement_type==_actor_movement_type_flaming)
		{
			face_current_direction = TRUE;
		}

		if (TEST_FLAG(actor_definition->flags, _actor_definition_flying_bit))
		{
			move_in_3d = TRUE;
			free_movement = TRUE;
			free_movement_dist_sq = actor_definition->moving.free_flying_sidestep_distance * actor_definition->moving.free_flying_sidestep_distance;

			if (actor->control.moving_forced_by_aiming)
			{
				free_movement_dist_sq *= 4.f;
			}
		}
	}

	if (actor->control.moving && !actor->control.movement_complete)
	{
		actor_move_calculate_movement(
			actor_index,
			move_in_3d,
			override_facing,
			free_movement,
			free_movement_dist_sq,
			allow_all_moving_turns,
			steering_maximum_angle,
			oversteer_minimum_angle,
			oversteer_maximum_angle,
			rotation_emergency_amount,
			maximum_throttle,
			&actor->control.moving_towards_vector,
			&actor->control.desired_facing_vector,
			&actor->control.moving_facing_direction,
			&actor->output.throttle,
			&actor->control.movement_thwarted,
			&actor->control.movement_complete);

		if (actor->control.movement_complete)
		{
			actor->control.moving = FALSE;
		}
	}

	if (actor->control.moving)
	{
		actor->control.optional_facing_vector = FALSE;
		actor->control.free_facing_vector = FALSE;
	}
	else if (face_current_direction)
	{
		actor->control.desired_facing_vector = actor->input.facing_vector;
		actor->control.optional_facing_vector = FALSE;
		actor->control.moving_facing_direction = _actor_facing_forward;
		actor->control.free_facing_vector = FALSE;
	}
	else if (actor->control.fixed_stationary_facing)
	{
		actor->control.desired_facing_vector = actor->control.fixed_stationary_facing_vector;
		actor->control.optional_facing_vector = TRUE;
		actor->control.moving_facing_direction = _actor_facing_forward;
		actor->control.free_facing_vector = FALSE;
	}

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_moving.c", 655, &actor->control.desired_facing_vector);

	if (normal_movement && !actor->control.moving)
	{
		actor_clear_discarded_firing_positions(actor_index, TRUE);
	}

	if (actor->control.moving && TEST_FLAG(actor_definition->flags, _actor_definition_cannot_move_crouching_bit))
	{
		crouch = FALSE;
	}

	if (AI_BEHAVIOR(force_crouch))
	{
		crouch = TRUE;
	}

	actor->control.desire_stationary_facing = FALSE;

	if (crouch && TEST_FLAG(actor_definition->flags, _actor_definition_fixed_crouch_facing_bit))
	{
		actor->control.desire_stationary_facing = TRUE;
	}

	actor->control.crouching = crouch;
	actor_unit_control_crouch(actor_index, crouch);

	{
		boolean busy = actor_move_animation_busy(actor_index) || actor->input.vehicle_index!=NONE || actor->input.in_midair;

		if (!busy && actor->emotions.berserk && !actor->emotions.played_berserk_sound)
		{
			real_vector2d alignment_vector = *(real_vector2d *)&actor->input.facing_vector;
			long target_unit_index = NONE;

			if (actor->target.target_prop_index!=NONE)
			{
				struct prop_datum *prop = prop_get(actor->target.target_prop_index);

				target_unit_index = prop->unit_index;
				alignment_vector = *(real_vector2d *)&prop->actor_to_prop;

				if (normalize2d(&alignment_vector)==0.f)
				{
					alignment_vector = *(real_vector2d *)&actor->input.facing_vector;
				}
			}

			actor_move_animation_impulse(actor_index, _unit_animation_impulse_berserk, &alignment_vector);
			ai_communication_event(_ai_communication_berserk, actor->meta.unit_index, target_unit_index, _comm_hostility_enemy, NONE, NONE, NULL);
			actor->emotions.played_berserk_sound = TRUE;
			busy = TRUE;
		}
	}

	if (jump)
	{
		actor_unit_control_jump(actor_index);
	}
	else if (actor->input.in_midair || actor->input.vehicle_index!=NONE)
	{
		actor->control.jumping_targeted = FALSE;
	}
	else if (actor_move_animation_busy(actor_index))
	{
	}
	else if (actor->orders.move.jump)
	{
		boolean leaping = FALSE;

		if (actor->orders.move.jump_leap)
		{
			real_vector2d alignment_vector;

			if (actor->orders.move.jump_targeted)
			{
				alignment_vector = actor->orders.move.jump_alignment_vector;
			}
			else
			{
				alignment_vector = *(real_vector2d *)&actor->input.facing_vector;

				if (normalize2d(&alignment_vector)==0.f)
				{
					alignment_vector = *global_forward2d;
				}
			}

			leaping = unit_leap_begin(actor->meta.unit_index, &alignment_vector);
		}

		if (leaping)
		{
			ai_communication_event(_ai_communication_leap, actor->meta.unit_index, NONE, NONE, NONE, NONE, NULL);
		}
		else
		{
			actor_unit_control_jump(actor_index);
		}

		if (actor->orders.move.jump_targeted)
		{
			actor->control.jumping_targeted = TRUE;
			actor->control.jump_alignment_vector = actor->orders.move.jump_alignment_vector;
			actor->control.jump_target_horizontal_vel = actor->orders.move.jump_target_horizontal_vel;
			actor->control.jump_target_vertical_vel = actor->orders.move.jump_target_vertical_vel;
		}
	}

	actor->output.animation = actor->orders.move.animation;

	return;
}

void actor_get_stopping_distances(
	long actor_index,
	real *current_stopping_distance_reference,
	real *maximum_stopping_distance_reference)
{
	struct actor_datum *actor = actor_get(actor_index);
	real current_speed = 0.f;
	real maximum_speed = 2.5f / TICKS_PER_SECOND;
	real acceleration = 0.5f / TICKS_PER_SECOND;
	real deceleration = 0.8f / TICKS_PER_SECOND;

	if (actor->input.vehicle_index!=NONE)
	{
		switch (actor->input.vehicle_driver_type)
		{
		case _actor_vehicle_driver_hovering_ground:
		case _actor_vehicle_driver_nondirectional_ground:
		{
			struct vehicle_datum *vehicle = vehicle_get(actor->input.vehicle_index);
			struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

			current_speed = dot_product3d(&vehicle->object.translational_velocity, &vehicle->object.forward);
			maximum_speed = vehicle_definition->vehicle.speed.maximum_forward_speed;
			acceleration = vehicle_definition->vehicle.speed.acceleration;
			deceleration = acceleration;
			break;
		}
		}
	}
	else if (actor->meta.unit_index!=NONE)
	{
		struct biped_datum *biped = biped_try_and_get(actor->meta.unit_index);

		if (biped)
		{
			struct biped_definition *biped_definition = biped_definition_get(biped->definition_index);

			current_speed = dot_product3d(&biped->object.translational_velocity, &biped->object.forward);

			if (TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit))
			{
				maximum_speed = biped_definition->biped.flying_velocity / TICKS_PER_SECOND;
				acceleration = biped_definition->biped.flying_acceleration / TICKS_PER_SECOND;
				deceleration = biped_definition->biped.flying_deceleration / TICKS_PER_SECOND;

				if (actor->control.crouching && biped_definition->biped.flying_crouch_velocity_modifier>0.f)
				{
					maximum_speed *= biped_definition->biped.flying_crouch_velocity_modifier;
					acceleration *= biped_definition->biped.flying_crouch_velocity_modifier;
					deceleration *= biped_definition->biped.flying_crouch_velocity_modifier;
				}
			}
		}
	}

	if (current_stopping_distance_reference)
	{
		*current_stopping_distance_reference = current_speed * current_speed / (2.f * deceleration);
	}

	if (maximum_stopping_distance_reference)
	{
		maximum_speed = MAX(current_speed, maximum_speed);
		*maximum_stopping_distance_reference = (maximum_speed * maximum_speed - current_speed * current_speed) / (2.f * acceleration) + maximum_speed * maximum_speed / (2.f * deceleration);
	}

	return;
}

boolean actor_move_halt(
	long actor_index)
{
	boolean result;
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->input.vehicle_driver_type==_actor_vehicle_driver_directional_flying && actor->control.moving)
	{
		result = actor_move_to_point(actor_index, &actor->input.position.body_position, actor->input.pathfinding_surface_index, NONE);
	}
	else
	{
		actor->firing_positions.current_position_index = NONE;

		if (actor->control.path.destination_orders.destination_type!=_destination_halt)
		{
			actor->orders.move.destination.destination_type = _destination_halt;
			actor->control.path.destination_orders = actor->orders.move.destination;
		}

		result = actor_path_refresh(actor_index, TRUE, NULL);
	}

	return result;
}

boolean actor_move_halt_at_firing_position(
	long actor_index)
{
	boolean result;
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->input.vehicle_driver_type==_actor_vehicle_driver_directional_flying)
	{
		short firing_position_index = actor->firing_positions.current_position_index;

		if (firing_position_index==NONE)
		{
			result = actor_move_halt(actor_index);
		}
		else
		{
			result = actor_move_to_firing_position(actor_index, firing_position_index, NULL);
		}
	}
	else
	{
		if (actor->control.path.destination_orders.destination_type!=_destination_halt)
		{
			actor->orders.move.destination.destination_type = _destination_halt;
			actor->control.path.destination_orders = actor->orders.move.destination;
		}

		result = actor_path_refresh(actor_index, TRUE, NULL);
	}

	return result;
}

boolean actor_move_to_point(
	long actor_index,
	real_point3d *destination,
	long surface_index,
	long ignore_target_object_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = TRUE;

	match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 951, destination);

	actor->firing_positions.current_position_index = NONE;
	actor_set_dormant(actor_index, FALSE);

	if (actor->control.path.destination_orders.destination_type!=_destination_raw_location ||
		actor->control.path.destination_orders.raw.surface_index!=surface_index ||
		distance_squared3d(destination, &actor->control.path.destination_orders.raw.point)>0.1f * 0.1f)
	{
		actor->orders.move.destination.destination_type = _destination_raw_location;
		actor->orders.move.destination.keep_moving = FALSE;
		actor->orders.move.destination.raw.point = *destination;
		actor->orders.move.destination.raw.surface_index = surface_index;
		actor->orders.move.destination.ignore_target_object_index = ignore_target_object_index;
		actor->control.path.destination_orders = actor->orders.move.destination;
		result = actor_path_refresh(actor_index, TRUE, NULL);
	}
	else if (actor->meta.timeslice && !actor->control.path.refreshed_this_tick)
	{
		result = actor_path_refresh(actor_index, FALSE, NULL);
	}

	return result;
}

boolean actor_move_to_move_position(
	long actor_index,
	short move_position_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = TRUE;

	actor->firing_positions.current_position_index = NONE;
	actor_set_dormant(actor_index, FALSE);

	if (actor->control.path.destination_orders.destination_type!=_destination_move_position ||
		actor->control.path.destination_orders.move_position_index!=move_position_index)
	{
		actor->orders.move.destination.destination_type = _destination_move_position;
		actor->orders.move.destination.keep_moving = FALSE;
		actor->orders.move.destination.move_position_index = move_position_index;
		actor->orders.move.destination.ignore_target_object_index = NONE;
		actor->control.path.destination_orders = actor->orders.move.destination;
		result = actor_path_refresh(actor_index, TRUE, NULL);
	}
	else if (actor->meta.timeslice && !actor->control.path.refreshed_this_tick)
	{
		result = actor_path_refresh(actor_index, FALSE, NULL);
	}

	return result;
}

boolean actor_move_to_firing_position(
	long actor_index,
	short firing_position_index,
	struct path_state *cached_path_state)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = TRUE;

	actor_set_dormant(actor_index, FALSE);

	if (actor->control.path.destination_orders.destination_type!=_destination_firing_position ||
		actor->control.path.destination_orders.firing_position_index!=firing_position_index)
	{
		actor->orders.move.destination.destination_type = _destination_firing_position;
		actor->orders.move.destination.keep_moving = FALSE;
		actor->orders.move.destination.firing_position_index = firing_position_index;
		actor->orders.move.destination.ignore_target_object_index = NONE;
		actor->firing_positions.moved_away_from_firing_position = FALSE;
		actor->control.path.destination_orders = actor->orders.move.destination;
		result = actor_path_refresh(actor_index, TRUE, cached_path_state);
	}
	else if (actor->meta.timeslice && !actor->control.path.refreshed_this_tick)
	{
		result = actor_path_refresh(actor_index, FALSE, cached_path_state);
	}

	return result;
}

boolean actor_move_to_prop(
	long actor_index,
	long prop_index,
	real accept_radius)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = TRUE;

	actor->firing_positions.current_position_index = NONE;
	actor_set_dormant(actor_index, FALSE);

	if (actor->control.path.destination_orders.destination_type!=_destination_prop ||
		actor->control.path.destination_orders.prop.prop_index!=prop_index ||
		actor->control.path.destination_orders.prop.accept_radius!=accept_radius)
	{
		struct prop_datum *prop = prop_get(prop_index);

		actor->orders.move.destination.destination_type = _destination_prop;
		actor->orders.move.destination.keep_moving = FALSE;
		actor->orders.move.destination.prop.prop_index = prop_index;
		actor->orders.move.destination.prop.accept_radius = accept_radius;
		actor->orders.move.destination.ignore_target_object_index = prop->vehicle_index==NONE ? prop->unit_index : prop->vehicle_index;
		actor->control.path.destination_orders = actor->orders.move.destination;
		result = actor_path_refresh(actor_index, TRUE, NULL);
	}
	else if (actor->meta.timeslice && !actor->control.path.refreshed_this_tick)
	{
		result = actor_path_refresh(actor_index, FALSE, NULL);
	}

	return result;
}

void actor_move_keep_moving_past_destination(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->orders.move.destination.keep_moving = TRUE;
	actor->control.path.destination_orders.keep_moving = TRUE;

	return;
}

boolean actor_move_animation_impulse(
	long actor_index,
	short animation_impulse,
	real_vector2d const *animation_alignment)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	actor_set_dormant(actor_index, FALSE);

	if (!actor_move_animation_busy(actor_index))
	{
		actor->orders.move.animation.impulse = animation_impulse;
		actor->orders.move.animation.alignment_vector = *animation_alignment;
		result = TRUE;
	}

	return result;
}

boolean actor_move_force_stop(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (!actor_move_animation_busy(actor_index) && !actor_action_deny_transition(actor_index))
	{
		actor->control.moving = FALSE;
		actor->output.throttle = *global_zero_vector3d;
		actor_unit_control_stop_animation_impulse(actor_index);
		result = TRUE;
	}

	return result;
}

boolean actor_move_animation_busy(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	return (boolean)(actor->orders.move.animation.impulse!=NONE || (actor->meta.unit_index!=NONE && unit_is_busy(actor->meta.unit_index)));
}

boolean actor_move_try_evasion_vector(
	long actor_index,
	real_vector2d *evasion_vector,
	real evade_distance,
	real maximum_ledge_height,
	boolean *evasion_is_ledge_reference,
	struct path_collision_result *result)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean success = FALSE;
	boolean evasion_is_ledge = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1156, evasion_vector && result);

	if (actor->state.flying)
	{
	}
	else
	{
		real_point2d evasion_point;

		point_from_line2d((real_point2d *)&actor->input.position.body_position, evasion_vector, evade_distance, &evasion_point);
		actor_find_pathfinding_location(actor_index);

		if (!structure_test_pill2d(
			global_structure_bsp_get(),
			actor->emotions.ignorant_of_broken_surfaces,
			(real_point2d *)&actor->input.pathfinding_point,
			actor->input.pathfinding_surface_index,
			&evasion_point,
			NONE,
			actor_definition->moving.pathfinding_radius,
			0,
			result))
		{
			real height_difference = result->point.z - actor->input.position.body_position.z;

			success = TRUE;

			if (height_difference>evade_distance * 0.5f)
			{
				success = FALSE;
			}
			else if (maximum_ledge_height==0.f && height_difference<evade_distance * -0.5f)
			{
				success = FALSE;
			}
		}

		if (!success && maximum_ledge_height>0.f)
		{
			real_point3d actor_midpoint;
			struct collision_bsp_test_vector_result result;
			real_vector3d collision_vector;
			struct collision_bsp *collision_bsp = global_collision_bsp_get();

			midpoint3d(&actor->input.position.body_position, &actor->input.position.head_position, &actor_midpoint);
			scale_vector2d(evasion_vector, evade_distance, (real_vector2d *)&collision_vector);
			collision_vector.k = 0.f;

			if (!collision_bsp_test_vector(
				FLAG(_collision_bsp_test_front_facing_surfaces_bit) | FLAG(_collision_bsp_test_back_facing_surfaces_bit),
				collision_bsp,
				0,
				NULL,
				&actor_midpoint,
				&collision_vector,
				REAL_MAX,
				&result))
			{
				success = TRUE;
				evasion_is_ledge = TRUE;

				if (maximum_ledge_height<REAL_MAX)
				{
					real_point3d down_ray_start;
					real_vector3d down_ray_vector;

					point_from_line3d(&actor_midpoint, &collision_vector, 1.f, &down_ray_start);
					scale_vector3d(global_down3d, maximum_ledge_height, &down_ray_vector);

					if (!collision_bsp_test_vector(
						FLAG(_collision_bsp_test_front_facing_surfaces_bit) | FLAG(_collision_bsp_test_back_facing_surfaces_bit),
						collision_bsp,
						0,
						NULL,
						&down_ray_start,
						&down_ray_vector,
						REAL_MAX,
						&result))
					{
						success = FALSE;
					}
				}
			}
		}
	}

	if (evasion_is_ledge_reference)
	{
		*evasion_is_ledge_reference = evasion_is_ledge;
	}

	return success;
}

boolean actor_move_try_evasion_direction(
	long actor_index,
	real_vector2d *alignment_vector,
	real evade_distance,
	short *evade_direction_reference,
	real maximum_ledge_height,
	boolean *evasion_is_ledge,
	struct path_collision_result *result)
{
	short evade_direction;
	real_vector2d evasion_vector;
	short attempt_index;
	struct actor_datum *actor = actor_get(actor_index);
	boolean success = FALSE;
	short attempt_count = 1;

	match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1252, alignment_vector && evade_direction_reference && result);

	evade_direction = *evade_direction_reference;

	switch (evade_direction)
	{
	case _actor_evade_left:
		set_real_vector2d(&evasion_vector, -alignment_vector->j, alignment_vector->i);
		break;

	case _actor_evade_right:
		set_real_vector2d(&evasion_vector, alignment_vector->j, -alignment_vector->i);
		break;

	case _actor_evade_forward:
		set_real_vector2d(&evasion_vector, alignment_vector->i, alignment_vector->j);
		break;

	case _actor_evade_back:
		set_real_vector2d(&evasion_vector, -alignment_vector->i, -alignment_vector->j);
		break;

	case _actor_evade_random_side:
		if (random_boolean())
		{
			evade_direction = _actor_evade_left;
			set_real_vector2d(&evasion_vector, -alignment_vector->j, alignment_vector->i);
		}
		else
		{
			evade_direction = _actor_evade_right;
			set_real_vector2d(&evasion_vector, alignment_vector->j, -alignment_vector->i);
		}

		attempt_count = 2;
		break;

	default:
		match_halt("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1288);
		break;
	}

	for (attempt_index = 0; attempt_index<attempt_count; attempt_index++, evade_direction ^= 1)
	{
		if (actor_move_try_evasion_vector(actor_index, &evasion_vector, evade_distance, maximum_ledge_height, evasion_is_ledge, result))
		{
			success = TRUE;
			break;
		}

		negate_vector2d(&evasion_vector, &evasion_vector);
	}

	*evade_direction_reference = success ? evade_direction : NONE;

	return success;
}

boolean actor_aim_jump(
	long actor_index,
	long unit_index,
	boolean leap,
	real jump_magnitude,
	real_vector3d *jump_velocity)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->input.vehicle_index!=NONE)
	{
	}
	else
	{
		if (actor->meta.swarm)
		{
			actor_type_swarm_aim_jump(actor_index, unit_index, jump_magnitude, jump_velocity);
		}
		else if (actor->control.jumping_targeted)
		{
			real jump_speed;

			if (actor->state.action==_actor_action_charge && actor->state.action_data.charge.goal==_charge_goal_melee_leaping)
			{
				leap = TRUE;
			}

			set_real_vector3d(
				jump_velocity,
				actor->control.jump_alignment_vector.i * actor->control.jump_target_horizontal_vel,
				actor->control.jump_alignment_vector.j * actor->control.jump_target_horizontal_vel,
				actor->control.jump_target_vertical_vel);
			jump_speed = magnitude3d(jump_velocity);

			if (!leap && jump_speed>jump_magnitude)
			{
				scale_vector3d(jump_velocity, jump_magnitude / jump_speed, jump_velocity);
			}
		}
	}

	actor->control.jumping_targeted = FALSE;

	return TRUE;
}

static void actor_move_calculate_movement(
	long actor_index,
	boolean move_in_3d,
	short override_facing,
	boolean free_movement,
	real free_movement_dist_sq,
	boolean allow_all_moving_turns,
	real steering_maximum_angle,
	real oversteer_minimum_angle,
	real oversteer_maximum_angle,
	real rotation_emergency_amount,
	real maximum_throttle,
	real_vector3d const *desired_movement_vector,
	real_vector3d *desired_facing_vector,
	short *desired_facing_direction,
	real_vector3d *desired_throttle,
	boolean *movement_thwarted,
	boolean *movement_complete)
{
	real facing_projection;
	boolean begin_movement;
	real tolerance;
	real distance_squared;
	real maximum_stopping_distance;
	real current_stopping_distance;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	real begin_movement_angle_cosine = _cosine30;
	real_vector3d facing_vector;
	real_vector3d free_throttle;
	real_vector3d throttle;
	short facing_direction = NONE;

	if (actor->orders.move.move_face_exactly)
	{
		actor->control.face_exactly = TRUE;
	}

	if (override_facing>=_actor_facing_forward && override_facing<=_actor_facing_right)
	{
		real_vector3d movement_vector;

		facing_direction = override_facing;
		movement_vector = *desired_movement_vector;

		if (!move_in_3d)
		{
			movement_vector.k = 0.f;
		}

		if (normalize3d(&movement_vector)==0.f)
		{
			movement_vector = actor->input.facing_vector;
		}

		switch (facing_direction)
		{
		case _actor_facing_forward:
			set_real_vector3d(&facing_vector, movement_vector.i, movement_vector.j, movement_vector.k);
			break;

		case _actor_facing_backward:
			set_real_vector3d(&facing_vector, -movement_vector.i, -movement_vector.j, movement_vector.k);
			break;

		case _actor_facing_left:
			set_real_vector3d(&facing_vector, -movement_vector.j, movement_vector.i, movement_vector.k);
			break;

		case _actor_facing_right:
			set_real_vector3d(&facing_vector, movement_vector.j, -movement_vector.i, movement_vector.k);
			break;

		default:
			match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1432, !"unreachable");
			break;
		}

		if (free_movement)
		{
			actor_move_calculate_free(move_in_3d, &movement_vector, &facing_vector, &free_throttle);
			facing_direction = _actor_facing_free;
		}
	}
	else
	{
		real movement_distance_squared = magnitude_squared3d(desired_movement_vector);

		if (movement_distance_squared>0.8f * 0.8f)
		{
			begin_movement_angle_cosine = actor_definition->moving.runtime_begin_movement_angle_cosine;
		}

		if (free_movement && movement_distance_squared<free_movement_dist_sq)
		{
			real_vector3d current_facing_vector;
			real_vector3d movement_vector = *desired_movement_vector;
			boolean use_current_facing = FALSE;

			if (actor->control.moving_forced_by_aiming)
			{
				facing_vector = actor->control.moving_forced_aim_direction;

				if (actor->input.vehicle_driver_type>_actor_vehicle_driver_none)
				{
					use_current_facing = TRUE;
				}
			}
			else
			{
				facing_vector = actor->input.facing_vector;
			}

			if (!move_in_3d)
			{
				movement_vector.k = 0.f;
				facing_vector.k = 0.f;
			}

			if (normalize3d(&facing_vector)==0.f)
			{
				facing_vector = actor->input.facing_vector;
			}

			if (normalize3d(&movement_vector)==0.f)
			{
				movement_vector = facing_vector;
			}

			if (use_current_facing)
			{
				current_facing_vector = actor->input.facing_vector;

				if (!move_in_3d)
				{
					current_facing_vector.k = 0.f;
				}

				if (normalize3d(&current_facing_vector)==0.f)
				{
					current_facing_vector = facing_vector;
				}
			}

			actor_move_calculate_free(move_in_3d, &movement_vector, use_current_facing ? &current_facing_vector : &facing_vector, &free_throttle);
			facing_direction = _actor_facing_free;
		}
		else if (actor->control.moving_forced_by_aiming)
		{
			actor_move_calculate_controlled_by_aiming(
				move_in_3d,
				desired_movement_vector,
				&actor->control.moving_forced_aim_direction,
				&actor->input.facing_vector,
				&facing_vector,
				&facing_direction);
		}
		else
		{
			facing_vector = *desired_movement_vector;

			if (!move_in_3d)
			{
				facing_vector.k = 0.f;
			}

			if (normalize3d(&facing_vector)==0.f)
			{
				facing_vector = actor->input.facing_vector;
			}

			facing_direction = _actor_facing_forward;
		}
	}

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1532, &facing_vector);

	facing_projection = dot_product3d(&facing_vector, &actor->input.facing_vector);
	begin_movement = FALSE;

	if (allow_all_moving_turns || actor->output.movement_type==_actor_movement_type_flaming)
	{
		begin_movement = TRUE;
	}
	else
	{
		if (!actor->state.flying)
		{
			actor_find_pathfinding_location(actor_index);

			if (actor->input.pathfinding_surface_index!=NONE)
			{
				real_vector3d current_movement_vector;
				boolean valid = TRUE;

				switch (facing_direction)
				{
				case _actor_facing_forward:
					set_real_vector3d(&current_movement_vector, actor->input.facing_vector.i, actor->input.facing_vector.j, actor->input.facing_vector.k);
					break;

				case _actor_facing_backward:
					set_real_vector3d(&current_movement_vector, -actor->input.facing_vector.i, -actor->input.facing_vector.j, actor->input.facing_vector.k);
					break;

				case _actor_facing_left:
					set_real_vector3d(&current_movement_vector, actor->input.facing_vector.j, -actor->input.facing_vector.i, actor->input.facing_vector.k);
					break;

				case _actor_facing_right:
					set_real_vector3d(&current_movement_vector, -actor->input.facing_vector.j, actor->input.facing_vector.i, actor->input.facing_vector.k);
					break;

				default:
					valid = FALSE;
					break;
				}

				if (valid)
				{
					if (normalize2d((real_vector2d *)&current_movement_vector)>0.f)
					{
						struct path_collision_result result;
						real_point3d movement_lookahead_point;

						current_movement_vector.k = 0.f;
						point_from_line3d(&actor->input.position.body_position, &current_movement_vector, 0.4f, &movement_lookahead_point);

						if (structure_test_line2d(
							global_structure_bsp_get(),
							actor->emotions.ignorant_of_broken_surfaces,
							(real_point2d *)&actor->input.position.body_position,
							actor->input.pathfinding_surface_index,
							(real_point2d *)&movement_lookahead_point,
							NONE,
							&result))
						{
							begin_movement_angle_cosine = MAX(begin_movement_angle_cosine, 0.95f);
						}
					}
				}
			}
		}

		begin_movement = facing_projection>begin_movement_angle_cosine;
	}

	tolerance = actor_destination_tolerance(actor_index);
	distance_squared = magnitude_squared3d(desired_movement_vector);
	*movement_complete = distance_squared<tolerance * tolerance;
	actor_get_stopping_distances(actor_index, &current_stopping_distance, &maximum_stopping_distance);

	if (!actor->control.path.destination_orders.keep_moving && current_stopping_distance * current_stopping_distance>distance_squared)
	{
		real distance = square_root(distance_squared);

		if (maximum_stopping_distance + 0.05f<distance && current_stopping_distance>maximum_stopping_distance)
		{
			real throttle_fraction = (distance - maximum_stopping_distance) / (current_stopping_distance - maximum_stopping_distance);

			maximum_throttle = MIN(maximum_throttle, throttle_fraction);
		}
		else
		{
			maximum_throttle = 0.f;
		}

		match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1634, (maximum_throttle >= 0.0f) && (maximum_throttle <= 1.0f));
	}

	throttle = *global_zero_vector3d;

	if (begin_movement)
	{
		switch (facing_direction)
		{
		case _actor_facing_forward:
			throttle.i = 1.f;
			break;

		case _actor_facing_backward:
			throttle.i = -1.f;
			break;

		case _actor_facing_left:
			throttle.j = -1.f;
			break;

		case _actor_facing_right:
			throttle.j = 1.f;
			break;

		case _actor_facing_free:
			throttle = free_throttle;
			break;

		default:
			match_halt("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1649);
			break;
		}

		scale_vector3d(&throttle, maximum_throttle, &throttle);
		*movement_thwarted = FALSE;
	}
	else
	{
		actor->control.face_exactly = TRUE;
		*movement_thwarted = TRUE;
	}

	if (AI_BEHAVIOR(oversteer_disable))
	{
		oversteer_maximum_angle = 0.f;
		oversteer_minimum_angle = 0.f;
	}

	if (steering_maximum_angle>0.f || oversteer_maximum_angle>0.f)
	{
		real facing_angle;
		real steer_angle;
		real adjust_angle;

		if (facing_projection>=1.f)
		{
			facing_angle = 0.f;
		}
		else if (facing_projection<=-1.f)
		{
			facing_angle = _pi;
		}
		else
		{
			facing_angle = arccosine(facing_projection);
		}

		steer_angle = facing_angle;

		if (steering_maximum_angle>0.f)
		{
			real minimum_steer_angle = steering_maximum_angle * rotation_emergency_amount;
			real maximum_steer_angle = steering_maximum_angle;

			if (rotation_emergency_amount>1.f)
			{
				maximum_steer_angle *= MIN(rotation_emergency_amount, 1.5f);
			}

			minimum_steer_angle = MIN(3.f * steer_angle, minimum_steer_angle);
			steer_angle = PIN(steer_angle, minimum_steer_angle, maximum_steer_angle);
		}

		if (steer_angle>actor->control.face_exactly_oversteer_angle)
		{
			if (actor->control.face_exactly && steer_angle>oversteer_minimum_angle)
			{
				actor->control.face_exactly_oversteer_angle = MIN(steer_angle, oversteer_maximum_angle);

#ifdef DEBUG
				if (ai_debug.print_oversteer)
				{
					console_printf(FALSE, "steer %.4f (set oversteer %.4f)", steer_angle, actor->control.face_exactly_oversteer_angle);
				}
#endif
			}
#ifdef DEBUG
			else if (ai_debug.print_oversteer)
			{
				console_printf(FALSE, "steer %.4f", steer_angle);
			}
#endif
		}
		else if (actor->control.face_exactly_oversteer_angle>0.f)
		{
			if (steer_angle<oversteer_minimum_angle)
			{
#ifdef DEBUG
				if (ai_debug.print_oversteer)
				{
					console_printf(FALSE, "steer %.4f < %.4f - clear oversteer %.4f", steer_angle, oversteer_minimum_angle, actor->control.face_exactly_oversteer_angle);
				}
#endif

				actor->control.face_exactly_oversteer_angle = 0.f;
			}
			else
			{
#ifdef DEBUG
				if (ai_debug.print_oversteer)
				{
					console_printf(FALSE, "steer %.4f - oversteer to %.4f", steer_angle, actor->control.face_exactly_oversteer_angle);
				}
#endif

				steer_angle = actor->control.face_exactly_oversteer_angle;
			}
		}

		adjust_angle = steer_angle - facing_angle;

		if (fabs(adjust_angle)>_real_epsilon)
		{
			real_vector3d rotation_axis;

			cross_product3d(&actor->input.facing_vector, &facing_vector, &rotation_axis);

			if (normalize3d(&rotation_axis)>0.f)
			{
				rotate_vector_about_axis(&facing_vector, &rotation_axis, sine(adjust_angle), cosine(adjust_angle));

#ifdef DEBUG
				if (ai_debug.print_oversteer)
				{
					console_printf(FALSE, "adjust angle %.4f -> %.4f (%.4f)", facing_angle, steer_angle, adjust_angle);
				}
#endif
			}
		}
	}

	if (!actor->control.moving_forced_by_aiming && actor->orders.move.override_movement_facing==NONE)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1777, (facing_direction == _actor_facing_forward) || (facing_direction == _actor_facing_free));
	}

	*desired_facing_direction = facing_direction;
	*desired_facing_vector = facing_vector;
	*desired_throttle = throttle;

	return;
}

static void actor_move_calculate_controlled_by_aiming(
	boolean move_in_3d,
	real_vector3d const *movement_vector,
	real_vector3d const *forced_aim_direction,
	real_vector3d const *current_facing_vector,
	real_vector3d *desired_facing_vector,
	short *desired_facing_direction)
{
	real_vector3d facings[NUMBER_OF_ACTOR_FACINGS - 1];
	short best_facing_direction;
	real best_desired_aiming_projection;
	real best_facing_projection;
	short facing_direction;

	if (move_in_3d)
	{
		facings[_actor_facing_forward] = *movement_vector;

		if (normalize3d(&facings[_actor_facing_forward])==0.f)
		{
			facings[_actor_facing_forward] = *current_facing_vector;
		}

		facings[_actor_facing_left] = *global_zero_vector3d;
	}
	else
	{
		facings[_actor_facing_forward] = *movement_vector;
		facings[_actor_facing_forward].k = 0.f;

		if (normalize3d(&facings[_actor_facing_forward])==0.f)
		{
			facings[_actor_facing_forward] = *current_facing_vector;
		}

		set_real_vector3d(&facings[_actor_facing_left], -facings[_actor_facing_forward].j, facings[_actor_facing_forward].i, 0.f);
	}

	negate_vector3d(&facings[_actor_facing_forward], &facings[_actor_facing_backward]);
	negate_vector3d(&facings[_actor_facing_left], &facings[_actor_facing_right]);

	best_facing_direction = NONE;

	for (facing_direction = 0; facing_direction<NUMBER_OF_ACTOR_FACINGS - 1; facing_direction++)
	{
		real desired_aiming_projection;
		real facing_projection;

		if (move_in_3d)
		{
			desired_aiming_projection = dot_product3d(forced_aim_direction, &facings[facing_direction]);
			facing_projection = dot_product3d(current_facing_vector, &facings[facing_direction]);
		}
		else
		{
			desired_aiming_projection = dot_product2d((real_vector2d *)forced_aim_direction, (real_vector2d *)&facings[facing_direction]);
			facing_projection = dot_product2d((real_vector2d *)current_facing_vector, (real_vector2d *)&facings[facing_direction]);
		}

		if (best_facing_direction==NONE ||
			(desired_aiming_projection>best_desired_aiming_projection ?
				(facing_projection>best_facing_projection || best_facing_projection<0.5f) :
				(facing_projection>best_facing_projection && desired_aiming_projection>0.5f)))
		{
			best_facing_direction = facing_direction;
			best_desired_aiming_projection = desired_aiming_projection;
			best_facing_projection = facing_projection;
		}
	}

	*desired_facing_direction = best_facing_direction;
	*desired_facing_vector = facings[best_facing_direction];

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1892, desired_facing_vector);

	return;
}

static void actor_move_calculate_free(
	boolean move_in_3d,
	real_vector3d const *movement_direction,
	real_vector3d const *facing_direction,
	real_vector3d *throttle)
{
	if (move_in_3d)
	{
		real_vector3d facing_left_direction;
		real_vector3d facing_up_direction;

		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1909, movement_direction);
		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1910, facing_direction);

		biped_build_flying_axes(facing_direction, &facing_left_direction, &facing_up_direction);
		throttle->i = dot_product3d(movement_direction, facing_direction);
		throttle->j = dot_product3d(movement_direction, &facing_left_direction);
		throttle->k = dot_product3d(movement_direction, &facing_up_direction);
		normalize3d(throttle);
	}
	else
	{
		real_vector3d facing_left_direction;

		match_assert_valid_real_normal2d("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1925, (real_vector2d *) movement_direction);
		match_assert_valid_real_normal2d("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1926, (real_vector2d *) facing_direction);
		match_assert_valid_realcmp("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1927, movement_direction->k, 0.0f);
		match_assert_valid_realcmp("c:\\halo\\SOURCE\\ai\\actor_moving.c", 1928, facing_direction->k, 0.0f);

		set_real_vector3d(&facing_left_direction, -facing_direction->j, facing_direction->i, 0.f);
		throttle->i = dot_product2d((real_vector2d *)movement_direction, (real_vector2d *)facing_direction);
		throttle->j = dot_product2d((real_vector2d *)movement_direction, (real_vector2d *)&facing_left_direction);
		throttle->k = 0.f;
		normalize3d(throttle);
	}

	return;
}

static void actor_move_avoidance_setup(
	struct vector_avoidance_data *avoidance_data)
{
	long object_indices[MAXIMUM_OBJECTS_PER_MAP];
	short object_index_index;
	struct object_datum *object = object_get(avoidance_data->object_index);
	short object_count = objects_in_sphere(
		FLAG(_object_class_collideable),
		_object_mask_vehicle | _object_mask_scenery | _object_mask_machine,
		&object->object.location,
		&avoidance_data->origin,
		MAX(avoidance_ray_length, sense_ray_length) * avoidance_data->avoid_distance,
		object_indices,
		NUMBEROF(object_indices));

	avoidance_data->avoidance_object_count = 0;

	for (object_index_index = 0; object_index_index<object_count; object_index_index++)
	{
		long avoid_object_index = object_indices[object_index_index];
		struct object_datum *avoid_object = object_get(avoid_object_index);

		if (avoid_object_index!=NONE && avoid_object_index!=avoidance_data->object_index)
		{
			struct object_definition *object_definition = object_definition_get(avoid_object->definition_index);
			struct collision_model *collision_model = collision_model_definition_get(object_definition->object.collision_model.index);

			if (collision_model->pathfinding_spheres.count>0)
			{
				real_matrix4x3 world_matrix;
				real_point3d bounding_sphere_center;
				real bounding_sphere_radius;
				short sphere_index;
				real maximum_radius = 0.f;

				object_get_bounding_sphere(avoid_object_index, &bounding_sphere_center, &bounding_sphere_radius);
				object_get_world_matrix(avoid_object_index, &world_matrix);

				for (sphere_index = 0; sphere_index<collision_model->pathfinding_spheres.count; sphere_index++)
				{
					real_point3d transformed_center;
					real radius;
					real distance;
					struct pathfinding_sphere *sphere = TAG_BLOCK_GET_ELEMENT(&collision_model->pathfinding_spheres, sphere_index, struct pathfinding_sphere);

					if (sphere->node_index!=NONE)
					{
						real_matrix4x3 *node_matrix = object_get_node_matrix(avoid_object_index, sphere->node_index);

						matrix4x3_transform_point(node_matrix, &sphere->center, &transformed_center);
						radius = node_matrix->scale * sphere->radius;
					}
					else
					{
						matrix4x3_transform_point(&world_matrix, &sphere->center, &transformed_center);
						radius = world_matrix.scale * sphere->radius;
					}

					distance = distance2d((real_point2d *)&bounding_sphere_center, (real_point2d *)&transformed_center);
					maximum_radius = MAX(maximum_radius, distance + radius);
				}

				if (avoidance_data->avoidance_object_count<MAXIMUM_NUMBER_OF_AVOIDANCE_OBJECTS)
				{
					short cylinder_index = avoidance_data->avoidance_object_count++;

					avoidance_data->avoidance_objects[cylinder_index].object_index = avoid_object_index;
					avoidance_data->avoidance_objects[cylinder_index].width = maximum_radius;
					avoidance_data->avoidance_objects[cylinder_index].base = bounding_sphere_center;
					avoidance_data->avoidance_objects[cylinder_index].base.z -= bounding_sphere_radius - maximum_radius;
					avoidance_data->avoidance_objects[cylinder_index].height = MAX(0.f, 2.f * bounding_sphere_radius - 2.f * maximum_radius);
				}
			}
		}
	}

	return;
}

static short actor_move_test_avoidance_vector(
	struct vector_avoidance_data *avoidance_data,
	struct vector_avoidance_ray const *avoidance_ray,
	real_point3d *ray_origin,
	real_vector3d *ray_direction,
	real *collision_t,
	byte *collision_timer)
{
	real_vector3d divergence_direction;
	struct collision_bsp_test_vector_result collision_result;
	boolean collision;
	short object_index;
	short result = _actor_vector_avoidance_clear;

	match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 2039, avoidance_data && avoidance_ray);
	match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 2040, ray_origin && ray_direction);
	match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 2041, collision_t);

	*collision_t = REAL_MAX;

	{
		real_vector3d origin_offset;

		origin_offset = *global_zero_vector3d;
		vector_from_line3d(&origin_offset, &avoidance_data->forward, avoidance_ray->offset.i, &origin_offset);
		vector_from_line3d(&origin_offset, &avoidance_data->left, avoidance_ray->offset.j, &origin_offset);
		vector_from_line3d(&origin_offset, &avoidance_data->up, avoidance_ray->offset.k, &origin_offset);

		divergence_direction = *global_zero_vector3d;
		vector_from_line3d(&divergence_direction, &avoidance_data->forward, avoidance_ray->divergence.i, &divergence_direction);
		vector_from_line3d(&divergence_direction, &avoidance_data->left, avoidance_ray->divergence.j, &divergence_direction);
		vector_from_line3d(&divergence_direction, &avoidance_data->up, avoidance_ray->divergence.k, &divergence_direction);

		point_from_line3d(&avoidance_data->origin, &origin_offset, avoidance_data->avoid_width, ray_origin);
		scale_vector3d(&divergence_direction, avoidance_ray->length * avoidance_data->avoid_distance, ray_direction);
	}

	{
		real_vector3d vector_to_origin;

		vector_from_points3d(&avoidance_data->origin, ray_origin, &vector_to_origin);
		collision = collision_bsp_test_vector(
			FLAG(_collision_bsp_test_front_facing_surfaces_bit) | FLAG(_collision_bsp_test_back_facing_surfaces_bit),
			avoidance_data->bsp,
			0,
			NULL,
			&avoidance_data->origin,
			&vector_to_origin,
			1.f,
			&collision_result);
	}

	if (collision)
	{
		result = _actor_vector_avoidance_obstructed_structure;
		*collision_t = 0.f;
	}
	else
	{
		collision = collision_bsp_test_vector(
			FLAG(_collision_bsp_test_front_facing_surfaces_bit) | FLAG(_collision_bsp_test_back_facing_surfaces_bit),
			avoidance_data->bsp,
			0,
			NULL,
			ray_origin,
			ray_direction,
			1.f,
			&collision_result);

		if (collision)
		{
			result = _actor_vector_avoidance_obstructed_structure;
			*collision_t = collision_result.t;
		}
	}

	for (object_index = 0; object_index<avoidance_data->avoidance_object_count; object_index++)
	{
		struct vehicle_avoidance_cylinder *avoidance_object = &avoidance_data->avoidance_objects[object_index];
		real_vector3d dummy_normal;
		real intersect_t;

		if (pill_test_vector3d(&avoidance_object->base, avoidance_object->height, avoidance_object->width, ray_origin, ray_direction, &intersect_t, &dummy_normal))
		{
			if (intersect_t<*collision_t)
			{
				result = _actor_vector_avoidance_obstructed_object;
				*collision_t = intersect_t;
			}
		}
	}

	if (collision_timer)
	{
		if (result>_actor_vector_avoidance_clear)
		{
			*collision_timer = 0;
		}
		else if (*collision_timer<UNSIGNED_CHAR_MAX)
		{
			(*collision_timer)++;
		}
	}

	return result;
}

static boolean actor_move_vector_avoidance_find_direction(
	short direction_count,
	real_vector3d const *avoidance_directions,
	real const *avoidance_weights,
	real_vector3d const *direction_vector,
	real *approximate_direction_reference,
	real *approximate_weight_reference)
{
	real_vector3d temp_vector;
	real previous_cross;
	short direction_index;
	boolean found = FALSE;
	short previous_index = direction_count - 1;

	cross_product3d(&avoidance_directions[previous_index], direction_vector, &temp_vector);
	previous_cross = temp_vector.i;

	for (direction_index = 0; direction_index<direction_count; direction_index++)
	{
		real current_cross;

		cross_product3d(&avoidance_directions[direction_index], direction_vector, &temp_vector);
		current_cross = temp_vector.i;

		if (current_cross * previous_cross<=0.f)
		{
			real projection = dot_product3d(&avoidance_directions[direction_index], direction_vector);

			if (projection>0.f)
			{
				*approximate_direction_reference =
					((real)previous_index * current_cross - previous_cross * (real)(direction_index==0 ? direction_count : direction_index)) /
					(current_cross - previous_cross);
				*approximate_weight_reference =
					(current_cross * avoidance_weights[previous_index] - previous_cross * avoidance_weights[direction_index]) /
					(current_cross - previous_cross);
				found = TRUE;
				break;
			}
		}

		previous_cross = current_cross;
		previous_index = direction_index;
	}

	return found;
}

static void actor_move_vector_avoidance(
	long actor_index,
	real_vector3d const *movement_direction,
	real_vector3d *avoidance_rotation,
	real *emergency_amount)
{
	struct actor_datum *actor = actor_get(actor_index);
	real_vector3d rotation_vector = *global_zero_vector3d;
	real rotation_emergency = 0.f;
	long object_index = actor->input.vehicle_index==NONE ? actor->meta.unit_index : actor->input.vehicle_index;

	match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 2183, avoidance_rotation && emergency_amount);

	if (object_index!=NONE)
	{
		struct vector_avoidance_data avoidance_data;
		real maximum_obstruction;
		real avoidance_weights[VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS];
		short sense_ray_index;
		short weight_index;
		short direction_index;
		short turn_index;
		short test_index;
		real turn_speed;
		real best_avoidance_weight;
		short best_avoidance_direction;
		real_vector3d movement_vector;
		real forward_projection;
		real_vector3d movement_in_vector_space;
		real movement_direction_approximation;
		real movement_avoidance_weight_approximation;
		real avoidance_weight_difference;
		boolean sharp_turn;
		real obstruction_emergency;
		struct object_datum *object = object_get(object_index);
		boolean avoiding = FALSE;
#ifdef DEBUG
		struct actor_debug_info *actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];

		actor_debug_info->field_19C = game_time_get();
#endif

		avoidance_data.structure = global_structure_bsp_get();
		avoidance_data.bsp = global_collision_bsp_get();
		avoidance_data.object_index = object_index;
		object_get_origin(object_index, &avoidance_data.origin);
		avoidance_data.forward = object->object.forward;
		avoidance_data.up = object->object.up;
		cross_product3d(&object->object.up, &object->object.forward, &avoidance_data.left);
		avoidance_data.avoid_distance = 12.f;
		avoidance_data.avoid_width = 1.f;
		actor_move_avoidance_setup(&avoidance_data);

		maximum_obstruction = 0.f;
		memset(avoidance_weights, 0, sizeof(avoidance_weights));

		if (actor->control.vector_avoidance_current_direction>=0 && actor->control.vector_avoidance_current_direction<VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS)
		{
			short current_direction = actor->control.vector_avoidance_current_direction;
			short next_direction = (current_direction + 1) % VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS;
			short next_direction2 = (current_direction + 2) % VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS;
			short previous_direction = (current_direction + 7) % VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS;
			short previous_direction2 = (current_direction + 6) % VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS;

			avoidance_weights[current_direction] += 0.4f;
			avoidance_weights[next_direction] += avoid_ray_adjacent_fractions[0] * 0.4f;
			avoidance_weights[next_direction2] += avoid_ray_adjacent_fractions[1] * 0.4f;
			avoidance_weights[previous_direction] += avoid_ray_adjacent_fractions[0] * 0.4f;
			avoidance_weights[previous_direction2] += avoid_ray_adjacent_fractions[1] * 0.4f;
		}

		for (sense_ray_index = 0; sense_ray_index<VECTOR_AVOIDANCE_NUMBER_OF_SENSE_RAYS; sense_ray_index++)
		{
			real sense_ray_t;
			real_point3d ray_origin;
			real_vector3d ray_direction;
			short sense_ray_result = actor_move_test_avoidance_vector(&avoidance_data, &sense_rays[sense_ray_index], &ray_origin, &ray_direction, &sense_ray_t, NULL);

#ifdef DEBUG
			actor_debug_info->ray_origin[sense_ray_index] = ray_origin;
			actor_debug_info->ray_direction[sense_ray_index] = ray_direction;
			actor_debug_info->avoidance_type[sense_ray_index] = sense_ray_result;
			actor_debug_info->collision_t[sense_ray_index] = sense_ray_t;
#endif

			if (sense_ray_result>_actor_vector_avoidance_clear)
			{
				for (weight_index = 0; weight_index<VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS; weight_index++)
				{
					real obstruction = (1.f - sense_ray_t) * 2.f;

					obstruction = MIN(obstruction, 1.f);
					avoidance_weights[weight_index] += obstruction * sense_ray_avoidance_weights[sense_ray_index][weight_index];
				}

				maximum_obstruction = MAX(maximum_obstruction, 1.f - sense_ray_t);
			}
		}

		for (direction_index = 0; direction_index<VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS; direction_index++)
		{
			short avoid_ray_result[VECTOR_AVOIDANCE_NUMBER_OF_AVOID_RAYS];
			real avoid_ray_t[VECTOR_AVOIDANCE_NUMBER_OF_AVOID_RAYS];
			short avoid_ray_index;
			boolean obstructed;
			real direction_weight;
			short next_direction;
			short next_direction2;
			short previous_direction;
			short previous_direction2;

			for (avoid_ray_index = 0; avoid_ray_index<VECTOR_AVOIDANCE_NUMBER_OF_AVOID_RAYS; avoid_ray_index++)
			{
				real_point3d ray_origin;
				real_vector3d ray_direction;

				avoid_ray_result[avoid_ray_index] = actor_move_test_avoidance_vector(
					&avoidance_data,
					&avoidance_rays[direction_index][avoid_ray_index],
					&ray_origin,
					&ray_direction,
					&avoid_ray_t[avoid_ray_index],
					&actor->control.vector_avoidance_clear_times[direction_index][avoid_ray_index]);
#ifdef DEBUG
				actor_debug_info->field_6358[direction_index][avoid_ray_index] = ray_origin;
				actor_debug_info->field_6418[direction_index][avoid_ray_index] = ray_direction;
				actor_debug_info->avoid_result[direction_index][avoid_ray_index] = avoid_ray_result[avoid_ray_index];
				actor_debug_info->avoid_t[direction_index][avoid_ray_index] = avoid_ray_t[avoid_ray_index];
#endif
			}

			obstructed = FALSE;
			direction_weight = 0.f;

			for (avoid_ray_index = VECTOR_AVOIDANCE_NUMBER_OF_AVOID_RAYS - 1; avoid_ray_index>=0; avoid_ray_index--)
			{
				if (avoid_ray_result[avoid_ray_index]==_actor_vector_avoidance_clear)
				{
					real clear_fraction = 1.f;

					if (!obstructed)
					{
						if (actor->control.vector_avoidance_clear_times[direction_index][avoid_ray_index]<avoid_ray_clear_bias_time)
						{
							clear_fraction = 0.f;
						}
						else
						{
							clear_fraction = 1.f - (real)avoid_ray_clear_bias_time / (real)actor->control.vector_avoidance_clear_times[direction_index][avoid_ray_index];
						}

						clear_fraction = PIN(clear_fraction, 0.f, 1.f);
					}

					direction_weight += clear_fraction * avoid_ray_avoidance_weights[avoid_ray_index];
				}
				else
				{
					real obstruction = (1.f - avoid_ray_t[avoid_ray_index]) * 2.f;

					obstruction = MIN(obstruction, 1.f);
					direction_weight -= obstruction * avoid_ray_avoidance_weights[avoid_ray_index];
					obstructed = TRUE;
				}
			}

			next_direction = (direction_index + 1) % VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS;
			next_direction2 = (direction_index + 2) % VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS;
			previous_direction = (direction_index + 7) % VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS;
			previous_direction2 = (direction_index + 6) % VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS;
			avoidance_weights[direction_index] += direction_weight;
			avoidance_weights[next_direction] += avoid_ray_adjacent_fractions[0] * direction_weight;
			avoidance_weights[next_direction2] += avoid_ray_adjacent_fractions[1] * direction_weight;
			avoidance_weights[previous_direction] += avoid_ray_adjacent_fractions[0] * direction_weight;
			avoidance_weights[previous_direction2] += avoid_ray_adjacent_fractions[1] * direction_weight;
		}

		turn_speed = magnitude3d(&object->object.angular_velocity);
#ifdef DEBUG
		actor_debug_info->field_6551 = FALSE;
#endif

		if (turn_speed>0.02f)
		{
			real_vector3d turn_direction_in_vector_space;
			real turn_direction_approximation;
			real turn_avoidance_weight_approximation;
			real turn_weight = (turn_speed - 0.02f) / 0.08f;
			boolean found = FALSE;

			turn_weight = 0.8f * MIN(turn_weight, 1.f);
			turn_direction_in_vector_space.i = 0.f;
			turn_direction_in_vector_space.j = dot_product3d(&avoidance_data.up, &object->object.angular_velocity);
			turn_direction_in_vector_space.k = -dot_product3d(&avoidance_data.left, &object->object.angular_velocity);

			if (normalize3d(&turn_direction_in_vector_space)>0.f)
			{
				found = actor_move_vector_avoidance_find_direction(
					VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS,
					avoidance_directions,
					avoidance_weights,
					&turn_direction_in_vector_space,
					&turn_direction_approximation,
					&turn_avoidance_weight_approximation);
			}

			if (found && turn_avoidance_weight_approximation>0.5f)
			{
				for (turn_index = 0; turn_index<VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS; turn_index++)
				{
					real projection = dot_product3d(&turn_direction_in_vector_space, &avoidance_directions[turn_index]);

					if (projection<0.f)
					{
						avoidance_weights[turn_index] += turn_weight * projection;
					}
				}
			}
			else
			{
				turn_weight = 0.f;
			}

#ifdef DEBUG
			actor_debug_info->field_6551 = TRUE;
			actor_debug_info->field_6554 = turn_weight;
			actor_debug_info->field_6558 = turn_speed;
			actor_debug_info->avoidance_vector = turn_direction_in_vector_space;
			actor_debug_info->field_6568 = turn_avoidance_weight_approximation;
#endif
		}

		best_avoidance_weight = -REAL_MAX;
		best_avoidance_direction = NONE;

		for (test_index = 0; test_index<VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS; test_index++)
		{
			if (avoidance_weights[test_index]>best_avoidance_weight)
			{
				best_avoidance_weight = avoidance_weights[test_index];
				best_avoidance_direction = test_index;
			}
		}

		match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 2435, (best_avoidance_direction >= 0) && (best_avoidance_direction < VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS));

#ifdef DEBUG
		memcpy(actor_debug_info->avoidance_weights, avoidance_weights, sizeof(avoidance_weights));
#endif

		movement_vector = *movement_direction;
		forward_projection = 1.f;
		movement_in_vector_space = *global_zero_vector3d;
		movement_direction_approximation = 0.f;
		movement_avoidance_weight_approximation = 0.f;

		if (normalize3d(&movement_vector)>0.f)
		{
			forward_projection = dot_product3d(&avoidance_data.forward, &movement_vector);
			movement_in_vector_space.i = 0.f;
			movement_in_vector_space.j = dot_product3d(&avoidance_data.left, &movement_vector);
			movement_in_vector_space.k = dot_product3d(&avoidance_data.up, &movement_vector);

			if (normalize3d(&movement_in_vector_space)>0.f)
			{
				actor_move_vector_avoidance_find_direction(
					VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS,
					avoidance_directions,
					avoidance_weights,
					&movement_in_vector_space,
					&movement_direction_approximation,
					&movement_avoidance_weight_approximation);
			}
		}

		match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 2466, (movement_direction_approximation >= 0) && (movement_direction_approximation <= ((real) VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS)));

#ifdef DEBUG
		actor_debug_info->field_6524 = avoidance_data.forward;
		actor_debug_info->field_6530 = *movement_direction;
		actor_debug_info->field_64FC = best_avoidance_weight;
		actor_debug_info->field_6500 = best_avoidance_direction;
		actor_debug_info->field_6504 = movement_direction_approximation;
		actor_debug_info->field_6508 = movement_avoidance_weight_approximation;
#endif

		avoidance_weight_difference = (best_avoidance_weight - movement_avoidance_weight_approximation);
		sharp_turn = FALSE;

		if (maximum_obstruction>0.6f)
		{
			obstruction_emergency = MIN(1.f, (maximum_obstruction - 0.6f) / (1.f - 0.6f)) + 1.f;
		}
		else
		{
			obstruction_emergency = MIN(1.f, maximum_obstruction / 0.3f);
		}

#ifdef DEBUG
		actor_debug_info->field_6510 = forward_projection;
		actor_debug_info->sign_no_danger = avoidance_weight_difference;
#endif

		if (forward_projection<-0.2f)
		{
			if (actor->control.vector_avoidance_sharp_turn_timer!=NONE && actor->control.vector_avoidance_sharp_turn_timer<3 * TICKS_PER_SECOND)
			{
				sharp_turn = TRUE;
#ifdef DEBUG
				actor_debug_info->field_653C = _vector_avoidance_decision_continue_sharp_turn;
#endif
			}
			else if (magnitude_squared3d(&object->object.angular_velocity)>0.05f * 0.05f)
			{
				sharp_turn = avoidance_weight_difference>2.f && best_avoidance_weight>2.f;

				if (sharp_turn)
				{
#ifdef DEBUG
					actor_debug_info->field_653C = _vector_avoidance_decision_sharp_turn_moving;
#endif
				}
			}
			else
			{
				sharp_turn = obstruction_emergency>0.5f;

				if (sharp_turn)
				{
#ifdef DEBUG
					actor_debug_info->field_653C = _vector_avoidance_decision_sharp_turn_stationary;
#endif
				}
			}
		}

		if (sharp_turn)
		{
			if (actor->control.vector_avoidance_sharp_turn_timer==NONE)
			{
				actor->control.vector_avoidance_sharp_turn_timer = 0;
			}
			else
			{
				actor->control.vector_avoidance_sharp_turn_timer++;
			}
		}
		else
		{
			actor->control.vector_avoidance_sharp_turn_timer = NONE;
		}

		if (sharp_turn)
		{
			real_vector3d worldspace_best_direction;
			real_vector3d rotation_axis;

			actor_move_transform_avoidance_vector(&avoidance_data, &avoidance_directions[best_avoidance_direction], &worldspace_best_direction);
			cross_product3d(movement_direction, &worldspace_best_direction, &rotation_axis);

			if (normalize3d(&rotation_axis)>0.f)
			{
				scale_vector3d(&rotation_axis, angle_between_vectors3d(movement_direction, &worldspace_best_direction), &rotation_vector);
			}

			rotation_emergency = (2.f - movement_avoidance_weight_approximation) / 2.f;
			rotation_emergency = PIN(rotation_emergency - 0.5f, 0.f, 1.f);
			rotation_emergency = MAX(rotation_emergency, obstruction_emergency);
			avoiding = TRUE;
		}
		else if (forward_projection<0.5f)
		{
			if (avoidance_weight_difference>1.3f)
			{
				real cosangle = dot_product3d(&movement_in_vector_space, &avoidance_directions[best_avoidance_direction]);

				if (cosangle>0.5f)
				{
					real rotation_angle;
					real_vector3d cross_vector;

					rotation_emergency = avoidance_weight_difference / 1.3f - 0.5f;
					rotation_emergency = PIN(rotation_emergency, 0.f, 1.f);
					rotation_emergency = MAX(rotation_emergency, obstruction_emergency);
					rotation_angle = (_pi / 3.f) * rotation_emergency;
					cross_product3d(&avoidance_directions[best_avoidance_direction], &movement_in_vector_space, &cross_vector);

					if (cross_vector.i>0.f)
					{
						rotation_angle = -rotation_angle;
					}

					scale_vector3d(&avoidance_data.forward, rotation_angle, &rotation_vector);
					avoiding = TRUE;
#ifdef DEBUG
					actor_debug_info->field_653C = _vector_avoidance_decision_sign_rotated;
					actor_debug_info->sign_rotated = rotation_angle;
#endif
				}
				else
				{
#ifdef DEBUG
					actor_debug_info->field_653C = _vector_avoidance_decision_sign_too_far;
					actor_debug_info->sign_too_far_cosangle = cosangle;
#endif
				}
			}
			else
			{
#ifdef DEBUG
				actor_debug_info->field_653C = _vector_avoidance_decision_sign_no_danger;
#endif
			}
		}
		else if (maximum_obstruction>0.f)
		{
			real rotation_angle = 0.f;

			rotation_emergency = obstruction_emergency;
			rotation_vector = *global_zero_vector3d;
			vector_from_line3d(&rotation_vector, &avoidance_data.left, -avoidance_directions[best_avoidance_direction].k, &rotation_vector);
			vector_from_line3d(&rotation_vector, &avoidance_data.up, avoidance_directions[best_avoidance_direction].j, &rotation_vector);

			if (normalize3d(&rotation_vector)>0.f)
			{
				rotation_angle = (_pi / 3.f) * obstruction_emergency;
				scale_vector3d(&rotation_vector, rotation_angle, &rotation_vector);
			}

			avoiding = TRUE;
#ifdef DEBUG
			actor_debug_info->field_653C = _vector_avoidance_decision_turn;
			actor_debug_info->field_6520 = rotation_angle;
			actor_debug_info->field_651C = maximum_obstruction;
#endif
		}
		else
		{
#ifdef DEBUG
			actor_debug_info->field_653C = _vector_avoidance_decision_none;
#endif
		}

#ifdef DEBUG
		actor_debug_info->field_6550 = avoiding;
#endif

		if (avoiding)
		{
			actor->control.vector_avoidance_current_direction = best_avoidance_direction;
		}
		else
		{
			actor->control.vector_avoidance_current_direction = NONE;
		}

#ifdef DEBUG
		actor_debug_info->avoidance_data = avoidance_data;
		actor_debug_info->field_6540 = rotation_vector;
		actor_debug_info->field_654C = rotation_emergency;
#endif
	}

	*avoidance_rotation = rotation_vector;
	*emergency_amount = rotation_emergency;

	return;
}

void actor_move_transform_avoidance_vector(
	struct vector_avoidance_data *avoidance_data,
	real_vector3d const *avoidance_vector,
	real_vector3d *direction_vector)
{
	*direction_vector = *global_zero_vector3d;
	vector_from_line3d(direction_vector, &avoidance_data->forward, avoidance_vector->i, direction_vector);
	vector_from_line3d(direction_vector, &avoidance_data->left, avoidance_vector->j, direction_vector);
	vector_from_line3d(direction_vector, &avoidance_data->up, avoidance_vector->k, direction_vector);

	return;
}

void actor_move_get_avoidance_direction(
	struct vector_avoidance_data *avoidance_data,
	real direction,
	real_vector3d *direction_vector)
{
	short direction_index;
	real_vector3d vectorspace_direction;
	real angle = REAL_MAX;

	if (direction<0.f || direction>=(real)VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS)
	{
		direction = 0.f;
	}

	for (direction_index = 0; direction_index<VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS; direction_index++)
	{
		if ((real)direction_index + 1.f>direction)
		{
			real fraction = direction - (real)direction_index;
			real current_angle = avoidance_ray_angles[direction_index];
			real next_angle = direction_index==VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS - 1 ? avoidance_ray_angles[0] : avoidance_ray_angles[direction_index + 1];

			angle = (1.f - fraction) * current_angle + next_angle * fraction;
			break;
		}
	}

	if (angle==REAL_MAX)
	{
		angle = 0.f;
		error(_error_silent, "warning: actor_move_get_avoidance_vector couldn't find out-of-bounds direction %.4f", direction);
	}

	match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 2743, (angle >= 0.0f) && (angle < _full_circle));

	set_real_vector3d(&vectorspace_direction, 0.f, cosine(angle), sine(angle));
	actor_move_transform_avoidance_vector(avoidance_data, &vectorspace_direction, direction_vector);

	return;
}

void actor_move_initialize(
	void)
{
	short sense_ray_index;
	short avoid_ray_index;

	for (sense_ray_index = 0; sense_ray_index<VECTOR_AVOIDANCE_NUMBER_OF_SENSE_RAYS; sense_ray_index++)
	{
		real sine_angle = sine(sense_ray_angles[sense_ray_index]);
		real cosine_angle = cosine(sense_ray_angles[sense_ray_index]);
		real sine_divergence = sine(sense_ray_divergence * sense_ray_divergences[sense_ray_index]);
		real cosine_divergence = cosine(sense_ray_divergence * sense_ray_divergences[sense_ray_index]);

		sense_rays[sense_ray_index].length = sense_ray_length;
		sense_rays[sense_ray_index].offset.i = 0.f;
		sense_rays[sense_ray_index].offset.j = sense_ray_offset * sense_ray_offsets[sense_ray_index] * cosine_angle;
		sense_rays[sense_ray_index].offset.k = sense_ray_offset * sense_ray_offsets[sense_ray_index] * sine_angle;
		sense_rays[sense_ray_index].divergence.i = cosine_divergence;
		sense_rays[sense_ray_index].divergence.j = sine_divergence * cosine_angle;
		sense_rays[sense_ray_index].divergence.k = sine_divergence * sine_angle;
	}

	for (avoid_ray_index = 0; avoid_ray_index<VECTOR_AVOIDANCE_NUMBER_OF_AVOID_RAYS; avoid_ray_index++)
	{
		short direction_index;
		real sine_divergence = sine(avoidance_ray_divergence[avoid_ray_index]);
		real cosine_divergence = cosine(avoidance_ray_divergence[avoid_ray_index]);

		for (direction_index = 0; direction_index<VECTOR_AVOIDANCE_NUMBER_OF_DIRECTIONS; direction_index++)
		{
			real sine_angle = sine(avoidance_ray_angles[direction_index]);
			real cosine_angle = cosine(avoidance_ray_angles[direction_index]);

			avoidance_directions[direction_index].i = 0.f;
			avoidance_directions[direction_index].j = cosine_angle;
			avoidance_directions[direction_index].k = sine_angle;
			avoidance_rays[direction_index][avoid_ray_index].length = avoidance_ray_length;
			scale_vector3d(&avoidance_directions[direction_index], avoidance_ray_offsets[avoid_ray_index], &avoidance_rays[direction_index][avoid_ray_index].offset);
			scale_vector3d(&avoidance_directions[direction_index], sine_divergence, &avoidance_rays[direction_index][avoid_ray_index].divergence);
			avoidance_rays[direction_index][avoid_ray_index].divergence.i = cosine_divergence;
		}
	}

	return;
}

void actor_path_clear(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->control.path.path.valid = FALSE;
	actor->control.path.at_destination = TRUE;
	actor->control.path.destination_original_distance = 0.f;

	return;
}

boolean actor_path_refresh(
	long actor_index,
	boolean new_destination,
	struct path_state *cached_path_state)
{
	real_point3d previous_destination_point;
	struct actor_datum *actor = actor_get(actor_index);
	boolean had_previous_destination = FALSE;
	boolean success = TRUE;

	if (actor->control.path.destination_orders.destination_type!=_destination_none &&
		actor->control.path.destination_orders.destination_type!=_destination_halt)
	{
		had_previous_destination = TRUE;
		previous_destination_point = actor->control.path.destination.point;
	}

	if (actor->input.vehicle_passenger ||
		actor->control.path.destination_orders.destination_type==_destination_none ||
		actor->control.path.destination_orders.destination_type==_destination_halt ||
		(actor->control.path.destination_orders.destination_type==_destination_firing_position && actor->firing_positions.moved_away_from_firing_position))
	{
		actor_path_clear(actor_index);
	}
	else
	{
		actor->control.path.path.valid = FALSE;
		actor->control.path.at_destination = FALSE;
		actor->control.path.destination_original_distance = 0.f;
		actor->control.movement_complete = FALSE;

		switch (actor->control.path.destination_orders.destination_type)
		{
		case _destination_raw_location:
			actor->control.path.destination.point = actor->control.path.destination_orders.raw.point;
			actor->control.path.destination.surface_index = actor->control.path.destination_orders.raw.surface_index;
			actor->control.path.destination.target_radius = 0.f;
			break;

		case _destination_move_position:
			success = FALSE;

			if (actor->meta.encounter_index!=NONE)
			{
				struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
					&global_scenario_get()->ai_encounters,
					DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
					struct encounter_definition);
				struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter_definition->squads,
					actor->meta.squad_index,
					struct squad_definition);

				if (actor->control.path.destination_orders.move_position_index>=0 &&
					actor->control.path.destination_orders.move_position_index<squad_definition->move_positions.count)
				{
					struct move_position_definition *move_position = TAG_BLOCK_GET_ELEMENT(
						&squad_definition->move_positions,
						actor->control.path.destination_orders.move_position_index,
						struct move_position_definition);

					actor->control.path.destination.point = move_position->position;
					actor->control.path.destination.surface_index = move_position->surface_index;
					actor->control.path.destination.target_radius = 0.f;
					success = TRUE;
				}
			}
			break;

		case _destination_firing_position:
			if (actor->meta.encounter_index!=NONE)
			{
				struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
					&global_scenario_get()->ai_encounters,
					DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
					struct encounter_definition);
				struct firing_position_definition *firing_position = TAG_BLOCK_GET_ELEMENT(
					&encounter_definition->firing_positions,
					actor->control.path.destination_orders.firing_position_index,
					struct firing_position_definition);

				actor->control.path.destination.point = firing_position->position;
				actor->control.path.destination.surface_index = firing_position->surface_index;
				actor->control.path.destination.target_radius = 0.f;
			}
			else
			{
				success = FALSE;
			}
			break;

		case _destination_prop:
		{
			struct prop_datum *prop = prop_get(actor->control.path.destination_orders.prop.prop_index);

			if (!prop_orphaned(prop))
			{
				actor_perception_find_prop_pathfinding_location(actor_index, actor->control.path.destination_orders.prop.prop_index);
			}

			if (actor->state.flying)
			{
				actor->control.path.destination.point = prop->center_of_mass;
			}
			else
			{
				actor->control.path.destination.point = prop->pathfinding_point;
			}

			actor->control.path.destination.surface_index = prop->pathfinding_surface_index;
			actor->control.path.destination.target_radius = actor->control.path.destination_orders.prop.accept_radius;
			break;
		}

		default:
			success = FALSE;
			match_halt("c:\\halo\\SOURCE\\ai\\actor_moving.c", 2943);
			break;
		}

		if (success)
		{
			real avoidance_distance = 0.f;
			boolean destination_valid = TRUE;

			if (actor->state.flying)
			{
				real available_avoidance_distance = 0.f;

				destination_valid = actor_path_3d_available(actor_index, &actor->control.path.destination.point, &available_avoidance_distance);
			}
			else if (actor->control.path.destination.target_radius==0.f)
			{
				destination_valid = actor->control.path.destination.surface_index!=NONE;
			}

			if (!destination_valid)
			{
				success = FALSE;
			}
			else
			{
				boolean refresh = TRUE;

				if (actor_test_destination(actor_index))
				{
					refresh = had_previous_destination &&
						distance_squared3d(&previous_destination_point, &actor->control.path.destination.point)>0.1f * 0.1f;
				}

				if (refresh)
				{
					struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
					real destination_distance = distance3d(&actor->input.position.body_position, &actor->control.path.destination.point);
#ifdef DEBUG
					struct actor_debug_info *actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];

					actor_debug_info->last_path_refresh = game_time_get();
#endif

					if (actor->state.flying)
					{
						success = path_3d_build_path(
							global_structure_bsp_get(),
							&actor->input.position.body_position,
							avoidance_distance,
							&actor->control.path.destination.point,
							&actor->control.path.path);
					}
					else if (cached_path_state)
					{
						match_assert("c:\\halo\\SOURCE\\ai\\actor_moving.c", 3004, actor->control.path.destination_orders.ignore_target_object_index == NONE);

						path_state_destination(
							cached_path_state,
							&actor->control.path.destination.point,
							actor->control.path.destination.surface_index,
							actor->control.path.destination.target_radius);
						success = path_state_build_path(cached_path_state, &actor->control.path.path);
					}
					else
					{
						struct path_input input;
						struct path_state state;

						actor_path_input_new(actor_index, &input);

						if (actor->control.path.destination_orders.ignore_target_object_index!=NONE)
						{
							path_input_set_target_object(&input, actor->control.path.destination_orders.ignore_target_object_index);
						}

						if (actor->danger_zone.danger_type>_actor_danger_zone_none &&
							!actor->danger_zone.attached_to_us &&
							!TEST_FLAG(actor_definition->flags2, _actor_definition_flags2_pathfinding_ignores_danger_bit))
						{
							path_input_set_attractor(
								&input,
								&actor->danger_zone.position,
								actor->danger_zone.danger_radius,
								actor->danger_zone.object_index,
								10.f);
						}

#ifdef DEBUG
						path_state_new(&input, &state, ai_debug_get_path_storage(actor_index));
#endif
						path_state_destination(
							&state,
							&actor->control.path.destination.point,
							actor->control.path.destination.surface_index,
							actor->control.path.destination.target_radius);
						success = FALSE;

						if (path_state_find(&state) && path_state_build_path(&state, &actor->control.path.path))
						{
							success = TRUE;
						}
					}

					actor->control.path.refreshed_this_tick = TRUE;

					if (new_destination)
					{
						actor->control.path.destination_original_distance = destination_distance;
					}

					if (success &&
						actor->control.path.path.endpoint.target_radius>0.f &&
						destination_distance<actor->control.path.destination.target_radius &&
						destination_distance - actor->control.path.path.endpoint.target_radius<0.5f)
					{
						actor_path_clear(actor_index);
					}
				}
			}
		}

		if (!success)
		{
			actor_path_clear(actor_index);
		}
	}

	return success;
}

boolean actor_path_has_path(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	return actor->control.path.path.valid;
}

boolean actor_path_at_destination(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	return !actor->control.path.path.valid || actor->control.path.at_destination;
}

short actor_path_get_destination_firing_position_index(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	short firing_position_index = NONE;

	if (actor->control.path.destination_orders.destination_type==_destination_firing_position)
	{
		firing_position_index = actor->control.path.destination_orders.firing_position_index;
	}

	return firing_position_index;
}

void actor_path_input_new(
	long actor_index,
	struct path_input *input)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	real pathfinding_radius = actor_definition->moving.pathfinding_radius;
	long source_object_index = actor->meta.unit_index;

	if (actor->input.vehicle_driver_type>_actor_vehicle_driver_none)
	{
		struct vehicle_datum *vehicle = vehicle_get(actor->input.vehicle_index);
		struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

		source_object_index = actor->input.vehicle_index;

		if (vehicle_definition->vehicle.ai_pathfinding_radius>0.f)
		{
			pathfinding_radius = vehicle_definition->vehicle.ai_pathfinding_radius;
		}
	}

	actor_find_pathfinding_location(actor_index);
	path_input_new(input, pathfinding_radius, actor->emotions.ignorant_of_broken_surfaces, source_object_index);
	path_input_set_start(input, &actor->input.pathfinding_point, actor->input.pathfinding_surface_index);

	return;
}

boolean actor_path_3d_available(
	long actor_index,
	real_point3d const *destination_point,
	real *avoidance_distance_reference)
{
	struct actor_datum *actor = actor_get(actor_index);
	real avoidance_distance = 0.f;
	boolean available = TRUE;

	if (actor->input.vehicle_driver_type==_actor_vehicle_driver_directional_flying)
	{
		struct vehicle_datum *vehicle = vehicle_get(actor->input.vehicle_index);
		struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

		avoidance_distance = vehicle_definition->vehicle.ai_avoidance_distance;
	}

	if (avoidance_distance>0.f && actor->control.vector_avoidance_rotation_emergency_instantaneous>0.9f)
	{
		real_vector3d path_direction;

		vector_from_points3d(&actor->input.position.body_position, destination_point, &path_direction);

		if (normalize3d(&path_direction)>0.f)
		{
			if (dot_product3d(&path_direction, &actor->input.facing_vector)>0.984f)
			{
				available = FALSE;
			}
		}
	}

	if (avoidance_distance_reference)
	{
		*avoidance_distance_reference = avoidance_distance;
	}

	return available;
}
