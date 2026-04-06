/*
ACTORS.H

header included in hcex build.
*/

#ifndef __ACTORS_H
#define __ACTORS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/ACTORS.C */

boolean actor_get_running_blind_vector(long actor_index, union real_vector3d *run_vector);
boolean actor_has_unlimited_grenades(long actor_index);

/* ---------- globals */

/* ---------- public code */

#endif // __ACTORS_H
