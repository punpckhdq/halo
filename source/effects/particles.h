/*
PARTICLES.H

header included in hcex build.
*/

#ifndef __PARTICLES_H
#define __PARTICLES_H
#pragma once

#include "real_math.h"

/* ---------- constants */

/* ---------- macros */

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

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __PARTICLES_H
