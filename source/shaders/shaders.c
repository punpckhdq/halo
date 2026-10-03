/*
SHADERS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "shaders.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"

/* ---------- constants */

enum
{
	_vertex_shader_permutation_none = 0, /* fake name */
	_vertex_shader_permutation_first_type, /* fake name */
	_vertex_shader_permutation_transparent_lit = 5 /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static long numeric_countdown_timer_milliseconds;
static boolean numeric_countdown_timer_on;

/* ---------- public code */

short shader_get_vertex_shader_permutation(
	struct shader const *shader)
{
	short result;

	match_assert("c:\\halo\\SOURCE\\shaders\\shaders.c", 20, shader);

	if (shader == (struct shader const *)NONE)
	{
		result = _vertex_shader_permutation_none;
	}
	else
	{
		switch (shader->base.type)
		{
		case _shader_type_model:
		{
			struct shader_model const *shader_model = shader_get_and_verify_type(shader, _shader_type_model);

			if (shader_model->model.translucency > 0.0f)
			{
				result = _vertex_shader_permutation_first_type;
			}
			else
			{
				result = _vertex_shader_permutation_none;
			}
			break;
		}
		case _shader_type_effect:
			if (((struct shader_effect const *)shader_get_and_verify_type(shader, _shader_type_effect))->effect.secondary_map.index != NONE)
			{
				result = ((struct shader_effect const *)shader_get_and_verify_type(shader, _shader_type_effect))->effect.secondary_map_anchor + _vertex_shader_permutation_first_type;
			}
			else
			{
				result = _vertex_shader_permutation_none;
			}
			break;
		case _shader_type_transparent_generic:
			result = ((struct shader_transparent_generic const *)shader_get_and_verify_type(shader, _shader_type_transparent_generic))->generic.type + _vertex_shader_permutation_first_type;
			if (result == _vertex_shader_permutation_first_type && !TEST_FLAG(((struct shader_transparent_generic const *)shader_get_and_verify_type(shader, _shader_type_transparent_generic))->generic.flags, _shader_transparent_generic_first_map_is_in_screenspace_bit))
			{
				result = _vertex_shader_permutation_none;
			}
			if (TEST_FLAG(shader->base.radiosity.flags, _shader_radiosity_FILTHY_transparent_lit_bit))
			{
				result = _vertex_shader_permutation_transparent_lit;
			}
			break;
		case _shader_type_transparent_chicago:
			result = ((struct shader_transparent_chicago const *)shader_get_and_verify_type(shader, _shader_type_transparent_chicago))->chicago.type + _vertex_shader_permutation_first_type;
			if (result == _vertex_shader_permutation_first_type && !TEST_FLAG(((struct shader_transparent_chicago const *)shader_get_and_verify_type(shader, _shader_type_transparent_chicago))->chicago.flags, _shader_transparent_chicago_first_map_is_in_screenspace_bit))
			{
				result = _vertex_shader_permutation_none;
			}
			if (TEST_FLAG(shader->base.radiosity.flags, _shader_radiosity_FILTHY_transparent_lit_bit))
			{
				result = _vertex_shader_permutation_transparent_lit;
			}
			break;
		default:
			result = _vertex_shader_permutation_none;
			break;
		}
	}

	return result;
}

boolean shader_is_mirror(
	struct shader *shader)
{
	boolean result = FALSE;

	if (shader)
	{
		switch (shader->base.type)
		{
		case _shader_type_environment:
		{
			struct shader_environment const *shader_environment = shader_get_and_verify_type(shader, _shader_type_environment);

			result = TEST_FLAG(shader_environment->environment.reflection.flags, _shader_environment_reflection_mirror_bit);
			break;
		}
		case _shader_type_transparent_glass:
		{
			struct shader_transparent_glass const *shader_transparent_glass = shader_get_and_verify_type(shader, _shader_type_transparent_glass);

			result = shader_transparent_glass->glass.reflection_type == _shader_transparent_glass_reflection_type_mirror;
			break;
		}
		}
	}

	return result;
}

boolean shader_is_destructable(
	struct shader *shader)
{
	boolean result = FALSE;

	return result;
}

boolean shader_is_double_sided(
	struct shader *shader)
{
	boolean result = FALSE;

	return result;
}

boolean shader_is_decal(
	struct shader const *shader)
{
	boolean result = FALSE;

	if (shader)
	{
		switch (shader->base.type)
		{
		case _shader_type_transparent_generic:
		{
			struct shader_transparent_generic const *shader_transparent_generic = shader_get_and_verify_type(shader, _shader_type_transparent_generic);

			result = TEST_FLAG(shader_transparent_generic->generic.flags, _shader_transparent_generic_decal_bit);
			break;
		}
		case _shader_type_transparent_chicago:
		{
			struct shader_transparent_chicago const *shader_transparent_chicago = shader_get_and_verify_type(shader, _shader_type_transparent_chicago);

			result = TEST_FLAG(shader_transparent_chicago->chicago.flags, _shader_transparent_chicago_decal_bit);
			break;
		}
		case _shader_type_transparent_glass:
		{
			struct shader_transparent_glass const *shader_transparent_glass = shader_get_and_verify_type(shader, _shader_type_transparent_glass);

			result = TEST_FLAG(shader_transparent_glass->glass.flags, _shader_transparent_glass_decal_bit);
			break;
		}
		case _shader_type_transparent_meter:
		{
			struct shader_transparent_meter const *shader_transparent_meter = shader_get_and_verify_type(shader, _shader_type_transparent_meter);

			result = TEST_FLAG(shader_transparent_meter->meter.flags, _shader_transparent_meter_decal_bit);
			break;
		}
		}
	}

	return result;
}

boolean shader_is_water_decal(
	struct shader const *shader)
{
	boolean result = FALSE;

	if (shader)
	{
		switch (shader->base.type)
		{
		case _shader_type_transparent_generic:
		{
			struct shader_transparent_generic const *shader_transparent_generic = shader_get_and_verify_type(shader, _shader_type_transparent_generic);

			result = TEST_FLAG(shader_transparent_generic->generic.flags, _shader_transparent_generic_draw_before_water_bit);
			break;
		}
		case _shader_type_transparent_chicago:
		{
			struct shader_transparent_chicago const *shader_transparent_chicago = shader_get_and_verify_type(shader, _shader_type_transparent_chicago);

			result = TEST_FLAG(shader_transparent_chicago->chicago.flags, _shader_transparent_chicago_draw_before_water_bit);
			break;
		}
		}
	}

	return result;
}

boolean shader_ignores_effect(
	struct shader const *shader)
{
	boolean result = FALSE;

	if (shader)
	{
		switch (shader->base.type)
		{
		case _shader_type_transparent_generic:
		{
			struct shader_transparent_generic const *shader_transparent_generic = shader_get_and_verify_type(shader, _shader_type_transparent_generic);

			result = TEST_FLAG(shader_transparent_generic->generic.flags, _shader_transparent_generic_ignore_effect_bit);
			break;
		}
		case _shader_type_transparent_chicago:
		{
			struct shader_transparent_chicago const *shader_transparent_chicago = shader_get_and_verify_type(shader, _shader_type_transparent_chicago);

			result = TEST_FLAG(shader_transparent_chicago->chicago.flags, _shader_transparent_chicago_ignore_effect_bit);
			break;
		}
		}
	}

	return result;
}

boolean shader_type_is_transparent(
	short shader_type)
{
	boolean result = FALSE;

	switch (shader_type)
	{
	case _shader_type_effect:
	case _shader_type_transparent_generic:
	case _shader_type_transparent_chicago:
	case _shader_type_transparent_water:
	case _shader_type_transparent_glass:
	case _shader_type_transparent_meter:
	case _shader_type_transparent_plasma:
		result = TRUE;
		break;
	}

	return result;
}

boolean shader_type_is_lightmapped(
	short shader_type)
{
	boolean result = FALSE;

	switch (shader_type)
	{
	case _shader_type_environment:
	case _shader_type_model:
	case _shader_type_transparent_glass:
		result = TRUE;
		break;
	}

	return result;
}

boolean shader_type_is_vertex_lit(
	short shader_type)
{
	boolean result = FALSE;

	switch (shader_type)
	{
	case _shader_type_model:
	case _shader_type_transparent_glass:
		result = TRUE;
		break;
	}

	return result;
}

boolean shader_type_is_valid_for_environment(
	short shader_type)
{
	boolean result = FALSE;

	switch (shader_type)
	{
	case _shader_type_environment:
	case _shader_type_transparent_generic:
	case _shader_type_transparent_chicago:
	case _shader_type_transparent_water:
	case _shader_type_transparent_glass:
	case _shader_type_transparent_meter:
		result = TRUE;
		break;
	}

	return result;
}

boolean shader_type_is_valid_for_model(
	short shader_type)
{
	boolean result = FALSE;

	switch (shader_type)
	{
	case _shader_type_environment:
	case _shader_type_model:
	case _shader_type_transparent_generic:
	case _shader_type_transparent_chicago:
	case _shader_type_transparent_water:
	case _shader_type_transparent_glass:
	case _shader_type_transparent_meter:
	case _shader_type_transparent_plasma:
		result = TRUE;
		break;
	}

	return result;
}

boolean shader_type_is_valid_for_modifier(
	short shader_type)
{
	boolean result = FALSE;

	switch (shader_type)
	{
	case _shader_type_effect:
	case _shader_type_transparent_generic:
	case _shader_type_transparent_chicago:
	case _shader_type_transparent_water:
	case _shader_type_transparent_glass:
	case _shader_type_transparent_meter:
	case _shader_type_transparent_plasma:
		result = TRUE;
		break;
	}

	return result;
}

void shader_texture_animation_evaluate(
	struct shader_texture_animation const *texture_animation,
	struct render_animation const *render_animation,
	real u_scale,
	real v_scale,
	real u_offset,
	real v_offset,
	real r_offset,
	real time_value,
	real_vector4d *u_transform_reference,
	real_vector4d *v_transform_reference)
{
	real u_period, v_period, r_period;
	real u_value, v_value, r_value;
	real cosine_angle, sine_angle;

	match_assert("c:\\halo\\SOURCE\\shaders\\shaders.c", 275, texture_animation);
	match_assert("c:\\halo\\SOURCE\\shaders\\shaders.c", 276, texture_animation->u_source>=0 && texture_animation->u_source<NUMBER_OF_OBJECT_FUNCTION_REFERENCES);
	match_assert("c:\\halo\\SOURCE\\shaders\\shaders.c", 277, texture_animation->v_source>=0 && texture_animation->v_source<NUMBER_OF_OBJECT_FUNCTION_REFERENCES);
	match_assert("c:\\halo\\SOURCE\\shaders\\shaders.c", 278, texture_animation->r_source>=0 && texture_animation->r_source<NUMBER_OF_OBJECT_FUNCTION_REFERENCES);
	match_assert("c:\\halo\\SOURCE\\shaders\\shaders.c", 279, u_transform_reference);
	match_assert("c:\\halo\\SOURCE\\shaders\\shaders.c", 280, v_transform_reference);

	u_period = texture_animation->u_period == 0.0f ? 1.0f : texture_animation->u_period;
	v_period = texture_animation->v_period == 0.0f ? 1.0f : texture_animation->v_period;
	r_period = texture_animation->r_period == 0.0f ? 1.0f : texture_animation->r_period;

	if (render_animation)
	{
		real const *values = render_animation->values;

		u_value = texture_animation->u_source == _object_function_reference_none ? 1.0f : values[texture_animation->u_source - 1];
		v_value = texture_animation->v_source == _object_function_reference_none ? 1.0f : values[texture_animation->v_source - 1];
		r_value = texture_animation->r_source == _object_function_reference_none ? 1.0f : values[texture_animation->r_source - 1];
	}
	else
	{
		u_value = v_value = r_value = 1.0f;
	}

	u_value *= periodic_function_evaluate(texture_animation->u_function, (time_value + texture_animation->u_phase) / u_period) * texture_animation->u_scale;
	v_value *= periodic_function_evaluate(texture_animation->v_function, (time_value + texture_animation->v_phase) / v_period) * texture_animation->v_scale;
	r_value *= periodic_function_evaluate(texture_animation->r_function, (time_value + texture_animation->r_phase) / r_period) * texture_animation->r_scale;

	u_value = u_offset - texture_animation->r_center.x + u_value;
	v_value = v_offset - texture_animation->r_center.y + v_value;
	r_value += r_offset;

	if (r_value != 0.0f)
	{
		real angle = DEGREES_TO_RADIANS(r_value);

		cosine_angle = cosine(angle);
		sine_angle = sine(angle);
	}
	else
	{
		cosine_angle = 1.0f;
		sine_angle = 0.0f;
	}

	u_transform_reference->k = 0.0f;
	u_transform_reference->i = cosine_angle * u_scale;
	u_transform_reference->j = -(v_scale * sine_angle);
	u_transform_reference->l = cosine_angle * u_value - sine_angle * v_value + texture_animation->r_center.x;
	v_transform_reference->k = 0.0f;
	v_transform_reference->i = u_scale * sine_angle;
	v_transform_reference->j = cosine_angle * v_scale;
	v_transform_reference->l = cosine_angle * v_value + sine_angle * u_value + texture_animation->r_center.y;

	return;
}

void shader_environment_texture_animation_evaluate(
	struct shader const *shader,
	real time_value,
	real *u_offset,
	real *v_offset)
{
	struct shader_environment const *shader_environment;
	struct shader_environment_diffuse_properties const *diffuse;

	match_assert("c:\\halo\\SOURCE\\shaders\\shaders.c", 345, shader);
	match_assert("c:\\halo\\SOURCE\\shaders\\shaders.c", 346, u_offset);
	match_assert("c:\\halo\\SOURCE\\shaders\\shaders.c", 347, v_offset);

	shader_environment = shader_get_and_verify_type(shader, _shader_type_environment);
	diffuse = &shader_environment->environment.diffuse;

	match_assert("c:\\halo\\SOURCE\\shaders\\shaders.c", 352, diffuse->u_animation_period!=0.0f);
	match_assert("c:\\halo\\SOURCE\\shaders\\shaders.c", 353, diffuse->v_animation_period!=0.0f);

	*u_offset = periodic_function_evaluate(diffuse->u_animation_function, time_value / diffuse->u_animation_period) * diffuse->u_animation_scale;
	*v_offset = periodic_function_evaluate(diffuse->v_animation_function, time_value / diffuse->v_animation_period) * diffuse->v_animation_scale;

	return;
}

void numeric_countdown_timer_set(
	long milliseconds,
	boolean auto_start)
{
	numeric_countdown_timer_milliseconds = milliseconds;
	numeric_countdown_timer_on = auto_start;

	return;
}

short numeric_countdown_timer_get(
	short digit_index)
{
	long digit = 0;

	switch (digit_index)
	{
	case NONE:
		digit = numeric_countdown_timer_milliseconds;
		break;
	case 0:
		digit = numeric_countdown_timer_milliseconds % 10;
		break;
	case 1:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND / 100) % 10;
		break;
	case 2:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND / 10) % 10;
		break;
	case 3:
		digit = numeric_countdown_timer_milliseconds / MILLISECONDS_PER_SECOND % 10;
		break;
	case 4:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND * 10) % 6;
		break;
	case 5:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND * SECONDS_PER_MINUTE) % 10;
		break;
	case 6:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND * SECONDS_PER_MINUTE * 10) % 6;
		break;
	case 7:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND * SECONDS_PER_MINUTE * MINUTES_PER_HOUR) % 10;
		break;
	case 8:
		digit = numeric_countdown_timer_milliseconds / (MILLISECONDS_PER_SECOND * SECONDS_PER_MINUTE * MINUTES_PER_HOUR * 10) % 10;
		break;
	}

	return digit;
}

void numeric_countdown_timer_stop(
	void)
{
	numeric_countdown_timer_on = FALSE;

	return;
}

void numeric_countdown_timer_restart(
	void)
{
	numeric_countdown_timer_on = TRUE;

	return;
}

void numeric_countdown_timer_update(
	void)
{
	static long previous_game_time;

	if (numeric_countdown_timer_on)
	{
		long game_time = MILLISECONDS_PER_SECOND * game_time_get() / TICKS_PER_SECOND;

		if (game_time >= previous_game_time)
		{
			numeric_countdown_timer_milliseconds = numeric_countdown_timer_milliseconds - game_time + previous_game_time;

			if (numeric_countdown_timer_milliseconds < 0)
			{
				numeric_countdown_timer_milliseconds = 0;
			}
		}

		previous_game_time = game_time;
	}

	return;
}

/* ---------- private code */
