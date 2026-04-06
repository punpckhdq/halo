/*
ITEMS.H

header included in hcex build.
*/

#ifndef __ITEMS_H
#define __ITEMS_H
#pragma once

/* ---------- headers */

#include "objects/objects.h"

/* ---------- constants */

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

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __ITEMS_H
