/*
EDITOR_FLYING_CAMERA.C
*/

/* ---------- headers */

#include "cseries.h"
#include "editor_flying_camera.h"
#include "director.h"
#include "observer.h"
#include "camera_scripting.h"
#include "flying_camera.h"
#include "orbiting_camera.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "console.h"
#include "collisions.h"
#include "terminal.h"
#include "scenario.h"
#include "scenario_definitions.h"
#include "objects.h"

/* ---------- prototypes */

static void translate_orbiting_to_flying(
	struct flying_camera *camera);
static void translate_flying_to_orbiting(
	struct flying_camera *camera);
static void editor_camera_flying_update(
	struct flying_camera *camera,
	struct camera_control const *controls,
	struct observer_command *result);
static void editor_camera_orbiting_update(
	struct flying_camera *camera,
	struct camera_control const *controls,
	struct observer_command *result);

/* ---------- globals */

static real speed = 1.f;
static long unit_focus = NONE;
struct render_globals *editor_custom_render = &render;
static void (*update_funcs[NUMBER_OF_EDITOR_CAMERA_MODES])(struct flying_camera *, struct camera_control const *, struct observer_command *) =
{
	editor_camera_flying_update,
	editor_camera_orbiting_update,
};
static void (*translate_funcs[NUMBER_OF_EDITOR_CAMERA_MODES][NUMBER_OF_CAMERA_TRANSLATIONS])(struct flying_camera *) =
{
	{ NULL, NULL },
	{ translate_flying_to_orbiting, translate_orbiting_to_flying },
};

static boolean is_scripted = FALSE;
static boolean use_roll = FALSE;
static boolean initialized = FALSE;
static struct editor_camera_focus_definition editor_camera_focus = {0};
static struct flying_camera *editor_camera = NULL;
static boolean reset_all = FALSE;
static real_vector3d unit_offset = {0};
static short camera_mode = _editor_camera_flying;
static short local_player_index = 0;
static boolean last_scripted = FALSE;
static struct persisted_camera_data persisted_cameras[NUMBER_OF_EDITOR_CAMERA_MODES] = {0};

/* ---------- public code */

void editor_camera_new(
	struct flying_camera *camera,
	short local_player_index)
{
	real_vector3d forward;

	if (!initialized)
	{
		if (global_scenario_get()->players.count && global_scenario_get()->players.address)
		{
			struct scenario_player *player = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->players, 0, struct scenario_player);

			editor_camera_focus.position = player->position;
			editor_camera_focus.angles.yaw = player->facing;
		}
		else
		{
			memset(&editor_camera_focus, 0, sizeof(editor_camera_focus));
		}
	}

	initialized = TRUE;
	vector3d_from_euler_angles2d(&forward, &editor_camera_focus.angles);
	flying_camera_new_from_point_and_vector(camera, &editor_camera_focus.position, &forward);
	if (local_player_index == 0)
	{
		editor_camera = camera;
	}

	if (camera_mode != _editor_camera_flying)
	{
		translate_funcs[camera_mode][_translate_to](camera);
	}

	return;
}

void editor_camera_get_focus(
	real_point3d *position,
	real_euler_angles2d *angles)
{
	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 120, position);
	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 121, angles);

	*position = editor_camera_focus.position;
	*angles = editor_camera_focus.angles;

	return;
}

void editor_camera_set_focus(
	real_point3d const *position,
	real_euler_angles2d const *angles)
{
	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 129, position);
	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 130, angles);

	editor_camera_focus.position = *position;
	editor_camera_focus.angles = *angles;

	return;
}

void editor_camera_set_position(
	real_point3d const *point,
	real_euler_angles2d const *angles)
{
	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 148, point);
	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 149, angles);

	if (!editor_camera)
	{
		editor_camera_set_focus(point, angles);
		initialized = TRUE;
	}
	else
	{
		editor_camera->position = *point;
		editor_camera->orientation = *angles;
	}

	return;
}

void editor_camera_bump_speed(
	void)
{
	static unsigned long index = 0;
	static long const multiple[5] = { 1, 5, 20, 40, 60 };

	index = (index + 1) % NUMBEROF(multiple);
	speed = (real)multiple[index];
	terminal_printf(global_real_argb_white, "speed is now x%f", speed);

	return;
}

long editor_camera_get_speed(
	void)
{
	return (long)speed;
}

boolean editor_camera_use_roll(
	boolean new_use_roll)
{
	boolean old_use_roll = use_roll;

	use_roll = new_use_roll;
	if (!use_roll && editor_camera)
	{
		editor_camera->roll = 0.f;
	}

	return old_use_roll;
}

long editor_camera_get_unit_focus(
	void)
{
	return unit_focus;
}

void editor_camera_set_mode(
	short mode)
{
	static char const *mode_str[NUMBER_OF_EDITOR_CAMERA_MODES] =
	{
		"flying camera",
		"orbiting camera",
	};

	if (editor_camera && camera_mode != mode)
	{
		if (camera_mode != _editor_camera_flying)
		{
			match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 302, translate_funcs[camera_mode][_translate_from]);
			translate_funcs[camera_mode][_translate_from](editor_camera);
		}

		if (mode != _editor_camera_flying)
		{
			match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 308, translate_funcs[mode][_translate_to]);
			translate_funcs[mode][_translate_to](editor_camera);
		}
	}

	camera_mode = mode;
	console_printf(FALSE, mode_str[mode]);

	return;
}

short editor_camera_get_mode(
	void)
{
	return camera_mode;
}

boolean editor_camera_get_scripted(
	void)
{
	return is_scripted;
}

static real const orbiting_camera_field_of_view = DEGREES_TO_RADIANS(70.f);
static real const orbiting_camera_minimum_distance = 1.f;
static real const orbiting_camera_latency = 0.5f;
static real const orbiting_camera_z_offset = 0.52f;

static void translate_orbiting_to_flying(
	struct flying_camera *camera)
{
	static real const default_distance = 1.f;

	persisted_cameras[_editor_camera_flying].camera_data = *camera;
	if (persisted_cameras[_editor_camera_orbiting].saved)
	{
		*camera = persisted_cameras[_editor_camera_orbiting].camera_data;
	}
	else
	{
		camera->position.x = 0.f;
		camera->position.y = default_distance;
		camera->position.z = 0.f;
		euler_angles2d_from_vector3d(&camera->orientation, &editor_custom_render->camera.forward);
	}

	return;
}

real editor_camera_get_field_of_view(
	void)
{
	static real const fov[NUMBER_OF_EDITOR_CAMERA_MODES] =
	{
		DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW),
		DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW),
	};

	return fov[camera_mode];
}

void editor_camera_move_to_point(
	real_point3d const *point)
{
	real_vector3d forward;

	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 139, point);

	vector3d_from_euler_angles2d(&forward, &editor_camera->orientation);
	point_from_line3d(point, &forward, -2.5f, &editor_camera->position);

	return;
}

void editor_camera_set_position_and_roll(
	real_point3d const *point,
	real_euler_angles3d const *angles)
{
	real_matrix4x3 matrix;
	real_vector3d forward;
	real_vector3d left;
	real_vector3d up;
	real_vector3d diff;
	real_euler_angles2d hack_angles;

	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 169, point);
	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 170, angles);

	if (!editor_camera)
	{
		editor_camera_set_focus(point, (real_euler_angles2d const *)angles);
		initialized = TRUE;
	}
	else
	{
		editor_camera->position = *point;
		matrix4x3_rotation_from_angles(&matrix, angles->yaw, angles->pitch, angles->roll);
		euler_angles2d_from_vector3d(&editor_camera->orientation, &matrix.forward);
		vector3d_from_euler_angles2d(&forward, &editor_camera->orientation);
		hack_angles = editor_camera->orientation;
		hack_angles.pitch += _half_pi;
		vector3d_from_euler_angles2d(&up, &hack_angles);
		normalize3d(&forward);
		normalize3d(&up);
		normalize3d(cross_product3d(&up, &forward, &left));
		normalize3d(cross_product3d(&up, &matrix.up, &diff));
		editor_camera->roll = angle_between_vectors3d(&up, &matrix.up) * dot_product3d(&forward, &diff);
		if (unit_focus != NONE)
		{
			unit_offset = *(real_vector3d const *)point;
		}
	}

	reset_all = TRUE;

	return;
}

void editor_camera_set_unit_focus(
	long unit_index)
{
	struct flying_camera *camera = editor_camera;

	unit_focus = unit_index;
	if (camera)
	{
		if (unit_index != NONE)
		{
			struct object_datum *object = object_get_and_verify_type(unit_index, NONE);

			vector_from_points3d(&object->object.bounding_sphere_center, &camera->position, &unit_offset);
		}
		else
		{
			unit_offset = *global_zero_vector3d;
		}
	}

	return;
}

void editor_camera_update(
	struct flying_camera *camera,
	struct camera_control const *controls,
	struct observer_command *result)
{
	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 340, update_funcs[camera_mode]);

	if (is_scripted && !controls->active)
	{
		scripted_camera_update(NULL, controls, result);
	}
	else
	{
		if (is_scripted)
		{
			editor_camera->position = editor_custom_render->camera.position;
			euler_angles2d_from_vector3d(&editor_camera->orientation, &editor_custom_render->camera.forward);
			editor_camera_set_unit_focus(unit_focus);
			if (camera_mode != _editor_camera_flying)
			{
				match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 356, translate_funcs[camera_mode][_translate_from]);
				translate_funcs[camera_mode][_translate_to](editor_camera);
			}
		}

		update_funcs[camera_mode](camera, controls, result);
		if (is_scripted)
		{
			result->timer = 0.f;
			result->flags |= FLAG(_observer_command_valid_bit) | FLAG(_observer_command_force_time_bit);
		}
	}

	return;
}

void editor_camera_set_scripted(
	boolean scripted)
{
	static char const *str[2] =
	{
		"exiting",
		"entering",
	};

	if (scripted)
	{
		real_euler_angles2d angles;

		if (camera_mode != _editor_camera_flying)
		{
			match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 387, translate_funcs[camera_mode][_translate_from]);
			translate_funcs[camera_mode][_translate_from](editor_camera);
		}

		euler_angles2d_from_vector3d(&angles, &editor_custom_render->camera.forward);
		editor_camera_set_position(&editor_custom_render->camera.position, &angles);
		if (unit_focus != NONE)
		{
			scripted_camera_set_camera_point_relative((real_point3d const *)&unit_offset, &editor_custom_render->camera.forward, &editor_custom_render->camera.up, DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW), 0, unit_focus);
		}
		else
		{
			scripted_camera_set_camera_point_relative(&editor_custom_render->camera.position, &editor_custom_render->camera.forward, &editor_custom_render->camera.up, DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW), 0, NONE);
		}
	}
	else
	{
		editor_camera->position = editor_custom_render->camera.position;
		euler_angles2d_from_vector3d(&editor_camera->orientation, &editor_custom_render->camera.forward);
		editor_camera_set_unit_focus(unit_focus);
		if (camera_mode != _editor_camera_flying)
		{
			match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 414, translate_funcs[camera_mode][_translate_from]);
			translate_funcs[camera_mode][_translate_to](editor_camera);
		}
	}

	last_scripted = is_scripted;
	is_scripted = scripted;
	console_printf(FALSE, "%s scripted camera mode", str[scripted]);

	return;
}

static void translate_flying_to_orbiting(
	struct flying_camera *camera)
{
	persisted_cameras[_editor_camera_orbiting].camera_data = *camera;
	persisted_cameras[_editor_camera_orbiting].saved = TRUE;
	camera->position = editor_custom_render->camera.position;
	euler_angles2d_from_vector3d(&camera->orientation, &editor_custom_render->camera.forward);
	editor_camera_set_unit_focus(unit_focus);

	return;
}

static void editor_camera_flying_update(
	struct flying_camera *camera,
	struct camera_control const *controls,
	struct observer_command *result)
{
	real_vector3d right;
	real_vector3d displacement;
	real_point3d new_position;

	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 448, camera);
	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 449, controls);
	match_assert("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 450, result);

	if (controls->active)
	{
		camera->orientation.yaw += controls->facing_delta.yaw;
		camera->orientation.pitch = PIN(camera->orientation.pitch + controls->facing_delta.pitch, -1.5676548f, 1.5676548f);
		if (use_roll)
		{
			camera->roll += controls->facing_delta.roll;
		}
		else
		{
			camera->roll = 0.f;
		}
	}

	result->timer = 0.3f;
	vector3d_from_euler_angles2d(&result->forward, &camera->orientation);
	right.i = result->forward.j;
	right.j = -result->forward.i;
	right.k = 0.f;
	if (normalize3d(&right) == 0.f)
	{
		right.i = 1.f;
		right.k = 0.f;
		right.j = 0.f;
	}
	cross_product3d(&right, &result->forward, &result->up);
	rotate_vector_about_axis(&result->up, &result->forward, (real)sin(camera->roll), (real)cos(camera->roll));

	{
		real cosine = (real)cos(camera->orientation.yaw);
		real sine = (real)sin(camera->orientation.yaw);

		displacement.i = cosine*controls->position_delta.i - sine*controls->position_delta.j;
		displacement.j = sine*controls->position_delta.i + cosine*controls->position_delta.j;
		displacement.k = controls->position_delta.k;
		scale_vector3d(&displacement, speed, &displacement);
	}

	if (unit_focus != NONE && object_try_and_get_and_verify_type(unit_focus, NONE))
	{
		struct object_datum *object;

		add_vectors3d(&unit_offset, &displacement, &unit_offset);
		object = object_get_and_verify_type(unit_focus, NONE);
		point_from_line3d(&object->object.bounding_sphere_center, &unit_offset, 1.f, &new_position);
	}
	else
	{
		new_position.x = displacement.i + camera->position.x;
		new_position.y = displacement.j + camera->position.y;
		new_position.z = displacement.k + camera->position.z;
	}

	camera->position = new_position;
	result->focus_position = new_position;
	result->focus_offset = *global_zero_vector3d;
	result->focus_distance = 0.f;
	result->field_of_view = DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW);
	result->flags = FLAG(_observer_command_valid_bit);
	match_assert_valid_observer_command("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 518, result);

	return;
}

static void editor_camera_orbiting_update(
	struct flying_camera *camera,
	struct camera_control const *controls,
	struct observer_command *result)
{
	struct unit_camera_info camera_info;

	player_control_get_unit_camera_info(controls->local_player_index, &camera_info);
	result->focus_position = camera_info.unit_origin;
	if (controls->active)
	{
		camera->orientation.yaw += controls->facing_delta.yaw;
		camera->orientation.pitch = PIN(camera->orientation.pitch + controls->facing_delta.pitch, DEGREES_TO_RADIANS(-72.f), DEGREES_TO_RADIANS(72.f));
		director_inhibit_input(controls->local_player_index);
	}

	camera->position.y = MAX(camera->position.y - controls->wheel_delta * (1.f/3.f), 0.6f);
	if (camera_info.unit_index != NONE)
	{
		vector3d_from_euler_angles2d(&result->forward, &camera->orientation);
		observer_up_from_forward(&result->forward, &result->up);
		object_get_velocities(camera_info.unit_index, &result->focus_velocity, NULL);
		result->flags = FLAG(_observer_command_valid_bit);
	}

	result->focus_offset = *global_zero_vector3d;
	result->focus_distance = camera->position.y;
	result->field_of_view = orbiting_camera_field_of_view;
	result->timer = orbiting_camera_latency;
	match_assert_valid_observer_command("c:\\halo\\SOURCE\\camera\\editor_flying_camera.c", 571, result);

	return;
}
