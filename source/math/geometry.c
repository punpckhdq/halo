/*
GEOMETRY.C
*/

/* ---------- headers */

#include "cseries.h"
#include "real_math.h"
#include "geometry.h"

/* ---------- constants */

enum
{
	CLIP_BUFFER_SIZE = 512,
};

enum
{
	GEOSPHERE_PRIMITIVE_TRIANGLE_COUNT = 8,
	GEOSPHERE_PRIMITIVE_VERTEX_COUNT = 6,
	GEOSPHERE_PRIMITIVE_EDGE_COUNT = 12,
};


/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void subdivide_triangle(struct geosphere *sphere, short v1, short v2, short v3, short *vertex_index, short *triangle_strip_vertex_indices_index, short *vertex_subdivision_indices);
static short get_face_vertex(struct geosphere *sphere, short v1, short v2, short v3, short s1, short s2, short *vertex_index, short *vertex_subdivision_indices, short *vertex_face_indices);
static short get_edge_vertex(struct geosphere *sphere, short v1, short v2, short segment_index, short *vertex_index, short *vertex_subdivision_indices);
static void calculate_vertex(struct geosphere *sphere, short new_vertex, short parent1, short parent2, short subdivision_count, short subdivision_index);
static short points_dimension2d(short count, real_point2d const *points);

/* ---------- globals */

real global_convex_hull3d_delta = 0.01f;
real global_convex_hull3d_epsilon = 0.001f;

static real_point3d const geosphere_primitive_vertices[GEOSPHERE_PRIMITIVE_VERTEX_COUNT] =
{
	{ 0.f, 0.f, 1.f },
	{ 0.f, 1.f, 0.f },
	{ 1.f, 0.f, 0.f },
	{ 0.f, -1.f, 0.f },
	{ -1.f, 0.f, 0.f },
	{ 0.f, 0.f, -1.f }
};

static short const geosphere_primitive_triangles[GEOSPHERE_PRIMITIVE_TRIANGLE_COUNT][NUMBER_OF_VERTICES_PER_TRIANGLE] =
{
	{ 0, 1, 2 },
	{ 0, 2, 3 },
	{ 0, 3, 4 },
	{ 0, 4, 1 },
	{ 5, 1, 4 },
	{ 5, 4, 3 },
	{ 5, 3, 2 },
	{ 5, 2, 1 }
};

/* ---------- public code */

struct geosphere *geosphere_new(
	short segment_count)
{
	struct geosphere *result = match_malloc("c:\\halo\\SOURCE\\math\\geometry.c", 58, sizeof(struct geosphere));

	if (result)
	{
		short *vertex_subdivision_indices;

		result->segment_count = segment_count;
		result->triangle_count = GEOSPHERE_PRIMITIVE_TRIANGLE_COUNT*segment_count*segment_count;
		result->vertex_count = GEOSPHERE_PRIMITIVE_VERTEX_COUNT + GEOSPHERE_PRIMITIVE_EDGE_COUNT*(segment_count-1) + GEOSPHERE_PRIMITIVE_TRIANGLE_COUNT*(segment_count-2)*(segment_count-1)/2;
		result->vertices = match_malloc("c:\\halo\\SOURCE\\math\\geometry.c", 66, result->vertex_count*sizeof(real_point3d));
		result->triangle_strip_vertex_indices = match_malloc("c:\\halo\\SOURCE\\math\\geometry.c", 67, result->triangle_count*(NUMBER_OF_VERTICES_PER_TRIANGLE+1)*sizeof(short));
		result->triangle_strip_count = 0;
		vertex_subdivision_indices = match_malloc("c:\\halo\\SOURCE\\math\\geometry.c", 69, GEOSPHERE_PRIMITIVE_TRIANGLE_COUNT*GEOSPHERE_PRIMITIVE_TRIANGLE_COUNT*sizeof(short));

		if (result->vertices && result->triangle_strip_vertex_indices && vertex_subdivision_indices)
		{
			short triangle_strip_vertex_indices_index = 0;
			short vertex_index = 0;
			long si_index;
			short primitive_vertex_index;
			short primitive_triangle_index;

			for (si_index = 0; si_index<GEOSPHERE_PRIMITIVE_TRIANGLE_COUNT*GEOSPHERE_PRIMITIVE_TRIANGLE_COUNT; si_index++)
			{
				vertex_subdivision_indices[si_index] = NONE;
			}

			for (primitive_vertex_index = 0; primitive_vertex_index<GEOSPHERE_PRIMITIVE_VERTEX_COUNT; primitive_vertex_index++)
			{
				result->vertices[vertex_index++] = geosphere_primitive_vertices[primitive_vertex_index];
			}

			for (primitive_triangle_index = 0; primitive_triangle_index<GEOSPHERE_PRIMITIVE_TRIANGLE_COUNT; primitive_triangle_index++)
			{
				subdivide_triangle(result, geosphere_primitive_triangles[primitive_triangle_index][0], geosphere_primitive_triangles[primitive_triangle_index][1], geosphere_primitive_triangles[primitive_triangle_index][2], &vertex_index, &triangle_strip_vertex_indices_index, vertex_subdivision_indices);
			}

			match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 98, triangle_strip_vertex_indices_index < (NUMBER_OF_VERTICES_PER_TRIANGLE + 1) * result->triangle_count);
			match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 99, vertex_index == result->vertex_count);
		}
		else
		{
			if (result->vertices)
			{
				match_free("c:\\halo\\SOURCE\\math\\geometry.c", 103, result->vertices);
			}

			if (result->triangle_strip_vertex_indices)
			{
				match_free("c:\\halo\\SOURCE\\math\\geometry.c", 104, result->triangle_strip_vertex_indices);
			}
		}

		if (vertex_subdivision_indices)
		{
			match_free("c:\\halo\\SOURCE\\math\\geometry.c", 107, vertex_subdivision_indices);
		}
	}

	return result;
}

void geosphere_dispose(
	struct geosphere *sphere)
{
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 117, sphere);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 118, sphere->vertices);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 119, sphere->triangle_strip_vertex_indices);

	match_free("c:\\halo\\SOURCE\\math\\geometry.c", 121, sphere->vertices);
	match_free("c:\\halo\\SOURCE\\math\\geometry.c", 122, sphere->triangle_strip_vertex_indices);
	match_free("c:\\halo\\SOURCE\\math\\geometry.c", 123, sphere);

	return;
}

static void subdivide_triangle(
	struct geosphere *sphere,
	short v1,
	short v2,
	short v3,
	short *vertex_index,
	short *triangle_strip_vertex_indices_index,
	short *vertex_subdivision_indices)
{
	short face_area = (sphere->segment_count+1)*(sphere->segment_count+1);
	short *face_vertices;

	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 146, vertex_index);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 147, triangle_strip_vertex_indices_index);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 148, vertex_subdivision_indices);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 149, v1 >= 0 && v1 < sphere->vertex_count);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 150, v2 >= 0 && v2 < sphere->vertex_count);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 151, v3 >= 0 && v3 < sphere->vertex_count);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 152, *triangle_strip_vertex_indices_index < (NUMBER_OF_VERTICES_PER_TRIANGLE + 1) * sphere->triangle_count);

	face_vertices = match_malloc("c:\\halo\\SOURCE\\math\\geometry.c", 154, face_area*sizeof(short));
	if (face_vertices)
	{
		short i, j;

		for (i = 0; i<face_area; i++)
		{
			face_vertices[i] = NONE;
		}

		for (i = 1; i<=sphere->segment_count; i++)
		{
			sphere->triangle_strip_vertex_indices[(*triangle_strip_vertex_indices_index)++] = 2*i+1;
			sphere->triangle_strip_count++;

			for (j = 1; j<=i; j++)
			{
				short top_vertex = get_face_vertex(sphere, v1, v2, v3, i-1, j-1, vertex_index, vertex_subdivision_indices, face_vertices);
				short left_vertex = get_face_vertex(sphere, v1, v2, v3, i, j-1, vertex_index, vertex_subdivision_indices, face_vertices);
				short right_vertex = get_face_vertex(sphere, v1, v2, v3, i, j, vertex_index, vertex_subdivision_indices, face_vertices);

				match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 176, top_vertex >= 0 && top_vertex <= sphere->vertex_count);
				match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 177, left_vertex >= 0 && left_vertex <= sphere->vertex_count);
				match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 178, right_vertex >= 0 && right_vertex <= sphere->vertex_count);

				if (j==1)
				{
					sphere->triangle_strip_vertex_indices[(*triangle_strip_vertex_indices_index)++] = left_vertex;
					sphere->triangle_strip_vertex_indices[(*triangle_strip_vertex_indices_index)++] = top_vertex;
				}

				sphere->triangle_strip_vertex_indices[(*triangle_strip_vertex_indices_index)++] = right_vertex;

				if (j<i)
				{
					short topright_vertex = get_face_vertex(sphere, v1, v2, v3, i-1, j, vertex_index, vertex_subdivision_indices, face_vertices);

					match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 194, topright_vertex >= 0 && topright_vertex <= sphere->vertex_count);
					sphere->triangle_strip_vertex_indices[(*triangle_strip_vertex_indices_index)++] = topright_vertex;
				}
			}
		}

		match_free("c:\\halo\\SOURCE\\math\\geometry.c", 200, face_vertices);
	}

	return;
}

static short get_face_vertex(
	struct geosphere *sphere,
	short v1,
	short v2,
	short v3,
	short s1,
	short s2,
	short *vertex_index,
	short *vertex_subdivision_indices,
	short *vertex_face_indices)
{
	short face_vertex_index = s1*(sphere->segment_count+1) + s2;

	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 222, v1 >=0 && v1 <= sphere->vertex_count);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 223, v2 >=0 && v2 <= sphere->vertex_count);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 224, v3 >=0 && v3 <= sphere->vertex_count);

	if (vertex_face_indices[face_vertex_index]==NONE)
	{
		if (s2==0)
		{
			vertex_face_indices[face_vertex_index] = get_edge_vertex(sphere, v1, v3, s1, vertex_index, vertex_subdivision_indices);
		}
		else if (s1==sphere->segment_count)
		{
			vertex_face_indices[face_vertex_index] = get_edge_vertex(sphere, v3, v2, s2, vertex_index, vertex_subdivision_indices);
		}
		else if (s2==s1)
		{
			vertex_face_indices[face_vertex_index] = get_edge_vertex(sphere, v1, v2, s1, vertex_index, vertex_subdivision_indices);
		}
		else
		{
			short new_vertex_index = (*vertex_index)++;
			short parent1 = get_edge_vertex(sphere, v1, v3, s1, vertex_index, vertex_subdivision_indices);
			short parent2 = get_edge_vertex(sphere, v1, v2, s1, vertex_index, vertex_subdivision_indices);

			vertex_face_indices[face_vertex_index] = new_vertex_index;
			calculate_vertex(sphere, new_vertex_index, parent1, parent2, s1, s2);
		}
	}

	return vertex_face_indices[face_vertex_index];
}

static short get_edge_vertex(
	struct geosphere *sphere,
	short v1,
	short v2,
	short segment_index,
	short *vertex_index,
	short *vertex_subdivision_indices)
{
	short va = v1>v2 ? v2 : v1;
	short vb = v1>v2 ? v1 : v2;
	boolean invert = v1>v2;
	short result;

	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 269, sphere);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 270, va >= 0 && va < sphere->vertex_count);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 271, vb >= 0 && vb < sphere->vertex_count);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 272, vb != va);

	if (segment_index==0)
	{
		result = v1;
	}
	else if (segment_index==sphere->segment_count)
	{
		result = v2;
	}
	else
	{
		if (vertex_subdivision_indices[va*GEOSPHERE_PRIMITIVE_TRIANGLE_COUNT+vb]==NONE)
		{
			short new_segment_index;

			vertex_subdivision_indices[va*GEOSPHERE_PRIMITIVE_TRIANGLE_COUNT+vb] = *vertex_index;
			for (new_segment_index = 1; new_segment_index<sphere->segment_count; new_segment_index++)
			{
				calculate_vertex(sphere, (*vertex_index)++, va, vb, sphere->segment_count, new_segment_index);
			}
		}

		result = vertex_subdivision_indices[va*GEOSPHERE_PRIMITIVE_TRIANGLE_COUNT+vb] + (invert ? sphere->segment_count-segment_index : segment_index) - 1;
	}

	return result;
}

static void calculate_vertex(
	struct geosphere *sphere,
	short new_vertex,
	short parent1,
	short parent2,
	short subdivision_count,
	short subdivision_index)
{
	real w2 = (real)subdivision_index/subdivision_count;
	real w1 = 1.f-w2;

	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 315, subdivision_index > 0 && subdivision_index < subdivision_count);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 316, parent1 >=0 && parent1 <= sphere->vertex_count);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 317, parent2 >=0 && parent2 <= sphere->vertex_count);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 318, new_vertex >=0 && new_vertex <= sphere->vertex_count);

	set_real_point3d(&sphere->vertices[new_vertex],
		w1*sphere->vertices[parent1].x + w2*sphere->vertices[parent2].x,
		w1*sphere->vertices[parent1].y + w2*sphere->vertices[parent2].y,
		w1*sphere->vertices[parent1].z + w2*sphere->vertices[parent2].z);
	normalize3d((real_vector3d *)&sphere->vertices[new_vertex]);

	return;
}

void build_torus(
	real_matrix4x3 const *matrix,
	short *vertex_count_reference,
	short *triangle_strip_count_reference,
	real_point3d *points,
	real_point2d *texture_uvs,
	short *triangle_strip_vertex_indices,
	short ring_segment_count,
	real ring_radius,
	short cylinder_segment_count,
	real cylinder_radius)
{
	short triangle_strip_count = 0;
	short vertex_count = 0;
	short ring_segment_index;

	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 346, ring_segment_count>2);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 347, cylinder_segment_count>2);

	for (ring_segment_index = 0; ring_segment_index<=ring_segment_count; ring_segment_index++)
	{
		real u = (real)ring_segment_index/ring_segment_count;
		real ring_angle = 2.f*_pi*u;
		real_point3d cylinder_center;
		real_point3d cylinder_first_point;
		real_vector3d cylinder_rotation_axis;
		short cylinder_segment_index;

		set_real_point3d(&cylinder_center, cosine(ring_angle)*ring_radius, sine(ring_angle)*ring_radius, 0.f);
		cross_product3d((real_vector3d *)&cylinder_center, global_up3d, &cylinder_rotation_axis);
		normalize3d(&cylinder_rotation_axis);
		set_real_point3d(&cylinder_first_point, cosine(ring_angle)*cylinder_radius, sine(ring_angle)*cylinder_radius, 0.f);

		for (cylinder_segment_index = 0; cylinder_segment_index<=cylinder_segment_count; cylinder_segment_index++)
		{
			texture_uvs->y = 2.f*u;

			if (ring_segment_index>0)
			{
				if (cylinder_segment_index==0)
				{
					*triangle_strip_vertex_indices++ = 2*cylinder_segment_count+2;
					triangle_strip_count++;
				}

				*triangle_strip_vertex_indices++ = vertex_count;
				*triangle_strip_vertex_indices++ = vertex_count-(cylinder_segment_count+1);
			}

			if (ring_segment_index==ring_segment_count)
			{
				points[0] = points[-ring_segment_count*(cylinder_segment_count+1)];
				texture_uvs[0].x = texture_uvs[-ring_segment_count*(cylinder_segment_count+1)].x;
			}
			else
			{
				real v = (real)cylinder_segment_index/cylinder_segment_count;

				texture_uvs->x = 2.f*v;
				if (cylinder_segment_index==cylinder_segment_count)
				{
					points[0] = points[-cylinder_segment_count];
				}
				else
				{
					real cylinder_angle = 2.f*_pi*v;

					*points = cylinder_first_point;
					rotate_vector_about_axis((real_vector3d *)points, &cylinder_rotation_axis, sine(cylinder_angle), cosine(cylinder_angle));
					add_vectors3d((real_vector3d *)&cylinder_center, (real_vector3d *)points, (real_vector3d *)points);
					matrix4x3_transform_point(matrix, points, points);
				}
			}

			points++;
			texture_uvs++;
			vertex_count++;
		}
	}

	*vertex_count_reference = vertex_count;
	*triangle_strip_count_reference = triangle_strip_count;

	return;
}

static short points_dimension2d(
	short count,
	real_point2d const *points)
{
	short dimension = NONE;
	real_point2d dimension0_point;
	real_plane2d dimension1_plane;
	short point_index;

	for (point_index = 0; dimension<_z && point_index<count; point_index++)
	{
		switch (dimension)
		{
		case NONE:
			dimension0_point = points[point_index];
			dimension = _x;
			break;
		case _x:
			if (plane2d_from_points(&dimension1_plane, &points[point_index], &dimension0_point))
			{
				dimension = _y;
			}
			break;
		case _y:
			if (!realcmp(plane2d_distance_to_point(&dimension1_plane, &points[point_index]), 0.f))
			{
				dimension = _z;
			}
			break;
		}
	}

	return dimension;
}

short convex_hull2d(
	short vertex_count,
	real_point2d const *points,
	short *hull_vertex_indices)
{
	short hull_vertex_count = 0;

	if (points_dimension2d(vertex_count, points)==_z)
	{
		real total_angle = 0.f;
		boolean left_first_vertex = FALSE;
		real minimum_x = REAL_MAX;
		real minimum_y = REAL_MAX;
		short last_vertex_index;
		short vertex_index;

		for (vertex_index = 0; vertex_index<vertex_count; vertex_index++)
		{
			if (minimum_y-_real_epsilon>points[vertex_index].y ||
				(points[vertex_index].y<minimum_y && minimum_x+_real_epsilon>points[vertex_index].x) ||
				(minimum_y+_real_epsilon>points[vertex_index].y && minimum_x-_real_epsilon>points[vertex_index].x))
			{
				last_vertex_index = vertex_index;
				minimum_x = points[vertex_index].x;
				minimum_y = points[vertex_index].y;
			}
		}

		do
		{
			real minimum_angle = REAL_MAX;
			short minimum_angle_vertex;

			if (hull_vertex_count>=vertex_count)
			{
				short end_vertex_index = hull_vertex_count-1;
				short start_vertex_index;

				for (start_vertex_index = end_vertex_index-1; start_vertex_index>0; start_vertex_index--)
				{
					if (hull_vertex_indices[start_vertex_index]==hull_vertex_indices[end_vertex_index])
					{
						hull_vertex_count = end_vertex_index-start_vertex_index;
						for (vertex_index = 0; vertex_index<hull_vertex_count; vertex_index++)
						{
							match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 633, vertex_index<vertex_count);
							match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 634, start_vertex_index+vertex_index<vertex_count);
							hull_vertex_indices[vertex_index] = hull_vertex_indices[start_vertex_index+vertex_index];
						}
						break;
					}
				}

				match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 642, start_vertex_index>0);
				break;
			}

			hull_vertex_indices[hull_vertex_count++] = last_vertex_index;
			for (vertex_index = 0; vertex_index<vertex_count; vertex_index++)
			{
				if (points[vertex_index].x!=points[last_vertex_index].x || points[vertex_index].y!=points[last_vertex_index].y)
				{
					real angle = arctangent(points[vertex_index].y-points[last_vertex_index].y, points[vertex_index].x-points[last_vertex_index].x) - total_angle;

					while (angle<-_real_epsilon)
					{
						angle+= 2.f*_pi;
					}

					if (angle<minimum_angle)
					{
						minimum_angle = angle;
						minimum_angle_vertex = vertex_index;
					}
				}
			}

			total_angle+= minimum_angle;
			last_vertex_index = minimum_angle_vertex;
			if (!left_first_vertex)
			{
				left_first_vertex = !(realcmp(points[minimum_angle_vertex].x, points[hull_vertex_indices[0]].x) && realcmp(points[minimum_angle_vertex].y, points[hull_vertex_indices[0]].y));
			}
		}
		while (last_vertex_index!=hull_vertex_indices[0] && !(left_first_vertex && realcmp(points[last_vertex_index].x, points[hull_vertex_indices[0]].x) && realcmp(points[last_vertex_index].y, points[hull_vertex_indices[0]].y)));
	}

	return hull_vertex_count;
}

boolean convex_hull2d_verify(
	short point_count,
	real_point2d const *points,
	short hull_count,
	short const *hull_indices)
{
	real winding = 0.f;
	short hull_index;

	for (hull_index = 0; hull_index<hull_count; hull_index++)
	{
		real_point2d const *point0 = &points[hull_indices[hull_index-1>=0 ? hull_index-1 : hull_count-1]];
		real_point2d const *point1 = &points[hull_indices[hull_index]];
		real_point2d const *point2 = &points[hull_indices[hull_index+1<hull_count ? hull_index+1 : 0]];
		real_vector2d edge0;
		real_vector2d edge1;

		vector_from_points2d(point0, point1, &edge0);
		vector_from_points2d(point1, point2, &edge1);
		if (cross_product2d(&edge0, &edge1)<0.f)
		{
			return FALSE;
		}

		winding+= angle_between_vectors2d(&edge0, &edge1);
	}

	return fabs(winding-2.f*_pi)<0.001f;
}

boolean convex_hull2d_test_circle(
	short count,
	real_point2d const *points,
	real_point2d const *p,
	real radius)
{
	real radius_squared = radius*radius;
	boolean inside = TRUE;
	short point_index;

	for (point_index = 0; point_index<count; point_index++)
	{
		real_point2d const *p0 = &points[point_index];
		real_point2d const *p1 = &points[point_index+1<count ? point_index+1 : 0];
		real_vector2d p0p1;
		real_vector2d p0p;
		real magnitude_squared;

		vector_from_points2d(p0, p1, &p0p1);
		vector_from_points2d(p0, p, &p0p);
		magnitude_squared = magnitude_squared2d(&p0p1);
		if (magnitude_squared!=0.f)
		{
			real cross = cross_product2d(&p0p, &p0p1);

			if (cross>0.f && cross*cross>magnitude_squared*radius_squared)
			{
				inside = FALSE;
				break;
			}
		}
	}

	return inside;
}

boolean convex_hull2d_test_point(
	short count,
	real_point2d const *points,
	real_point2d const *point,
	real epsilon)
{
	boolean inside = TRUE;
	short point_index;

	for (point_index = 0; point_index<count; point_index++)
	{
		real_point2d const *point0 = &points[point_index];
		real_point2d const *point1 = &points[point_index+1<count ? point_index+1 : 0];
		real_vector2d edge_vector;
		real_vector2d test_vector;

		vector_from_points2d(point0, point1, &edge_vector);
		vector_from_points2d(point0, point, &test_vector);
		if (cross_product2d(&edge_vector, &test_vector)<-epsilon)
		{
			inside = FALSE;
			break;
		}
	}

	return inside;
}

boolean convex_hull2d_test_point_indexed(
	short count,
	short const *indices,
	real_point2d const *points,
	real_point2d const *point,
	real epsilon)
{
	boolean inside = TRUE;
	short point_index;

	for (point_index = 0; point_index<count; point_index++)
	{
		real_point2d const *point0 = &points[indices[point_index]];
		real_point2d const *point1 = &points[indices[point_index+1<count ? point_index+1 : 0]];
		real_vector2d edge_vector;
		real_vector2d test_vector;

		vector_from_points2d(point0, point1, &edge_vector);
		vector_from_points2d(point0, point, &test_vector);
		if (cross_product2d(&edge_vector, &test_vector)<-epsilon)
		{
			inside = FALSE;
			break;
		}
	}

	return inside;
}

real convex_hull2d_area(
	short count,
	real_point2d const *points)
{
	real area = 0.f;
	short point_index;

	for (point_index = 2; point_index<count; point_index++)
	{
		real_vector2d v0;
		real_vector2d v1;

		vector_from_points2d(&points[0], &points[point_index-1], &v0);
		vector_from_points2d(&points[0], &points[point_index], &v1);
		area+= cross_product2d(&v0, &v1)*0.5f;
	}

	return (real)fabs(area);
}

real convex_hull2d_perimeter(
	short count,
	real_point2d const *points)
{
	real perimeter = distance2d(&points[count-1], &points[0]);
	short point_index;

	for (point_index = 1; point_index<count; point_index++)
	{
		perimeter+= distance2d(&points[point_index-1], &points[point_index]);
	}

	return perimeter;
}

short convex_hull2d_intersect(
	short p_count,
	real_point2d const *p,
	short q_count,
	real_point2d const *q,
	short maximum_count,
	real_point2d *result,
	real epsilon)
{
	real_point2d storage[2][CLIP_BUFFER_SIZE];
	short result_count = q_count;
	real_point2d const *points = q;
	short vertex_index;

	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1016, maximum_count<=CLIP_BUFFER_SIZE);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1017, p);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1018, p_count);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1019, q);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1020, q_count);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1021, result);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1022, p!=result && q!=result);

	for (vertex_index = 0; vertex_index<p_count && result_count>0; vertex_index++)
	{
		short vertex_index1 = vertex_index ? vertex_index-1 : p_count-1;
		real_point2d *out = vertex_index==p_count-1 ? result : storage[vertex_index&1];
		real_plane2d edge;

		if (plane2d_from_points(&edge, &p[vertex_index], &p[vertex_index1]))
		{
			result_count = convex_polygon2d_clip_to_plane(result_count, points, &edge, maximum_count, out, NULL, NULL, epsilon);
			if (result_count==NONE)
			{
				return NONE;
			}
		}
		else
		{
			match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1037, result_count>=0 && result_count<=maximum_count);
			memcpy(out, points, result_count*sizeof(real_point2d));
		}

		points = out;
	}

	return result_count;
}

boolean convex_hull2d_test_vector(
	short count,
	real_point2d const *points,
	real_point2d const *point,
	real_vector2d const *vector,
	real *t_in_reference,
	real *t_out_reference)
{
	real t_in = -REAL_MAX;
	real t_out = REAL_MAX;
	boolean hit = TRUE;
	short point_index;

	for (point_index = 0; point_index<count; point_index++)
	{
		real_point2d const *point0 = &points[point_index];
		real_point2d const *point1 = &points[point_index+1<count ? point_index+1 : 0];
		real_vector2d edge_vector;
		real_vector2d test_vector;
		real vector_cross_edge;
		real edge_cross_test;

		vector_from_points2d(point0, point1, &edge_vector);
		vector_from_points2d(point0, point, &test_vector);
		vector_cross_edge = cross_product2d(vector, &edge_vector);
		edge_cross_test = cross_product2d(&edge_vector, &test_vector);
		if (!realcmp(vector_cross_edge, 0.f))
		{
			real t = edge_cross_test/vector_cross_edge;

			if (vector_cross_edge>0.f)
			{
				if (t_in<t)
				{
					t_in = t;
				}
			}
			else
			{
				if (t_out>t)
				{
					t_out = t;
				}
			}

			if (t_in>t_out)
			{
				hit = FALSE;
				break;
			}
		}
		else if (edge_cross_test<_real_epsilon)
		{
			hit = FALSE;
			break;
		}
	}

	if (hit)
	{
		if (t_in_reference)
		{
			*t_in_reference = t_in;
		}

		if (t_out_reference)
		{
			*t_out_reference = t_out;
		}
	}

	return hit;
}

short convex_polygon2d_clip_to_plane(
	short count,
	real_point2d const *points,
	real_plane2d const *plane,
	short maximum_count,
	real_point2d *result,
	unsigned long *clip_flags,
	boolean *clipped,
	real epsilon)
{
	real_point2d storage[CLIP_BUFFER_SIZE];
	short result_count = 0;
	boolean found_in_vertex = FALSE;
	boolean found_out_vertex = FALSE;
	unsigned long clip_flags_out = 0;
	real_point2d const *last_vertex;
	boolean last_in;
	short vertex_index;

	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1350, count>=NUMBER_OF_VERTICES_PER_TRIANGLE);

	if (clipped)
	{
		*clipped = FALSE;
	}

	if (points==result)
	{
		match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1357, count<=CLIP_BUFFER_SIZE);
		memcpy(storage, points, count*sizeof(real_point2d));
		points = storage;
	}

	last_vertex = &points[count-1];
	last_in = dot_product2d((real_vector2d const *)last_vertex, &plane->n)-plane->d>=0.f;
	for (vertex_index = 0; vertex_index<count; vertex_index++)
	{
		real_point2d const *vertex = &points[vertex_index];
		real d = plane2d_distance_to_point(plane, vertex);
		boolean in = d>=0.f;

		if (d>epsilon)
		{
			found_in_vertex = TRUE;
		}
		else if (d<-epsilon)
		{
			found_out_vertex = TRUE;
		}

		if (in!=last_in)
		{
			real_vector2d v;
			real t;

			if (result_count==maximum_count)
			{
				result_count = NONE;
				break;
			}

			if (clipped)
			{
				*clipped = TRUE;
			}

			vector_from_points2d(vertex, last_vertex, &v);
			t = vector_intersect_plane2d(vertex, &v, plane);
			point_from_line2d(vertex, &v, PIN(t, 0.f, 1.f), &result[result_count]);
			SET_FLAG(clip_flags_out, result_count++, TRUE);

				if (result_count!=1)
				{
					if ((realcmp_epsilon(result[result_count-1].x, result[0].x, epsilon) && realcmp_epsilon(result[result_count-1].y, result[0].y, epsilon)) ||
						(realcmp_epsilon(result[result_count-1].x, result[result_count-2].x, epsilon) && realcmp_epsilon(result[result_count-1].y, result[result_count-2].y, epsilon)))
					{
						result_count--;
					}
				}
		}

		if (in)
		{
			if (result_count==maximum_count)
			{
				result_count = NONE;
				break;
			}

			result[result_count] = *vertex;
			SET_FLAG(clip_flags_out, result_count++, clip_flags ? TEST_FLAG(*clip_flags, vertex_index) : FALSE);

				if (result_count!=1)
				{
					if ((realcmp_epsilon(result[result_count-1].x, result[0].x, epsilon) && realcmp_epsilon(result[result_count-1].y, result[0].y, epsilon)) ||
						(realcmp_epsilon(result[result_count-1].x, result[result_count-2].x, epsilon) && realcmp_epsilon(result[result_count-1].y, result[result_count-2].y, epsilon)))
					{
						result_count--;
					}
				}
		}

		last_vertex = vertex;
		last_in = in;
	}

	if (result_count!=NONE)
	{
		if (result_count<NUMBER_OF_VERTICES_PER_TRIANGLE)
		{
			result_count = 0;
		}

		if (!found_in_vertex)
		{
			result_count = 0;
		}
		else if (!found_out_vertex)
		{
			match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1441, count>=0 && count<=maximum_count);
			memcpy(result, points, count*sizeof(real_point2d));
			result_count = count;
		}
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1448, count>=0 && count<=maximum_count);
		memcpy(result, points, count*sizeof(real_point2d));
	}

	if (clip_flags)
	{
		*clip_flags = clip_flags_out;
	}

	return result_count;
}

boolean convex_polygon2d_verify(
	short count,
	real_point2d const *points)
{
	short vertex_index;

	for (vertex_index = 0; vertex_index<count; vertex_index++)
	{
		if (!valid_real_point2d(&points[vertex_index]))
		{
			return FALSE;
		}
	}

	return TRUE;
}

short convex_polygon3d_clip_to_plane(
	short count,
	real_point3d const *points,
	real_plane3d const *plane,
	short maximum_count,
	real_point3d *result,
	boolean *clipped,
	real epsilon,
	boolean keep_coplanar)
{
	real_point3d storage[CLIP_BUFFER_SIZE];
	short result_count = 0;
	boolean found_in_vertex = FALSE;
	boolean found_out_vertex = FALSE;
	real_point3d const *last_vertex;
	boolean last_in;
	short vertex_index;

	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1493, count>=NUMBER_OF_VERTICES_PER_TRIANGLE);

	if (clipped)
	{
		*clipped = FALSE;
	}

	if (points==result)
	{
		match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1500, count<=CLIP_BUFFER_SIZE);
		memcpy(storage, points, count*sizeof(real_point3d));
		points = storage;
	}

	last_vertex = &points[count-1];
	last_in = plane3d_distance_to_point(plane, last_vertex)>=0.f;
	for (vertex_index = 0; vertex_index<count; vertex_index++)
	{
		real_point3d const *vertex = &points[vertex_index];
		real d = plane3d_distance_to_point(plane, vertex);
		boolean in = d>=0.f;

		if (d>epsilon)
		{
			found_in_vertex = TRUE;
		}
		else if (d<-epsilon)
		{
			found_out_vertex = TRUE;
		}

		if (in!=last_in)
		{
			real_vector3d v;
			real t;

			if (result_count==maximum_count)
			{
				result_count = NONE;
				break;
			}

			if (clipped)
			{
				*clipped = TRUE;
			}

			vector_from_points3d(vertex, last_vertex, &v);
			t = vector_intersect_plane3d(vertex, &v, plane);
			point_from_line3d(vertex, &v, PIN(t, 0.f, 1.f), &result[result_count++]);

				if (result_count!=1)
				{
					if ((realcmp_epsilon(result[result_count-1].x, result[0].x, epsilon) && realcmp_epsilon(result[result_count-1].y, result[0].y, epsilon) && realcmp_epsilon(result[result_count-1].z, result[0].z, epsilon)) ||
						(realcmp_epsilon(result[result_count-1].x, result[result_count-2].x, epsilon) && realcmp_epsilon(result[result_count-1].y, result[result_count-2].y, epsilon) && realcmp_epsilon(result[result_count-1].z, result[result_count-2].z, epsilon)))
					{
						result_count--;
					}
				}
		}

		if (in)
		{
			if (result_count>=maximum_count)
			{
				result_count = NONE;
				break;
			}

			result[result_count++] = *vertex;

				if (result_count!=1)
				{
					if ((realcmp_epsilon(result[result_count-1].x, result[0].x, epsilon) && realcmp_epsilon(result[result_count-1].y, result[0].y, epsilon) && realcmp_epsilon(result[result_count-1].z, result[0].z, epsilon)) ||
						(realcmp_epsilon(result[result_count-1].x, result[result_count-2].x, epsilon) && realcmp_epsilon(result[result_count-1].y, result[result_count-2].y, epsilon) && realcmp_epsilon(result[result_count-1].z, result[result_count-2].z, epsilon)))
					{
						result_count--;
					}
				}
		}

		last_vertex = vertex;
		last_in = in;
	}

	if (result_count!=NONE)
	{
		if (result_count<NUMBER_OF_VERTICES_PER_TRIANGLE)
		{
			result_count = 0;
		}

		if (!found_in_vertex && (found_out_vertex || !keep_coplanar))
		{
			result_count = 0;
		}
		else if (!found_out_vertex)
		{
			match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1584, count>=0 && count<=maximum_count);
			memcpy(result, points, count*sizeof(real_point3d));
			result_count = count;
		}
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1591, count>=0 && count<=maximum_count);
		memcpy(result, points, count*sizeof(real_point3d));
	}

	return result_count;
}

boolean convex_polygon3d_verify(
	short count,
	real_point3d const *points)
{
	short vertex_index;
	real_vector3d v1v0;
	real_vector3d v1v2;
	real_vector3d cross;

	vector_from_points3d(&points[1], &points[0], &v1v0);
	vector_from_points3d(&points[1], &points[2], &v1v2);
	cross_product3d(&v1v0, &v1v2, &cross);

	for (vertex_index = 0; vertex_index<count; vertex_index++)
	{
		real_point3d const *v0 = vertex_index==0 ? &points[count-1] : &points[vertex_index-1];
		real_point3d const *v1 = &points[vertex_index];
		real_point3d const *v2 = vertex_index==count-1 ? &points[0] : &points[vertex_index+1];
		real_vector3d v1v0;
		real_vector3d v1v2;
		real_vector3d my_cross;

		if (!valid_real_point3d(v1))
		{
			return FALSE;
		}

		vector_from_points3d(v1, v0, &v1v0);
		vector_from_points3d(v1, v2, &v1v2);
		cross_product3d(&v1v0, &v1v2, &my_cross);
		if (dot_product3d(&my_cross, &cross)<-1e-6f)
		{
			return FALSE;
		}
	}

	return TRUE;
}

boolean convex_hull3d_begin(
	short point_count,
	real_point3d const *points,
	short vertex_count,
	struct vertex3d *vertices,
	short edge_count,
	struct edge3d *edges,
	short surface_count,
	struct surface3d *surfaces)
{
	short point0_index = NONE;
	short point1_index = NONE;
	short point2_index = NONE;
	short point3_index = NONE;
	real_vector3d v;
	real_vector3d v0;
	real_vector3d v1;
	real_plane3d p;
	short point_index;
	short vertex_index;
	short edge_index;
	short surface_index;

	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1710, points);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1711, vertices);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1712, edges);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1713, surfaces);

	if (vertex_count<4 || edge_count<6 || surface_count<4)
	{
		return FALSE;
	}

	{
		real x = REAL_MAX;

		for (point_index = 0; point_index<point_count; point_index++)
		{
			if (x>points[point_index].x)
			{
				x = points[point_index].x;
				point0_index = point_index;
			}
		}
	}

	if (point0_index==NONE)
	{
		return FALSE;
	}

	{
		real d = 0.f;

		for (point_index = 0; point_index<point_count; point_index++)
		{
			if (distance_squared3d(&points[point_index], &points[point0_index])>d)
			{
				d = distance_squared3d(&points[point_index], &points[point0_index]);
				point1_index = point_index;
			}
		}

		if (point1_index==NONE || d<global_convex_hull3d_delta)
		{
			return FALSE;
		}
	}

	{
		real d = 0.f;

		vector_from_points3d(&points[point0_index], &points[point1_index], &v);
		for (point_index = 0; point_index<point_count; point_index++)
		{
			real d2;

			v0 = v;
			vector_from_points3d(&points[point0_index], &points[point_index], &v1);
			scale_vector3d(&v0, dot_product3d(&v1, &v0)/magnitude_squared3d(&v0), &v0);
			d2 = distance_squared3d((real_point3d *)&v0, (real_point3d *)&v1);
			if (d2>d)
			{
				d = d2;
				point2_index = point_index;
			}
		}

		if (point2_index==NONE || d<global_convex_hull3d_delta)
		{
			return FALSE;
		}
	}

	{
		real d = 0.f;

		plane3d_from_points(&p, &points[point0_index], &points[point1_index], &points[point2_index]);
		for (point_index = 0; point_index<point_count; point_index++)
		{
			real d2 = plane3d_distance_to_point(&p, &points[point_index]);

			if (fabs(d2)>fabs(d))
			{
				d = d2;
				point3_index = point_index;
			}
		}

		if (point3_index==NONE || fabs(d)<global_convex_hull3d_delta)
		{
			return FALSE;
		}

		if (d>0.f)
		{
			short temporary = point1_index;

			point1_index = point2_index;
			point2_index = temporary;
		}
	}

	vertices[0].extant = TRUE;
	vertices[0].point_index = point0_index;
	vertices[0].edge_index = 0;
	vertices[1].extant = TRUE;
	vertices[1].point_index = point1_index;
	vertices[1].edge_index = 0;
	vertices[2].extant = TRUE;
	vertices[2].point_index = point2_index;
	vertices[2].edge_index = 1;
	vertices[3].extant = TRUE;
	vertices[3].point_index = point3_index;
	vertices[3].edge_index = 3;

	edges[0].extant = TRUE;
	edges[0].vertex_indices[0] = 0;
	edges[0].vertex_indices[1] = 1;
	edges[0].edge_indices[0] = 1;
	edges[0].edge_indices[1] = 3;
	edges[0].surface_indices[0] = 0;
	edges[0].surface_indices[1] = 1;

	edges[1].extant = TRUE;
	edges[1].vertex_indices[0] = 1;
	edges[1].vertex_indices[1] = 2;
	edges[1].edge_indices[0] = 2;
	edges[1].edge_indices[1] = 4;
	edges[1].surface_indices[0] = 0;
	edges[1].surface_indices[1] = 2;

	edges[2].extant = TRUE;
	edges[2].vertex_indices[0] = 2;
	edges[2].vertex_indices[1] = 0;
	edges[2].edge_indices[0] = 0;
	edges[2].edge_indices[1] = 5;
	edges[2].surface_indices[0] = 0;
	edges[2].surface_indices[1] = 3;

	edges[3].extant = TRUE;
	edges[3].vertex_indices[0] = 0;
	edges[3].vertex_indices[1] = 3;
	edges[3].edge_indices[0] = 4;
	edges[3].edge_indices[1] = 2;
	edges[3].surface_indices[0] = 1;
	edges[3].surface_indices[1] = 3;

	edges[4].extant = TRUE;
	edges[4].vertex_indices[0] = 3;
	edges[4].vertex_indices[1] = 1;
	edges[4].edge_indices[0] = 0;
	edges[4].edge_indices[1] = 5;
	edges[4].surface_indices[0] = 1;
	edges[4].surface_indices[1] = 2;

	edges[5].extant = TRUE;
	edges[5].vertex_indices[0] = 3;
	edges[5].vertex_indices[1] = 2;
	edges[5].edge_indices[0] = 1;
	edges[5].edge_indices[1] = 3;
	edges[5].surface_indices[0] = 2;
	edges[5].surface_indices[1] = 3;

	surfaces[0].extant = TRUE;
	plane3d_from_points(&surfaces[0].plane, &points[point0_index], &points[point1_index], &points[point2_index]);
	surfaces[0].edge_index = 0;
	surfaces[1].extant = TRUE;
	plane3d_from_points(&surfaces[1].plane, &points[point0_index], &points[point3_index], &points[point1_index]);
	surfaces[1].edge_index = 0;
	surfaces[2].extant = TRUE;
	plane3d_from_points(&surfaces[2].plane, &points[point1_index], &points[point3_index], &points[point2_index]);
	surfaces[2].edge_index = 1;
	surfaces[3].extant = TRUE;
	plane3d_from_points(&surfaces[3].plane, &points[point0_index], &points[point2_index], &points[point3_index]);
	surfaces[3].edge_index = 2;

	for (vertex_index = 4; vertex_index<vertex_count; vertex_index++)
	{
		vertices[vertex_index].extant = FALSE;
	}

	for (edge_index = 6; edge_index<edge_count; edge_index++)
	{
		edges[edge_index].extant = FALSE;
	}

	for (surface_index = 4; surface_index<surface_count; surface_index++)
	{
		surfaces[surface_index].extant = FALSE;
	}

	return TRUE;
}

boolean convex_hull3d_expand(
	short point_count,
	real_point3d const *points,
	short vertex_count,
	struct vertex3d *vertices,
	short edge_count,
	struct edge3d *edges,
	short surface_count,
	struct surface3d *surfaces,
	short point_index)
{
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1973, points);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1974, vertices);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1975, edges);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 1976, surfaces);

	if (point_index<0 || point_index>=point_count)
	{
		return FALSE;
	}
	else
	{
		real_point3d const *point = &points[point_index];
		short first_boundary_edge_index = NONE;
		boolean inside = TRUE;
		short surface_index;
		short edge_index;
		short vertex_index;

		for (surface_index = 0; surface_index<surface_count; surface_index++)
		{
			struct surface3d const *surface = &surfaces[surface_index];

			if (surface->extant && plane3d_distance_to_point(&surface->plane, point)>global_convex_hull3d_delta)
			{
				inside = FALSE;
				break;
			}
		}

		if (inside)
		{
			return TRUE;
		}

		for (surface_index = 0; surface_index<surface_count; surface_index++)
		{
			struct surface3d *surface = &surfaces[surface_index];

			if (surface->extant)
			{
				surface->extant = !(plane3d_distance_to_point(&surface->plane, point)>-global_convex_hull3d_epsilon);
			}
		}

		for (edge_index = 0; edge_index<edge_count; edge_index++)
		{
			struct edge3d *edge = &edges[edge_index];

			if (edge->extant)
			{
				struct surface3d const *surface0 = &surfaces[edge->surface_indices[0]];
				struct surface3d const *surface1 = &surfaces[edge->surface_indices[1]];

				if (edge->surface_indices[0]<0 || edge->surface_indices[0]>=surface_count || edge->surface_indices[1]<0 || edge->surface_indices[1]>=surface_count)
				{
					return FALSE;
				}

				edge->extant = surface0->extant || surface1->extant;
				if (edge->extant && surface0->extant!=surface1->extant)
				{
					if (!surface0->extant)
					{
						edge->surface_indices[0] = NONE;
					}
					else
					{
						edge->surface_indices[1] = NONE;
					}

					if (first_boundary_edge_index==NONE)
					{
						first_boundary_edge_index = edge_index;
					}
				}
			}
		}

		match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2076, first_boundary_edge_index!=NONE);

		for (vertex_index = 0; vertex_index<vertex_count; vertex_index++)
		{
			struct vertex3d *vertex = &vertices[vertex_index];

			if (vertex->extant)
			{
				short first_good_edge_index = NONE;
				short last_good_edge_index = NONE;

				edge_index = (short)vertex->edge_index;
				do
				{
					struct edge3d const *edge = &edges[edge_index];

					match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2098, edge_index>=0 && edge_index<edge_count);
					match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2099, edge->vertex_indices[0]==vertex_index || edge->vertex_indices[1]==vertex_index);

					if (edge->extant)
					{
						if (last_good_edge_index==NONE)
						{
							first_good_edge_index = edge_index;
						}
						else
						{
							struct edge3d *last_good_edge = &edges[last_good_edge_index];

							last_good_edge->edge_indices[last_good_edge->vertex_indices[0]==vertex_index] = edge_index;
						}

						last_good_edge_index = edge_index;
					}

					edge_index = (short)edge->edge_indices[edge->vertex_indices[0]==vertex_index];
				}
				while (edge_index!=vertex->edge_index);

				if (last_good_edge_index==NONE)
				{
					vertex->extant = FALSE;
				}
				else
				{
					struct edge3d *last_good_edge = &edges[last_good_edge_index];

					vertex->edge_index = first_good_edge_index;
					last_good_edge->edge_indices[last_good_edge->vertex_indices[0]==vertex_index] = first_good_edge_index;
				}
			}
		}

		{
			short new_vertex_index;
			short first_new_edge_index = NONE;
			short last_new_edge_index = NONE;

			edge_index = first_boundary_edge_index;
			for (new_vertex_index = 0; new_vertex_index<vertex_count; new_vertex_index++)
			{
				if (!vertices[new_vertex_index].extant)
				{
					break;
				}
			}

			if (new_vertex_index>=vertex_count)
			{
				return FALSE;
			}

			do
			{
				struct edge3d *edge = &edges[edge_index];
				short new_surface_index;
				short new_edge_index;

				match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2167, edge_index>=0 && edge_index<edge_count);
				match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2168, edge->extant);
				match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2169, edge->vertex_indices[0]>=0 && edge->vertex_indices[0]<vertex_count);
				match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2170, edge->vertex_indices[1]>=0 && edge->vertex_indices[1]<vertex_count);
				match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2171, (edge->surface_indices[0]==NONE)!=(edge->surface_indices[1]==NONE));

				for (new_surface_index = 0; new_surface_index<surface_count; new_surface_index++)
				{
					if (!surfaces[new_surface_index].extant)
					{
						break;
					}
				}

				if (new_surface_index>=surface_count)
				{
					return FALSE;
				}

				for (new_edge_index = 0; new_edge_index<edge_count; new_edge_index++)
				{
					if (!edges[new_edge_index].extant)
					{
						break;
					}
				}

				if (new_edge_index>=edge_count)
				{
					return FALSE;
				}

				{
					struct surface3d *new_surface = &surfaces[new_surface_index];
					struct vertex3d const *vertex1 = &vertices[edge->vertex_indices[edge->surface_indices[0]!=NONE]];
					struct vertex3d const *vertex2 = &vertices[edge->vertex_indices[edge->surface_indices[1]!=NONE]];
					struct edge3d *new_edge;

					match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2206, vertex1->extant);
					match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2207, vertex1->point_index>=0 && vertex1->point_index<point_count);
					match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2209, vertex2->extant);
					match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2210, vertex2->point_index>=0 && vertex2->point_index<point_count);

					new_surface->extant = TRUE;
					plane3d_from_points(&new_surface->plane, &points[point_index], &points[vertex1->point_index], &points[vertex2->point_index]);
					new_surface->edge_index = edge_index;

					new_edge = &edges[new_edge_index];
					new_edge->extant = TRUE;
					new_edge->vertex_indices[0] = edge->vertex_indices[edge->surface_indices[1]!=NONE];
					new_edge->vertex_indices[1] = new_vertex_index;
					new_edge->edge_indices[0] = last_new_edge_index;
					new_edge->edge_indices[1] = edge->edge_indices[edge->surface_indices[0]!=NONE];
					new_edge->surface_indices[0] = new_surface_index;
					new_edge->surface_indices[1] = NONE;
					edge->edge_indices[edge->surface_indices[0]!=NONE] = new_edge_index;
					edge->surface_indices[edge->surface_indices[0]!=NONE] = new_surface_index;

					if (last_new_edge_index==NONE)
					{
						first_new_edge_index = new_edge_index;
					}
					else
					{
						edges[last_new_edge_index].surface_indices[1] = new_surface_index;
					}

					last_new_edge_index = new_edge_index;
					edge_index = (short)new_edge->edge_indices[1];
				}
			}
			while (edge_index!=first_boundary_edge_index);

			{
				struct vertex3d *new_vertex = &vertices[new_vertex_index];
				struct edge3d *first_new_edge;
				struct edge3d *last_new_edge;

				new_vertex->extant = TRUE;
				new_vertex->point_index = point_index;
				new_vertex->edge_index = first_new_edge_index;
				first_new_edge = &edges[first_new_edge_index];
				last_new_edge = &edges[last_new_edge_index];
				first_new_edge->edge_indices[0] = last_new_edge_index;
				last_new_edge->surface_indices[1] = first_new_edge->surface_indices[0];
			}
		}
	}

	return TRUE;
}

boolean convex_hull3d(
	short point_count,
	real_point3d const *points,
	short vertex_count,
	struct vertex3d *vertices,
	short edge_count,
	struct edge3d *edges,
	short surface_count,
	struct surface3d *surfaces)
{
	short point_index;

	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2284, points);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2285, vertices);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2286, edges);
	match_assert("c:\\halo\\SOURCE\\math\\geometry.c", 2287, surfaces);

	if (!convex_hull3d_begin(point_count, points, vertex_count, vertices, edge_count, edges, surface_count, surfaces))
	{
		return FALSE;
	}

	for (point_index = 0; point_index<point_count; point_index++)
	{
		if (!convex_hull3d_expand(point_count, points, vertex_count, vertices, edge_count, edges, surface_count, surfaces, point_index))
		{
			return FALSE;
		}
	}

	return TRUE;
}

boolean convex_hull3d_verify(
	short point_count,
	real_point3d const *points,
	short vertex_count,
	struct vertex3d const *vertices,
	short edge_count,
	struct edge3d const *edges,
	short surface_count,
	struct surface3d const *surfaces)
{
	return TRUE;
}

boolean convex_hull3d_test_point(
	short point_count,
	real_point3d const *points,
	short vertex_count,
	struct vertex3d const *vertices,
	short edge_count,
	struct edge3d const *edges,
	short surface_count,
	struct surface3d const *surfaces,
	real_point3d const *point)
{
	boolean inside = TRUE;
	short surface_index;

	for (surface_index = 0; surface_index<surface_count; surface_index++)
	{
		struct surface3d const *surface = &surfaces[surface_index];

		if (surface->extant && plane3d_distance_to_point(&surface->plane, point)>global_convex_hull3d_epsilon)
		{
			inside = FALSE;
			break;
		}
	}

	return inside;
}

boolean convex_hull3d_test_vector(
	short point_count,
	real_point3d const *points,
	short vertex_count,
	struct vertex3d const *vertices,
	short edge_count,
	struct edge3d const *edges,
	short surface_count,
	struct surface3d const *surfaces,
	real_point3d const *point,
	real_vector3d const *vector,
	real *t_in_reference,
	real *t_out_reference)
{
	real t_in = -REAL_MAX;
	real t_out = REAL_MAX;
	boolean hit = TRUE;
	short surface_index;

	for (surface_index = 0; surface_index<surface_count; surface_index++)
	{
		struct surface3d const *surface = &surfaces[surface_index];

		if (surface->extant)
		{
			real distance = plane3d_distance_to_point(&surface->plane, point);
			real vector_dot_normal = dot_product3d(vector, &surface->plane.n);

			if (!realcmp(vector_dot_normal, 0.f))
			{
				real t = -(distance/vector_dot_normal);

				if (vector_dot_normal>0.f)
				{
					if (t_in<t)
					{
						t_in = t;
					}
				}
				else
				{
					if (t_out>t)
					{
						t_out = t;
					}
				}

				if (t_in>t_out)
				{
					hit = FALSE;
					break;
				}
			}
			else if (distance>global_convex_hull3d_epsilon)
			{
				hit = FALSE;
				break;
			}
		}
	}

	if (hit)
	{
		if (t_in_reference)
		{
			*t_in_reference = t_in;
		}

		if (t_out_reference)
		{
			*t_out_reference = t_out;
		}
	}

	return hit;
}

/* ---------- private code */
