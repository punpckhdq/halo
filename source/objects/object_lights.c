/*
OBJECT_LIGHTS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "object_lights.h"
#include "light_definitions.h"
#include "cluster_partitions.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "rasterizer.h"
#include "players.h"
#include "bitmaps_inlines.h"
#include "game_state.h"
#include "collisions.h"
#include "render_debug.h"
#include "director.h"
#include "texture_cache.h"
#include "structures.h"
#include "devices.h"
#include "first_person_weapons.h"
#include "device_definitions.h"
#include "structure_render.h"

/* ---------- constants */

enum
{
	_point_light_dynamic_bit = 0,
	_point_light_connects_to_map_bit,
	_point_light_connected_to_map_bit,
	_point_light_attached_to_first_person_weapon_bit,
	NUMBER_OF_POINT_LIGHT_FLAGS,
};

enum
{
	MAXIMUM_CLUSTERS_PER_LIGHT = 512, /* fake name */
	NUMBER_OF_LIGHTMAP_SAMPLE_RAYCASTS_SIDEWAYS = 4, /* fake name */
};

#define SQUARE_ROOT_ONE_HALF 0.70710678f /* fake name */
#define MAXIMUM_SHADOW_VECTOR_COMPONENT 0.707f /* fake name */

/* ---------- macros */

#define light_get(index) ((struct point_light *)datum_get(light_data, (index))) /* fake name */

#define structure_material_get_vertex(material, vertex_index) ((struct environment_vertex_compressed const *)(material)->compressed_vertex_data.address + (vertex_index)) /* fake name */
#define structure_material_get_lightmap_vertex(material, vertex_index) ((struct environment_lightmap_vertex_compressed const *)structure_material_get_vertex((material), (material)->vertices.count) + (vertex_index)) /* fake name */

/* ---------- structures */

struct point_light
{
	short identifier;
	word flags;
	long definition_index;
	long rasterizer_light_index;
	long magic_number;
	long first_cluster_reference_index;
	real_rgb_color color;
	real_rgb_color lens_flare_color;
	long object_index;
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	real radius;
	long creation_time;
	union
	{
		struct
		{
			short object_attachment_index;
			short object_function_index;
			short object_change_color_index;
		};
		struct
		{
			short node_index;
			word pad;
			real_point3d node_relative_position;
			real_vector3d node_relative_forward;
		};
	};
	real scale;
};

struct lights_game_globals /* fake name */
{
	boolean render_lights;
	byte pad[3];
};

struct lights_globals /* fake name */
{
	boolean marker_initialized;
	long marker;
	short scene_point_light_count;
	long scene_point_lights[MAXIMUM_RENDERED_LIGHTS];
	struct rasterizer_lens_flare_submit_parameters queued_lens_flares[MAXIMUM_QUEUED_LENS_FLARES];
	short queued_lens_flare_count;
};

/* ---------- prototypes */

// belong to profile.h, bitmaps.h and bitmap_group.h, but adding them to the precompiled headers moves symbol bands in other units
void profile_texture_start(void);
void profile_texture_end(void);
pixel32 bitmap_2d_get_pixel(struct bitmap_data const *bitmap, real_point2d const *point, real lod);
struct bitmap_data *bitmap_group_try_and_get_bitmap(long bitmap_group_index, short bitmap_index);

static real shade_scalar(real v0, real v1, real v2, real s, real t);
static void shade_vector2d(real_vector2d const *v0, real_vector2d const *v1, real_vector2d const *v2, real s, real t, real_vector2d *result);
static void shade_vector3d(real_vector3d const *v0, real_vector3d const *v1, real_vector3d const *v2, real s, real t, real_vector3d *result);
static boolean lights_render_enabled(void); /* fake name */
static short light_build_cluster_array(long light_index, short maximum_count, short *cluster_indices);
static void find_point_lights_for_object_in_cluster(long object_index, short cluster_index, real_point3d const *point, real radius, long *light_indices, real *light_brightness, real *light_attenuations, short *light_count, short maximum_count);
static real light_attenuation(real radius, real distance);
static void build_distant_lights(long flags, real_vector3d const *surface_normal, real_rgb_color const *diffuse_color, real_vector3d const *radiosity_normal, real radiosity_accuracy, real_rgb_color const *lightmap_color, struct render_lighting *lighting);
static void brighten_real_rgb_color(real_rgb_color *color, real fraction);
static void light_compute_bounding_sphere(long light_index, boolean maximum, boolean specular, boolean lens_flare_only, real_point3d *bounding_sphere_center, real *bounding_sphere_radius);
static long cluster_get_first_light(long *state, short cluster_index);
static long cluster_get_next_light(long *state);
static void light_get_bounding_sphere(long light_index, real_point3d *position, real *radius);
static void light_marker_begin(void);
static boolean light_unmarked(long light_index);
static boolean light_mark(long light_index);
static void light_marker_end(void);
static void render_debug_light(long light_index);

/* ---------- globals */

struct render_lighting const default_object_lighting =
{
	{ 0.2f, 0.2f, 0.2f },
	2,
	0,
	{
		{ { 1.f, 1.f, 1.f }, { -0.577f, -0.577f, -0.577f } },
		{ { 0.4f, 0.4f, 0.5f }, { 0.f, 0.f, 1.f } },
	},
	0,
	0,
	{ 0, 0 },
	{ 0.5f, 1.f, 1.f, 1.f },
	{ 0.f, 0.f, -1.f },
	{ 0.f, 0.f, 0.f }
};

static real_vector3d const lightmap_sample_raycast_down = { 0.f, 0.f, -10.f };
static real_vector3d const lightmap_sample_raycast_sideways[NUMBER_OF_LIGHTMAP_SAMPLE_RAYCASTS_SIDEWAYS] =
{
	{ -10.f, 0.f, 0.f },
	{ 10.f, 0.f, 0.f },
	{ 0.f, -10.f, 0.f },
	{ 0.f, 10.f, 0.f }
};

static struct profile_section lights_section = {"render_lights", NONE, TRUE};

real object_light_ambient_base = 0.03f;
real object_light_ambient_scale = 0.4f;
real object_light_secondary_scale = 1.f;
boolean object_light_interpolate = TRUE;

struct lights_game_globals *lights_game_globals = NULL;

boolean debug_lights;
boolean debug_object_lights;
short debug_rasterizer_light_count;

struct lights_globals lights_globals;

struct cluster_partition light_cluster_partition;
struct data_array *light_data;

/* ---------- public code */

void *texture_cache_bitmap_load(
	struct bitmap_data const *bitmap)
{
	void *hardware_format;

	profile_texture_start();
	hardware_format = _texture_cache_bitmap_get_hardware_format(bitmap, TRUE, TRUE);
	profile_texture_end();

	return hardware_format;
}

static real shade_scalar(
	real v0,
	real v1,
	real v2,
	real s,
	real t)
{
	return (v1 - v0) * s + (v2 - v0) * t + v0;
}

static void shade_vector2d(
	real_vector2d const *v0,
	real_vector2d const *v1,
	real_vector2d const *v2,
	real s,
	real t,
	real_vector2d *result)
{
	result->i = shade_scalar(v0->i, v1->i, v2->i, s, t);
	result->j = shade_scalar(v0->j, v1->j, v2->j, s, t);

	return;
}

static void shade_vector3d(
	real_vector3d const *v0,
	real_vector3d const *v1,
	real_vector3d const *v2,
	real s,
	real t,
	real_vector3d *result)
{
	result->i = shade_scalar(v0->i, v1->i, v2->i, s, t);
	result->j = shade_scalar(v0->j, v1->j, v2->j, s, t);
	result->k = shade_scalar(v0->k, v1->k, v2->k, s, t);

	return;
}

void sample_lightmap(
	struct structure_material const *material,
	struct bitmap_data const *bitmap,
	struct structure_surface const *surface,
	real s,
	real t,
	real_rgb_color *lightmap_sample)
{
	real_point2d triangle_lightmap_uvs[3];
	real_point2d lightmap_uv;

	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_lights.c",
		143,
		material->lightmap_vertices.type==_rasterizer_vertex_type_environment_lightmap_uncompressed || material->lightmap_vertices.type==_rasterizer_vertex_type_environment_lightmap_compressed);

	environment_lightmap_vertex_compressed_get_texcoord(
		structure_material_get_lightmap_vertex(material, surface->vertex_indices[0]),
		&triangle_lightmap_uvs[0]);
	environment_lightmap_vertex_compressed_get_texcoord(
		structure_material_get_lightmap_vertex(material, surface->vertex_indices[1]),
		&triangle_lightmap_uvs[1]);
	environment_lightmap_vertex_compressed_get_texcoord(
		structure_material_get_lightmap_vertex(material, surface->vertex_indices[2]),
		&triangle_lightmap_uvs[2]);
	shade_vector2d(
		(real_vector2d const *)&triangle_lightmap_uvs[0],
		(real_vector2d const *)&triangle_lightmap_uvs[1],
		(real_vector2d const *)&triangle_lightmap_uvs[2],
		s,
		t,
		(real_vector2d *)&lightmap_uv);
	pixel32_to_real_rgb_color(bitmap_2d_get_pixel(bitmap, &lightmap_uv, 1.f), lightmap_sample);

	return;
}

void sample_diffuse_texture(
	struct structure_material const *material,
	struct bitmap_data const *bitmap,
	struct structure_surface const *surface,
	real s,
	real t,
	real_rgb_color *diffuse_sample)
{
	real_point2d triangle_diffuse_uvs[3];
	real_point2d diffuse_uv;

	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_lights.c",
		167,
		material->vertices.type==_rasterizer_vertex_type_environment_uncompressed || material->vertices.type==_rasterizer_vertex_type_environment_compressed);

	environment_vertex_compressed_get_texcoord(
		structure_material_get_vertex(material, surface->vertex_indices[0]),
		&triangle_diffuse_uvs[0]);
	environment_vertex_compressed_get_texcoord(
		structure_material_get_vertex(material, surface->vertex_indices[1]),
		&triangle_diffuse_uvs[1]);
	environment_vertex_compressed_get_texcoord(
		structure_material_get_vertex(material, surface->vertex_indices[2]),
		&triangle_diffuse_uvs[2]);
	shade_vector2d(
		(real_vector2d const *)&triangle_diffuse_uvs[0],
		(real_vector2d const *)&triangle_diffuse_uvs[1],
		(real_vector2d const *)&triangle_diffuse_uvs[2],
		s,
		t,
		(real_vector2d *)&diffuse_uv);
	pixel32_to_real_rgb_color(bitmap_2d_get_pixel(bitmap, &diffuse_uv, 0.3f), diffuse_sample);

	return;
}

static boolean lights_render_enabled(
	void)
{
	if (lights_game_globals->render_lights && game_engine_allow_dynamic_lighting())
	{
		return TRUE;
	}

	return FALSE;
}

void lights_initialize(
	void)
{
	light_data = game_state_data_new("lights", MAXIMUM_LIGHTS_PER_MAP, sizeof(struct point_light));
	lights_game_globals = game_state_malloc("lights globals", NULL, sizeof(*lights_game_globals));
	match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 194, light_data);
	match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 195, lights_game_globals);
	lights_game_globals->render_lights = TRUE;

	if (light_data)
	{
		cluster_partition_new(&light_cluster_partition, "light");
	}
	else
	{
		error(_error_silent, "couldn't allocate memory for object lights.");
	}

	return;
}

void lights_dispose(
	void)
{
	cluster_partition_delete(&light_cluster_partition);

	return;
}

void lights_initialize_for_new_map(
	void)
{
	data_make_valid(light_data);
	lights_game_globals->render_lights = TRUE;
	cluster_partition_make_valid(&light_cluster_partition);

	return;
}

void lights_dispose_from_old_map(
	void)
{
	data_make_invalid(light_data);
	cluster_partition_make_invalid(&light_cluster_partition);

	return;
}

boolean lights_enable(
	boolean enable)
{
	lights_game_globals->render_lights = enable;

	return enable;
}

long light_new(
	long definition_index,
	long object_index,
	short object_attachment_index,
	short object_function_index,
	short object_change_color_index)
{
	struct point_light_definition *definition = point_light_definition_get(definition_index);
	long light_index = NONE;

	if (TEST_FLAG(definition->flags, _light_dynamic_bit) || definition->lens_flare.reference.index != NONE)
	{
		light_index = datum_new(light_data);

		if (light_index != NONE)
		{
			struct point_light *light = light_get(light_index);

			light->definition_index = definition_index;
			light->object_index = object_index;
			light->object_attachment_index = object_attachment_index;
			light->object_function_index = object_function_index;
			light->object_change_color_index = object_change_color_index;
			light->flags = 0;
			SET_FLAG(light->flags, _point_light_dynamic_bit, TEST_FLAG(definition->flags, _light_dynamic_bit));
			SET_FLAG(
				light->flags,
				_point_light_connects_to_map_bit,
				TEST_FLAG(light->flags, _point_light_dynamic_bit) || definition->lens_flare.reference.index != NONE);
			light->first_cluster_reference_index = NONE;
			light->creation_time = NONE;
			light_reconnect_to_map(light_index);
			light->magic_number = lights_globals.marker - 1;
		}
	}

	return light_index;
}

void light_delete(
	long light_index)
{
	struct point_light *light = light_get(light_index);

	cluster_partition_disconnect(&light_cluster_partition, light_index, &light->first_cluster_reference_index);
	datum_delete(light_data, light_index);

	return;
}

long light_new_unattached(
	long definition_index,
	long object_index,
	short node_index,
	real_point3d const *position,
	real_vector3d const *forward,
	real scale)
{
	long light_index = datum_new(light_data);

	if (light_index != NONE)
	{
		struct point_light *light = light_get(light_index);
		struct point_light_definition *definition = point_light_definition_get(definition_index);

		light->flags = 0;
		light->creation_time = game_time_get();
		light->definition_index = definition_index;
		light->object_index = object_index;
		light->scale = scale;
		SET_FLAG(light->flags, _point_light_dynamic_bit, TRUE);
		SET_FLAG(light->flags, _point_light_connects_to_map_bit, TRUE);
		light->first_cluster_reference_index = NONE;

		if (object_index == NONE)
		{
			light->position = *position;
			light->forward = *forward;
		}
		else
		{
			light->node_index = node_index;
			light->node_relative_position = *position;
			light->node_relative_forward = *forward;
		}

		light_reconnect_to_map(light_index);
		light->magic_number = lights_globals.marker - 1;
	}

	return light_index;
}

void lights_preprocess_scene(
	void)
{
	long light_index;
	short rendered_cluster_index;
	short scene_light_index;
	short lens_flare_index;
	long game_time = game_time_get();

	profile_enter(lights_section);
	debug_rasterizer_light_count = 0;

	for (light_index = data_next_index(light_data, NONE);
		light_index != NONE;
		light_index = data_next_index(light_data, light_index))
	{
		struct point_light *light = light_get(light_index);

		SET_FLAG(light->flags, _point_light_attached_to_first_person_weapon_bit, FALSE);
		light->rasterizer_light_index = NONE;

		if (light->creation_time != NONE)
		{
			struct point_light_definition *definition = point_light_definition_get(light->definition_index);

			if ((real)(game_time - light->creation_time) > definition->effect.duration)
			{
				light_delete(light_index);
			}
			else if (object_try_and_get(light->object_index))
			{
				light_disconnect_from_map(light_index);
				light_reconnect_to_map(light_index);
			}
		}
	}

	light_marker_begin();
	lights_globals.scene_point_light_count = structure_visibility_find_objects(
		lights_globals.scene_point_lights,
		MAXIMUM_RENDERED_LIGHTS,
		cluster_get_first_light,
		cluster_get_next_light,
		light_get_bounding_sphere,
		light_unmarked,
		light_mark);
	light_marker_end();

	rasterizer_lights_begin();

	for (rendered_cluster_index = 0;
		rendered_cluster_index < render.rendered_cluster_count;
		rendered_cluster_index++)
	{
		rasterizer_lens_flare_submit_for_cluster(rendered_cluster_get(rendered_cluster_index)->cluster_index);
	}

	for (scene_light_index = 0;
		scene_light_index < lights_globals.scene_point_light_count;
		scene_light_index++)
	{
		struct object_datum *object;
		real intensity;
		real inverse_intensity;
		long scene_light = lights_globals.scene_point_lights[scene_light_index];
		struct point_light *light = light_get(scene_light);
		struct point_light_definition *definition = point_light_definition_get(light->definition_index);
		real scale = 1.f;

		render_debug_light(scene_light);
		object = light->object_index != NONE ? object_try_and_get(light->object_index) : NULL;

		if (light->creation_time == NONE)
		{
			match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 428, object);
			object_get_function_value(light->object_index, light->object_function_index, &intensity);
			rgb_colors_interpolate_and_scale(
				&light->color,
				definition->color.interpolation_flags,
				&definition->color.lower_bound,
				&definition->color.upper_bound,
				light->object_change_color_index == NONE ?
					global_real_rgb_white :
					&object->object.outgoing_change_colors[light->object_change_color_index],
				intensity);
		}
		else
		{
			intensity = (1.f - transition_function_evaluate(
				definition->effect.falloff_function,
				(real)(game_time - light->creation_time) / definition->effect.duration)) * light->scale;
			rgb_colors_interpolate(
				&light->color,
				definition->color.interpolation_flags,
				&definition->color.lower_bound.rgb,
				&definition->color.upper_bound.rgb,
				intensity);
		}

		inverse_intensity = 1.f - intensity;
		match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 443, light->color.red >=0.0f && light->color.red <=1.0f);
		match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 444, light->color.green>=0.0f && light->color.green<=1.0f);
		match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 445, light->color.blue >=0.0f && light->color.blue <=1.0f);

		if (object)
		{
			long ultimate_parent_index = object_get_ultimate_parent(light->object_index);

			if (TEST_FLAG(_object_mask_unit, object_get(ultimate_parent_index)->object.type))
			{
				struct unit_datum *unit = unit_get(ultimate_parent_index);

				if (unit->unit.active_camouflage > 0.f &&
					!TEST_FLAG(point_light_definition_get(light->definition_index)->flags, _light_dont_fade_active_camouflage_bit))
				{
					scale = 1.f - unit->unit.active_camouflage;
					light->color.red *= scale;
					light->color.green *= scale;
					light->color.blue *= scale;
					match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 465, light->color.red >=0.0f && light->color.red <=1.0f);
					match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 466, light->color.green>=0.0f && light->color.green<=1.0f);
					match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 467, light->color.blue >=0.0f && light->color.blue <=1.0f);
				}
			}
		}

		if (light->color.red != 0.f || light->color.green != 0.f || light->color.blue != 0.f)
		{
			if (TEST_FLAG(light->flags, _point_light_dynamic_bit))
			{
				light->radius =
					(inverse_intensity * definition->geometry.radius_modifier_lower_bound + intensity * definition->geometry.radius_modifier_upper_bound) *
					definition->geometry.radius;

				if (light->radius != 0.f)
				{
					struct rasterizer_light_submit_parameters light_parameters;

					light_parameters.definition = point_light_definition_get(light->definition_index);
					light_parameters.position = light->position;
					light_parameters.forward = light->forward;
					light_parameters.up = light->up;
					light_parameters.radius = light->radius;
					light_parameters.color = light->color;
					match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 494, light->color.red >=0.0f && light->color.red <=1.0f);
					match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 495, light->color.green>=0.0f && light->color.green<=1.0f);
					match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 496, light->color.blue >=0.0f && light->color.blue <=1.0f);

					if (light->creation_time == NONE)
					{
						if (TEST_FLAG(definition->flags, _light_is_first_person_flashlight_bit))
						{
							first_person_weapon_center_flashlight(
								light->object_index,
								&light_parameters.position,
								&light_parameters.forward,
								&light_parameters.up);
							SET_FLAG(light->flags, _point_light_attached_to_first_person_weapon_bit, TRUE);
						}
						else if (object->object.type == _object_type_weapon &&
							object->object.parent_object_index != NONE &&
							first_person_weapon_adjust_light(
								light->object_index,
								object_get_attachment_marker_name(light->object_index, light->object_attachment_index),
								&light_parameters.position,
								&light_parameters.forward,
								&light_parameters.up))
						{
							SET_FLAG(light->flags, _point_light_attached_to_first_person_weapon_bit, TRUE);
						}
					}

					light->rasterizer_light_index = rasterizer_light_submit(&light_parameters);
					debug_rasterizer_light_count = (short)(light->rasterizer_light_index + 1);
				}
			}
			else
			{
				light->radius = definition->geometry.radius;
			}

			if (definition->lens_flare.reference.index != NONE)
			{
				struct rasterizer_lens_flare_submit_parameters lens_flare_parameters;

				lens_flare_parameters.definition = lens_flare_definition_get(definition->lens_flare.reference.index);
				lens_flare_parameters.compressed_light_color = real_a_rgb_color_to_pixel32(scale, &light->color);
				lens_flare_parameters.compressed_light_scale = compress_real_to_int8(intensity);
				lens_flare_parameters.compressed_window_index = (byte)render.window_index;
				lens_flare_parameters.light_index = (short)DATUM_INDEX_TO_ABSOLUTE_INDEX(scene_light);
				lens_flare_parameters.light_identifier = (short)DATUM_INDEX_TO_IDENTIFIER(scene_light);
				match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 540, lens_flare_parameters.light_identifier!=0);

				if (lens_flare_parameters.light_identifier == NONE)
				{
					lens_flare_parameters.light_identifier = 0;
				}

				if (light->creation_time == NONE)
				{
					struct object_marker markers[MAXIMUM_LENS_FLARES_PER_LIGHT];
					char const *marker_name = object_get_attachment_marker_name(
						light->object_index,
						light->object_attachment_index);
					short marker_count = 0;
					short marker_index;

					if (object->object.type == _object_type_weapon && object->object.parent_object_index != NONE)
					{
						marker_count = first_person_weapon_get_marker_by_name_render(
							light->object_index,
							marker_name,
							markers,
							MAXIMUM_LENS_FLARES_PER_LIGHT);

						if (marker_count > 0)
						{
							SET_FLAG(
								lens_flare_parameters.compressed_window_index,
								_lens_flare_window_index_first_person_bit,
								TRUE);
						}
					}

					if (!marker_count)
					{
						marker_count = object_get_marker_by_name(
							light->object_index,
							marker_name,
							markers,
							MAXIMUM_LENS_FLARES_PER_LIGHT);
					}

					for (marker_index = 0; marker_index < marker_count; marker_index++)
					{
						struct object_marker *marker = &markers[marker_index];

						lens_flare_parameters.position = marker->matrix.position;
						lens_flare_parameters.compressed_direction = compress_real_vector3d_to_int32_clamp(&marker->matrix.forward);
						lens_flare_parameters.compressed_up = compress_real_vector3d_to_int32_clamp(&marker->matrix.up);
						lens_flare_parameters.lens_flare_index = marker_index;
						rasterizer_lens_flare_submit(&lens_flare_parameters);
					}
				}
				else
				{
					lens_flare_parameters.position = light->position;
					lens_flare_parameters.compressed_direction = compress_real_vector3d_to_int32_clamp(&light->forward);
					lens_flare_parameters.compressed_up = compress_real_vector3d_to_int32_clamp(&light->up);
					lens_flare_parameters.lens_flare_index = 0;
					rasterizer_lens_flare_submit(&lens_flare_parameters);
				}
			}
		}
	}

	for (lens_flare_index = 0; lens_flare_index < lights_globals.queued_lens_flare_count; lens_flare_index++)
	{
		rasterizer_lens_flare_submit(&lights_globals.queued_lens_flares[lens_flare_index]);
	}

	lights_globals.queued_lens_flare_count = 0;
	rasterizer_lights_end();
	profile_exit(lights_section);

	return;
}

static short light_build_cluster_array(
	long light_index,
	short maximum_count,
	short *cluster_indices)
{
	long iterator;
	short cluster_index;
	struct point_light *light = light_get(light_index);
	short cluster_count = 0;

	for (cluster_index = (short)cluster_partition_get_first_cluster(
			&light_cluster_partition,
			&iterator,
			light->first_cluster_reference_index);
		cluster_count < maximum_count && cluster_index != NONE;
		cluster_index = (short)cluster_partition_get_next_cluster(&light_cluster_partition, &iterator))
	{
		cluster_indices[cluster_count++] = cluster_index;
	}

	return cluster_count;
}

void lights_render_diffuse(
	void)
{
	short cluster_indices[MAXIMUM_CLUSTERS_PER_LIGHT];

	rasterizer_environment_diffuse_lights_begin();

	if (lights_render_enabled())
	{
		short scene_light_index;

		for (scene_light_index = 0;
			scene_light_index < lights_globals.scene_point_light_count;
			scene_light_index++)
		{
			struct point_light *light = light_get(lights_globals.scene_point_lights[scene_light_index]);

			if (TEST_FLAG(light->flags, _point_light_dynamic_bit) && light->rasterizer_light_index != NONE)
			{
				boolean supersize =
					TEST_FLAG(light->flags, _point_light_attached_to_first_person_weapon_bit) &&
					TEST_FLAG(point_light_definition_get(light->definition_index)->flags, _light_supersize_in_first_person_bit);
				short cluster_count = 0;
				real_point3d point;
				real radius;

				if (!supersize)
				{
					cluster_count = light_build_cluster_array(
						lights_globals.scene_point_lights[scene_light_index],
						MAXIMUM_CLUSTERS_PER_LIGHT,
						cluster_indices);
				}

				light_compute_bounding_sphere(
					lights_globals.scene_point_lights[scene_light_index],
					FALSE,
					FALSE,
					FALSE,
					&point,
					&radius);
				structure_render_diffuse_light(
					light->rasterizer_light_index,
					&point,
					radius,
					cluster_count,
					supersize ? NULL : cluster_indices);
			}
		}
	}

	rasterizer_environment_diffuse_lights_end();

	return;
}

void lights_render_specular(
	void)
{
	short cluster_indices[MAXIMUM_CLUSTERS_PER_LIGHT];

	rasterizer_environment_specular_lights_begin();

	if (lights_render_enabled())
	{
		short scene_light_index;

		for (scene_light_index = 0;
			scene_light_index < lights_globals.scene_point_light_count;
			scene_light_index++)
		{
			struct point_light *light = light_get(lights_globals.scene_point_lights[scene_light_index]);

			if (TEST_FLAG(light->flags, _point_light_dynamic_bit) &&
				light->rasterizer_light_index != NONE &&
				!TEST_FLAG(point_light_definition_get(light->definition_index)->flags, _light_no_specular_bit))
			{
				boolean supersize =
					TEST_FLAG(light->flags, _point_light_attached_to_first_person_weapon_bit) &&
					TEST_FLAG(point_light_definition_get(light->definition_index)->flags, _light_supersize_in_first_person_bit);
				short cluster_count = 0;
				real_point3d point;
				real radius;

				if (!supersize)
				{
					cluster_count = light_build_cluster_array(
						lights_globals.scene_point_lights[scene_light_index],
						MAXIMUM_CLUSTERS_PER_LIGHT,
						cluster_indices);
				}

				light_compute_bounding_sphere(
					lights_globals.scene_point_lights[scene_light_index],
					FALSE,
					TRUE,
					FALSE,
					&point,
					&radius);
				structure_render_specular_light(
					light->rasterizer_light_index,
					&point,
					radius,
					cluster_count,
					supersize ? NULL : cluster_indices);
			}
		}
	}

	rasterizer_environment_specular_lights_end();

	return;
}

real object_get_self_illumination(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *definition = object_definition_get(object->definition_index);
	real illumination = 0.f;
	short attachment_index = 0;

	if (definition->object.attachments.count > 0)
	{
		do
		{
			if (object->object.attachment_types[attachment_index] == _object_attachment_type_light &&
				object->object.attachment_indices[attachment_index] != NONE)
			{
				struct point_light *light = light_get(object->object.attachment_indices[attachment_index]);

				illumination += real_rgb_color_brightness(&light->color);
			}

			attachment_index++;
		}
		while (attachment_index < definition->object.attachments.count);
	}

	if (object->object.first_child_object_index != NONE)
	{
		illumination += object_get_self_illumination(object->object.first_child_object_index);
	}

	if (object->object.next_object_index != NONE)
	{
		illumination += object_get_self_illumination(object->object.next_object_index);
	}

	return illumination;
}

void lights_illumination_at_point(
	real_point3d const *point,
	struct location const *location,
	real_rgb_color *color)
{
	*color = *global_real_rgb_black;

	{
		real_point3d sample_point;
		short lightmap_index;
		short material_index;
		long surface_index;
		real s;
		real t;

		if (structure_test_vector(
			point,
			&lightmap_sample_raycast_down,
			&sample_point,
			&lightmap_index,
			&material_index,
			&surface_index,
			&s,
			&t))
		{
			struct structure_bsp *structure_bsp = global_structure_bsp_get();
			struct structure_lightmap *lightmap = TAG_BLOCK_GET_ELEMENT(
				&structure_bsp->lightmaps,
				lightmap_index,
				struct structure_lightmap);
			struct structure_material *material = TAG_BLOCK_GET_ELEMENT(
				&lightmap->materials,
				material_index,
				struct structure_material);

			if (structure_bsp->lightmap_group.index != NONE && lightmap->bitmap_index != NONE)
			{
				struct bitmap_data *bitmap = bitmap_group_try_and_get_bitmap(
					structure_bsp->lightmap_group.index,
					lightmap->bitmap_index);
				struct structure_surface *surface = TAG_BLOCK_GET_ELEMENT(
					&structure_bsp->surfaces,
					surface_index,
					struct structure_surface);

				match_assert(
					"c:\\halo\\SOURCE\\objects\\object_lights.c",
					854,
					material->lightmap_vertices.type==_rasterizer_vertex_type_environment_lightmap_uncompressed || material->lightmap_vertices.type==_rasterizer_vertex_type_environment_lightmap_compressed);

				if (_texture_cache_bitmap_get_hardware_format(bitmap, FALSE, FALSE))
				{
					sample_lightmap(material, bitmap, surface, s, t, color);
				}
			}
		}
	}

	if (location->cluster_index != NONE)
	{
		long light_indices[MAXIMUM_RENDERED_POINT_LIGHTS];
		real light_brightness[MAXIMUM_RENDERED_POINT_LIGHTS];
		real light_attenuations[MAXIMUM_RENDERED_POINT_LIGHTS];
		short light_index;
		short light_count = 0;

		light_marker_begin();
		find_point_lights_for_object_in_cluster(
			NONE,
			location->cluster_index,
			point,
			0.f,
			light_indices,
			light_brightness,
			light_attenuations,
			&light_count,
			MAXIMUM_RENDERED_POINT_LIGHTS);
		light_marker_end();

		for (light_index = 0; light_index < light_count; light_index++)
		{
			struct point_light *light = light_get(light_indices[light_index]);

			if (TEST_FLAG(light->flags, _point_light_dynamic_bit))
			{
				color->red += light->color.red * light_attenuations[light_index];
				color->green += light->color.green * light_attenuations[light_index];
				color->blue += light->color.blue * light_attenuations[light_index];
			}
		}
	}

	color->red = PIN(color->red, 0.f, 1.f);
	color->green = PIN(color->green, 0.f, 1.f);
	color->blue = PIN(color->blue, 0.f, 1.f);

	return;
}

void light_particle(
	real_point3d const *point,
	real_rgb_color *light_color,
	real_rgb_color *diffuse_color,
	boolean block)
{
	real_point3d sample_point;
	short lightmap_index;
	short material_index;
	long surface_index;
	real s;
	real t;

	*light_color = *global_real_rgb_grey;
	*diffuse_color = *global_real_rgb_grey;

	if (structure_test_vector(
		point,
		&lightmap_sample_raycast_down,
		&sample_point,
		&lightmap_index,
		&material_index,
		&surface_index,
		&s,
		&t))
	{
		struct structure_bsp *structure_bsp = global_structure_bsp_get();
		struct structure_lightmap *lightmap = TAG_BLOCK_GET_ELEMENT(
			&structure_bsp->lightmaps,
			lightmap_index,
			struct structure_lightmap);
		struct structure_material *material = TAG_BLOCK_GET_ELEMENT(
			&lightmap->materials,
			material_index,
			struct structure_material);
		struct shader *shader = shader_definition_get(material->shader.index);

		if (shader->base.type == _shader_type_environment)
		{
			struct shader_environment *shader_environment = shader_get_and_verify_type(shader, _shader_type_environment);

			if (structure_bsp->lightmap_group.index != NONE &&
				shader_environment->environment.diffuse.base_map.index != NONE &&
				lightmap->bitmap_index != NONE)
			{
				struct tag_reference *base_map = &shader_environment->environment.diffuse.base_map;
				struct bitmap_data *lightmap_bitmap = bitmap_group_try_and_get_bitmap(
					structure_bsp->lightmap_group.index,
					lightmap->bitmap_index);
				struct bitmap_data *diffuse_bitmap = bitmap_group_try_and_get_bitmap(
					base_map->index,
					(short)(material->permutation_index % bitmap_group_get(base_map->index)->bitmaps.count));
				struct structure_surface *surface = NULL;

				if (lightmap_bitmap &&
					((block && texture_cache_bitmap_load(lightmap_bitmap)) ||
					_texture_cache_bitmap_get_hardware_format(lightmap_bitmap, FALSE, FALSE)))
				{
					surface = TAG_BLOCK_GET_ELEMENT(&structure_bsp->surfaces, surface_index, struct structure_surface);
					sample_lightmap(material, lightmap_bitmap, surface, s, t, light_color);
					light_color->red = MIN(light_color->red + 0.1f, 1.f);
					light_color->green = MIN(light_color->green + 0.1f, 1.f);
					light_color->blue = MIN(light_color->blue + 0.1f, 1.f);
				}

				if (diffuse_bitmap &&
					((block && texture_cache_bitmap_load(diffuse_bitmap)) ||
					_texture_cache_bitmap_get_hardware_format(diffuse_bitmap, FALSE, FALSE)))
				{
					if (!surface)
					{
						surface = TAG_BLOCK_GET_ELEMENT(&structure_bsp->surfaces, surface_index, struct structure_surface);
					}

					sample_diffuse_texture(material, diffuse_bitmap, surface, s, t, diffuse_color);
				}
			}
		}
	}

	return;
}

void lights_prepare_for_object_static(
	long object_index,
	struct render_lighting *lighting)
{
	boolean success;
	struct object_datum *object = object_get(object_index);
	long flags = 0;

	match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 970, lighting);

	if (TEST_FLAG(object->object.flags, _object_static_lighting_raycast_sideways_bit))
	{
		SET_FLAG(flags, _distant_lighting_raycast_sideways_bit, TRUE);
	}

	if (TEST_FLAG(object_definition_get(object->definition_index)->object.flags, _object_artificially_bright_bit))
	{
		SET_FLAG(flags, _distant_lighting_brighten_bit, TRUE);
	}

	success = lights_distant_lighting_at_point(flags, &object->object.bounding_sphere_center, lighting);

	if (!TEST_FLAG(object->object.flags, _object_static_lighting_recompute_bit))
	{
		struct render_lighting sample;
		short sample_count;
		short sample_index;

		if (success)
		{
			sample_count = 1;
		}
		else
		{
			sample_count = 0;
			memset(lighting, 0, sizeof(*lighting));
			lighting->distant_light_count = MAXIMUM_RENDERED_DISTANT_LIGHTS;
		}

		for (sample_index = 0; sample_index < 4; sample_index++)
		{
			real_point3d sample_point;

			sample_point.x =
				(TEST_FLAG(sample_index, 0) ? SQUARE_ROOT_ONE_HALF : -SQUARE_ROOT_ONE_HALF) * object->object.bounding_sphere_radius +
				object->object.bounding_sphere_center.x;
			sample_point.y =
				(TEST_FLAG(sample_index, 1) ? SQUARE_ROOT_ONE_HALF : -SQUARE_ROOT_ONE_HALF) * object->object.bounding_sphere_radius +
				object->object.bounding_sphere_center.y;
			sample_point.z = object->object.bounding_sphere_center.z;

			if (lights_distant_lighting_at_point(flags, &sample_point, &sample))
			{
				sample_count++;
				lighting->ambient_color.red += sample.ambient_color.red;
				lighting->ambient_color.green += sample.ambient_color.green;
				lighting->ambient_color.blue += sample.ambient_color.blue;
				lighting->reflection_tint_color.alpha += sample.reflection_tint_color.alpha;
				lighting->reflection_tint_color.red += sample.reflection_tint_color.red;
				lighting->reflection_tint_color.green += sample.reflection_tint_color.green;
				lighting->reflection_tint_color.blue += sample.reflection_tint_color.blue;
				lighting->distant_lights[0].color.red += sample.distant_lights[0].color.red;
				lighting->distant_lights[0].color.green += sample.distant_lights[0].color.green;
				lighting->distant_lights[0].color.blue += sample.distant_lights[0].color.blue;
				lighting->distant_lights[0].direction.i += sample.distant_lights[0].direction.i;
				lighting->distant_lights[0].direction.j += sample.distant_lights[0].direction.j;
				lighting->distant_lights[0].direction.k += sample.distant_lights[0].direction.k;
				lighting->distant_lights[1].color.red += sample.distant_lights[1].color.red;
				lighting->distant_lights[1].color.green += sample.distant_lights[1].color.green;
				lighting->distant_lights[1].color.blue += sample.distant_lights[1].color.blue;
				lighting->distant_lights[1].direction.i += sample.distant_lights[1].direction.i;
				lighting->distant_lights[1].direction.j += sample.distant_lights[1].direction.j;
				lighting->distant_lights[1].direction.k += sample.distant_lights[1].direction.k;
				lighting->shadow_color.red += sample.shadow_color.red;
				lighting->shadow_color.green += sample.shadow_color.green;
				lighting->shadow_color.blue += sample.shadow_color.blue;
				lighting->shadow_vector.i += sample.shadow_vector.i;
				lighting->shadow_vector.j += sample.shadow_vector.j;
				lighting->shadow_vector.k += sample.shadow_vector.k;
			}
		}

		if (sample_count > 1)
		{
			real scale = 1.f / sample_count;

			lighting->ambient_color.red *= scale;
			lighting->ambient_color.green *= scale;
			lighting->ambient_color.blue *= scale;
			lighting->reflection_tint_color.alpha *= scale;
			lighting->reflection_tint_color.red *= scale;
			lighting->reflection_tint_color.green *= scale;
			lighting->reflection_tint_color.blue *= scale;
			lighting->distant_lights[0].color.red *= scale;
			lighting->distant_lights[0].color.green *= scale;
			lighting->distant_lights[0].color.blue *= scale;
			scale_vector3d(&lighting->distant_lights[0].direction, scale, &lighting->distant_lights[0].direction);
			normalize3d(&lighting->distant_lights[0].direction);
			lighting->distant_lights[1].color.red *= scale;
			lighting->distant_lights[1].color.green *= scale;
			lighting->distant_lights[1].color.blue *= scale;
			scale_vector3d(&lighting->distant_lights[1].direction, scale, &lighting->distant_lights[1].direction);
			normalize3d(&lighting->distant_lights[1].direction);
			lighting->shadow_color.red *= scale;
			lighting->shadow_color.green *= scale;
			lighting->shadow_color.blue *= scale;
			scale_vector3d(&lighting->shadow_vector, scale, &lighting->shadow_vector);
			normalize3d(&lighting->shadow_vector);
		}
		else if (sample_count == 0)
		{
			*lighting = sample;
		}
	}

	return;
}

void lights_prepare_for_object_dynamic(
	long object_index,
	struct render_lighting *lighting)
{
	struct object_cluster_iterator iterator;
	real_point3d object_center;
	real object_radius;
	real light_brightness[MAXIMUM_RENDERED_POINT_LIGHTS];
	real light_attenuations[MAXIMUM_RENDERED_POINT_LIGHTS];
	short cluster_index;
	short light_index;

	object_get_bounding_sphere(object_index, &object_center, &object_radius);
	lighting->point_light_count = 0;
	light_marker_begin();

	for (cluster_index = object_get_first_cluster(&iterator, object_index);
		cluster_index != NONE;
		cluster_index = object_get_next_cluster(&iterator, object_index))
	{
		find_point_lights_for_object_in_cluster(
			object_index,
			cluster_index,
			&object_center,
			object_radius,
			lighting->point_light_indices,
			light_brightness,
			light_attenuations,
			&lighting->point_light_count,
			MAXIMUM_RENDERED_POINT_LIGHTS);
	}

	light_marker_end();

	for (light_index = 0; light_index < lighting->point_light_count; light_index++)
	{
		struct point_light *light = light_get(lighting->point_light_indices[light_index]);

		lighting->point_light_indices[light_index] = light->rasterizer_light_index;
	}

	return;
}

boolean lights_distant_lighting_at_point(
	long flags,
	real_point3d const *position,
	struct render_lighting *lighting)
{
	real_vector3d const *raycasts;
	short raycast_count;
	short raycast_index;
	boolean success = FALSE;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	if (structure_bsp->default_lighting.ambient_color.red != 0.f)
	{
		*lighting = structure_bsp->default_lighting;
		lighting->distant_light_count = MAXIMUM_RENDERED_DISTANT_LIGHTS;
	}
	else
	{
		*lighting = default_object_lighting;
	}

	if (TEST_FLAG(flags, _distant_lighting_raycast_sideways_bit))
	{
		raycasts = lightmap_sample_raycast_sideways;
		raycast_count = NUMBER_OF_LIGHTMAP_SAMPLE_RAYCASTS_SIDEWAYS;
	}
	else
	{
		raycasts = &lightmap_sample_raycast_down;
		raycast_count = 1;
	}

	for (raycast_index = 0; raycast_index < raycast_count; raycast_index++)
	{
		real_point3d sample_point;
		short lightmap_index;
		short material_index;
		long surface_index;
		real s;
		real t;

		if (structure_test_vector(
			position,
			&raycasts[raycast_index],
			&sample_point,
			&lightmap_index,
			&material_index,
			&surface_index,
			&s,
			&t))
		{
			struct structure_lightmap *lightmap;
			struct structure_material *material;
			struct shader *shader;

			structure_bsp = global_structure_bsp_get();
			lightmap = TAG_BLOCK_GET_ELEMENT(&structure_bsp->lightmaps, lightmap_index, struct structure_lightmap);
			material = TAG_BLOCK_GET_ELEMENT(&lightmap->materials, material_index, struct structure_material);
			shader = shader_definition_get(material->shader.index);

			if (shader->base.type == _shader_type_environment)
			{
				struct shader_environment *shader_environment = shader_get_and_verify_type(shader, _shader_type_environment);

				if (structure_bsp->lightmap_group.index != NONE &&
					lightmap->bitmap_index != NONE &&
					shader_environment->environment.diffuse.base_map.index != NONE)
				{
					struct tag_reference *base_map = &shader_environment->environment.diffuse.base_map;
					struct structure_surface *surface = TAG_BLOCK_GET_ELEMENT(
						&structure_bsp->surfaces,
						surface_index,
						struct structure_surface);
					struct bitmap_data *lightmap_bitmap = bitmap_group_try_and_get_bitmap(
						structure_bsp->lightmap_group.index,
						lightmap->bitmap_index);
					struct bitmap_data *diffuse_bitmap = bitmap_group_try_and_get_bitmap(
						base_map->index,
						(short)(material->permutation_index % bitmap_group_get(base_map->index)->bitmaps.count));

					if (lightmap_bitmap &&
						diffuse_bitmap &&
						texture_cache_bitmap_load(lightmap_bitmap) &&
						texture_cache_bitmap_load(diffuse_bitmap))
					{
						real_rgb_color diffuse_color;
						real_rgb_color lightmap_color;
						real_vector3d normal;
						real_vector3d radiosity_normal;
						real radiosity_accuracy;

						sample_diffuse_texture(material, diffuse_bitmap, surface, s, t, &diffuse_color);
						sample_lightmap(material, lightmap_bitmap, surface, s, t, &lightmap_color);

						{
							real_vector3d triangle_normals[3];

							environment_vertex_compressed_get_normal(
								structure_material_get_vertex(material, surface->vertex_indices[0]),
								&triangle_normals[0]);
							environment_vertex_compressed_get_normal(
								structure_material_get_vertex(material, surface->vertex_indices[1]),
								&triangle_normals[1]);
							environment_vertex_compressed_get_normal(
								structure_material_get_vertex(material, surface->vertex_indices[2]),
								&triangle_normals[2]);
							shade_vector3d(&triangle_normals[0], &triangle_normals[1], &triangle_normals[2], s, t, &normal);
							normalize3d(&normal);
						}

						{
							real_vector3d triangle_radiosity_normals[3];
							real triangle_accuracies[3];

							environment_lightmap_vertex_compressed_get_incident_radiosity(
								structure_material_get_lightmap_vertex(material, surface->vertex_indices[0]),
								&triangle_radiosity_normals[0]);
							environment_lightmap_vertex_compressed_get_incident_radiosity(
								structure_material_get_lightmap_vertex(material, surface->vertex_indices[1]),
								&triangle_radiosity_normals[1]);
							environment_lightmap_vertex_compressed_get_incident_radiosity(
								structure_material_get_lightmap_vertex(material, surface->vertex_indices[2]),
								&triangle_radiosity_normals[2]);
							triangle_accuracies[0] = normalize3d(&triangle_radiosity_normals[0]);
							triangle_accuracies[1] = normalize3d(&triangle_radiosity_normals[1]);
							triangle_accuracies[2] = normalize3d(&triangle_radiosity_normals[2]);
							shade_vector3d(
								&triangle_radiosity_normals[0],
								&triangle_radiosity_normals[1],
								&triangle_radiosity_normals[2],
								s,
								t,
								&radiosity_normal);
							radiosity_accuracy = shade_scalar(
								triangle_accuracies[0],
								triangle_accuracies[1],
								triangle_accuracies[2],
								s,
								t);
							normalize3d(&radiosity_normal);
						}

						if (debug_object_lights)
						{
							real_argb_color lightmap_color_argb;

							lightmap_color_argb.rgb = lightmap_color;
							lightmap_color_argb.alpha = 1.f;
							render_debug_point(TRUE, position, 0.5f, &lightmap_color_argb);
							render_debug_vector(TRUE, position, &radiosity_normal, radiosity_accuracy, &lightmap_color_argb);
						}

						build_distant_lights(
							flags,
							&normal,
							&diffuse_color,
							&radiosity_normal,
							radiosity_accuracy,
							&lightmap_color,
							lighting);
						success = TRUE;
					}
				}
			}

			break;
		}
	}

	return success;
}

void light_disconnect_from_map(
	long light_index)
{
	struct point_light *light = light_get(light_index);

	if (TEST_FLAG(light->flags, _point_light_connects_to_map_bit))
	{
		match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 1232, TEST_FLAG(light->flags, _point_light_connected_to_map_bit));
		cluster_partition_disconnect(&light_cluster_partition, light_index, &light->first_cluster_reference_index);
		SET_FLAG(light->flags, _point_light_connected_to_map_bit, FALSE);
	}

	return;
}

void light_reconnect_to_map(
	long light_index)
{
	struct object_marker marker;
	struct location location;
	real_point3d position;
	real radius;
	struct point_light *light = light_get(light_index);
	struct point_light_definition *definition = point_light_definition_get(light->definition_index);

	if (light->creation_time == NONE)
	{
		object_get_marker_by_name(
			light->object_index,
			object_get_attachment_marker_name(light->object_index, light->object_attachment_index),
			&marker,
			1);
		light->position = marker.matrix.position;
		light->forward = marker.matrix.forward;
		light->up = marker.matrix.up;
	}
	else if (object_try_and_get(light->object_index))
	{
		real_matrix4x3 *node_matrix = object_get_node_matrix(light->object_index, light->node_index);

		matrix4x3_transform_point(node_matrix, &light->node_relative_position, &light->position);
		matrix4x3_transform_normal(node_matrix, &light->node_relative_forward, &light->forward);
		perpendicular3d(&light->forward, &light->up);
		normalize3d(&light->up);
	}

	if (TEST_FLAG(light->flags, _point_light_connects_to_map_bit))
	{
		light_compute_bounding_sphere(light_index, TRUE, FALSE, TRUE, &position, &radius);
		match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 1273, !TEST_FLAG(light->flags, _point_light_connected_to_map_bit));

		if (light->object_index != NONE && object_try_and_get(light->object_index))
		{
			object_get_location(light->object_index, &location);
		}
		else
		{
			scenario_location_from_point(&location, &position);
		}

		cluster_partition_reconnect(
			&light_cluster_partition,
			light_index,
			&light->first_cluster_reference_index,
			&position,
			radius,
			&location);
		SET_FLAG(light->flags, _point_light_connected_to_map_bit, TRUE);
	}

	return;
}

void lights_disconnect_from_structure_bsp(
	void)
{
	long light_index;

	for (light_index = data_next_index(light_data, NONE);
		light_index != NONE;
		light_index = data_next_index(light_data, light_index))
	{
		struct point_light *light = light_get(light_index);

		if (TEST_FLAG(light->flags, _point_light_connected_to_map_bit))
		{
			light_disconnect_from_map(light_index);
			SET_FLAG(light->flags, _point_light_connected_to_map_bit, TRUE);
		}
	}

	return;
}

void lights_reconnect_to_structure_bsp(
	void)
{
	long light_index;

	for (light_index = data_next_index(light_data, NONE);
		light_index != NONE;
		light_index = data_next_index(light_data, light_index))
	{
		struct point_light *light = light_get(light_index);

		if (TEST_FLAG(light->flags, _point_light_connected_to_map_bit))
		{
			SET_FLAG(light->flags, _point_light_connected_to_map_bit, FALSE);
			light_reconnect_to_map(light_index);
		}
	}

	return;
}

/* ---------- private code */

static void find_point_lights_for_object_in_cluster(
	long object_index,
	short cluster_index,
	real_point3d const *point,
	real radius,
	long *light_indices,
	real *light_brightness,
	real *light_attenuations,
	short *light_count,
	short maximum_count)
{
	long state;
	long light_index;

	match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 1348, lights_globals.marker_initialized);

	for (light_index = cluster_partition_get_first_datum(&light_cluster_partition, &state, cluster_index);
		light_index != NONE;
		light_index = cluster_partition_get_next_datum(&light_cluster_partition, &state))
	{
		if (light_unmarked(light_index))
		{
			struct point_light *light = light_get(light_index);

			if (light->rasterizer_light_index != NONE &&
				(light->object_index != object_index ||
				!TEST_FLAG(point_light_definition_get(light->definition_index)->flags, _light_no_reflexive_bit)))
			{
				real distance = distance3d(&light->position, point);

				if (distance < radius + light->radius)
				{
					short index;
					real attenuation = light_attenuation(light->radius, distance);
					real brightness = real_rgb_color_brightness(&light->color) * attenuation;

					if (*light_count < maximum_count)
					{
						index = (*light_count)++;
					}
					else
					{
						real minimum_brightness = REAL_MAX;
						short minimum_index = NONE;

						for (index = 0; index < *light_count; index++)
						{
							if (minimum_brightness > light_brightness[index])
							{
								minimum_brightness = light_brightness[index];
								minimum_index = index;
							}
						}

						if (minimum_brightness < brightness)
						{
							index = minimum_index;
						}
					}

					if (index < maximum_count)
					{
						light_indices[index] = light_index;
						light_brightness[index] = brightness;
						light_attenuations[index] = attenuation;
					}
				}
			}

			light_mark(light_index);
		}
	}

	return;
}

static real light_attenuation(
	real radius,
	real distance)
{
	return 1.f - (distance * distance) / (radius * radius);
}

static void build_distant_lights(
	long flags,
	real_vector3d const *surface_normal,
	real_rgb_color const *diffuse_color,
	real_vector3d const *radiosity_normal,
	real radiosity_accuracy,
	real_rgb_color const *lightmap_color,
	struct render_lighting *lighting)
{
	real accuracy;
	real shadow_length;
	real ambient;
	real brightness = real_rgb_color_brightness(lightmap_color);

	lighting->ambient_color.red = object_light_ambient_scale * lightmap_color->red + object_light_ambient_base;
	lighting->ambient_color.green = object_light_ambient_scale * lightmap_color->green + object_light_ambient_base;
	lighting->ambient_color.blue = object_light_ambient_scale * lightmap_color->blue + object_light_ambient_base;
	lighting->distant_light_count = MAXIMUM_RENDERED_DISTANT_LIGHTS;
	lighting->distant_lights[0].color = *lightmap_color;
	lighting->distant_lights[0].direction.i = -radiosity_normal->i;
	lighting->distant_lights[0].direction.j = -radiosity_normal->j;
	lighting->distant_lights[0].direction.k = -radiosity_normal->k;
	lighting->distant_lights[1].color.red = object_light_secondary_scale * diffuse_color->red * brightness;
	lighting->distant_lights[1].color.green = object_light_secondary_scale * brightness * diffuse_color->green;
	lighting->distant_lights[1].color.blue = object_light_secondary_scale * diffuse_color->blue * brightness;
	lighting->distant_lights[1].direction = *surface_normal;
	lighting->reflection_tint_color.alpha = PIN(brightness * 1.5f + 0.25f, 0.f, 1.f);
	lighting->reflection_tint_color.red = PIN(diffuse_color->red * 3.f + 0.5f, 0.f, 1.f);
	lighting->reflection_tint_color.green = PIN(diffuse_color->green * 3.f + 0.5f, 0.f, 1.f);
	lighting->reflection_tint_color.blue = PIN(diffuse_color->blue * 3.f + 0.5f, 0.f, 1.f);
	lighting->reflection_tint_color.red *= PIN(lightmap_color->red * 2.f + 0.25f, 0.f, 1.f);
	lighting->reflection_tint_color.green *= PIN(lightmap_color->green * 2.f + 0.25f, 0.f, 1.f);
	lighting->reflection_tint_color.blue *= PIN(lightmap_color->blue * 2.f + 0.25f, 0.f, 1.f);
	accuracy = power(radiosity_accuracy, 0.25f);
	lighting->shadow_vector.i = accuracy * lighting->distant_lights[0].direction.i;
	lighting->shadow_vector.j = accuracy * lighting->distant_lights[0].direction.j;
	shadow_length = magnitude2d((real_vector2d *)&lighting->shadow_vector);

	if (shadow_length < MAXIMUM_SHADOW_VECTOR_COMPONENT)
	{
		lighting->shadow_vector.k = -square_root(1.f - shadow_length * shadow_length);
	}
	else
	{
		lighting->shadow_vector.k = -MAXIMUM_SHADOW_VECTOR_COMPONENT;
		shadow_length = MAXIMUM_SHADOW_VECTOR_COMPONENT / shadow_length;
		lighting->shadow_vector.i *= shadow_length;
		lighting->shadow_vector.j *= shadow_length;
	}

	ambient = (1.f - radiosity_accuracy) * 0.5f;
	lighting->shadow_color.red = PIN(
		1.f - lighting->distant_lights[0].color.red * 1.3f + ambient,
		object_light_ambient_base,
		1.f);
	lighting->shadow_color.green = PIN(
		1.f - lighting->distant_lights[0].color.green * 1.3f + ambient,
		object_light_ambient_base,
		1.f);
	lighting->shadow_color.blue = PIN(
		1.f - lighting->distant_lights[0].color.blue * 1.3f + ambient,
		object_light_ambient_base,
		1.f);

	if (TEST_FLAG(flags, _distant_lighting_brighten_bit))
	{
		brighten_real_rgb_color(&lighting->ambient_color, 0.2f);
		brighten_real_rgb_color(&lighting->distant_lights[0].color, 0.3f);
		brighten_real_rgb_color(&lighting->distant_lights[1].color, 0.2f);
		brighten_real_rgb_color(&lighting->reflection_tint_color.rgb, 0.5f);
		lighting->reflection_tint_color.alpha = 1.f;
	}

	return;
}

static void brighten_real_rgb_color(
	real_rgb_color *color,
	real fraction)
{
	real maximum = MAX(color->red, MAX(color->green, color->blue));
	real scale = fraction + 1.f;
	real maximum_scaled = scale * maximum;

	if (maximum_scaled > 1.f)
	{
		scale = 1.f / maximum;
	}
	else if (maximum_scaled < fraction)
	{
		scale = fraction / maximum;
	}

	color->red *= scale;
	color->green *= scale;
	color->blue *= scale;

	return;
}

static void light_compute_bounding_sphere(
	long light_index,
	boolean maximum,
	boolean specular,
	boolean lens_flare_only,
	real_point3d *bounding_sphere_center,
	real *bounding_sphere_radius)
{
	struct point_light *light = light_get(light_index);
	struct point_light_definition *definition = point_light_definition_get(light->definition_index);
	real radius = maximum ?
		definition->geometry.radius_modifier_upper_bound * definition->geometry.radius :
		light->radius;

	if (!TEST_FLAG(definition->flags, _light_no_specular_bit) && (specular || maximum))
	{
		radius *= definition->geometry.specular_radius_multiplier;
	}

	if (lens_flare_only && radius < definition->geometry.lens_flare_radius)
	{
		*bounding_sphere_center = light->position;
		*bounding_sphere_radius = definition->geometry.lens_flare_radius;
	}
	else if (definition->geometry.cutoff_angle < _half_pi)
	{
		if (definition->geometry.cutoff_angle < _pi / 4.f)
		{
			*bounding_sphere_radius = radius / definition->geometry.runtime_cosine_cutoff_angle;
			point_from_line3d(&light->position, &light->forward, *bounding_sphere_radius, bounding_sphere_center);
		}
		else
		{
			*bounding_sphere_radius = radius * definition->geometry.runtime_sine_cutoff_angle;
			radius *= definition->geometry.runtime_cosine_cutoff_angle;
			point_from_line3d(&light->position, &light->forward, radius, bounding_sphere_center);
		}
	}
	else
	{
		*bounding_sphere_center = light->position;
		*bounding_sphere_radius = radius;
	}

	return;
}

static long cluster_get_first_light(
	long *state,
	short cluster_index)
{
	return cluster_partition_get_first_datum(&light_cluster_partition, state, cluster_index);
}

static long cluster_get_next_light(
	long *state)
{
	return cluster_partition_get_next_datum(&light_cluster_partition, state);
}

static void light_get_bounding_sphere(
	long light_index,
	real_point3d *position,
	real *radius)
{
	light_compute_bounding_sphere(light_index, TRUE, FALSE, TRUE, position, radius);

	return;
}

static void light_marker_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 1636, !lights_globals.marker_initialized);
	lights_globals.marker++;
	lights_globals.marker_initialized = TRUE;

	return;
}

static boolean light_unmarked(
	long light_index)
{
	struct point_light *light = light_get(light_index);

	match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 1647, lights_globals.marker_initialized);

	if (light->magic_number != lights_globals.marker)
	{
		return TRUE;
	}

	return FALSE;
}

static boolean light_mark(
	long light_index)
{
	struct point_light *light = light_get(light_index);

	match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 1663, lights_globals.marker_initialized);

	if (light->magic_number != lights_globals.marker)
	{
		light->magic_number = lights_globals.marker;
		return TRUE;
	}

	return FALSE;
}

static void light_marker_end(
	void)
{
	match_assert("c:\\halo\\SOURCE\\objects\\object_lights.c", 1678, lights_globals.marker_initialized);
	lights_globals.marker_initialized = FALSE;

	return;
}

static void render_debug_light(
	long light_index)
{
	if (debug_lights)
	{
		struct point_light *light = light_get(light_index);
		struct point_light_definition *definition = point_light_definition_get(light->definition_index);
		real radius = definition->geometry.radius_modifier_upper_bound * definition->geometry.radius;
		real_argb_color color = *global_real_argb_orange;

		render_debug_sphere(TRUE, &light->position, definition->geometry.lens_flare_radius, global_real_argb_white);
		render_debug_sphere(TRUE, &light->position, light->radius, &color);
		color.red *= 0.8f;
		color.green *= 0.8f;
		color.blue *= 0.8f;

		if (!TEST_FLAG(definition->flags, _light_no_specular_bit))
		{
			radius *= definition->geometry.specular_radius_multiplier;
			render_debug_sphere(
				TRUE,
				&light->position,
				definition->geometry.specular_radius_multiplier * light->radius,
				&color);
		}

		color.red *= 0.8f;
		color.green *= 0.8f;
		color.blue *= 0.8f;
		render_debug_sphere(TRUE, &light->position, radius, &color);
	}

	return;
}

/* ---------- public code */

void lights_queue_lens_flare(
	long definition_index,
	real_point3d const *position,
	real_vector3d const *direction,
	real_vector3d const *up,
	real_rgb_color const *color,
	real scale)
{
	if (lights_globals.queued_lens_flare_count < MAXIMUM_QUEUED_LENS_FLARES &&
		(color->red != 0.f || color->green != 0.f || color->blue != 0.f))
	{
		struct rasterizer_lens_flare_submit_parameters *lens_flare_parameters =
			&lights_globals.queued_lens_flares[lights_globals.queued_lens_flare_count];

		lens_flare_parameters->compressed_light_color = real_a_rgb_color_to_pixel32(1.f, color);
		lens_flare_parameters->compressed_light_scale = compress_real_to_int8(scale);
		lens_flare_parameters->definition = lens_flare_definition_get(definition_index);
		lens_flare_parameters->position = *position;
		lens_flare_parameters->compressed_direction = compress_real_vector3d_to_int32_clamp(direction);
		lens_flare_parameters->compressed_up = compress_real_vector3d_to_int32_clamp(up);
		lens_flare_parameters->compressed_window_index = (byte)render.window_index;
		lens_flare_parameters->light_index = NONE;
		lens_flare_parameters->light_identifier = NONE;
		lens_flare_parameters->lens_flare_index = lights_globals.queued_lens_flare_count++;
	}

	return;
}
