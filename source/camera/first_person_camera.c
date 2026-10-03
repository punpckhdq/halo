/*
FIRST_PERSON_CAMERA.C
*/

/* ---------- headers */

#include "cseries.h"
#include "first_person_camera.h"
#include "director.h"
#include "observer.h"
#include "network_game_globals.h"
#include "players.h"
#include "console.h"
#include "vehicles.h"
#include "player_effects.h"

/* ---------- prototypes */

static void first_person_camera_for_unit_and_vector(long unit_index, real_vector3d const *forward, struct observer_command *result);

/* ---------- public code */

void first_person_camera_new(
	struct first_person_camera *camera)
{
	match_assert("c:\\halo\\SOURCE\\camera\\first_person_camera.c", 24, camera);

	camera->last_field_of_view = 0.f;

	return;
}

void first_person_camera_deterministic(
	long unit_index,
	real_point3d *position,
	real_vector3d *forward)
{
	struct object_marker marker;
	struct unit_datum *unit = unit_get(unit_index);

	unit_get_camera_position(unit_index, position);
	*forward = unit->unit.aiming_vector;

	if (unit->object.parent_object_index!=NONE)
	{
		struct object_datum *vehicle = (struct object_datum *)object_try_and_get_and_verify_type(unit->object.parent_object_index, _object_mask_vehicle);

		if (vehicle)
		{
			struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

			if (TEST_FLAG(seat->flags, _unit_seat_slave_first_person_camera_bit))
			{
				if (object_get_marker_by_name(unit->object.parent_object_index, "primary trigger", &marker, 1))
				{
					*position = marker.matrix.position;
					*forward = marker.matrix.forward;
				}
			}
		}
	}

	return;
}

void first_person_camera_fake(
	long unit_index,
	struct observer_command *result)
{
	struct unit_datum *unit = unit_get(unit_index);

	first_person_camera_for_unit_and_vector(unit_index, &unit->unit.aiming_vector, result);

	return;
}

void first_person_camera_update(
	struct first_person_camera *camera,
	struct camera_control const *controls,
	struct observer_command *result)
{
	real_vector3d forward;
	long unit_index = player_control_get_unit_index(controls->local_player_index);

	match_assert("c:\\halo\\SOURCE\\camera\\first_person_camera.c", 157, camera);
	match_assert("c:\\halo\\SOURCE\\camera\\first_person_camera.c", 158, result);

	player_control_get_facing_direction(controls->local_player_index, &forward);
	first_person_camera_for_unit_and_vector(unit_index, &forward, result);

	result->field_of_view = player_control_get_field_of_view(controls->local_player_index);
	if (result->field_of_view!=camera->last_field_of_view)
	{
		result->field_of_view_timer = DEFAULT_HORIZONTAL_FIELD_OF_VIEW_CHANGE_TIME;
		result->field_of_view_flags = FLAG(_observer_time_valid_bit);
		camera->last_field_of_view = result->field_of_view;
	}

	return;
}

/* ---------- private code */

static void first_person_camera_for_unit_and_vector(
	long unit_index,
	real_vector3d const *forward,
	struct observer_command *result)
{
	struct object_marker marker;
	real_matrix4x3 matrix;

	result->timer = 0.f;
	result->flags = 0;
	result->focus_offset = *global_zero_vector3d;
	result->focus_distance = 0.f;
	result->forward = *forward;
	result->field_of_view = DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW);
	observer_up_from_forward(&result->forward, &result->up);
	match_assert("c:\\halo\\SOURCE\\camera\\first_person_camera.c", 82, valid_real_vector3d_axes2(&result->forward, &result->up));

	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		unit_get_camera_position(unit_index, &result->focus_position);
		object_get_velocities(unit_index, &result->focus_velocity, NULL);

		if (unit->object.parent_object_index!=NONE)
		{
			struct object_datum *vehicle = (struct object_datum *)object_try_and_get_and_verify_type(unit->object.parent_object_index, _object_mask_vehicle);

			if (vehicle)
			{
				struct vehicle_definition *definition = vehicle_definition_get(vehicle->definition_index);
				struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

				if (TEST_FLAG(seat->flags, _unit_seat_slave_first_person_camera_bit))
				{
					if (object_get_marker_by_name(unit->object.parent_object_index, "primary trigger", &marker, 1))
					{
						result->focus_position = marker.matrix.position;
						result->forward = marker.matrix.forward;
						result->up = marker.matrix.up;
					}
				}
				else
				{
					matrix4x3_from_point_and_vectors(&matrix, &vehicle->object.position, &vehicle->object.forward, &vehicle->object.up);
					matrix4x3_inverse_transform_normal(&matrix, &result->forward, &result->forward);
					observer_up_from_forward(&result->forward, &result->up);
					matrix4x3_transform_normal(&matrix, &result->forward, &result->forward);
					matrix4x3_transform_normal(&matrix, &result->up, &result->up);
				}
			}
		}

		result->flags = FLAG(_observer_command_valid_bit);
	}

	match_assert_valid_observer_command("c:\\halo\\SOURCE\\camera\\first_person_camera.c", 133, result);

	return;
}
