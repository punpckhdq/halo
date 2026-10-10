/*
RASTERIZER_HARDWARE_FORMAT_UTILITIES.H

header included in hcex build.
*/

#ifndef __RASTERIZER_HARDWARE_FORMAT_UTILITIES_H
#define __RASTERIZER_HARDWARE_FORMAT_UTILITIES_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/RASTERIZER_XBOX_HARDWARE_BITMAPS.C */

boolean rasterizer_bitmap_new(struct bitmap_data *bitmap);
void rasterizer_bitmap_delete(struct bitmap_data *bitmap);
void rasterizer_bitmap_changed(struct bitmap_data *bitmap);

/* ---------- globals */

/* ---------- public code */

#endif // __RASTERIZER_HARDWARE_FORMAT_UTILITIES_H
