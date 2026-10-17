/*
RENDER_PARTICLES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "render.h"
#include "render_debug.h"
#include "collision_bsp.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "console.h"
#include "director.h"
#include "structures.h"
#include "physics_constants.h"
#include "object_lights.h"
#include "first_person_weapons.h"
#include "particles.h"
#include "particle_definitions.h"

/* ---------- constants */

enum
{
	MAXIMUM_PARTICLE_SPRITE_GROUPS = 512,	/* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct rendered_particle_datum
{
	short particle_index;
	short definition_index;
	short cluster_index;
	boolean attached_to_first_person_weapon;
};

/* ---------- prototypes */

static int compare_rendered_particles(void const *a, void const *b);

/* ---------- globals */

static struct profile_section render_particles_section = {"render_particles", NONE, TRUE};

/* ---------- public code */

boolean local_player_is_first_person(
	short local_player_index)
{
	boolean first_person = director_get_perspective(local_player_index) == _director_perspective_first_person;

	if (!first_person)
	{
		long player_index = local_player_get_player_index(local_player_index);
		struct player_datum *player = player_get(player_index);
		long unit_index = player->unit_index;

		if (unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(unit_index);

			if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
			{
				struct unit_datum *parent = unit_get(unit->object.parent_object_index);
				struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
					&unit_definition_get(parent->definition_index)->unit.seats,
					unit->unit.parent_seat_index,
					struct unit_seat);

				if (TEST_FLAG(seat->flags, _unit_seat_is_gunner_bit))
				{
					first_person = TRUE;
				}
			}
		}
	}

	return first_person;
}

void render_particles(
	void)
{
	struct rendered_particle_datum rendered_particles[MAXIMUM_PARTICLES_PER_MAP];
	short rendered_particle_count = 0;

	profile_enter(render_particles_section);

	if (render_particles_enabled)
	{
		long particle_index;
		short local_player_index = render.local_player_index;

		if (local_player_index == NONE || !local_player_is_first_person(local_player_index))
		{
			local_player_index = MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		}

		for (particle_index = data_next_index(particle_data, NONE);
			particle_index != NONE;
			particle_index = data_next_index(particle_data, particle_index))
		{
			struct particle_datum *particle = particle_get(particle_index);
			boolean local_particle = particle->local_player_index == local_player_index;

			if (render_location_visible(&particle->location) &&
				(!TEST_FLAG(particle->flags, _particle_dont_draw_first_person_bit) || !local_particle) &&
				(!TEST_FLAG(particle->flags, _particle_dont_draw_third_person_bit) || local_particle))
			{
				struct rendered_particle_datum *rendered_particle = &rendered_particles[rendered_particle_count++];

				rendered_particle->particle_index = (short)particle_index;
				rendered_particle->definition_index = (short)particle->definition_index;
				rendered_particle->cluster_index = particle->location.cluster_index;
				rendered_particle->attached_to_first_person_weapon =
					local_particle &&
					TEST_FLAG(particle->flags, _particle_dont_draw_third_person_bit);
			}
		}
	}

	if (rendered_particle_count > 0)
	{
		short sprite_groups[MAXIMUM_PARTICLE_SPRITE_GROUPS];
		struct rendered_particle_datum *rendered_particle;
		short *current_sprite_group;
		short definition_index;
		short cluster_index;
		boolean attached_to_first_person_weapon;
		short sprite_group_index;
		short sprite_group_count = 0;

		qsort(
			rendered_particles,
			rendered_particle_count,
			sizeof(*rendered_particles),
			compare_rendered_particles);
		rendered_particle = rendered_particles;
		definition_index = NONE;
		cluster_index = NONE;
		attached_to_first_person_weapon = FALSE;

		while (rendered_particle_count-- > 0)
		{
			if (rendered_particle->definition_index == definition_index &&
				rendered_particle->cluster_index == cluster_index &&
				rendered_particle->attached_to_first_person_weapon == attached_to_first_person_weapon)
			{
				(*current_sprite_group)++;
			}
			else
			{
				if (sprite_group_count >= MAXIMUM_PARTICLE_SPRITE_GROUPS)
				{
					break;
				}

				current_sprite_group = &sprite_groups[sprite_group_count++];
				*current_sprite_group = 1;
				definition_index = rendered_particle->definition_index;
				cluster_index = rendered_particle->cluster_index;
				attached_to_first_person_weapon = rendered_particle->attached_to_first_person_weapon;
			}

			rendered_particle++;
		}

		rendered_particle = rendered_particles;

		for (sprite_group_index = 0; sprite_group_index < sprite_group_count; sprite_group_index++)
		{
			struct build_sprite_data data;
			struct particle_definition *definition = particle_definition_get(rendered_particle->definition_index);
			real total_radius = 0.f;
			short rendered_sprite_count = 0;
			short sprite_count = sprite_groups[sprite_group_index];

			build_sprites_begin(
				&data,
				sprite_count,
				definition->bitmap.index,
				&definition->shader,
				rendered_particle->attached_to_first_person_weapon ? FLAG(_build_sprites_first_person_bit) : 0);

			while (sprite_count-- > 0)
			{
				real_point3d world_position;
				real_vector3d world_direction;
				real pixels;
				struct particle_datum *particle = particle_get(rendered_particle->particle_index);
				real radius = particle_get_radius(rendered_particle->particle_index);

				if (particle->object_index != NONE)
				{
					real_matrix4x3 const *node_matrix;

					if (TEST_FLAG(particle->flags, _particle_attached_to_local_player_bit))
					{
						node_matrix = first_person_weapon_get_node_matrix(particle->local_player_index, particle->node_index);
					}
					else if (object_try_and_get_and_verify_type(particle->object_index, NONE))
					{
						node_matrix = object_get_node_matrix(particle->object_index, particle->node_index);
					}
					else
					{
						particle_delete(rendered_particle->particle_index);
						rendered_particle++;
						continue;
					}

					matrix4x3_transform_point(node_matrix, &particle->position, &world_position);
					matrix4x3_transform_normal(node_matrix, &particle->direction, &world_direction);
				}
				else
				{
					world_position = particle->position;
					world_direction = particle->direction;
					particle->object_index = NONE;
				}

				pixels = render_frustum_sphere_diameter_in_pixels(&render.frustum, &world_position, radius);

				if (pixels > definition->lod_cutoff)
				{
					real scale = definition->runtime_oo_width * (2.f * radius);
					real fade = 1.f;
					real life_remaining = particle->life_span - particle->life_time;
					unsigned long flags = 0;

					total_radius += radius;
					rendered_sprite_count++;

					if (pixels < definition->minimum_pixels)
					{
						scale *= definition->minimum_pixels / pixels;
					}

					if (definition->fade_in_time > 0.f && particle->life_time < definition->fade_in_time)
					{
						fade *= particle->life_time / definition->fade_in_time;
					}

					if (definition->fade_out_time > 0.f && life_remaining < definition->fade_out_time)
					{
						fade *= life_remaining / definition->fade_out_time;
					}

					SET_FLAG(flags, _build_sprite_u_mirror_bit, TEST_FLAG(particle->flags, _particle_u_mirror_bit));
					SET_FLAG(flags, _build_sprite_v_mirror_bit, TEST_FLAG(particle->flags, _particle_v_mirror_bit));
					build_sprite(
						&data,
						definition->sprite_orientation,
						particle->sequence_index,
						particle->frame_index,
						&world_position,
						&world_direction,
						particle->rotation,
						scale,
						&particle->color,
						fade,
						flags);
					particle->last_rendered_frame_index = render.frame_index;
				}

				rendered_particle++;
			}

			((struct shader_effect *)data.shader)->effect.secondary_map_radius = total_radius / rendered_sprite_count;
			build_sprites_end(&data);
		}
	}

	profile_exit(render_particles_section);

	return;
}

/* ---------- private code */

static int compare_rendered_particles(
	void const *a,
	void const *b)
{
	struct rendered_particle_datum const *particle0 = a;
	struct rendered_particle_datum const *particle1 = b;
	int result = particle0->definition_index - particle1->definition_index;

	if (result == 0)
	{
		result = particle0->cluster_index - particle1->cluster_index;

		if (result == 0)
		{
			result = particle0->attached_to_first_person_weapon - particle1->attached_to_first_person_weapon;
		}
	}

	return result;
}
