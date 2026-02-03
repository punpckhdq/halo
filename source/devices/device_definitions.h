/*
DEVICE_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __DEVICE_DEFINITIONS_H
#define __DEVICE_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "objects/object_definitions.h"

/* ---------- constants */

enum
{
	DEVICE_DEFINITION_TAG= 'devi',
	DEVICE_DEFINITION_VERSION= 1,
};

enum
{
	_device_position_loops_bit= 0,
	_device_position_animation_not_interpolated_bit,
	NUMBER_OF_DEVICE_DEFINITION_FLAGS,
};

enum
{
	MACHINE_DEFINITION_TAG= 'mach',
	MACHINE_DEFINITION_VERSION= 1,
};

enum
{
	_machine_is_pathfinding_obstacle_bit= 0,
	_machine_is_not_pathfinding_obstacle_when_open_bit,
	_machine_is_elevator_bit,
	NUMBER_OF_MACHINE_FLAGS,
};


/* ---------- macros */

#define device_definition_get(index) ((struct device_definition *)tag_get(DEVICE_DEFINITION_TAG, index))

#define machine_definition_get(index) ((struct machine_definition *)tag_get(MACHINE_DEFINITION_TAG, index))

/* ---------- structures */

struct _device_definition
{
	unsigned int flags;
	real power_transition_time;
	real power_acceleration_time;
	real powered_position_transition_time;
	real powered_position_acceleration_time;
	real depowered_position_transition_time;
	real depowered_position_acceleration_time;
	short function_modes[4];
	struct tag_reference positive_start_effect;
	struct tag_reference negative_start_effect;
	struct tag_reference positive_stop_effect;
	struct tag_reference negative_stop_effect;
	struct tag_reference depowered_effect;
	struct tag_reference repowered_effect;
	real delay_time;
	unsigned long delay_unused[2];
	struct tag_reference delay_effect;
	real automatic_activation_radius;
	unsigned long unused[21];
	real runtime_maximum_power_acceleration;
	real runtime_maximum_power_velocity;
	real runtime_maximum_depowered_position_acceleration;
	real runtime_maximum_depowered_position_velocity;
	real runtime_maximum_powered_position_acceleration;
	real runtime_maximum_powered_position_velocity;
	real runtime_delay_ticks;
};

struct device_definition
{
	struct _object_definition object;
	struct _device_definition device;
};

struct _machine_definition
{
	short type;
	word flags;
	real door_open_time;
	unsigned long unused1[20];
	short collision_response;
	short elevator_node_index;
	unsigned int unused2[13];
	long runtime_door_open_ticks;
};

struct machine_definition
{
	struct _object_definition object;
	struct _device_definition device;
	struct _machine_definition machine;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __DEVICE_DEFINITIONS_H
