/*
PHYSICS_VARIABLES.H

header included in hcex build.
*/

#ifndef __PHYSICS_VARIABLES_H
#define __PHYSICS_VARIABLES_H
#pragma once

/* ---------- headers */

#include "real_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct physics_variable_position
{
	real maximum_position;
	real minimum_position;
};

struct physics_variable_speed
{
	real maximum_forward_speed;
	real maximum_reverse_speed;
	real acceleration;
	real deceleration;
};

struct physics_variable
{
	struct physics_variable_position position;
	struct physics_variable_speed speed;
};

/* ---------- prototypes/PHYSICS_VARIABLES.C */

void physics_variable_position_update(real *position, struct physics_variable_position const *definition, boolean cyclical_position, real speed);
void physics_variable_speed_update(real *speed, struct physics_variable_speed const *definition, real magnitude);
boolean physics_variable_speed_update_seek(real *speed, struct physics_variable_speed const *definition, real desired_speed, real magnitude);
void physics_variable_update(real *position, real *speed, struct physics_variable const *definition, boolean cyclical_position, real magnitude);
boolean physics_variable_position_update_seek(real *position, struct physics_variable_position const *definition, boolean cyclical_position, real desired_position, real speed);
boolean physics_variable_update_seek(real *position, real *speed, struct physics_variable const *definition, boolean cyclical_position, real desired_position, real magnitude);

/* ---------- globals */

/* ---------- public code */

#endif // __PHYSICS_VARIABLES_H
