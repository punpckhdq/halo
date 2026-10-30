/*
AI.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ai.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "ai_profile.h"
#include "ai_script.h"
#include "ai_globals.h"
#include "network_game_globals.h"
#include "players.h"
#include "console.h"
#include "game_state.h"
#include "collisions.h"
#include "render_debug.h"
#include "bipeds.h"
#include "editor_stubs.h"
#include "damage_effect_definitions.h"

/* ---------- constants */

/* ---------- macros */

#define MAXIMUM_POTENTIALLY_RELEASABLE_ENCOUNTERS 256 /* fake name */
#define MAXIMUM_LINE_OF_FIRE_PILLS 32 /* fake name */
#define NEXT_SPATIAL_EFFECT_INDEX(index) (((index) + 1) & (NUMBER_OF_CACHED_SPATIAL_EFFECTS - 1))

/* ---------- structures */

struct potentially_releasable_encounter
{
	boolean encounterless_actor;

	union
	{
		long actor_index;
		long encounter_index;
	};

	long last_active_time;
};

struct potentially_releasable_storage
{
	short releasable_count;
	short next_release_index;
	struct potentially_releasable_encounter releasable[MAXIMUM_POTENTIALLY_RELEASABLE_ENCOUNTERS];
};

struct line_of_fire_pill
{
	boolean hit; /* fake name */
	boolean spherical;
	real_point3d base;
	real_vector3d directed_height;
	long prop_index;
	long unit_index;
	real width;
};

/* ---------- prototypes */

static int compare_releasable_inactive_encounters(void const *p1, void const *p2);
static void ai_generate_line_of_fire_pill(long unit_index, long prop_index, struct line_of_fire_pill *pill);
static short ai_find_line_of_fire_friend_pills(long actor_index, short max_pill_count, struct line_of_fire_pill *pills);
static void ai_place_pending_mounted_weapons(void);
static boolean ai_enemies_endanger_player(boolean must_be_attacking);
static void ai_flush_spatial_effects(void);

/* ---------- globals */

struct ai_globals *ai_globals;

static struct profile_section ai_update_section = {"ai_update", NONE, TRUE}; /* fake name */

static char *ai_sound_volume_enum_strings[NUMBER_OF_AI_SOUND_VOLUMES] =
{
	"silent",
	"medium",
	"loud",
	"shout",
	"quiet"
};

struct enum_definition ai_sound_volume_enum =
{
	NUMBER_OF_AI_SOUND_VOLUMES,
	ai_sound_volume_enum_strings
};

/* ---------- public code */

void ai_initialize(
	void)
{
	ai_globals = game_state_malloc("ai globals", NULL, sizeof(*ai_globals));
	match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 140, ai_globals);

	memset(ai_globals, 0, sizeof(*ai_globals));
	ai_debug_initialize();
	ai_profile_initialize();
	paths_initialize();
	actors_initialize();
	props_initialize();
	encounters_initialize();
	ai_script_initialize();
	ai_communication_initialize();
	actor_move_initialize();

	return;
}

void ai_dispose(
	void)
{
	ai_communication_dispose();
	ai_script_dispose();
	encounters_dispose();
	props_dispose();
	actors_dispose();
	paths_dispose();
	ai_profile_dispose();
	ai_debug_dispose();

	return;
}

void ai_initialize_for_new_map(
	void)
{
	memset(ai_globals, 0, sizeof(*ai_globals));

	ai_globals->ai_active = TRUE;
	ai_globals->ai_has_control_data = TRUE;
	ai_globals->first_encounterless_actor_index = NONE;
	ai_globals->grenades_enabled = TRUE;
	ai_globals->dialogue_triggers_enabled = TRUE;

	memset(ai_globals->last_chatter_time, NONE, sizeof(ai_globals->last_chatter_time));
	memset(ai_globals->last_talk_time, NONE, sizeof(ai_globals->last_talk_time));
	memset(ai_globals->last_shout_time, NONE, sizeof(ai_globals->last_shout_time));

	ai_debug_initialize_for_new_map();
	ai_profile_initialize_for_new_map();
	paths_initialize_for_new_map();
	actors_initialize_for_new_map();
	props_initialize_for_new_map();
	encounters_initialize_for_new_map();
	ai_script_initialize_for_new_map();
	ai_communication_initialize_for_new_map();
	ai_flush_spatial_effects();

	ai_globals->ai_initialized_for_map = TRUE;

	return;
}

void ai_dispose_from_old_map(
	void)
{
	ai_communication_dispose_from_old_map();
	ai_script_dispose_from_old_map();
	encounters_dispose_from_old_map();
	props_dispose_from_old_map();
	actors_dispose_from_old_map();
	paths_dispose_from_old_map();
	ai_profile_dispose_from_old_map();
	ai_debug_dispose_from_old_map();

	ai_globals->ai_initialized_for_map = FALSE;

	return;
}

void ai_place(
	void)
{
	encounters_create_for_new_map();

	return;
}

void ai_update(
	void)
{
	boolean active = ai_globals->ai_initialized_for_map && !ai_profile.disable_ai;
	boolean move_randomly = ai_profile.move_randomly;

	profile_enter(ai_update_section);

	if (active)
	{
		ai_debug_update();
		ai_profile_update();
		ai_place_pending_mounted_weapons();

		if (move_randomly)
		{
			actors_move_randomly();
			ai_globals->ai_has_control_data = TRUE;
		}
		else if (ai_globals->ai_active)
		{
			ai_conversation_update();
			encounters_update();
			actors_update();
			ai_globals->ai_has_control_data = TRUE;
		}
		else if (ai_globals->ai_has_control_data)
		{
			actors_freeze();
			ai_globals->ai_has_control_data = FALSE;
		}
	}

	profile_exit(ai_update_section);

	return;
}

void ai_globals_ai_active(
	boolean enable)
{
	match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 314, ai_globals);

	ai_globals->ai_active = enable;

	return;
}

void ai_globals_dialogue_triggers_enabled(
	boolean enable)
{
	match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 323, ai_globals);

	ai_globals->dialogue_triggers_enabled = enable;

	return;
}

void ai_globals_grenades_enabled(
	boolean enable)
{
	match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 332, ai_globals);

	ai_globals->grenades_enabled = enable;

	return;
}

void ai_get_major_upgrade_chance(
	short upgrade_type,
	boolean *force_major,
	boolean *is_random,
	real *random_chance)
{
	match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 344, force_major && is_random && random_chance);

	switch (upgrade_type)
	{
	case _squad_major_upgrade_none:
		*is_random = FALSE;
		*force_major = FALSE;
		break;
	case _squad_major_upgrade_all:
		*is_random = FALSE;
		*force_major = TRUE;
		break;
	case _squad_major_upgrade_few:
		*is_random = TRUE;
		*random_chance = game_difficulty_get_value(_game_difficulty_major_few_placement);
		break;
	case _squad_major_upgrade_many:
		*is_random = TRUE;
		*random_chance = game_difficulty_get_value(_game_difficulty_major_many_placement);
		break;
	default:
		*is_random = TRUE;
		*random_chance = game_difficulty_get_value(_game_difficulty_major_normal_placement);
		break;
	}

	return;
}

boolean ai_consider_major_upgrade(
	long encounter_index,
	short squad_index,
	real upgrade_chance)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);
	struct squad_datum *squad = encounter_get_squad(encounter, squad_index);
	real global_error = -ai_globals->major_upgrade_error / 3.f;
	real squad_error = -squad->major_upgrade_error;
	real upgrade_error = (fabs(global_error) > fabs(squad_error)) ? global_error : squad_error;
	real random = real_random();
	boolean upgrade = random < upgrade_chance + upgrade_error;
	real delta = (real)upgrade - upgrade_chance;

	squad->major_upgrade_error += delta;
	ai_globals->major_upgrade_error += delta;

	if (ai_debug.print_major_upgrade)
	{
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
			struct encounter_definition);

		console_printf(
			FALSE,
			csprintf(
				temporary,
				"%s/%s major upgrade chance %.2f: %s (g %.2f s %.2f -> chance %.2f rand %.2f err %.2f)",
				encounter_definition->name,
				TAG_BLOCK_GET_ELEMENT(&encounter_definition->squads, squad_index, struct squad_definition)->name,
				upgrade_chance,
				upgrade ? "YES" : "NO",
				global_error,
				squad_error,
				upgrade_chance + upgrade_error,
				random,
				delta));
		error(
			_error_silent,
			"%s",
			temporary);
	}

	return upgrade;
}

boolean ai_adjust_damage(
	long actor_index,
	struct damage_data *damage_data,
	real *damage)
{
	boolean result = FALSE;

	if (actor_index != NONE)
	{
		struct actor_datum *actor = actor_get(actor_index);

		if (TEST_FLAG(damage_data->flags, _damage_from_weapon_bit) && actor->control.burst_damage_modifier > 0.f)
		{
			*damage *= actor->control.burst_damage_modifier;
			result = TRUE;
		}

		if (actor->external_orders.playfighting)
		{
			*damage *= 0.3f;
			result = TRUE;
		}
	}

	return result;
}

void ai_erase(
	long encounter_index,
	long platoon_index,
	long squad_index,
	boolean immediate)
{
	if (ai_globals->ai_initialized_for_map)
	{
		if (encounter_index == NONE)
		{
			struct actor_iterator iterator;
			struct actor_datum *actor;

			actor_iterator_new(&iterator, FALSE);

			while (actor = actor_iterator_next(&iterator))
			{
				actor_erase(iterator.index, immediate);
			}
		}
		else
		{
			struct encounter_actor_iterator iterator;
			struct actor_datum *actor;

			encounter_actor_iterator_new(&iterator, encounter_index);

			while (actor = encounter_actor_iterator_next(&iterator))
			{
				if ((platoon_index == NONE || actor->meta.platoon_index == platoon_index) &&
					(squad_index == NONE || actor->meta.squad_index == squad_index))
				{
					actor_erase(iterator.index, immediate);
				}
			}
		}
	}

	return;
}

boolean ai_release_inactive_swarms(
	char *result_description,
	boolean *more_to_release,
	byte *working_memory,
	short working_memory_size)
{
	struct actor_iterator iterator;
	struct actor_datum *actor;
	short swarm_unit_count = 0;

	match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 503, result_description && more_to_release);

	actor_iterator_new(&iterator, FALSE);

	while (actor = actor_iterator_next(&iterator))
	{
		if (actor->meta.swarm && !actor->meta.active && actor->meta.last_active_time != NONE)
		{
			swarm_unit_count += actor->meta.swarm_unit_count;
			actor_erase(iterator.index, TRUE);
		}
	}

	sprintf(result_description, "%d swarm units", swarm_unit_count);
	*more_to_release = FALSE;

	return swarm_unit_count > 0;
}

static int compare_releasable_inactive_encounters(
	void const *p1,
	void const *p2)
{
	struct potentially_releasable_encounter const *releasable1 = (struct potentially_releasable_encounter const *)p1;
	struct potentially_releasable_encounter const *releasable2 = (struct potentially_releasable_encounter const *)p2;

	if (releasable1->last_active_time > releasable2->last_active_time)
	{
		return 1;
	}
	else if (releasable2->last_active_time > releasable1->last_active_time)
	{
		return -1;
	}
	else if (releasable1->encounterless_actor > releasable2->encounterless_actor)
	{
		return -1;
	}
	else if (releasable1->encounterless_actor < releasable2->encounterless_actor)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void ai_find_inactive_encounters(
	byte *working_memory,
	short working_memory_size)
{
	struct potentially_releasable_storage *storage;

	match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 558, working_memory_size >= sizeof(struct potentially_releasable_storage));

	storage = (struct potentially_releasable_storage *)working_memory;
	storage->releasable_count = 0;
	storage->next_release_index = 0;

	{
		struct encounter_actor_iterator iterator;
		struct actor_datum *actor;

		encounter_actor_iterator_new(&iterator, NONE);

		while (actor = encounter_actor_iterator_next(&iterator))
		{
			if (storage->releasable_count >= MAXIMUM_POTENTIALLY_RELEASABLE_ENCOUNTERS)
			{
				break;
			}
			else if (!actor->meta.active && actor->meta.last_active_time != NONE)
			{
				storage->releasable[storage->releasable_count].encounterless_actor = TRUE;
				storage->releasable[storage->releasable_count].actor_index = iterator.index;
				storage->releasable[storage->releasable_count].last_active_time = actor->meta.last_active_time;
				storage->releasable_count++;
			}
		}
	}

	{
		struct encounter_iterator iterator;
		struct encounter_datum *encounter;

		encounter_iterator_new(&iterator, FALSE);

		while (encounter = encounter_iterator_next(&iterator))
		{
			if (storage->releasable_count >= MAXIMUM_POTENTIALLY_RELEASABLE_ENCOUNTERS)
			{
				break;
			}
			else if (!encounter->active && encounter->current_count > 0 && encounter->last_active_time != NONE)
			{
				storage->releasable[storage->releasable_count].encounterless_actor = FALSE;
				storage->releasable[storage->releasable_count].encounter_index = iterator.index;
				storage->releasable[storage->releasable_count].last_active_time = encounter->last_active_time;
				storage->releasable_count++;
			}
		}
	}

	if (storage->releasable_count > 0)
	{
		qsort(
			storage->releasable,
			storage->releasable_count,
			sizeof(struct potentially_releasable_encounter),
			compare_releasable_inactive_encounters);
	}

	return;
}

boolean ai_release_inactive_encounters(
	char *result_description,
	boolean *more_to_release,
	byte *working_memory,
	short working_memory_size)
{
	struct potentially_releasable_storage *storage = (struct potentially_releasable_storage *)working_memory;
	boolean released = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 624, result_description && more_to_release);
	match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 625, working_memory_size >= sizeof(struct potentially_releasable_storage));

	if (storage->next_release_index < storage->releasable_count)
	{
		struct potentially_releasable_encounter *releasable = &storage->releasable[storage->next_release_index];

		if (releasable->encounterless_actor)
		{
			struct actor_datum *actor = actor_get(releasable->actor_index);

			sprintf(
				result_description,
				"encounterless-actor %s",
				tag_name_strip_path(tag_get_name(actor->meta.variant_definition_index)));
			actor_erase(releasable->actor_index, TRUE);
		}
		else
		{
			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(releasable->encounter_index),
				struct encounter_definition);
			struct encounter_datum *encounter = encounter_get(releasable->encounter_index);

			sprintf(
				result_description,
				"encounter %s (%d units)",
				encounter_definition->name,
				encounter->current_count);
			ai_erase(releasable->encounter_index, NONE, NONE, TRUE);
		}

		storage->next_release_index++;
		released = TRUE;
	}

	*more_to_release = storage->next_release_index < storage->releasable_count;

	return released;
}

static void ai_generate_line_of_fire_pill(
	long unit_index,
	long prop_index,
	struct line_of_fire_pill *pill)
{
	real biped_height;
	real biped_width;

	biped_get_physics_pill(unit_index, &pill->base, &biped_height, &biped_width);
	pill->spherical = biped_height == 0.f;
	set_real_vector3d(&pill->directed_height, 0.f, 0.f, biped_height);
	pill->width = biped_width + 0.15f;
	pill->prop_index = prop_index;
	pill->unit_index = unit_index;
	pill->hit = FALSE;

	return;
}

static short ai_find_line_of_fire_friend_pills(
	long actor_index,
	short max_pill_count,
	struct line_of_fire_pill *pills)
{
	struct actor_datum *actor = actor_get(actor_index);
	short pill_count = 0;

	if (actor->meta.encounter_index != NONE)
	{
		struct encounter_actor_iterator iterator;
		struct actor_datum *friend_actor;

		encounter_actor_iterator_new(&iterator, actor->meta.encounter_index);

		while (friend_actor = encounter_actor_iterator_next(&iterator))
		{
			if (iterator.index != actor_index && pill_count < max_pill_count && friend_actor->meta.unit_index != NONE && friend_actor->input.vehicle_index == NONE)
			{
				ai_generate_line_of_fire_pill(
					friend_actor->meta.unit_index,
					prop_get_active_by_unit_index(actor_index, friend_actor->meta.unit_index),
					&pills[pill_count++]);
			}
		}
	}

	{
		struct prop_iterator iterator;
		struct prop_datum *prop;

		prop_iterator_new(&iterator, actor_index);

		while (prop = prop_iterator_next(&iterator))
		{
			if (!prop->enemy && !prop->dead && prop->state == _prop_state_acknowledged && prop->vehicle_index == NONE)
			{
				struct object_datum *object = object_get(prop->unit_index);

				if (TEST_FLAG(_object_mask_biped, object->object.type))
				{
					if (actor->meta.encounter_index != NONE && prop->actor_index != NONE)
					{
						struct actor_datum *prop_actor = actor_get(prop->actor_index);

						if (prop_actor->meta.encounter_index == actor->meta.encounter_index)
						{
							continue;
						}
					}

					if (pill_count < max_pill_count)
					{
						ai_generate_line_of_fire_pill(prop->unit_index, iterator.index, &pills[pill_count++]);
					}
				}
			}
		}
	}

	return pill_count;
}

boolean ai_test_line_of_fire(
	long actor_index,
	long ignore_unit_index,
	real_point3d const *origin,
	real_vector3d const *vector,
	long *prop_index_reference)
{
	struct line_of_fire_pill pills[MAXIMUM_LINE_OF_FIRE_PILLS];
	short pill_count;
	short pill_index;
	struct actor_datum *actor = actor_get(actor_index);
	boolean success = TRUE;
	long prop_index = NONE;

	ai_profile.meters[_ai_meter_line_of_fire].current_count++;
	pill_count = ai_find_line_of_fire_friend_pills(actor_index, NUMBEROF(pills), pills);

	for (pill_index = 0; pill_index < pill_count; pill_index++)
	{
		boolean intersects = FALSE;

		if (pills[pill_index].unit_index == ignore_unit_index)
		{
			continue;
		}

		if (pills[pill_index].spherical)
		{
			intersects = fast_vector_intersects_sphere(
				origin,
				vector,
				&pills[pill_index].base,
				(real)(pills[pill_index].width));
		}
		else
		{
			intersects = vector_intersects_pill3d(
				origin,
				vector,
				&pills[pill_index].base,
				&pills[pill_index].directed_height,
				pills[pill_index].width);
		}

		if (intersects)
		{
			success = FALSE;
			prop_index = pills[pill_index].prop_index;
			pills[pill_index].hit = TRUE;
			break;
		}
	}

	if (ai_debug.render_lineoffire)
	{
		ai_debug_lineoffire_new(origin, vector);

		for (pill_index = 0; pill_index < pill_count; pill_index++)
		{
			ai_debug_lineoffire_addpill(
				&pills[pill_index].base,
				&pills[pill_index].directed_height,
				pills[pill_index].width,
				pills[pill_index].hit);
		}

		ai_debug_lineoffire_success(success);
	}

	if (prop_index_reference)
	{
		*prop_index_reference = prop_index;
	}

	return success;
}

short ai_test_line_of_sight(
	real_point3d const *p0,
	short p0_cluster_index,
	real_point3d const *p1,
	short p1_cluster_index,
	short mode,
	boolean test_line_of_fire,
	long ignore_object_index,
	boolean ignore_vehicles)
{
	short line_of_sight;
	real collision_t;
	boolean clear;
	boolean occluded = FALSE;

	if (ai_debug.render_lineofsight)
	{
		ai_debug_lineofsight(p0, p0_cluster_index, p1, p1_cluster_index);
	}

	match_collision_log_begin_user("c:\\halo\\SOURCE\\ai\\ai.c", 841, _collision_user_ai_lineofsight);
	ai_profile.meters[_ai_meter_line_of_sight].current_count++;

	if (p0_cluster_index != NONE && p1_cluster_index != NONE && !scenario_test_pvs(p0_cluster_index, p1_cluster_index))
	{
		line_of_sight = _ai_line_of_sight_obstructed;
	}
	else
	{
		real fog;
		unsigned long flags;
		struct collision_result collision;

		flags = FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_back_facing_surfaces_bit) | FLAG(_collision_test_structure_bit) | FLAG(_collision_test_objects_bit) | _collision_test_objects_sight_blocking_flags;

		if (game_connection() == _game_connection_local && ai_debug.fast_los)
		{
			flags &= ~(FLAG(_collision_test_objects_bit) | _collision_test_objects_all_types_flags);
		}

		if (test_line_of_fire)
		{
			SET_FLAG(flags, _collision_test_ignore_breakable_surfaces_bit, TRUE);
		}
		else
		{
			SET_FLAG(flags, _collision_test_ignore_two_sided_surfaces_bit, TRUE);
		}

		if (ignore_vehicles)
		{
			SET_FLAG(flags, _collision_test_objects_vehicles_bit, FALSE);
		}

		ai_profile.meters[_ai_meter_collision_vector].current_count++;

		if (!collision_test_line(flags, p0, p1, ignore_object_index, &collision))
		{
			clear = TRUE;
		}
		else
		{
			clear = FALSE;
			collision_t = collision.t;
		}

		fog = scenario_fog_at_point(&collision.start_location, p0, &collision.point);

		if (fog > 0.8f)
		{
			clear = FALSE;
		}
		else if (fog > 0.6f)
		{
			occluded = TRUE;
		}
		else if (!mode)
		{
		}
		else
		{
			real_vector3d perpendicular;

			set_real_vector3d(&perpendicular, p0->y - p1->y, p1->x - p0->x, 0.f);

			if (normalize3d(&perpendicular) == 0.f)
			{
				perpendicular = *global_forward3d;
			}

			if (mode == 1)
			{
				real_point3d p0a;
				real_point3d p0b;

				point_from_line3d(p0, &perpendicular, 0.25f, &p0a);
				point_from_line3d(p0, &perpendicular, -0.25f, &p0b);

				if (clear)
				{
					if (!occluded)
					{
						ai_profile.meters[_ai_meter_collision_vector].current_count++;
						occluded = collision_test_line(flags, &p0a, p1, ignore_object_index, &collision);
					}

					if (!occluded)
					{
						ai_profile.meters[_ai_meter_collision_vector].current_count++;
						occluded = collision_test_line(flags, &p0b, p1, ignore_object_index, &collision);
					}
				}
				else
				{
					if (!occluded)
					{
						ai_profile.meters[_ai_meter_collision_vector].current_count++;
						occluded = !collision_test_line(flags, &p0a, p1, ignore_object_index, &collision);
					}

					if (!occluded)
					{
						ai_profile.meters[_ai_meter_collision_vector].current_count++;
						occluded = !collision_test_line(flags, &p0b, p1, ignore_object_index, &collision);
					}
				}
			}
			else if (clear)
			{
				real_point3d p1a;
				real_point3d p1b;
				real_point3d p1c;

				point_from_line3d(p1, &perpendicular, 0.1f, &p1a);
				point_from_line3d(p1, &perpendicular, -0.1f, &p1b);
				point_from_line3d(p1, global_down3d, 0.1f, &p1c);

				if (!occluded)
				{
					ai_profile.meters[_ai_meter_collision_vector].current_count++;
					occluded = collision_test_line(flags, &p1a, p0, ignore_object_index, &collision);
				}

				if (!occluded)
				{
					ai_profile.meters[_ai_meter_collision_vector].current_count++;
					occluded = collision_test_line(flags, &p1b, p0, ignore_object_index, &collision);
				}

				if (!occluded)
				{
					ai_profile.meters[_ai_meter_collision_vector].current_count++;
					occluded = collision_test_line(flags, &p1c, p0, ignore_object_index, &collision);
				}
			}
		}

		if (occluded)
		{
			line_of_sight = _ai_line_of_sight_occluded;
		}
		else if (clear)
		{
			line_of_sight = _ai_line_of_sight_clear;
		}
		else
		{
			real distance = distance3d(p0, p1);

			if (distance < 1.f)
			{
				line_of_sight = _ai_line_of_sight_obstructed;
			}
			else if (collision_t * distance < 1.f)
			{
				line_of_sight = _ai_line_of_sight_from_cover;
			}
			else if ((1.f - collision_t) * distance < 4.f)
			{
				line_of_sight = _ai_line_of_sight_to_cover;
			}
			else
			{
				line_of_sight = _ai_line_of_sight_obstructed;
			}
		}
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\ai\\ai.c", 1008);

	return line_of_sight;
}

boolean ai_test_ballistic_line_of_fire(
	long actor_index,
	real_point3d const *origin,
	real arc_time,
	real_vector3d const *arc_initial_velocity,
	real arc_acceleration,
	long ignore_object_index,
	boolean ignore_vehicles)
{
	struct line_of_fire_pill friend_pills[MAXIMUM_LINE_OF_FIRE_PILLS];
	real_point3d current_point;
	real_point3d end_point;
	real_vector3d current_velocity;
	struct collision_result collision;
	real current_time;
	real next_time;
	unsigned long flags;
	struct actor_datum *actor = actor_get(actor_index);
	boolean success = TRUE;
	short friend_pill_count = 0;

	match_collision_log_begin_user("c:\\halo\\SOURCE\\ai\\ai.c", 1032, _collision_user_ai_lineoffire);
	friend_pill_count = ai_find_line_of_fire_friend_pills(actor_index, NUMBEROF(friend_pills), friend_pills);

	if (!ai_debug.ballistic_lineoffire_freeze)
	{
		short pill_index;

		ai_debug.ballistic_valid = TRUE;
		ai_debug.ballistic_origin = *origin;
		ai_debug.ballistic_initial_velocity = *arc_initial_velocity;
		ai_debug.ballistic_numpoints = 0;
		ai_debug.ballistic_numpills = MIN(friend_pill_count, MAXIMUM_BALLISTIC_PILLS);

		for (pill_index = 0; pill_index < ai_debug.ballistic_numpills; pill_index++)
		{
			ai_debug.ballistic_pillbase[pill_index] = friend_pills[pill_index].base;
			ai_debug.ballistic_pilldirectedheight[pill_index] = friend_pills[pill_index].directed_height;
			ai_debug.ballistic_pillwidth[pill_index] = friend_pills[pill_index].width;
		}
	}

	flags = FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_back_facing_surfaces_bit) | FLAG(_collision_test_ignore_breakable_surfaces_bit) | FLAG(_collision_test_structure_bit) | FLAG(_collision_test_objects_bit) | _collision_test_objects_sight_blocking_flags;

	if (ignore_vehicles)
	{
		SET_FLAG(flags, _collision_test_objects_vehicles_bit, FALSE);
	}

	current_time = 0.f;
	current_point = *origin;
	current_velocity = *arc_initial_velocity;
	next_time = MIN(6.f, arc_time);

	do
	{
		if (!ai_debug.ballistic_lineoffire_freeze && ai_debug.ballistic_numpoints < MAXIMUM_BALLISTIC_POINTS)
		{
			ai_debug.ballistic_points[ai_debug.ballistic_numpoints++] = current_point;
		}

		point_from_line3d(&current_point, &current_velocity, next_time - current_time, &end_point);
		end_point.z += arc_acceleration * 0.5f * ((next_time - current_time) * (next_time - current_time));
		success = !collision_test_line(flags, &current_point, &end_point, ignore_object_index, &collision);

		if (success)
		{
			real_vector3d current_delta;
			short pill_index;

			vector_from_points3d(&current_point, &end_point, &current_delta);

			for (pill_index = 0; pill_index < friend_pill_count; pill_index++)
			{
				if (vector_intersects_pill3d(
					&current_point,
					&current_delta,
					&friend_pills[pill_index].base,
					&friend_pills[pill_index].directed_height,
					friend_pills[pill_index].width))
				{
					success = FALSE;
					break;
				}
			}

			if (success)
			{
				current_velocity.k += (next_time - current_time) * arc_acceleration;
				current_time = next_time;
				current_point = end_point;
				next_time += 6.f;
				next_time = MIN(next_time, arc_time);
			}
		}
	}
	while (success && current_time < arc_time);

	if (!ai_debug.ballistic_lineoffire_freeze && ai_debug.ballistic_numpoints < MAXIMUM_BALLISTIC_POINTS)
	{
		ai_debug.ballistic_points[ai_debug.ballistic_numpoints++] = end_point;
	}

	if (!ai_debug.ballistic_lineoffire_freeze)
	{
		ai_debug.ballistic_success = success;
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\ai\\ai.c", 1144);

	return success;
}

boolean ai_try_vehicle_eviction(
	long actor_index,
	long requesting_unit_index,
	boolean actually_evict)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (requesting_unit_index != NONE)
	{
		struct unit_datum *requesting_unit = unit_get(requesting_unit_index);

		if (requesting_unit->unit.player_index != NONE && !game_team_is_enemy(requesting_unit->object.owner_team_index, actor->meta.team_index))
		{
			result = TRUE;

			if (actually_evict)
			{
				actor_stimulus_vehicle_eviction(actor_index);
			}
		}
	}

	return result;
}

long ai_get_responsible_unit(
	long object_index,
	boolean responsible_for_weapon_fire)
{
	long unit_index = NONE;

	if (object_index != NONE)
	{
		struct unit_datum *unit = unit_try_and_get(object_index);

		if (unit)
		{
			if (responsible_for_weapon_fire && unit->unit.gunner_object_index != NONE)
			{
				unit_index = unit->unit.gunner_object_index;
			}
			else if (unit->unit.driver_object_index != NONE)
			{
				unit_index = unit->unit.driver_object_index;
			}
			else
			{
				unit_index = object_index;
			}
		}

		if (unit_index != NONE && game_connection() == _game_connection_local && ai_debug.ignore_player)
		{
			struct unit_datum *responsible_unit = unit_get(unit_index);

			if (responsible_unit->unit.player_index != NONE)
			{
				unit_index = NONE;
			}
		}
	}

	return unit_index;
}

void ai_handle_death(
	long unit_index,
	long owner_object_index,
	short damage_category)
{
	long responsible_unit_index = ai_get_responsible_unit(owner_object_index, damage_category != _damage_category_vehicle);
	short hostility = _comm_hostility_none;

	if (unit_index == responsible_unit_index)
	{
		hostility = _comm_hostility_self;
	}
	else if (responsible_unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		struct unit_datum *responsible_unit = unit_get(responsible_unit_index);

		hostility = game_team_is_enemy(unit->object.owner_team_index, responsible_unit->object.owner_team_index) ? _comm_hostility_enemy : _comm_hostility_friend;
	}

	ai_communication_event(
		_ai_communication_death,
		unit_index,
		responsible_unit_index,
		hostility,
		damage_category,
		NONE,
		NULL);
	ai_conversation_unit_died(unit_index, FALSE);
	encounters_unit_died(unit_index);

	return;
}

boolean ai_handle_killing_spree(
	long unit_index,
	short killing_spree_count)
{
	struct unit_datum *unit = unit_get(unit_index);
	short minimum_killing_spree_count = (unit->unit.player_index != NONE) ? 5 : 3;
	boolean result = FALSE;

	if (ai_debug.print_killing_sprees)
	{
		char unitbuf[512];

		if (unit->unit.player_index == NONE)
		{
			ai_debug_describe_actor(unit->unit.actor_index, unit_index, TRUE, unitbuf, sizeof(unitbuf));
		}
		else
		{
			sprintf(unitbuf, "player%d", DATUM_INDEX_TO_ABSOLUTE_INDEX(unit->unit.player_index));
		}

		console_printf(FALSE, "%s killing spree: %d", unitbuf, killing_spree_count);
	}

	if (killing_spree_count >= minimum_killing_spree_count)
	{
		ai_communication_event(
			_ai_communication_killing_spree,
			unit_index,
			NONE,
			NONE,
			NONE,
			NONE,
			NULL);
		result = TRUE;
	}

	return result;
}

void ai_handle_allegiance_status_changed(
	short team1_index,
	short team2_index,
	boolean broken,
	boolean permanently_broken)
{
	struct actor_iterator iterator;
	struct actor_datum *actor;

	if (ai_debug.print_allegiance)
	{
		console_printf(
			FALSE,
			"allegiance between teams %s and %s %s%s",
			global_game_team_names[team1_index],
			global_game_team_names[team2_index],
			broken ? "broken" : "reformed",
			permanently_broken ? " permanently" : "");
	}

	actor_iterator_new(&iterator, TRUE);

	while (actor = actor_iterator_next(&iterator))
	{
		short team_index = actor->meta.team_index;
		short other_team_index = NONE;

		if (team_index == team1_index)
		{
			other_team_index = team2_index;
		}
		else if (team_index == team2_index)
		{
			other_team_index = team1_index;
		}

		if (other_team_index != NONE)
		{
			struct prop_iterator prop_iterator;
			struct prop_datum *prop;

			prop_iterator_new(&prop_iterator, iterator.index);

			while (prop = prop_iterator_next(&prop_iterator))
			{
				if (prop->team_index == other_team_index)
				{
					if (!permanently_broken)
					{
						prop->ally = TRUE;
						prop->ally_status_changed = TRUE;
					}

					if (!broken || permanently_broken)
					{
						prop->enemy = broken;
						prop->unopposable_enemy = actor_compute_prop_unopposable(iterator.index, prop_iterator.index);
						prop->target_weight = actor_compute_prop_target_weight(iterator.index, prop_iterator.index);
					}
				}
			}
		}
	}

	return;
}

void ai_handle_allegiance_broken_notification(
	short team1_index,
	short team2_index,
	boolean broken)
{
	struct actor_iterator iterator;
	struct actor_datum *actor;

	if (ai_debug.print_allegiance)
	{
		console_printf(
			FALSE,
			"allegiance between teams %s and %s communicated as %s",
			global_game_team_names[team1_index],
			global_game_team_names[team2_index],
			broken ? "broken" : "reformed");
	}

	actor_iterator_new(&iterator, TRUE);

	while (actor = actor_iterator_next(&iterator))
	{
		short team_index = actor->meta.team_index;
		short other_team_index = NONE;

		if (team_index == team1_index)
		{
			other_team_index = team2_index;
		}
		else if (team_index == team2_index)
		{
			other_team_index = team1_index;
		}

		if (other_team_index != NONE)
		{
			struct prop_iterator prop_iterator;
			struct prop_datum *prop;

			prop_iterator_new(&prop_iterator, iterator.index);

			while (prop = prop_iterator_next(&prop_iterator))
			{
				if (prop->team_index == other_team_index)
				{
					prop->ally = TRUE;
					prop->ally_status_changed = FALSE;
					prop->enemy = broken;
					prop->unopposable_enemy = actor_compute_prop_unopposable(iterator.index, prop_iterator.index);
					prop->target_weight = actor_compute_prop_target_weight(iterator.index, prop_iterator.index);
				}
			}
		}
	}

	game_allegiance_notify_change(team1_index, team2_index);

	return;
}

void ai_update_team_status(
	void)
{
	struct actor_iterator iterator;
	struct actor_datum *actor;

	actor_iterator_new(&iterator, TRUE);

	while (actor = actor_iterator_next(&iterator))
	{
		struct prop_iterator prop_iterator;
		struct prop_datum *prop;

		prop_iterator_new(&prop_iterator, iterator.index);

		while (prop = prop_iterator_next(&prop_iterator))
		{
			struct unit_datum *unit = unit_get(prop->unit_index);

			prop->team_index = unit->object.owner_team_index;
			prop->enemy = game_team_is_enemy(actor->meta.team_index, prop->team_index);
			prop->ally = game_team_is_ally(actor->meta.team_index, prop->team_index);
			prop->unopposable_enemy = actor_compute_prop_unopposable(iterator.index, prop_iterator.index);
			prop->target_weight = actor_compute_prop_target_weight(iterator.index, prop_iterator.index);
		}
	}

	return;
}

void ai_handle_bump(
	long biped_index,
	long object_index,
	real_vector3d const *old_velocity)
{
	long bumped_unit_index = ai_get_responsible_unit(object_index, FALSE);

	if (bumped_unit_index != NONE)
	{
		struct unit_datum *bumped_unit = unit_get(bumped_unit_index);

		if (bumped_unit->object.type == _object_type_biped)
		{
			struct unit_datum *biped = unit_get(biped_index);

			if (biped->unit.actor_index != NONE)
			{
				long prop_index = prop_get_base_by_unit_index(biped->unit.actor_index, bumped_unit_index, TRUE, FALSE);

				if (prop_index != NONE)
				{
					actor_handle_unit_effect(biped->unit.actor_index, prop_index, _ai_unit_effect_bump);
				}
			}

			if (bumped_unit_index != NONE && bumped_unit->unit.actor_index != NONE)
			{
				long prop_index = prop_get_base_by_unit_index(bumped_unit->unit.actor_index, biped_index, TRUE, FALSE);

				if (prop_index != NONE)
				{
					actor_handle_unit_effect(bumped_unit->unit.actor_index, prop_index, _ai_unit_effect_bump);
				}
			}
		}
	}

	return;
}

void ai_handle_damage(
	long unit_index,
	long shooter_object_index,
	short damage_category,
	real fraction,
	real_vector3d *damage_velocity,
	boolean delayed)
{
	short hostility;
	struct unit_datum *unit = unit_get(unit_index);
	long responsible_unit_index = ai_get_responsible_unit(shooter_object_index, damage_category != _damage_category_vehicle);
	struct unit_datum *responsible_unit = responsible_unit_index == NONE ? NULL : unit_get(responsible_unit_index);

	if (!delayed && damage_category != _damage_category_falling && unit->unit.actor_index != NONE)
	{
		actor_handle_damage(unit->unit.actor_index, responsible_unit_index, fraction, damage_velocity);
	}

	hostility = _comm_hostility_none;

	if (unit_index == responsible_unit_index)
	{
		hostility = _comm_hostility_self;
	}
	else if (responsible_unit)
	{
		hostility = game_team_is_enemy(unit->object.owner_team_index, responsible_unit->object.owner_team_index) ? _comm_hostility_enemy : _comm_hostility_friend;
	}

	if (!delayed && hostility == _comm_hostility_friend)
	{
		ai_communication_event(
			_ai_communication_damage,
			unit_index,
			responsible_unit_index,
			_comm_hostility_friend,
			damage_category,
			NONE,
			NULL);
	}
	else if (fraction >= 0.3f)
	{
		ai_communication_event(
			_ai_communication_hurt,
			unit_index,
			responsible_unit_index,
			hostility,
			damage_category,
			NONE,
			NULL);
	}

	if (responsible_unit)
	{
		game_allegiance_provoke(responsible_unit->object.owner_team_index, unit->object.owner_team_index);
	}

	return;
}

static void ai_place_pending_mounted_weapons(
	void)
{
	short unit_index_index;

	for (unit_index_index = 0; unit_index_index < ai_globals->mounted_weapon_unit_count; unit_index_index++)
	{
		short seat_index;
		long unit_index = ai_globals->mounted_weapon_unit_indices[unit_index_index];
		struct unit_datum *unit = unit_get(unit_index);
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

		for (seat_index = 0; seat_index < unit_definition->unit.seats.count; seat_index++)
		{
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
				&unit_definition->unit.seats,
				seat_index,
				struct unit_seat);

			if (seat->built_in_actor_reference.index != NONE)
			{
				struct actor_starting_location_definition fake_starting_location;
				long actor_index;

				memset(&fake_starting_location, 0, sizeof(fake_starting_location));
				fake_starting_location.command_list_index = NONE;
				object_get_origin(unit_index, &fake_starting_location.position);
				actor_index = actor_place(
					seat->built_in_actor_reference.index,
					NONE,
					NONE,
					&fake_starting_location,
					FALSE,
					0);

				if (actor_index != NONE)
				{
					struct actor_datum *actor = actor_get(actor_index);

					unit_enter_seat(actor->meta.unit_index, unit_index, seat_index);
				}
			}
		}
	}

	ai_globals->mounted_weapon_unit_count = 0;

	return;
}

void ai_create_mounted_weapons_for_unit(
	long unit_index)
{
	if (ai_globals->ai_initialized_for_map)
	{
		if (ai_globals->mounted_weapon_unit_count < MAXIMUM_NUMBER_OF_MOUNTED_WEAPON_UNITS)
		{
			ai_globals->mounted_weapon_unit_indices[ai_globals->mounted_weapon_unit_count] = unit_index;
			ai_globals->mounted_weapon_unit_count++;
		}
		else
		{
			struct unit_datum *unit = unit_get(unit_index);

			error(
				_error_silent,
				"WARNING: cannot create mounted weapons for %s, exceeded MAXIMUM_NUMBER_OF_MOUNTED_WEAPON_UNITS",
				tag_name_strip_path(tag_get_name(unit->definition_index)));
		}
	}

	return;
}

void ai_handle_deleted_object(
	long deleted_object_index)
{
	if (ai_globals->ai_initialized_for_map)
	{
		struct object_datum *object = object_get(deleted_object_index);

		if (TEST_FLAG(_object_mask_unit, object->object.type))
		{
			struct data_iterator iterator;
			struct prop_datum *prop;
			short unit_index_index;
			struct unit_datum *unit = unit_get(deleted_object_index);

			if (unit->unit.actor_index != NONE)
			{
				actor_delete(unit->unit.actor_index, FALSE);
			}
			else if (unit->unit.swarm_actor_index != NONE)
			{
				actor_swarm_unit_died(unit->unit.swarm_actor_index, deleted_object_index);
			}

			data_iterator_new(&iterator, prop_data);

			while (prop = (struct prop_datum *)data_iterator_next(&iterator))
			{
				if (prop->unit_index == deleted_object_index)
				{
					actor_switch_props(prop->owner_actor_index, iterator.index, NONE);
					prop_delete(prop->owner_actor_index, iterator.index);
				}
				else if (prop->vehicle_index == deleted_object_index)
				{
					prop->vehicle_index = NONE;
					prop->dangerous_vehicle_driver = FALSE;
					prop->vehicle_gunner = FALSE;
				}
			}

			ai_conversation_unit_died(deleted_object_index, TRUE);

			for (unit_index_index = 0; unit_index_index < ai_globals->mounted_weapon_unit_count; unit_index_index++)
			{
				if (ai_globals->mounted_weapon_unit_indices[unit_index_index] == deleted_object_index && --ai_globals->mounted_weapon_unit_count > 0)
				{
					ai_globals->mounted_weapon_unit_indices[unit_index_index] = ai_globals->mounted_weapon_unit_indices[ai_globals->mounted_weapon_unit_count];
				}
			}
		}
	}

	return;
}

void ai_handle_editing(
	long encounter_index)
{
	if (ai_globals->ai_initialized_for_map)
	{
		struct scenario *scenario = global_scenario_get();
		short encounter_absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index);

		if (encounter_absolute_index < 0 || encounter_absolute_index >= scenario->ai_encounters.count)
		{
			struct actor_iterator iterator;
			struct actor_datum *actor;

			ai_debug.selected_encounter_index = NONE;
			actor_iterator_new(&iterator, FALSE);

			while (actor = actor_iterator_next(&iterator))
			{
				actor_kill(iterator.index, FALSE, FALSE);
			}

			ai_dispose_from_old_map();
			ai_initialize_for_new_map();
		}
		else
		{
			struct encounter_actor_iterator iterator;
			struct actor_datum *actor;
			struct encounter_datum *encounter = encounter_get(encounter_index);
			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
				struct encounter_definition);
			short squad_delta = encounter_definition->squads.count - encounter->squad_count;
			short platoon_delta = encounter_definition->platoons.count - encounter->platoon_count;

			if (squad_delta)
			{
				struct encounter_datum *last_encounter = encounter_get(scenario->ai_encounters.count - 1);
				short total_squad_count = last_encounter->squad_base + last_encounter->squad_count;

				match_vassert("c:\\halo\\SOURCE\\ai\\ai.c", 1712, total_squad_count + squad_delta <= MAXIMUM_SQUADS_PER_MAP, csprintf(temporary, "editing caused an overflow of MAXIMUM_SQUADS_PER_MAP (%d)", MAXIMUM_SQUADS_PER_MAP));
				memmove(
					&squad_array[encounter->squad_base + encounter_definition->squads.count],
					&squad_array[encounter->squad_base + encounter->squad_count],
					total_squad_count - (encounter->squad_base + encounter->squad_count));

				if (squad_delta > 0)
				{
					memset(
						&squad_array[encounter->squad_base + encounter->squad_count],
						0,
						squad_delta * sizeof(struct squad_datum));
				}

				encounter->squad_count += squad_delta;
			}

			if (platoon_delta)
			{
				struct encounter_datum *last_encounter = encounter_get(scenario->ai_encounters.count - 1);
				short total_platoon_count = last_encounter->platoon_base + last_encounter->platoon_count;

				match_vassert("c:\\halo\\SOURCE\\ai\\ai.c", 1732, total_platoon_count + platoon_delta <= MAXIMUM_PLATOONS_PER_MAP, csprintf(temporary, "editing caused an overflow of MAXIMUM_PLATOONS_PER_MAP (%d)", MAXIMUM_PLATOONS_PER_MAP));
				memmove(
					&platoon_array[encounter->platoon_base + encounter_definition->platoons.count],
					&platoon_array[encounter->platoon_base + encounter->platoon_count],
					total_platoon_count - (encounter->platoon_base + encounter->platoon_count));

				if (platoon_delta > 0)
				{
					memset(
						&platoon_array[encounter->platoon_base + encounter->platoon_count],
						0,
						platoon_delta * sizeof(struct platoon_datum));
				}

				encounter->platoon_count += platoon_delta;
			}

			if (squad_delta || platoon_delta)
			{
				short adjust_encounter_index;

				for (adjust_encounter_index = encounter_absolute_index + 1; adjust_encounter_index < scenario->ai_encounters.count; adjust_encounter_index++)
				{
					struct encounter_datum *adjust_encounter = encounter_get(adjust_encounter_index);

					adjust_encounter->squad_base += squad_delta;
					match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 1756, adjust_encounter->squad_base >= encounter->squad_base + encounter->squad_count);
					match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 1757, adjust_encounter->squad_base + adjust_encounter->squad_count <= MAXIMUM_SQUADS_PER_MAP);

					adjust_encounter->platoon_base += platoon_delta;
					match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 1760, adjust_encounter->platoon_base >= encounter->platoon_base + encounter->platoon_count);
					match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 1761, adjust_encounter->platoon_base + adjust_encounter->platoon_count <= MAXIMUM_PLATOONS_PER_MAP);
				}
			}

			encounter_actor_iterator_new(&iterator, encounter_index);

			while (actor = encounter_actor_iterator_next(&iterator))
			{
				boolean kill_actor = FALSE;

				if (actor->meta.squad_index < 0 || actor->meta.squad_index >= encounter_definition->squads.count)
				{
					if (encounter_definition->squads.count == 0)
					{
						kill_actor = TRUE;
					}
					else
					{
						struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
							&encounter_definition->squads,
							0,
							struct squad_definition);

						actor->meta.squad_index = 0;
						actor->meta.platoon_index = squad_definition->platoon_index;

						if (squad_definition->platoon_index < 0 || squad_definition->platoon_index >= encounter_definition->platoons.count)
						{
							actor->meta.platoon_index = NONE;
						}
					}
				}

				if (kill_actor)
				{
					actor_kill(iterator.index, FALSE, FALSE);
				}
				else
				{
					if (actor->meta.platoon_index < 0 || actor->meta.platoon_index >= encounter_definition->platoons.count)
					{
						actor->meta.platoon_index = NONE;
					}

					if (actor->state.command_list_index < 0 || actor->state.command_list_index >= scenario->ai_command_lists.count)
					{
						actor->state.command_list_index = NONE;
					}

					if (actor->state.action == _actor_action_obey)
					{
						action_obey_flush_command_indices(iterator.index);
					}

					actor_flush_position_indices(iterator.index);
				}
			}
		}
	}

	return;
}

void ai_handle_unit_effect(
	long owner_unit_index,
	short effect_type,
	short volume)
{
	if (ai_globals->ai_initialized_for_map)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 1833, volume>=0 && volume<NUMBER_OF_AI_SOUND_VOLUMES);
		match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 1834, effect_type>=0 && effect_type<NUMBER_OF_AI_UNIT_EFFECTS);

		if (owner_unit_index != NONE && volume > 0)
		{
			struct unit_datum *unit = unit_get(owner_unit_index);
			long current_time = game_time_get();

			if (game_connection() == _game_connection_local && ai_debug.ignore_player && unit->unit.player_index != NONE)
			{
			}
			else if (effect_type <= unit->unit.last_unit_effect_type && current_time <= unit->unit.game_time_at_last_unit_effect + TICKS_PER_SECOND)
			{
			}
			else
			{
				unit->unit.last_unit_effect_type = effect_type;
				unit->unit.game_time_at_last_unit_effect = current_time;

				if (unit->object.type == _object_type_vehicle)
				{
					long object_index = unit->object.first_child_object_index;

					while (object_index != NONE)
					{
						struct object_datum *object = object_get(object_index);

						if (object->object.type == _object_type_biped)
						{
							actors_handle_unit_effect(object_index, effect_type, volume);
						}

						object_index = object->object.next_object_index;
					}
				}
				else if (unit->object.type == _object_type_biped)
				{
					actors_handle_unit_effect(owner_unit_index, effect_type, volume);
				}
			}
		}
	}

	return;
}

void ai_handle_enter_vehicle(
	long unit_index,
	long vehicle_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.actor_index != NONE)
	{
		ai_communication_event(
			_ai_communication_vehicle_entry,
			unit_index,
			NONE,
			NONE,
			NONE,
			NONE,
			NULL);
	}

	return;
}

void ai_handle_exit_vehicle(
	long unit_index,
	long vehicle_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.actor_index != NONE)
	{
		struct actor_datum *actor = actor_get(unit->unit.actor_index);

		if (!actor->emotions.last_vehicle_exit_forced)
		{
			ai_communication_event(
				_ai_communication_vehicle_exit,
				unit_index,
				NONE,
				NONE,
				NONE,
				NONE,
				NULL);
		}

		actor->emotions.last_vehicle_exit_forced = FALSE;
	}

	return;
}

static boolean ai_enemies_endanger_player(
	boolean must_be_attacking)
{
	struct data_iterator iterator;
	struct prop_datum *prop;
	long current_time = game_time_get();

	data_iterator_new(&iterator, prop_data);

	while (prop = (struct prop_datum *)data_iterator_next(&iterator))
	{
		if (prop->player && prop->enemy)
		{
			struct unit_datum *prop_unit = unit_get(prop->unit_index);

			if (prop_unit->unit.player_index != NONE)
			{
				struct actor_datum *actor = actor_get(prop->owner_actor_index);
				long unit_index = actor->meta.swarm ? actor->meta.swarm_unit_index : actor->meta.unit_index;
				struct unit_datum *unit = unit_get(unit_index);
				struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
				boolean ignore = FALSE;

				if (TEST_FLAG(unit_definition->unit.flags, _unit_is_inconsequential_bit) && prop->distance > 4.f)
				{
					ignore = TRUE;
				}

				if (must_be_attacking && actor->control.fire_state == _actor_fire_state_none && actor->state.action != _actor_action_charge && prop->distance > 15.f)
				{
					ignore = TRUE;
				}

				if (!ignore)
				{
					if ((prop->state < _prop_state_uninspected_orphan || prop->state > _prop_state_inspected_orphan) && prop->last_visible_time != NONE && prop->last_visible_time + 3 * TICKS_PER_SECOND >= current_time)
					{
						return TRUE;
					}

					if ((prop->state < _prop_state_uninspected_orphan || prop->state > _prop_state_inspected_orphan) && prop->distance < 4.f)
					{
						return TRUE;
					}

					if (actor->target.target_prop_index == iterator.index)
					{
						if (prop->state >= _prop_state_becoming_unacknowledged && prop->state <= _prop_state_acknowledged)
						{
							return TRUE;
						}
						else if (prop->state >= _prop_state_uninspected_orphan && prop->state <= _prop_state_inspected_orphan)
						{
							if (prop->definitely_located)
							{
								return TRUE;
							}
							else
							{
								struct prop_datum *orphan_prop = prop_get(prop->orphan_prop_index);

								if (prop->state == _prop_state_uninspected_orphan && prop->distance < 12.f && distance_squared3d(&orphan_prop->body_position, &prop->body_position) < 16.f)
								{
									return TRUE;
								}
							}
						}
					}
				}
			}
		}
	}

	return FALSE;
}

boolean ai_enemies_can_see_player(
	void)
{
	return ai_enemies_endanger_player(FALSE);
}

boolean ai_enemies_attacking_player(
	void)
{
	return ai_enemies_endanger_player(TRUE);
}

static void ai_flush_spatial_effects(
	void)
{
	match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 2043, NEXT_SPATIAL_EFFECT_INDEX(NUMBER_OF_CACHED_SPATIAL_EFFECTS-1)==0);

	ai_globals->spatial_effects_last_index = 0;
	ai_globals->spatial_effects_first_index = 0;
	memset(ai_globals->spatial_effects, 0, sizeof(ai_globals->spatial_effects));

	return;
}

// Optimizations are turned OFF on this function (by Bungie!)
// it's also OFF in Halo 2, HCEX, and other builds of Halo
// HCEX pdb shows a 12-line gap here, that could contain lines of comments explaining why
// Reason is Unknown.
#pragma optimize("", off)
void ai_handle_spatial_effect(
	long object_index,
	real_point3d const *position,
	short effect_type,
	short volume,
	short count)
{
	if (ai_globals->ai_initialized_for_map)
	{
		long current_time = game_time_get();

		match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 2062, count>0);
		match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 2063, volume>=0 && volume<NUMBER_OF_AI_SOUND_VOLUMES);
		match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 2064, effect_type>=0 && effect_type<NUMBER_OF_AI_SPATIAL_EFFECTS);

		if (volume > 0)
		{
			long discard_start_time;
			short empty_effect_index;
			struct ai_spatial_effect *spatial_effect;
			long repeating_start_time;
			long resubmit_start_time;
			short effect_index;
			boolean submit;

			resubmit_start_time = current_time - TICKS_PER_SECOND;
			repeating_start_time = current_time - TICKS_PER_SECOND;
			discard_start_time = repeating_start_time - 3 * TICKS_PER_SECOND;
			spatial_effect = NULL;
			submit = TRUE;
			empty_effect_index = NONE;

			for (effect_index = ai_globals->spatial_effects_first_index; effect_index != ai_globals->spatial_effects_last_index; effect_index = NEXT_SPATIAL_EFFECT_INDEX(effect_index))
			{
				boolean redundant = FALSE;

				if (effect_type == ai_globals->spatial_effects[effect_index].type && distance_squared3d(&ai_globals->spatial_effects[effect_index].position, position) < 1.f)
				{
					redundant = TRUE;
				}

				if (ai_globals->spatial_effects[effect_index].last_tick <= discard_start_time)
				{
					ai_globals->spatial_effects[effect_index].type = NONE;

					if (effect_index == ai_globals->spatial_effects_first_index)
					{
						ai_globals->spatial_effects_first_index = NEXT_SPATIAL_EFFECT_INDEX(effect_index);
					}
					else
					{
						empty_effect_index = effect_index;
					}
				}
				else if (redundant)
				{
					spatial_effect = &ai_globals->spatial_effects[effect_index];
					spatial_effect->count += 1;
					submit = spatial_effect->last_tick < resubmit_start_time;

					if (spatial_effect->last_tick < repeating_start_time)
					{
						spatial_effect->position = *position;
						match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 2119, submit);
					}
					else
					{
						real weight;
						real one_minus_weight;

						weight = 1.f / spatial_effect->count;
						one_minus_weight = 1.f - weight;
						spatial_effect->position.x = one_minus_weight * spatial_effect->position.x + weight * position->x;
						spatial_effect->position.y = one_minus_weight * spatial_effect->position.y + weight * position->y;
						spatial_effect->position.z = one_minus_weight * spatial_effect->position.z + weight * position->z;
					}

					break;
				}
			}

			if (!spatial_effect)
			{
				short distance_to_effect;
				short distance_to_end_of_queue;

				if (empty_effect_index == NONE)
				{
					effect_index = ai_globals->spatial_effects_last_index;
					ai_globals->spatial_effects_last_index = NEXT_SPATIAL_EFFECT_INDEX(ai_globals->spatial_effects_last_index);

					if (ai_globals->spatial_effects_last_index == ai_globals->spatial_effects_first_index)
					{
						ai_globals->spatial_effects_first_index = NEXT_SPATIAL_EFFECT_INDEX(ai_globals->spatial_effects_first_index);
					}
				}
				else
				{
					effect_index = empty_effect_index;
				}

				distance_to_effect = (effect_index - ai_globals->spatial_effects_first_index + NUMBER_OF_CACHED_SPATIAL_EFFECTS) & (NUMBER_OF_CACHED_SPATIAL_EFFECTS - 1);
				distance_to_end_of_queue = (ai_globals->spatial_effects_last_index - ai_globals->spatial_effects_first_index + NUMBER_OF_CACHED_SPATIAL_EFFECTS) & (NUMBER_OF_CACHED_SPATIAL_EFFECTS - 1);
				match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 2161, (distance_to_effect >= 0) && (distance_to_effect < distance_to_end_of_queue));
				spatial_effect = &ai_globals->spatial_effects[effect_index];
				spatial_effect->position = *position;
				spatial_effect->last_tick = current_time;
				spatial_effect->type = effect_type;
				spatial_effect->count = 1;
			}

			if (submit)
			{
				actors_handle_spatial_effect(
					object_index,
					spatial_effect->type,
					&spatial_effect->position,
					volume,
					spatial_effect->count);
			}
		}
	}

	return;
}
#pragma optimize("", on)

void ai_disconnect_from_structure_bsp(
	void)
{
	short encounter_index;
	struct scenario *scenario = global_scenario_get();

	for (encounter_index = 0; encounter_index < scenario->ai_encounters.count; encounter_index++)
	{
		struct encounter_datum *encounter = encounter_get(encounter_index);

		if (encounter->active && encounter->current_count > 0)
		{
			struct encounter_actor_iterator iterator;
			struct actor_datum *actor;

			encounter_actor_iterator_new(&iterator, encounter_index);

			while (actor = encounter_actor_iterator_next(&iterator))
			{
				char aibuf[256];
				boolean transition = FALSE;

				ai_debug_describe_actor(iterator.index, NONE, TRUE, aibuf, sizeof(aibuf));

				if (actor->target.target_prop_index != NONE && actor->target.target_type >= _actor_target_uninspected_orphan)
				{
					struct prop_datum *target_prop = prop_get(actor->target.target_prop_index);
					struct prop_datum *prop = target_prop;

					if (target_prop->state >= _prop_state_uninspected_orphan && target_prop->state <= _prop_state_inspected_orphan)
					{
						match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 2233, target_prop->parent_prop_index != NONE);
						prop = prop_get(target_prop->parent_prop_index);
					}

					if (prop->player && actor->state.uncertain_combat_timer != NONE && actor->state.uncertain_combat_timer < 3 * TICKS_PER_SECOND && prop->distance < 10.f)
					{
						transition = TRUE;
					}
				}
				else if (!game_team_is_enemy(actor->meta.team_index, _game_team_player))
				{
					struct prop_iterator prop_iterator;
					struct prop_datum *prop;

					prop_iterator_new(&prop_iterator, iterator.index);

					while (prop = prop_iterator_next(&prop_iterator))
					{
						if (prop->player && (prop->visibility >= _actor_perception_full || prop->distance < 3.f))
						{
							transition = TRUE;
						}
					}
				}

				if (transition && actor->meta.swarm)
				{
					if (actor->meta.swarm_cache_index == NONE)
					{
						transition = FALSE;
					}
					else
					{
						long components_outside_pvs_unit_indices[MAXIMUM_NUMBER_OF_UNITS_PER_SWARM];
						short unit_index_index;
						struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
						unsigned long const *combined_pvs = players_get_combined_pvs();
						short components_outside_pvs_count = 0;

						for (unit_index_index = 0; unit_index_index < swarm->unit_count; unit_index_index++)
						{
							long ultimate_parent_index = object_get_ultimate_parent(swarm->unit_indices[unit_index_index]);
							struct object_datum *object = object_get(ultimate_parent_index);

							if (object->object.location.cluster_index == NONE || !BIT_VECTOR_TEST_FLAG(combined_pvs, object->object.location.cluster_index))
							{
								match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 2283, components_outside_pvs_count < MAXIMUM_NUMBER_OF_UNITS_PER_SWARM);
								components_outside_pvs_unit_indices[components_outside_pvs_count++] = swarm->unit_indices[unit_index_index];
							}
						}

						if (!components_outside_pvs_count)
						{
							if (ai_debug.print_bsp_transition)
							{
								error(
									_error_silent,
									"%s: all swarm inside PVS, transition unchanged",
									aibuf);
							}
						}
						else if (components_outside_pvs_count == swarm->unit_count)
						{
							transition = FALSE;

							if (ai_debug.print_bsp_transition)
							{
								error(
									_error_silent,
									"%s: no units inside PVS, do not transition",
									aibuf);
							}
						}
						else
						{
							short unit_count = swarm->unit_count;
							short reattached_count = 0;

							for (unit_index_index = 0; unit_index_index < components_outside_pvs_count; unit_index_index++)
							{
								long new_actor_index;

								actor_swarm_detach_from_unit(iterator.index, components_outside_pvs_unit_indices[unit_index_index]);
								new_actor_index = actor_create_for_unit(
									TRUE,
									components_outside_pvs_unit_indices[unit_index_index],
									actor->meta.variant_definition_index,
									actor->meta.encounter_index,
									actor->meta.squad_index,
									FALSE,
									iterator.index,
									FALSE,
									_actor_default_state_alert,
									_actor_default_state_none,
									NONE,
									0);

								if (new_actor_index == NONE)
								{
									object_delete(components_outside_pvs_unit_indices[unit_index_index]);
								}
								else
								{
									reattached_count++;
								}
							}

							if (ai_debug.print_bsp_transition)
							{
								error(
									_error_silent,
									"%s: %d of %d units outside PVS, reattached %d%s",
									aibuf,
									components_outside_pvs_count,
									unit_count,
									reattached_count,
									reattached_count < components_outside_pvs_count ? " (deleted some)" : "");
							}
						}
					}
				}

				if (transition)
				{
					if (ai_debug.print_bsp_transition)
					{
						error(
							_error_silent,
							"%s: disconnect and transition to new bsp",
							aibuf);
					}

					actor->meta.disconnected_encounter_index = encounter_index;
					actor->meta.disconnected_squad_index = actor->meta.squad_index;
					actor_flush_position_indices(iterator.index);
					encounter_detach_actor(iterator.index, FALSE);
					encounterless_attach_actor(iterator.index);
				}
			}
		}

		encounter_force_deactivate(encounter_index);
	}

	{
		long actor_index = ai_globals->first_encounterless_actor_index;

		while (actor_index != NONE)
		{
			struct prop_iterator iterator;
			struct prop_datum *prop;
			struct actor_datum *actor = actor_get(actor_index);
			long next_actor_index = actor->meta.next_actor_index;

			match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 2378, actor->meta.encounterless);
			actor_flush_structure_indices(actor_index);
			prop_iterator_new(&iterator, actor_index);

			while (prop = prop_iterator_next(&iterator))
			{
				prop->body_location.cluster_index = NONE;
				prop->body_location.leaf_index = NONE;
				prop->pathfinding_surface_index = NONE;
			}

			actor_index = next_actor_index;
		}
	}

	return;
}

void ai_reconnect_to_structure_bsp(
	void)
{
	short structure_bsp_index = global_structure_bsp_index_get();
	long actor_index = ai_globals->first_encounterless_actor_index;

	while (actor_index != NONE)
	{
		struct actor_datum *actor = actor_get(actor_index);
		long next_actor_index = actor->meta.next_actor_index;

		match_assert("c:\\halo\\SOURCE\\ai\\ai.c", 2415, actor->meta.encounterless);

		if (actor->meta.disconnected_encounter_index != NONE)
		{
			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.disconnected_encounter_index),
				struct encounter_definition);

			if (encounter_definition->runtime_structure_bsp_reference_index == structure_bsp_index)
			{
				encounterless_detach_actor(actor_index);
				encounter_attach_actor(
					actor_index,
					actor->meta.disconnected_encounter_index,
					actor->meta.disconnected_squad_index,
					TRUE);
			}
		}

		actor_index = next_actor_index;
	}

	return;
}

short ai_get_race_from_team_index(
	short team_index)
{
	short race = _race_none;

	if (team_index == _game_team_player)
	{
		race = _race_player;
	}
	else if (team_index == _game_team_human)
	{
		race = _race_human;
	}
	else if (team_index == _game_team_covenant)
	{
		race = _race_covenant;
	}
	else if (team_index == _game_team_flood)
	{
		race = _race_flood;
	}
	else if (team_index == _game_team_sentinel)
	{
		race = _race_sentinel;
	}

	return race;
}

/* ---------- private code */
