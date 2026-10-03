/*
WIND_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __WIND_DEFINITIONS_H
#define __WIND_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	WIND_DEFINITION_TAG = 'wind' /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct wind_definition
{
	real velocity_lower_bound;
	real velocity_upper_bound;
	real_euler_angles2d variation_area;
	real local_variation_weight;
	real local_variation_rate;
	real damping;
	long unused[9];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __WIND_DEFINITIONS_H
