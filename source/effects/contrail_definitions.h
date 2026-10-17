/*
CONTRAIL_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __CONTRAIL_DEFINITIONS_H
#define __CONTRAIL_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "shader_definitions.h"

/* ---------- constants */

enum
{
	CONTRAIL_DEFINITION_TAG = 'cont',
	CONTRAIL_DEFINITION_VERSION = 3,
	MAXIMUM_CONTRAIL_POINT_STATES_PER_CONTRAIL = 16,
};

enum
{
	_contrail_first_point_unfaded_bit = 0,
	_contrail_last_point_unfaded_bit,
	_contrail_points_start_on_media_bit,
	_contrail_points_start_on_ground_bit,
	_contrail_points_pinned_to_media_bit,
	_contrail_points_pinned_to_ground_bit,
	_contrail_fades_slowly_bit,
	NUMBER_OF_CONTRAIL_DEFINITION_FLAGS
};

enum
{
	_contrail_point_generation_rate_bit = 0,
	_contrail_point_velocity_bit,
	_contrail_point_velocity_delta_bit,
	_contrail_point_velocity_cone_angle_bit,
	_contrail_point_velocity_fraction_bit,
	_contrail_animation_rate_scales_bit,
	_contrail_texture_repeats_u_bit,
	_contrail_texture_repeats_v_bit,
	_contrail_texture_animation_u_bit,
	_contrail_texture_animation_v_bit,
	NUMBER_OF_CONTRAIL_DEFINITION_SCALEABLE_VALUES
};

enum
{
	_contrail_state_duration_bit = 0,
	_contrail_state_duration_delta_bit,
	_contrail_state_transition_duration_bit,
	_contrail_state_transition_duration_delta_bit,
	_contrail_state_width_bit,
	_contrail_state_color_bit,
	NUMBER_OF_CONTRAIL_POINT_STATE_SCALEABLE_VALUES
};

enum
{
	_contrail_render_type_vertical = 0,
	_contrail_render_type_horizontal,
	_contrail_render_type_media,
	_contrail_render_type_ground,
	_contrail_render_type_viewer,
	_contrail_render_type_linked,
	NUMBER_OF_CONTRAIL_DEFINITION_RENDER_TYPES
};

/* ---------- macros */

#define contrail_definition_get(index) ((struct contrail_definition *)tag_get(CONTRAIL_DEFINITION_TAG, (index)))

/* ---------- structures */

struct contrail_point_state
{
	real duration_lower_bound;
	real duration_upper_bound;
	real transition_duration_lower_bound;
	real transition_duration_upper_bound;
	struct tag_reference physics;
	long unused0[8];
	real width;
	real_argb_color color_lower_bound;
	real_argb_color color_upper_bound;
	unsigned long scale_flags;
};

struct contrail_definition
{
	unsigned short flags;
	unsigned short scale_flags;
	real point_generation_rate;
	real point_velocity_lower_bound;
	real point_velocity_upper_bound;
	real point_velocity_cone_angle;
	real point_inherited_velocity_fraction;
	short render_type;
	unsigned short pad;
	real texture_repeats_u;
	real texture_repeats_v;
	real texture_animation_u;
	real texture_animation_v;
	real frames_per_second;
	struct tag_reference bitmap;
	short first_sequence_index;
	short sequence_count;
	long unused1[16];
	struct shader_effect shader;
	struct tag_block states;			// contrail_point_state
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __CONTRAIL_DEFINITIONS_H
