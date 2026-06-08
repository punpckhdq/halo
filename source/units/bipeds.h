/*
BIPEDS.H

header included in hcex build.
*/

#ifndef __BIPEDS_H
#define __BIPEDS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/BIPEDS.C */

void biped_stop_limp_body_physics(long biped_index);

void biped_get_sight_position(long biped_index, short estimate_mode, real_point3d const *estimated_body_position, real_vector3d *desired_facing, real_vector3d const *desired_gun_offset, real_point3d *sight_position);

/* ---------- globals */

/* ---------- public code */

#endif // __BIPEDS_H
