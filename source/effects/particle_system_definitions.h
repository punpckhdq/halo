/*
PARTICLE_SYSTEM_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __PARTICLE_SYSTEM_DEFINITIONS_H
#define __PARTICLE_SYSTEM_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "shader_definitions.h"

/* ---------- constants */

enum
{
	PARTICLE_SYSTEM_DEFINITION_TAG = 'pctl',
	PARTICLE_SYSTEM_DEFINITION_VERSION = 4,
};

enum
{
	MAXIMUM_NUMBER_OF_PARTICLE_SYSTEM_PHYSICS_CONSTANTS = 16,
	MAXIMUM_NUMBER_OF_PARTICLE_SYSTEM_TYPES = 4,
	MAXIMUM_NUMBER_OF_PARTICLE_SYSTEM_TYPE_STATES = 8,
	MAXIMUM_NUMBER_OF_PARTICLE_SYSTEM_TYPE_PARTICLE_STATES = 8,
};

enum
{
	_particle_system_type_create_default = 0,
	_particle_system_type_create_explosion,
	_particle_system_type_create_jet,
	NUMBER_OF_PARTICLE_SYSTEM_TYPE_CREATION_PHYSICS
};

enum
{
	_particle_system_type_update_default = 0,
	NUMBER_OF_PARTICLE_SYSTEM_TYPE_UPDATE_PHYSICS
};

enum
{
	_particle_system_update_default = 0,
	_particle_system_update_explosion,
	NUMBER_OF_PARTICLE_SYSTEM_UPDATE_PHYSICS
};

enum
{
	_particle_system_type_type_states_loop_bit = 0,
	_particle_system_type_type_states_loop_forward_backward_bit,
	_particle_system_type_particle_states_loop_bit,
	_particle_system_type_particle_states_loop_forward_backward_bit,
	_particle_system_type_dies_in_water_bit,
	_particle_system_type_dies_in_air_bit,
	_particle_system_type_dies_on_ground_bit,
	_particle_system_type_rotational_sprites_animate_sideways_bit,
	_particle_system_type_disabled_bit,
	_particle_system_type_tint_by_effect_color_bit,
	_particle_system_type_initial_count_scales_bit,
	_particle_system_type_minimum_count_scales_bit,
	_particle_system_type_creation_rate_scales_bit,
	_particle_system_type_scale_scales_bit,
	_particle_system_type_animation_rate_scales_bit,
	_particle_system_type_rotation_rate_scales_bit,
	_particle_system_type_nodraw_first_person_bit,
	_particle_system_type_nodraw_third_person_bit,
	NUMBER_OF_PARTICLE_SYSTEM_TYPE_FLAGS
};

enum
{
	_particle_system_complex_sprite_render_mode_simple = 0,
	_particle_system_complex_sprite_render_mode_rotational,
	NUMBER_OF_PARTICLE_SYSTEM_COMPLEX_SPRITE_RENDER_MODES
};

enum
{
	_explosion_type_definition_physics_constant_xy_spread = 0,
	_explosion_type_definition_physics_constant_z_spread,
	_explosion_type_definition_physics_constant_intensity,
	NUMBER_OF_EXPLOSION_TYPE_PHYSICS_CONSTANTS
};

enum
{
	_jet_type_definition_physics_constant_velocity = 0,
	_jet_type_definition_physics_constant_spread_fraction,
	_jet_type_definition_physics_constant_rotates_up,
	NUMBER_OF_JET_TYPE_PHYSICS_CONSTANTS
};

/* ---------- macros */

#define particle_system_definition_get(index) ((struct particle_system_definition *)tag_get(PARTICLE_SYSTEM_DEFINITION_TAG, index))

/* ---------- structures */

struct particle_system_type_particle_state_interpolated_randomized_variables
{
	real scale;
	real animation_rate;
	real rotation_rate;
	real_argb_color color;
};

struct particle_system_type_particle_state_interpolated_variables
{
	real radius;
};

struct particle_system_type_state_interpolated_variables
{
	struct particle_system_type_particle_state_interpolated_randomized_variables particle_state_randomized_multipliers;
	struct particle_system_type_particle_state_interpolated_variables particle_state_multipliers;
	real minimum_particle_count;
	real particle_creation_rate;
};

struct particle_system_type_particle_state
{
	char name[TAG_STRING_LENGTH+1];
	real duration_lower_bound;
	real duration_upper_bound;
	real transition_time_lower_bound;
	real transition_time_upper_bound;
	struct tag_reference bitmaps;
	short sequence_index;
	word pad;
	long unused0[1];
	real scale_lower_bound;
	real scale_upper_bound;
	real animation_rate_lower_bound;
	real animation_rate_upper_bound;
	real rotation_rate_lower_bound;
	real rotation_rate_upper_bound;
	real_argb_color color_lower_bound;
	real_argb_color color_upper_bound;
	struct particle_system_type_particle_state_interpolated_variables variables;
	struct tag_reference point_physics;
	long unused1[9];
	struct shader_effect shader;
	struct tag_block physics_constants;
};

struct particle_system_type_state
{
	char name[TAG_STRING_LENGTH+1];
	real duration_lower_bound;
	real duration_upper_bound;
	real transition_time_lower_bound;
	real transition_time_upper_bound;
	unsigned long flags;
	struct particle_system_type_state_interpolated_variables variables;
	long unused[21];
	short particle_creation_physics;
	short particle_update_physics;
	struct tag_block physics_constants;
};

struct particle_system_type
{
	char name[TAG_STRING_LENGTH+1];
	unsigned long flags;
	short initial_particle_count;
	word pad;
	short complex_sprite_render_mode;
	short sprite_render_mode;
	struct particle_system_type_particle_state_interpolated_variables variables;
	long unused[9];
	short initial_particle_creation_physics;
	word pad2;
	unsigned long physics_flags;
	struct tag_block physics_constants;
	struct tag_block type_states;
	struct tag_block particle_states;
};

struct particle_system_definition
{
	unsigned long flags;
	long unused[13];
	struct tag_reference system_update_point_physics;
	short system_update_physics;
	word pad;
	unsigned long system_physics_flags;
	struct tag_block physics_constants;
	struct tag_block types;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __PARTICLE_SYSTEM_DEFINITIONS_H
