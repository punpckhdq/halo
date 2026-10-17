/*
WEATHER_PARTICLE_SYSTEMS.H

header included in hcex build.
*/

#ifndef __WEATHER_PARTICLE_SYSTEMS_H
#define __WEATHER_PARTICLE_SYSTEMS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/WEATHER_PARTICLE_SYSTEMS.C */

void weather_particle_systems_initialize(void);
void weather_particle_systems_initialize_for_new_map(void);
void weather_particle_system_new(short system_index, long definition_index, real scale);
void weather_particle_system_delete(short system_index);
void weather_particle_systems_dispose_from_old_map(void);
void weather_particle_systems_dispose(void);
void weather_particle_systems_render(void);

/* ---------- globals */

extern boolean weather;
extern struct data_array *weather_particle_data;

/* ---------- public code */

#endif // __WEATHER_PARTICLE_SYSTEMS_H
