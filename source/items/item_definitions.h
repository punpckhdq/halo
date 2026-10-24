/*
ITEM_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __ITEM_DEFINITIONS_H
#define __ITEM_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "object_definitions.h"

/* ---------- constants */

enum
{
	ITEM_DEFINITION_TAG = 'item',
	ITEM_DEFINITION_VERSION = 2,
	ITEM_COLLECTION_DEFINITION_TAG = 'itmc',
	ITEM_COLLECTION_DEFINITION_VERSION = 0,
};

enum
{
	_item_always_maintains_z_up_bit = 0,
	_item_destroyed_by_explosions_bit,
	_item_antigravity_bit,
	NUMBER_OF_ITEM_DEFINITION_FLAGS,
};

/* ---------- macros */

#define item_definition_get(index) ((struct item_definition *)tag_get(ITEM_DEFINITION_TAG, index))
#define item_collection_definition_get(index) ((struct item_collection_definition *)tag_get(ITEM_COLLECTION_DEFINITION_TAG, index))

/* ---------- structures */

struct _item_definition
{
	unsigned long flags;
	short hud_message_index;
	short inventory_sort_order;
	real scale;
	short hud_message_value_scale;
	short padd;
	long unused[4];
	short function_modes[4];
	long inventory_unused[41];
	struct tag_reference material_effects;
	struct tag_reference collision_sound;
	long unused2[30];
	real detonation_delay_timer_lower_bound;
	real detonation_delay_timer_upper_bound;
	struct tag_reference detonating_effect;
	struct tag_reference detonation_effect;
};

struct item_definition
{
	struct _object_definition object;
	struct _item_definition item;
};

struct item_permutation_definition
{
	long unused1[8];
	real weight;
	struct tag_reference item;
	long unused2[8];
};

struct item_collection_definition
{
	struct tag_block permutations;
	short spawn_time;
	short pad;
	long unused[19];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __ITEM_DEFINITIONS_H
