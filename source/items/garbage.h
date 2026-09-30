/*
GARBAGE.H

header included in hcex build.
*/

#ifndef __GARBAGE_H
#define __GARBAGE_H
#pragma once

/* ---------- headers */

#include "items.h"

/* ---------- constants */

/* ---------- macros */

#define garbage_get(index)			((struct garbage_datum*)object_get_and_verify_type(index, _object_mask_garbage))
#define garbage_try_and_get(index)	((struct garbage_datum*)object_try_and_get_and_verify_type(index, _object_mask_garbage))

/* ---------- structures */

struct _garbage_datum
{
	short destroy_timer;
	short pad;
	long unused[5];
};

struct garbage_datum
{
	long definition_index;
	struct _object_datum object;
	struct _item_datum item;
	struct _garbage_datum garbage;
};

/* ---------- prototypes/GARBAGE.C */

boolean garbage_new(long garbage_index);
boolean garbage_update(long garbage_index);

/* ---------- globals */

/* ---------- public code */

#endif // __GARBAGE_H
