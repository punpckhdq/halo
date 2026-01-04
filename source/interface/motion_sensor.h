/*
MOTION_SENSOR.H

header included in hcex build.
*/

#ifndef __MOTION_SENSOR_H
#define __MOTION_SENSOR_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/motion_sensor.c */

void* motion_sensor_initialize(void);
void motion_sensor_dispose_from_old_map(void);
void motion_sensor_dispose(void);

/* ---------- globals */

/* ---------- public code */

#endif // __MOTION_SENSOR_H
