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
	SKY_DEFINITION_TAG = 'sky ' /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct sky_atmospheric_fog
{
	real_rgb_color color;
	real unused[2];
	real maximum_density;
	real z_near;
	real z_far;
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
	struct tag_block shader_functions;
	struct tag_block animations;
	struct tag_block lights;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __SKY_DEFINITIONS_H
