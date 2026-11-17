/*
ACTOR_TYPE_INFECTION.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "props.h"
#include "actor_types.h"
#include "actor_type_definitions.h"
#include "items.h"
#include "bipeds.h"
#include "projectiles.h"

/* ---------- constants */

enum
{
	_swarm_movement_none = 0,
	_swarm_movement_wander_noncombat,
	_swarm_movement_wander_idle,
	_swarm_movement_wander_combat,
	_swarm_movement_towards_prop,
	_swarm_movement_away_from_prop,
	_swarm_movement_obey,
	NUMBER_OF_SWARM_MOVEMENT_TYPES,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

void infection_decide_action(long actor_index);
static byte infection_wander_pause_time(short movement_type);
static byte infection_wander_move_time(short movement_type);
static void infection_swarm_control(long actor_index);
void infection_swarm_aim_jump(long actor_index, long unit_index, real jump_magnitude, real_vector3d *jump_velocity);

/* ---------- globals */

struct actor_type_definition actor_type_infection =
{
	"infection",
	_race_floodinfection,
	2,
	2,
	2,
	FALSE,
	TRUE,
	NULL,
	infection_decide_action,
	infection_swarm_control,
	infection_swarm_aim_jump
};

/* ---------- public code */

void infection_decide_action(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	actor_action_handle_initial_action(actor_index);
	actor_action_handle_pending_command_list(actor_index);
	if (!actor_action_deny_transition(actor_index))
	{
		actor_action_handle_combat_transition(actor_index);
	}

	switch (actor->state.action)
	{
		case _actor_action_fight:
		case _actor_action_charge:
			if (!actor_action_handle_combat_status(actor_index, TRUE, FALSE))
			{
				actor_action_handle_combat_failure(actor_index);
			}
			break;
		case _actor_action_flee:
			if (actor->state.action_data.flee.unable_to_flee)
			{
				actor_action_handle_combat_status(actor_index, TRUE, TRUE);
			}
			else
			{
				actor_action_handle_done_fleeing(actor_index);
			}
			break;
		case _actor_action_guard:
			actor_action_handle_combat_status(actor_index, actor_action_can_stop_guarding(actor_index, _actor_combat_status_definite, _actor_combat_status_dangerous), FALSE);
			break;
		case _actor_action_uncover:
		case _actor_action_search:
		case _actor_action_wait:
			if (!actor_action_handle_combat_status(actor_index, TRUE, FALSE))
			{
				actor_action_handle_exit_pursuit(actor_index);
			}
			break;
		case _actor_action_obey:
			actor_action_handle_combat_status(actor_index, actor->state.action_data.obey.initiative, actor->state.action_data.obey.finished);
			break;
	}

	return;
}

static byte infection_wander_pause_time(
	short movement_type)
{
	real time = 1.0f;
	short ticks;

	switch (movement_type)
	{
		case _swarm_movement_wander_noncombat:
			time = real_random_range(4.0f, 5.0f);
			break;

		case _swarm_movement_wander_idle:
			time = real_random_range(2.0f, 2.8f);
			break;

		case _swarm_movement_wander_combat:
			time = real_random_range(0.4f, 1.0f);
			break;
	}

	ticks = (short)(time * TICKS_PER_SECOND);

	return MIN(ticks, UNSIGNED_CHAR_MAX);
}

static byte infection_wander_move_time(
	short movement_type)
{
	real time = 1.0f;
	short ticks;

	switch (movement_type)
	{
		case _swarm_movement_wander_noncombat:
			time = real_random_range(1.0f, 2.5f);
			break;

		case _swarm_movement_wander_idle:
		case _swarm_movement_wander_combat:
			time = real_random_range(0.6f, 1.8f);
			break;
	}

	ticks = (short)(time * TICKS_PER_SECOND);

	return MIN(ticks, UNSIGNED_CHAR_MAX);
}

static void infection_swarm_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
	short intermittent_action_index = NONE;
	short swarm_unit_index;

	if (swarm->swarm_intermittent_action_timer > 0)
	{
		swarm->swarm_intermittent_action_timer--;
	}
	else
	{
		switch (actor->state.action)
		{
			case _actor_action_search:
			case _actor_action_charge:
			{
				real ticks = TICKS_PER_SECOND * (real_random_range(6.0f, 8.0f) / swarm->unit_count);

				swarm->swarm_intermittent_action_timer = (short)MAX(ticks, 6.0f);
				intermittent_action_index = random_range(0, swarm->unit_count);
				break;
			}
		}
	}

	for (swarm_unit_index = 0; swarm_unit_index < swarm->unit_count; swarm_unit_index++)
	{
		long unit_index = swarm->unit_indices[swarm_unit_index];
		struct unit_datum *unit = unit_get(unit_index);
		struct swarm_component_datum *swarm_component = swarm_component_get(swarm->component_indices[swarm_unit_index]);
		real_vector3d up_vector;
		short movement_type = _swarm_movement_none;
		long prop_index = NONE;
		short animation_state = _unit_animation_state_in_combat;
		short aiming_speed = 1;
		boolean moving_vertically = FALSE;
		boolean in_melee_range = FALSE;
		struct prop_datum *target_prop;
		boolean moving = FALSE;
		real_vector3d movement_vector;
		boolean jump = FALSE;
		boolean airborne;

		up_vector = unit->object.up;
		airborne = FALSE;
		if (unit->object.type == _object_type_biped)
		{
			struct biped_datum *biped = (struct biped_datum *)unit;

			if (biped->biped.support_surface_index != NONE)
			{
				up_vector = biped->biped.ground_plane.n;
			}
			airborne = TEST_FLAG(biped->biped.flags, _biped_airborne_bit);
		}

		if (actor->state.combat_status >= _actor_combat_status_definite)
		{
			real combat_range = actor_variant_definition->ranged_combat.combat_range_upper_bound;
			long best_prop_index = NONE;
			struct prop_datum *best_prop = NULL;
			real best_weight = 0.0f;
			real best_distance = 0.0f;
			struct prop_iterator iterator;
			struct prop_datum *prop;

			prop_iterator_new(&iterator, actor_index);
			while ((prop = prop_iterator_next(&iterator)) != NULL)
			{
				if (prop->target_weight > 0.0f)
				{
					real distance = distance3d(&prop->body_position, &swarm_component->position);
					real weight = 0.0f;

					if (distance < combat_range)
					{
						weight = (1.0f - distance / combat_range) * 10.0f;
					}

					if (prop_acknowledged(prop))
					{
						if (iterator.index == swarm_component->combat_target_prop_index)
						{
							weight += 7.0f;
						}
						else
						{
							weight += 5.0f;
						}

						if (!prop->child_units_attached)
						{
							weight += 5.0f;
						}
					}

					if (weight > best_weight)
					{
						best_weight = weight;
						best_prop = prop;
						best_prop_index = iterator.index;
						best_distance = distance;
					}
				}
			}

			swarm_component->combat_target_prop_index = best_prop_index;
			if (best_prop_index != NONE &&
				best_distance < actor_variant_definition->ranged_combat.melee_range &&
				prop_acknowledged(best_prop))
			{
				in_melee_range = TRUE;
			}
			target_prop = best_prop;
		}
		else
		{
			swarm_component->combat_target_prop_index = NONE;
			target_prop = NULL;
		}

		switch (actor->state.action)
		{
			case _actor_action_sleep:
				movement_type = _swarm_movement_none;
				animation_state = _unit_animation_state_asleep;
				break;

			case _actor_action_alert:
				movement_type = _swarm_movement_wander_noncombat;
				animation_state = _unit_animation_state_alert;
				break;

			case _actor_action_guard:
				movement_type = _swarm_movement_wander_idle;
				animation_state = _unit_animation_state_alert;
				break;

			case _actor_action_search:
				if (actor->state.action_data.search.pursuit_location.type == _pursuit_location_target && actor->target.target_prop_index != NONE)
				{
					movement_type = _swarm_movement_towards_prop;
					prop_index = actor->target.target_prop_index;
				}
				else
				{
					movement_type = _swarm_movement_wander_combat;
				}
				animation_state = _unit_animation_state_in_combat;
				break;

			case _actor_action_flee:
				animation_state = actor->state.action_data.flee.panic_type > _actor_panic_none ? _unit_animation_state_flee : _unit_animation_state_in_combat;
				if (actor->state.action_data.flee.flee_prop_index != NONE)
				{
					movement_type = _swarm_movement_away_from_prop;
					prop_index = actor->state.action_data.flee.flee_prop_index;
				}
				break;

			case _actor_action_charge:
			case _actor_action_obey:
				animation_state = _unit_animation_state_in_combat;
				if (actor->state.action == _actor_action_obey && TEST_FLAG(swarm_component->flags, _swarm_component_obey_bit))
				{
					movement_type = _swarm_movement_obey;
				}
				else if (swarm_component->combat_target_prop_index != NONE)
				{
					aiming_speed = 0;
					prop_index = swarm_component->combat_target_prop_index;
					movement_type = swarm_component->attack_delay_ticks > 0 ? _swarm_movement_away_from_prop : _swarm_movement_towards_prop;
				}
				else
				{
					movement_type = _swarm_movement_wander_combat;
				}
				break;
		}

		if (unit->object.parent_object_index == NONE)
		{
			swarm_component->attached_to_unit_ticks = 0;
			if (swarm_component->attack_delay_ticks > 0)
			{
				swarm_component->attack_delay_ticks--;
			}
		}
		else
		{
			struct unit_datum *parent_unit = unit_get(unit->object.parent_object_index);
			boolean parent_dead = TEST_FLAG(parent_unit->object.damage_flags, _object_dead_bit);
			boolean detach = FALSE;

			if (swarm_component->attached_to_unit_ticks < UNSIGNED_CHAR_MAX)
			{
				swarm_component->attached_to_unit_ticks++;
			}

			if (parent_dead)
			{
				if (parent_unit->unit.time_of_death != NONE &&
					parent_unit->unit.time_of_death + 75 < game_time_get() &&
					target_prop &&
					target_prop->unit_index != unit->object.parent_object_index &&
					prop_acknowledged(target_prop))
				{
					detach = TRUE;
				}
			}
			else
			{
				struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);

				if ((parent_unit->object.type != _object_type_biped || TEST_FLAG(parent_unit_definition->unit.flags, _unit_melee_attackers_cannot_attach_bit)) &&
					swarm_component->attached_to_unit_ticks > 45)
				{
					swarm_component->attack_delay_ticks = 45;
					detach = TRUE;
				}
			}

			if (detach)
			{
				unit_detach_from_parent(unit_index);
				SET_FLAG(swarm_component->flags, _swarm_component_attached_to_unit_bit, FALSE);
				SET_FLAG(swarm_component->flags, _swarm_component_attacking_in_melee_bit, FALSE);
			}
			else
			{
				SET_FLAG(swarm_component->flags, _swarm_component_attached_to_unit_bit, TRUE);
				SET_FLAG(swarm_component->flags, _swarm_component_attacking_in_melee_bit, !parent_dead);
			}
		}

		if (unit->object.parent_object_index != NONE)
		{
		}
		else
		{
			if (airborne)
			{
				swarm_component->ground_ticks = 0;
				SET_FLAG(swarm_component->flags, _swarm_component_attached_to_unit_bit, FALSE);
			}
			else
			{
				if (swarm_component->ground_ticks < UNSIGNED_CHAR_MAX)
				{
					swarm_component->ground_ticks++;
				}

				SET_FLAG(swarm_component->flags, _swarm_component_attacking_in_melee_bit, FALSE);
				SET_FLAG(swarm_component->flags, _swarm_component_attached_to_unit_bit, FALSE);

				switch (movement_type)
				{
					case _swarm_movement_wander_noncombat:
					case _swarm_movement_wander_idle:
					case _swarm_movement_wander_combat:
						if (!TEST_FLAG(swarm_component->flags, _swarm_component_wander_bit))
						{
							memset(&swarm_component->wander, 0, sizeof(swarm_component->wander));
							SET_FLAG(swarm_component->flags, _swarm_component_obey_bit, FALSE);
							SET_FLAG(swarm_component->flags, _swarm_component_wander_bit, TRUE);
						}

						if (swarm_component->wander.move_ticks > 0)
						{
							swarm_component->wander.move_ticks--;
							if (swarm_component->wander.move_ticks == 0)
							{
								swarm_component->wander.pause_ticks = infection_wander_pause_time(movement_type);
							}
							else
							{
								real restoring_angle = -swarm_component->wander.angle / 15.0f;
								real maximum_angle_change = DEGREES_TO_RADIANS(36.f) / TICKS_PER_SECOND;

								swarm_component->wander.angle += real_random_range(-maximum_angle_change, maximum_angle_change) + restoring_angle;
								rotate_vector_about_axis(&swarm_component->wander.vector, &up_vector, sine(swarm_component->wander.angle), cosine(swarm_component->wander.angle));
							}
						}
						else
						{
							if (swarm_component->wander.pause_ticks > 0)
							{
								swarm_component->wander.pause_ticks--;
							}

							if (swarm_component->wander.pause_ticks == 0)
							{
								real_vector3d vector_to_swarm;
								real distance_squared;

								swarm_component->wander.move_ticks = infection_wander_move_time(movement_type);

								vector_from_points3d(&swarm_component->position, &swarm->swarm_center, &vector_to_swarm);
								distance_squared = magnitude_squared3d(&vector_to_swarm);
								if (distance_squared < 0.25f)
								{
									real angle = real_random_range(-_pi, _pi);

									swarm_component->wander.vector = unit->object.forward;
									rotate_vector_about_axis(&swarm_component->wander.vector, &up_vector, sine(angle), cosine(angle));
								}
								else
								{
									real maximum_angle = (0.5f / square_root(distance_squared)) * _pi;
									real angle = real_random_range(-maximum_angle, maximum_angle);

									swarm_component->wander.vector = vector_to_swarm;
									rotate_vector_about_axis(&swarm_component->wander.vector, &up_vector, sine(angle), cosine(angle));
								}
								swarm_component->wander.angle = 0.0f;
							}
						}

						match_assert("c:\\halo\\SOURCE\\ai\\actor_type_infection.c", 493, swarm_component->wander.move_ticks || swarm_component->wander.pause_ticks);
						if (swarm_component->wander.move_ticks)
						{
							moving = TRUE;
							movement_vector = swarm_component->wander.vector;
						}
						break;

					case _swarm_movement_towards_prop:
					case _swarm_movement_away_from_prop:
					{
						struct prop_datum *prop = prop_get(prop_index);

						vector_from_points3d(&swarm_component->position, &prop->body_position, &movement_vector);
						if (movement_type == _swarm_movement_away_from_prop)
						{
							negate_vector3d(&movement_vector, &movement_vector);
						}
						moving = TRUE;
						break;
					}

					case _swarm_movement_obey:
						match_assert("c:\\halo\\SOURCE\\ai\\actor_type_infection.c", 516, TEST_FLAG(swarm_component->flags, _swarm_component_obey_bit));

						if (TEST_FLAG(swarm_component->obey.simple_control_flags, 0))
						{
							moving = TRUE;
							switch (swarm_component->obey.directmovement.facing)
							{
								case 2:
								case 3:
									cross_product3d(&up_vector, &swarm_component->obey.directmovement.vector, &movement_vector);
									if (swarm_component->obey.directmovement.facing == 3)
									{
										negate_vector3d(&movement_vector, &movement_vector);
									}
									break;

								default:
									movement_vector = swarm_component->obey.directmovement.vector;
									if (swarm_component->obey.directmovement.facing == 1)
									{
										negate_vector3d(&movement_vector, &movement_vector);
									}
									break;
							}
						}

						if (TEST_FLAG(swarm_component->obey.simple_control_flags, 2))
						{
							if (!TEST_FLAG(swarm_component->obey.simple_control_flags, 3) &&
								swarm_component->obey.jump.delay_ticks == 0 &&
								!unit_is_busy(unit_index))
							{
								SET_FLAG(swarm_component->flags, _swarm_component_obey_desire_jump_bit, TRUE);
								SET_FLAG(swarm_component->obey.simple_control_flags, 3, TRUE);
							}

							moving = TRUE;
							movement_vector = unit->object.forward;
						}
						break;
				}
			}
		}

		if (moving)
		{
			real up_component;

			normalize3d(&movement_vector);

			up_component = dot_product3d(&movement_vector, &up_vector);
			if (up_component > 0.9f)
			{
				moving_vertically = TRUE;
			}

			if (up_component < -0.9f)
			{
				movement_vector = unit->object.forward;
			}
			else
			{
				real_vector3d left_vector;

				cross_product3d(&up_vector, &movement_vector, &left_vector);
				cross_product3d(&left_vector, &up_vector, &movement_vector);
				if (normalize3d(&movement_vector) == 0.0f)
				{
					movement_vector = unit->object.forward;
				}
			}

			if (movement_type != _swarm_movement_obey)
			{
				real_point3d behind_point;
				real_vector3d left_vector;
				real avoidance = 0.0f;
				short other_unit_index;

				point_from_line3d(&swarm_component->position, &movement_vector, -0.2f, &behind_point);
				cross_product3d(&up_vector, &movement_vector, &left_vector);

				for (other_unit_index = 0; other_unit_index < swarm->unit_count; other_unit_index++)
				{
					if (other_unit_index != swarm_unit_index)
					{
						struct swarm_component_datum *other_component = swarm_component_get(swarm->component_indices[other_unit_index]);
						real_vector3d vector_to_friend;
						real distance_squared;

						vector_from_points3d(&behind_point, &other_component->position, &vector_to_friend);
						distance_squared = magnitude_squared3d(&vector_to_friend);
						if (distance_squared < 0.8f * 0.8f)
						{
							real distance = square_root(distance_squared);
							real forward_cosine = dot_product3d(&movement_vector, &vector_to_friend) / distance;

							if (forward_cosine > 0.5f)
							{
								if (dot_product3d(&vector_to_friend, &left_vector) > 0.0f)
								{
									avoidance -= (forward_cosine - 0.5f) * 0.5f;
								}
								else
								{
									avoidance += (forward_cosine - 0.5f) * 0.5f;
								}
							}
						}
					}
				}

				if (avoidance != 0.0f)
				{
					real angle;

					if (avoidance > 1.0f)
					{
						angle = _pi / 2;
					}
					else if (avoidance < -1.0f)
					{
						angle = -_pi / 2;
					}
					else
					{
						angle = avoidance * (_pi / 2);
					}

					rotate_vector_about_axis(&movement_vector, &up_vector, sine(angle), cosine(angle));
				}
			}

			{
				real_vector3d left_vector;

				cross_product3d(&up_vector, &movement_vector, &left_vector);
				if (normalize3d(&left_vector) == 0.0f)
				{
					movement_vector = unit->object.forward;
				}
				else
				{
					cross_product3d(&left_vector, &up_vector, &movement_vector);
				}
			}
		}

		if (TEST_FLAG(swarm_component->flags, _swarm_component_obey_desire_jump_bit))
		{
			jump = TRUE;
		}
		else if (moving && swarm_component->ground_ticks >= 45)
		{
			if (swarm_unit_index == intermittent_action_index)
			{
				jump = TRUE;
			}
			else if (in_melee_range)
			{
				jump = TRUE;
			}
			else if (moving_vertically)
			{
				jump = TRUE;
			}
		}

		if (TEST_FLAG(swarm_component->flags, _swarm_component_attached_to_unit_bit))
		{
			if (TEST_FLAG(swarm_component->flags, _swarm_component_attacking_in_melee_bit))
			{
				unit->unit.melee_attack_state = 4;
			}
			else
			{
				unit->unit.melee_attack_state = 0;
			}
		}
		else
		{
			SET_FLAG(swarm_component->flags, _swarm_component_attacking_in_melee_bit, in_melee_range && swarm_component->attack_delay_ticks == 0);
			if (TEST_FLAG(swarm_component->flags, _swarm_component_attacking_in_melee_bit))
			{
				struct biped_datum *biped = (struct biped_datum *)unit;

				unit->unit.melee_attack_state = 3;
				biped->biped.impact_target_object_index = target_prop == NULL ? NONE : target_prop->unit_index;
			}
			else
			{
				unit->unit.melee_attack_state = 0;
			}
		}

		{
			struct unit_control_data control_data;
			long control_flags = 0;

			SET_FLAG(control_flags, _unit_control_jump_bit, jump);
			memset(&control_data, 0, sizeof(control_data));
			control_data.control_flags = (word)control_flags;
			control_data.primary_trigger = 0.0f;
			control_data.weapon_index = NONE;
			control_data.grenade_index = NONE;
			control_data.zoom_level = NONE;
			control_data.animation_state = (char)animation_state;
			control_data.aiming_speed = (char)aiming_speed;
			if (moving)
			{
				set_real_vector3d(&control_data.throttle, 1.0f, 0.0f, 0.0f);
				control_data.facing_vector = movement_vector;
			}
			else
			{
				control_data.throttle = *global_zero_vector3d;
				control_data.facing_vector = unit->object.forward;
			}
			control_data.aiming_vector = control_data.facing_vector;
			control_data.looking_vector = control_data.facing_vector;

			unit_control(unit_index, &control_data);
		}
	}

	return;
}

void infection_swarm_aim_jump(
	long actor_index,
	long unit_index,
	real jump_magnitude,
	real_vector3d *jump_velocity)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.swarm_cache_index == NONE)
	{
	}
	else
	{
		struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
		short swarm_unit_index;

		for (swarm_unit_index = 0; swarm_unit_index < swarm->unit_count; swarm_unit_index++)
		{
			if (swarm->unit_indices[swarm_unit_index] == unit_index)
			{
				struct unit_datum *unit = unit_get(unit_index);
				struct swarm_component_datum *swarm_component = swarm_component_get(swarm->component_indices[swarm_unit_index]);

				if (TEST_FLAG(swarm_component->flags, _swarm_component_attacking_in_melee_bit) && swarm_component->combat_target_prop_index != NONE)
				{
					struct prop_datum *prop = prop_get(swarm_component->combat_target_prop_index);
					real_vector3d aim_vector;
					real vertical_velocity;
					real horizontal_velocity;
					real minimum_velocity = 0.06f;
					real ballistic_fraction = 0.8f;

					jump_magnitude = MAX(jump_magnitude, 0.12f);
					if (projectile_aim_ballistic(jump_magnitude, 1.0f, &swarm_component->position, &prop->center_of_mass,
						&minimum_velocity, &ballistic_fraction, NULL, FALSE, &aim_vector, NULL, NULL, NULL,
						&vertical_velocity, &horizontal_velocity))
					{
						real magnitude_squared;

						if (normalize2d((real_vector2d *)&aim_vector) == 0.0f)
						{
							aim_vector = actor->input.facing_vector;
							if (normalize2d((real_vector2d *)&aim_vector) == 0.0f)
							{
								aim_vector = *global_forward3d;
							}
						}

						if (!prop->flying)
						{
							vertical_velocity = MIN(vertical_velocity, 0.075f);
						}

						set_real_vector3d(jump_velocity, aim_vector.i * horizontal_velocity, aim_vector.j * horizontal_velocity, vertical_velocity);
						magnitude_squared = magnitude_squared3d(jump_velocity);
						if (magnitude_squared > jump_magnitude * jump_magnitude)
						{
							scale_vector3d(jump_velocity, jump_magnitude / square_root(magnitude_squared), jump_velocity);
						}
					}
				}
				else if (TEST_FLAG(swarm_component->flags, _swarm_component_obey_bit) &&
					TEST_FLAG(swarm_component->flags, _swarm_component_obey_desire_jump_bit))
				{
					if (TEST_FLAG(swarm_component->obey.simple_control_flags, 2) && TEST_FLAG(swarm_component->obey.simple_control_flags, 4))
					{
						real_vector2d alignment_vector;

						alignment_vector = *(real_vector2d *)&unit->object.forward;
						if (normalize2d(&alignment_vector) == 0.0f)
						{
							alignment_vector = *(real_vector2d *)&unit->object.up;
							if (normalize2d(&alignment_vector) == 0.0f)
							{
								alignment_vector = *global_forward2d;
							}
						}

						set_real_vector3d(jump_velocity,
							alignment_vector.i * swarm_component->obey.jump.target_horizontal_vel,
							alignment_vector.j * swarm_component->obey.jump.target_horizontal_vel,
							swarm_component->obey.jump.target_vertical_vel);
					}
					SET_FLAG(swarm_component->flags, _swarm_component_obey_desire_jump_bit, FALSE);
				}
			}
		}
	}

	return;
}

/* ---------- private code */
