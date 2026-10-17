/*
RENDER_OBJECTS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "render.h"
#include "render_debug.h"
#include "collision_bsp.h"
#include "network_game_globals.h"
#include "units.h"
#include "rasterizer.h"
#include "players.h"
#include "console.h"
#include "rasterizer_console_vars.h"
#include "game_state.h"
#include "shaders.h"
#include "director.h"
#include "structures.h"
#include "structure_render.h"
#include "editor_stubs.h"
#include "object_lights.h"
#include "object_types.h"
#include "cinematics.h"
#include "devices.h"
#include "physics.h"
#include "first_person_weapons.h"
#include "widgets.h"
#include "camera_scripting.h"

/* ---------- constants */

enum
{
	MAXIMUM_CACHED_OBJECT_RENDER_STATES = 256,	/* fake name */
	OBJECT_RENDER_STATE_LARGE_INTERVAL = 3,		/* fake name */
	OBJECT_RENDER_STATE_SMALL_INTERVAL = 10,	/* fake name */
};

#define OBJECT_RENDER_STATE_LARGE_PIXELS 400.f				/* fake name */
#define OBJECT_RENDER_STATE_SMALL_PIXELS 100.f				/* fake name */
#define MAXIMUM_OBJECT_RENDER_STATE_AGE 1000.f				/* fake name */
#define OBJECT_LIGHTING_MAXIMUM_DELTA 0.03f					/* fake name */
#define OBJECT_SHADOW_LEVEL_OF_DETAIL_SCALE 0.3f			/* fake name */
#define OBJECT_SHADOW_MINIMUM_PIXELS 30.f					/* fake name */
#define OBJECT_SHADOW_FADE_PIXELS 15.f						/* fake name */
#define OBJECT_SHADOW_MINIMUM_DARKNESS 0.19f				/* fake name */
#define OBJECT_SHADOW_MAXIMUM_DARKNESS 0.3f					/* fake name */
#define OBJECT_SHADOW_LENGTH_SCALE 4.f						/* fake name */
#define OBJECT_SHADOW_OFFSET_SCALE 0.5f						/* fake name */
#define INACTIVE_OBJECT_TEXT_HEIGHT 0.2f					/* fake name */

/* ---------- macros */

#define object_render_state_get(index) ((struct object_render_state *)datum_get(cached_object_render_states, (index)))

/* ---------- structures */

struct object_render_data
{
	long object_index;
	struct render_lighting *lighting;
	boolean shadow;
	boolean no_planar_fog;
	real_matrix4x3 shadow_matrix;
	real shadow_bounding_radius;
	long rasterizer_bitmap_cookie;
};

struct object_render_state
{
	short identifier;
	word pad;
	long object_index;
	long refresh_frame_index;
	long render_scene_index;
	long render_frame_index;
	struct render_lighting lighting;
	struct render_lighting desired_lighting;
	real lod_pixels;
};

struct render_object_globals
{
	short rendered_object_count;
	long rendered_object_indices[MAXIMUM_RENDERED_OBJECTS];
};

/* ---------- prototypes */

static boolean object_is_first_person_camera(long object_index);
static void find_rendered_objects(void);
static void process_rendered_objects(struct object_render_data *data);
static void render_object(struct object_render_data *data);
static void render_object_list(struct object_render_data *data, struct render_model_effect *parent_model_effect, long object_index);
static long object_get_cached_render_state(long object_index, real level_of_detail_pixels);
static void object_render_state_refresh(long cache_index, long object_index, real level_of_detail_pixels, boolean dirty);
static void interpolate_real_rgb_color(real_rgb_color *current, real_rgb_color const *desired, real maximum_speed);
static void interpolate_real_argb_color(real_argb_color *current, real_argb_color const *desired, real maximum_speed);
static void interpolate_normal(real_vector3d *current, real_vector3d const *desired, real maximum_speed);
static real object_get_level_of_detail_pixels(long object_index);
static boolean render_object_shadow_begin(struct object_render_data *data, real lod);
static void bounding_planes_from_point_and_normal(real_vector3d const *n, real_point3d const *p, real_plane3d *lb, real_plane3d *ub);
static void render_object_shadow_end(struct object_render_data *data);

/* ---------- globals */

boolean render_shadows = TRUE;

static struct profile_section render_objects_section = {"render_objects", NONE, TRUE};
static struct profile_section render_object_shadows_section = {"render_object_shadows", NONE, TRUE};

boolean debug_objects;
struct data_array *cached_object_render_states;

static struct render_object_globals render_object_globals;

boolean debug_inactive_objects = FALSE;

/* ---------- public code */

void render_objects_initialize(
	void)
{
	cached_object_render_states = game_state_data_new(
		"cached object render states",
		MAXIMUM_CACHED_OBJECT_RENDER_STATES,
		sizeof(struct object_render_state));
	match_assert("c:\\halo\\SOURCE\\render\\render_objects.c", 125, cached_object_render_states);

	return;
}

void render_objects_initialize_for_new_map(
	void)
{
	data_make_valid(cached_object_render_states);

	return;
}

void render_objects_dispose_from_old_map(
	void)
{
	if (cached_object_render_states && cached_object_render_states->valid)
	{
		data_make_invalid(cached_object_render_states);
	}

	return;
}

void render_objects_dispose(
	void)
{
	cached_object_render_states = NULL;

	return;
}

void render_objects(
	void)
{
	struct object_render_data data;
	boolean first_person_weapon = FALSE;

	profile_enter(render_objects_section);

	rasterizer_models_begin(FALSE);
	find_rendered_objects();
	data.shadow = FALSE;

	do
	{
		if (first_person_weapon != rasterizer_debug_options.draw_first_person_weapon_first)
		{
			first_person_weapon_draw();
		}
		else
		{
			process_rendered_objects(&data);
		}

		first_person_weapon = !first_person_weapon;
	}
	while (first_person_weapon);

	rasterizer_models_end();

	profile_exit(render_objects_section);

	return;
}

void render_object_shadows(
	void)
{
	profile_enter(render_object_shadows_section);

	if (render_shadows)
	{
		struct object_render_data data;

		rasterizer_environment_shadows_begin();
		data.shadow = TRUE;
		process_rendered_objects(&data);
		rasterizer_environment_shadows_end();
	}

	profile_exit(render_object_shadows_section);

	return;
}

struct render_lighting *object_get_cached_render_lighting(
	long object_index,
	real level_of_detail_pixels)
{
	static struct render_lighting lighting_storage;
	struct render_lighting *lighting;
	long cache_index = object_get_cached_render_state(object_index, level_of_detail_pixels);

	if (cache_index != NONE)
	{
		lighting = &object_render_state_get(cache_index)->lighting;
	}
	else
	{
		lights_prepare_for_object_static(object_index, &lighting_storage);
		lights_prepare_for_object_dynamic(object_index, &lighting_storage);
		lighting = &lighting_storage;
	}

	return lighting;
}

static boolean object_is_first_person_camera(
	long object_index)
{
	return
		(local_player_get_player_index(render.local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(render.local_player_index))->unit_index) == object_index &&
		director_get_perspective(render.local_player_index) == _director_perspective_first_person ||
		scripted_camera_object_is_first_person_camera(object_index);
}

static void find_rendered_objects(
	void)
{
	static boolean once = FALSE;

	object_marker_begin();
	render_object_globals.rendered_object_count = structure_visibility_find_objects(
		render_object_globals.rendered_object_indices,
		MAXIMUM_RENDERED_OBJECTS,
		cluster_get_first_collideable_object,
		cluster_get_next_collideable_object,
		object_get_render_bounding_sphere,
		object_unmarked_function,
		object_mark_function);
	render_object_globals.rendered_object_count += structure_visibility_find_objects(
		&render_object_globals.rendered_object_indices[render_object_globals.rendered_object_count],
		MAXIMUM_RENDERED_OBJECTS - render_object_globals.rendered_object_count,
		cluster_get_first_noncollideable_object,
		cluster_get_next_noncollideable_object,
		object_get_render_bounding_sphere,
		object_unmarked_function,
		object_mark_function);
	object_marker_end();

	if (render_object_globals.rendered_object_count == MAXIMUM_RENDERED_OBJECTS && !once)
	{
		error(_error_silent, "MAXIMUM_RENDERED_OBJECTS exceeded.");
		once = TRUE;
	}

	return;
}

static void process_rendered_objects(
	struct object_render_data *data)
{
	short rendered_object_index;

	for (rendered_object_index = 0; rendered_object_index < render_object_globals.rendered_object_count; rendered_object_index++)
	{
		data->object_index = render_object_globals.rendered_object_indices[rendered_object_index];
		render_object(data);
	}

	return;
}

static void render_object(
	struct object_render_data *data)
{
	if (data->shadow)
	{
		struct object_datum *object = object_get(data->object_index);

		if (!object_is_first_person_camera(data->object_index) &&
			!TEST_FLAG(object->object.flags, _object_shadowless_bit) &&
			(!TEST_FLAG(object->object.flags, _object_invisible_bit) || object->object.first_child_object_index != NONE))
		{
			real level_of_detail_pixels;
			real darkness;

			data->lighting = object_get_cached_render_lighting(
				data->object_index,
				object_get_level_of_detail_pixels(data->object_index));
			level_of_detail_pixels = object_get_level_of_detail_pixels(data->object_index);
			darkness = 1.f - real_rgb_color_brightness(&data->lighting->shadow_color);

			if (level_of_detail_pixels > OBJECT_SHADOW_MINIMUM_PIXELS && darkness > OBJECT_SHADOW_MINIMUM_DARKNESS)
			{
				level_of_detail_pixels = (level_of_detail_pixels - OBJECT_SHADOW_MINIMUM_PIXELS) / OBJECT_SHADOW_FADE_PIXELS;
				darkness = (darkness - OBJECT_SHADOW_MINIMUM_DARKNESS) / (OBJECT_SHADOW_MAXIMUM_DARKNESS - OBJECT_SHADOW_MINIMUM_DARKNESS);
				level_of_detail_pixels = PIN(level_of_detail_pixels, 0.f, 1.f);
				darkness = PIN(darkness, 0.f, 1.f);

				if (render_object_shadow_begin(data, darkness * level_of_detail_pixels))
				{
					render_object_list(data, NULL, data->object_index);
					render_object_shadow_end(data);
				}
			}
		}
	}
	else
	{
		struct object_datum *object = object_get(data->object_index);
		boolean needs_lighting =
			!TEST_FLAG(object->object.flags, _object_invisible_bit) ||
			object->object.first_child_object_index != NONE ||
			widgets_need_lighting(object->object.first_widget_index);

		if (needs_lighting || object->object.first_widget_index != NONE)
		{
			struct render_model_effect model_effect;
			struct object_definition *definition = object_definition_get(object->definition_index);

			if (needs_lighting)
			{
				data->lighting = object_get_cached_render_lighting(
					data->object_index,
					object_get_level_of_detail_pixels(data->object_index));
			}
			else
			{
				data->lighting = NULL;
			}

			if (editor_preprocess_rendered_object(data->object_index, data->lighting))
			{
				model_effect.type = _render_model_effect_type_none;
				model_effect.modifier_shader = NULL;
				model_effect.modifier_animation.values = NULL;
				model_effect.modifier_animation.colors = NULL;
				data->no_planar_fog =
					render.fog.planar_mode != _render_planar_fog_mode_normal ||
					plane3d_distance_to_point(&render.fog.plane, &object->object.bounding_sphere_center) > definition->object.bounding_radius;
				render_object_list(data, &model_effect, data->object_index);
			}
		}
	}

	return;
}

static void render_object_list(
	struct object_render_data *data,
	struct render_model_effect *parent_model_effect,
	long object_index)
{
	while (object_index != NONE)
	{
		struct object_datum *object = object_get(object_index);

		if (!object_is_first_person_camera(object_index) || render.camera.mirrored)
		{
			struct render_model_effect model_effect;

			if (!data->shadow)
			{
				match_assert("c:\\halo\\SOURCE\\render\\render_objects.c", 390, parent_model_effect);

				model_effect = *parent_model_effect;

				if (parent_model_effect->type == _render_model_effect_type_transparent_zbuffered)
				{
					model_effect.type = _render_model_effect_type_none;
					model_effect.modifier_shader = NULL;
					model_effect.modifier_animation.values = NULL;
					model_effect.modifier_animation.colors = NULL;
				}
				else
				{
					model_effect = *parent_model_effect;
				}
			}

			if (!TEST_FLAG(object->object.flags, _object_invisible_bit))
			{
				struct object_definition *definition = object_definition_get(object->definition_index);
				real level_of_detail_pixels = object_get_level_of_detail_pixels(object_index);

				match_assert("c:\\halo\\SOURCE\\render\\render_objects.c", 415, data->lighting);

				if (!data->shadow)
				{
					struct object_header_datum *object_header;

					if (definition->object.modifier_shader.index != NONE)
					{
						model_effect.modifier_shader = shader_definition_get(definition->object.modifier_shader.index);

						if (shader_type_is_valid_for_modifier(model_effect.modifier_shader->base.type))
						{
							model_effect.modifier_animation.colors = object->object.outgoing_change_colors;
							model_effect.modifier_animation.values = object->object.outgoing_function_values;
						}
						else
						{
							error(_error_silent, "### ERROR invalid modifier shader", model_effect.modifier_shader->base.type);
							model_effect.modifier_shader = NULL;
						}
					}

					if (TEST_FLAG(_object_mask_unit, object->object.type))
					{
						struct unit_datum *unit = unit_get(object_index);

						if (unit->unit.active_camouflage > 0.f)
						{
							model_effect.type = _render_model_effect_type_active_camouflage;
							model_effect.source_object_index = object_index;
							model_effect.source_object_centroid = object->object.bounding_sphere_center;
							model_effect.intensity = unit->unit.active_camouflage;
							model_effect.parameter = unit->unit.active_camouflage_super_amount;
						}
					}

					if (TEST_FLAG(definition->object.flags, _object_transparency_self_occludes_bit))
					{
						model_effect.type = _render_model_effect_type_transparent_zbuffered;
						model_effect.source_object_index = object_index;
						model_effect.source_object_centroid = object->object.bounding_sphere_center;
					}

					object_header = object_header_get(object_index);

					if (debug_inactive_objects &&
						object->object.parent_object_index == NONE &&
						!TEST_FLAG(object_header->flags, _object_header_active_bit) &&
						definition->object.model.index != NONE)
					{
						real_point3d text_point;
						char text_buffer[512];

						point_from_line3d(
							&object->object.bounding_sphere_center,
							global_up3d,
							INACTIVE_OBJECT_TEXT_HEIGHT,
							&text_point);
						sprintf(text_buffer, "inactive %s", tag_name_strip_path(tag_get_name(object->definition_index)));
						render_debug_point(
							FALSE,
							&object->object.bounding_sphere_center,
							object->object.bounding_sphere_radius,
							global_real_argb_blue);
						render_debug_string_at_point(FALSE, &text_point, text_buffer, global_real_argb_blue);
					}

					render_model(
						definition->object.model.index,
						level_of_detail_pixels,
						object_get_node_matrices(object_index),
						object->object.region_permutations,
						object->object.outgoing_change_colors,
						object->object.outgoing_function_values,
						data->lighting,
						&object->object.bounding_sphere_center,
						object->object.bounding_sphere_radius,
						&model_effect,
						object_index,
						object->object.forced_shader_permutation_index,
						data->no_planar_fog ? FLAG(_rasterizer_geometry_no_fog_bit) : 0);

					if (debug_objects)
					{
						object_type_render_debug(object_index);
					}
				}
				else
				{
					render_model(
						definition->object.model.index,
						level_of_detail_pixels * OBJECT_SHADOW_LEVEL_OF_DETAIL_SCALE,
						object_get_node_matrices(object_index),
						object->object.region_permutations,
						object->object.outgoing_change_colors,
						object->object.outgoing_function_values,
						data->lighting,
						&object->object.bounding_sphere_center,
						object->object.bounding_sphere_radius,
						NULL,
						object_index,
						object->object.forced_shader_permutation_index,
						FLAG(_rasterizer_geometry_no_queue_bit));
				}
			}

			if (!data->shadow && object->object.first_widget_index != NONE)
			{
				struct render_animation animation;

				animation.colors = object->object.outgoing_change_colors;
				animation.values = object->object.outgoing_function_values;
				widgets_render(object_index, data->lighting, &animation);
			}

			if (object->object.first_child_object_index != NONE)
			{
				render_object_list(data, data->shadow ? NULL : &model_effect, object->object.first_child_object_index);
			}
		}

		object_index = object->object.next_object_index;
	}

	return;
}

static long object_get_cached_render_state(
	long object_index,
	real level_of_detail_pixels)
{
	struct object_datum *object = object_get(object_index);
	long cache_index = NONE;

	if (object->object.cached_render_state_index != NONE && object_render_state_get(object->object.cached_render_state_index)->object_index == object_index)
	{
		cache_index = object->object.cached_render_state_index;
	}

	if (cache_index == NONE)
	{
		cache_index = datum_new(cached_object_render_states);

		if (cache_index == NONE)
		{
			long state_index;
			real oldest_age = REAL_MIN;

			for (state_index = data_next_index(cached_object_render_states, NONE);
				state_index != NONE;
				state_index = data_next_index(cached_object_render_states, state_index))
			{
				real age = (real)(render.scene_index - object_render_state_get(state_index)->render_scene_index);

				if (age < 0.f)
				{
					age = MAXIMUM_OBJECT_RENDER_STATE_AGE;
				}

				if (age > oldest_age)
				{
					oldest_age = age;
					cache_index = state_index;
				}
			}
		}

		if (cache_index != NONE)
		{
			object_render_state_refresh(cache_index, object_index, level_of_detail_pixels, TRUE);
			object->object.cached_render_state_index = cache_index;
		}
	}
	else
	{
		object_render_state_refresh(cache_index, object_index, level_of_detail_pixels, FALSE);
	}

	return cache_index;
}

static void object_render_state_refresh(
	long cache_index,
	long object_index,
	real level_of_detail_pixels,
	boolean dirty)
{
	short point_light_index;
	struct object_render_state *state = object_render_state_get(cache_index);
	long scene_age = render.scene_index - state->render_scene_index;
	long frame_age = render.frame_index - state->render_frame_index;
	long refresh_age = render.frame_index - state->refresh_frame_index;
	boolean refresh = FALSE;

	if (refresh_age < 0 || scene_age < 0)
	{
		refresh_age = 1;
		scene_age = 1;
	}

	if (TEST_FLAG(object_get(object_index)->object.flags, _object_static_lighting_recompute_bit))
	{
		if (level_of_detail_pixels > OBJECT_RENDER_STATE_LARGE_PIXELS)
		{
			refresh = refresh_age > 0;
		}
		else if (level_of_detail_pixels > OBJECT_RENDER_STATE_SMALL_PIXELS)
		{
			refresh = refresh_age > OBJECT_RENDER_STATE_LARGE_INTERVAL;
		}
		else
		{
			refresh = refresh_age > OBJECT_RENDER_STATE_SMALL_INTERVAL;
		}
	}

	if (refresh && frame_age > 1 && TEST_FLAG(object_get(object_index)->object.flags, _object_static_lighting_recompute_bit))
	{
		dirty = TRUE;
	}

	if (dirty || refresh)
	{
		state->object_index = object_index;
		lights_prepare_for_object_static(object_index, &state->desired_lighting);
		state->lod_pixels = level_of_detail_pixels;
		state->refresh_frame_index = render.frame_index;
	}

	if (dirty || scene_age > 0)
	{
		lights_prepare_for_object_dynamic(object_index, &state->desired_lighting);
	}

	if (dirty || refresh)
	{
		if (dirty || !object_light_interpolate)
		{
			state->lighting = state->desired_lighting;
		}
		else
		{
			real_vector3d translational_velocity;
			struct render_lighting *lighting = &state->lighting;
			struct render_lighting *desired_lighting = &state->desired_lighting;

			match_assert("c:\\halo\\SOURCE\\render\\render_objects.c", 635, state->desired_lighting.distant_light_count==2);

			object_get_velocities(object_index, &translational_velocity, NULL);

			if (translational_velocity.i != 0.f ||
				translational_velocity.j != 0.f ||
				translational_velocity.k != 0.f ||
				object_try_and_get_and_verify_type(object_index, _object_mask_machine))
			{
				interpolate_real_rgb_color(
					&lighting->ambient_color,
					&desired_lighting->ambient_color,
					OBJECT_LIGHTING_MAXIMUM_DELTA);
				interpolate_real_argb_color(
					&lighting->reflection_tint_color,
					&desired_lighting->reflection_tint_color,
					OBJECT_LIGHTING_MAXIMUM_DELTA);
				interpolate_real_rgb_color(
					&lighting->distant_lights[0].color,
					&desired_lighting->distant_lights[0].color,
					OBJECT_LIGHTING_MAXIMUM_DELTA);
				interpolate_normal(
					&lighting->distant_lights[0].direction,
					&desired_lighting->distant_lights[0].direction,
					OBJECT_LIGHTING_MAXIMUM_DELTA);
				interpolate_real_rgb_color(
					&lighting->distant_lights[1].color,
					&desired_lighting->distant_lights[1].color,
					OBJECT_LIGHTING_MAXIMUM_DELTA);
				interpolate_normal(
					&lighting->distant_lights[1].direction,
					&desired_lighting->distant_lights[1].direction,
					OBJECT_LIGHTING_MAXIMUM_DELTA);
				interpolate_normal(
					&lighting->shadow_vector,
					&desired_lighting->shadow_vector,
					0.4f * OBJECT_LIGHTING_MAXIMUM_DELTA);
				interpolate_real_rgb_color(
					&lighting->shadow_color,
					&desired_lighting->shadow_color,
					OBJECT_LIGHTING_MAXIMUM_DELTA);
			}

			state->lighting.point_light_count = state->desired_lighting.point_light_count;
			state->lighting.point_light_indices[0] = state->desired_lighting.point_light_indices[0];
			state->lighting.point_light_indices[1] = state->desired_lighting.point_light_indices[1];
		}
	}
	else if (scene_age > 0)
	{
		state->lighting.point_light_count = state->desired_lighting.point_light_count;
		state->lighting.point_light_indices[0] = state->desired_lighting.point_light_indices[0];
		state->lighting.point_light_indices[1] = state->desired_lighting.point_light_indices[1];
	}

	for (point_light_index = 0; point_light_index < state->lighting.point_light_count; point_light_index++)
	{
		match_assert(
			"c:\\halo\\SOURCE\\render\\render_objects.c",
			690,
			state->lighting.point_light_indices[point_light_index]>=0 && state->lighting.point_light_indices[point_light_index]<debug_rasterizer_light_count);
	}

	state->render_scene_index = render.scene_index;
	state->render_frame_index = render.frame_index;

	return;
}

static void interpolate_real_rgb_color(
	real_rgb_color *current,
	real_rgb_color const *desired,
	real maximum_speed)
{
	real delta;

	delta = desired->red - current->red;
	current->red += PIN(delta, -maximum_speed, maximum_speed);
	delta = desired->green - current->green;
	current->green += PIN(delta, -maximum_speed, maximum_speed);
	delta = desired->blue - current->blue;
	current->blue += PIN(delta, -maximum_speed, maximum_speed);

	return;
}

static void interpolate_real_argb_color(
	real_argb_color *current,
	real_argb_color const *desired,
	real maximum_speed)
{
	real delta;

	delta = desired->alpha - current->alpha;
	current->alpha += PIN(delta, -maximum_speed, maximum_speed);
	delta = desired->red - current->red;
	current->red += PIN(delta, -maximum_speed, maximum_speed);
	delta = desired->green - current->green;
	current->green += PIN(delta, -maximum_speed, maximum_speed);
	delta = desired->blue - current->blue;
	current->blue += PIN(delta, -maximum_speed, maximum_speed);

	return;
}

static void interpolate_normal(
	real_vector3d *current,
	real_vector3d const *desired,
	real maximum_speed)
{
	real delta;

	delta = desired->i - current->i;
	current->i += PIN(delta, -maximum_speed, maximum_speed);
	delta = desired->j - current->j;
	current->j += PIN(delta, -maximum_speed, maximum_speed);
	delta = desired->k - current->k;
	current->k += PIN(delta, -maximum_speed, maximum_speed);
	normalize3d(current);

	return;
}

static real object_get_level_of_detail_pixels(
	long object_index)
{
	real pixels;

	if (cinematic_in_progress() && TEST_FLAG(object_get(object_index)->object.flags, _object_movie_star_bit))
	{
		pixels = REAL_MAX;
	}
	else
	{
		real_point3d bounding_sphere_center;
		real bounding_sphere_radius;

		object_get_bounding_sphere(object_index, &bounding_sphere_center, &bounding_sphere_radius);
		pixels = render_frustum_sphere_diameter_in_pixels(&render.frustum, &bounding_sphere_center, bounding_sphere_radius);
	}

	return pixels;
}

static boolean render_object_shadow_begin(
	struct object_render_data *data,
	real lod)
{
	real_point3d bounding_sphere_center;
	real bounding_sphere_radius;
	real_rgb_color shadow_color;
	real inverse_shadow_scale;
	real_vector3d perpendicular;
	real shadow_scale = lod;

	object_get_bounding_sphere(data->object_index, &bounding_sphere_center, &bounding_sphere_radius);
	perpendicular3d(&data->lighting->shadow_vector, &perpendicular);
	normalize3d(&perpendicular);
	matrix4x3_from_point_and_vectors(
		&data->shadow_matrix,
		&bounding_sphere_center,
		&perpendicular,
		&data->lighting->shadow_vector);
	shadow_color = data->lighting->shadow_color;

	if (TEST_FLAG(_object_mask_unit, object_get(data->object_index)->object.type))
	{
		struct unit_datum *unit = unit_get(data->object_index);

		if (unit->unit.active_camouflage > 0.f)
		{
			shadow_scale = (1.f - unit->unit.active_camouflage) * shadow_scale;
		}
	}

	inverse_shadow_scale = 1.f - shadow_scale;
	shadow_color.red = shadow_color.red * shadow_scale + inverse_shadow_scale;
	shadow_color.green = shadow_color.green * shadow_scale + inverse_shadow_scale;
	shadow_color.blue = shadow_color.blue * shadow_scale + inverse_shadow_scale;

	return rasterizer_environment_shadow_begin(
		data->object_index,
		&data->shadow_matrix,
		&shadow_color,
		bounding_sphere_radius,
		&data->shadow_bounding_radius);
}

static void bounding_planes_from_point_and_normal(
	real_vector3d const *n,
	real_point3d const *p,
	real_plane3d *lb,
	real_plane3d *ub)
{
	lb->n = *n;
	lb->d = dot_product3d(n, (real_vector3d const *)p);
	ub->n.i = -lb->n.i;
	ub->n.j = -lb->n.j;
	ub->n.k = -lb->n.k;
	ub->d = -lb->d;

	return;
}

static void render_object_shadow_end(
	struct object_render_data *data)
{
	real_plane3d bounding_cube[6];
	real_rectangle3d axis_aligned_bounding_cube;
	real shadow_length;

	bounding_planes_from_point_and_normal(
		&data->shadow_matrix.up,
		&data->shadow_matrix.position,
		&bounding_cube[0],
		&bounding_cube[1]);
	bounding_cube[0].d -= OBJECT_SHADOW_OFFSET_SCALE * data->shadow_bounding_radius;
	shadow_length = OBJECT_SHADOW_LENGTH_SCALE * data->shadow_bounding_radius;
	bounding_cube[1].d -= shadow_length;
	bounding_planes_from_point_and_normal(
		&data->shadow_matrix.forward,
		&data->shadow_matrix.position,
		&bounding_cube[2],
		&bounding_cube[3]);
	bounding_cube[2].d -= data->shadow_bounding_radius;
	bounding_cube[3].d -= data->shadow_bounding_radius;
	bounding_planes_from_point_and_normal(
		&data->shadow_matrix.left,
		&data->shadow_matrix.position,
		&bounding_cube[4],
		&bounding_cube[5]);
	bounding_cube[4].d -= data->shadow_bounding_radius;
	bounding_cube[5].d -= data->shadow_bounding_radius;

	axis_aligned_bounding_cube.x1 = ABS(data->shadow_matrix.forward.i) + ABS(data->shadow_matrix.left.i);
	axis_aligned_bounding_cube.x0 = -axis_aligned_bounding_cube.x1;
	axis_aligned_bounding_cube.y1 = ABS(data->shadow_matrix.forward.j) + ABS(data->shadow_matrix.left.j);
	axis_aligned_bounding_cube.y0 = -axis_aligned_bounding_cube.y1;
	axis_aligned_bounding_cube.z1 = ABS(data->shadow_matrix.forward.k) + ABS(data->shadow_matrix.left.k);
	axis_aligned_bounding_cube.z0 = -axis_aligned_bounding_cube.z1;
	axis_aligned_bounding_cube.x0 += data->shadow_matrix.up.i <= 0.f ?
		data->shadow_matrix.up.i * OBJECT_SHADOW_LENGTH_SCALE :
		data->shadow_matrix.up.i * -OBJECT_SHADOW_OFFSET_SCALE;
	axis_aligned_bounding_cube.x1 += data->shadow_matrix.up.i > 0.f ?
		data->shadow_matrix.up.i * OBJECT_SHADOW_LENGTH_SCALE :
		data->shadow_matrix.up.i * -OBJECT_SHADOW_OFFSET_SCALE;
	axis_aligned_bounding_cube.y0 += data->shadow_matrix.up.j <= 0.f ?
		data->shadow_matrix.up.j * OBJECT_SHADOW_LENGTH_SCALE :
		data->shadow_matrix.up.j * -OBJECT_SHADOW_OFFSET_SCALE;
	axis_aligned_bounding_cube.y1 += data->shadow_matrix.up.j > 0.f ?
		data->shadow_matrix.up.j * OBJECT_SHADOW_LENGTH_SCALE :
		data->shadow_matrix.up.j * -OBJECT_SHADOW_OFFSET_SCALE;
	axis_aligned_bounding_cube.z0 += data->shadow_matrix.up.k * OBJECT_SHADOW_LENGTH_SCALE;
	axis_aligned_bounding_cube.z1 -= data->shadow_matrix.up.k * OBJECT_SHADOW_OFFSET_SCALE;
	axis_aligned_bounding_cube.x0 = axis_aligned_bounding_cube.x0 * data->shadow_bounding_radius + data->shadow_matrix.position.x;
	axis_aligned_bounding_cube.x1 = axis_aligned_bounding_cube.x1 * data->shadow_bounding_radius + data->shadow_matrix.position.x;
	axis_aligned_bounding_cube.y0 = axis_aligned_bounding_cube.y0 * data->shadow_bounding_radius + data->shadow_matrix.position.y;
	axis_aligned_bounding_cube.y1 = axis_aligned_bounding_cube.y1 * data->shadow_bounding_radius + data->shadow_matrix.position.y;
	axis_aligned_bounding_cube.z0 = axis_aligned_bounding_cube.z0 * data->shadow_bounding_radius + data->shadow_matrix.position.z;
	axis_aligned_bounding_cube.z1 = axis_aligned_bounding_cube.z1 * data->shadow_bounding_radius + data->shadow_matrix.position.z;
	structure_render_shadow(
		&data->shadow_matrix.position,
		shadow_length,
		&axis_aligned_bounding_cube,
		NUMBEROF(bounding_cube),
		bounding_cube);
	rasterizer_environment_shadow_end();

	return;
}
