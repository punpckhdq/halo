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
	long definition_index; // 0x0
	long object_index; // 0x4
	short node_index; // 0x8
	short local_player_index; // 0xA
	unsigned char attached_to_local_player; // 0xC
	unsigned char dont_draw_first_person; // 0xD
	unsigned char dont_draw_third_person; // 0xE
	real_point3d position; // 0x10
	real_vector3d direction; // 0x1C
	real_vector3d velocity; // 0x28
	real_vector3d initial_impulse; // 0x34
	real rotation; // 0x40
	real angular_velocity; // 0x44
	real radius; // 0x48
	real_argb_color color; // 0x4C
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __PARTICLES_H
