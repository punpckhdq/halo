/*
RASTERIZER_COMMON.H
*/

#ifndef __RASTERIZER_COMMON_H
#define __RASTERIZER_COMMON_H
#pragma once

/* ---------- prototypes/RASTERIZER_COMMON.C */

void rasterizer_initialize_for_new_map(void);
void rasterizer_dispose_from_old_map(void);
void rasterizer_frame_update(real dt);

#endif // __RASTERIZER_COMMON_H
