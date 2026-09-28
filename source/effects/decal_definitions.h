/*
DECAL_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __DECAL_DEFINITIONS_H
#define __DECAL_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "shader_definitions.h"

/* ---------- constants */

enum
{
	DECAL_DEFINITION_TAG = 'deca'
};

/* ---------- macros */

#define decal_definition_get(index) ((struct decal_definition *)tag_get(DECAL_DEFINITION_TAG, (index)))

/* ---------- structures */

struct decal_definition
{
	unsigned short flags;
	short type;
	short layer;
	unsigned short pad1;
	struct tag_reference next_decal_in_chain;
	real radius_lower_bounds;
	real radius_upper_bounds;
	long unused1[3];
	real intensity_lower_bounds;
	real intensity_upper_bounds;
	real_rgb_color color_lower_bounds;
	real_rgb_color color_upper_bounds;
	long unused2[3];
	short animation_loop_frame_index;
	short animation_speed;
	long unused3[7];
	real lifetime_lower_bounds;
	real lifetime_upper_bounds;
	real decay_time_lower_bounds;
	real decay_time_upper_bounds;
	long unused5[3];
	struct shader_decal shader;
	real runtime_maximum_sprite_extent;
	unsigned short runtime_incremental_counter;
	unsigned short pad2;
	long unused6[2];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __DECAL_DEFINITIONS_H
