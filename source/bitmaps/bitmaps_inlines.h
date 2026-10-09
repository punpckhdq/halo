/*
BITMAPS_INLINES.H

file has inline function assertions.
*/

#ifndef __BITMAPS_INLINES_H
#define __BITMAPS_INLINES_H
#pragma once

/* ---------- headers */

#include "bitmap_macros.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/BITMAPS_INLINES.H */

pixel32 real_rgb_color_to_pixel32(union real_rgb_color const *color);
pixel32 real_argb_color_to_pixel32(real_argb_color const *color);
pixel32 real_a_rgb_color_to_pixel32(real alpha, union real_rgb_color const *color);

/* ---------- globals */

/* ---------- public code */

#endif // __BITMAPS_INLINES_H
