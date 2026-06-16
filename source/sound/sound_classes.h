/*
SOUND_CLASSES.H

file has inline function assertions.
*/

#ifndef __SOUND_CLASSES_H
#define __SOUND_CLASSES_H
#pragma once

#include "real_math.h"
#include "objects.h"

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

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __SOUND_CLASSES_H
