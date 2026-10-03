/*
CAMERA_SCRIPTING.C
*/

/* ---------- headers */

#include "cseries.h"
#include "camera_scripting.h"
#include "director.h"
#include "observer.h"
#include "dead_camera.h"
#include "first_person_camera.h"
#include "collision_bsp.h"
#include "render.h"
#include "units.h"
#include "console.h"
#include "player_effects.h"
#include "model_animation_definitions.h"
#include "scenario_definitions.h"
#include "scenario.h"

/* ---------- globals */

static struct
{
	boolean enabled;
	boolean first_update;
	short mode;
	short camera_point_index;
	real time_stop;
	real_point3d point;
	real_vector3d forward;
	real_vector3d up;
	real field_of_view;
	long relative_object_index;
	long animation_graph_index;
	short animation_index;
} camera_script_globals =
{
	FALSE,
	FALSE,
	NONE,
	NONE,
	0.f,
	{ 0.f, 0.f, 0.f },
	{ 0.f, 0.f, 1.f },
	{ 0.f, 1.f, 0.f },
	DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW),
	NONE,
	NONE,
	0,
};

/* ---------- public code */

void scripted_camera_enable(
	boolean enabled)
{
	camera_script_globals.enabled = enabled;
	camera_script_globals.first_update = TRUE;

	return;
}

void scripted_camera_set_animation(
	long animation_graph_index,
	char const *animation_name)
{
	if (animation_graph_index != NONE)
	{
		struct animation_graph const *animation_graph = animation_graph_definition_get(animation_graph_index);

		if (animation_graph->nodes.count == 1)
		{
			short animation_index;

			for (animation_index = 0; animation_index < animation_graph->animations.count; animation_index++)
			{
				struct animation const *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);

				if (_stricmp(animation_name, animation->name) == 0)
				{
					camera_script_globals.camera_point_index = NONE;
					camera_script_globals.relative_object_index = NONE;
					camera_script_globals.mode = _camera_script_mode_animation;
					camera_script_globals.first_update = TRUE;
					camera_script_globals.animation_index = animation_index;
					camera_script_globals.animation_graph_index = animation_graph_index;
					camera_script_globals.field_of_view = DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW);
					camera_script_globals.time_stop = (real)(animation->frame_count / TICKS_PER_SECOND);

					break;
				}
			}
		}
	}

	return;
}

void scripted_camera_set_first_person(
	long unit_index)
{
	if (unit_index != NONE)
	{
		camera_script_globals.mode = _camera_script_mode_first_person;
		camera_script_globals.first_update = TRUE;
		camera_script_globals.relative_object_index = unit_index;
	}
	else
	{
		error(_error_silent, "cannot set first person camera on a unit that doesn't exist.");
	}

	return;
}

void scripted_camera_set_dead(
	long unit_index)
{
	if (unit_index != NONE)
	{
		camera_script_globals.mode = _camera_script_mode_dead;
		camera_script_globals.first_update = TRUE;
		camera_script_globals.relative_object_index = unit_index;
	}
	else
	{
		error(_error_silent, "cannot set first person camera on a unit that doesn't exist.");
	}

	return;
}

boolean scripted_camera_object_is_first_person_camera(
	long object_index)
{
	return camera_script_globals.enabled &&
		camera_script_globals.mode == _camera_script_mode_first_person &&
		camera_script_globals.relative_object_index == object_index;
}

void scripted_camera_set(
	short camera_point_index,
	short tick_count,
	long relative_to_object_index)
{
	struct scenario_cutscene_camera_point const *camera_point = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->cutscene_camera_points, camera_point_index, struct scenario_cutscene_camera_point);
	long seconds = tick_count / TICKS_PER_SECOND;

	camera_script_globals.mode = _camera_script_mode_point;
	camera_script_globals.first_update = TRUE;
	camera_script_globals.camera_point_index = camera_point_index;
	camera_script_globals.point = camera_point->position;
	vectors3d_from_euler_angles3d(&camera_script_globals.forward, &camera_script_globals.up, &camera_point->orientation);
	camera_script_globals.field_of_view = camera_point->field_of_view != 0.f ? camera_point->field_of_view : DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW);
	camera_script_globals.time_stop = (real)seconds;
	camera_script_globals.relative_object_index = relative_to_object_index;

	director_update(0.f);
	observer_update(_real_epsilon);

	return;
}

void scripted_camera_set_absolute(
	short camera_point_index,
	short tick_count)
{
	scripted_camera_set(camera_point_index, tick_count, NONE);

	return;
}

void scripted_camera_set_camera_point_relative(
	real_point3d const *position,
	real_vector3d const *forward,
	real_vector3d const *up,
	real fov,
	short tick_count,
	long relative_to_object_index)
{
	camera_script_globals.mode = _camera_script_mode_point;
	camera_script_globals.camera_point_index = NONE;
	camera_script_globals.point = *position;
	camera_script_globals.forward = *forward;
	camera_script_globals.up = *up;
	camera_script_globals.field_of_view = fov != 0.f ? fov : DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW);
	camera_script_globals.time_stop = (real)(tick_count / TICKS_PER_SECOND);
	camera_script_globals.relative_object_index = relative_to_object_index;

	director_update(0.f);
	observer_update(_real_epsilon);

	return;
}

void scripted_camera_set_camera_point_absolute(
	real_point3d const *position,
	real_vector3d const *forward,
	real_vector3d const *up,
	real fov,
	short tick_count)
{
	scripted_camera_set_camera_point_relative(position, forward, up, fov, tick_count, NONE);

	return;
}

short scripted_camera_next_camera_point(void)
{
	return camera_script_globals.camera_point_index;
}

long scripted_camera_object_relative_to(void)
{
	return camera_script_globals.relative_object_index;
}

short scripted_camera_time(void)
{
	return (short)(camera_script_globals.time_stop * TICKS_PER_SECOND);
}

void scripted_camera_update(
	struct scripted_camera *camera,
	struct camera_control const *controls,
	struct observer_command *result)
{
	real_point3d reference_point = *global_origin3d;
	boolean valid = TRUE;
	real speed = game_time_get_speed();

	result->flags = 0;
	SET_FLAG(result->flags, _observer_command_force_time_bit, TRUE);
	SET_FLAG(result->flags, _observer_command_freeze_camera_bit, game_time_get_paused());

	switch (camera_script_globals.mode)
	{
		case _camera_script_mode_point:
		{
			if (camera_script_globals.relative_object_index != NONE)
			{
				struct object_datum const *object = object_try_and_get(camera_script_globals.relative_object_index);

				if (object)
				{
					reference_point = object->object.bounding_sphere_center;
				}
				else
				{
					valid = FALSE;
				}
			}

			if (valid)
			{
				result->timer = speed != 0.f ? camera_script_globals.time_stop / speed : 0.f;
				result->field_of_view = camera_script_globals.field_of_view;
				result->forward = camera_script_globals.forward;
				result->up = camera_script_globals.up;

				if (camera_script_globals.relative_object_index != NONE)
				{
					real angle = arctangent(result->forward.j, result->forward.i);
					real distance = dot_product3d((real_vector3d const *)&camera_script_globals.point, &result->forward);
					real_vector3d world_offset;

					distance = MIN(distance, 0.f);

					result->focus_distance = -distance;
					result->focus_position = reference_point;
					world_offset.i = camera_script_globals.point.x - distance * result->forward.i;
					world_offset.j = camera_script_globals.point.y - distance * result->forward.j;
					world_offset.k = camera_script_globals.point.z - distance * result->forward.k;

					result->focus_offset.i = world_offset.i * cosine(angle) + world_offset.j * sine(angle);
					result->focus_offset.j = world_offset.i * sine(angle) - world_offset.j * cosine(angle);
					result->focus_offset.k = world_offset.k;

					result->position_timer = 0.f;
					result->position_flags = FLAG(_observer_time_valid_bit);
				}
				else
				{
					result->focus_position = camera_script_globals.point;
				}

				SET_FLAG(result->flags, _observer_command_valid_bit, TRUE);
			}

			break;
		}

		case _camera_script_mode_animation:
		{
			struct animation_graph const *animation_graph = animation_graph_definition_get(camera_script_globals.animation_graph_index);
			struct animation const *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, camera_script_globals.animation_index, struct animation);
			short frame_index = (short)(animation->frame_count - camera_script_globals.time_stop * TICKS_PER_SECOND);
			real_matrix4x3 frame_matrix;

			frame_index = PIN(frame_index, 0, animation->frame_count - 1);
			animation_get_root_matrix(NULL, animation, frame_index, &frame_matrix);

			result->field_of_view = DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW);
			result->forward = frame_matrix.forward;
			result->up = frame_matrix.up;
			result->focus_position = frame_matrix.position;
			result->focus_distance = 0.f;
			result->timer = 0.f;
			SET_FLAG(result->flags, _observer_command_valid_bit, TRUE);

			break;
		}

		case _camera_script_mode_first_person:
		{
			if (unit_try_and_get(camera_script_globals.relative_object_index))
			{
				first_person_camera_fake(camera_script_globals.relative_object_index, result);
			}

			break;
		}

		case _camera_script_mode_dead:
		{
			if (unit_try_and_get(camera_script_globals.relative_object_index))
			{
				if (camera_script_globals.first_update)
				{
					dead_camera_new((struct dead_camera *)camera, controls->local_player_index, camera_script_globals.relative_object_index);
				}

				dead_camera_update((struct dead_camera *)camera, controls, result);
			}

			break;
		}
	}

	camera_script_globals.time_stop = MAX(0.f, camera_script_globals.time_stop - speed * controls->seconds_elapsed);
	camera_script_globals.first_update = FALSE;

	match_assert_valid_observer_command("c:\\halo\\SOURCE\\camera\\camera_scripting.c", 370, result);

	return;
}
