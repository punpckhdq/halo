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

/* ---------- prototypes/MOTION_SENSOR.C */

void motion_sensor_initialize(void);
void motion_sensor_initialize_for_new_map(void);
void motion_sensor_draw_screen(short local_player_index, boolean in_multiplayer, union point2d const *point);
void motion_sensor_dispose_from_old_map(void);
void motion_sensor_dispose(void);
void motion_sensor_tick(void);

/* ---------- globals */

/* ---------- public code */

#endif // __MOTION_SENSOR_H
