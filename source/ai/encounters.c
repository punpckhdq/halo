/*
ENCOUNTERS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "encounters.h"
#include "actors.h"
#include "ai_debug.h"
#include "props.h"
#include "actor_types.h"
#include "ai_profile.h"
#include "ai_script.h"
#include "ai_globals.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "console.h"
#include "game_state.h"
#include "editor_stubs.h"

/* ---------- constants */

#define MAXIMUM_ENCOUNTERS_PER_MAP 128 /* fake name? */
#define MAXIMUM_EXAMINED_PURSUIT_POSITIONS_PER_MAP 256
#define NUMBER_OF_ACTOR_INDICES_PER_EXAMINED_PURSUIT_POSITION 6
#define MAXIMUM_STARTING_LOCATIONS_PER_SQUAD 64 /* fake name? */

#define ENCOUNTER_UPDATE_TICKS 15 /* fake name */
#define ENCOUNTER_ACTIVATION_TEST_TICKS TICKS_PER_SECOND /* fake name */
#define ENCOUNTER_REMAIN_ACTIVE_TICKS (5*TICKS_PER_SECOND) /* fake name */
#define ENCOUNTERLESS_REMAIN_ACTIVE_TICKS (3*TICKS_PER_SECOND) /* fake name */
#define ENCOUNTER_POST_COMBAT_DELAY_TICKS (4*TICKS_PER_SECOND) /* fake name */
#define ENCOUNTER_TIMER_FOREVER 999 /* fake name */
#define NUMBER_OF_FIRING_POSITION_GROUP_LETTERS 26 /* fake name */

enum
{
	_actor_definition_flags2_avoid_all_enemy_attack_vectors_bit = 0,
	_actor_definition_flags2_must_stand_to_fire_bit,
	_actor_definition_flags2_must_stop_to_fire_bit,
	_actor_definition_flags2_disallow_vehicle_combat_bit,
	_actor_definition_flags2_pathfinding_ignores_danger_bit,
	_actor_definition_flags2_panic_in_groups_bit,
	_actor_definition_flags2_no_corpse_shooting_bit,
	NUMBER_OF_ACTOR_DEFINITION_FLAGS2
};

enum
{
	_firing_point_evaluation_mode_fight = 0,
	_firing_point_evaluation_mode_panic,
	_firing_point_evaluation_mode_cover,
	_firing_point_evaluation_mode_uncover,
	_firing_point_evaluation_mode_guard,
	_firing_point_evaluation_mode_pursue,
	_firing_point_evaluation_mode_avoid,
	NUMBER_OF_FIRING_POINT_EVALUATION_MODES
};

enum
{
	_leader_normal = 0,
	_leader_none,
	_leader_random,
	_leader_sergeant_johnson,
	_leader_sergeant_lehto,
	NUMBER_OF_UNIQUE_LEADER_TYPES
};

enum
{
	_encounter_search_normal = 0,
	_encounter_search_never,
	_encounter_search_tenacious,
	NUMBER_OF_ENCOUNTER_SEARCHING_BEHAVIORS
};

enum
{
	_platoon_flee_upon_maneuver_bit = 0,
	_platoon_advancing_maneuver_bit,
	_platoon_initially_defending_bit,
	NUMBER_OF_PLATOON_FLAGS
};

enum
{
	_platoon_rule_never = 0,
	_platoon_rule_75_strength,
	_platoon_rule_50_strength,
	_platoon_rule_25_strength,
	_platoon_rule_anybody_dead,
	_platoon_rule_25_dead,
	_platoon_rule_50_dead,
	_platoon_rule_75_dead,
	_platoon_rule_all_but_one_dead,
	_platoon_rule_all_dead,
	NUMBER_OF_PLATOON_RULE_TYPES
};

enum
{
	_actor_starting_location_required_bit = 0,
	NUMBER_OF_ACTOR_STARTING_LOCATION_FLAGS
};

enum
{
	_post_combat_behavior_shoot_target = 0,
	_post_combat_behavior_check_enemy,
	_post_combat_behavior_check_friend,
	_post_combat_behavior_celebrate,
	NUMBER_OF_POST_COMBAT_BEHAVIOR_TYPES
};

enum
{
	NUMBER_OF_POST_COMBAT_POSSIBILITIES = 2,		/* fake name */
	NUMBER_OF_PRIMARY_POST_COMBAT_BEHAVIORS = 2		/* fake name */
};

enum
{
	_group_pursuit_restriction_none = 0,		/* fake name */
	_group_pursuit_restriction_never,			/* fake name */
	_group_pursuit_restriction_unlimited		/* fake name */
};

/* ---------- macros */

#define pursuit_get(index) ((struct pursuit_datum *)datum_get(pursuit_data, (index))) /* fake name */

/* ---------- structures */

struct pursuit_datum
{
	short identifier;
	short firing_position_index;
	long last_examined_time;
	short actor_count;
	short next_actor_index_index;
	long actor_indices[NUMBER_OF_ACTOR_INDICES_PER_EXAMINED_PURSUIT_POSITION];
	long next_pursuit_index;
};

struct post_combat_possibility
{
	long actor_index;
	real weight;
	long prop_index;
	long unit_index;
};

/* ---------- prototypes */

static void encounter_clear_pursuit(long encounter_index);
static long encounter_find_pursuit(long encounter_index, short firing_position_index, long history_start_time, boolean force_create);
static void squad_reset_starting_locations(long encounter_index, short squad_index);
static void encounter_new(struct encounter_definition *encounter_definition, short *squad_base, short *platoon_base);
static short squad_get_actor_type(struct squad_definition *squad_definition);
static void encounterless_deactivate(long actor_index);
static boolean encounter_activate(long encounter_index);
static void encounter_deactivate(long encounter_index);
static void encounters_test_activation(void);
static boolean encounter_post_combat_add_possibility(struct post_combat_possibility *possibility_array, long actor_index, real weight, long prop_index, long unit_index);
static short encounter_post_combat_select_random_behavior(struct post_combat_possibility (*possibilities)[NUMBER_OF_POST_COMBAT_POSSIBILITIES], struct post_combat_possibility *selected_possibility);
static void encounter_post_combat(long encounter_index);
static void encounter_update_timers(long encounter_index);
static boolean encounter_place_actor(long encounter_index, short squad_index, short variant_number, boolean spawn);
static void encounter_update_respawn(long encounter_index);
static void encounter_update_squads(long encounter_index);
static void encounter_update_platoons(long encounter_index);
static void encounter_update_follow(long encounter_index);
static void encounter_control_actors(long encounter_index);
static boolean encounter_test_rule(long encounter_index, struct platoon_rule *rule);

/* ---------- globals */

struct data_array *encounter_data;
struct platoon_datum *platoon_array;
struct squad_datum *squad_array;
struct data_array *pursuit_data;

struct post_combat_possibility const global_empty_possibility = {NONE, 0.f, NONE, NONE};

short const global_post_combat_translation_table[NUMBER_OF_POST_COMBAT_BEHAVIOR_TYPES] =
{
	_actor_postcombat_shoot_corpse,
	_actor_postcombat_check_enemy,
	_actor_postcombat_check_friend,
	_actor_postcombat_celebrate
};

/* ---------- public code */

void encounters_initialize(
	void)
{
	encounter_data = game_state_data_new("encounter", MAXIMUM_ENCOUNTERS_PER_MAP, sizeof(struct encounter_datum));
	match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 110, encounter_data);

	squad_array = game_state_malloc("squad", "squad", MAXIMUM_SQUADS_PER_MAP * sizeof(struct squad_datum));
	match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 113, squad_array);

	platoon_array = game_state_malloc("platoon", "platoon", MAXIMUM_PLATOONS_PER_MAP * sizeof(struct platoon_datum));
	match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 116, platoon_array);

	pursuit_data = game_state_data_new("ai pursuit", MAXIMUM_EXAMINED_PURSUIT_POSITIONS_PER_MAP, sizeof(struct pursuit_datum));
	match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 119, pursuit_data);

	return;
}

void encounters_dispose(
	void)
{
	return;
}

void encounters_initialize_for_new_map(
	void)
{
	short encounter_index;
	struct scenario *scenario = global_scenario_get();
	short squad_base = 0;
	short platoon_base = 0;

	data_make_valid(encounter_data);
	data_make_valid(pursuit_data);

	memset(squad_array, 0, MAXIMUM_SQUADS_PER_MAP * sizeof(struct squad_datum));
	memset(platoon_array, 0, MAXIMUM_PLATOONS_PER_MAP * sizeof(struct platoon_datum));

	for (encounter_index = 0; encounter_index < scenario->ai_encounters.count; encounter_index++)
	{
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&scenario->ai_encounters,
			encounter_index,
			struct encounter_definition);

		encounter_new(encounter_definition, &squad_base, &platoon_base);
	}

	return;
}

void encounters_dispose_from_old_map(
	void)
{
	data_make_invalid(encounter_data);
	data_make_invalid(pursuit_data);

	return;
}

void encounters_create_for_new_map(
	void)
{
	struct encounter_iterator iterator;
	struct encounter_datum *encounter;
	struct scenario *scenario = global_scenario_get();

	encounter_iterator_new(&iterator, FALSE);

	while (encounter = encounter_iterator_next(&iterator))
	{
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&scenario->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index),
			struct encounter_definition);
		boolean create = !TEST_FLAG(encounter_definition->flags, _encounter_not_initially_placed_bit);

		if (DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index) == DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_debug.selected_encounter_index))
		{
			create = TRUE;
		}

		if (create)
		{
			encounter_create(iterator.index, NONE, NONE);
		}
	}

	return;
}

void encounters_update(
	void)
{
	short update_index;
	struct encounter_iterator iterator;
	struct encounter_datum *encounter;
	long game_time = game_time_get();

	if (!(game_time % ENCOUNTER_ACTIVATION_TEST_TICKS))
	{
		encounters_update_dirty_status();
		encounters_test_activation();
	}

	update_index = (short)(game_time % ENCOUNTER_UPDATE_TICKS);
	encounter_iterator_new(&iterator, TRUE);

	while (encounter = encounter_iterator_next(&iterator))
	{
		ai_profile.meters[_ai_meter_encounter_active].current_count++;

		if ((short)(DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index) % ENCOUNTER_UPDATE_TICKS) == update_index)
		{
			encounter_update_status(iterator.index);
			encounter_update_timers(iterator.index);
			encounter_update_respawn(iterator.index);
			encounter_update_squads(iterator.index);
			encounter_update_platoons(iterator.index);
			encounter_update_follow(iterator.index);
			encounter_control_actors(iterator.index);
		}
	}

	return;
}

void encounters_update_dirty_status(
	void)
{
	struct encounter_iterator iterator;
	struct encounter_datum *encounter;

	encounter_iterator_new(&iterator, FALSE);

	while (encounter = encounter_iterator_next(&iterator))
	{
		if (encounter->status_dirty)
		{
			encounter_update_status(iterator.index);
		}
	}

	return;
}

void encounters_unit_died(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->object.owner_team_index != NONE)
	{
		struct encounter_iterator iterator;
		struct encounter_datum *encounter;

		encounter_iterator_new(&iterator, TRUE);

		while (encounter = encounter_iterator_next(&iterator))
		{
			if (game_team_is_enemy(encounter->team_index, unit->object.owner_team_index) &&
				encounter->enemy_target &&
				!encounter->stand_down &&
				!encounter->post_combat)
			{
				encounter->enemies_defeated++;
			}
		}
	}

	return;
}

void encounter_verify_firing_position_owner_actor_indices(
	long encounter_index)
{
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	long owner_actor_indices[MAXIMUM_FIRING_POSITIONS_PER_ENCOUNTER];
	struct encounter_actor_iterator iterator;
	struct actor_datum *actor;

	memset(owner_actor_indices, NONE, encounter_definition->firing_positions.count * sizeof(long));
	encounter_actor_iterator_new(&iterator, encounter_index);

	while (actor = encounter_actor_iterator_next(&iterator))
	{
		if (actor->firing_positions.current_position_index != NONE)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 302, actor->firing_positions.current_position_index>=0 && actor->firing_positions.current_position_index < encounter_definition->firing_positions.count);
			match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 303, owner_actor_indices[actor->firing_positions.current_position_index]==NONE);

			owner_actor_indices[actor->firing_positions.current_position_index] = iterator.index;
		}
	}

	return;
}

void encounter_build_firing_position_owner_actor_indices(
	long encounter_index,
	long *firing_position_owner_actor_indices)
{
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	struct encounter_actor_iterator iterator;
	struct actor_datum *actor;

	memset(firing_position_owner_actor_indices, NONE, encounter_definition->firing_positions.count * sizeof(long));
	encounter_actor_iterator_new(&iterator, encounter_index);

	while (actor = encounter_actor_iterator_next(&iterator))
	{
		if (actor->firing_positions.current_position_index != NONE)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 332, actor->firing_positions.current_position_index>=0 && actor->firing_positions.current_position_index < encounter_definition->firing_positions.count);
			match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 333, firing_position_owner_actor_indices[actor->firing_positions.current_position_index]==NONE);

			firing_position_owner_actor_indices[actor->firing_positions.current_position_index] = iterator.index;
		}
	}

	return;
}

void encounter_compute_activation_cluster_bit_vector(
	long encounter_index,
	boolean update_actor_dormancy,
	long bit_vector_size,
	unsigned long const *active_area,
	unsigned long *bit_vector)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	struct encounter_datum *encounter = encounter_get(encounter_index);
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	struct actor_datum *actor;
	long actor_index;
	unsigned long squad_mask = 0;
	unsigned long firing_position_group_mask = 0;

	match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 355, structure_bsp->clusters.count <= bit_vector_size);

	memset(bit_vector, 0, BIT_VECTOR_SIZE_IN_BYTES(structure_bsp->clusters.count));

	for (actor_index = encounter->first_actor_index; actor_index != NONE; actor_index = actor->meta.next_actor_index)
	{
		boolean outside_active_area;

		actor = actor_get(actor_index);
		outside_active_area = TRUE;

		if (actor->meta.swarm)
		{
			struct unit_datum *unit;
			long unit_index;

			for (unit_index = actor->meta.swarm_unit_index; unit_index != NONE; unit_index = unit->unit.swarm_next_unit_index)
			{
				long ultimate_parent_index;
				struct object_datum *object;
				short cluster_index;

				unit = unit_get(unit_index);
				ultimate_parent_index = object_get_ultimate_parent(unit_index);
				object = object_get(ultimate_parent_index);
				cluster_index = object->object.location.cluster_index;

				if (cluster_index != NONE)
				{
					match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 382, (cluster_index >= 0) && (cluster_index < bit_vector_size));

					BIT_VECTOR_SET_FLAG(bit_vector, cluster_index, TRUE);

					if (active_area && BIT_VECTOR_TEST_FLAG(active_area, cluster_index))
					{
						outside_active_area = FALSE;
					}
				}
			}
		}
		else
		{
			long ultimate_parent_index;
			struct object_datum *object;
			short cluster_index;
			struct unit_datum *unit = unit_get(actor->meta.unit_index);

			ultimate_parent_index = object_get_ultimate_parent(actor->meta.unit_index);
			object = object_get(ultimate_parent_index);
			cluster_index = object->object.location.cluster_index;

			if (cluster_index != NONE)
			{
				match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 408, (cluster_index >= 0) && (cluster_index < bit_vector_size));

				BIT_VECTOR_SET_FLAG(bit_vector, cluster_index, TRUE);

				if (active_area && BIT_VECTOR_TEST_FLAG(active_area, cluster_index))
				{
					outside_active_area = FALSE;
				}
			}

			if (encounter->active)
			{
				if (actor->state.mode == _actor_mode_combat)
				{
					if (actor->state.combat_status >= _actor_combat_status_investigate)
					{
						firing_position_group_mask |= actor_get_firing_position_group(actor_index, _firing_point_evaluation_mode_pursue, 1);
					}

					if (actor->state.action == _actor_action_guard || actor->state.action == _actor_action_flee)
					{
						firing_position_group_mask |= actor_get_firing_position_group(actor_index, _firing_point_evaluation_mode_guard, 0);
					}
					else if (actor->state.action == _actor_action_fight || actor->state.action == _actor_action_uncover)
					{
						firing_position_group_mask |= actor_get_firing_position_group(actor_index, _firing_point_evaluation_mode_fight, 0);
					}
				}
				else if (actor->state.mode == _actor_mode_alert && actor->state.action_data.alert.move_position_order)
				{
					squad_mask |= FLAG(actor->meta.squad_index);
				}

				if (actor->target.target_prop_index != NONE)
				{
					struct prop_datum *prop = prop_get(actor->target.target_prop_index);
					short cluster_index = prop->body_location.cluster_index;

					if (cluster_index != NONE)
					{
						match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 448, (cluster_index >= 0) && (cluster_index < bit_vector_size));

						BIT_VECTOR_SET_FLAG(bit_vector, cluster_index, TRUE);
					}
				}
			}
		}

		if (update_actor_dormancy)
		{
			struct squad_datum *squad = encounter_get_squad(encounter, actor->meta.squad_index);

			if (squad->disable_dormant)
			{
				outside_active_area = FALSE;
			}

			actor->meta.dormant_desire = outside_active_area;
		}
	}

	if (firing_position_group_mask)
	{
		short firing_position_index;

		for (firing_position_index = 0; firing_position_index < encounter_definition->firing_positions.count; firing_position_index++)
		{
			struct firing_position_definition *firing_position = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->firing_positions,
				firing_position_index,
				struct firing_position_definition);

			if (firing_position->cluster_index != NONE && TEST_FLAG(firing_position_group_mask, firing_position->group_index))
			{
				match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 487, (firing_position->cluster_index >= 0) && (firing_position->cluster_index < bit_vector_size));

				BIT_VECTOR_SET_FLAG(bit_vector, firing_position->cluster_index, TRUE);
			}
		}
	}

	if (squad_mask)
	{
		short squad_index;

		for (squad_index = 0; squad_index < encounter_definition->squads.count; squad_index++)
		{
			if (TEST_FLAG(squad_mask, squad_index))
			{
				struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter_definition->squads,
					squad_index,
					struct squad_definition);
				short move_position_index;

				for (move_position_index = 0; move_position_index < squad_definition->move_positions.count; move_position_index++)
				{
					struct move_position_definition *move_position = TAG_BLOCK_GET_ELEMENT(
						&squad_definition->move_positions,
						move_position_index,
						struct move_position_definition);

					if (move_position->cluster_index != NONE)
					{
						match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 511, (move_position->cluster_index >= 0) && (move_position->cluster_index < bit_vector_size));

						BIT_VECTOR_SET_FLAG(bit_vector, move_position->cluster_index, TRUE);
					}
				}
			}
		}
	}

	return;
}

void encounter_detach_actor(
	long actor_index,
	boolean died)
{
	if (ai_globals->ai_initialized_for_map)
	{
		struct actor_datum *actor = actor_get(actor_index);

		if (actor->meta.encounter_index != NONE)
		{
			long *actor_index_reference;
			struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

			for (actor_index_reference = &encounter->first_actor_index; *actor_index_reference != actor_index; actor_index_reference = &actor_get(*actor_index_reference)->meta.next_actor_index)
			{
				match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 544, *actor_index_reference!=NONE);
			}

			*actor_index_reference = actor->meta.next_actor_index;

			if (!died)
			{
				struct squad_datum *squad = encounter_get_squad(encounter, actor->meta.squad_index);

				match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 556, encounter->original_count > 0);
				encounter->original_count--;

				if (actor->meta.unique_leader)
				{
					match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 561, encounter->unique_leader_count > 0);
					encounter->unique_leader_count--;
				}

				match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 565, squad->original_count > 0);
				squad->original_count--;

				if (actor->meta.platoon_index != NONE)
				{
					struct platoon_datum *platoon = encounter_get_platoon(encounter, actor->meta.platoon_index);

					match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 573, platoon->original_count > 0);
					platoon->original_count--;
				}
			}

			actor->meta.next_actor_index = NONE;
			actor->meta.encounter_index = NONE;
			actor->meta.platoon_index = NONE;
			actor->meta.squad_index = NONE;
			encounter->status_dirty = TRUE;
		}
	}

	return;
}

void encounter_attach_unit(
	long encounter_index,
	long unit_index)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.swarm_actor_index != NONE)
	{
		struct actor_datum *actor = actor_get(unit->unit.swarm_actor_index);

		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 603, actor->meta.encounter_index == encounter_index);
	}
	else
	{
		struct actor_datum *actor = actor_get(unit->unit.actor_index);

		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 609, actor->meta.encounter_index == encounter_index);
		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 610, actor->meta.unit_index == unit_index);
	}

	if (!game_engine_running() && encounter->team_index == _game_team_default)
	{
		encounter->team_index = unit->object.owner_team_index;

		if (encounter->first_actor_index != NONE)
		{
			ai_update_team_status();
		}
	}

	return;
}

void encounter_attach_actor(
	long actor_index,
	long encounter_index,
	short squad_index,
	boolean has_previous_team)
{
	if (ai_globals->ai_initialized_for_map)
	{
		struct actor_datum *actor = actor_get(actor_index);
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
		struct encounter_datum *encounter = encounter_get(encounter_index);
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
			struct encounter_definition);
		struct squad_datum *squad = encounter_get_squad(encounter, squad_index);
		struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
			&encounter_definition->squads,
			squad_index,
			struct squad_definition);
		short platoon_index = squad_definition->platoon_index;
		boolean activated = FALSE;

		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 650, actor->meta.encounter_index==NONE);

		actor->meta.disconnected_encounter_index = NONE;
		actor->meta.disconnected_squad_index = NONE;
		actor->meta.next_actor_index = encounter->first_actor_index;
		encounter->first_actor_index = actor_index;

		if (platoon_index < 0 || platoon_index >= encounter_definition->platoons.count)
		{
			platoon_index = NONE;
		}

		actor->meta.encounter_index = encounter_index;
		actor->meta.squad_index = squad_index;
		actor->meta.platoon_index = platoon_index;

		if (actor->meta.active && !actor->meta.dormant)
		{
			activated = encounter_force_activate(encounter_index);
		}

		if (!activated)
		{
			actor_set_active(actor_index, encounter->active);

			if (encounter->active)
			{
				actor_set_dormant(actor_index, FALSE);
			}
		}

		if (actor->meta.unit_index != NONE)
		{
			encounter_attach_unit(encounter_index, actor->meta.unit_index);
		}

		if (actor->meta.team_index != encounter->team_index)
		{
			if (has_previous_team)
			{
				if (!encounter->current_count)
				{
					encounter->team_index = actor->meta.team_index;
					ai_update_team_status();
				}
				else
				{
					error(
						_error_silent,
						"WARNING: actor changing to encounter %s/%s is being forced to change teams",
						encounter_definition->name,
						squad_definition->name);
					actor_set_team(actor_index, encounter->team_index);
				}
			}
			else
			{
				actor_set_team(actor_index, encounter->team_index);
			}
		}

		encounter->original_count++;
		squad->original_count++;

		if (actor->meta.unique_leader)
		{
			encounter->unique_leader_count++;
		}

		if (platoon_index != NONE)
		{
			struct platoon_datum *platoon = encounter_get_platoon(encounter, platoon_index);

			actor->external_orders.defending = platoon->defending;
			actor->emotions.currently_defending = platoon->defending;
			platoon->original_count++;
		}

		encounter->status_dirty = TRUE;
	}

	return;
}

void encounterless_attach_actor(
	long actor_index)
{
	if (ai_globals->ai_initialized_for_map)
	{
		struct actor_datum *actor = actor_get(actor_index);

		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 758, actor->meta.encounter_index==NONE);
		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 759, !actor->meta.encounterless);

		actor->meta.next_actor_index = ai_globals->first_encounterless_actor_index;
		ai_globals->first_encounterless_actor_index = actor_index;
		actor->meta.encounterless = TRUE;
		actor->meta.encounterless_active_timer = actor->meta.active ? ENCOUNTERLESS_REMAIN_ACTIVE_TICKS : 0;
		actor_flush_position_indices(actor_index);
	}

	return;
}

void encounterless_detach_actor(
	long actor_index)
{
	if (ai_globals->ai_initialized_for_map)
	{
		long *actor_index_reference;
		struct actor_datum *actor = actor_get(actor_index);

		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 782, actor->meta.encounterless);
		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 783, actor->meta.encounter_index == NONE);
		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 784, actor->meta.squad_index == NONE);
		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 785, actor->meta.platoon_index == NONE);

		for (actor_index_reference = &ai_globals->first_encounterless_actor_index; *actor_index_reference != actor_index; actor_index_reference = &actor_get(*actor_index_reference)->meta.next_actor_index)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 796, *actor_index_reference!=NONE);
		}

		*actor_index_reference = actor->meta.next_actor_index;
		actor->meta.encounterless = FALSE;
		actor->meta.next_actor_index = NONE;
		actor->meta.force_active = FALSE;
	}

	return;
}

long encounter_get_by_name(
	char const *encounter_name)
{
	long result = NONE;
	struct scenario *scenario = global_scenario_get();

	if (scenario)
	{
		long encounter_index;

		for (encounter_index = 0; encounter_index < scenario->ai_encounters.count; encounter_index++)
		{
			struct encounter_definition *encounter = TAG_BLOCK_GET_ELEMENT(
				&scenario->ai_encounters,
				encounter_index,
				struct encounter_definition);

			if (!strncmp(encounter->name, encounter_name, TAG_STRING_LENGTH + 1))
			{
				result = encounter_index;
				break;
			}
		}
	}

	return result;
}

void encounter_iterator_new(
	struct encounter_iterator *iterator,
	boolean active_only)
{
	if (ai_globals->ai_initialized_for_map)
	{
		data_iterator_new(&iterator->encounter_iterator, encounter_data);
		iterator->active_only = active_only;
	}

	return;
}

struct encounter_datum *encounter_iterator_next(
	struct encounter_iterator *iterator)
{
	struct encounter_datum *encounter = NULL;

	if (ai_globals->ai_initialized_for_map)
	{
		do
		{
			encounter = data_iterator_next(&iterator->encounter_iterator);
		}
		while (encounter && iterator->active_only && !encounter->active);

		iterator->index = iterator->encounter_iterator.index;
	}

	return encounter;
}

void encounter_actor_iterator_new(
	struct encounter_actor_iterator *iterator,
	long encounter_index)
{
	if (ai_globals->ai_initialized_for_map)
	{
		iterator->encounter_index = encounter_index;
		iterator->index = NONE;

		if (encounter_index == NONE)
		{
			iterator->next_index = ai_globals->first_encounterless_actor_index;
		}
		else
		{
			iterator->next_index = encounter_get(encounter_index)->first_actor_index;
		}
	}

	return;
}

struct actor_datum *encounter_actor_iterator_next(
	struct encounter_actor_iterator *iterator)
{
	struct actor_datum *actor = NULL;

	if (ai_globals->ai_initialized_for_map)
	{
		iterator->index = iterator->next_index;

		if (iterator->index != NONE)
		{
			actor = actor_get(iterator->index);
			iterator->next_index = actor->meta.next_actor_index;
		}
	}

	return actor;
}

struct actor_datum *encounter_actor_iterator_prev(
	struct encounter_actor_iterator *iterator)
{
	struct actor_datum *actor = NULL;

	if (ai_globals->ai_initialized_for_map)
	{
		long actor_index = encounter_get(iterator->encounter_index)->first_actor_index;
		long previous_actor_index = NONE;

		while (actor_index != iterator->index && actor_index != NONE)
		{
			previous_actor_index = actor_index;
			actor = actor_get(actor_index);
			actor_index = actor->meta.next_actor_index;
		}

		if (actor_index != iterator->index)
		{
			return NULL;
		}

		iterator->next_index = actor_index;
		iterator->index = previous_actor_index;
	}

	return actor;
}

void actor_iterator_new(
	struct actor_iterator *iterator,
	boolean active_only)
{
	if (ai_globals->ai_initialized_for_map)
	{
		data_iterator_new(&iterator->encounter_iterator, encounter_data);
		iterator->iterated_encounterless_list = FALSE;
		iterator->next_index = NONE;
		iterator->index = NONE;
		iterator->active_only = active_only;
	}

	return;
}

struct actor_datum *actor_iterator_next(
	struct actor_iterator *iterator)
{
	struct actor_datum *actor = NULL;

	if (ai_globals->ai_initialized_for_map)
	{
		while (iterator->next_index == NONE)
		{
			struct encounter_datum *encounter = data_iterator_next(&iterator->encounter_iterator);

			if (!encounter)
			{
				if (!iterator->iterated_encounterless_list)
				{
					iterator->next_index = ai_globals->first_encounterless_actor_index;
					iterator->iterated_encounterless_list = TRUE;
				}

				break;
			}

			if (!iterator->active_only || encounter->active)
			{
				iterator->next_index = encounter->first_actor_index;
			}
		}

		while (!actor)
		{
			iterator->index = iterator->next_index;

			if (iterator->index == NONE)
			{
				break;
			}

			actor = actor_get(iterator->index);
			iterator->next_index = actor->meta.next_actor_index;

			if (iterator->active_only && !actor->meta.active)
			{
				actor = NULL;
			}
		}
	}

	return actor;
}

boolean encounter_mark_examined_pursuit_position(
	long encounter_index,
	long actor_index,
	short firing_position_index,
	long history_start_time)
{
	boolean result = FALSE;
	long pursuit_index = encounter_find_pursuit(encounter_index, firing_position_index, history_start_time, TRUE);

	if (pursuit_index != NONE)
	{
		short index;
		struct pursuit_datum *pursuit = pursuit_get(pursuit_index);

		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 1031, pursuit->firing_position_index == firing_position_index);

		for (index = 0; index < NUMBER_OF_ACTOR_INDICES_PER_EXAMINED_PURSUIT_POSITION; index++)
		{
			if (pursuit->actor_indices[index] == actor_index)
			{
				break;
			}
		}

		if (index >= NUMBER_OF_ACTOR_INDICES_PER_EXAMINED_PURSUIT_POSITION)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 1044, (pursuit->next_actor_index_index >= 0) && (pursuit->next_actor_index_index < NUMBER_OF_ACTOR_INDICES_PER_EXAMINED_PURSUIT_POSITION));

			pursuit->actor_indices[pursuit->next_actor_index_index] = actor_index;
			pursuit->next_actor_index_index = (pursuit->next_actor_index_index + 1) % NUMBER_OF_ACTOR_INDICES_PER_EXAMINED_PURSUIT_POSITION;
			pursuit->actor_count++;
			result = TRUE;
		}

		pursuit->last_examined_time = game_time_get();
	}

	return result;
}

boolean encounter_pursuit_position_already_examined(
	long encounter_index,
	long actor_index,
	short firing_position_index,
	long history_start_time,
	short *actor_count_reference,
	long *last_examined_time_reference)
{
	long pursuit_index = encounter_find_pursuit(encounter_index, firing_position_index, history_start_time, FALSE);
	long last_examined_time = NONE;
	boolean examined = FALSE;
	short actor_count = 0;

	if (pursuit_index != NONE)
	{
		struct pursuit_datum *pursuit = pursuit_get(pursuit_index);

		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 1076, pursuit->firing_position_index == firing_position_index);

		actor_count = pursuit->actor_count;
		last_examined_time = pursuit->last_examined_time;

		if (actor_count < NUMBER_OF_ACTOR_INDICES_PER_EXAMINED_PURSUIT_POSITION + 1)
		{
			short index;

			for (index = 0; index < NUMBER_OF_ACTOR_INDICES_PER_EXAMINED_PURSUIT_POSITION; index++)
			{
				if (pursuit->actor_indices[index] == actor_index)
				{
					examined = TRUE;
					break;
				}
			}
		}
		else
		{
			examined = TRUE;
		}
	}

	if (actor_count_reference)
	{
		*actor_count_reference = actor_count;
	}

	if (last_examined_time_reference)
	{
		*last_examined_time_reference = last_examined_time;
	}

	return examined;
}

static void encounter_clear_pursuit(
	long encounter_index)
{
	long pursuit_index;
	struct encounter_datum *encounter = encounter_get(encounter_index);

	for (pursuit_index = encounter->first_pursuit_index; pursuit_index != NONE; pursuit_index = encounter->first_pursuit_index)
	{
		encounter->first_pursuit_index = pursuit_get(pursuit_index)->next_pursuit_index;
		datum_delete(pursuit_data, pursuit_index);
	}

	return;
}

static long encounter_find_pursuit(
	long encounter_index,
	short firing_position_index,
	long history_start_time,
	boolean force_create)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);
	long pursuit_index = encounter->first_pursuit_index;
	boolean reset = FALSE;

	while (pursuit_index != NONE)
	{
		struct pursuit_datum *pursuit = pursuit_get(pursuit_index);

		if (pursuit->firing_position_index == firing_position_index)
		{
			if (pursuit->last_examined_time < history_start_time)
			{
				reset = TRUE;
			}

			break;
		}

		pursuit_index = pursuit->next_pursuit_index;
	}

	if (pursuit_index == NONE && force_create)
	{
		pursuit_index = datum_new(pursuit_data);

		if (pursuit_index != NONE)
		{
			struct pursuit_datum *pursuit = pursuit_get(pursuit_index);

			pursuit->firing_position_index = firing_position_index;
			pursuit->next_pursuit_index = encounter->first_pursuit_index;
			encounter->first_pursuit_index = pursuit_index;
			reset = TRUE;
		}
		else
		{
			error(
				_error_silent,
				"WARNING: too many actors searching, exceeded MAXIMUM_EXAMINED_PURSUIT_POSITIONS_PER_MAP (%d)",
				MAXIMUM_EXAMINED_PURSUIT_POSITIONS_PER_MAP);
		}
	}

	if (reset)
	{
		struct pursuit_datum *pursuit = pursuit_get(pursuit_index);

		pursuit->last_examined_time = NONE;
		pursuit->actor_count = 0;
		pursuit->next_actor_index_index = 0;
		memset(pursuit->actor_indices, NONE, sizeof(pursuit->actor_indices));

		if (!force_create)
		{
			pursuit_index = NONE;
		}
	}

	return pursuit_index;
}

void encounter_modify_pursuit_desires(
	long encounter_index,
	short squad_index,
	boolean *pursue_tenacious,
	short *group_pursuit_restriction,
	boolean *group_pursuit_controller,
	short *desired_target_search,
	short *desired_pursuit,
	short *desired_pursuit_search)
{
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
		&encounter_definition->squads,
		squad_index,
		struct squad_definition);
	short searching = encounter_definition->searching;

	if (TEST_FLAG(squad_definition->flags, _squad_never_search_bit))
	{
		searching = _encounter_search_never;
	}

	switch (searching)
	{
	case _encounter_search_never:
		*group_pursuit_restriction = _group_pursuit_restriction_never;
		*desired_pursuit = 2;
		*desired_pursuit_search = 2;
		break;
	case _encounter_search_tenacious:
		*pursue_tenacious = TRUE;
		*desired_target_search = 0;
		*desired_pursuit = 0;
		*desired_pursuit_search = 0;
		*group_pursuit_controller = FALSE;
		break;
	}

	return;
}

void encounter_determine_pursuit_availability(
	long encounter_index,
	long actor_index,
	short group_pursuit_restriction,
	boolean is_pursuit_controller,
	boolean *allow_target_uncover,
	boolean *allow_indefinite_target_uncover,
	boolean *allow_target_search,
	boolean *allow_pursuit,
	boolean *allow_pursuit_search,
	boolean *controlling_group_pursuit,
	boolean *controlled_by_group_pursuit,
	boolean *wait_after_pursuit)
{
	struct encounter_actor_iterator iterator;
	struct actor_datum *actor;
	struct actor_datum *pursuit_actor;
	short combat_count;
	short pursue_count;
	short search_count;
	short uncover_count;
	short maximum_uncover_count;
	short maximum_search_count;
	short maximum_pursue_count;

	encounter_actor_iterator_new(&iterator, encounter_index);
	combat_count = 0;
	search_count = 0;
	pursue_count = 0;
	uncover_count = 0;

	while (actor = encounter_actor_iterator_next(&iterator))
	{
		if (iterator.index != actor_index && actor->external_orders.pursuit_is_coordinator == is_pursuit_controller)
		{
			if (actor->state.action == _actor_action_uncover)
			{
				if (actor->state.action_data.uncover.pursuit_location.type == _pursuit_location_target)
				{
					if (actor->state.combat_status < _actor_combat_status_definite)
					{
						uncover_count++;
					}
				}
				else
				{
					pursue_count++;
				}
			}
			else if (actor->state.action == _actor_action_search)
			{
				if (actor->state.action_data.search.pursuit_location.type == _pursuit_location_target)
				{
					search_count++;
				}
				else
				{
					pursue_count++;
				}
			}
		}

		if (actor->state.mode == _actor_mode_combat)
		{
			combat_count++;
		}
	}

	maximum_uncover_count = 6;
	maximum_search_count = 4;

	switch (group_pursuit_restriction)
	{
	case _group_pursuit_restriction_never:
		maximum_pursue_count = 0;
		break;
	case _group_pursuit_restriction_unlimited:
		maximum_pursue_count = ENCOUNTER_TIMER_FOREVER;
		break;
	default:
		maximum_pursue_count = FLOOR(combat_count / 3, 3);
		break;
	}

	pursuit_actor = actor_get(actor_index);

	if (is_pursuit_controller)
	{
		*controlling_group_pursuit = actor_pursuit_find_nearby_actors(actor_index, TRUE) >= 2;
		pursuit_actor->external_orders.pursuit_is_coordinator = *controlling_group_pursuit;
	}
	else
	{
		actor_pursuit_find_nearby_actors(actor_index, FALSE);
		*controlled_by_group_pursuit = pursuit_actor->external_orders.pursuit_group_prop_index != NONE;
		pursuit_actor->external_orders.pursuit_is_coordinator = FALSE;
	}

	*wait_after_pursuit = controlling_group_pursuit || controlled_by_group_pursuit;
	*allow_indefinite_target_uncover = uncover_count < maximum_uncover_count;
	*allow_target_search = search_count < maximum_search_count;
	*allow_pursuit_search = pursue_count < maximum_pursue_count;
	*allow_pursuit = pursue_count < maximum_pursue_count;

	if (ai_debug.print_pursuit_checks &&
		encounter_index == ai_debug.selected_encounter_index &&
		(actor_index == ai_debug.selected_actor_index || ai_debug.selected_actor_index == NONE))
	{
		struct actor_datum *debug_actor = actor_get(actor_index);

		csprintf(
			temporary,
			"%s %04X: coord %d current %d/%d/%d max %d/%d/%d allow %c%c%c",
			actor_type_get_name(debug_actor->meta.type),
			DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index),
			is_pursuit_controller,
			uncover_count,
			search_count,
			pursue_count,
			maximum_uncover_count,
			maximum_search_count,
			maximum_pursue_count,
			*allow_indefinite_target_uncover ? 'Y' : 'N',
			*allow_target_search ? 'Y' : 'N',
			*allow_pursuit ? 'Y' : 'N');
		error(
			_error_silent,
			temporary);
		console_printf(FALSE, temporary);
	}

	return;
}

static void squad_reset_starting_locations(
	long encounter_index,
	short squad_index)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	struct squad_datum *squad = encounter_get_squad(encounter, squad_index);
	struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
		&encounter_definition->squads,
		squad_index,
		struct squad_definition);
	short index;

	memset(squad->unused_locations, NONE, BIT_VECTOR_SIZE_IN_BYTES(squad_definition->starting_locations.count));

	for (index = 0; index < squad_definition->starting_locations.count; index++)
	{
		struct actor_starting_location_definition *starting_location = TAG_BLOCK_GET_ELEMENT(
			&squad_definition->starting_locations,
			index,
			struct actor_starting_location_definition);

		if (TEST_FLAG(starting_location->flags, _actor_starting_location_required_bit))
		{
			BIT_VECTOR_SET_FLAG(squad->required_locations, index, TRUE);
		}
	}

	return;
}

static void encounter_new(
	struct encounter_definition *encounter_definition,
	short *squad_base,
	short *platoon_base)
{
	long encounter_index = datum_new(encounter_data);

	if (encounter_index != NONE)
	{
		short squad_index;
		short platoon_index;
		struct encounter_datum *encounter = encounter_get(encounter_index);

		encounter->team_index = encounter_definition->team_index;
		encounter->first_actor_index = NONE;
		encounter->first_pursuit_index = NONE;
		encounter->blind = TEST_FLAG(encounter_definition->flags, _encounter_blind_bit);
		encounter->deaf = TEST_FLAG(encounter_definition->flags, _encounter_deaf_bit);
		encounter->respawn_enabled = TEST_FLAG(encounter_definition->flags, _encounter_respawn_enable_bit);
		encounter->respawn_delay_ticks = 0;
		encounter->enemy_traitor = FALSE;
		encounter->enemy_visible = FALSE;
		encounter->enemy_visible_timer = NONE;
		encounter->enemy_alive = FALSE;
		encounter->enemy_alive_timer = NONE;
		encounter->corpse_ignore_time = NONE;
		encounter->stand_down = TRUE;
		encounter->last_grenade_throw_time = NONE;
		encounter->link_encounter_count = 0;
		encounter->last_active_time = NONE;

		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 1444, encounter_definition->squads.count <= MAXIMUM_SQUADS_PER_ENCOUNTER);

		encounter->squad_count = (short)encounter_definition->squads.count;
		encounter->squad_base = *squad_base;
		*squad_base += encounter->squad_count;
		match_vassert("c:\\halo\\SOURCE\\ai\\encounters.c", 1448, *squad_base <= MAXIMUM_SQUADS_PER_MAP, csprintf(temporary, "overflowed MAXIMUM_SQUADS_PER_MAP (%d)", MAXIMUM_SQUADS_PER_MAP));

		for (squad_index = 0; squad_index < encounter->squad_count; squad_index++)
		{
			struct squad_datum *squad = encounter_get_squad(encounter, squad_index);
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->squads,
				squad_index,
				struct squad_definition);

			squad->delay_timer_started = FALSE;

			if (TEST_FLAG(squad_definition->flags, _squad_delay_forever_bit))
			{
				squad->delay_timer = ENCOUNTER_TIMER_FOREVER;
			}
			else
			{
				squad->delay_timer = (short)(squad_definition->squad_delay_timer * TICKS_PER_SECOND);
			}

			squad->automatic_migration_target = TEST_FLAG(squad_definition->flags, _squad_automatic_migration_bit);
			squad_reset_starting_locations(encounter_index, squad_index);

			if (squad_definition->respawn_max_actors > 0 || squad_definition->respawn_min_actors > 0)
			{
				squad->respawn_actors_left = !squad_definition->respawn_total_count ? ENCOUNTER_TIMER_FOREVER : squad_definition->respawn_total_count;
			}
		}

		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 1483, encounter_definition->platoons.count <= MAXIMUM_PLATOONS_PER_ENCOUNTER);

		encounter->platoon_count = (short)encounter_definition->platoons.count;
		encounter->platoon_base = *platoon_base;
		*platoon_base += encounter->platoon_count;
		match_vassert("c:\\halo\\SOURCE\\ai\\encounters.c", 1487, *platoon_base <= MAXIMUM_PLATOONS_PER_MAP, csprintf(temporary, "overflowed MAXIMUM_PLATOONS_PER_MAP (%d)", MAXIMUM_PLATOONS_PER_MAP));

		for (platoon_index = 0; platoon_index < encounter->platoon_count; platoon_index++)
		{
			struct platoon_datum *platoon = encounter_get_platoon(encounter, platoon_index);
			struct platoon_definition *platoon_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->platoons,
				platoon_index,
				struct platoon_definition);

			platoon->defending = TEST_FLAG(platoon_definition->flags, _platoon_initially_defending_bit);
		}
	}

	return;
}

short encounter_get_actor_starting_location(
	long encounter_index,
	long squad_index,
	boolean spawn)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	struct squad_datum *squad = encounter_get_squad(encounter, (short)squad_index);
	struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
		&encounter_definition->squads,
		squad_index,
		struct squad_definition);
	unsigned long disallowed_starting_locations[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_STARTING_LOCATIONS_PER_SQUAD)];
	short found_index = NONE;
	short required_count;
	short index;

	memset(disallowed_starting_locations, 0, sizeof(disallowed_starting_locations));
	required_count = 0;

	for (index = 0; index < squad_definition->starting_locations.count; index++)
	{
		if (BIT_VECTOR_TEST_FLAG(squad->required_locations, index) && !BIT_VECTOR_TEST_FLAG(disallowed_starting_locations, index))
		{
			required_count++;
		}
	}

	if (required_count > 0)
	{
		short random_index = random_range(0, required_count);

		for (index = 0; index < squad_definition->starting_locations.count; index++)
		{
			if (BIT_VECTOR_TEST_FLAG(squad->required_locations, index) && !BIT_VECTOR_TEST_FLAG(disallowed_starting_locations, index))
			{
				if (!random_index)
				{
					BIT_VECTOR_SET_FLAG(squad->required_locations, index, FALSE);
					match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 1557, BIT_VECTOR_TEST_FLAG(squad->unused_locations, index));
					BIT_VECTOR_SET_FLAG(squad->unused_locations, index, FALSE);
					found_index = index;
					break;
				}

				random_index--;
			}
		}

		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 1571, found_index != NONE);
	}

	if (found_index == NONE)
	{
		boolean reset;
		short unused_count;

		do
		{
			boolean any_used = FALSE;

			reset = FALSE;
			unused_count = 0;

			for (index = 0; index < squad_definition->starting_locations.count; index++)
			{
				if (!BIT_VECTOR_TEST_FLAG(disallowed_starting_locations, index))
				{
					if (BIT_VECTOR_TEST_FLAG(squad->unused_locations, index))
					{
						unused_count++;
					}
					else
					{
						any_used = TRUE;
					}
				}
			}

			if (any_used && !unused_count)
			{
				memset(squad->unused_locations, NONE, BIT_VECTOR_SIZE_IN_BYTES(squad_definition->starting_locations.count));
				reset = TRUE;
			}
		}
		while (reset);

		if (unused_count > 0)
		{
			short random_index = random_range(0, unused_count);

			for (index = 0; index < squad_definition->starting_locations.count; index++)
			{
				if (BIT_VECTOR_TEST_FLAG(squad->unused_locations, index) && !BIT_VECTOR_TEST_FLAG(disallowed_starting_locations, index))
				{
					if (!random_index)
					{
						BIT_VECTOR_SET_FLAG(squad->unused_locations, index, FALSE);
						found_index = index;
						break;
					}

					random_index--;
				}
			}

			match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 1637, found_index != NONE);
		}
	}

	return found_index;
}

static short squad_get_actor_type(
	struct squad_definition *squad_definition)
{
	struct scenario *scenario = global_scenario_get();
	short actor_type = _actor_none;

	if (squad_definition->actor_palette_index >= 0 && squad_definition->actor_palette_index < scenario->ai_actor_palette.count)
	{
		struct actor_palette_entry *palette_entry = TAG_BLOCK_GET_ELEMENT(
			&scenario->ai_actor_palette,
			squad_definition->actor_palette_index,
			struct actor_palette_entry);

		if (palette_entry->reference.index != NONE)
		{
			struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(palette_entry->reference.index);

			if (actor_variant_definition->actor_reference.index != NONE)
			{
				actor_type = actor_definition_get(actor_variant_definition->actor_reference.index)->type;
			}
		}
	}

	return actor_type;
}

void encounter_create(
	long encounter_index,
	short desired_platoon_index,
	short desired_squad_index)
{
	if (ai_globals->ai_initialized_for_map)
	{
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
			struct encounter_definition);
		boolean create_all = desired_platoon_index == NONE && desired_squad_index == NONE;
		short squad_index;

		if (ai_debug.print_placement)
		{
			if (desired_platoon_index != NONE)
			{
				struct platoon_definition *platoon_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter_definition->platoons,
					desired_platoon_index,
					struct platoon_definition);

				console_printf(
					FALSE,
					"ai_place %s/%s",
					encounter_definition->name,
					platoon_definition->name);
			}
			else if (desired_squad_index != NONE)
			{
				struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter_definition->squads,
					desired_squad_index,
					struct squad_definition);

				console_printf(
					FALSE,
					"ai_place %s/%s",
					encounter_definition->name,
					squad_definition->name);
			}
			else
			{
				console_printf(
					FALSE,
					"ai_place %s",
					encounter_definition->name);
			}
		}

		for (squad_index = 0; squad_index < encounter_definition->squads.count; squad_index++)
		{
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->squads,
				squad_index,
				struct squad_definition);

			if (create_all || squad_index == desired_squad_index || (squad_definition->platoon_index != NONE && squad_definition->platoon_index == desired_platoon_index))
			{
				short count;
				short actor_type;
				short index;
				short variant_number = 0;

				switch (game_difficulty_level_get())
				{
				case _game_difficulty_level_easy:
				case _game_difficulty_level_normal:
					count = squad_definition->min_count;
					break;
				case _game_difficulty_level_hard:
					count = (squad_definition->min_count + squad_definition->max_count) / 2;
					break;
				case _game_difficulty_level_impossible:
					count = squad_definition->max_count;
					break;
				default:
					match_halt("c:\\halo\\SOURCE\\ai\\encounters.c", 1730);
				}

				actor_type = squad_get_actor_type(squad_definition);

				switch (squad_definition->unique_leader_type)
				{
				case _leader_normal:
				{
					struct encounter_datum *encounter = encounter_get(encounter_index);
					boolean create_leader = FALSE;

					switch (actor_type)
					{
					case _actor_marine:
						if (encounter->unique_leader_count == 0)
						{
							create_leader = count + encounter->original_count >= 4;
						}
						else if (encounter->unique_leader_count == 1)
						{
							create_leader = count + encounter->original_count >= 10;
						}

						if (ai_debug.print_placement)
						{
							console_printf(
								FALSE,
								"%s/%s: %d current %d leaders, create %d -> %s",
								encounter_definition->name,
								squad_definition->name,
								encounter->original_count,
								encounter->unique_leader_count,
								count,
								create_leader ? "new leader" : "no leader");
						}
						break;
					}

					if (!create_leader)
					{
						break;
					}
				}
				case _leader_random:
					switch (actor_type)
					{
					case _actor_marine:
						variant_number = random_range(0, 2) + 100;
						break;
					}
					break;
				case _leader_sergeant_johnson:
					if (actor_type == _actor_marine)
					{
						variant_number = 100;
					}
					break;
				case _leader_sergeant_lehto:
					if (actor_type == _actor_marine)
					{
						variant_number = 101;
					}
					break;
				}

				for (index = 0; index < count; index++)
				{
					encounter_place_actor(encounter_index, squad_index, variant_number, FALSE);
					variant_number = 0;
				}
			}
		}

		encounter_update_status(encounter_index);
		encounters_test_activation();
	}

	return;
}

void encounterless_activate(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 1824, actor->meta.encounterless);

	actor->meta.encounterless_active_timer = ENCOUNTERLESS_REMAIN_ACTIVE_TICKS;
	actor_set_active(actor_index, TRUE);

	return;
}

static void encounterless_deactivate(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 1838, actor->meta.encounterless);

	actor->meta.encounterless_active_timer = 0;
	actor_set_active(actor_index, FALSE);

	return;
}

boolean encounter_force_activate(
	long encounter_index)
{
	encounter_get(encounter_index)->remain_active_timer = ENCOUNTER_REMAIN_ACTIVE_TICKS;

	return encounter_activate(encounter_index);
}

static boolean encounter_activate(
	long encounter_index)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);

	if (encounter_definition->runtime_structure_bsp_reference_index == NONE ||
		encounter_definition->runtime_structure_bsp_reference_index == global_structure_bsp_index)
	{
		if (!encounter->active)
		{
			struct encounter_actor_iterator iterator;

			encounter_actor_iterator_new(&iterator, encounter_index);

			while (encounter_actor_iterator_next(&iterator))
			{
				actor_set_active(iterator.index, TRUE);
			}
		}

		encounter->last_active_time = game_time_get();
		encounter->active = TRUE;
	}

	return encounter->active;
}

boolean encounter_link_activation(
	long encounter_index,
	short link_encounter_index)
{
	short link_index;
	struct encounter_datum *encounter = encounter_get(encounter_index);
	boolean found = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 1916, (link_encounter_index >= 0) && (link_encounter_index < global_scenario_get()->ai_encounters.count));

	for (link_index = 0; link_index < encounter->link_encounter_count; link_index++)
	{
		if (encounter->link_encounter_indices[link_index] == link_encounter_index)
		{
			found = TRUE;
			break;
		}
	}

	if (!found && encounter->link_encounter_count < MAXIMUM_ACTIVATION_LINK_INDICES_PER_ENCOUNTER)
	{
		encounter->link_encounter_indices[encounter->link_encounter_count++] = link_encounter_index;
		found = TRUE;
	}

	return found;
}

void encounter_force_deactivate(
	long encounter_index)
{
	encounter_get(encounter_index)->remain_active_timer = 0;
	encounter_deactivate(encounter_index);

	return;
}

static void encounter_deactivate(
	long encounter_index)
{
	struct encounter_actor_iterator iterator;
	struct actor_datum *actor;

	encounter_get(encounter_index)->active = FALSE;
	encounter_clear_pursuit(encounter_index);
	encounter_actor_iterator_new(&iterator, encounter_index);

	while (actor = encounter_actor_iterator_next(&iterator))
	{
		if (actor->meta.active)
		{
			actor_set_active(iterator.index, FALSE);
		}

		actor_verify_activation(iterator.index);
	}

	return;
}

void encounter_update_status(
	long encounter_index)
{
	struct encounter_actor_iterator iterator;
	struct actor_datum *actor;
	short squad_index;
	short platoon_index;
	struct encounter_datum *encounter = encounter_get(encounter_index);
	boolean nonally_target = FALSE;
	boolean postcombat_actors = FALSE;
	boolean in_combat = FALSE;
	boolean had_visible_enemy = FALSE;
	boolean been_in_combat = FALSE;
	boolean combat_continues = FALSE;

	encounter->enemy_alive = FALSE;
	encounter->enemy_visible = FALSE;
	encounter->current_fighting_count = 0;
	encounter->current_in_combat_count = 0;
	encounter->current_swarm_count = 0;
	encounter->current_count = 0;
	encounter->current_strength_fraction = 0.f;

	for (squad_index = 0; squad_index < encounter->squad_count; squad_index++)
	{
		struct squad_datum *squad = encounter_get_squad(encounter, squad_index);

		squad->current_swarm_count = 0;
		squad->current_count = 0;
		squad->current_strength_fraction = 0.f;
	}

	for (platoon_index = 0; platoon_index < encounter->platoon_count; platoon_index++)
	{
		struct platoon_datum *platoon = encounter_get_platoon(encounter, platoon_index);

		platoon->current_swarm_count = 0;
		platoon->current_count = 0;
		platoon->current_strength_fraction = 0.f;
	}

	encounter_actor_iterator_new(&iterator, encounter_index);

	while (actor = encounter_actor_iterator_next(&iterator))
	{
		real strength;
		short count;
		struct squad_datum *squad = encounter_get_squad(encounter, actor->meta.squad_index);

		if (actor->meta.unit_index != NONE)
		{
			strength = unit_get(actor->meta.unit_index)->object.body_vitality;
			count = 1;
		}
		else
		{
			strength = (real)actor->meta.swarm_unit_count / (real)actor->meta.swarm_original_unit_count;
			count = actor->meta.swarm_unit_count;
		}

		if (actor->meta.platoon_index != NONE)
		{
			struct platoon_datum *platoon = encounter_get_platoon(encounter, actor->meta.platoon_index);

			platoon->current_count += count;
			platoon->current_swarm_count += actor->meta.swarm * count;
			platoon->current_strength_fraction += strength;
		}

		squad->current_count += count;
		squad->current_swarm_count += actor->meta.swarm * count;
		squad->current_strength_fraction += strength;
		encounter->current_count += count;
		encounter->current_swarm_count += actor->meta.swarm * count;
		encounter->current_in_combat_count += actor_in_combat(iterator.index) * count;
		encounter->current_fighting_count += actor_is_fighting(iterator.index) * count;
		encounter->current_strength_fraction += strength;

		if (actor->target.target_prop_index != NONE)
		{
			struct prop_datum *prop = prop_get(actor->target.target_prop_index);

			encounter->enemy_target = TRUE;

			if (!game_team_is_ally(actor->meta.team_index, prop->team_index))
			{
				nonally_target = TRUE;
			}

			if (actor_in_combat(iterator.index))
			{
				in_combat = TRUE;
			}

			if (actor->state.had_visible_enemy)
			{
				had_visible_enemy = TRUE;
			}

			if (actor->state.been_in_combat)
			{
				been_in_combat = TRUE;
			}

			if (actor->state.combat_status >= _actor_combat_status_visible)
			{
				encounter->enemy_visible = TRUE;
				encounter->enemy_alive = TRUE;
			}
			else if (prop->state < _prop_state_becoming_unacknowledged || prop->state > _prop_state_acknowledged)
			{
				if (!TEST_FLAG(unit_get(prop->unit_index)->object.damage_flags, _object_dead_bit))
				{
					encounter->enemy_alive = TRUE;
				}
			}
			else if (!prop->dead)
			{
				encounter->enemy_alive = TRUE;
			}
		}

		if (actor->external_orders.postcombat_type > _actor_postcombat_none)
		{
			postcombat_actors = TRUE;
		}
	}

	if (nonally_target)
	{
		encounter->enemy_traitor = FALSE;
	}

	if (encounter->enemy_visible || (encounter->enemy_visible_timer != NONE && encounter->enemy_visible_timer < 2 * TICKS_PER_SECOND))
	{
		combat_continues = TRUE;
	}
	else if ((encounter->enemy_alive || (encounter->enemy_alive_timer != NONE && encounter->enemy_alive_timer < 2 * TICKS_PER_SECOND)) &&
		encounter->enemy_visible_timer != NONE &&
		encounter->enemy_visible_timer < 15 * TICKS_PER_SECOND)
	{
		combat_continues = TRUE;
	}

	if (combat_continues)
	{
		encounter->stand_down = FALSE;
		encounter->post_combat = FALSE;
	}
	else if (encounter->stand_down)
	{
		encounter->post_combat = FALSE;
		encounter->prebattle_living_count = encounter->current_count;
		encounter->corpse_ignore_time = game_time_get();
		encounter->enemies_defeated = 0;

		if (!encounter->enemy_target)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 2156, !encounter->enemy_visible && !encounter->enemy_alive);

			encounter->enemy_visible_timer = NONE;
			encounter->enemy_alive_timer = NONE;
		}
	}
	else if (encounter->post_combat)
	{
		encounter->post_combat_delay = !postcombat_actors;

		if (!encounter->post_combat_delay_timer)
		{
			encounter_stand_down(encounter_index);
		}
	}
	else if (had_visible_enemy && been_in_combat)
	{
		encounter_post_combat(encounter_index);
	}
	else
	{
		encounter_stand_down(encounter_index);
	}

	if (encounter->original_count > 0)
	{
		encounter->current_strength_fraction = MAX(0.f, encounter->current_strength_fraction / (real)encounter->original_count - 0.001f);
	}

	for (squad_index = 0; squad_index < encounter->squad_count; squad_index++)
	{
		struct squad_datum *squad = encounter_get_squad(encounter, squad_index);

		squad->current_strength_fraction = MAX(0.f, squad->current_strength_fraction / (real)squad->original_count - 0.001f);
	}

	for (platoon_index = 0; platoon_index < encounter->platoon_count; platoon_index++)
	{
		struct platoon_datum *platoon = encounter_get_platoon(encounter, platoon_index);

		platoon->current_strength_fraction = MAX(0.f, platoon->current_strength_fraction / (real)platoon->original_count - 0.001f);
	}

	encounter->status_dirty = FALSE;

	return;
}

static void encounters_test_activation(
	void)
{
	struct data_iterator iterator;
	struct encounter_datum *encounter;
	struct actor_datum *actor;
	long actor_index;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	unsigned long const *combined_pvs = players_get_combined_pvs();

	for (actor_index = ai_globals->first_encounterless_actor_index; actor_index != NONE; actor_index = actor->meta.next_actor_index)
	{
		boolean activate;

		actor = actor_get(actor_index);
		activate = FALSE;
		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 2234, actor->meta.encounterless);

		if (actor->meta.swarm)
		{
			actor->meta.dormant_desire = TRUE;

			if (actor->meta.swarm_cache_index == NONE)
			{
				struct unit_datum *unit;
				long unit_index;

				for (unit_index = actor->meta.swarm_unit_index; unit_index != NONE; unit_index = unit->unit.swarm_next_unit_index)
				{
					long ultimate_parent_index;
					struct object_datum *object;

					unit = unit_get(unit_index);
					ultimate_parent_index = object_get_ultimate_parent(unit_index);
					object = object_get(ultimate_parent_index);

					if (object->object.location.cluster_index != NONE && BIT_VECTOR_TEST_FLAG(combined_pvs, object->object.location.cluster_index))
					{
						actor->meta.dormant_desire = FALSE;
						break;
					}
				}
			}
			else
			{
				short unit_index;
				struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

				for (unit_index = 0; unit_index < swarm->unit_count; unit_index++)
				{
					long ultimate_parent_index = object_get_ultimate_parent(swarm->unit_indices[unit_index]);
					struct object_datum *object = object_get(ultimate_parent_index);

					if (object->object.location.cluster_index != NONE && BIT_VECTOR_TEST_FLAG(combined_pvs, object->object.location.cluster_index))
					{
						actor->meta.dormant_desire = FALSE;
						break;
					}
				}
			}
		}
		else
		{
			long ultimate_parent_index = object_get_ultimate_parent(actor->meta.unit_index);
			struct object_datum *object = object_get(ultimate_parent_index);
			short cluster_index = object->object.location.cluster_index;

			if (cluster_index == NONE)
			{
				actor->meta.dormant_desire = TRUE;
			}
			else
			{
				actor->meta.dormant_desire = !BIT_VECTOR_TEST_FLAG(combined_pvs, cluster_index);
			}
		}

		activate |= actor->meta.force_active;
		activate |= game_in_editor();
		activate |= ai_debug.force_all_active | !actor->meta.dormant_desire;

		if (activate)
		{
			encounterless_activate(actor_index);
		}
		else
		{
			boolean deactivate = FALSE;

			if (actor->meta.encounterless_active_timer > ENCOUNTER_ACTIVATION_TEST_TICKS)
			{
				actor->meta.encounterless_active_timer -= ENCOUNTER_ACTIVATION_TEST_TICKS;
			}
			else
			{
				actor->meta.encounterless_active_timer = 0;
				deactivate = TRUE;
			}

			if (deactivate)
			{
				encounterless_deactivate(actor_index);
			}
		}

		actor_verify_activation(actor_index);
	}

	data_iterator_new(&iterator, encounter_data);

	while (encounter = data_iterator_next(&iterator))
	{
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index),
			struct encounter_definition);
		unsigned long activation_bitvector[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CLUSTERS_PER_STRUCTURE)];
		boolean activate;

		activate = encounter->force_active;
		activate |= game_in_editor();
		activate |= encounter->respawn_delay_ticks > 0;
		activate |= ai_debug.force_all_active;

		if (encounter_definition->runtime_structure_bsp_reference_index == NONE ||
			encounter_definition->runtime_structure_bsp_reference_index == global_structure_bsp_index)
		{
			encounter_compute_activation_cluster_bit_vector(iterator.index, TRUE, MAXIMUM_CLUSTERS_PER_STRUCTURE, combined_pvs, activation_bitvector);
			activate = bit_vector_and(structure_bsp->clusters.count, combined_pvs, activation_bitvector, NULL) | activate;
		}
		else
		{
			activate = FALSE;
		}

		if (activate)
		{
			encounter->remain_active_timer = ENCOUNTER_REMAIN_ACTIVE_TICKS;
			encounter_activate(iterator.index);
		}
		else if (encounter->active && encounter->remain_active_timer > ENCOUNTER_ACTIVATION_TEST_TICKS)
		{
			encounter->remain_active_timer -= ENCOUNTER_ACTIVATION_TEST_TICKS;
		}
		else
		{
			short link_index;
			boolean link_active = FALSE;

			for (link_index = 0; link_index < encounter->link_encounter_count; link_index++)
			{
				struct encounter_datum *link_encounter = encounter_get(encounter->link_encounter_indices[link_index]);

				if (link_encounter->remain_active_timer > 0)
				{
					link_active = TRUE;
				}
			}

			encounter->remain_active_timer = 0;

			if (link_active)
			{
				encounter_activate(iterator.index);
			}
			else
			{
				encounter_deactivate(iterator.index);
			}
		}
	}

	return;
}

void encounter_stand_down(
	long encounter_index)
{
	struct encounter_actor_iterator actor_iterator;
	struct actor_datum *actor;
	struct encounter_datum *encounter = encounter_get(encounter_index);

	match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 2431, !encounter->enemy_visible);

	encounter->stand_down = TRUE;
	encounter->enemies_defeated = 0;
	encounter_clear_pursuit(encounter_index);
	encounter_actor_iterator_new(&actor_iterator, encounter_index);

	while (actor = encounter_actor_iterator_next(&actor_iterator))
	{
		struct prop_iterator prop_iterator;
		struct prop_datum *prop;

		prop_iterator_new(&prop_iterator, actor_iterator.index);

		while (prop = prop_iterator_next(&prop_iterator))
		{
			if (prop->state >= _prop_state_uninspected_orphan &&
				prop->state <= _prop_state_inspected_orphan &&
				prop->enemy &&
				prop_iterator.index != actor->target.target_prop_index)
			{
				struct prop_datum *parent_prop;

				match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 2458, prop->parent_prop_index != NONE);

				parent_prop = prop_get(prop->parent_prop_index);
				match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 2463, parent_prop->orphan_prop_index == prop_iterator.index);

				parent_prop->orphan_prop_index = NONE;
				actor_switch_props(actor_iterator.index, prop_iterator.index, NONE);
				prop_delete(actor_iterator.index, prop_iterator.index);
			}
		}
	}

	return;
}

static boolean encounter_post_combat_add_possibility(
	struct post_combat_possibility *possibility_array,
	long actor_index,
	real weight,
	long prop_index,
	long unit_index)
{
	short possibility_index;
	boolean added = FALSE;

	for (possibility_index = 0; possibility_index < NUMBER_OF_POST_COMBAT_POSSIBILITIES; possibility_index++)
	{
		if (weight > possibility_array[possibility_index].weight)
		{
			short index;

			for (index = NUMBER_OF_POST_COMBAT_POSSIBILITIES - 1; index > possibility_index; index--)
			{
				possibility_array[index] = possibility_array[index - 1];
			}

			possibility_array[possibility_index].actor_index = actor_index;
			possibility_array[possibility_index].weight = weight;
			possibility_array[possibility_index].prop_index = prop_index;
			possibility_array[possibility_index].unit_index = unit_index;
			added = TRUE;
		}
	}

	return added;
}

static short encounter_post_combat_select_random_behavior(
	struct post_combat_possibility (*possibilities)[NUMBER_OF_POST_COMBAT_POSSIBILITIES],
	struct post_combat_possibility *selected_possibility)
{
	short behavior_index;
	short selected_behavior_index = NONE;
	real total_weight = 0.f;
	short possible_count = 0;

	for (behavior_index = 0; behavior_index < NUMBER_OF_POST_COMBAT_BEHAVIOR_TYPES; behavior_index++)
	{
		if (possibilities[behavior_index][0].weight > 0.f && possibilities[behavior_index][0].actor_index != NONE)
		{
			selected_behavior_index = behavior_index;
			possible_count++;
			total_weight += possibilities[behavior_index][0].weight;
		}
	}

	if (possible_count > 1)
	{
		real weight = 0.f;
		real random_weight = real_random() * total_weight;

		for (behavior_index = 0; behavior_index < NUMBER_OF_POST_COMBAT_BEHAVIOR_TYPES; behavior_index++)
		{
			if (possibilities[behavior_index][0].weight > 0.f && possibilities[behavior_index][0].actor_index != NONE)
			{
				weight += possibilities[behavior_index][0].weight;

				if (weight > random_weight)
				{
					selected_behavior_index = behavior_index;
					break;
				}
			}
		}
	}

	if (selected_behavior_index != NONE)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 2562, (selected_behavior_index >= 0) && (selected_behavior_index < NUMBER_OF_POST_COMBAT_BEHAVIOR_TYPES));

		*selected_possibility = possibilities[selected_behavior_index][0];
	}

	return selected_behavior_index;
}

static void encounter_post_combat(
	long encounter_index)
{
	short primary_postcombat_behaviors[NUMBER_OF_PRIMARY_POST_COMBAT_BEHAVIORS];
	struct post_combat_possibility primary_possibilities[NUMBER_OF_PRIMARY_POST_COMBAT_BEHAVIORS];
	struct post_combat_possibility possibilities[NUMBER_OF_POST_COMBAT_BEHAVIOR_TYPES][NUMBER_OF_POST_COMBAT_POSSIBILITIES];
	short secondary_reply_type;
	long secondary_reply_actor_index;
	long secondary_reply_target_actor_index;
	long secondary_reply_target_prop_index;
	boolean any_possibilities;
	short behavior_index;
	short possibility_index;
	short primary_behavior_index;
	struct encounter_datum *encounter = encounter_get(encounter_index);

	memset(primary_postcombat_behaviors, NONE, sizeof(primary_postcombat_behaviors));
	secondary_reply_type = NONE;
	secondary_reply_actor_index = NONE;
	secondary_reply_target_actor_index = NONE;

	for (behavior_index = 0; behavior_index < NUMBER_OF_POST_COMBAT_BEHAVIOR_TYPES; behavior_index++)
	{
		for (possibility_index = 0; possibility_index < NUMBER_OF_POST_COMBAT_POSSIBILITIES; possibility_index++)
		{
			possibilities[behavior_index][possibility_index] = global_empty_possibility;
		}
	}

	any_possibilities = FALSE;

	{
		struct encounter_actor_iterator iterator;
		struct actor_datum *actor;

		encounter_actor_iterator_new(&iterator, encounter_index);

		while (actor = encounter_actor_iterator_next(&iterator))
		{
			struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
			boolean any_enemy_corpses = FALSE;

			if (actor->meta.unit_index != NONE)
			{
				struct prop_iterator prop_iterator;
				struct prop_datum *prop;
				real player_rating = ai_communication_get_player_rating(actor->meta.unit_index, TRUE, NULL, NULL);

				prop_iterator_new(&prop_iterator, iterator.index);

				while (prop = prop_iterator_next(&prop_iterator))
				{
					if (prop->dead)
					{
						short behavior;
						real maximum_distance;
						real bonus;

						if (prop->enemy)
						{
							if (TEST_FLAG(actor_definition->flags2, _actor_definition_flags2_no_corpse_shooting_bit) || prop->dead_ticks >= 7 * TICKS_PER_SECOND)
							{
								behavior = _post_combat_behavior_check_enemy;
								maximum_distance = 5.f;
								bonus = 0.f;
							}
							else
							{
								behavior = _post_combat_behavior_shoot_target;
								maximum_distance = 10.f;
								bonus = 0.7f;
							}
						}
						else
						{
							behavior = _post_combat_behavior_check_friend;
							maximum_distance = 9.f;
							bonus = 0.4f;
						}

						if (maximum_distance > prop->distance)
						{
							real distance_factor = MIN(2.f, maximum_distance / prop->distance);
							real rating_factor = MAX(player_rating, 1.5f);
							real age_factor = MIN(1.f, (real)prop->dead_ticks / (8 * TICKS_PER_SECOND));
							real weight = (2.f - age_factor) * rating_factor * distance_factor + bonus;

							if (prop->player)
							{
								weight += 2.f;
							}

							if (prop->enemy)
							{
								any_enemy_corpses = TRUE;
							}

							if (encounter_post_combat_add_possibility(possibilities[behavior], iterator.index, weight, prop_iterator.index, prop->unit_index))
							{
								any_possibilities = TRUE;
							}
						}
					}
				}

				if (any_enemy_corpses)
				{
					struct unit_datum *unit = unit_get(actor->meta.unit_index);
					real weight = (real)unit->unit.killing_spree_count * 0.7f + player_rating;

					if (encounter_post_combat_add_possibility(possibilities[_post_combat_behavior_celebrate], iterator.index, weight, NONE, NONE))
					{
						any_possibilities = TRUE;
					}
				}
			}
		}
	}

	if (any_possibilities)
	{
		primary_postcombat_behaviors[0] = encounter_post_combat_select_random_behavior(possibilities, &primary_possibilities[0]);

		if (encounter->team_index != _game_team_human || encounter->enemies_defeated >= 8)
		{
			any_possibilities = FALSE;

			for (behavior_index = 0; behavior_index < NUMBER_OF_POST_COMBAT_BEHAVIOR_TYPES; behavior_index++)
			{
				if (behavior_index == primary_postcombat_behaviors[0])
				{
					for (possibility_index = 0; possibility_index < NUMBER_OF_POST_COMBAT_POSSIBILITIES; possibility_index++)
					{
						possibilities[behavior_index][possibility_index] = global_empty_possibility;
					}
				}
				else if (possibilities[behavior_index][0].actor_index == primary_possibilities[0].actor_index ||
					possibilities[behavior_index][0].unit_index == primary_possibilities[0].unit_index)
				{
					for (possibility_index = 0; possibility_index < NUMBER_OF_POST_COMBAT_POSSIBILITIES - 1; possibility_index++)
					{
						possibilities[behavior_index][possibility_index] = possibilities[behavior_index][possibility_index + 1];
					}

					possibilities[behavior_index][NUMBER_OF_POST_COMBAT_POSSIBILITIES - 1] = global_empty_possibility;
				}

				if (possibilities[behavior_index][0].actor_index != NONE)
				{
					any_possibilities = TRUE;
				}
			}

			if (any_possibilities)
			{
				primary_postcombat_behaviors[1] = encounter_post_combat_select_random_behavior(possibilities, &primary_possibilities[1]);
			}
		}
	}

	if (encounter->team_index != _game_team_human || encounter->enemies_defeated >= 4)
	{
		struct encounter_actor_iterator iterator;
		struct actor_datum *actor;
		boolean find_target = FALSE;
		long best_actor_index = NONE;
		real best_rating = 0.f;

		encounter_actor_iterator_new(&iterator, encounter_index);

		while (actor = encounter_actor_iterator_next(&iterator))
		{
			if (actor->meta.unit_index != NONE)
			{
				real player_rating = ai_communication_get_player_rating(actor->meta.unit_index, TRUE, NULL, NULL);

				if (!actor_communication_team(iterator.index) && player_rating > 2.f && player_rating > best_rating)
				{
					best_actor_index = iterator.index;
					best_rating = player_rating;
				}
			}
		}

		secondary_reply_actor_index = best_actor_index;

		if (secondary_reply_actor_index != NONE)
		{
			struct actor_datum *reply_actor = actor_get(secondary_reply_actor_index);

			if (!(reply_actor->input.body_vitality < 0.5f) || !(reply_actor->emotions.original_body_vitality - reply_actor->input.body_vitality > 0.3f))
			{
				if (encounter->current_count == 1 && encounter->prebattle_living_count > 1)
				{
					secondary_reply_type = _actor_postcombat_speak_alone;
				}
				else if (encounter->current_count >= 2 && encounter->prebattle_living_count >= encounter->current_count + MIN(encounter->current_count, 2))
				{
					secondary_reply_type = _actor_postcombat_speak_massacre;
				}
				else if (encounter->current_count >= 2 && encounter->current_count >= encounter->prebattle_living_count - 1)
				{
					secondary_reply_type = _actor_postcombat_speak_triumph;
				}
				else if (reply_actor->input.body_vitality > 0.8f)
				{
					secondary_reply_type = _actor_postcombat_speak_unscathed;
				}
			}
			else
			{
				secondary_reply_type = _actor_postcombat_speak_wounded;
				find_target = TRUE;
			}

			if (find_target)
			{
				struct encounter_actor_iterator reply_iterator;
				struct actor_datum *friend_actor;
				long closest_actor_index = NONE;
				long closest_prop_index = NONE;
				real closest_distance = REAL_MAX;

				encounter_actor_iterator_new(&reply_iterator, encounter_index);

				while (friend_actor = encounter_actor_iterator_next(&reply_iterator))
				{
					if (friend_actor->meta.unit_index != NONE && reply_iterator.index != secondary_reply_actor_index)
					{
						real distance_squared = distance_squared3d(&reply_actor->input.position.head_position, &friend_actor->input.position.head_position);

						if (closest_distance == REAL_MAX || closest_distance * closest_distance > distance_squared)
						{
							long prop_index = prop_get_active_by_unit_index(reply_iterator.index, reply_actor->meta.unit_index);

							if (prop_index != NONE)
							{
								closest_distance = distance_squared;
								closest_actor_index = reply_iterator.index;
								closest_prop_index = prop_index;
							}
						}
					}
				}

				if (closest_actor_index != NONE)
				{
					secondary_reply_target_actor_index = closest_actor_index;
					secondary_reply_target_prop_index = closest_prop_index;
				}
			}
		}
	}

	for (primary_behavior_index = 0; primary_behavior_index < NUMBER_OF_PRIMARY_POST_COMBAT_BEHAVIORS; primary_behavior_index++)
	{
		if (primary_postcombat_behaviors[primary_behavior_index] != NONE && primary_possibilities[primary_behavior_index].actor_index != NONE)
		{
			struct actor_datum *actor = actor_get(primary_possibilities[primary_behavior_index].actor_index);

			match_assert("c:\\halo\\SOURCE\\ai\\encounters.c", 2927, (primary_postcombat_behaviors[primary_behavior_index] >= 0) && (primary_postcombat_behaviors[primary_behavior_index] < NUMBER_OF_POST_COMBAT_BEHAVIOR_TYPES));

			actor->external_orders.postcombat_type = global_post_combat_translation_table[primary_postcombat_behaviors[primary_behavior_index]];
			actor->external_orders.postcombat_prop_index = primary_possibilities[primary_behavior_index].prop_index;
		}
	}

	if (secondary_reply_type != NONE && secondary_reply_actor_index != NONE)
	{
		struct actor_datum *actor = actor_get(secondary_reply_actor_index);

		actor->external_orders.postcombat_type = secondary_reply_type;
		actor->external_orders.postcombat_prop_index = NONE;

		if (secondary_reply_target_actor_index != NONE)
		{
			struct actor_datum *target_actor = actor_get(secondary_reply_target_actor_index);

			target_actor->external_orders.postcombat_type = _actor_postcombat_run_to;
			target_actor->external_orders.postcombat_prop_index = secondary_reply_target_prop_index;
		}
	}

	encounter->post_combat = TRUE;
	encounter->post_combat_delay = FALSE;
	encounter->post_combat_delay_timer = ENCOUNTER_POST_COMBAT_DELAY_TICKS;
	encounter->enemies_defeated = 0;

	return;
}

static void encounter_update_timers(
	long encounter_index)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);

	if (encounter->enemy_visible)
	{
		encounter->enemy_visible_timer = 0;
	}
	else if (encounter->enemy_visible_timer != NONE)
	{
		encounter->enemy_visible_timer += ENCOUNTER_UPDATE_TICKS;
	}

	if (encounter->enemy_alive)
	{
		encounter->enemy_alive_timer = 0;
	}
	else if (encounter->enemy_alive_timer != NONE)
	{
		encounter->enemy_alive_timer += ENCOUNTER_UPDATE_TICKS;
	}

	if (encounter->post_combat && encounter->post_combat_delay)
	{
		if (encounter->post_combat_delay_timer > ENCOUNTER_UPDATE_TICKS)
		{
			encounter->post_combat_delay_timer -= ENCOUNTER_UPDATE_TICKS;
		}
		else
		{
			encounter->post_combat_delay_timer = 0;
		}
	}

	return;
}

static boolean encounter_place_actor(
	long encounter_index,
	short squad_index,
	short variant_number,
	boolean spawn)
{
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
		&encounter_definition->squads,
		squad_index,
		struct squad_definition);
	boolean success = FALSE;
	short starting_location_index = encounter_get_actor_starting_location(encounter_index, squad_index, spawn);

	if (starting_location_index != NONE)
	{
		struct actor_starting_location_definition *starting_location = TAG_BLOCK_GET_ELEMENT(
			&squad_definition->starting_locations,
			starting_location_index,
			struct actor_starting_location_definition);
		short actor_palette_index = squad_definition->actor_palette_index;
		struct scenario *scenario = global_scenario_get();

		if (starting_location->actor_palette_index != NONE)
		{
			actor_palette_index = starting_location->actor_palette_index;
		}

		if (actor_palette_index >= 0 && actor_palette_index < scenario->ai_actor_palette.count)
		{
			struct actor_palette_entry *palette_entry = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_actor_palette,
				actor_palette_index,
				struct actor_palette_entry);

			if (palette_entry->reference.index != NONE)
			{
				struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(palette_entry->reference.index);
				boolean is_major = FALSE;

				if (actor_variant_definition->major_upgrade_reference.index != NONE)
				{
					boolean random_major = FALSE;
					real random_upgrade_chance = 0.f;

					ai_get_major_upgrade_chance(squad_definition->major_upgrade, &is_major, &random_major, &random_upgrade_chance);

					if (random_major)
					{
						is_major = ai_consider_major_upgrade(encounter_index, squad_index, random_upgrade_chance);
					}
				}

				success = actor_place(palette_entry->reference.index, encounter_index, squad_index, starting_location, is_major, variant_number) != NONE;
			}
		}
		else
		{
			error(
				_error_silent,
				"WARNING: cannot spawn actors in %s/%s because the actor variant specified for this squad is NONE or invalid",
				encounter_definition->name,
				squad_definition->name);
		}
	}

	return success;
}

boolean encounter_spawn_actor(
	long encounter_index,
	short squad_index)
{
	if (ai_globals->ai_initialized_for_map)
	{
		if (encounter_place_actor(encounter_index, squad_index, 0, TRUE))
		{
			struct encounter_datum *encounter = encounter_get(encounter_index);
			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
				struct encounter_definition);
			struct squad_datum *squad = encounter_get_squad(encounter, squad_index);
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->squads,
				squad_index,
				struct squad_definition);

			encounter->current_count++;
			squad->current_count++;

			if (squad_definition->respawn_total_count > 0)
			{
				squad->respawn_actors_left--;
			}

			encounter->respawn_delay_ticks = (short)(real_random_range(encounter_definition->respawn_time_lower_bound, encounter_definition->respawn_time_upper_bound) * TICKS_PER_SECOND);
			squad->respawn_delay_ticks = (short)(real_random_range(squad_definition->respawn_time_lower_bound, squad_definition->respawn_time_upper_bound) * TICKS_PER_SECOND);
		}
	}

	return FALSE;
}

void encounter_set_respawn(
	long encounter_index,
	boolean respawn)
{
	if (ai_globals->ai_initialized_for_map)
	{
		encounter_get(encounter_index)->respawn_enabled = respawn;
		encounter_force_activate(encounter_index);
	}

	return;
}

void encounter_set_blind(
	long encounter_index,
	boolean blind)
{
	if (ai_globals->ai_initialized_for_map)
	{
		encounter_get(encounter_index)->blind = blind;
	}

	return;
}

void encounter_set_deaf(
	long encounter_index,
	boolean deaf)
{
	if (ai_globals->ai_initialized_for_map)
	{
		encounter_get(encounter_index)->deaf = deaf;
	}

	return;
}

static void encounter_update_respawn(
	long encounter_index)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);

	if (encounter->respawn_enabled)
	{
		if (encounter->respawn_delay_ticks > ENCOUNTER_UPDATE_TICKS)
		{
			encounter->respawn_delay_ticks -= ENCOUNTER_UPDATE_TICKS;
		}
		else
		{
			struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
				struct encounter_definition);
			unsigned long squad_desires_respawn[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_SQUADS_PER_ENCOUNTER)];
			short desire_count;
			short squad_index;

			encounter->respawn_delay_ticks = 0;
			desire_count = 0;
			memset(squad_desires_respawn, 0, sizeof(squad_desires_respawn));

			for (squad_index = 0; squad_index < encounter_definition->squads.count; squad_index++)
			{
				struct squad_datum *squad = encounter_get_squad(encounter, squad_index);
				struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
					&encounter_definition->squads,
					squad_index,
					struct squad_definition);

				while (squad->respawn_actors_left > 0 && squad->current_count < squad_definition->respawn_min_actors)
				{
					if (ai_debug.print_respawn)
					{
						console_printf(
							FALSE,
							"%s/%s: current %d < min %d -> spawn (%d left)",
							encounter_definition->name,
							squad_definition->name,
							squad->current_count,
							squad_definition->respawn_min_actors,
							squad->respawn_actors_left);
					}

					if (!encounter_spawn_actor(encounter_index, squad_index))
					{
						if (ai_debug.print_respawn)
						{
							console_printf(
								FALSE,
								"%s/%s: unable to spawn, out of starting points",
								encounter_definition->name,
								squad_definition->name);
						}

						break;
					}
				}

				if (squad->respawn_actors_left > 0 && squad->current_count < squad_definition->respawn_max_actors)
				{
					if (squad->respawn_delay_ticks > ENCOUNTER_UPDATE_TICKS)
					{
						squad->respawn_delay_ticks -= ENCOUNTER_UPDATE_TICKS;
					}
					else
					{
						squad->respawn_delay_ticks = 0;
						desire_count++;
						BIT_VECTOR_SET_FLAG(squad_desires_respawn, squad_index, TRUE);
					}

					if (ai_debug.print_respawn)
					{
						console_printf(
							FALSE,
							"%s/%s: current %d < max %d -> desire spawn",
							encounter_definition->name,
							squad_definition->name,
							squad->current_count,
							squad_definition->respawn_max_actors);
					}
				}
			}

			if (desire_count > 0 && !encounter->respawn_delay_ticks)
			{
				short random_index = random_range(0, desire_count);

				for (squad_index = 0; squad_index < encounter->squad_count; squad_index++)
				{
					if (BIT_VECTOR_TEST_FLAG(squad_desires_respawn, squad_index))
					{
						if (random_index > 0)
						{
							random_index--;
						}
						else if (encounter_spawn_actor(encounter_index, squad_index))
						{
							if (ai_debug.print_respawn)
							{
								struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
									&encounter_definition->squads,
									squad_index,
									struct squad_definition);

								console_printf(
									FALSE,
									"%s/%s: randomly selected to spawn",
									encounter_definition->name,
									squad_definition->name);
							}

							break;
						}
						else if (ai_debug.print_respawn)
						{
							struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
								&encounter_definition->squads,
								squad_index,
								struct squad_definition);

							console_printf(
								FALSE,
								"%s/%s: unable to spawn, out of starting points",
								encounter_definition->name,
								squad_definition->name);
						}
					}
				}
			}
		}
	}

	return;
}

void encounter_squad_timer_expire(
	long encounter_index,
	short squad_index)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	struct squad_datum *squad = encounter_get_squad(encounter, squad_index);
	struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
		&encounter_definition->squads,
		squad_index,
		struct squad_definition);

	squad->delay_timer = 0;

	if (TEST_FLAG(squad_definition->flags, _squad_magic_sight_after_timer_bit))
	{
		ai_scripting_magically_see_players(AI_INDEX_NEW(_ai_index_squad, encounter_index, squad_index));
	}

	if (ai_debug.print_rules)
	{
		console_printf(
			FALSE,
			"%s/%s: delay timer finished",
			encounter_definition->name,
			squad_definition->name);
	}

	return;
}

static void encounter_update_squads(
	long encounter_index)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	short squad_index;

	for (squad_index = 0; squad_index < encounter->squad_count; squad_index++)
	{
		struct squad_datum *squad = encounter_get_squad(encounter, squad_index);
		struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
			&encounter_definition->squads,
			squad_index,
			struct squad_definition);

		if (squad->delay_timer > 0 && !TEST_FLAG(squad_definition->flags, _squad_delay_forever_bit))
		{
			if (squad->delay_timer_started)
			{
				if (squad->delay_timer > ENCOUNTER_UPDATE_TICKS)
				{
					squad->delay_timer -= ENCOUNTER_UPDATE_TICKS;
				}
				else
				{
					encounter_squad_timer_expire(encounter_index, squad_index);
				}
			}
			else
			{
				squad->delay_timer_started = TEST_FLAG(squad_definition->flags, _squad_timer_starts_immediately_bit) || encounter->current_in_combat_count > 0;

				if (squad->delay_timer_started && ai_debug.print_rules)
				{
					console_printf(
						FALSE,
						"%s/%s: delay timer started (%.1f sec)",
						encounter_definition->name,
						squad_definition->name,
						squad_definition->squad_delay_timer);
				}
			}
		}
	}

	return;
}

static void encounter_update_platoons(
	long encounter_index)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	short platoon_index;

	for (platoon_index = 0; platoon_index < encounter->platoon_count; platoon_index++)
	{
		struct platoon_datum *platoon = encounter_get_platoon(encounter, platoon_index);

		if (platoon->current_count > 0)
		{
			struct platoon_definition *platoon_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->platoons,
				platoon_index,
				struct platoon_definition);

			if (!platoon->maneuvering)
			{
				platoon->maneuvering = encounter_test_rule(encounter_index, &platoon_definition->maneuvering_rule);

				if (platoon->maneuvering && ai_debug.print_rules)
				{
					console_printf(
						FALSE,
						"%s/%s triggered maneuvering rule",
						encounter_definition->name,
						platoon_definition->name);
				}
			}

			if (platoon->maneuver_disable || !platoon->maneuvering)
			{
				boolean defending = !TEST_FLAG(platoon_definition->flags, _platoon_initially_defending_bit);

				if (platoon->defending != defending && encounter_test_rule(encounter_index, &platoon_definition->attacking_defending_rule))
				{
					platoon->defending = defending;

					if (ai_debug.print_rules)
					{
						console_printf(
							FALSE,
							"%s/%s triggered %s rule",
							encounter_definition->name,
							platoon_definition->name,
							defending ? "defending" : "attacking");
					}
				}
			}
		}
	}

	return;
}

static void encounter_update_follow(
	long encounter_index)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	long target_object_indices[8];
	real_point3d target_object_position;
	long target_object_index = NONE;
	short target_object_count = 0;

	switch (encounter->follow_target_type)
	{
	case _follow_target_players:
	{
		struct data_iterator iterator;
		struct player_datum *player;

		data_iterator_new(&iterator, player_data);

		while (player = data_iterator_next(&iterator))
		{
			if (player->unit_index != NONE && target_object_count < (short)NUMBEROF(target_object_indices))
			{
				target_object_indices[target_object_count++] = player->unit_index;
			}
		}

		break;
	}
	case _follow_target_unit:
		if (object_try_and_get_and_verify_type(encounter->follow_target_unit_index, _object_mask_unit))
		{
			target_object_indices[target_object_count++] = encounter->follow_target_unit_index;
		}
		else
		{
			encounter->follow_target_unit_index = NONE;
		}
		break;
	case _follow_target_ai:
		if (encounter->follow_target_ai_index != NONE)
		{
			struct ai_index_actor_iterator iterator;
			struct actor_datum *actor;

			ai_index_actor_iterator_new(encounter->follow_target_ai_index, &iterator);

			while ((actor = ai_index_actor_iterator_next(&iterator)) && target_object_count < (short)NUMBEROF(target_object_indices))
			{
				target_object_indices[target_object_count++] = actor->meta.unit_index;
			}
		}
		break;
	}

	if (target_object_count > 0)
	{
		unsigned long squad_source_bitvector[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_SQUADS_PER_ENCOUNTER)];
		unsigned long squad_target_bitvector[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_SQUADS_PER_ENCOUNTER)];
		unsigned long squad_occupied_bitvector[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_SQUADS_PER_ENCOUNTER)];
		unsigned long squad_firing_position_masks[MAXIMUM_SQUADS_PER_ENCOUNTER];
		short squad_index;
		unsigned long firing_position_mask = 0;
		short follower_count = 0;

		memset(squad_source_bitvector, 0, sizeof(squad_source_bitvector));
		memset(squad_target_bitvector, 0, sizeof(squad_target_bitvector));
		memset(squad_occupied_bitvector, 0, sizeof(squad_occupied_bitvector));

		for (squad_index = 0; squad_index < encounter->squad_count; squad_index++)
		{
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->squads,
				squad_index,
				struct squad_definition);

			if (TEST_FLAG(squad_definition->flags, _squad_automatic_migration_bit))
			{
				struct squad_datum *squad = encounter_get_squad(encounter, squad_index);

				BIT_VECTOR_SET_FLAG(squad_source_bitvector, squad_index, TRUE);
				follower_count += squad->current_count;

				if (squad->automatic_migration_target)
				{
					short group_index;
					unsigned long squad_firing_position_mask = 0;
					boolean defending = FALSE;

					BIT_VECTOR_SET_FLAG(squad_target_bitvector, squad_index, TRUE);

					if (squad_definition->platoon_index >= 0 && squad_definition->platoon_index < encounter_definition->platoons.count)
					{
						defending = encounter_get_platoon(encounter, squad_definition->platoon_index)->defending;
					}
					else if (squad_definition->platoon_index != NONE)
					{
						error(
							_error_silent,
							"WARNING: squad %s/%s has an invalid platoon - %d is outside range of [0, %d)",
							encounter_definition->name,
							squad_definition->name,
							squad_definition->platoon_index,
							encounter_definition->platoons.count);
					}

					if (defending)
					{
						for (group_index = _firing_position_group_defending; group_index <= _firing_position_group_defending_guard; group_index++)
						{
							squad_firing_position_mask |= squad_definition->firing_position_groups[group_index];
						}
					}
					else
					{
						for (group_index = _firing_position_group_attacking; group_index <= _firing_position_group_attacking_guard; group_index++)
						{
							squad_firing_position_mask |= squad_definition->firing_position_groups[group_index];
						}
					}

					squad_firing_position_masks[squad_index] = squad_firing_position_mask;
					firing_position_mask |= squad_firing_position_mask;

					if (squad->current_count > 0)
					{
						BIT_VECTOR_SET_FLAG(squad_occupied_bitvector, squad_index, TRUE);
					}
				}
			}
		}

		if (follower_count > 0 && firing_position_mask > 0)
		{
			if (target_object_count == 1)
			{
				target_object_index = target_object_indices[0];
				object_get_origin(target_object_index, &target_object_position);
			}
			else
			{
				real_point3d target_object_positions[NUMBEROF(target_object_indices)];
				real target_object_best_dist_sq[NUMBEROF(target_object_indices)];
				struct encounter_actor_iterator iterator;
				struct actor_datum *actor;
				short index;
				real best_dist_sq = REAL_MAX;

				for (index = 0; index < target_object_count; index++)
				{
					target_object_best_dist_sq[index] = REAL_MAX;
					object_get_origin(target_object_indices[index], &target_object_positions[index]);
				}

				encounter_actor_iterator_new(&iterator, encounter_index);

				while (actor = encounter_actor_iterator_next(&iterator))
				{
					if (BIT_VECTOR_TEST_FLAG(squad_source_bitvector, actor->meta.squad_index))
					{
						for (index = 0; index < target_object_count; index++)
						{
							real dist_sq = distance_squared3d(&actor->input.position.body_position, &target_object_positions[index]);

							target_object_best_dist_sq[index] = MIN(target_object_best_dist_sq[index], dist_sq);
						}
					}
				}

				for (index = 0; index < target_object_count; index++)
				{
					if (target_object_best_dist_sq[index] < best_dist_sq)
					{
						target_object_index = target_object_indices[index];
						target_object_position = target_object_positions[index];
					}
				}
			}

			if (target_object_index != NONE)
			{
				real group_best_dist_sq[NUMBER_OF_FIRING_POSITION_GROUP_LETTERS];
				short firing_position_index;
				short group_index;
				short best_squad_index;
				short current_squad_index;
				short squad_index;
				real best_dist_sq;
				real current_dist_sq;

				best_squad_index = NONE;

				for (group_index = 0; group_index < NUMBEROF(group_best_dist_sq); group_index++)
				{
					group_best_dist_sq[group_index] = REAL_MAX;
				}

				for (firing_position_index = 0; firing_position_index < encounter_definition->firing_positions.count; firing_position_index++)
				{
					struct firing_position_definition *firing_position = TAG_BLOCK_GET_ELEMENT(
						&encounter_definition->firing_positions,
						firing_position_index,
						struct firing_position_definition);

					if (TEST_FLAG(firing_position_mask, firing_position->group_index))
					{
						real dist_sq = distance_squared3d(&firing_position->position, &target_object_position);

						group_best_dist_sq[firing_position->group_index] = MIN(group_best_dist_sq[firing_position->group_index], dist_sq);
					}
				}

				best_dist_sq = REAL_MAX;
				current_dist_sq = -REAL_MAX;
				current_squad_index = NONE;

				for (squad_index = 0; squad_index < encounter->squad_count; squad_index++)
				{
					if (BIT_VECTOR_TEST_FLAG(squad_target_bitvector, squad_index))
					{
						real squad_dist_sq = REAL_MAX;

						for (group_index = 0; group_index < NUMBEROF(group_best_dist_sq); group_index++)
						{
							if (TEST_FLAG(squad_firing_position_masks[squad_index], group_index))
							{
								squad_dist_sq = MIN(squad_dist_sq, group_best_dist_sq[group_index]);
							}
						}

						if (squad_dist_sq < best_dist_sq)
						{
							best_squad_index = squad_index;
							best_dist_sq = squad_dist_sq;
						}

						if (BIT_VECTOR_TEST_FLAG(squad_occupied_bitvector, squad_index) && squad_dist_sq > current_dist_sq)
						{
							current_squad_index = squad_index;
							current_dist_sq = squad_dist_sq;
						}
					}
				}

				if (best_squad_index != NONE && current_squad_index != NONE)
				{
					real best_dist = square_root(best_dist_sq);
					real current_dist = square_root(current_dist_sq);
					real tolerance = (encounter->follow_target_distance > 0.f) ? encounter->follow_target_distance : 2.f;
					boolean migrate = best_dist < current_dist - tolerance;

					if (ai_debug.print_automatic_migration)
					{
						struct squad_definition *current_squad_definition = (current_squad_index == NONE) ? NULL : TAG_BLOCK_GET_ELEMENT(
							&encounter_definition->squads,
							current_squad_index,
							struct squad_definition);
						struct squad_definition *best_squad_definition = (best_squad_index == NONE) ? NULL : TAG_BLOCK_GET_ELEMENT(
							&encounter_definition->squads,
							best_squad_index,
							struct squad_definition);

						console_printf(
							FALSE,
							"%s: current %s/%.1f best %s/%.1f tol %.1f -> %s",
							encounter_definition->name,
							!current_squad_definition ? "<none>" : current_squad_definition->name,
							(current_dist_sq == -REAL_MAX) ? -1000.f : current_dist,
							best_squad_definition ? best_squad_definition->name : "<none>",
							(best_dist_sq == REAL_MAX) ? 1000.f : best_dist,
							tolerance,
							migrate ? "migrate" : "stay");
					}

					if (!migrate)
					{
						best_squad_index = NONE;
					}
				}

				if (best_squad_index != NONE)
				{
					struct encounter_actor_iterator iterator;
					struct actor_datum *actor;

					encounter_actor_iterator_new(&iterator, encounter_index);

					while (actor = encounter_actor_iterator_next(&iterator))
					{
						if (BIT_VECTOR_TEST_FLAG(squad_source_bitvector, actor->meta.squad_index) && actor->meta.squad_index != best_squad_index)
						{
							actor_change_encounter(iterator.index, encounter_index, best_squad_index);
						}
					}
				}
			}
		}
	}

	return;
}

static void encounter_control_actors(
	long encounter_index)
{
	struct encounter_datum *encounter = encounter_get(encounter_index);
	struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->ai_encounters,
		DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
		struct encounter_definition);
	struct encounter_actor_iterator iterator;
	struct actor_datum *actor;

	encounter_actor_iterator_new(&iterator, encounter_index);

	while (actor = encounter_actor_iterator_next(&iterator))
	{
		boolean defending = FALSE;
		boolean maneuvering = FALSE;

		actor->external_orders.stand_down = encounter->stand_down;
		actor->external_orders.playfighting = encounter->playfighting;

		if (!encounter->post_combat)
		{
			actor->external_orders.postcombat_type = _actor_postcombat_none;
			actor->external_orders.postcombat_prop_index = NONE;
		}

		if (actor->meta.platoon_index != NONE)
		{
			struct platoon_datum *platoon = encounter_get_platoon(encounter, actor->meta.platoon_index);

			defending = platoon->defending;
			maneuvering = platoon->maneuvering && !platoon->maneuver_disable;
		}

		actor->external_orders.defending = defending;

		if (maneuvering)
		{
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->squads,
				actor->meta.squad_index,
				struct squad_definition);
			struct platoon_definition *platoon_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->platoons,
				actor->meta.platoon_index,
				struct platoon_definition);

			if (squad_definition->maneuver_squad_index >= 0 && squad_definition->maneuver_squad_index < encounter_definition->squads.count)
			{
				actor_change_encounter(iterator.index, encounter_index, squad_definition->maneuver_squad_index);
				actor_stimulus_maneuvering(
					iterator.index,
					TEST_FLAG(platoon_definition->flags, _platoon_advancing_maneuver_bit),
					TEST_FLAG(platoon_definition->flags, _platoon_flee_upon_maneuver_bit));
			}
		}
	}

	encounters_update_dirty_status();

	return;
}

static boolean encounter_test_rule(
	long encounter_index,
	struct platoon_rule *rule)
{
	short original_count;
	short current_count;
	real strength;
	struct encounter_datum *encounter = encounter_get(encounter_index);
	boolean result = FALSE;

	if (rule->platoon_index >= 0 && rule->platoon_index < encounter->platoon_count)
	{
		struct platoon_datum *platoon = encounter_get_platoon(encounter, rule->platoon_index);

		original_count = platoon->original_count;
		current_count = platoon->current_count;
		strength = platoon->current_strength_fraction;
	}
	else
	{
		original_count = encounter->original_count;
		current_count = encounter->current_count;
		strength = encounter->current_strength_fraction;
	}

	if (original_count > 0)
	{
		switch (rule->rule_type)
		{
		case _platoon_rule_never:
			result = FALSE;
			break;
		case _platoon_rule_75_strength:
			result = strength < 0.75f;
			break;
		case _platoon_rule_50_strength:
			result = strength < 0.5f;
			break;
		case _platoon_rule_25_strength:
			result = strength < 0.25f;
			break;
		case _platoon_rule_anybody_dead:
			result = current_count < original_count;
			break;
		case _platoon_rule_25_dead:
			result = 4 * current_count / 3 <= original_count;
			break;
		case _platoon_rule_50_dead:
			result = 2 * current_count <= original_count;
			break;
		case _platoon_rule_75_dead:
			result = 4 * current_count <= original_count;
			break;
		case _platoon_rule_all_but_one_dead:
			result = current_count <= 1;
			break;
		case _platoon_rule_all_dead:
			result = current_count == 0;
			break;
		default:
			result = FALSE;
			break;
		}
	}

	if (ai_debug.print_rule_values && result)
	{
		switch (rule->rule_type)
		{
		case _platoon_rule_75_strength:
			console_printf(FALSE, "strength %.2f < 75%%", strength);
			break;
		case _platoon_rule_50_strength:
			console_printf(FALSE, "strength %.2f < 50%%", strength);
			break;
		case _platoon_rule_25_strength:
			console_printf(FALSE, "strength %.2f < 25%%", strength);
			break;
		case _platoon_rule_anybody_dead:
			console_printf(FALSE, "survivors %d < total %d", current_count, original_count);
			break;
		case _platoon_rule_25_dead:
			console_printf(FALSE, "survivors %d <= 25%% of total %d", current_count, original_count);
			break;
		case _platoon_rule_50_dead:
			console_printf(FALSE, "survivors %d <= 50%% of total %d", current_count, original_count);
			break;
		case _platoon_rule_75_dead:
			console_printf(FALSE, "survivors %d <= 75%% of total %d", current_count, original_count);
			break;
		case _platoon_rule_all_but_one_dead:
			console_printf(FALSE, "survivors %d <= 1", current_count);
			break;
		case _platoon_rule_all_dead:
			console_printf(FALSE, "survivors %d = 0", current_count);
			break;
		}
	}

	return result;
}
