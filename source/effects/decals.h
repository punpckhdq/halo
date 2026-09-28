/*
DECALS.H

header included in hcex build.
*/

#ifndef __DECALS_H
#define __DECALS_H
#pragma once

/* ---------- headers */

#include "decal_definitions.h"

/* ---------- constants */

enum
{
	MAXIMUM_DECALS_PER_MAP = 2048,
	MAXIMUM_DECAL_VERTICES_PER_MAP = 10240
};

enum
{
	_decal_locked_bit = 0,
	_decal_permanent_bit,
	NUMBER_OF_DECAL_FLAGS
};

enum
{
	_decal_layer_primary = 0,
	_decal_layer_secondary,
	_decal_layer_light,
	_decal_layer_alpha_tested,
	_decal_layer_water,
	NUMBER_OF_DECAL_LAYERS
};

/* ---------- macros */

#define decal_get(index) ((struct decal_datum *)datum_get(global_decal_data, (index)))

/* ---------- structures */

struct decal_vertex
{
	real_point3d position;
	unsigned long texcoord;
};

struct decal_datum
{
	short identifier;
	unsigned short flags;
	short cluster_index;
	short layer;
	real_point3d position;
	long creation_time;
	char sequence_index;
	char unused___was_frames_remaining;
	char sprite_index;
	char bitmap_index;
	real lifetime;
	real decay_time;
	unsigned long color;
	byte intensity;
	byte pad;
	short quad_count;
	long definition_index;
	long prev_decal_index;
	long next_decal_index;
};

/* ---------- prototypes/DECALS.C */

void decal_delete(long decal_index);
void decals_unlock(boolean permanent);
long decal_get_first_decal_index(short cluster_index, short layer);

/* ---------- globals */

extern struct data_array *global_decal_data;

/* ---------- public code */

#endif // __DECALS_H
