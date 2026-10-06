/*
PERIODIC_FUNCTIONS.H

header included in hcex build.
*/

#ifndef __PERIODIC_FUNCTIONS_H
#define __PERIODIC_FUNCTIONS_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	_periodic_function_one = 0,
	_periodic_function_zero,
	_periodic_function_cosine,
	_periodic_function_cosine_with_random_period,
	_periodic_function_diagonal_wave,
	_periodic_function_diagonal_wave_with_random_period,
	_periodic_function_slide,
	_periodic_function_slide_with_random_period,
	_periodic_function_noise,
	_periodic_function_jitter,
	_periodic_function_wander,
	_periodic_function_spark,
	NUMBER_OF_PERIODIC_FUNCTIONS,
};

enum
{
	_transition_function_linear = 0,
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
