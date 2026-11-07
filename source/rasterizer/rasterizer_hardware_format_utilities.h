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

/* ---------- prototypes/RASTERIZER_XBOX_HARDWARE_GEOMETRY.C */

boolean rasterizer_vertex_buffer_new(struct vertex_buffer *vertex_buffer, short type, long count, void *vertices, long buffer_size);
void rasterizer_vertex_buffer_delete(struct vertex_buffer *vertex_buffer);
boolean rasterizer_triangle_buffer_new(struct triangle_buffer *triangle_buffer, short type, long count, void *triangles);
void rasterizer_triangle_buffer_delete(struct triangle_buffer *triangle_buffer);

/* ---------- globals */

/* ---------- public code */

#endif // __RASTERIZER_HARDWARE_FORMAT_UTILITIES_H
