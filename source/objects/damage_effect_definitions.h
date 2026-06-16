/*
DAMAGE_EFFECT_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __DAMAGE_EFFECT_DEFINITIONS_H
#define __DAMAGE_EFFECT_DEFINITIONS_H
#pragma once

#include "real_math.h"
#include "objects.h"


/* ---------- constants */

enum
{
	DAMAGE_EFFECT_DEFINITION_TAG = 'jpt!',
	DAMAGE_EFFECT_DEFINITION_VERSION = 6,
	CONTINUOUS_DAMAGE_EFFECT_DEFINITION_TAG = 'cdmg',
	CONTINUOUS_DAMAGE_EFFECT_DEFINITION_VERSION = 1,
};

/* ---------- macros */

#define damage_effect_definition_get(index) ((struct damage_effect_definition *)tag_get(DAMAGE_EFFECT_DEFINITION_TAG, index))

/* ---------- structures */

struct damage_definition
{
	short side_effect; 
	short category; 
	unsigned long flags; 
	real area_of_effect_core_radius; 
	real damage_minimum; 
	real damage_lower_bound; 
	real damage_upper_bound; 
	real vehicle_passthrough_penalty; 
	real active_camouflage_damage; 
	real stun; 
	real maximum_stun; 
	real stun_time; 
	unsigned long stun_unused[1]; 
	real instantaneous_acceleration; 
	real instantaneous_acceleration_zero_scale_factor; 
	unsigned long instantaneous_acceleration_unused[1]; 
	real material_modifiers[40]; 
};

struct continuous_camera_shake_definition
{
	real random_translation_magnitude; 
	real random_rotation_magnitude; 
	long unused_motion_scales[3]; 
	short periodic_function; 
	unsigned short pad1; 
	real periodic_period; 
	real periodic_weight; 
	real zero_scale_factor; 
	long unused[7]; 
};

struct screen_fade_definition
{
	real_rgb_color color; 
	long start_time; 
	short ticks; 
	unsigned char fading_out; 
};

struct camera_impulse_definition
{
	real temporary_duration; 
	short temporary_transition; 
	unsigned short pad; 
	real temporary_rotation; 
	real temporary_translation; 
	real temporary_jitter_lower_bound; 
	real temporary_jitter_upper_bound; 
	real temporary_zero_scale_factor; 
	unsigned long temporary_unused[1]; 
	real permanent_angle; 
	real permanent_zero_scale_factor; 
	unsigned long permanent_unused[3]; 
};

struct camera_shake_definition
{
	real duration; 
	short falloff_transition_function; 
	unsigned short pad0; 
	real random_translation_magnitude; 
	real random_rotation_magnitude; 
	long unused_motion_scales[3]; 
	short periodic_function; 
	unsigned short pad1; 
	real periodic_period; 
	real periodic_weight; 
	real zero_scale_factor; 
	long unused[7]; 
};

struct continuous_player_effect_datum
{
	real vibrate_frequencies[2]; 
	real translational_shake; 
	real rotational_shake; 
};

struct screen_flash_definition
{
	short type; 
	short priority; 
	unsigned long unused1[3]; 
	real duration; 
	short fade_function; 
	unsigned short pad; 
	unsigned long unused2[2]; 
	real max_intensity; 
	real zero_scale_factor; 
	real_argb_color screen_flash_color; 
};

struct player_effect_datum
{
	real_vector3d direction; 
	real_vector3d jitter; 
	struct screen_flash_definition screen_flash; 
	struct camera_impulse_definition camera_impulse; 
	struct camera_shake_definition camera_shake; 
	struct continuous_player_effect_datum continuous_effect; 
	short continuous_effect_timer; 
	short screen_flash_time_left; 
	short camera_impulse_time_left; 
	short camera_shake_time_left; 
	unsigned char damage_indicator_ticks[4]; 
	unsigned char flags; 
};

struct scripted_player_effect_definition
{
	real_point3d max_translation; 
	real_euler_angles3d max_rotation; 
	real max_intensity; 
	short timer; 
	short total_time; 
};

struct player_effect_globals_definition
{
	struct player_effect_datum local_player_effect_data[2]; 
	struct screen_fade_definition screen_fade; 
	struct scripted_player_effect_definition scripted_effect; 
	unsigned long global_flags; 
	long reference_time; 
};

struct continuous_vibrate_definition
{
	real frequencies[2]; 
	unsigned long unused[6]; 
};

struct continuous_damage_effect_definition
{
	real falloff_radius; 
	real cutoff_radius; 
	real cutoff_scale; 
	unsigned long area_of_effect_unused[6]; 
	struct continuous_vibrate_definition vibrate; 
	struct continuous_camera_shake_definition camera_shake; 
	unsigned long unused[40]; 
	struct damage_definition damage; 
};

struct vibrate_frequency_definition
{
	real frequency; 
	real duration; 
	short fade_function; 
	unsigned short pad; 
	unsigned long unused[2]; 
};

struct vibrate_definition
{
	struct vibrate_frequency_definition vibrate_frequencies[2]; 
	real zero_scale_factor; 
	unsigned long unused[4]; 
};

struct damage_breaking_effect_definition
{
	real forward_velocity; 
	real forward_radius; 
	real forward_exponent; 
	long forward_unused[3]; 
	real outward_velocity; 
	real outward_radius; 
	real outward_exponent; 
	long outward_unused[3]; 
};

struct damage_effect_definition
{
	real falloff_radius; 
	real cutoff_radius; 
	real cutoff_scale; 
	unsigned long flags; 
	unsigned long area_of_effect_unused[5]; 
	struct screen_flash_definition screen_flash; 
	struct vibrate_definition vibrate; 
	struct camera_impulse_definition camera_impulse; 
	struct camera_shake_definition camera_shake; 
	struct tag_reference sound; 
	unsigned long unused1[28]; 
	struct damage_breaking_effect_definition breaking_effect; 
	struct damage_definition damage; 
};

struct damage_data
{
	long definition_index; 
	unsigned long flags; 
	long owner_player_index; 
	long owner_object_index; 
	short owner_team_index; 
	struct location location; 
	real_point3d origin; 
	real_point3d epicenter; 
	real_vector3d direction; 
	real scale; 
	real multiplier; 
	real material_effect_scale; 
	short material_type; 
	struct projectile_material_response_definition const *material_response; 
};

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

#endif // __DAMAGE_EFFECT_DEFINITIONS_H
