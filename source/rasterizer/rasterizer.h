/*
RASTERIZER.H

header included in hcex build.
*/

#ifndef __RASTERIZER_H
#define __RASTERIZER_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/RASTERIZER.C */

boolean rasterizer_initialize(void);
void rasterizer_dispose(void);

void rasterizer_decals_update_function_pointers(void);


/* ---------- prototypes/RASTERIZER_TEXT.C */

void rasterizer_draw_string(union rectangle2d const *bounds, union rectangle2d const *clip, union point2d *cursor_reference, short height_adjust, char const *string);

/* ---------- globals */

/* ---------- public code */

#endif // __RASTERIZER_H
