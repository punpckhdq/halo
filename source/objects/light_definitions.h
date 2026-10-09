/*
LIGHT_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __LIGHT_DEFINITIONS_H
#define __LIGHT_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	LIGHT_DEFINITION_TAG = 'ligh',
	LIGHT_DEFINITION_VERSION = 3,
};

enum
{
	_light_dynamic_bit = 0,
	_light_no_specular_bit,
	_light_no_reflexive_bit,
	_light_supersize_in_first_person_bit,
	_light_is_first_person_flashlight_bit,
	_light_dont_fade_active_camouflage_bit,
	NUMBER_OF_LIGHT_DEFINITION_FLAGS,
};

enum
{
	LENS_FLARE_DEFINITION_TAG = 'lens',
	LENS_FLARE_DEFINITION_VERSION = 2,
};

enum
{
	_lens_flare_scale_function_none = 0,
	_lens_flare_scale_function_non_local_incident_angle,
	_lens_flare_scale_function_local_incident_angle,
	_lens_flare_scale_function_viewer_angle,
	NUMBER_OF_LENS_FLARE_SCALE_FUNCTIONS,
};

enum
{
	_lens_flare_reflection_rotate_from_center_of_screen_bit = 0,
	_lens_flare_reflection_radius_not_scaled_by_distance_bit,
	_lens_flare_reflection_radius_scaled_by_occlusion_bit,
	_lens_flare_reflection_zbuffer_bit,
	NUMBER_OF_LENS_FLARE_REFLECTION_FLAGS,
};

enum
{
	_lens_flare_reflection_animation_color_interpolate_in_hsv_bit = 0,
	_lens_flare_reflection_animation_color_interpolate_along_farthest_hue_path_bit,
	NUMBER_OF_LENS_FLARE_REFLECTION_ANIMATION_FLAGS,
};

enum
{
	_lens_flare_occlusion_offset_direction_toward_viewer = 0,
	_lens_flare_occlusion_offset_direction_marker_forward,
	_lens_flare_occlusion_offset_direction_none,
	NUMBER_OF_LENS_FLARE_OCCLUSION_OFFSET_DIRECTIONS,
};

enum
{
	_lens_flare_corona_rotation_function_none = 0,
	_lens_flare_corona_rotation_function_eye_in_light_space,
	_lens_flare_corona_rotation_function_light_in_eye_space,
	_lens_flare_corona_rotation_function_eye_to_light_in_light_space,
	_lens_flare_corona_rotation_function_eye_to_light_in_eye_space,
	NUMBER_OF_LENS_FLARE_CORONA_ROTATION_FUNCTIONS,
};

enum
{
	_lens_flare_sun_bit = 0,
	NUMBER_OF_LENS_FLARE_FLAGS,
};

/* ---------- macros */

#define point_light_definition_get(index) ((struct point_light_definition *)tag_get(LIGHT_DEFINITION_TAG, (index))) /* fake name */
#define lens_flare_definition_get(index) ((struct lens_flare_definition *)tag_get(LENS_FLARE_DEFINITION_TAG, (index))) /* fake name */

/* ---------- structures */

struct point_light_geometry_parameters
{
	real radius;
	real radius_modifier_lower_bound;
	real radius_modifier_upper_bound;
	real falloff_angle;
	real cutoff_angle;
	real lens_flare_radius;
	real runtime_cosine_falloff_angle;
	real runtime_cosine_cutoff_angle;
	real specular_radius_multiplier;
	real runtime_sine_cutoff_angle;
	long unused[2];
};

struct point_light_color_parameters
{
	unsigned long interpolation_flags;
	real_argb_color lower_bound;
	real_argb_color upper_bound;
	long unused[3];
};

struct point_light_gel_parameters
{
	struct tag_reference map;
	word pad0;
	short texture_animation_function;
	real texture_animation_rate;
	struct tag_reference secondary_map;
	word pad1;
	short yaw_function;
	real yaw_period;
	word pad2;
	short roll_function;
	real roll_period;
	word pad3;
	short pitch_function;
	real pitch_period;
	long unused[2];
};

struct point_light_lens_flare_parameters
{
	struct tag_reference reference;
	long unused[6];
};

struct point_light_radiosity_parameters
{
	real intensity;
	real_rgb_color color;
	long unused[4];
};

struct point_light_effect_parameters
{
	real duration;
	word pad;
	short falloff_function;
	real unused[2];
};

struct point_light_definition
{
	unsigned long flags;
	struct point_light_geometry_parameters geometry;
	struct point_light_color_parameters color;
	struct point_light_gel_parameters gel;
	struct point_light_lens_flare_parameters lens_flare;
	struct point_light_radiosity_parameters radiosity;
	struct point_light_effect_parameters effect;
	long unused[23];
};

struct lens_flare_reflection
{
	word flags;
	short type;
	short bitmap_index;
	word pad;
	long unused1[5];
	real offset;
	real rotation_offset;
	long unused2[1];
	real radius_lower_bounds;
	real radius_upper_bounds;
	short radius_scale_function;
	word radius_pad;
	real brightness_lower_bounds;
	real brightness_upper_bounds;
	short brightness_scale_function;
	word brightness_pad;
	real_argb_color tint_color;
	real_argb_color animation_color_lower_bound;
	real_argb_color animation_color_upper_bound;
	word animation_flags;
	short animation_function;
	real animation_period;
	real animation_phase;
	long unused3[1];
};

struct lens_flare_definition
{
	real falloff_angle;
	real cutoff_angle;
	real runtime_cosine_falloff_angle;
	real runtime_cosine_cutoff_angle;
	real occlusion_radius;
	short occlusion_offset_direction;
	word occlusion_pad;
	real near_fade_distance;
	real far_fade_distance;
	struct tag_reference primary_map;
	word flags;
	word pad;
	long unused1[19];
	short corona_rotation_function;
	word corona_rotation_pad;
	real corona_rotation_function_scale;
	long unused2[6];
	real_vector2d corona_radius_scale;
	long unused3[7];
	struct tag_block reflections;		// lens_flare_reflection
	long unused4[8];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __LIGHT_DEFINITIONS_H
