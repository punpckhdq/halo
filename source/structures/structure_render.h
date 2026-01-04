/*
STRUCTURE_RENDER.H
*/

#ifndef __STRUCTURE_RENDER_H
#define __STRUCTURE_RENDER_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct render_fog
{
	word fog_definition_flags;
	word runtime_flags;
	real_rgb_color atmospheric_color;
	real atmospheric_maximum_density;
	real atmospheric_minimum_distance;
	real atmospheric_maximum_distance;
	short planar_mode;
	real_plane3d plane;
	real_rgb_color planar_color;
	real planar_maximum_density;
	real planar_maximum_distance;
	real  planar_maximum_depth;
	const struct fog_screen *screen;
	real screen_external_intensity;
};


/* ---------- prototypes/STRUCTURE_RENDER.C */

/* ---------- globals */

/* ---------- public code */

#endif // __STRUCTURE_RENDER_H
