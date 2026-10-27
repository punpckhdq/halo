/*
PROPS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "props.h"
#include "actors.h"
#include "actor_types.h"
#include "units.h"
#include "unit_definitions.h"
#include "game_state.h"

/* ---------- prototypes */

static void prop_add(long actor_index, long unit_index, long prop_index);
static long prop_new_blank(long actor_index);
static void prop_setup_orphan(long actor_index, long orphan_prop_index, long acknowledged_prop_index);
static void prop_remove(long actor_index, long prop_index);

/* ---------- globals */

struct data_array *prop_data;

/* ---------- public code */

void props_initialize(
	void)
{
	prop_data = game_state_data_new("prop", MAXIMUM_NUMBER_OF_PROPS, sizeof(struct prop_datum));
	match_assert("c:\\halo\\SOURCE\\ai\\props.c", 36, prop_data);

	return;
}

void props_dispose(
	void)
{
	return;
}

void props_initialize_for_new_map(
	void)
{
	data_make_valid(prop_data);

	return;
}

void props_dispose_from_old_map(
	void)
{
	data_make_invalid(prop_data);

	return;
}

long prop_new_unacknowledged(
	long actor_index,
	long unit_index,
	boolean enemy)
{
	struct prop_iterator iterator;
	struct prop_datum *prop;
	long prop_index = NONE;
	long closest_undesired_prop_index = NONE;
	long closest_desired_prop_index = NONE;
	real closest_undesired_distance = REAL_MAX;
	real closest_desired_distance = REAL_MAX;
	short desired_count = 0;

	prop_iterator_new(&iterator, actor_index);

	while (prop = prop_iterator_next(&iterator))
	{
		if ((prop->state < _prop_state_uninspected_orphan || prop->state > _prop_state_inspected_orphan) && prop->orphan_prop_index == NONE)
		{
			boolean optional = FALSE;

			if (!actor_perception_desire_prop(
				actor_index,
				NONE,
				prop->unit_index,
				prop->actor_index,
				prop->in_use,
				prop->player,
				prop->enemy,
				prop->dead,
				prop->dead_ticks,
				prop->suicide_radius,
				prop->distance * prop->distance,
				prop->required_ticks,
				&optional))
			{
				if (prop->distance < closest_undesired_distance)
				{
					closest_undesired_prop_index = iterator.index;
					closest_undesired_distance = prop->distance;
				}
			}
			else if (prop->enemy == enemy)
			{
				desired_count++;

				if (optional && prop->distance < closest_desired_distance)
				{
					closest_desired_prop_index = iterator.index;
					closest_desired_distance = prop->distance;
				}
			}
		}
	}

	if (closest_undesired_prop_index == NONE)
	{
		if (closest_desired_prop_index != NONE)
		{
			short minimum_desired_count = enemy ? 6 : 4;

			if (desired_count >= minimum_desired_count)
			{
				prop_index = closest_desired_prop_index;
			}
		}
	}
	else
	{
		prop_index = closest_undesired_prop_index;
	}

	if (prop_index == NONE)
	{
		prop_index = datum_new(prop_data);
	}
	else
	{
		short identifier;

		prop = prop_get(prop_index);
		match_assert("c:\\halo\\SOURCE\\ai\\props.c", 158, prop->orphan_prop_index == NONE);
		match_assert("c:\\halo\\SOURCE\\ai\\props.c", 159, prop->parent_prop_index == NONE);
		actor_switch_props(actor_index, prop_index, NONE);
		prop_remove(actor_index, prop_index);
		identifier = prop->identifier;
		memset(prop, 0, sizeof(*prop));
		prop->identifier = identifier;
	}

	prop_add(actor_index, unit_index, prop_index);

	return prop_index;
}

static void prop_add(
	long actor_index,
	long unit_index,
	long prop_index)
{
	if (prop_index == NONE)
	{
		static long last_warning_time = NONE;
		long current_time = game_time_get();

		if (last_warning_time == NONE || current_time >= last_warning_time + 30 * TICKS_PER_SECOND)
		{
			error(_error_silent, "AI knowledge database (%d entries) is full (warns once every 30 sec)", MAXIMUM_NUMBER_OF_PROPS);
			last_warning_time = current_time;
		}
	}
	else
	{
		struct actor_datum *actor = actor_get(actor_index);
		struct prop_datum *prop = prop_get(prop_index);

		prop->owner_actor_index = actor_index;
		prop->unit_effect = NONE;
		prop->ticks_since_damage = NONE;
		prop->unit_index = unit_index;
		prop->currently_damaging_me = FALSE;
		prop->damage_inflicted_on_me = 0.f;
		prop->ticks_since_definitely_located = NONE;
		prop->definitely_located = FALSE;
		prop->definite_knowledge_source_actor = NONE;
		prop->last_perceived_time = NONE;
		prop->last_visible_time = NONE;
		prop->orphan_corpse_cheated = FALSE;
		prop->actor_index = NONE;
		prop->orphan_prop_index = NONE;
		prop->required_ticks = 0;
		prop->last_unreachable_time = NONE;

		if (unit_index != NONE)
		{
			struct unit_datum *prop_unit = unit_get(unit_index);
			struct unit_definition *unit_definition = unit_definition_get(prop_unit->definition_index);

			match_assert("c:\\halo\\SOURCE\\ai\\props.c", 232, unit_index != actor->meta.unit_index);
			match_assert("c:\\halo\\SOURCE\\ai\\props.c", 233, prop_unit->object.type == _object_type_biped);
			prop->team_index = prop_unit->object.owner_team_index;
			prop->enemy = game_team_is_enemy(actor->meta.team_index, prop->team_index);
			prop->ally = game_team_is_ally(actor->meta.team_index, prop->team_index);
			prop->ally_status_changed = game_team_ally_status_changed(actor->meta.team_index, prop->team_index);
			prop->dead = TEST_FLAG(prop_unit->object.damage_flags, _object_dead_bit);
			prop->suicide_radius = unit_definition->unit.ai_danger_radius;
			prop->really_dead = prop->dead && !prop_unit->unit.feign_death_timer;
			prop->dead_ticks = prop->dead ? 1000 : 0;
			prop->player = prop_unit->object.owner_player_index != NONE;

			if (prop_unit->unit.swarm_actor_index != NONE)
			{
				prop->swarm = TRUE;
				prop->actor_index = prop_unit->unit.swarm_actor_index;
				prop->swarm_unit_selected_time = game_time_get();
			}
			else
			{
				prop->actor_index = prop_unit->unit.actor_index;
			}

			if (prop->player)
			{
				prop->type = _actor_player;
			}
			else if (prop->actor_index != NONE)
			{
				struct actor_datum *prop_actor = actor_get(prop->actor_index);

				prop->type = prop_actor->meta.type;
			}
			else
			{
				prop->type = NONE;
			}
		}

		prop->next_prop_index = actor->meta.first_prop_index;
		actor->meta.first_prop_index = prop_index;
	}

	return;
}

static long prop_new_blank(
	long actor_index)
{
	long prop_index = datum_new(prop_data);

	prop_add(actor_index, NONE, prop_index);

	return prop_index;
}

static void prop_setup_orphan(
	long actor_index,
	long orphan_prop_index,
	long acknowledged_prop_index)
{
	struct prop_datum *acknowledged_prop = prop_get(acknowledged_prop_index);
	struct prop_datum *orphan_prop = prop_get(orphan_prop_index);
	short identifier = orphan_prop->identifier;
	long owner_actor_index = orphan_prop->owner_actor_index;
	long next_prop_index = orphan_prop->next_prop_index;
	long parent_prop_index = orphan_prop->parent_prop_index;

	memcpy(orphan_prop, acknowledged_prop, sizeof(*orphan_prop));
	orphan_prop->identifier = identifier;
	orphan_prop->owner_actor_index = owner_actor_index;
	orphan_prop->next_prop_index = next_prop_index;
	orphan_prop->parent_prop_index = parent_prop_index;
	orphan_prop->state = _prop_state_uninspected_orphan;
	orphan_prop->orphan_lifespan_ticks = 30 * TICKS_PER_SECOND;
	orphan_prop->orphan_inspection_ticks = 0;
	orphan_prop->tried_to_uncover = FALSE;
	orphan_prop->tried_to_search = FALSE;
	orphan_prop->abandoned_search = FALSE;
	vector_from_points3d(&orphan_prop->last_perceived_body_position, &orphan_prop->body_position, &orphan_prop->orphan_hint_vector);
	orphan_prop->velocity = *global_zero_vector3d;
	orphan_prop->quantized_speed = 0;

	return;
}

long prop_orphan_transition(
	long actor_index,
	long parent_prop_index)
{
	long orphan_prop_index = prop_new_blank(actor_index);

	if (orphan_prop_index != NONE)
	{
		struct prop_datum *parent_prop = prop_get(parent_prop_index);
		struct prop_datum *orphan_prop = prop_get(orphan_prop_index);

		match_assert("c:\\halo\\SOURCE\\ai\\props.c", 341, parent_prop->owner_actor_index == actor_index);
		match_assert("c:\\halo\\SOURCE\\ai\\props.c", 342, parent_prop->orphan_prop_index == NONE);
		prop_setup_orphan(actor_index, orphan_prop_index, parent_prop_index);
		parent_prop->orphan_prop_index = orphan_prop_index;
		orphan_prop->parent_prop_index = parent_prop_index;
	}

	return orphan_prop_index;
}

long prop_orphan_from_friend(
	long actor_index,
	long unacknowledged_prop_index,
	long friend_acknowledged_prop_index)
{
	long orphan_prop_index = prop_new_blank(actor_index);

	if (orphan_prop_index != NONE)
	{
		struct prop_datum *parent_prop = prop_get(unacknowledged_prop_index);
		struct prop_datum *orphan_prop = prop_get(orphan_prop_index);
		struct prop_datum *friend_prop = prop_get(friend_acknowledged_prop_index);

		match_assert("c:\\halo\\SOURCE\\ai\\props.c", 365, parent_prop->owner_actor_index == actor_index);
		match_assert("c:\\halo\\SOURCE\\ai\\props.c", 366, parent_prop->orphan_prop_index == NONE);
		prop_setup_orphan(actor_index, orphan_prop_index, friend_acknowledged_prop_index);
		parent_prop->orphan_prop_index = orphan_prop_index;
		orphan_prop->parent_prop_index = unacknowledged_prop_index;

		if (friend_prop->state >= _prop_state_uninspected_orphan && friend_prop->state <= _prop_state_inspected_orphan)
		{
			orphan_prop->state = friend_prop->state;
		}
	}

	return orphan_prop_index;
}

void prop_orphan_update_information(
	long actor_index,
	long orphan_prop_index,
	long friend_acknowledged_prop_index)
{
	prop_setup_orphan(actor_index, orphan_prop_index, friend_acknowledged_prop_index);

	return;
}

void prop_delete(
	long actor_index,
	long prop_index)
{
	prop_remove(actor_index, prop_index);
	datum_delete(prop_data, prop_index);

	return;
}

static void prop_remove(
	long actor_index,
	long prop_index)
{
	long *prop_index_reference;
	struct prop_datum *prop;
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\props.c", 411, actor->target.target_prop_index != prop_index);
	match_assert("c:\\halo\\SOURCE\\ai\\props.c", 414, !((actor->control.secondary_look_type != _secondary_look_none) && (actor->control.secondary_look_direction.type == _direction_specification_prop) && (actor->control.secondary_look_direction.prop_index == prop_index)));
	match_assert("c:\\halo\\SOURCE\\ai\\props.c", 417, !((actor->control.idle_major_active) && (actor->control.idle_major_direction.type == _direction_specification_prop) && (actor->control.idle_major_direction.prop_index == prop_index)));
	match_assert("c:\\halo\\SOURCE\\ai\\props.c", 420, !((actor->control.idle_minor_active) && (actor->control.idle_minor_direction.type == _direction_specification_prop) && (actor->control.idle_minor_direction.prop_index == prop_index)));

	for (prop_index_reference = &actor->meta.first_prop_index; ; prop_index_reference = &prop->next_prop_index)
	{
		prop = prop_get(*prop_index_reference);

		if (*prop_index_reference == prop_index)
		{
			break;
		}
	}

	*prop_index_reference = prop->next_prop_index;

	return;
}

long prop_get_active_by_unit_index(
	long actor_index,
	long unit_index)
{
	struct prop_iterator iterator;
	struct prop_datum *prop;
	struct unit_datum *unit = unit_get(unit_index);
	long unit_actor_index = unit->unit.swarm_actor_index != NONE ? unit->unit.swarm_actor_index : unit->unit.actor_index;
	long prop_index = NONE;

	prop_iterator_new(&iterator, actor_index);

	while (prop = prop_iterator_next(&iterator))
	{
		if ((prop->state < _prop_state_unacknowledged || prop->state > _prop_state_becoming_acknowledged) &&
			(prop->unit_index == unit_index || (prop->swarm && prop->actor_index != NONE && prop->actor_index == unit_actor_index)))
		{
			prop_index = iterator.index;
			break;
		}
	}

	return prop_index;
}

long prop_get_base_by_unit_index(
	long actor_index,
	long unit_index,
	boolean acquire,
	boolean refresh_upon_acquisition)
{
	long prop_index = NONE;

	if (unit_index != NONE)
	{
		struct actor_datum *actor = actor_get(actor_index);
		struct unit_datum *unit = unit_get(unit_index);
		long unit_actor_index = unit->unit.swarm_actor_index != NONE ? unit->unit.swarm_actor_index : unit->unit.actor_index;

		if (unit->object.type == _object_type_biped && unit_actor_index != actor_index)
		{
			struct prop_iterator iterator;
			struct prop_datum *prop;

			prop_iterator_new(&iterator, actor_index);

			while (prop = prop_iterator_next(&iterator))
			{
				if (prop->unit_index == unit_index || (prop->swarm && prop->actor_index != NONE && prop->actor_index == unit_actor_index))
				{
					prop_index = prop->orphan_prop_index == NONE ? iterator.index : prop->orphan_prop_index;
					break;
				}
			}

			if (prop_index == NONE && acquire && actor->meta.active)
			{
				boolean enemy = game_team_is_enemy(actor->meta.team_index, unit->object.owner_team_index);

				prop_index = prop_new_unacknowledged(actor_index, unit_index, enemy);

				if (prop_index != NONE)
				{
					struct actor_position_data sense_position;
					struct prop_datum *new_prop = prop_get(prop_index);

					prop_position_refresh(actor_index, prop_index, &sense_position, FALSE, refresh_upon_acquisition);
					new_prop->required_ticks = TICKS_PER_SECOND;
					new_prop->delay_requirement_decision = TRUE;

					if (refresh_upon_acquisition)
					{
						prop_status_refresh(actor_index, prop_index, &sense_position);

						if (new_prop->perception >= _actor_perception_full)
						{
							boolean expected = actor_expected_acknowledgement(actor_index, prop_index);

							new_prop->state = _prop_state_acknowledged;
							actor_perception_acknowledge(actor_index, prop_index, FALSE, expected);
						}
					}
				}
			}
		}
	}

	return prop_index;
}

void prop_iterator_new(
	struct prop_iterator *iterator,
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	iterator->next_index = actor->meta.first_prop_index;

	return;
}

struct prop_datum *prop_iterator_next(
	struct prop_iterator *iterator)
{
	struct prop_datum *prop = NULL;

	iterator->index = iterator->next_index;

	if (iterator->index != NONE)
	{
		prop = prop_get(iterator->index);
		iterator->next_index = prop->next_prop_index;
	}

	return prop;
}
