/*
TRIANGLE_STRIPS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "triangle_strips.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void triangle_strip_iterator_new(
	struct triangle_strip_iterator *iterator,
	short const *triangle_strip_vertex_indices,
	short strip_count)
{
	match_assert("c:\\halo\\SOURCE\\render\\triangle_strips.c", 22, iterator);
	match_assert("c:\\halo\\SOURCE\\render\\triangle_strips.c", 23, triangle_strip_vertex_indices);

	iterator->triangle_strip_vertex_indices = triangle_strip_vertex_indices;
	iterator->strip_count = strip_count;
	iterator->vertex_count = 0;
	iterator->signature = _valid_strip_iterator_signature;

	return;
}

boolean triangle_strip_iterator_get_triangle(
	struct triangle_strip_iterator *iterator,
	short *vertices)
{
	boolean result = TRUE;

	match_assert("c:\\halo\\SOURCE\\render\\triangle_strips.c", 39, iterator);
	match_assert("c:\\halo\\SOURCE\\render\\triangle_strips.c", 40, vertices);
	match_assert("c:\\halo\\SOURCE\\render\\triangle_strips.c", 41, iterator->signature==_valid_strip_iterator_signature);

	if (iterator->vertex_count)
	{
		if (iterator->reverse_winding)
		{
			vertices[0] = iterator->triangle_strip_vertex_indices[-1];
			vertices[1] = iterator->triangle_strip_vertex_indices[-2];
		}
		else
		{
			vertices[0] = iterator->triangle_strip_vertex_indices[-2];
			vertices[1] = iterator->triangle_strip_vertex_indices[-1];
		}

		vertices[2] = iterator->triangle_strip_vertex_indices[0];
		iterator->triangle_strip_vertex_indices++;
		iterator->reverse_winding = !iterator->reverse_winding;
		iterator->vertex_count--;
	}
	else if (iterator->strip_count)
	{
		iterator->vertex_count = *iterator->triangle_strip_vertex_indices++ - NUMBER_OF_VERTICES_PER_TRIANGLE;
		match_assert("c:\\halo\\SOURCE\\render\\triangle_strips.c", 65, iterator->vertex_count>=0);

		iterator->reverse_winding = TRUE;
		vertices[0] = *iterator->triangle_strip_vertex_indices++;
		vertices[1] = *iterator->triangle_strip_vertex_indices++;
		vertices[2] = *iterator->triangle_strip_vertex_indices++;
		iterator->strip_count--;
	}
	else
	{
		result = FALSE;
	}

	return result;
}
