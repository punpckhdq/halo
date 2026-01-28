/*
EFFECTS.H

header included in hcex build.
*/

#ifndef __EFFECTS_H
#define __EFFECTS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/EFFECTS.C */

long effect_new_looping(
	long definition_index,
	long object_index,
	short scale_a_function_index,
	short scale_b_function_index,
	short change_color_index);

long effect_new_from_object(
	long definition_index,
	long owner_object_index,
	long object_index,
	short force_local_player_index,
	real scale_a,
	real scale_b,
	real_rgb_color const *color,
	struct effect_vector_field const *impulse_field);

void effect_delete(long effect_index);

/* ---------- globals */

/* ---------- public code */

#endif // __EFFECTS_H
