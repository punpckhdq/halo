/*
RASTERIZER_CINEMATICS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer_cinematics.h"
#include "rasterizer.h"
#include "rasterizer_console_vars.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "bitmaps_inlines.h"
#include "game_state.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "main.h"
#include "game.h"
#include "game_globals.h"

/* ---------- constants */

enum
{
	NUMBER_OF_SCRIPT_SCREEN_EFFECT_VALUES = 4, /* fake name */
};

/* ---------- structures */

struct cinematic_screen_effect_globals
{
	struct rasterizer_screen_effect_parameters parameters;
	boolean has_control;
	boolean initialized;
	real convolution_radius[2];
	real convolution_time[2];
	real filter_light_enhancement_intensity[2];
	real filter_desaturation_intensity[2];
	real filter_time[2];
	real script_values[NUMBER_OF_SCRIPT_SCREEN_EFFECT_VALUES];
	real near_clip_distance;
};

/* ---------- globals */

static struct cinematic_screen_effect_globals *cinematic_screen_effect_globals;

/* ---------- public code */

static real rasterizer_screen_effects_time(
	void)
{
	return (real)game_time_get()*SECONDS_PER_TICK;
}

void rasterizer_screen_effects_initialize(
	void)
{
	cinematic_screen_effect_globals = game_state_malloc("screen effect filth", NULL, sizeof(*cinematic_screen_effect_globals));
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_cinematics.c", 54, cinematic_screen_effect_globals);

	return;
}

void rasterizer_screen_effects_initialize_for_new_map(
	void)
{
	if (cinematic_screen_effect_globals)
	{
		memset(cinematic_screen_effect_globals, 0, sizeof(*cinematic_screen_effect_globals));
		cinematic_screen_effect_globals->script_values[0] = 1.f;
		cinematic_screen_effect_globals->script_values[1] = 1.f;
		cinematic_screen_effect_globals->script_values[2] = 1.f;
		cinematic_screen_effect_globals->script_values[3] = 1.f;
	}

	return;
}

void rasterizer_screen_effects_dispose_from_old_map(
	void)
{
	return;
}

void rasterizer_screen_effects_dispose(
	void)
{
	return;
}

void rasterizer_script_screen_effect_set_value(
	short index,
	real value)
{
	if (cinematic_screen_effect_globals && index>=0 && index<NUMBER_OF_SCRIPT_SCREEN_EFFECT_VALUES)
	{
		cinematic_screen_effect_globals->script_values[index] = value;
	}

	return;
}

real rasterizer_script_screen_effect_get_value(
	short index)
{
	real value = 0.f;

	if (cinematic_screen_effect_globals && index>=0 && index<NUMBER_OF_SCRIPT_SCREEN_EFFECT_VALUES)
	{
		value = cinematic_screen_effect_globals->script_values[index];
	}

	return value;
}

void rasterizer_screen_effect_start(
	boolean clear)
{
	if (cinematic_screen_effect_globals)
	{
		if (clear || !cinematic_screen_effect_globals->initialized)
		{
			memset(&cinematic_screen_effect_globals->parameters, 0, sizeof(cinematic_screen_effect_globals->parameters));
			cinematic_screen_effect_globals->initialized = TRUE;
		}
		cinematic_screen_effect_globals->has_control = TRUE;
	}

	return;
}

void rasterizer_screen_effect_set_convolution(
	short convolution_extra_passes,
	short convolution_type,
	real convolution_radius_lower_bound,
	real convolution_radius_upper_bound,
	real convolution_time)
{
	if (cinematic_screen_effect_globals)
	{
		cinematic_screen_effect_globals->parameters.video_on = FALSE;
		cinematic_screen_effect_globals->parameters.video_overbright_mode = _rasterizer_screen_effect_video_overbright_mode_none;
		cinematic_screen_effect_globals->parameters.video_scanline_map = NULL;
		cinematic_screen_effect_globals->parameters.video_noise_intensity = 0.f;
		cinematic_screen_effect_globals->parameters.video_noise_map_scale = 0.f;
		cinematic_screen_effect_globals->parameters.video_noise_map = NULL;
		cinematic_screen_effect_globals->parameters.convolution_extra_passes = convolution_extra_passes;
		cinematic_screen_effect_globals->parameters.convolution_type = convolution_type;
		cinematic_screen_effect_globals->convolution_radius[0] = convolution_radius_lower_bound;
		cinematic_screen_effect_globals->convolution_radius[1] = convolution_radius_upper_bound;
		cinematic_screen_effect_globals->convolution_time[0] = rasterizer_screen_effects_time();
		cinematic_screen_effect_globals->convolution_time[1] = cinematic_screen_effect_globals->convolution_time[0] + convolution_time;
	}

	return;
}

void rasterizer_screen_effect_set_filter(
	real filter_light_enhancement_intensity_lower_bound,
	real filter_light_enhancement_intensity_upper_bound,
	real filter_desaturation_intensity_lower_bound,
	real filter_desaturation_intensity_upper_bound,
	boolean filter_desaturation_is_additive,
	real filter_time)
{
	if (cinematic_screen_effect_globals)
	{
		cinematic_screen_effect_globals->parameters.video_on = FALSE;
		cinematic_screen_effect_globals->parameters.video_overbright_mode = _rasterizer_screen_effect_video_overbright_mode_none;
		cinematic_screen_effect_globals->parameters.video_scanline_map = NULL;
		cinematic_screen_effect_globals->parameters.video_noise_intensity = 0.f;
		cinematic_screen_effect_globals->parameters.video_noise_map_scale = 0.f;
		cinematic_screen_effect_globals->parameters.video_noise_map = NULL;
		cinematic_screen_effect_globals->filter_light_enhancement_intensity[0] = filter_light_enhancement_intensity_lower_bound;
		cinematic_screen_effect_globals->filter_light_enhancement_intensity[1] = filter_light_enhancement_intensity_upper_bound;
		cinematic_screen_effect_globals->filter_desaturation_intensity[0] = filter_desaturation_intensity_lower_bound;
		cinematic_screen_effect_globals->filter_desaturation_intensity[1] = filter_desaturation_intensity_upper_bound;
		cinematic_screen_effect_globals->filter_time[0] = rasterizer_screen_effects_time();
		cinematic_screen_effect_globals->filter_time[1] = cinematic_screen_effect_globals->filter_time[0] + filter_time;
		cinematic_screen_effect_globals->parameters.filter_desaturation_is_additive = filter_desaturation_is_additive;
		cinematic_screen_effect_globals->parameters.filter_light_enhancement_uses_convolution_mask = FALSE;
		cinematic_screen_effect_globals->parameters.filter_desaturation_uses_convolution_mask = FALSE;
	}

	return;
}

void rasterizer_screen_effect_set_filter_desaturation_tint(
	real red,
	real green,
	real blue)
{
	if (cinematic_screen_effect_globals)
	{
		cinematic_screen_effect_globals->parameters.filter_desaturation_tint.red = red;
		cinematic_screen_effect_globals->parameters.filter_desaturation_tint.green = green;
		cinematic_screen_effect_globals->parameters.filter_desaturation_tint.blue = blue;
	}

	return;
}

void rasterizer_screen_effect_set_video(
	short video_overbright_mode,
	real video_noise_intensity)
{
	if (cinematic_screen_effect_globals)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_cinematics.c", 225, global_rasterizer_data);

		if (global_rasterizer_data->screen_effect_video_scanline_map.index!=NONE && global_rasterizer_data->screen_effect_video_noise_map.index!=NONE)
		{
			memset(&cinematic_screen_effect_globals->parameters, 0, sizeof(cinematic_screen_effect_globals->parameters));
			cinematic_screen_effect_globals->convolution_radius[0] = 0.f;
			cinematic_screen_effect_globals->convolution_radius[1] = 0.f;
			cinematic_screen_effect_globals->convolution_time[0] = 0.f;
			cinematic_screen_effect_globals->convolution_time[1] = 0.f;
			cinematic_screen_effect_globals->filter_light_enhancement_intensity[0] = 0.f;
			cinematic_screen_effect_globals->filter_light_enhancement_intensity[1] = 0.f;
			cinematic_screen_effect_globals->filter_desaturation_intensity[0] = 0.f;
			cinematic_screen_effect_globals->filter_desaturation_intensity[1] = 0.f;
			cinematic_screen_effect_globals->filter_time[0] = 0.f;
			cinematic_screen_effect_globals->filter_time[1] = 0.f;
			cinematic_screen_effect_globals->parameters.video_on = TRUE;
			cinematic_screen_effect_globals->parameters.video_overbright_mode = video_overbright_mode;
			cinematic_screen_effect_globals->parameters.video_scanline_map = TAG_BLOCK_GET_ELEMENT(&bitmap_group_get(global_rasterizer_data->screen_effect_video_scanline_map.index)->bitmaps, 0, struct bitmap_data);
			cinematic_screen_effect_globals->parameters.video_noise_intensity = video_noise_intensity;
			cinematic_screen_effect_globals->parameters.video_noise_map_scale = 1.f;
			cinematic_screen_effect_globals->parameters.video_noise_map = TAG_BLOCK_GET_ELEMENT(&bitmap_group_get(global_rasterizer_data->screen_effect_video_noise_map.index)->bitmaps, 0, struct bitmap_data);
		}
		else
		{
			error(_error_silent, "### ERROR cinematics failed to set video mode; global bitmaps are not set");
		}
	}

	return;
}

void rasterizer_screen_effect_stop(
	void)
{
	if (cinematic_screen_effect_globals)
	{
		cinematic_screen_effect_globals->has_control = FALSE;
	}

	return;
}

struct rasterizer_screen_effect_parameters const *rasterizer_screen_effect_get_cinematic_parameters(
	struct rasterizer_screen_effect_parameters const *parameters)
{
	if (cinematic_screen_effect_globals && cinematic_screen_effect_globals->has_control)
	{
		real convolution_t = cinematic_screen_effect_globals->convolution_time[1]!=cinematic_screen_effect_globals->convolution_time[0] ?
			PIN((rasterizer_screen_effects_time()-cinematic_screen_effect_globals->convolution_time[0])/(cinematic_screen_effect_globals->convolution_time[1]-cinematic_screen_effect_globals->convolution_time[0]), 0.f, 1.f) :
			1.f;
		real filter_t = cinematic_screen_effect_globals->filter_time[1]!=cinematic_screen_effect_globals->filter_time[0] ?
			PIN((rasterizer_screen_effects_time()-cinematic_screen_effect_globals->filter_time[0])/(cinematic_screen_effect_globals->filter_time[1]-cinematic_screen_effect_globals->filter_time[0]), 0.f, 1.f) :
			1.f;

		scalars_interpolate(cinematic_screen_effect_globals->convolution_radius[0], cinematic_screen_effect_globals->convolution_radius[1], convolution_t, &cinematic_screen_effect_globals->parameters.convolution_radius);
		scalars_interpolate_and_clamp_0_to_1(cinematic_screen_effect_globals->filter_light_enhancement_intensity[0], cinematic_screen_effect_globals->filter_light_enhancement_intensity[1], filter_t, &cinematic_screen_effect_globals->parameters.filter_light_enhancement_intensity);
		scalars_interpolate_and_clamp_0_to_1(cinematic_screen_effect_globals->filter_desaturation_intensity[0], cinematic_screen_effect_globals->filter_desaturation_intensity[1], filter_t, &cinematic_screen_effect_globals->parameters.filter_desaturation_intensity);

		if (!memcmp(&cinematic_screen_effect_globals->parameters.filter_desaturation_tint, global_real_rgb_black, sizeof(real_rgb_color)))
		{
			cinematic_screen_effect_globals->parameters.filter_desaturation_tint = *global_real_rgb_green;
		}

		if (cinematic_screen_effect_globals->parameters.convolution_radius<=_real_epsilon)
		{
			cinematic_screen_effect_globals->parameters.convolution_radius = 0.f;
			cinematic_screen_effect_globals->parameters.convolution_type = _rasterizer_screen_effect_convolution_type_none;
			cinematic_screen_effect_globals->parameters.convolution_extra_passes = 0;
		}
		else
		{
			match_vassert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_cinematics.c", 336, main_get_window_count()<=1, "### FATAL_ERROR screen effects can't use convolution when main_get_window_count>1\r\nmaybe you forgot to turn off the cinematic screen effect?");
		}

		if (cinematic_screen_effect_globals->parameters.filter_light_enhancement_intensity<=_real_epsilon &&
			cinematic_screen_effect_globals->parameters.filter_desaturation_intensity<=_real_epsilon &&
			filter_t>=1.f)
		{
			cinematic_screen_effect_globals->parameters.filter_light_enhancement_intensity = 0.f;
			cinematic_screen_effect_globals->parameters.filter_desaturation_intensity = 0.f;
		}

		parameters = &cinematic_screen_effect_globals->parameters;
	}

	return parameters;
}

void rasterizer_set_near_clip_distance(
	real near_clip_distance)
{
	if (cinematic_screen_effect_globals)
	{
		cinematic_screen_effect_globals->near_clip_distance = near_clip_distance;
	}

	return;
}

real rasterizer_get_near_clip_distance(
	void)
{
	real near_clip_distance = rasterizer_global_defaults.z_near;

	if (cinematic_screen_effect_globals && cinematic_screen_effect_globals->near_clip_distance>0.f)
	{
		near_clip_distance = cinematic_screen_effect_globals->near_clip_distance;
	}

	return near_clip_distance;
}
