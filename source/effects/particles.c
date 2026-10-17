/*
PARTICLES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "particles.h"
#include "effects.h"
#include "material_effects.h"
#include "particle_definitions.h"
#include "material_effect_definitions.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "unit_definitions.h"
#include "network_game_globals.h"
#include "players.h"
#include "game_state.h"
#include "sound_manager.h"
#include "game_sound.h"
#include "sound_definitions.h"
#include "structures.h"
#include "physics_constants.h"
#include "object_lights.h"
#include "physics.h"
#include "first_person_weapons.h"
#include "point_physics.h"

/* ---------- constants */

enum
{
	_effect_marker_velocity = 0,
	_effect_marker_gravity,
	NUMBER_OF_PARTICLE_EFFECT_MARKERS
};

enum
{
	MAXIMUM_PARTICLE_UNRENDERED_FRAMES = 15,	/* fake name */
};

#define PARTICLE_MINIMUM_LIFE_SPAN 0.7f				/* fake name */
#define PARTICLE_AT_REST_VELOCITY_SQUARED 0.0625f	/* fake name */
#define PARTICLE_SETTLE_NORMAL_K 0.8f				/* fake name */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean new_particle_is_visible(struct particle_definition const *definition, struct location const *location);
static boolean particle_next_sequence(long particle_index);
static boolean particle_next_frame(long particle_index);
static boolean particle_update_frame_time(long particle_index, real dt);
static boolean particle_update_physics(long particle_index, real dt);
static void particle_die(long particle_index);
static void particle_effect_new(struct particle_datum const *particle, unsigned long group_tag, long effect_index, real scale);

/* ---------- globals */

static struct profile_section particles_update_section =
{
	"particles_update",
	NONE,
	TRUE
};

static char const *particle_effect_marker_names[NUMBER_OF_PARTICLE_EFFECT_MARKERS] =
{
	"velocity",
	"gravity"
};

static real const maximum_particle_velocity = 1.5f;
static real const minimum_particle_velocity = 0.5f;

struct data_array *particle_data;

/* ---------- public code */

void particles_initialize(
	void)
{
	particle_data = game_state_data_new("particle", MAXIMUM_PARTICLES_PER_MAP, sizeof(struct particle_datum));

	if (!particle_data)
	{
		error(_error_immediate, "couldn't allocate particle globals");
	}

	return;
}

void particles_initialize_for_new_map(
	void)
{
	data_make_valid(particle_data);

	return;
}

void particles_dispose_from_old_map(
	void)
{
	data_make_invalid(particle_data);

	return;
}

void particles_dispose(
	void)
{
	if (particle_data)
	{
		particle_data = NULL;
	}

	return;
}

void particle_new(
	struct new_particle_data const *data)
{
	long particle_index = NONE;

	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\effects\\particles.c", 109, &data->velocity);
	match_assert_valid_real_point3d("c:\\halo\\SOURCE\\effects\\particles.c", 110, &data->position);
	match_assert_valid_real_argb_color("c:\\halo\\SOURCE\\effects\\particles.c", 111, &data->color);

	if (data->definition_index != NONE)
	{
		real_point3d world_position;
		struct location location;
		struct particle_definition const *definition = particle_definition_get(data->definition_index);

		if (data->object_index != NONE)
		{
			if (data->attached_to_local_player)
			{
				matrix4x3_transform_point(
					first_person_weapon_get_node_matrix(data->local_player_index, data->node_index),
					&data->position,
					&world_position);
			}
			else
			{
				matrix4x3_transform_point(
					object_get_node_matrix(data->object_index, data->node_index),
					&data->position,
					&world_position);
			}
		}
		else
		{
			world_position = data->position;
		}

		scenario_location_from_point(&location, &world_position);

		if (location.leaf_index != NONE && new_particle_is_visible(definition, &location))
		{
			particle_index = datum_new(particle_data);

			if (particle_index != NONE)
			{
				struct particle_datum *particle = particle_get(particle_index);

				particle->flags = 0;

				if (TEST_FLAG(definition->flags, _particle_can_animate_backwards_bit))
				{
					particle->flags |= local_random() & FLAG(_particle_animates_backwards_bit);
				}

				if (TEST_FLAG(definition->flags, _particle_random_u_mirror_bit))
				{
					particle->flags |= local_random() & FLAG(_particle_u_mirror_bit);
				}

				if (TEST_FLAG(definition->flags, _particle_random_v_mirror_bit))
				{
					particle->flags |= local_random() & FLAG(_particle_v_mirror_bit);
				}

				SET_FLAG(particle->flags, _particle_dont_draw_first_person_bit, data->dont_draw_first_person);
				SET_FLAG(particle->flags, _particle_dont_draw_third_person_bit, data->dont_draw_third_person);
				SET_FLAG(particle->flags, _particle_attached_to_local_player_bit, data->attached_to_local_player);
				particle->definition_index = data->definition_index;
				particle->local_player_index = (byte)data->local_player_index;
				particle->object_index = data->object_index;
				particle->node_index = data->node_index;
				particle->state = _particle_state_next_sequence_initial;
				particle->last_rendered_frame_index = render.frame_index;
				particle->life_span = real_local_random_range(
					definition->life_span_lower_bound,
					definition->life_span_upper_bound);

				if (particle->life_span > PARTICLE_MINIMUM_LIFE_SPAN)
				{
					particle->life_span = (particle->life_span - PARTICLE_MINIMUM_LIFE_SPAN) / local_player_count() + PARTICLE_MINIMUM_LIFE_SPAN;
				}

				particle->frame_span = definition->frames_per_second_upper_bound != 0.f ?
					1.f / real_local_random_range(definition->frames_per_second_lower_bound, definition->frames_per_second_upper_bound) :
					REAL_MAX;
				particle->frame_time = -1.f;
				particle->location = location;
				particle->position = data->position;
				particle->direction = data->direction;
				particle->rotation = data->rotation;
				particle->radius = data->radius;
				particle->color = data->color;
				particle->translational_velocity = data->velocity;

				if (particle->object_index == NONE)
				{
					real mass = point_physics_definition_get_mass(
						point_physics_definition_get(definition->physics.index),
						particle_get_radius(particle_index));

					particle->translational_velocity.i = data->initial_impulse.i * mass + particle->translational_velocity.i;
					particle->translational_velocity.j = data->initial_impulse.j * mass + particle->translational_velocity.j;
					particle->translational_velocity.k = data->initial_impulse.k * mass + particle->translational_velocity.k;
				}

				particle->angular_velocity = data->angular_velocity;

				if (!TEST_FLAG(definition->flags, _particle_self_illuminated_bit) || TEST_FLAG(definition->flags, _particle_tint_with_diffuse_texture_bit))
				{
					real_rgb_color light;
					real_rgb_color diffuse;

					light_particle(&world_position, &light, &diffuse, FALSE);
					match_assert_valid_real_rgb_color("c:\\halo\\SOURCE\\effects\\particles.c", 207, &light);
					match_assert_valid_real_rgb_color("c:\\halo\\SOURCE\\effects\\particles.c", 208, &diffuse);

					if (!TEST_FLAG(definition->flags, _particle_self_illuminated_bit))
					{
						particle->color.red *= light.red;
						particle->color.green *= light.green;
						particle->color.blue *= light.blue;
					}

					if (TEST_FLAG(definition->flags, _particle_tint_with_diffuse_texture_bit))
					{
						particle->color.red *= diffuse.red;
						particle->color.green *= diffuse.green;
						particle->color.blue *= diffuse.blue;
					}
				}

				if (particle_next_sequence(particle_index))
				{
					struct bitmap_group const *bitmap = bitmap_group_get(definition->bitmap.index);
					struct bitmap_group_sequence const *sequence = TAG_BLOCK_GET_ELEMENT(&bitmap->sequences, particle->sequence_index, struct bitmap_group_sequence);

					if (TEST_FLAG(definition->flags, _particle_animation_starts_on_random_frame_bit))
					{
						particle->frame_index = (TEST_FLAG(particle->flags, _particle_animates_backwards_bit) ? 1 : NONE) + local_random_range(0, (short)sequence->sprites.count);
					}
					else if (TEST_FLAG(particle->flags, _particle_animates_backwards_bit))
					{
						particle->frame_index = (short)sequence->sprites.count;
					}
					else
					{
						particle->frame_index = NONE;
					}
				}
			}
		}
	}

	return;
}

void particle_delete(
	long particle_index)
{
	datum_delete(particle_data, particle_index);

	return;
}

void particles_update(
	real dt)
{
	long particle_index;

	profile_enter(particles_update_section);

	for (particle_index = data_next_index(particle_data, NONE); particle_index != NONE; particle_index = data_next_index(particle_data, particle_index))
	{
		struct particle_datum *particle = particle_get(particle_index);
		struct particle_definition const *definition = particle_definition_get(particle->definition_index);
		boolean first_update = particle->life_time == 0.f;

		if (render.frame_index - particle->last_rendered_frame_index > MAXIMUM_PARTICLE_UNRENDERED_FRAMES)
		{
			particle_delete(particle_index);
		}
		else
		{
			particle->life_time += dt;

			if (particle->life_time >= particle->life_span && !first_update && !definition->final_sequence_count)
			{
				particle_die(particle_index);
			}
			else if (particle_update_frame_time(particle_index, dt))
			{
				particle_update_physics(particle_index, dt);
			}
		}
	}

	profile_exit(particles_update_section);

	return;
}

void particles_stop_on_first_person_weapon(
	short local_player_index)
{
	long particle_index;

	for (particle_index = data_next_index(particle_data, NONE); particle_index != NONE; particle_index = data_next_index(particle_data, particle_index))
	{
		struct particle_datum const *particle = particle_get(particle_index);

		if (particle->local_player_index == local_player_index &&
			TEST_FLAG(particle->flags, _particle_attached_to_local_player_bit) &&
			particle->object_index != NONE)
		{
			particle_delete(particle_index);
		}
	}

	return;
}

void particles_disconnect_from_structure_bsp(
	void)
{
	return;
}

void particles_reconnect_to_structure_bsp(
	void)
{
	long particle_index;

	for (particle_index = data_next_index(particle_data, NONE); particle_index != NONE; particle_index = data_next_index(particle_data, particle_index))
	{
		struct particle_datum *particle = particle_get(particle_index);
		real_point3d const *position = NULL;

		if (particle->object_index == NONE)
		{
			position = &particle->position;
		}
		else if (TEST_FLAG(particle->flags, _particle_attached_to_local_player_bit))
		{
			position = &first_person_weapon_get_node_matrix(particle->local_player_index, particle->node_index)->position;
		}
		else if (object_try_and_get(particle->object_index))
		{
			position = &object_get_node_matrix(particle->object_index, particle->node_index)->position;
		}

		if (!position)
		{
			particle_delete(particle_index);
		}
		else
		{
			scenario_location_from_point(&particle->location, position);

			if (particle->location.cluster_index == NONE)
			{
				particle_delete(particle_index);
			}
		}
	}

	return;
}

static boolean new_particle_is_visible(
	struct particle_definition const *definition,
	struct location const *location)
{
	if (scenario_location_potentially_visible_local(location))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static boolean particle_next_sequence(
	long particle_index)
{
	struct particle_datum *particle = particle_get(particle_index);
	struct particle_definition const *definition = particle_definition_get(particle->definition_index);
	struct bitmap_group const *bitmap = bitmap_group_get(definition->bitmap.index);

	particle->sequence_index = NONE;

	if (particle->state == _particle_state_next_sequence_initial)
	{
		if (definition->initial_sequence_count > 0)
		{
			particle->sequence_index = definition->first_sequence_index + local_random_range(0, definition->initial_sequence_count);
		}

		particle->state++;
	}

	if (particle->sequence_index == NONE && particle->state == _particle_state_next_sequence_looping || particle->state == _particle_state_still_looping)
	{
		if (particle->state == _particle_state_next_sequence_looping)
		{
			particle->state = _particle_state_still_looping;
		}

		if (particle->life_time < particle->life_span && definition->looping_sequence_count > 0)
		{
			particle->sequence_index = definition->first_sequence_index + definition->initial_sequence_count + local_random_range(0, definition->looping_sequence_count);
		}
		else
		{
			particle->state++;
		}
	}

	if (particle->sequence_index == NONE && particle->state == _particle_state_next_sequence_final)
	{
		if (definition->final_sequence_count > 0)
		{
			particle->sequence_index = definition->first_sequence_index + definition->initial_sequence_count + definition->looping_sequence_count + local_random_range(0, definition->final_sequence_count);
		}

		particle->state++;
	}

	if (particle->sequence_index == NONE || !bitmap->sequences.count)
	{
		particle_die(particle_index);
		return FALSE;
	}
	else
	{
		particle->sequence_index = PIN(particle->sequence_index, 0, bitmap->sequences.count - 1);
		return TRUE;
	}
}

static boolean particle_next_frame(
	long particle_index)
{
	struct particle_datum *particle = particle_get(particle_index);
	struct particle_definition const *definition = particle_definition_get(particle->definition_index);
	struct bitmap_group const *bitmap = bitmap_group_get(definition->bitmap.index);
	boolean result = TRUE;

	particle->frame_time = 0.f;

	if (TEST_FLAG(particle->flags, _particle_animates_backwards_bit))
	{
		if (particle->frame_index > 0)
		{
			particle->frame_index--;
		}
		else
		{
			result = particle_next_sequence(particle_index);

			if (result)
			{
				struct bitmap_group_sequence const *sequence = TAG_BLOCK_GET_ELEMENT(&bitmap->sequences, particle->sequence_index, struct bitmap_group_sequence);

				particle->frame_index = sequence->sprites.count - 1;
			}
		}
	}
	else
	{
		struct bitmap_group_sequence const *sequence = TAG_BLOCK_GET_ELEMENT(&bitmap->sequences, particle->sequence_index, struct bitmap_group_sequence);

		if (particle->frame_index + 1 < sequence->sprites.count)
		{
			particle->frame_index++;
		}
		else
		{
			result = particle_next_sequence(particle_index);
			particle->frame_index = 0;
		}
	}

	return result;
}

static boolean particle_update_frame_time(
	long particle_index,
	real dt)
{
	struct particle_datum *particle = particle_get(particle_index);
	struct particle_definition const *definition = particle_definition_get(particle->definition_index);
	boolean result = TRUE;

	if (!TEST_FLAG(definition->flags, _particle_animation_stops_at_rest_bit) || !TEST_FLAG(particle->flags, _particle_at_rest_bit))
	{
		if (TEST_FLAG(definition->flags, _particle_animates_at_frame_time_bit))
		{
			if (dt != 0.f)
			{
				result = particle_next_frame(particle_index);
			}
		}
		else
		{
			if (particle->frame_time == -1.f)
			{
				result = particle_next_frame(particle_index);
				particle->frame_time = 0.f;
			}

			while (dt > 0.f && result)
			{
				real frame_time_remaining = particle->frame_span - particle->frame_time;

				if (frame_time_remaining <= dt)
				{
					result = particle_next_frame(particle_index);
					dt -= frame_time_remaining;
				}
				else
				{
					particle->frame_time += dt;
					break;
				}
			}
		}
	}

	return result;
}

static boolean particle_update_physics(
	long particle_index,
	real dt)
{
	struct particle_datum *particle = particle_get(particle_index);
	struct particle_definition const *definition = particle_definition_get(particle->definition_index);

	if (!TEST_FLAG(particle->flags, _particle_at_rest_bit))
	{
		boolean at_rest = FALSE;
		struct point_physics_definition *physics = point_physics_definition_get(definition->physics.index);

		if (particle->object_index == NONE)
		{
			real_vector3d collision_normal;
			short collision_material_type;
			unsigned long physics_flags = point_physics_update(
				0,
				physics,
				&particle->location,
				NONE,
				&particle->position,
				&particle->translational_velocity,
				NULL,
				&collision_normal,
				&collision_material_type,
				particle_get_radius(particle_index),
				dt);

			if (TEST_FLAG(physics_flags, _point_physics_collided_with_structure_bit))
			{
				if (definition->collision_effect.index != NONE || definition->collision_material_effects.index)
				{
					real scale = (magnitude3d(&particle->translational_velocity) - minimum_particle_velocity) / (maximum_particle_velocity - minimum_particle_velocity);

					scale = PIN(scale, 0.f, 1.f);

					if (definition->collision_effect.index != NONE)
					{
						particle_effect_new(
							particle,
							definition->collision_effect.group_tag,
							definition->collision_effect.index,
							scale);
					}

					if (definition->collision_material_effects.index != NONE && material_effect_visible(&particle->position))
					{
						material_effect_new(
							definition->collision_material_effects.index,
							_material_effect_impact,
							collision_material_type,
							&particle->position,
							&collision_normal,
							&particle->location,
							scale);
					}
				}

				if (TEST_FLAG(definition->flags, _particle_dies_on_contact_with_structure_bit))
				{
					if (definition->collision_effect.index == NONE)
					{
						particle_die(particle_index);
					}
					else
					{
						particle_delete(particle_index);
					}

					return FALSE;
				}
			}

			if (TEST_FLAG(physics_flags, _point_physics_in_air_bit) && TEST_FLAG(definition->flags, _particle_dies_on_contact_with_air_bit))
			{
				particle_die(particle_index);
				return FALSE;
			}

			if (TEST_FLAG(physics_flags, _point_physics_in_water_bit) && TEST_FLAG(definition->flags, _particle_dies_on_contact_with_water_bit))
			{
				particle_die(particle_index);
				return FALSE;
			}

			if (TEST_FLAG(physics_flags, _point_physics_collided_with_structure_bit) || TEST_FLAG(physics_flags, _point_physics_collided_with_water_bit))
			{
				if (collision_normal.k > PARTICLE_SETTLE_NORMAL_K)
				{
					at_rest = TRUE;
				}

				particle->frame_span += definition->frames_per_second_contact_deterioration;
			}
		}
		else if (!TEST_FLAG(particle->flags, _particle_attached_to_local_player_bit) && !object_try_and_get(particle->object_index))
		{
			particle_delete(particle_index);
			return FALSE;
		}
		else
		{
			real scale;
			real radius = particle_get_radius(particle_index);
			real friction = radius * physics->air_friction * radius;
			real mass = point_physics_definition_get_mass(physics, radius);

			if (mass == 0.f)
			{
				scale = friction == 0.f ? 1.f : 0.f;
			}
			else
			{
				scale = 1.f - friction / mass * dt;
				scale = PIN(scale, 0.f, 1.f);
			}

			particle->translational_velocity.i *= scale;
			particle->translational_velocity.j *= scale;
			particle->translational_velocity.k *= scale;
			particle->position.x += particle->translational_velocity.i * dt;
			particle->position.y += particle->translational_velocity.j * dt;
			particle->position.z += particle->translational_velocity.k * dt;
			at_rest = TRUE;
		}

		if (magnitude_squared3d(&particle->translational_velocity) < PARTICLE_AT_REST_VELOCITY_SQUARED)
		{
			if (at_rest)
			{
				if (TEST_FLAG(definition->flags, _particle_dies_at_rest_bit))
				{
					particle_die(particle_index);
					return FALSE;
				}

				SET_FLAG(particle->flags, _particle_at_rest_bit, TRUE);
			}
		}
		else
		{
			particle->direction = particle->translational_velocity;
		}

		particle->rotation += dt * particle->angular_velocity;
	}
	else if (particle->object_index != NONE && !object_try_and_get(particle->object_index))
	{
		particle_delete(particle_index);
		return FALSE;
	}

	return TRUE;
}

static void particle_die(
	long particle_index)
{
	struct particle_datum *particle = particle_get(particle_index);
	struct particle_definition const *definition = particle_definition_get(particle->definition_index);

	if (definition->death_effect.index != NONE)
	{
		particle_effect_new(particle, definition->death_effect.group_tag, definition->death_effect.index, 0.f);
	}

	particle_delete(particle_index);

	return;
}

static void particle_effect_new(
	struct particle_datum const *particle,
	unsigned long group_tag,
	long effect_index,
	real scale)
{
	real_vector3d velocity;

	scale_vector3d(&particle->translational_velocity, 1.f / TICKS_PER_SECOND, &velocity);

	if (group_tag == EFFECT_DEFINITION_TAG)
	{
		real_point3d marker_points[NUMBER_OF_PARTICLE_EFFECT_MARKERS];
		real_vector3d marker_forwards[NUMBER_OF_PARTICLE_EFFECT_MARKERS];

		marker_points[_effect_marker_gravity] = particle->position;
		marker_points[_effect_marker_velocity] = particle->position;
		marker_forwards[_effect_marker_velocity] = particle->direction;
		marker_forwards[_effect_marker_gravity] = *global_down3d;
		normalize3d(&marker_forwards[_effect_marker_velocity]);
		effect_new_unattached_from_markers(
			effect_index,
			NONE,
			&velocity,
			NUMBER_OF_PARTICLE_EFFECT_MARKERS,
			particle_effect_marker_names,
			marker_points,
			marker_forwards,
			scale,
			0.f,
			NULL,
			NULL,
			FALSE);
	}
	else if (group_tag == SOUND_DEFINITION_TAG)
	{
		struct sound_location location;

		location.position = particle->position;
		location.forward = *global_forward3d;
		location.translational_velocity = velocity;
		location.game_location = particle->location;
		unattached_impulse_sound_new(effect_index, &location, scale);
	}
	else
	{
		match_halt("c:\\halo\\SOURCE\\effects\\particles.c", 799);
	}

	return;
}

real particle_get_radius(
	long particle_index)
{
	struct particle_datum const *particle = particle_get(particle_index);
	struct particle_definition const *definition = particle_definition_get(particle->definition_index);

	return ((definition->final_radius_modifier - definition->initial_radius_modifier) * (particle->life_time / particle->life_span) + definition->initial_radius_modifier) * particle->radius;
}

/* ---------- private code */
