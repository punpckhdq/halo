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

/* ---------- globals */

/* ---------- public code */

#endif // __VEHICLES_H
