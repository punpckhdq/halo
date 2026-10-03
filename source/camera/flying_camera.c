/*
FLYING_CAMERA.C
*/

/* ---------- headers */

#include "cseries.h"
#include "flying_camera.h"
#include "director.h"
#include "observer.h"
#include "objects.h"
#include "network_game_globals.h"
#include "players.h"
#include "collisions.h"
#include "rasterizer.h"

/* ---------- globals */

#define FLYING_CAMERA_MAXIMUM_PITCH 1.5676548f

/* ---------- public code */

void flying_camera_new(
	struct flying_camera *camera)
{
	camera->position.x = camera->position.y = 0.f;
	camera->orientation.yaw = 0.f;
	camera->orientation.pitch = 0.f;
	camera->roll = 0.f;
	camera->field_of_view = DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW);

	return;
}

void flying_camera_new_from_point_and_vector(
	struct flying_camera *camera,
	real_point3d const *focus,
	real_vector3d const *orientation)
{
	flying_camera_new(camera);
	camera->position = *focus;
	euler_angles2d_from_vector3d(&camera->orientation, orientation);

	return;
}

void flying_camera_update(
	struct flying_camera *camera,
	struct camera_control const *controls,
	struct observer_command *result)
{
	real_point3d new_position;

	match_assert("c:\\halo\\SOURCE\\camera\\flying_camera.c", 41, camera);
	match_assert("c:\\halo\\SOURCE\\camera\\flying_camera.c", 42, controls);
	match_assert("c:\\halo\\SOURCE\\camera\\flying_camera.c", 43, result);

	if (controls->active)
	{
		camera->orientation.yaw += controls->facing_delta.yaw;
		camera->orientation.pitch = PIN(camera->orientation.pitch + controls->facing_delta.pitch, -FLYING_CAMERA_MAXIMUM_PITCH, FLYING_CAMERA_MAXIMUM_PITCH);
		camera->roll += controls->facing_delta.roll;
	}

	if (rasterizer_debug_options.freeze_flying_camera > 0)
	{
		camera->orientation.yaw = 0.f;
		camera->orientation.pitch = 0.f;
		camera->roll = 0.f;
		rasterizer_debug_options.freeze_flying_camera--;
	}

	result->timer = 0.3f;
	vector3d_from_euler_angles2d(&result->forward, &camera->orientation);
	observer_up_from_forward(&result->forward, &result->up);
	rotate_vector_about_axis(&result->up, &result->forward, sine(camera->roll), cosine(camera->roll));

	if (controls->active)
	{
		real cosine_yaw = cosine(camera->orientation.yaw);
		real sine_yaw = sine(camera->orientation.yaw);
		real cosine_pitch = cosine(camera->orientation.pitch);
		real sine_pitch = sine(camera->orientation.pitch);
		real_vector3d displacement;

		displacement.i = cosine_yaw * controls->position_delta.i - sine_yaw * controls->position_delta.j;
		displacement.j = sine_yaw * controls->position_delta.i + cosine_yaw * controls->position_delta.j;
		displacement.k = controls->position_delta.k;
		new_position.x = camera->position.x + displacement.i;
		new_position.y = camera->position.y + displacement.j;
		new_position.z = camera->position.z + displacement.k;
		camera->position = new_position;
	}

	result->focus_position = camera->position;
	result->focus_offset = *global_zero_vector3d;
	result->focus_distance = 0.f;
	result->field_of_view = camera->field_of_view;
	result->flags = FLAG(_observer_command_valid_bit);

	match_assert_valid_observer_command("c:\\halo\\SOURCE\\camera\\flying_camera.c", 149, result);

	return;
}
