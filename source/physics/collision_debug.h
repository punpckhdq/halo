/*
COLLISION_DEBUG.H

header included in hcex build.
*/

#ifndef __COLLISION_DEBUG_H
#define __COLLISION_DEBUG_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/COLLISION_DEBUG.C */

void collision_debug_render(void);

/* ---------- globals */

extern real_point3d collision_debug_point;
extern real_vector3d collision_debug_vector;
extern real collision_debug_length;
extern real collision_debug_width;
extern real collision_debug_height;
extern boolean collision_debug_repeat;

/* ---------- public code */

#endif // __COLLISION_DEBUG_H
