/*
CONTRAILS.H

header included in hcex build.
*/

#ifndef __CONTRAILS_H
#define __CONTRAILS_H
#pragma once

/* ---------- constants */

enum
{
	MAXIMUM_CONTRAILS_PER_MAP = 256,
	MAXIMUM_CONTRAIL_POINTS_PER_MAP = 1024,
	MAXIMUM_INSTANCES_PER_CONTRAIL = 4
};

enum
{
	_contrail_active_bit = 0,
	NUMBER_OF_CONTRAIL_FLAGS
};

enum
{
	_contrail_point_new_bit = 0,
	_contrail_point_transitioning_bit,
	_contrail_point_living_dead_bit,
	NUMBER_OF_CONTRAIL_POINT_FLAGS
};

/* ---------- macros */

#define contrail_get(index) ((struct contrail_datum *)datum_get(contrail_data, (index)))
#define contrail_point_get(index) ((struct contrail_point_datum *)datum_get(contrail_point_data, (index)))

/* ---------- structures */

struct contrail_datum
{
	short identifier;
	unsigned short flags;
	long definition_index;
	long object_index;
	short attachment_index;
	short density_function_index;
	real density;
	short sequence_index;
	short frame_index;
	real texture_offset_u;
	real texture_offset_v;
	real time_until_point;
	real frame_time;
	real expired_dt;
	short contrail_point_counts[MAXIMUM_INSTANCES_PER_CONTRAIL];
	long first_contrail_point_indices[MAXIMUM_INSTANCES_PER_CONTRAIL];
};

struct contrail_point_datum
{
	short identifier;
	unsigned char flags;
	char state_index;
	real time;
	real delta;
	real density;
	real width;
	struct location location;
	real_point3d position;
	real_vector3d velocity;
	long next_contrail_point_index;
};

/* ---------- prototypes/CONTRAILS.C */

void contrails_initialize(void);
void contrails_initialize_for_new_map(void);
void contrails_dispose_from_old_map(void);
void contrails_dispose(void);
long contrail_new(long definition_index, long object_index, short attachment_index);
void contrail_owner_collision(long contrail_index, boolean object_dying, real dt);
void contrail_delete(long contrail_index);
void contrails_update(real dt);
void contrails_disconnect_from_structure_bsp(void);
void contrails_reconnect_to_structure_bsp(void);

/* ---------- globals */

extern struct data_array *contrail_data;
extern struct data_array *contrail_point_data;

/* ---------- public code */

__inline real contrail_scale_value(
	real density,
	real value,
	unsigned long scale_flags,
	short bit_index)
{
	real scaled_value = value;

	if (TEST_FLAG(scale_flags, bit_index))
	{
		scaled_value *= density;
	}

	return scaled_value;
}

__inline real contrail_scale_random_value(
	real density,
	real lower_bound,
	real upper_bound,
	unsigned long scale_flags,
	short first_bit_index)
{
	real lower = contrail_scale_value(density, lower_bound, scale_flags, first_bit_index);
	real delta = contrail_scale_value(density, upper_bound - lower_bound, scale_flags, first_bit_index + 1);

	return real_local_random_range(0.f, delta) + lower;
}

#endif // __CONTRAILS_H
