/*
WEATHER_PARTICLE_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __WEATHER_PARTICLE_DEFINITIONS_H
#define __WEATHER_PARTICLE_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "shader_definitions.h"

/* ---------- constants */

enum
{
	WEATHER_PARTICLE_SYSTEM_DEFINITION_TAG = 'rain',
	WEATHER_PARTICLE_SYSTEM_DEFINITION_VERSION = 1,
};

enum
{
	MAXIMUM_NUMBER_OF_WEATHER_PARTICLE_TYPES = 8,
};

enum
{
	_weather_particle_type_interpolate_colors_in_hsv_bit = 0,
	_weather_particle_type_along_long_hue_path_bit,
	_weather_particle_type_random_rotation_bit,
	NUMBER_OF_WEATHER_PARTICLE_TYPE_FLAGS
};

enum
{
	_weather_particle_direction_source_velocity = 0,
	_weather_particle_direction_source_acceleration,
	NUMBER_OF_WEATHER_PARTICLE_DIRECTION_SOURCE_TYPES
};

/* ---------- macros */

#define weather_particle_system_definition_get(index) ((struct weather_particle_system_definition *)tag_get(WEATHER_PARTICLE_SYSTEM_DEFINITION_TAG, (index)))	/* fake name */

/* ---------- structures */

struct weather_particle_type_definition
{
	char name[TAG_STRING_LENGTH+1];
	unsigned long flags;
	real distance_fadein_start;
	real distance_fadein_end;
	real distance_fadeout_start;
	real distance_fadeout_end;
	real height_fadein_start;
	real height_fadein_end;
	real height_fadeout_start;
	real height_fadeout_end;
	long unused[24];
	real particle_count_lower_bound;
	real particle_count_upper_bound;
	struct tag_reference physics;	// point_physics_definition
	long unused2[4];
	real acceleration_lower_bound;
	real acceleration_upper_bound;
	real acceleration_turning_rate;
	real acceleration_magnitude_rate;
	long unused3[8];
	real radius_lower_bound;
	real radius_upper_bound;
	real animation_rate_lower_bound;
	real animation_rate_upper_bound;
	real rotation_rate_lower_bound;
	real rotation_rate_upper_bound;
	long unused4[8];
	real_argb_color color_lower_bound;
	real_argb_color color_upper_bound;
	real runtime_oo_width;
	long unused5[15];
	struct tag_reference bitmap;	// bitmap_group
	short direction_render_type;
	short direction_source;
	struct shader_effect shader;
};

struct weather_particle_system_definition
{
	unsigned long flags;
	long unused[8];
	struct tag_block particle_types;	// weather_particle_type_definition
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __WEATHER_PARTICLE_DEFINITIONS_H
