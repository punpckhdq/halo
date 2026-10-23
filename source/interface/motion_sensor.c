/*
MOTION_SENSOR.C
*/

/* ---------- headers */

#include "cseries.h"
#include "motion_sensor.h"
#include "hud.h"
#include "interface_panels.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "bitmap_macros.h"
#include "game_state.h"
#include "vehicles.h"
#include "render_debug.h"
#include "director.h"
#include "texture_cache.h"
#include "game_engine.h"

/* ---------- constants */

enum
{
	MAX_SENSOR_HISTORY = 10,
	MAX_CUSTOM_BLIPS = 16,
	MAXIMUM_BLIPS_PER_SENSOR = 16, /* fake name */
	SENSOR_UPDATE_TICKS = 15,
	SENSOR_LATENCY_TICKS = 1
};

enum
{
	_blip_type_self = 0,
	_blip_type_friend,
	_blip_type_enemy,
	_blip_type_vehicle_friend,
	_blip_type_vehicle_enemy,
	_blip_type_custom,
	_blip_type_none,
	NUMBER_OF_BLIP_TYPES
};

enum
{
	_blip_size_medium = 0,	/* fake name */
	_blip_size_small,		/* fake name */
	_blip_size_large,		/* fake name */
	NUMBER_OF_BLIP_SIZES	/* fake name */
};

/* ---------- macros */

#define DROPSHIP_SEAT_LABEL "c_dropship"										/* fake name */
#define CUSTOM_BLIP_FLASH_ANGULAR_VELOCITY (3.141592f / TICKS_PER_SECOND)		/* fake name */

/* ---------- structures */

struct tiny_point2d
{
	char x;
	char y;
};

struct motion_sensor_blip
{
	struct tiny_point2d pt;
	char type;
	char size;
};

struct motion_sensor_datum
{
	struct motion_sensor_blip blips[MAXIMUM_BLIPS_PER_SENSOR];
	struct tiny_point2d custom_blips[MAX_CUSTOM_BLIPS];
	char custom_blip_goal_indices[MAX_CUSTOM_BLIPS];
	real_point2d reference_point;
	long num_blips_active;
	real yaw;
	byte custom_blips_active;
};

struct motion_sensor_player_datum
{
	struct motion_sensor_datum sensor_data[MAX_SENSOR_HISTORY];
	long current_unit_indices[MAXIMUM_BLIPS_PER_SENSOR];
};

struct motion_sensor_globals_definition
{
	struct motion_sensor_player_datum motion_sensors[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	long last_update_time;
	short sensor_active_index;
	boolean update;
};

/* ---------- prototypes */

static void tiny_point2d_set(struct tiny_point2d *tiny_position, real_point2d const *position);
static void tiny_point2d_get(struct tiny_point2d const *tiny_position, real_point2d *position);
static char blip_type_get(long object_index, long local_player_index);
static void motion_sensor_blip_set_type_and_size(struct motion_sensor_blip *blip, long unit_index, long local_player_index);
static real blip_size_get(char size_type);
static struct motion_sensor_player_datum *get_motion_sensor_data(short local_player_index);
static boolean should_track_object(long object_index);
static boolean should_draw_object(long object_index);
static void render_blip(struct motion_sensor_datum const *sensor_datum, char blip_type, char blip_size_type, real_point2d const *point, real relative_scale, real fade, real radius);
static void motion_sensor_update(void);
static void update_motion_sensor(short local_player_index);
static void render_motion_sensor(short local_player_index, boolean in_multiplayer, point2d const *reference);
static void blip_begin(short num_blips, real_point2d const *reference, boolean in_multiplayer, short local_player_index);

/* ---------- globals */

real_rgb_color const blip_colors[NUMBER_OF_BLIP_TYPES] =
{
	{ 1.f, .5f, 0.f },
	{ 1.f, 1.f, 0.f },
	{ 1.f, 0.f, 0.f },
	{ 1.f, 1.f, 0.f },
	{ 1.f, 0.f, 0.f },
	{ .5f, .5f, 1.f },
	{ 0.f, 0.f, 0.f }
};

real scale = 1.f;

long next_vertex_index = 0;

static struct motion_sensor_globals_definition *motion_sensor_globals = NULL;
static real sweep_theta = 0.f;

boolean debug_motion_sensor_draw_all_units = FALSE;

/* ---------- private code */

static void tiny_point2d_set(
	struct tiny_point2d *tiny_position,
	real_point2d const *position)
{
	match_assert("c:\\halo\\SOURCE\\interface\\motion_sensor.c", 105, fabs(position->x) < hud_globals->defaults.motion_sensor_range);
	match_assert("c:\\halo\\SOURCE\\interface\\motion_sensor.c", 106, fabs(position->y) < hud_globals->defaults.motion_sensor_range);

	tiny_position->x = (char)(position->x / hud_globals->defaults.motion_sensor_range * CHAR_MAX);
	tiny_position->y = (char)(position->y / hud_globals->defaults.motion_sensor_range * CHAR_MAX);

	return;
}

static void tiny_point2d_get(
	struct tiny_point2d const *tiny_position,
	real_point2d *position)
{
	position->x = tiny_position->x * hud_globals->defaults.motion_sensor_range * (1.f / CHAR_MAX);
	position->y = tiny_position->y * hud_globals->defaults.motion_sensor_range * (1.f / CHAR_MAX);

	return;
}

static char blip_type_get(
	long object_index,
	long local_player_index)
{
	struct unit_datum *unit;
	long team_index = player_get(local_player_get_player_index(local_player_index))->team_index;

	if (object_index == NONE)
	{
		return _blip_type_custom;
	}

	if ((player_index_from_unit_index(object_index) == NONE ? NONE : player_get(player_index_from_unit_index(object_index))->local_player_index) == local_player_index)
	{
		return _blip_type_self;
	}

	if (!unit_try_and_get(object_index))
	{
		return _blip_type_enemy;
	}

	unit = unit_get(object_index);

	if (vehicle_try_and_get(object_index))
	{
		struct vehicle_datum *vehicle = vehicle_get(object_index);

		if (vehicle->unit.gunner_object_index != NONE)
		{
			struct unit_datum *gunner = unit_get(vehicle->unit.gunner_object_index);

			if (game_team_is_enemy(gunner->object.owner_team_index, team_index))
			{
				return _blip_type_vehicle_enemy;
			}
			else
			{
				return _blip_type_vehicle_friend;
			}
		}
		else if (vehicle->unit.driver_object_index != NONE)
		{
			struct unit_datum *driver = unit_get(vehicle->unit.driver_object_index);

			if (game_team_is_enemy(driver->object.owner_team_index, team_index))
			{
				return _blip_type_vehicle_enemy;
			}
			else
			{
				return _blip_type_vehicle_friend;
			}
		}
		else
		{
			struct unit_definition *definition = unit_definition_get(vehicle->definition_index);

			if (definition->unit.seats.count > 1)
			{
				struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&definition->unit.seats, 0, struct unit_seat);

				if (!strncmp(seat->label, DROPSHIP_SEAT_LABEL, sizeof(DROPSHIP_SEAT_LABEL) - 1))
				{
					return _blip_type_vehicle_enemy;
				}
			}

			return _blip_type_vehicle_friend;
		}
	}
	else
	{
		long player_team_index = player_get(local_player_get_player_index(local_player_index))->team_index;

		if (game_team_is_enemy(unit->object.owner_team_index, player_team_index))
		{
			return _blip_type_enemy;
		}
		else
		{
			return _blip_type_friend;
		}
	}
}

static void motion_sensor_blip_set_type_and_size(
	struct motion_sensor_blip *blip,
	long unit_index,
	long local_player_index)
{
	char size_type;

	blip->type = blip_type_get(unit_index, local_player_index);

	if (unit_index != NONE && unit_try_and_get(unit_index))
	{
		short blip_size = unit_definition_get(unit_get(unit_index)->definition_index)->unit.blip_type;

		if (blip_size < 0 || blip_size >= NUMBER_OF_BLIP_SIZES)
		{
			blip_size = _blip_size_medium;
		}

		size_type = (char)blip_size;
	}
	else
	{
		size_type = _blip_size_medium;
	}

	blip->size = size_type;

	return;
}

static real blip_size_get(
	char size_type)
{
	static real sizes[NUMBER_OF_BLIP_SIZES] = { 0.f, -.75f, 1.f };

	return sizes[size_type];
}

static struct motion_sensor_player_datum *get_motion_sensor_data(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\motion_sensor.c", 287, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);

	return &motion_sensor_globals->motion_sensors[local_player_index];
}

/* ---------- public code */

void motion_sensor_initialize(
	void)
{
	motion_sensor_globals = (struct motion_sensor_globals_definition *)game_state_malloc("motion sensor (radar)", "sensor data", sizeof(*motion_sensor_globals));
	match_assert("c:\\halo\\SOURCE\\interface\\motion_sensor.c", 298, motion_sensor_globals);

	return;
}

void motion_sensor_dispose(
	void)
{
	return;
}

void motion_sensor_initialize_for_new_map(
	void)
{
	long local_player_index;

	memset(motion_sensor_globals, 0, sizeof(*motion_sensor_globals));

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		long sensor_index;

		for (sensor_index = 0; sensor_index < MAX_SENSOR_HISTORY; sensor_index++)
		{
			long blip_index;

			for (blip_index = 0; blip_index < MAXIMUM_BLIPS_PER_SENSOR; blip_index++)
			{
				motion_sensor_globals->motion_sensors[local_player_index].sensor_data[sensor_index].blips[blip_index].type = _blip_type_none;
			}
		}
	}

	return;
}

void motion_sensor_dispose_from_old_map(
	void)
{
	return;
}

/* ---------- private code */

static boolean should_track_object(
	long object_index)
{
	struct unit_datum *unit = unit_try_and_get(object_index);
	boolean track = FALSE;

	if (unit && !TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
	{
		track = TRUE;
	}

	return track;
}

static boolean should_draw_object(
	long object_index)
{
	boolean draw = FALSE;

	if (unit_try_and_get(object_index) && game_engine_draw_object_in_motion_sensor(object_index))
	{
		struct unit_datum *unit = unit_get(object_index);

		if (TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit) ||
			(unit->unit.grenade_throw_state != _unit_grenade_throw_idle && unit->unit.grenade_throw_state != _unit_grenade_throw_ending))
		{
			draw = TRUE;
		}
		else
		{
			real_vector3d velocity;
			boolean visible;
			boolean moving;

			object_get_velocities(object_index, &velocity, NULL);
			visible = game_engine_running() || !TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit);
			moving = magnitude_squared3d(&velocity) >= hud_globals->defaults.motion_sensor_velocity_sensitivity || debug_motion_sensor_draw_all_units;

			draw = visible && moving;
		}
	}

	return draw;
}

static void render_blip(
	struct motion_sensor_datum const *sensor_datum,
	char blip_type,
	char blip_size_type,
	real_point2d const *point,
	real relative_scale,
	real fade,
	real radius)
{
	real_vector2d relative_diff = *(real_vector2d const *)point;

	match_assert("c:\\halo\\SOURCE\\interface\\motion_sensor.c", 410, _blip_type_none != blip_type);
	rotate_vector2d(&relative_diff, sine(-sensor_datum->yaw), cosine(-sensor_datum->yaw), &relative_diff);

	if (magnitude_squared2d(&relative_diff) < hud_globals->defaults.motion_sensor_range * hud_globals->defaults.motion_sensor_range)
	{
		real distance;
		real normalized_distance;
		real scaled_distance;
		real_rgb_color const *color;
		real flash;
		real size;

		distance = magnitude2d(&relative_diff);

		if (distance < 1.f / 64.f)
		{
			distance = 1.f / 64.f;
		}

		scale_vector2d(&relative_diff, 1.f / distance, &relative_diff);
		normalized_distance = distance / hud_globals->defaults.motion_sensor_range;
		scaled_distance = (real)pow(normalized_distance, 0.7);
		scale_vector2d(&relative_diff, scaled_distance * hud_globals->defaults.motion_sensor_range, &relative_diff);
		scale_vector2d(&relative_diff, relative_scale, &relative_diff);
		color = &blip_colors[blip_type];
		flash = 1.f;
		size = blip_size_get(blip_size_type);

		if (blip_type == _blip_type_custom)
		{
			real angle = game_time_get() * CUSTOM_BLIP_FLASH_ANGULAR_VELOCITY;

			flash = (real)((sine(angle) + 1.0) * (1.0 / 3.0) + 1.0);
		}

		rasterizer_hud_motion_sensor_blip_draw((real_point2d *)&relative_diff, fade, flash * radius + size, color, blip_type == _blip_type_custom);
	}

	return;
}

/* ---------- public code */

void motion_sensor_draw_screen(
	short local_player_index,
	boolean in_multiplayer,
	point2d const *pt)
{
	match_assert("c:\\halo\\SOURCE\\interface\\motion_sensor.c", 476, pt);

	if (local_player_index != NONE)
	{
		update_motion_sensor(local_player_index);
		render_motion_sensor(local_player_index, in_multiplayer, pt);
	}

	return;
}

static real sweep_speed = 1.f;

void motion_sensor_tick(
	void)
{
	static real test = 1.1f;
	real t = (real)fmod(game_time_get() * SECONDS_PER_TICK, 2.1f);

	if (t < 2.0375f)
	{
		sweep_theta = 1.f / ((t + .0625f) * test);
	}
	else
	{
		sweep_theta = .4f;
	}

	motion_sensor_update();

	return;
}

/* ---------- private code */

static void motion_sensor_update(
	void)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	long game_time;
	boolean done;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	game_engine_running();
	game_time = game_time_get();
	done = FALSE;
	motion_sensor_globals->update = TRUE;
	motion_sensor_globals->sensor_active_index++;
	motion_sensor_globals->sensor_active_index %= MAX_SENSOR_HISTORY;
	motion_sensor_globals->last_update_time = game_time;

	if (game_time % SENSOR_UPDATE_TICKS && game_time)
	{
		short i;
		short sensor_active_index = motion_sensor_globals->sensor_active_index;
		short previous_index = (sensor_active_index + MAX_SENSOR_HISTORY - 1) % MAX_SENSOR_HISTORY;
		short player_count = local_player_count();
		short local_player_index = local_player_get_next(NONE);

		for (i = 0; i < player_count; i++)
		{
			memcpy(
				&motion_sensor_globals->motion_sensors[local_player_index].sensor_data[sensor_active_index],
				&motion_sensor_globals->motion_sensors[local_player_index].sensor_data[previous_index],
				sizeof(struct motion_sensor_datum));
			local_player_index = local_player_get_next(local_player_index);
		}
	}
	else
	{
		real_point3d positions[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
		short local_player_true_index[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
		struct object_iterator iter;
		short player_count = local_player_count();
		short num_recorded_blips[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS] = { 0 };
		short local_player_index = local_player_get_next(NONE);
		short i = 0;

		while (i < player_count)
		{
			short blip_index;
			struct motion_sensor_datum *sensor = &motion_sensor_globals->motion_sensors[local_player_index].sensor_data[motion_sensor_globals->sensor_active_index];
			long unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index;

			local_player_true_index[i] = local_player_index;

			if (unit_index != NONE)
			{
				unit_get_camera_position(unit_index, &positions[local_player_index]);
			}

			sensor->num_blips_active = 0;

			for (blip_index = 0; blip_index < (long)NUMBEROF(sensor->blips); blip_index++)
			{
				sensor->blips[blip_index].type = _blip_type_none;
			}

			i++;
			local_player_index = local_player_get_next(local_player_index);
		}

		object_iterator_new(&iter, _object_mask_unit, 1);

		while (object_iterator_next(&iter) && !done)
		{
			if (should_track_object(iter.index) && should_draw_object(iter.index))
			{
				real_point3d center;
				real radius;
				long full_sensor_count = 0;

				object_get_bounding_sphere(iter.index, &center, &radius);

				for (i = 0; i < player_count; i++)
				{
					short true_local_player_index = local_player_true_index[i];
					short *num_blips = &num_recorded_blips[true_local_player_index];
					short blip_index = *num_blips;

					if (blip_index < MAXIMUM_BLIPS_PER_SENSOR)
					{
						center.z = positions[true_local_player_index].z;

						if (game_engine_running() ||
							distance_squared3d(&positions[true_local_player_index], &center) <= hud_globals->defaults.motion_sensor_range * hud_globals->defaults.motion_sensor_range)
						{
							struct motion_sensor_player_datum *player_sensor = &motion_sensor_globals->motion_sensors[true_local_player_index];
							struct motion_sensor_datum *sensor = &player_sensor->sensor_data[motion_sensor_globals->sensor_active_index];

							motion_sensor_blip_set_type_and_size(&sensor->blips[blip_index], iter.index, true_local_player_index);
							player_sensor->current_unit_indices[blip_index] = iter.index;
							(*num_blips)++;
							sensor->num_blips_active++;
						}
					}
					else
					{
						full_sensor_count++;
					}
				}

				if (full_sensor_count == player_count)
				{
					done = TRUE;
				}
			}
		}
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\motion_sensor.c", 642);
	}

	return;
}

static void update_motion_sensor(
	short local_player_index)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	struct motion_sensor_player_datum *player_sensor;
	long game_time;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	player_sensor = get_motion_sensor_data(local_player_index);
	game_time = game_time_get();

	if (motion_sensor_globals->update)
	{
		short sensor_active_index = motion_sensor_globals->sensor_active_index;
		long unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index;
		struct motion_sensor_datum *sensor = &player_sensor->sensor_data[sensor_active_index];

		if (unit_index != NONE)
		{
			short blip_index;

			{
				real_point3d camera_position;

				unit_get_camera_position(unit_index, &camera_position);
				sensor->reference_point.x = camera_position.x;
				sensor->reference_point.y = camera_position.y;
			}

			for (blip_index = 0; blip_index < (long)NUMBEROF(player_sensor->current_unit_indices); blip_index++)
			{
				long object_index = player_sensor->current_unit_indices[blip_index];

				if (unit_try_and_get(object_index))
				{
					real_point3d center;
					real radius;
					real_point2d relative_position;
					real_point3d sensor_position;
					boolean draw = should_draw_object(object_index);

					object_get_bounding_sphere(object_index, &center, &radius);
					relative_position.x = center.x - sensor->reference_point.x;
					relative_position.y = center.y - sensor->reference_point.y;
					sensor_position.x = sensor->reference_point.x;
					sensor_position.y = sensor->reference_point.y;
					sensor_position.z = center.z;

					if (draw && distance_squared3d(&sensor_position, &center) <= hud_globals->defaults.motion_sensor_range * hud_globals->defaults.motion_sensor_range)
					{
						tiny_point2d_set(&sensor->blips[blip_index].pt, &relative_position);
					}
					else
					{
						sensor->blips[blip_index].type = _blip_type_none;
						player_sensor->current_unit_indices[blip_index] = NONE;
					}
				}
			}

			sensor->yaw = player_control_get_facing_angles(local_player_index)->yaw + _pi2;

			{
				real_point2d customs[MAX_CUSTOM_BLIPS];
				real_point3d reference_point;
				long custom_index;
				long custom_blips_skipped = 0;

				sensor->custom_blips_active = game_engine_player_get_custom_motion_sensor_positions(local_player_get_player_index(local_player_index), customs, sensor->custom_blip_goal_indices, MAX_CUSTOM_BLIPS);
				unit_get_camera_position(unit_index, &reference_point);

				for (custom_index = 0; custom_index < sensor->custom_blips_active; custom_index++)
				{
					real_point3d custom_position;

					custom_position.x = customs[custom_index].x;
					custom_position.y = customs[custom_index].y;
					custom_position.z = reference_point.z;
					customs[custom_index].x -= reference_point.x;
					customs[custom_index].y -= reference_point.y;

					if (distance_squared3d(&reference_point, &custom_position) <= hud_globals->defaults.motion_sensor_range * hud_globals->defaults.motion_sensor_range)
					{
						tiny_point2d_set(&sensor->custom_blips[custom_index - custom_blips_skipped], &customs[custom_index]);
					}
					else
					{
						custom_blips_skipped++;
					}
				}

				sensor->custom_blips_active -= custom_blips_skipped;
			}
		}
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\motion_sensor.c", 756);
	}

	return;
}

static void render_motion_sensor(
	short local_player_index,
	boolean in_multiplayer,
	point2d const *reference)
{
	long return_eip = get_return_eip();
	long stack_buffer[HUD_STACK_BUFFER_LONG_COUNT];
	short perspective;

	memset(stack_buffer, HUD_STACK_BUFFER_BYTE, sizeof(stack_buffer));

	perspective = director_get_perspective(local_player_index);

	if (perspective != _director_perspective_neutral && perspective != _director_perspective_scripted)
	{
		real_point2d corner;
		short history_index;
		struct motion_sensor_player_datum *player_sensor = get_motion_sensor_data(local_player_index);
		real relative_scale = hud_globals->defaults.motion_sensor_scale / hud_globals->defaults.motion_sensor_range;

		corner.x = reference->x;
		corner.y = reference->y;
		blip_begin(0, &corner, in_multiplayer, local_player_index);

		for (history_index = 0; history_index < MAX_SENSOR_HISTORY; history_index++)
		{
			short blip_index;
			short sensor_index = (motion_sensor_globals->sensor_active_index - history_index + MAX_SENSOR_HISTORY) % MAX_SENSOR_HISTORY;
			struct motion_sensor_datum *sensor = &player_sensor->sensor_data[sensor_index];
			real age = MAX_SENSOR_HISTORY - history_index;
			real radius = age * (1.f / MAX_SENSOR_HISTORY);
			real fade = radius * radius;

			radius = (real)(pow(1.f - radius, 3.5) * 7.0 + 1.0);

			for (blip_index = 0; blip_index < (long)NUMBEROF(sensor->blips); blip_index++)
			{
				struct motion_sensor_blip *blip = &sensor->blips[blip_index];

				if (blip->type != _blip_type_none)
				{
					real_point2d blip_pos;

					tiny_point2d_get(&blip->pt, &blip_pos);
					render_blip(sensor, blip->type, blip->size, &blip_pos, relative_scale, fade, radius);
				}
			}

			for (blip_index = 0; blip_index < sensor->custom_blips_active; blip_index++)
			{
				match_assert("c:\\halo\\SOURCE\\interface\\motion_sensor.c", 815, blip_index<MAX_CUSTOM_BLIPS);

				if (game_engine_get_goal_in_use(sensor->custom_blip_goal_indices[blip_index]))
				{
					real_point2d blip_pos;

					tiny_point2d_get(&sensor->custom_blips[blip_index], &blip_pos);
					render_blip(sensor, _blip_type_custom, _blip_size_medium, &blip_pos, relative_scale, fade, radius);
				}
			}
		}

		rasterizer_hud_motion_sensor_blip_end(&center_point, sweep_theta);
	}

	{
		short corrupt_index = check_stack_buffer(stack_buffer);

		match_hud_verify_stack("c:\\halo\\SOURCE\\interface\\motion_sensor.c", 830);
	}

	return;
}

static void blip_begin(
	short num_blips,
	real_point2d const *reference,
	boolean in_multiplayer,
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\interface\\motion_sensor.c", 841, reference);

	blip_player_index = local_player_index;
	scale = in_multiplayer ? .75f : 1.f;
	center_point = *reference;
	rasterizer_hud_motion_sensor_blip_begin();

	return;
}
