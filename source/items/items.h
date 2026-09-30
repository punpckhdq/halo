/*
ITEMS.H

header included in hcex build.
*/

#ifndef __ITEMS_H
#define __ITEMS_H
#pragma once

/* ---------- headers */

#include "objects.h"

/* ---------- constants */

enum
{
	_item_in_unit_inventory_bit = 0,
	_item_belongs_to_player_bit,
	_item_has_nonzero_angular_velocity_bit,
	_item_on_structure_bit,
	_item_on_object_bit,
	_item_does_not_accelerate_bit,
	_item_part_of_respawn_system,
	NUMBER_OF_ITEM_DATUM_FLAGS,
};

/* ---------- macros */

#define item_get(index)			((struct item_datum*)object_get_and_verify_type(index, _object_mask_item))
#define item_try_and_get(index)	((struct item_datum*)object_try_and_get_and_verify_type(index, _object_mask_item))

/* ---------- structures */

struct _item_datum
{
	unsigned long flags;
	short detonation_ticks;
	short rested_surface_index;
	short bsp_index;
	short pad;
	long ignore_object_index;
	long last_owned_time;
	long item_on_rest_object_index;
	real_point3d item_rest_object_offset;
	real_vector3d rotation_axis;
	real rotation_sine;
	real rotation_cosine;
};

struct item_datum
{
	long definition_index;
	struct _object_datum object;
	struct _item_datum item;
};

/* ---------- prototypes/ITEMS.C */

void item_in_unit_inventory(long item_index, long owner_unit_index);
void item_accelerate(long item_index, real_vector3d const *acceleration, boolean detonates_explosives);

/* ---------- globals */

/* ---------- public code */

#endif // __ITEMS_H
