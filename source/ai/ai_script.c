/*
AI_SCRIPT.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ai_script.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "ai_globals.h"
#include "network_game_globals.h"
#include "players.h"
#include "console.h"
#include "bipeds.h"
#include "hs.h"
#include "object_lists.h"

/* ---------- constants */

enum
{
	_ai_count_living = 0,
	_ai_count_swarm,
	_ai_count_nonswarm,
	NUMBER_OF_AI_COUNT_TYPES,
};

enum
{
	_ai_command_list_status_none = 0, /* fake name */
	_ai_command_list_status_finished, /* fake name */
	_ai_command_list_status_waiting, /* fake name */
	_ai_command_list_status_running, /* fake name */
	NUMBER_OF_AI_COMMAND_LIST_STATUSES, /* fake name */
};

enum
{
	_ai_scripting_status_inactive = 0, /* fake name */
	_ai_scripting_status_noncombat, /* fake name */
	_ai_scripting_status_guarding, /* fake name */
	_ai_scripting_status_searching, /* fake name */
	_ai_scripting_status_definite_target, /* fake name */
	_ai_scripting_status_visible_target, /* fake name */
	_ai_scripting_status_engaging, /* fake name */
	NUMBER_OF_AI_SCRIPTING_STATUSES, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct ai_script_vehicle_candidate
{
	long actor_index;
	real dist_sq;
	boolean already_entering_vehicle;
};

/* ---------- prototypes */

static void ai_scripting_kill_internal(long ai_index, boolean silent);
static long ai_scripting_count_internal(long ai_index, short count_type, long *original_count_reference, real *strength_reference);
static short ai_scripting_migrate_find_target_squad(long source_encounter_index, short source_squad_index, struct actor_definition const *source_actor, struct actor_variant_definition const *source_actor_variant, boolean same_encounter, long target_ai_index, char const *debug_description);
static void ai_scripting_migrate_internal(long source_index, long target_index, boolean generate_stimulus, boolean advancing_stimulus);
static void ai_scripting_migrate_by_unit_internal(long unit_index, long ai_index, boolean generate_stimulus, boolean advancing_stimulus);
static int __cdecl ai_scripting_vehicle_candidate_qsort(void const *p1, void const *p2);
static void ai_scripting_go_to_vehicle_internal(long ai_index, long unit_index, char const *seat_substring_name, boolean override_previous_vehicles);
static void ai_scripting_teleport_starting_location_private(long ai_index, boolean only_if_unsupported);
static short ai_scripting_command_list_status_internal(long actor_index, long unit_index, short command_list_index, struct obey_individual_simple_control *simple_control, struct obey_individual_complex_control *complex_control);
static short ai_scripting_assess_status(long actor_index);

/* ---------- globals */

char const ai_script_squad_separator = '/';

/* ---------- public code */

void ai_script_initialize(
	void)
{
	return;
}

void ai_script_dispose(
	void)
{
	return;
}

void ai_script_initialize_for_new_map(
	void)
{
	return;
}

void ai_script_dispose_from_old_map(
	void)
{
	return;
}

boolean ai_index_from_string(
	struct scenario *scenario,
	char const *ai_string,
	long *ai_index_reference)
{
	long ai_index = NONE;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 87, ai_string && ai_index_reference);

	if (!_stricmp(ai_string, "none"))
	{
		success = TRUE;
	}
	else
	{
		long encounter_index;
		char const *separator = strrchr(ai_string, ai_script_squad_separator);

		if (!separator)
		{
			encounter_index = scenario_get_encounter_by_name(scenario, ai_string);

			if (encounter_index != NONE)
			{
				ai_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index);
			}
		}
		else if (separator - ai_string <= TAG_STRING_LENGTH)
		{
			char encounter_namebuf[TAG_STRING_LENGTH+1];

			strncpy(encounter_namebuf, ai_string, separator - ai_string);
			encounter_namebuf[separator - ai_string] = 0;
			encounter_index = scenario_get_encounter_by_name(scenario, encounter_namebuf);

			if (encounter_index != NONE)
			{
				struct encounter_definition *encounter = TAG_BLOCK_GET_ELEMENT(
					&scenario->ai_encounters,
					encounter_index,
					struct encounter_definition);
				long squad_index = encounter_definition_get_squad_by_name(encounter, separator + 1);

				if (squad_index != NONE)
				{
					ai_index = AI_INDEX_NEW(_ai_index_squad, encounter_index, squad_index);
				}
				else
				{
					long platoon_index = encounter_definition_get_platoon_by_name(encounter, separator + 1);

					if (platoon_index != NONE)
					{
						ai_index = AI_INDEX_NEW(_ai_index_platoon, encounter_index, platoon_index);
					}
				}
			}
		}

		success = ai_index != NONE;
	}

	*ai_index_reference = ai_index;

	return success;
}

void ai_index_to_string(
	long ai_index,
	struct scenario *scenario,
	char *buffer,
	long bufsize)
{
	if (ai_index == NONE)
	{
		strncpy(buffer, "none", bufsize);
	}
	else
	{
		struct encounter_definition *encounter = TAG_BLOCK_GET_ELEMENT(
			&scenario->ai_encounters,
			AI_INDEX_GET_ENCOUNTER_INDEX(ai_index),
			struct encounter_definition);

		switch (AI_INDEX_GET_TYPE(ai_index))
		{
		case _ai_index_encounter:
			_snprintf(buffer, bufsize, "%s", encounter->name);
			break;
		case _ai_index_platoon:
		{
			struct platoon_definition *platoon = TAG_BLOCK_GET_ELEMENT(
				&encounter->platoons,
				AI_INDEX_GET_SUBINDEX(ai_index),
				struct platoon_definition);

			_snprintf(buffer, bufsize, "%s/%s", encounter->name, platoon->name);
			break;
		}
		case _ai_index_squad:
		{
			struct squad_definition *squad = TAG_BLOCK_GET_ELEMENT(
				&encounter->squads,
				AI_INDEX_GET_SUBINDEX(ai_index),
				struct squad_definition);

			_snprintf(buffer, bufsize, "%s/%s", encounter->name, squad->name);
			break;
		}
		default:
			strncpy(buffer, "<error>", bufsize);
			break;
		}
	}

	return;
}

void ai_index_platoon_iterator_new(
	long ai_index,
	struct ai_index_platoon_iterator *iterator)
{
	struct scenario *scenario = global_scenario_try_and_get();

	match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 191, iterator);

	iterator->encounter_index = AI_INDEX_GET_ENCOUNTER_INDEX(ai_index);

	if (scenario && ai_globals->ai_initialized_for_map && iterator->encounter_index >= 0 && iterator->encounter_index < scenario->ai_encounters.count)
	{
		struct encounter_definition *encounter = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator->encounter_index),
			struct encounter_definition);

		switch (AI_INDEX_GET_TYPE(ai_index))
		{
		case _ai_index_encounter:
			iterator->platoon_index = 0;
			iterator->last_platoon_index = encounter->platoons.count - 1;
			break;
		case _ai_index_platoon:
		case _ai_index_squad:
			if (AI_INDEX_GET_TYPE(ai_index) == _ai_index_squad)
			{
				long squad_index = AI_INDEX_GET_SUBINDEX(ai_index);

				if (squad_index >= 0 && squad_index < encounter->squads.count)
				{
					struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
						&encounter->squads,
						squad_index,
						struct squad_definition);

					iterator->platoon_index = squad_definition->platoon_index;
				}
				else
				{
					iterator->platoon_index = NONE;
				}
			}
			else
			{
				iterator->platoon_index = AI_INDEX_GET_SUBINDEX(ai_index);
			}

			if (iterator->platoon_index >= 0 && iterator->platoon_index < encounter->platoons.count)
			{
				iterator->last_platoon_index = iterator->platoon_index;
			}
			else
			{
				iterator->encounter_index = NONE;
			}
			break;
		default:
			iterator->encounter_index = NONE;
			break;
		}
	}
	else
	{
		iterator->encounter_index = NONE;
	}

	return;
}

struct platoon_datum *ai_index_platoon_iterator_next(
	struct ai_index_platoon_iterator *iterator)
{
	struct platoon_datum *platoon = NULL;

	match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 265, iterator);

	if (iterator->encounter_index != NONE && iterator->platoon_index <= iterator->last_platoon_index)
	{
		platoon = encounter_get_platoon(encounter_get(iterator->encounter_index), (short)iterator->platoon_index);
		iterator->platoon_index++;
	}

	return platoon;
}

void ai_index_squad_iterator_new(
	long ai_index,
	struct ai_index_squad_iterator *iterator)
{
	struct scenario *scenario = global_scenario_try_and_get();

	match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 282, iterator);

	iterator->encounter_index = AI_INDEX_GET_ENCOUNTER_INDEX(ai_index);

	if (scenario && ai_globals->ai_initialized_for_map && iterator->encounter_index >= 0 && iterator->encounter_index < scenario->ai_encounters.count)
	{
		struct encounter_definition *encounter = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator->encounter_index),
			struct encounter_definition);

		switch (AI_INDEX_GET_TYPE(ai_index))
		{
		case _ai_index_encounter:
		case _ai_index_platoon:
			iterator->squad_index = NONE;
			iterator->next_squad_index = 0;
			iterator->last_squad_index = encounter->squads.count - 1;

			if (AI_INDEX_GET_TYPE(ai_index) == _ai_index_encounter)
			{
				iterator->required_platoon_index = NONE;
			}
			else
			{
				iterator->required_platoon_index = AI_INDEX_GET_SUBINDEX(ai_index);
			}
			break;
		case _ai_index_squad:
		{
			short squad_index = (short)AI_INDEX_GET_SUBINDEX(ai_index);

			if (squad_index >= 0 && squad_index < encounter->squads.count)
			{
				iterator->squad_index = NONE;
				iterator->last_squad_index = squad_index;
				iterator->next_squad_index = squad_index;
				iterator->required_platoon_index = NONE;
			}
			else
			{
				iterator->encounter_index = NONE;
			}
			break;
		}
		default:
			iterator->encounter_index = NONE;
			break;
		}
	}
	else
	{
		iterator->encounter_index = NONE;
	}

	return;
}

struct squad_datum *ai_index_squad_iterator_next(
	struct ai_index_squad_iterator *iterator)
{
	struct squad_datum *squad = NULL;

	match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 351, iterator);

	if (iterator->encounter_index != NONE)
	{
		struct encounter_datum *encounter = encounter_get(iterator->encounter_index);
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator->encounter_index),
			struct encounter_definition);

		while (iterator->next_squad_index <= iterator->last_squad_index)
		{
			struct squad_definition *squad_definition;

			iterator->squad_index = iterator->next_squad_index++;
			squad_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->squads,
				iterator->squad_index,
				struct squad_definition);

			if (iterator->required_platoon_index == NONE || squad_definition->platoon_index == iterator->required_platoon_index)
			{
				squad = encounter_get_squad(encounter, (short)iterator->squad_index);
				break;
			}
		}
	}

	return squad;
}

void ai_index_actor_iterator_new(
	long ai_index,
	struct ai_index_actor_iterator *iterator)
{
	struct scenario *scenario = global_scenario_try_and_get();

	match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 384, iterator);

	iterator->encounter_index = AI_INDEX_GET_ENCOUNTER_INDEX(ai_index);

	if (scenario && ai_globals->ai_initialized_for_map && iterator->encounter_index >= 0 && iterator->encounter_index < scenario->ai_encounters.count)
	{
		struct encounter_definition *encounter = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator->encounter_index),
			struct encounter_definition);

		iterator->platoon_index = NONE;
		iterator->squad_index = NONE;

		switch (AI_INDEX_GET_TYPE(ai_index))
		{
		case _ai_index_encounter:
			break;
		case _ai_index_platoon:
			iterator->platoon_index = AI_INDEX_GET_SUBINDEX(ai_index);
			break;
		case _ai_index_squad:
			iterator->squad_index = AI_INDEX_GET_SUBINDEX(ai_index);
			break;
		default:
			iterator->encounter_index = NONE;
			break;
		}

		if (iterator->encounter_index != NONE)
		{
			encounter_actor_iterator_new(&iterator->iterator, iterator->encounter_index);
		}
	}
	else
	{
		iterator->encounter_index = NONE;
	}

	return;
}

struct actor_datum *ai_index_actor_iterator_next(
	struct ai_index_actor_iterator *iterator)
{
	struct actor_datum *actor;

	match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 442, iterator);

	do
	{
		actor = encounter_actor_iterator_next(&iterator->iterator);

		if (actor &&
			(iterator->squad_index == NONE || iterator->squad_index == actor->meta.squad_index) &&
			(iterator->platoon_index == NONE || iterator->platoon_index == actor->meta.platoon_index))
		{
			break;
		}
	}
	while (actor);

	return actor;
}

long object_list_from_ai_reference(
	long ai_index)
{
	long object_list_index = NONE;

	if (ai_index != NONE)
	{
		object_list_index = object_list_new();

		if (object_list_index != NONE)
		{
			struct ai_index_actor_iterator iterator;
			struct actor_datum *actor;

			ai_index_actor_iterator_new(ai_index, &iterator);

			while (actor = ai_index_actor_iterator_next(&iterator))
			{
				long unit_index;

				if (actor->meta.unit_index != NONE)
				{
					object_list_add(object_list_index, actor->meta.unit_index);
				}

				unit_index = actor->meta.swarm_unit_index;

				while (unit_index != NONE)
				{
					struct unit_datum *unit = unit_get(unit_index);

					object_list_add(object_list_index, unit_index);
					unit_index = unit->unit.swarm_next_unit_index;
				}
			}
		}
	}

	return object_list_index;
}

void ai_scripting_attach_unit(
	long unit_index,
	long ai_index)
{
	struct scenario *scenario = global_scenario_get();

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, scenario, aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_attach_unit 0x%04X %s",
			hs_runtime_get_executing_thread_name(),
			DATUM_INDEX_TO_ABSOLUTE_INDEX(unit_index),
			aibuf);
	}
#endif

	if (ai_globals->ai_initialized_for_map && unit_index != NONE && ai_index != NONE)
	{
		long encounter_index = AI_INDEX_GET_ENCOUNTER_INDEX(ai_index);

		if (encounter_index >= 0 && encounter_index < scenario->ai_encounters.count)
		{
			struct encounter_definition *encounter = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
				struct encounter_definition);
			long squad_index = 0;

			if (AI_INDEX_GET_TYPE(ai_index) == _ai_index_squad)
			{
				squad_index = AI_INDEX_GET_SUBINDEX(ai_index);
			}
			else if (AI_INDEX_GET_TYPE(ai_index) == _ai_index_platoon)
			{
				long squad_search_index;
				long platoon_index = AI_INDEX_GET_SUBINDEX(ai_index);

				for (squad_search_index = 0; squad_search_index < encounter->squads.count; squad_search_index++)
				{
					struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
						&encounter->squads,
						squad_search_index,
						struct squad_definition);

					if (squad_definition->platoon_index == platoon_index)
					{
						squad_index = squad_search_index;
						break;
					}
				}
			}

			if (squad_index >= 0 && squad_index < encounter->squads.count)
			{
				struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter->squads,
					squad_index,
					struct squad_definition);
				boolean success = FALSE;

				if (squad_definition->actor_palette_index != NONE)
				{
					struct actor_palette_entry *palette_entry = TAG_BLOCK_GET_ELEMENT(
						&scenario->ai_actor_palette,
						squad_definition->actor_palette_index,
						struct actor_palette_entry);

					if (palette_entry->reference.index != NONE)
					{
						struct actor_variant_definition *actor_variant = actor_variant_definition_get(palette_entry->reference.index);

						if (actor_variant->actor_reference.index != NONE)
						{
							struct actor_definition *actor = actor_definition_get(actor_variant->actor_reference.index);

							success = TRUE;
							actor_create_for_unit(
								TEST_FLAG(actor->flags, _actor_definition_swarm_actor_bit),
								unit_index,
								palette_entry->reference.index,
								encounter_index,
								(short)squad_index,
								FALSE,
								NONE,
								TEST_FLAG(encounter->flags, _encounter_braindead_bit),
								squad_definition->initial_state,
								squad_definition->default_state,
								NONE,
								0);
							encounters_update_dirty_status();
						}
					}
				}

				if (!success)
				{
					error(
						_error_silent,
						"ai_attach: no actor variant specified for %s/%s, cannot create actor to attach to biped",
						encounter->name,
						squad_definition->name);
				}
			}
			else
			{
				error(
					_error_silent,
					"ai_attach: could not find a squad in encounter %s to attach actor",
					encounter->name);
			}
		}
	}

	return;
}

void ai_scripting_attach_units(
	long object_list_index,
	long ai_index)
{
	long object_list_reference_index;
	long unit_index = object_list_get_first(object_list_index, &object_list_reference_index);

	while (unit_index != NONE)
	{
		ai_scripting_attach_unit(unit_index, ai_index);
		unit_index = object_list_get_next(object_list_index, &object_list_reference_index);
	}

	return;
}

void ai_scripting_detach_unit(
	long unit_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_detach_unit 0x%04X",
			hs_runtime_get_executing_thread_name(),
			DATUM_INDEX_TO_ABSOLUTE_INDEX(unit_index));
	}
#endif

	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->unit.actor_index != NONE)
		{
			actor_delete(unit->unit.actor_index, FALSE);
		}
	}

	return;
}

void ai_scripting_detach_units(
	long object_list_index)
{
	long object_list_reference_index;
	long unit_index = object_list_get_first(object_list_index, &object_list_reference_index);

	while (unit_index != NONE)
	{
		ai_scripting_detach_unit(unit_index);
		unit_index = object_list_get_next(object_list_index, &object_list_reference_index);
	}

	return;
}

void ai_scripting_place(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_place %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		long squad_index = AI_INDEX_GET_TYPE(ai_index) == _ai_index_squad ? AI_INDEX_GET_SUBINDEX(ai_index) : NONE;
		long platoon_index = AI_INDEX_GET_TYPE(ai_index) == _ai_index_platoon ? AI_INDEX_GET_SUBINDEX(ai_index) : NONE;

		encounter_create(
			AI_INDEX_GET_ENCOUNTER_INDEX(ai_index),
			platoon_index,
			squad_index);
	}

	return;
}

static void ai_scripting_kill_internal(
	long ai_index,
	boolean silent)
{
	if (ai_index != NONE)
	{
		struct ai_index_actor_iterator iterator;
		struct actor_datum *actor;

		ai_index_actor_iterator_new(ai_index, &iterator);

		while (actor = ai_index_actor_iterator_next(&iterator))
		{
			actor_kill(iterator.iterator.index, silent, FALSE);
		}
	}

	return;
}

void ai_scripting_kill(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_kill %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	ai_scripting_kill_internal(ai_index, FALSE);

	return;
}

void ai_scripting_kill_silent(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_kill_silent %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	ai_scripting_kill_internal(ai_index, TRUE);

	return;
}

void ai_scripting_erase(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_erase %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		long squad_index = AI_INDEX_GET_TYPE(ai_index) == _ai_index_squad ? AI_INDEX_GET_SUBINDEX(ai_index) : NONE;
		long platoon_index = AI_INDEX_GET_TYPE(ai_index) == _ai_index_platoon ? AI_INDEX_GET_SUBINDEX(ai_index) : NONE;

		ai_erase(
			AI_INDEX_GET_ENCOUNTER_INDEX(ai_index),
			platoon_index,
			squad_index,
			FALSE);
	}

	return;
}

void ai_scripting_erase_all(
	void)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_erase_all",
			hs_runtime_get_executing_thread_name());
	}
#endif

	ai_erase(NONE, NONE, NONE, FALSE);

	return;
}

void ai_scripting_deselect(
	void)
{
	if (ai_globals->ai_initialized_for_map)
	{
#ifdef DEBUG
		ai_debug_select_actor(NONE, NONE);
#endif
	}

	return;
}

void ai_scripting_select(
	long ai_index)
{
	if (ai_globals->ai_initialized_for_map)
	{
#ifdef DEBUG
		ai_debug_select_encounter(ai_index == NONE ? NONE : AI_INDEX_GET_ENCOUNTER_INDEX(ai_index));
#endif
	}

	return;
}

void ai_scripting_spawn_actor(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_spawn_actor %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_globals->ai_initialized_for_map && ai_index != NONE)
	{
		long encounter_index = AI_INDEX_GET_ENCOUNTER_INDEX(ai_index);
		short squad_index = AI_INDEX_GET_TYPE(ai_index) == _ai_index_squad ? (short)AI_INDEX_GET_SUBINDEX(ai_index) : NONE;

		if (squad_index == NONE && AI_INDEX_GET_TYPE(ai_index) == _ai_index_platoon)
		{
			short squad_search_index;
			struct encounter_definition *encounter = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
				struct encounter_definition);

			for (squad_search_index = 0; squad_search_index < encounter->squads.count; squad_search_index++)
			{
				struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter->squads,
					squad_search_index,
					struct squad_definition);

				if (squad_definition->platoon_index == AI_INDEX_GET_SUBINDEX(ai_index))
				{
					squad_index = squad_search_index;
					break;
				}
			}
		}

		if (squad_index != NONE)
		{
			encounter_spawn_actor(encounter_index, squad_index);
		}
	}

	return;
}

void ai_scripting_set_respawn(
	long ai_index,
	boolean respawn)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_set_respawn %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			respawn ? "on" : "off");
	}
#endif

	if (ai_index != NONE)
	{
		encounter_set_respawn(AI_INDEX_GET_ENCOUNTER_INDEX(ai_index), respawn);
	}

	return;
}

void ai_scripting_set_deaf(
	long ai_index,
	boolean deaf)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_set_deaf %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			deaf ? "on" : "off");
	}
#endif

	if (ai_index != NONE)
	{
		encounter_set_deaf(AI_INDEX_GET_ENCOUNTER_INDEX(ai_index), deaf);
	}

	return;
}

void ai_scripting_set_blind(
	long ai_index,
	boolean blind)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_set_blind %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			blind ? "on" : "off");
	}
#endif

	if (ai_index != NONE)
	{
		encounter_set_blind(AI_INDEX_GET_ENCOUNTER_INDEX(ai_index), blind);
	}

	return;
}

void ai_scripting_magically_see_encounter(
	long ai_index,
	long target_ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];
		char aibuf2[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		ai_index_to_string(target_ai_index, global_scenario_get(), aibuf2, sizeof(aibuf2));
		error(
			_error_silent,
			"%s: ai_magically_see_encounter %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			aibuf2);
	}
#endif

	if (ai_index != NONE && target_ai_index != NONE)
	{
		struct ai_index_actor_iterator iterator;
		struct actor_datum *actor;

		ai_index_actor_iterator_new(target_ai_index, &iterator);

		while (actor = ai_index_actor_iterator_next(&iterator))
		{
			if (actor->meta.unit_index != NONE)
			{
				ai_scripting_magically_see_unit(ai_index, actor->meta.unit_index);
			}
			else if (actor->meta.swarm_unit_index != NONE)
			{
				ai_scripting_magically_see_unit(ai_index, actor->meta.swarm_unit_index);
			}
		}
	}

	return;
}

void ai_scripting_magically_see_players(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_magically_see_players %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		struct data_iterator iterator;
		struct player_datum *player;

		data_iterator_new(&iterator, player_data);

		while (player = (struct player_datum *)data_iterator_next(&iterator))
		{
			ai_scripting_magically_see_unit(ai_index, player->unit_index);
		}
	}

	return;
}

void ai_scripting_magically_see_unit(
	long ai_index,
	long unit_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_magically_see_unit %s 0x%04X",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(unit_index));
	}
#endif

	if (ai_index != NONE && unit_index != NONE)
	{
		struct ai_index_actor_iterator iterator;
		struct actor_datum *actor;

		ai_index_actor_iterator_new(ai_index, &iterator);

		while (actor = ai_index_actor_iterator_next(&iterator))
		{
			long prop_index;

			if (actor->meta.encounter_index != NONE)
			{
				encounter_force_activate(actor->meta.encounter_index);
			}

			prop_index = prop_get_base_by_unit_index(iterator.iterator.index, unit_index, TRUE, FALSE);

			if (prop_index != NONE)
			{
				actor_handle_unit_effect(iterator.iterator.index, prop_index, _ai_unit_effect_magic_sight);
			}
		}
	}

	return;
}

void ai_scripting_magically_see_units(
	long ai_index,
	long object_list_index)
{
	long object_list_reference_index;
	long unit_index = object_list_get_first(object_list_index, &object_list_reference_index);

	while (unit_index != NONE)
	{
		ai_scripting_magically_see_unit(ai_index, unit_index);
		unit_index = object_list_get_next(object_list_index, &object_list_reference_index);
	}

	return;
}

void ai_scripting_timer_start(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_timer_start %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		struct ai_index_squad_iterator iterator;
		struct squad_datum *squad;

		ai_index_squad_iterator_new(ai_index, &iterator);

		while (squad = ai_index_squad_iterator_next(&iterator))
		{
			squad->delay_timer_started = TRUE;
		}
	}

	return;
}

void ai_scripting_timer_expire(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_timer_expire %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		struct ai_index_squad_iterator iterator;
		struct squad_datum *squad;

		ai_index_squad_iterator_new(ai_index, &iterator);

		while (squad = ai_index_squad_iterator_next(&iterator))
		{
			encounter_squad_timer_expire(iterator.encounter_index, (short)iterator.squad_index);
		}
	}

	return;
}

static long ai_scripting_count_internal(
	long ai_index,
	short count_type,
	long *original_count_reference,
	real *strength_reference)
{
	long count = 0;
	long original_count = 0;
	real strength = 0.f;

	match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 1029, (count_type >= 0) && (count_type < NUMBER_OF_AI_COUNT_TYPES));

	if (ai_index != NONE)
	{
		struct scenario *scenario = global_scenario_get();

		switch (AI_INDEX_GET_TYPE(ai_index))
		{
		case _ai_index_encounter:
		{
			long encounter_index = AI_INDEX_GET_ENCOUNTER_INDEX(ai_index);

			if (encounter_index >= 0 && encounter_index < scenario->ai_encounters.count)
			{
				struct encounter_datum *encounter = encounter_get(encounter_index);

				switch (count_type)
				{
				case _ai_count_living:
					count = encounter->current_count;
					break;
				case _ai_count_swarm:
					count = encounter->current_swarm_count;
					break;
				case _ai_count_nonswarm:
					count = MAX(0, encounter->current_count - encounter->current_swarm_count);
					break;
				default:
					match_unreachable("c:\\halo\\SOURCE\\ai\\ai_script.c", 1060);
					break;
				}

				original_count = encounter->original_count;
				strength = encounter->current_strength_fraction;
			}
			break;
		}
		case _ai_index_platoon:
		{
			long encounter_index = AI_INDEX_GET_ENCOUNTER_INDEX(ai_index);

			if (encounter_index >= 0 && encounter_index < scenario->ai_encounters.count)
			{
				struct encounter_datum *encounter = encounter_get(encounter_index);
				short platoon_index = AI_INDEX_GET_SUBINDEX(ai_index);

				if (platoon_index >= 0 && platoon_index < encounter->platoon_count)
				{
					struct platoon_datum *platoon = encounter_get_platoon(encounter, platoon_index);

					switch (count_type)
					{
					case _ai_count_living:
						count = platoon->current_count;
						break;
					case _ai_count_swarm:
						count = platoon->current_swarm_count;
						break;
					case _ai_count_nonswarm:
						count = MAX(0, platoon->current_count - platoon->current_swarm_count);
						break;
					default:
						match_unreachable("c:\\halo\\SOURCE\\ai\\ai_script.c", 1096);
						break;
					}

					original_count = platoon->original_count;
					strength = platoon->current_strength_fraction;
				}
			}
			break;
		}
		case _ai_index_squad:
		{
			long encounter_index = AI_INDEX_GET_ENCOUNTER_INDEX(ai_index);

			if (encounter_index >= 0 && encounter_index < scenario->ai_encounters.count)
			{
				struct encounter_datum *encounter = encounter_get(encounter_index);
				short squad_index = AI_INDEX_GET_SUBINDEX(ai_index);

				if (squad_index >= 0 && squad_index < encounter->squad_count)
				{
					struct squad_datum *squad = encounter_get_squad(encounter, squad_index);

					switch (count_type)
					{
					case _ai_count_living:
						count = squad->current_count;
						break;
					case _ai_count_swarm:
						count = squad->current_swarm_count;
						break;
					case _ai_count_nonswarm:
						count = MAX(0, squad->current_count - squad->current_swarm_count);
						break;
					default:
						match_unreachable("c:\\halo\\SOURCE\\ai\\ai_script.c", 1133);
						break;
					}

					original_count = squad->original_count;
					strength = squad->current_strength_fraction;
				}
			}
			break;
		}
		default:
			match_unreachable("c:\\halo\\SOURCE\\ai\\ai_script.c", 1143);
			break;
		}
	}

	if (original_count_reference)
	{
		*original_count_reference = original_count;
	}

	if (strength_reference)
	{
		*strength_reference = strength;
	}

	return count;
}

long ai_scripting_swarm_count(
	long ai_index)
{
	return ai_scripting_count_internal(ai_index, _ai_count_swarm, NULL, NULL);
}

long ai_scripting_nonswarm_count(
	long ai_index)
{
	return ai_scripting_count_internal(ai_index, _ai_count_nonswarm, NULL, NULL);
}

long ai_scripting_living_count(
	long ai_index)
{
	return ai_scripting_count_internal(ai_index, _ai_count_living, NULL, NULL);
}

real ai_scripting_living_fraction(
	long ai_index)
{
	long original_count;
	long living_count;
	real fraction = 0.f;

	living_count = ai_scripting_count_internal(ai_index, _ai_count_living, &original_count, NULL);

	if (original_count > 0)
	{
		fraction = (real)living_count / (real)original_count;
	}

	return fraction;
}

real ai_scripting_strength(
	long ai_index)
{
	real strength = 0.f;

	ai_scripting_count_internal(ai_index, _ai_count_living, NULL, &strength);

	return strength;
}

boolean ai_scripting_is_attacking(
	long ai_index)
{
	boolean attacking = FALSE;

	if (ai_index != NONE)
	{
		struct ai_index_platoon_iterator iterator;
		struct platoon_datum *platoon;

		ai_index_platoon_iterator_new(ai_index, &iterator);

		while (platoon = ai_index_platoon_iterator_next(&iterator))
		{
			if (!platoon->defending)
			{
				attacking = TRUE;
				break;
			}
		}
	}

	return attacking;
}

void ai_scripting_attack(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_attack %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		struct ai_index_platoon_iterator iterator;
		struct platoon_datum *platoon;

		ai_index_platoon_iterator_new(ai_index, &iterator);

		while (platoon = ai_index_platoon_iterator_next(&iterator))
		{
			platoon->defending = FALSE;
		}
	}

	return;
}

void ai_scripting_defend(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_defend %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		struct ai_index_platoon_iterator iterator;
		struct platoon_datum *platoon;

		ai_index_platoon_iterator_new(ai_index, &iterator);

		while (platoon = ai_index_platoon_iterator_next(&iterator))
		{
			platoon->defending = TRUE;
		}
	}

	return;
}

void ai_scripting_retreat(
	long ai_index)
{
	ai_scripting_maneuver(ai_index);

	return;
}

void ai_scripting_maneuver(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_maneuver %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		struct ai_index_platoon_iterator iterator;
		struct platoon_datum *platoon;

		ai_index_platoon_iterator_new(ai_index, &iterator);

		while (platoon = ai_index_platoon_iterator_next(&iterator))
		{
			platoon->maneuvering = TRUE;
		}
	}

	return;
}

void ai_scripting_maneuver_enable(
	long ai_index,
	boolean enabled)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_maneuver_enable %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			enabled ? "on" : "off");
	}
#endif

	if (ai_index != NONE)
	{
		struct ai_index_platoon_iterator iterator;
		struct platoon_datum *platoon;

		ai_index_platoon_iterator_new(ai_index, &iterator);

		while (platoon = ai_index_platoon_iterator_next(&iterator))
		{
			platoon->maneuver_disable = !enabled;
		}
	}

	return;
}

static short ai_scripting_migrate_find_target_squad(
	long source_encounter_index,
	short source_squad_index,
	struct actor_definition const *source_actor,
	struct actor_variant_definition const *source_actor_variant,
	boolean same_encounter,
	long target_ai_index,
	char const *debug_description)
{
	struct ai_index_squad_iterator target_iterator;
	struct squad_datum *squad;
	struct scenario *scenario = global_scenario_get();
	long target_encounter_index = AI_INDEX_GET_ENCOUNTER_INDEX(target_ai_index);
	struct encounter_definition *target_encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(target_encounter_index),
		struct encounter_definition);
	short any_squad_index = NONE;
	short same_type_squad_index = NONE;
	short same_actor_squad_index = NONE;
	short same_variant_squad_index = NONE;
	short same_squad_index = NONE;
	short found_squad_index = NONE;

	ai_index_squad_iterator_new(target_ai_index, &target_iterator);

	while (squad = ai_index_squad_iterator_next(&target_iterator))
	{
		struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
			&target_encounter_definition->squads,
			target_iterator.squad_index,
			struct squad_definition);
		struct actor_variant_definition *actor_variant = NULL;
		struct actor_definition *actor = NULL;

		if (squad_definition->actor_palette_index >= 0 && squad_definition->actor_palette_index < scenario->ai_actor_palette.count)
		{
			struct actor_palette_entry *palette_entry = TAG_BLOCK_GET_ELEMENT(
				&scenario->ai_actor_palette,
				squad_definition->actor_palette_index,
				struct actor_palette_entry);

			if (palette_entry->reference.index != NONE && tag_get_group_tag(palette_entry->reference.index) == ACTOR_VARIANT_DEFINITION_TAG)
			{
				actor_variant = actor_variant_definition_get(palette_entry->reference.index);

				if (actor_variant->actor_reference.index != NONE)
				{
					actor = actor_definition_get(actor_variant->actor_reference.index);
				}
			}
		}

		if (same_squad_index == NONE && same_encounter && source_squad_index == target_iterator.squad_index)
		{
			same_squad_index = (short)target_iterator.squad_index;
		}

		if (same_variant_squad_index == NONE && source_actor_variant && actor_variant && source_actor_variant == actor_variant)
		{
			same_variant_squad_index = (short)target_iterator.squad_index;
		}

		if (same_actor_squad_index == NONE && source_actor && actor && source_actor == actor)
		{
			same_actor_squad_index = (short)target_iterator.squad_index;
		}

		if (same_type_squad_index == NONE && source_actor && actor && source_actor->type == actor->type)
		{
			same_type_squad_index = (short)target_iterator.squad_index;
		}

		if (any_squad_index == NONE)
		{
			any_squad_index = (short)target_iterator.squad_index;
		}
	}

	if (same_squad_index != NONE)
	{
		found_squad_index = same_squad_index;

#ifdef DEBUG
		if (ai_debug.print_migration)
		{
			error(
				_error_silent,
				"  %s unchanged",
				debug_description);
		}
#endif
	}
	else if (same_variant_squad_index != NONE)
	{
		found_squad_index = same_variant_squad_index;

#ifdef DEBUG
		if (ai_debug.print_migration)
		{
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&target_encounter_definition->squads,
				same_variant_squad_index,
				struct squad_definition);
			char const *variant_name = "<error>";

			if (source_encounter_index != NONE)
			{
				struct encounter_definition *source_encounter_definition = TAG_BLOCK_GET_ELEMENT(
					&global_scenario_get()->ai_encounters,
					DATUM_INDEX_TO_ABSOLUTE_INDEX(source_encounter_index),
					struct encounter_definition);

				if (source_squad_index != NONE)
				{
					struct squad_definition *source_squad_definition = TAG_BLOCK_GET_ELEMENT(
						&source_encounter_definition->squads,
						source_squad_index,
						struct squad_definition);

					if (source_squad_definition->actor_palette_index >= 0 && source_squad_definition->actor_palette_index < scenario->ai_actor_palette.count)
					{
						struct actor_palette_entry *palette_entry = TAG_BLOCK_GET_ELEMENT(
							&scenario->ai_actor_palette,
							source_squad_definition->actor_palette_index,
							struct actor_palette_entry);

						if (palette_entry->reference.index != NONE)
						{
							variant_name = tag_get_name(palette_entry->reference.index);
						}
					}
				}
			}

			error(
				_error_silent,
				"  %s -> %s (same-variant %s)",
				debug_description,
				squad_definition->name,
				variant_name);
		}
#endif
	}
	else if (same_actor_squad_index != NONE)
	{
		found_squad_index = same_actor_squad_index;

#ifdef DEBUG
		if (ai_debug.print_migration)
		{
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&target_encounter_definition->squads,
				same_actor_squad_index,
				struct squad_definition);
			char const *actor_name = "<error>";

			if (source_actor_variant)
			{
				if (source_actor_variant->actor_reference.index == NONE)
				{
					actor_name = "<none>";
				}
				else
				{
					actor_name = tag_get_name(source_actor_variant->actor_reference.index);
				}
			}

			error(
				_error_silent,
				"  %s -> %s (same-actor %s)",
				debug_description,
				squad_definition->name,
				actor_name);
		}
#endif
	}
	else if (same_type_squad_index != NONE)
	{
		found_squad_index = same_type_squad_index;

#ifdef DEBUG
		if (ai_debug.print_migration)
		{
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&target_encounter_definition->squads,
				same_type_squad_index,
				struct squad_definition);

			error(
				_error_silent,
				"  %s -> %s (same-type %s)",
				debug_description,
				squad_definition->name,
				actor_type_get_name(source_actor->type));
		}
#endif
	}
	else if (any_squad_index != NONE)
	{
		found_squad_index = any_squad_index;

#ifdef DEBUG
		if (ai_debug.print_migration)
		{
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&target_encounter_definition->squads,
				any_squad_index,
				struct squad_definition);

			error(
				_error_silent,
				"  %s -> %s (no matching types found)",
				debug_description,
				squad_definition->name);
		}
#endif
	}
	else if (target_encounter_definition->squads.count > 0)
	{
		found_squad_index = 0;

#ifdef DEBUG
		if (ai_debug.print_migration)
		{
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&target_encounter_definition->squads,
				0,
				struct squad_definition);

			error(
				_error_silent,
				"  %s -> %s (no matching squads found, going to squad 0)",
				debug_description,
				squad_definition->name);
		}
#endif
	}

	if (found_squad_index != NONE)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 1532, (found_squad_index >= 0) && (found_squad_index < target_encounter_definition->squads.count));
	}

	return found_squad_index;
}

static void ai_scripting_migrate_internal(
	long source_index,
	long target_index,
	boolean generate_stimulus,
	boolean advancing_stimulus)
{
	if (source_index != NONE && target_index != NONE)
	{
		long source_encounter_index = AI_INDEX_GET_ENCOUNTER_INDEX(source_index);
		long target_encounter_index = AI_INDEX_GET_ENCOUNTER_INDEX(target_index);

		if (source_encounter_index != NONE && target_encounter_index != NONE)
		{
			short target_squad_indices[MAXIMUM_SQUADS_PER_ENCOUNTER];
			struct scenario *scenario = global_scenario_get();
			struct encounter_datum *source_encounter = encounter_get(source_encounter_index);
			struct encounter_datum *target_encounter = encounter_get(target_encounter_index);
			struct encounter_definition *source_encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(source_encounter_index),
				struct encounter_definition);
			struct encounter_definition *target_encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(target_encounter_index),
				struct encounter_definition);
			boolean same_encounter = source_encounter_index == target_encounter_index;

			memset(target_squad_indices, NONE, sizeof(target_squad_indices));

			{
				struct ai_index_squad_iterator source_iterator;
				struct squad_datum *squad;

				ai_index_squad_iterator_new(source_index, &source_iterator);

				while (squad = ai_index_squad_iterator_next(&source_iterator))
				{
					match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 1571, (source_iterator.squad_index >= 0) && (source_iterator.squad_index < MAXIMUM_SQUADS_PER_ENCOUNTER));

					if (squad->current_count > 0 || source_encounter->is_prevehicle_encounter)
					{
						struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
							&source_encounter_definition->squads,
							source_iterator.squad_index,
							struct squad_definition);
						struct actor_variant_definition *actor_variant = NULL;
						struct actor_definition *actor = NULL;
						char const *debug_description = NULL;

						if (squad_definition->actor_palette_index >= 0 && squad_definition->actor_palette_index < scenario->ai_actor_palette.count)
						{
							struct actor_palette_entry *palette_entry = TAG_BLOCK_GET_ELEMENT(
								&scenario->ai_actor_palette,
								squad_definition->actor_palette_index,
								struct actor_palette_entry);

							if (palette_entry->reference.index != NONE && tag_get_group_tag(palette_entry->reference.index) == ACTOR_VARIANT_DEFINITION_TAG)
							{
								actor_variant = actor_variant_definition_get(palette_entry->reference.index);

								if (actor_variant->actor_reference.index != NONE)
								{
									actor = actor_definition_get(actor_variant->actor_reference.index);
								}
							}
						}

						sprintf(temporary, "squad %s", squad_definition->name);
						debug_description = temporary;
						target_squad_indices[source_iterator.squad_index] = ai_scripting_migrate_find_target_squad(
							source_encounter_index,
							(short)source_iterator.squad_index,
							actor,
							actor_variant,
							same_encounter,
							target_index,
							debug_description);
					}
				}
			}

			{
				struct encounter_actor_iterator iterator;
				struct actor_datum *actor;

				encounter_actor_iterator_new(&iterator, source_encounter_index);

				while (actor = encounter_actor_iterator_next(&iterator))
				{
					short current_squad_index = actor->meta.squad_index;

					match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 1617, (current_squad_index >= 0) && (current_squad_index < MAXIMUM_SQUADS_PER_ENCOUNTER));

					if (target_squad_indices[current_squad_index] == NONE)
					{
						// this squad is not migrating
					}
					else if (same_encounter && target_squad_indices[current_squad_index] == current_squad_index)
					{
						// this squad is staying where it is
					}
					else
					{
						match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 1629, (target_squad_indices[current_squad_index] >= 0) && (target_squad_indices[current_squad_index] < target_encounter_definition->squads.count));

						actor_change_encounter(
							iterator.index,
							target_encounter_index,
							target_squad_indices[current_squad_index]);

						if (generate_stimulus)
						{
							actor_stimulus_maneuvering(iterator.index, advancing_stimulus, FALSE);
						}
					}
				}
			}

			if (source_encounter->is_prevehicle_encounter)
			{
				struct actor_iterator iterator;
				struct actor_datum *actor;

				actor_iterator_new(&iterator, FALSE);

				while (actor = actor_iterator_next(&iterator))
				{
					if (DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.prevehicle_encounter_index) == DATUM_INDEX_TO_ABSOLUTE_INDEX(source_encounter_index))
					{
						short current_squad_index = actor->meta.prevehicle_squad_index;

						match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 1655, (current_squad_index >= 0) && (current_squad_index < MAXIMUM_SQUADS_PER_ENCOUNTER));

						if (target_squad_indices[current_squad_index] == NONE)
						{
							// this squad is not migrating
						}
						else if (same_encounter && target_squad_indices[current_squad_index] == current_squad_index)
						{
							// this squad is staying where it is
						}
						else
						{
							match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 1667, (target_squad_indices[current_squad_index] >= 0) && (target_squad_indices[current_squad_index] < target_encounter_definition->squads.count));

							actor->meta.prevehicle_encounter_index = target_encounter_index;
							actor->meta.prevehicle_squad_index = target_squad_indices[current_squad_index];
						}
					}
				}

				if (!same_encounter)
				{
					if (AI_INDEX_GET_TYPE(source_encounter_index) == _ai_index_encounter)
					{
						source_encounter->is_prevehicle_encounter = FALSE;
					}

					target_encounter->is_prevehicle_encounter = TRUE;
				}
			}

			{
				struct encounter_actor_iterator iterator;
				struct actor_datum *actor;

				encounter_actor_iterator_new(&iterator, NONE);

				while (actor = encounter_actor_iterator_next(&iterator))
				{
					if (DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.disconnected_encounter_index) == DATUM_INDEX_TO_ABSOLUTE_INDEX(source_encounter_index))
					{
						short current_squad_index = actor->meta.disconnected_squad_index;

						match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 1700, (current_squad_index >= 0) && (current_squad_index < MAXIMUM_SQUADS_PER_ENCOUNTER));

						if (target_squad_indices[current_squad_index] == NONE)
						{
							// this squad is not migrating
						}
						else if (same_encounter && target_squad_indices[current_squad_index] == current_squad_index)
						{
							// this squad is staying where it is
						}
						else
						{
							match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 1712, (target_squad_indices[current_squad_index] >= 0) && (target_squad_indices[current_squad_index] < target_encounter_definition->squads.count));

							actor->meta.disconnected_encounter_index = target_encounter_index;
							actor->meta.disconnected_squad_index = target_squad_indices[current_squad_index];

							if (!same_encounter)
							{
								if (target_encounter_definition->runtime_structure_bsp_reference_index == global_structure_bsp_index_get())
								{
									encounterless_detach_actor(iterator.index);
									encounter_attach_actor(
										iterator.index,
										actor->meta.disconnected_encounter_index,
										actor->meta.disconnected_squad_index,
										TRUE);
								}
							}
						}
					}
				}
			}

			if (source_encounter->team_index != target_encounter->team_index)
			{
				ai_update_team_status();
			}

			encounters_update_dirty_status();
		}
	}

	return;
}

void ai_scripting_migrate(
	long source_index,
	long target_index)
{
#ifdef DEBUG
	if (ai_debug.print_migration || ai_debug.print_scripting)
	{
		char source_name[512];
		char target_name[512];

		ai_index_to_string(source_index, global_scenario_get(), source_name, sizeof(source_name));
		ai_index_to_string(target_index, global_scenario_get(), target_name, sizeof(target_name));
		error(
			_error_silent,
			"%s: ai_migrate %s %s",
			hs_runtime_get_executing_thread_name(),
			source_name,
			target_name);
	}
#endif

	ai_scripting_migrate_internal(source_index, target_index, FALSE, FALSE);

	return;
}

static void ai_scripting_migrate_by_unit_internal(
	long unit_index,
	long ai_index,
	boolean generate_stimulus,
	boolean advancing_stimulus)
{
	struct unit_datum *unit = unit_get(unit_index);
	long actor_index = unit->unit.actor_index != NONE ? unit->unit.actor_index : unit->unit.swarm_actor_index;

	if (actor_index != NONE && ai_index != NONE)
	{
		struct actor_datum *actor = actor_get(actor_index);
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
		struct actor_variant_definition *actor_variant = actor_variant_definition_get(actor->meta.variant_definition_index);
		long target_encounter_index = AI_INDEX_GET_ENCOUNTER_INDEX(ai_index);
		boolean same_encounter = DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index) == DATUM_INDEX_TO_ABSOLUTE_INDEX(target_encounter_index);
		short squad_index = ai_scripting_migrate_find_target_squad(
			actor->meta.encounter_index,
			actor->meta.squad_index,
			actor_definition,
			actor_variant,
			same_encounter,
			ai_index,
			"unit");

		if (squad_index == NONE)
		{
			// this actor is not migrating
		}
		else if (same_encounter && squad_index == actor->meta.squad_index)
		{
			// this actor is staying where it is
		}
		else
		{
			actor_change_encounter(actor_index, target_encounter_index, squad_index);

			if (generate_stimulus)
			{
				actor_stimulus_maneuvering(actor_index, advancing_stimulus, FALSE);
			}
		}
	}

	return;
}

void ai_scripting_migrate_by_unit(
	long object_list_index,
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_migration || ai_debug.print_scripting)
	{
		char target_name[512];

		ai_index_to_string(ai_index, global_scenario_get(), target_name, sizeof(target_name));
		error(
			_error_silent,
			"%s: ai_migrate_by_unit <some guys> %s",
			hs_runtime_get_executing_thread_name(),
			target_name);
	}
#endif

	if (object_list_index != NONE && ai_index != NONE)
	{
		long object_list_reference_index;
		long unit_index = object_list_get_first(object_list_index, &object_list_reference_index);

		while (unit_index != NONE)
		{
			struct unit_datum *unit = unit_try_and_get(unit_index);

			if (unit)
			{
				long child_object_index;

				ai_scripting_migrate_by_unit_internal(unit_index, ai_index, FALSE, FALSE);
				child_object_index = unit->object.first_child_object_index;

				while (child_object_index != NONE)
				{
					struct object_datum *child_object = object_get(child_object_index);

					if (TEST_FLAG(_object_mask_unit, child_object->object.type))
					{
						ai_scripting_migrate_by_unit_internal(child_object_index, ai_index, FALSE, FALSE);
					}

					child_object_index = child_object->object.next_object_index;
				}
			}

			unit_index = object_list_get_next(object_list_index, &object_list_reference_index);
		}

		ai_update_team_status();
		encounters_update_dirty_status();
	}

	return;
}

void ai_scripting_migrate_and_speak(
	long source_index,
	long target_index,
	char const *speech_type)
{
	boolean advancing;

#ifdef DEBUG
	if (ai_debug.print_migration || ai_debug.print_scripting)
	{
		char source_name[512];
		char target_name[512];

		ai_index_to_string(source_index, global_scenario_get(), source_name, sizeof(source_name));
		ai_index_to_string(target_index, global_scenario_get(), target_name, sizeof(target_name));
		error(
			_error_silent,
			"%s: ai_migrate_and_speak %s %s %s",
			hs_runtime_get_executing_thread_name(),
			source_name,
			target_name);
	}
#endif

	if (!_stricmp(speech_type, "advance"))
	{
		advancing = TRUE;
	}
	else if (!_stricmp(speech_type, "retreat"))
	{
		advancing = FALSE;
	}
	else
	{
		error(
			_error_silent,
			"ai_migrate_and_speak: unknown speech type '%s' (must be 'advance' or 'retreat')",
			speech_type);
		advancing = FALSE;
	}

	ai_scripting_migrate_internal(source_index, target_index, TRUE, advancing);

	return;
}

void ai_scripting_allegiance(
	short team1_index,
	short team2_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_allegiance %d %d",
			hs_runtime_get_executing_thread_name(),
			team1_index,
			team2_index);
	}
#endif

	if (team1_index != NONE && team2_index != NONE)
	{
		short ally_team_index = NONE;
		short incident_threshold = NONE;
		short incident_decay_time = NONE;
		boolean allegiance_requires_communication = FALSE;
		boolean suspicious = FALSE;

		if (team1_index == _game_team_player)
		{
			ally_team_index = team2_index;
		}
		else if (team2_index == _game_team_player)
		{
			ally_team_index = team1_index;
		}

		switch (ally_team_index)
		{
		case _game_team_human:
		case _game_team_sentinel:
		{
			short const traitor_timer[NUMBER_OF_GAME_DIFFICULTY_LEVELS] = { 300, 450, 1200, 2700 };

			suspicious = TRUE;
			incident_threshold = 5;
			incident_decay_time = traitor_timer[game_difficulty_level_get()];
			allegiance_requires_communication = ally_team_index == _game_team_human;
			break;
		}
		}

		game_allegiance_create(
			team1_index,
			suspicious && team1_index == ally_team_index,
			team2_index,
			suspicious && team2_index == ally_team_index,
			incident_threshold,
			incident_decay_time,
			allegiance_requires_communication);
	}

	return;
}

void ai_scripting_allegiance_remove(
	short team1_index,
	short team2_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_allegiance_remove %d %d",
			hs_runtime_get_executing_thread_name(),
			team1_index,
			team2_index);
	}
#endif

	if (team1_index != NONE && team2_index != NONE)
	{
		game_allegiance_remove(team1_index, team2_index);
	}

	return;
}

boolean ai_scripting_allegiance_broken(
	short team1_index,
	short team2_index)
{
	boolean broken = FALSE;

	if (team1_index != NONE && team2_index != NONE)
	{
		broken = game_team_is_ally(team1_index, team2_index) && game_team_is_enemy(team1_index, team2_index);
	}

	return broken;
}

static int __cdecl ai_scripting_vehicle_candidate_qsort(
	void const *p1,
	void const *p2)
{
	struct ai_script_vehicle_candidate const *candidate1 = p1;
	struct ai_script_vehicle_candidate const *candidate2 = p2;

	if (candidate1->already_entering_vehicle != candidate2->already_entering_vehicle)
	{
		return candidate1->already_entering_vehicle ? 1 : -1;
	}
	else if (candidate1->dist_sq < candidate2->dist_sq)
	{
		return -1;
	}
	else if (candidate1->dist_sq > candidate2->dist_sq)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static void ai_scripting_go_to_vehicle_internal(
	long ai_index,
	long unit_index,
	char const *seat_substring_name,
	boolean override_previous_vehicles)
{
	struct unit_datum *unit = unit_try_and_get(unit_index);

	if (ai_index != NONE && unit)
	{
		short seat_indices[16];
		struct ai_index_actor_iterator iterator;
		struct ai_script_vehicle_candidate candidates[64];
		real_point3d vehicle_position;
		short candidate_count = 0;
		short available_seat_count = 0;

		object_get_origin(unit_index, &vehicle_position);
		available_seat_count = vehicle_scripting_find_available_seats(
			unit_index,
			seat_substring_name,
			NONE,
			seat_indices,
			NUMBEROF(seat_indices));

		if (available_seat_count > 0)
		{
			struct actor_datum *actor;
			short candidate_index;

			ai_index_actor_iterator_new(ai_index, &iterator);

			while (actor = ai_index_actor_iterator_next(&iterator))
			{
				if (candidate_count < NUMBEROF(candidates))
				{
					candidates[candidate_count].actor_index = iterator.iterator.index;
					candidates[candidate_count].dist_sq = distance_squared3d(&actor->input.position.body_position, &vehicle_position);
					candidates[candidate_count].already_entering_vehicle = actor->state.action == _actor_action_vehicle;
					candidate_count++;
				}
			}

			qsort(candidates, candidate_count, sizeof(*candidates), ai_scripting_vehicle_candidate_qsort);

			for (candidate_index = 0; candidate_index < candidate_count; candidate_index++)
			{
				if (candidates[candidate_index].already_entering_vehicle && !override_previous_vehicles)
				{
					break;
				}

				actor_action_try_to_enter_vehicle(
					candidates[candidate_index].actor_index,
					unit_index,
					NULL,
					NONE,
					available_seat_count,
					seat_indices);
			}
		}
	}

	return;
}

void ai_scripting_go_to_vehicle(
	long ai_index,
	long unit_index,
	char const *seat_substring_name)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_go_to_vehicle %s 0x%04X %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(unit_index),
			seat_substring_name);
	}
#endif

	ai_scripting_go_to_vehicle_internal(ai_index, unit_index, seat_substring_name, FALSE);

	return;
}

void ai_scripting_go_to_vehicle_override(
	long ai_index,
	long unit_index,
	char const *seat_substring_name)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_go_to_vehicle_override %s 0x%04X %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(unit_index),
			seat_substring_name);
	}
#endif

	ai_scripting_go_to_vehicle_internal(ai_index, unit_index, seat_substring_name, TRUE);

	return;
}

short ai_scripting_going_to_vehicle(
	long unit_index)
{
	struct actor_iterator iterator;
	struct actor_datum *actor;
	short count = 0;

	actor_iterator_new(&iterator, TRUE);

	while (actor = actor_iterator_next(&iterator))
	{
		if (actor->state.action == _actor_action_vehicle && actor->state.action_data.vehicle.vehicle_index == unit_index)
		{
			count++;
		}
	}

	return count;
}

void ai_scripting_exit_vehicle(
	long ai_index)
{
	struct ai_index_actor_iterator iterator;
	struct actor_datum *actor;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_exit_vehicle %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	ai_index_actor_iterator_new(ai_index, &iterator);

	while (actor = ai_index_actor_iterator_next(&iterator))
	{
		if (actor->input.vehicle_index != NONE && actor->meta.unit_index != NONE)
		{
			unit_try_and_exit_seat(actor->meta.unit_index);
		}
	}

	return;
}

void ai_scripting_braindead(
	long ai_index,
	boolean braindead)
{
	struct ai_index_actor_iterator iterator;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_braindead %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			braindead ? "on" : "off");
	}
#endif

	if (ai_index != NONE)
	{
		ai_index_actor_iterator_new(ai_index, &iterator);

		while (ai_index_actor_iterator_next(&iterator))
		{
			actor_braindead(iterator.iterator.index, braindead);
		}
	}

	return;
}

void ai_scripting_braindead_by_unit(
	long object_list_index,
	boolean braindead)
{
	long object_list_reference_index;
	long object_index = object_list_get_first(object_list_index, &object_list_reference_index);

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_braindead_by_unit <some guys> %s",
			hs_runtime_get_executing_thread_name(),
			braindead ? "on" : "off");
	}
#endif

	while (object_index != NONE)
	{
		struct unit_datum *unit = unit_try_and_get(object_index);

		if (unit)
		{
			long child_object_index;
			long actor_index = unit->unit.actor_index != NONE ? unit->unit.actor_index : unit->unit.swarm_actor_index;

			if (actor_index != NONE)
			{
				actor_braindead(actor_index, braindead);
			}

			child_object_index = unit->object.first_child_object_index;

			while (child_object_index != NONE)
			{
				struct object_datum *child_object = object_get(child_object_index);

				if (TEST_FLAG(_object_mask_unit, child_object->object.type))
				{
					struct unit_datum *child_unit = (struct unit_datum *)child_object;
					long child_actor_index = child_unit->unit.actor_index != NONE ? child_unit->unit.actor_index : child_unit->unit.swarm_actor_index;

					if (child_actor_index != NONE)
					{
						actor_braindead(child_actor_index, braindead);
					}
				}

				child_object_index = child_object->object.next_object_index;
			}
		}

		object_index = object_list_get_next(object_list_index, &object_list_reference_index);
	}

	return;
}

void ai_scripting_ignore(
	long object_list_index,
	boolean ignore)
{
	long object_list_reference_index;
	long object_index = object_list_get_first(object_list_index, &object_list_reference_index);

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_disregard <some guys> %s",
			hs_runtime_get_executing_thread_name(),
			ignore ? "on" : "off");
	}
#endif

	while (object_index != NONE)
	{
		struct unit_datum *unit = unit_try_and_get(object_index);

		if (unit)
		{
			SET_FLAG(unit->unit.flags, _unit_ignored_by_actors_bit, ignore);
		}

		object_index = object_list_get_next(object_list_index, &object_list_reference_index);
	}

	return;
}

void ai_scripting_prefer_target(
	long object_list_index,
	boolean prefer)
{
	long object_list_reference_index;
	long object_index = object_list_get_first(object_list_index, &object_list_reference_index);

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_prefer_target <some guys> %s",
			hs_runtime_get_executing_thread_name(),
			prefer ? "on" : "off");
	}
#endif

	while (object_index != NONE)
	{
		struct unit_datum *unit = unit_try_and_get(object_index);

		if (unit)
		{
			SET_FLAG(unit->unit.flags, _unit_preferred_target_bit, prefer);
		}

		object_index = object_list_get_next(object_list_index, &object_list_reference_index);
	}

	return;
}

static void ai_scripting_teleport_starting_location_private(
	long ai_index,
	boolean only_if_unsupported)
{
	struct ai_index_actor_iterator iterator;
	struct actor_datum *actor;

	ai_index_actor_iterator_new(ai_index, &iterator);

	while (actor = ai_index_actor_iterator_next(&iterator))
	{
		if (actor->meta.unit_index == NONE)
		{
			continue;
		}

		if (only_if_unsupported && (actor->input.vehicle_index != NONE || biped_approximate_surface_index(actor->meta.unit_index, NULL) != NONE))
		{
			continue;
		}

		if (actor->meta.encounter_index != NONE)
		{
			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
				struct encounter_definition);
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->squads,
				actor->meta.squad_index,
				struct squad_definition);
			short starting_location_index = encounter_get_actor_starting_location(
				actor->meta.encounter_index,
				actor->meta.squad_index,
				TRUE);

			if (starting_location_index != NONE)
			{
				struct actor_starting_location_definition *starting_location = TAG_BLOCK_GET_ELEMENT(
					&squad_definition->starting_locations,
					starting_location_index,
					struct actor_starting_location_definition);
				real_vector3d forward;

				vector3d_from_angle(&forward, starting_location->facing);
				object_set_position(actor->meta.unit_index, &starting_location->position, &forward, NULL);
				object_reset(actor->meta.unit_index);
				actor_move_halt(iterator.iterator.index);
			}
		}
	}

	return;
}

void ai_scripting_teleport_starting_location_if_unsupported(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_teleport_starting_location_if_unsupported %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	ai_scripting_teleport_starting_location_private(ai_index, TRUE);

	return;
}

void ai_scripting_teleport_starting_location(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_teleport_starting_location %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	ai_scripting_teleport_starting_location_private(ai_index, FALSE);

	return;
}

void ai_scripting_renew(
	long ai_index)
{
	struct ai_index_actor_iterator iterator;
	struct actor_datum *actor;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_renew %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	ai_index_actor_iterator_new(ai_index, &iterator);

	while (actor = ai_index_actor_iterator_next(&iterator))
	{
		if (actor->meta.unit_index != NONE)
		{
			struct actor_variant_definition *variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
			struct unit_datum *unit = unit_get(actor->meta.unit_index);

			unit->object.body_vitality = unit->object.maximum_body_vitality > 0.f ? 1.f : 0.f;
			unit->object.shield_vitality = unit->object.maximum_shield_vitality > 0.f ? 1.f : 0.f;

			if (variant_definition->grenade_combat.grenade_type != NONE)
			{
				short grenade_count = random_range(
					variant_definition->items.grenades_lower_bound,
					variant_definition->items.grenades_upper_bound + 1);
				short current_grenade_count = unit_get_grenade_count(
					actor->meta.unit_index,
					unit_get_current_grenade_type(actor->meta.unit_index));

				if (current_grenade_count < grenade_count)
				{
					unit_add_grenade_type_to_inventory(
						actor->meta.unit_index,
						variant_definition->grenade_combat.grenade_type,
						grenade_count - current_grenade_count);
				}
			}
		}
	}

	return;
}

void ai_scripting_try_to_fight_nothing(
	long ai_index)
{
	struct ai_index_actor_iterator iterator;
	struct actor_datum *actor;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_try_to_fight_nothing %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	ai_index_actor_iterator_new(ai_index, &iterator);

	while (actor = ai_index_actor_iterator_next(&iterator))
	{
		actor->external_orders.desired_target_type = _desired_target_none;
	}

	return;
}

void ai_scripting_try_to_fight(
	long ai_index,
	long target_ai_index)
{
	struct ai_index_actor_iterator iterator;
	struct actor_datum *actor;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];
		char aibuf2[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		ai_index_to_string(target_ai_index, global_scenario_get(), aibuf2, sizeof(aibuf2));
		error(
			_error_silent,
			"%s: ai_try_to_fight %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			aibuf2);
	}
#endif

	ai_index_actor_iterator_new(ai_index, &iterator);

	while (actor = ai_index_actor_iterator_next(&iterator))
	{
		actor->external_orders.desired_target_type = _desired_target_ai;
		actor->external_orders.desired_target_ai_index = target_ai_index;
	}

	return;
}

void ai_scripting_try_to_fight_player(
	long ai_index)
{
	struct ai_index_actor_iterator iterator;
	struct actor_datum *actor;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_try_to_fight_player %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	ai_index_actor_iterator_new(ai_index, &iterator);

	while (actor = ai_index_actor_iterator_next(&iterator))
	{
		actor->external_orders.desired_target_type = _desired_target_player;
	}

	return;
}

void ai_scripting_allow_charge(
	long ai_index,
	boolean allow_charge)
{
	struct ai_index_actor_iterator iterator;
	struct actor_datum *actor;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_allow_charge %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			allow_charge ? "true" : "false");
	}
#endif

	ai_index_actor_iterator_new(ai_index, &iterator);

	while (actor = ai_index_actor_iterator_next(&iterator))
	{
		actor->external_orders.disable_charging = !allow_charge;
	}

	return;
}

void ai_scripting_command_list(
	long ai_index,
	short command_list_index)
{
	struct ai_index_actor_iterator iterator;
	struct actor_datum *actor;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_command_list %s %d",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			command_list_index);
	}
#endif

	ai_index_actor_iterator_new(ai_index, &iterator);

	while (actor = ai_index_actor_iterator_next(&iterator))
	{
		struct action_state_data new_state_data;

		if (action_obey_command_list_setup(iterator.iterator.index, command_list_index, &new_state_data.obey))
		{
			actor_action_change(iterator.iterator.index, _actor_action_obey, &new_state_data);
		}
	}

	return;
}

void ai_scripting_command_list_by_unit(
	long unit_index,
	short command_list_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_command_list_by_unit <unit> %d",
			hs_runtime_get_executing_thread_name(),
			command_list_index);
	}
#endif

	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_try_and_get(unit_index);

		if (unit)
		{
			if (unit->unit.actor_index != NONE)
			{
				struct action_state_data new_state_data;
				struct actor_datum *actor = actor_get(unit->unit.actor_index);

				if (action_obey_command_list_setup(unit->unit.actor_index, command_list_index, &new_state_data.obey))
				{
					actor_action_change(unit->unit.actor_index, _actor_action_obey, &new_state_data);
				}
			}
		}
	}

	return;
}

void ai_scripting_command_list_advance(
	long ai_index)
{
	struct ai_index_actor_iterator iterator;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_command_list_advance %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	ai_index_actor_iterator_new(ai_index, &iterator);

	while (ai_index_actor_iterator_next(&iterator))
	{
		action_obey_advance_command_list(iterator.iterator.index);
	}

	return;
}

void ai_scripting_command_list_advance_by_unit(
	long unit_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_command_list_advance_by_unit <some unit>",
			hs_runtime_get_executing_thread_name());
	}
#endif

	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_try_and_get(unit_index);

		if (unit)
		{
			if (unit->unit.actor_index != NONE)
			{
				action_obey_advance_command_list(unit->unit.actor_index);
			}
			else if (unit->unit.swarm_actor_index != NONE)
			{
				action_obey_advance_command_list(unit->unit.swarm_actor_index);
			}
		}
	}

	return;
}

static short ai_scripting_command_list_status_internal(
	long actor_index,
	long unit_index,
	short command_list_index,
	struct obey_individual_simple_control *simple_control,
	struct obey_individual_complex_control *complex_control)
{
	struct ai_command_list_definition *command_list = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_command_lists,
		command_list_index,
		struct ai_command_list_definition);
	struct ai_command_definition *command = NULL;
	short status = _ai_command_list_status_none;

	if (simple_control->current_command_index >= 0 && simple_control->current_command_index < command_list->commands.count)
	{
		command = TAG_BLOCK_GET_ELEMENT(
			&command_list->commands,
			simple_control->current_command_index,
			struct ai_command_definition);
	}

	if (!command)
	{
		status = _ai_command_list_status_finished;
	}
	else if (TEST_FLAG(simple_control->metadata_flags, _obey_metadata_waiting_for_advance_notification_bit))
	{
		status = _ai_command_list_status_waiting;
	}
	else
	{
		status = _ai_command_list_status_running;
	}

	return status;
}

short ai_scripting_command_list_status(
	long object_list_index)
{
	long object_list_reference_index;
	long object_index;
	short status = _ai_command_list_status_none;
	long current_time = game_time_get();

	object_index = object_list_get_first(object_list_index, &object_list_reference_index);

	while (object_index != NONE)
	{
		struct unit_datum *unit = unit_try_and_get(object_index);

		if (unit)
		{
			struct actor_datum *actor = NULL;
			short actor_status = _ai_command_list_status_none;

			if (unit->unit.actor_index != NONE)
			{
				actor = actor_get(unit->unit.actor_index);
				match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 2688, !actor->meta.swarm);

				if (actor->state.action == _actor_action_obey)
				{
					struct obey_state_data *obey = &actor->state.action_data.obey;

					actor_status = ai_scripting_command_list_status_internal(
						unit->unit.actor_index,
						actor->meta.unit_index,
						obey->command_list_index,
						&obey->simple_control,
						&obey->complex_control);
				}
			}
			else if (unit->unit.swarm_actor_index != NONE)
			{
				actor = actor_get(unit->unit.swarm_actor_index);
				match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 2701, actor->meta.swarm);

				if (actor->state.action == _actor_action_obey && actor->meta.swarm_cache_index != NONE)
				{
					short swarm_unit_index;
					struct obey_state_data *obey = &actor->state.action_data.obey;
					struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

					for (swarm_unit_index = 0; swarm_unit_index < swarm->unit_count; swarm_unit_index++)
					{
						if (swarm->unit_indices[swarm_unit_index] == object_index)
						{
							break;
						}
					}

					if (swarm_unit_index < swarm->unit_count)
					{
						struct swarm_component_datum *component = swarm_component_get(swarm->component_indices[swarm_unit_index]);

						if (TEST_FLAG(component->flags, _swarm_component_obey_bit))
						{
							actor_status = ai_scripting_command_list_status_internal(
								unit->unit.swarm_actor_index,
								object_index,
								obey->command_list_index,
								&component->obey,
								NULL);
						}
					}
				}
			}

			if (!actor_status && actor && actor->state.last_command_list_time != NONE &&
				actor->state.last_command_list_time + 5 * TICKS_PER_SECOND >= current_time)
			{
				actor_status = _ai_command_list_status_finished;
			}

			status = MAX(status, actor_status);
		}

		object_index = object_list_get_next(object_list_index, &object_list_reference_index);
	}

	return status;
}

void ai_scripting_free(
	long ai_index)
{
	struct ai_index_actor_iterator iterator;
	struct actor_datum *actor;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_free %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		ai_index_actor_iterator_new(ai_index, &iterator);

		while (actor = ai_index_actor_iterator_next(&iterator))
		{
			match_assert("c:\\halo\\SOURCE\\ai\\ai_script.c", 2769, actor->meta.encounter_index != NONE);

			actor_flush_position_indices(iterator.iterator.index);
			encounter_detach_actor(iterator.iterator.index, FALSE);
			encounterless_attach_actor(iterator.iterator.index);
		}

		encounters_update_dirty_status();
	}

	return;
}

void ai_scripting_free_units(
	long object_list_index)
{
	long object_list_reference_index;
	long object_index = object_list_get_first(object_list_index, &object_list_reference_index);
	short freed_count = 0;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_free_units <some units>",
			hs_runtime_get_executing_thread_name());
	}
#endif

	while (object_index != NONE)
	{
		struct unit_datum *unit = unit_try_and_get(object_index);

		if (unit && unit->unit.actor_index != NONE)
		{
			struct actor_datum *actor = actor_get(unit->unit.actor_index);

			if (actor->meta.encounter_index != NONE)
			{
				actor_flush_position_indices(unit->unit.actor_index);
				encounter_detach_actor(unit->unit.actor_index, FALSE);
				encounterless_attach_actor(unit->unit.actor_index);
				freed_count++;
			}
		}

		object_index = object_list_get_next(object_list_index, &object_list_reference_index);
	}

	if (freed_count > 0)
	{
		encounters_update_dirty_status();
	}

	return;
}

void ai_scripting_attach_free(
	long unit_index,
	long actor_variant_tag_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_attach_free 0x%04X %s",
			hs_runtime_get_executing_thread_name(),
			DATUM_INDEX_TO_ABSOLUTE_INDEX(unit_index),
			actor_variant_tag_index == NONE ? "<error>" : tag_name_strip_path(tag_get_name(actor_variant_tag_index)));
	}
#endif

	if (ai_globals->ai_initialized_for_map && unit_index != NONE && actor_variant_tag_index != NONE)
	{
		struct actor_variant_definition *variant_definition = actor_variant_definition_get(actor_variant_tag_index);

		if (variant_definition->actor_reference.index != NONE)
		{
			struct actor_definition *actor_definition = actor_definition_get(variant_definition->actor_reference.index);

			if (TEST_FLAG(actor_definition->flags, _actor_definition_swarm_actor_bit))
			{
				error(
					_error_silent,
					"%s: ai_attach_free %s cannot be used for swarm actors",
					hs_runtime_get_executing_thread_name(),
					actor_variant_tag_index == NONE ? "<error>" : tag_name_strip_path(tag_get_name(actor_variant_tag_index)));
			}
			else
			{
				actor_create_for_unit(
					FALSE,
					unit_index,
					actor_variant_tag_index,
					NONE,
					NONE,
					FALSE,
					NONE,
					FALSE,
					_actor_default_state_alert,
					_actor_default_state_none,
					NONE,
					0);
			}
		}
	}

	return;
}

void ai_scripting_force_active(
	long ai_index,
	boolean active)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_force_active %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			active ? "true" : "false");
	}
#endif

	if (ai_globals->ai_initialized_for_map && ai_index != NONE)
	{
		long encounter_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_index);
		struct scenario *scenario = global_scenario_get();

		if (encounter_index >= 0 && encounter_index < scenario->ai_encounters.count)
		{
			struct encounter_datum *encounter = encounter_get(encounter_index);

			encounter->force_active = active;
		}
	}

	return;
}

void ai_scripting_force_active_by_unit(
	long unit_index,
	boolean active)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_force_active_by_unit <some unit> %s",
			hs_runtime_get_executing_thread_name(),
			active ? "true" : "false");
	}
#endif

	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->unit.actor_index != NONE)
		{
			struct actor_datum *actor = actor_get(unit->unit.actor_index);

			if (actor->meta.encounterless)
			{
				actor->meta.force_active = active;
			}
			else
			{
				struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
					&global_scenario_get()->ai_encounters,
					DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
					struct encounter_definition);
				struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter_definition->squads,
					actor->meta.squad_index,
					struct squad_definition);

				error(
					_error_silent,
					"ai_force_active_by_unit: unit is a member of encounter %s/%s, you must use ai_force_active instead",
					encounter_definition->name,
					squad_definition->name);
			}
		}
	}

	return;
}

void ai_scripting_set_return_state(
	long ai_index,
	short default_state)
{
	struct ai_index_actor_iterator iterator;
	struct actor_datum *actor;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_set_return_state %s %d",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			default_state);
	}
#endif

	if (default_state >= 0 && default_state < NUMBER_OF_ACTOR_DEFAULT_STATES)
	{
		ai_index_actor_iterator_new(ai_index, &iterator);

		while (actor = ai_index_actor_iterator_next(&iterator))
		{
			short action_class = actor_action_class(iterator.iterator.index);

			actor->state.default_state = default_state;

			if (actor->state.combat_status == _actor_combat_status_none &&
				(action_class == _action_class_noncombat || action_class == _action_class_passive || action_class == _action_class_transitory))
			{
				actor_action_set_default_state(iterator.iterator.index, NONE);
			}
		}
	}

	return;
}

void ai_scripting_set_current_state(
	long ai_index,
	short current_state)
{
	struct ai_index_actor_iterator iterator;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_set_current_state %s %d",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			current_state);
	}
#endif

	if (current_state >= 0 && current_state < NUMBER_OF_ACTOR_DEFAULT_STATES)
	{
		ai_index_actor_iterator_new(ai_index, &iterator);

		while (ai_index_actor_iterator_next(&iterator))
		{
			actor_action_set_default_state(iterator.iterator.index, current_state);
		}
	}

	return;
}

static short ai_scripting_assess_status(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	short status = _ai_scripting_status_inactive;

	if (!actor->meta.active)
	{
		status = _ai_scripting_status_inactive;
	}
	else if (actor->state.mode < _actor_mode_combat)
	{
		status = _ai_scripting_status_noncombat;
	}
	else if (actor->state.combat_status == _actor_combat_status_none)
	{
		status = _ai_scripting_status_guarding;
	}
	else if (actor->target.target_type < _actor_target_definite_orphan)
	{
		status = _ai_scripting_status_searching;
	}
	else if (actor->target.target_type < _actor_target_visible_enemy)
	{
		status = _ai_scripting_status_definite_target;
	}
	else if (actor->orders.combat.shoot_at_target || actor->orders.combat.throw_grenade)
	{
		status = _ai_scripting_status_engaging;
	}
	else
	{
		status = _ai_scripting_status_visible_target;
	}

	return status;
}

short ai_scripting_status(
	long ai_index)
{
	struct ai_index_actor_iterator iterator;
	struct actor_datum *actor;
	short status = _ai_scripting_status_inactive;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_status %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	ai_index_actor_iterator_new(ai_index, &iterator);

	while (actor = ai_index_actor_iterator_next(&iterator))
	{
		short actor_status = ai_scripting_assess_status(iterator.iterator.index);

		status = MAX(status, actor_status);
	}

	return status;
}

void ai_scripting_reconnect(
	void)
{
	return;
}

void ai_scripting_playfight(
	long ai_index,
	boolean playfight)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_playfight %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			playfight ? "on" : "off");
	}
#endif

	if (ai_index != NONE)
	{
		struct encounter_datum *encounter = encounter_get(DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_index));

		encounter->playfighting = playfight;
	}

	return;
}

void ai_scripting_vehicle_encounter(
	long unit_index,
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_vehicle_encounter <some unit> %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		short encounter_index = NONE;
		short squad_index = NONE;

		if (ai_index != NONE)
		{
			struct scenario *scenario = global_scenario_get();
			short ai_encounter_index = (short)ai_index;

			if (ai_encounter_index >= 0 && ai_encounter_index < scenario->ai_encounters.count)
			{
				struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
					&global_scenario_get()->ai_encounters,
					DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_encounter_index),
					struct encounter_definition);
				short ai_squad_index = 0;

				switch (AI_INDEX_GET_TYPE(ai_index))
				{
				case _ai_index_encounter:
					break;
				case _ai_index_platoon:
				{
					short platoon_index = (short)AI_INDEX_GET_SUBINDEX(ai_index);

					for (ai_squad_index = 0; ai_squad_index < encounter_definition->squads.count; ai_squad_index++)
					{
						struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
							&encounter_definition->squads,
							ai_squad_index,
							struct squad_definition);

						if (squad_definition->platoon_index == platoon_index)
						{
							break;
						}
					}

					if (ai_squad_index >= encounter_definition->squads.count)
					{
						ai_squad_index = 0;
					}
					break;
				}
				case _ai_index_squad:
					ai_squad_index = (short)AI_INDEX_GET_SUBINDEX(ai_index);
					break;
				default:
					match_unreachable("c:\\halo\\SOURCE\\ai\\ai_script.c", 3154);
					break;
				}

				if (ai_squad_index >= 0 && ai_squad_index < encounter_definition->squads.count)
				{
					encounter_index = ai_encounter_index;
					squad_index = ai_squad_index;
				}
			}
		}

		if (encounter_index != NONE && squad_index != NONE && unit->unit.fake_encounter_index != NONE)
		{
			struct encounter_actor_iterator iterator;
			struct actor_datum *actor;

			encounter_actor_iterator_new(&iterator, unit->unit.fake_encounter_index);

			while (actor = encounter_actor_iterator_next(&iterator))
			{
				if (actor->input.vehicle_index == unit_index)
				{
					actor_change_encounter(iterator.index, encounter_index, squad_index);
				}
			}
		}

		unit->unit.fake_encounter_index = encounter_index;
		unit->unit.fake_squad_index = squad_index;
	}

	return;
}

struct ai_vehicle_enterable *ai_scripting_find_vehicle_enterable(
	long vehicle_index)
{
	struct ai_vehicle_enterable *enterable = NULL;

	if (vehicle_index != NONE)
	{
		short enterable_index;

		for (enterable_index = 0; enterable_index < ai_globals->enterable_vehicle_count; enterable_index++)
		{
			if (ai_globals->enterable_vehicles[enterable_index].vehicle_index == vehicle_index)
			{
				break;
			}
		}

		if (enterable_index >= MAXIMUM_ENTERABLE_VEHICLES)
		{
			error(
				_error_silent,
				"ai_vehicle_enterable: too many enterable vehicles (max is %d)",
				MAXIMUM_ENTERABLE_VEHICLES);
		}
		else
		{
			enterable = &ai_globals->enterable_vehicles[enterable_index];

			if (enterable_index >= ai_globals->enterable_vehicle_count)
			{
				memset(enterable, 0, sizeof(*enterable));
				enterable->vehicle_index = vehicle_index;
				enterable->radius = 8.f;
				ai_globals->enterable_vehicle_count++;
			}
		}
	}

	return enterable;
}

void ai_scripting_vehicle_enterable_distance(
	long unit_index,
	real distance)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_vehicle_enterable_distance <some vehicle>",
			hs_runtime_get_executing_thread_name());
	}
#endif

	if (unit_index != NONE)
	{
		struct ai_vehicle_enterable *enterable = ai_scripting_find_vehicle_enterable(unit_index);

		if (enterable)
		{
			enterable->radius = distance;
		}
	}

	return;
}

void ai_scripting_vehicle_enterable_team(
	long unit_index,
	short team_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_vehicle_enterable_team <some vehicle> %d",
			hs_runtime_get_executing_thread_name(),
			team_index);
	}
#endif

	if (unit_index != NONE)
	{
		struct ai_vehicle_enterable *enterable = ai_scripting_find_vehicle_enterable(unit_index);

		if (enterable)
		{
			SET_FLAG(enterable->team_bitmask, team_index, TRUE);
		}
	}

	return;
}

void ai_scripting_vehicle_enterable_actor_type(
	long unit_index,
	short actor_type)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_vehicle_enterable_actor_type <some vehicle> %d",
			hs_runtime_get_executing_thread_name(),
			actor_type);
	}
#endif

	if (unit_index != NONE)
	{
		struct ai_vehicle_enterable *enterable = ai_scripting_find_vehicle_enterable(unit_index);

		if (enterable)
		{
			SET_FLAG(enterable->actor_type_bitmask, actor_type, TRUE);
		}
	}

	return;
}

void ai_scripting_vehicle_enterable_actors(
	long unit_index,
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_vehicle_enterable_actors <some vehicle> %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (unit_index != NONE && ai_index != NONE)
	{
		struct ai_vehicle_enterable *enterable = ai_scripting_find_vehicle_enterable(unit_index);

		if (enterable)
		{
			if (enterable->ai_indices_count < (long)NUMBEROF(enterable->ai_indices))
			{
				enterable->ai_indices[enterable->ai_indices_count++] = ai_index;
			}
			else
			{
				error(
					_error_silent,
					"ai_vehicle_enterable_actors: too many groups of actors (max is %d)",
					NUMBEROF(enterable->ai_indices));
			}
		}
	}

	return;
}

void ai_scripting_vehicle_enterable_disable(
	long unit_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_vehicle_enterable_disable <some vehicle>",
			hs_runtime_get_executing_thread_name());
	}
#endif

	if (unit_index != NONE)
	{
		short enterable_index;

		for (enterable_index = 0; enterable_index < ai_globals->enterable_vehicle_count; enterable_index++)
		{
			if (ai_globals->enterable_vehicles[enterable_index].vehicle_index == unit_index)
			{
				ai_globals->enterable_vehicle_count--;

				if (enterable_index < ai_globals->enterable_vehicle_count)
				{
					ai_globals->enterable_vehicles[enterable_index] = ai_globals->enterable_vehicles[ai_globals->enterable_vehicle_count];
				}

				break;
			}
		}
	}

	return;
}

void ai_scripting_look_at_object(
	long unit_index,
	long object_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_look_at_object <some unit> <some object>",
			hs_runtime_get_executing_thread_name());
	}
#endif

	if (unit_index != NONE && object_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->unit.actor_index != NONE)
		{
			struct direction_specification direction;

			direction.type = _direction_specification_object;
			direction.object_index = object_index;
			actor_look_secondary(
				unit->unit.actor_index,
				_secondary_look_scripted,
				_secondary_look_priority_default,
				&direction);
		}
	}

	return;
}

void ai_scripting_stop_looking(
	long unit_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		error(
			_error_silent,
			"%s: ai_stop_looking <some unit>",
			hs_runtime_get_executing_thread_name());
	}
#endif

	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->unit.actor_index != NONE)
		{
			actor_look_secondary_stop(unit->unit.actor_index);
		}
	}

	return;
}

void ai_scripting_automatic_migration_target(
	long ai_index,
	boolean enable)
{
	struct ai_index_squad_iterator iterator;
	struct squad_datum *squad;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_automatic_migration_target %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			enable ? "on" : "off");
	}
#endif

	if (ai_index != NONE)
	{
		ai_index_squad_iterator_new(ai_index, &iterator);

		while (squad = ai_index_squad_iterator_next(&iterator))
		{
			squad->automatic_migration_target = enable;
		}
	}

	return;
}

void ai_scripting_follow_target_disable(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_follow_target_disable %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		struct encounter_datum *encounter = encounter_get(DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_index));

		encounter->follow_target_type = _follow_target_none;
	}

	return;
}

void ai_scripting_follow_target_players(
	long ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_follow_target_players %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		struct encounter_datum *encounter = encounter_get(DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_index));

		encounter->follow_target_type = _follow_target_players;
	}

	return;
}

void ai_scripting_follow_target_unit(
	long ai_index,
	long unit_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_follow_target_unit %s <some unit>",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		struct encounter_datum *encounter = encounter_get(DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_index));

		if (unit_index == NONE)
		{
			encounter->follow_target_type = _follow_target_none;
		}
		else
		{
			encounter->follow_target_type = _follow_target_unit;
			encounter->follow_target_unit_index = unit_index;
		}
	}

	return;
}

void ai_scripting_follow_target_ai(
	long ai_index,
	long target_ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];
		char aibuf2[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		ai_index_to_string(ai_index, global_scenario_get(), aibuf2, sizeof(aibuf2));
		error(
			_error_silent,
			"%s: ai_follow_target_ai %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			aibuf2);
	}
#endif

	if (ai_index != NONE)
	{
		struct encounter_datum *encounter = encounter_get(DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_index));

		if (target_ai_index == NONE)
		{
			encounter->follow_target_type = _follow_target_none;
		}
		else
		{
			encounter->follow_target_type = _follow_target_ai;
			encounter->follow_target_ai_index = target_ai_index;
		}
	}

	return;
}

void ai_scripting_follow_distance(
	long ai_index,
	real follow_target_distance)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_follow_distance %s %.1f",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			follow_target_distance);
	}
#endif

	if (ai_index != NONE)
	{
		struct encounter_datum *encounter = encounter_get(DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_index));

		encounter->follow_target_distance = follow_target_distance;
	}

	return;
}

boolean ai_scripting_conversation(
	short conversation_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		struct scenario *scenario = global_scenario_get();
		char const *conversation_name = "<error>";

		if (conversation_index >= 0 && conversation_index < scenario->ai_conversations.count)
		{
			struct ai_conversation *conversation = TAG_BLOCK_GET_ELEMENT(
				&scenario->ai_conversations,
				conversation_index,
				struct ai_conversation);

			conversation_name = conversation->name;
		}

		error(
			_error_silent,
			"%s: ai_conversation %s",
			hs_runtime_get_executing_thread_name(),
			conversation_name);
	}
#endif

	return ai_conversation(conversation_index, TRUE);
}

void ai_scripting_conversation_stop(
	short conversation_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		struct scenario *scenario = global_scenario_get();
		char const *conversation_name = "<error>";

		if (conversation_index >= 0 && conversation_index < scenario->ai_conversations.count)
		{
			struct ai_conversation *conversation = TAG_BLOCK_GET_ELEMENT(
				&scenario->ai_conversations,
				conversation_index,
				struct ai_conversation);

			conversation_name = conversation->name;
		}

		error(
			_error_silent,
			"%s: ai_conversation_stop %s",
			hs_runtime_get_executing_thread_name(),
			conversation_name);
	}
#endif

	ai_conversation_stop(conversation_index);

	return;
}

void ai_scripting_conversation_advance(
	short conversation_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		struct scenario *scenario = global_scenario_get();
		char const *conversation_name = "<error>";

		if (conversation_index >= 0 && conversation_index < scenario->ai_conversations.count)
		{
			struct ai_conversation *conversation = TAG_BLOCK_GET_ELEMENT(
				&scenario->ai_conversations,
				conversation_index,
				struct ai_conversation);

			conversation_name = conversation->name;
		}

		error(
			_error_silent,
			"%s: ai_conversation_advance %s",
			hs_runtime_get_executing_thread_name(),
			conversation_name);
	}
#endif

	ai_conversation_advance(conversation_index);

	return;
}

short ai_scripting_conversation_line(
	short conversation_index)
{
	return ai_conversation_line(conversation_index);
}

short ai_scripting_conversation_status(
	short conversation_index)
{
	return ai_conversation_status(conversation_index);
}

void ai_scripting_link_activation(
	long target_ai_index,
	long link_ai_index)
{
#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[512];
		char aibuf2[512];

		ai_index_to_string(target_ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		ai_index_to_string(link_ai_index, global_scenario_get(), aibuf2, sizeof(aibuf2));
		error(
			_error_silent,
			"%s: ai_link_activation %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			aibuf2);
	}
#endif

	if (target_ai_index != NONE && link_ai_index != NONE)
	{
		if (!encounter_link_activation(DATUM_INDEX_TO_ABSOLUTE_INDEX(target_ai_index), link_ai_index))
		{
			error(
				_error_silent,
				"ai_link_activation: cannot link to another encounter, MAXIMUM_ACTIVATION_LINK_INDICES_PER_ENCOUNTER is %d",
				MAXIMUM_ACTIVATION_LINK_INDICES_PER_ENCOUNTER);
		}
	}

	return;
}

void ai_scripting_berserk(
	long ai_index,
	boolean berserk)
{
	struct ai_index_actor_iterator iterator;
	struct actor_datum *actor;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_berserk %s",
			hs_runtime_get_executing_thread_name(),
			aibuf);
	}
#endif

	if (ai_index != NONE)
	{
		ai_index_actor_iterator_new(ai_index, &iterator);

		while (actor = ai_index_actor_iterator_next(&iterator))
		{
			actor_berserk(iterator.iterator.index, berserk);
		}
	}

	return;
}

void ai_scripting_set_team(
	long ai_index,
	short team_index)
{
	struct encounter_actor_iterator iterator;
	struct actor_datum *actor;
	long encounter_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_index);
	struct encounter_datum *encounter = encounter_get(encounter_index);

	encounter->team_index = team_index;
	encounter_actor_iterator_new(&iterator, encounter_index);

	while (actor = encounter_actor_iterator_next(&iterator))
	{
		actor_set_team(iterator.index, team_index);
	}

	ai_update_team_status();

	return;
}

void ai_scripting_allow_dormant(
	long ai_index,
	boolean allow_dormant)
{
	struct ai_index_squad_iterator iterator;
	struct squad_datum *squad;

#ifdef DEBUG
	if (ai_debug.print_scripting)
	{
		char aibuf[256];

		ai_index_to_string(ai_index, global_scenario_get(), aibuf, sizeof(aibuf));
		error(
			_error_silent,
			"%s: ai_allow_dormant %s %s",
			hs_runtime_get_executing_thread_name(),
			aibuf,
			allow_dormant ? "true" : "false");
	}
#endif

	ai_index_squad_iterator_new(ai_index, &iterator);

	while (squad = ai_index_squad_iterator_next(&iterator))
	{
		squad->disable_dormant = !allow_dormant;
	}

	return;
}

/* ---------- private code */
