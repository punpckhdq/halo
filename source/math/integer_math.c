/*
INTEGER_MATH.C
*/

/* ---------- headers */

#include "cseries.h"
#include "integer_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static point2d integer_origin2d= {{ 0, 0 }};

short direction_delta_x[9]= {1, 1, -1, -1, 0, 1, 0, -1, 0};
short direction_delta_y[9]= {1, -1, -1, 1, 1, 0, -1, 0, 0};
short reversed_directions[9]= {2, 3, 0, 1, 6, 7, 4, 5, 8};

point2d *global_integer_origin2d= &integer_origin2d;

/* ---------- public code */

rectangle2d *set_rectangle2d(
	rectangle2d *rectangle,
	short x0,
	short y0,
	short x1,
	short y1)
{
	rectangle->x0= x0;
	rectangle->y0= y0;
	rectangle->x1= x1;
	rectangle->y1= y1;

	return rectangle;
}

point2d *set_point2d(
	point2d *point,
	short x,
	short y)
{
	point->n[0]= x;
	point->n[1]= y;

	return point;
}

point2d *offset_point2d(
	point2d *point,
	short dx,
	short dy)
{
	point->n[0]+= dx;
	point->n[1]+= dy;

	return point;
}

short rectangle2d_width(
	const rectangle2d *rect)
{
	return rect->x1 - rect->x0;
}

short rectangle2d_height(
	const rectangle2d *rect)
{
	return rect->y1 - rect->y0;
}

rectangle2d *inset_rectangle2d(
	rectangle2d *rectangle,
	short dx,
	short dy)
{
	rectangle->x0+= dx;
	rectangle->x1-= dx;
	rectangle->y0+= dy;
	rectangle->y1-= dy;

	return rectangle;
}

rectangle2d *offset_rectangle2d(
	rectangle2d *rectangle,
	short dx,
	short dy)
{
	rectangle->x0+= dx;
	rectangle->x1+= dx;
	rectangle->y0+= dy;
	rectangle->y1+= dy;

	return rectangle;
}

rectangle2d* adjust_rectangle2d(
	rectangle2d const *bounds,
	rectangle2d const *source,
	rectangle2d *destination,
	short mode)
{
	rectangle2d new_rectangle;
	short adjust_height, adjust_width;

	short bounds_width= rectangle2d_width(bounds);
	short bounds_height= rectangle2d_height(bounds);
	short source_width= rectangle2d_width(source);
	short source_height= rectangle2d_height(source);

	new_rectangle= *source;

	switch (mode)
	{
	case _adjust_rectangle_center:
		adjust_height= bounds->y0 + bounds_height/2 - source_height/2 - source->y0;
		adjust_width= bounds->x0 + bounds_width/2 - source_width/2 - source->x0;

		new_rectangle.x0+= adjust_width;
		new_rectangle.x1+= adjust_width;
		new_rectangle.y0+= adjust_height;
		new_rectangle.y1+= adjust_height;
		break;
	case _adjust_rectangle_alert:
		adjust_height= (bounds_height - source_height)/3 - source->y0 + bounds->y0;
		adjust_width= (bounds_width - source_width)/2 - source->x0 + bounds->x0;

		new_rectangle.x0+= adjust_width;
		new_rectangle.x1+= adjust_width;
		new_rectangle.y0+= adjust_height;
		new_rectangle.y1+= adjust_height;
		break;
	default:
		match_vwarn(
			"c:\\halo\\SOURCE\\math\\integer_math.c",
			173,
			FALSE,
			csprintf(temporary, "adjust_rectangle2d() can't handle mode #%d", mode)
		);
		break;
	}

	*destination= new_rectangle;

	return destination;
}

boolean intersect_rectangles2d(
	const rectangle2d *r1,
	const rectangle2d *r2,
	rectangle2d *intersection)
{
	boolean result= FALSE;

	rectangle2d new_rectangle;
	new_rectangle.x0= MAX(r1->x0, r2->x0);
	new_rectangle.x1= MIN(r1->x1, r2->x1);
	new_rectangle.y0= MAX(r1->y0, r2->y0);
	new_rectangle.y1= MIN(r1->y1, r2->y1);

	if (new_rectangle.x0 >= new_rectangle.x1 || new_rectangle.y0 >= new_rectangle.y1)
	{
		memset(intersection, 0, sizeof(new_rectangle));
	}
	else
	{
		*intersection= new_rectangle;
		result= TRUE;
	}

	return result;
}

rectangle2d *rectangle2d_hull_from_rectangles2d(
	rectangle2d const *r1,
	rectangle2d const *r2,
	rectangle2d *hull)
{
	hull->x0= MIN(r1->x0, r2->x0);
	hull->x1= MAX(r1->x1, r2->x1);
	hull->y0= MIN(r1->y0, r2->y0);
	hull->y1= MAX(r1->y1, r2->y1);

	return hull;
}

boolean point2d_in_rectangle2d(
	const rectangle2d *rectangle,
	const point2d *point)
{
	boolean result= FALSE;
	if (point->x>=rectangle->x0 && point->x<rectangle->x1)
	{
		if (point->y>=rectangle->y0 && point->y<rectangle->y1)
		{
			result= TRUE;
		}
	}

	return result;
}

boolean interior_rectangle2d(
	const rectangle2d *outer,
	const rectangle2d *inner)
{
	return inner->x0>=outer->x0 && inner->x1<=outer->x1 && inner->y0>=outer->y0 && inner->y1<=outer->y1;
}

boolean equal_rectangle2d(
	const rectangle2d *r1,
	const rectangle2d *r2)
{
	return r1->x0==r2->x0 && r1->x1==r2->x1 && r1->y0==r2->y0 && r1->y1==r2->y1;
}

boolean equal_point2d(
	const point2d *p1,
	const point2d *p2)
{
	return p1->x==p2->x && p1->y==p2->y;
}

short floor_log2(
	unsigned long k)
{
	unsigned long counter= k;
	long result= 0;

	if (k > 0)
	{
		for (; counter!=1; result++)
		{
			counter>>= 1;
		}
	}

	return result;
}

short ceiling_log2(
	unsigned long k)
{
	unsigned long counter;
	long result= 0;

	if (k > 0)
	{
		counter= k - 1;
		for (; counter != 1; result++)
		{
			counter >>= 1;
		}
	}

	return result + 1;
}

unsigned long floor_power2(
	word x)
{
	long result= 1;
	while (result*2<= x)
	{
		result*= 2;
	}

	return result;
}

unsigned long ceiling_power2(
	word x)
{
	long result= 1;
	while (result < x)
	{
		result<<= 1;
	}

	return result;
}

unsigned long integer_square_root(
	unsigned long k)
{
	unsigned long sum;
	unsigned long result= 0;
	unsigned long magic= 0x40000000;

	do
	{
		sum= magic + result;
		if (magic + result <= k)
		{
			k -= sum;
			result= sum + magic;
		}
		magic>>= 2;
		result>>= 1;
	}
	while (magic);
	
	if (k>result)
	{
		result++;
	}

	return result;
}

boolean bit_vector_and(
	short count,
	unsigned long const *v0,
	unsigned long const *v1,
	unsigned long *result)
{
	short i;
	boolean res= FALSE;

	match_assert("c:\\halo\\SOURCE\\math\\integer_math.c", 348, v0 && v1);

	for (i= (BIT_VECTOR_SIZE_IN_LONGS(count))-1; i>= 0; i--)
	{
		long and_result= v0[i] & v1[i];
		if (result)
		{
			result[i]= and_result;
		}
		if (and_result)
		{
			res= TRUE;
		}
	}

	return res;
}

void bit_vector_or(
	short count,
	const unsigned long *v0,
	const unsigned long *v1,
	unsigned long *result)
{
	short i;

	match_assert("c:\\halo\\SOURCE\\math\\integer_math.c", 369, v0 && v1);
	match_assert("c:\\halo\\SOURCE\\math\\integer_math.c", 370, result);

	for (i= BIT_VECTOR_SIZE_IN_LONGS(count) - 1; i>= 0; i--)
	{
		result[i]= v0[i] | v1[i];
	}

	return;
}

void bit_vector_not(
	short count,
	const unsigned long *vector,
	unsigned long *result)
{
	short i;

	match_assert("c:\\halo\\SOURCE\\math\\integer_math.c", 387, vector && result);

	for (i= BIT_VECTOR_SIZE_IN_LONGS(count) - 1; i>= 0; i--)
	{
		result[i]= ~vector[i];
	}

	return;
}

rectangle2d *scale_rectangle2d(
	const rectangle2d *bounds,
	const rectangle2d *source,
	rectangle2d *destination,
	short scale)
{
	rectangle2d new_rectangle;
	short new_height;
	short new_width;

	short bounds_width= rectangle2d_width(bounds);
	short bounds_height= rectangle2d_height(bounds);
	short source_width= rectangle2d_width(source);
	short source_height= rectangle2d_height(source);

	if (bounds_height*source_width > source_height*bounds_width)
	{
		new_width= bounds_width;
		new_height= source_height*bounds_width/source_width;
	}
	else
	{
		new_height= bounds_height;
		new_width= bounds_height*source_width/source_height;
	}

	new_width= SHORT_FIXED_TO_LONG(new_width*scale);
	new_height= SHORT_FIXED_TO_LONG(new_height*scale);
	set_rectangle2d(&new_rectangle, 0, 0, new_width, new_height);
	offset_rectangle2d(
		&new_rectangle,
		bounds->x0 + (bounds_width - new_width) / 2,
		bounds->y0 + (bounds_height - new_height) / 2
	);
	*destination= new_rectangle;

	return destination;
}

/* ---------- private code */
