/*
EFFECTS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "effects.h"
#include "decals.h"
#include "particles.h"
#include "particle_definitions.h"
#include "particle_systems.h"
#include "particle_system_definitions.h"
#include "render.h"
#include "network_game_globals.h"
#include "players.h"
#include "rasterizer_console_vars.h"
#include "game_state.h"
#include "light_definitions.h"
#include "collisions.h"
#include "sound_manager.h"
#include "items.h"
#include "render_debug.h"
#include "director.h"
#include "game_sound.h"
#include "sound_definitions.h"
#include "bipeds.h"
#include "structures.h"
#include "editor_stubs.h"
#include "object_lights.h"
#include "damage_effect_definitions.h"
#include "first_person_weapons.h"

/* ---------- constants */

enum
{
	MAXIMUM_EFFECTS_PER_MAP = 256,
	MAXIMUM_EFFECT_LOCATIONS_PER_MAP = 512,
	MAXIMUM_EFFECT_OBJECT_MARKERS = 16,
	MAXIMUM_EFFECT_EVENTS_PER_UPDATE = 8,
	MAXIMUM_SPLIT_SCREEN_PARTICLE_COUNT = 6,	/* fake name */
};

enum
{
	_effect_in_duration_bit = 0,
	_effect_loop_bit,
	_effect_stopping_bit,
	_effect_stopped_bit,
	_effect_invisible_bit,
	_effect_delete_on_stop_bit,
	_effect_nonviolent_bit,
	NUMBER_OF_EFFECT_FLAGS
};

enum
{
	_effect_location_first_person_bit = 15,	/* fake name */
};

/* ---------- macros */

#define effect_get(index) ((struct effect_datum *)datum_get(effect_data, (index)))	/* fake name */
#define effect_try_and_get(index) ((struct effect_datum *)datum_try_and_get(effect_data, (index)))
#define effect_location_get(index) ((struct effect_location_datum *)datum_get(effect_location_data, (index)))	/* fake name */

/* ---------- structures */

struct effect_location_datum
{
	short identifier;
	short node_designator;
	long next_instance_location_index;
	real_matrix4x3 matrix;
};

struct effect_datum
{
	short identifier;
	word flags;
	long definition_index;
	short scale_a_function_index;
	short scale_b_function_index;
	short change_color_index;
	struct location location;
	real_rgb_color color;
	real_vector3d velocity;
	struct effect_vector_field impulse_field;
	long object_index;
	long owner_object_index;
	real scale_a;
	real scale_b;
	short local_player_index;
	short event_index;
	real event_time;
	real event_duration;
	real last_event_fraction;
	long location_datum_indices[MAXIMUM_EFFECT_LOCATIONS];
	byte particles_counts[MAXIMUM_EFFECT_PARTICLES_PER_EVENT];
};

struct effect_marker_list
{
	short node_index;
	real_matrix4x3 const *node_matrix;
	short count;
	char const **names;
	real_point3d const *points;
	real_vector3d const *forwards;
};

/* ---------- prototypes */

static boolean effects_object_is_corpse(long object_index);
static real effect_scale(struct effect_datum const *effect, real upper_bound, unsigned long a_scale_flags, unsigned long b_scale_flags, short bit_index);
static unsigned long *effect_get_random_seed(long effect_definition_index);
static real effect_real_random_range(unsigned long *seed, struct effect_datum const *effect, real lower_bound, real upper_bound, unsigned long a_scale_flags, unsigned long b_scale_flags, short first_bit_index);
static void effect_random_translational_velocity(unsigned long *seed, struct effect_datum const *effect, real_vector3d const *forward, real_vector3d *direction, real_vector3d *velocity, real lower_bound, real upper_bound, real cone_angle, unsigned long a_scale_flags, unsigned long b_scale_flags);
static void effect_random_angular_velocity(unsigned long *seed, struct effect_datum const *effect, real_vector3d *result, real lower_bound, real upper_bound, unsigned long a_scale_flags, unsigned long b_scale_flags);
static boolean effect_allowed_by_environment(short environment, struct location const *location, real_point3d const *world_point);
static boolean effect_part_allowed_by_disposition(boolean effect_is_nonviolent, short part_disposition);
static long effect_allocate(long definition_index, long owner_object_index, boolean can_be_deterministic);
static void impulse_effect_initialize(struct effect_datum *effect, real scale_a, real scale_b, real_rgb_color const *color, struct effect_vector_field const *impulse_field);
static void effect_set_event(long effect_index, short event_index);
static void effect_build_locations(struct effect_datum *effect, short (*get_markers_by_name)(long object_index, char const *name, struct object_marker *markers, short maximum_marker_count));
static long effect_build_location(struct effect_datum *effect, struct object_marker const *marker, short location_index, boolean first_person_weapon);
static short effect_marker_list_get_markers_by_name(long object_index, char const *name, struct object_marker *results, short maximum_count);
static void effect_marker_list_get_marker(struct object_marker *result, struct effect_marker_list const *marker_list, short effect_marker_index);
static void effect_update(long effect_index, real dt);
static void effect_generate_particles(struct effect_datum *effect);
static void effect_generate_parts(struct effect_datum *effect);
static void effect_generate_part(struct effect_datum const *effect, struct effect_part_definition const *part_definition, struct effect_location_datum const *location, real_point3d const *world_point, real_vector3d const *world_forward, real_vector3d const *world_up, real scale);
static struct effect_location_datum *effect_location_get_next_instance(struct effect_datum const *effect, long *location_datum_index, short camera_mode);
static real_matrix4x3 *effect_get_node_matrix(struct effect_datum const *effect, short node_designator);
static real effect_evaluate_function_integral(short function, real fraction);

/* ---------- globals */

boolean effects_corpse_nonviolent = TRUE;

static struct profile_section effects_update_section =
{
	"effects_update",
	NONE,
	TRUE
};

static struct effect_marker_list const *global_effect_marker_list;
boolean debug_effects_nonviolent = FALSE;

struct data_array *effect_data;
struct data_array *effect_location_data;

/* ---------- public code */

void effects_initialize(
	void)
{
	effect_data = game_state_data_new("effect", MAXIMUM_EFFECTS_PER_MAP, sizeof(struct effect_datum));
	effect_location_data = game_state_data_new("effect location", MAXIMUM_EFFECT_LOCATIONS_PER_MAP, sizeof(struct effect_location_datum));

	if (!effect_data || !effect_location_data)
	{
		error(_error_immediate, "couldn't allocate effect globals");
	}

	return;
}

void effects_initialize_for_new_map(
	void)
{
	data_make_valid(effect_data);
	data_make_valid(effect_location_data);

	return;
}

void effects_dispose_from_old_map(
	void)
{
	data_make_invalid(effect_data);
	data_make_invalid(effect_location_data);

	return;
}

void effects_dispose(
	void)
{
	if (effect_data)
	{
		effect_data = NULL;
	}

	if (effect_location_data)
	{
		effect_location_data = NULL;
	}

	return;
}

static boolean effects_object_is_corpse(
	long object_index)
{
	boolean corpse = FALSE;
	struct biped_datum const *biped = biped_try_and_get(object_index);

	if (biped && TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
	{
		corpse = biped->unit.time_of_death != NONE && biped->unit.time_of_death + TICKS_PER_SECOND < game_time_get();
	}

	return corpse;
}

long effect_new_looping(
	long definition_index,
	long object_index,
	short scale_a_function_index,
	short scale_b_function_index,
	short change_color_index)
{
	long effect_index;

	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 249, object_index!=NONE);
	effect_index = effect_allocate(definition_index, object_index, TRUE);

	if (effect_index != NONE)
	{
		struct effect_datum *effect = effect_get(effect_index);

		effect->object_index = object_index;
		effect->local_player_index = first_person_weapon_get_local_index(object_index);
		effect->scale_a_function_index = scale_a_function_index;
		effect->scale_b_function_index = scale_b_function_index;
		effect->change_color_index = change_color_index;
		effect->impulse_field.translational = NULL;
		effect->impulse_field.angular = NULL;

		if (change_color_index == NONE)
		{
			effect->color = *global_real_rgb_white;
		}

		SET_FLAG(effect->flags, _effect_loop_bit, TRUE);
		memset(effect->location_datum_indices, NONE, sizeof(effect->location_datum_indices));
		effect_build_locations(effect, object_get_marker_by_name);

		if (effect->local_player_index != NONE)
		{
			effect_build_locations(effect, first_person_weapon_get_marker_by_name);
		}

		effect_update(effect_index, 0.f);
	}

	return effect_index;
}

long effect_new_from_object(
	long definition_index,
	long owner_object_index,
	long object_index,
	short force_local_player_index,
	real scale_a,
	real scale_b,
	real_rgb_color const *color,
	struct effect_vector_field const *impulse_field)
{
	long effect_index;

	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 305, object_index!=NONE);
	match_vassert("c:\\halo\\SOURCE\\effects\\effects.c", 306, scale_a>=0.f && scale_a<=1.f, csprintf(temporary, "scale_a %f not in [0,1]", scale_a));
	match_vassert("c:\\halo\\SOURCE\\effects\\effects.c", 307, scale_b>=0.f && scale_b<=1.f, csprintf(temporary, "scale_b %f not in [0,1]", scale_b));
	effect_index = effect_allocate(definition_index, owner_object_index, TRUE);

	if (effect_index != NONE)
	{
		struct effect_datum *effect = effect_get(effect_index);

		impulse_effect_initialize(effect, scale_a, scale_b, color, impulse_field);
		effect->object_index = object_index;
		effect->local_player_index = first_person_weapon_get_local_index(object_index);

		if (effects_corpse_nonviolent && effects_object_is_corpse(effect->object_index))
		{
			SET_FLAG(effect->flags, _effect_nonviolent_bit, TRUE);
		}

		if (debug_effects_nonviolent)
		{
			struct object_datum const *object = object_try_and_get(object_index);

			error(
				_error_silent,
				"created %sviolent %s on %s",
				TEST_FLAG(effect->flags, _effect_nonviolent_bit) ? "non" : "",
				tag_name_strip_path(tag_get_name(definition_index)),
				!object ? "<none>" : tag_name_strip_path(tag_get_name(object->definition_index)));
		}

		memset(effect->location_datum_indices, NONE, sizeof(effect->location_datum_indices));
		effect_build_locations(effect, object_get_marker_by_name);

		if (effect->local_player_index != NONE)
		{
			effect_build_locations(effect, first_person_weapon_get_marker_by_name);
		}

		if (force_local_player_index != NONE)
		{
			effect->local_player_index = force_local_player_index;
		}

		effect_update(effect_index, 0.f);
	}

	return effect_index;
}

long effect_new_attached_from_markers(
	long definition_index,
	long owner_object_index,
	long object_index,
	short node_index,
	short marker_count,
	char const **marker_names,
	real_point3d const *marker_points,
	real_vector3d const *marker_forwards,
	real scale_a,
	real scale_b,
	real_rgb_color const *color,
	struct effect_vector_field const *impulse_field)
{
	long effect_index;

	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 375, object_index!=NONE);
	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 377, marker_count>0);
	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 379, marker_points);
	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 380, marker_forwards);
	match_vassert("c:\\halo\\SOURCE\\effects\\effects.c", 381, scale_a>=0.f && scale_a<=1.f, csprintf(temporary, "scale_a %f not in [0,1]", scale_a));
	match_vassert("c:\\halo\\SOURCE\\effects\\effects.c", 382, scale_b>=0.f && scale_b<=1.f, csprintf(temporary, "scale_b %f not in [0,1]", scale_b));
	effect_index = effect_allocate(definition_index, owner_object_index, TRUE);

	if (effect_index != NONE)
	{
		struct effect_marker_list marker_list;
		struct effect_datum *effect = effect_get(effect_index);

		impulse_effect_initialize(effect, scale_a, scale_b, color, impulse_field);
		effect->object_index = object_index;

		if (effects_corpse_nonviolent && effects_object_is_corpse(effect->object_index))
		{
			SET_FLAG(effect->flags, _effect_nonviolent_bit, TRUE);
		}

		if (debug_effects_nonviolent)
		{
			struct object_datum const *object = object_try_and_get(object_index);

			error(
				_error_silent,
				"created %sviolent %s on %s",
				TEST_FLAG(effect->flags, _effect_nonviolent_bit) ? "non" : "",
				tag_name_strip_path(tag_get_name(definition_index)),
				!object ? "<none>" : tag_name_strip_path(tag_get_name(object->definition_index)));
		}

		marker_list.count = marker_count;
		marker_list.names = marker_names;
		marker_list.points = marker_points;
		marker_list.forwards = marker_forwards;
		marker_list.node_index = node_index != NONE ? node_index : 0;
		marker_list.node_matrix = object_get_node_matrix(object_index, marker_list.node_index);
		global_effect_marker_list = &marker_list;
		memset(effect->location_datum_indices, NONE, sizeof(effect->location_datum_indices));
		effect_build_locations(effect, effect_marker_list_get_markers_by_name);
		effect_update(effect_index, 0.f);
	}

	return effect_index;
}

long effect_new_unattached_from_markers(
	long definition_index,
	long owner_object_index,
	real_vector3d const *translational_velocity,
	short marker_count,
	char const **marker_names,
	real_point3d const *marker_points,
	real_vector3d const *marker_forwards,
	real scale_a,
	real scale_b,
	real_rgb_color const *color,
	struct effect_vector_field const *impulse_field,
	boolean deterministic)
{
	long effect_index;

	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 448, !translational_velocity || valid_real_vector3d(translational_velocity));
	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 449, marker_count>0);
	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 451, marker_points);
	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 452, marker_forwards);
	match_vassert("c:\\halo\\SOURCE\\effects\\effects.c", 453, scale_a>=0.f && scale_a<=1.f, csprintf(temporary, "scale_a %f not in [0,1]", scale_a));
	match_vassert("c:\\halo\\SOURCE\\effects\\effects.c", 454, scale_b>=0.f && scale_b<=1.f, csprintf(temporary, "scale_b %f not in [0,1]", scale_b));
	effect_index = effect_allocate(definition_index, owner_object_index, deterministic);

	if (effect_index != NONE)
	{
		struct effect_marker_list marker_list;
		struct effect_datum *effect = effect_get(effect_index);

		impulse_effect_initialize(effect, scale_a, scale_b, color, impulse_field);
		effect->object_index = NONE;
		marker_list.count = marker_count;
		marker_list.names = marker_names;
		marker_list.points = marker_points;
		marker_list.forwards = marker_forwards;
		marker_list.node_index = NONE;
		marker_list.node_matrix = NULL;
		scenario_location_from_point(&effect->location, marker_points);
		effect->velocity = translational_velocity ? *translational_velocity : *global_zero_vector3d;
		global_effect_marker_list = &marker_list;
		memset(effect->location_datum_indices, NONE, sizeof(effect->location_datum_indices));
		effect_build_locations(effect, effect_marker_list_get_markers_by_name);
		effect_update(effect_index, 0.f);
	}

	return effect_index;
}

void effects_update(
	real dt)
{
	long effect_index;

	profile_enter(effects_update_section);

	for (effect_index = data_next_index(effect_data, NONE); effect_index != NONE; effect_index = data_next_index(effect_data, effect_index))
	{
		effect_update(effect_index, dt);
	}

	profile_exit(effects_update_section);

	return;
}

void effect_stop(
	long effect_index,
	boolean and_delete)
{
	struct effect_datum *effect = effect_try_and_get(effect_index);

	if (effect)
	{
		struct effect_definition const *definition = effect_definition_get(effect->definition_index);

		if (TEST_FLAG(effect->flags, _effect_loop_bit))
		{
			SET_FLAG(effect->flags, _effect_delete_on_stop_bit, and_delete);

			if (definition->loop_stop_index < 0 || definition->loop_stop_index + 1 >= definition->events.count)
			{
				SET_FLAG(effect->flags, _effect_stopped_bit, TRUE);
			}
			else
			{
				effect_set_event(effect_index, definition->loop_stop_index + 1);
				SET_FLAG(effect->flags, _effect_stopping_bit, TRUE);
			}
		}
		else
		{
			effect_delete(effect_index);
		}
	}

	return;
}

void effect_delete(
	long effect_index)
{
	struct effect_datum *effect = effect_try_and_get(effect_index);

	if (effect)
	{
		short location_index;
		struct effect_definition const *definition = effect_definition_get(effect->definition_index);

		for (location_index = 0; location_index < definition->locations.count; location_index++)
		{
			long location_datum_index = effect->location_datum_indices[location_index];

			while (location_datum_index != NONE)
			{
				struct effect_location_datum *location = effect_location_get(location_datum_index);

				datum_delete(effect_location_data, location_datum_index);
				location_datum_index = location->next_instance_location_index;
			}
		}

		datum_delete(effect_data, effect_index);
	}

	return;
}

void effects_start_on_first_person_weapon(
	short local_player_index,
	long object_index)
{
	long effect_index;

	for (effect_index = data_next_index(effect_data, NONE); effect_index != NONE; effect_index = data_next_index(effect_data, effect_index))
	{
		struct effect_datum *effect = effect_get(effect_index);
		struct effect_definition const *definition = effect_definition_get(effect->definition_index);

		if (effect->object_index == object_index)
		{
			match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 585, effect->local_player_index==NONE);
			effect->local_player_index = local_player_index;
			effect_build_locations(effect, first_person_weapon_get_marker_by_name);
		}
	}

	return;
}

void effects_stop_on_first_person_weapon(
	short local_player_index)
{
	long effect_index;

	for (effect_index = data_next_index(effect_data, NONE); effect_index != NONE; effect_index = data_next_index(effect_data, effect_index))
	{
		struct effect_datum *effect = effect_get(effect_index);
		struct effect_definition const *definition = effect_definition_get(effect->definition_index);

		if (effect->local_player_index == local_player_index)
		{
			short location_index;

			for (location_index = 0; location_index < definition->locations.count; location_index++)
			{
				long *location_datum_index = &effect->location_datum_indices[location_index];

				while (*location_datum_index != NONE)
				{
					struct effect_location_datum *location = effect_location_get(*location_datum_index);

					if (location->node_designator != NONE && TEST_FLAG(location->node_designator, _effect_location_first_person_bit))
					{
						datum_delete(effect_location_data, *location_datum_index);
						*location_datum_index = location->next_instance_location_index;
					}
					else
					{
						location_datum_index = &location->next_instance_location_index;
					}
				}
			}

			effect->local_player_index = NONE;
		}
	}

	return;
}

void effects_information_get(
	struct effects_information *information)
{
	long effect_index;

	information->effect_count = effect_data->actual_count;
	information->location_count = effect_location_data->actual_count;
	information->active_effect_count = 0;

	for (effect_index = data_next_index(effect_data, NONE); effect_index != NONE; effect_index = data_next_index(effect_data, effect_index))
	{
		struct effect_datum const *effect = effect_get(effect_index);

		if (!TEST_FLAG(effect->flags, _effect_stopped_bit) && !TEST_FLAG(effect->flags, _effect_invisible_bit))
		{
			information->active_effect_count++;
		}
	}

	return;
}

void effects_disconnect_from_structure_bsp(
	void)
{
	return;
}

void effects_reconnect_to_structure_bsp(
	void)
{
	long effect_index;

	for (effect_index = data_next_index(effect_data, NONE); effect_index != NONE; effect_index = data_next_index(effect_data, effect_index))
	{
		struct effect_datum *effect = effect_get(effect_index);

		if (effect->object_index == NONE)
		{
			long location_datum_index = effect->location_datum_indices[0];
			struct effect_location_datum const *location = effect_location_get_next_instance(effect, &location_datum_index, _effect_camera_mode_independent);

			if (location)
			{
				scenario_location_from_point(&effect->location, &location->matrix.position);
			}
			else
			{
				effect_delete(effect_index);
			}
		}
	}

	return;
}

boolean dangerous_effects_near_player(
	void)
{
	long effect_index;

	for (effect_index = data_next_index(effect_data, NONE); effect_index != NONE; effect_index = data_next_index(effect_data, effect_index))
	{
		struct effect_datum const *effect = effect_get(effect_index);
		struct effect_definition const *definition = effect_definition_get(effect->definition_index);

		if (!TEST_FLAG(effect->flags, _effect_stopped_bit) && definition->runtime_danger_radius != 0.f)
		{
			struct data_iterator player_iterator;
			struct player_datum const *player;

			data_iterator_new(&player_iterator, player_data);

			while (player = (struct player_datum const *)data_iterator_next(&player_iterator))
			{
				if (player->unit_index != NONE)
				{
					struct effect_location_datum const *location;
					struct object_datum const *unit = object_get(player->unit_index);
					long location_datum_index = effect->location_datum_indices[0];

					while (location = effect_location_get_next_instance(effect, &location_datum_index, _effect_camera_mode_independent))
					{
						real_point3d world_point;

						if (location->node_designator != NONE)
						{
							matrix4x3_transform_point(
								effect_get_node_matrix(effect, location->node_designator),
								&location->matrix.position,
								&world_point);
						}
						else
						{
							world_point = location->matrix.position;
						}

						if (point_in_sphere(&unit->object.bounding_sphere_center, &world_point, unit->object.bounding_sphere_radius + definition->runtime_danger_radius))
						{
							return TRUE;
						}
					}
				}
			}
		}
	}

	return FALSE;
}

static real effect_scale(
	struct effect_datum const *effect,
	real upper_bound,
	unsigned long a_scale_flags,
	unsigned long b_scale_flags,
	short bit_index)
{
	real scale = upper_bound;

	if (TEST_FLAG(a_scale_flags, bit_index))
	{
		scale *= effect->scale_a;
	}

	if (TEST_FLAG(b_scale_flags, bit_index))
	{
		scale *= effect->scale_b;
	}

	return scale;
}

static unsigned long *effect_get_random_seed(
	long effect_definition_index)
{
	struct effect_definition const *definition = effect_definition_get(effect_definition_index);

	if (TEST_FLAG(definition->flags, _effect_must_be_deterministic_bit))
	{
		return get_global_random_seed_address();
	}
	else
	{
		return get_global_local_random_seed_address();
	}
}

static real effect_real_random_range(
	unsigned long *seed,
	struct effect_datum const *effect,
	real lower_bound,
	real upper_bound,
	unsigned long a_scale_flags,
	unsigned long b_scale_flags,
	short first_bit_index)
{
	real lower = effect_scale(effect, lower_bound, a_scale_flags, b_scale_flags, first_bit_index);
	real delta = effect_scale(effect, upper_bound - lower_bound, a_scale_flags, b_scale_flags, first_bit_index + 1);

	return real_seed_random_range(seed, 0.f, delta) + lower;
}

static void effect_random_translational_velocity(
	unsigned long *seed,
	struct effect_datum const *effect,
	real_vector3d const *forward,
	real_vector3d *direction,
	real_vector3d *velocity,
	real lower_bound,
	real upper_bound,
	real cone_angle,
	unsigned long a_scale_flags,
	unsigned long b_scale_flags)
{
	real magnitude = effect_real_random_range(seed, effect, lower_bound, upper_bound, a_scale_flags, b_scale_flags, _effect_velocity_bit);
	real angle = real_seed_random(seed) * effect_scale(effect, cone_angle, a_scale_flags, b_scale_flags, _effect_velocity_cone_bit);

	*direction = *forward;

	if (angle != 0.f)
	{
		real_vector3d axis;

		rotate_vector_about_axis(direction, seed_random_direction3d(seed, &axis), sine(angle), cosine(angle));
	}

	scale_vector3d(direction, magnitude, velocity);

	return;
}

static void effect_random_angular_velocity(
	unsigned long *seed,
	struct effect_datum const *effect,
	real_vector3d *result,
	real lower_bound,
	real upper_bound,
	unsigned long a_scale_flags,
	unsigned long b_scale_flags)
{
	real magnitude = effect_real_random_range(seed, effect, lower_bound, upper_bound, a_scale_flags, b_scale_flags, _effect_angular_velocity_bit);

	if (magnitude != 0.f)
	{
		seed_random_direction3d(seed, result);
		scale_vector3d(result, magnitude, result);
	}
	else
	{
		*result = *global_zero_vector3d;
	}

	return;
}

static boolean effect_allowed_by_environment(
	short environment,
	struct location const *location,
	real_point3d const *world_point)
{
	boolean allowed;

	switch (environment)
	{
	case _effect_environment_anywhere:
		allowed = TRUE;
		break;
	case _effect_environment_air:
		allowed = !scenario_location_underwater(location, world_point, NULL);
		break;
	case _effect_environment_water:
		allowed = scenario_location_underwater(location, world_point, NULL);
		break;
	case _effect_environment_vacuum:
		allowed = FALSE;
		break;
	default:
		match_halt("c:\\halo\\SOURCE\\effects\\effects.c", 888);
		break;
	}

	return allowed;
}

static boolean effect_part_allowed_by_disposition(
	boolean effect_is_nonviolent,
	short part_disposition)
{
	if (effect_is_nonviolent)
	{
		return part_disposition != _effect_disposition_violent;
	}
	else
	{
		return part_disposition != _effect_disposition_nonviolent;
	}
}

static long effect_allocate(
	long definition_index,
	long owner_object_index,
	boolean can_be_deterministic)
{
	long effect_index = NONE;

	if (!game_in_editor() && definition_index != NONE)
	{
		struct effect_definition const *definition = effect_definition_get(definition_index);

		if (!can_be_deterministic && TEST_FLAG(definition->flags, _effect_must_be_deterministic_bit))
		{
			error(_error_silent, "cannot create objects, lights, or damage from an effect created by particles.");
		}
		else if (definition->events.count > 0)
		{
			effect_index = datum_new(effect_data);

			if (effect_index == NONE && TEST_FLAG(definition->flags, _effect_must_be_deterministic_bit))
			{
				for (effect_index = data_next_index(effect_data, NONE); effect_index != NONE; effect_index = data_next_index(effect_data, effect_index))
				{
					struct effect_datum const *effect = effect_get(effect_index);

					if (!TEST_FLAG(effect_definition_get(effect->definition_index)->flags, _effect_must_be_deterministic_bit))
					{
						datum_delete(effect_data, effect_index);
						effect_index = datum_new(effect_data);
						match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 941, effect_index!=NONE);
						break;
					}
				}
			}

			if (effect_index != NONE)
			{
				struct effect_datum *effect = effect_get(effect_index);

				effect->definition_index = definition_index;
				effect->owner_object_index = owner_object_index;
				effect->local_player_index = NONE;
				effect->flags = 0;
				effect_set_event(effect_index, 0);
			}
		}
	}

	return effect_index;
}

static void impulse_effect_initialize(
	struct effect_datum *effect,
	real scale_a,
	real scale_b,
	real_rgb_color const *color,
	struct effect_vector_field const *impulse_field)
{
	effect->scale_a = scale_a;
	effect->scale_b = scale_b;
	effect->color = color ? *color : *global_real_rgb_white;
	match_assert_valid_real_rgb_color("c:\\halo\\SOURCE\\effects\\effects.c", 980, &effect->color);

	if (impulse_field)
	{
		effect->impulse_field = *impulse_field;
	}
	else
	{
		effect->impulse_field.translational = NULL;
		effect->impulse_field.angular = NULL;
	}

	return;
}

static void effect_set_event(
	long effect_index,
	short event_index)
{
	struct effect_datum *effect = effect_try_and_get(effect_index);

	if (effect)
	{
		struct effect_definition const *definition = effect_definition_get(effect->definition_index);

		if (event_index >= 0 && event_index < definition->events.count)
		{
			struct effect_event_definition const *event = TAG_BLOCK_GET_ELEMENT(&definition->events, event_index, struct effect_event_definition);

			SET_FLAG(effect->flags, _effect_in_duration_bit, FALSE);
			effect->event_index = event_index;
			effect->event_time = 0.f;
			effect->event_duration = real_seed_random_range(
				effect_get_random_seed(effect->definition_index),
				event->delay_lower_bound,
				event->delay_upper_bound);
		}
	}

	return;
}

static void effect_build_locations(
	struct effect_datum *effect,
	short (*get_markers_by_name)(long object_index, char const *name, struct object_marker *markers, short maximum_marker_count))
{
	struct object_marker markers[MAXIMUM_EFFECT_OBJECT_MARKERS];
	short location_index;
	struct effect_definition const *definition = effect_definition_get(effect->definition_index);

	for (location_index = 0; location_index < definition->locations.count; location_index++)
	{
		short marker_index;
		struct effect_location_definition const *location = TAG_BLOCK_GET_ELEMENT(&definition->locations, location_index, struct effect_location_definition);
		short marker_count = get_markers_by_name(effect->object_index, location->marker_name, markers, NUMBEROF(markers));

		for (marker_index = 0; marker_index < marker_count; marker_index++)
		{
			if (effect_build_location(effect, &markers[marker_index], location_index, get_markers_by_name == first_person_weapon_get_marker_by_name) == NONE)
			{
				break;
			}
		}
	}

	return;
}

static long effect_build_location(
	struct effect_datum *effect,
	struct object_marker const *marker,
	short location_index,
	boolean first_person_weapon)
{
	long location_datum_index = datum_new(effect_location_data);

	if (location_datum_index != NONE)
	{
		long node_designator;
		struct effect_location_datum *location = effect_location_get(location_datum_index);

		if (marker->node_index == NONE)
		{
			node_designator = NONE;
		}
		else if (first_person_weapon)
		{
			node_designator = marker->node_index | FLAG(_effect_location_first_person_bit);
		}
		else
		{
			node_designator = marker->node_index & MASK(_effect_location_first_person_bit);
		}

		location->node_designator = (short)node_designator;
		location->matrix = marker->node_matrix;
		location->next_instance_location_index = effect->location_datum_indices[location_index];
		effect->location_datum_indices[location_index] = location_datum_index;
	}

	return location_datum_index;
}

static short effect_marker_list_get_markers_by_name(
	long object_index,
	char const *name,
	struct object_marker *results,
	short maximum_count)
{
	short count = 0;

	if (global_effect_marker_list->names && strlen(name))
	{
		short marker_index;

		for (marker_index = 0; count < maximum_count && marker_index < global_effect_marker_list->count; marker_index++)
		{
			if (!strcmp(name, global_effect_marker_list->names[marker_index]))
			{
				struct object_marker *result = &results[count++];

				effect_marker_list_get_marker(result, global_effect_marker_list, marker_index);
			}
		}
	}

	if (!count)
	{
		effect_marker_list_get_marker(results, global_effect_marker_list, 0);
		count = 1;
	}

	return count;
}

static void effect_marker_list_get_marker(
	struct object_marker *result,
	struct effect_marker_list const *marker_list,
	short effect_marker_index)
{
	real_point3d point;
	real_vector3d forward;
	real_vector3d up;

	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 1120, effect_marker_index>=0 && effect_marker_index<marker_list->count);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\effects\\effects.c", 1121, &marker_list->forwards[effect_marker_index]);
	result->node_index = marker_list->node_index;

	if (marker_list->node_matrix)
	{
		matrix4x3_inverse_transform_point(marker_list->node_matrix, &marker_list->points[effect_marker_index], &point);
		matrix4x3_inverse_transform_normal(
			marker_list->node_matrix,
			&marker_list->forwards[effect_marker_index],
			&forward);
	}
	else
	{
		point = marker_list->points[effect_marker_index];
		forward = marker_list->forwards[effect_marker_index];
	}

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\effects\\effects.c", 1136, &forward);
	perpendicular3d(&forward, &up);
	normalize3d(&up);
	matrix4x3_from_point_and_vectors(&result->node_matrix, &point, &forward, &up);

	return;
}

static void effect_update(
	long effect_index,
	real dt)
{
	boolean visible;
	short event_count;
	struct effect_datum *effect = effect_get(effect_index);
	struct effect_definition const *definition = effect_definition_get(effect->definition_index);

	if (effect->object_index != NONE)
	{
		long parent_object_index;
		struct object_datum const *parent;
		struct object_datum *object = object_try_and_get(effect->object_index);

		if (!object)
		{
			effect_delete(effect_index);
			return;
		}

		parent_object_index = object_get_ultimate_parent(effect->object_index);
		parent = object_get(parent_object_index);

		if (TEST_FLAG(parent->object.flags, _object_connected_to_map_bit))
		{
			effect->location = parent->object.location;
			effect->velocity = parent->object.translational_velocity;
		}
		else
		{
			effect->location.cluster_index = NONE;
		}

		if (TEST_FLAG(effect->flags, _effect_loop_bit))
		{
			if (!object_get_function_value(effect->object_index, effect->scale_a_function_index, &effect->scale_a))
			{
				if (TEST_FLAG(definition->flags, _effect_deleted_when_inactive_bit))
				{
					short attachment_index;
					struct object_definition const *object_definition = object_definition_get(object->definition_index);

					for (attachment_index = 0; attachment_index < object_definition->object.attachments.count; attachment_index++)
					{
						if (object->object.attachment_indices[attachment_index] == effect_index)
						{
							object->object.attachment_indices[attachment_index] = NONE;
							break;
						}
					}

					effect_delete(effect_index);
					return;
				}
				else if (!TEST_FLAG(effect->flags, _effect_stopping_bit) && !TEST_FLAG(effect->flags, _effect_stopped_bit))
				{
					effect_stop(effect_index, FALSE);
				}
			}
			else if (TEST_FLAG(effect->flags, _effect_stopped_bit))
			{
				if (TEST_FLAG(effect->flags, _effect_delete_on_stop_bit))
				{
					effect_delete(effect_index);
				}
				else
				{
					SET_FLAG(effect->flags, _effect_stopped_bit, FALSE);
					effect_set_event(effect_index, 0);
				}
			}

			object_get_function_value(effect->object_index, effect->scale_b_function_index, &effect->scale_b);

			if (effect->change_color_index != NONE)
			{
				effect->color = object->object.outgoing_change_colors[effect->change_color_index];
				match_assert_valid_real_rgb_color("c:\\halo\\SOURCE\\effects\\effects.c", 1227, &effect->color);
			}
		}
	}

	visible = FALSE;

	if (effect->location.cluster_index != NONE)
	{
		visible = TEST_FLAG(definition->flags, _effect_must_be_deterministic_bit) ?
			scenario_location_potentially_visible(&effect->location) :
			scenario_location_potentially_visible_local(&effect->location);
	}

	if (!visible)
	{
		if (!TEST_FLAG(effect->flags, _effect_invisible_bit))
		{
			if (TEST_FLAG(effect->flags, _effect_loop_bit))
			{
				SET_FLAG(effect->flags, _effect_invisible_bit, TRUE);
			}
			else
			{
				effect_delete(effect_index);
				return;
			}
		}
	}
	else if (TEST_FLAG(effect->flags, _effect_invisible_bit))
	{
		SET_FLAG(effect->flags, _effect_invisible_bit, FALSE);
	}

	for (event_count = 0; dt >= 0.f; event_count++)
	{
		boolean event_completed;

		if (TEST_FLAG(effect->flags, _effect_stopped_bit) || event_count >= MAXIMUM_EFFECT_EVENTS_PER_UPDATE)
		{
			break;
		}

		if (effect->event_duration - effect->event_time <= dt)
		{
			dt -= effect->event_duration - effect->event_time;
			event_completed = TRUE;
			effect->event_time = effect->event_duration;
		}
		else
		{
			effect->event_time += dt;
			event_completed = FALSE;
			dt = -1.f;
		}

		if (TEST_FLAG(effect->flags, _effect_in_duration_bit))
		{
			short next_event_index;

			if (!TEST_FLAG(effect->flags, _effect_invisible_bit))
			{
				effect_generate_particles(effect);
			}

			if (event_completed)
			{
				if (TEST_FLAG(effect->flags, _effect_loop_bit) && effect->event_index == definition->loop_stop_index && definition->loop_start_index != NONE)
				{
					next_event_index = definition->loop_start_index;
				}
				else
				{
					next_event_index = effect->event_index + 1;
				}

				while (next_event_index < definition->events.count &&
					real_seed_random(effect_get_random_seed(effect->definition_index)) < TAG_BLOCK_GET_ELEMENT(&definition->events, next_event_index, struct effect_event_definition)->skip_fraction)
				{
					next_event_index++;
				}

				if (next_event_index >= definition->events.count)
				{
					if (TEST_FLAG(effect->flags, _effect_loop_bit))
					{
						SET_FLAG(effect->flags, _effect_stopped_bit, TRUE);
					}
					else
					{
						effect_delete(effect_index);
					}

					return;
				}

				effect_set_event(effect_index, next_event_index);
			}
		}
		else if (event_completed)
		{
			short particle_index;
			struct effect_event_definition const *event = TAG_BLOCK_GET_ELEMENT(&definition->events, effect->event_index, struct effect_event_definition);

			SET_FLAG(effect->flags, _effect_in_duration_bit, TRUE);
			effect->event_time = 0.f;
			effect->last_event_fraction = -1.f;
			effect->event_duration = real_seed_random_range(
				effect_get_random_seed(effect->definition_index),
				event->duration_lower_bound,
				event->duration_upper_bound);

			for (particle_index = 0; particle_index < event->particles.count; particle_index++)
			{
				struct effect_particles_definition const *particles = TAG_BLOCK_GET_ELEMENT(&event->particles, particle_index, struct effect_particles_definition);
				byte count = (byte)effect_real_random_range(
					get_global_local_random_seed_address(),
					effect,
					particles->count_lower_bound,
					particles->count_upper_bound,
					particles->a_scales,
					particles->b_scales,
					_effect_particle_count_bit);

				effect->particles_counts[particle_index] = count;

				if (count > MAXIMUM_SPLIT_SCREEN_PARTICLE_COUNT)
				{
					effect->particles_counts[particle_index] = (byte)(((real)count - MAXIMUM_SPLIT_SCREEN_PARTICLE_COUNT) / local_player_count() + MAXIMUM_SPLIT_SCREEN_PARTICLE_COUNT);
				}
			}

			if (!TEST_FLAG(effect->flags, _effect_invisible_bit))
			{
				effect_generate_parts(effect);
			}
		}
	}

	return;
}

static void effect_generate_particles(
	struct effect_datum *effect)
{
	short particle_index;
	struct effect_definition const *definition = effect_definition_get(effect->definition_index);
	struct effect_event_definition const *event = TAG_BLOCK_GET_ELEMENT(&definition->events, effect->event_index, struct effect_event_definition);
	real last_event_fraction = effect->last_event_fraction;
	real event_fraction = effect->event_duration > 0.f ? effect->event_time / effect->event_duration : 1.f;

	for (particle_index = 0; particle_index < event->particles.count; particle_index++)
	{
		struct effect_particles_definition const *particles = TAG_BLOCK_GET_ELEMENT(&event->particles, particle_index, struct effect_particles_definition);

		if (particles->location_index >= 0 &&
			particles->location_index < definition->locations.count &&
			effect_part_allowed_by_disposition(TEST_FLAG(effect->flags, _effect_nonviolent_bit), particles->disposition))
		{
			long last_particle_count = (long)(effect_evaluate_function_integral(particles->distribution_function, last_event_fraction) * effect->particles_counts[particle_index]);
			short count_delta = (short)((long)(effect_evaluate_function_integral(particles->distribution_function, event_fraction) * effect->particles_counts[particle_index]) - last_particle_count);

			match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 1411, count_delta>=0);

			if (count_delta > 0)
			{
				struct effect_location_datum const *location;
				long location_datum_index = effect->location_datum_indices[particles->location_index];

				while (location = effect_location_get_next_instance(effect, &location_datum_index, particles->camera_mode))
				{
					short count = count_delta;

					while (count-- > 0)
					{
						struct new_particle_data data;
						real_vector3d relative_offset;
						real_vector3d random_offset;
						real_point3d world_point;
						real_vector3d world_direction;
						real_vector3d world_velocity;
						real tint_interpolation;
						real distribution_radius = effect_real_random_range(
							get_global_local_random_seed_address(),
							effect,
							particles->distribution_radius_lower_bound,
							particles->distribution_radius_upper_bound,
							particles->a_scales,
							particles->b_scales,
							_effect_particle_distribution_radius_bit);

						local_random_direction3d(&random_offset);
						matrix4x3_transform_vector(&location->matrix, &particles->offset, &relative_offset);
						data.position.x = location->matrix.position.x + relative_offset.i + random_offset.i * distribution_radius;
						data.position.y = location->matrix.position.y + relative_offset.j + random_offset.j * distribution_radius;
						data.position.z = location->matrix.position.z + relative_offset.k + random_offset.k * distribution_radius;
						effect_random_translational_velocity(
							get_global_local_random_seed_address(),
							effect,
							&particles->runtime_direction,
							&data.direction,
							&data.velocity,
							particles->velocity_lower_bound,
							particles->velocity_upper_bound,
							particles->velocity_cone_angle,
							particles->a_scales,
							particles->b_scales);
						matrix4x3_transform_normal(&location->matrix, &data.direction, &data.direction);
						matrix4x3_transform_vector(&location->matrix, &data.velocity, &data.velocity);

						if (location->node_designator != NONE)
						{
							real_matrix4x3 const *node_matrix = effect_get_node_matrix(effect, location->node_designator);

							matrix4x3_transform_point(node_matrix, &data.position, &world_point);
							matrix4x3_transform_normal(node_matrix, &data.direction, &world_direction);
							matrix4x3_transform_vector(node_matrix, &data.velocity, &world_velocity);
						}
						else
						{
							world_point = data.position;
							world_direction = data.direction;
							world_velocity = data.velocity;
						}

						if (effect_allowed_by_environment(particles->environment, &effect->location, &world_point))
						{
							data.definition_index = particles->particle.index;

							if (TEST_FLAG(particles->flags, _effect_particle_attached_bit))
							{
								data.object_index = effect->object_index;
								data.node_index = location->node_designator == NONE ? NONE : location->node_designator & MASK(_effect_location_first_person_bit);
								data.initial_impulse = *global_zero_vector3d;
							}
							else
							{
								if (effect->impulse_field.translational)
								{
									effect->impulse_field.translational(
										&data.initial_impulse,
										&world_point,
										effect->impulse_field.identifier);
								}
								else
								{
									data.initial_impulse = *global_zero_vector3d;
								}

								data.object_index = NONE;
								data.position = world_point;
								data.direction = world_direction;
								data.velocity.i = effect->velocity.i * TICKS_PER_SECOND + world_velocity.i;
								data.velocity.j = effect->velocity.j * TICKS_PER_SECOND + world_velocity.j;
								data.velocity.k = effect->velocity.k * TICKS_PER_SECOND + world_velocity.k;
							}

							data.radius = effect_real_random_range(
								get_global_local_random_seed_address(),
								effect,
								particles->radius_lower_bound,
								particles->radius_upper_bound,
								particles->a_scales,
								particles->b_scales,
								_effect_particle_radius_bit);
							data.angular_velocity = effect_real_random_range(
								get_global_local_random_seed_address(),
								effect,
								particles->angular_velocity_lower_bound,
								particles->angular_velocity_upper_bound,
								particles->a_scales,
								particles->b_scales,
								_effect_angular_velocity_bit);
							data.rotation = TEST_FLAG(particles->flags, _effect_particle_random_orientation_bit) ? real_local_random_range(0.f, _pi * 2.f) : 0.f;

							if (TEST_FLAG(particles->a_scales, _effect_particle_tint_bit) || TEST_FLAG(particles->b_scales, _effect_particle_tint_bit))
							{
								tint_interpolation = effect_scale(
									effect,
									1.f,
									particles->a_scales,
									particles->b_scales,
									_effect_particle_tint_bit);
							}
							else
							{
								tint_interpolation = real_local_random();
							}

							rgb_colors_interpolate(
								&data.color.rgb,
								(short)((particles->flags >> _effect_particle_tint_interpolate_hsv_bit) & MASK(2)),
								&particles->tint_lower_bound.rgb,
								&particles->tint_upper_bound.rgb,
								tint_interpolation);
							data.color.alpha = (1.f - tint_interpolation) * particles->tint_lower_bound.alpha + tint_interpolation * particles->tint_upper_bound.alpha;

							if (TEST_FLAG(particles->flags, _effect_particle_tint_from_change_color_bit))
							{
								data.color.red *= effect->color.red;
								data.color.green *= effect->color.green;
								data.color.blue *= effect->color.blue;
							}

							data.local_player_index = effect->local_player_index;
							data.attached_to_local_player = location->node_designator != NONE && TEST_FLAG(location->node_designator, _effect_location_first_person_bit);
							data.dont_draw_first_person = particles->camera_mode == _effect_camera_mode_third_person_only;
							data.dont_draw_third_person = particles->camera_mode == _effect_camera_mode_first_person_only;
							particle_new(&data);
						}
					}
				}
			}
		}
	}

	effect->last_event_fraction = event_fraction;

	return;
}

static void effect_generate_parts(
	struct effect_datum *effect)
{
	short part_index;
	struct effect_definition const *definition = effect_definition_get(effect->definition_index);
	struct effect_event_definition const *event = TAG_BLOCK_GET_ELEMENT(&definition->events, effect->event_index, struct effect_event_definition);

	for (part_index = 0; part_index < event->parts.count; part_index++)
	{
		struct effect_part_definition const *part_definition = TAG_BLOCK_GET_ELEMENT(&event->parts, part_index, struct effect_part_definition);

		if (part_definition->location_index >= 0 &&
			part_definition->location_index < definition->locations.count &&
			part_definition->reference.index != NONE &&
			effect_part_allowed_by_disposition(TEST_FLAG(effect->flags, _effect_nonviolent_bit), part_definition->disposition))
		{
			struct effect_location_datum const *location;
			long location_datum_index = effect->location_datum_indices[part_definition->location_index];

			while (location = effect_location_get_next_instance(effect, &location_datum_index, _effect_camera_mode_independent))
			{
				real_point3d world_point;
				real_vector3d world_forward;
				real_vector3d world_up;

				if (location->node_designator != NONE)
				{
					real_matrix4x3 const *node_matrix = effect_get_node_matrix(effect, location->node_designator);

					matrix4x3_transform_point(node_matrix, &location->matrix.position, &world_point);
					matrix4x3_transform_normal(node_matrix, &location->matrix.forward, &world_forward);
					matrix4x3_transform_normal(node_matrix, &location->matrix.up, &world_up);
				}
				else
				{
					world_point = location->matrix.position;
					world_forward = location->matrix.forward;
					world_up = location->matrix.up;
				}

				if (TEST_FLAG(part_definition->flags, _effect_part_definition_world_down_bit))
				{
					world_forward = *global_down3d;
					world_up = *global_forward3d;
				}

				if (effect_allowed_by_environment(part_definition->environment, &effect->location, &world_point))
				{
					effect_generate_part(
						effect,
						part_definition,
						location,
						&world_point,
						&world_forward,
						&world_up,
						effect_scale(effect, 1.f, part_definition->a_scales, part_definition->b_scales, _effect_part_type_scale_bit));
				}
			}
		}
	}

	return;
}

static void effect_generate_part(
	struct effect_datum const *effect,
	struct effect_part_definition const *part_definition,
	struct effect_location_datum const *location,
	real_point3d const *world_point,
	real_vector3d const *world_forward,
	real_vector3d const *world_up,
	real scale)
{
	real_vector3d direction;
	real_vector3d velocity;

	switch (part_definition->runtime_tag_reference_base_class_tag)
	{
	case PARTICLE_SYSTEM_DEFINITION_TAG:
	{
		real_argb_color tint;

		tint.alpha = 1.f;
		tint.rgb = effect->color;
		effect_random_translational_velocity(
			get_global_local_random_seed_address(),
			effect,
			world_forward,
			&direction,
			&velocity,
			part_definition->velocity_lower_bound,
			part_definition->velocity_upper_bound,
			part_definition->velocity_cone_angle,
			part_definition->a_scales,
			part_definition->b_scales);
		add_vectors3d(&velocity, &effect->velocity, &velocity);
		particle_system_new_unattached(part_definition->reference.index, world_point, &velocity, &tint, scale);
		break;
	}
	case SOUND_DEFINITION_TAG:
		if (effect->object_index != NONE)
		{
			object_impulse_sound_new(
				effect->object_index,
				part_definition->reference.index,
				location->node_designator == NONE ? NONE : location->node_designator & MASK(_effect_location_first_person_bit),
				&location->matrix.position,
				&location->matrix.forward,
				scale);
		}
		else
		{
			struct sound_location sound_location;

			sound_location.position = *world_point;
			sound_location.forward = *world_forward;
			sound_location.translational_velocity = *global_zero_vector3d;
			sound_location.game_location = effect->location;
			unattached_impulse_sound_new(part_definition->reference.index, &sound_location, scale);
		}

		break;
	case OBJECT_DEFINITION_TAG:
	{
		struct object_placement_data data;

		object_placement_data_new(&data, part_definition->reference.index, effect->owner_object_index);
		data.position = *world_point;
		data.forward = *world_forward;
		data.up = *world_up;
		effect_random_translational_velocity(
			get_global_random_seed_address(),
			effect,
			world_forward,
			&direction,
			&data.translational_velocity,
			part_definition->velocity_lower_bound,
			part_definition->velocity_upper_bound,
			part_definition->velocity_cone_angle,
			part_definition->a_scales,
			part_definition->b_scales);
		add_vectors3d(&data.translational_velocity, &effect->velocity, &data.translational_velocity);
		effect_random_angular_velocity(
			get_global_random_seed_address(),
			effect,
			&data.angular_velocity,
			part_definition->angular_velocity_lower_bound,
			part_definition->angular_velocity_upper_bound,
			part_definition->a_scales,
			part_definition->b_scales);
		object_new(&data);
		break;
	}
	case DECAL_DEFINITION_TAG:
	{
		real radius_modifier;

		effect_random_translational_velocity(
			get_global_local_random_seed_address(),
			effect,
			world_forward,
			&direction,
			&velocity,
			part_definition->velocity_lower_bound,
			part_definition->velocity_upper_bound,
			part_definition->velocity_cone_angle,
			part_definition->a_scales,
			part_definition->b_scales);
		radius_modifier = real_local_random_range(
			part_definition->radius_modifier_lower_bound,
			part_definition->radius_modifier_upper_bound);
		decal_new(part_definition->reference.index, world_point, &velocity, radius_modifier, FALSE, NONE, NULL);
		break;
	}
	case DAMAGE_EFFECT_DEFINITION_TAG:
	{
		struct damage_data damage_data;
		struct object_datum const *owner = object_try_and_get(effect->owner_object_index);

		damage_data_new(&damage_data, part_definition->reference.index);

		if (owner)
		{
			damage_data.owner_player_index = owner->object.owner_player_index;
			damage_data.owner_object_index = effect->owner_object_index;
			damage_data.owner_team_index = owner->object.owner_team_index;
		}

		damage_data.scale = scale;
		damage_data.location = effect->location;
		damage_data.epicenter = *world_point;
		damage_data.origin = *world_point;
		damage_data.direction = *world_forward;
		area_of_effect_cause_damage(&damage_data, NONE);
		break;
	}
	case LIGHT_DEFINITION_TAG:
		light_new_unattached(
			part_definition->reference.index,
			effect->object_index,
			location->node_designator == NONE ? NONE : location->node_designator & MASK(_effect_location_first_person_bit),
			&location->matrix.position,
			&location->matrix.forward,
			scale);
		break;
	default:
		match_vhalt("c:\\halo\\SOURCE\\effects\\effects.c", 1753, csprintf("effect %s has a bad part %s", tag_get_name(effect->definition_index), part_definition->reference.name));
		break;
	}

	return;
}

static struct effect_location_datum *effect_location_get_next_instance(
	struct effect_datum const *effect,
	long *location_datum_index,
	short camera_mode)
{
	struct effect_location_datum *location = NULL;

	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 1766, effect);
	match_assert("c:\\halo\\SOURCE\\effects\\effects.c", 1767, location_datum_index);

	if (*location_datum_index != NONE)
	{
		location = effect_location_get(*location_datum_index);
		*location_datum_index = location->next_instance_location_index;

		if (camera_mode == _effect_camera_mode_first_person_only || camera_mode == _effect_camera_mode_both && effect->local_player_index != NONE && local_player_count() == 1)
		{
			if (location->node_designator == NONE || !TEST_FLAG(location->node_designator, _effect_location_first_person_bit))
			{
				location = effect_location_get_next_instance(effect, location_datum_index, camera_mode);
			}
		}
		else if (location->node_designator != NONE && TEST_FLAG(location->node_designator, _effect_location_first_person_bit))
		{
			location = effect_location_get_next_instance(effect, location_datum_index, camera_mode);
		}
	}

	return location;
}

static real_matrix4x3 *effect_get_node_matrix(
	struct effect_datum const *effect,
	short node_designator)
{
	if (node_designator != NONE && TEST_FLAG((word)node_designator, _effect_location_first_person_bit))
	{
		return first_person_weapon_get_node_matrix(
			effect->local_player_index,
			node_designator == NONE ? NONE : node_designator & MASK(_effect_location_first_person_bit));
	}
	else
	{
		return object_get_node_matrix(
			effect->object_index,
			node_designator == NONE ? NONE : node_designator & MASK(_effect_location_first_person_bit));
	}
}

static real effect_evaluate_function_integral(
	short function,
	real fraction)
{
	real integral;

	if (fraction == -1.f)
	{
		return 0.f;
	}

	switch (function)
	{
	case _effect_particle_distribution_function_start:
		integral = 1.f;
		break;
	case _effect_particle_distribution_function_end:
		integral = fraction < 1.f ? 0.f : 1.f;
		break;
	case _effect_particle_distribution_function_constant:
		integral = fraction;
		break;
	case _effect_particle_distribution_function_buildup:
		integral = fraction * fraction;
		break;
	case _effect_particle_distribution_function_falloff:
		integral = (2.f - fraction) * fraction;
		break;
	case _effect_particle_distribution_function_quadratic:
		integral = (3.f - (fraction + fraction)) * fraction * fraction;
		break;
	default:
		match_halt("c:\\halo\\SOURCE\\effects\\effects.c", 1886);
		break;
	}

	return integral;
}

/* ---------- private code */
