/*
PLAYER_EFFECTS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "player_effects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "console.h"
#include "game_state.h"
#include "network_messages.h"
#include "sound_manager.h"
#include "director.h"
#include "game_sound.h"
#include "observer.h"
#include "network_server_message_handler.h"
#include "network_server_manager.h"
#include "damage_effect_definitions.h"
#include "player_rumble.h"
#include "damage.h"

/* ---------- constants */

enum
{
	_screen_flash_just_started_bit = 0,
	_camera_impulse_just_started_bit,
	_camera_shake_just_started_bit
};

enum
{
	_scripted_effect_running_bit = 0,
	_scripted_effect_ending_bit,
	NUMBER_OF_SCRIPTED_EFFECT_FLAGS
};

/* ---------- macros */

/* ---------- structures */

struct continuous_player_effect_datum
{
	real vibrate_frequencies[NUMBER_OF_vibrate_FREQUENCIES];
	real translational_shake;
	real rotational_shake;
};

struct player_effect_datum
{
	real_vector3d direction;
	real_vector3d jitter;
	struct screen_flash_definition screen_flash;
	struct camera_impulse_definition camera_impulse;
	struct camera_shake_definition camera_shake;
	struct continuous_player_effect_datum continuous_effect;
	short continuous_effect_timer;
	short screen_flash_time_left;
	short camera_impulse_time_left;
	short camera_shake_time_left;
	byte damage_indicator_ticks[NUMBER_OF_DAMAGE_INDICATOR_DIRECTIONS];
	byte flags;
};

struct screen_fade_definition
{
	real_rgb_color color;
	long start_time;
	short ticks;
	boolean fading_out;
};

struct scripted_player_effect_definition
{
	real_point3d max_translation;
	real_euler_angles3d max_rotation;
	real max_intensity;
	short timer;
	short total_time;
};

struct player_effect_globals_definition
{
	struct player_effect_datum local_player_effect_data[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	struct screen_fade_definition screen_fade;
	struct scripted_player_effect_definition scripted_effect;
	unsigned long global_flags;
	long reference_time;
};

/* ---------- prototypes */

struct player_effect_datum *player_effect_get(short local_player_index);
static void get_shake_matrix(real translation, real rotation, real_matrix4x3 *matrix);
static real effect_scale_factor(real zero_scale_factor, real scale);
static void player_effect_update_screen_flash(short local_player_index, struct player_effect_datum *datum, struct screen_flash_definition const *screen_flash, real scale, real time_scale);
static void player_effect_update_camera_impulse(short local_player_index, struct player_effect_datum *datum, struct camera_impulse_definition const *camera_impulse, real_vector3d const *direction, real scale, real time_scale);
static void player_effect_update_camera_shake(short local_player_index, struct player_effect_datum *datum, struct camera_shake_definition const *camera_shake, real scale, real time_scale);
static real effect_scale_value(short function, real scale, real time, real duration);

/* ---------- globals */

static short render_screen_flash_type_map[NUMBER_OF_SCREEN_FLASH_TYPES] =
{
	_render_screen_flash_type_none,
	_render_screen_flash_type_lighten,
	_render_screen_flash_type_darken,
	_render_screen_flash_type_max,
	_render_screen_flash_type_min,
	_render_screen_flash_type_invert,
	_render_screen_flash_type_tint
};

static struct player_effect_globals_definition *player_effect_globals;

/* ---------- public code */

struct player_effect_datum *player_effect_get(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\effects\\player_effects.c", 115, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
	match_assert("c:\\halo\\SOURCE\\effects\\player_effects.c", 116, player_effect_globals);

	return &player_effect_globals->local_player_effect_data[local_player_index];
}

void player_effect_initialize(
	void)
{
	player_effect_globals = game_state_malloc("player effects", NULL, sizeof(struct player_effect_globals_definition));
	match_assert("c:\\halo\\SOURCE\\effects\\player_effects.c", 127, player_effect_globals);

	return;
}

void player_effect_dispose(
	void)
{
	return;
}

void player_effect_initialize_for_new_map(
	void)
{
	memset(player_effect_globals, 0, sizeof(struct player_effect_globals_definition));
	player_effect_globals->screen_fade.ticks = NONE;
	player_effect_globals->reference_time = game_time_get();

	return;
}

void player_effect_dispose_from_old_map(
	void)
{
	return;
}

void player_effect_update(
	void)
{
	short local_player_index;

	for (local_player_index = local_player_get_next(NONE); local_player_index != NONE; local_player_index = local_player_get_next(local_player_index))
	{
		long unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index;

		if (unit_index == NONE)
		{
			player_effect_clear_damage_indicators(local_player_index);
			memset(player_effect_get(local_player_index), 0, sizeof(struct player_effect_datum));
			rumble_player_clear(local_player_index);
		}
	}

	return;
}

void player_effect_add_continuous_effect(
	short local_player_index,
	long effect_index,
	real distance)
{
	struct continuous_damage_effect_definition const *definition = continuous_damage_effect_definition_get(effect_index);

	if (distance < definition->cutoff_radius)
	{
		struct player_effect_datum *datum = player_effect_get(local_player_index);
		struct continuous_player_effect_datum *continuous_effect = &datum->continuous_effect;
		real scale = PIN(1.f - (distance - definition->falloff_radius) / (definition->falloff_radius - definition->cutoff_radius), 0.f, 1.f);
		real periodic_value = periodic_function_evaluate(definition->camera_shake.periodic_function, (real)game_time_get() / definition->camera_shake.periodic_period);
		real shake_scale = (1.f - definition->camera_shake.periodic_weight) * scale + scale * periodic_value * definition->camera_shake.periodic_weight;

		if (datum->continuous_effect_timer > 0)
		{
			datum->continuous_effect_timer = 0;
			memset(continuous_effect, 0, sizeof(*continuous_effect));
		}

		continuous_effect->translational_shake += MAX(definition->camera_shake.random_translation_magnitude * shake_scale, 0.f);
		continuous_effect->rotational_shake += MAX(definition->camera_shake.random_rotation_magnitude * shake_scale, 0.f);
		continuous_effect->vibrate_frequencies[_vibrate_frequency_low] += scale * definition->vibrate.frequencies[_vibrate_frequency_low];
		continuous_effect->vibrate_frequencies[_vibrate_frequency_high] += scale * definition->vibrate.frequencies[_vibrate_frequency_high];
	}

	return;
}

void player_effect_continuous_refresh(
	long effect_index,
	real_point3d const *origin)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		long unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index;

		if (unit_index != NONE)
		{
			real_point3d position;

			object_get_origin(unit_index, &position);
			player_effect_add_continuous_effect(local_player_index, effect_index, distance3d(&position, origin));
		}
	}

	return;
}

void scripted_player_effect_set_translation(
	real i,
	real j,
	real k)
{
	set_real_point3d(&player_effect_globals->scripted_effect.max_translation, i, j, k);

	return;
}

void scripted_player_effect_set_rotation(
	real yaw,
	real pitch,
	real roll)
{
	player_effect_globals->scripted_effect.max_rotation.yaw = yaw * (_pi / 180.f);
	player_effect_globals->scripted_effect.max_rotation.pitch = pitch * (_pi / 180.f);
	player_effect_globals->scripted_effect.max_rotation.roll = roll * (_pi / 180.f);

	return;
}

void scripted_player_effect_set_rumble(
	real left,
	real right)
{
	rumble_player_set_scripted_values(left, right);

	return;
}

void scripted_player_effect_start(
	real max_intensity,
	real attack)
{
	player_effect_globals->scripted_effect.max_intensity = max_intensity;
	player_effect_globals->scripted_effect.timer = (short)fast_ftol(attack * TICKS_PER_SECOND);
	player_effect_globals->scripted_effect.total_time = player_effect_globals->scripted_effect.timer;
	SET_FLAG(player_effect_globals->global_flags, _scripted_effect_running_bit, TRUE);
	SET_FLAG(player_effect_globals->global_flags, _scripted_effect_ending_bit, FALSE);

	return;
}

void scripted_player_effect_stop(
	real decay)
{
	player_effect_globals->scripted_effect.timer = (short)fast_ftol(decay * TICKS_PER_SECOND);
	player_effect_globals->scripted_effect.total_time = player_effect_globals->scripted_effect.timer;
	SET_FLAG(player_effect_globals->global_flags, _scripted_effect_ending_bit, TRUE);

	return;
}

void player_effect_screen_flash(
	long player_index,
	struct screen_flash_definition const *screen_flash,
	real scale)
{
	if (player_index != NONE)
	{
		long local_player_index = player_get(player_index)->local_player_index;

		if (local_player_index != NONE)
		{
			struct player_effect_datum *datum = player_effect_get(local_player_index);

			player_effect_update_screen_flash(local_player_index, datum, screen_flash, scale, 1.f);
		}
	}

	return;
}

void player_telefrag_effect_start(
	long player_index,
	real scale)
{
	struct screen_flash_definition screen_flash = {0};
	struct camera_shake_definition camera_shake = {0};
	long local_player_index = player_get(player_index)->local_player_index;

	if (local_player_index != NONE)
	{
		struct player_effect_datum *datum = player_effect_get(local_player_index);

		camera_shake.duration = 1.f;
		camera_shake.random_translation_magnitude = scale * 0.01;
		screen_flash.type = _screen_flash_type_lighten;
		screen_flash.priority = _screen_flash_high_priority;
		screen_flash.duration = 1.f;
		screen_flash.max_intensity = scale;
		screen_flash.zero_scale_factor = 0.f;
		screen_flash.screen_flash_color = *global_real_argb_white;
		rumble_player_continuous(local_player_index, scale, scale);
		player_effect_update_screen_flash(local_player_index, datum, &screen_flash, scale, 1.f);
		player_effect_update_camera_shake(local_player_index, datum, &camera_shake, scale, 1.f);
	}

	return;
}

void player_telefrag_effect_stop(
	long player_index)
{
	long local_player_index = player_get(player_index)->local_player_index;

	if (local_player_index != NONE)
	{
		player_effect_get(local_player_index);
		rumble_player_continuous(local_player_index, 0.f, 0.f);
	}

	return;
}

void player_effect_start(
	long player_index,
	struct damage_data const *damage,
	real_vector3d const *direction,
	real scale,
	real damage_amount)
{
	short local_player_index = player_get(player_index)->local_player_index;

	match_assert("c:\\halo\\SOURCE\\effects\\player_effects.c", 342, direction);

	lock_global_random_seed();

	if (local_player_index != NONE)
	{
		struct damage_effect_definition const *definition = damage_effect_definition_get(damage->definition_index);
		struct player_effect_datum *datum = player_effect_get(local_player_index);

		player_effect_update_screen_flash(local_player_index, datum, &definition->screen_flash, scale, 1.f);
		player_effect_update_camera_impulse(
			local_player_index,
			datum,
			&definition->camera_impulse,
			direction,
			scale,
			1.f);
		player_effect_update_camera_shake(local_player_index, datum, &definition->camera_shake, scale, 1.f);
		rumble_player_impulse(local_player_index, &definition->vibrate, scale, 1.f);

		if (definition->sound.index != NONE)
		{
			unspatialized_impulse_sound_new(definition->sound.index, 1.f);
		}

		if (damage_amount > 0.f && damage->owner_object_index != NONE)
		{
			if (TEST_FLAG(definition->damage.flags, _damage_draw_indicators_down_bit))
			{
				datum->damage_indicator_ticks[_damage_indicate_bottom] = 1;
			}
			else
			{
				long unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index;
				struct unit_datum const *unit = unit_try_and_get(unit_index);
				struct object_datum const *source_object = object_try_and_get(damage->owner_object_index);

				if (unit && source_object)
				{
					struct observer_result const *camera = observer_get_camera(local_player_index);

					if (camera)
					{
						real_point3d position;
						real_point3d source_object_position;
						real_vector3d line_of_sight_direction;
						real_vector3d local_direction;
						real_vector3d right;

						unit_get_head_position(unit_index, &position);
						object_get_origin(damage->owner_object_index, &source_object_position);
						vector_from_points3d(&position, &source_object_position, &line_of_sight_direction);
						cross_product3d(&camera->forward, &camera->up, &right);
						set_real_vector3d(
							&local_direction,
							dot_product3d(&line_of_sight_direction, &right),
							dot_product3d(&line_of_sight_direction, &camera->forward),
							dot_product3d(&line_of_sight_direction, &camera->up));

						if (normalize3d(&local_direction) != 0.f)
						{
							real angle;
							real absolute_angle;

							if (fabs(local_direction.k) > 0.5f)
							{
								if (local_direction.k > 0.f)
								{
									datum->damage_indicator_ticks[_damage_indicate_top] = 1;
								}
								else
								{
									datum->damage_indicator_ticks[_damage_indicate_bottom] = 1;
								}
							}

							angle = arctangent(local_direction.j, local_direction.i);
							absolute_angle = fabs(angle);

							if (angle < _pi / 4.f || angle > 3.f * _pi / 4.f)
							{
								if (absolute_angle > _pi2)
								{
									datum->damage_indicator_ticks[_damage_indicate_left] = 1;
								}
								else
								{
									datum->damage_indicator_ticks[_damage_indicate_right] = 1;
								}
							}
						}
					}
				}
			}
		}
	}

	unlock_global_random_seed();

	return;
}

void player_effect_screen_fade_in(
	real red,
	real green,
	real blue,
	short ticks)
{
	player_effect_globals->screen_fade.color.red = red;
	player_effect_globals->screen_fade.color.green = green;
	player_effect_globals->screen_fade.color.blue = blue;
	player_effect_globals->screen_fade.ticks = ticks;
	player_effect_globals->screen_fade.fading_out = FALSE;
	player_effect_globals->screen_fade.start_time = game_time_get();

	return;
}

void player_effect_screen_fade_out(
	real red,
	real green,
	real blue,
	short ticks)
{
	player_effect_globals->screen_fade.color.red = red;
	player_effect_globals->screen_fade.color.green = green;
	player_effect_globals->screen_fade.color.blue = blue;
	player_effect_globals->screen_fade.ticks = ticks;
	player_effect_globals->screen_fade.fading_out = TRUE;
	player_effect_globals->screen_fade.start_time = game_time_get();

	return;
}

void player_effect_get_screen_flash(
	short local_player_index,
	struct render_screen_flash *screen_flash)
{
	match_assert("c:\\halo\\SOURCE\\effects\\player_effects.c", 484, screen_flash);

	if (!console_is_active())
	{
		if (player_effect_globals->screen_fade.ticks == NONE ||
			(!player_effect_globals->screen_fade.fading_out && game_time_get() - player_effect_globals->screen_fade.start_time > player_effect_globals->screen_fade.ticks))
		{
			if (local_player_index != NONE)
			{
				struct player_effect_datum *datum = player_effect_get(local_player_index);
				struct screen_flash_definition const *definition = &datum->screen_flash;

				player_effect_globals->screen_fade.ticks = NONE;

				if (datum->screen_flash_time_left > 0 || TEST_FLAG(datum->flags, _screen_flash_just_started_bit))
				{
					SET_FLAG(datum->flags, _screen_flash_just_started_bit, FALSE);
					screen_flash->type = render_screen_flash_type_map[definition->type];
					screen_flash->color = definition->screen_flash_color;

					if (definition->duration > 0.f)
					{
						screen_flash->intensity = transition_function_evaluate(
							definition->fade_function,
							((real)datum->screen_flash_time_left / definition->duration) * definition->zero_scale_factor);
					}
					else
					{
						screen_flash->intensity = definition->zero_scale_factor;
					}

					datum->screen_flash_time_left -= game_time_get_elapsed();
					match_assert("c:\\halo\\SOURCE\\effects\\player_effects.c", 530, screen_flash->intensity>=0.0f && screen_flash->intensity<=1.0f);
				}
			}
		}
		else
		{
			real intensity;

			screen_flash->type = _render_screen_flash_type_lighten;
			screen_flash->color.rgb = player_effect_globals->screen_fade.color;
			screen_flash->color.alpha = 1.f;

			if (player_effect_globals->screen_fade.ticks > 0)
			{
				intensity = transition_function_evaluate(
					_transition_function_cosine,
					PIN((real)(game_time_get() - player_effect_globals->screen_fade.start_time) / (real)player_effect_globals->screen_fade.ticks, 0.f, 1.f));
			}
			else
			{
				intensity = 1.f;
			}

			screen_flash->intensity = intensity;

			if (!player_effect_globals->screen_fade.fading_out)
			{
				screen_flash->intensity = 1.f - screen_flash->intensity;
			}

			screen_flash->intensity = PIN(screen_flash->intensity, 0.f, 1.f);
		}
	}

	match_assert_valid_real("c:\\halo\\SOURCE\\effects\\player_effects.c", 535, screen_flash->intensity);

	return;
}

void player_effect_get_damage_indicators(
	short local_player_index,
	byte *damage_indicators)
{
	short direction_index;
	struct player_effect_datum *datum = player_effect_get(local_player_index);

	memcpy(damage_indicators, datum->damage_indicator_ticks, sizeof(datum->damage_indicator_ticks));

	for (direction_index = 0; direction_index < NUMBEROF(datum->damage_indicator_ticks); direction_index++)
	{
		if (datum->damage_indicator_ticks[direction_index])
		{
			datum->damage_indicator_ticks[direction_index] = MIN(UNSIGNED_CHAR_MAX, game_time_get_elapsed() + datum->damage_indicator_ticks[direction_index]);
		}
	}

	return;
}

void player_effect_clear_damage_indicators(
	short local_player_index)
{
	struct player_effect_datum *datum = player_effect_get(local_player_index);

	memset(datum->damage_indicator_ticks, 0, sizeof(datum->damage_indicator_ticks));

	return;
}

/* ---------- private code */

static void get_shake_matrix(
	real translation,
	real rotation,
	real_matrix4x3 *matrix)
{
	if (rotation != 0.f)
	{
		real_vector3d axis;

		local_random_direction3d(&axis);
		matrix4x3_rotation_from_axis_and_angle(matrix, &axis, sine(rotation), cosine(rotation));
	}

	if (translation != 0.f)
	{
		real_vector3d offset;

		local_random_direction3d(&offset);
		scale_vector3d(&offset, translation, (real_vector3d *)&matrix->position);
	}

	return;
}

void player_effect_get_camera_effect_matrix(
	short local_player_index,
	real_matrix4x3 *matrix)
{
	match_assert("c:\\halo\\SOURCE\\effects\\player_effects.c", 601, matrix);

	if (local_player_index != NONE)
	{
		long game_time = game_time_get();

		if (TEST_FLAG(player_effect_globals->global_flags, _scripted_effect_running_bit))
		{
			struct scripted_player_effect_definition *scripted_effect = &player_effect_globals->scripted_effect;
			real intensity = scripted_effect->max_intensity;

			*matrix = *global_identity4x3;

			if (scripted_effect->timer > 0)
			{
				if (TEST_FLAG(player_effect_globals->global_flags, _scripted_effect_ending_bit))
				{
					intensity = (real)scripted_effect->timer / scripted_effect->total_time * intensity;
				}
				else
				{
					intensity *= 1.f - (real)scripted_effect->timer / scripted_effect->total_time;
				}

				scripted_effect->timer -= game_time_get_elapsed();
			}
			else if (TEST_FLAG(player_effect_globals->global_flags, _scripted_effect_ending_bit))
			{
				SET_FLAG(player_effect_globals->global_flags, _scripted_effect_running_bit, FALSE);
				rumble_player_set_scale(0.f);
			}

			if (TEST_FLAG(player_effect_globals->global_flags, _scripted_effect_running_bit))
			{
				if (intensity < 0.f)
				{
					intensity = 0.f;
				}
				else if (intensity > 1.f)
				{
					intensity = 1.f;
				}

				rumble_player_set_scale(intensity);
				matrix4x3_rotation_from_angles(
					matrix,
					scripted_effect->max_rotation.yaw * intensity * real_local_random_range(-1.f, 1.f),
					scripted_effect->max_rotation.pitch * intensity * real_local_random_range(-1.f, 1.f),
					scripted_effect->max_rotation.roll * intensity * real_local_random_range(-1.f, 1.f));
				set_real_point3d(
					&matrix->position,
					scripted_effect->max_translation.y * intensity * real_local_random_range(-1.f, 1.f),
					scripted_effect->max_translation.x * intensity * real_local_random_range(-1.f, 1.f),
					scripted_effect->max_translation.z * intensity * real_local_random_range(-1.f, 1.f));
			}
		}
		else
		{
			struct player_effect_datum *datum = player_effect_get(local_player_index);
			struct camera_impulse_definition const *camera_impulse = &datum->camera_impulse;
			struct camera_shake_definition const *camera_shake = &datum->camera_shake;

			if (datum->camera_impulse_time_left > 0 || TEST_FLAG(datum->flags, _camera_impulse_just_started_bit))
			{
				real_matrix4x3 impulse_matrix;
				real impulse_scale;
				real_vector3d axis;

				if (TEST_FLAG(datum->flags, _camera_impulse_just_started_bit))
				{
					impulse_scale = 1.f;
				}
				else
				{
					impulse_scale = effect_scale_value(
						camera_impulse->temporary_transition,
						camera_impulse->temporary_zero_scale_factor,
						camera_impulse->temporary_duration - (real)datum->camera_impulse_time_left,
						camera_impulse->temporary_duration);
				}

				SET_FLAG(datum->flags, _camera_impulse_just_started_bit, FALSE);
				cross_product3d(global_up3d, &datum->direction, &axis);
				matrix4x3_rotation_from_axis_and_angle(
					&impulse_matrix,
					&axis,
					sine(impulse_scale * camera_impulse->temporary_rotation),
					cosine(impulse_scale * camera_impulse->temporary_rotation));
				scale_vector3d(
					&datum->direction,
					impulse_scale * camera_impulse->temporary_translation,
					(real_vector3d *)&impulse_matrix.position);
				point_from_line3d(&impulse_matrix.position, &datum->jitter, impulse_scale, &impulse_matrix.position);
				datum->camera_impulse_time_left -= game_time_get_elapsed();
				*matrix = impulse_matrix;
			}
			else
			{
				*matrix = *global_identity4x3;
			}

			if (datum->camera_shake_time_left > 0 || TEST_FLAG(datum->flags, _camera_shake_just_started_bit))
			{
				real_matrix4x3 shake_matrix;
				real shake_scale;
				real periodic_value;
				real shake_intensity;
				real translation;
				real rotation;
				struct continuous_player_effect_datum *continuous_effect = &datum->continuous_effect;

				shake_matrix = *global_identity4x3;

				if (TEST_FLAG(datum->flags, _camera_shake_just_started_bit))
				{
					shake_scale = 1.f;
				}
				else
				{
					shake_scale = effect_scale_value(
						camera_shake->falloff_transition_function,
						camera_shake->zero_scale_factor,
						camera_shake->duration - (real)datum->camera_shake_time_left,
						camera_shake->duration);
				}

				periodic_value = periodic_function_evaluate(
					camera_shake->periodic_function,
					(camera_shake->duration - (real)datum->camera_shake_time_left) / camera_shake->periodic_period);
				shake_intensity = (1.f - camera_shake->periodic_weight + periodic_value * camera_shake->periodic_weight) * shake_scale;
				translation = MAX(shake_intensity * camera_shake->random_translation_magnitude, 0.f);
				rotation = MAX(shake_intensity * camera_shake->random_rotation_magnitude, 0.f);
				SET_FLAG(datum->flags, _camera_shake_just_started_bit, FALSE);
				get_shake_matrix(
					continuous_effect->translational_shake + translation,
					continuous_effect->rotational_shake + rotation,
					&shake_matrix);
				rumble_player_continuous(
					local_player_index,
					continuous_effect->vibrate_frequencies[_vibrate_frequency_low],
					continuous_effect->vibrate_frequencies[_vibrate_frequency_high]);
				datum->continuous_effect_timer += game_time_get_elapsed();

				if (datum->continuous_effect_timer > 0)
				{
					datum->continuous_effect_timer = 0;
					memset(continuous_effect, 0, sizeof(*continuous_effect));
				}

				get_shake_matrix(translation, rotation, &shake_matrix);
				datum->camera_shake_time_left -= game_time_get_elapsed();
				matrix4x3_multiply(matrix, &shake_matrix, matrix);
			}
		}
	}

	return;
}

static real effect_scale_factor(
	real zero_scale_factor,
	real scale)
{
	return (1.f - zero_scale_factor) * scale + zero_scale_factor;
}

static void player_effect_update_screen_flash(
	short local_player_index,
	struct player_effect_datum *datum,
	struct screen_flash_definition const *screen_flash,
	real scale,
	real time_scale)
{
	boolean replace = TRUE;
	real ticks = time_scale * TICKS_PER_SECOND;

	if (datum->screen_flash.priority > screen_flash->priority && (real)datum->screen_flash_time_left > screen_flash->duration * ticks ||
		!render_screen_flash_type_map[screen_flash->type])
	{
		replace = FALSE;
	}

	if (replace)
	{
		datum->screen_flash = *screen_flash;
		datum->screen_flash.duration *= ticks;
		datum->screen_flash_time_left = (short)datum->screen_flash.duration;
		datum->screen_flash.zero_scale_factor = PIN(effect_scale_factor(screen_flash->zero_scale_factor, scale), 0.f, screen_flash->max_intensity);
		SET_FLAG(datum->flags, _screen_flash_just_started_bit, TRUE);
	}

	return;
}

static void player_effect_update_camera_impulse(
	short local_player_index,
	struct player_effect_datum *datum,
	struct camera_impulse_definition const *camera_impulse,
	real_vector3d const *direction,
	real scale,
	real time_scale)
{
	long game_time = game_time_get();
	real ticks = time_scale * TICKS_PER_SECOND;
	real temporary_scale = effect_scale_factor(camera_impulse->temporary_zero_scale_factor, scale);

	if (datum->camera_impulse.temporary_duration > (real)datum->camera_impulse_time_left ||
		temporary_scale > datum->camera_impulse.temporary_zero_scale_factor ||
		temporary_scale >= datum->camera_impulse.temporary_zero_scale_factor && (real)datum->camera_impulse_time_left < camera_impulse->temporary_duration * ticks)
	{
		real_vector3d normalized_direction;
		real_euler_angles2d facing;
		real_vector3d normalized_facing;

		normalized_direction = *direction;
		normalized_direction.k = 0.f;
		normalize3d(&normalized_direction);
		facing = *player_control_get_facing_angles(local_player_index);
		vector3d_from_euler_angles2d(&normalized_facing, &facing);
		normalized_facing.k = 0.f;
		normalize3d(&normalized_facing);

		if (fabs((real)(magnitude_squared3d(&normalized_direction) - 1.f)) < _real_epsilon &&
			fabs((real)(magnitude_squared3d(&normalized_facing) - 1.f)) < _real_epsilon)
		{
			real jitter;
			real jitter_angle;
			real angle = signed_angle_between_vectors2d((real_vector2d const *)&normalized_facing, (real_vector2d const *)&normalized_direction);

			datum->camera_impulse = *camera_impulse;
			datum->camera_impulse.temporary_zero_scale_factor = temporary_scale;
			datum->camera_impulse.temporary_duration *= ticks;
			datum->camera_impulse_time_left = (short)datum->camera_impulse.temporary_duration;
			datum->direction.i = cosine(angle);
			datum->direction.j = sine(angle);
			datum->direction.k = 0.f;
			jitter = real_local_random_range(
				datum->camera_impulse.temporary_jitter_lower_bound,
				datum->camera_impulse.temporary_jitter_upper_bound);
			jitter_angle = real_local_random_range(0.f, 2.f * _pi);
			cross_product3d(&datum->direction, global_up3d, &datum->jitter);
			normalize3d(&datum->jitter);
			rotate_vector_about_axis(&datum->jitter, &datum->direction, sine(jitter_angle), cosine(jitter_angle));
			scale_vector3d(&datum->jitter, jitter, &datum->jitter);
			SET_FLAG(datum->flags, _camera_impulse_just_started_bit, TRUE);
		}
	}

	{
		real_vector3d facing;
		real_vector3d perp;
		real_euler_angles2d permanent_impulse;
		real permanent_scale = effect_scale_factor(camera_impulse->permanent_zero_scale_factor, scale);

		player_control_get_facing_direction(local_player_index, &facing);
		cross_product3d(global_up3d, &facing, &perp);
		permanent_impulse.yaw = dot_product3d(&perp, direction) * camera_impulse->permanent_angle * permanent_scale;
		permanent_impulse.pitch = dot_product3d(&facing, direction) * camera_impulse->permanent_angle * permanent_scale;
		player_control_permanent_impulse(local_player_index, &permanent_impulse);
	}

	return;
}

static void player_effect_update_camera_shake(
	short local_player_index,
	struct player_effect_datum *datum,
	struct camera_shake_definition const *camera_shake,
	real scale,
	real time_scale)
{
	real ticks;
	real shake_scale;

	game_time_get();
	ticks = time_scale * TICKS_PER_SECOND;
	shake_scale = effect_scale_factor(camera_shake->zero_scale_factor, scale);

	if ((real)datum->camera_shake_time_left < camera_shake->duration * ticks ||
		shake_scale > datum->camera_shake.zero_scale_factor ||
		shake_scale >= datum->camera_shake.zero_scale_factor && (real)datum->camera_shake_time_left < camera_shake->duration * ticks)
	{
		datum->camera_shake = *camera_shake;
		datum->camera_shake.zero_scale_factor = shake_scale;
		datum->camera_shake.duration *= ticks;
		datum->camera_shake_time_left = (short)datum->camera_shake.duration;
		datum->camera_shake.periodic_period *= ticks;
		SET_FLAG(datum->flags, _camera_shake_just_started_bit, TRUE);
	}

	return;
}

static real effect_scale_value(
	short function,
	real scale,
	real time,
	real duration)
{
	return transition_function_evaluate(function, 1.f - time / duration) * scale;
}
