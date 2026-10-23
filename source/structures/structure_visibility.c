/*
STRUCTURE_VISIBILITY.C
*/

/* ---------- headers */

#include "cseries.h"
#include "structures.h"
#include "ai_debug.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "players.h"
#include "light_definitions.h"
#include "render_debug.h"
#include "structures/radiosity/radiosity.h"
#include "vector_tree.h"
#include "sky_definitions.h"
#include "structures/radiosity/radiosity_io.h"

/* ---------- constants */

enum
{
	MAXIMUM_PORTAL_HULL_VERTICES = 256,	/* fake name */
};

enum
{
	_intersection_none = 0,		/* fake name */
	_intersection_partial,		/* fake name */
	_intersection_complete,		/* fake name */
};

enum
{
	_portal_hull_visible = 0,	/* fake name */
	_portal_hull_invisible,		/* fake name */
	_portal_hull_degenerate,	/* fake name */
};

#define PORTAL_DEGENERATE_DISTANCE 0.1f		/* fake name */
#define PORTAL_HULL_EPSILON 0.0001f			/* fake name */
#define DEBUG_HULL_OFFSET 0.05f				/* fake name */

/* ---------- macros */

#define valid_portal_hull(hull) ((hull) && (hull)->vertex_count>=0 && (hull)->vertex_count<=MAXIMUM_PORTAL_HULL_VERTICES)

/* ---------- structures */

struct portal_hull
{
	short vertex_count;
	real_point2d vertices[MAXIMUM_PORTAL_HULL_VERTICES];
};

/* ---------- prototypes */

static void structure_visibility_traverse_subclusters(struct structure_bsp *structure_bsp);
static void structure_visibility_traverse_surface_lists(struct structure_bsp *structure_bsp);
static short structure_visibility_build_surfaces_traverse_clusters(long *surface_indices, short maximum_count, real_point3d const *bounding_sphere_center, real bounding_sphere_radius, real_rectangle3d const *bounding_box, short bounding_surface_count, real_plane3d const *bounding_surfaces, unsigned long *surface_flags, short cluster_count, short const *cluster_indices);
static short structure_visibility_build_surfaces_traverse_node(long node_index, real_rectangle3d const *parent_bounds, unsigned long *surface_flags, long *surface_indices, short maximum_count, real_point3d const *cull_sphere_center, real cull_sphere_radius, real_rectangle3d const *cull_bounds, short cull_plane_count, real_plane3d const *cull_planes, short intersection);
static short structure_visibility_build_surfaces_traverse_leaf(long leaf_index, real_rectangle3d const *parent_bounds, unsigned long *surface_flags, long *surface_indices, short maximum_count, real_point3d const *cull_sphere_center, real cull_sphere_radius, real_rectangle3d const *cull_bounds, short cull_plane_count, real_plane3d const *cull_planes, short intersection);
static short bounding_rectangles_intersect(real_rectangle3d const *test_rectangle, real_rectangle3d const *rectangle);
static short planes_intersect_rectangle(real_rectangle3d const *bounds, short cull_plane_count, real_plane3d const *cull_planes);
static void structure_visibility_find_clusters(void);
static void structure_visibility_traverse_cluster(short cluster_index, struct portal_hull const *visible_region);
static short portal_hull_from_points(struct render_camera const *camera, struct render_frustum const *frustum, real_plane3d const *plane, short vertex_count, real_point3d const *vertices, short winding, struct portal_hull *result);
static short portal_hull_from_portal(short portal_index, boolean flip, struct portal_hull *result);
static boolean points_within_distance(short point_count, real_point3d const *points, real distance);
static void grow_clipping_rectangle_by_portal_hull(real_rectangle2d *rectangle, struct portal_hull const *hull);
static void render_debug_hull(struct portal_hull const *hull, real offset, real_argb_color const *color);

/* ---------- globals */

boolean debug_leaf_portals;
boolean debug_portals;
boolean structures_use_pvs_for_vs;

static struct profile_section render_structure_visibility_portal_traversal =
{
	"render_structure_visibility_portal_traversal",
	NONE,
	TRUE
};

static struct profile_section render_structure_visibility_subcluster_traversal =
{
	"render_structure_visibility_subcluster_traversal",
	NONE,
	TRUE
};

static struct profile_section render_structure_visibility_surface_traversal =
{
	"render_structure_visibility_surface_traversal",
	NONE,
	TRUE
};

real structure_sphere_threshold = 2.f;

static real_plane3d const portal_clip_plane = { { 0.f, 0.f, -1.f }, 0.01f };	/* fake name */

static struct
{
	unsigned long *visited_cluster_flags;
	short rendered_cluster_indices[MAXIMUM_CLUSTERS_PER_STRUCTURE];
} structure_visibility_globals;
static boolean warned_about_missing_subclusters;

/* ---------- public code */

void debug_pvs(
	boolean enable)
{
	debug_portals = enable;
	structures_use_pvs_for_vs = enable;

	return;
}

void structure_visibility_find_camera(
	struct render_camera const *camera)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	long leaf_index = bsp3d_test_point(
		&TAG_BLOCK_GET_ELEMENT(&structure_bsp->collision_bsp, 0, struct collision_bsp)->bsp3d,
		0,
		&camera->position);

	if (leaf_index == NONE)
	{
		if (render.leaf_index >= structure_bsp->leaves.count)
		{
			render.leaf_index = NONE;
		}
	}
	else
	{
		render.leaf_index = leaf_index;
	}

	render.cluster_index = NONE;
	render.visible_sky_index = NONE;
	render.visible_sky_model = FALSE;

	if (render.leaf_index != NONE)
	{
		struct sky *sky;

		render.cluster_index = TAG_BLOCK_GET_ELEMENT(&structure_bsp->leaves, render.leaf_index & LONG_MAX, struct structure_leaf)->cluster_index;
		render.visible_sky_index = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, render.cluster_index, struct structure_cluster)->sky_index;
		sky = scenario_get_sky(render.visible_sky_index);

		if (sky && sky->model.index != NONE)
		{
			render.visible_sky_model = TRUE;
		}
	}

	return;
}

boolean structure_visibility_find_mirror(
	struct render_camera const *camera,
	struct render_frustum const *frustum,
	struct render_mirror *result)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	boolean found = FALSE;

	if (render.cluster_index != NONE)
	{
		real_rectangle2d screen_clip_bounds;
		struct portal_hull screen;
		unsigned long *cluster_pvs;
		short cluster_index;

		render_frustum_get_projection_bounds(frustum, &screen_clip_bounds);
		screen.vertex_count = NUMBER_OF_POINTS_PER_RECTANGLE;
		screen.vertices[0].x = screen_clip_bounds.x0;
		screen.vertices[0].y = screen_clip_bounds.y0;
		screen.vertices[1].x = screen_clip_bounds.x1;
		screen.vertices[1].y = screen_clip_bounds.y0;
		screen.vertices[2].x = screen_clip_bounds.x1;
		screen.vertices[2].y = screen_clip_bounds.y1;
		screen.vertices[3].x = screen_clip_bounds.x0;
		screen.vertices[3].y = screen_clip_bounds.y1;
		cluster_pvs = structure_bsp_get_cluster_pvs(structure_bsp, (short)render.cluster_index);

		for (cluster_index = 0; cluster_index < structure_bsp->clusters.count; cluster_pvs++)
		{
			if (*cluster_pvs)
			{
				short bit;

				for (bit = 0; bit < LONG_BITS && cluster_index < structure_bsp->clusters.count; bit++, cluster_index++)
				{
					if (TEST_FLAG(*cluster_pvs, bit))
					{
						short mirror_index;
						struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_index, struct structure_cluster);

						for (mirror_index = 0; mirror_index < cluster->mirrors.count; mirror_index++)
						{
							struct portal_hull mirror_hull;
							struct portal_hull intersection_hull;
							struct structure_mirror *mirror = TAG_BLOCK_GET_ELEMENT(&cluster->mirrors, mirror_index, struct structure_mirror);
							short hull_result = portal_hull_from_points(
								camera,
								frustum,
								&mirror->plane,
								(short)mirror->points.count,
								(real_point3d const *)mirror->points.address,
								1,
								&mirror_hull);

							if (hull_result == _portal_hull_visible)
							{
								intersection_hull.vertex_count = convex_hull2d_intersect(
									screen.vertex_count,
									screen.vertices,
									mirror_hull.vertex_count,
									mirror_hull.vertices,
									MAXIMUM_PORTAL_HULL_VERTICES,
									intersection_hull.vertices,
									PORTAL_HULL_EPSILON);
							}

							if (hull_result == _portal_hull_visible && intersection_hull.vertex_count ||
								hull_result == _portal_hull_degenerate)
							{
								struct shader *shader = shader_definition_get(mirror->shader.index);

								if (shader->base.type == _shader_type_environment)
								{
									struct shader_environment *environment = shader_get_and_verify_type(shader, _shader_type_environment);

									result->index_of_refraction = environment->environment.reflection.mirror_index_of_refraction;
									result->depth = environment->environment.reflection.mirror_depth;
								}
								else
								{
									result->index_of_refraction = 0.f;
									result->depth = 0.f;
								}

								if (found &&
									!(fabs(mirror->plane.n.i - result->plane.n.i) < PORTAL_HULL_EPSILON &&
									fabs(mirror->plane.n.j - result->plane.n.j) < PORTAL_HULL_EPSILON &&
									fabs(mirror->plane.n.k - result->plane.n.k) < PORTAL_HULL_EPSILON &&
									fabs(mirror->plane.d - result->plane.d) < PORTAL_HULL_EPSILON))
								{
									error(_error_silent, "two mirrors visible with different planes");
								}

								result->plane = mirror->plane;
								result->cluster_index = cluster_index;
								found = TRUE;
							}
						}
					}
				}
			}
			else
			{
				cluster_index += LONG_BITS;
			}
		}
	}

	return found;
}

void structure_visibility_compute(
	void)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	profile_enter(render_structure_visibility_portal_traversal);

	if (render.cluster_index != NONE)
	{
		memset(render.visible_cluster_flags, 0, BIT_VECTOR_SIZE_IN_BYTES(structure_bsp->clusters.count));
	}
	else
	{
		memset(render.visible_cluster_flags, NONE, BIT_VECTOR_SIZE_IN_BYTES(structure_bsp->clusters.count));
	}

	render.environment_surface_count = 0;
	memset(render.environment_surface_flags, 0, BIT_VECTOR_SIZE_IN_BYTES(structure_bsp->surfaces.count));
	render.rendered_cluster_count = 0;
	structure_visibility_find_clusters();

	if (structures_use_pvs_for_vs)
	{
		short cluster_index;

		render.rendered_cluster_count = 0;
		memcpy(
			render.visible_cluster_flags,
			structure_bsp_get_cluster_pvs(structure_bsp, (short)render.cluster_index),
			BIT_VECTOR_SIZE_IN_BYTES(structure_bsp->clusters.count));

		for (cluster_index = 0; cluster_index < structure_bsp->clusters.count; cluster_index++)
		{
			if (BIT_VECTOR_TEST_FLAG(render.visible_cluster_flags, cluster_index))
			{
				struct rendered_cluster *rendered_cluster;

				TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_index, struct structure_cluster);
				match_vassert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 280, render.rendered_cluster_count<MAXIMUM_RENDERED_CLUSTERS, "raise MAXIMUM_RENDERED_CLUSTERS");
				match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 283, cluster_index>=0 && cluster_index<MAXIMUM_CLUSTERS_PER_STRUCTURE);
				structure_visibility_globals.rendered_cluster_indices[cluster_index] = render.rendered_cluster_count++;
				rendered_cluster = rendered_cluster_get(structure_visibility_globals.rendered_cluster_indices[cluster_index]);
				rendered_cluster->cluster_index = cluster_index;
				render_frustum_get_projection_bounds(&render.frustum, &rendered_cluster->clip_bounds);
			}
		}
	}

	profile_exit(render_structure_visibility_portal_traversal);

	if (TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, 0, struct structure_cluster)->subclusters.count)
	{
		structure_visibility_traverse_subclusters(structure_bsp);
	}
	else
	{
		if (!warned_about_missing_subclusters)
		{
			if (structure_bsp->surfaces.count > 0)
			{
				error(_error_silent, "### WARNING: this structure_bsp needs to be reimported for new, faster visibility.");
			}

			warned_about_missing_subclusters = TRUE;
		}

		structure_visibility_traverse_surface_lists(structure_bsp);
	}

	return;
}

static void structure_visibility_traverse_subclusters(
	struct structure_bsp *structure_bsp)
{
	short rendered_cluster_index;

	profile_enter(render_structure_visibility_subcluster_traversal);

	for (rendered_cluster_index = 0; rendered_cluster_index < render.rendered_cluster_count; rendered_cluster_index++)
	{
		struct rendered_cluster *rendered_cluster;
		struct structure_cluster *cluster;
		struct render_frustum *frustum;
		short subcluster_index;

		if (render.environment_surface_count >= MAXIMUM_RENDERED_ENVIRONMENT_SURFACES)
		{
			break;
		}

		rendered_cluster = rendered_cluster_get(rendered_cluster_index);
		cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, rendered_cluster->cluster_index, struct structure_cluster);

		if (structures_use_pvs_for_vs || render.cluster_index == NONE)
		{
			frustum = &render.frustum;
		}
		else
		{
			frustum = &rendered_cluster->frustum;
		}

		for (subcluster_index = 0; subcluster_index < cluster->subclusters.count; subcluster_index++)
		{
			struct structure_subcluster *subcluster;

			if (render.environment_surface_count >= MAXIMUM_RENDERED_ENVIRONMENT_SURFACES)
			{
				break;
			}

			subcluster = TAG_BLOCK_GET_ELEMENT(&cluster->subclusters, subcluster_index, struct structure_subcluster);

			if (render_frustum_cube_visible(frustum, &subcluster->world_bounds, FALSE))
			{
				short surface_reference_index;
				long *surface_index = TAG_BLOCK_GET_ELEMENT(&subcluster->surface_indices, 0, long);

				for (surface_reference_index = 0; surface_reference_index < subcluster->surface_indices.count; surface_reference_index++)
				{
					if (!BIT_VECTOR_TEST_FLAG(render.environment_surface_flags, *surface_index))
					{
						if (render.environment_surface_count >= MAXIMUM_RENDERED_ENVIRONMENT_SURFACES)
						{
							break;
						}

						BIT_VECTOR_SET_FLAG(render.environment_surface_flags, *surface_index, TRUE);
						render.environment_surface_count++;
					}

					surface_index++;
				}
			}
		}
	}

	profile_exit(render_structure_visibility_subcluster_traversal);

	return;
}

static void structure_visibility_traverse_surface_lists(
	struct structure_bsp *structure_bsp)
{
	short rendered_cluster_index;

	profile_enter(render_structure_visibility_surface_traversal);

	for (rendered_cluster_index = 0; rendered_cluster_index < render.rendered_cluster_count; rendered_cluster_index++)
	{
		struct render_frustum *frustum;
		long surface_index_index;
		struct rendered_cluster *rendered_cluster = rendered_cluster_get(rendered_cluster_index);
		struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, rendered_cluster->cluster_index, struct structure_cluster);
		long *surface_index_buffer = (long *)cluster->surface_indices.address;

		if (structures_use_pvs_for_vs || render.cluster_index == NONE)
		{
			frustum = &render.frustum;
		}
		else
		{
			frustum = &rendered_cluster->frustum;
		}

		surface_index_index = 0;

		while (surface_index_index < cluster->surface_indices.count)
		{
			struct structure_material *material = TAG_BLOCK_GET_ELEMENT(
				&TAG_BLOCK_GET_ELEMENT(&structure_bsp->lightmaps, surface_index_buffer[0], struct structure_lightmap)->materials,
				surface_index_buffer[1],
				struct structure_material);
			long surface_index_end;

			surface_index_buffer += 2;
			surface_index_end = surface_index_index + *surface_index_buffer + 3;
			surface_index_buffer++;

			for (surface_index_index += 3; surface_index_index < surface_index_end; surface_index_index++)
			{
				long surface_index;

				if (render.environment_surface_count >= MAXIMUM_RENDERED_ENVIRONMENT_SURFACES)
				{
					break;
				}

				surface_index = *surface_index_buffer++;
				match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 416, surface_index_buffer-(long *) cluster->surface_indices.address<=cluster->surface_indices.count);

				if (!BIT_VECTOR_TEST_FLAG(render.environment_surface_flags, surface_index))
				{
					struct structure_surface *surface = TAG_BLOCK_GET_ELEMENT(&structure_bsp->surfaces, surface_index, struct structure_surface);
					struct environment_vertex_compressed *vertex0 = (struct environment_vertex_compressed *)material->compressed_vertex_data.address + surface->vertex_indices[0];
					struct environment_vertex_compressed *vertex1 = (struct environment_vertex_compressed *)material->compressed_vertex_data.address + surface->vertex_indices[1];
					struct environment_vertex_compressed *vertex2 = (struct environment_vertex_compressed *)material->compressed_vertex_data.address + surface->vertex_indices[2];

					if (render_frustum_triangle_visible(frustum, &vertex0->position, &vertex1->position, &vertex2->position))
					{
						BIT_VECTOR_SET_FLAG(render.environment_surface_flags, surface_index, TRUE);
						render.environment_surface_count++;
					}
				}
			}
		}
	}

	profile_exit(render_structure_visibility_surface_traversal);

	return;
}

static short structure_visibility_build_surfaces_traverse_clusters(
	long *surface_indices,
	short maximum_count,
	real_point3d const *bounding_sphere_center,
	real bounding_sphere_radius,
	real_rectangle3d const *bounding_box,
	short bounding_surface_count,
	real_plane3d const *bounding_surfaces,
	unsigned long *surface_flags,
	short cluster_count,
	short const *cluster_indices)
{
	short cluster_index;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	short surface_count = 0;

	for (cluster_index = 0; cluster_index < cluster_count && surface_count < maximum_count; cluster_index++)
	{
		short subcluster_index;
		struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_indices[cluster_index], struct structure_cluster);

		for (subcluster_index = 0; subcluster_index < cluster->subclusters.count && surface_count < maximum_count; subcluster_index++)
		{
			struct structure_subcluster *subcluster = TAG_BLOCK_GET_ELEMENT(&cluster->subclusters, subcluster_index, struct structure_subcluster);

			if (bounding_rectangles_intersect(bounding_box, &subcluster->world_bounds) &&
				planes_intersect_rectangle(&subcluster->world_bounds, bounding_surface_count, bounding_surfaces))
			{
				short surface_reference_index;
				long *surface_index = TAG_BLOCK_GET_ELEMENT(&subcluster->surface_indices, 0, long);

				for (surface_reference_index = 0; surface_reference_index < subcluster->surface_indices.count; surface_reference_index++)
				{
					if (BIT_VECTOR_TEST_FLAG(render.environment_surface_flags, *surface_index) &&
						!BIT_VECTOR_TEST_FLAG(surface_flags, *surface_index))
					{
						if (surface_count >= maximum_count)
						{
							break;
						}

						BIT_VECTOR_SET_FLAG(surface_flags, *surface_index, TRUE);
						surface_indices[surface_count++] = *surface_index;
					}

					surface_index++;
				}
			}
		}
	}

	return surface_count;
}

short structure_visibility_build_surfaces(
	long *surface_indices,
	short maximum_count,
	real_point3d const *bounding_sphere_center,
	real bounding_sphere_radius,
	real_rectangle3d const *bounding_box,
	short bounding_surface_count,
	real_plane3d const *bounding_surfaces,
	short cluster_count,
	short const *cluster_indices)
{
	unsigned long surface_flags[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_SURFACES_PER_STRUCTURE)];
	real_rectangle3d bounding_box_storage;
	struct location location;
	short temporary_cluster_indices[MAXIMUM_CLUSTERS_PER_STRUCTURE];
	short temporary_cluster_count;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 613, bounding_sphere_center);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 614, !bounding_surface_count || bounding_surfaces);
	memset(surface_flags, 0, BIT_VECTOR_SIZE_IN_BYTES(structure_bsp->surfaces.count));

	if (!bounding_box)
	{
		bounding_box_storage.x0 = bounding_sphere_center->x - bounding_sphere_radius;
		bounding_box_storage.x1 = bounding_sphere_center->x + bounding_sphere_radius;
		bounding_box_storage.y0 = bounding_sphere_center->y - bounding_sphere_radius;
		bounding_box_storage.y1 = bounding_sphere_center->y + bounding_sphere_radius;
		bounding_box_storage.z0 = bounding_sphere_center->z - bounding_sphere_radius;
		bounding_box_storage.z1 = bounding_sphere_center->z + bounding_sphere_radius;
		bounding_box = &bounding_box_storage;
	}

	if (bounding_sphere_radius < structure_sphere_threshold)
	{
		return structure_visibility_build_surfaces_traverse_node(
			0,
			&structure_bsp->world_bounds,
			surface_flags,
			surface_indices,
			maximum_count,
			bounding_sphere_center,
			bounding_sphere_radius,
			bounding_box,
			bounding_surface_count,
			bounding_surfaces,
			_intersection_partial);
	}

	if (cluster_indices)
	{
		return structure_visibility_build_surfaces_traverse_clusters(
			surface_indices,
			maximum_count,
			bounding_sphere_center,
			bounding_sphere_radius,
			bounding_box,
			bounding_surface_count,
			bounding_surfaces,
			surface_flags,
			cluster_count,
			cluster_indices);
	}

	scenario_location_from_point(&location, bounding_sphere_center);

	if (location.cluster_index != NONE)
	{
		temporary_cluster_count = structure_clusters_in_sphere(
			location.cluster_index,
			bounding_sphere_center,
			bounding_sphere_radius,
			MAXIMUM_CLUSTERS_PER_STRUCTURE,
			temporary_cluster_indices);

		return structure_visibility_build_surfaces_traverse_clusters(
			surface_indices,
			maximum_count,
			bounding_sphere_center,
			bounding_sphere_radius,
			bounding_box,
			bounding_surface_count,
			bounding_surfaces,
			surface_flags,
			temporary_cluster_count,
			temporary_cluster_indices);
	}

	return structure_visibility_build_surfaces_traverse_node(
		0,
		&structure_bsp->world_bounds,
		surface_flags,
		surface_indices,
		maximum_count,
		bounding_sphere_center,
		bounding_sphere_radius,
		bounding_box,
		bounding_surface_count,
		bounding_surfaces,
		_intersection_partial);
}

static short structure_visibility_build_surfaces_traverse_node(
	long node_index,
	real_rectangle3d const *parent_bounds,
	unsigned long *surface_flags,
	long *surface_indices,
	short maximum_count,
	real_point3d const *cull_sphere_center,
	real cull_sphere_radius,
	real_rectangle3d const *cull_bounds,
	short cull_plane_count,
	real_plane3d const *cull_planes,
	short intersection)
{
	real_rectangle3d bounds;
	short surface_count = 0;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	struct collision_bsp *collision_bsp = TAG_BLOCK_GET_ELEMENT(&structure_bsp->collision_bsp, 0, struct collision_bsp);

	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 683, parent_bounds);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 684, cull_sphere_center);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 685, cull_bounds);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 686, intersection);
	dequantize_byte_to_real_rectangle3d(
		parent_bounds,
		&TAG_BLOCK_GET_ELEMENT(&structure_bsp->nodes, node_index, struct structure_node)->bounds,
		&bounds);

	if (intersection != _intersection_complete)
	{
		short rectangle_intersection = bounding_rectangles_intersect(&bounds, cull_bounds);
		short plane_intersection = rectangle_intersection ? planes_intersect_rectangle(&bounds, cull_plane_count, cull_planes) : _intersection_none;

		if (plane_intersection == _intersection_complete)
		{
			cull_plane_count = 0;
		}

		intersection = MIN(rectangle_intersection, plane_intersection);
	}

	if (intersection)
	{
		struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(&collision_bsp->bsp3d.nodes, node_index, struct bsp3d_node);
		real distance = plane3d_distance_to_point(
			TAG_BLOCK_GET_ELEMENT(&collision_bsp->bsp3d.planes, node->plane, real_plane3d),
			cull_sphere_center);
		boolean in[2];
		short child_index;

		in[0] = distance < cull_sphere_radius;
		in[1] = distance > -cull_sphere_radius;

		for (child_index = 0; child_index < NUMBEROF(in); child_index++)
		{
			if (in[child_index])
			{
				if (!(node->child_indices[child_index] & LONG_MIN))
				{
					surface_count += structure_visibility_build_surfaces_traverse_node(
						node->child_indices[child_index],
						&bounds,
						surface_flags,
						&surface_indices[surface_count],
						maximum_count - surface_count,
						cull_sphere_center,
						cull_sphere_radius,
						cull_bounds,
						cull_plane_count,
						cull_planes,
						intersection);
				}
				else if (node->child_indices[child_index] != NONE)
				{
					surface_count += structure_visibility_build_surfaces_traverse_leaf(
						node->child_indices[child_index],
						&bounds,
						surface_flags,
						&surface_indices[surface_count],
						maximum_count - surface_count,
						cull_sphere_center,
						cull_sphere_radius,
						cull_bounds,
						cull_plane_count,
						cull_planes,
						intersection);
				}
			}
		}
	}

	return surface_count;
}

static short structure_visibility_build_surfaces_traverse_leaf(
	long leaf_index,
	real_rectangle3d const *parent_bounds,
	unsigned long *surface_flags,
	long *surface_indices,
	short maximum_count,
	real_point3d const *cull_sphere_center,
	real cull_sphere_radius,
	real_rectangle3d const *cull_bounds,
	short cull_plane_count,
	real_plane3d const *cull_planes,
	short intersection)
{
	real_rectangle3d bounds;
	short surface_count = 0;
	struct structure_bsp *structure = global_structure_bsp_get();
	struct structure_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&structure->leaves, leaf_index & LONG_MAX, struct structure_leaf);

	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 752, intersection);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 753, parent_bounds);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 754, cull_sphere_center);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 755, cull_bounds);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 756, leaf->cluster_index>=0 && leaf->cluster_index<structure->clusters.count);
	dequantize_byte_to_real_rectangle3d(parent_bounds, &leaf->bounds, &bounds);

	if (intersection != _intersection_complete)
	{
		short rectangle_intersection = bounding_rectangles_intersect(&bounds, cull_bounds);
		short plane_intersection = planes_intersect_rectangle(&bounds, cull_plane_count, cull_planes);

		intersection = MIN(rectangle_intersection, plane_intersection);
	}

	if (intersection)
	{
		long surface_reference_index;

		for (surface_reference_index = leaf->first_surface_reference_index;
			surface_reference_index < leaf->first_surface_reference_index + leaf->surface_reference_count;
			surface_reference_index++)
		{
			long surface_index = TAG_BLOCK_GET_ELEMENT(&structure->surface_references, surface_reference_index, struct structure_surface_reference)->surface_index;

			if (BIT_VECTOR_TEST_FLAG(render.environment_surface_flags, surface_index) &&
				!BIT_VECTOR_TEST_FLAG(surface_flags, surface_index))
			{
				if (surface_count >= maximum_count)
				{
					return surface_count;
				}

				BIT_VECTOR_SET_FLAG(surface_flags, surface_index, TRUE);
				surface_indices[surface_count++] = surface_index;
			}
		}
	}

	return surface_count;
}

static short bounding_rectangles_intersect(
	real_rectangle3d const *test_rectangle,
	real_rectangle3d const *rectangle)
{
	short result;

	if (rectangle->x1 < test_rectangle->x0 ||
		rectangle->x0 > test_rectangle->x1 ||
		rectangle->y1 < test_rectangle->y0 ||
		rectangle->y0 > test_rectangle->y1 ||
		rectangle->z1 < test_rectangle->z0 ||
		rectangle->z0 > test_rectangle->z1)
	{
		result = _intersection_none;
	}
	else if (test_rectangle->x0 < rectangle->x0 ||
		test_rectangle->x1 > rectangle->x1 ||
		test_rectangle->y0 < rectangle->y0 ||
		test_rectangle->y1 > rectangle->y1 ||
		test_rectangle->z0 < rectangle->z0 ||
		test_rectangle->z1 > rectangle->z1)
	{
		result = _intersection_partial;
	}
	else
	{
		result = _intersection_complete;
	}

	return result;
}

static short planes_intersect_rectangle(
	real_rectangle3d const *bounds,
	short cull_plane_count,
	real_plane3d const *cull_planes)
{
	short plane_index;
	short outside = 0;
	short result = NONE;
	real_rectangle3d local_bounds = *bounds;

	for (plane_index = 0; plane_index < cull_plane_count; plane_index++)
	{
		real_plane3d plane = cull_planes[plane_index];
		short corners_outside = 0;

		if (plane.n.i * local_bounds.x0 + plane.n.j * local_bounds.y0 + plane.n.k * local_bounds.z0 - plane.d < 0.f)
		{
			corners_outside |= FLAG(0);
		}

		if (plane.n.i * local_bounds.x1 + plane.n.j * local_bounds.y0 + plane.n.k * local_bounds.z0 - plane.d < 0.f)
		{
			corners_outside |= FLAG(1);
		}

		if (plane.n.i * local_bounds.x0 + plane.n.j * local_bounds.y1 + plane.n.k * local_bounds.z0 - plane.d < 0.f)
		{
			corners_outside |= FLAG(2);
		}

		if (plane.n.i * local_bounds.x1 + plane.n.j * local_bounds.y1 + plane.n.k * local_bounds.z0 - plane.d < 0.f)
		{
			corners_outside |= FLAG(3);
		}

		if (plane.n.i * local_bounds.x0 + plane.n.j * local_bounds.y0 + plane.n.k * local_bounds.z1 - plane.d < 0.f)
		{
			corners_outside |= FLAG(4);
		}

		if (plane.n.i * local_bounds.x1 + plane.n.j * local_bounds.y0 + plane.n.k * local_bounds.z1 - plane.d < 0.f)
		{
			corners_outside |= FLAG(5);
		}

		if (plane.n.i * local_bounds.x0 + plane.n.j * local_bounds.y1 + plane.n.k * local_bounds.z1 - plane.d < 0.f)
		{
			corners_outside |= FLAG(6);
		}

		if (plane.n.i * local_bounds.x1 + plane.n.j * local_bounds.y1 + plane.n.k * local_bounds.z1 - plane.d < 0.f)
		{
			corners_outside |= FLAG(7);
		}

		if (corners_outside == MASK(NUMBER_OF_VERTICES_PER_CUBE))
		{
			result = _intersection_none;
			break;
		}

		outside |= corners_outside;
	}

	if (result == NONE)
	{
		if (outside)
		{
			result = _intersection_partial;
		}
		else
		{
			result = _intersection_complete;
		}
	}

	return result;
}

short structure_visibility_find_objects(
	long *result_indices,
	short maximum_count,
	long (*cluster_get_first)(long *, short),
	long (*cluster_get_next)(long *),
	void (*get_bounding_sphere)(long, real_point3d *, real *),
	boolean (*unmarked)(long),
	boolean (*mark)(long))
{
	short rendered_cluster_index;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	short result_count = 0;

	for (rendered_cluster_index = 0; rendered_cluster_index < render.rendered_cluster_count; rendered_cluster_index++)
	{
		long state;
		long object_index;
		struct rendered_cluster *rendered_cluster = rendered_cluster_get(rendered_cluster_index);

		for (object_index = cluster_get_first(&state, rendered_cluster->cluster_index); object_index != NONE; object_index = cluster_get_next(&state))
		{
			if (unmarked(object_index))
			{
				real_point3d bounding_sphere_center;
				real bounding_sphere_radius;

				get_bounding_sphere(object_index, &bounding_sphere_center, &bounding_sphere_radius);

				if (result_count < maximum_count &&
					(render.cluster_index == NONE || render_frustum_sphere_visible(
					&rendered_cluster->frustum,
					&bounding_sphere_center,
					bounding_sphere_radius)))
				{
					result_indices[result_count++] = object_index;
					mark(object_index);
				}
			}
		}
	}

	return result_count;
}

static void structure_visibility_find_clusters(
	void)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	if (render.cluster_index != NONE)
	{
		real_rectangle2d screen_clip_bounds;
		struct portal_hull screen;
		unsigned long visited_cluster_flags[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CLUSTERS_PER_STRUCTURE)];
		short rendered_cluster_index;

		render_frustum_get_projection_bounds(&render.frustum, &screen_clip_bounds);
		screen.vertex_count = NUMBER_OF_POINTS_PER_RECTANGLE;
		screen.vertices[0].x = screen_clip_bounds.x0;
		screen.vertices[0].y = screen_clip_bounds.y0;
		screen.vertices[1].x = screen_clip_bounds.x1;
		screen.vertices[1].y = screen_clip_bounds.y0;
		screen.vertices[2].x = screen_clip_bounds.x1;
		screen.vertices[2].y = screen_clip_bounds.y1;
		screen.vertices[3].x = screen_clip_bounds.x0;
		screen.vertices[3].y = screen_clip_bounds.y1;
		structure_visibility_globals.visited_cluster_flags = visited_cluster_flags;
		memset(visited_cluster_flags, 0, sizeof(visited_cluster_flags));
		structure_visibility_traverse_cluster((short)render.cluster_index, &screen);

		for (rendered_cluster_index = 0; rendered_cluster_index < render.rendered_cluster_count; rendered_cluster_index++)
		{
			real_rectangle2d frustum_bounds;
			struct rendered_cluster *rendered_cluster = rendered_cluster_get(rendered_cluster_index);

			TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, rendered_cluster->cluster_index, struct structure_cluster);
			render_camera_build_clipped_frustum_bounds(&render.camera, &rendered_cluster->clip_bounds, &frustum_bounds);
			render_camera_build_frustum(&render.camera, &frustum_bounds, &rendered_cluster->frustum, TRUE);
		}
	}

	return;
}

static void structure_visibility_traverse_cluster(
	short cluster_index,
	struct portal_hull const *visible_region)
{
	struct rendered_cluster *rendered_cluster;
	short portal_reference_index;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_index, struct structure_cluster);
	unsigned long *cluster_pvs = structure_bsp_get_cluster_pvs(structure_bsp, (short)render.cluster_index);

	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 1006, valid_portal_hull(visible_region));
	BIT_VECTOR_SET_FLAG(structure_visibility_globals.visited_cluster_flags, cluster_index, TRUE);

	if (!BIT_VECTOR_TEST_FLAG(render.visible_cluster_flags, cluster_index))
	{
		match_vassert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 1013, render.rendered_cluster_count<MAXIMUM_RENDERED_CLUSTERS, "raise MAXIMUM_RENDERED_CLUSTERS");
		match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 1016, cluster_index>=0 && cluster_index<MAXIMUM_CLUSTERS_PER_STRUCTURE);
		structure_visibility_globals.rendered_cluster_indices[cluster_index] = render.rendered_cluster_count++;
		rendered_cluster = rendered_cluster_get(structure_visibility_globals.rendered_cluster_indices[cluster_index]);
		rendered_cluster->cluster_index = cluster_index;
		rendered_cluster->clip_bounds = *global_null_rectangle2d;
	}
	else
	{
		rendered_cluster = rendered_cluster_get(structure_visibility_globals.rendered_cluster_indices[cluster_index]);
		match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 1027, rendered_cluster->cluster_index==cluster_index);
	}

	BIT_VECTOR_SET_FLAG(render.visible_cluster_flags, cluster_index, TRUE);
	grow_clipping_rectangle_by_portal_hull(&rendered_cluster->clip_bounds, visible_region);

	{
		real_argb_color const *render_color;

		if (debug_portals)
		{
			render_debug_hull(visible_region, DEBUG_HULL_OFFSET, global_real_argb_red);
		}
		else if (ai_debug_highlight_cluster(cluster_index, &render_color))
		{
			render_debug_hull(visible_region, DEBUG_HULL_OFFSET, render_color);
		}
	}

	for (portal_reference_index = 0; portal_reference_index < cluster->portal_indices.count; portal_reference_index++)
	{
		short portal_index = *TAG_BLOCK_GET_ELEMENT(&cluster->portal_indices, portal_reference_index, short);
		struct cluster_portal *portal = TAG_BLOCK_GET_ELEMENT(&structure_bsp->cluster_portals, portal_index, struct cluster_portal);
		boolean flip = portal->cluster_indices[0] == cluster_index;
		short next_cluster_index = portal->cluster_indices[flip];

		if (next_cluster_index >= 0 &&
			next_cluster_index < structure_bsp->clusters.count &&
			!BIT_VECTOR_TEST_FLAG(structure_visibility_globals.visited_cluster_flags, next_cluster_index) &&
			BIT_VECTOR_TEST_FLAG(cluster_pvs, next_cluster_index))
		{
			struct portal_hull portal_hull;
			short hull_result = portal_hull_from_portal(portal_index, flip, &portal_hull);

			if (hull_result == _portal_hull_degenerate)
			{
				structure_visibility_traverse_cluster(next_cluster_index, visible_region);
			}
			else if (hull_result == _portal_hull_visible)
			{
				if (render.visible_sky_model ||
					points_within_distance(
					(short)portal->vertices.count,
					(real_point3d const *)portal->vertices.address,
					render.camera.z_far))
				{
					struct portal_hull intersection_hull;

					intersection_hull.vertex_count = convex_hull2d_intersect(
						visible_region->vertex_count,
						visible_region->vertices,
						portal_hull.vertex_count,
						portal_hull.vertices,
						MAXIMUM_PORTAL_HULL_VERTICES,
						intersection_hull.vertices,
						PORTAL_HULL_EPSILON);

					if (intersection_hull.vertex_count > 0)
					{
						structure_visibility_traverse_cluster(next_cluster_index, &intersection_hull);
					}
					else if (intersection_hull.vertex_count == NONE)
					{
						error(_error_silent, "portal intersection failed.");
						structure_visibility_traverse_cluster(next_cluster_index, visible_region);
					}
				}
			}
		}
	}

	BIT_VECTOR_SET_FLAG(structure_visibility_globals.visited_cluster_flags, cluster_index, FALSE);

	return;
}

static short portal_hull_from_points(
	struct render_camera const *camera,
	struct render_frustum const *frustum,
	real_plane3d const *plane,
	short vertex_count,
	real_point3d const *vertices,
	short winding,
	struct portal_hull *result)
{
	short visibility;
	real distance;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	result->vertex_count = 0;
	distance = plane3d_distance_to_point(plane, &camera->position) * winding;

	if (camera->mirrored)
	{
		winding = -winding;
	}

	if (fabs(distance) < PORTAL_DEGENERATE_DISTANCE)
	{
		visibility = _portal_hull_degenerate;
	}
	else if (distance > 0.f)
	{
		real_point3d viewer_points[MAXIMUM_PORTAL_HULL_VERTICES];
		short vertex_index;
		short hull_vertex_index;
		short first_vertex_index;
		short last_vertex_index;

		for (vertex_index = 0; vertex_index < vertex_count; vertex_index++)
		{
			matrix4x3_transform_point(&frustum->world_to_view, &vertices[vertex_index], &viewer_points[vertex_index]);
		}

		result->vertex_count = convex_polygon3d_clip_to_plane(
			vertex_count,
			viewer_points,
			&portal_clip_plane,
			MAXIMUM_PORTAL_HULL_VERTICES,
			viewer_points,
			NULL,
			PORTAL_HULL_EPSILON,
			TRUE);
		match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 1157, result->vertex_count!=NONE);

		if (winding == 1)
		{
			first_vertex_index = 0;
			last_vertex_index = result->vertex_count;
		}
		else
		{
			first_vertex_index = result->vertex_count - 1;
			last_vertex_index = NONE;
		}

		for (hull_vertex_index = 0, vertex_index = first_vertex_index;
			vertex_index != last_vertex_index;
			vertex_index += winding, hull_vertex_index++)
		{
			real ooz = -1.f / viewer_points[vertex_index].z;

			match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 1175, ooz>0.f);
			result->vertices[hull_vertex_index].x = viewer_points[vertex_index].x * ooz;
			result->vertices[hull_vertex_index].y = viewer_points[vertex_index].y * ooz;
		}

		visibility = result->vertex_count < NUMBER_OF_VERTICES_PER_TRIANGLE ? _portal_hull_invisible : _portal_hull_visible;
	}
	else
	{
		visibility = _portal_hull_invisible;
	}

	return visibility;
}

static short portal_hull_from_portal(
	short portal_index,
	boolean flip,
	struct portal_hull *result)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	struct cluster_portal *portal = TAG_BLOCK_GET_ELEMENT(&structure_bsp->cluster_portals, portal_index, struct cluster_portal);
	short winding = flip ? -1 : 1;

	return portal_hull_from_points(
		&render.camera,
		&render.frustum,
		TAG_BLOCK_GET_ELEMENT(
			&TAG_BLOCK_GET_ELEMENT(&structure_bsp->collision_bsp, 0, struct collision_bsp)->bsp3d.planes,
			portal->plane_index,
			real_plane3d),
		(short)portal->vertices.count,
		(real_point3d const *)portal->vertices.address,
		winding,
		result);
}

static boolean points_within_distance(
	short point_count,
	real_point3d const *points,
	real distance)
{
	short point_index;

	for (point_index = 0; point_index < point_count; point_index++)
	{
		real_vector3d incident;

		vector_from_points3d(&render.camera.position, &points[point_index], &incident);

		if (dot_product3d(&incident, &render.camera.forward) <= distance)
		{
			return TRUE;
		}
	}

	return FALSE;
}

static void grow_clipping_rectangle_by_portal_hull(
	real_rectangle2d *rectangle,
	struct portal_hull const *hull)
{
	real_point2d const *vertex;
	short vertex_index;

	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 1231, rectangle);
	match_assert("c:\\halo\\SOURCE\\structures\\structure_visibility.c", 1232, valid_portal_hull(hull));

	vertex = hull->vertices;

	for (vertex_index = 0; vertex_index < hull->vertex_count; vertex_index++)
	{
		if (rectangle->x0 > vertex->x)
		{
			rectangle->x0 = vertex->x;
		}

		if (rectangle->x1 < vertex->x)
		{
			rectangle->x1 = vertex->x;
		}

		if (rectangle->y0 > vertex->y)
		{
			rectangle->y0 = vertex->y;
		}

		if (rectangle->y1 < vertex->y)
		{
			rectangle->y1 = vertex->y;
		}

		vertex++;
	}

	return;
}

static void render_debug_hull(
	struct portal_hull const *hull,
	real offset,
	real_argb_color const *color)
{
	short vertex_index;
	real_point3d last_vertex;
	real_point3d vertex;

	if (hull->vertex_count)
	{
		last_vertex.x = hull->vertices[hull->vertex_count - 1].x;
		last_vertex.y = hull->vertices[hull->vertex_count - 1].y;
		last_vertex.z = -1.f;
		matrix4x3_transform_point(&render.frustum.view_to_world, &last_vertex, &last_vertex);
	}

	for (vertex_index = 0; vertex_index < hull->vertex_count; vertex_index++)
	{
		vertex.x = hull->vertices[vertex_index].x;
		vertex.y = hull->vertices[vertex_index].y;
		vertex.z = -1.f;
		matrix4x3_transform_point(&render.frustum.view_to_world, &vertex, &vertex);
		render_debug_line(TRUE, &vertex, &last_vertex, color);
		last_vertex = vertex;
	}

	return;
}

/* ---------- private code */
