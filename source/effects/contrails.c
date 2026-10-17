/*
CONTRAILS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "contrails.h"
#include "contrail_definitions.h"
#include "objects.h"
#include "game_state.h"
#include "physics.h"
#include "point_physics.h"
#include "scenario.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void contrail_next_frame(struct contrail_datum *contrail);
static short contrail_compute_new_point_count(long contrail_index, real dt);
static void contrail_add_points(long contrail_index, short point_count, boolean force);
static void contrail_update_points(long contrail_index, real dt);
static void contrail_verify(long contrail_index);

/* ---------- globals */

struct data_array *contrail_data;
struct data_array *contrail_point_data;

/* ---------- public code */

void contrails_initialize(
	void)
{
	contrail_data = game_state_data_new("contrail", MAXIMUM_CONTRAILS_PER_MAP, sizeof(struct contrail_datum));
	contrail_point_data = game_state_data_new("contrail point", MAXIMUM_CONTRAIL_POINTS_PER_MAP, sizeof(struct contrail_point_datum));

	if (!contrail_data || !contrail_point_data)
	{
		contrails_dispose();
		error(_error_immediate, "couldn't allocate contrail globals");
	}

	return;
}

void contrails_initialize_for_new_map(
	void)
{
	data_make_valid(contrail_data);
	data_make_valid(contrail_point_data);

	return;
}

void contrails_dispose_from_old_map(
	void)
{
	data_make_invalid(contrail_point_data);
	data_make_invalid(contrail_data);

	return;
}

void contrails_dispose(
	void)
{
	if (contrail_point_data)
	{
		contrail_point_data = NULL;
	}

	if (contrail_data)
	{
		contrail_data = NULL;
	}

	return;
}

long contrail_new(
	long definition_index,
	long object_index,
	short attachment_index)
{
	long contrail_index = NONE;

	match_assert("c:\\halo\\SOURCE\\effects\\contrails.c", 97, object_index!=NONE);
	match_assert("c:\\halo\\SOURCE\\effects\\contrails.c", 98, definition_index!=NONE);

	if (definition_index != NONE)
	{
		struct contrail_definition const *definition = contrail_definition_get(definition_index);

		contrail_index = datum_new(contrail_data);

		if (contrail_index != NONE)
		{
			short instance_index;
			struct contrail_datum *contrail = contrail_get(contrail_index);
			struct object_datum const *object = object_get(object_index);

			contrail->flags = 0;
			contrail->definition_index = definition_index;
			contrail->object_index = object_index;
			contrail->attachment_index = attachment_index;
			contrail->density_function_index = TAG_BLOCK_GET_ELEMENT(
				&object_definition_get(object->definition_index)->object.attachments,
				attachment_index,
				struct object_attachment_definition)->primary_scale_function_reference - 1;
			contrail->sequence_index = NONE;
			contrail_next_frame(contrail);
			contrail->texture_offset_u = 0.f;
			contrail->texture_offset_v = 0.f;

			for (instance_index = 0; instance_index < MAXIMUM_INSTANCES_PER_CONTRAIL; instance_index++)
			{
				contrail->contrail_point_counts[instance_index] = 0;
				contrail->first_contrail_point_indices[instance_index] = NONE;
			}

			if (object_get_function_value(contrail->object_index, contrail->density_function_index, &contrail->density))
			{
				SET_FLAG(contrail->flags, _contrail_active_bit, TRUE);
				contrail_add_points(contrail_index, 1, TRUE);
			}
		}
	}

	return contrail_index;
}

void contrail_owner_collision(
	long contrail_index,
	boolean object_dying,
	real dt)
{
	struct contrail_datum *contrail = contrail_get(contrail_index);
	struct contrail_definition const *definition = contrail_definition_get(contrail->definition_index);

	if (TEST_FLAG(contrail->flags, _contrail_active_bit))
	{
		short point_count = contrail_compute_new_point_count(contrail_index, dt);

		contrail_add_points(contrail_index, point_count < 1 ? 1 : point_count, FALSE);
	}

	if (object_dying)
	{
		contrail->object_index = NONE;
	}

	contrail->expired_dt += dt;

	return;
}

void contrail_delete(
	long contrail_index)
{
	short instance_index;
	struct contrail_datum const *contrail = contrail_get(contrail_index);

	for (instance_index = 0; instance_index < MAXIMUM_INSTANCES_PER_CONTRAIL; instance_index++)
	{
		long contrail_point_index = contrail->first_contrail_point_indices[instance_index];

		while (contrail_point_index != NONE)
		{
			long next_contrail_point_index = contrail_point_get(contrail_point_index)->next_contrail_point_index;

			datum_delete(contrail_point_data, contrail_point_index);
			contrail_point_index = next_contrail_point_index;
		}
	}

	datum_delete(contrail_data, contrail_index);

	return;
}

void contrails_update(
	real dt)
{
	long contrail_index;

	for (contrail_index = data_next_index(contrail_data, NONE); contrail_index != NONE; contrail_index = data_next_index(contrail_data, contrail_index))
	{
		real remaining_dt;
		real frame_period;
		short instance_index;
		struct contrail_datum *contrail = contrail_get(contrail_index);
		struct contrail_definition const *definition = contrail_definition_get(contrail->definition_index);
		real expired_dt = dt - contrail->expired_dt;

		contrail_verify(contrail_index);
		contrail->expired_dt = 0.f;

		if (contrail->object_index != NONE)
		{
			boolean active = object_get_function_value(contrail->object_index, contrail->density_function_index, &contrail->density);

			if (active != TEST_FLAG(contrail->flags, _contrail_active_bit))
			{
				real density = contrail->density;

				contrail->density = 0.f;
				contrail_add_points(contrail_index, 1, TRUE);
				contrail->density = density;
			}

			SET_FLAG(contrail->flags, _contrail_active_bit, active);

			if (active)
			{
				contrail_add_points(contrail_index, contrail_compute_new_point_count(contrail_index, dt), TRUE);
			}
		}

		remaining_dt = expired_dt;
		frame_period = 1.f / contrail_scale_value(
			contrail->density,
			definition->frames_per_second,
			definition->scale_flags,
			_contrail_animation_rate_scales_bit);

		while (remaining_dt > 0.f)
		{
			real frame_dt = frame_period - contrail->frame_time;

			if (frame_dt <= remaining_dt)
			{
				contrail_next_frame(contrail);
				remaining_dt -= frame_dt;
			}
			else
			{
				contrail->frame_time += remaining_dt;
				remaining_dt = 0.f;
			}
		}

		contrail->texture_offset_u -= contrail_scale_value(
			contrail->density,
			definition->texture_animation_u,
			definition->scale_flags,
			_contrail_texture_animation_u_bit) * expired_dt;
		contrail->texture_offset_v += contrail_scale_value(
			contrail->density,
			definition->texture_animation_v,
			definition->scale_flags,
			_contrail_texture_animation_v_bit) * expired_dt;
		contrail_update_points(contrail_index, dt);

		for (instance_index = 0; instance_index < MAXIMUM_INSTANCES_PER_CONTRAIL; instance_index++)
		{
			match_assert("c:\\halo\\SOURCE\\effects\\contrails.c", 266, contrail->contrail_point_counts[instance_index]>0==contrail->first_contrail_point_indices[instance_index]!=NONE);

			if (contrail->first_contrail_point_indices[instance_index] != NONE)
			{
				break;
			}
		}

		contrail_verify(contrail_index);

		if (instance_index == MAXIMUM_INSTANCES_PER_CONTRAIL && contrail->object_index == NONE)
		{
			contrail_delete(contrail_index);
		}
	}

	return;
}

void contrails_disconnect_from_structure_bsp(
	void)
{
	return;
}

void contrails_reconnect_to_structure_bsp(
	void)
{
	long contrail_index;

	for (contrail_index = data_next_index(contrail_data, NONE); contrail_index != NONE; contrail_index = data_next_index(contrail_data, contrail_index))
	{
		short instance_index;
		struct contrail_datum const *contrail = contrail_get(contrail_index);
		struct contrail_definition const *definition = contrail_definition_get(contrail->definition_index);

		for (instance_index = 0; instance_index < MAXIMUM_INSTANCES_PER_CONTRAIL; instance_index++)
		{
			struct contrail_point_datum *point;
			long contrail_point_index;

			for (contrail_point_index = contrail->first_contrail_point_indices[instance_index]; contrail_point_index != NONE; contrail_point_index = point->next_contrail_point_index)
			{
				point = contrail_point_get(contrail_point_index);

				if (point->location.cluster_index != NONE)
				{
					scenario_location_from_point(&point->location, &point->position);
				}
			}
		}
	}

	return;
}

/* ---------- private code */

static void contrail_next_frame(
	struct contrail_datum *contrail)
{
	struct contrail_definition const *definition = contrail_definition_get(contrail->definition_index);
	struct bitmap_group const *bitmap = bitmap_group_get(definition->bitmap.index);

	contrail->frame_time = 0.f;
	contrail->frame_index++;

	if (contrail->sequence_index < 0 ||
		contrail->sequence_index >= bitmap->sequences.count ||
		contrail->frame_index < 0 ||
		contrail->frame_index >= TAG_BLOCK_GET_ELEMENT(&bitmap->sequences, contrail->sequence_index, struct bitmap_group_sequence)->bitmap_count)
	{
		contrail->sequence_index = local_random_range(
			definition->first_sequence_index,
			definition->first_sequence_index + definition->sequence_count);
		contrail->frame_index = 0;
	}

	return;
}

static short contrail_compute_new_point_count(
	long contrail_index,
	real dt)
{
	struct contrail_datum *contrail = contrail_get(contrail_index);
	struct contrail_definition const *definition = contrail_definition_get(contrail->definition_index);
	short point_count = 0;
	real point_period = 1.f / contrail_scale_value(
		contrail->density,
		definition->point_generation_rate,
		definition->scale_flags,
		_contrail_point_generation_rate_bit);

	while (dt != 0.f)
	{
		if (dt >= contrail->time_until_point)
		{
			dt -= contrail->time_until_point;
			contrail->time_until_point = point_period;
			point_count++;
		}
		else
		{
			contrail->time_until_point -= dt;
			dt = 0.f;
		}
	}

	return point_count;
}

static void contrail_add_points(
	long contrail_index,
	short point_count,
	boolean force)
{
	struct contrail_datum *contrail = contrail_get(contrail_index);
	struct contrail_definition const *definition = contrail_definition_get(contrail->definition_index);

	if (point_count)
	{
		struct object_marker markers[MAXIMUM_INSTANCES_PER_CONTRAIL];
		struct object_datum const *object = object_get(contrail->object_index);
		short marker_count = object_get_marker_by_name(
			contrail->object_index,
			TAG_BLOCK_GET_ELEMENT(
				&object_definition_get(object->definition_index)->object.attachments,
				contrail->attachment_index,
				struct object_attachment_definition)->marker_name,
			markers,
			MAXIMUM_INSTANCES_PER_CONTRAIL);

		if (marker_count > 0)
		{
			short instance_index;
			real velocity = contrail_scale_random_value(
				contrail->density,
				definition->point_velocity_lower_bound,
				definition->point_velocity_upper_bound,
				definition->scale_flags,
				_contrail_point_velocity_bit);
			real cone_angle = contrail_scale_value(
				contrail->density,
				definition->point_velocity_cone_angle,
				definition->scale_flags,
				_contrail_point_velocity_cone_angle_bit);
			real inherited_velocity_fraction = contrail_scale_value(
				contrail->density,
				definition->point_inherited_velocity_fraction,
				definition->scale_flags,
				_contrail_point_velocity_fraction_bit);

			for (instance_index = 0; instance_index < marker_count; instance_index++)
			{
				struct contrail_point_datum const *previous_point;
				short new_point_count;
				struct object_marker const *marker = &markers[instance_index];

				if (contrail->first_contrail_point_indices[instance_index] != NONE)
				{
					previous_point = contrail_point_get(contrail->first_contrail_point_indices[instance_index]);
					new_point_count = point_count;
				}
				else
				{
					previous_point = NULL;
					new_point_count = 1;
				}

				if (!previous_point || memcmp(&marker->matrix.position, &previous_point->position, sizeof(real_point3d)) || force)
				{
					short point_index;

					for (point_index = 1; point_index <= new_point_count; point_index++)
					{
						long contrail_point_index = datum_new(contrail_point_data);

						if (contrail_point_index != NONE)
						{
							real_vector3d direction;
							real_vector3d object_velocity;
							struct contrail_point_datum *point = contrail_point_get(contrail_point_index);

							point->time = 0.f;
							point->delta = 0.f;
							point->flags = FLAG(_contrail_point_new_bit) | FLAG(_contrail_point_transitioning_bit);
							point->state_index = NONE;
							point->density = contrail->density;
							local_random_vector_in_cone3d(&marker->matrix.forward, 0.f, cone_angle, &direction);
							point->position = marker->matrix.position;
							scenario_location_from_point(&point->location, &point->position);
							object_get_velocities(contrail->object_index, &object_velocity, NULL);
							point->velocity.i = velocity * direction.i + inherited_velocity_fraction * object_velocity.i;
							point->velocity.j = velocity * direction.j + inherited_velocity_fraction * object_velocity.j;
							point->velocity.k = velocity * direction.k + inherited_velocity_fraction * object_velocity.k;

							if (point_index < new_point_count)
							{
								real_point3d interpolated_position;
								real t = (real)point_index / (real)new_point_count;
								real one_minus_t = 1.f - t;

								match_assert_valid_real("c:\\halo\\SOURCE\\effects\\contrails.c", 471, t);
								point->density = t * point->density + one_minus_t * previous_point->density;
								interpolated_position.x = t * point->position.x + one_minus_t * previous_point->position.x;
								interpolated_position.y = t * point->position.y + one_minus_t * previous_point->position.y;
								interpolated_position.z = t * point->position.z + one_minus_t * previous_point->position.z;
								scenario_location_from_line(
									&point->location,
									&point->location,
									&point->position,
									&interpolated_position);
								point->position = interpolated_position;
								point->velocity.i = t * point->velocity.i + one_minus_t * previous_point->velocity.i;
								point->velocity.j = t * point->velocity.j + one_minus_t * previous_point->velocity.j;
								point->velocity.k = t * point->velocity.k + one_minus_t * previous_point->velocity.k;
							}

							point->next_contrail_point_index = contrail->first_contrail_point_indices[instance_index];
							contrail->contrail_point_counts[instance_index]++;
							contrail->first_contrail_point_indices[instance_index] = contrail_point_index;
						}
					}
				}
			}
		}
	}

	return;
}

static void contrail_update_points(
	long contrail_index,
	real dt)
{
	short instance_index;
	struct contrail_datum *contrail = contrail_get(contrail_index);
	struct contrail_definition const *definition = contrail_definition_get(contrail->definition_index);

	for (instance_index = 0; instance_index < MAXIMUM_INSTANCES_PER_CONTRAIL; instance_index++)
	{
		long point_indices[MAXIMUM_CONTRAIL_POINTS_PER_MAP];
		struct contrail_point_datum *point;
		long contrail_point_index;
		short point_count = 0;

		for (contrail_point_index = contrail->first_contrail_point_indices[instance_index]; contrail_point_index != NONE; contrail_point_index = point->next_contrail_point_index)
		{
			point = contrail_point_get(contrail_point_index);

			if (!TEST_FLAG(point->flags, _contrail_point_living_dead_bit))
			{
				point->time += point->delta * dt;

				while (point->delta == 0.f || point->time > 1.f)
				{
					if (TEST_FLAG(point->flags, _contrail_point_transitioning_bit))
					{
						struct contrail_point_state const *state = TAG_BLOCK_GET_ELEMENT(&definition->states, point->state_index + 1, struct contrail_point_state);

						point->state_index++;
						point->time = 0.f;
						point->delta = contrail_scale_random_value(
							point->density,
							state->duration_lower_bound,
							state->duration_upper_bound,
							state->scale_flags,
							_contrail_state_duration_bit);

						if (point->delta != 0.f)
						{
							point->delta = 1.f / point->delta;
						}

						SET_FLAG(point->flags, _contrail_point_transitioning_bit, FALSE);
					}
					else
					{
						struct contrail_point_state const *state;

						if (point->state_index + 1 >= definition->states.count)
						{
							SET_FLAG(point->flags, _contrail_point_living_dead_bit, TRUE);
							break;
						}

						state = TAG_BLOCK_GET_ELEMENT(&definition->states, point->state_index, struct contrail_point_state);
						point->time = 0.f;
						point->delta = contrail_scale_random_value(
							point->density,
							state->transition_duration_lower_bound,
							state->transition_duration_upper_bound,
							state->scale_flags,
							_contrail_state_transition_duration_bit);

						if (point->delta != 0.f)
						{
							point->delta = 1.f / point->delta;
						}

						SET_FLAG(point->flags, _contrail_point_transitioning_bit, TRUE);
					}
				}
			}

			if (TEST_FLAG(point->flags, _contrail_point_new_bit))
			{
				SET_FLAG(point->flags, _contrail_point_new_bit, FALSE);
			}
			else if (!TEST_FLAG(point->flags, _contrail_point_living_dead_bit))
			{
				struct contrail_point_state const *state = TAG_BLOCK_GET_ELEMENT(&definition->states, point->state_index, struct contrail_point_state);

				if (state->physics.index != NONE)
				{
					point_physics_update(
						0,
						point_physics_definition_get(state->physics.index),
						&point->location,
						NONE,
						&point->position,
						&point->velocity,
						NULL,
						NULL,
						NULL,
						0.5f * state->width,
						dt);
				}
			}

			point_indices[point_count++] = contrail_point_index;
		}

		while (point_count-- > 1)
		{
			struct contrail_point_datum const *last_point = contrail_point_get(point_indices[point_count]);
			struct contrail_point_datum *previous_point = contrail_point_get(point_indices[point_count - 1]);

			if (!TEST_FLAG(last_point->flags, _contrail_point_living_dead_bit) ||
				!TEST_FLAG(previous_point->flags, _contrail_point_living_dead_bit) ||
				last_point->next_contrail_point_index != NONE)
			{
				break;
			}

			previous_point->next_contrail_point_index = NONE;
			contrail->contrail_point_counts[instance_index]--;
			datum_delete(contrail_point_data, point_indices[point_count]);
		}

		if (contrail->contrail_point_counts[instance_index] == 1 &&
			TEST_FLAG(contrail_point_get(contrail->first_contrail_point_indices[instance_index])->flags, _contrail_point_living_dead_bit))
		{
			datum_delete(contrail_point_data, contrail->first_contrail_point_indices[instance_index]);
			contrail->first_contrail_point_indices[instance_index] = NONE;
			contrail->contrail_point_counts[instance_index] = 0;
		}
	}

	return;
}

static void contrail_verify(
	long contrail_index)
{
	short instance_index;
	struct contrail_datum const *contrail = contrail_get(contrail_index);
	struct contrail_definition const *definition = contrail_definition_get(contrail->definition_index);

	if (contrail->object_index != NONE)
	{
		struct object_datum const *object = object_get(contrail->object_index);

		match_vassert(
			"c:\\halo\\SOURCE\\effects\\contrails.c",
			654,
			contrail->attachment_index >= 0 && contrail->attachment_index < object_definition_get(object->definition_index)->object.attachments.count,
			csprintf(
				temporary,
				"contrail %s attachment index %d is outside the valid range.",
				tag_get_name(contrail->definition_index),
				contrail->attachment_index));
		match_vassert(
			"c:\\halo\\SOURCE\\effects\\contrails.c",
			655,
			object->object.attachment_indices[contrail->attachment_index] == contrail_index,
			csprintf(
				temporary,
				"contrail %s (%ld) has an object that thinks it's attached to %ld",
				tag_get_name(contrail->definition_index),
				contrail_index,
				object->object.attachment_indices[contrail->attachment_index]));
	}

	for (instance_index = 0; instance_index < MAXIMUM_INSTANCES_PER_CONTRAIL; instance_index++)
	{
		long contrail_point_index = contrail->first_contrail_point_indices[instance_index];
		short point_count = 0;

		while (contrail_point_index != NONE)
		{
			contrail_point_index = contrail_point_get(contrail_point_index)->next_contrail_point_index;
			point_count++;
		}

		match_vassert(
			"c:\\halo\\SOURCE\\effects\\contrails.c",
			677,
			point_count == contrail->contrail_point_counts[instance_index],
			csprintf(
				temporary,
				"contrail %s thinks it has %d points but really it has %d",
				tag_get_name(contrail->definition_index),
				contrail->contrail_point_counts[instance_index],
				point_count));
	}

	return;
}
