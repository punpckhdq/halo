/*
ACTION_VEHICLE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "collisions.h"
#include "vehicles.h"
#include "game.h"
#include "scenario.h"

/* ---------- constants */

enum
{
	VEHICLE_STUCK_DETECTION_MAXIMUM_COUNT = 8, /* fake name */
	VEHICLE_FAKE_ENTRY_TICKS = 30, /* fake name */
};

/* ---------- macros */

#define VEHICLE_STUCK_DETECTION_TICKS (5*TICKS_PER_SECOND) /* fake name */

/* ---------- structures */

/* ---------- prototypes */

static boolean action_vehicle_desirable(long actor_index, long vehicle_index, boolean scripted_request, real attempt_start_distance, real attempt_continue_distance, boolean already_inside, boolean already_attempting_entry);
static boolean action_vehicle_evaluate_seat(long actor_index, long vehicle_index, short seat_index, boolean allow_any_seat, real_point3d *entry_point, real_vector3d *entry_facing, real_point3d *hint_point, real *seat_weight_reference, boolean *within_range_reference, boolean *correct_facing_reference, boolean *could_potentially_fake_reference);
static boolean action_vehicle_find_destination(long actor_index, long vehicle_index, real_point3d *entry_point, real_vector3d *entry_facing, real_point3d *hint_point, boolean *ignore_hint_reference, real_point3d *destination_point, long *surface_index_reference);
static short action_vehicle_find_impromptu_seat(long actor_index, long vehicle_index, real_point3d *entry_point, real_vector3d *entry_facing, real_point3d *hint_point);

/* ---------- globals */

/* ---------- public code */

void action_vehicle_begin(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct vehicle_state_data *state_data = &actor->state.action_data.vehicle;

	state_data->stuck_detection_counter = 0;
	state_data->stuck_detection_time = game_time_get();
	state_data->stuck_detection_point = actor->input.position.body_position;

	return;
}

void action_vehicle_update(
	long actor_index)
{
	return;
}

void action_vehicle_flush_structure_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct vehicle_state_data *state_data = &actor->state.action_data.vehicle;

	state_data->destination_surface_index = NONE;

	return;
}

void action_vehicle_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct vehicle_state_data *state_data = &actor->state.action_data.vehicle;

	if (state_data->lock_facing)
	{
		actor->orders.look.primary_priority = _primary_priority_exact_facing;
		actor->orders.look.primary_direction.type = _direction_specification_vector;
		actor->orders.look.primary_direction.vector = state_data->destination_facing;
	}
	else if (actor_path_has_path(actor_index))
	{
		actor->orders.look.primary_priority = _primary_priority_facing;
		actor->orders.look.primary_direction.type = _direction_specification_movement;
	}
	else
	{
		actor->orders.look.primary_priority = _primary_priority_none;
	}

	actor->orders.look.idle_look_type = _idle_look_combat;
	actor->orders.combat.shoot_at_target = FALSE;
	actor->orders.move.stationary_crouch = FALSE;
	actor->orders.move.moving_crouch = FALSE;
	actor->orders.move.panicked = FALSE;
	actor->orders.move.dive_into_cover = FALSE;
	actor->orders.move.emerge_from_cover = FALSE;

	return;
}

static boolean action_vehicle_desirable(
	long actor_index,
	long vehicle_index,
	boolean scripted_request,
	real attempt_start_distance,
	real attempt_continue_distance,
	boolean already_inside,
	boolean already_attempting_entry)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *vehicle = unit_get(vehicle_index);
	boolean desirable = FALSE;

	if (TEST_FLAG(vehicle->object.damage_flags, _object_dead_bit))
	{
		desirable = FALSE;
	}
	else if (scripted_request)
	{
		desirable = TRUE;
	}
	else
	{
		real_point3d origin;
		real distance = already_attempting_entry ? attempt_start_distance : attempt_continue_distance;

		object_get_origin(vehicle_index, &origin);
		if (already_inside || distance_squared3d(&actor->input.position.body_position, &origin) < distance * distance)
		{
			desirable = TRUE;
			if (!already_inside && !already_attempting_entry && magnitude_squared3d(&vehicle->object.translational_velocity) > (0.5f / TICKS_PER_SECOND) * (0.5f / TICKS_PER_SECOND))
			{
				desirable = FALSE;
			}
		}
	}

	if (!already_inside && vehicle->object.up.k < 0.5f)
	{
		desirable = FALSE;
	}

	return desirable;
}

static boolean action_vehicle_evaluate_seat(
	long actor_index,
	long vehicle_index,
	short seat_index,
	boolean allow_any_seat,
	real_point3d *entry_point,
	real_vector3d *entry_facing,
	real_point3d *hint_point,
	real *seat_weight_reference,
	boolean *within_range_reference,
	boolean *correct_facing_reference,
	boolean *could_potentially_fake_reference)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	boolean result = FALSE;
	real_point3d seat_point;
	real_point3d seat_entry_point;
	real_point3d seat_hint_point;
	real_vector3d seat_entry_facing;

	if (!unit_seat_filled(vehicle_index, seat_index) &&
		(!TEST_FLAG(actor_definition->flags2, _actor_definition_flags2_disallow_vehicle_combat_bit) || unit_seat_allow_noncombatants(vehicle_index, seat_index)) &&
		unit_get_seat_entrance_point(actor->meta.unit_index, vehicle_index, seat_index, &seat_entry_point, &seat_point, &seat_hint_point))
	{
		real_point3d vehicle_origin;
		real distance;
		boolean within_range;
		boolean correct_facing;
		boolean could_fake;
		boolean driver_seat_claimed = FALSE;
		real weight = 0.0f;

		result = TRUE;
		object_get_origin(vehicle_index, &vehicle_origin);

		vector_from_points2d((real_point2d *)&seat_entry_point, (real_point2d *)&seat_point, (real_vector2d *)&seat_entry_facing);
		seat_entry_facing.k = 0.0f;
		if (normalize2d((real_vector2d *)&seat_entry_facing) == 0.0f)
		{
			seat_entry_facing = actor->input.facing_vector;
		}

		distance = MIN(distance2d((real_point2d *)&actor->input.position.body_position, (real_point2d *)&seat_entry_point),
			distance2d((real_point2d *)&actor->input.position.body_position, (real_point2d *)&seat_point));

		{
			struct prop_iterator iterator;
			struct prop_datum *prop;

			prop_iterator_new(&iterator, actor_index);
			while (prop = prop_iterator_next(&iterator))
			{
				if (!prop->enemy && prop->actor_index != NONE)
				{
					struct actor_datum *other_actor = actor_try_and_get(prop->actor_index);

					if (other_actor->state.action == _actor_action_vehicle && other_actor->state.action_data.vehicle.vehicle_index == vehicle_index)
					{
						if (other_actor->state.action_data.vehicle.seat_index == seat_index)
						{
							if (distance_squared2d((real_point2d *)&other_actor->input.position.body_position, (real_point2d *)&other_actor->state.action_data.vehicle.destination_point) < distance * distance)
							{
								result = FALSE;
								break;
							}
						}
						else if (unit_seat_is_driver(vehicle_index, other_actor->state.action_data.vehicle.seat_index))
						{
							driver_seat_claimed = TRUE;
						}
					}
				}
			}
		}

		if (result)
		{
			struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
			real_vector2d actor_to_seat;
			real facing_dot;

			if (!allow_any_seat)
			{
				struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

				if (unit_seat_is_driver(vehicle_index, seat_index))
				{
					result = TEST_FLAG(vehicle_definition->vehicle.flags, _vehicle_ai_driver_enable_bit);
				}
				else if (!TEST_FLAG(vehicle_definition->vehicle.flags, _vehicle_ai_does_not_require_driver_bit))
				{
					result = vehicle->unit.driver_object_index != NONE || driver_seat_claimed;
				}
			}

			vector_from_points2d((real_point2d *)&actor->input.position.body_position, (real_point2d *)&seat_point, &actor_to_seat);
			normalize2d(&actor_to_seat);
			facing_dot = dot_product2d(&actor_to_seat, (real_vector2d *)&actor->input.facing_vector);

			within_range = distance < 0.7f;
			correct_facing = facing_dot > 0.6f;
			could_fake = distance < 1.1f && facing_dot > 0.0f;

			weight += 10.0f / (distance + 1.0f);
			if (TEST_FLAG(actor_variant_definition->flags, _actor_variant_definition_prefer_passenger_seat_bit))
			{
				if (!unit_seat_is_gunner(vehicle_index, seat_index))
				{
					weight += 3.5f;
				}
			}
			else
			{
				if (unit_seat_is_gunner(vehicle_index, seat_index))
				{
					weight += 3.5f;
				}
			}

			if (entry_point)
			{
				*entry_point = seat_entry_point;
			}
			if (entry_facing)
			{
				*entry_facing = seat_entry_facing;
			}
			if (hint_point)
			{
				*hint_point = seat_hint_point;
			}
			if (seat_weight_reference)
			{
				*seat_weight_reference = weight;
			}
			if (within_range_reference)
			{
				*within_range_reference = within_range;
			}
			if (correct_facing_reference)
			{
				*correct_facing_reference = correct_facing;
			}
			if (could_potentially_fake_reference)
			{
				*could_potentially_fake_reference = could_fake;
			}

			// overrides the driver checks above
			result = TRUE;
		}
	}

	return result;
}

static boolean action_vehicle_find_destination(
	long actor_index,
	long vehicle_index,
	real_point3d *entry_point,
	real_vector3d *entry_facing,
	real_point3d *hint_point,
	boolean *ignore_hint_reference,
	real_point3d *destination_point,
	long *surface_index_reference)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *vehicle = unit_get(vehicle_index);
	struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
	struct actor_debug_info *actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
	real_point3d modified_destination = *entry_point;
	boolean success = FALSE;
	boolean ignore_hint = ignore_hint_reference ? *ignore_hint_reference : FALSE;

	if (!TEST_FLAG(vehicle_definition->unit.flags, _unit_has_entrance_points_inside_bounding_sphere_bit))
	{
		real_point3d vehicle_center;
		real vehicle_radius;
		real_point3d target_point;
		real_vector2d actor_to_vehicle;
		real_vector2d actor_to_target_point;
		real_vector2d target_point_to_vehicle;
		real distance_squared;
		real_point3d *actor_position;

		object_get_bounding_sphere(vehicle_index, &vehicle_center, &vehicle_radius);
		if (vehicle_definition->unit.ai_vehicle_avoidance_radius > 0.0f)
		{
			vehicle_radius = vehicle_definition->unit.ai_vehicle_avoidance_radius;
		}

		if (!ignore_hint)
		{
			real hint_distance = distance3d(&vehicle_center, hint_point);

			if (hint_distance < 0.5f)
			{
				ignore_hint = TRUE;
			}
			else
			{
				vehicle_radius = MAX(vehicle_radius, hint_distance + 0.3f);
			}
		}

		target_point = *(ignore_hint ? entry_point : hint_point);

		actor_position = &actor->input.position.body_position;
		set_real_vector2d(&actor_to_vehicle, vehicle_center.x - actor_position->x, vehicle_center.y - actor_position->y);
		vector_from_points2d((real_point2d *)actor_position, (real_point2d *)&target_point, &actor_to_target_point);
		vector_from_points2d((real_point2d *)&target_point, (real_point2d *)&vehicle_center, &target_point_to_vehicle);

		if (!ignore_hint)
		{
			real_vector2d hint_vector;
			real_vector2d perpendicular_vector;

			vector_from_points2d((real_point2d *)entry_point, (real_point2d *)hint_point, &hint_vector);
			point_from_line2d((real_point2d *)&actor_to_target_point, &hint_vector, -dot_product2d(&actor_to_target_point, &hint_vector), (real_point2d *)&perpendicular_vector);
			if (magnitude_squared2d(&perpendicular_vector) < 0.35f * 0.35f)
			{
				ignore_hint = TRUE;
			}
		}

		actor_debug_info->last_vehicle_avoidance_time = game_time_get();
		actor_debug_info->vehicle_avoidance_point = *actor_position;
		actor_debug_info->vehicle_center = vehicle_center;
		actor_debug_info->vehicle_radius = vehicle_radius;
		actor_debug_info->vehicle_intended_entry_point = target_point;
		actor_debug_info->vehicle_intersect_t = REAL_MAX;
		actor_debug_info->vehicle_modified = FALSE;

		distance_squared = magnitude_squared2d(&actor_to_target_point);
		if (distance_squared > 0.0f)
		{
			real_vector2d avoid_vehicle_direction;
			real t = dot_product2d(&actor_to_target_point, &actor_to_vehicle) / distance_squared;
			boolean avoid = FALSE;

			actor_debug_info->vehicle_intersect_t = t;
			if (t > 0.0f && t < 1.2f)
			{
				perpendicular2d(&actor_to_target_point, &avoid_vehicle_direction);
				if (dot_product2d(&actor_to_vehicle, &avoid_vehicle_direction) > 0.0f)
				{
					negate_vector2d(&avoid_vehicle_direction, &avoid_vehicle_direction);
				}
				avoid = TRUE;
			}
			else if (!ignore_hint)
			{
				negate_vector2d(&target_point_to_vehicle, &avoid_vehicle_direction);
				avoid = TRUE;
			}

			if (avoid && normalize2d(&avoid_vehicle_direction) > 0.0f)
			{
				real_vector3d actor_to_destination;
				real destination_distance_squared;

				point_from_line2d((real_point2d *)&vehicle_center, &avoid_vehicle_direction, vehicle_radius * 1.1f, (real_point2d *)&modified_destination);

				actor_to_destination.i = modified_destination.x - actor_position->x;
				actor_to_destination.j = modified_destination.y - actor_position->y;
				actor_to_destination.k = modified_destination.z - actor_position->z;
				destination_distance_squared = magnitude_squared3d(&actor_to_destination);
				if (destination_distance_squared > 0.0001f && destination_distance_squared < 4.0f)
				{
					real_vector3d tangential_offset_vector;
					real destination_distance = square_root(destination_distance_squared);

					perpendicular2d(&target_point_to_vehicle, (real_vector2d *)&tangential_offset_vector);
					if (dot_product2d((real_vector2d *)&tangential_offset_vector, (real_vector2d *)&actor_to_destination) < 0.0f)
					{
						negate_vector2d((real_vector2d *)&tangential_offset_vector, (real_vector2d *)&tangential_offset_vector);
					}
					tangential_offset_vector.k = 0.0f;

					if (normalize2d((real_vector2d *)&tangential_offset_vector) > 0.0f)
					{
						point_from_line3d(&modified_destination, &tangential_offset_vector, 2.0f - destination_distance, &modified_destination);
					}
				}

				actor_debug_info->vehicle_modified = TRUE;
				actor_debug_info->vehicle_modified_point = modified_destination;
			}
		}
	}

	if (ignore_hint_reference)
	{
		*ignore_hint_reference = ignore_hint;
	}

	{
		real_point3d collision_origin;
		real_vector3d collision_vector;
		struct collision_bsp const *bsp = global_collision_bsp_get();
		struct collision_bsp_test_vector_result result;

		point_from_line3d(&modified_destination, global_up3d, 1.0f, &collision_origin);
		scale_vector3d(global_down3d, 4.0f, &collision_vector);

		if (collision_bsp_test_vector(FLAG(_collision_bsp_test_front_facing_surfaces_bit), bsp, 0, NULL, &collision_origin, &collision_vector, REAL_MAX, &result))
		{
			*surface_index_reference = result.surface_index;
			point_from_line3d(&collision_origin, &collision_vector, result.t, destination_point);
			success = TRUE;
		}
	}

	return success;
}

boolean action_vehicle_setup_specific(
	long actor_index,
	long vehicle_index,
	short seat_index,
	struct vehicle_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;
	real_point3d entry_point;
	real_vector3d entry_facing;
	real_point3d hint_point;

	match_assert("c:\\halo\\SOURCE\\ai\\action_vehicle.c", 104, state_data);
	memset(state_data, 0, sizeof(*state_data));

	if (actor->input.vehicle_index == NONE &&
		!actor->meta.swarm &&
		action_vehicle_desirable(actor_index, vehicle_index, TRUE, 0.0f, 0.0f, FALSE, FALSE))
	{
		state_data->vehicle_index = vehicle_index;
		state_data->seat_index = seat_index;
		state_data->impromptu = FALSE;

		if (unit_has_animation_to_enter_seat(actor->meta.unit_index, vehicle_index, seat_index) &&
			action_vehicle_evaluate_seat(actor_index, vehicle_index, seat_index, TRUE, &entry_point, &entry_facing, &hint_point, NULL, NULL, NULL, NULL) &&
			action_vehicle_find_destination(actor_index, vehicle_index, &entry_point, &entry_facing, &hint_point, NULL, &state_data->destination_point, &state_data->destination_surface_index) &&
			actor_move_to_point(actor_index, &state_data->destination_point, state_data->destination_surface_index, vehicle_index))
		{
			result = TRUE;
		}
	}

	return result;
}

boolean action_vehicle_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct vehicle_state_data *state_data = &actor->state.action_data.vehicle;
	struct vehicle_datum *vehicle = vehicle_try_and_get(state_data->vehicle_index);

	match_assert("c:\\halo\\SOURCE\\ai\\action_vehicle.c", 161, !actor->meta.swarm);

	if (actor->input.vehicle_index != NONE)
	{
		state_data->vehicle_entry_done = TRUE;
	}
	else if (state_data->started_entry)
	{
		// waiting for the entry animation to finish
	}
	else if (!vehicle)
	{
		state_data->vehicle_entry_failed = TRUE;
		state_data->vehicle_index = NONE;
	}
	else if (!action_vehicle_desirable(actor_index, state_data->vehicle_index, !state_data->impromptu, state_data->attempt_distance, state_data->continue_distance, FALSE, TRUE))
	{
		state_data->vehicle_entry_failed = TRUE;
	}
	else
	{
		long time = game_time_get();
		real_point3d entry_point;
		real_vector3d entry_facing;
		real_point3d hint_point;
		boolean within_range;
		boolean correct_facing;
		boolean could_fake;

		if (time >= state_data->stuck_detection_time + VEHICLE_STUCK_DETECTION_TICKS)
		{
			state_data->stuck_detection_time = time;
			if (distance_squared3d(&state_data->stuck_detection_point, &actor->input.position.body_position) < 25.0f)
			{
				state_data->stuck_detection_counter++;
			}
			else
			{
				state_data->stuck_detection_counter = 0;
				state_data->stuck_detection_point = actor->input.position.body_position;
			}
		}

		if (state_data->stuck_detection_counter >= VEHICLE_STUCK_DETECTION_MAXIMUM_COUNT)
		{
			state_data->vehicle_entry_failed = TRUE;
		}
		else if (action_vehicle_evaluate_seat(actor_index, state_data->vehicle_index, state_data->seat_index, !state_data->impromptu, &entry_point, &entry_facing, &hint_point, NULL, &within_range, &correct_facing, &could_fake))
		{
			if (could_fake)
			{
				if (++state_data->fake_entry_potential_timer >= VEHICLE_FAKE_ENTRY_TICKS)
				{
					correct_facing = TRUE;
					within_range = TRUE;
				}
			}
			else
			{
				state_data->fake_entry_potential_timer = 0;
			}

			if (within_range && correct_facing)
			{
				unit_enter_seat(actor->meta.unit_index, state_data->vehicle_index, state_data->seat_index);
				state_data->started_entry = TRUE;
			}
			else if (within_range)
			{
				actor_move_halt(actor_index);
			}
			else if (actor->meta.timeslice)
			{
				boolean path_found = FALSE;

				if (action_vehicle_find_destination(actor_index, state_data->vehicle_index, &entry_point, &entry_facing, &hint_point, &state_data->ignore_hint, &state_data->destination_point, &state_data->destination_surface_index) &&
					actor_move_to_point(actor_index, &state_data->destination_point, state_data->destination_surface_index, state_data->vehicle_index))
				{
					path_found = TRUE;
				}

				if (path_found)
				{
					state_data->pathfinding_failures = 0;
				}
				else
				{
					short maximum_pathfinding_failures = state_data->impromptu ? 5 : 50;

					if (++state_data->pathfinding_failures > maximum_pathfinding_failures)
					{
						state_data->vehicle_entry_failed = TRUE;
					}
				}
			}

			state_data->lock_facing = distance_squared3d(&actor->input.position.body_position, &entry_point) < 1.0f;
			state_data->destination_facing = entry_facing;
			state_data->currently_correct_facing = correct_facing;
			state_data->currently_within_range = within_range;
		}
		else
		{
			state_data->vehicle_entry_failed = TRUE;
		}
	}

	return state_data->vehicle_entry_done || state_data->vehicle_entry_failed;
}

static short action_vehicle_find_impromptu_seat(
	long actor_index,
	long vehicle_index,
	real_point3d *entry_point,
	real_vector3d *entry_facing,
	real_point3d *hint_point)
{
	struct unit_datum *vehicle = unit_get(vehicle_index);
	struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
	short best_seat_index = NONE;
	real best_weight = 0.0f;
	real_point3d seat_entry_point;
	real_vector3d seat_entry_facing;
	real_point3d seat_hint_point;
	short seat_index;

	for (seat_index = 0; seat_index < vehicle_definition->unit.seats.count; seat_index++)
	{
		real weight;
		real_point3d entry_point;
		real_vector3d entry_facing;
		real_point3d hint_point;

		if (action_vehicle_evaluate_seat(actor_index, vehicle_index, seat_index, FALSE, &entry_point, &entry_facing, &hint_point, &weight, NULL, NULL, NULL) &&
			weight > best_weight)
		{
			best_seat_index = seat_index;
			best_weight = weight;
			seat_entry_point = entry_point;
			seat_entry_facing = entry_facing;
			seat_hint_point = hint_point;
		}
	}

	if (entry_point)
	{
		*entry_point = seat_entry_point;
	}
	if (entry_facing)
	{
		*entry_facing = seat_entry_facing;
	}
	if (hint_point)
	{
		*hint_point = seat_hint_point;
	}

	return best_seat_index;
}

boolean action_vehicle_setup_impromptu(
	long actor_index,
	long vehicle_index,
	real attempt_distance,
	real continue_distance,
	struct vehicle_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;
	real_point3d entry_point;
	real_vector3d entry_facing;
	real_point3d hint_point;

	match_assert("c:\\halo\\SOURCE\\ai\\action_vehicle.c", 56, state_data);
	memset(state_data, 0, sizeof(*state_data));

	state_data->attempt_distance = attempt_distance;
	state_data->continue_distance = continue_distance;

	if (actor->input.vehicle_index == NONE &&
		!actor->meta.swarm &&
		actor->state.action != _actor_action_vehicle &&
		action_vehicle_desirable(actor_index, vehicle_index, FALSE, state_data->attempt_distance, state_data->continue_distance, FALSE, FALSE))
	{
		state_data->vehicle_index = vehicle_index;
		state_data->seat_index = action_vehicle_find_impromptu_seat(actor_index, vehicle_index, &entry_point, &entry_facing, &hint_point);
		if (state_data->seat_index != NONE)
		{
			state_data->impromptu = TRUE;

			if (unit_has_animation_to_enter_seat(actor->meta.unit_index, vehicle_index, state_data->seat_index) &&
				action_vehicle_find_destination(actor_index, vehicle_index, &entry_point, &entry_facing, &hint_point, NULL, &state_data->destination_point, &state_data->destination_surface_index) &&
				actor_move_to_point(actor_index, &state_data->destination_point, state_data->destination_surface_index, vehicle_index))
			{
				result = TRUE;
			}
		}
	}

	return result;
}

/* ---------- private code */
