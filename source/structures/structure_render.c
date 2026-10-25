/*
STRUCTURE_RENDER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "structure_render.h"
#include "structures.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "rasterizer_console_vars.h"
#include "shaders.h"
#include "render_debug.h"
#include "object_lights.h"
#include "structures/radiosity/radiosity.h"
#include "breakable_surfaces.h"

/* ---------- constants */

enum
{
	MAXIMUM_RENDERED_SURFACES_PER_POINT_LIGHT = 4096,
	MAXIMUM_RENDERED_SURFACES_PER_SHADOW = 4096,
};

enum
{
	_structure_material_coplanar_bit = 0,
	_structure_material_fog_plane_bit,
	NUMBER_OF_STRUCTURE_MATERIAL_FLAGS
};

enum
{
	_rasterizer_lightmap_mode_normal = 0, /* fake name */
	_rasterizer_lightmap_mode_no_lightmap, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct structure_render_globals
{
	boolean environment_geometry_valid;
	long environment_triangles_index;
	boolean fog_offset_valid;
	real_vector3d fog_offset_vector;
};

/* ---------- prototypes */

static boolean compare_surface_indices(long index1, long index2);
static void structure_render_dynamic_triangles_from_bitvector(short surface_count, long *result_surface_indices, unsigned long *surface_flags, struct rasterizer_triangle *triangles);
static void structure_render_dynamic_triangles_from_indices(short surface_count, long *surface_indices, struct rasterizer_triangle *triangles);
static long structure_render_build_dynamic_triangles(long *surface_indices, short surface_count, unsigned long *surface_flags);
static void structure_render_pass(long *surface_indices, short surface_count, long dynamic_triangles_index, void (*begin_lightmap)(struct bitmap_data const *lightmap), void (*draw_triangles)(struct shader const *shader, short shader_permutation_index, long dynamic_triangles_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer), void (*end_lightmap)(void), void (*draw_transparent_triangles)(struct shader const *shader, short shader_permutation_index, struct bitmap_data const *lightmap, long dynamic_triangles_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffers, real_point3d const *centroid, real_plane3d const *plane, real_vector3d const *vector, struct render_lighting const *lighting, unsigned long geometry_flags));

/* ---------- globals */

long debug_leaf_index = NONE;
long debug_leaf_portal_index = NONE;

static struct profile_section render_structure_build_triangle_arrays_section = {"render_structure_build_triangle_arrays", NONE, TRUE}; /* fake name */
static struct profile_section render_structure_lightmaps_section = {"render_structure_lightmaps", NONE, TRUE}; /* fake name */
static struct profile_section render_structure_diffuse_textures_section = {"render_structure_diffuse_textures", NONE, TRUE}; /* fake name */
static struct profile_section render_structure_reflections_section = {"render_structure_reflections", NONE, TRUE}; /* fake name */
static struct profile_section render_structure_transparent_geometry_section = {"render_structure_transparent_geometry", NONE, TRUE}; /* fake name */
static struct profile_section render_structure_fog_section = {"render_structure_fog", NONE, TRUE}; /* fake name */
static struct profile_section render_structure_specular_lights_section = {"render_structure_specular_lights", NONE, TRUE}; /* fake name */
static struct profile_section render_structure_specular_lights_draw_section = {"render_structure_specular_lights_draw", NONE, TRUE}; /* fake name */
static struct profile_section render_structure_diffuse_lights_section = {"render_structure_diffuse_lights", NONE, TRUE}; /* fake name */
static struct profile_section render_structure_diffuse_lights_draw_section = {"render_structure_diffuse_lights_draw", NONE, TRUE}; /* fake name */
static struct profile_section render_structure_shadows_section = {"render_structure_shadows", NONE, TRUE}; /* fake name */
static struct profile_section render_structure_shadows_draw_section = {"render_structure_shadows_draw", NONE, TRUE}; /* fake name */

static struct structure_render_globals structure_render_globals;

/* ---------- public code */

void structure_render_set_fog_offset(
	real_vector3d const *vector_offset)
{
	match_assert("c:\\halo\\SOURCE\\structures\\structure_render.c", 103, !structure_render_globals.fog_offset_valid);

	structure_render_globals.fog_offset_valid = TRUE;
	structure_render_globals.fog_offset_vector = *vector_offset;

	return;
}

static boolean compare_surface_indices(
	long index1,
	long index2)
{
	if (index1 < index2)
	{
		return FALSE;
	}
	else if (index2 < index1)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static void structure_render_dynamic_triangles_from_bitvector(
	short surface_count,
	long *result_surface_indices,
	unsigned long *surface_flags,
	struct rasterizer_triangle *triangles)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	short surface_index_index = 0;
	long surface_index;

	for (surface_index = 0; surface_index < structure_bsp->surfaces.count; surface_flags++)
	{
		if (*surface_flags)
		{
			short bit_index;

			for (bit_index = 0; bit_index < LONG_BITS && surface_index < structure_bsp->surfaces.count; bit_index++, surface_index++)
			{
				if (TEST_FLAG(*surface_flags, bit_index))
				{
					struct structure_surface *surface = TAG_BLOCK_GET_ELEMENT(&structure_bsp->surfaces, surface_index, struct structure_surface);

					match_assert("c:\\halo\\SOURCE\\structures\\structure_render.c", 421, surface_index_index>=0 && surface_index_index<surface_count);

					*result_surface_indices++ = surface_index;
					triangles[surface_index_index].vertex_indices[0] = surface->vertex_indices[0];
					triangles[surface_index_index].vertex_indices[1] = surface->vertex_indices[1];
					triangles[surface_index_index].vertex_indices[2] = surface->vertex_indices[2];
					surface_index_index++;
				}
			}
		}
		else
		{
			surface_index += LONG_BITS;
		}
	}

	return;
}

static void structure_render_dynamic_triangles_from_indices(
	short surface_count,
	long *surface_indices,
	struct rasterizer_triangle *triangles)
{
	short surface_index_index;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	qsort_4byte(surface_indices, surface_count, compare_surface_indices);

	for (surface_index_index = 0; surface_index_index < surface_count; surface_index_index++)
	{
		struct structure_surface *surface = TAG_BLOCK_GET_ELEMENT(&structure_bsp->surfaces, *surface_indices, struct structure_surface);

		triangles->vertex_indices[0] = surface->vertex_indices[0];
		triangles->vertex_indices[1] = surface->vertex_indices[1];
		triangles->vertex_indices[2] = surface->vertex_indices[2];
		surface_indices++;
		triangles++;
	}

	return;
}

static long structure_render_build_dynamic_triangles(
	long *surface_indices,
	short surface_count,
	unsigned long *surface_flags)
{
	static short once = TRUE;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	long dynamic_triangle_buffer_index = NONE;

	if (surface_count > 0)
	{
		dynamic_triangle_buffer_index = rasterizer_dynamic_triangles_new(surface_count);

		if (dynamic_triangle_buffer_index != NONE)
		{
			struct rasterizer_triangle *triangles = rasterizer_dynamic_triangles_lock(dynamic_triangle_buffer_index);

			match_assert("c:\\halo\\SOURCE\\structures\\structure_render.c", 486, triangles);

			if (surface_flags)
			{
				structure_render_dynamic_triangles_from_bitvector(
					surface_count,
					surface_indices,
					surface_flags,
					triangles);
			}
			else
			{
				structure_render_dynamic_triangles_from_indices(surface_count, surface_indices, triangles);
			}

			rasterizer_dynamic_triangles_unlock(dynamic_triangle_buffer_index);
		}
		else if (once)
		{
			error(_error_silent, "unable to allocate dynamic structure triangles.");
			once = FALSE;
		}
	}

	return dynamic_triangle_buffer_index;
}

static void structure_render_pass(
	long *surface_indices,
	short surface_count,
	long dynamic_triangles_index,
	void (*begin_lightmap)(struct bitmap_data const *lightmap),
	void (*draw_triangles)(struct shader const *shader, short shader_permutation_index, long dynamic_triangles_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffer),
	void (*end_lightmap)(void),
	void (*draw_transparent_triangles)(struct shader const *shader, short shader_permutation_index, struct bitmap_data const *lightmap, long dynamic_triangles_index, long first_triangle_index, long triangle_count, struct vertex_buffer const *vertex_buffers, real_point3d const *centroid, real_plane3d const *plane, real_vector3d const *vector, struct render_lighting const *lighting, unsigned long geometry_flags))
{
	short lightmap_index;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	long *last_surface_index = surface_indices + surface_count;
	long first_triangle_index = 0;

	for (lightmap_index = 0; lightmap_index < structure_bsp->lightmaps.count && surface_indices < last_surface_index; lightmap_index++)
	{
		struct structure_lightmap *lightmap = TAG_BLOCK_GET_ELEMENT(&structure_bsp->lightmaps, lightmap_index, struct structure_lightmap);
		struct structure_material *last_material = TAG_BLOCK_GET_ELEMENT(&lightmap->materials, lightmap->materials.count - 1, struct structure_material);

		if (*surface_indices < last_material->first_surface_index + last_material->surface_count)
		{
			struct bitmap_data *lightmap_bitmap;
			short material_index;

			if (structure_bsp->lightmap_group.index == NONE)
			{
				lightmap_bitmap = NULL;
			}
			else
			{
				lightmap_bitmap = bitmap_group_try_and_get_bitmap(structure_bsp->lightmap_group.index, lightmap->bitmap_index);
			}

			if (begin_lightmap)
			{
				begin_lightmap(lightmap_bitmap);
			}

			for (material_index = 0; material_index < lightmap->materials.count && surface_indices < last_surface_index; material_index++)
			{
				struct structure_material *material = TAG_BLOCK_GET_ELEMENT(&lightmap->materials, material_index, struct structure_material);

				if (*surface_indices < material->first_surface_index + material->surface_count)
				{
					short triangle_count;
					long *first_surface_index = surface_indices;
					struct shader *shader = shader_definition_get(material->shader.index);

					do
					{
						surface_indices++;
					}
					while (surface_indices < last_surface_index && *surface_indices < material->first_surface_index + material->surface_count);

					triangle_count = surface_indices - first_surface_index;

					if (breakable_surface_extant(material->breakable_surface_index))
					{
						if (!shader_type_is_transparent(shader->base.type))
						{
							if (draw_triangles)
							{
								draw_triangles(
									shader,
									material->permutation_index,
									dynamic_triangles_index,
									first_triangle_index,
									triangle_count,
									&material->vertices);
							}
						}
						else if (draw_transparent_triangles)
						{
							draw_transparent_triangles(
								shader,
								material->permutation_index,
								lightmap_bitmap,
								dynamic_triangles_index,
								first_triangle_index,
								triangle_count,
								&material->vertices,
								&material->centroid,
								TEST_FLAG(material->flags, _structure_material_coplanar_bit) ? &material->plane : NULL,
								TEST_FLAG(material->flags, _structure_material_fog_plane_bit) ? &structure_render_globals.fog_offset_vector : global_zero_vector3d,
								&material->lighting,
								0);
						}
					}

					first_triangle_index += triangle_count;
				}
			}

			if (end_lightmap)
			{
				end_lightmap();
			}
		}
	}

	match_vassert(
		"c:\\halo\\SOURCE\\structures\\structure_render.c",
		599,
		surface_indices>=last_surface_index,
		"there are more surfaces than materials that reference them, stupid.");

	return;
}

void structure_render_preprocess(
	void)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	profile_enter(render_structure_build_triangle_arrays_section);
	structure_render_globals.environment_triangles_index = structure_render_build_dynamic_triangles(
		render.environment_surface_indices,
		render.environment_surface_count,
		render.environment_surface_flags);
	profile_exit(render_structure_build_triangle_arrays_section);
	structure_render_globals.environment_geometry_valid = structure_render_globals.environment_triangles_index != NONE;

	if (debug_leaf_index >= 0 && debug_leaf_index < structure_bsp->leaf_map.leaves.count)
	{
		render_debug_leaf_portals(&structure_bsp->leaf_map, debug_leaf_index);
	}

	if (debug_leaf_portal_index >= 0 && debug_leaf_portal_index < structure_bsp->leaf_map.portals.count)
	{
		render_debug_leaf_portal(&structure_bsp->leaf_map, debug_leaf_portal_index);
	}

	if (debug_leaf_portals)
	{
		long portal_index;

		for (portal_index = 0; portal_index < structure_bsp->leaf_map.portals.count; portal_index++)
		{
			struct leaf_portal *portal = TAG_BLOCK_GET_ELEMENT(&structure_bsp->leaf_map.portals, portal_index, struct leaf_portal);

			render_debug_leaf_portal(&structure_bsp->leaf_map, portal_index);
		}
	}

	structure_render_globals.fog_offset_valid = FALSE;
	structure_render_globals.fog_offset_vector = *global_zero_vector3d;

	return;
}

void structure_render_lightmaps(
	void)
{
	profile_enter(render_structure_lightmaps_section);

	if (structure_render_globals.environment_geometry_valid)
	{
		short lightmap_mode = rasterizer_globals.lightmap_mode;

		if (global_structure_bsp_get()->lightmap_group.index == NONE && rasterizer_globals.lightmap_mode == _rasterizer_lightmap_mode_normal)
		{
			rasterizer_globals.lightmap_mode = _rasterizer_lightmap_mode_no_lightmap;
		}

		rasterizer_environment_lightmaps_begin();
		structure_render_pass(
			render.environment_surface_indices,
			render.environment_surface_count,
			structure_render_globals.environment_triangles_index,
			rasterizer_environment_lightmap_begin,
			rasterizer_environment_lightmap_draw,
			rasterizer_environment_lightmap_end,
			NULL);
		rasterizer_environment_lightmaps_end();
		rasterizer_globals.lightmap_mode = lightmap_mode;
	}

	profile_exit(render_structure_lightmaps_section);

	return;
}

void structure_render_diffuse_texture(
	void)
{
	profile_enter(render_structure_diffuse_textures_section);

	if (structure_render_globals.environment_geometry_valid)
	{
		rasterizer_environment_diffuse_textures_begin();
		structure_render_pass(
			render.environment_surface_indices,
			render.environment_surface_count,
			structure_render_globals.environment_triangles_index,
			NULL,
			rasterizer_environment_diffuse_texture_draw,
			NULL,
			NULL);
		rasterizer_environment_diffuse_textures_end();
	}

	profile_exit(render_structure_diffuse_textures_section);

	return;
}

void structure_render_specular_lightmaps(
	void)
{
	if (structure_render_globals.environment_geometry_valid)
	{
		short lightmap_mode = rasterizer_globals.lightmap_mode;

		if (global_structure_bsp_get()->lightmap_group.index == NONE && rasterizer_globals.lightmap_mode == _rasterizer_lightmap_mode_normal)
		{
			rasterizer_globals.lightmap_mode = _rasterizer_lightmap_mode_no_lightmap;
		}

		rasterizer_environment_specular_lightmaps_begin();
		structure_render_pass(
			render.environment_surface_indices,
			render.environment_surface_count,
			structure_render_globals.environment_triangles_index,
			rasterizer_environment_specular_lightmap_begin,
			rasterizer_environment_specular_lightmap_draw,
			rasterizer_environment_specular_lightmap_end,
			NULL);
		rasterizer_environment_specular_lightmaps_end();
		rasterizer_globals.lightmap_mode = lightmap_mode;
	}

	return;
}

void structure_render_reflection_lightmap_masks(
	void)
{
	if (structure_render_globals.environment_geometry_valid)
	{
		rasterizer_environment_reflection_lightmap_masks_begin();
		structure_render_pass(
			render.environment_surface_indices,
			render.environment_surface_count,
			structure_render_globals.environment_triangles_index,
			rasterizer_environment_reflection_lightmap_mask_begin,
			rasterizer_environment_reflection_lightmap_mask_draw,
			rasterizer_environment_reflection_lightmap_mask_end,
			NULL);
		rasterizer_environment_reflection_lightmap_masks_end();
	}

	return;
}

void structure_render_reflection_mirrors(
	void)
{
	if (structure_render_globals.environment_geometry_valid)
	{
		rasterizer_environment_reflection_mirrors_begin();
		structure_render_pass(
			render.environment_surface_indices,
			render.environment_surface_count,
			structure_render_globals.environment_triangles_index,
			NULL,
			rasterizer_environment_reflection_mirror_draw,
			NULL,
			NULL);
		rasterizer_environment_reflection_mirrors_end();
	}

	return;
}

void structure_render_reflections(
	void)
{
	profile_enter(render_structure_reflections_section);

	if (structure_render_globals.environment_geometry_valid)
	{
		rasterizer_environment_reflections_begin();
		structure_render_pass(
			render.environment_surface_indices,
			render.environment_surface_count,
			structure_render_globals.environment_triangles_index,
			NULL,
			rasterizer_environment_reflection_draw,
			NULL,
			NULL);
		rasterizer_environment_reflections_end();
	}

	profile_exit(render_structure_reflections_section);

	return;
}

void structure_render_transparent_geometry(
	void)
{
	profile_enter(render_structure_transparent_geometry_section);

	if (structure_render_globals.environment_geometry_valid)
	{
		rasterizer_environment_transparent_geometry_begin();
		structure_render_pass(
			render.environment_surface_indices,
			render.environment_surface_count,
			structure_render_globals.environment_triangles_index,
			NULL,
			NULL,
			NULL,
			rasterizer_environment_transparent_geometry_submit);
		rasterizer_environment_transparent_geometry_end();
	}

	profile_exit(render_structure_transparent_geometry_section);

	return;
}

void structure_render_fog(
	void)
{
	profile_enter(render_structure_fog_section);

	if (structure_render_globals.environment_geometry_valid)
	{
		rasterizer_environment_fog_begin();
		structure_render_pass(
			render.environment_surface_indices,
			render.environment_surface_count,
			structure_render_globals.environment_triangles_index,
			NULL,
			rasterizer_environment_fog_draw,
			NULL,
			NULL);
		rasterizer_environment_fog_end();
	}

	profile_exit(render_structure_fog_section);

	return;
}

void structure_render_fog_screen(
	void)
{
	if (structure_render_globals.environment_geometry_valid)
	{
		rasterizer_environment_fog_screen_begin(0);
		structure_render_pass(
			render.environment_surface_indices,
			render.environment_surface_count,
			structure_render_globals.environment_triangles_index,
			NULL,
			rasterizer_environment_fog_screen_draw,
			NULL,
			NULL);
		rasterizer_environment_fog_screen_end();
		rasterizer_environment_fog_screen_begin(1);
		structure_render_pass(
			render.environment_surface_indices,
			render.environment_surface_count,
			structure_render_globals.environment_triangles_index,
			NULL,
			rasterizer_environment_fog_screen_draw,
			NULL,
			NULL);
		rasterizer_environment_fog_screen_end();
	}

	return;
}

void structure_render_specular_light(
	long rasterizer_light_index,
	real_point3d const *bounding_sphere_center,
	real bounding_sphere_radius,
	short cluster_count,
	short const *cluster_indices)
{
	long surface_indices_storage[MAXIMUM_RENDERED_SURFACES_PER_POINT_LIGHT];
	long surface_count;
	long dynamic_triangle_buffer_index;
	long *surface_indices = surface_indices_storage;

	profile_enter(render_structure_specular_lights_section);

	if (cluster_indices)
	{
		surface_count = structure_visibility_build_surfaces(
			surface_indices_storage,
			MAXIMUM_RENDERED_SURFACES_PER_POINT_LIGHT,
			bounding_sphere_center,
			bounding_sphere_radius,
			NULL,
			0,
			NULL,
			cluster_count,
			cluster_indices);
		dynamic_triangle_buffer_index = structure_render_build_dynamic_triangles(surface_indices_storage, surface_count, NULL);
	}
	else
	{
		dynamic_triangle_buffer_index = structure_render_globals.environment_triangles_index;
		surface_indices = render.environment_surface_indices;
		surface_count = render.environment_surface_count;
	}

	if (dynamic_triangle_buffer_index != NONE)
	{
		profile_enter(render_structure_specular_lights_draw_section);
		rasterizer_environment_specular_light_begin(rasterizer_light_index);
		structure_render_pass(
			surface_indices,
			surface_count,
			dynamic_triangle_buffer_index,
			NULL,
			rasterizer_environment_specular_light_draw,
			NULL,
			NULL);
		rasterizer_environment_specular_light_end();

		if (cluster_indices)
		{
			rasterizer_dynamic_triangles_delete(dynamic_triangle_buffer_index);
		}

		profile_exit(render_structure_specular_lights_draw_section);
	}

	profile_exit(render_structure_specular_lights_section);

	return;
}

void structure_render_diffuse_light(
	long rasterizer_light_index,
	real_point3d const *bounding_sphere_center,
	real bounding_sphere_radius,
	short cluster_count,
	short const *cluster_indices)
{
	long surface_indices_storage[MAXIMUM_RENDERED_SURFACES_PER_POINT_LIGHT];
	long surface_count;
	long dynamic_triangle_buffer_index;
	long *surface_indices = surface_indices_storage;

	profile_enter(render_structure_diffuse_lights_section);

	if (cluster_indices)
	{
		surface_count = structure_visibility_build_surfaces(
			surface_indices_storage,
			MAXIMUM_RENDERED_SURFACES_PER_POINT_LIGHT,
			bounding_sphere_center,
			bounding_sphere_radius,
			NULL,
			0,
			NULL,
			cluster_count,
			cluster_indices);
		dynamic_triangle_buffer_index = structure_render_build_dynamic_triangles(surface_indices_storage, surface_count, NULL);
	}
	else
	{
		dynamic_triangle_buffer_index = structure_render_globals.environment_triangles_index;
		surface_indices = render.environment_surface_indices;
		surface_count = render.environment_surface_count;
	}

	if (dynamic_triangle_buffer_index != NONE)
	{
		profile_enter(render_structure_diffuse_lights_draw_section);
		rasterizer_environment_diffuse_light_begin(rasterizer_light_index);
		structure_render_pass(
			surface_indices,
			surface_count,
			dynamic_triangle_buffer_index,
			NULL,
			rasterizer_environment_diffuse_light_draw,
			NULL,
			NULL);

		if (cluster_indices)
		{
			rasterizer_dynamic_triangles_delete(dynamic_triangle_buffer_index);
		}

		rasterizer_environment_diffuse_light_end();
		profile_exit(render_structure_diffuse_lights_draw_section);
	}

	profile_exit(render_structure_diffuse_lights_section);

	return;
}

void structure_render_shadow(
	real_point3d const *bounding_sphere_center,
	real bounding_radius,
	real_rectangle3d const *bounding_box,
	short bounding_polyhedra_count,
	real_plane3d const *bounding_polyhedra)
{
	long surface_indices[MAXIMUM_RENDERED_SURFACES_PER_SHADOW];
	long surface_count;
	long dynamic_triangle_buffer_index;

	profile_enter(render_structure_shadows_section);
	surface_count = structure_visibility_build_surfaces(
		surface_indices,
		MAXIMUM_RENDERED_SURFACES_PER_SHADOW,
		bounding_sphere_center,
		bounding_radius,
		bounding_box,
		bounding_polyhedra_count,
		bounding_polyhedra,
		0,
		NULL);
	dynamic_triangle_buffer_index = structure_render_build_dynamic_triangles(surface_indices, surface_count, NULL);

	if (dynamic_triangle_buffer_index != NONE)
	{
		profile_enter(render_structure_shadows_draw_section);
		structure_render_pass(
			surface_indices,
			surface_count,
			dynamic_triangle_buffer_index,
			NULL,
			rasterizer_environment_shadow_draw,
			NULL,
			NULL);
		rasterizer_dynamic_triangles_delete(dynamic_triangle_buffer_index);
		profile_exit(render_structure_shadows_draw_section);
	}

	profile_exit(render_structure_shadows_section);

	return;
}
