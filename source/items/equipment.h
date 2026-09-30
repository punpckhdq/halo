/*
EQUIPMENT.H

header included in hcex build.
*/

#ifndef __EQUIPMENT_H
#define __EQUIPMENT_H
#pragma once

/* ---------- headers */

#include "items.h"

/* ---------- constants */

/* ---------- macros */

#define equipment_get(index)			((struct equipment_datum*)object_get_and_verify_type(index, _object_mask_equipment))
#define equipment_try_and_get(index)	((struct equipment_datum*)object_try_and_get_and_verify_type(index, _object_mask_equipment))

/* ---------- structures */

struct _equipment_datum
{
	unsigned long flags;
	long ignore_object_index;
	real detonation_timer;
	real detonation_timer_delta;
	real arming_timer;
	real arming_timer_delta;
};

struct equipment_datum
{
	long definition_index;
	struct _object_datum object;
	struct _item_datum item;
	struct _equipment_datum equipment;
};

/* ---------- prototypes/EQUIPMENT.C */

void equipment_place(long equipment_index, struct scenario_equipment_datum *scenario_equipment);
void equipment_handle_pickup(long equipment_index);
void equipment_definition_handle_pickup(long equipment_definition_index);

/* ---------- globals */

/* ---------- public code */

#endif // __EQUIPMENT_H
