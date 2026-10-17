/*
TRIANGLE_STRIPS.H

header included in hcex build.
*/

#ifndef __TRIANGLE_STRIPS_H
#define __TRIANGLE_STRIPS_H
#pragma once

/* ---------- constants */

enum
{
	_valid_strip_iterator_signature = 's',
};

/* ---------- macros */

/* ---------- structures */

struct triangle_strip_iterator
{
	short strip_count;	/* fake name */
	short vertex_count;
	short const *triangle_strip_vertex_indices;
	boolean reverse_winding;	/* fake name */
	char signature;
};

/* ---------- prototypes/TRIANGLE_STRIPS.C */

void triangle_strip_iterator_new(struct triangle_strip_iterator *iterator, short const *triangle_strip_vertex_indices, short strip_count);
boolean triangle_strip_iterator_get_triangle(struct triangle_strip_iterator *iterator, short *vertices);

/* ---------- globals */

/* ---------- public code */

#endif // __TRIANGLE_STRIPS_H
