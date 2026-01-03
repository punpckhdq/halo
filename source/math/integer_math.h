/*
INTEGER_MATH.H
*/

#ifndef __INTEGER_MATH_H
#define __INTEGER_MATH_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

union point2d
{
	short n[2];
	struct { short x, y; };
};
typedef union point2d point2d;

union rectangle2d
{
	short v[4];
	struct { short y0, x0, y1, x1; };
};
typedef union rectangle2d rectangle2d;

typedef unsigned long pixel32;

/* ---------- prototypes/INTEGER_MATH.C */

rectangle2d *set_rectangle2d(rectangle2d *rectangle, short x0, short y0, short x1, short y1);
point2d *set_point2d(point2d *point, short x, short y);
point2d *offset_point2d(point2d *point, short dx, short dy);
short rectangle2d_width(rectangle2d const *rectangle);
short rectangle2d_height(rectangle2d const *rectangle);
rectangle2d *scale_rectangle2d(rectangle2d const *bounds, rectangle2d const *source, rectangle2d *destination, short scale);
rectangle2d *inset_rectangle2d(rectangle2d *rectangle, short dx, short dy);
rectangle2d *offset_rectangle2d(rectangle2d *rectangle, short dx, short dy);
rectangle2d *adjust_rectangle2d(rectangle2d const *bounds, rectangle2d const *source, rectangle2d *destination, short mode);
unsigned char intersect_rectangles2d(rectangle2d const *r1, rectangle2d const *r2, rectangle2d *intersection);
rectangle2d *rectangle2d_hull_from_rectangles2d(rectangle2d const *r1, rectangle2d const *r2, rectangle2d *hull);
unsigned char point2d_in_rectangle2d(rectangle2d const *rectangle, point2d const *point);
unsigned char interior_rectangle2d(rectangle2d const *outer, rectangle2d const *inner);
unsigned char equal_rectangle2d(rectangle2d const *r1, rectangle2d const *r2);
unsigned char equal_point2d(point2d const *p1, point2d const *p2);
short floor_log2(unsigned long k);
short ceiling_log2(unsigned long k);
unsigned long floor_power2(unsigned short x);
unsigned long ceiling_power2(unsigned short x);
unsigned long integer_square_root(unsigned long k);
unsigned char bit_vector_and(short count, unsigned long const *v0, unsigned long const *v1, unsigned long *result);
void bit_vector_or(short count, unsigned long const *v0, unsigned long const *v1, unsigned long *result);
void bit_vector_not(short count, unsigned long const *vector, unsigned long *result);


/* ---------- globals */

extern short direction_delta_x[9];
extern short direction_delta_y[9];
extern short reversed_directions[9];
extern point2d *global_integer_origin2d;

/* ---------- public code */

#endif // __INTEGER_MATH_H
