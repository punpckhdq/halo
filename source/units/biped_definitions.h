/*
BIPED_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __BIPED_DEFINITIONS_H
#define __BIPED_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "unit_definitions.h"

/* ---------- constants */

enum
{
	BIPED_DEFINITION_TAG = 'bipd',
	BIPED_DEFINITION_VERSION = 3,
};

enum
{
	_biped_turns_without_animating_bit = 0,
	_biped_uses_player_physics_bit,
	_biped_flying_bit,
	_biped_pill_centered_at_origin_bit,
	_biped_spherical_bit,
	_biped_passes_through_bipeds_bit,
	_biped_climbs_anything_bit,
	_biped_immune_to_falling_damage_bit,
	_biped_rotate_while_airborne_bit,
	_biped_uses_limp_body_physics_bit,
	_biped_has_no_dying_airborne_bit,
	_biped_random_speed_increase_bit,
	_biped_uses_old_player_physics_bit,
	NUMBER_OF_BIPED_DEFINITION_FLAGS,
};

/* ---------- macros */

#define biped_definition_get(index) ((struct biped_definition *)tag_get(BIPED_DEFINITION_TAG, (index)))

/* ---------- structures */

struct _biped_definition
{
	real moving_turning_speed;
	unsigned long flags;
	real stationary_turning_threshold;
	real turning_unused[4];
	short function_modes[4];
	struct tag_reference melee_damage;
	real flying_bank_amount;
	real flying_bank_apply_time;
	real flying_bank_decay_time;
	real flying_pitch_amount;
	real flying_velocity;
	real flying_sidestep_velocity;
	real flying_acceleration;
	real flying_deceleration;
	real flying_angular_velocity;
	real flying_angular_acceleration;
	real flying_crouch_velocity_modifier;
	real flying_unused[2];
	real maximum_slope_angle;
	real downhill_falloff_angle;
	real downhill_cutoff_angle;
	real downhill_velocity_scale;
	real uphill_falloff_angle;
	real uphill_cutoff_angle;
	real uphill_velocity_scale;
	real velocity_unused[6];
	struct tag_reference material_effects;
	real more_unused[6];
	real jump_velocity;
	real jumping_unused[7];
	real maximum_soft_landing_time;
	real maximum_hard_landing_time;
	real minimum_soft_landing_velocity;
	real minimum_hard_landing_velocity;
	real maximum_hard_landing_velocity;
	real death_hard_landing_velocity;
	real landing_unused[5];
	real standing_camera_height;
	real crouching_camera_height;
	real crouch_transition_time;
	real camera_unused[6];
	real collision_height_standing;
	real collision_height_crouching;
	real collision_radius;
	real collision_unused[10];
	real autoaim_width;
	real unused[27];
	real runtime_cosine_stationary_turning_threshold;
	real runtime_crouch_transition_velocity;
	real runtime_minimum_normal_k;
	real runtime_downhill_k0;
	real runtime_downhill_k1;
	real runtime_uphill_k0;
	real runtime_uphill_k1;
	short runtime_pelvis_node_index;
	short runtime_head_node_index;
	struct tag_block contact_points;
};

struct biped_definition
{
	struct _object_definition object;
	struct _unit_definition unit;
	struct _biped_definition biped;
};

/* ---------- prototypes/BIPED_DEFINITIONS.C */

/* ---------- globals */

/* ---------- public code */

#endif // __BIPED_DEFINITIONS_H
