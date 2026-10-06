/*
RASTERIZER_SWIZZLE.H

header included in hcex build.
*/

#ifndef __RASTERIZER_SWIZZLE_H
#define __RASTERIZER_SWIZZLE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/RASTERIZER_SWIZZLE.C */

void bitmap_swizzle_vector2d(short dim_x, short dim_y, short x, short y, long *result);
void bitmap_swizzle_vector3d(short dim_x, short dim_y, short dim_z, short x, short y, short z, long *result);
void rasterizer_xbox_bitmap_swizzle2d_byte(void *dst, void const *src, short width, short height);
void rasterizer_xbox_bitmap_swizzle2d_word(void *dst, void const *src, short width, short height);
void rasterizer_xbox_bitmap_swizzle2d_long(void *dst, void const *src, short width, short height);
void rasterizer_xbox_bitmap_swizzle3d_byte(void *dst, void const *src, short width, short height, short depth);
void rasterizer_xbox_bitmap_swizzle3d_word(void *dst, void const *src, short width, short height, short depth);
void rasterizer_xbox_bitmap_swizzle3d_long(void *dst, void const *src, short width, short height, short depth);
void rasterizer_xbox_bitmap_swizzle(struct bitmap_data *bitmap);
short rasterizer_xbox_bitmap_get_max_mipmap_count(struct bitmap_data const *bitmap);
long rasterizer_xbox_bitmap_get_pixel_data_size(struct bitmap_data const *bitmap);
boolean rasterizer_xbox_bitmap_rebuild_hardware_format(struct bitmap_data *bitmap);

/* ---------- globals */

/* ---------- public code */

#endif // __RASTERIZER_SWIZZLE_H
