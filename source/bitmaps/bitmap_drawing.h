/*
BITMAP_DRAWING.H

header included in hcex build.
*/

#ifndef __BITMAP_DRAWING_H
#define __BITMAP_DRAWING_H
#pragma once

/* ---------- constants */

enum
{
	_bitmap_line_step_pixel = 0, /* fake name */
	_bitmap_line_step_vertical, /* fake name */
	_bitmap_line_step_horizontal, /* fake name */
	NUMBER_OF_BITMAP_LINE_STEP_MODES
};

enum
{
	_bitmap_copy_blend_bit = 0, /* fake name */
	_bitmap_copy_modulate_bit, /* fake name */
	NUMBER_OF_BITMAP_COPY_FLAGS
};

/* ---------- macros */

/* ---------- structures */

struct bitmap_line /* fake name */
{
	short two_dx;
	short two_dy;
	short x_step;
	short y_step;
	short dx;
	short dy;
	short error;
	point2d point;
	point2d end_point;
};

/* ---------- prototypes/BITMAP_DRAWING.C */

void bitmap_initialize_line(struct bitmap_line *line, point2d const *p0, point2d const *p1);
boolean bitmap_step_line(struct bitmap_line *line, short step_mode);
void bitmap_fill_rectangle(struct bitmap_data *destination, pixel32 color, rectangle2d const *rectangle, rectangle2d const *clip_rectangle);
void bitmap_copy(struct bitmap_data *destination, point2d const *destination_point, rectangle2d const *destination_clip_rectangle, struct bitmap_data const *source, rectangle2d const *source_rectangle, pixel32 color, short flags);
void bitmap_tile_and_bevel_rectangle(struct bitmap_data *destination, long bitmap_group_index, short sequence_index, rectangle2d const *rectangle, rectangle2d const *clip_rectangle, pixel32 color, long flags);
void bitmap_draw_line(struct bitmap_data *destination, pixel32 color, rectangle2d const *clip_rectangle, real_point2d const *p0, real_point2d const *p1);
void bitmap_frame_rectangle(struct bitmap_data *destination, pixel32 color, real_rectangle2d const *bounds, rectangle2d const *clip_rectangle);

/* ---------- globals */

/* ---------- public code */

#endif // __BITMAP_DRAWING_H
