/*
STRUCTURES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "structures.h"
#include "objects.h"
#include "render.h"
#include "rasterizer.h"
#include "console.h"
#include "light_definitions.h"
#include "sound_environment_definitions.h"
#include "collisions.h"
#include "fog_definitions.h"
#include "structures/radiosity/radiosity.h"
#include "vector_tree.h"
#include "sky_definitions.h"
#include "structures/radiosity/radiosity_io.h"

/* ---------- constants */

enum
{
	_cluster_fog_designator_plane_bit = 15	/* fake name */
};

#define STRUCTURE_TEST_VECTOR_STEP (1.f / 4096.f)	/* fake name */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static short structure_clusters_in_sphere_recursive(short cluster_index, real_point3d const *position, real radius, short maximum_count, short *intersected_indices);

/* ---------- globals */

boolean debug_fog_planes;

static struct
{
	boolean initialized;
	boolean cluster_marker_initialized;
	long cluster_marker;
	long cluster_magic_numbers[MAXIMUM_CLUSTERS_PER_STRUCTURE];
} structure_globals;

/* ---------- public code */

void structures_initialize(
	void)
{
	structure_detail_objects_initialize();
	structure_decals_initialize();

	return;
}

void structures_initialize_for_new_map(
	void)
{
	structure_detail_objects_initialize_for_new_map();
	structure_decals_initialize_for_new_map();

	return;
}

void structures_dispose_from_old_map(
	void)
{
	structure_decals_dispose_from_old_map();
	structure_detail_objects_dispose_from_old_map();

	return;
}

void structures_dispose(
	void)
{
	structure_decals_dispose();
	structure_detail_objects_dispose();

	return;
}

boolean sphere_intersects_cluster_portal(
	struct structure_bsp *structure,
	short portal_index,
	real_point3d const *point,
	real radius)
{
	boolean result = FALSE;
	struct cluster_portal *portal = TAG_BLOCK_GET_ELEMENT(&structure->cluster_portals, portal_index, struct cluster_portal);
	real distance = plane3d_distance_to_point(
		TAG_BLOCK_GET_ELEMENT(
			&TAG_BLOCK_GET_ELEMENT(&structure->collision_bsp, 0, struct collision_bsp)->bsp3d.planes,
			portal->plane_index,
			real_plane3d),
		point);

	if (fabs(distance) < radius)
	{
		if (distance_squared3d(point, &portal->centroid) < (radius + portal->bounding_radius) * (radius + portal->bounding_radius))
		{
			real_point2d projected_points[MAXIMUM_VERTICES_PER_CLUSTER_PORTAL];
			real_point3d projected_centroid3d;
			real_point2d projected_centroid2d;
			short vertex_index;
			real_plane3d *plane = TAG_BLOCK_GET_ELEMENT(&global_bsp3d_get()->planes, portal->plane_index, real_plane3d);
			short projection = projection_from_vector3d(&plane->n);
			short sign = projection_sign_from_vector3d(&plane->n, projection);

			vector_from_line3d(point, &plane->n, -distance, &projected_centroid3d);
			project_point3d(&projected_centroid3d, projection, sign, &projected_centroid2d);

			for (vertex_index = 0; vertex_index < portal->vertices.count; vertex_index++)
			{
				project_point3d(
					TAG_BLOCK_GET_ELEMENT(&portal->vertices, vertex_index, real_point3d),
					projection,
					sign,
					&projected_points[vertex_index]);
			}

			if (convex_hull2d_test_circle(
				(short)portal->vertices.count,
				projected_points,
				&projected_centroid2d,
				square_root(radius * radius - distance * distance)))
			{
				result = TRUE;
			}
		}
	}

	return result;
}

short structure_clusters_in_sphere(
	short cluster_index,
	real_point3d const *position,
	real radius,
	short maximum_count,
	short *intersected_indices)
{
	short intersected_count = 0;

	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 134, position);
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 135, radius>=0.f);
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 136, maximum_count>0);
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 137, intersected_indices);

	if (cluster_index != NONE)
	{
		if (radius > 0.f)
		{
			structure_cluster_marker_begin();
			intersected_count = structure_clusters_in_sphere_recursive(
				cluster_index,
				position,
				radius,
				maximum_count,
				intersected_indices);
			structure_cluster_marker_end();
		}
		else if (maximum_count > 0)
		{
			*intersected_indices = cluster_index;
			intersected_count = 1;
		}
	}

	return intersected_count;
}

static short structure_clusters_in_sphere_recursive(
	short cluster_index,
	real_point3d const *position,
	real radius,
	short maximum_count,
	short *intersected_indices)
{
	short intersected_count;
	short portal_reference_index;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_index, struct structure_cluster);

	if (maximum_count-- > 0)
	{
		*intersected_indices++ = cluster_index;
	}

	structure_cluster_mark(cluster_index);
	intersected_count = 1;

	for (portal_reference_index = 0; portal_reference_index < cluster->portal_indices.count; portal_reference_index++)
	{
		short portal_index = *TAG_BLOCK_GET_ELEMENT(&cluster->portal_indices, portal_reference_index, short);
		struct cluster_portal *portal = TAG_BLOCK_GET_ELEMENT(&structure_bsp->cluster_portals, portal_index, struct cluster_portal);
		short next_cluster_index = portal->cluster_indices[0] == cluster_index ? portal->cluster_indices[1] : portal->cluster_indices[0];

		if (structure_cluster_unmarked(next_cluster_index) &&
			sphere_intersects_cluster_portal(structure_bsp, portal_index, position, radius))
		{
			short count = structure_clusters_in_sphere_recursive(
				next_cluster_index,
				position,
				radius,
				maximum_count,
				intersected_indices);

			intersected_count += count;
			maximum_count -= count;
			intersected_indices += count;
		}
	}

	return intersected_count;
}

short structure_clusters_in_cone(
	short position_cluster_index,
	real_point3d const *position,
	real_vector3d const *direction,
	real distance,
	real sine,
	real cosine,
	short maximum_cluster_count,
	short *cluster_indices)
{
	struct structure_bsp *structure_bsp;
	short stack_depth;
	short cluster_stack[MAXIMUM_CLUSTERS_PER_STRUCTURE];
	short cluster_count = 0;

	structure_cluster_marker_begin();
	structure_bsp = global_structure_bsp_get();
	structure_cluster_mark(position_cluster_index);
	cluster_stack[0] = position_cluster_index;
	stack_depth = 1;

	while (stack_depth > 0 && cluster_count < maximum_cluster_count)
	{
		short portal_reference_index;
		short cluster_index = cluster_stack[--stack_depth];
		struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_index, struct structure_cluster);

		cluster_indices[cluster_count++] = cluster_index;

		for (portal_reference_index = 0; portal_reference_index < cluster->portal_indices.count; portal_reference_index++)
		{
			short portal_index = *TAG_BLOCK_GET_ELEMENT(&cluster->portal_indices, portal_reference_index, short);
			struct cluster_portal *portal = TAG_BLOCK_GET_ELEMENT(&structure_bsp->cluster_portals, portal_index, struct cluster_portal);
			short next_cluster_index = portal->cluster_indices[0] == cluster_index ? portal->cluster_indices[1] : portal->cluster_indices[0];

			if (structure_cluster_unmarked(next_cluster_index) &&
				sphere_intersects_cone3d(&portal->centroid, portal->bounding_radius, position, direction, distance, sine, cosine))
			{
				structure_cluster_mark(next_cluster_index);
				match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 245, stack_depth<MAXIMUM_CLUSTERS_PER_STRUCTURE);
				cluster_stack[stack_depth++] = next_cluster_index;
			}
		}
	}

	structure_cluster_marker_end();

	return cluster_count;
}

void structure_cluster_marker_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 259, !structure_globals.cluster_marker_initialized);
	structure_globals.cluster_marker++;
	structure_globals.cluster_marker_initialized = TRUE;

	return;
}

boolean structure_cluster_unmarked(
	short cluster_index)
{
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 270, structure_globals.cluster_marker_initialized);
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 271, cluster_index>=0 && cluster_index<MAXIMUM_CLUSTERS_PER_STRUCTURE);

	if (structure_globals.cluster_magic_numbers[cluster_index] != structure_globals.cluster_marker)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

boolean structure_cluster_mark(
	short cluster_index)
{
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 286, structure_globals.cluster_marker_initialized);
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 287, cluster_index>=0 && cluster_index<MAXIMUM_CLUSTERS_PER_STRUCTURE);

	if (structure_globals.cluster_magic_numbers[cluster_index] != structure_globals.cluster_marker)
	{
		structure_globals.cluster_magic_numbers[cluster_index] = structure_globals.cluster_marker;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void structure_cluster_marker_end(
	void)
{
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 304, structure_globals.cluster_marker_initialized);
	structure_globals.cluster_marker_initialized = FALSE;

	return;
}

boolean structure_render_surface_from_point_and_leaf(
	real_point3d const *point,
	long leaf_index,
	long plane_index,
	short *lightmap_index,
	short *material_index,
	long *surface_index,
	real *s,
	real *t)
{
	long surface_reference_index;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	struct structure_leaf *leaf = TAG_BLOCK_GET_ELEMENT(&structure_bsp->leaves, leaf_index & LONG_MAX, struct structure_leaf);
	boolean success = FALSE;

	for (surface_reference_index = leaf->first_surface_reference_index;
		surface_reference_index < leaf->first_surface_reference_index + leaf->surface_reference_count;
		surface_reference_index++)
	{
		struct structure_surface_reference *surface_reference = TAG_BLOCK_GET_ELEMENT(
			&structure_bsp->surface_references,
			surface_reference_index,
			struct structure_surface_reference);

		if (surface_reference->bsp3d_node_index != NONE)
		{
			struct bsp3d_node *node = TAG_BLOCK_GET_ELEMENT(
				&TAG_BLOCK_GET_ELEMENT(&structure_bsp->collision_bsp, 0, struct collision_bsp)->bsp3d.nodes,
				surface_reference->bsp3d_node_index,
				struct bsp3d_node);

			if (node->plane == plane_index)
			{
				struct structure_lightmap *lightmap;
				struct structure_material *material;
				struct structure_surface *surface = TAG_BLOCK_GET_ELEMENT(&structure_bsp->surfaces, surface_reference->surface_index, struct structure_surface);

				structure_bsp_find_material_for_surface(
					structure_bsp,
					surface_reference->surface_index,
					lightmap_index,
					material_index);
				lightmap = TAG_BLOCK_GET_ELEMENT(&structure_bsp->lightmaps, *lightmap_index, struct structure_lightmap);
				material = TAG_BLOCK_GET_ELEMENT(&lightmap->materials, *material_index, struct structure_material);

				if (material->vertices.type == _rasterizer_vertex_type_environment_uncompressed ||
					material->vertices.type == _rasterizer_vertex_type_environment_compressed)
				{
					real_point3d triangle_points[NUMBER_OF_VERTICES_PER_TRIANGLE];

					environment_vertex_compressed_get_point(
						(struct environment_vertex_compressed *)material->compressed_vertex_data.address + surface->vertex_indices[0],
						&triangle_points[0]);
					environment_vertex_compressed_get_point(
						(struct environment_vertex_compressed *)material->compressed_vertex_data.address + surface->vertex_indices[1],
						&triangle_points[1]);
					environment_vertex_compressed_get_point(
						(struct environment_vertex_compressed *)material->compressed_vertex_data.address + surface->vertex_indices[2],
						&triangle_points[2]);

					if (point_in_triangle3d(point, &triangle_points[0], &triangle_points[1], &triangle_points[2], s, t))
					{
						*surface_index = surface_reference->surface_index;
						success = TRUE;
						break;
					}
				}
			}
		}
	}

	return success;
}

boolean structure_test_vector(
	real_point3d const *p,
	real_vector3d const *v,
	real_point3d *collision_point,
	short *lightmap_index,
	short *material_index,
	long *surface_index,
	real *s,
	real *t)
{
	struct collision_result collision;
	boolean done;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 392, p);
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 393, v);
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 394, material_index);
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 395, surface_index);
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 396, s);
	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 397, t);

	*collision_point = *p;

	do
	{
		done = TRUE;

		match_collision_log_begin_user("c:\\halo\\SOURCE\\structures\\structures.c", 406, _collision_user_structure_lighting);

		if (collision_test_vector(
			FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_structure_bit),
			collision_point,
			v,
			NONE,
			&collision))
		{
			struct structure_bsp *structure_bsp = global_structure_bsp_get();

			*collision_point = collision.point;

			if (structure_render_surface_from_point_and_leaf(
				collision_point,
				collision.location.leaf_index,
				collision.plane_designator & LONG_MAX,
				lightmap_index,
				material_index,
				surface_index,
				s,
				t) &&
				TAG_BLOCK_GET_ELEMENT(&structure_bsp->lightmaps, *lightmap_index, struct structure_lightmap)->bitmap_index != NONE)
			{
				success = TRUE;
			}
			else if (TEST_FLAG(collision.flags, _collision_surface_two_sided_bit))
			{
				point_from_line3d(collision_point, v, STRUCTURE_TEST_VECTOR_STEP, collision_point);
				done = FALSE;
			}
		}

		match_collision_log_end_user("c:\\halo\\SOURCE\\structures\\structures.c", 426);
	}
	while (!done);

	return success;
}

long structure_get_planar_fog_definition_index(
	struct structure_bsp *structure_bsp,
	short cluster_index,
	boolean sky)
{
	long definition_index = NONE;

	if (cluster_index != NONE)
	{
		struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, cluster_index, struct structure_cluster);

		if (sky)
		{
			struct sky *sky_definition = scenario_get_sky(0);

			if (sky_definition)
			{
				definition_index = sky_definition->indoor_fog_plane.index;
			}
		}
		else if (cluster->fog_designator != NONE)
		{
			short region_index;

			if (cluster->fog_designator & FLAG(_cluster_fog_designator_plane_bit))
			{
				region_index = TAG_BLOCK_GET_ELEMENT(
					&structure_bsp->fog_planes,
					cluster->fog_designator & MASK(_cluster_fog_designator_plane_bit),
					struct structure_fog_plane)->region_index;
			}
			else
			{
				region_index = cluster->fog_designator & MASK(_cluster_fog_designator_plane_bit);
			}

			if (region_index != NONE)
			{
				struct structure_fog_region *region = TAG_BLOCK_GET_ELEMENT(&structure_bsp->fog_regions, region_index, struct structure_fog_region);

				if (region->fog_palette_index != NONE)
				{
					struct structure_fog_palette_entry *palette_entry = TAG_BLOCK_GET_ELEMENT(
						&structure_bsp->fog_palette,
						region->fog_palette_index,
						struct structure_fog_palette_entry);

					definition_index = palette_entry->fog.index;
				}
			}
		}
	}

	return definition_index;
}

void structure_get_planar_fog(
	short cluster_index,
	struct render_fog *fog)
{
	long definition_index;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	boolean sky_fog = FALSE;

	match_assert("c:\\halo\\SOURCE\\structures\\structures.c", 497, fog);

	fog->planar_mode = _render_planar_fog_mode_off;
	fog->fog_definition_flags = 0;
	fog->screen = NULL;
	definition_index = structure_get_planar_fog_definition_index(structure_bsp, cluster_index, FALSE);

	if (definition_index == NONE)
	{
		definition_index = structure_get_planar_fog_definition_index(structure_bsp, cluster_index, TRUE);
		sky_fog = TRUE;
	}

	if (definition_index != NONE)
	{
		struct structure_bsp *structure = global_structure_bsp_get();
		struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure->clusters, cluster_index, struct structure_cluster);
		struct fog_definition *definition = fog_definition_get(definition_index);

		if (sky_fog)
		{
			SET_FLAG(fog->runtime_flags, _render_fog_runtime_screen_use_sky_interpolator_bit, TRUE);
			fog->fog_definition_flags = (word)definition->flags;
			fog->screen = &definition->screen;
		}
		else
		{
			if (cluster->fog_designator & FLAG(_cluster_fog_designator_plane_bit))
			{
				fog->planar_mode = _render_planar_fog_mode_normal;
				fog->plane = TAG_BLOCK_GET_ELEMENT(
					&structure->fog_planes,
					cluster->fog_designator & MASK(_cluster_fog_designator_plane_bit),
					struct structure_fog_plane)->plane;
			}
			else
			{
				fog->planar_mode = _render_planar_fog_mode_fully_fogged;
			}

			fog->planar_color = definition->color;
			fog->planar_maximum_density = definition->maximum_density;
			fog->planar_maximum_depth = definition->maximum_depth;
			fog->planar_maximum_distance = definition->maximum_distance;

			if (cluster->fog_designator & FLAG(_cluster_fog_designator_plane_bit))
			{
				struct structure_fog_plane *fog_plane = TAG_BLOCK_GET_ELEMENT(
					&structure->fog_planes,
					cluster->fog_designator & MASK(_cluster_fog_designator_plane_bit),
					struct structure_fog_plane);
				real offset = definition->animation_distance * 0.f;
				real_vector3d vector_offset;

				fog->plane.d += offset;
				scale_vector3d(&fog->plane.n, offset, &vector_offset);
				structure_render_set_fog_offset(&vector_offset);
			}

			fog->fog_definition_flags = (word)definition->flags;
			fog->screen = &definition->screen;
		}
	}

	return;
}

void render_debug_fog_planes(
	void)
{
	if (debug_fog_planes)
	{
		if (render.fog.planar_mode == _render_planar_fog_mode_normal && render.cluster_index != NONE)
		{
			struct structure_bsp *structure_bsp = global_structure_bsp_get();
			struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(&structure_bsp->clusters, render.cluster_index, struct structure_cluster);
			struct structure_fog_plane *fog_plane = TAG_BLOCK_GET_ELEMENT(
				&structure_bsp->fog_planes,
				cluster->fog_designator & MASK(_cluster_fog_designator_plane_bit),
				struct structure_fog_plane);
			short vertex_index;

			for (vertex_index = 0; vertex_index < fog_plane->vertices.count; vertex_index++)
			{
				real_point3d offset_vertex;
				real_point3d next_offset_vertex;
				short next_vertex_index = (vertex_index + 1) % fog_plane->vertices.count;
				real_point3d *vertex = TAG_BLOCK_GET_ELEMENT(&fog_plane->vertices, vertex_index, real_point3d);
				real_point3d *next_vertex = TAG_BLOCK_GET_ELEMENT(&fog_plane->vertices, next_vertex_index, real_point3d);

				point_from_line3d(vertex, &fog_plane->plane.n, -render.fog.planar_maximum_distance, &offset_vertex);
				point_from_line3d(next_vertex, &fog_plane->plane.n, -render.fog.planar_maximum_distance, &next_offset_vertex);
				rasterizer_debug_line(vertex, next_vertex, global_real_argb_white);
				rasterizer_debug_line(&offset_vertex, &next_offset_vertex, global_real_argb_black);
				rasterizer_debug_line_shaded(vertex, &offset_vertex, global_real_argb_white, global_real_argb_black);
				rasterizer_debug_line_shaded(next_vertex, &next_offset_vertex, global_real_argb_white, global_real_argb_black);
			}
		}
	}

	return;
}

/* ---------- private code */

