/*
PARTICLE_SYSTEMS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "particle_systems.h"
#include "particle_system_definitions.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "rasterizer_console_vars.h"
#include "game_state.h"
#include "render_debug.h"
#include "physics_constants.h"
#include "object_lights.h"
#include "physics.h"
#include "first_person_weapons.h"
#include "point_physics.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

typedef void (*particle_system_update_proc)(struct particle_system_datum *system, real dtime);
typedef void (*particle_system_particle_creation_proc)(struct particle_system_datum const *system, short type_index, struct ps_particle_datum *particle, struct object_marker const *marker);
typedef void (*particle_system_particle_update_proc)(struct particle_system_datum const *system, short type_index, real dtime, struct ps_particle_datum *particle);

/* ---------- prototypes */

static boolean particle_system_initialize(long new_system_index);
static void particle_system_new_particles(struct particle_system_datum *system, short type_index, real dt);
static void particle_system_delete(long system_index);
static void randomize_particle_variables(struct particle_system_type const *type_definition, struct particle_system_type_particle_state_interpolated_randomized_variables *variables, short state_index);
static void particle_system_next_type_state_index(struct particle_system_datum const *system, struct particle_type *type, struct particle_system_type const *type_definition);
static void particle_system_next_particle_state_index(struct ps_particle_datum *particle, struct particle_system_type const *type_definition);
static void particle_system_update(real dtime, long system_index);
static void particle_system_render(long system_index);
static void particle_system_update_default(struct particle_system_datum *system, real dtime);
static void particle_system_new_particle_default(struct particle_system_datum const *system, short type_index, struct ps_particle_datum *particle, struct object_marker const *marker);
static void particle_system_update_particle_default(struct particle_system_datum const *system, short type_index, real dtime, struct ps_particle_datum *particle);
static void particle_system_update_explosion(struct particle_system_datum *system, real dtime);
static void particle_system_new_particle_explosion(struct particle_system_datum const *system, short type_index, struct ps_particle_datum *particle, struct object_marker const *marker);
static void particle_system_new_particle_jet(struct particle_system_datum const *system, short type_index, struct ps_particle_datum *particle, struct object_marker const *marker);

/* ---------- globals */

static particle_system_update_proc const system_update_functions[NUMBER_OF_PARTICLE_SYSTEM_UPDATE_PHYSICS] =
{
	particle_system_update_default,
	particle_system_update_explosion
};

static particle_system_particle_creation_proc const particle_creation_functions[NUMBER_OF_PARTICLE_SYSTEM_TYPE_CREATION_PHYSICS] =
{
	particle_system_new_particle_default,
	particle_system_new_particle_explosion,
	particle_system_new_particle_jet
};

static particle_system_particle_update_proc const particle_update_functions[NUMBER_OF_PARTICLE_SYSTEM_TYPE_UPDATE_PHYSICS] =
{
	particle_system_update_particle_default
};

real const ground_error = 0.05f;
static real const seconds_per_tick = 1.f / TICKS_PER_SECOND;

struct data_array *particle_systems;
struct data_array *system_particles;

/* ---------- public code */

void particle_systems_initialize(
	void)
{
	particle_systems = game_state_data_new("particle systems", MAXIMUM_NUMBER_OF_PARTICLE_SYSTEMS_PER_MAP, sizeof(struct particle_system_datum));
	system_particles = game_state_data_new("particle system particles", MAXIMUM_NUMBER_OF_SYSTEM_PARTICLES_PER_MAP, sizeof(struct ps_particle_datum));

	return;
}

void particle_systems_initialize_for_new_map(
	void)
{
	data_make_valid(particle_systems);
	data_make_valid(system_particles);

	return;
}

void particle_systems_dispose_from_old_map(
	void)
{
	if (particle_systems && particle_systems->valid)
	{
		long system_index;

		for (system_index = data_next_index(particle_systems, NONE); system_index != NONE; system_index = data_next_index(particle_systems, system_index))
		{
			particle_system_delete(system_index);
		}

		data_make_invalid(particle_systems);
		data_make_invalid(system_particles);
	}

	return;
}

long particle_system_new_unattached(
	long definition_index,
	real_point3d const *position,
	real_vector3d const *velocity,
	real_argb_color const *color,
	real scale)
{
	long system_index = datum_new(particle_systems);

	if (system_index != NONE)
	{
		real_rgb_color unused;
		struct particle_system_datum *system = particle_system_get(system_index);

		system->definition_index = definition_index;
		system->object_index = NONE;
		system->position = *position;
		system->velocity = *velocity;
		system->color = *color;
		system->scale = scale;
		SET_FLAG(system->flags, _particle_system_active_bit, TRUE);
		light_particle(&system->position, &system->lighting, &unused, FALSE);

		if (!particle_system_initialize(system_index))
		{
			datum_delete(particle_systems, system_index);
			system_index = NONE;
		}
	}

	return system_index;
}

long particle_system_new_attached(
	long definition_index,
	long object_index,
	short attachment_index)
{
	long system_index = datum_new(particle_systems);

	if (system_index != NONE)
	{
		struct object_marker marker;
		struct particle_system_datum *system = particle_system_get(system_index);
		struct object_datum const *object = object_get(object_index);
		struct object_attachment_definition const *attachment = TAG_BLOCK_GET_ELEMENT(&object_definition_get(object->definition_index)->object.attachments, attachment_index, struct object_attachment_definition);

		system->definition_index = definition_index;
		system->object_index = object_index;
		system->attachment_index = attachment_index;
		system->function_index = attachment->primary_scale_function_reference - 1;

		if (attachment->change_color_reference)
		{
			system->color.rgb = object->object.outgoing_change_colors[attachment->change_color_reference];
			system->color.alpha = 1.f;
		}
		else
		{
			system->color = *global_real_argb_white;
		}

		object_get_marker_by_name(object_index, attachment->marker_name, &marker, 1);
		system->position = marker.matrix.position;
		object_get_velocities(object_index, &system->velocity, NULL);
		system->velocity.i *= TICKS_PER_SECOND;
		system->velocity.j *= TICKS_PER_SECOND;
		system->velocity.k *= TICKS_PER_SECOND;
		system->lighting = *global_real_rgb_white;
		SET_FLAG(system->flags, _particle_system_active_bit, object_get_function_value(object_index, system->function_index, &system->scale));

		if (!particle_system_initialize(system_index))
		{
			datum_delete(particle_systems, system_index);
			system_index = NONE;
		}
	}

	return system_index;
}

static boolean particle_system_initialize(
	long new_system_index)
{
	short type_index;
	struct particle_system_datum *system = particle_system_get(new_system_index);
	struct particle_system_definition const *definition = particle_system_definition_get(system->definition_index);
	boolean success = TRUE;

	scenario_location_from_point(&system->location, &system->position);
	SET_FLAG(system->flags, _particle_system_initializing_bit, TRUE);

	for (type_index = 0; type_index < definition->types.count; type_index++)
	{
		struct particle_type *type = &system->types[type_index];
		struct particle_system_type const *type_definition = TAG_BLOCK_GET_ELEMENT(&definition->types, type_index, struct particle_system_type);

		if (type_definition->type_states.count)
		{
			type->state_index = 0;
			type->transition_state_index = NONE;
			type->states_moving_forward = TRUE;
			type->particle_count = 0;
			type->first_particle_index = NONE;

			if (type_definition->type_states.count > 0)
			{
				struct particle_system_type_state const *state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->type_states, type->state_index, struct particle_system_type_state);
				real duration = real_local_random_range(state_definition->duration_lower_bound, state_definition->duration_upper_bound);

				type->time_left_in_state = duration;
				type->state_length = duration;
			}
		}
		else
		{
			success = FALSE;
		}
	}

	if (success)
	{
		particle_system_update(0.001f, new_system_index);
	}

	return success;
}

void particle_system_orphan(
	long system_index)
{
	struct particle_system_datum *system = particle_system_get(system_index);

	SET_FLAG(system->flags, _particle_system_active_bit, FALSE);
	system->object_index = NONE;

	return;
}

static void particle_system_new_particles(
	struct particle_system_datum *system,
	short type_index,
	real dt)
{
	short desired_particle_count;
	struct particle_system_definition const *definition = particle_system_definition_get(system->definition_index);
	struct particle_type *type = &system->types[type_index];
	struct particle_system_type const *type_definition = TAG_BLOCK_GET_ELEMENT(&definition->types, type_index, struct particle_system_type);
	boolean initializing = TEST_FLAG(system->flags, _particle_system_initializing_bit);
	struct particle_system_type_state const *state_definition = initializing ? NULL : TAG_BLOCK_GET_ELEMENT(&type_definition->type_states, type->state_index, struct particle_system_type_state);

	if (initializing)
	{
		if (TEST_FLAG(type_definition->flags, _particle_system_type_initial_count_scales_bit))
		{
			desired_particle_count = (short)(type_definition->initial_particle_count * system->scale + 0.5f);
		}
		else
		{
			desired_particle_count = type_definition->initial_particle_count;
		}
	}
	else
	{
		real new_particle_count = dt * type->variables.particle_creation_rate;
		short whole_particle_count = (short)new_particle_count;

		desired_particle_count = type->particle_count + whole_particle_count;
		type->fractional_particle_count += new_particle_count - whole_particle_count;

		if (type->fractional_particle_count > 1.f)
		{
			desired_particle_count++;
			type->fractional_particle_count -= 1.f;
		}
	}

	if (type->particle_count < desired_particle_count)
	{
		struct object_marker markers[8];
		short marker_count;

		if (system->object_index != NONE)
		{
			struct object_datum const *object = object_get(system->object_index);

			marker_count = object_get_marker_by_name(
				system->object_index,
				TAG_BLOCK_GET_ELEMENT(&object_definition_get(object->definition_index)->object.attachments, system->attachment_index, struct object_attachment_definition)->marker_name,
				markers,
				NUMBEROF(markers));
			object_get_location(system->object_index, &system->location);
		}
		else
		{
			marker_count = 1;
			markers[0].matrix.position = system->position;
			markers[0].matrix.forward = *global_zero_vector3d;
		}

		if (system->location.cluster_index != NONE)
		{
			short attempt_count = 0;

			while (type->particle_count < desired_particle_count)
			{
				long particle_index;
				struct ps_particle_datum *particle;
				short creation_function_index;
				short marker_index;
				real rotation;

				if (!marker_count || attempt_count >= MAXIMUM_NUMBER_OF_PARTICLE_SYSTEM_CREATION_ATTEMPTS)
				{
					break;
				}

				particle_index = datum_new(system_particles);

				if (particle_index == NONE)
				{
					break;
				}

				particle = system_particle_get(particle_index);
				creation_function_index = initializing ? type_definition->initial_particle_creation_physics : state_definition->particle_creation_physics;
				match_assert("c:\\halo\\SOURCE\\effects\\particle_systems.c", 476, particle);
				particle->valid = TRUE;
				particle->state_index = NONE;
				particle->transition_state_index = NONE;
				particle->states_moving_forward = TRUE;
				particle->sprite_index = -1.f;
				rotation = real_local_random_range(0.f, _pi * 2.f);
				particle->rotation = rotation;
				match_assert("c:\\halo\\SOURCE\\effects\\particle_systems.c", 488, creation_function_index>=0 && creation_function_index<NUMBER_OF_PARTICLE_SYSTEM_TYPE_CREATION_PHYSICS);
				marker_index = local_random_range(0, marker_count);
				particle_creation_functions[creation_function_index](system, type_index, particle, &markers[marker_index]);
				scenario_location_from_point(&particle->location, &particle->position);

				if (particle->location.cluster_index != NONE)
				{
					type->particle_count++;
					particle->next_particle_index = type->first_particle_index;
					type->first_particle_index = particle_index;
				}
				else
				{
					datum_delete(system_particles, particle_index);
				}

				attempt_count++;
			}
		}
	}

	if (type->particle_count < type->variables.minimum_particle_count)
	{
		type->time_left_in_state *= 0.3f;
	}

	return;
}

static void particle_system_delete(
	long system_index)
{
	short type_index;
	struct particle_system_datum *system = particle_system_get(system_index);
	struct particle_system_definition const *definition = particle_system_definition_get(system->definition_index);

	for (type_index = 0; type_index < definition->types.count; type_index++)
	{
		struct particle_type *type = &system->types[type_index];
		long particle_index = type->first_particle_index;

		while (particle_index != NONE)
		{
			long next_particle_index = system_particle_get(particle_index)->next_particle_index;

			datum_delete(system_particles, particle_index);
			particle_index = next_particle_index;
		}
	}

	datum_delete(particle_systems, system_index);

	return;
}

void particle_systems_dispose(
	void)
{
	return;
}

void particle_systems_update(
	real dtime)
{
	long system_index;

	match_assert("c:\\halo\\SOURCE\\effects\\particle_systems.c", 562, particle_systems && particle_systems->valid);

	for (system_index = data_next_index(particle_systems, NONE); system_index != NONE; system_index = data_next_index(particle_systems, system_index))
	{
		particle_system_update(dtime, system_index);
	}

	return;
}

void particle_systems_disconnect_from_structure_bsp(
	void)
{
	return;
}

void particle_systems_reconnect_to_structure_bsp(
	void)
{
	long system_index;

	for (system_index = data_next_index(particle_systems, NONE); system_index != NONE; system_index = data_next_index(particle_systems, system_index))
	{
		short type_index;
		struct particle_system_datum *system = particle_system_get(system_index);
		struct particle_system_definition const *definition = particle_system_definition_get(system->definition_index);

		if (system->object_index != NONE)
		{
			object_get_location(system->object_index, &system->location);
		}
		else
		{
			scenario_location_from_point(&system->location, &system->position);

			if (system->location.cluster_index == NONE)
			{
				particle_system_delete(system_index);
				continue;
			}
		}

		for (type_index = 0; type_index < definition->types.count; type_index++)
		{
			long *particle_index_reference = &system->types[type_index].first_particle_index;

			while (*particle_index_reference != NONE)
			{
				struct ps_particle_datum *particle = system_particle_get(*particle_index_reference);

				scenario_location_from_point(&particle->location, &particle->position);

				if (particle->location.cluster_index == NONE)
				{
					datum_delete(system_particles, *particle_index_reference);
					*particle_index_reference = particle->next_particle_index;
				}
				else
				{
					particle_index_reference = &particle->next_particle_index;
				}
			}
		}
	}

	return;
}

static void randomize_particle_variables(
	struct particle_system_type const *type_definition,
	struct particle_system_type_particle_state_interpolated_randomized_variables *variables,
	short state_index)
{
	struct particle_system_type_particle_state const *state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->particle_states, state_index, struct particle_system_type_particle_state);
	real color_interpolation = real_local_random_range(0.f, 1.f);

	variables->animation_rate = real_local_random_range(
		state_definition->animation_rate_lower_bound,
		state_definition->animation_rate_upper_bound);
	variables->rotation_rate = real_local_random_range(
		state_definition->rotation_rate_lower_bound,
		state_definition->rotation_rate_upper_bound);
	variables->scale = real_local_random_range(
		state_definition->scale_lower_bound,
		state_definition->scale_upper_bound);
	variables->color.alpha = real_local_random_range(
		state_definition->color_lower_bound.alpha,
		state_definition->color_upper_bound.alpha);
	variables->color.red = (state_definition->color_upper_bound.red - state_definition->color_lower_bound.red) * color_interpolation + state_definition->color_lower_bound.red;
	variables->color.green = (state_definition->color_upper_bound.green - state_definition->color_lower_bound.green) * color_interpolation + state_definition->color_lower_bound.green;
	variables->color.blue = (state_definition->color_upper_bound.blue - state_definition->color_lower_bound.blue) * color_interpolation + state_definition->color_lower_bound.blue;

	return;
}

static void particle_system_next_type_state_index(
	struct particle_system_datum const *system,
	struct particle_type *type,
	struct particle_system_type const *type_definition)
{
	short direction = type->states_moving_forward ? 1 : -1;

	type->transition_state_index = type->state_index + direction;

	if (type->transition_state_index < 0 || type->transition_state_index >= type_definition->type_states.count)
	{
		if (TEST_FLAG(type_definition->flags, _particle_system_type_type_states_loop_bit) &&
			system->object_index != NONE &&
			type_definition->type_states.count > 0)
		{
			if (TEST_FLAG(type_definition->flags, _particle_system_type_type_states_loop_forward_backward_bit))
			{
				type->transition_state_index = PIN(type->state_index - direction, 0, type_definition->type_states.count - 1);
				type->states_moving_forward = !type->states_moving_forward;
			}
			else
			{
				type->transition_state_index = 0;
			}
		}
		else
		{
			type->state_index = NONE;
			type->transition_state_index = NONE;
		}
	}

	return;
}

static void particle_system_next_particle_state_index(
	struct ps_particle_datum *particle,
	struct particle_system_type const *type_definition)
{
	short direction = particle->states_moving_forward ? 1 : -1;

	particle->transition_state_index = particle->state_index + direction;

	if (particle->transition_state_index < 0 || particle->transition_state_index >= type_definition->particle_states.count)
	{
		if (TEST_FLAG(type_definition->flags, _particle_system_type_particle_states_loop_bit) &&
			type_definition->particle_states.count > 0)
		{
			if (TEST_FLAG(type_definition->flags, _particle_system_type_particle_states_loop_forward_backward_bit))
			{
				particle->transition_state_index = PIN(particle->state_index - direction, 0, type_definition->particle_states.count - 1);
				particle->states_moving_forward = !particle->states_moving_forward;
			}
			else
			{
				particle->transition_state_index = 0;
			}
		}
		else
		{
			particle->state_index = NONE;
			particle->transition_state_index = NONE;
		}
	}

	return;
}

static void particle_system_update(
	real dtime,
	long system_index)
{
	short type_index;
	struct particle_system_datum *system = particle_system_get(system_index);
	struct particle_system_definition const *system_definition = particle_system_definition_get(system->definition_index);
	short active_type_count = 0;

	if (system->object_index != NONE)
	{
		struct object_datum const *object = object_get(system->object_index);

		if (TEST_FLAG(object->object.flags, _object_connected_to_map_bit) && object_get_function_value(system->object_index, system->function_index, &system->scale))
		{
			SET_FLAG(system->flags, _particle_system_active_bit, TRUE);
		}
		else
		{
			SET_FLAG(system->flags, _particle_system_active_bit, FALSE);
		}

		object_get_origin(system->object_index, &system->position);
		object_get_velocities(system->object_index, &system->velocity, NULL);
		system->velocity.i *= TICKS_PER_SECOND;
		system->velocity.j *= TICKS_PER_SECOND;
		system->velocity.k *= TICKS_PER_SECOND;
	}

	match_assert("c:\\halo\\SOURCE\\effects\\particle_systems.c", 737, system_definition->system_update_physics>=0 && system_definition->system_update_physics<NUMBER_OF_PARTICLE_SYSTEM_UPDATE_PHYSICS);
	system_update_functions[system_definition->system_update_physics](system, dtime);

	for (type_index = 0; type_index < system_definition->types.count; type_index++)
	{
		struct particle_system_type const *type_definition = TAG_BLOCK_GET_ELEMENT(&system_definition->types, type_index, struct particle_system_type);
		struct particle_type *type = &system->types[type_index];

		if (!TEST_FLAG(type_definition->flags, _particle_system_type_disabled_bit))
		{
			type->time_left_in_state -= dtime;

			while (type->state_index != NONE)
			{
				struct particle_system_type_state const *state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->type_states, type->state_index, struct particle_system_type_state);

				if (type->time_left_in_state < 0.f)
				{
					real duration;

					if (type->transition_state_index == NONE)
					{
						particle_system_next_type_state_index(system, type, type_definition);
						duration = real_local_random_range(
							state_definition->transition_time_lower_bound,
							state_definition->transition_time_upper_bound);
					}
					else
					{
						type->state_index = type->transition_state_index;
						type->transition_state_index = NONE;
						state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->type_states, type->state_index, struct particle_system_type_state);
						duration = real_local_random_range(
							state_definition->duration_lower_bound,
							state_definition->duration_upper_bound);
					}

					type->state_length = duration;
					type->time_left_in_state += duration;
				}
				else
				{
					if (type->transition_state_index == NONE)
					{
						memcpy(&type->variables, &state_definition->variables, sizeof(type->variables));
					}
					else
					{
						short variable_index;
						struct particle_system_type_state const *transition_state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->type_states, type->transition_state_index, struct particle_system_type_state);
						real *variable = (real *)&type->variables;
						real const *state_variable = (real const *)&state_definition->variables;
						real const *transition_state_variable = (real const *)&transition_state_definition->variables;
						real t = type->time_left_in_state / type->state_length;

						t = PIN(t, 0.f, 1.f);

						for (variable_index = 0; variable_index < sizeof(type->variables) / sizeof(real); variable_index++)
						{
							*variable++ = t * *state_variable++ + (1.f - t) * *transition_state_variable++;
						}
					}

					if (TEST_FLAG(type_definition->flags, _particle_system_type_tint_by_effect_color_bit))
					{
						type->variables.particle_state_randomized_multipliers.color.alpha *= system->color.alpha;
						type->variables.particle_state_randomized_multipliers.color.red *= system->color.red;
						type->variables.particle_state_randomized_multipliers.color.green *= system->color.green;
						type->variables.particle_state_randomized_multipliers.color.blue *= system->color.blue;
					}

					if (TEST_FLAG(type_definition->flags, _particle_system_type_minimum_count_scales_bit))
					{
						type->variables.minimum_particle_count *= system->scale;
					}

					if (TEST_FLAG(type_definition->flags, _particle_system_type_creation_rate_scales_bit))
					{
						type->variables.particle_creation_rate *= system->scale;
					}

					if (TEST_FLAG(type_definition->flags, _particle_system_type_scale_scales_bit))
					{
						type->variables.particle_state_randomized_multipliers.scale *= system->scale;
					}

					if (TEST_FLAG(type_definition->flags, _particle_system_type_animation_rate_scales_bit))
					{
						type->variables.particle_state_randomized_multipliers.animation_rate *= system->scale;
					}

					if (TEST_FLAG(type_definition->flags, _particle_system_type_rotation_rate_scales_bit))
					{
						type->variables.particle_state_randomized_multipliers.rotation_rate *= system->scale;
					}

					break;
				}
			}

			if (type->state_index != NONE)
			{
				short particle_index;
				struct ps_particle_datum *previous_particle = NULL;

				if (TEST_FLAG(system->flags, _particle_system_active_bit))
				{
					particle_system_new_particles(system, type_index, dtime);
				}

				for (particle_index = (short)type->first_particle_index; particle_index != NONE; )
				{
					struct ps_particle_datum *particle = system_particle_get(particle_index);

					particle->time_left_in_state -= dtime;

					if (particle->state_index == NONE && type_definition->particle_states.count > 0)
					{
						struct particle_system_type_particle_state const *state_definition;

						particle->state_index = 0;
						state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->particle_states, 0, struct particle_system_type_particle_state);
						particle->time_left_in_state = real_local_random_range(
							state_definition->duration_lower_bound,
							state_definition->duration_upper_bound);
						particle->state_length = particle->time_left_in_state;
						randomize_particle_variables(
							type_definition,
							&particle->randomized_variables,
							particle->state_index);
					}

					if (!particle->valid)
					{
						particle->state_index = NONE;
					}

					while (particle->state_index != NONE)
					{
						struct particle_system_type_particle_state const *state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->particle_states, particle->state_index, struct particle_system_type_particle_state);

						if (particle->time_left_in_state < 0.f)
						{
							real duration;

							if (particle->transition_state_index == NONE)
							{
								particle_system_next_particle_state_index(particle, type_definition);
								duration = real_local_random_range(
									state_definition->transition_time_lower_bound,
									state_definition->transition_time_upper_bound);
							}
							else
							{
								particle->state_index = particle->transition_state_index;
								particle->transition_state_index = NONE;
								state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->particle_states, particle->state_index, struct particle_system_type_particle_state);
								duration = real_local_random_range(
									state_definition->duration_lower_bound,
									state_definition->duration_upper_bound);
							}

							particle->state_length = duration;
							particle->time_left_in_state += duration;

							if (particle->transition_state_index != NONE)
							{
								randomize_particle_variables(
									type_definition,
									&particle->transition_randomized_variables,
									particle->transition_state_index);
							}
							else
							{
								particle->randomized_variables = particle->transition_randomized_variables;
							}
						}
						else
						{
							break;
						}
					}

					if (particle->state_index == NONE)
					{
						if (previous_particle)
						{
							previous_particle->next_particle_index = particle->next_particle_index;
						}
						else
						{
							type->first_particle_index = particle->next_particle_index;
						}

						datum_delete(system_particles, particle_index);
						particle_index = (short)particle->next_particle_index;
						type->particle_count--;
					}
					else
					{
						struct particle_system_type_state const *type_state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->type_states, type->state_index, struct particle_system_type_state);

						if (particle->transition_state_index == NONE)
						{
							particle->rotation += particle->randomized_variables.rotation_rate * type->variables.particle_state_randomized_multipliers.rotation_rate * dtime;
							particle->sprite_index += particle->randomized_variables.animation_rate * type->variables.particle_state_randomized_multipliers.animation_rate * dtime;
						}
						else
						{
							struct particle_system_type_particle_state const *state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->particle_states, particle->state_index, struct particle_system_type_particle_state);
							real t = particle->time_left_in_state / particle->state_length;

							t = PIN(t, 0.f, 1.f);
							particle->rotation += ((1.f - t) * particle->transition_randomized_variables.rotation_rate + t * particle->randomized_variables.rotation_rate) * type->variables.particle_state_randomized_multipliers.rotation_rate * dtime;
							particle->sprite_index += ((1.f - t) * particle->transition_randomized_variables.animation_rate + t * particle->randomized_variables.animation_rate) * type->variables.particle_state_randomized_multipliers.animation_rate * dtime;
						}

						match_assert("c:\\halo\\SOURCE\\effects\\particle_systems.c", 943, type_state_definition->particle_update_physics>=0 && type_state_definition->particle_update_physics<NUMBER_OF_PARTICLE_SYSTEM_TYPE_UPDATE_PHYSICS);
						particle_update_functions[type_state_definition->particle_update_physics](system, type_index, dtime, particle);
						previous_particle = particle;
						particle_index = (short)particle->next_particle_index;
					}
				}

				active_type_count++;
			}
		}
	}

	SET_FLAG(system->flags, _particle_system_initializing_bit, FALSE);

	if (!active_type_count && system->object_index == NONE)
	{
		particle_system_delete(system_index);
	}

	return;
}

void particle_systems_render(
	void)
{
	if (render_particle_systems_enabled)
	{
		long system_index;

		match_assert("c:\\halo\\SOURCE\\effects\\particle_systems.c", 976, particle_systems && particle_systems->valid);

		for (system_index = data_next_index(particle_systems, NONE); system_index != NONE; system_index = data_next_index(particle_systems, system_index))
		{
			struct particle_system_datum const *system = particle_system_get(system_index);

			if (system->location.cluster_index != NONE && scenario_location_potentially_visible_local(&system->location))
			{
				particle_system_render(system_index);
			}
		}
	}

	return;
}

static void particle_system_render(
	long system_index)
{
	short type_index;
	struct particle_system_datum const *system = particle_system_get(system_index);
	struct particle_system_definition const *definition = particle_system_definition_get(system->definition_index);

	for (type_index = 0; type_index < definition->types.count; type_index++)
	{
		struct particle_system_type const *type_definition = TAG_BLOCK_GET_ELEMENT(&definition->types, type_index, struct particle_system_type);
		struct particle_type const *type = &system->types[type_index];

		if (type->state_index != NONE && !TEST_FLAG(type_definition->flags, _particle_system_type_disabled_bit))
		{
			short particle_index = (short)type->first_particle_index;

			while (particle_index != NONE)
			{
				struct ps_particle_datum *particle = system_particle_get(particle_index);

				if (particle->valid && render_location_visible(&particle->location))
				{
					struct particle_system_type_particle_state const *transition_state_definition;
					real_point3d position_viewer_space;
					real_vector3d axis_viewer_space;
					real state_weight;
					real transition_weight;
					real scale;
					real_argb_color color;
					struct bitmap_group const *bitmap;
					short sequence_index;
					struct bitmap_group_sequence const *sequence;
					short sprite_index;
					struct build_sprite_data sprite_data;
					struct particle_system_type_particle_state const *state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->particle_states, particle->state_index, struct particle_system_type_particle_state);
					struct shader_effect const *shader = &state_definition->shader;
					struct shader_effect const *transition_shader = NULL;

					matrix4x3_transform_point(
						&render.frustum.world_to_view,
						&particle->position,
						&position_viewer_space);
					matrix4x3_transform_normal(&render.frustum.world_to_view, &particle->axis, &axis_viewer_space);

					if (particle->transition_state_index == NONE)
					{
						transition_state_definition = NULL;
						scale = particle->randomized_variables.scale * type->variables.particle_state_randomized_multipliers.scale;
						color.alpha = particle->randomized_variables.color.alpha * type->variables.particle_state_randomized_multipliers.color.alpha;
						color.red = particle->randomized_variables.color.red * type->variables.particle_state_randomized_multipliers.color.red;
						color.green = particle->randomized_variables.color.green * type->variables.particle_state_randomized_multipliers.color.green;
						color.blue = particle->randomized_variables.color.blue * type->variables.particle_state_randomized_multipliers.color.blue;
						state_weight = 1.f;
						transition_weight = 0.f;
					}
					else
					{
						transition_state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->particle_states, particle->transition_state_index, struct particle_system_type_particle_state);
						transition_shader = &transition_state_definition->shader;
						state_weight = particle->time_left_in_state / particle->state_length;
						state_weight = PIN(state_weight, 0.f, 1.f);
						transition_weight = 1.f - state_weight;
						scale = (transition_weight * particle->transition_randomized_variables.scale + state_weight * particle->randomized_variables.scale) * type->variables.particle_state_randomized_multipliers.scale;
						color.alpha = (transition_weight * particle->transition_randomized_variables.color.alpha + state_weight * particle->randomized_variables.color.alpha) * type->variables.particle_state_randomized_multipliers.color.alpha;
						color.red = (transition_weight * particle->transition_randomized_variables.color.red + state_weight * particle->randomized_variables.color.red) * type->variables.particle_state_randomized_multipliers.color.red;
						color.green = (transition_weight * particle->transition_randomized_variables.color.green + state_weight * particle->randomized_variables.color.green) * type->variables.particle_state_randomized_multipliers.color.green;
						color.blue = (transition_weight * particle->transition_randomized_variables.color.blue + state_weight * particle->randomized_variables.color.blue) * type->variables.particle_state_randomized_multipliers.color.blue;

						if (shader && transition_shader &&
							shader->effect.framebuffer_blend_function == transition_shader->effect.framebuffer_blend_function &&
							shader->effect.primary_map_flags == transition_shader->effect.primary_map_flags &&
							state_definition->sequence_index == transition_state_definition->sequence_index)
						{
							state_weight = 1.f;
							transition_weight = 0.f;
						}
					}

					bitmap = bitmap_group_get(state_definition->bitmaps.index);
					sequence_index = type_definition->complex_sprite_render_mode == _particle_system_complex_sprite_render_mode_rotational ? state_definition->sequence_index + 1 : state_definition->sequence_index;
					sequence = TAG_BLOCK_GET_ELEMENT(&bitmap->sequences, sequence_index, struct bitmap_group_sequence);

					if (particle->sprite_index == -1.f)
					{
						particle->sprite_index = local_random_range(0, (short)sequence->sprites.count);
						sprite_index = (short)particle->sprite_index;
					}
					else
					{
						sprite_index = (short)particle->sprite_index % sequence->sprites.count;

						if (sprite_index < 0)
						{
							sprite_index += (short)sequence->sprites.count;
						}
					}

					if (state_weight > 0.01f)
					{
						real_argb_color lit_color = color;

						if (!state_definition->shader.effect.framebuffer_blend_function)
						{
							lit_color.red *= system->lighting.red;
							lit_color.green *= system->lighting.green;
							lit_color.blue *= system->lighting.blue;
						}

						build_sprites_begin(
							&sprite_data,
							2,
							state_definition->bitmaps.index,
							&state_definition->shader,
							0);

						if (type_definition->complex_sprite_render_mode == _particle_system_complex_sprite_render_mode_rotational)
						{
							unsigned long flags = FLAG(_build_sprite_rotational_viewer_space_bit);

							if (TEST_FLAG(type_definition->flags, _particle_system_type_rotational_sprites_animate_sideways_bit))
							{
								SET_FLAG(flags, _build_sprite_rotational_sideways_rotation_animates_bit, TRUE);
							}

							build_sprite_rotational(
								&sprite_data,
								flags,
								state_definition->sequence_index,
								sprite_index,
								&position_viewer_space,
								&axis_viewer_space,
								particle->rotation,
								scale,
								&lit_color,
								state_weight);
						}
						else
						{
							build_sprite(
								&sprite_data,
								type_definition->sprite_render_mode,
								state_definition->sequence_index,
								sprite_index,
								&position_viewer_space,
								&axis_viewer_space,
								particle->rotation,
								scale,
								&lit_color,
								state_weight,
								FLAG(_build_sprite_viewer_space_bit));
						}

						sprite_data.shader->effect.secondary_map_radius = state_definition->variables.radius;
						build_sprites_end(&sprite_data);
					}

					if (transition_weight > 0.01f)
					{
						real_argb_color lit_color = color;

						if (!state_definition->shader.effect.framebuffer_blend_function)
						{
							lit_color.red *= system->lighting.red;
							lit_color.green *= system->lighting.green;
							lit_color.blue *= system->lighting.blue;
						}

						build_sprites_begin(
							&sprite_data,
							2,
							transition_state_definition->bitmaps.index,
							transition_shader,
							0);
						position_viewer_space.z += 0.001f;

						if (type_definition->complex_sprite_render_mode == _particle_system_complex_sprite_render_mode_rotational)
						{
							unsigned long flags = FLAG(_build_sprite_rotational_viewer_space_bit);

							if (TEST_FLAG(type_definition->flags, _particle_system_type_rotational_sprites_animate_sideways_bit))
							{
								SET_FLAG(flags, _build_sprite_rotational_sideways_rotation_animates_bit, TRUE);
							}

							build_sprite_rotational(
								&sprite_data,
								flags,
								transition_state_definition->sequence_index,
								sprite_index,
								&position_viewer_space,
								&axis_viewer_space,
								particle->rotation,
								scale,
								&lit_color,
								transition_weight);
						}
						else
						{
							build_sprite(
								&sprite_data,
								type_definition->sprite_render_mode,
								transition_state_definition->sequence_index,
								sprite_index,
								&position_viewer_space,
								&axis_viewer_space,
								particle->rotation,
								scale,
								&lit_color,
								transition_weight,
								FLAG(_build_sprite_viewer_space_bit));
						}

						sprite_data.shader->effect.secondary_map_radius = state_definition->variables.radius;
						build_sprites_end(&sprite_data);
					}
				}

				particle_index = (short)particle->next_particle_index;
			}
		}
	}

	return;
}

static void particle_system_update_default(
	struct particle_system_datum *system,
	real dtime)
{
	struct particle_system_definition const *definition = particle_system_definition_get(system->definition_index);

	if (system->object_index == NONE && definition->system_update_point_physics.index != NONE)
	{
		point_physics_update(
			0,
			point_physics_definition_get(definition->system_update_point_physics.index),
			&system->location,
			NONE,
			&system->position,
			&system->velocity,
			NULL,
			NULL,
			NULL,
			1.f,
			dtime);
	}

	return;
}

static void particle_system_new_particle_default(
	struct particle_system_datum const *system,
	short type_index,
	struct ps_particle_datum *particle,
	struct object_marker const *marker)
{
	particle->position = marker->matrix.position;
	particle->velocity = system->velocity;

	return;
}

static void particle_system_update_particle_default(
	struct particle_system_datum const *system,
	short type_index,
	real dtime,
	struct ps_particle_datum *particle)
{
	struct particle_system_type_particle_state_interpolated_variables variables;
	real *variable;
	real const *type_variable;
	real const *multiplier;
	real const *state_variable;
	struct point_physics_definition const *physics;
	struct point_physics_definition interpolated_physics;
	unsigned long physics_flags;
	struct particle_system_definition const *definition = particle_system_definition_get(system->definition_index);
	struct particle_system_type const *type_definition = TAG_BLOCK_GET_ELEMENT(&definition->types, type_index, struct particle_system_type);
	struct particle_type const *type = &system->types[type_index];
	struct particle_system_type_particle_state const *state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->particle_states, particle->state_index, struct particle_system_type_particle_state);

	if (particle->transition_state_index == NONE)
	{
		variable = (real *)&variables;
		type_variable = (real const *)&type_definition->variables;
		multiplier = (real const *)&type->variables.particle_state_multipliers;
		state_variable = (real const *)&state_definition->variables;

		while (variable < (real *)(&variables + 1))
		{
			*variable++ = *type_variable++ * *multiplier++ * *state_variable++;
		}

		physics = point_physics_definition_get(state_definition->point_physics.index);
	}
	else
	{
		real const *transition_state_variable;
		real inverse_t;
		struct particle_system_type_particle_state const *transition_state_definition = TAG_BLOCK_GET_ELEMENT(&type_definition->particle_states, particle->transition_state_index, struct particle_system_type_particle_state);
		real t = particle->time_left_in_state / particle->state_length;

		t = PIN(t, 0.f, 1.f);
		inverse_t = 1.f - t;
		variable = (real *)&variables;
		type_variable = (real const *)&type_definition->variables;
		multiplier = (real const *)&type->variables.particle_state_multipliers;
		state_variable = (real const *)&state_definition->variables;
		transition_state_variable = (real const *)&transition_state_definition->variables;

		while (variable < (real *)(&variables + 1))
		{
			*variable++ = *type_variable++ * *multiplier++ * (t * *state_variable++ + inverse_t * *transition_state_variable++);
		}

		physics = point_physics_definition_interpolate(
			point_physics_definition_get(state_definition->point_physics.index),
			point_physics_definition_get(transition_state_definition->point_physics.index),
			t,
			&interpolated_physics);
	}

	physics_flags = point_physics_update(
		0,
		physics,
		&particle->location,
		NONE,
		&particle->position,
		&particle->velocity,
		NULL,
		NULL,
		NULL,
		variables.radius,
		dtime);

	if (TEST_FLAG(physics_flags, _point_physics_in_air_bit) && TEST_FLAG(type_definition->flags, _particle_system_type_dies_in_air_bit) ||
		TEST_FLAG(physics_flags, _point_physics_in_water_bit) && TEST_FLAG(type_definition->flags, _particle_system_type_dies_in_water_bit) ||
		TEST_FLAG(physics_flags, _point_physics_collided_with_structure_bit) && TEST_FLAG(type_definition->flags, _particle_system_type_dies_on_ground_bit))
	{
		particle->valid = FALSE;
	}

	return;
}

static void particle_system_update_explosion(
	struct particle_system_datum *system,
	real dtime)
{
	struct particle_system_definition const *definition = particle_system_definition_get(system->definition_index);

	particle_system_update_default(system, dtime);

	return;
}

static void particle_system_new_particle_explosion(
	struct particle_system_datum const *system,
	short type_index,
	struct ps_particle_datum *particle,
	struct object_marker const *marker)
{
	struct particle_system_definition const *definition = particle_system_definition_get(system->definition_index);
	struct particle_system_type const *type_definition = TAG_BLOCK_GET_ELEMENT(&definition->types, type_index, struct particle_system_type);
	real xy_spread = *TAG_BLOCK_GET_ELEMENT(&type_definition->physics_constants, _explosion_type_definition_physics_constant_xy_spread, real);
	real z_spread = *TAG_BLOCK_GET_ELEMENT(&type_definition->physics_constants, _explosion_type_definition_physics_constant_z_spread, real);
	real intensity = *TAG_BLOCK_GET_ELEMENT(&type_definition->physics_constants, _explosion_type_definition_physics_constant_intensity, real);

	local_random_direction3d(&particle->velocity);
	particle->velocity.i *= xy_spread;
	particle->velocity.j *= xy_spread;
	particle->velocity.k *= z_spread;

	if (system->physics_states.explosion_state.grounded)
	{
		particle->velocity.k = (real)fabs(particle->velocity.k);
	}

	particle->position.x = marker->matrix.position.x + particle->velocity.i;
	particle->position.y = marker->matrix.position.y + particle->velocity.j;
	particle->position.z = marker->matrix.position.z + particle->velocity.k;
	particle->axis.i = particle->velocity.i;
	particle->axis.j = particle->velocity.j;
	particle->axis.k = 0.f;
	particle->velocity.i = intensity * particle->velocity.i + system->velocity.i;
	particle->velocity.j = intensity * particle->velocity.j + system->velocity.j;
	particle->velocity.k = intensity * particle->velocity.k + system->velocity.k;
	rotate_vector_about_axis(&particle->axis, global_up3d, 1.f, 0.f);

	return;
}

static void particle_system_new_particle_jet(
	struct particle_system_datum const *system,
	short type_index,
	struct ps_particle_datum *particle,
	struct object_marker const *marker)
{
	real spread_velocity;
	real forward_velocity;
	real_vector3d spread;
	struct particle_system_definition const *definition = particle_system_definition_get(system->definition_index);
	struct particle_system_type const *type_definition = TAG_BLOCK_GET_ELEMENT(&definition->types, type_index, struct particle_system_type);
	struct particle_type const *type = &system->types[type_index];
	real velocity = *TAG_BLOCK_GET_ELEMENT(&type_definition->physics_constants, _jet_type_definition_physics_constant_velocity, real);
	real spread_fraction = *TAG_BLOCK_GET_ELEMENT(&type_definition->physics_constants, _jet_type_definition_physics_constant_spread_fraction, real);
	real rotates_up = *TAG_BLOCK_GET_ELEMENT(&type_definition->physics_constants, _jet_type_definition_physics_constant_rotates_up, real);

	velocity *= seconds_per_tick;
	spread_velocity = velocity * spread_fraction;
	forward_velocity = (1.f - spread_fraction) * velocity;
	local_random_direction3d(&spread);
	particle->velocity.i = forward_velocity * marker->matrix.forward.i + system->velocity.i + spread_velocity * spread.i;
	particle->velocity.j = forward_velocity * marker->matrix.forward.j + system->velocity.j + spread_velocity * spread.j;
	particle->velocity.k = forward_velocity * marker->matrix.forward.k + system->velocity.k + spread_velocity * spread.k;
	particle->position = marker->matrix.position;

	if (rotates_up != 0.f)
	{
		cross_product3d(&particle->velocity, global_up3d, &particle->axis);
	}
	else
	{
		cross_product3d(&marker->matrix.forward, &particle->velocity, &particle->axis);
	}

	return;
}

/* ---------- private code */
