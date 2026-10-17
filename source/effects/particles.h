/*
PARTICLES.H

header included in hcex build.
*/

#ifndef __PARTICLES_H
#define __PARTICLES_H
#pragma once

/* ---------- constants */

enum
{
	MAXIMUM_PARTICLES_PER_MAP = 1024,
};

enum
{
	_particle_animates_backwards_bit = 0,
	_particle_at_rest_bit,
	_particle_u_mirror_bit,
	_particle_v_mirror_bit,
	_particle_dont_draw_first_person_bit,
	_particle_dont_draw_third_person_bit,
	_particle_attached_to_local_player_bit,
	NUMBER_OF_PARTICLE_FLAGS
};

enum
{
	_particle_state_next_sequence_initial = 0,
	_particle_state_next_sequence_looping,
	_particle_state_still_looping,
	_particle_state_next_sequence_final,
	_particle_state_final_sequence,
	NUMBER_OF_PARTICLE_STATES
};

/* ---------- macros */

#define particle_get(index) ((struct particle_datum *)datum_get(particle_data, (index)))

/* ---------- structures */

struct new_particle_data
{
	long definition_index;
	long object_index;
	short node_index;
	short local_player_index;
	boolean attached_to_local_player;
	boolean dont_draw_first_person;
	boolean dont_draw_third_person;
	real_point3d position;
	real_vector3d direction;
	real_vector3d velocity;
	real_vector3d initial_impulse;
	real rotation;
	real angular_velocity;
	real radius;
	real_argb_color color;
};

struct particle_datum
{
	short identifier;
	word flags;
	long definition_index;
	long object_index;
	short node_index;
	byte state;
	byte local_player_index;
	long last_rendered_frame_index;
	real life_time;
	real life_span;
	real frame_time;
	real frame_span;
	short sequence_index;
	short frame_index;
	struct location location;
	real_point3d position;
	real_vector3d direction;
	real_vector3d translational_velocity;
	real rotation;
	real angular_velocity;
	real radius;
	real_argb_color color;
};

/* ---------- prototypes/PARTICLES.C */

void particles_initialize(void);
void particles_initialize_for_new_map(void);
void particles_dispose_from_old_map(void);
void particles_dispose(void);
void particle_new(struct new_particle_data const *data);
void particle_delete(long particle_index);
void particles_update(real dt);
void particles_stop_on_first_person_weapon(short local_player_index);
void particles_disconnect_from_structure_bsp(void);
void particles_reconnect_to_structure_bsp(void);
real particle_get_radius(long particle_index);

/* ---------- globals */

extern struct data_array *particle_data;

/* ---------- public code */

#endif // __PARTICLES_H
