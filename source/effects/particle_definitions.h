/*
PARTICLE_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __PARTICLE_DEFINITIONS_H
#define __PARTICLE_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "shader_definitions.h"

/* ---------- constants */

enum
{
	PARTICLE_DEFINITION_TAG = 'part',
	PARTICLE_DEFINITION_VERSION = 2,
};

enum
{
	_particle_can_animate_backwards_bit = 0,
	_particle_animation_stops_at_rest_bit,
	_particle_animation_starts_on_random_frame_bit,
	_particle_animates_at_frame_time_bit,
	_particle_dies_at_rest_bit,
	_particle_dies_on_contact_with_structure_bit,
	_particle_tint_with_diffuse_texture_bit,
	_particle_dies_on_contact_with_water_bit,
	_particle_dies_on_contact_with_air_bit,
	_particle_self_illuminated_bit,
	_particle_random_u_mirror_bit,
	_particle_random_v_mirror_bit,
	NUMBER_OF_PARTICLE_DEFINITION_FLAGS
};

/* ---------- macros */

#define particle_definition_get(index) ((struct particle_definition *)tag_get(PARTICLE_DEFINITION_TAG, index))

/* ---------- structures */

struct particle_definition
{
	unsigned long flags;
	struct tag_reference bitmap;
	struct tag_reference physics;
	struct tag_reference collision_material_effects;
	long unused0[1];
	real life_span_lower_bound;
	real life_span_upper_bound;
	real fade_in_time;
	real fade_out_time;
	struct tag_reference collision_effect;
	struct tag_reference death_effect;
	real minimum_pixels;
	long unused2[2];
	real initial_radius_modifier;
	real final_radius_modifier;
	long unused3[1];
	real frames_per_second_lower_bound;
	real frames_per_second_upper_bound;
	real frames_per_second_contact_deterioration;
	real lod_falloff;
	real lod_cutoff;
	long unused4[1];
	short first_sequence_index;
	short initial_sequence_count;
	short looping_sequence_count;
	short final_sequence_count;
	long unused5[2];
	real runtime_oo_width;
	short sprite_orientation;
	word pad;
	struct shader_effect shader;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __PARTICLE_DEFINITIONS_H
