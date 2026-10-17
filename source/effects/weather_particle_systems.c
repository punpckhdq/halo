/*
WEATHER_PARTICLE_SYSTEMS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "weather_particle_systems.h"
#include "weather_particle_definitions.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "players.h"
#include "physics.h"
#include "point_physics.h"
#include "scenario.h"

/* ---------- constants */

enum
{
	MAXIMUM_NUMBER_OF_VISIBLE_WEATHER_PARTICLE_BOXES = 26,
	MAXIMUM_NUMBER_OF_WEATHER_PARTICLES_PER_MAP = 512,
	MAXIMUM_NUMBER_OF_VISIBLE_WEATHER_POLYHEDRA = 8
};

enum
{
	_clip_left = 0,
	_clip_right,
	_clip_top,
	_clip_bottom,
	_clip_far,
	NUMBER_OF_CLIPPING_PLANES
};

#define WEATHER_PARTICLE_NUDGE_DISTANCE 0.001f		/* fake name */
#define WEATHER_PARTICLE_BOXES_PER_AXIS 3			/* fake name */

/* ---------- macros */

#define weather_particle_get(index) ((struct weather_particle_datum *)datum_get(weather_particle_data, (index)))	/* fake name */

/* ---------- structures */

struct weather_particle_type_datum
{
	real density;
	real box_width;
	short particle_count;
	word pad;
	long first_particle_index;
};

struct weather_particle_system_datum
{
	long definition_index;
	real time;
	real time_delta_sec;
	real scale;
	struct location location;
	short weather_palette_index;
	boolean under_water;
	byte pad;
	struct weather_particle_type_datum types[MAXIMUM_NUMBER_OF_WEATHER_PARTICLE_TYPES];
};

struct weather_particle_datum
{
	short identifier;
	word pad1;
	real_point3d position;
	real_vector3d velocity;
	real_vector3d acceleration;
	short sequence_index;
	word pad2;
	real sprite_index;
	real rotation;
	real_argb_color color;
	real radius;
	real rotation_rate;
	real animation_rate;
	long next_particle_index;
};

/* ---------- prototypes */

static struct weather_particle_system_datum *weather_particle_system_get(short local_player_index);
static struct weather_particle_type_datum *weather_particle_system_get_type(struct weather_particle_system_datum *system, short type_index);
static void weather_particle_system_update_particle_count(short system_index, short type_index, real scale);
static long weather_particle_system_new_particle(short system_index, short type_index);
static void weather_particle_system_type_delete_particle(struct weather_particle_type_datum *type);
static void weather_particle_system_box_offset_from_point3d(real box_width, real_point3d const *point, real_point3d *box_offset);
static void weather_particle_system_update(short system_index);
static void weather_particle_update_physics(short system_index, short type_index, short particle_index);
static void weather_particle_system_wrap_point(real box_width, real_point3d const *point, real_point3d *result);
static void weather_particle_system_build_clipping_planes(real_plane3d *clip_planes, real far_clip_distance);
static void weather_particle_system_transform_clip_planes_to_box(real *clip_plane_d_transforms, real_plane3d const *clip_planes, real_point3d *box_offset);
static short weather_polyhedra_find(short *weather_polyhedra_indices, real maximum_distance);
static void weather_particle_system_render(short system_index);

/* ---------- globals */

real const one_over_char_max = 1.f/UNSIGNED_CHAR_MAX;

boolean weather = TRUE;

struct data_array *weather_particle_data;

static struct
{
	short active_system_count;
	struct weather_particle_system_datum systems[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
} weather_particle_system_globals;

/* ---------- public code */

static struct weather_particle_system_datum *weather_particle_system_get(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\effects\\weather_particle_systems.c", 91, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);

	return &weather_particle_system_globals.systems[local_player_index];
}

static struct weather_particle_type_datum *weather_particle_system_get_type(
	struct weather_particle_system_datum *system,
	short type_index)
{
	struct weather_particle_system_definition const *definition = weather_particle_system_definition_get(system->definition_index);

	match_assert("c:\\halo\\SOURCE\\effects\\weather_particle_systems.c", 102, type_index>=0 && type_index<definition->particle_types.count);

	return &system->types[type_index];
}

void weather_particle_systems_initialize(
	void)
{
	weather_particle_data = data_new(
		"weather particles",
		MAXIMUM_NUMBER_OF_WEATHER_PARTICLES_PER_MAP,
		sizeof(struct weather_particle_datum));

	if (!weather_particle_data)
	{
		error(_error_immediate, "couldn't allocate weather particle system globals.");
	}

	return;
}

void weather_particle_systems_initialize_for_new_map(
	void)
{
	short system_index;

	for (system_index = 0; system_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; system_index++)
	{
		weather_particle_system_get(system_index)->definition_index = NONE;
	}

	weather_particle_system_globals.active_system_count = 0;
	data_make_valid(weather_particle_data);

	return;
}

void weather_particle_system_new(
	short system_index,
	long definition_index,
	real scale)
{
	short type_index;
	struct weather_particle_system_datum *system = weather_particle_system_get(system_index);
	struct weather_particle_system_definition const *definition = weather_particle_system_definition_get(definition_index);

	match_assert("c:\\halo\\SOURCE\\effects\\weather_particle_systems.c", 177, system->definition_index==NONE);

	system->definition_index = definition_index;
	system->scale = scale;
	system->time = 0.f;
	system->time_delta_sec = 0.f;
	weather_particle_system_globals.active_system_count++;

	for (type_index = 0; type_index < definition->particle_types.count; type_index++)
	{
		struct weather_particle_type_datum *type = weather_particle_system_get_type(system, type_index);
		struct weather_particle_type_definition const *type_definition = TAG_BLOCK_GET_ELEMENT(
			&definition->particle_types,
			type_index,
			struct weather_particle_type_definition);

		type->first_particle_index = NONE;
		type->particle_count = 0;
		type->density = real_local_random_range(
			type_definition->particle_count_lower_bound,
			type_definition->particle_count_upper_bound);
		type->box_width = type_definition->distance_fadeout_end;
	}

	return;
}

void weather_particle_system_delete(
	short system_index)
{
	short type_index;
	struct weather_particle_system_datum *system = weather_particle_system_get(system_index);
	struct weather_particle_system_definition const *definition = weather_particle_system_definition_get(system->definition_index);

	for (type_index = 0; type_index < definition->particle_types.count; type_index++)
	{
		struct weather_particle_type_datum *type = weather_particle_system_get_type(system, type_index);

		while (type->first_particle_index != NONE)
		{
			weather_particle_system_type_delete_particle(type);
		}
	}

	weather_particle_system_globals.active_system_count--;
	system->definition_index = NONE;

	return;
}

void weather_particle_systems_dispose_from_old_map(
	void)
{
	if (weather_particle_data->valid)
	{
		data_make_invalid(weather_particle_data);
	}

	return;
}

void weather_particle_systems_dispose(
	void)
{
	if (weather_particle_data)
	{
		data_dispose(weather_particle_data);
		weather_particle_data = NULL;
	}

	return;
}

static void weather_particle_system_update_particle_count(
	short system_index,
	short type_index,
	real scale)
{
	struct weather_particle_system_datum *system = weather_particle_system_get(system_index);
	struct weather_particle_system_definition const *definition = weather_particle_system_definition_get(system->definition_index);
	struct weather_particle_type_datum *type = weather_particle_system_get_type(system, type_index);
	long particle_count = (long)(type->density*type->box_width*type->box_width*type->box_width*scale/weather_particle_system_globals.active_system_count);

	particle_count = MAX(0, particle_count);

	while (type->particle_count < particle_count)
	{
		if (weather_particle_system_new_particle(system_index, type_index) == NONE)
		{
			break;
		}
	}

	while (type->particle_count > particle_count)
	{
		weather_particle_system_type_delete_particle(type);
	}

	return;
}

static long weather_particle_system_new_particle(
	short system_index,
	short type_index)
{
	long particle_index = datum_new(weather_particle_data);

	if (particle_index != NONE)
	{
		struct weather_particle_system_datum *system = weather_particle_system_get(system_index);
		struct weather_particle_system_definition const *definition = weather_particle_system_definition_get(system->definition_index);
		struct weather_particle_type_datum *type = weather_particle_system_get_type(system, type_index);
		struct weather_particle_type_definition const *type_definition = TAG_BLOCK_GET_ELEMENT(
			&definition->particle_types,
			type_index,
			struct weather_particle_type_definition);
		struct weather_particle_datum *particle = weather_particle_get(particle_index);
		struct bitmap_group const *bitmap_group = bitmap_group_get(type_definition->bitmap.index);

		particle->position.x = real_local_random_range(0.f, type->box_width);
		particle->position.y = real_local_random_range(0.f, type->box_width);
		particle->position.z = real_local_random_range(0.f, type->box_width);
		particle->velocity.k = 0.f;
		particle->velocity.j = 0.f;
		particle->velocity.i = 0.f;
		local_random_direction3d(&particle->acceleration);
		scale_vector3d(
			&particle->acceleration,
			real_local_random_range(type_definition->acceleration_lower_bound, type_definition->acceleration_upper_bound),
			&particle->acceleration);
		particle->radius = real_local_random_range(
			type_definition->radius_lower_bound,
			type_definition->radius_upper_bound);
		particle->animation_rate = real_local_random_range(
			type_definition->animation_rate_lower_bound,
			type_definition->animation_rate_upper_bound);
		particle->rotation_rate = real_local_random_range(
			type_definition->rotation_rate_lower_bound,
			type_definition->rotation_rate_upper_bound);

		if (TEST_FLAG(type_definition->flags, _weather_particle_type_random_rotation_bit))
		{
			particle->rotation = real_local_random_range(0.f, 2.f*_pi);
		}
		else
		{
			particle->rotation = 0.f;
		}

		particle->sequence_index = local_random_range(0, bitmap_group->sequences.count);
		particle->sprite_index = real_local_random_range(
			0.f,
			(real)TAG_BLOCK_GET_ELEMENT(&bitmap_group->sequences, particle->sequence_index, struct bitmap_group_sequence)->sprites.count);
		rgb_colors_interpolate(
			&particle->color.rgb,
			type_definition->flags,
			&type_definition->color_lower_bound.rgb,
			&type_definition->color_upper_bound.rgb,
			real_local_random());
		particle->color.alpha = real_local_random_range(
			type_definition->color_lower_bound.alpha,
			type_definition->color_upper_bound.alpha);
		particle->next_particle_index = type->first_particle_index;
		type->first_particle_index = particle_index;
		type->particle_count++;
	}

	return particle_index;
}

static void weather_particle_system_type_delete_particle(
	struct weather_particle_type_datum *type)
{
	long next_particle_index = weather_particle_get(type->first_particle_index)->next_particle_index;

	datum_delete(weather_particle_data, type->first_particle_index);
	type->first_particle_index = next_particle_index;
	type->particle_count--;

	return;
}

static void weather_particle_system_box_offset_from_point3d(
	real box_width,
	real_point3d const *point,
	real_point3d *box_offset)
{
	weather_particle_system_wrap_point(box_width, point, box_offset);
	box_offset->x = point->x - box_offset->x;
	box_offset->y = point->y - box_offset->y;
	box_offset->z = point->z - box_offset->z;

	return;
}

static void weather_particle_system_update(
	short system_index)
{
	short type_index;
	struct weather_particle_system_datum *system = weather_particle_system_get(system_index);
	struct weather_particle_system_definition const *definition = weather_particle_system_definition_get(system->definition_index);

	system->time_delta_sec = render.time_delta_since_tick_sec;
	system->time += system->time_delta_sec;

	for (type_index = 0; type_index < definition->particle_types.count; type_index++)
	{
		long particle_index;
		struct weather_particle_type_datum *type = weather_particle_system_get_type(system, type_index);
		struct weather_particle_type_definition const *type_definition = TAG_BLOCK_GET_ELEMENT(
			&definition->particle_types,
			type_index,
			struct weather_particle_type_definition);
		struct bitmap_group const *bitmap_group = bitmap_group_get(type_definition->bitmap.index);
		real scale = system->scale;
		real height = render.camera.position.z;

		scale *= PIN(
			(height - type_definition->height_fadein_start)/(type_definition->height_fadein_end - type_definition->height_fadein_start),
			0.f,
			1.f);
		scale *= 1.f - PIN(
			(height - type_definition->height_fadeout_start)/(type_definition->height_fadeout_end - type_definition->height_fadeout_start),
			0.f,
			1.f);
		weather_particle_system_update_particle_count(system_index, type_index, scale);
		particle_index = type->first_particle_index;

		while (particle_index != NONE)
		{
			struct weather_particle_datum *particle = weather_particle_get(particle_index);

			particle->sprite_index += particle->animation_rate*system->time_delta_sec;
			particle->sprite_index = (real)fmod(
				particle->sprite_index,
				TAG_BLOCK_GET_ELEMENT(&bitmap_group->sequences, particle->sequence_index, struct bitmap_group_sequence)->sprites.count);
			particle->rotation += particle->rotation_rate*system->time_delta_sec*(particle_index & 1 ? -1 : 1);
			weather_particle_update_physics(system_index, type_index, particle_index);
			particle_index = particle->next_particle_index;
		}
	}

	return;
}

static void weather_particle_update_physics(
	short system_index,
	short type_index,
	short particle_index)
{
	struct weather_particle_system_datum *system = weather_particle_system_get(system_index);
	struct weather_particle_system_definition const *definition = weather_particle_system_definition_get(system->definition_index);
	struct weather_particle_type_datum *type = weather_particle_system_get_type(system, type_index);
	struct weather_particle_type_definition const *type_definition = TAG_BLOCK_GET_ELEMENT(
		&definition->particle_types,
		type_index,
		struct weather_particle_type_definition);
	struct weather_particle_datum *particle = weather_particle_get(particle_index);

	if (type_definition->acceleration_lower_bound != 0.f || type_definition->acceleration_upper_bound != 0.f)
	{
		real_vector3d random_direction;
		real magnitude = normalize3d(&particle->acceleration);
		real one_minus_turning_rate = 1.f - type_definition->acceleration_turning_rate;

		magnitude += real_local_random_range(
			-type_definition->acceleration_magnitude_rate,
			type_definition->acceleration_magnitude_rate);
		magnitude = PIN(magnitude, type_definition->acceleration_lower_bound, type_definition->acceleration_upper_bound);
		local_random_direction3d(&random_direction);
		particle->acceleration.i = type_definition->acceleration_turning_rate*random_direction.i + one_minus_turning_rate*particle->acceleration.i;
		particle->acceleration.j = type_definition->acceleration_turning_rate*random_direction.j + one_minus_turning_rate*particle->acceleration.j;
		particle->acceleration.k = type_definition->acceleration_turning_rate*random_direction.k + one_minus_turning_rate*particle->acceleration.k;
		scale_vector3d(&particle->acceleration, magnitude, &particle->acceleration);
		vector_from_line3d(&particle->velocity, &particle->acceleration, system->time_delta_sec, &particle->velocity);
	}

	{
		real_vector3d nudge;
		unsigned long flags = FLAG(_point_physics_ignore_position_bit) | FLAG(_point_physics_force_no_collisions_bit);
		unsigned long seed = particle_index;

		SET_FLAG(flags, _point_physics_ignore_position_under_water_bit, system->under_water);
		point_physics_update(
			flags,
			point_physics_definition_get(type_definition->physics.index),
			&system->location,
			system->weather_palette_index,
			&particle->position,
			&particle->velocity,
			NULL,
			NULL,
			NULL,
			particle->radius,
			system->time_delta_sec);
		point_from_line3d(
			&particle->position,
			seed_random_direction3d(&seed, &nudge),
			WEATHER_PARTICLE_NUDGE_DISTANCE,
			&particle->position);
	}

	weather_particle_system_wrap_point(type->box_width, &particle->position, &particle->position);

	return;
}

static void weather_particle_system_wrap_point(
	real box_width,
	real_point3d const *point,
	real_point3d *result)
{
	result->x = (point->x < 0.f ? box_width : 0.f) + (real)fmod(point->x, box_width);
	result->y = (point->y < 0.f ? box_width : 0.f) + (real)fmod(point->y, box_width);
	result->z = (point->z < 0.f ? box_width : 0.f) + (real)fmod(point->z, box_width);

	return;
}

static void weather_particle_system_build_clipping_planes(
	real_plane3d *clip_planes,
	real far_clip_distance)
{
	real_point3d far_point;

	far_point.x = render.camera.forward.i*far_clip_distance + render.camera.position.x;
	far_point.y = render.camera.forward.j*far_clip_distance + render.camera.position.y;
	far_point.z = render.camera.forward.k*far_clip_distance + render.camera.position.z;
	plane3d_from_point_and_normal(&clip_planes[_clip_far], &far_point, &render.camera.forward);
	clip_planes[_clip_left] = render.frustum.world_planes[0];
	clip_planes[_clip_right] = render.frustum.world_planes[1];
	clip_planes[_clip_top] = render.frustum.world_planes[2];
	clip_planes[_clip_bottom] = render.frustum.world_planes[3];

	return;
}

static void weather_particle_system_transform_clip_planes_to_box(
	real *clip_plane_d_transforms,
	real_plane3d const *clip_planes,
	real_point3d *box_offset)
{
	short plane_index;

	for (plane_index = 0; plane_index < NUMBER_OF_CLIPPING_PLANES; plane_index++)
	{
		clip_plane_d_transforms[plane_index] = dot_product3d(&clip_planes[plane_index].n, (real_vector3d const *)box_offset);
	}

	return;
}

void weather_particle_systems_render(
	void)
{
	if (render_particle_systems_enabled)
	{
		if (weather)
		{
			if (render.local_player_index != NONE)
			{
				struct weather_particle_system_datum *system = weather_particle_system_get(render.local_player_index);
				long definition_index = NONE;

				system->location.cluster_index = render.cluster_index;
				system->location.leaf_index = render.leaf_index;
				system->under_water = scenario_location_underwater(
					&system->location,
					&render.camera.position,
					&system->weather_palette_index);

				if (system->weather_palette_index != NONE)
				{
					definition_index = TAG_BLOCK_GET_ELEMENT(
						&global_structure_bsp_get()->weather_palette,
						system->weather_palette_index,
						struct structure_weather_palette_entry)->particle_system.index;
				}

				if (system->definition_index != definition_index)
				{
					if (system->definition_index != NONE)
					{
						weather_particle_system_delete(render.local_player_index);
					}

					if (definition_index != NONE)
					{
						weather_particle_system_new(render.local_player_index, definition_index, 1.f);
					}
				}

				if (system->definition_index != NONE)
				{
					weather_particle_system_render(render.local_player_index);
				}
			}
		}
	}

	return;
}

static short weather_polyhedra_find(
	short *weather_polyhedra_indices,
	real maximum_distance)
{
	short polyhedron_index;
	struct structure_bsp const *structure_bsp = global_structure_bsp_get();
	short weather_polyhedra_count = 0;

	for (polyhedron_index = 0; polyhedron_index < structure_bsp->weather_polyhedra.count; polyhedron_index++)
	{
		struct structure_weather_polyhedron const *polyhedron = TAG_BLOCK_GET_ELEMENT(
			&structure_bsp->weather_polyhedra,
			polyhedron_index,
			struct structure_weather_polyhedron);

		if (distance_squared3d(&render.camera.position, &polyhedron->bounding_sphere_center) <
			(polyhedron->bounding_sphere_radius + maximum_distance)*(polyhedron->bounding_sphere_radius + maximum_distance))
		{
			if (weather_polyhedra_count < MAXIMUM_NUMBER_OF_VISIBLE_WEATHER_POLYHEDRA)
			{
				weather_polyhedra_indices[weather_polyhedra_count++] = polyhedron_index;
			}
			else
			{
				error(_error_silent, "too many weather polyhedra visible.");
			}
		}
	}

	return weather_polyhedra_count;
}

static void weather_particle_system_render(
	short system_index)
{
	short type_index;
	struct weather_particle_system_datum *system = weather_particle_system_get(system_index);
	struct weather_particle_system_definition const *definition = weather_particle_system_definition_get(system->definition_index);
	struct structure_bsp const *structure_bsp = global_structure_bsp_get();

	weather_particle_system_update(system_index);

	for (type_index = 0; type_index < definition->particle_types.count; type_index++)
	{
		long particle_index;
		struct weather_particle_type_datum const *type = &system->types[type_index];
		struct weather_particle_type_definition const *type_definition = TAG_BLOCK_GET_ELEMENT(
			&definition->particle_types,
			type_index,
			struct weather_particle_type_definition);
		short rendered_particle_count = 0;

		if (type->particle_count)
		{
			struct build_sprite_data build_sprite_data;
			real_point3d box_offsets[MAXIMUM_NUMBER_OF_VISIBLE_WEATHER_PARTICLE_BOXES];
			real far_clip_distance;
			real_plane3d clip_planes[NUMBER_OF_CLIPPING_PLANES];
			real clip_plane_d_transforms[MAXIMUM_NUMBER_OF_VISIBLE_WEATHER_PARTICLE_BOXES][NUMBER_OF_CLIPPING_PLANES];
			short weather_polyhedra_indices[MAXIMUM_NUMBER_OF_VISIBLE_WEATHER_POLYHEDRA];
			short weather_polyhedra_count;
			real box_deltas[WEATHER_PARTICLE_BOXES_PER_AXIS];
			short i;
			short j;
			short k;
			real_rectangle3d viewer_box;
			short box_count = 0;

			weather_polyhedra_count = weather_polyhedra_find(weather_polyhedra_indices, type->box_width);
			far_clip_distance = type->box_width;
			weather_particle_system_build_clipping_planes(clip_planes, far_clip_distance);
			weather_particle_system_box_offset_from_point3d(
				type->box_width,
				&render.camera.position,
				&box_offsets[box_count]);
			weather_particle_system_transform_clip_planes_to_box(
				clip_plane_d_transforms[box_count],
				clip_planes,
				&box_offsets[box_count]);
			box_count++;

			viewer_box.x0 = viewer_box.y0 = viewer_box.z0 = 0.f;
			viewer_box.x1 = viewer_box.y1 = viewer_box.z1 = type->box_width;
			viewer_box.x0 += box_offsets[0].x;
			viewer_box.y0 += box_offsets[0].y;
			viewer_box.z0 += box_offsets[0].z;
			viewer_box.x1 += box_offsets[0].x;
			viewer_box.y1 += box_offsets[0].y;
			viewer_box.z1 += box_offsets[0].z;

			box_deltas[0] = -type->box_width;
			box_deltas[1] = 0.f;
			box_deltas[2] = type->box_width;

			for (i = 0; i < WEATHER_PARTICLE_BOXES_PER_AXIS; i++)
			{
				for (j = 0; j < WEATHER_PARTICLE_BOXES_PER_AXIS; j++)
				{
					for (k = 0; k < WEATHER_PARTICLE_BOXES_PER_AXIS; k++)
					{
						if (i != 1 || j != 1 || k != 1)
						{
							real_vector3d box_delta;
							real_rectangle3d box;

							box_delta.i = box_deltas[i];
							box_delta.j = box_deltas[j];
							box_delta.k = box_deltas[k];
							box.x0 = viewer_box.x0 + box_delta.i;
							box.y0 = viewer_box.y0 + box_delta.j;
							box.z0 = viewer_box.z0 + box_delta.k;
							box.x1 = viewer_box.x1 + box_delta.i;
							box.y1 = viewer_box.y1 + box_delta.j;
							box.z1 = viewer_box.z1 + box_delta.k;

							if (render_frustum_cube_visible(&render.frustum, &box, TRUE))
							{
								match_assert("c:\\halo\\SOURCE\\effects\\weather_particle_systems.c", 673, box_count<MAXIMUM_NUMBER_OF_VISIBLE_WEATHER_PARTICLE_BOXES);
								box_offsets[box_count].x = box.x0;
								box_offsets[box_count].y = box.y0;
								box_offsets[box_count].z = box.z0;
								weather_particle_system_transform_clip_planes_to_box(
									clip_plane_d_transforms[box_count],
									clip_planes,
									&box_offsets[box_count]);
								box_count++;
							}
						}
					}
				}
			}

			build_sprites_begin(
				&build_sprite_data,
				type->particle_count,
				type_definition->bitmap.index,
				&type_definition->shader,
				0);
			particle_index = type->first_particle_index;

			while (particle_index != NONE)
			{
				short box_index;
				short plane_index;
				real plane_evaluations[NUMBER_OF_CLIPPING_PLANES];
				struct weather_particle_datum const *particle = weather_particle_get(particle_index);
				real_point3d const *box_offset = NULL;

				for (plane_index = 0; plane_index < NUMBER_OF_CLIPPING_PLANES; plane_index++)
				{
					plane_evaluations[plane_index] = plane3d_distance_to_point(&clip_planes[plane_index], &particle->position);
				}

				for (box_index = 0; box_index < box_count && !box_offset; box_index++)
				{
					boolean inside = TRUE;

					for (plane_index = 0; plane_index < NUMBER_OF_CLIPPING_PLANES && inside; plane_index++)
					{
						inside = plane_evaluations[plane_index] + clip_plane_d_transforms[box_index][plane_index] < 0.f;
					}

					if (inside)
					{
						box_offset = &box_offsets[box_index];
						break;
					}
				}

				if (box_offset)
				{
					real_point3d position;
					real_vector3d viewer_to_particle;
					real depth;
					real fade_out_end = MIN(type_definition->distance_fadeout_end, far_clip_distance);

					position.x = particle->position.x + box_offset->x;
					position.y = particle->position.y + box_offset->y;
					position.z = particle->position.z + box_offset->z;
					viewer_to_particle.i = position.x - render.camera.position.x;
					viewer_to_particle.j = position.y - render.camera.position.y;
					viewer_to_particle.k = position.z - render.camera.position.z;
					depth = dot_product3d(&viewer_to_particle, &render.camera.forward);

					if (depth > type_definition->distance_fadein_start && depth < fade_out_end)
					{
						short polyhedron_index;
						real fade_in = PIN(
							(depth - type_definition->distance_fadein_start)/(type_definition->distance_fadein_end - type_definition->distance_fadein_start),
							0.f,
							1.f);
						real fade_out = 1.f - PIN(
							(depth - type_definition->distance_fadeout_start)/(fade_out_end - type_definition->distance_fadeout_start),
							0.f,
							1.f);
						boolean visible = TRUE;

						for (polyhedron_index = 0; polyhedron_index < weather_polyhedra_count; polyhedron_index++)
						{
							short polyhedron_plane_index;
							struct structure_weather_polyhedron const *polyhedron = TAG_BLOCK_GET_ELEMENT(
								&structure_bsp->weather_polyhedra,
								weather_polyhedra_indices[polyhedron_index],
								struct structure_weather_polyhedron);

							for (polyhedron_plane_index = 0; polyhedron_plane_index < polyhedron->planes.count; polyhedron_plane_index++)
							{
								if (plane3d_distance_to_point(TAG_BLOCK_GET_ELEMENT(&polyhedron->planes, polyhedron_plane_index, real_plane3d), &position) < 0.f)
								{
									break;
								}
							}

							if (polyhedron_plane_index == polyhedron->planes.count)
							{
								visible = FALSE;
								break;
							}
						}

						if (visible)
						{
							real_vector3d const *direction = type_definition->direction_source == _weather_particle_direction_source_acceleration ?
								&particle->acceleration :
								&particle->velocity;

							if (type_definition->direction_render_type != _build_sprite_normal && magnitude_squared3d(direction) == 0.f)
							{
								direction = global_up3d;
							}

							build_sprite(
								&build_sprite_data,
								type_definition->direction_render_type,
								particle->sequence_index,
								(short)particle->sprite_index,
								&position,
								direction,
								particle->rotation,
								(2.f*particle->radius)*type_definition->runtime_oo_width,
								&particle->color,
								fade_in*fade_out,
								0);
						}
					}
				}

				particle_index = particle->next_particle_index;
			}

			build_sprites_end(&build_sprite_data);
		}
	}

	return;
}
