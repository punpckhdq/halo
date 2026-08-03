/*
COLLISION_FEATURES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "collision_features.h"
#include "collision_bsp.h"
#include "collision_bsp_definitions.h"
#include "render/render_debug.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void collision_features_new(
	struct collision_feature_list *features)
{
	memset(features->count, 0, sizeof(features->count));
	return;
}

void collision_features_from_point(
	real_point3d const *point,
	real height,
	real width,
	long object_index,
	long surface_index,
	byte flags,
	byte breakable_surface_index,
	short material_index,
	struct collision_feature_list *features)
{
	if (features->count[_collision_feature_sphere] < MAXIMUM_COLLISION_FEATURES_PER_TEST)
	{
		struct collision_sphere *sphere = &features->spheres[features->count[_collision_feature_sphere]++];
		sphere->object_index = object_index;
		sphere->surface_index = surface_index;
		sphere->flags = flags;
		sphere->breakable_surface_index = breakable_surface_index;
		sphere->material_index = material_index;
		sphere->center = *point;
		sphere->radius = width;
	}

	if (height > 0.0f)
	{
		real new_height = point->z - height;
		if (features->count[_collision_feature_sphere] < MAXIMUM_COLLISION_FEATURES_PER_TEST)
		{
			struct collision_sphere *sphere = &features->spheres[features->count[_collision_feature_sphere]++];
			sphere->object_index = object_index;
			sphere->surface_index = surface_index;
			sphere->flags = flags;
			sphere->breakable_surface_index = breakable_surface_index;
			sphere->material_index = material_index;
			set_real_point3d(&sphere->center, point->x, point->y, new_height);
			sphere->radius = width;
		}

		if (features->count[_collision_feature_cylinder] < MAXIMUM_COLLISION_FEATURES_PER_TEST)
		{
			struct collision_cylinder *cylinder = &features->cylinders[features->count[_collision_feature_cylinder]++];
			cylinder->object_index = object_index;
			cylinder->surface_index = surface_index;
			cylinder->flags = flags;
			cylinder->breakable_surface_index = breakable_surface_index;
			cylinder->material_index = material_index;
			set_real_point3d(&cylinder->base, point->x, point->y, new_height);
			set_real_vector3d(&cylinder->height, 0.0f, 0.0f, height);
			cylinder->width = width;
		}
	}

	return;
}

void collision_features_from_line(
	real_point3d const *point,
	real_vector3d const *vector,
	real height,
	real width,
	long object_index,
	long surface_index,
	byte flags,
	byte breakable_surface_index,
	short material_index,
	struct collision_feature_list *features)
{
	real_vector2d n;

	if (features->count[_collision_feature_cylinder] < MAXIMUM_COLLISION_FEATURES_PER_TEST)
	{
		struct collision_cylinder *cylinder = &features->cylinders[features->count[_collision_feature_cylinder]++];
		cylinder->object_index = object_index;
		cylinder->surface_index = surface_index;
		cylinder->flags = flags;
		cylinder->breakable_surface_index = breakable_surface_index;
		cylinder->material_index = material_index;
		cylinder->base = *point;
		cylinder->height = *vector;
		cylinder->width = width;
	}

	if (height > 0.0f)
	{
		if (features->count[_collision_feature_cylinder] < MAXIMUM_COLLISION_FEATURES_PER_TEST)
		{
			struct collision_cylinder *cylinder = &features->cylinders[features->count[_collision_feature_cylinder]++];
			cylinder->object_index = object_index;
			cylinder->surface_index = surface_index;
			cylinder->flags = flags;
			cylinder->breakable_surface_index = breakable_surface_index;
			cylinder->material_index = material_index;
			set_real_point3d(&cylinder->base, point->x, point->y, point->z - height);
			cylinder->height = *vector;
			cylinder->width = width;
		}

		perpendicular2d((real_vector2d const *)vector, &n);

		if (normalize2d(&n) != 0.0f)
		{
			real plane_distance = dot_product2d((real_vector2d const *)point, &n);
			short point_index;
			real_point3d points[4];

			points[0] = *point;
			add_vectors3d((real_vector3d const *)point, vector, (real_vector3d *)&points[1]);
			points[2] = points[1];
			points[2].z -= height;
			points[3] = *point;
			points[3].z -= height;

			if (features->count[_collision_feature_prism] < MAXIMUM_COLLISION_FEATURES_PER_TEST)
			{
				struct collision_prism *prism = &features->prisms[features->count[_collision_feature_prism]++];
				prism->object_index = object_index;
				prism->surface_index = surface_index;
				prism->flags = flags;
				prism->breakable_surface_index = breakable_surface_index;
				prism->material_index = material_index;
				set_real_vector3d(&prism->plane.n, n.i, n.j, 0.0f);
				prism->plane.d = plane_distance;
				prism->height = width;
				prism->projection_axis = projection_from_vector3d(&prism->plane.n);
				prism->projection_sign = projection_sign_from_vector3d(&prism->plane.n, prism->projection_axis);
				prism->point_count = NUMBER_OF_VERTICES_PER_QUADRILATERAL;
				for (point_index = 0; point_index < prism->point_count; point_index++)
				{
					project_point3d(&points[point_index], prism->projection_axis, prism->projection_sign, &prism->points[point_index]);
				}
			}

			{
				real_point3d temp = points[1];
				points[1] = points[3];
				points[3] = temp;
			}

			if (features->count[_collision_feature_prism] < MAXIMUM_COLLISION_FEATURES_PER_TEST)
			{
				struct collision_prism *prism = &features->prisms[features->count[_collision_feature_prism]++];
				prism->object_index = object_index;
				prism->surface_index = surface_index;
				prism->flags = flags;
				prism->breakable_surface_index = breakable_surface_index;
				prism->material_index = material_index;
				set_real_vector3d(&prism->plane.n, -n.i, -n.j, 0.0f);
				prism->plane.d = -plane_distance;
				prism->height = width;
				prism->projection_axis = projection_from_vector3d(&prism->plane.n);
				prism->projection_sign = projection_sign_from_vector3d(&prism->plane.n, prism->projection_axis);
				prism->point_count = NUMBER_OF_VERTICES_PER_QUADRILATERAL;
				for (point_index = 0; point_index < prism->point_count; point_index++)
				{
					project_point3d(&points[point_index], prism->projection_axis, prism->projection_sign, &prism->points[point_index]);
				}
			}
		}
	}

	return;
}

void collision_features_from_polygon(
	short point_count,
	real_point3d const *points,
	real_plane3d const *plane,
	real height,
	real width,
	long object_index,
	long surface_index,
	byte flags,
	byte breakable_surface_index,
	short material_index,
	struct collision_feature_list *features)
{
	match_assert("c:\\halo\\SOURCE\\physics\\collision_features.c", 241, point_count<=MAXIMUM_POINTS_PER_COLLISION_PRISM);

	if (features->count[_collision_feature_prism] < MAXIMUM_COLLISION_FEATURES_PER_TEST)
	{
		struct collision_prism *prism = &features->prisms[features->count[_collision_feature_prism]++];
		short point_index;

		prism->object_index = object_index;
		prism->surface_index = surface_index;
		prism->flags = flags;
		prism->breakable_surface_index = breakable_surface_index;
		prism->material_index = material_index;
		prism->plane = *plane;
		prism->height = width;
		prism->projection_axis = projection_from_vector3d(&prism->plane.n);
		prism->projection_sign = projection_sign_from_vector3d(&prism->plane.n, prism->projection_axis);
		prism->point_count = point_count;

		for (point_index = 0; point_index < prism->point_count; point_index++)
		{
			project_point3d(&points[point_index], prism->projection_axis, prism->projection_sign, &prism->points[point_index]);
		}

		if (height > 0.0f && plane->n.k < 0.0f)
		{
			prism->plane.d -= height * prism->plane.n.k;
			if (prism->projection_axis != _z)
			{
				short component = global_projection3d_mappings[prism->projection_axis][prism->projection_sign][1] == _z;
				match_assert("c:\\halo\\SOURCE\\physics\\collision_features.c", 283, global_projection3d_mappings[prism->projection_axis][prism->projection_sign][component]==_z);
				for (point_index = 0; point_index < prism->point_count; point_index++)
				{
					prism->points[point_index].n[component] -= height;
				}
			}
		}
	}

	return;
}

void collision_features_from_vertex(
	struct collision_bsp const *bsp,
	long vertex_index,
	real_matrix4x3 const *matrix,
	real height,
	real width,
	long object_index,
	struct collision_feature_list *features)
{
	struct collision_vertex const *vertex;
	struct collision_edge const *edge;
	struct collision_surface const *surface;
	real_point3d point;
	real_point3d const *feature_point;
	long surface_index;

	vertex = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, vertex_index, struct collision_vertex);
	edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, vertex->first_edge_index, struct collision_edge);
	surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, edge->surface_indices[0], struct collision_surface);
	surface_index = object_index != NONE ? NONE : edge->surface_indices[0];

	if (matrix)
	{
		feature_point = matrix4x3_transform_point(matrix, &vertex->point, &point);
	}
	else
	{
		feature_point = &vertex->point;
	}

	collision_features_from_point(
		feature_point,
		height,
		width,
		object_index,
		surface_index,
		surface->flags,
		surface->breakable_surface_index,
		surface->material_index,
		features);

	return;
}

void collision_features_from_edge(
	struct collision_bsp const *bsp,
	long edge_index,
	real_matrix4x3 const *matrix,
	real height,
	real width,
	long object_index,
	struct collision_feature_list *features)
{
	struct collision_edge const *edge;
	struct collision_surface const *surface0;
	struct collision_surface const *surface1;
	struct collision_vertex const *vertex0;
	struct collision_vertex const *vertex1;
	real_plane3d const *plane0;
	real_plane3d const *plane1;
	real_point3d point;
	real_point3d const *feature_point;
	real_vector3d vector;
	real_vector3d const *feature_vector;
	boolean plane0_negated;
	boolean plane1_negated;
	boolean valid_edge;
	long plane0_index;
	long plane1_index;
	long surface_index;

	edge = TAG_BLOCK_GET_ELEMENT(&bsp->edges, edge_index, struct collision_edge);
	surface0 = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, edge->surface_indices[0], struct collision_surface);
	surface1 = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, edge->surface_indices[1], struct collision_surface);

	if (surface0->plane_designator != surface1->plane_designator)
	{
		vertex0 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[0], struct collision_vertex);
		vertex1 = TAG_BLOCK_GET_ELEMENT(&bsp->vertices, edge->vertex_indices[1], struct collision_vertex);
		plane0_index = surface0->plane_designator & LONG_MAX;
		plane1_index = surface1->plane_designator & LONG_MAX;
		plane0 = TAG_BLOCK_GET_ELEMENT(&bsp->bsp3d.planes, plane0_index, real_plane3d);
		plane1 = TAG_BLOCK_GET_ELEMENT(&bsp->bsp3d.planes, plane1_index, real_plane3d);
		plane0_negated = (surface0->plane_designator & LONG_MIN) != 0;
		plane1_negated = (surface1->plane_designator & LONG_MIN) != 0;
		vector_from_points3d(&vertex0->point, &vertex1->point, &vector);

		if (plane0_index == plane1_index)
		{
			valid_edge = TRUE;
		}
		else if (plane0_negated == plane1_negated)
		{
			valid_edge = triple_product3d(&plane0->n, &plane1->n, &vector) > -_real_epsilon;
		}
		else
		{
			valid_edge = triple_product3d(&plane0->n, &plane1->n, &vector) < _real_epsilon;
		}

		if (valid_edge)
		{
			surface_index = object_index == NONE ? edge->surface_indices[0] : NONE;

			if (matrix)
			{
				feature_vector = matrix4x3_transform_vector(matrix, &vector, &vector);
			}
			else
			{
				feature_vector = &vector;
			}

			if (matrix)
			{
				feature_point = matrix4x3_transform_point(matrix, &vertex0->point, &point);
			}
			else
			{
				feature_point = &vertex0->point;
			}

			collision_features_from_line(
				feature_point,
				feature_vector,
				height,
				width,
				object_index,
				surface_index,
				surface0->flags,
				surface0->breakable_surface_index,
				surface0->material_index,
				features);
		}
	}

	return;
}

void collision_features_from_surface(
	struct collision_bsp const *bsp,
	long surface_index,
	real_matrix4x3 const *matrix,
	real height,
	real width,
	long object_index,
	struct collision_feature_list *features)
{
	struct collision_surface const *surface;
	real_plane3d plane;
	real_point3d points[MAXIMUM_VERTICES_PER_COLLISION_SURFACE];
	short point_count;
	short point_index;
	long feature_surface_index;

	surface = TAG_BLOCK_GET_ELEMENT(&bsp->surfaces, surface_index, struct collision_surface);
	point_count = collision_surface_polygon(bsp, surface_index, points);
	bsp3d_get_plane_from_designator(&bsp->bsp3d, surface->plane_designator, &plane);

	if (matrix)
	{
		for (point_index = 0; point_index < point_count; point_index++)
		{
			matrix4x3_transform_point(matrix, &points[point_index], &points[point_index]);
		}

		matrix4x3_transform_plane(matrix, &plane, &plane);
	}

	feature_surface_index = object_index != NONE ? NONE : surface_index;
	collision_features_from_polygon(
		point_count,
		points,
		&plane,
		height,
		width,
		object_index,
		feature_surface_index,
		surface->flags,
		surface->breakable_surface_index,
		surface->material_index,
		features);

	return;
}

void collision_bsp_get_features_in_sphere(
	struct collision_bsp const *bsp,
	struct collision_bsp_test_sphere_result const *result,
	real_matrix4x3 const *matrix,
	real height,
	real width,
	long object_index,
	struct collision_feature_list *features)
{
	match_assert("c:\\halo\\SOURCE\\physics\\collision_features.c", 429, features->count[_collision_feature_sphere]>=0 && features->count[_collision_feature_sphere]<=MAXIMUM_COLLISION_FEATURES_PER_TEST);
	match_assert("c:\\halo\\SOURCE\\physics\\collision_features.c", 430, features->count[_collision_feature_cylinder]>=0 && features->count[_collision_feature_cylinder]<=MAXIMUM_COLLISION_FEATURES_PER_TEST);
	match_assert("c:\\halo\\SOURCE\\physics\\collision_features.c", 431, features->count[_collision_feature_prism]>=0 && features->count[_collision_feature_prism]<=MAXIMUM_COLLISION_FEATURES_PER_TEST);

	{
		long feature_index;
		for (feature_index = 0; feature_index < result->vertex_count; feature_index++)
		{
			collision_features_from_vertex(bsp, result->vertex_indices[feature_index], matrix, height, width, object_index, features);
		}
	}

	{
		long feature_index;
		for (feature_index = 0; feature_index < result->edge_count; feature_index++)
		{
			collision_features_from_edge(bsp, result->edge_indices[feature_index], matrix, height, width, object_index, features);
		}
	}

	{
		long feature_index;
		for (feature_index = 0; feature_index < result->surface_count; feature_index++)
		{
			collision_features_from_surface(bsp, result->surface_indices[feature_index], matrix, height, width, object_index, features);
		}
	}

	return;
}

boolean collision_features_test_point(
	struct collision_feature_list const *features,
	real_point3d const *point,
	struct collision_plane *collision)
{
	real_plane3d deepest_feature_plane;
	real_plane3d feature_plane;
	real deepest_feature_depth;
	real feature_depth;
	short deepest_feature_type;
	short deepest_feature_index;
	short feature_type;
	short feature_index;

	deepest_feature_type = NONE;
	deepest_feature_index = NONE;
	deepest_feature_depth = REAL_MIN;

	for (feature_type = 0; feature_type < NUMBER_OF_COLLISION_FEATURE_TYPES; feature_type++)
	{
		for (feature_index = 0; feature_index < features->count[feature_type]; feature_index++)
		{
			if (((feature_type == _collision_feature_sphere && collision_sphere_test_point(&features->spheres[feature_index], point, &feature_depth, &feature_plane)) ||
				(feature_type == _collision_feature_cylinder && collision_cylinder_test_point(&features->cylinders[feature_index], point, &feature_depth, &feature_plane)) ||
				(feature_type == _collision_feature_prism && collision_prism_test_point(&features->prisms[feature_index], point, &feature_depth, &feature_plane))) && deepest_feature_depth < feature_depth)
			{
				deepest_feature_type = feature_type;
				deepest_feature_index = feature_index;
				deepest_feature_depth = feature_depth;
				deepest_feature_plane = feature_plane;
			}
		}
	}

	if (deepest_feature_type != NONE)
	{
		collision->t = deepest_feature_depth;
		collision->plane = deepest_feature_plane;
		switch (deepest_feature_type)
		{
		case _collision_feature_sphere:
		{
			struct collision_sphere const *sphere = &features->spheres[deepest_feature_index];

			collision->object_index = sphere->object_index;
			collision->surface_index = sphere->surface_index;
			collision->flags = sphere->flags;
			collision->breakable_surface_index = sphere->breakable_surface_index;
			collision->material_index = sphere->material_index;
			break;
		}
		case _collision_feature_cylinder:
		{
			struct collision_cylinder const *cylinder = &features->cylinders[deepest_feature_index];

			collision->object_index = cylinder->object_index;
			collision->surface_index = cylinder->surface_index;
			collision->flags = cylinder->flags;
			collision->breakable_surface_index = cylinder->breakable_surface_index;
			collision->material_index = cylinder->material_index;
			break;
		}
		case _collision_feature_prism:
		{
			struct collision_prism const *prism = &features->prisms[deepest_feature_index];

			collision->object_index = prism->object_index;
			collision->surface_index = prism->surface_index;
			collision->flags = prism->flags;
			collision->breakable_surface_index = prism->breakable_surface_index;
			collision->material_index = prism->material_index;
			break;
		}
		default:
			return TRUE;
		}

		return TRUE;
	}

	return FALSE;
}

boolean collision_features_test_vector(
	struct collision_feature_list const *features,
	real_point3d const *point,
	real_vector3d const *vector,
	struct collision_plane *collision)
{
	struct collision_feature const *feature;
	real_plane3d closest_feature_plane;
	real_plane3d feature_plane;
	real closest_feature_t;
	real feature_t;
	short closest_feature_type;
	short closest_feature_index;
	short feature_type;
	short feature_index;

	closest_feature_type = NONE;
	closest_feature_index = NONE;
	closest_feature_t = REAL_MAX;

	for (feature_type = 0; feature_type < NUMBER_OF_COLLISION_FEATURE_TYPES; feature_type++)
	{
		for (feature_index = 0; feature_index < features->count[feature_type]; feature_index++)
		{
			if (((feature_type == _collision_feature_sphere && collision_sphere_test_vector(&features->spheres[feature_index], point, vector, &feature_t, &feature_plane)) ||
				(feature_type == _collision_feature_cylinder && collision_cylinder_test_vector(&features->cylinders[feature_index], point, vector, &feature_t, &feature_plane)) ||
				(feature_type == _collision_feature_prism && collision_prism_test_vector(&features->prisms[feature_index], point, vector, &feature_t, &feature_plane))) &&
				closest_feature_t > feature_t)
			{
				if (dot_product3d(vector, &feature_plane.n) < -_real_epsilon)
				{
					closest_feature_t = feature_t;
					closest_feature_type = feature_type;
					closest_feature_index = feature_index;
					closest_feature_plane = feature_plane;
				}
			}
		}
	}

	if (closest_feature_type != NONE)
	{
		collision->t = closest_feature_t;
		point_from_line3d(point, vector, closest_feature_t, &collision->point);
		collision->plane = closest_feature_plane;
		switch (closest_feature_type)
		{
		case _collision_feature_sphere:
			feature = (struct collision_feature const *)&features->spheres[closest_feature_index];
			break;
		case _collision_feature_cylinder:
			feature = (struct collision_feature const *)&features->cylinders[closest_feature_index];
			break;
		case _collision_feature_prism:
			feature = (struct collision_feature const *)&features->prisms[closest_feature_index];
			break;
		default:
			return TRUE;
		}

		collision->object_index = feature->object_index;
		collision->surface_index = feature->surface_index;
		collision->flags = feature->flags;
		collision->breakable_surface_index = feature->breakable_surface_index;
		collision->material_index = feature->material_index;

		return TRUE;
	}

	collision->t = 1.0f;
	collision->point.x = point->x + vector->i;
	collision->point.y = point->y + vector->j;
	collision->point.z = point->z + vector->k;

	return FALSE;
}

void render_debug_collision_features(
	struct collision_feature_list const *features)
{
	short feature_index;

	match_assert("c:\\halo\\SOURCE\\physics\\collision_features.c", 1108, features->count[_collision_feature_sphere]<=MAXIMUM_COLLISION_FEATURES_PER_TEST);
	match_assert("c:\\halo\\SOURCE\\physics\\collision_features.c", 1109, features->count[_collision_feature_cylinder]<=MAXIMUM_COLLISION_FEATURES_PER_TEST);
	match_assert("c:\\halo\\SOURCE\\physics\\collision_features.c", 1110, features->count[_collision_feature_prism]<=MAXIMUM_COLLISION_FEATURES_PER_TEST);

	for (feature_index = 0; feature_index < features->count[_collision_feature_prism]; feature_index++)
	{
		render_debug_collision_prism(&features->prisms[feature_index], global_real_argb_blue);
	}

	for (feature_index = 0; feature_index < features->count[_collision_feature_cylinder]; feature_index++)
	{
		render_debug_collision_cylinder(&features->cylinders[feature_index], global_real_argb_green);
	}

	for (feature_index = 0; feature_index < features->count[_collision_feature_sphere]; feature_index++)
	{
		render_debug_collision_sphere(&features->spheres[feature_index], global_real_argb_red);
	}

	return;
}

/* ---------- private code */

boolean collision_sphere_test_point(
	struct collision_sphere const *sphere,
	real_point3d const *point,
	real *depth,
	real_plane3d *plane)
{
	real_vector3d w;
	real distance_squared;
	real distance;

	vector_from_points3d(&sphere->center, point, &w);
	distance_squared = magnitude_squared3d(&w);
	if (distance_squared < sphere->radius * sphere->radius)
	{
		distance = square_root(distance_squared);
		if (distance > 0.0f)
		{
			scale_vector3d(&w, 1.0f / distance, &plane->n);
		}
		else
		{
			set_real_vector3d(&plane->n, 0.0f, 0.0f, 1.0f);
		}

		plane->d = dot_product3d((real_vector3d const *)&sphere->center, &plane->n) + sphere->radius;
		*depth = sphere->radius - distance;
		return TRUE;
	}

	return FALSE;
}

boolean collision_cylinder_test_point(
	struct collision_cylinder const *cylinder,
	real_point3d const *point,
	real *depth,
	real_plane3d *plane)
{
	real_vector3d w;
	real_vector3d scaled_height;
	real height_squared;
	real height_distance;
	real radial_distance;

	vector_from_points3d(&cylinder->base, point, &w);
	height_distance = dot_product3d(&w, &cylinder->height);
	if (height_distance >= 0.0f)
	{
		height_squared = magnitude_squared3d(&cylinder->height);
		if (height_distance <= height_squared &&
			magnitude_squared3d(&w) * height_squared - height_distance * height_distance < cylinder->width * cylinder->width * height_squared)
		{
			if (height_squared > 0.0f)
			{
				scale_vector3d(&cylinder->height, height_distance / height_squared, &scaled_height);
				subtract_vectors3d(&w, &scaled_height, &plane->n);
			}
			else
			{
				plane->n = w;
			}

			radial_distance = normalize3d(&plane->n);
			if (radial_distance == 0.0f)
			{
				set_real_vector3d(&plane->n, 0.0f, 0.0f, 1.0f);
			}

			plane->d = dot_product3d((real_vector3d const *)&cylinder->base, &plane->n) + cylinder->width;
			*depth = cylinder->width - radial_distance;
			return TRUE;
		}
	}

	return FALSE;
}

boolean collision_prism_test_point(
	struct collision_prism const *prism,
	real_point3d const *point,
	real *depth,
	real_plane3d *plane)
{
	real_point3d point_on_plane;
	real_point2d point2d;
	real_vector2d vector0;
	real_vector2d vector1;
	real distance;
	long point_index;
	long next_point_index;

	distance = plane3d_distance_to_point(&prism->plane, point);
	if (distance >= 0.0f)
	{
		if (distance < prism->height)
		{
			point_from_line3d(point, &prism->plane.n, -distance, &point_on_plane);
			project_point3d(&point_on_plane, prism->projection_axis, prism->projection_sign, &point2d);

			for (point_index = 0; point_index < prism->point_count; point_index++)
			{
				next_point_index = point_index + 1 < prism->point_count ? point_index + 1 : 0;
				vector_from_points2d(&point2d, &prism->points[point_index], &vector0);
				vector_from_points2d(&point2d, &prism->points[next_point_index], &vector1);
				if (cross_product2d(&vector0, &vector1) < 0.0f)
				{
					return FALSE;
				}
			}

			plane->n = prism->plane.n;
			plane->d = prism->plane.d + prism->height;
			*depth = prism->height - distance;

			return TRUE;
		}
	}

	return FALSE;
}

boolean collision_sphere_test_vector(
	struct collision_sphere const *sphere,
	real_point3d const *point,
	real_vector3d const *vector,
	real *t,
	real_plane3d *plane)
{
	real_vector3d w;
	real_vector3d point_on_vector;
	real distance;
	real projection;
	real vector_squared;
	real discriminant;
	boolean result;

	result = FALSE;
	vector_from_points3d(point, &sphere->center, &w);
	distance = magnitude_squared3d(&w) - sphere->radius * sphere->radius;
	if (distance <= 0.0f)
	{
		*t = 0.0f;
		result = TRUE;
	}
	else
	{
		projection = dot_product3d(&w, vector);
		if (projection > 0.0f)
		{
			vector_squared = magnitude_squared3d(vector);
			discriminant = projection * projection - vector_squared * distance;
			if (discriminant >= 0.0f)
			{
				projection -= square_root(discriminant);
				if (projection <= vector_squared)
				{
					*t = projection / vector_squared;
					result = TRUE;
				}
			}
		}
	}

	if (result)
	{
		scale_vector3d(vector, *t, &point_on_vector);
		subtract_vectors3d(&point_on_vector, &w, &plane->n);
		if (normalize3d(&plane->n) == 0.0f)
		{
			set_real_vector3d(&plane->n, 0.0f, 0.0f, 1.0f);
		}

		plane->d = dot_product3d((real_vector3d const *)&sphere->center, &plane->n) + sphere->radius;
	}

	return result;
}

boolean collision_cylinder_test_vector(
	struct collision_cylinder const *cylinder,
	real_point3d const *point,
	real_vector3d const *vector,
	real *t,
	real_plane3d *plane)
{
	real_vector3d w;
	real_point3d point_on_vector;
	real height_squared;
	real height_vector;
	real height_w;
	real vector_squared;
	real vector_w;
	real w_squared;
	real width_squared;
	real quadratic_a;
	real quadratic_b;
	real quadratic_c;
	real discriminant;
	real inverse_quadratic_a;
	real minimum_t;
	real maximum_t;
	real bottom_t;
	real top_t;
	real height_t;

	height_squared = magnitude_squared3d(&cylinder->height);
	height_vector = dot_product3d(&cylinder->height, vector);
	vector_squared = magnitude_squared3d(vector);
	quadratic_a = height_squared * vector_squared - height_vector * height_vector;
	if (quadratic_a == 0.0f)
	{
		return FALSE;
	}

	vector_from_points3d(&cylinder->base, point, &w);
	vector_w = dot_product3d(vector, &w);
	height_w = dot_product3d(&cylinder->height, &w);
	w_squared = magnitude_squared3d(&w);
	width_squared = cylinder->width * cylinder->width;
	quadratic_b = height_w * height_vector - height_squared * vector_w;
	quadratic_c = (w_squared - width_squared) * height_squared - height_w * height_w;
	discriminant = quadratic_b * quadratic_b - quadratic_a * quadratic_c;
	if (discriminant < 0.0f)
	{
		return FALSE;
	}

	discriminant = square_root(discriminant);
	inverse_quadratic_a = 1.0f / quadratic_a;
	minimum_t = (quadratic_b - discriminant) * inverse_quadratic_a;
	maximum_t = (quadratic_b + discriminant) * inverse_quadratic_a;
	if (minimum_t > 1.0f || maximum_t < 0.0f)
	{
		return FALSE;
	}

	if (minimum_t < 0.0f)
	{
		minimum_t = 0.0f;
	}
	if (maximum_t > 1.0f)
	{
		maximum_t = 1.0f;
	}
	if (height_vector != 0.0f)
	{
		bottom_t = -height_w / height_vector;
		top_t = (height_squared - height_w) / height_vector;
		if (height_vector > 0.0f)
		{
			if (minimum_t < bottom_t)
			{
				minimum_t = bottom_t;
			}
			maximum_t = MIN(maximum_t, top_t);
		}
		else
		{
			if (minimum_t < top_t)
			{
				minimum_t = top_t;
			}
			maximum_t = MIN(maximum_t, bottom_t);
		}

		if (minimum_t > maximum_t)
		{
			return FALSE;
		}
	}
	else if (height_w < 0.0f || height_w > height_squared)
	{
		return FALSE;
	}

	*t = minimum_t;
	point_from_line3d((real_point3d const *)&w, vector, minimum_t, &point_on_vector);
	height_t = -(dot_product3d((real_vector3d const *)&point_on_vector, &cylinder->height) / height_squared);
	point_from_line3d(&point_on_vector, &cylinder->height, height_t, (real_point3d *)&plane->n);
	if (normalize3d(&plane->n) == 0.0f)
	{
		set_real_vector3d(&plane->n, 1.0f, 0.0f, 0.0f);
	}

	plane->d = dot_product3d((real_vector3d const *)&cylinder->base, &plane->n) + cylinder->width;

	return TRUE;
}

boolean collision_prism_test_vector(
	struct collision_prism const *prism,
	real_point3d const *point,
	real_vector3d const *vector,
	real *t,
	real_plane3d *plane)
{

	real t_out = 0.0f;
	real t_in = 1.0f;
	real d = plane3d_distance_to_point(&prism->plane, point);
	real vn = dot_product3d(vector, &prism->plane.n);

	if (vn != 0.0f)
	{
		real oovn = 1.0f / vn;
		real t0 = -oovn * d;
		real t1 = -oovn * (d - prism->height);

		if (vn > 0.0f)
		{
			if (t_out < t0)
			{
				t_out = t0;
			}
			t_in = MIN(t_in, t1);
		}
		else
		{
			if (t_out < t1)
			{
				t_out = t1;
			}
			t_in = MIN(t_in, t0);
		}

		if (t_out > t_in)
		{
			return FALSE;
		}
	}
	else if (d < 0.0f || d >= prism->height)
	{
		return FALSE;
	}

	{
		real_point3d p3d;
		real_vector3d v3d;
		real_point2d p2d;
		real_vector2d v2d;
		long point_index;

		point_from_line3d(point, &prism->plane.n, -d, &p3d);
		point_from_line3d((real_point3d const *)vector, &prism->plane.n, -vn, (real_point3d *)&v3d);
		project_point3d(&p3d, prism->projection_axis, prism->projection_sign, &p2d);
		project_point3d((real_point3d const *)&v3d, prism->projection_axis, prism->projection_sign, (real_point2d *)&v2d);

		for (point_index = 0; point_index < prism->point_count; point_index++)
		{
			real vw;
			real wx;
			real_vector2d w;
			real_vector2d x;

			long next_point_index = point_index + 1 >= prism->point_count ? 0 : point_index + 1;
			real_point2d const *p0 = &prism->points[point_index];
			real_point2d const *p1 = &prism->points[next_point_index];

			vector_from_points2d(p0, p1, &w);
			vector_from_points2d(p0, &p2d, &x);
			vw = cross_product2d(&v2d, &w);
			wx = cross_product2d(&w, &x);

			if (vw != 0.0f)
			{
				real t_edge = wx / vw;
				
				if (vw < 0.0f)
				{
					if (t_out < t_edge)
					{
						t_out = t_edge;
					}
				}
				else
				{
					if (t_in > t_edge)
					{
						t_in = t_edge;
					}
				}

				if (t_out > t_in)
				{
					return FALSE;
				}
			}
			else
			{
				if (wx < 0.0f)
				{
					return FALSE;
				}
			}
		}

		*t = t_out;
		plane->n = prism->plane.n;
		plane->d = prism->plane.d + prism->height;
	}

	return TRUE;
}

void render_debug_collision_sphere(
	struct collision_sphere const *sphere,
	real_argb_color const *color)
{
	render_debug_sphere(TRUE, &sphere->center, sphere->radius, color);
	return;
}

void render_debug_collision_cylinder(
	struct collision_cylinder const *cylinder,
	real_argb_color const *color)
{
	render_debug_cylinder(TRUE, &cylinder->base, &cylinder->height, cylinder->width, color);
	return;
}

void render_debug_collision_prism(
	struct collision_prism const *prism,
	real_argb_color const *color)
{
	real_point3d points[MAXIMUM_POINTS_PER_COLLISION_PRISM];
	short point_index;

	match_assert("c:\\halo\\SOURCE\\physics\\collision_features.c", 1079, prism->point_count<=MAXIMUM_POINTS_PER_COLLISION_PRISM);

	for (point_index = 0; point_index < prism->point_count; point_index++)
	{
		project_point2d(&prism->points[point_index], &prism->plane, prism->projection_axis, prism->projection_sign, &points[point_index]);
		point_from_line3d(&points[point_index], &prism->plane.n, prism->height, &points[point_index]);
	}

	for (point_index = 0; point_index < prism->point_count; point_index++)
	{
		render_debug_line(TRUE, &points[point_index], &points[(point_index + 1) % prism->point_count], color);
	}

	return;
}
