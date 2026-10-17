/*
PARTICLE_SYSTEMS.H

header included in hcex build.
*/

#ifndef __PARTICLE_SYSTEMS_H
#define __PARTICLE_SYSTEMS_H
#pragma once

/* ---------- headers */

#include "particle_system_definitions.h"

/* ---------- constants */

enum
{
	MAXIMUM_NUMBER_OF_PARTICLE_SYSTEMS_PER_MAP = 64,
	MAXIMUM_NUMBER_OF_SYSTEM_PARTICLES_PER_MAP = 512,
	MAXIMUM_NUMBER_OF_PARTICLE_SYSTEM_CREATION_ATTEMPTS = 128,
};

enum
{
	_particle_system_active_bit = 0,
	_particle_system_initializing_bit,
	NUMBER_OF_PARTICLE_SYSTEM_FLAGS
};

/* ---------- macros */

#define particle_system_get(index) ((struct particle_system_datum *)datum_get(particle_systems, (index)))	/* fake name */
#define system_particle_get(index) ((struct ps_particle_datum *)datum_get(system_particles, (index)))		/* fake name */

/* ---------- structures */

struct default_system_physics_state
{
	long placeholder;
};

struct explosion_system_physics_state
{
	boolean grounded;
};

struct jet_system_physics_state
{
	long placeholder;
};

union system_physics_state
{
	struct default_system_physics_state default_state;
	struct explosion_system_physics_state explosion_state;
	struct jet_system_physics_state jet_state;
};

struct particle_type
{
	short state_index;
	short transition_state_index;
	real time_left_in_state;
	real state_length;
	struct particle_system_type_state_interpolated_variables variables;
	real fractional_particle_count;
	boolean states_moving_forward;
	byte pad;
	short particle_count;
	long first_particle_index;
};

struct particle_system_datum
{
	short identifier;
	unsigned long flags;
	long definition_index;
	long object_index;
	short attachment_index;
	short function_index;
	real scale;
	struct location location;
	real_point3d position;
	real_vector3d velocity;
	real_argb_color color;
	real_rgb_color lighting;
	union system_physics_state physics_states;
	struct particle_type types[MAXIMUM_NUMBER_OF_PARTICLE_SYSTEM_TYPES];
};

struct ps_particle_datum
{
	short identifier;
	boolean states_moving_forward;
	boolean valid;
	long next_particle_index;
	short state_index;
	short transition_state_index;
	real time_left_in_state;
	real state_length;
	struct location location;
	real_point3d position;
	real_vector3d velocity;
	real_vector3d axis;
	real rotation;
	real sprite_index;
	struct particle_system_type_particle_state_interpolated_randomized_variables randomized_variables;
	struct particle_system_type_particle_state_interpolated_randomized_variables transition_randomized_variables;
};

/* ---------- prototypes/PARTICLE_SYSTEMS.C */

void particle_systems_initialize(void);
void particle_systems_initialize_for_new_map(void);
void particle_systems_dispose_from_old_map(void);
long particle_system_new_unattached(long definition_index, real_point3d const *position, real_vector3d const *velocity, real_argb_color const *color, real scale);
long particle_system_new_attached(long definition_index, long object_index, short attachment_index);
void particle_system_orphan(long system_index);
void particle_systems_dispose(void);
void particle_systems_update(real dt);
void particle_systems_disconnect_from_structure_bsp(void);
void particle_systems_reconnect_to_structure_bsp(void);
void particle_systems_render(void);

/* ---------- globals */

extern struct data_array *particle_systems;
extern struct data_array *system_particles;

/* ---------- public code */

#endif // __PARTICLE_SYSTEMS_H
