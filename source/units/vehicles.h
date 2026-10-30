/*
VEHICLES.H

header included in hcex build.
*/

#ifndef __VEHICLES_H
#define __VEHICLES_H
#pragma once

/* ---------- headers */

#include "units.h"
#include "vehicle_definitions.h"

/* ---------- constants */

enum
{
	_vehicle_animation_steering = 0,
	_vehicle_animation_roll,
	_vehicle_animation_throttle,
	_vehicle_animation_velocity,
	_vehicle_animation_braking,
	_vehicle_animation_ground_speed,
	_vehicle_animation_occupied,
	_vehicle_animation_unoccupied,
	NUMBER_OF_VEHICLE_ANIMATIONS,
};

enum
{
	_vehicle_blurred_bit = 0,
	_vehicle_hovering_bit,
	_vehicle_control_crouch_bit,
	_vehicle_control_jump_bit,
	_vehicle_upending_bit,
	NUMBER_OF_VEHICLE_FLAGS,
};

enum
{
	_vehicle_not_upending = 0,
	_vehicle_upending_along_back,
	_vehicle_upending_along_forward,
	_vehicle_upending_along_left,
	_vehicle_upending_along_right,
	NUMBER_OF_VEHICLE_UPENDING_TYPES,
};

/* ---------- macros */

#define vehicle_get(index) ((struct vehicle_datum *)object_get_and_verify_type((index), _object_mask_vehicle))

/* ---------- structures */

struct _vehicle_datum
{
	word flags;
	short stop_time;
	byte airborne_ticks;
	byte upending_type;
	byte upending_ticks;
	byte on_ground_ticks;
	real speed;
	real slide;
	real turn;
	real wheel;
	real left_tread;
	real right_tread;
	real hover;
	real thrust;
	byte suspension[8];
	real_point3d hover_position;
	real_vector3d collision_force;
	real_vector3d collision_torque;
	unsigned long stuck_mass_point_flags;
};

struct vehicle_datum
{
	long definition_index;
	struct _object_datum object;
	struct _unit_datum unit;
	struct _vehicle_datum vehicle;
};

/* ---------- prototypes/VEHICLES.C */

void vehicles_initialize(void);
void vehicles_dispose(void);
void vehicles_initialize_for_new_map(void);
void vehicles_dispose_from_old_map(void);

boolean vehicle_new(long vehicle_index);
void vehicle_place(long vehicle_index, struct scenario_vehicle_datum *scenario_vehicle);
void vehicle_delete(long vehicle_index);
boolean vehicle_update(long vehicle_index);
void vehicle_export_function_values(long vehicle_index);
void vehicle_preprocess_node_orientations(long vehicle_index, struct real_orientation *node_orientations);
void vehicle_reset(long vehicle_index);
void vehicle_render_debug(long vehicle_index);

void vehicle_hover(long vehicle_index, boolean hover);
void vehicle_accelerate(long vehicle_index, real_vector3d const *acceleration);

boolean vehicle_causes_collision_damage(long vehicle_index);
boolean vehicle_build_update(void);
boolean vehicle_parse_update(void);
long vehicle_find_pathfinding_surface_index(long vehicle_index, real_point3d *pathfinding_point);
boolean vehicle_is_flipped(long vehicle_index);
boolean vehicle_moving_near_any_player(void);
boolean vehicle_stuck(long vehicle_index, real_vector3d *approximate_direction);

/* ---------- globals */

extern boolean debug_objects_vehicle_powered_mass_points;

/* ---------- public code */

#endif // __VEHICLES_H
