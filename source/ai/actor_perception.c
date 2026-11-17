/*
ACTOR_PERCEPTION.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "ai_profile.h"
#include "ai_script.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "network_game_manager.h"
#include "network_messages.h"
#include "vehicles.h"
#include "meter_definitions.h"
#include "weapon_interface_definitions.h"
#include "weapons.h"
#include "bipeds.h"
#include "projectiles.h"
#include "projectile_definitions.h"

/* ---------- constants */

enum
{
	MAXIMUM_OPTIONAL_PROPS_PER_ACTOR = 128, /* fake name */
	MAXIMUM_UNOPPOSABLE_ENEMIES_PER_ACTOR = 16, /* fake name */
};

/* ---------- structures */

struct actor_optional_prop
{
	long unit_index;
	long prop_index;
	real distance_squared;
};

struct actor_unopposable_enemy
{
	short unopposable_type;
	long prop_index;
	long unit_index;
	struct prop_datum *prop;
	short friends_retreating;
	real closest_retreating_friend_dist_sq;
	long closest_retreating_friend_actor_index;
};

struct actor_prop_environment
{
	short existing_count;
	short optional_count;
	struct actor_optional_prop optional_props[MAXIMUM_OPTIONAL_PROPS_PER_ACTOR];
};

/* ---------- prototypes */

static long actor_perception_unit_from_swarm(long actor_index, struct actor_position_data *sense_position, long swarm_actor_index, long existing_unit_index, boolean mark_units);
static int __cdecl actor_perception_qsort_compare_optional_props(const void *p1, const void *p2);
static void actor_perception_refresh(long actor_index);
static void actor_perception_refresh_test_object(long actor_index, long object_index, struct actor_prop_environment *enemies, struct actor_prop_environment *friends);
static boolean actor_perception_assess_vehicle_danger(long actor_index, long vehicle_index, struct actor_position_data *sense_position, boolean currently_perceived);
static boolean actor_perception_assess_suicide_danger(long actor_index, long unit_index, real suicide_radius, real current_distance, boolean enemy, boolean currently_perceived);
static void actor_perception_refresh_danger_zone(long actor_index);
static short actor_emotion_get_unopposable_enemy(long actor_index, long unit_index, short *enemy_count, short maximum_enemy_count, struct actor_unopposable_enemy *enemies);
static short actor_emotion_assess_unopposable_danger(long actor_index, long prop_index);
static void actor_emotion_unopposable_retreat(long actor_index);

/* ---------- globals */

const short global_combat_status_table[NUMBER_OF_ACTOR_TARGET_TYPES] =
{
	_actor_combat_status_none,			// _actor_target_none
	_actor_combat_status_none,			// _actor_target_partial_enemy
	_actor_combat_status_none,			// _actor_target_dead_enemy
	_actor_combat_status_wary,			// _actor_target_disregarded_orphan
	_actor_combat_status_investigate,	// _actor_target_inspected_orphan
	_actor_combat_status_investigate,	// _actor_target_uninspected_orphan
	_actor_combat_status_definite,		// _actor_target_definite_orphan
	_actor_combat_status_certain,		// _actor_target_acknowledged_enemy
	_actor_combat_status_clear_los,		// _actor_target_clear_line_of_sight_enemy
	_actor_combat_status_clear_los,		// _actor_target_potentially_dangerous_enemy
	_actor_combat_status_visible,		// _actor_target_visible_enemy
	_actor_combat_status_visible,		// _actor_target_damaging_enemy
};

const short global_acknowledgement_speeds[NUMBER_OF_ACTOR_KNOWLEDGE_TYPES][NUMBER_OF_ACTOR_PERCEPTION_TYPES] =
{
	{ _actor_acknowledgement_never, _actor_acknowledgement_never, _actor_acknowledgement_non_combat, _actor_acknowledgement_combat },	// _actor_knowledge_noncombat
	{ _actor_acknowledgement_never, _actor_acknowledgement_non_combat, _actor_acknowledgement_guard, _actor_acknowledgement_combat },	// _actor_knowledge_guard
	{ _actor_acknowledgement_never, _actor_acknowledgement_guard, _actor_acknowledgement_combat, _actor_acknowledgement_instant },		// _actor_knowledge_searching
	{ _actor_acknowledgement_never, _actor_acknowledgement_combat, _actor_acknowledgement_instant, _actor_acknowledgement_instant },		// _actor_knowledge_definite
};

/* ---------- public code */

void actor_perception_update(
	long actor_index)
{
	struct prop_iterator iterator;
	struct prop_datum *prop;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	short highest_prop_timer = 1;
	boolean refreshed_status = FALSE;
	long interesting_orphan_index = NONE;
	real interesting_orphan_distance = REAL_MAX;

	if (!actor->meta.dormant)
	{
		if (actor->meta.timeslice)
		{
			actor_perception_refresh(actor_index);
		}

		actor_perception_refresh_danger_zone(actor_index);

		if (actor->danger_zone.danger_type>_actor_danger_zone_none)
		{
			boolean notice = FALSE;

			if (actor->danger_zone.attached_to_us || actor->danger_zone.hostility!=_actor_danger_hostility_enemy)
			{
				notice = actor->danger_zone.acknowledgement_timer>0;
				actor->danger_zone.noticed_danger = TRUE;
				actor->danger_zone.acknowledgement_timer = 0;
			}
			else if (actor->danger_zone.acknowledgement_timer>0 && actor->danger_zone.currently_perceived)
			{
				if (actor->danger_zone.hostility!=_actor_danger_hostility_enemy ||
					actor->danger_zone.attached_to_us ||
					(actor->state.uncertain_combat_timer!=NONE && actor->state.uncertain_combat_timer<2 * TICKS_PER_SECOND))
				{
					actor->danger_zone.acknowledgement_timer = 0;
					notice = TRUE;
				}
				else
				{
					notice = --actor->danger_zone.acknowledgement_timer==0;
				}
			}

			if (notice)
			{
				real notice_chance = 0.f;

				switch (actor->danger_zone.danger_type)
				{
					case _actor_danger_zone_suicide:
						actor->danger_zone.noticed_danger = TRUE;
						break;

					case _actor_danger_zone_projectile:
						notice_chance = actor_definition->perception.notice_projectile_chance;
						break;

					case _actor_danger_zone_vehicle:
						notice_chance = actor_definition->perception.notice_vehicle_chance;
						break;
				}

				if (notice_chance>0.f && real_random()<notice_chance)
				{
					actor->danger_zone.noticed_danger = TRUE;
				}

				if (actor->danger_zone.noticed_danger)
				{
					if (actor->danger_zone.attached_to_us)
					{
						actor->danger_zone.allow_dive_evasion = FALSE;
					}
					else if (actor->danger_zone.hostility!=_actor_danger_hostility_enemy ||
						actor->danger_zone.danger_type==_actor_danger_zone_vehicle ||
						actor->danger_zone.danger_type==_actor_danger_zone_suicide)
					{
						actor->danger_zone.allow_dive_evasion = TRUE;
					}
					else
					{
						actor->danger_zone.allow_dive_evasion = real_random()<actor_definition->moving.grenade_dive_chance;
					}

					actor_stimulus_noticed_danger_zone(
						actor_index,
						actor->danger_zone.danger_type,
						actor->danger_zone.hostility,
						actor->danger_zone.object_index,
						&actor->danger_zone.position);
				}
			}

			if (!actor->danger_zone.acknowledgement_timer)
			{
				if (actor->control.secondary_look_type==_secondary_look_dangerous_object)
				{
					actor->control.secondary_look_priority = MIN(actor->control.secondary_look_priority, _secondary_look_priority_turn_and_aim);
				}

				if (actor->danger_zone.attached_to_us)
				{
					actor->danger_zone.noticed_danger = TRUE;
					actor->danger_zone.allow_dive_evasion = FALSE;
				}
			}
		}
	}

	prop_iterator_new(&iterator, actor_index);

	while (prop = prop_iterator_next(&iterator))
	{
		struct prop_datum *parent_prop;
		boolean communicate;
		short new_state = NONE;
		boolean delete_orphan = FALSE;
		boolean refresh_position = FALSE;
		boolean refresh_status = FALSE;
		boolean became_acknowledged = FALSE;
		boolean became_acknowledged_from_orphan = FALSE;

		if (prop->unit_effect_decay_ticks>0 && !--prop->unit_effect_decay_ticks)
		{
			prop->unit_effect = NONE;
		}

		if (prop->ticks_since_damage!=NONE && ++prop->ticks_since_damage>=45)
		{
			prop->currently_damaging_me = FALSE;
		}

		if (prop->ticks_since_definitely_located!=NONE && ++prop->ticks_since_definitely_located>=2 * TICKS_PER_SECOND)
		{
			prop->definitely_located = FALSE;
			prop->definite_knowledge_source_actor = NONE;
		}

		if (prop->dead)
		{
			prop->dead_ticks++;
		}
		else
		{
			prop->dead_ticks = 0;
		}

		if (prop->ticks_until_orphan>0)
		{
			prop->ticks_until_orphan--;
		}

		if (prop->required_ticks>0 && !prop->delay_requirement_decision)
		{
			prop->required_ticks--;
		}

		if (prop->unreachable_ticks>0 && prop->unreachable_ticks<SHRT_MAX)
		{
			prop->unreachable_ticks++;
		}

		if (prop->unopposable_casualty_decay_timer>0 && !--prop->unopposable_casualty_decay_timer)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 316, prop->unopposable_casualties_inflicted > 0);

			if (--prop->unopposable_casualties_inflicted>0)
			{
				prop->unopposable_casualty_decay_timer = 25 * TICKS_PER_SECOND;
			}
		}

		if (prop->visibility>=_actor_perception_full)
		{
			if (prop->visible_ticks<SHRT_MAX)
			{
				prop->visible_ticks++;
			}
		}
		else
		{
			prop->visible_ticks = 0;
		}

		if (actor->meta.dormant)
		{
			prop->in_use = FALSE;
			prop->timer = 0;
		}
		else
		{
			struct actor_position_data sense_position;
			short prop_timer = ++prop->timer;

			if (!prop->enemy)
			{
				prop_timer >>= 3;
			}

			if (prop->quantized_distance>=_prop_distance_far)
			{
				prop_timer >>= 1;
			}

			if (!refreshed_status && prop_timer>=actor->meta.highest_prop_timer)
			{
				refresh_status = TRUE;
				refresh_position = TRUE;
				prop_timer = 0;
				prop->timer = 0;
				refreshed_status = TRUE;
			}

			if (prop_timer>highest_prop_timer)
			{
				highest_prop_timer = prop_timer;
			}

			if (!prop_unacknowledged(prop) || prop->orphan_prop_index==NONE)
			{
				if (actor->meta.swarm)
				{
					prop->in_use = FALSE;
				}
				else
				{
					prop->in_use = actor->target.target_prop_index==iterator.index ||
						actor->meta.interesting_orphan_index==iterator.index ||
						actor->emotions.unopposable_retreat_prop_index==iterator.index ||
						actor->external_orders.pursuit_group_prop_index==iterator.index ||
						(actor->control.secondary_look_type!=_secondary_look_none &&
							actor->control.secondary_look_direction.type==_direction_specification_prop &&
							actor->control.secondary_look_direction.prop_index==iterator.index) ||
						(actor->control.idle_major_active &&
							actor->control.idle_major_direction.type==_direction_specification_prop &&
							actor->control.idle_major_direction.prop_index==iterator.index) ||
						(actor->control.idle_minor_active &&
							actor->control.idle_minor_direction.type==_direction_specification_prop &&
							actor->control.idle_minor_direction.prop_index==iterator.index);

					if (prop_orphaned(prop))
					{
						parent_prop = prop_get(prop->parent_prop_index);
						match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 402, parent_prop->orphan_prop_index == iterator.index);
						parent_prop->in_use = prop->in_use;
					}
				}
			}

			if (prop->in_use && !prop_unacknowledged(prop))
			{
				refresh_position = TRUE;
			}

			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 416, !refresh_status || refresh_position);

			if (refresh_position)
			{
				prop_position_refresh(actor_index, iterator.index, &sense_position, FALSE, refresh_status);
			}

			if (refresh_status)
			{
				prop_status_refresh(actor_index, iterator.index, &sense_position);
			}
		}

		switch (prop->state)
		{
			case _prop_state_unacknowledged:
				if (prop->perception>_actor_perception_none)
				{
					new_state = _prop_state_becoming_acknowledged;
					prop->awareness = 0.f;

#ifdef DEBUG
					if (prop->player && ai_debug.print_acknowledgement)
					{
						char aibuf[256];

						ai_debug_describe_actor(actor_index, actor->meta.unit_index, NONE, aibuf, sizeof(aibuf));
						error(_error_silent, "%s: start to become aware", aibuf);
					}
#endif
				}
				else
				{
					break;
				}

			case _prop_state_becoming_acknowledged:
			{
#ifdef DEBUG
				struct actor_debug_info *actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
#endif

				if (!prop->perception)
				{
					prop->awareness = 0.f;
					new_state = _prop_state_unacknowledged;

					if (prop->player)
					{
#ifdef DEBUG
						actor_debug_info->num_debug_evaluations = NONE;

						if (ai_debug.print_acknowledgement)
						{
							char aibuf[256];

							ai_debug_describe_actor(actor_index, actor->meta.unit_index, NONE, aibuf, sizeof(aibuf));
							error(_error_silent, "%s: stop becoming aware", aibuf);
						}
#endif
					}
				}
				else
				{
					short acknowledgement_speed;
					real awareness_delta;
					short knowledge_type = actor_get_perception_knowledge(actor_index, iterator.index);

					match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 489, (knowledge_type >= 0) && (knowledge_type < NUMBER_OF_ACTOR_KNOWLEDGE_TYPES));
					match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 490, (prop->perception >= 0) && (prop->perception < NUMBER_OF_ACTOR_PERCEPTION_TYPES));

					acknowledgement_speed = global_acknowledgement_speeds[knowledge_type][prop->perception];

					switch (acknowledgement_speed)
					{
						case _actor_acknowledgement_never:
							awareness_delta = 0.f;
							break;

						case _actor_acknowledgement_non_combat:
							awareness_delta = actor_definition->perception.runtime_awareness_delta_non_combat;
							break;

						case _actor_acknowledgement_guard:
							awareness_delta = actor_definition->perception.runtime_awareness_delta_guard;
							break;

						case _actor_acknowledgement_combat:
							awareness_delta = actor_definition->perception.runtime_awareness_delta_combat;
							break;

						case _actor_acknowledgement_instant:
							awareness_delta = 1.f;
							break;

						default:
							match_unreachable("c:\\halo\\SOURCE\\ai\\actor_perception.c", 516);
							break;
					}

#ifdef DEBUG
					if (prop->player && actor_debug_info->num_debug_evaluations!=acknowledgement_speed)
					{
						actor_debug_info->num_debug_evaluations = acknowledgement_speed;

						if (ai_debug.print_acknowledgement)
						{
							char const *acknowledgement_speed_names[NUMBER_OF_ACTOR_ACKNOWLEDGEMENT_SPEEDS] = { "never", "noncombat", "guard", "combat", "instant" };
							char const *knowledge_names[NUMBER_OF_ACTOR_KNOWLEDGE_TYPES] = { "noncombat", "guard", "searching", "definite" };
							char const *perception_names[NUMBER_OF_ACTOR_PERCEPTION_TYPES] = { "none", "partial", "full", "unmistakable" };
							char aibuf[256];

							ai_debug_describe_actor(actor_index, actor->meta.unit_index, NONE, aibuf, sizeof(aibuf));
							error(
								_error_silent,
								"%s: knowledge %s percep %s -> awareness %s",
								aibuf,
								knowledge_names[knowledge_type],
								perception_names[prop->perception],
								acknowledgement_speed_names[acknowledgement_speed]);

							if (awareness_delta>0.f && awareness_delta<1.f)
							{
								error(
									_error_silent,
									"  awareness delta: %.2f (current awareness %.2f -> time %.2fsec)",
									awareness_delta,
									prop->awareness,
									(1.f - prop->awareness) / (awareness_delta * TICKS_PER_SECOND));
							}
						}
					}
#endif

					prop->awareness += awareness_delta;

					if (prop->awareness>=1.f)
					{
						new_state = _prop_state_acknowledged;

						if (prop->player)
						{
#ifdef DEBUG
							actor_debug_info->num_debug_evaluations = NONE;

							if (ai_debug.print_acknowledgement)
							{
								char aibuf[256];

								ai_debug_describe_actor(actor_index, actor->meta.unit_index, NONE, aibuf, sizeof(aibuf));
								error(_error_silent, "%s: become aware!", aibuf);
							}
#endif
						}
					}
				}

				break;
			}

			case _prop_state_becoming_unacknowledged:
				if (prop->perception>_actor_perception_none)
				{
					new_state = _prop_state_acknowledged;
				}
				else if (!prop->ticks_until_orphan ||
					distance_squared2d((real_point2d *)&prop->last_perceived_body_position, (real_point2d *)&prop->body_position)>1.f)
				{
					long orphan_prop_index = NONE;

					if (actor_perception_desire_prop(
						actor_index,
						_prop_state_uninspected_orphan,
						prop->unit_index,
						prop->actor_index,
						prop->in_use,
						prop->player,
						prop->enemy,
						prop->dead,
						prop->dead_ticks,
						prop->suicide_radius,
						((prop->distance) * (prop->distance)),
						prop->required_ticks,
						NULL))
					{
						struct actor_position_data sense_position;

						prop_position_refresh(actor_index, iterator.index, &sense_position, FALSE, FALSE);
						actor_perception_find_prop_pathfinding_location(actor_index, iterator.index);
						orphan_prop_index = prop_orphan_transition(actor_index, iterator.index);
					}

					actor_switch_props(actor_index, iterator.index, orphan_prop_index);
					new_state = _prop_state_unacknowledged;
				}

				break;

			case _prop_state_acknowledged:
				if (!prop->perception)
				{
					if (actor_perception_desire_prop(
						actor_index,
						_prop_state_uninspected_orphan,
						prop->unit_index,
						prop->actor_index,
						prop->in_use,
						prop->player,
						prop->enemy,
						prop->dead,
						prop->dead_ticks,
						prop->suicide_radius,
						((prop->distance) * (prop->distance)),
						prop->required_ticks,
						NULL))
					{
						new_state = _prop_state_becoming_unacknowledged;
					}
					else
					{
						actor_switch_props(actor_index, iterator.index, NONE);
						new_state = _prop_state_unacknowledged;
					}
				}

				break;

			case _prop_state_uninspected_orphan:
			case _prop_state_inspected_orphan:
			{
				short lifespan_decay;

				if (prop->state==_prop_state_uninspected_orphan)
				{
					boolean inspecting = FALSE;
					short inspection_ticks = actor->input.vehicle_gunner_bombardment ? 10 * TICKS_PER_SECOND : 45;

					if (prop->visibility>=_actor_perception_full)
					{
						inspecting = TRUE;
					}
					else if (actor->control.current_fire_target_type==_actor_fire_target_prop &&
						actor->control.current_fire_target_prop_index==iterator.index &&
						!(game_time_get() % 3))
					{
						inspecting = TRUE;
					}

					if (inspecting && ++prop->orphan_inspection_ticks>=inspection_ticks)
					{
						new_state = _prop_state_inspected_orphan;
					}
				}

				if (iterator.index==actor->emotions.unopposable_retreat_prop_index ||
					(actor->state.action==_actor_action_flee && actor->state.action_data.flee.flee_prop_index==iterator.index))
				{
					lifespan_decay = 0;
				}
				else if (iterator.index==actor->target.target_prop_index)
				{
					lifespan_decay = prop->abandoned_search!=FALSE;
				}
				else if (iterator.index==actor->meta.interesting_orphan_index)
				{
					lifespan_decay = actor->state.combat_status>=_actor_combat_status_certain ? 6 : 1;
				}
				else
				{
					lifespan_decay = 10;
				}

				prop->orphan_lifespan_ticks -= lifespan_decay;

				if (prop->orphan_lifespan_ticks<0)
				{
					delete_orphan = TRUE;
				}

				break;
			}
		}

		if (new_state!=NONE)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 694, new_state!=prop->state);

			switch (new_state)
			{
				case _prop_state_unacknowledged:
					prop->definitely_located = FALSE;
					prop->definite_knowledge_source_actor = NONE;
					break;

				case _prop_state_becoming_acknowledged:
					break;

				case _prop_state_becoming_unacknowledged:
					prop->ticks_until_orphan = prop->visibility>=_actor_perception_full ? 2 * TICKS_PER_SECOND : 10;
					break;

				case _prop_state_acknowledged:
					became_acknowledged = actor_perception_become_acknowledged(actor_index, iterator.index, &became_acknowledged_from_orphan);
					iterator.next_index = prop->next_prop_index;
					break;

				case _prop_state_uninspected_orphan:
					match_halt("c:\\halo\\SOURCE\\ai\\actor_perception.c", 721);

				case _prop_state_inspected_orphan:
					prop->definitely_located = FALSE;
					prop->definite_knowledge_source_actor = NONE;
					break;

				default:
					match_halt("c:\\halo\\SOURCE\\ai\\actor_perception.c", 730);
					break;
			}

			prop->state = new_state;
			prop->unopposable_enemy = actor_compute_prop_unopposable(actor_index, iterator.index);
			prop->target_weight = actor_compute_prop_target_weight(actor_index, iterator.index);
		}

		if (delete_orphan)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 746, prop->parent_prop_index != NONE);
			parent_prop = prop_get(prop->parent_prop_index);
			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 751, parent_prop->orphan_prop_index == iterator.index);
			parent_prop->orphan_prop_index = NONE;
			actor_switch_props(actor_index, iterator.index, NONE);
			prop_delete(actor_index, iterator.index);
		}
		else if (prop->refresh_stimuli && prop_acknowledged(prop))
		{
			if (prop->just_killed)
			{
				actor_stimulus_prop_just_killed(actor_index, iterator.index);
				prop->just_killed = FALSE;
			}

			if (prop->just_became_visible || (became_acknowledged && prop->visibility>_actor_perception_none))
			{
				boolean new_enemy = became_acknowledged && !became_acknowledged_from_orphan;

				actor_stimulus_prop_sighted(actor_index, iterator.index, new_enemy);
				prop->just_became_visible = FALSE;
			}

			if (!actor->emotions.sighted_friendly_player &&
				!prop->enemy &&
				prop->player &&
				prop->visibility>=_actor_perception_full &&
				prop->quantized_facing<=_prop_facing_central &&
				prop->distance<7.f)
			{
				actor->emotions.sighted_friendly_player = TRUE;
				ai_communication_event(_ai_communication_sighted_friend_player, actor->meta.unit_index, prop->unit_index, 2, NONE, NONE, NULL);
				actor_stimulus_prop_sighted(actor_index, iterator.index, FALSE);
			}

			if (actor->meta.unit_index!=NONE && !prop->dead && prop->ally && prop->ally_status_changed)
			{
				real communication_distance;
				boolean betrayed = game_team_is_enemy(actor->meta.team_index, prop->team_index);

				if (betrayed)
				{
					communication_distance = 15.f;
				}
				else
				{
					communication_distance = prop->quantized_facing<=_prop_facing_central ? 10.f : 3.f;
				}

				communicate = communication_distance>prop->distance;

				if (betrayed && prop->currently_damaging_me)
				{
					communicate = TRUE;
				}

				if (communicate)
				{
					struct ai_information_data information;

					information.allegiance.team1_index = actor->meta.team_index;
					information.allegiance.team2_index = prop->team_index;
					information.allegiance.broken = betrayed;

					ai_communication_event(8, actor->meta.unit_index, prop->unit_index, betrayed ? 4 : 2, NONE, 1, &information);
				}
			}

			if (actor->state.mode<_actor_mode_combat)
			{
				if (prop->dead && !prop->enemy)
				{
					actor_stimulus_enter_combat_found_body(actor_index, iterator.index);
				}
				else if (prop->enemy)
				{
					actor_stimulus_enter_combat_perceived_enemy(actor_index, iterator.index);
				}
			}

			if (!prop->enemy && !prop->dead && !prop->player)
			{
				communicate = FALSE;

				if (actor->target.target_prop_index!=NONE &&
					(actor->target.since_any_target_visible_timer==NONE || actor->target.since_any_target_visible_timer>=6 * TICKS_PER_SECOND))
				{
					communicate = TRUE;
				}
				else if (actor->meta.encounter_index!=NONE)
				{
					struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

					if (encounter->enemy_visible_timer==NONE || (encounter->enemy_visible_timer>=6 * TICKS_PER_SECOND && encounter->enemy_alive))
					{
						communicate = TRUE;
					}
				}

				if (communicate && actor->meta.unit_index!=NONE)
				{
					if (actor->state.mode<_actor_mode_combat)
					{
						if (prop->in_combat)
						{
							ai_communication_event(15, prop->unit_index, actor->meta.unit_index, 2, NONE, 2, NULL);
						}
					}
					else if (actor_in_combat(actor_index) &&
						!actor_is_fighting(actor_index) &&
						prop->noncombat &&
						prop->visibility>=_actor_perception_full)
					{
						ai_communication_event(15, actor->meta.unit_index, prop->unit_index, 2, NONE, 2, NULL);
					}
				}
			}
		}
		else if (prop_orphaned(prop) && prop->distance<interesting_orphan_distance)
		{
			interesting_orphan_index = iterator.index;
			interesting_orphan_distance = prop->distance;
		}

		if (prop->dead)
		{
			if (prop_acknowledged(prop))
			{
#ifdef DEBUG
				ai_profile.meters[_ai_meter_prop_acknowledged_body].current_count++;
#endif
			}
			else if (prop_orphaned(prop))
			{
#ifdef DEBUG
				ai_profile.meters[_ai_meter_prop_orphaned_body].current_count++;
#endif
			}
			else if (prop_unacknowledged(prop))
			{
#ifdef DEBUG
				ai_profile.meters[_ai_meter_prop_unacknowledged_body].current_count++;
#endif
			}
		}
		else if (prop->enemy)
		{
			if (prop_acknowledged(prop))
			{
#ifdef DEBUG
				ai_profile.meters[_ai_meter_prop_acknowledged_enemy].current_count++;
#endif
			}
			else if (prop_orphaned(prop))
			{
#ifdef DEBUG
				ai_profile.meters[_ai_meter_prop_orphaned_enemy].current_count++;
#endif
			}
			else if (prop_unacknowledged(prop))
			{
#ifdef DEBUG
				ai_profile.meters[_ai_meter_prop_unacknowledged_enemy].current_count++;
#endif
			}
		}
		else
		{
			if (prop_acknowledged(prop))
			{
#ifdef DEBUG
				ai_profile.meters[_ai_meter_prop_acknowledged_friend].current_count++;
#endif
			}
			else if (prop_orphaned(prop))
			{
#ifdef DEBUG
				ai_profile.meters[_ai_meter_prop_orphaned_friend].current_count++;
#endif
			}
			else if (prop_unacknowledged(prop))
			{
#ifdef DEBUG
				ai_profile.meters[_ai_meter_prop_unacknowledged_friend].current_count++;
#endif
			}
		}
	}

	if (actor->target.target_prop_index!=NONE)
	{
		struct prop_datum *target_prop = prop_get(actor->target.target_prop_index);

		if (prop_orphaned(target_prop))
		{
			interesting_orphan_index = NONE;
		}
	}

	if (actor->target.target_type>=_actor_target_definite_orphan)
	{
		actor->target.any_target_ever = TRUE;
	}

	if (actor->target.target_type>=_actor_target_visible_enemy)
	{
		actor->target.since_any_target_visible_timer = 0;
	}
	else if (actor->external_orders.stand_down)
	{
		actor->target.since_any_target_visible_timer = NONE;
	}
	else if (actor->target.since_any_target_visible_timer!=NONE)
	{
		actor->target.since_any_target_visible_timer++;
	}

	actor->meta.highest_prop_timer = highest_prop_timer;
	actor->meta.interesting_orphan_index = interesting_orphan_index;

	return;
}

boolean actor_perception_become_acknowledged(
	long actor_index,
	long prop_index,
	boolean *expected_reference)
{
	struct prop_datum *prop = prop_get(prop_index);
	boolean became_acknowledged = FALSE;
	boolean expected = FALSE;

	if (!prop_acknowledged(prop))
	{
		boolean had_orphan = prop->orphan_prop_index!=NONE;

		expected = actor_expected_acknowledgement(actor_index, prop_index);

		if (had_orphan)
		{
			struct prop_datum *orphan_prop = prop_get(prop->orphan_prop_index);

			prop->target_weight = orphan_prop->target_weight;
			prop->look_interest = orphan_prop->look_interest;
			prop->last_idle_look_interest = orphan_prop->last_idle_look_interest;
			prop->last_idle_look_time = orphan_prop->last_idle_look_time;
			prop->unreachable_ticks = orphan_prop->unreachable_ticks;
			prop->last_unreachable_time = orphan_prop->last_unreachable_time;
			prop->unopposable_enemy = orphan_prop->unopposable_enemy;
			prop->unopposable_casualties_inflicted = orphan_prop->unopposable_casualties_inflicted;
			prop->unopposable_casualty_decay_timer = orphan_prop->unopposable_casualty_decay_timer;
			actor_switch_props(actor_index, prop->orphan_prop_index, prop_index);
			prop_delete(actor_index, prop->orphan_prop_index);
			prop->orphan_prop_index = NONE;
		}

		prop->state = _prop_state_acknowledged;
		actor_perception_acknowledge(actor_index, prop_index, had_orphan, expected);
		became_acknowledged = TRUE;
	}

	if (expected_reference)
	{
		*expected_reference = expected;
	}

	return became_acknowledged;
}

void actor_perception_acknowledge(
	long actor_index,
	long prop_index,
	boolean had_orphan,
	boolean expected)
{
	struct prop_datum *prop = prop_get(prop_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 1037, prop->owner_actor_index == actor_index);
	match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 1038, prop_acknowledged(prop));
	match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 1039, prop->orphan_prop_index == NONE);

	prop->tried_to_search = FALSE;
	prop->tried_to_uncover = FALSE;
	prop->abandoned_search = FALSE;
	prop->refresh_stimuli = TRUE;
	actor_stimulus_prop_acknowledged(actor_index, prop_index, had_orphan, expected);

	return;
}

short actor_visibility_at_point(
	long actor_index,
	struct actor_position_data *sense_position,
	real_point3d const *position,
	char lighting,
	short line_of_sight,
	boolean use_frustum,
	boolean store_debugging_information,
	short combat_knowledge_type)
{
	short visibility = _actor_perception_none;

	if (line_of_sight==_ai_line_of_sight_clear || line_of_sight==_ai_line_of_sight_occluded)
	{
		real_vector3d relative_position;
		real distance_squared;
		real knowledge_factor;
		struct actor_datum *actor = actor_get(actor_index);
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
		struct actor_variant_definition *firing_variant = actor_combat_get_firing_variant_definition(actor_index);
		real maximum_vision_distance = actor_definition->perception.maximum_vision_distance;

		if (firing_variant->ranged_combat.modified_vision_range>0.f)
		{
			maximum_vision_distance = firing_variant->ranged_combat.modified_vision_range;
		}

		knowledge_factor = 1.f;

		switch (combat_knowledge_type)
		{
			case _actor_knowledge_noncombat:
				knowledge_factor = 0.4f;
				break;

			case _actor_knowledge_guard:
				knowledge_factor = 0.6f;
				break;

			case _actor_knowledge_searching:
				knowledge_factor = 0.8f;
				break;

			case _actor_knowledge_definite:
				knowledge_factor = 1.f;
				break;

			default:
				match_unreachable("c:\\halo\\SOURCE\\ai\\actor_perception.c", 1268);
				break;
		}

		maximum_vision_distance *= knowledge_factor;
		vector_from_points3d(&sense_position->head_position, position, &relative_position);
		distance_squared = magnitude_squared3d(&relative_position);

		if (distance_squared<maximum_vision_distance * maximum_vision_distance)
		{
			real fog;
#ifdef DEBUG
			struct actor_debug_info *actor_debug_info;
#endif
			real perception_factor = 1.f;

			if (!TEST_FLAG(actor_definition->flags, _actor_definition_can_see_in_darkness_bit))
			{
				switch (lighting)
				{
					case _prop_lighting_darkness:
						perception_factor *= 0.3f;
						break;

					case _prop_lighting_dim:
						perception_factor *= 0.7f;
						break;
				}
			}

			fog = scenario_fog_at_point(&sense_position->body_location, &sense_position->head_position, position);

			if (fog>0.8f)
			{
				perception_factor = 0.f;
			}
			else if (fog>0.2f)
			{
				perception_factor *= (0.8f - fog) / 0.6f;
			}

			perception_factor = MAX(perception_factor, 0.15f);
#ifdef DEBUG
			actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
#endif

			if (store_debugging_information)
			{
#ifdef DEBUG
				actor_debug_info->vision_last_time = game_time_get();
				actor_debug_info->vision_last_maximum_distance = maximum_vision_distance;
				actor_debug_info->vision_last_perception_factor = perception_factor;
#endif
			}

			if (distance_squared<(maximum_vision_distance * perception_factor) * (maximum_vision_distance * perception_factor))
			{
				real partial_vision_distance;
				real full_vision_distance;

				if (actor->meta.swarm || !use_frustum)
				{
					partial_vision_distance = maximum_vision_distance * perception_factor;
					full_vision_distance = 0.7f * partial_vision_distance;
				}
				else
				{
					real_vector3d head_relative_vector;
					real vertical_angle;
					real planar_distance;

					head_relative_vector.i = dot_product3d(&relative_position, &actor->input.looking_vector);
					head_relative_vector.j = dot_product3d(&relative_position, &actor->input.looking_left_vector);
					head_relative_vector.k = dot_product3d(&relative_position, &actor->input.looking_up_vector);
					planar_distance = magnitude2d((real_vector2d *)&head_relative_vector);
					vertical_angle = arctangent(head_relative_vector.k, planar_distance);

					if (vertical_angle>_pi / 6.f || vertical_angle<-_pi / 4.f)
					{
						full_vision_distance = 0.f;
						partial_vision_distance = 0.f;
					}
					else
					{
						real horizontal_angle = arctangent(head_relative_vector.j, head_relative_vector.i);

						actor_get_vision_distances(
							actor_index,
							maximum_vision_distance,
							perception_factor,
							(real)fabs(horizontal_angle),
							&full_vision_distance,
							&partial_vision_distance);
					}
				}

				if (line_of_sight==_ai_line_of_sight_clear && full_vision_distance * full_vision_distance>distance_squared)
				{
					visibility = distance_squared<6.f * 6.f ? _actor_perception_unmistakable : _actor_perception_full;
				}
				else if (distance_squared<partial_vision_distance * partial_vision_distance)
				{
					visibility = _actor_perception_partial;
				}
			}
		}
	}

	return visibility;
}

short actor_get_perception_knowledge(
	long actor_index,
	long prop_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	short knowledge = NONE;

	if (prop_index!=NONE)
	{
		struct prop_datum *prop = prop_get(prop_index);

		match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 1394, prop->owner_actor_index == actor_index);

		if (prop_acknowledged(prop))
		{
			knowledge = _actor_knowledge_definite;
		}
		else if (prop->unit_effect==_ai_unit_effect_shooting || prop->unit_effect==_ai_unit_effect_death_scream)
		{
			knowledge = _actor_knowledge_definite;
		}
		else if (!prop->enemy && (!prop->dead || actor->state.mode>=_actor_mode_combat))
		{
			knowledge = _actor_knowledge_definite;
		}

		if (knowledge==NONE && prop->orphan_prop_index!=NONE)
		{
			struct prop_datum *orphan_prop = prop_get(prop->orphan_prop_index);

			knowledge = orphan_prop->definitely_located ? _actor_knowledge_definite : _actor_knowledge_searching;
		}
	}

	if (knowledge==NONE)
	{
		if (actor->state.combat_status>=_actor_combat_status_investigate)
		{
			knowledge = _actor_knowledge_searching;
		}
		else if (actor->state.mode<_actor_mode_combat)
		{
			knowledge = _actor_knowledge_noncombat;
		}
		else
		{
			knowledge = _actor_knowledge_guard;
		}
	}

	return knowledge;
}

void actor_get_vision_distances(
	long actor_index,
	real maximum_vision_distance,
	real perception_factor,
	real horizontal_angle,
	real *full_distance_reference,
	real *partial_distance_reference)
{
	real partial_distance;
	real full_distance;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	if (horizontal_angle>actor_definition->perception.peripheral_vision_angle)
	{
		partial_distance = 0.f;
		full_distance = 0.f;
	}
	else
	{
		real central_partial_distance = perception_factor * maximum_vision_distance;
		real peripheral_partial_distance = perception_factor * actor_definition->perception.peripheral_distance;
		real central_full_distance = 0.7f * central_partial_distance;
		real peripheral_full_distance = 0.7f * peripheral_partial_distance;

		peripheral_full_distance = MIN(peripheral_full_distance, 3.5f);

		if (horizontal_angle>actor_definition->perception.maximum_vision_angle)
		{
			partial_distance = peripheral_partial_distance;
			full_distance = peripheral_full_distance;
		}
		else
		{
			real partial_angle = actor_definition->perception.central_vision_angle;
			real full_angle = partial_angle * 0.8f;

			if (horizontal_angle<partial_angle)
			{
				partial_distance = central_partial_distance;
			}
			else
			{
				real fraction = (horizontal_angle - partial_angle) / (actor_definition->perception.maximum_vision_angle - partial_angle);

				partial_distance = (1.f - fraction) * central_partial_distance + peripheral_partial_distance * fraction;
			}

			if (horizontal_angle<full_angle)
			{
				full_distance = central_full_distance;
			}
			else
			{
				real fraction = (horizontal_angle - full_angle) / (actor_definition->perception.maximum_vision_angle - full_angle);

				full_distance = (1.f - fraction) * central_full_distance + peripheral_full_distance * fraction;
			}
		}
	}

	*partial_distance_reference = partial_distance;
	*full_distance_reference = full_distance;

	return;
}

short actor_audibility_at_point(
	long actor_index,
	struct actor_position_data *sense_position,
	real_point3d const *position,
	struct location const *location,
	short sound_volume,
	real perception_factor,
	short line_of_sight)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	short audibility = _actor_perception_none;

	if (sound_volume!=_ai_sound_volume_silent && sense_position->body_location.cluster_index!=NONE && location->cluster_index!=NONE)
	{
		real_vector3d actor_to_position;
		real distance_squared;
#ifdef DEBUG
		struct actor_debug_info *actor_debug_info;
#endif
		real hearing_distance = actor_definition->perception.hearing_distance;
		real propagation_distance = -1.f;
		real final_distance = -1.f;

		vector_from_points3d(&sense_position->head_position, position, &actor_to_position);
		distance_squared = magnitude_squared3d(&actor_to_position);

		if (dot_product3d(&actor_to_position, &sense_position->body_facing)<0.f)
		{
			hearing_distance *= 0.8f;
		}

		if (actor->state.mode==_actor_mode_alert)
		{
			hearing_distance *= 0.7f;
		}
		else if (actor->state.mode==_actor_mode_asleep)
		{
			hearing_distance *= 0.4f;
		}

		if (sound_volume==_ai_sound_volume_quiet)
		{
			hearing_distance *= 0.2f;
		}
		else if (sound_volume==_ai_sound_volume_medium)
		{
			hearing_distance *= 0.45f;
		}
		else if (sound_volume==_ai_sound_volume_shout)
		{
			hearing_distance *= 0.7f;
		}

		if (scenario_location_deafening(&sense_position->body_location) || scenario_location_deafening(location))
		{
			hearing_distance *= 0.25f;
		}

		if (line_of_sight!=_ai_line_of_sight_clear && line_of_sight!=_ai_line_of_sight_occluded)
		{
			hearing_distance *= 0.7f;
		}

		if (hearing_distance * hearing_distance>distance_squared)
		{
			byte encoded_sound_distance = structure_bsp_get_cluster_encoded_sound_distance(
				global_structure_bsp_get(),
				location->cluster_index,
				sense_position->body_location.cluster_index);

			if (!TEST_FLAG(encoded_sound_distance, 7))
			{
				propagation_distance = (real)(encoded_sound_distance & ~FLAG(7)) * 256.f / 127.f;
				final_distance = MAX(propagation_distance / 0.5f, square_root(distance_squared));

				if (final_distance<hearing_distance)
				{
					audibility = sound_volume<_ai_sound_volume_shout ? _actor_perception_full : _actor_perception_unmistakable;
				}
			}
		}

#ifdef DEBUG
		actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
		actor_debug_info->audibility_valid = TRUE;
		actor_debug_info->audibility_result = audibility;
		actor_debug_info->audibility_perception_distance = hearing_distance;
		actor_debug_info->audibility_propagation_distance = propagation_distance;
		actor_debug_info->audibility_straight_distance = square_root(distance_squared);
		actor_debug_info->audibility_final_distance = final_distance;
#endif
	}

	return audibility;
}

void actor_perception_find_sense_position(
	long actor_index,
	real_point3d const *position,
	long prop_index,
	struct actor_position_data *sense_position)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.swarm)
	{
		short index;
		struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
		real best_distance_squared = REAL_MAX;
		long best_unit_index = NONE;

		match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 1637, actor->meta.swarm_unit_count > 0);
		match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 1638, actor->meta.swarm_unit_index != NONE);

		for (index = 0; index<swarm->unit_count; index++)
		{
			struct swarm_component_datum *component = swarm_component_get(swarm->component_indices[index]);
			real distance_squared = distance_squared3d(&component->position, position);

			if (distance_squared<best_distance_squared)
			{
				best_distance_squared = distance_squared;
				best_unit_index = swarm->unit_indices[index];
			}
		}

		match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 1655, best_unit_index != NONE);
		actor_input_sample_position(actor_index, best_unit_index, sense_position);
	}
	else
	{
		*sense_position = actor->input.position;
	}

	return;
}

static long actor_perception_unit_from_swarm(
	long actor_index,
	struct actor_position_data *sense_position,
	long swarm_actor_index,
	long existing_unit_index,
	boolean mark_units)
{
	struct actor_datum *swarm_actor = actor_get(swarm_actor_index);
	long best_unit_index = NONE;

	match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 1677, swarm_actor->meta.swarm);

	if (swarm_actor->meta.swarm_cache_index!=NONE)
	{
		short index;
		struct swarm_datum *swarm = swarm_get(swarm_actor->meta.swarm_cache_index);
		real best_distance_squared = REAL_MAX;

		for (index = 0; index<swarm->unit_count; index++)
		{
			struct swarm_component_datum *component = swarm_component_get(swarm->component_indices[index]);
			real distance_squared = distance_squared3d(&component->position, &sense_position->body_position);

			if (TEST_FLAG(component->flags, _swarm_component_attached_to_unit_bit))
			{
				distance_squared *= 1.5f * 1.5f;
			}
			else if (swarm->unit_indices[index]==existing_unit_index)
			{
				distance_squared *= 0.6f * 0.6f;
			}

			if (distance_squared<best_distance_squared)
			{
				best_distance_squared = distance_squared;
				best_unit_index = swarm->unit_indices[index];
			}

			if (mark_units)
			{
				object_mark_function(swarm->unit_indices[index]);
			}
		}
	}
	else
	{
		long unit_index = swarm_actor->meta.swarm_unit_index;
		real best_distance_squared = REAL_MAX;

		while (unit_index!=NONE)
		{
			real_point3d unit_position;
			real distance_squared;
			struct unit_datum *unit = unit_get(unit_index);

			object_get_origin(unit_index, &unit_position);
			distance_squared = distance_squared3d(&unit_position, &sense_position->body_position);

			if (unit_index==existing_unit_index)
			{
				distance_squared *= 0.6f * 0.6f;
			}

			if (distance_squared<best_distance_squared)
			{
				best_distance_squared = distance_squared;
				best_unit_index = unit_index;
			}

			if (mark_units)
			{
				object_mark_function(unit_index);
			}

			unit_index = unit->unit.swarm_next_unit_index;
		}
	}

	match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 1753, (existing_unit_index == NONE) || (best_unit_index != NONE));

	return best_unit_index;
}

void prop_position_refresh(
	long actor_index,
	long prop_index,
	struct actor_position_data *sense_position,
	boolean forced_position_update,
	boolean full_status_update)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.active)
	{
		struct prop_datum *prop = prop_get(prop_index);
		struct unit_datum *unit = unit_get(prop->unit_index);
		boolean update_position = FALSE;

		if (forced_position_update || !prop_orphaned(prop))
		{
			update_position = TRUE;
		}
		else if (!prop->orphan_corpse_cheated)
		{
			update_position = TEST_FLAG(unit->object.damage_flags, _object_dead_bit) &&
				!unit->unit.feign_death_timer &&
				!prop->perception &&
				magnitude_squared3d(&unit->object.translational_velocity)<0.1f * 0.1f;

			if (actor->target.target_prop_index==prop_index && (!prop->tried_to_uncover || !prop->tried_to_search))
			{
				update_position = FALSE;
			}

			if (update_position)
			{
				prop->orphan_corpse_cheated = TRUE;
				prop->dead = TRUE;
			}
		}

		if (update_position)
		{
			if (prop->swarm && prop->actor_index!=NONE && full_status_update)
			{
				long current_time = game_time_get();

				if (prop->swarm_unit_selected_time + 3 * TICKS_PER_SECOND<=current_time)
				{
					long new_unit_index;

					prop->swarm_unit_selected_time = current_time;
					new_unit_index = actor_perception_unit_from_swarm(actor_index, &actor->input.position, prop->actor_index, prop->unit_index, FALSE);

					if (new_unit_index!=prop->unit_index)
					{
						prop->unit_index = new_unit_index;
						unit = unit_get(prop->unit_index);

						if (prop_orphaned(prop))
						{
							struct prop_datum *parent_prop = prop_get(prop->parent_prop_index);

							parent_prop->unit_index = prop->unit_index;
						}
						else if (prop->orphan_prop_index!=NONE)
						{
							struct prop_datum *orphan_prop = prop_get(prop->orphan_prop_index);

							orphan_prop->unit_index = prop->unit_index;
						}
					}
				}
			}

			unit_get_head_position(prop->unit_index, &prop->head_position);
			object_get_origin(prop->unit_index, &prop->body_position);
			unit_get_center_of_mass(prop->unit_index, &prop->center_of_mass);
			prop->velocity = unit->object.translational_velocity;
			prop->pathfinding_surface_index = NONE;
			prop->body_location = object_get(object_get_ultimate_parent(prop->unit_index))->object.location;
			prop->underwater = scenario_location_underwater(&prop->body_location, &prop->center_of_mass, NULL);
			prop->vehicle_index = NONE;
			prop->vehicle_gunner = FALSE;
			prop->dangerous_vehicle_driver = FALSE;
			prop->attached_to_unit_index = NONE;

			if (unit->object.parent_object_index!=NONE)
			{
				struct object_datum *parent = object_get(unit->object.parent_object_index);

				if (parent->object.type==_object_type_vehicle)
				{
					struct vehicle_datum *vehicle = (struct vehicle_datum *)parent;

					prop->vehicle_index = unit->object.parent_object_index;
					prop->vehicle_gunner = vehicle->unit.gunner_object_index==prop->unit_index || prop->type==_actor_mounted_weapon;
					prop->dangerous_vehicle_driver = vehicle->unit.driver_object_index==prop->unit_index && vehicle_causes_collision_damage(prop->vehicle_index);
				}
				else if (TEST_FLAG(_object_mask_unit, parent->object.type))
				{
					prop->attached_to_unit_index = unit->object.parent_object_index;
				}
			}

			prop->child_units_attached = 0;

			{
				long child_index;

				for (child_index = unit->object.first_child_object_index; child_index!=NONE; )
				{
					struct object_datum *child = object_get(child_index);

					if (TEST_FLAG(_object_mask_unit, child->object.type))
					{
						prop->child_units_attached++;
					}

					child_index = child->object.next_object_index;
				}
			}
		}

		actor_perception_find_sense_position(actor_index, &prop->body_position, prop_index, sense_position);
		vector_from_points3d(&sense_position->body_position, &prop->body_position, &prop->actor_to_prop);
		prop->distance = normalize3d(&prop->actor_to_prop);

		if (prop->distance==0.f)
		{
			prop->actor_to_prop = *global_forward3d;
		}
	}

	return;
}

void prop_status_refresh(
	long actor_index,
	long prop_index,
	struct actor_position_data *sense_position)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.active)
	{
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
		struct encounter_datum *encounter = actor->meta.encounter_index==NONE ? NULL : encounter_get(actor->meta.encounter_index);
		struct prop_datum *prop = prop_get(prop_index);
		struct unit_datum *unit = unit_get(prop->unit_index);
		long current_time = game_time_get();
		boolean blind = (encounter && encounter->blind) || actor->state.mode==_actor_mode_asleep;

		prop->ignore = TEST_FLAG(unit->unit.flags, _unit_ignored_by_actors_bit);

		if (prop->player && AI_BEHAVIOR(ignore_player))
		{
			prop->ignore = TRUE;
		}

		if (prop->enemy)
		{
			prop->preferred_target = TEST_FLAG(unit->unit.flags, _unit_preferred_target_bit);

			switch (actor->external_orders.desired_target_type)
			{
				case _desired_target_player:
					if (prop->player)
					{
						prop->preferred_target = TRUE;
					}
					break;

				case _desired_target_ai:
					if (prop->actor_index!=NONE && actor->external_orders.desired_target_ai_index!=NONE)
					{
						struct actor_datum *prop_actor = actor_get(prop->actor_index);

						if (DATUM_INDEX_TO_ABSOLUTE_INDEX(prop_actor->meta.encounter_index)==AI_INDEX_GET_ENCOUNTER_INDEX(actor->external_orders.desired_target_ai_index))
						{
							switch (AI_INDEX_GET_TYPE(actor->external_orders.desired_target_ai_index))
							{
								case _ai_index_encounter:
									prop->preferred_target = TRUE;
									break;

								case _ai_index_platoon:
									if (AI_INDEX_GET_SUBINDEX(actor->external_orders.desired_target_ai_index)==prop_actor->meta.squad_index)
									{
										prop->preferred_target = TRUE;
									}
									break;

								case _ai_index_squad:
									if (AI_INDEX_GET_SUBINDEX(actor->external_orders.desired_target_ai_index)==prop_actor->meta.platoon_index)
									{
										prop->preferred_target = TRUE;
									}
									break;
							}
						}
					}
					break;
			}
		}

		{
			real_vector3d velocity;
			real_vector3d closing_velocity;
			real speed;
			real closing_speed;
			short old_speed = prop->quantized_speed;

			object_get_velocities(prop->unit_index, &velocity, NULL);
			speed = magnitude3d(&velocity);

			if (speed<0.1f / TICKS_PER_SECOND)
			{
				prop->quantized_speed = _prop_speed_stopped;
			}
			else if (speed<0.5f / TICKS_PER_SECOND)
			{
				prop->quantized_speed = _prop_speed_walking;
			}
			else if (speed<1.f / TICKS_PER_SECOND)
			{
				prop->quantized_speed = _prop_speed_running;
			}
			else
			{
				prop->quantized_speed = _prop_speed_driving;
			}

			subtract_vectors3d(&velocity, &sense_position->velocity, &closing_velocity);
			closing_speed = -dot_product3d(&closing_velocity, &prop->actor_to_prop);

			if (closing_speed<-1.f / TICKS_PER_SECOND)
			{
				prop->quantized_closing_speed = 0;
			}
			else if (closing_speed<-0.5f / TICKS_PER_SECOND)
			{
				prop->quantized_closing_speed = 1;
			}
			else if (closing_speed<-0.1f / TICKS_PER_SECOND)
			{
				prop->quantized_closing_speed = 2;
			}
			else if (closing_speed<0.1f / TICKS_PER_SECOND)
			{
				prop->quantized_closing_speed = 3;
			}
			else if (closing_speed<0.5f / TICKS_PER_SECOND)
			{
				prop->quantized_closing_speed = 4;
			}
			else if (closing_speed<1.f / TICKS_PER_SECOND)
			{
				prop->quantized_closing_speed = 5;
			}
			else
			{
				prop->quantized_closing_speed = 6;
			}

			if (prop_acknowledged(prop) && old_speed<=_prop_speed_walking && prop->quantized_speed>_prop_speed_walking)
			{
				struct direction_specification direction;

				direction.type = _direction_specification_prop;
				direction.prop_index = prop_index;
				actor_look_secondary(actor_index, _secondary_look_started_moving_prop, 1, &direction);
			}
		}

		if (prop->distance<1.f)
		{
			prop->quantized_distance = _prop_distance_melee;
		}
		else if (prop->distance<6.f)
		{
			prop->quantized_distance = _prop_distance_near;
		}
		else if (prop->distance<10.f)
		{
			prop->quantized_distance = _prop_distance_middle;
		}
		else if (prop->distance<30.f)
		{
			prop->quantized_distance = _prop_distance_far;
		}
		else
		{
			prop->quantized_distance = _prop_distance_distant;
		}

		{
			real_vector3d aiming_vector;
			real facing;
			real facing_distance;

			unit_get_aiming_vector(prop->unit_index, &aiming_vector);
			facing = -dot_product3d(&prop->actor_to_prop, &aiming_vector);

			if (facing<=0.f)
			{
				facing_distance = REAL_MAX;
			}
			else if (facing>=1.f)
			{
				facing_distance = 0.f;
			}
			else
			{
				facing_distance = square_root(1.f - facing * facing) * prop->distance;
			}

			if (facing>0.9925f || facing_distance<0.5f)
			{
				prop->quantized_facing = _prop_facing_direct;
			}
			else if (facing>0.9063f || facing_distance<1.5f)
			{
				prop->quantized_facing = _prop_facing_nearby;
			}
			else if (facing>0.5f)
			{
				prop->quantized_facing = _prop_facing_central;
			}
			else if (facing>0.f)
			{
				prop->quantized_facing = _prop_facing_peripheral;
			}
			else
			{
				prop->quantized_facing = _prop_facing_behind;
			}
		}

		prop->shooting = prop->unit_effect==_ai_unit_effect_shooting;

		if (prop_orphaned(prop))
		{
			prop->line_of_sight = ai_test_line_of_sight(
				&sense_position->head_position,
				sense_position->body_location.cluster_index,
				&prop->head_position,
				prop->body_location.cluster_index,
				prop->player && prop->enemy ? 2 : 0,
				FALSE,
				prop->vehicle_index,
				actor->input.vehicle_index!=NONE);

			if (prop->ignore || blind)
			{
				prop->perception = _actor_perception_none;
				prop->ineffability = _actor_perception_none;
				prop->audibility = _actor_perception_none;
				prop->visibility = _actor_perception_none;
			}
			else
			{
				prop->visibility = actor_visibility_at_point(
					actor_index,
					sense_position,
					&prop->head_position,
					prop->lighting,
					prop->line_of_sight,
					TRUE,
					FALSE,
					_actor_knowledge_searching);
				prop->audibility = _actor_perception_none;
				prop->ineffability = _actor_perception_none;
				prop->perception = prop->visibility;
			}
		}
		else
		{
			boolean dead;
			boolean really_dead;
			long new_actor_index;
			boolean noncombat;
			boolean in_combat;
			boolean fighting;
			boolean swarm;
			boolean need_validation = FALSE;

			prop->line_of_sight = ai_test_line_of_sight(
				&sense_position->head_position,
				sense_position->body_location.cluster_index,
				&prop->head_position,
				prop->body_location.cluster_index,
				prop->player && prop->enemy ? 2 : 0,
				FALSE,
				prop->vehicle_index,
				actor->input.vehicle_index!=NONE);
			prop->lighting = _prop_lighting_bright;

			if (unit->object.type==_object_type_biped)
			{
				struct biped_definition *biped_definition = biped_definition_get(unit->definition_index);

				prop->flying = TEST_FLAG(biped_definition->biped.flags, _biped_flying_bit);
			}
			else
			{
				prop->flying = FALSE;
			}

			prop->active_camouflage = unit->unit.active_camouflage>0.5f;
			prop->flashlight = TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit);
			dead = TEST_FLAG(unit->object.damage_flags, _object_dead_bit);
			really_dead = dead && !unit->unit.feign_death_timer;
			prop->just_killed = dead && !prop->dead;
			prop->dead = dead;
			prop->really_dead = really_dead;

			if (prop->just_killed && !prop->enemy && actor->state.mode<_actor_mode_combat)
			{
				need_validation = TRUE;
			}

			if (prop->dead)
			{
				prop->required_ticks = 0;
			}

			if (unit->unit.swarm_actor_index!=NONE)
			{
				new_actor_index = unit->unit.swarm_actor_index;
				swarm = TRUE;
			}
			else
			{
				new_actor_index = unit->unit.actor_index;
				swarm = FALSE;
			}

			if (new_actor_index!=prop->actor_index)
			{
				prop->swarm = swarm;
				prop->actor_index = new_actor_index;

				if (prop->orphan_prop_index!=NONE)
				{
					struct prop_datum *orphan_prop = prop_get(prop->orphan_prop_index);

					orphan_prop->actor_index = prop->actor_index;
					orphan_prop->swarm = prop->swarm;
				}
			}

			if (new_actor_index==NONE)
			{
				noncombat = FALSE;
				in_combat = FALSE;
				fighting = !prop->dead;
			}
			else
			{
				noncombat = actor_is_noncombat(prop->actor_index);
				in_combat = actor_in_combat(prop->actor_index);
				fighting = actor_is_fighting(prop->actor_index);
			}

			if (in_combat && !prop->in_combat && !prop->enemy && actor->state.mode<_actor_mode_combat)
			{
				need_validation = TRUE;
			}

			prop->noncombat = noncombat;
			prop->in_combat = in_combat;
			prop->fighting = fighting;

			if (need_validation)
			{
				short visibility = _actor_perception_none;

				if (!blind)
				{
					char lighting = prop->flashlight ? _prop_lighting_bright : prop->lighting;

					visibility = actor_visibility_at_point(
						actor_index,
						sense_position,
						&prop->head_position,
						lighting,
						prop->line_of_sight,
						TRUE,
						FALSE,
						actor_get_perception_knowledge(actor_index, prop_index));
				}

				if (visibility<_actor_perception_full)
				{
					prop->perception = visibility;
					prop->visibility = visibility;
					prop->state = _prop_state_unacknowledged;
				}
			}

			if (prop->ignore)
			{
				prop->perception = _actor_perception_none;
				prop->ineffability = _actor_perception_none;
				prop->audibility = _actor_perception_none;
				prop->visibility = _actor_perception_none;
			}
			else
			{
				boolean invisible = blind;

				if (AI_BEHAVIOR(blind) ||
					(AI_BEHAVIOR(invisible_player) && prop->player))
				{
					invisible = TRUE;
				}

				if (prop->active_camouflage)
				{
					if (prop->enemy)
					{
						invisible = TRUE;
					}
					else
					{
						invisible = prop->player && prop->distance>4.f;
					}
				}

				if (invisible)
				{
					prop->visibility = _actor_perception_none;
					prop->just_became_visible = FALSE;
				}
				else
				{
					char lighting;
					short visibility;
					boolean use_frustum = TRUE;

					if (actor->input.vehicle_driver_type==_actor_vehicle_driver_directional_flying)
					{
						use_frustum = FALSE;
					}
					else if (actor->meta.type==_actor_mounted_weapon)
					{
						use_frustum = FALSE;
					}
					else if (!prop->enemy)
					{
						use_frustum = FALSE;

						if (actor->state.mode<_actor_mode_combat && (prop->dead || prop->in_combat))
						{
							use_frustum = TRUE;
						}
					}
					else if (prop_acknowledged(prop))
					{
						use_frustum = FALSE;
					}

					lighting = prop->flashlight ? _prop_lighting_bright : prop->lighting;
					visibility = actor_visibility_at_point(
						actor_index,
						sense_position,
						&prop->head_position,
						lighting,
						prop->line_of_sight,
						use_frustum,
						prop->player,
						actor_get_perception_knowledge(actor_index, prop_index));
					prop->just_became_visible = !prop->visibility && visibility>_actor_perception_none;
					prop->visibility = visibility;

					if (prop->visibility)
					{
						prop->last_visible_head_position = prop->head_position;
						prop->last_visible_time = current_time;
					}
				}

				if (AI_BEHAVIOR(deaf) || (encounter && encounter->deaf))
				{
					prop->audibility = _actor_perception_none;
				}
				else if (prop->unit_effect==_ai_unit_effect_shooting || prop->unit_effect==_ai_unit_effect_death_scream)
				{
					prop->audibility = _actor_perception_unmistakable;
				}
				else
				{
					long sound_unit_index = prop->vehicle_index==NONE ? prop->unit_index : prop->vehicle_index;
					struct unit_datum *sound_unit = unit_get(sound_unit_index);
					struct unit_definition *unit_definition = unit_definition_get(sound_unit->definition_index);

					prop->audibility = actor_audibility_at_point(
						actor_index,
						sense_position,
						&prop->body_position,
						&prop->body_location,
						unit_definition->unit.constant_sound,
						1.f,
						prop->line_of_sight);
				}

				prop->ineffability = _actor_perception_none;

				if (prop->unit_effect==_ai_unit_effect_bump)
				{
					prop->ineffability = MAX(prop->ineffability, _actor_perception_unmistakable);
				}

				if (prop->flashlight &&
					prop->quantized_facing<=_prop_facing_central &&
					prop->quantized_distance<=_prop_distance_middle &&
					(prop->line_of_sight==_ai_line_of_sight_clear || prop->line_of_sight==_ai_line_of_sight_occluded))
				{
					prop->ineffability = MAX(prop->ineffability, _actor_perception_partial);
				}

				prop->perception = MAX(prop->visibility, MAX(prop->audibility, prop->ineffability));

				if (prop->perception==_actor_perception_partial && prop_acknowledged(prop))
				{
					prop->perception = _actor_perception_full;
				}
			}

			if (prop->perception)
			{
				prop->last_perceived_body_position = prop->body_position;
				prop->last_perceived_time = current_time;
			}

			if (prop_acknowledged(prop))
			{
				boolean definitely_located = FALSE;

				if (prop->visibility>=_actor_perception_full)
				{
					definitely_located = TRUE;
				}
				else if (prop->definitely_located && prop->definite_knowledge_source_actor!=NONE)
				{
					struct actor_datum *source_actor = actor_try_and_get(prop->definite_knowledge_source_actor);

					if (source_actor &&
						source_actor->target.target_type>=_actor_target_visible_enemy &&
						source_actor->target.target_prop_index!=NONE &&
						source_actor->orders.combat.shoot_at_target)
					{
						struct prop_datum *source_target_prop = prop_get(source_actor->target.target_prop_index);

						if (source_target_prop->unit_index==prop->unit_index)
						{
							definitely_located = TRUE;
						}
					}
				}

				if (definitely_located)
				{
					prop->definitely_located = TRUE;
					prop->ticks_since_definitely_located = 0;
				}
			}
		}

		if (prop->dangerous_vehicle_driver)
		{
			actor_perception_assess_vehicle_danger(actor_index, prop->vehicle_index, sense_position, prop->perception>=_actor_perception_full);
		}

		if (prop->suicide_radius>0.f && (prop->dead || unit->unit.animation.state==_unit_state_melee_attack))
		{
			actor_perception_assess_suicide_danger(
				actor_index,
				prop->unit_index,
				prop->suicide_radius,
				prop->distance,
				prop->enemy,
				prop->perception>=_actor_perception_full);
		}

		if (prop->enemy && prop_acknowledged(prop))
		{
			boolean reachable = FALSE;

			if (actor_has_ranged_weapon(actor_index) && prop->distance<actor->control.weapon_maximum_range)
			{
				reachable = TRUE;
			}
			else if (TEST_FLAG(actor_definition->flags, _actor_definition_suicidal_melee_attack_bit) && prop->distance<actor_definition->berserk.melee_attack_range)
			{
				reachable = TRUE;
			}

			if (reachable)
			{
				actor_perception_unreachable(actor_index, prop_index, FALSE);
			}
		}

		if (prop->last_unreachable_time!=NONE && prop->last_unreachable_time + 5 * TICKS_PER_SECOND<current_time)
		{
			actor_perception_unreachable(actor_index, prop_index, FALSE);
		}

		if (prop->delay_requirement_decision)
		{
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
				((prop->distance) * (prop->distance)),
				0,
				NULL))
			{
				prop->required_ticks = 0;
			}

			prop->delay_requirement_decision = FALSE;
		}

		prop->unopposable_enemy = actor_compute_prop_unopposable(actor_index, prop_index);
		prop->target_weight = actor_compute_prop_target_weight(actor_index, prop_index);
		prop->look_interest = actor_look_compute_prop_interest(actor_index, prop_index);
		prop->refresh_stimuli = TRUE;
	}

	return;
}

static int __cdecl actor_perception_qsort_compare_optional_props(
	const void *p1,
	const void *p2)
{
	real distance_squared1 = ((struct actor_optional_prop const *)p1)->distance_squared;
	real distance_squared2 = ((struct actor_optional_prop const *)p2)->distance_squared;

	if (distance_squared1<distance_squared2)
	{
		return -1;
	}
	else if (distance_squared1>distance_squared2)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static void actor_perception_refresh(
	long actor_index)
{
	unsigned long computed_pvs[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CLUSTERS_PER_STRUCTURE)];
	struct actor_prop_environment enemies;
	struct actor_prop_environment friends;
	struct prop_iterator iterator;
	struct prop_datum *prop;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	struct actor_datum *actor = actor_get(actor_index);
	unsigned long *pvs = NULL;

	enemies.optional_count = 0;
	enemies.existing_count = 0;
	friends.optional_count = 0;
	friends.existing_count = 0;

	if (actor->meta.swarm)
	{
		short index;
		struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
		boolean pvs_valid = FALSE;

		memset(computed_pvs, 0, sizeof(computed_pvs));

		for (index = 0; index<swarm->unit_count; index++)
		{
			struct unit_datum *unit = unit_get(swarm->unit_indices[index]);
			short cluster_index = unit->object.location.cluster_index;

			if (cluster_index!=NONE)
			{
				unsigned long *cluster_pvs = structure_bsp_get_cluster_pvs(structure_bsp, cluster_index);

				bit_vector_or((short)structure_bsp->clusters.count, cluster_pvs, computed_pvs, computed_pvs);
				pvs_valid = TRUE;
			}
		}

		if (pvs_valid)
		{
			pvs = computed_pvs;
		}
	}
	else if (actor->input.position.body_location.cluster_index!=NONE)
	{
		pvs = structure_bsp_get_cluster_pvs(structure_bsp, actor->input.position.body_location.cluster_index);
	}

	object_marker_begin();
	prop_iterator_new(&iterator, actor_index);

	while (prop = prop_iterator_next(&iterator))
	{
		if (!prop_orphaned(prop))
		{
			real distance_squared = ((prop->distance) * (prop->distance));
			boolean optional = FALSE;
			boolean desired = actor_perception_desire_prop(
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
				distance_squared,
				prop->required_ticks,
				&optional);

			if (desired && pvs)
			{
				struct object_cluster_iterator cluster_iterator;
				short cluster_index;

				desired = FALSE;

				for (cluster_index = object_get_first_cluster(&cluster_iterator, prop->unit_index);
					cluster_index!=NONE;
					cluster_index = object_get_next_cluster(&cluster_iterator, prop->unit_index))
				{
					if (BIT_VECTOR_TEST_FLAG(pvs, cluster_index))
					{
						desired = TRUE;
						break;
					}
				}
			}

			if (prop->swarm && prop->actor_index!=NONE)
			{
				struct actor_datum *swarm_actor = actor_get(prop->actor_index);

				if (swarm_actor->meta.swarm_cache_index!=NONE)
				{
					short index;
					struct swarm_datum *swarm = swarm_get(swarm_actor->meta.swarm_cache_index);

					for (index = 0; index<swarm->unit_count; index++)
					{
						object_mark_function(swarm->unit_indices[index]);
					}
				}
				else
				{
					long unit_index;

					for (unit_index = actor->meta.swarm_unit_index; unit_index!=NONE; )
					{
						struct unit_datum *unit = unit_get(unit_index);

						object_mark_function(unit_index);
						unit_index = unit->unit.swarm_next_unit_index;
					}
				}
			}

			object_mark_function(prop->unit_index);

			if (desired)
			{
				struct actor_prop_environment *environment = prop->enemy ? &enemies : &friends;

				if (optional)
				{
					match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 2669, !prop->dead);

					if (environment->optional_count<MAXIMUM_OPTIONAL_PROPS_PER_ACTOR)
					{
						environment->optional_props[environment->optional_count].unit_index = prop->unit_index;
						environment->optional_props[environment->optional_count].prop_index = iterator.index;
						environment->optional_props[environment->optional_count++].distance_squared = distance_squared * (1.f / (1.2f * 1.2f));
					}
					else
					{
						static long last_overflow_time = NONE;

						desired = FALSE;

						if (last_overflow_time==NONE || game_time_get()>last_overflow_time + 5 * TICKS_PER_SECOND)
						{
							error(
								_error_silent,
								"actor_perception_refresh overflowed max %s (%d), discarding",
								prop->enemy ? "enemies" : "friends",
								MAXIMUM_OPTIONAL_PROPS_PER_ACTOR);
							last_overflow_time = game_time_get();
						}
					}
				}
				else if (!prop->dead)
				{
					environment->existing_count++;
				}
			}
			else
			{
				if (!prop_orphaned(prop) && prop->orphan_prop_index!=NONE)
				{
					actor_switch_props(actor_index, prop->orphan_prop_index, NONE);
					prop_delete(actor_index, prop->orphan_prop_index);
				}

				actor_switch_props(actor_index, iterator.index, NONE);
				prop_delete(actor_index, iterator.index);
			}
		}
	}

	if (pvs)
	{
		short cluster_index;

		for (cluster_index = 0; cluster_index<structure_bsp->clusters.count; cluster_index++)
		{
			if (BIT_VECTOR_TEST_FLAG(pvs, cluster_index))
			{
				long object_iterator;
				long object_index;

				for (object_index = cluster_get_first_collideable_object(&object_iterator, cluster_index);
					object_index!=NONE;
					object_index = cluster_get_next_collideable_object(&object_iterator))
				{
					actor_perception_refresh_test_object(actor_index, object_index, &enemies, &friends);
				}

				for (object_index = cluster_get_first_noncollideable_object(&object_iterator, cluster_index);
					object_index!=NONE;
					object_index = cluster_get_next_noncollideable_object(&object_iterator))
				{
					actor_perception_refresh_test_object(actor_index, object_index, &enemies, &friends);
				}
			}
		}
	}

	if (enemies.optional_count>0)
	{
		short optional_index = 0;

		if (enemies.existing_count<4)
		{
			qsort(enemies.optional_props, enemies.optional_count, sizeof(enemies.optional_props[0]), actor_perception_qsort_compare_optional_props);

			while (optional_index<enemies.optional_count)
			{
				long prop_index = enemies.optional_props[optional_index].prop_index;

				if (prop_index==NONE)
				{
					prop_index = prop_new_unacknowledged(actor_index, enemies.optional_props[optional_index].unit_index, TRUE);

					if (prop_index!=NONE)
					{
						struct actor_position_data sense_position;

						prop_position_refresh(actor_index, prop_index, &sense_position, FALSE, FALSE);
					}
				}

				if (prop_index!=NONE && ++enemies.existing_count>=4)
				{
					break;
				}

				optional_index++;
			}
		}

		while (optional_index<enemies.optional_count)
		{
			if (enemies.optional_props[optional_index].prop_index!=NONE)
			{
				struct prop_datum *optional_prop = prop_get(enemies.optional_props[optional_index].prop_index);

				if (!prop_orphaned(optional_prop) && optional_prop->orphan_prop_index!=NONE)
				{
					actor_switch_props(actor_index, optional_prop->orphan_prop_index, NONE);
					prop_delete(actor_index, optional_prop->orphan_prop_index);
				}

				actor_switch_props(actor_index, enemies.optional_props[optional_index].prop_index, NONE);
				prop_delete(actor_index, enemies.optional_props[optional_index].prop_index);
			}

			optional_index++;
		}
	}

	if (friends.optional_count>0)
	{
		short total_count;
		short maximum_count;
		short optional_index;

		optional_index = 0;
		total_count = friends.existing_count + enemies.existing_count;
		maximum_count = MAX(enemies.existing_count + 2, 4);

		if (total_count<maximum_count)
		{
			qsort(friends.optional_props, friends.optional_count, sizeof(friends.optional_props[0]), actor_perception_qsort_compare_optional_props);

			while (optional_index<friends.optional_count)
			{
				long prop_index = friends.optional_props[optional_index].prop_index;

				if (prop_index==NONE)
				{
					prop_index = prop_new_unacknowledged(actor_index, friends.optional_props[optional_index].unit_index, FALSE);

					if (prop_index!=NONE)
					{
						struct actor_position_data sense_position;

						prop_position_refresh(actor_index, prop_index, &sense_position, FALSE, FALSE);
					}
				}

				if (prop_index!=NONE)
				{
					friends.existing_count++;

					if (++total_count>=maximum_count)
					{
						break;
					}
				}

				optional_index++;
			}
		}

		while (optional_index<friends.optional_count)
		{
			if (friends.optional_props[optional_index].prop_index!=NONE)
			{
				struct prop_datum *optional_prop = prop_get(friends.optional_props[optional_index].prop_index);

				if (!prop_orphaned(optional_prop) && optional_prop->orphan_prop_index!=NONE)
				{
					actor_switch_props(actor_index, optional_prop->orphan_prop_index, NONE);
					prop_delete(actor_index, optional_prop->orphan_prop_index);
				}

				actor_switch_props(actor_index, friends.optional_props[optional_index].prop_index, NONE);
				prop_delete(actor_index, friends.optional_props[optional_index].prop_index);
			}

			optional_index++;
		}
	}

	object_marker_end();

	return;
}

static void actor_perception_refresh_test_object(
	long actor_index,
	long object_index,
	struct actor_prop_environment *enemies,
	struct actor_prop_environment *friends)
{
	struct actor_datum *actor = actor_get(actor_index);

	while (object_index!=NONE)
	{
		struct object_datum *object = object_get(object_index);

		if (object_mark_function(object_index))
		{
			if (object->object.type==_object_type_biped)
			{
				struct actor_position_data sense_position;
				real_point3d unit_position;
				long unit_actor_index;
				struct unit_datum *unit = (struct unit_datum *)object;
				long unit_index = object_index;

				object_get_origin(object_index, &unit_position);
				actor_perception_find_sense_position(actor_index, &unit_position, NONE, &sense_position);

				if (unit->unit.swarm_actor_index!=NONE)
				{
					unit_actor_index = unit->unit.swarm_actor_index;
					unit_index = actor_perception_unit_from_swarm(actor_index, &sense_position, unit->unit.swarm_actor_index, NONE, TRUE);

					if (unit_index!=NONE)
					{
						unit = unit_get(unit_index);
						object_get_origin(unit_index, &unit_position);
					}
				}
				else
				{
					unit_actor_index = unit->unit.actor_index;
				}

				if (unit_index==NONE)
				{
				}
				else if (unit_actor_index==actor_index)
				{
				}
				else
				{
					struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
					boolean player = unit->unit.player_index!=NONE;
					boolean enemy = game_team_is_enemy(actor->meta.team_index, unit->object.owner_team_index);
					boolean dead = TEST_FLAG(unit->object.damage_flags, _object_dead_bit) && !unit->unit.feign_death_timer;
					short dead_ticks = !dead ? 0 : (unit->unit.time_of_death==NONE ? SHRT_MAX : (short)(game_time_get() - unit->unit.time_of_death));
					real suicide_radius = unit_definition->unit.ai_danger_radius;
					boolean optional = FALSE;
					real distance_squared = distance_squared3d(&sense_position.body_position, &unit_position);

					if (suicide_radius>0.f && (dead || unit->unit.animation.state==_unit_state_melee_attack))
					{
						actor_perception_assess_suicide_danger(actor_index, unit_index, suicide_radius, square_root(distance_squared), enemy, FALSE);
					}

					if (actor_perception_desire_prop(
						actor_index,
						_prop_state_unacknowledged,
						unit_index,
						unit_actor_index,
						FALSE,
						player,
						enemy,
						dead,
						dead_ticks,
						suicide_radius,
						distance_squared,
						0,
						&optional))
					{
						struct actor_prop_environment *environment = enemy ? enemies : friends;

						if (optional)
						{
							match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 2966, !dead);

							if (environment->optional_count<MAXIMUM_OPTIONAL_PROPS_PER_ACTOR)
							{
								environment->optional_props[environment->optional_count].prop_index = NONE;
								environment->optional_props[environment->optional_count].unit_index = unit_index;
								environment->optional_props[environment->optional_count++].distance_squared = distance_squared;
							}
						}
						else
						{
							long prop_index = prop_new_unacknowledged(actor_index, unit_index, enemy);

							if (prop_index!=NONE)
							{
								prop_position_refresh(actor_index, prop_index, &sense_position, FALSE, FALSE);

								if (!dead)
								{
									environment->existing_count++;
								}
							}
						}
					}
				}
			}
			else if (object->object.type==_object_type_vehicle)
			{
				struct vehicle_datum *vehicle = (struct vehicle_datum *)object;

				if (vehicle->unit.driver_object_index==NONE)
				{
					actor_perception_assess_vehicle_danger(actor_index, object_index, NULL, FALSE);
				}
			}
			else if (object->object.type==_object_type_projectile)
			{
				struct projectile_definition *projectile_definition = projectile_definition_get(object->definition_index);

				if (projectile_definition->projectile.danger_radius>0.f)
				{
					struct projectile_datum *projectile = (struct projectile_datum *)object;
					boolean dangerous = FALSE;

					dangerous = projectile->object.parent_object_index==NONE || TEST_FLAG(projectile->projectile.flags, _projectile_counting_down_bit);

					if (dangerous)
					{
						real_point3d projectile_origin;
						real current_distance;
						struct actor_position_data sense_position;

						object_get_origin(object_index, &projectile_origin);
						actor_perception_find_sense_position(actor_index, &projectile_origin, NONE, &sense_position);
						current_distance = distance3d(&sense_position.body_position, &projectile_origin);

						if (projectile_definition->projectile.danger_radius + 10.f>current_distance)
						{
							if (actor->danger_zone.danger_type<_actor_danger_zone_projectile ||
								(actor->danger_zone.danger_type==_actor_danger_zone_projectile &&
								actor->danger_zone.object_index!=object_index &&
								current_distance<actor->danger_zone.current_distance_from_actor))
							{
								long owner_unit_index;

								memset(&actor->danger_zone, 0, sizeof(actor->danger_zone));
								actor->danger_zone.danger_type = _actor_danger_zone_projectile;
								actor->danger_zone.object_index = object_index;
								actor->danger_zone.danger_radius = projectile_definition->projectile.danger_radius;
								actor->danger_zone.initial_position = projectile_origin;
								actor->danger_zone.initial_velocity = object->object.translational_velocity;
								actor->danger_zone.acknowledgement_timer = 30;
								actor->danger_zone.currently_perceived = FALSE;
								actor->danger_zone.hostility = _actor_danger_hostility_enemy;
								owner_unit_index = NONE;

								if (object->object.owner_object_index!=NONE)
								{
									struct object_datum *owner = object_try_and_get(object->object.owner_object_index);

									if (owner && TEST_FLAG(_object_mask_unit, owner->object.type))
									{
										owner_unit_index = object->object.owner_object_index;

										if (actor->meta.unit_index!=NONE && owner_unit_index==actor->meta.unit_index)
										{
											actor->danger_zone.hostility = _actor_danger_hostility_self;
										}
										else if (!game_team_is_enemy(actor->meta.team_index, object->object.owner_team_index))
										{
											actor->danger_zone.hostility = _actor_danger_hostility_friend;
										}
									}
								}

								actor->danger_zone.owner_unit_index = owner_unit_index;
							}
						}
					}
				}
			}
		}

		if (object->object.first_child_object_index!=NONE)
		{
			actor_perception_refresh_test_object(actor_index, object->object.first_child_object_index, enemies, friends);
		}

		object_index = object->object.next_object_index;
	}

	return;
}

static boolean actor_perception_assess_vehicle_danger(
	long actor_index,
	long vehicle_index,
	struct actor_position_data *sense_position,
	boolean currently_perceived)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->input.vehicle_index==NONE)
	{
		struct vehicle_datum *vehicle = vehicle_get(vehicle_index);
		struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

		if (TEST_FLAG(vehicle_definition->vehicle.flags, _vehicle_causes_collision_damage_bit) &&
			magnitude_squared3d(&vehicle->object.translational_velocity)>(1.f / TICKS_PER_SECOND) * (1.f / TICKS_PER_SECOND))
		{
			real_point3d vehicle_origin;
			struct actor_position_data computed_sense_position;
			real current_distance;

			object_get_origin(vehicle_index, &vehicle_origin);

			if (!sense_position)
			{
				actor_perception_find_sense_position(actor_index, &vehicle_origin, NONE, &computed_sense_position);
				sense_position = &computed_sense_position;
			}

			current_distance = distance3d(&sense_position->body_position, &vehicle_origin);

			if (vehicle_definition->object.bounding_radius + 10.f>current_distance &&
				(actor->danger_zone.danger_type<_actor_danger_zone_vehicle ||
				(actor->danger_zone.danger_type==_actor_danger_zone_vehicle &&
				actor->danger_zone.object_index!=vehicle_index &&
				current_distance<actor->danger_zone.current_distance_from_actor)))
			{
				memset(&actor->danger_zone, 0, sizeof(actor->danger_zone));
				actor->danger_zone.danger_type = _actor_danger_zone_vehicle;
				actor->danger_zone.object_index = vehicle_index;
				actor->danger_zone.owner_unit_index = vehicle->unit.driver_object_index;
				actor->danger_zone.danger_radius = vehicle_definition->object.bounding_radius;
				actor->danger_zone.initial_position = vehicle_origin;
				actor->danger_zone.initial_velocity = vehicle->object.translational_velocity;
				actor->danger_zone.acknowledgement_timer = 20;
				actor->danger_zone.currently_perceived = currently_perceived;
				actor->danger_zone.hostility = _actor_danger_hostility_enemy;

				if (actor->danger_zone.owner_unit_index!=NONE)
				{
					struct unit_datum *owner_unit = unit_get(vehicle->unit.driver_object_index);

					if (!game_team_is_enemy(actor->meta.team_index, owner_unit->object.owner_team_index))
					{
						actor->danger_zone.hostility = _actor_danger_hostility_friend;
					}
				}

				result = TRUE;
			}
		}
	}

	return result;
}

static boolean actor_perception_assess_suicide_danger(
	long actor_index,
	long unit_index,
	real suicide_radius,
	real current_distance,
	boolean enemy,
	boolean currently_perceived)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (suicide_radius + 10.f>current_distance)
	{
		if (actor->danger_zone.danger_type<_actor_danger_zone_suicide ||
			(actor->danger_zone.danger_type==_actor_danger_zone_suicide &&
			actor->danger_zone.object_index!=unit_index &&
			current_distance<actor->danger_zone.current_distance_from_actor))
		{
			struct unit_datum *unit = unit_get(unit_index);

			memset(&actor->danger_zone, 0, sizeof(actor->danger_zone));
			actor->danger_zone.danger_type = _actor_danger_zone_suicide;
			actor->danger_zone.object_index = unit_index;
			actor->danger_zone.danger_radius = suicide_radius;
			object_get_origin(unit_index, &actor->danger_zone.initial_position);
			actor->danger_zone.initial_velocity = unit->object.translational_velocity;
			actor->danger_zone.acknowledgement_timer = 6;
			actor->danger_zone.currently_perceived = currently_perceived;
			actor->danger_zone.hostility = enemy ? _actor_danger_hostility_enemy : _actor_danger_hostility_friend;
			result = TRUE;
		}
	}

	return result;
}

static void actor_perception_refresh_danger_zone(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->danger_zone.danger_type>_actor_danger_zone_none)
	{
		struct object_datum *object = object_try_and_get(actor->danger_zone.object_index);

		if (!object)
		{
			actor->danger_zone.danger_type = _actor_danger_zone_none;
		}
		else
		{
			struct actor_position_data sense_position;
			boolean currently_perceived;
			boolean attached_to_us;

			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 3227, (actor->danger_zone.danger_type == _actor_danger_zone_projectile) || (actor->danger_zone.danger_type == _actor_danger_zone_vehicle) || (actor->danger_zone.danger_type == _actor_danger_zone_suicide));

			object_get_origin(actor->danger_zone.object_index, &actor->danger_zone.position);
			actor_perception_find_sense_position(actor_index, &actor->danger_zone.position, NONE, &sense_position);
			actor->danger_zone.velocity = object->object.translational_velocity;
			actor->danger_zone.current_distance_from_actor = distance3d(&sense_position.body_position, &actor->danger_zone.position);
			point_from_line3d(&actor->danger_zone.position, &actor->danger_zone.velocity, 1.5f * TICKS_PER_SECOND, &actor->danger_zone.predict_danger_position);
			midpoint3d(&actor->danger_zone.position, &actor->danger_zone.predict_danger_position, &actor->danger_zone.bounding_sphere_center);
			actor->danger_zone.bounding_sphere_radius = distance3d(&actor->danger_zone.bounding_sphere_center, &actor->danger_zone.position) + actor->danger_zone.danger_radius;
			currently_perceived = FALSE;
			attached_to_us = FALSE;

			switch (actor->danger_zone.danger_type)
			{
				case _actor_danger_zone_projectile:
				{
					struct projectile_datum *projectile = (struct projectile_datum *)object;

					if (actor->meta.unit_index!=NONE && projectile->object.parent_object_index==actor->meta.unit_index)
					{
						attached_to_us = TRUE;
					}

					match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 3257, object->object.type == _object_type_projectile);

					if (projectile->projectile.detonation_timer>0.f && projectile->projectile.detonation_timer_delta>0.f)
					{
						actor->danger_zone.projectile.time_until_explosion = (short)fast_ftol((1.f - projectile->projectile.detonation_timer) / projectile->projectile.detonation_timer_delta);
					}
					else
					{
						actor->danger_zone.projectile.time_until_explosion = NONE;
					}

					currently_perceived = actor->danger_zone.currently_perceived || attached_to_us;

					if (!currently_perceived)
					{
						struct encounter_datum *encounter = actor->meta.encounter_index==NONE ? NULL : encounter_get(actor->meta.encounter_index);

						if (actor->state.mode==_actor_mode_asleep || (encounter && encounter->blind))
						{
						}
						else
						{
							struct projectile_definition *projectile_definition = projectile_definition_get(projectile->definition_index);

							if (actor->danger_zone.current_distance_from_actor<projectile_definition->projectile.danger_perception_radius)
							{
								short line_of_sight;
								short cluster_index = projectile->object.location.cluster_index;

								if (projectile->object.parent_object_index!=NONE)
								{
									struct object_datum *parent = object_get(object_get_ultimate_parent(actor->danger_zone.object_index));

									cluster_index = parent->object.location.cluster_index;
								}

								line_of_sight = ai_test_line_of_sight(
									&sense_position.head_position,
									sense_position.body_location.cluster_index,
									&actor->danger_zone.position,
									cluster_index,
									0,
									FALSE,
									actor->danger_zone.object_index,
									actor->input.vehicle_index!=NONE);

								if (actor_visibility_at_point(
									actor_index,
									&sense_position,
									&actor->danger_zone.position,
									_prop_lighting_darkness,
									line_of_sight,
									TRUE,
									FALSE,
									actor_get_perception_knowledge(actor_index, NONE))>=_actor_perception_full)
								{
									currently_perceived = TRUE;
								}
							}
						}
					}

					break;
				}

				case _actor_danger_zone_vehicle:
				{
					struct vehicle_datum *vehicle = (struct vehicle_datum *)object;
					struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

					if (magnitude_squared3d(&vehicle->object.translational_velocity)<(0.2f / TICKS_PER_SECOND) * (0.2f / TICKS_PER_SECOND) ||
						actor->danger_zone.current_distance_from_actor>vehicle_definition->object.bounding_radius + 10.f)
					{
						actor->danger_zone.danger_type = _actor_danger_zone_none;
					}
					else
					{
						currently_perceived = actor->danger_zone.currently_perceived;

						if (!currently_perceived)
						{
							long prop_index = NONE;

							if (vehicle->unit.driver_object_index!=NONE)
							{
								prop_index = prop_get_active_by_unit_index(actor_index, vehicle->unit.driver_object_index);
							}

							if (prop_index!=NONE)
							{
								struct prop_datum *prop = prop_get(prop_index);

								currently_perceived = prop->perception>=_actor_perception_full;
							}
							else
							{
								struct location *location;
								short line_of_sight;
								struct encounter_datum *encounter = actor->meta.encounter_index==NONE ? NULL : encounter_get(actor->meta.encounter_index);
								boolean blind = FALSE;

								if (actor->state.mode==_actor_mode_asleep || (encounter && encounter->blind))
								{
									blind = TRUE;
								}

								if (vehicle->object.parent_object_index!=NONE)
								{
									long parent_index = object_get_ultimate_parent(actor->danger_zone.object_index);
									struct object_datum *parent = object_get(parent_index);

									location = &parent->object.location;
								}
								else
								{
									location = &vehicle->object.location;
								}

								line_of_sight = ai_test_line_of_sight(
									&sense_position.head_position,
									sense_position.body_location.cluster_index,
									&actor->danger_zone.position,
									location->cluster_index,
									0,
									FALSE,
									actor->danger_zone.object_index,
									actor->input.vehicle_index!=NONE);

								if (!blind &&
									actor_visibility_at_point(
										actor_index,
										&sense_position,
										&actor->danger_zone.position,
										_prop_lighting_darkness,
										line_of_sight,
										TRUE,
										FALSE,
										actor_get_perception_knowledge(actor_index, NONE))>=_actor_perception_full)
								{
									currently_perceived = TRUE;
								}
								else if (actor_audibility_at_point(
									actor_index,
									&sense_position,
									&actor->danger_zone.position,
									location,
									vehicle_definition->unit.constant_sound,
									1.f,
									line_of_sight)>=_actor_perception_full)
								{
									currently_perceived = TRUE;
								}
							}
						}
					}

					break;
				}

				case _actor_danger_zone_suicide:
				{
					short animation_state;
					short frames_remaining;

					currently_perceived = actor->danger_zone.currently_perceived;

					if (!currently_perceived)
					{
						long prop_index = prop_get_active_by_unit_index(actor_index, actor->danger_zone.object_index);

						if (prop_index!=NONE)
						{
							struct prop_datum *prop = prop_get(prop_index);

							currently_perceived = prop->perception>=_actor_perception_full;
						}
					}

					frames_remaining = unit_get_animation_frames_remaining(actor->danger_zone.object_index, &animation_state);
					actor->danger_zone.suicide.time_until_death = animation_state==_unit_state_dying ? frames_remaining : NONE;

					break;
				}
			}

			if (currently_perceived && !actor->danger_zone.currently_perceived)
			{
				struct direction_specification direction;

				direction.type = _direction_specification_danger;
				actor_look_secondary(actor_index, _secondary_look_dangerous_object, 1, &direction);
			}

			actor->danger_zone.currently_perceived = currently_perceived;
			actor->danger_zone.attached_to_us = attached_to_us;
		}
	}

	return;
}

boolean actor_perception_desire_prop(
	long actor_index,
	short desired_target_state,
	long unit_index,
	long unit_actor_index,
	boolean in_use,
	boolean player,
	boolean enemy,
	boolean dead,
	short dead_ticks,
	real suicide_radius,
	real distance_squared,
	short required_ticks,
	boolean *optional_reference)
{
	boolean desired;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_datum *unit_actor = unit_actor_index==NONE ? NULL : actor_get(unit_actor_index);
	boolean optional = FALSE;

	if ((!enemy || dead) && desired_target_state>=_prop_state_uninspected_orphan && desired_target_state<=_prop_state_inspected_orphan)
	{
		desired = FALSE;
	}
	else if (player)
	{
		desired = TRUE;
	}
	else if (unit_actor && (!unit_actor->meta.active || unit_actor->meta.dormant))
	{
		desired = FALSE;
	}
	else if (desired_target_state==NONE && (in_use || required_ticks>0))
	{
		desired = TRUE;
	}
	else if (distance_squared>40.f * 40.f)
	{
		desired = FALSE;
	}
	else if (dead)
	{
		boolean encounter_idle = FALSE;

		desired = TRUE;

		if (actor->meta.encounter_index!=NONE)
		{
			struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);
			struct unit_datum *unit = unit_get(unit_index);
			long corpse_ignore_time = MAX(encounter->corpse_ignore_time, actor->emotions.corpse_ignore_time);

			if (corpse_ignore_time!=NONE && (unit->unit.time_of_death==NONE || unit->unit.time_of_death<corpse_ignore_time))
			{
				desired = FALSE;
			}

			encounter_idle = !encounter->enemy_visible && !encounter->enemy_alive && !encounter->stand_down;
		}

		if (desired)
		{
			if (encounter_idle)
			{
				desired = distance_squared<15.f * 15.f;
			}
			else if (suicide_radius>0.f)
			{
				desired = TRUE;
			}
			else if (enemy && dead_ticks>5 * TICKS_PER_SECOND)
			{
				desired = FALSE;
			}
			else if (actor_action_class(actor_index)>_action_class_passive)
			{
				desired = FALSE;
			}
			else
			{
				real desired_distance_squared = 4.f * 4.f;

				if (!enemy && actor->state.mode<_actor_mode_combat)
				{
					desired_distance_squared = 8.f * 8.f;
				}

				if (distance_squared<desired_distance_squared)
				{
					desired = TRUE;
				}
				else
				{
					desired = FALSE;
				}
			}
		}
	}
	else if (enemy)
	{
		desired = TRUE;
		optional = distance_squared>6.f * 6.f;
	}
	else
	{
		desired = distance_squared<15.f * 15.f;

		if (actor->state.combat_status>=_actor_combat_status_certain)
		{
			optional = TRUE;
		}
		else if (actor->external_orders.pursuit_is_coordinator)
		{
			optional = FALSE;
		}
		else
		{
			optional = distance_squared>4.f * 4.f;
		}
	}

	if (optional_reference)
	{
		*optional_reference = optional;
	}

	return desired;
}

void actor_perception_find_prop_pathfinding_location(
	long actor_index,
	long prop_index)
{
	struct prop_datum *prop = prop_get(prop_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 3585, prop->owner_actor_index == actor_index);

	if (prop->pathfinding_surface_index==NONE)
	{
		if (prop->vehicle_index!=NONE)
		{
			prop->pathfinding_surface_index = vehicle_find_pathfinding_surface_index(prop->vehicle_index, &prop->pathfinding_point);
		}
		else if (biped_try_and_get(prop->unit_index))
		{
			prop->pathfinding_surface_index = biped_find_pathfinding_surface_index(prop->unit_index, &prop->pathfinding_point);
		}
	}

	return;
}

boolean actor_expected_acknowledgement(
	long actor_index,
	long prop_index)
{
	struct prop_iterator iterator;
	struct prop_datum *other_prop;
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *prop = prop_get(prop_index);
	boolean expected = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 3618, !prop_orphaned(prop));

	prop_iterator_new(&iterator, actor_index);

	while (other_prop = prop_iterator_next(&iterator))
	{
		if (iterator.index!=prop_index &&
			(other_prop->unit_index==prop->unit_index ||
			other_prop->actor_index==prop->actor_index ||
			(prop->enemy && other_prop->enemy && (prop_orphaned(other_prop) || prop_acknowledged(other_prop)))))
		{
			if (distance_squared2d((real_point2d *)&other_prop->body_position, (real_point2d *)&prop->body_position)<2.5f * 2.5f &&
				fabs(other_prop->body_position.z - prop->body_position.z)<1.5f &&
				dot_product3d(&prop->actor_to_prop, &other_prop->actor_to_prop)>0.5f)
			{
				expected = TRUE;
			}
		}
	}

	return expected;
}

long actor_perception_find_killer_prop_index(
	long actor_index,
	long killed_prop_index,
	boolean skip_friendlies)
{
	short attacker_index;
	struct prop_datum *killed_prop = prop_get(killed_prop_index);
	struct unit_datum *unit = unit_get(killed_prop->unit_index);
	long killer_prop_index = NONE;
	long killer_time = 0;

	for (attacker_index = 0; attacker_index<MAXIMUM_ATTACKERS_PER_UNIT; attacker_index++)
	{
		long attacker_time = unit->unit.attackers[attacker_index].game_time_stamp;
		long attacker_unit_index = ai_get_responsible_unit(unit->unit.attackers[attacker_index].object_index, TRUE);

		if (attacker_unit_index!=NONE)
		{
			long prop_index = prop_get_active_by_unit_index(actor_index, attacker_unit_index);

			if (prop_index!=NONE)
			{
				struct prop_datum *prop = prop_get(prop_index);

				if (prop_acknowledged(prop) && (prop->enemy || !skip_friendlies) && attacker_time>killer_time)
				{
					killer_prop_index = prop_index;
					killer_time = attacker_time;
				}
			}
		}
	}

	return killer_prop_index;
}

long actor_perception_find_recent_damaging_prop_index(
	long actor_index,
	boolean skip_friendlies)
{
	struct actor_datum *actor = actor_get(actor_index);
	long damaging_prop_index = NONE;

	if (actor->meta.unit_index!=NONE)
	{
		short attacker_index;
		struct unit_datum *unit = unit_get(actor->meta.unit_index);
		unsigned long damaging_time = 0;

		for (attacker_index = 0; attacker_index<MAXIMUM_ATTACKERS_PER_UNIT; attacker_index++)
		{
			long attacker_unit_index = ai_get_responsible_unit(unit->unit.attackers[attacker_index].object_index, TRUE);

			if (attacker_unit_index!=NONE)
			{
				long prop_index = prop_get_active_by_unit_index(actor_index, attacker_unit_index);

				if (prop_index!=NONE)
				{
					struct prop_datum *prop = prop_get(prop_index);

					if (prop_acknowledged(prop) && (prop->enemy || !skip_friendlies) && unit->unit.attackers[attacker_index].game_time_stamp>damaging_time)
					{
						damaging_prop_index = prop_index;
						damaging_time = unit->unit.attackers[attacker_index].game_time_stamp;
					}
				}
			}
		}

		match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 3726, damaging_prop_index != 0x00000000);
	}

	return damaging_prop_index;
}

boolean actor_perception_create_orphan_from_friend(
	long actor_index,
	long unit_index,
	long friend_actor_index,
	long friend_prop_index)
{
	boolean success = TRUE;
	long current_prop_index = prop_get_base_by_unit_index(actor_index, unit_index, TRUE, FALSE);

	if (current_prop_index!=NONE)
	{
		struct prop_datum *current_prop = prop_get(current_prop_index);

		if (prop_acknowledged(current_prop))
		{
			success = FALSE;
		}
		else if (current_prop->orphan_prop_index!=NONE)
		{
			struct actor_position_data sense_position;
			long current_orphan_index = current_prop->orphan_prop_index;
			struct prop_datum *current_orphan = prop_get(current_orphan_index);
			boolean forced_position_update = FALSE;

			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 3764, prop_unacknowledged(current_prop));
			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 3765, prop_orphaned(current_orphan));
			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 3766, current_prop->owner_actor_index == actor_index);
			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 3767, current_orphan->owner_actor_index == actor_index);
			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 3768, current_prop->orphan_prop_index == current_orphan_index);
			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 3769, current_orphan->parent_prop_index == current_prop_index);

			if (friend_prop_index!=NONE)
			{
				prop_orphan_update_information(actor_index, current_orphan_index, friend_prop_index);
				current_prop->unit_index = current_orphan->unit_index;
			}
			else
			{
				current_orphan->state = _prop_state_uninspected_orphan;
				current_orphan->orphan_inspection_ticks = 0;
				forced_position_update = TRUE;
			}

			prop_position_refresh(actor_index, current_orphan_index, &sense_position, forced_position_update, TRUE);
			prop_status_refresh(actor_index, current_orphan_index, &sense_position);
			current_prop_index = current_orphan_index;
			current_prop = prop_get(current_orphan_index);
		}
		else
		{
			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 3807, prop_unacknowledged(current_prop));

			if (friend_prop_index!=NONE)
			{
				long new_prop_index = prop_orphan_from_friend(actor_index, current_prop_index, friend_prop_index);

				if (new_prop_index!=NONE)
				{
					struct prop_datum *new_prop = prop_get(new_prop_index);

					new_prop->unit_index = current_prop->unit_index;
					new_prop->actor_index = current_prop->actor_index;
					new_prop->swarm = current_prop->swarm;
				}

				current_prop_index = new_prop_index;
			}
			else
			{
				struct actor_position_data sense_position;

				prop_position_refresh(actor_index, current_prop_index, &sense_position, FALSE, FALSE);
				current_prop_index = prop_orphan_transition(actor_index, current_prop_index);
			}

			if (current_prop_index==NONE)
			{
				current_prop = NULL;
				success = FALSE;
			}
			else
			{
				current_prop = prop_get(current_prop_index);
			}
		}

		if (current_prop)
		{
			boolean definitely_located = FALSE;

			if (friend_actor_index==NONE)
			{
				definitely_located = TRUE;
			}
			else if (friend_prop_index!=NONE)
			{
				struct prop_datum *friend_prop = prop_get(friend_prop_index);

				if (friend_prop->visibility>=_actor_perception_full)
				{
					definitely_located = TRUE;
				}
			}

			if (definitely_located)
			{
				current_prop->definitely_located = TRUE;
				current_prop->ticks_since_definitely_located = 0;
				current_prop->definite_knowledge_source_actor = friend_actor_index;
			}

			current_prop->unopposable_enemy = actor_compute_prop_unopposable(actor_index, current_prop_index);
			current_prop->target_weight = actor_compute_prop_target_weight(actor_index, current_prop_index);
		}
	}

	return success;
}

void actor_perception_forget_recent_damage(
	long actor_index)
{
	struct prop_iterator iterator;
	struct prop_datum *prop;

	prop_iterator_new(&iterator, actor_index);

	while (prop = prop_iterator_next(&iterator))
	{
		prop->currently_damaging_me = FALSE;
		prop->ticks_since_damage = NONE;
	}

	return;
}

void actor_perception_retreat_successful(
	long actor_index)
{
	struct prop_iterator iterator;
	struct prop_datum *prop;
	struct actor_datum *actor = actor_get(actor_index);

	prop_iterator_new(&iterator, actor_index);

	while (prop = prop_iterator_next(&iterator))
	{
		prop->unopposable_trigger_hysteresis = 0;
		prop->unopposable_trigger_threshold = 0;
		prop->unopposable_trigger_timer = 0;
	}

	return;
}

void actor_perception_unreachable(
	long actor_index,
	long prop_index,
	boolean unreachable)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *prop = prop_get(prop_index);

	if (unreachable)
	{
		if (!prop->unreachable_ticks)
		{
			prop->unreachable_ticks = 1;
		}

		prop->last_unreachable_time = game_time_get();
	}
	else
	{
		prop->unreachable_ticks = 0;
		prop->last_unreachable_time = NONE;
	}

	prop->unopposable_enemy = actor_compute_prop_unopposable(actor_index, prop_index);
	prop->target_weight = actor_compute_prop_target_weight(actor_index, prop_index);

	return;
}

void actor_perception_tried_to_uncover(
	long actor_index,
	long prop_index)
{
	if (prop_index!=NONE)
	{
		struct actor_datum *actor = actor_get(actor_index);
		struct prop_datum *prop = prop_get(prop_index);

		prop->tried_to_uncover = TRUE;

		if (prop_index==actor->target.target_prop_index)
		{
			actor_situation_update_target_status(actor_index);
			actor_situation_combat_status_update(actor_index);
		}
	}

	return;
}

void actor_perception_tried_to_search(
	long actor_index,
	long prop_index)
{
	if (prop_index!=NONE)
	{
		struct actor_datum *actor = actor_get(actor_index);
		struct prop_datum *prop = prop_get(prop_index);

		prop->tried_to_search = TRUE;

		if (prop_index==actor->target.target_prop_index)
		{
			actor_situation_update_target_status(actor_index);
			actor_situation_combat_status_update(actor_index);
		}
	}

	return;
}

void actor_perception_abandoned_search(
	long actor_index,
	long prop_index)
{
	if (prop_index==NONE)
	{
		struct actor_datum *actor = actor_get(actor_index);

		actor->firing_positions.pursuit_positions_count = 0;
		actor->firing_positions.pursuit_fired_at_orphan = FALSE;
		actor->firing_positions.pursuit_communicated_lost_contact = FALSE;
		actor->state.artificial_combat_status = _actor_combat_status_none;
		actor->state.suspicion_combat_status = _actor_combat_status_none;
		actor_situation_combat_status_update(actor_index);
	}
	else
	{
		struct actor_datum *actor = actor_get(actor_index);
		struct prop_datum *prop = prop_get(prop_index);

		if (prop->state==_prop_state_uninspected_orphan)
		{
			prop->state = _prop_state_inspected_orphan;
		}

		prop->abandoned_search = TRUE;

		if (prop_index==actor->target.target_prop_index)
		{
			actor_situation_update_target_status(actor_index);
			actor_situation_combat_status_update(actor_index);
		}
	}

	return;
}

boolean actor_compute_prop_unopposable(
	long actor_index,
	long prop_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *prop = prop_get(prop_index);
	boolean unopposable = FALSE;

	if (prop_acknowledged(prop) && prop->enemy && !prop->dead)
	{
		if (prop->unreachable_ticks && (actor->target.target_prop_index==prop_index || !actor->situation.visible_reachable_enemies))
		{
			unopposable = TRUE;
		}
		else if ((prop->vehicle_gunner || prop->dangerous_vehicle_driver) && !actor->input.vehicle_gunner && !actor->situation.area_fire_support_friends)
		{
			unopposable = TRUE;
		}
		else if (prop->type==_actor_mounted_weapon)
		{
			unopposable = TRUE;
		}
	}

	if (prop->unopposable_enemy && !unopposable)
	{
		prop->unopposable_trigger_hysteresis = 0;
		prop->unopposable_trigger_threshold = 0;
		prop->unopposable_trigger_timer = 0;
	}

	if (prop_acknowledged(prop) && !unopposable && actor->emotions.unopposable_retreat_timer>0 && actor->emotions.unopposable_retreat_prop_index==prop_index)
	{
		actor->emotions.unopposable_retreat_timer = 0;
		actor->emotions.unopposable_retreat_prop_index = NONE;
	}

	prop->unopposable_enemy = unopposable;

	return unopposable;
}

real actor_compute_prop_target_weight(
	long actor_index,
	long prop_index)
{
	real weight;
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *prop = prop_get(prop_index);

	if (prop->ignore ||
		!prop->enemy ||
		prop_unacknowledged(prop) ||
		(prop->dead && prop->dead_ticks>=5 * TICKS_PER_SECOND) ||
		prop->type==_actor_mounted_weapon)
	{
		weight = 0.f;
	}
	else
	{
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
		struct actor_variant_definition *actor_variant = actor_variant_definition_get(actor->meta.variant_definition_index);
		short status_weight = 0;
		short distance_weight = 0;
		short current_target_weight = 0;
		short preferred_target_weight = 0;
		real bonus_weight = 0.f;

		if (actor->meta.swarm)
		{
			distance_weight = 0;
		}
		else if (prop->unreachable_ticks>0)
		{
			distance_weight = 0;
		}
		else if (!actor_has_ranged_weapon(actor_index))
		{
			real melee_range = actor->emotions.berserk ? actor_variant->ranged_combat.melee_range : actor_variant->ranged_combat.berserk_melee_range;

			if (prop->distance<2.f && prop->state!=_prop_state_inspected_orphan)
			{
				distance_weight = 5;
			}
			else if (prop->vehicle_index!=NONE)
			{
				distance_weight = 0;
			}
			else if (prop->flying && actor_definition->berserk.melee_leap_velocity==0.f)
			{
				distance_weight = 0;
			}
			else if (prop->underwater!=actor->input.underwater)
			{
				distance_weight = 1;
			}
			else if (prop->distance<melee_range)
			{
				distance_weight = 3;
			}
			else
			{
				distance_weight = 2;
			}
		}
		else
		{
			struct weapon_definition *weapon_definition = actor_get_weapon_definition(actor_index);
			struct actor_variant_definition *firing_variant = actor_combat_get_firing_variant_definition(actor_index);

			if (weapon_definition && prop->distance<weapon_definition->weapon.ai_minimum_target_range)
			{
				distance_weight = 2;
			}
			else if (prop->underwater!=actor->input.underwater)
			{
				distance_weight = 2;
			}
			else if (prop->distance<2.f && prop->state!=_prop_state_inspected_orphan)
			{
				distance_weight = 5;
			}
			else if (prop->distance<firing_variant->ranged_combat.combat_range_upper_bound)
			{
				distance_weight = 3;
			}
			else if (prop->distance<firing_variant->ranged_combat.maximum_firing_range)
			{
				distance_weight = 2;
			}
			else
			{
				distance_weight = 1;
			}
		}

		if (prop->dead)
		{
			status_weight = 1;
		}
		else if (!actor->meta.swarm && prop->currently_damaging_me && !prop->unreachable_ticks)
		{
			status_weight = 6;
		}
		else if (prop_acknowledged(prop))
		{
			if (actor->meta.swarm)
			{
				status_weight = 4;
			}
			else if (prop->unreachable_ticks>0)
			{
				status_weight = 3;
			}
			else if (prop->line_of_sight!=_ai_line_of_sight_clear && prop->line_of_sight!=_ai_line_of_sight_occluded)
			{
				status_weight = 3;
			}
			else if (prop->shooting && prop->quantized_facing<=_prop_facing_nearby)
			{
				status_weight = 5;
			}
			else
			{
				status_weight = 4;
			}
		}
		else
		{
			match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 4230, prop_orphaned(prop));

			if (prop->definitely_located)
			{
				status_weight = 3;
			}
			else if (prop->state==_prop_state_uninspected_orphan)
			{
				status_weight = 2;
			}
			else
			{
				status_weight = 1;
			}
		}

		if (actor->target.target_prop_index==NONE)
		{
			if (prop->player || prop_index==actor->meta.interesting_orphan_index)
			{
				bonus_weight += 3.f;
			}
		}
		else if (prop_index==actor->target.target_prop_index && actor->state.combat_status>=_actor_combat_status_definite)
		{
			current_target_weight = 1;
		}

		if (prop->preferred_target)
		{
			preferred_target_weight = 2;
		}

		weight = (real)(preferred_target_weight + current_target_weight + distance_weight + status_weight) * 10.f +
			(5.f / (0.1f * prop->distance + 1.f) + bonus_weight);
	}

	return weight;
}

void actor_situation_update_target_status(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->target.target_prop_index==NONE)
	{
		actor->target.target_type = _actor_target_none;
		actor->target.target_last_visible_time = NONE;
		actor->target.target_really_alive = FALSE;
	}
	else
	{
		short current_target_type;
		struct prop_datum *target_prop = prop_get(actor->target.target_prop_index);
		struct unit_datum *target_unit = unit_get(target_prop->unit_index);

		match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 4291, target_prop->enemy);

		switch (target_prop->state)
		{
			case _prop_state_unacknowledged:
				current_target_type = _actor_target_none;
				actor->target.target_prop_index = NONE;
				actor->target.target_last_visible_time = NONE;
				break;

			case _prop_state_becoming_acknowledged:
				current_target_type = _actor_target_partial_enemy;
				break;

			case _prop_state_becoming_unacknowledged:
			case _prop_state_acknowledged:
				if (target_prop->dead)
				{
					current_target_type = _actor_target_dead_enemy;
				}
				else if (target_prop->currently_damaging_me)
				{
					current_target_type = _actor_target_damaging_enemy;
				}
				else if (target_prop->visibility>=_actor_perception_full)
				{
					current_target_type = _actor_target_visible_enemy;
				}
				else if (target_prop->line_of_sight!=_ai_line_of_sight_clear && target_prop->line_of_sight!=_ai_line_of_sight_occluded)
				{
					current_target_type = _actor_target_acknowledged_enemy;
				}
				else if (target_prop->quantized_facing<=_prop_facing_central && target_prop->distance<6.f)
				{
					current_target_type = _actor_target_potentially_dangerous_enemy;
				}
				else
				{
					current_target_type = _actor_target_clear_line_of_sight_enemy;
				}
				break;

			case _prop_state_inspected_orphan:
				if (target_prop->dead)
				{
					current_target_type = _actor_target_dead_enemy;
				}
				else if (target_prop->abandoned_search)
				{
					current_target_type = _actor_target_disregarded_orphan;
				}
				else
				{
					current_target_type = _actor_target_inspected_orphan;
				}
				break;

			case _prop_state_uninspected_orphan:
				if (target_prop->definitely_located)
				{
					current_target_type = _actor_target_definite_orphan;
				}
				else
				{
					current_target_type = _actor_target_uninspected_orphan;
				}
				break;

			default:
				match_halt("c:\\halo\\SOURCE\\ai\\actor_perception.c", 4362);
				break;
		}

		actor->target.target_type = current_target_type;

		if (prop_acknowledged(target_prop))
		{
			actor->target.target_really_alive = !target_prop->dead;

			if (target_prop->visibility>_actor_perception_none)
			{
				actor->target.target_last_visible_time = target_prop->last_visible_time;
			}
		}
		else
		{
			actor->target.target_really_alive = !TEST_FLAG(target_unit->object.damage_flags, _object_dead_bit);
		}
	}

	return;
}

void actor_situation_combat_status_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->stimuli.suspicion_combat_status>_actor_combat_status_none)
	{
		if (actor->state.suspicion_combat_status<actor->stimuli.suspicion_combat_status)
		{
			actor->state.suspicion_combat_status = actor->stimuli.suspicion_combat_status;
			actor->state.suspicion_timer = actor->stimuli.suspicion_timer;
		}
		else if (actor->state.suspicion_combat_status==actor->stimuli.suspicion_combat_status)
		{
			actor->state.suspicion_timer = MAX(actor->state.suspicion_timer, actor->stimuli.suspicion_timer);
		}

		actor->stimuli.suspicion_combat_status = _actor_combat_status_none;
	}

	match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 4408, (actor->target.target_type >= 0) && (actor->target.target_type < NUMBER_OF_ACTOR_TARGET_TYPES));

	actor->state.combat_status = MAX(actor->state.suspicion_combat_status, MAX(actor->state.artificial_combat_status, global_combat_status_table[actor->target.target_type]));

	if (actor->state.combat_status>actor->state.suspicion_combat_status)
	{
		actor->state.suspicion_combat_status = _actor_combat_status_none;
	}

	if (actor->state.mode<_actor_mode_combat)
	{
		actor->state.combat_mode_timer = 0;
	}
	else
	{
		actor->state.combat_mode_timer++;
	}

	if (actor->state.combat_status==_actor_combat_status_none)
	{
		actor->state.in_combat_timer = 0;
	}
	else
	{
		actor->state.in_combat_timer++;
	}

	if (actor->state.combat_status>=_actor_combat_status_certain)
	{
		actor->state.certain_combat_timer++;
		actor->state.uncertain_combat_timer = 0;
	}
	else
	{
		actor->state.certain_combat_timer = 0;

		if (actor->state.uncertain_combat_timer!=NONE)
		{
			actor->state.uncertain_combat_timer++;
		}
	}

	if (actor->state.combat_status>=_actor_combat_status_visible)
	{
		actor->state.had_visible_enemy = TRUE;
	}

	return;
}

void actor_situation_update(
	long actor_index)
{
	struct prop_iterator iterator;
	struct prop_datum *prop;
	struct actor_datum *actor = actor_get(actor_index);
	long best_prop_index = NONE;
	real best_target_weight = 0.f;
	boolean ignore_close_enemies = actor->emotions.berserk || actor->state.action==_actor_action_charge;

	memset(&actor->situation, 0, sizeof(actor->situation));
	prop_iterator_new(&iterator, actor_index);

	while (prop = prop_iterator_next(&iterator))
	{
		if (prop_acknowledged(prop) && !prop->dead)
		{
			if (prop->enemy)
			{
				boolean visible = prop->visibility>=_actor_perception_full;
				short threat_type = _actor_threat_none;

				actor->situation.known_enemies++;

				if (visible && !prop->unreachable_ticks)
				{
					actor->situation.visible_reachable_enemies++;
				}

				if (visible || (prop->shooting && prop->line_of_sight==_ai_line_of_sight_clear))
				{
					if (visible)
					{
						actor->situation.cumulative_threats[_actor_threat_visible]++;
						threat_type = _actor_threat_visible;
					}

					if (prop->currently_damaging_me)
					{
						actor->situation.cumulative_threats[_actor_threat_damaging_me]++;
						threat_type = MAX(threat_type, _actor_threat_damaging_me);
					}

					if (prop->shooting)
					{
						actor->situation.cumulative_threats[_actor_threat_shooting]++;
						threat_type = MAX(threat_type, _actor_threat_shooting);
					}

					if (prop->quantized_facing<=_prop_facing_central)
					{
						if (visible)
						{
							actor->situation.cumulative_threats[_actor_threat_visible_facing_me]++;
							threat_type = MAX(threat_type, _actor_threat_visible_facing_me);

							if (!ignore_close_enemies && prop->distance<2.f)
							{
								actor->situation.cumulative_threats[_actor_threat_extremely_close_to_me]++;
								threat_type = MAX(threat_type, _actor_threat_extremely_close_to_me);
							}
						}

						if (prop->quantized_facing<=_prop_facing_nearby)
						{
							if (prop->shooting)
							{
								actor->situation.cumulative_threats[_actor_threat_shooting_near_me]++;
								threat_type = MAX(threat_type, _actor_threat_shooting_near_me);
							}

							if (prop->quantized_facing<=_prop_facing_direct)
							{
								if (visible)
								{
									actor->situation.cumulative_threats[_actor_threat_visible_aiming_at_me]++;
									threat_type = MAX(threat_type, _actor_threat_visible_aiming_at_me);
								}

								if (prop->shooting)
								{
									actor->situation.cumulative_threats[_actor_threat_shooting_at_me]++;
									threat_type = MAX(threat_type, _actor_threat_shooting_at_me);
								}
							}
						}
					}
				}

				actor->situation.specific_threats[threat_type]++;
			}
			else
			{
				struct unit_datum *unit = unit_get(prop->unit_index);
				struct actor_datum *friend_actor = unit->unit.actor_index==NONE ? NULL : actor_get(unit->unit.actor_index);
				short actor_type = unit->unit.player_index!=NONE ? _actor_player : (friend_actor ? friend_actor->meta.type : _actor_none);
				boolean area_friend = FALSE;
				boolean visible_friend = FALSE;
				boolean close_friend = FALSE;

				match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 4572, (actor_type >= 0) && (actor_type < NUMBER_OF_ACTOR_TYPES));

				if (prop->distance<8.f)
				{
					area_friend = TRUE;
				}
				else if (prop->fighting && friend_actor && actor->target.target_prop_index!=NONE && friend_actor->target.target_prop_index!=NONE)
				{
					struct prop_datum *target_prop = prop_get(actor->target.target_prop_index);
					struct prop_datum *friend_target_prop = prop_get(friend_actor->target.target_prop_index);

					if (target_prop->unit_index==friend_target_prop->unit_index)
					{
						area_friend = TRUE;
					}
				}

				if (prop->line_of_sight==_ai_line_of_sight_clear || prop->line_of_sight==_ai_line_of_sight_occluded)
				{
					visible_friend = TRUE;
					close_friend = prop->distance<3.f;
				}

				if (area_friend)
				{
					actor->situation.area_friends++;

					if (prop->fighting)
					{
						actor->situation.area_fighting_friends++;
					}

					if (prop->fighting && prop->vehicle_gunner)
					{
						actor->situation.area_fire_support_friends++;
					}

					actor->situation.area_friends_by_type[actor_type]++;

					if (prop->fighting)
					{
						actor->situation.area_fighting_friends_by_type[actor_type]++;
					}
				}

				if (visible_friend)
				{
					actor->situation.visible_friends++;

					if (prop->fighting)
					{
						actor->situation.visible_fighting_friends++;
					}

					actor->situation.visible_friends_by_type[actor_type]++;

					if (prop->fighting)
					{
						actor->situation.visible_fighting_friends_by_type[actor_type]++;
					}
				}

				if (close_friend)
				{
					actor->situation.close_friends++;

					if (prop->fighting)
					{
						actor->situation.close_fighting_friends++;
					}

					actor->situation.close_friends_by_type[actor_type]++;

					if (prop->fighting)
					{
						actor->situation.close_fighting_friends_by_type[actor_type]++;
					}
				}
			}
		}

		if (prop->target_weight>best_target_weight)
		{
			best_prop_index = iterator.index;
			best_target_weight = prop->target_weight;
		}
	}

	if (best_prop_index!=actor->target.target_prop_index)
	{
		long old_prop_index = actor->target.target_prop_index;

		actor->target.target_type = _actor_target_none;
		actor->target.target_prop_index = best_prop_index;
		actor->target.target_last_visible_time = NONE;

		if (old_prop_index!=NONE)
		{
			struct prop_datum *old_prop = prop_get(old_prop_index);

			old_prop->target_weight = actor_compute_prop_target_weight(actor_index, old_prop_index);
		}

		if (best_prop_index!=NONE)
		{
			struct prop_datum *best_prop = prop_get(best_prop_index);

			best_prop->target_weight = actor_compute_prop_target_weight(actor_index, actor->target.target_prop_index);
		}
	}

	actor_situation_update_target_status(actor_index);
	actor_situation_combat_status_update(actor_index);

	return;
}

boolean actor_situation_try_new_target(
	long actor_index,
	long prop_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *new_prop = prop_get(prop_index);
	struct prop_datum *target_prop = actor->target.target_prop_index==NONE ? NULL : prop_get(actor->target.target_prop_index);
	boolean result = FALSE;

	new_prop->target_weight = actor_compute_prop_target_weight(actor_index, prop_index);

	if (new_prop->target_weight>0.f)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 4685, new_prop->enemy);

		if (!target_prop || new_prop->target_weight>=target_prop->target_weight)
		{
			actor->target.target_type = _actor_target_none;
			actor->target.target_prop_index = prop_index;
			actor->target.target_last_visible_time = NONE;
			actor_situation_update_target_status(actor_index);
			actor_situation_combat_status_update(actor_index);
			result = TRUE;
		}
	}

	return result;
}

boolean actor_perception_friend_prop_is_attacking(
	long actor_index,
	long friend_prop_index,
	real_vector3d *attack_vector)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *friend_prop = prop_get(friend_prop_index);
	boolean attacking = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\actor_perception.c", 4712, prop_acknowledged(friend_prop) && !friend_prop->enemy && !friend_prop->dead);

	if (friend_prop->swarm)
	{
		attacking = FALSE;
	}
	else if (friend_prop->player)
	{
		attacking = friend_prop->shooting;
		unit_get_aiming_vector(friend_prop->unit_index, attack_vector);

		if (!attacking && actor->situation.known_enemies>0)
		{
			struct prop_iterator enemy_iterator;
			struct prop_datum *enemy_prop;

			prop_iterator_new(&enemy_iterator, actor_index);

			while (enemy_prop = prop_iterator_next(&enemy_iterator))
			{
				if (prop_acknowledged(enemy_prop) && enemy_prop->enemy)
				{
					real_vector3d vector_from_friend_to_enemy;

					vector_from_points3d(&friend_prop->body_position, &enemy_prop->body_position, &vector_from_friend_to_enemy);

					if (normalize3d(&vector_from_friend_to_enemy)>0.f && dot_product3d(&vector_from_friend_to_enemy, attack_vector)>0.5f)
					{
						attacking = TRUE;
						break;
					}
				}
			}
		}
	}
	else if (friend_prop->actor_index!=NONE)
	{
		attacking = actor_attacking_target(friend_prop->actor_index, attack_vector);
	}

	return attacking;
}

short actor_perception_aiming_vector_test_blockage(
	real_point3d const *source_position,
	real_vector3d const *source_vector,
	real_point3d const *friend_position,
	real_vector3d *friend_direction_to_aiming_vector)
{
	short result = _actor_aiming_clear;
	real_vector2d source_planar_direction = *(real_vector2d *)source_vector;

	if (normalize2d(&source_planar_direction)>0.f)
	{
		real_vector3d friend_vector;
		real planar_distance;
		real planar_projection;

		vector_from_points3d(source_position, friend_position, &friend_vector);
		planar_distance = magnitude2d((real_vector2d *)&friend_vector);
		planar_projection = dot_product2d((real_vector2d *)&friend_vector, &source_planar_direction);

		if (planar_projection>_cosine30 * planar_distance)
		{
			real_point3d friend_perpendicular_vector;

			point_from_line3d((real_point3d *)&friend_vector, source_vector, -planar_projection, &friend_perpendicular_vector);

			if (friend_direction_to_aiming_vector)
			{
				negate_vector3d((real_vector3d *)&friend_perpendicular_vector, friend_direction_to_aiming_vector);
			}

			if (friend_perpendicular_vector.z>-0.5f && friend_perpendicular_vector.z<0.9f)
			{
				result = _actor_aiming_blocked;
			}
			else if (friend_perpendicular_vector.z>-0.8f && friend_perpendicular_vector.z<1.2f)
			{
				result = _actor_aiming_occluded;
			}
			else
			{
				result = _actor_aiming_clear;
			}

			if (result>_actor_aiming_clear)
			{
				real distance_squared = magnitude_squared2d((real_vector2d *)&friend_perpendicular_vector);

				if (distance_squared<0.36f)
				{
				}
				else if (distance_squared<1.21f)
				{
					result = _actor_aiming_occluded;
				}
				else
				{
					result = _actor_aiming_clear;
				}
			}
		}
	}

	return result;
}

boolean actor_emotion_flee_with_friends(
	long actor_index,
	real *flee_chance)
{
	struct prop_iterator iterator;
	struct prop_datum *prop;
	struct actor_datum *actor = actor_get(actor_index);
	boolean flee = FALSE;
	short fleeing_friends = 0;
	short fighting_friends = 0;

	prop_iterator_new(&iterator, actor_index);

	while (prop = prop_iterator_next(&iterator))
	{
		if (prop_acknowledged(prop) && !prop->enemy && prop->type==actor->meta.type && prop->actor_index!=NONE)
		{
			struct actor_datum *friend_actor = actor_get(prop->actor_index);

			if (friend_actor->stimuli.panic_type>_actor_panic_none ||
				(friend_actor->state.action==_actor_action_flee && friend_actor->state.action_data.flee.panic_type>_actor_panic_none))
			{
				fleeing_friends++;
			}
			else if (prop->in_combat)
			{
				fighting_friends++;
			}
		}
	}

	if (fleeing_friends>1)
	{
		flee = TRUE;
	}
	else
	{
		real modifier = 1.f;

		if (fighting_friends>1)
		{
			modifier = 1.f - (fighting_friends - 1) * 0.25f;
		}
		else
		{
			modifier = (1 - fighting_friends) * 0.5f + 1.f;
		}

		*flee_chance *= PIN(modifier, 0.f, 2.f);
	}

	return flee;
}

static short actor_emotion_get_unopposable_enemy(
	long actor_index,
	long unit_index,
	short *enemy_count,
	short maximum_enemy_count,
	struct actor_unopposable_enemy *enemies)
{
	short index;
	short enemy_index = NONE;
	short count = *enemy_count;

	for (index = 0; index<count; index++)
	{
		if (enemies[index].unit_index==unit_index)
		{
			enemy_index = index;
			break;
		}
	}

	if (enemy_index==NONE && count<maximum_enemy_count)
	{
		enemy_index = *enemy_count;
		*enemy_count = count + 1;
		enemies[count].unopposable_type = _actor_unopposable_danger_none;
		enemies[count].prop_index = NONE;
		enemies[count].unit_index = NONE;
		enemies[count].prop = NULL;
		enemies[count].friends_retreating = 0;
		enemies[count].closest_retreating_friend_dist_sq = REAL_MAX;
		enemies[count].closest_retreating_friend_actor_index = NONE;
	}

	return enemy_index;
}

static short actor_emotion_assess_unopposable_danger(
	long actor_index,
	long prop_index)
{
	struct prop_datum *prop = prop_get(prop_index);
	short danger = _actor_unopposable_danger_none;

	if (prop_acknowledged(prop) && prop->unopposable_enemy)
	{
		if (prop->currently_damaging_me)
		{
			danger = _actor_unopposable_danger_damaging;
		}
		else if (prop->shooting)
		{
			if (prop->quantized_facing>_prop_facing_nearby)
			{
				danger = _actor_unopposable_danger_shooting;
			}
			else
			{
				danger = _actor_unopposable_danger_shooting_nearby;
			}
		}
		else if (prop->visibility>=_actor_perception_full)
		{
			danger = _actor_unopposable_danger_visible;
		}
	}

	return danger;
}

static void actor_emotion_unopposable_retreat(
	long actor_index)
{
	struct actor_unopposable_enemy enemies[MAXIMUM_UNOPPOSABLE_ENEMIES_PER_ACTOR];
	short enemy_index;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	short maximum_enemy_count = NUMBEROF(enemies);
	short enemy_count = 0;

	{
		struct prop_iterator iterator;
		struct prop_datum *prop;

		prop_iterator_new(&iterator, actor_index);

		while (prop = prop_iterator_next(&iterator))
		{
			short danger = actor_emotion_assess_unopposable_danger(actor_index, iterator.index);

			if (danger>_actor_unopposable_danger_none)
			{
				enemy_index = actor_emotion_get_unopposable_enemy(actor_index, prop->unit_index, &enemy_count, maximum_enemy_count, enemies);

				if (enemy_index!=NONE)
				{
					struct actor_unopposable_enemy *enemy = &enemies[enemy_index];

					if (enemy->unopposable_type<danger)
					{
						enemy->prop_index = iterator.index;
						enemy->unit_index = prop->unit_index;
						enemy->prop = prop;
						enemy->unopposable_type = danger;
					}
				}
			}
			else if (prop_acknowledged(prop) && !prop->enemy && prop->actor_index!=NONE && prop->distance<8.f)
			{
				struct actor_datum *friend_actor = actor_get(prop->actor_index);

				if (friend_actor->emotions.unopposable_retreat_timer &&
					friend_actor->emotions.unopposable_retreat_prop_index!=NONE &&
					(actor->emotions.unopposable_friend_ignore_time==NONE || friend_actor->emotions.unopposable_retreat_start_time>=actor->emotions.unopposable_friend_ignore_time))
				{
					struct prop_datum *friend_enemy_prop = prop_get(friend_actor->emotions.unopposable_retreat_prop_index);
					long enemy_prop_index = prop_get_active_by_unit_index(actor_index, friend_enemy_prop->unit_index);

					if (enemy_prop_index!=NONE)
					{
						struct prop_datum *enemy_prop = prop_get(enemy_prop_index);

						if (prop_acknowledged(enemy_prop) && enemy_prop->unopposable_enemy)
						{
							enemy_index = actor_emotion_get_unopposable_enemy(actor_index, friend_enemy_prop->unit_index, &enemy_count, maximum_enemy_count, enemies);

							if (enemy_index!=NONE)
							{
								struct actor_unopposable_enemy *enemy = &enemies[enemy_index];
								real friend_distance_squared = ((friend_enemy_prop->distance) * (friend_enemy_prop->distance));

								enemy->friends_retreating++;

								if (enemy->closest_retreating_friend_dist_sq>friend_distance_squared)
								{
									enemy->closest_retreating_friend_actor_index = prop->actor_index;
									enemy->closest_retreating_friend_dist_sq = friend_distance_squared;
								}

								if (enemy->prop_index==NONE)
								{
									enemy->prop_index = enemy_prop_index;
									enemy->unit_index = enemy_prop->unit_index;
									enemy->prop = enemy_prop;
								}
							}
						}
					}
				}
			}
		}
	}

	for (enemy_index = 0; enemy_index<enemy_count; enemy_index++)
	{
		struct actor_unopposable_enemy *enemy = &enemies[enemy_index];
		struct prop_datum *enemy_prop = enemy->prop;
		short trigger_level = actor_definition->unopposable.trigger_unreachable_level;
		boolean immediate_trigger = FALSE;
		boolean start_trigger = FALSE;

		if (enemy_prop->vehicle_gunner || enemy_prop->dangerous_vehicle_driver)
		{
			trigger_level = actor_definition->unopposable.trigger_vehicle_level;
		}

		if (enemy_prop->player && actor_definition->unopposable.trigger_player_level>0)
		{
			trigger_level = MIN(trigger_level, actor_definition->unopposable.trigger_player_level);
		}

		if (trigger_level>0 && enemy->unopposable_type>=trigger_level)
		{
			if (enemy_prop->player)
			{
				immediate_trigger = TRUE;
			}
			else
			{
				start_trigger = TRUE;
			}
		}
		else if (enemy_prop->player)
		{
			start_trigger = TRUE;
		}

		if (start_trigger)
		{
			enemy_prop->unopposable_trigger_hysteresis = 22;
		}

		if (enemy_prop->unopposable_trigger_hysteresis>0)
		{
			if (!enemy_prop->unopposable_trigger_timer)
			{
				enemy_prop->unopposable_trigger_threshold = (short)(real_random_range(
					actor_definition->unopposable.trigger_time_lower_bound,
					actor_definition->unopposable.trigger_time_upper_bound) * TICKS_PER_SECOND);
			}

			enemy_prop->unopposable_trigger_hysteresis--;
			enemy_prop->unopposable_trigger_timer++;
		}

		if (enemy_prop->visible_ticks>=45 || enemy->unopposable_type>=_actor_unopposable_danger_damaging)
		{
			if (enemy_prop->unopposable_trigger_threshold>0 && enemy_prop->unopposable_trigger_timer>=enemy_prop->unopposable_trigger_threshold)
			{
				enemy->unopposable_type = MAX(enemy->unopposable_type, _actor_unopposable_danger_time_triggered);
			}

			if (immediate_trigger)
			{
				enemy->unopposable_type = MAX(enemy->unopposable_type, _actor_unopposable_danger_immediately_triggered);
			}

			if (actor_definition->unopposable.casualties_trigger>0 &&
				enemy_prop->unopposable_casualties_inflicted>=actor_definition->unopposable.casualties_trigger)
			{
				enemy->unopposable_type = MAX(enemy->unopposable_type, _actor_unopposable_danger_friends_killed);
			}

			if (actor_definition->unopposable.retreating_friends_trigger>0 &&
				enemy->friends_retreating>=actor_definition->unopposable.retreating_friends_trigger)
			{
				enemy->unopposable_type = MAX(enemy->unopposable_type, _actor_unopposable_danger_friends_retreating);
			}
		}
	}

	if (actor->emotions.unopposable_retreat_timer>0)
	{
		actor->emotions.unopposable_retreat_timer--;

		if (!actor->emotions.unopposable_retreat_timer)
		{
			actor->emotions.unopposable_friend_ignore_time = game_time_get();
		}
	}
	else
	{
		short retreat_type = _actor_unopposable_danger_retreat;
		long retreat_prop_index = NONE;

		for (enemy_index = 0; enemy_index<enemy_count; enemy_index++)
		{
			struct actor_unopposable_enemy *enemy = &enemies[enemy_index];

			if (enemy->unopposable_type>retreat_type && enemy->prop_index!=NONE)
			{
				retreat_type = enemy->unopposable_type;
				retreat_prop_index = enemy->prop_index;
			}
		}

		if (retreat_prop_index!=NONE)
		{
			actor->emotions.unopposable_retreat_timer = (short)(real_random_range(
				actor_definition->unopposable.retreat_time_lower_bound,
				actor_definition->unopposable.retreat_time_upper_bound) * TICKS_PER_SECOND);
			actor->emotions.unopposable_retreat_prop_index = retreat_prop_index;
			actor->emotions.unopposable_retreat_start_time = game_time_get();
		}
	}

	return;
}

void actor_emotion_update(
	long actor_index)
{
	short threat_type;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	if (actor->emotions.berserk &&
		(!actor->state.combat_status ||
		actor->state.mode<_actor_mode_combat ||
		(actor->input.shield_vitality==1.f && actor->state.combat_status<_actor_combat_status_definite)))
	{
		actor_berserk(actor_index, FALSE);
	}

	if (actor->emotions.currently_defending!=actor->external_orders.defending)
	{
		short communication_type = actor->external_orders.defending ? _ai_communication_retreat : _ai_communication_advance;

		actor->emotions.currently_defending = actor->external_orders.defending;

		if (actor->meta.unit_index!=NONE)
		{
			ai_communication_event(communication_type, actor->meta.unit_index, NONE, NONE, NONE, NONE, NULL);
		}
	}

	actor->emotions.forced_to_charge = actor->emotions.berserk || TEST_FLAG(actor_definition->flags, _actor_definition_always_charge_bit);

	if (actor->input.vehicle_index!=NONE)
	{
		actor->emotions.forced_to_charge = FALSE;
	}
	else if (TEST_FLAG(actor_definition->flags, _actor_definition_charge_in_attacking_mode_bit) && !actor->emotions.currently_defending)
	{
		actor->emotions.forced_to_charge = TRUE;
	}

	for (threat_type = NUMBER_OF_ACTOR_THREAT_TYPES; threat_type>0; threat_type--)
	{
		if (actor->situation.specific_threats[threat_type]>0)
		{
			break;
		}
	}

	if (threat_type>=_actor_threat_damaging_me)
	{
		actor->emotions.instantaneous_danger = 2.f;
	}
	else if (threat_type>=_actor_threat_extremely_close_to_me)
	{
		actor->emotions.instantaneous_danger = 1.8f;
	}
	else if (threat_type>=_actor_threat_shooting_at_me)
	{
		actor->emotions.instantaneous_danger = 1.6f;
	}
	else if (threat_type>=_actor_threat_shooting_near_me)
	{
		actor->emotions.instantaneous_danger = 1.2f;
	}
	else if (threat_type>=_actor_threat_visible_aiming_at_me)
	{
		actor->emotions.instantaneous_danger = 0.7f;
	}
	else
	{
		actor->emotions.instantaneous_danger = 0.f;
	}

	actor->emotions.perceived_danger += (1.f - (real)exp((real)(-M_LN2 / (TICKS_PER_SECOND / 2)))) * (actor->emotions.instantaneous_danger - actor->emotions.perceived_danger);

	if (actor->external_orders.stand_down)
	{
		actor->emotions.original_body_vitality = actor->input.body_vitality;
	}

	if (TEST_FLAG(actor_definition->flags, _actor_definition_crouch_in_line_of_fire_bit) ||
		TEST_FLAG(actor_definition->flags, _actor_definition_avoid_friend_line_of_fire_bit))
	{
		if (actor->input.vehicle_index==NONE && actor->state.combat_status>=_actor_combat_status_definite)
		{
			boolean has_target;
			struct prop_iterator iterator;
			real_vector3d attack_vector;
			struct prop_datum *prop;

			actor->emotions.crouch_blocking_line_of_fire = FALSE;
			actor->emotions.crouch_blocking_player_line_of_fire = FALSE;
			actor->emotions.crouch_friends_in_line_of_fire = FALSE;
			actor->emotions.moving_into_player_line_of_fire = FALSE;
			has_target = actor->target.target_type>_actor_target_clear_line_of_sight_enemy;

			if (has_target)
			{
				struct prop_datum *target_prop = prop_get(actor->target.target_prop_index);

				attack_vector = target_prop->actor_to_prop;
			}

			prop_iterator_new(&iterator, actor_index);

			while (prop = prop_iterator_next(&iterator))
			{
				if (prop_acknowledged(prop) && !prop->enemy && !prop->dead && !prop->swarm && (prop->player || prop->vehicle_index==NONE))
				{
					real_vector3d friend_attack_vector;

					if (actor_perception_friend_prop_is_attacking(actor_index, iterator.index, &friend_attack_vector))
					{
						real_vector3d vector_to_line_of_fire;
						short blockage = actor_perception_aiming_vector_test_blockage(&prop->body_position, &friend_attack_vector, &actor->input.position.body_position, &vector_to_line_of_fire);

						if (blockage>=_actor_aiming_occluded)
						{
							actor->emotions.crouch_blocking_line_of_fire = TRUE;

							if (prop->player)
							{
								actor->emotions.crouch_blocking_player_line_of_fire = TRUE;
							}
						}

						if (TEST_FLAG(actor_definition->flags, _actor_definition_avoid_friend_line_of_fire_bit) && prop->player && prop->shooting)
						{
							if (magnitude_squared3d(&vector_to_line_of_fire)<1.f && (actor->control.moving || actor->emotions.moving_into_fire_timer>0))
							{
								real_vector3d movement_direction = actor->control.moving_towards_vector;

								if (normalize3d(&movement_direction)>0.f)
								{
									real_point3d future_point;
									short future_blockage;
									short combined_blockage;

									point_from_line3d(&actor->input.position.body_position, &movement_direction, 0.4f, &future_point);
									future_blockage = actor_perception_aiming_vector_test_blockage(&prop->body_position, &friend_attack_vector, &future_point, NULL);
									combined_blockage = MAX(future_blockage, blockage);

									if (combined_blockage>=_actor_aiming_occluded)
									{
										real movement_into_fire = dot_product3d(&movement_direction, &vector_to_line_of_fire);
										real threshold = magnitude_squared3d(&vector_to_line_of_fire)<0.5f * 0.5f ? 0.f : 0.8660254f;

										if (movement_into_fire>threshold)
										{
											actor->emotions.moving_into_player_line_of_fire = TRUE;
										}
									}
								}
							}
						}
					}

					if (has_target && actor_perception_aiming_vector_test_blockage(&actor->input.position.body_position, &attack_vector, &prop->body_position, NULL)>=_actor_aiming_blocked)
					{
						actor->emotions.crouch_friends_in_line_of_fire = TRUE;
					}
				}
			}
		}
		else
		{
			actor->emotions.crouch_blocking_line_of_fire = FALSE;
			actor->emotions.crouch_blocking_player_line_of_fire = FALSE;
			actor->emotions.crouch_friends_in_line_of_fire = FALSE;
			actor->emotions.moving_into_player_line_of_fire = FALSE;
		}
	}

	if (actor->emotions.moving_into_player_line_of_fire)
	{
		actor_discard_firing_position(actor_index, actor->firing_positions.current_position_index, TRUE);
		actor->emotions.moving_into_fire_timer = 22;
	}
	else if (actor->emotions.moving_into_fire_timer>0)
	{
		actor->emotions.moving_into_fire_timer--;
	}

	if (actor->emotions.defensive_crouch_timer>0)
	{
		actor->emotions.defensive_crouch_timer--;
	}
	else
	{
		real threshold = actor->emotions.currently_defending && !actor->emotions.berserk ?
			actor_definition->defensive.defensive_threshold_defending :
			actor_definition->defensive.defensive_threshold_attacking;
		boolean crouch;

		switch (actor_definition->defensive.defensive_crouch_type)
		{
			case _defensive_crouch_danger:
				crouch = actor->emotions.perceived_danger>threshold;
				break;

			case _defensive_crouch_shield_low:
				crouch = actor->input.shield_vitality<threshold;
				break;

			case _defensive_crouch_hide_behind_shield:
				crouch = actor->input.shield_vitality>threshold && actor->situation.cumulative_threats[_actor_threat_visible_facing_me]>0;
				break;

			case _defensive_crouch_any_target:
				crouch = actor->state.combat_status>_actor_combat_status_none;
				break;

			case _defensive_crouch_flood_shamble:
				crouch = actor_type_flood_desire_shamble(actor_index);
				break;

			default:
				crouch = FALSE;
				break;
		}

		if (TEST_FLAG(actor_definition->flags, _actor_definition_crouch_in_line_of_fire_bit))
		{
			if (actor->emotions.crouch_blocking_player_line_of_fire)
			{
				crouch = TRUE;
			}
			else if (actor->emotions.crouch_friends_in_line_of_fire)
			{
				crouch = FALSE;
			}
			else if (actor->emotions.crouch_blocking_line_of_fire)
			{
				crouch = TRUE;
			}
		}

		if (actor->emotions.defensive_crouch && !crouch)
		{
			actor->emotions.defensive_crouch = FALSE;

			if (actor_definition->defensive.defensive_crouch_min_stand_time>0.f)
			{
				actor->emotions.defensive_crouch_timer = (short)(actor_definition->defensive.defensive_crouch_min_stand_time * TICKS_PER_SECOND);
			}
			else
			{
				actor->emotions.defensive_crouch_timer = 45;
			}
		}
		else if (!actor->emotions.defensive_crouch && crouch)
		{
			actor->emotions.defensive_crouch = TRUE;

			if (actor_definition->defensive.defensive_crouch_min_crouch_time>0.f)
			{
				actor->emotions.defensive_crouch_timer = (short)(actor_definition->defensive.defensive_crouch_min_crouch_time * TICKS_PER_SECOND);
			}
			else
			{
				actor->emotions.defensive_crouch_timer = 45;
			}
		}
	}

	if (actor->emotions.evasion_delay_timer>0)
	{
		actor->emotions.evasion_delay_timer--;
	}

	actor_emotion_unopposable_retreat(actor_index);

	return;
}

void actor_berserk(
	long actor_index,
	boolean berserk)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (berserk!=actor->emotions.berserk)
	{
		actor->emotions.berserk = berserk;
		actor->emotions.played_berserk_sound = FALSE;

		if (actor->meta.swarm)
		{
			long unit_index;

			for (unit_index = actor->meta.swarm_unit_index; unit_index!=NONE; )
			{
				struct unit_datum *unit = unit_get(unit_index);

				SET_FLAG(unit->object.damage_flags, _unit_ignore_hard_pings_bit, TRUE);
				unit_index = unit->unit.swarm_next_unit_index;
			}
		}
		else
		{
			struct unit_datum *unit = unit_get(actor->meta.unit_index);

			SET_FLAG(unit->unit.flags, _unit_ignore_hard_pings_bit, berserk);
		}

		if (berserk)
		{
			actor->emotions.forced_to_charge = TRUE;
		}
	}

	return;
}
