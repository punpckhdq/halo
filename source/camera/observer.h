/*
OBSERVER.H

header included in hcex build.
*/

#ifndef __OBSERVER_H
#define __OBSERVER_H
#pragma once

/* ---------- headers */

#include "objects/objects.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct observer_result
{
	real_point3d position;
	struct location location;
	real_vector3d velocity;
	real_vector3d forward;
	real_vector3d up;
	real field_of_view;
};

/* ---------- prototypes/OBSERVER.C */

void observer_initialize_for_new_map(void);

struct observer_result const *observer_get_camera(short local_player_index);

/* ---------- globals */

/* ---------- public code */

#endif // __OBSERVER_H
