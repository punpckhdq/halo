/*
ACTOR_STIMULUS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "console.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void actor_stimulus_combat(long actor_index, short transition_type, real_point3d const *guard_point, long guard_point_surface_index, real guard_distance, long guard_timer, real_vector3d const *transition_vector, long prop_index, long prop_look_timer, boolean prop_look_while_moving);

/* ---------- globals */

/* ---------- public code */

void actor_stimulus_clear(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	memset(&actor->stimuli, 0, sizeof(actor->stimuli));

	return;
}

static void actor_stimulus_combat(
	long actor_index,
	short transition_type,
	real_point3d const *guard_point,
	long guard_point_surface_index,
	real guard_distance,
	long guard_timer,
	real_vector3d const *transition_vector,
	long prop_index,
	long prop_look_timer,
	boolean prop_look_while_moving)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->state.mode < _actor_mode_combat && transition_type >= actor->stimuli.combat_transition)
	{
		actor->stimuli.combat_transition = transition_type;
		if (!guard_point)
		{
			actor->stimuli.combat_transition_guard_at_point = FALSE;
		}
		else
		{
			actor->stimuli.combat_transition_guard_at_point = TRUE;
			actor->stimuli.combat_transition_guard_point = *guard_point;
			actor->stimuli.combat_transition_guard_point_surface_index = guard_point_surface_index;
			actor->stimuli.combat_transition_guard_point_distance = guard_distance;
		}

		if (!transition_vector)
		{
			actor->stimuli.combat_transition_has_vector = FALSE;
		}
		else
		{
			actor->stimuli.combat_transition_has_vector = TRUE;
			actor->stimuli.combat_transition_vector = *transition_vector;
		}

		actor->stimuli.combat_transition_prop_index = prop_index;
		actor->stimuli.combat_transition_prop_look_timer = prop_look_timer;
		actor->stimuli.combat_transition_prop_look_while_moving = prop_look_while_moving;
		actor->stimuli.combat_transition_guard_timer = guard_timer;
	}

	return;
}

void actor_stimulus_surprise(
	long actor_index,
	short surprise_level,
	long prop_index,
	real_vector3d const *surprise_vector)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (surprise_level > actor->stimuli.surprise_level)
	{
		actor->stimuli.surprise_level = surprise_level;
		actor->stimuli.surprise_prop_index = prop_index;
		if (!surprise_vector)
		{
			actor->stimuli.surprise_has_vector = FALSE;
		}
		else
		{
			actor->stimuli.surprise_has_vector = TRUE;
			actor->stimuli.surprise_vector = *surprise_vector;
		}
	}

	return;
}

void actor_stimulus_suspicion(
	long actor_index,
	short suspicion_combat_status,
	long suspicion_timer)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->stimuli.suspicion_combat_status < suspicion_combat_status)
	{
		actor->stimuli.suspicion_combat_status = suspicion_combat_status;
		actor->stimuli.suspicion_timer = suspicion_timer;
	}
	else if (actor->stimuli.suspicion_combat_status == suspicion_combat_status)
	{
		actor->stimuli.suspicion_timer = MAX(actor->stimuli.suspicion_timer, suspicion_timer);
	}

	return;
}

void actor_stimulus_prop_sighted(
	long actor_index,
	long prop_index,
	boolean new_enemy)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *prop = prop_get(prop_index);

	if (!prop->dead && actor->meta.unit_index != NONE && prop->enemy)
	{
		ai_communication_event(
			new_enemy ? _ai_communication_sighted_enemy : _ai_communication_found_enemy,
			actor->meta.unit_index,
			prop->unit_index,
			_comm_hostility_enemy,
			NONE,
			NONE,
			NULL);
	}

	return;
}

void actor_stimulus_enter_combat_found_body(
	long actor_index,
	long prop_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *prop = prop_get(prop_index);

	actor->emotions.corpse_ignore_time = game_time_get();
	actor_perception_find_prop_pathfinding_location(actor_index, prop_index);
	actor_stimulus_combat(
		actor_index,
		_actor_stimulus_combat_body,
		&prop->pathfinding_point,
		prop->pathfinding_surface_index,
		1.5f,
		3 * TICKS_PER_SECOND,
		NULL,
		prop_index,
		3 * TICKS_PER_SECOND,
		TRUE);

	return;
}

void actor_stimulus_enter_combat_perceived_enemy(
	long actor_index,
	long prop_index)
{
	struct prop_datum *prop = prop_get(prop_index);

	actor_stimulus_combat(
		actor_index,
		_actor_stimulus_combat_enemy,
		NULL,
		NONE,
		0.f,
		3 * TICKS_PER_SECOND,
		&prop->actor_to_prop,
		prop_index,
		5 * TICKS_PER_SECOND,
		FALSE);

	return;
}

void actor_stimulus_enter_combat_friend_in_combat(
	long actor_index,
	long prop_index)
{
	struct prop_datum *prop = prop_get(prop_index);

	actor_stimulus_combat(
		actor_index,
		_actor_stimulus_combat_friend,
		NULL,
		NONE,
		0.f,
		3 * TICKS_PER_SECOND,
		&prop->actor_to_prop,
		prop_index,
		5 * TICKS_PER_SECOND,
		FALSE);

	if (prop->actor_index != NONE)
	{
		struct actor_datum *friend_actor = actor_get(prop->actor_index);

		if (friend_actor->state.suspicion_combat_status > _actor_combat_status_none)
		{
			actor_stimulus_suspicion(actor_index, friend_actor->state.suspicion_combat_status, 15 * TICKS_PER_SECOND);
		}
	}

	return;
}

void actor_stimulus_bumped(
	long actor_index,
	long prop_index)
{
	struct direction_specification direction;

	direction.type = _direction_specification_prop;
	direction.prop_index = prop_index;
	actor_look_secondary(actor_index, _secondary_look_bumped_prop, _secondary_look_priority_default, &direction);

	return;
}

void actor_stimulus_environmental_noise(
	long actor_index,
	long object_index,
	real_point3d const *position,
	short count)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->state.mode != _actor_mode_asleep)
	{
		struct direction_specification direction;

		direction.type = _direction_specification_point;
		direction.point = *position;
		actor_look_secondary(actor_index, _secondary_look_environmental_noise, _secondary_look_priority_default, &direction);
	}

	return;
}

void actor_stimulus_heard_shooting(
	long actor_index,
	long prop_index)
{
	struct prop_datum *prop = prop_get(prop_index);

	if (prop->enemy)
	{
		actor_stimulus_enter_combat_perceived_enemy(actor_index, prop_index);
	}
	else
	{
		struct unit_datum *unit = unit_get(prop->unit_index);

		actor_stimulus_enter_combat_friend_in_combat(actor_index, prop_index);
		if (unit->unit.player_index != NONE)
		{
			struct player_datum *player = player_get(unit->unit.player_index);

			if (player->aim_assist_unit_index != NONE && player->aim_assist_timestamp + 3 * TICKS_PER_SECOND >= game_time_get())
			{
				struct actor_datum *actor = actor_get(actor_index);
				struct unit_datum *target_unit = unit_get(player->aim_assist_unit_index);

				if (game_team_is_enemy(actor->meta.team_index, target_unit->object.owner_team_index))
				{
					actor_perception_create_orphan_from_friend(actor_index, player->aim_assist_unit_index, NONE, NONE);
				}
			}
		}
		else if (unit->unit.actor_index != NONE)
		{
			struct actor_datum *friend_actor = actor_get(unit->unit.actor_index);

			if (friend_actor->state.combat_status >= _actor_combat_status_certain)
			{
				actor_derive_target_information(actor_index, unit->unit.actor_index);
			}
		}
	}

	{
		struct direction_specification direction;

		direction.type = _direction_specification_prop;
		direction.prop_index = prop_index;
		actor_look_secondary(actor_index, _secondary_look_shooting_prop, _secondary_look_priority_default, &direction);
	}

	return;
}

void actor_stimulus_was_surprised(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->stimuli.was_surprised = TRUE;

	return;
}

void actor_stimulus_maneuvering(
	long actor_index,
	boolean advancing,
	boolean flee)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.unit_index != NONE)
	{
		ai_communication_event(
			advancing ? _ai_communication_advance : _ai_communication_retreat,
			actor->meta.unit_index,
			NONE,
			NONE,
			NONE,
			NONE,
			NULL);
	}

	if (flee && actor->stimuli.panic_type < _actor_panic_platoon_retreating)
	{
		actor->stimuli.panic_type = _actor_panic_platoon_retreating;
		actor->stimuli.panic_prop_index = NONE;
	}

	return;
}

void actor_stimulus_vehicle_eviction(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->stimuli.vehicle_eviction = TRUE;

	return;
}

void actor_stimulus_abandon_stationary_facing(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	if (actor->emotions.defensive_crouch && actor_definition->defensive.change_facing_stand_time > 0.f)
	{
		actor->emotions.defensive_crouch = FALSE;
		actor->emotions.defensive_crouch_timer = (short)(actor_definition->defensive.change_facing_stand_time * TICKS_PER_SECOND);
		if (actor->target.target_prop_index != NONE)
		{
			struct prop_datum *target_prop = prop_get(actor->target.target_prop_index);

			if (target_prop->distance < 4.f)
			{
				actor->emotions.perceived_danger = MAX(actor->emotions.perceived_danger, 1.8f);
			}
		}
	}

	return;
}

void actor_stimulus_prop_acknowledged(
	long actor_index,
	long prop_index,
	boolean reappearance,
	boolean expected)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct prop_datum *prop = prop_get(prop_index);

	if (!prop->dead)
	{
		{
			struct direction_specification direction;

			direction.type = _direction_specification_prop;
			direction.prop_index = prop_index;
			actor_look_secondary(actor_index, _secondary_look_newly_acknowledged_prop, _secondary_look_priority_default, &direction);
		}

		if (prop->enemy)
		{
			short surprise_level = _actor_surprise_none;
			boolean flanked = dot_product3d(&prop->actor_to_prop, &actor->input.facing_vector) < 0.5f;

			if (actor->state.combat_status == _actor_combat_status_none)
			{
				expected = FALSE;
				if (actor->state.mode < _actor_mode_combat)
				{
					if (prop->shooting)
					{
						surprise_level = MAX(surprise_level, _actor_surprise_unprepared_enemy_shooting);
					}

					if (prop->distance < actor_definition->panic.surprise_distance)
					{
						surprise_level = MAX(surprise_level, _actor_surprise_unprepared_enemy_close);
					}
				}
			}
			else if (actor->state.combat_status >= _actor_combat_status_clear_los && !flanked)
			{
				expected = TRUE;
			}

			if (!expected && prop->shooting && prop->distance < actor_definition->panic.surprise_distance)
			{
				if (flanked)
				{
					surprise_level = MAX(surprise_level, _actor_surprise_unexpected_enemy_close_flanked_shooting);
				}
				else
				{
					surprise_level = MAX(surprise_level, _actor_surprise_unexpected_enemy_close_shooting);
				}
			}

			if (ai_debug.print_surprise && prop->player)
			{
				char const *surprise_names[NUMBER_OF_ACTOR_SURPRISE_TYPES] =
				{
					"none",
					"enemy-shoot",
					"impact",
					"enemy-close",
					"grenade",
					"damage",
					"unexp-close-shoot",
					"unexp-behind-shoot"
				};

				console_printf(
					FALSE,
					"%s %d: surprise %s: %s %sexp %s %sshoot %s (%.1f%c%.1f)",
					actor_type_get_name(actor->meta.type),
					DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index),
					surprise_names[surprise_level],
					actor->state.combat_status == _actor_combat_status_none ? "noncom" : "combat",
					expected ? "" : "un",
					flanked ? "flank" : "front",
					prop->shooting ? "" : "not",
					prop->distance < actor_definition->panic.surprise_distance ? "close" : "far",
					prop->distance,
					prop->distance < actor_definition->panic.surprise_distance ? '<' : '>',
					actor_definition->panic.surprise_distance);
			}

			if (surprise_level > _actor_surprise_none)
			{
				actor_stimulus_surprise(actor_index, surprise_level, prop_index, &prop->actor_to_prop);
			}

			if (actor->state.combat_status < _actor_combat_status_definite &&
				!expected &&
				prop->visibility < 2 &&
				actor->meta.unit_index != NONE)
			{
				ai_communication_event(
					_ai_communication_unexpected_enemy,
					actor->meta.unit_index,
					prop->unit_index,
					_comm_hostility_enemy,
					NONE,
					NONE,
					NULL);
			}
		}
	}

	if (prop->player && prop->enemy && !prop->dead && actor->meta.type != _actor_mounted_weapon && cheat.medusa)
	{
		actor_kill(actor_index, FALSE, TRUE);
	}

	return;
}

void actor_stimulus_damage(
	long actor_index,
	long prop_index,
	real damage_fraction,
	real_vector3d *damage_velocity)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *prop = NULL;
	real_vector3d const *surprise_vector = NULL;
	boolean has_damage_direction = FALSE;
	real_vector3d damage_direction;

	if (prop_index != NONE)
	{
		prop = prop_get(prop_index);
		match_assert("c:\\halo\\SOURCE\\ai\\actor_stimulus.c", 340, prop_acknowledged(prop));
	}

	if (prop)
	{
		surprise_vector = &prop->actor_to_prop;
	}
	else if (damage_velocity)
	{
		real magnitude_squared = magnitude_squared3d(damage_velocity);

		if (magnitude_squared > 0.25f)
		{
			scale_vector3d(damage_velocity, -1.f / square_root(magnitude_squared), &damage_direction);
			surprise_vector = &damage_direction;
			has_damage_direction = TRUE;
		}
	}

	actor->stimuli.was_damaged = TRUE;
	if ((!prop || prop->enemy) && actor->state.mode < _actor_mode_combat)
	{
		actor_stimulus_surprise(actor_index, _actor_surprise_unprepared_damage, prop_index, surprise_vector);
		actor_stimulus_combat(
			actor_index,
			_actor_stimulus_combat_damage,
			NULL,
			NONE,
			0.f,
			3 * TICKS_PER_SECOND,
			surprise_vector,
			prop_index,
			5 * TICKS_PER_SECOND,
			FALSE);
	}

	{
		struct direction_specification direction;
		boolean look = TRUE;

		if (prop_index != NONE)
		{
			direction.type = _direction_specification_prop;
			direction.prop_index = prop_index;
		}
		else if (has_damage_direction)
		{
			direction.type = _direction_specification_vector;
			direction.vector = damage_direction;
		}
		else
		{
			look = FALSE;
		}

		if (look)
		{
			actor_look_secondary(actor_index, _secondary_look_damage, _secondary_look_priority_default, &direction);
		}
	}

	return;
}

void actor_stimulus_weapon_impact(
	long actor_index,
	long object_index,
	real_point3d const *position,
	short count)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	if (actor->danger_zone.danger_type > _actor_danger_zone_none &&
		actor->danger_zone.object_index == object_index &&
		actor->danger_zone.acknowledgement_timer > 0)
	{
		ai_communication_event(_ai_communication_grenade_startle, actor->meta.unit_index, NONE, NONE, NONE, NONE, NULL);
	}
	else
	{
		real_vector3d impact_vector;
		real distance;

		vector_from_points3d(&actor->input.position.head_position, position, &impact_vector);
		distance = normalize3d(&impact_vector);
		if (realcmp(distance, 0.f))
		{
			impact_vector = actor->input.facing_vector;
		}

		if (actor->state.mode < _actor_mode_combat && distance < actor_definition->panic.surprise_distance)
		{
			actor_stimulus_surprise(actor_index, _actor_surprise_unprepared_weapon_impact_close, NONE, &impact_vector);
		}

		actor_stimulus_combat(
			actor_index,
			_actor_stimulus_combat_impact,
			NULL,
			NONE,
			0.f,
			3 * TICKS_PER_SECOND,
			&impact_vector,
			NONE,
			0,
			FALSE);
	}

	{
		struct direction_specification direction;

		direction.type = _direction_specification_point;
		direction.point = *position;
		actor_look_secondary(actor_index, _secondary_look_weapon_impact, _secondary_look_priority_default, &direction);
	}

	return;
}

void actor_stimulus_weapon_detonation(
	long actor_index,
	long object_index,
	real_point3d const *position,
	short count)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	{
		real_vector3d impact_vector;
		real distance;

		vector_from_points3d(&actor->input.position.head_position, position, &impact_vector);
		distance = normalize3d(&impact_vector);
		if (realcmp(distance, 0.f))
		{
			impact_vector = actor->input.facing_vector;
		}

		if (actor->state.mode < _actor_mode_combat && distance < actor_definition->panic.surprise_distance)
		{
			actor_stimulus_surprise(actor_index, _actor_surprise_unprepared_grenade, NONE, &impact_vector);
		}

		actor_stimulus_combat(
			actor_index,
			_actor_stimulus_combat_impact,
			NULL,
			NONE,
			0.f,
			3 * TICKS_PER_SECOND,
			&impact_vector,
			NONE,
			0,
			FALSE);
	}

	if (object_index != NONE)
	{
		if (game_team_is_enemy(actor->meta.team_index, object_get(object_index)->object.owner_team_index))
		{
			actor_stimulus_suspicion(actor_index, _actor_combat_status_investigate, 30 * TICKS_PER_SECOND);
		}
	}

	{
		struct direction_specification direction;

		direction.type = _direction_specification_point;
		direction.point = *position;
		actor_look_secondary(actor_index, _secondary_look_detonation, _secondary_look_priority_default, &direction);
	}

	return;
}

void actor_stimulus_prop_just_killed(
	long actor_index,
	long prop_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *prop = prop_get(prop_index);

	actor->state.been_in_combat = TRUE;
	if (!prop->enemy)
	{
		struct actor_datum *actor = actor_get(actor_index);
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
		long killer_prop_index = actor_perception_find_killer_prop_index(actor_index, prop_index, TRUE);

		if (prop->type == actor_definition->panic.panic_leader_type &&
			actor->stimuli.panic_type < _actor_panic_friend_leader_type_killed &&
			real_random() < actor_definition->panic.panic_chance_leader_type_killed)
		{
			actor->stimuli.panic_type = _actor_panic_friend_leader_type_killed;
			actor->stimuli.panic_prop_index = killer_prop_index;
		}

		if (prop->distance < 8.f && killer_prop_index != NONE)
		{
			struct prop_datum *killer_prop = prop_get(killer_prop_index);

			if (killer_prop->enemy)
			{
				if (killer_prop->visibility > 0 && killer_prop->quantized_facing <= 2)
				{
					real panic_chance = actor_definition->panic.panic_chance_friend_killed;
					boolean panic = FALSE;

					if (TEST_FLAG(actor_definition->flags2, _actor_definition_flags2_panic_in_groups_bit) &&
						game_time_get() > actor->emotions.flee_with_friends_disable_time)
					{
						panic = actor_emotion_flee_with_friends(actor_index, &panic_chance);
					}

					if (panic || real_random() < panic_chance)
					{
						if (actor->stimuli.panic_type < _actor_panic_friend_same_type_killed)
						{
							actor->stimuli.panic_type = _actor_panic_friend_same_type_killed;
							actor->stimuli.panic_prop_index = killer_prop_index;
						}
					}
				}

				if (killer_prop->unopposable_enemy)
				{
					killer_prop->unopposable_casualties_inflicted++;
					killer_prop->unopposable_casualty_decay_timer = 25 * TICKS_PER_SECOND;
				}
			}
		}
	}

	return;
}

void actor_stimulus_prop_fleeing(
	long actor_index,
	long prop_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct prop_datum *prop = prop_get(prop_index);

	if (!prop->enemy &&
		TEST_FLAG(actor_definition->flags2, _actor_definition_flags2_panic_in_groups_bit) &&
		game_time_get() > actor->emotions.flee_with_friends_disable_time)
	{
		real panic_chance = actor_definition->panic.panic_chance_friend_killed;

		if (actor_emotion_flee_with_friends(actor_index, &panic_chance) || real_random() < panic_chance)
		{
			if (actor->stimuli.panic_type < _actor_panic_friend_same_type_killed)
			{
				long panic_prop_index = actor->target.target_prop_index;

				if (prop->actor_index != NONE)
				{
					struct actor_datum *friend_actor = actor_get(prop->actor_index);

					if (friend_actor->state.action == _actor_action_flee && friend_actor->state.action_data.flee.flee_prop_index != NONE)
					{
						struct prop_datum *flee_prop = prop_get(friend_actor->state.action_data.flee.flee_prop_index);

						panic_prop_index = prop_get_active_by_unit_index(actor_index, flee_prop->unit_index);
					}

					actor->stimuli.panic_type = _actor_panic_friend_fleeing;
					actor->stimuli.panic_prop_index = panic_prop_index;
				}
			}
		}
	}

	return;
}

void actor_stimulus_noticed_danger_zone(
	long actor_index,
	short danger_type,
	short danger_hostility,
	long danger_object_index,
	real_point3d const *position)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	if (actor->meta.unit_index != NONE)
	{
		real_vector3d danger_vector;
		real distance;

		if (danger_type == _actor_danger_zone_projectile)
		{
			short hostility = NONE;

			switch (danger_hostility)
			{
			case _actor_danger_hostility_enemy:
				hostility = _comm_hostility_enemy;
				break;
			case _actor_danger_hostility_friend:
				hostility = _comm_hostility_friend;
				break;
			case _actor_danger_hostility_self:
				hostility = _comm_hostility_self;
				break;
			}

			ai_communication_event(_ai_communication_grenade_sighted, actor->meta.unit_index, NONE, hostility, NONE, NONE, NULL);
		}

		vector_from_points3d(&actor->input.position.head_position, position, &danger_vector);
		distance = normalize3d(&danger_vector);
		if (actor->state.mode < _actor_mode_combat &&
			distance < actor_definition->panic.surprise_distance &&
			danger_type == _actor_danger_zone_projectile)
		{
			actor_stimulus_surprise(actor_index, _actor_surprise_unprepared_grenade, NONE, &danger_vector);
		}

		actor_stimulus_combat(
			actor_index,
			_actor_stimulus_combat_danger,
			NULL,
			NONE,
			0.f,
			0,
			&danger_vector,
			NONE,
			0,
			FALSE);
	}

	return;
}

/* ---------- private code */
