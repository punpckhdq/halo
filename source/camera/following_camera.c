/*
FOLLOWING_CAMERA.C
*/

/* ---------- headers */

#include "cseries.h"
#include "following_camera.h"
#include "director.h"
#include "observer.h"
#include "camera_track_definitions.h"
#include "network_game_globals.h"
#include "players.h"
#include "console.h"
#include "vehicles.h"
#include "units.h"
#include "unit_definitions.h"
#include "vehicle_definitions.h"
#include "game_globals.h"
#include "scenario.h"

/* ---------- prototypes */

static struct unit_camera *unit_camera_get(long unit_index);
static void camera_track_splut(struct unit_camera const *camera, real pitch, real_vector3d *offset);

/* ---------- globals */

/* This is from DEFAULT_HORIZONTAL_FIELD_OF_VIEW zoomed 2.5x per level in degrees and then rounded */
real following_camera_zoom_levels[4] =
{
	31.29f,
	12.78f,
	5.13f,
	2.05f,
};

/* ---------- public code */

void following_camera_new(
	struct following_camera *camera)
{
	match_assert("c:\\halo\\SOURCE\\camera\\following_camera.c", 19, camera);

	camera->initialized = FALSE;
	camera->confined = FALSE;
	camera->crouched = FALSE;
	camera->zoomed = FALSE;
	camera->zoom_level = 0;
	camera->unit_index = NONE;
	camera->seat_index = NONE;
	camera->facing_offset.pitch = 0.f;
	camera->facing_offset.yaw = 0.f;
	camera->distance_scale = 1.f;

	return;
}

void following_camera_deterministic(
	long unit_index,
	real_point3d *position,
	real_vector3d *forward)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_camera *camera = unit_camera_get(unit_index);
	real_vector3d offset;
	real_vector2d direction;

	unit_get_camera_position(unit_index, position);
	*forward = unit->unit.aiming_vector;
	camera_track_splut(camera, arcsine(forward->k), &offset);
	direction.i = forward->i;
	direction.j = forward->j;
	normalize2d(&direction);
	position->x += offset.i * direction.i + offset.j * direction.j;
	position->y += offset.i * direction.j - offset.j * direction.i;
	position->z += offset.k;

	return;
}

void following_camera_update(
	struct following_camera *camera,
	struct camera_control const *controls,
	struct observer_command *result)
{
	struct unit_camera_info camera_info;
	real_euler_angles2d facing;
	real_vector3d track_offset;

	match_assert("c:\\halo\\SOURCE\\camera\\following_camera.c", 138, camera);
	match_assert("c:\\halo\\SOURCE\\camera\\following_camera.c", 139, result);

	player_control_get_unit_camera_info(controls->local_player_index, &camera_info);

	result->focus_position = camera_info.unit_origin;
	result->timer = 0.f;
	result->flags = 0;
	result->field_of_view = DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW);

	if (camera->initialized)
	{
		if (camera_info.unit_index != camera->unit_index || camera_info.seat_index != camera->seat_index)
		{
			result->timer = MAX(1.f, result->timer);
		}
	}

	camera->seat_index = camera_info.seat_index;
	camera->unit_index = camera_info.unit_index;

	if (camera_info.unit_camera)
	{
		struct unit_datum *unit = unit_get(camera_info.unit_index);
		boolean crouched = TEST_FLAG(unit->unit.control_flags, _unit_control_crouch_modifier_bit) || TEST_FLAG(unit->unit.control_flags, _unit_control_jump_bit);

		if (crouched != camera->crouched)
		{
			result->focus_offset_flags = TRUE;
			result->focus_offset_timer = MAX(0.5f, result->focus_offset_timer);
			camera->crouched = crouched;
		}

		if (controls->active)
		{
			camera->facing_offset.yaw += controls->facing_delta.yaw;
			camera->facing_offset.pitch += controls->facing_delta.pitch;
			result->orientation_flags = TRUE;
			result->orientation_timer = MAX(0.4f, result->orientation_timer);
		}
		else if (camera->facing_offset.yaw != 0.f || camera->facing_offset.pitch != 0.f)
		{
			camera->facing_offset.pitch = 0.f;
			camera->facing_offset.yaw = 0.f;
		}

		facing = *player_control_get_facing_angles(controls->local_player_index);
		facing.yaw += camera->facing_offset.yaw;
		facing.pitch = PIN(facing.pitch + camera->facing_offset.pitch, -_half_pi, _half_pi);
		vector3d_from_euler_angles2d(&result->forward, &facing);
		match_assert("c:\\halo\\SOURCE\\camera\\following_camera.c", 212, magnitude3d(&result->forward) > 0.9999f && magnitude3d(&result->forward) < 1.0001f);

		camera_track_splut(camera_info.unit_camera, facing.pitch, &track_offset);
		result->focus_distance = magnitude3d(&track_offset);
		result->focus_offset.i = (cosine(facing.pitch) * result->focus_distance + track_offset.i) * camera->distance_scale;
		result->focus_offset.j = -(track_offset.j * camera->distance_scale);
		result->focus_offset.k = (sine(facing.pitch) * result->focus_distance + track_offset.k) * camera->distance_scale;
		result->focus_distance = MAX(((result->focus_distance - 0.6f) * camera->distance_scale) + 0.6f, 0.6f);
		object_get_velocities(camera_info.unit_index, &result->focus_velocity, NULL);
		SET_FLAG(result->flags, _observer_command_valid_bit, TRUE);
	}

	observer_up_from_forward(&result->forward, &result->up);

	match_assert_valid_observer_command("c:\\halo\\SOURCE\\camera\\following_camera.c", 238, result);

	camera->initialized = TRUE;

	return;
}

/* ---------- private code */

static struct unit_camera *unit_camera_get(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_camera *camera = NULL;

	if (unit->object.parent_object_index != NONE)
	{
		struct object_datum *parent = object_try_and_get_and_verify_type(unit->object.parent_object_index, _object_mask_vehicle);

		if (parent)
		{
			struct vehicle_definition *vehicle_definition = vehicle_definition_get(parent->definition_index);
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&vehicle_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

			if (TEST_FLAG(seat->flags, _unit_seat_is_driver_bit) || TEST_FLAG(seat->flags, _unit_seat_is_invisible_bit) || TEST_FLAG(seat->flags, _unit_seat_has_third_person_camera_bit))
			{
				camera = &seat->camera;
			}
		}
	}

	if (!camera)
	{
		camera = &unit_definition_get(unit->definition_index)->unit.camera;
	}

	return camera;
}

static void camera_track_splut(
	struct unit_camera const *camera,
	real pitch,
	real_vector3d *offset)
{
	struct unit_camera_track const *unit_camera_track = camera->unit_camera_tracks.count ? TAG_BLOCK_GET_ELEMENT(&camera->unit_camera_tracks, MIN(_unit_camera_track_loose, camera->unit_camera_tracks.count - 1), struct unit_camera_track) : NULL;
	long camera_track_index = unit_camera_track && unit_camera_track->track.index != NONE ? unit_camera_track->track.index : TAG_BLOCK_GET_ELEMENT(&scenario_get_game_globals()->camera, 0, struct game_globals_camera)->default_unit_camera_track.index;
	struct camera_track_definition const *camera_track = camera_track_definition_get(camera_track_index);
	real t = (pitch + _half_pi) / _pi;
	short start_index = (short)(t * (camera_track->control_points.count - 1));
	short index = start_index;
	real h = 1.f / (camera_track->control_points.count - 1);

	match_assert("c:\\halo\\SOURCE\\camera\\following_camera.c", 86, camera_track->control_points.count >= 4);

	while (index > 0 && (index + 4 > camera_track->control_points.count || index > start_index - 1))
	{
		index--;
	}

	uniform_cubic_spline_vector3d(
		offset,
		&TAG_BLOCK_GET_ELEMENT(&camera_track->control_points, index, struct camera_track_control_point)->position,
		&TAG_BLOCK_GET_ELEMENT(&camera_track->control_points, index + 1, struct camera_track_control_point)->position,
		&TAG_BLOCK_GET_ELEMENT(&camera_track->control_points, index + 2, struct camera_track_control_point)->position,
		&TAG_BLOCK_GET_ELEMENT(&camera_track->control_points, index + 3, struct camera_track_control_point)->position,
		index * h,
		h,
		t);

	return;
}
