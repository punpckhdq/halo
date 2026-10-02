/*
CONNECTED_GEOMETRY.C

*/

/* ---------- headers */

#include "cseries.h"
#include "connected_geometry.h"
#include "error_geometry.h"
#include "intermediate_geometry.h"

/* ---------- constants */

#define CONNECTED_GEOMETRY_POINT_EPSILON 0.001f /* fake name */
#define CONNECTED_GEOMETRY_COPLANAR_EPSILON 0.01f /* fake name */

/* ---------- macros */

#define POINT_COORDINATES_EQUAL(a, b) (fabs((a)-(b))<CONNECTED_GEOMETRY_POINT_EPSILON) /* fake name */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void connected_geometry_new(
	struct connected_geometry *geometry)
{
	dynamic_array_new(&geometry->points, sizeof(real_point3d));
	dynamic_array_new(&geometry->edges, sizeof(struct connected_edge));
	dynamic_array_new(&geometry->triangles, sizeof(struct connected_triangle));

	return;
}

void connected_geometry_delete(
	struct connected_geometry *geometry)
{
	long edge_index;

	for (edge_index= 0; edge_index<geometry->edges.count; edge_index++)
	{
		struct connected_edge *edge= dynamic_array_get_element(&geometry->edges, edge_index, sizeof(struct connected_edge));

		dynamic_array_delete(&edge->triangle_indices);
	}

	dynamic_array_delete(&geometry->points);
	dynamic_array_delete(&geometry->edges);
	dynamic_array_delete(&geometry->triangles);

	return;
}

void connected_geometry_group_recursive(
	struct connected_geometry *geometry,
	boolean (*group_test)(void *data, struct connected_geometry *geometry, struct connected_triangle *triangle, long group_index),
	void *data,
	long group_index,
	long triangle_index)
{
	struct connected_triangle *triangle= dynamic_array_get_element(&geometry->triangles, triangle_index, sizeof(struct connected_triangle));

	if (triangle->group_index==NONE)
	{
		if (!group_test || group_test(data, geometry, triangle, group_index))
		{
			short edge_index;

			triangle->group_index= group_index;
			for (edge_index= 0; edge_index<NUMBER_OF_EDGES_PER_TRIANGLE; edge_index++)
			{
				if (triangle->edge_designators[edge_index]!=NONE)
				{
					struct connected_edge *edge= dynamic_array_get_element(&geometry->edges, triangle->edge_designators[edge_index]&LONG_MAX, sizeof(struct connected_edge));
					short edge_triangle_index;

					for (edge_triangle_index= 0; edge_triangle_index<edge->triangle_indices.count; edge_triangle_index++)
					{
						connected_geometry_group_recursive(geometry, group_test, data, group_index,
							*(long *)dynamic_array_get_element(&edge->triangle_indices, edge_triangle_index, sizeof(long)));
					}
				}
			}
		}
	}

	return;
}

static long connected_geometry_find_or_add_vertex(
	struct connected_geometry *geometry,
	real_point3d const *point)
{
	long point_index;

	for (point_index= 0; point_index<geometry->points.count; point_index++)
	{
		if (POINT_COORDINATES_EQUAL(point->x, ((real_point3d *)dynamic_array_get_element(&geometry->points, point_index, sizeof(real_point3d)))->x) &&
			POINT_COORDINATES_EQUAL(point->y, ((real_point3d *)dynamic_array_get_element(&geometry->points, point_index, sizeof(real_point3d)))->y) &&
			POINT_COORDINATES_EQUAL(point->z, ((real_point3d *)dynamic_array_get_element(&geometry->points, point_index, sizeof(real_point3d)))->z))
		{
			break;
		}
	}

	if (point_index==geometry->points.count)
	{
		point_index= dynamic_array_add_element(&geometry->points);
		if (point_index!=NONE)
		{
			*(real_point3d *)dynamic_array_get_element(&geometry->points, point_index, sizeof(real_point3d))= *point;
		}
	}

	return point_index;
}

static long connected_geometry_find_or_add_edge(
	struct connected_geometry *geometry,
	long triangle_index,
	long point_index0,
	long point_index1)
{
	long edge_index;
	boolean forward;

	for (edge_index= 0; edge_index<geometry->edges.count; edge_index++)
	{
		struct connected_edge *edge= dynamic_array_get_element(&geometry->edges, edge_index, sizeof(struct connected_edge));

		if (edge->point_indices[0]==point_index0 && edge->point_indices[1]==point_index1)
		{
			forward= TRUE;
			break;
		}
		if (edge->point_indices[0]==point_index1 && edge->point_indices[1]==point_index0)
		{
			forward= FALSE;
			break;
		}
	}

	if (edge_index==geometry->edges.count)
	{
		edge_index= dynamic_array_add_element(&geometry->edges);
		forward= TRUE;
		if (edge_index!=NONE)
		{
			struct connected_edge *edge= dynamic_array_get_element(&geometry->edges, edge_index, sizeof(struct connected_edge));

			dynamic_array_new(&edge->triangle_indices, sizeof(long));
			edge->point_indices[0]= point_index0;
			edge->point_indices[1]= point_index1;
		}
	}

	if (edge_index!=NONE)
	{
		struct connected_edge *edge= dynamic_array_get_element(&geometry->edges, edge_index, sizeof(struct connected_edge));
		long edge_triangle_index= dynamic_array_add_element(&edge->triangle_indices);

		if (edge_triangle_index==NONE)
		{
			edge_index= NONE;
		}
		else
		{
			*(long *)dynamic_array_get_element(&edge->triangle_indices, edge_triangle_index, sizeof(long))= triangle_index;
		}
	}

	return (edge_index==NONE) ? NONE : (forward ? (edge_index|LONG_MIN) : (edge_index&LONG_MAX));
}

long connected_geometry_add_triangle(
	struct connected_geometry *geometry,
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	boolean check_for_duplicates)
{
	static boolean warned= FALSE; /* fake name */
	long triangle_index= dynamic_array_add_element(&geometry->triangles);

	if (triangle_index!=NONE)
	{
		struct connected_triangle *triangle= dynamic_array_get_element(&geometry->triangles, triangle_index, sizeof(struct connected_triangle));
		long point_indices[NUMBER_OF_VERTICES_PER_TRIANGLE];
		short edge_index;

		point_indices[0]= connected_geometry_find_or_add_vertex(geometry, p0);
		point_indices[1]= connected_geometry_find_or_add_vertex(geometry, p1);
		point_indices[2]= connected_geometry_find_or_add_vertex(geometry, p2);
		if (point_indices[0]==NONE || point_indices[1]==NONE || point_indices[2]==NONE)
		{
			triangle_index= NONE;
		}

		for (edge_index= 0; edge_index<NUMBER_OF_EDGES_PER_TRIANGLE; edge_index++)
		{
			triangle->edge_designators[edge_index]= connected_geometry_find_or_add_edge(geometry, triangle_index,
				point_indices[edge_index], point_indices[(edge_index+1)%NUMBER_OF_VERTICES_PER_TRIANGLE]);
			if (triangle->edge_designators[edge_index]==NONE)
			{
				triangle_index= NONE;
			}
		}
		triangle->group_index= NONE;
		memset(triangle->unused, 0, sizeof(triangle->unused));

		if (check_for_duplicates)
		{
			long other_triangle_index;

			for (other_triangle_index= 0; other_triangle_index<geometry->triangles.count-1; other_triangle_index++)
			{
				struct connected_triangle *other_triangle= dynamic_array_get_element(&geometry->triangles, other_triangle_index, sizeof(struct connected_triangle));
				boolean duplicate= TRUE;

				for (edge_index= 0; edge_index<NUMBER_OF_EDGES_PER_TRIANGLE; edge_index++)
				{
					short other_edge_index;

					for (other_edge_index= 0;
						other_edge_index<NUMBER_OF_EDGES_PER_TRIANGLE && (other_triangle->edge_designators[other_edge_index]&LONG_MAX)!=(triangle->edge_designators[edge_index]&LONG_MAX);
						other_edge_index++);
					if (other_edge_index==NUMBER_OF_EDGES_PER_TRIANGLE)
					{
						duplicate= FALSE;
						break;
					}
				}

				if (duplicate)
				{
					error_geometry_triangle(p0, p1, p2, global_real_argb_orange);
					if (!warned)
					{
						printf("### WARNING: found duplicate triangle building connected geometry. YOU SHOULD FIX THIS. (see orange in error geometry)\r\n");
						warned= TRUE;
					}

					triangle_index= NONE;
					break;
				}
			}
		}
	}

	return triangle_index;
}

static boolean triangle_coplanar(
	void *data,
	struct connected_geometry *geometry,
	struct connected_triangle *triangle,
	long group_index)
{
	real_point3d const *point0= CONNECTED_GEOMETRY_GET_EDGE_POINT(geometry, triangle->edge_designators[0]);
	real_point3d const *point1= CONNECTED_GEOMETRY_GET_EDGE_POINT(geometry, triangle->edge_designators[1]);
	real_point3d const *point2= CONNECTED_GEOMETRY_GET_EDGE_POINT(geometry, triangle->edge_designators[2]);
	real_plane3d triangle_plane;
	boolean coplanar;

	if (fabs(plane3d_distance_to_point((real_plane3d const *)data, point0))<CONNECTED_GEOMETRY_COPLANAR_EPSILON &&
		fabs(plane3d_distance_to_point((real_plane3d const *)data, point1))<CONNECTED_GEOMETRY_COPLANAR_EPSILON &&
		fabs(plane3d_distance_to_point((real_plane3d const *)data, point2))<CONNECTED_GEOMETRY_COPLANAR_EPSILON &&
		plane3d_from_points(&triangle_plane, point0, point2, point1) &&
		dot_product3d(&triangle_plane.n, &((real_plane3d const *)data)->n)>0.f)
	{
		coplanar= TRUE;
	}
	else
	{
		coplanar= FALSE;
	}

	return coplanar;
}

long connected_geometry_add_intermediate_triangle(
	struct connected_geometry *geometry,
	struct intermediate_geometry const *intermediate_geometry,
	long triangle_index,
	boolean check_for_duplicates)
{
	struct intermediate_triangle const *triangle= dynamic_array_get_element(&intermediate_geometry->triangles, triangle_index, sizeof(struct intermediate_triangle));

	return connected_geometry_add_triangle(geometry,
		&((struct intermediate_vertex *)dynamic_array_get_element(&intermediate_geometry->vertices, triangle->vertex_indices[0], sizeof(struct intermediate_vertex)))->position,
		&((struct intermediate_vertex *)dynamic_array_get_element(&intermediate_geometry->vertices, triangle->vertex_indices[1], sizeof(struct intermediate_vertex)))->position,
		&((struct intermediate_vertex *)dynamic_array_get_element(&intermediate_geometry->vertices, triangle->vertex_indices[2], sizeof(struct intermediate_vertex)))->position,
		check_for_duplicates);
}

long connected_geometry_group_coplanar(
	struct connected_geometry *geometry)
{
	long group_count= 0;
	long triangle_index;

	for (triangle_index= 0; triangle_index<geometry->triangles.count; triangle_index++)
	{
		struct connected_triangle *triangle= dynamic_array_get_element(&geometry->triangles, triangle_index, sizeof(struct connected_triangle));
		real_plane3d plane;

		plane3d_from_points(&plane,
			CONNECTED_GEOMETRY_GET_EDGE_POINT(geometry, triangle->edge_designators[0]),
			CONNECTED_GEOMETRY_GET_EDGE_POINT(geometry, triangle->edge_designators[2]),
			CONNECTED_GEOMETRY_GET_EDGE_POINT(geometry, triangle->edge_designators[1]));
		if (triangle->group_index==NONE)
		{
			connected_geometry_group_recursive(geometry, triangle_coplanar, &plane, group_count++, triangle_index);
		}
	}

	return group_count;
}

/* ---------- private code */
