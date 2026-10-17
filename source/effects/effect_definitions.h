/*
EFFECT_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __EFFECT_DEFINITIONS_H
#define __EFFECT_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	EFFECT_DEFINITION_TAG = 'effe',
	EFFECT_DEFINITION_VERSION = 4,
	MAXIMUM_EFFECT_PARTICLES_PER_EVENT = 32,
	MAXIMUM_EFFECT_PARTS_PER_EVENT = 32,
	MAXIMUM_EFFECT_LOCATIONS = 32,
	MAXIMUM_EFFECT_EVENTS = 32,
};

enum
{
	_effect_velocity_bit = 0,
	_effect_velocity_delta_bit,
	_effect_velocity_cone_bit,
	_effect_angular_velocity_bit,
	_effect_angular_velocity_delta_bit,
	NUMBER_OF_EFFECT_VELOCITY_SCALEABLE_VALUES
};

enum
{
	_effect_deleted_when_inactive_bit = 0,
	_effect_must_be_deterministic_bit,
	NUMBER_OF_EFFECT_DEFINITION_FLAGS
};

enum
{
	_effect_particle_count_bit = NUMBER_OF_EFFECT_VELOCITY_SCALEABLE_VALUES,
	_effect_particle_count_delta_bit,
	_effect_particle_distribution_radius_bit,
	_effect_particle_distribution_radius_delta_bit,
	_effect_particle_radius_bit,
	_effect_particle_radius_delta_bit,
	_effect_particle_tint_bit,
	NUMBER_OF_EFFECT_PARTICLES_SCALEABLE_VALUES
};

enum
{
	_effect_part_type_scale_bit = NUMBER_OF_EFFECT_VELOCITY_SCALEABLE_VALUES,
	NUMBER_OF_EFFECT_PART_SCALEABLE_VALUES
};

enum
{
	_effect_particle_attached_bit = 0,
	_effect_particle_random_orientation_bit,
	_effect_particle_tint_from_change_color_bit,
	_effect_particle_tint_interpolate_hsv_bit,
	_effect_particle_tint_do_it_the_hard_way_bit,
	NUMBER_OF_EFFECT_PARTICLE_DEFINITION_FLAGS
};

enum
{
	_effect_particle_distribution_function_start = 0,
	_effect_particle_distribution_function_end,
	_effect_particle_distribution_function_constant,
	_effect_particle_distribution_function_buildup,
	_effect_particle_distribution_function_falloff,
	_effect_particle_distribution_function_quadratic,
	NUMBER_OF_EFFECT_PARTICLE_DISTRIBUTION_FUNCTIONS
};

enum
{
	_effect_disposition_agnostic = 0,
	_effect_disposition_violent,
	_effect_disposition_nonviolent,
	NUMBER_OF_EFFECT_DISPOSITIONS
};

enum
{
	_effect_environment_anywhere = 0,
	_effect_environment_air,
	_effect_environment_water,
	_effect_environment_vacuum,
	NUMBER_OF_EFFECT_ENVIRONMENTS
};

enum
{
	_effect_camera_mode_independent = 0,
	_effect_camera_mode_first_person_only,
	_effect_camera_mode_third_person_only,
	_effect_camera_mode_both,
	NUMBER_OF_EFFECT_CAMERA_MODES
};

enum
{
	_effect_part_definition_world_down_bit = 0,
	NUMBER_OF_EFFECT_PART_DEFINITION_FLAGS
};

/* ---------- macros */

#define effect_definition_get(index) ((struct effect_definition *)tag_get(EFFECT_DEFINITION_TAG, index))

/* ---------- structures */

struct effect_location_definition
{
	char marker_name[TAG_STRING_LENGTH+1];
};

struct effect_part_definition
{
	short environment;
	short disposition;
	short location_index;
	word flags;
	long unused0[3];
	unsigned long runtime_tag_reference_base_class_tag;
	struct tag_reference reference;
	long unused1[6];
	real velocity_lower_bound;
	real velocity_upper_bound;
	real velocity_cone_angle;
	real angular_velocity_lower_bound;
	real angular_velocity_upper_bound;
	real radius_modifier_lower_bound;
	real radius_modifier_upper_bound;
	long unused2[1];
	unsigned long a_scales;
	unsigned long b_scales;
};

struct effect_particles_definition
{
	short environment;
	short disposition;
	short camera_mode;
	word pad;
	short location_index;
	word pad2;
	real_euler_angles2d direction;
	real_vector3d offset;
	real_vector3d runtime_direction;
	long unused1[10];
	struct tag_reference particle;
	unsigned long flags;
	short distribution_function;
	short pad3;
	short count_lower_bound;
	short count_upper_bound;
	real distribution_radius_lower_bound;
	real distribution_radius_upper_bound;
	long unused2[3];
	real velocity_lower_bound;
	real velocity_upper_bound;
	real velocity_cone_angle;
	real angular_velocity_lower_bound;
	real angular_velocity_upper_bound;
	long unused3[2];
	real radius_lower_bound;
	real radius_upper_bound;
	long unused4[2];
	real_argb_color tint_lower_bound;
	real_argb_color tint_upper_bound;
	long unused5[4];
	unsigned long a_scales;
	unsigned long b_scales;
};

struct effect_event_definition
{
	unsigned long flags;
	real skip_fraction;
	real delay_lower_bound;
	real delay_upper_bound;
	real duration_lower_bound;
	real duration_upper_bound;
	real unused[5];
	struct tag_block parts;
	struct tag_block particles;
};

struct effect_definition
{
	unsigned long flags;
	short loop_start_index;
	short loop_stop_index;
	real runtime_danger_radius;
	real unused[7];
	struct tag_block locations;
	struct tag_block events;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __EFFECT_DEFINITIONS_H
