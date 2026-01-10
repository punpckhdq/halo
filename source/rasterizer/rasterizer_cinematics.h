/*
RASTERIZER_CINEMATICS.H

header included in hcex build.
*/

#ifndef __RASTERIZER_CINEMATICS_H
#define __RASTERIZER_CINEMATICS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/RASTERIZER_CINEMATICS.C */

void rasterizer_screen_effects_initialize_for_new_map(void);
void rasterizer_screen_effects_dispose_from_old_map(void);

void rasterizer_set_near_clip_distance(real near_clip_distance);

/* ---------- globals */

/* ---------- public code */

#endif // __RASTERIZER_CINEMATICS_H
