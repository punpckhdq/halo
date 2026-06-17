/*
GAME_SOUND.H

header included in hcex build.
*/

#ifndef __GAME_SOUND_H
#define __GAME_SOUND_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct sound_location
{
	real_point3d position;
	real_vector3d forward;
	real_vector3d translational_velocity;
	struct location game_location;
};

/* ---------- prototypes/GAME_SOUND.C */

void game_sound_clear(void);
void game_sound_restore(void);
long game_looping_sound_new(long object_index, long definition_index, char const *marker_name, short function_index);
void game_looping_sound_delete(long looping_sound_index);
long object_impulse_sound_new(
	long object_index,
	long definition_index,
	short node_index,
	real_point3d const *position,
	real_vector3d const *forward,
	real scale);
long unattached_impulse_sound_new(long definition_index, struct sound_location const *location, real scale);
long unspatialized_impulse_sound_new(long definition_index, real scale);

/* ---------- globals */

/* ---------- public code */

#endif // __GAME_SOUND_H
