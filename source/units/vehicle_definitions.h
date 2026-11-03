/*
VEHICLE_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __VEHICLE_DEFINITIONS_H
#define __VEHICLE_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "unit_definitions.h"
#include "physics_variables.h"

/* ---------- constants */

enum
{
	VEHICLE_DEFINITION_TAG = 'vehi',
	VEHICLE_DEFINITION_VERSION = 1,
};

enum
{
	_vehicle_human_tank = 0,
	_vehicle_human_jeep,
	_vehicle_human_boat,
	_vehicle_human_plane,
	_vehicle_alien_scout,
	_vehicle_alien_fighter,
	_vehicle_turret,
	NUMBER_OF_VEHICLE_DEFINITION_TYPES,

	_vehicle_mask_flying = FLAG(_vehicle_human_plane) | FLAG(_vehicle_alien_fighter),
};

enum
{
	_vehicle_speed_wakes_physics_bit = 0,
	_vehicle_turn_wakes_physics_bit,
	_vehicle_driver_power_wakes_physics_bit,
	_vehicle_gunner_power_wakes_physics_bit,
	_vehicle_control_opposite_speed_sets_jump_bit,
	_vehicle_slide_wakes_physics_bit,
	_vehicle_kills_riders_at_terminal_velocity_bit,
	_vehicle_causes_collision_damage_bit,
	_vehicle_ai_weapon_cannot_rotate_bit,
	_vehicle_ai_does_not_require_driver_bit,
	_vehicle_ai_unused_bit,
	_vehicle_ai_driver_enable_bit,
	_vehicle_ai_driver_flying_bit,
	_vehicle_ai_driver_nondirectional_bit,
	_vehicle_ai_driver_hovering_bit,
	NUMBER_OF_VEHICLE_DEFINITION_FLAGS,
};

/* ---------- macros */

#define vehicle_definition_get(index) ((struct vehicle_definition *)tag_get(VEHICLE_DEFINITION_TAG, index))

/* ---------- structures */

struct _vehicle_definition
{
	unsigned long flags;
	short type;
	short pad;
	struct physics_variable_speed speed;
	struct physics_variable_position turn;
	real wheel_circumference;
	real turn_rate;
	real blur_speed;
	short function_modes[4];
	real unused0[3];
	struct physics_variable_speed slide;
	real flipping_angular_velocity_min;
	real flipping_angular_velocity_max;
	real unused1[6];
	real fixed_gun_yaw;
	real fixed_gun_pitch;
	real unused2[6];
	real ai_sideslip_distance;
	real ai_destination_radius;
	real ai_avoidance_distance;
	real ai_pathfinding_radius;
	real ai_charge_repeat_time;
	real ai_strafing_stop_range;
	real ai_oversteer_angle_lower_bound;
	real ai_oversteer_angle_upper_bound;
	real ai_steering_max_angle;
	real ai_steering_max_throttle;
	real ai_movement_max_time;
	real ai_unused;
	struct tag_reference suspension_sound;
	struct tag_reference crash_sound;
	struct tag_reference material_effects;
	struct tag_reference effect;
};

struct vehicle_definition
{
	struct _object_definition object;
	struct _unit_definition unit;
	struct _vehicle_definition vehicle;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __VEHICLE_DEFINITIONS_H
