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

/* ---------- macros */

#define vehicle_get(index) ((struct vehicle_datum *)object_get_and_verify_type((index), _object_mask_vehicle))

/* ---------- structures */

struct vehicle_datum_network_data
{
	boolean at_rest_bit;
	real_point3d position;
	real_vector3d translational_velocity;
	real_vector3d angular_velocity;
	real_vector3d forward;
	real_vector3d up;
};

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
	boolean has_been_accelerated_since_last_incremental;
	boolean baseline_valid;
	byte baseline_index;
	byte message_index;
	struct vehicle_datum_network_data baseline;
	boolean last_network_data_valid;
	struct vehicle_datum_network_data last_network_data;
	long last_controlled_time;
	union
	{
		short vehicle_scenario_datum_index;
		short vehicle_netgame_flag_index;
	};
	real_point3d spawn_position;
};

struct vehicle_datum
{
	long definition_index;
	struct _object_datum object;
	struct _unit_datum unit;
	struct _vehicle_datum vehicle;
};

/* ---------- prototypes/VEHICLES.C */

void vehicle_hover(long vehicle_index, boolean hover);

/* ---------- globals */

/* ---------- public code */

#endif // __VEHICLES_H
