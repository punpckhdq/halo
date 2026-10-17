/*
SKY_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __SKY_DEFINITIONS_H
#define __SKY_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	SKY_TAG = 'sky ',
	SKY_VERSION = 1,
	MAXIMUM_SHADER_FUNCTIONS_PER_SKY = 8,
	MAXIMUM_ANIMATIONS_PER_SKY = 8,
	MAXIMUM_LIGHTS_PER_SKY = 8
};

/* ---------- macros */

#define sky_definition_get(index) ((struct sky *)tag_get(SKY_TAG, index)) /* fake name */

/* ---------- structures */

struct sky_atmospheric_fog
{
	real_rgb_color color;
	real unused[2];
	real maximum_density;
	real z_near;
	real z_far;
};

struct sky_shader_function
{
	word pad;
	short runtime_global_function_index;
	char global_function_name[TAG_STRING_LENGTH+1];
};

struct sky_animation
{
	short animation_index;
	word pad;
	real period;
	long unused[7];
};

struct sky_radiosity_light
{
	unsigned long flags;
	real_rgb_color color;
	real power;
	real test_distance;
	long unused;
	real_euler_angles2d direction;
	real diameter;
};

struct sky_light
{
	struct tag_reference lens_flare;	// lens_flare
	char lens_flare_marker_name[TAG_STRING_LENGTH+1];
	long unused[7];
	struct sky_radiosity_light radiosity;
};

struct sky
{
	struct tag_reference model;				// model
	struct tag_reference animation_graph;	// animation_graph
	long unused[6];
	real_rgb_color radiosity_indoor_ambient_color;
	real radiosity_indoor_ambient_power;
	real_rgb_color radiosity_outdoor_ambient_color;
	real radiosity_outdoor_ambient_power;
	struct sky_atmospheric_fog outdoor_fog;
	struct sky_atmospheric_fog indoor_fog;
	struct tag_reference indoor_fog_plane;	// fog_definition
	long unused2[1];
	struct tag_block shader_functions;		// sky_shader_function
	struct tag_block animations;			// sky_animation
	struct tag_block lights;				// sky_light
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __SKY_DEFINITIONS_H
