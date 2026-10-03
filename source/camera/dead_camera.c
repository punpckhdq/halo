/*
DEAD_CAMERA.C
*/

/* ---------- headers */

#include "cseries.h"
#include "dead_camera.h"
#include "director.h"
#include "observer.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"

/* ---------- prototypes */

static boolean player_has_allies(long player_index);
static long player_get_next_player_with_a_unit(long player_index, long old_player_index, boolean match_team);

/* ---------- globals */

static real const dead_timer = 3.f;
static real const multiplayer_switch_timer = 15.f;
static real const singleplayer_switch_timer = 3.f;

/* ---------- public code */

void dead_camera_new(
	struct dead_camera *camera,
	short local_player_index,
	long unit_index)
{
	struct observer_result const *observer = observer_get_camera(local_player_index);

	match_assert("c:\\halo\\SOURCE\\camera\\dead_camera.c", 22, sizeof(struct dead_camera)<=DIRECTOR_CAMERA_DATA_SIZE);
	match_assert("c:\\halo\\SOURCE\\camera\\dead_camera.c", 23, camera);

	camera->position = observer->position;
	camera->field_of_view = DEGREES_TO_RADIANS(DEFAULT_HORIZONTAL_FIELD_OF_VIEW);
	camera->distance = (real_local_random_range(2.f, 6.f));
	camera->orientation.yaw = (real_local_random_range(0.f, 2*_pi));
	camera->orientation.pitch = -(real_local_random_range(0.15f*_pi, DEGREES_TO_RADIANS(63.f)));
	camera->timer = dead_timer;
	camera->switch_timer = unit_index!=NONE ? REAL_MAX : (game_engine_running() ? multiplayer_switch_timer : singleplayer_switch_timer);
	camera->player_index = local_player_get_player_index(local_player_index);

	camera->unit_index = unit_index==NONE ? player_get(camera->player_index)->dead_unit_index : unit_index;

	camera->current_player_index = camera->player_index;

	return;
}

void dead_camera_update(
	struct dead_camera *camera,
	struct camera_control const *controls,
	struct observer_command *result)
{
	struct object_datum *object = camera->unit_index!=NONE ? object_try_and_get(camera->unit_index) : NULL;

	if (object)
	{
		result->focus_position = object->object.bounding_sphere_center;
	}
	else
	{
		result->focus_position = camera->position;
	}

	result->focus_distance = camera->distance;
	vector3d_from_euler_angles2d(&result->forward, &camera->orientation);
	observer_up_from_forward(&result->forward, &result->up);
	result->field_of_view = camera->field_of_view;
	result->focus_offset = *global_zero_vector3d;
	result->focus_velocity = *global_zero_vector3d;
	result->flags = FLAG(_observer_command_valid_bit);
	result->timer = MAX(0.f, camera->timer);
	result->position_timer = 0.f;
	result->position_flags = FLAG(_observer_time_valid_bit) | FLAG(_observer_time_force_bit);

	if (camera->timer==dead_timer)
	{
		result->focus_distance = 0.5f;
		result->distance_timer = 0.f;
		result->distance_flags = FLAG(_observer_time_valid_bit) | FLAG(_observer_time_force_bit);
	}

	camera->timer -= controls->seconds_elapsed;
	camera->switch_timer = MAX(0.f, camera->switch_timer - controls->seconds_elapsed);

	if (0.f==camera->switch_timer && !game_time_get_paused())
	{
		long unit_index;

		camera->current_player_index = player_get_next_player_with_a_unit(camera->player_index, camera->current_player_index, player_has_allies(camera->player_index));
		if (camera->current_player_index!=NONE)
		{
			unit_index = player_get(camera->current_player_index)->unit_index;
		}

		if (unit_index!=camera->unit_index && unit_index!=NONE)
		{
			camera->timer = dead_timer;
			camera->unit_index = unit_index;
		}

		camera->switch_timer = game_engine_running() ? multiplayer_switch_timer : singleplayer_switch_timer;
	}

	match_assert_valid_observer_command("c:\\halo\\SOURCE\\camera\\dead_camera.c", 158, result);

	return;
}

/* ---------- private code */

static boolean player_has_allies(
	long player_index)
{
	struct data_iterator player_iterator;
	struct player_datum *player = player_get(player_index);
	long team_index = player->team_index;
	boolean has_allies = FALSE;
	struct player_datum *other_player;

	data_iterator_new(&player_iterator, player_data);
	while ((other_player = (struct player_datum *)data_iterator_next(&player_iterator))!=NULL)
	{
		if (player_iterator.index!=player_index && other_player->team_index==team_index)
		{
			has_allies = TRUE;
			break;
		}
	}

	return has_allies;
}

static long player_get_next_player_with_a_unit(
	long player_index,
	long old_player_index,
	boolean match_team)
{
	struct data_iterator player_iterator;
	long team_index = match_team ? player_get(player_index)->team_index : NONE;
	long next_player_index = NONE;
	struct player_datum *player;

	data_iterator_new(&player_iterator, player_data);
	while ((player = (struct player_datum *)data_iterator_next(&player_iterator))!=NULL)
	{
		if (player_iterator.index!=player_index && player->unit_index!=NONE && (!match_team || player->team_index==team_index))
		{
			if (next_player_index==NONE)
			{
				next_player_index = player_iterator.index;
			}
			else if (DATUM_INDEX_TO_ABSOLUTE_INDEX(player_iterator.index)>DATUM_INDEX_TO_ABSOLUTE_INDEX(old_player_index))
			{
				next_player_index = player_iterator.index;
				break;
			}
		}
	}

	if (next_player_index==NONE)
	{
		next_player_index = old_player_index;
	}

	return next_player_index;
}
