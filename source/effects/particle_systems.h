/*
PARTICLE_SYSTEMS.H

header included in hcex build.
*/

#ifndef __PARTICLE_SYSTEMS_H
#define __PARTICLE_SYSTEMS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/PARTICLE_SYSTEMS.C */

long particle_system_new_attached(long definition_index, long object_index, short attachment_index);

void particle_system_orphan(long system_index);

/* ---------- globals */

/* ---------- public code */

#endif // __PARTICLE_SYSTEMS_H
