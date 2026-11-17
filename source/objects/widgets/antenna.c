/*
ANTENNA.C
*/

/* ---------- headers */

#include "cseries.h"
#include "antenna.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "game_state.h"
#include "point_physics.h"

/* ---------- constants */

enum
{
	MAXIMUM_ANTENNAS_PER_MAP = 12,
};

#define ANTENNA_RENDER_CATCH_UP_UPDATE_THRESHOLD 5
#define ANTENNA_RENDER_CATCH_UP_DT 0.05f
#define ANTENNA_RENDER_PROJECTED_PIXELS 100.f
#define ANTENNA_VERTEX_RADIUS 0.02f					/* fake name */

/* ---------- macros */

#define SECONDS_PER_TICK (1.f/TICKS_PER_SECOND) /* fake name */

#define antenna_get(index) DATUM_GET(antenna_data, index, struct antenna_datum) /* fake name */
#define antenna_try_and_get(index) ((struct antenna_datum *)datum_try_and_get(antenna_data, (index))) /* fake name */

/* ---------- structures */

struct antenna_vertex_datum
{
	real_point3d p;
	real_vector3d v;
	real sprite_scale;
	short sprite_index;
	word pad;
};

struct antenna_datum
{
	short identifier;
	word pad;
	boolean initialized;
	boolean error;
	short updates_since_last_render;
	long definition_index;
	long object_index;
	real_point3d last_attachment_location;
	struct antenna_vertex_datum vertices[MAXIMUM_NUMBER_OF_ANTENNA_VERTICES+1];
};

/* ---------- prototypes */

static void antenna_update(struct antenna_datum *antenna, struct antenna_definition const *antenna_definition, real dt);
static void antenna_update_attachment(struct antenna_datum *antenna, struct antenna_definition const *antenna_definition, struct location *attachment_location, real_point3d *attachment_point, real_vector3d *attachment_vector);
static void antenna_render_proper(struct antenna_datum *antenna, struct antenna_definition const *antenna_definition);

/* ---------- globals */

struct data_array *antenna_data;

/* ---------- public code */

void antennas_initialize(
	void)
{
	antenna_data = game_state_data_new("antenna", MAXIMUM_ANTENNAS_PER_MAP, sizeof(struct antenna_datum));

	if (!antenna_data)
	{
		error(_error_immediate, "couldn't allocate antenna globals");
	}

	return;
}

void antennas_initialize_for_new_map(
	void)
{
	data_make_valid(antenna_data);

	return;
}

void antennas_dispose_from_old_map(
	void)
{
	data_make_invalid(antenna_data);

	return;
}

void antennas_dispose(
	void)
{
	if (antenna_data)
	{
		antenna_data = NULL;
	}

	return;
}

long antenna_new(
	long antenna_definition_index)
{
	long antenna_index = NONE;

	if (antenna_definition_index != NONE)
	{
		struct antenna_definition *definition = antenna_definition_get(antenna_definition_index);

		antenna_index = datum_new(antenna_data);

		if (antenna_index != NONE)
		{
			struct antenna_datum *antenna = antenna_get(antenna_index);
			short vertex_index;
			real_point3d last_point;

			antenna->initialized = FALSE;
			antenna->error = definition->vertices.count < 2;
			antenna->definition_index = antenna_definition_index;
			antenna->object_index = NONE;
			antenna->updates_since_last_render = 0;
			antenna->last_attachment_location.z = 0.f;
			antenna->last_attachment_location.y = 0.f;
			antenna->last_attachment_location.x = 0.f;
			last_point = antenna->last_attachment_location;

			for (vertex_index = 0; vertex_index < definition->vertices.count; vertex_index++)
			{
				struct antenna_vertex_datum *vertex = &antenna->vertices[vertex_index];
				struct antenna_vertex const *vertex_definition = TAG_BLOCK_GET_ELEMENT(
					&definition->vertices,
					vertex_index,
					struct antenna_vertex);

				vertex->p = last_point;
				vertex->v.k = 0.f;
				vertex->v.j = 0.f;
				vertex->v.i = 0.f;
				vertex->sprite_index = 0;
				vertex->sprite_scale = 0.f;

				if (definition->texture.index != NONE)
				{
					struct bitmap_group *bitmap_group = bitmap_group_get(definition->texture.index);

					if (vertex_definition->sequence_index >= 0 &&
						vertex_definition->sequence_index < bitmap_group->sequences.count)
					{
						struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(
							&bitmap_group->sequences,
							vertex_definition->sequence_index,
							struct bitmap_group_sequence);

						if (sequence->sprites.count)
						{
							struct bitmap_group_sprite *sprite = TAG_BLOCK_GET_ELEMENT(
								&sequence->sprites,
								0,
								struct bitmap_group_sprite);
							struct bitmap_data *bitmap = bitmap_group_try_and_get_bitmap(
								definition->texture.index,
								sprite->bitmap_index);

							if (bitmap)
							{
								vertex->sprite_scale = vertex_definition->length_to_next /
									(bitmap->width * (sprite->bounds.x1 - sprite->bounds.x0) - bitmap_group->sprite_spacing * 2.f - 1.f);
							}
						}
					}
				}

				last_point.x += vertex_definition->vector_to_next.i;
				last_point.y += vertex_definition->vector_to_next.j;
				last_point.z += vertex_definition->vector_to_next.k;
			}

			{
				struct antenna_vertex_datum *vertex = &antenna->vertices[vertex_index];

				vertex->p = last_point;
				vertex->v.k = 0.f;
				vertex->v.j = 0.f;
				vertex->v.i = 0.f;
			}
		}
	}

	return antenna_index;
}

void antenna_delete(
	long antenna_index)
{
	datum_delete(antenna_data, antenna_index);

	return;
}

void antenna_render(
	long object_index,
	long antenna_index,
	struct render_lighting const *lighting,
	struct render_animation const *animation)
{
	struct object_datum *object = object_get(object_index);
	struct antenna_datum *antenna = antenna_get(antenna_index);
	struct antenna_definition *definition = antenna_definition_get(antenna->definition_index);

	if (!antenna->error)
	{
		antenna->object_index = object_index;

		if (antenna->updates_since_last_render > ANTENNA_RENDER_CATCH_UP_UPDATE_THRESHOLD)
		{
			antenna_update(antenna, definition, ANTENNA_RENDER_CATCH_UP_DT);
			antenna_update(antenna, definition, ANTENNA_RENDER_CATCH_UP_DT);
			antenna_update(antenna, definition, ANTENNA_RENDER_CATCH_UP_DT);
		}

		antenna->updates_since_last_render = 0;
		antenna_render_proper(antenna, definition);
	}

	return;
}

void antennas_update(
	real dt)
{
	long antenna_index;

	for (antenna_index = data_next_index(antenna_data, NONE);
		antenna_index != NONE;
		antenna_index = data_next_index(antenna_data, antenna_index))
	{
		struct antenna_datum *antenna = antenna_get(antenna_index);
		struct antenna_definition *definition = antenna_definition_get(antenna->definition_index);

		if (!antenna->error)
		{
			antenna->updates_since_last_render++;

			if (antenna->object_index != NONE &&
				antenna->updates_since_last_render < ANTENNA_RENDER_CATCH_UP_UPDATE_THRESHOLD)
			{
				real const maximum_update_dt = 2 * SECONDS_PER_TICK;

				antenna_update(antenna, definition, MIN(dt, maximum_update_dt));
			}
		}
	}

	return;
}

/* ---------- private code */

static void antenna_update(
	struct antenna_datum *antenna,
	struct antenna_definition const *antenna_definition,
	real dt)
{
	struct location attachment_location;
	real_point3d attachment_point;
	real_vector3d attachment_vector;
	real_point3d next_desired_point;
	real_point3d last_point;
	real_vector3d last_vector;
	short vertex_index;
	struct antenna_vertex_datum *vertex;
	struct antenna_vertex const *vertex_definition;
	real_point3d new_position;
	real spring;
	real one_over_dt;
	real_vector3d to_new;
	real magnitude;
	real scale;
	real_point3d p_parent;
	real_vector3d up;
	real_vector3d rotation_axis;

	antenna_update_attachment(
		antenna,
		antenna_definition,
		&attachment_location,
		&attachment_point,
		&attachment_vector);

	if (!antenna->error && dt > 0.f)
	{
		for (vertex_index = 0; vertex_index < antenna_definition->vertices.count + 1; vertex_index++)
		{
			vertex = &antenna->vertices[vertex_index];
			vertex_definition = TAG_BLOCK_GET_ELEMENT(
				&antenna_definition->vertices,
				vertex_index == antenna_definition->vertices.count ? antenna_definition->vertices.count - 1 : vertex_index,
				struct antenna_vertex);
			spring = antenna_definition->spring_coefficient * vertex_definition->spring_coefficient;
			one_over_dt = 1.f / dt;
			vertex->sprite_index++;

			if (vertex_index == 0)
			{
				new_position = attachment_point;
				last_vector = attachment_vector;
			}
			else
			{
				new_position = vertex->p;
				point_physics_update(
					0,
					point_physics_definition_get(antenna_definition->physics.index),
					&attachment_location,
					NONE,
					&new_position,
					&vertex->v,
					NULL,
					NULL,
					NULL,
					ANTENNA_VERTEX_RADIUS,
					dt);
				to_new.i = new_position.x - last_point.x;
				to_new.j = new_position.y - last_point.y;
				to_new.k = new_position.z - last_point.z;
				magnitude = magnitude3d(&to_new);
				scale = vertex_definition->length_to_next / magnitude;
				p_parent.x = scale * to_new.i + last_point.x;
				p_parent.y = scale * to_new.j + last_point.y;
				p_parent.z = scale * to_new.k + last_point.z;
				new_position.x = (1.f - spring) * p_parent.x + spring * next_desired_point.x;
				new_position.y = (1.f - spring) * p_parent.y + spring * next_desired_point.y;
				new_position.z = (1.f - spring) * p_parent.z + spring * next_desired_point.z;
				last_vector.i = new_position.x - last_point.x;
				last_vector.j = new_position.y - last_point.y;
				last_vector.k = new_position.z - last_point.z;
			}

			up.i = 0.f;
			up.j = 0.f;
			up.k = 1.f;
			cross_product3d(&up, &last_vector, &rotation_axis);

			if (normalize3d(&rotation_axis) == 0.f)
			{
				rotation_axis = *global_left3d;
			}

			{
				real_vector3d rotated_vector_to_next = vertex_definition->vector_to_next;
				real angle = angle_between_vectors3d(&up, &last_vector);

				rotate_vector_about_axis(&rotated_vector_to_next, &rotation_axis, sine(angle), cosine(angle));
				next_desired_point.x = new_position.x + rotated_vector_to_next.i;
				next_desired_point.y = new_position.y + rotated_vector_to_next.j;
				next_desired_point.z = new_position.z + rotated_vector_to_next.k;
			}

			last_point = new_position;
			vertex->v.i = (new_position.x - vertex->p.x) * one_over_dt;
			vertex->v.j = (new_position.y - vertex->p.y) * one_over_dt;
			vertex->v.k = (new_position.z - vertex->p.z) * one_over_dt;
			vertex->p = new_position;
		}
	}

	return;
}

static void antenna_update_attachment(
	struct antenna_datum *antenna,
	struct antenna_definition const *antenna_definition,
	struct location *attachment_location,
	real_point3d *attachment_point,
	real_vector3d *attachment_vector)
{
	real const maximum_displacement = 1.f;
	struct object_marker marker;
	real_vector3d attachment_displacement;
	short vertex_index;

	object_get_marker_by_name(antenna->object_index, antenna_definition->attachment_marker, &marker, 1);
	*attachment_point = marker.matrix.position;
	*attachment_vector = marker.matrix.forward;
	scenario_location_from_point(attachment_location, &marker.matrix.position);
	attachment_displacement.i = attachment_point->x - antenna->last_attachment_location.x;
	attachment_displacement.j = attachment_point->y - antenna->last_attachment_location.y;
	attachment_displacement.k = attachment_point->z - antenna->last_attachment_location.z;

	if (abs(attachment_displacement.i) > maximum_displacement ||
		abs(attachment_displacement.j) > maximum_displacement ||
		abs(attachment_displacement.k) > maximum_displacement)
	{
		for (vertex_index = 0; vertex_index < antenna_definition->vertices.count + 1; vertex_index++)
		{
			real_point3d *vertex_position = &antenna->vertices[vertex_index].p;

			vertex_position->x += attachment_displacement.i;
			vertex_position->y += attachment_displacement.j;
			vertex_position->z += attachment_displacement.k;
		}
	}

	antenna->last_attachment_location = *attachment_point;

	return;
}

static void antenna_render_proper(
	struct antenna_datum *antenna,
	struct antenna_definition const *antenna_definition)
{
	if (antenna_definition->vertices.count)
	{
		struct build_sprite_data build_sprite_data;
		short vertex_index;
		real projected_pixels = ANTENNA_RENDER_PROJECTED_PIXELS;
		real fade = (projected_pixels - antenna_definition->cutoff_pixels) /
			(antenna_definition->falloff_pixels - antenna_definition->cutoff_pixels);

		fade = PIN(fade, 0.f, 1.f);
		build_sprites_begin(
			&build_sprite_data,
			(short)antenna_definition->vertices.count,
			antenna_definition->texture.index,
			&global_shader_effect_alpha_blended,
			0);

		for (vertex_index = 0; vertex_index < antenna_definition->vertices.count; vertex_index++)
		{
			struct antenna_vertex_datum *vertex = &antenna->vertices[vertex_index];
			struct antenna_vertex const *vertex_definition = TAG_BLOCK_GET_ELEMENT(
				&antenna_definition->vertices,
				vertex_index,
				struct antenna_vertex);
			struct antenna_vertex_datum *next_vertex = &antenna->vertices[vertex_index + 1];
			real_vector3d difference;
			real_argb_color color;

			difference.i = next_vertex->p.x - vertex->p.x;
			difference.j = next_vertex->p.y - vertex->p.y;
			difference.k = next_vertex->p.z - vertex->p.z;
			color = vertex_definition->color;

			if (vertex->sprite_scale != 0.f && fade > 0.f)
			{
				build_sprite(
					&build_sprite_data,
					_build_sprite_parallel,
					vertex_definition->sequence_index,
					0,
					&vertex->p,
					&difference,
					0.f,
					vertex->sprite_scale,
					&color,
					fade,
					0);
			}
		}

		build_sprites_end(&build_sprite_data);
	}

	return;
}
