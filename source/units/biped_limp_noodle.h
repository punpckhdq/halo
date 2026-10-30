/*
BIPED_LIMP_NOODLE.H

header included in hcex build.
*/

#ifndef __BIPED_LIMP_NOODLE_H
#define __BIPED_LIMP_NOODLE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/BIPED_LIMP_NOODLE.C */

boolean biped_limp_noodle_relax_nodes_onto_environment(long biped_index);
long biped_limp_noodle_get_max_relaxation_iterations(long biped_index);
void validate_real_vector3d_axes3(real_vector3d *forward, real_vector3d *left, real_vector3d *up);

/* ---------- globals */

/* ---------- public code */

#endif // __BIPED_LIMP_NOODLE_H
