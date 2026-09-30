/*
EQUIPMENT_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __EQUIPMENT_DEFINITIONS_H
#define __EQUIPMENT_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "item_definitions.h"

/* ---------- constants */

enum
{
	EQUIPMENT_DEFINITION_TAG = 'eqip',
	EQUIPMENT_DEFINITION_VERSION = 2,
};

/* ---------- macros */

#define equipment_definition_get(index) ((struct equipment_definition *)tag_get(EQUIPMENT_DEFINITION_TAG, index))

/* ---------- structures */

struct _equipment_definition
{
	short powerup_type;
	short grenade_type;
	real powerup_duration;
	struct tag_reference pickup_sound;
	unsigned long unused[36];
};

struct equipment_definition
{
	struct _object_definition object;
	struct _item_definition item;
	struct _equipment_definition equipment;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __EQUIPMENT_DEFINITIONS_H
