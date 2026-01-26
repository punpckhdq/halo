/*
PERIODIC_FUNCTIONS.H

header included in hcex build.
*/

#ifndef __PERIODIC_FUNCTIONS_H
#define __PERIODIC_FUNCTIONS_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"

/* ---------- constants */

enum
{
	_transition_function_linear= 0,
	_transition_function_early,
	_transition_function_very_early,
	_transition_function_late,
	_transition_function_very_late,
	_transition_function_cosine,
	NUMBER_OF_TRANSITION_FUNCTIONS,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/PERIODIC_FUNCTIONS.C */

real periodic_function_evaluate(short function_type, real time);
real transition_function_evaluate(short function_type, real value);

/* ---------- globals */

/* ---------- public code */

#endif // __PERIODIC_FUNCTIONS_H
