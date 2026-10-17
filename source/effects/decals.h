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
	MAXIMUM_DECAL_VERTICES_PER_MAP = 10240,
	MAXIMUM_TOTAL_VERTICES_PER_DECAL = 1024,
	MAXIMUM_DECAL_SURFACE_QUEUE_SIZE = 1024
};

enum
{
	DECAL_UNLOCK_PROBABILITY = 41,
	DECAL_LOCK_PROBABILITY = 10,
	DECAL_LOCK_LOWER_LIMIT = 256,
	DECAL_LOCK_UPPER_LIMIT = 512
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
	short texcoord_u;
	short texcoord_v;
};

struct decal_datum
{
	short identifier;
	word flags;
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

struct decal_wrap_parameters
{
	real minimum_wrap_angle;
	real minimum_skip_angle;
	real radius_exclusion_multiplier;
	boolean wrap;
};

/* ---------- prototypes/DECALS.C */

void decals_initialize(void);
void decals_initialize_for_new_map(void);
void decals_reconnect_to_structure_bsp(void);
void decals_disconnect_from_structure_bsp(void);
void decals_dispose_from_old_map(void);
void decals_dispose(void);
void decals_unlock(boolean permanent);
void decals_update(void);
void decals_delete_permanent_from_cluster(short cluster_index);
void decal_delete(long decal_index);
long decal_get_first_decal_index(short cluster_index, short layer);
void decal_new(long definition_index, real_point3d const *origin, real_vector3d const *velocity, real radius_modifier, boolean permanent, short forced_sequence_index, struct decal_editor_geometry *editor_geometry);
void decal_new_from_media_collision(void);
void decal_new_from_collision(long definition_index, struct collision_result const *collision, real_vector3d const *velocity, real radius_modifier, boolean permanent, short forced_sequence_index, struct decal_editor_geometry *editor_geometry);
void render_debug_decals(void);

/* ---------- globals */

extern struct data_array *global_decal_data;
extern boolean debug_decals;
extern boolean decals_enabled;
extern struct decal_wrap_parameters const decal_wrap_parameters[NUMBER_OF_DECAL_TYPES];

/* ---------- public code */

#endif // __DECALS_H
