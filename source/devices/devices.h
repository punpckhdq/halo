/*
DEVICES.H

header included in hcex build.
*/

#ifndef __DEVICES_H
#define __DEVICES_H
#pragma once

/* ---------- headers */

#include "objects/objects.h"

/* ---------- constants */

enum
{
	_scenario_device_initially_open_bit= 0,
	_scenario_device_initially_off_bit,
	_scenario_device_changes_only_once_bit,
	_scenario_device_position_reversed_bit,
	_scenario_device_not_usable_bit,
	SCENARIO_DEVICE_DATUM_FLAGS,
};

enum
{
	_machine_does_not_operate_automatically_bit= 0,
	_machine_one_sided_bit,
	_machine_never_appears_locked_bit,
	_machine_opened_by_melee_attack_bit,
	NUMBER_OF_MACHINE_DATUM_FLAGS,
};

/* ---------- macros */

/* ---------- structures */

struct _device_datum
{
	unsigned long flags;
	short power_group_index;
	real power;
	real power_velocity;
	short position_group_index;
	real position;
	real position_velocity;
	short delay_ticks;
};

struct device_datum
{
	long definition_index;
	struct _object_datum object;
	struct _device_datum device;
};

struct _machine_datum
{
	unsigned long flags;
	long door_open_ticks;
	real_point3d elevator_position;
};

struct machine_datum
{
	long definition_index;
	struct _object_datum object;
	struct _device_datum device;
	struct _machine_datum machine;
};

/* ---------- prototypes/DEVICES.C */

/* ---------- globals */

/* ---------- public code */

#endif // __DEVICES_H
