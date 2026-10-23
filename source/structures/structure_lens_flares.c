/*
STRUCTURE_LENS_FLARES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "structures.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "connected_geometry.h"
#include "intermediate_geometry.h"
#include "bsp3d.h"
#include "shader_definitions.h"

/* ---------- constants */

enum
{
	MAXIMUM_POINTS_PER_CONNECTED_GEOMETRY_COPLANAR_GROUP = MAXIMUM_TRIANGLES_PER_CONNECTED_GEOMETRY_COPLANAR_GROUP*NUMBER_OF_VERTICES_PER_TRIANGLE, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct temporary_lens_flare_marker /* fake name */
{
	struct structure_lens_flare_marker marker;
	short cluster_index;
	word pad;
};

/* ---------- prototypes */

static int compare_temp_markers(void const *a, void const *b);

/* ---------- globals */

/* ---------- public code */

static int compare_temp_markers(
	void const *a,
	void const *b)
{
	struct temporary_lens_flare_marker const *marker_a = a;
	struct temporary_lens_flare_marker const *marker_b = b;

	return marker_a->cluster_index > marker_b->cluster_index ? 1 : -1;
}

long cluster_index_from_point(
	struct structure_bsp const *structure_bsp,
	real_point3d const *point)
{
	long leaf_index = bsp3d_test_point(
		&TAG_BLOCK_GET_ELEMENT(&structure_bsp->collision_bsp, 0, struct collision_bsp)->bsp3d,
		0,
		point);
	long result = NONE;

	if (leaf_index != NONE)
	{
		result = TAG_BLOCK_GET_ELEMENT(&structure_bsp->leaves, leaf_index & LONG_MAX, struct structure_leaf)->cluster_index;
	}

	return result;
}

boolean build_structure_lens_flares(
	struct structure_bsp *structure_bsp)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 66, structure_bsp);

	fprintf(stdout, "building structure lens flares...");
	fflush(stdout);

	if (tag_block_resize(&structure_bsp->lens_flares, 0) && tag_block_resize(&structure_bsp->lens_flare_markers, 0))
	{
		real_point2d *projected_points = match_malloc("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 74, MAXIMUM_POINTS_PER_CONNECTED_GEOMETRY_COPLANAR_GROUP * sizeof(real_point2d));
		real_point3d *points = match_malloc("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 75, MAXIMUM_POINTS_PER_CONNECTED_GEOMETRY_COPLANAR_GROUP * sizeof(real_point3d));
		short *hull_indices = match_malloc("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 76, MAXIMUM_POINTS_PER_CONNECTED_GEOMETRY_COPLANAR_GROUP * sizeof(short));

		if (structure_bsp && projected_points && points && hull_indices)
		{
			struct temporary_lens_flare_marker *temp_markers;
			short cluster_index;
			short lightmap_index;

			for (cluster_index = 0; cluster_index < structure_bsp->clusters.count; cluster_index++)
			{
				TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_index, struct structure_cluster)->first_lens_flare_marker_index = 0;
				TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_index, struct structure_cluster)->lens_flare_marker_count = 0;
			}

			for (lightmap_index = 0; lightmap_index < structure_bsp->lightmaps.count; lightmap_index++)
			{
				short material_index;
				struct structure_lightmap *lightmap = TAG_BLOCK_GET_ELEMENT(&structure_bsp->lightmaps, lightmap_index, struct structure_lightmap);

				for (material_index = 0; material_index < lightmap->materials.count; material_index++)
				{
					struct structure_material *material = TAG_BLOCK_GET_ELEMENT(&lightmap->materials, material_index, struct structure_material);

					if (material->shader.index != NONE)
					{
						real lens_flare_spacing;
						struct shader *shader = shader_definition_get(material->shader.index);
						struct tag_reference *lens_flare = NULL;

						switch (shader->base.type)
						{
						case _shader_type_environment:
							lens_flare = &((struct shader_environment *)shader_get_and_verify_type(shader, _shader_type_environment))->environment.lens_flare;
							lens_flare_spacing = ((struct shader_environment *)shader_get_and_verify_type(shader, _shader_type_environment))->environment.lens_flare_spacing;
							break;
						case _shader_type_transparent_generic:
							lens_flare = &((struct shader_transparent_generic *)shader_get_and_verify_type(shader, _shader_type_transparent_generic))->generic.lens_flare;
							lens_flare_spacing = ((struct shader_transparent_generic *)shader_get_and_verify_type(shader, _shader_type_transparent_generic))->generic.lens_flare_spacing;
							break;
						case _shader_type_transparent_chicago:
							lens_flare = &((struct shader_transparent_chicago *)shader_get_and_verify_type(shader, _shader_type_transparent_chicago))->chicago.lens_flare;
							lens_flare_spacing = ((struct shader_transparent_chicago *)shader_get_and_verify_type(shader, _shader_type_transparent_chicago))->chicago.lens_flare_spacing;
							break;
						}

						if (lens_flare && lens_flare->index != NONE)
						{
							short lens_flare_index;

							for (lens_flare_index = 0; lens_flare_index < structure_bsp->lens_flares.count; lens_flare_index++)
							{
								if (TAG_BLOCK_GET_ELEMENT(&structure_bsp->lens_flares, lens_flare_index, struct structure_lens_flare)->lens_flare.index == lens_flare->index)
								{
									break;
								}
							}

							if (structure_bsp->lens_flares.count == lens_flare_index)
							{
								if (structure_bsp->lens_flares.count < MAXIMUM_LENS_FLARES_PER_STRUCTURE)
								{
									struct structure_lens_flare *structure_lens_flare = TAG_BLOCK_GET_ELEMENT(&structure_bsp->lens_flares, tag_block_add_element(&structure_bsp->lens_flares), struct structure_lens_flare);

									tag_reference_set(&structure_lens_flare->lens_flare, lens_flare->group_tag, lens_flare->name);
									structure_lens_flare->lens_flare.index = lens_flare->index;
								}
								else
								{
									error(
										_error_silent,
										"### WARNING failed to add lens flare to structure_bsp (max=#%d)",
										MAXIMUM_LENS_FLARES_PER_STRUCTURE);
									lens_flare_index = NONE;
								}
							}

							if (lens_flare_index != NONE)
							{
								struct connected_geometry geometry;
								long coplanar_group_count;
								long coplanar_group_index;
								long surface_index;
								struct environment_vertex_uncompressed *vertices = material->uncompressed_vertex_data.address;

								connected_geometry_new(&geometry);

								for (surface_index = 0; surface_index < material->surface_count; surface_index++)
								{
									struct structure_surface *surface = TAG_BLOCK_GET_ELEMENT(&structure_bsp->surfaces, material->first_surface_index + surface_index, struct structure_surface);

									match_assert("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 179, surface->vertex_indices[0]>=0 && surface->vertex_indices[0]<material->vertices.count);
									match_assert("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 180, surface->vertex_indices[1]>=0 && surface->vertex_indices[1]<material->vertices.count);
									match_assert("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 181, surface->vertex_indices[2]>=0 && surface->vertex_indices[2]<material->vertices.count);

									connected_geometry_add_triangle(
										&geometry,
										&vertices[surface->vertex_indices[0]].position,
										&vertices[surface->vertex_indices[1]].position,
										&vertices[surface->vertex_indices[2]].position,
										FALSE);
								}

								coplanar_group_count = connected_geometry_group_coplanar(&geometry);

								for (coplanar_group_index = 0; coplanar_group_index < coplanar_group_count; coplanar_group_index++)
								{
									real_plane3d plane;
									short triangle_index;
									short hull_count;
									short projection;
									boolean sign;
									short point_count = 0;

									for (triangle_index = 0; triangle_index < geometry.triangles.count; triangle_index++)
									{
										struct connected_triangle *triangle = dynamic_array_get_element(&geometry.triangles, triangle_index, sizeof(struct connected_triangle));

										if (triangle->group_index == coplanar_group_index)
										{
											real_point3d *point0 = CONNECTED_GEOMETRY_GET_EDGE_POINT(&geometry, triangle->edge_designators[0]);
											real_point3d *point1 = CONNECTED_GEOMETRY_GET_EDGE_POINT(&geometry, triangle->edge_designators[1]);
											real_point3d *point2 = CONNECTED_GEOMETRY_GET_EDGE_POINT(&geometry, triangle->edge_designators[2]);

											if (!point_count)
											{
												plane3d_from_points(&plane, point2, point1, point0);
												projection = projection_from_vector3d(&plane.n);
												sign = projection_sign_from_vector3d(&plane.n, projection);
											}

											match_assert("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 218, point_count+2<MAXIMUM_TRIANGLES_PER_CONNECTED_GEOMETRY_COPLANAR_GROUP*NUMBER_OF_VERTICES_PER_TRIANGLE);

											points[point_count] = *point0;
											points[point_count + 1] = *point1;
											points[point_count + 2] = *point2;
											project_point3d(point0, projection, sign, &projected_points[point_count]);
											project_point3d(point1, projection, sign, &projected_points[point_count + 1]);
											project_point3d(point2, projection, sign, &projected_points[point_count + 2]);
											point_count += 3;
										}
									}

									hull_count = convex_hull2d(point_count, projected_points, hull_indices);

									if (hull_count >= 3)
									{
										real_plane3d s_plane;
										real_plane3d t_plane;
										real_rectangle2d bounds;
										rectangle2d grid_bounds;
										short hull_index;
										short t;
										real_point3d center = *global_origin3d;

										for (hull_index = 0; hull_index < hull_count; hull_index++)
										{
											real_point3d *point = &points[hull_indices[hull_index]];

											center.x += point->x;
											center.y += point->y;
											center.z += point->z;
										}

										scale_point3d(&center, 1.f / hull_count, &center);
										vector_from_points3d(&points[0], &points[1], &s_plane.n);
										normalize3d(&s_plane.n);
										cross_product3d(&plane.n, &s_plane.n, &t_plane.n);
										plane3d_from_point_and_normal(&s_plane, &center, &s_plane.n);
										plane3d_from_point_and_normal(&t_plane, &center, &t_plane.n);
										bounds.x1 = plane3d_distance_to_point(&s_plane, &points[hull_indices[0]]);
										bounds.x0 = bounds.x1;
										bounds.y1 = plane3d_distance_to_point(&t_plane, &points[hull_indices[0]]);
										bounds.y0 = bounds.y1;

										for (hull_index = 0; hull_index < hull_count; hull_index++)
										{
											real_point3d *point = &points[hull_indices[hull_index]];
											real s_distance = plane3d_distance_to_point(&s_plane, point);
											real t_distance = plane3d_distance_to_point(&t_plane, point);

											bounds.x0 = MIN(s_distance, bounds.x0);
											bounds.y0 = MIN(t_distance, bounds.y0);
											bounds.x1 = MAX(s_distance, bounds.x1);
											bounds.y1 = MAX(t_distance, bounds.y1);
										}

										if (lens_flare_spacing != 0.f)
										{
											real inverse_spacing = 1.f / lens_flare_spacing;

											grid_bounds.x0 = (short)fast_ftol(ceil(PIN(bounds.x0 * inverse_spacing, -1000.f, 1000.f)));
											grid_bounds.y0 = (short)fast_ftol(ceil(PIN(bounds.y0 * inverse_spacing, -1000.f, 1000.f)));
											grid_bounds.x1 = (short)fast_ftol(floor(PIN(bounds.x1 * inverse_spacing, -1000.f, 1000.f)));
											grid_bounds.y1 = (short)fast_ftol(floor(PIN(bounds.y1 * inverse_spacing, -1000.f, 1000.f)));
										}
										else
										{
											grid_bounds.y1 = 0;
											grid_bounds.x1 = 0;
											grid_bounds.y0 = 0;
											grid_bounds.x0 = 0;
										}

										for (t = grid_bounds.y0; t <= grid_bounds.y1; t++)
										{
											short s;

											for (s = grid_bounds.x0; s <= grid_bounds.x1; s++)
											{
												real_point2d projected_position;
												real_point3d position = center;

												point_from_line3d(&position, &s_plane.n, (real)(s * lens_flare_spacing), &position);
												point_from_line3d(&position, &t_plane.n, (real)(t * lens_flare_spacing), &position);
												project_point3d(&position, projection, sign, &projected_position);

												if (convex_hull2d_test_point_indexed(
													hull_count,
													hull_indices,
													projected_points,
													&projected_position,
													0.f))
												{
													long marker_index = tag_block_add_element(&structure_bsp->lens_flare_markers);

													if (marker_index != NONE)
													{
														struct structure_lens_flare_marker *marker = TAG_BLOCK_GET_ELEMENT(&structure_bsp->lens_flare_markers, marker_index, struct structure_lens_flare_marker);

														marker->position = position;
														marker->i_direction = (char)fast_ftol(floor(plane.n.i * 127.5f));
														marker->j_direction = (char)fast_ftol(floor(plane.n.j * 127.5f));
														marker->k_direction = (char)fast_ftol(floor(plane.n.k * 127.5f));
														marker->lens_flare_index = (byte)lens_flare_index;
													}
													else
													{
														error(
															_error_silent,
															"### WARNING failed to add lens flare marker to structure_bsp (max=#%d)",
															MAXIMUM_LENS_FLARE_MARKERS_PER_STRUCTURE);
													}
												}
											}
										}
									}
								}

								connected_geometry_delete(&geometry);
							}
							else
							{
								error(
									_error_silent,
									"### WARNING failed to add lens flare to structure_bsp (max=#%d)",
									MAXIMUM_LENS_FLARES_PER_STRUCTURE);
							}
						}
					}
				}
			}

			temp_markers = match_malloc("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 363, structure_bsp->lens_flare_markers.count * sizeof(struct temporary_lens_flare_marker));

			if (temp_markers)
			{
				long marker_index;
				long marker_count = 0;

				cluster_index = NONE;

				for (marker_index = 0; marker_index < structure_bsp->lens_flare_markers.count; marker_index++)
				{
					real_vector3d direction;
					real offset;
					struct structure_lens_flare_marker *marker = TAG_BLOCK_GET_ELEMENT(&structure_bsp->lens_flare_markers, marker_index, struct structure_lens_flare_marker);
					real_point3d point = marker->position;

					direction.i = marker->i_direction / 127.f;
					direction.j = marker->j_direction / 127.f;
					direction.k = marker->k_direction / 127.f;
					offset = 1.f / 65536.f;
					memcpy(&temp_markers[marker_index].marker, marker, sizeof(struct structure_lens_flare_marker));

					do
					{
						temp_markers[marker_index].cluster_index = (short)cluster_index_from_point(structure_bsp, &point);
						point_from_line3d(&point, &direction, offset, &point);
						offset *= 2.f;
					}
					while (temp_markers[marker_index].cluster_index == NONE && offset < 1.f);
				}

				qsort(
					temp_markers,
					structure_bsp->lens_flare_markers.count,
					sizeof(struct temporary_lens_flare_marker),
					compare_temp_markers);

				for (marker_index = 0; marker_index < structure_bsp->lens_flare_markers.count; marker_index++)
				{
					if (temp_markers[marker_index].cluster_index != NONE)
					{
						struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, temp_markers[marker_index].cluster_index, struct structure_cluster);

						if (cluster_index != temp_markers[marker_index].cluster_index)
						{
							match_assert("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 415, cluster_index<temp_markers[marker_index].cluster_index);

							cluster_index = temp_markers[marker_index].cluster_index;
							cluster->first_lens_flare_marker_index = (word)marker_count;
						}

						memcpy(
							TAG_BLOCK_GET_ELEMENT(&structure_bsp->lens_flare_markers, marker_count++, struct structure_lens_flare_marker),
							&temp_markers[marker_index].marker,
							sizeof(struct structure_lens_flare_marker));
						cluster->lens_flare_marker_count++;
					}
				}

				if (tag_block_resize(&structure_bsp->lens_flare_markers, marker_count))
				{
					for (cluster_index = 0; cluster_index < structure_bsp->clusters.count; cluster_index++)
					{
						struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_index, struct structure_cluster);

						if (cluster->lens_flare_marker_count > 0)
						{
							match_assert("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 444, cluster->first_lens_flare_marker_index<structure_bsp->lens_flare_markers.count);
							match_assert("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 445, cluster->first_lens_flare_marker_index+cluster->lens_flare_marker_count<=structure_bsp->lens_flare_markers.count);
						}
					}
				}
				else
				{
					error(_error_silent, "### ERROR failed to resize lens flare marker block");
					success = FALSE;
				}

				match_free("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 455, temp_markers);
			}
			else
			{
				error(_error_silent, "### ERROR failed to sort lens flare markers by cluster (out of memory)");
				success = FALSE;
			}
		}
		else
		{
			error(_error_silent, "### ERROR failed to allocate temporary buffers for lens flare placement");
			success = FALSE;
		}

		if (projected_points)
		{
			match_free("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 470, projected_points);
		}

		if (points)
		{
			match_free("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 471, points);
		}

		if (hull_indices)
		{
			match_free("c:\\halo\\SOURCE\\structures\\structure_lens_flares.c", 472, hull_indices);
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to clear lens flares and/or markers from structure_bsp");
		success = FALSE;
	}

	if (success)
	{
		fprintf(stdout, "done\r\n");
	}

	return success;
}

void structure_lens_flares_place(
	void)
{
	build_structure_lens_flares(global_structure_bsp_get());

	return;
}
