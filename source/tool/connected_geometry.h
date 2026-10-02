/*
CONNECTED_GEOMETRY.H

header included in hcex build.
*/

#ifndef __CONNECTED_GEOMETRY_H
#define __CONNECTED_GEOMETRY_H
#pragma once

/* ---------- constants */

enum
{
	MAXIMUM_TRIANGLES_PER_CONNECTED_GEOMETRY_COPLANAR_GROUP= 20000
};

/* ---------- macros */

#define CONNECTED_GEOMETRY_GET_EDGE_POINT(geometry, edge_designator) /* fake name */ \
	((real_point3d *)dynamic_array_get_element(&(geometry)->points,						\
		((struct connected_edge *)dynamic_array_get_element(&(geometry)->edges,			\
			(edge_designator)&LONG_MAX, sizeof(struct connected_edge)))->point_indices[((edge_designator)&LONG_MIN)!=0], \
		sizeof(real_point3d)))

/* ---------- structures */

struct connected_edge // [fake name?]
{
	struct dynamic_array triangle_indices;
	long point_indices[NUMBER_OF_VERTICES_PER_LINE];
	long unused[2];
};

struct connected_triangle // [fake name?]
{
	long edge_designators[NUMBER_OF_EDGES_PER_TRIANGLE];
	long group_index;
	long unused[2];
};

struct connected_geometry
{
	struct dynamic_array points;
	struct dynamic_array edges;
	struct dynamic_array triangles;
};

struct intermediate_geometry;

/* ---------- prototypes/CONNECTED_GEOMETRY.C */

void connected_geometry_new(struct connected_geometry *geometry);
void connected_geometry_delete(struct connected_geometry *geometry);

long connected_geometry_add_triangle(struct connected_geometry *geometry, real_point3d const *p0, real_point3d const *p1, real_point3d const *p2, boolean check_for_duplicates);
long connected_geometry_add_intermediate_triangle(struct connected_geometry *geometry, struct intermediate_geometry const *intermediate_geometry, long triangle_index, boolean check_for_duplicates);

void connected_geometry_group_recursive(struct connected_geometry *geometry, boolean (*group_test)(void *data, struct connected_geometry *geometry, struct connected_triangle *triangle, long group_index), void *data, long group_index, long triangle_index);
long connected_geometry_group_coplanar(struct connected_geometry *geometry);

/* ---------- globals */

/* ---------- public code */

#endif // __CONNECTED_GEOMETRY_H
