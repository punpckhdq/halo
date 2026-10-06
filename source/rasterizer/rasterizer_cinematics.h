/*
RASTERIZER_CINEMATICS.H

header included in hcex build.
*/

#ifndef __RASTERIZER_CINEMATICS_H
#define __RASTERIZER_CINEMATICS_H
#pragma once

/* ---------- constants */

enum
{
	_rasterizer_screen_effect_convolution_type_none = 0,
	_rasterizer_screen_effect_convolution_type_blur,
	_rasterizer_screen_effect_convolution_type_warp,
	NUMBER_OF_RASTERIZER_SCREEN_EFFECT_CONVOLUTION_TYPES
};

enum
{
	_rasterizer_screen_effect_video_overbright_mode_none = 0,
	_rasterizer_screen_effect_video_overbright_mode_2x,
	_rasterizer_screen_effect_video_overbright_mode_4x,
	NUMBER_OF_RASTERIZER_SCREEN_EFFECT_VIDEO_OVERBRIGHT_MODES
};

/* ---------- macros */

/* ---------- structures */

struct rasterizer_screen_effect_parameters
{
	short convolution_extra_passes;
	short convolution_type;
	real convolution_radius;
	struct bitmap_data *convolution_mask;
	real filter_light_enhancement_intensity;
	real filter_desaturation_intensity;
	real_rgb_color filter_desaturation_tint;
	boolean filter_desaturation_is_additive;
	boolean filter_light_enhancement_uses_convolution_mask;
	boolean filter_desaturation_uses_convolution_mask;
	boolean video_on;
	short video_overbright_mode;
	struct bitmap_data *video_scanline_map;
	real video_noise_intensity;
	real video_noise_map_scale;
	struct bitmap_data *video_noise_map;
};

/* ---------- prototypes/RASTERIZER_CINEMATICS.C */

void rasterizer_screen_effects_initialize(void);
void rasterizer_screen_effects_initialize_for_new_map(void);
void rasterizer_screen_effects_dispose_from_old_map(void);
void rasterizer_screen_effects_dispose(void);
void rasterizer_script_screen_effect_set_value(short index, real value);
real rasterizer_script_screen_effect_get_value(short index);
void rasterizer_screen_effect_start(boolean clear);
void rasterizer_screen_effect_set_convolution(short convolution_extra_passes, short convolution_type, real convolution_radius_lower_bound, real convolution_radius_upper_bound, real convolution_time);
void rasterizer_screen_effect_set_filter(real filter_light_enhancement_intensity_lower_bound, real filter_light_enhancement_intensity_upper_bound, real filter_desaturation_intensity_lower_bound, real filter_desaturation_intensity_upper_bound, boolean filter_desaturation_is_additive, real filter_time);
void rasterizer_screen_effect_set_filter_desaturation_tint(real red, real green, real blue);
void rasterizer_screen_effect_set_video(short video_overbright_mode, real video_noise_intensity);
void rasterizer_screen_effect_stop(void);
const struct rasterizer_screen_effect_parameters *rasterizer_screen_effect_get_cinematic_parameters(const struct rasterizer_screen_effect_parameters *parameters);
void rasterizer_set_near_clip_distance(real near_clip_distance);
real rasterizer_get_near_clip_distance(void);

/* ---------- globals */

/* ---------- public code */

#endif // __RASTERIZER_CINEMATICS_H
