/*
ACTOR_LOOKING.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "ai_profile.h"
#include "units.h"
#include "console.h"
#include "collisions.h"
#include "weapon_definitions.h"
#include "game.h"

/* ---------- constants */

enum
{
	_idle_timer_facing = 0,
	_idle_timer_aiming,
	_idle_timer_looking,
};

/* ---------- prototypes */

static boolean actor_look_decode_direction(long actor_index, struct direction_specification *specification, real_vector3d *direction);
static boolean actor_look_valid_aim_vector(real yaw_deviation, real_vector3d *facing_vector, real_vector3d *attempted_aiming_vector);
static boolean actor_look_valid_look_vector(real yaw_deviation, real *look_delta_deviations, real_vector3d *facing_vector, real_vector3d *aiming_vector, real_vector3d *attempted_looking_vector);
static boolean actor_look_idle_find_prop(long actor_index, boolean free_facing, boolean aim_at_prop, struct direction_specification *direction, boolean *interesting_prop);
static boolean actor_look_idle_new_major_direction(long actor_index, struct actor_idle_looking *looking_definition, real_vector3d const *base_vector, boolean free_facing, boolean major_is_aiming, boolean has_minor_variation);
static void actor_look_idle_new_minor_direction(long actor_index, struct actor_idle_looking *looking_definition, real_vector3d const *base_vector);
static struct actor_idle_looking *actor_look_get_looking_definition(long actor_index);
static long actor_look_idle_timer(long actor_index, struct actor_idle_looking *looking_definition, short timer_type, boolean interesting_direction);
static boolean actor_look_find_random_vector(real_point3d const *center_point, real_vector3d const *base_vector, boolean avoid_obstructions, real yaw_min, real yaw_max, real pitch_min, real pitch_max, real_vector3d *result_vector);

/* ---------- globals */

const short global_secondary_look_priorities[NUMBER_OF_SECONDARY_LOOK_TYPES][2] =
{
	{ _secondary_look_priority_default, _secondary_look_priority_default },				// _secondary_look_none
	{ _secondary_look_priority_idle_look, _secondary_look_priority_idle_look },			// _secondary_look_environmental_noise
	{ _secondary_look_priority_idle_look, _secondary_look_priority_idle_look },			// _secondary_look_started_moving_prop
	{ _secondary_look_priority_idle_aim, _secondary_look_priority_idle_aim },			// _secondary_look_weapon_impact
	{ _secondary_look_priority_turn_and_aim, _secondary_look_priority_idle_aim },		// _secondary_look_newly_acknowledged_prop
	{ _secondary_look_priority_aim, _secondary_look_priority_aim },						// _secondary_look_bumped_prop
	{ _secondary_look_priority_turn_and_aim, _secondary_look_priority_aim },			// _secondary_look_detonation
	{ _secondary_look_priority_turn_and_aim, _secondary_look_priority_aim },			// _secondary_look_shooting_prop
	{ _secondary_look_priority_aim, _secondary_look_priority_idle_aim },				// _secondary_look_communicating_prop
	{ _secondary_look_priority_turn_and_aim, _secondary_look_priority_turn_and_aim },	// _secondary_look_communicated_direction
	{ _secondary_look_priority_stop_and_aim, _secondary_look_priority_idle_aim },		// _secondary_look_combat_stimulus_prop
	{ _secondary_look_priority_stop_and_aim, _secondary_look_priority_idle_aim },		// _secondary_look_damage
	{ _secondary_look_priority_override, _secondary_look_priority_turn_and_aim },		// _secondary_look_dangerous_object
	{ _secondary_look_priority_override, _secondary_look_priority_idle_look },			// _secondary_look_scripted
};

const real global_secondary_look_times[NUMBER_OF_SECONDARY_LOOK_TYPES] =
{
	0.f,	// _secondary_look_none
	1.3f,	// _secondary_look_environmental_noise
	0.9f,	// _secondary_look_started_moving_prop
	0.9f,	// _secondary_look_weapon_impact
	0.9f,	// _secondary_look_newly_acknowledged_prop
	0.7f,	// _secondary_look_bumped_prop
	0.9f,	// _secondary_look_detonation
	0.9f,	// _secondary_look_shooting_prop
	1.2f,	// _secondary_look_communicating_prop
	2.f,	// _secondary_look_communicated_direction
	2.f,	// _secondary_look_combat_stimulus_prop
	2.5f,	// _secondary_look_damage
	1.5f,	// _secondary_look_dangerous_object
	1000.f,	// _secondary_look_scripted
};

/* ---------- public code */

void actor_look_affect_movement(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->orders.look.primary_direction.type==_direction_specification_movement && !actor_path_has_path(actor_index))
	{
		actor->orders.look.primary_priority = _primary_priority_none;
	}

	if (actor->orders.look.primary_priority>=_primary_priority_facing &&
		actor->orders.look.primary_direction.type!=_direction_specification_movement &&
		actor_look_decode_direction(actor_index, &actor->orders.look.primary_direction, &actor->control.moving_forced_aim_direction))
	{
		actor->control.moving_forced_by_aiming = TRUE;
	}
	else
	{
		actor->control.moving_forced_by_aiming = FALSE;
	}

	return;
}

boolean actor_look_secondary(
	long actor_index,
	short type,
	short priority,
	struct direction_specification *direction)
{
	boolean result;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actor_looking.c", 135, (type >= 0) && (type < NUMBER_OF_SECONDARY_LOOK_TYPES));

	if (actor->state.mode<=_actor_mode_asleep && type<_secondary_look_scripted)
	{
		result = FALSE;
	}
	else if (actor->control.secondary_look_type>type)
	{
		result = FALSE;
	}
	else
	{
		boolean primary_locked = actor->orders.look.primary_priority>=_primary_priority_locked_aiming;

		result = TRUE;

		if (type<_secondary_look_scripted && actor->state.action==_actor_action_obey && !actor->state.action_data.obey.allow_looking)
		{
			result = FALSE;
		}
		else if (primary_locked && type<_secondary_look_newly_acknowledged_prop)
		{
			result = FALSE;
		}
		else if (direction->type==_direction_specification_prop)
		{
			struct prop_datum *prop = prop_try_and_get(direction->prop_index);

			if (!prop)
			{
				result = FALSE;
			}
			else if (type<_secondary_look_communicating_prop &&
				((!prop->enemy && !prop->dead) || (prop->dead && actor->state.mode>=_actor_mode_combat)))
			{
				long current_time = game_time_get();

				if (primary_locked)
				{
					result = FALSE;
				}
				else if ((!prop->player || type<_secondary_look_newly_acknowledged_prop) &&
					prop->last_idle_look_time!=NONE &&
					prop->last_idle_look_time + 20 * TICKS_PER_SECOND>current_time)
				{
					result = FALSE;
				}

				if (result)
				{
					prop->last_idle_look_time = current_time;
					prop->last_idle_look_interest = MAX(prop->last_idle_look_interest, prop->look_interest);
				}
			}
		}
	}

	if (result)
	{
		short timer;
		long ticks;
		real time = global_secondary_look_times[type];

		if (actor->state.mode<_actor_mode_combat || actor->state.combat_status==_actor_combat_status_none)
		{
			time = time * 2.f;
		}

		if (actor_definition->looking.event_look_modifier_lower_bound!=0.f || actor_definition->looking.event_look_modifier_upper_bound!=0.f)
		{
			real lower_bound = MAX(actor_definition->looking.event_look_modifier_lower_bound, 0.5f);

			time = real_random_range(lower_bound, MIN(actor_definition->looking.event_look_modifier_upper_bound, 2.f)) * time;
		}

		ticks = fast_ftol(time * TICKS_PER_SECOND);
		timer = (short)MIN(ticks, SHRT_MAX);

		if (priority==_secondary_look_priority_default)
		{
			boolean certain = actor->state.combat_status>=_actor_combat_status_certain;

			priority = global_secondary_look_priorities[type][certain];
		}

#ifdef DEBUG
		if (ai_debug.print_secondary_looking)
		{
			char *priority_names[] =
			{
				"none",
				"default",
				"idle-look",
				"idle-aim",
				"aim",
				"turn-and-aim",
				"stop-and-aim",
				"override",
				"override-facing",
			};
			char *type_names[] =
			{
				"none",
				"environment",
				"moving-prop",
				"impact",
				"new-prop",
				"shooting-prop",
				"comm-prop",
				"comm-direction",
				"combat-prop",
				"damage",
				"danger",
				"scripted",
			};

			console_printf(
				FALSE,
				"%s: look %s %s %d",
				ai_debug_describe_actor(actor_index, NONE, FALSE, temporary, sizeof(temporary)),
				type_names[type],
				priority_names[priority],
				timer);
		}
#endif

		actor->control.secondary_look_type = type;
		actor->control.secondary_look_priority = priority;
		actor->control.secondary_look_timer = timer;
		actor->control.secondary_look_direction = *direction;
	}

	return result;
}

void actor_look_secondary_stop(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

#ifdef DEBUG
	if (actor->control.secondary_look_type>_secondary_look_none && ai_debug.print_secondary_looking)
	{
		console_printf(
			FALSE,
			"%s: look-stop",
			ai_debug_describe_actor(actor_index, NONE, FALSE, temporary, sizeof(temporary)));
	}
#endif

	actor->control.secondary_look_priority = _secondary_look_priority_none;
	actor->control.secondary_look_type = _secondary_look_none;
	actor->control.secondary_look_timer = 0;

	return;
}

real actor_look_compute_prop_interest(
	long actor_index,
	long prop_index)
{
	real modifier;
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *prop = prop_get(prop_index);
	real interest = 0.f;

	if (prop->state>=_prop_state_becoming_unacknowledged && prop->state<=_prop_state_acknowledged)
	{
		if (prop->dead)
		{
			interest = prop->dead_ticks<7 * TICKS_PER_SECOND ? 1.8f : 0.4f;
		}
		else if (prop->enemy)
		{
			interest = 2.f;
		}
		else
		{
			interest = 1.f;
		}
	}
	else if (prop->enemy && prop->state>=_prop_state_uninspected_orphan && prop->state<=_prop_state_inspected_orphan)
	{
		interest = 1.5f;
	}

	if (prop->unit_index==actor->input.vehicle_index || prop->vehicle_index==actor->input.vehicle_index)
	{
		interest = 0.f;
	}

	modifier = prop->vehicle_index!=NONE ? 1.5f : 1.f;

	switch (prop->quantized_speed)
	{
		case _prop_speed_walking:
			interest += 0.5f * modifier;
			break;

		case _prop_speed_running:
			interest += 1.f * modifier;
			break;

		case _prop_speed_driving:
			interest += 2.f * modifier;
			break;
	}

	if (prop->shooting)
	{
		interest += 2.f * modifier;
	}

	switch (prop->quantized_distance)
	{
		case _prop_distance_near:
			interest *= 0.6f;
			break;

		case _prop_distance_far:
			interest *= 0.4f;
			break;

		case _prop_distance_distant:
			interest *= 0.2f;
			break;
	}

	return interest;
}

static boolean actor_look_decode_direction(
	long actor_index,
	struct direction_specification *specification,
	real_vector3d *direction)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\actor_looking.c", 349, specification);
	match_assert("c:\\halo\\SOURCE\\ai\\actor_looking.c", 350, direction);

	switch (specification->type)
	{
		case _direction_specification_movement:
			if (actor->control.moving)
			{
				*direction = actor->control.moving_towards_vector;
				result = normalize3d(direction)>0.f;
			}
			break;

		case _direction_specification_target:
			if (actor->control.fire_state==_actor_fire_state_bursting)
			{
				*direction = actor->control.burst_aim_vector;
				result = TRUE;
			}
			else if (actor->control.aiming_at_fire_target)
			{
				*direction = actor->control.current_fire_target_aim_vector;
				result = TRUE;
			}
			else if (actor->target.target_prop_index!=NONE)
			{
				struct prop_datum *prop = prop_get(actor->target.target_prop_index);

				vector_from_points3d(&actor->input.position.head_position, &prop->center_of_mass, direction);
				result = normalize3d(direction)>0.f;
			}
			break;

		case _direction_specification_prop:
		{
			struct prop_datum *prop = prop_try_and_get(specification->prop_index);

			if (prop)
			{
				vector_from_points3d(&actor->input.position.head_position, &prop->head_position, direction);
				result = normalize3d(direction)>0.f;
			}
			break;
		}

		case _direction_specification_point:
			vector_from_points3d(&actor->input.position.head_position, &specification->point, direction);
			result = normalize3d(direction)>0.f;
			break;

		case _direction_specification_vector:
			*direction = specification->vector;
			result = TRUE;
			break;

		case _direction_specification_danger:
			if (actor->danger_zone.danger_type>_actor_danger_zone_none)
			{
				vector_from_points3d(&actor->input.position.head_position, &actor->danger_zone.position, direction);
				result = normalize3d(direction)>0.f;
			}
			break;

		case _direction_specification_object:
		{
			struct object_datum *object = object_try_and_get(specification->object_index);

			if (object)
			{
				real_point3d look_point;

				if (TEST_FLAG(_object_mask_unit, object->object.type))
				{
					unit_get_head_position(specification->object_index, &look_point);
				}
				else
				{
					object_get_origin(specification->object_index, &look_point);
				}

				vector_from_points3d(&actor->input.position.head_position, &look_point, direction);
				result = normalize3d(direction)>0.f;
			}
			break;
		}

		default:
			match_halt("c:\\halo\\SOURCE\\ai\\actor_looking.c", 449);
			break;
	}

	if (result)
	{
		real magnitude_squared = magnitude_squared3d(direction);

		if (!valid_realcmp(magnitude_squared, 1.f))
		{
			real magnitude = square_root(magnitude_squared);

			switch (specification->type)
			{
				case _direction_specification_movement:
					sprintf(
						temporary,
						"denormalized %f: %smoving (p%f %f %f) (v%f %f %f)",
						magnitude,
						actor->control.moving ? "" : "not ",
						actor->control.moving_towards_point.x,
						actor->control.moving_towards_point.z,
						actor->control.moving_towards_point.z,
						actor->control.moving_towards_vector.i,
						actor->control.moving_towards_vector.j,
						actor->control.moving_towards_vector.k);
					break;

				case _direction_specification_target:
					if (actor->control.fire_state==_actor_fire_state_bursting)
					{
						sprintf(
							temporary,
							"denormalized %f: target type %d (burst aim vector %f %f %f)",
							magnitude,
							actor->control.current_fire_target_type,
							actor->control.burst_aim_vector.i,
							actor->control.burst_aim_vector.j,
							actor->control.burst_aim_vector.k);
					}
					else if (actor->control.aiming_at_fire_target)
					{
						sprintf(
							temporary,
							"denormalized %f: target type %d (fire target aim vector %f %f %f)",
							magnitude,
							actor->control.current_fire_target_type,
							actor->control.current_fire_target_aim_vector.i,
							actor->control.current_fire_target_aim_vector.j,
							actor->control.current_fire_target_aim_vector.k);
					}
					else if (actor->target.target_prop_index!=NONE)
					{
						struct prop_datum *prop = prop_get(actor->target.target_prop_index);

						sprintf(
							temporary,
							"denormalized %f: target (prop 0x%08X center %f %f %f)",
							magnitude,
							actor->target.target_prop_index,
							prop->center_of_mass.x,
							prop->center_of_mass.y,
							prop->center_of_mass.z);
					}
					else
					{
						sprintf(
							temporary,
							"denormalized %f: target (none)",
							magnitude);
					}
					break;

				case _direction_specification_prop:
				{
					struct prop_datum *prop = prop_try_and_get(specification->prop_index);

					sprintf(
						temporary,
						"denormalized %f: prop 0x%08X%s (actor 0x%08X / us 0x%08X)",
						magnitude,
						specification->prop_index,
						!prop ? " (invalid)" : "",
						!prop ? 0 : prop->owner_actor_index,
						actor_index);
					break;
				}

				case _direction_specification_point:
					sprintf(
						temporary,
						"denormalized %f: point (%f %f %f)",
						magnitude,
						specification->point.x,
						specification->point.y,
						specification->point.z);
					break;

				case _direction_specification_vector:
					sprintf(
						temporary,
						"denormalized %f: vector (%f %f %f)",
						magnitude,
						specification->vector.i,
						specification->vector.j,
						specification->vector.k);
					break;

				case _direction_specification_danger:
					if (actor->danger_zone.danger_type>_actor_danger_zone_none)
					{
						sprintf(
							temporary,
							"denormalized %f: danger (%f %f %f)",
							magnitude,
							actor->danger_zone.position.x,
							actor->danger_zone.position.y,
							actor->danger_zone.position.z);
					}
					else
					{
						sprintf(
							temporary,
							"denormalized %f: danger (none)",
							magnitude);
					}
					break;

				case _direction_specification_object:
					sprintf(
						temporary,
						"denormalized %f: object (0x%08X)",
						magnitude,
						specification->object_index);
					break;

				default:
					sprintf(
						temporary,
						"denormalized %f: <error> (type %d)",
						magnitude,
						specification->type);
					break;
			}

			match_vwarn("c:\\halo\\SOURCE\\ai\\actor_looking.c", 526, FALSE, temporary);
		}

		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_looking.c", 529, direction);
	}

	return result;
}

static boolean actor_look_valid_aim_vector(
	real yaw_deviation,
	real_vector3d *facing_vector,
	real_vector3d *attempted_aiming_vector)
{
	real_vector2d looking_vector2d = *(real_vector2d *)attempted_aiming_vector;
	boolean result = FALSE;

	if (normalize2d(&looking_vector2d)>0.f &&
		dot_product2d(&looking_vector2d, (real_vector2d *)facing_vector)>yaw_deviation)
	{
		result = TRUE;
	}

	return result;
}

static boolean actor_look_valid_look_vector(
	real yaw_deviation,
	real *look_delta_deviations,
	real_vector3d *facing_vector,
	real_vector3d *aiming_vector,
	real_vector3d *attempted_looking_vector)
{
	real_vector2d looking_vector2d = *(real_vector2d *)attempted_looking_vector;
	real_vector2d aiming_vector2d = *(real_vector2d *)aiming_vector;
	boolean result = FALSE;

	if (normalize2d(&looking_vector2d)>0.f &&
		dot_product2d(&looking_vector2d, (real_vector2d *)facing_vector)>yaw_deviation &&
		normalize2d(&aiming_vector2d)>0.f)
	{
		boolean clockwise = cross_product2d(&looking_vector2d, &aiming_vector2d)>0.f;

		if (dot_product2d(&looking_vector2d, &aiming_vector2d)>look_delta_deviations[clockwise])
		{
			result = TRUE;
		}
	}

	return result;
}

void actor_looking_test_validity(
	long actor_index,
	real_vector3d *test_vector,
	boolean *valid_aiming,
	boolean *valid_looking)
{
	real look_delta_deviations[2];
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	*valid_aiming = actor_look_valid_aim_vector(
		actor_definition->looking.runtime_maximum_aiming_deviation_cosine.yaw,
		&actor->input.facing_vector,
		test_vector);

	if (actor->state.mode==_actor_mode_combat)
	{
		look_delta_deviations[0] = cosine(actor_definition->looking.combat_looking_delta_angles[0]);
		look_delta_deviations[1] = cosine(actor_definition->looking.combat_looking_delta_angles[1]);
	}
	else
	{
		look_delta_deviations[0] = cosine(actor_definition->looking.noncombat_looking_delta_angles[0]);
		look_delta_deviations[1] = cosine(actor_definition->looking.noncombat_looking_delta_angles[1]);
	}

	*valid_looking = actor_look_valid_look_vector(
		actor_definition->looking.runtime_maximum_looking_deviation_cosine.yaw,
		look_delta_deviations,
		&actor->input.facing_vector,
		&actor->input.aiming_vector,
		test_vector);

	return;
}

static boolean actor_look_idle_find_prop(
	long actor_index,
	boolean free_facing,
	boolean aim_at_prop,
	struct direction_specification *direction,
	boolean *interesting_prop)
{
	struct prop_iterator iterator;
	struct prop_datum *prop;
	real look_delta_deviations[2];
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	long best_prop_index = NONE;
	real best_interest = 0.f;
	struct prop_datum *best_prop = NULL;
	boolean best_interesting = FALSE;
	boolean result = FALSE;
	long current_time = game_time_get();
	real aiming_yaw_deviation = actor_definition->looking.runtime_maximum_aiming_deviation_cosine.yaw;
	real looking_yaw_deviation = actor_definition->looking.runtime_maximum_looking_deviation_cosine.yaw;

	if (actor->state.mode==_actor_mode_combat)
	{
		look_delta_deviations[0] = cosine(actor_definition->looking.combat_looking_delta_angles[0]);
		look_delta_deviations[1] = cosine(actor_definition->looking.combat_looking_delta_angles[1]);
	}
	else
	{
		look_delta_deviations[0] = cosine(actor_definition->looking.noncombat_looking_delta_angles[0]);
		look_delta_deviations[1] = cosine(actor_definition->looking.noncombat_looking_delta_angles[1]);
	}

#ifdef DEBUG
	if (actor_index==ai_debug.selected_actor_index)
	{
		ai_debug_idle_look_clear(actor_index);
	}
#endif

	prop_iterator_new(&iterator, actor_index);

	while (prop = prop_iterator_next(&iterator))
	{
		boolean valid;
		real interest = 0.f;
		boolean interesting = FALSE;

		if (prop->state>=_prop_state_becoming_unacknowledged && prop->state<=_prop_state_acknowledged && prop->visibility)
		{
			if (prop->look_interest>0.f)
			{
				real time_modifier;

				if (prop->last_idle_look_time==NONE)
				{
					time_modifier = 1.f;
				}
				else
				{
					time_modifier = ((real)current_time - (real)prop->last_idle_look_time) / (20 * TICKS_PER_SECOND) - 1.f;
				}

				time_modifier = (prop->look_interest - prop->last_idle_look_interest) / prop->look_interest + time_modifier;
				interest = prop->look_interest * MIN(time_modifier, 1.f);
				interesting = prop->look_interest>prop->last_idle_look_interest;
			}
		}
		else
		{
			prop->last_idle_look_interest = 0.f;
		}

#ifdef DEBUG
		if (actor_index==ai_debug.selected_actor_index)
		{
			ai_debug_idle_look_addprop(iterator.index, interest);
		}
#endif

		if (interest>0.f)
		{
			if (aim_at_prop)
			{
				if (free_facing && interesting)
				{
					valid = TRUE;
				}
				else
				{
					valid = actor_look_valid_aim_vector(
						aiming_yaw_deviation,
						&actor->control.desired_facing_vector,
						&prop->actor_to_prop);
				}
			}
			else
			{
				valid = actor_look_valid_look_vector(
					looking_yaw_deviation,
					look_delta_deviations,
					&actor->control.desired_facing_vector,
					&actor->control.desired_aiming_vector,
					&prop->actor_to_prop);
			}

			if (valid && interest>best_interest)
			{
				best_prop = prop;
				best_prop_index = iterator.index;
				best_interest = interest;
				best_interesting = interesting;
			}
		}
	}

	if (best_prop_index!=NONE)
	{
		best_prop->last_idle_look_time = current_time;
		best_prop->last_idle_look_interest = best_prop->look_interest;
		direction->type = _direction_specification_prop;
		direction->prop_index = best_prop_index;
		*interesting_prop = best_interesting;
		result = TRUE;
	}

	return result;
}

static boolean actor_look_idle_new_major_direction(
	long actor_index,
	struct actor_idle_looking *looking_definition,
	real_vector3d const *base_vector,
	boolean free_facing,
	boolean major_is_aiming,
	boolean has_minor_variation)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean interesting_direction = FALSE;
	boolean result = FALSE;

	actor->control.idle_major_active = FALSE;

	if (!has_minor_variation)
	{
		result = actor_look_idle_find_prop(actor_index, free_facing, major_is_aiming, &actor->control.idle_major_direction, &interesting_direction);
	}

	if (!result)
	{
		real yaw_range;
		real pitch_range;
		real pitch_min;
		real_vector3d base_direction = *base_vector;

		if (major_is_aiming)
		{
			if (free_facing)
			{
				yaw_range = _pi;
			}
			else
			{
				yaw_range = MIN(actor_definition->looking.maximum_aiming_deviation.yaw, actor_definition->looking.idle_aiming_deviation.yaw);
			}

			pitch_range = MIN(actor_definition->looking.maximum_aiming_deviation.pitch, actor_definition->looking.idle_aiming_deviation.pitch);
			base_direction.k = 0.f;

			if (normalize3d(&base_direction)==0.f)
			{
				base_direction = *global_forward3d;
			}
		}
		else
		{
			yaw_range = MIN(actor_definition->looking.maximum_looking_deviation.yaw, actor_definition->looking.idle_looking_deviation.yaw);
			pitch_range = MIN(actor_definition->looking.maximum_looking_deviation.pitch, actor_definition->looking.idle_looking_deviation.pitch);
		}

		pitch_min = -pitch_range;

		if (actor->input.vehicle_gunner)
		{
			pitch_min = pitch_min / 2.f;
		}

		actor->control.idle_major_direction.type = _direction_specification_vector;
		result = actor_look_find_random_vector(
			&actor->input.position.head_position,
			&base_direction,
			TRUE,
			-yaw_range,
			yaw_range,
			pitch_min,
			pitch_range,
			&actor->control.idle_major_direction.vector);
	}

	if (result)
	{
		actor->control.idle_major_timer = actor_look_idle_timer(
			actor_index,
			looking_definition,
			major_is_aiming ? _idle_timer_aiming : _idle_timer_looking,
			interesting_direction);

		if (actor->control.idle_major_timer)
		{
			actor->control.idle_major_active = TRUE;
			actor->control.idle_major_is_aiming = major_is_aiming;
		}
		else
		{
			result = FALSE;
		}
	}

	return interesting_direction;
}

static void actor_look_idle_new_minor_direction(
	long actor_index,
	struct actor_idle_looking *looking_definition,
	real_vector3d const *base_vector)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean interesting_direction = FALSE;
	boolean result = FALSE;

	actor->control.idle_minor_active = FALSE;
	result = actor_look_idle_find_prop(actor_index, FALSE, FALSE, &actor->control.idle_minor_direction, &interesting_direction);

	if (!result)
	{
		real_vector3d look_vector;
		real pitch_range;
		real yaw_range;
		real *delta_angles;
		real pitch_min;
		real pitch_max;
		real yaw_min;
		real yaw_max;

		yaw_range = MIN(actor_definition->looking.maximum_looking_deviation.yaw, actor_definition->looking.idle_looking_deviation.yaw);
		pitch_range = MIN(actor_definition->looking.maximum_looking_deviation.pitch, actor_definition->looking.idle_looking_deviation.pitch);
		delta_angles = actor->state.mode==_actor_mode_combat ? actor_definition->looking.combat_looking_delta_angles : actor_definition->looking.noncombat_looking_delta_angles;
		yaw_min = MAX(-yaw_range, -delta_angles[0]);
		yaw_max = MIN(yaw_range, delta_angles[1]);
		pitch_min = -pitch_range;
		pitch_max = pitch_range;
		result = actor_look_find_random_vector(
			&actor->input.position.head_position,
			base_vector,
			FALSE,
			yaw_min,
			yaw_max,
			pitch_min,
			pitch_max,
			&look_vector);

		if (result)
		{
			actor->control.idle_minor_direction.type = _direction_specification_vector;
			actor->control.idle_minor_direction.vector = look_vector;
			interesting_direction = FALSE;
		}
	}

	if (result)
	{
		actor->control.idle_minor_timer = actor_look_idle_timer(actor_index, looking_definition, _idle_timer_looking, interesting_direction);

		if (actor->control.idle_minor_timer)
		{
			actor->control.idle_minor_active = TRUE;
		}
		else
		{
			result = FALSE;
		}
	}

	return;
}

static struct actor_idle_looking *actor_look_get_looking_definition(
	long actor_index)
{
	struct actor_idle_looking *looking_definition;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	switch (actor->orders.look.idle_look_type)
	{
		case _idle_look_guarding:
			looking_definition = &actor_definition->looking.idle_look_guard;
			break;

		case _idle_look_searching:
		case _idle_look_combat:
			looking_definition = &actor_definition->looking.idle_look_search_combat;
			break;

		default:
			looking_definition = &actor_definition->looking.idle_look_noncombat;
			break;
	}

	return looking_definition;
}

static long actor_look_idle_timer(
	long actor_index,
	struct actor_idle_looking *looking_definition,
	short timer_type,
	boolean interesting_direction)
{
	real time;
	long ticks;
	real min_time;
	real max_time;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct weapon_definition *weapon_definition = actor_get_weapon_definition(actor_index);

	switch (timer_type)
	{
		case _idle_timer_facing:
			min_time = looking_definition->idle_facing_time_lower_bound;
			max_time = looking_definition->idle_facing_time_upper_bound;
			break;

		case _idle_timer_aiming:
			min_time = looking_definition->idle_aim_time_lower_bound;
			max_time = looking_definition->idle_aim_time_upper_bound;
			break;

		case _idle_timer_looking:
			min_time = looking_definition->idle_look_time_lower_bound;
			max_time = looking_definition->idle_look_time_upper_bound;
			break;

		default:
			match_halt("c:\\halo\\SOURCE\\ai\\actor_looking.c", 986);
			break;
	}

	if (min_time<=0.f && max_time<=0.f)
	{
		char *mode_names[] =
		{
			"facing",
			"aiming",
			"looking",
		};

#ifdef DEBUG
		error(
			_error_silent,
			"%s: logic error in actor looking: %s mode should be disabled (%.1f %.1f)",
			ai_debug_describe_actor(actor_index, NONE, TRUE, temporary, sizeof(temporary)),
			mode_names[timer_type],
			min_time,
			max_time);
#endif
		time = 0.5f;
	}
	else
	{
		time = real_random_range(min_time, max_time);
	}

	if (weapon_definition && weapon_definition->weapon.ai_idle_look_time_modifier>0.f)
	{
		time *= weapon_definition->weapon.ai_idle_look_time_modifier;
	}

	if (interesting_direction)
	{
		time *= 1.5f;
	}

	ticks = fast_ftol(time * TICKS_PER_SECOND);

	return MAX(ticks, 1);
}

static boolean actor_look_find_random_vector(
	real_point3d const *center_point,
	real_vector3d const *base_vector,
	boolean avoid_obstructions,
	real yaw_min,
	real yaw_max,
	real pitch_min,
	real pitch_max,
	real_vector3d *result_vector)
{
	real_vector3d pitch_axis;
	short attempt;
	boolean success = FALSE;

	set_real_vector3d(&pitch_axis, -base_vector->j, base_vector->i, 0.f);

	if (normalize3d(&pitch_axis)==0.f)
	{
		pitch_axis = *global_left3d;
	}

	for (attempt = 0; attempt<10; attempt++)
	{
		real_vector3d random_vector;
		boolean clear = TRUE;
		real yaw = real_random_range(yaw_min, yaw_max);
		real pitch = real_random_range(pitch_min, pitch_max);

		random_vector = *base_vector;
		rotate_vector_about_axis(&random_vector, &pitch_axis, sine(pitch), cosine(pitch));
		rotate_vector_about_axis(&random_vector, global_up3d, sine(yaw), cosine(yaw));
		clear = TRUE;

		if (avoid_obstructions)
		{
			struct collision_result result;
			real_vector3d test_vector;

			ai_profile.meters[_ai_meter_collision_vector].current_count++;
			match_collision_log_begin_user("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1066, _collision_user_ai_look);
			scale_vector3d(&random_vector, 3.f, &test_vector);
			clear = !collision_test_vector(
				FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_structure_bit),
				center_point,
				&test_vector,
				NONE,
				&result);
			match_collision_log_end_user("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1070);
		}

		if (clear)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1077, result_vector);
			normalize3d(&random_vector);
			*result_vector = random_vector;
			success = TRUE;
			break;
		}
	}

	return success;
}

void actor_look_update(
	long actor_index)
{
	real aiming_yaw_deviation;
	real looking_yaw_deviation;
	real look_delta_deviations[2];
	short primary_priority;
	short primary_type;
	short secondary_priority;
	real_vector3d primary_vector;
	real_vector3d secondary_vector;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean awake = TRUE;
	boolean looking_vector_available = TRUE;
	boolean desire_aiming_at_target = FALSE;

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1103, &actor->control.desired_facing_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1104, &actor->control.desired_aiming_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1105, &actor->control.desired_looking_vector);

	if (actor->output.movement_type==_actor_movement_type_asleep)
	{
		looking_vector_available = FALSE;
		awake = FALSE;
	}
	else if (actor->input.vehicle_gunner)
	{
		looking_vector_available = TRUE;
	}
	else if (actor->output.movement_type!=_actor_movement_type_alert && actor->output.movement_type!=_actor_movement_type_combat)
	{
		looking_vector_available = FALSE;
	}
	else
	{
		looking_vector_available = actor_get_weapon(actor_index)!=NONE;
	}

	if (!awake)
	{
		actor->control.desired_aiming_vector = actor->control.desired_facing_vector;
		actor->control.desired_looking_vector = actor->control.desired_facing_vector;
	}
	else
	{
		boolean firing_burst;
		struct actor_idle_looking *looking_definition;
		boolean idle_facing;
		boolean idle_aiming;
		boolean idle_looking;
		boolean free_facing = actor->control.free_facing_vector;
		boolean free_aiming = TRUE;
		boolean free_looking = looking_vector_available;
		boolean optional_facing = actor->control.optional_facing_vector;
		boolean facing_movement = !free_facing;
		boolean facing_locked = FALSE;
		boolean aiming_locked = FALSE;

		aiming_yaw_deviation = actor_definition->looking.runtime_maximum_aiming_deviation_cosine.yaw;
		looking_yaw_deviation = actor_definition->looking.runtime_maximum_looking_deviation_cosine.yaw;

		if (actor->state.mode==_actor_mode_combat)
		{
			look_delta_deviations[0] = cosine(actor_definition->looking.combat_looking_delta_angles[0]);
			look_delta_deviations[1] = cosine(actor_definition->looking.combat_looking_delta_angles[1]);
		}
		else
		{
			look_delta_deviations[0] = cosine(actor_definition->looking.noncombat_looking_delta_angles[0]);
			look_delta_deviations[1] = cosine(actor_definition->looking.noncombat_looking_delta_angles[1]);
		}

		firing_burst = FALSE;

		if (actor_combat_currently_firing_burst(actor_index) && !actor->orders.combat.abort_burst)
		{
			struct direction_specification temporary_direction;

			temporary_direction.type = _direction_specification_target;

			if (actor_look_decode_direction(actor_index, &temporary_direction, &primary_vector))
			{
				primary_type = _direction_specification_target;
				primary_priority = _primary_priority_locked_aiming;
				desire_aiming_at_target = TRUE;
				firing_burst = TRUE;
			}
		}

		if (!firing_burst)
		{
			primary_priority = actor->orders.look.primary_priority;
			primary_type = NONE;

			if (actor->orders.look.primary_priority!=_primary_priority_none && actor->orders.look.primary_priority!=_primary_priority_face_360)
			{
				if (actor_look_decode_direction(actor_index, &actor->orders.look.primary_direction, &primary_vector))
				{
					primary_type = actor->orders.look.primary_direction.type;
					desire_aiming_at_target = primary_type==_direction_specification_target;
				}
				else
				{
					primary_priority = _primary_priority_none;
				}
			}
		}

		secondary_priority = _secondary_look_priority_none;

		if (actor->control.secondary_look_type>=_secondary_look_none &&
			actor->control.secondary_look_timer>0 &&
			actor_look_decode_direction(actor_index, &actor->control.secondary_look_direction, &secondary_vector))
		{
			secondary_priority = actor->control.secondary_look_priority;
		}

		if (actor->control.moving && actor_action_class(actor_index)==_action_class_transitory)
		{
			secondary_priority = MIN(secondary_priority, _secondary_look_priority_turn_and_aim);
		}

		if (primary_priority!=_primary_priority_none && primary_priority!=_primary_priority_face_360)
		{
			match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1226, &primary_vector);
		}

		if (secondary_priority!=_secondary_look_priority_none)
		{
			match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1230, &secondary_vector);
		}

		if (actor->control.secondary_look_timer>0 && !--actor->control.secondary_look_timer)
		{
#ifdef DEBUG
			if (ai_debug.print_secondary_looking)
			{
				console_printf(
					FALSE,
					"%s: look timer expire",
					ai_debug_describe_actor(actor_index, NONE, FALSE, temporary, sizeof(temporary)));
			}
#endif

			actor->control.secondary_look_type = _secondary_look_none;
			actor->control.secondary_look_priority = _secondary_look_priority_none;
		}

		actor->control.aiming_away_from_primary = FALSE;

		if (primary_priority>=_primary_priority_opportunity_aiming)
		{
			boolean aim_at_primary = FALSE;

			if (primary_priority>=_primary_priority_aiming &&
				(free_facing || actor_look_valid_aim_vector(aiming_yaw_deviation, &actor->control.desired_facing_vector, &primary_vector)))
			{
				aim_at_primary = TRUE;
			}
			else if (primary_priority>=_primary_priority_facing && optional_facing)
			{
				optional_facing = FALSE;
				free_facing = TRUE;
				aim_at_primary = TRUE;
			}

			if (aim_at_primary)
			{
				actor->control.desired_aiming_vector = primary_vector;
				free_aiming = FALSE;
				free_looking = looking_vector_available;

				if (primary_priority>=_primary_priority_locked_aiming)
				{
					aiming_locked = TRUE;

					if (free_looking)
					{
						actor->control.desired_looking_vector = primary_vector;
					}
				}
			}

			if (primary_priority==_primary_priority_opportunity_aiming)
			{
				primary_priority = free_facing ? _primary_priority_aiming : _primary_priority_none;
			}

			if (free_facing)
			{
				free_facing = FALSE;
				optional_facing = FALSE;
				facing_movement = primary_type==_direction_specification_movement;
				actor->control.desired_facing_vector = primary_vector;
				actor->control.face_exactly |= primary_priority==_primary_priority_exact_facing;
			}

			facing_locked = (!actor->control.free_facing_vector && !actor->control.optional_facing_vector) ||
				primary_priority>=_primary_priority_locked_facing;
		}

		switch (secondary_priority)
		{
			case _secondary_look_priority_idle_look:
			case _secondary_look_priority_idle_aim:
			case _secondary_look_priority_aim:
			case _secondary_look_priority_turn_and_aim:
			case _secondary_look_priority_stop_and_aim:
			{
				boolean valid_aiming = actor_look_valid_aim_vector(aiming_yaw_deviation, &actor->control.desired_facing_vector, &secondary_vector);

				if (!facing_locked && !aiming_locked &&
					((secondary_priority>=_secondary_look_priority_stop_and_aim && actor_move_force_stop(actor_index)) ||
					(secondary_priority>=_secondary_look_priority_turn_and_aim && (optional_facing || actor->control.free_facing_vector)) ||
					(secondary_priority>=_secondary_look_priority_aim && !valid_aiming && free_facing && free_aiming)))
				{
					if (!(actor->control.face_exactly && valid_aiming))
					{
						actor->control.desired_facing_vector = secondary_vector;
						actor->control.face_exactly = FALSE;
						facing_movement = FALSE;
					}

					actor->control.desired_aiming_vector = secondary_vector;
					actor->control.desired_looking_vector = secondary_vector;
					actor->control.aiming_away_from_primary = TRUE;
					aiming_locked = FALSE;
					optional_facing = FALSE;
					free_facing = FALSE;
					free_aiming = FALSE;
					free_looking = looking_vector_available;
				}
				else if (!aiming_locked && valid_aiming &&
					(secondary_priority>=_secondary_look_priority_turn_and_aim || (secondary_priority>=_secondary_look_priority_idle_aim && free_aiming)))
				{
					actor->control.desired_aiming_vector = secondary_vector;
					actor->control.desired_looking_vector = secondary_vector;
					actor->control.aiming_away_from_primary = TRUE;
					aiming_locked = FALSE;
					free_aiming = FALSE;
					free_looking = looking_vector_available;
				}
				else if (free_looking &&
					actor_look_valid_look_vector(
						looking_yaw_deviation,
						look_delta_deviations,
						&actor->control.desired_facing_vector,
						&actor->control.desired_aiming_vector,
						&secondary_vector))
				{
					actor->control.desired_looking_vector = secondary_vector;
					free_looking = FALSE;
				}
				else if (free_aiming && valid_aiming)
				{
					actor->control.desired_aiming_vector = secondary_vector;
					actor->control.desired_looking_vector = secondary_vector;
					free_aiming = FALSE;
					free_looking = TRUE;
				}
				break;
			}

			case _secondary_look_priority_override:
			case _secondary_look_priority_override_exact_facing:
			{
				boolean aim_at_secondary = FALSE;
				boolean face_secondary = secondary_priority==_secondary_look_priority_override_exact_facing;
				boolean face_exactly = secondary_priority==_secondary_look_priority_override_exact_facing;

				if (actor->control.free_facing_vector)
				{
					face_secondary = TRUE;
					aim_at_secondary = TRUE;
				}
				else if (actor_look_valid_aim_vector(aiming_yaw_deviation, &actor->control.desired_facing_vector, &secondary_vector))
				{
					aim_at_secondary = TRUE;
				}
				else if (actor_move_force_stop(actor_index))
				{
					face_exactly = TRUE;
					face_secondary = TRUE;
					aim_at_secondary = TRUE;
				}

				if (aim_at_secondary)
				{
					if (face_secondary)
					{
						actor->control.desired_facing_vector = secondary_vector;
						actor->control.face_exactly = face_exactly;
						facing_locked = FALSE;
						facing_movement = FALSE;
					}

					actor->control.desired_aiming_vector = secondary_vector;
					actor->control.desired_looking_vector = secondary_vector;
					actor->control.aiming_away_from_primary = TRUE;
					aiming_locked = FALSE;
					optional_facing = FALSE;
					free_facing = FALSE;
					free_aiming = FALSE;
					free_looking = looking_vector_available;
				}
				break;
			}
		}

		if (primary_priority==_primary_priority_opportunity_aiming &&
			free_aiming &&
			actor_look_valid_aim_vector(aiming_yaw_deviation, &actor->control.desired_facing_vector, &primary_vector))
		{
			actor->control.desired_aiming_vector = primary_vector;

			if (free_looking)
			{
				actor->control.desired_looking_vector = primary_vector;
			}

			actor->control.aiming_away_from_primary = FALSE;
			free_aiming = FALSE;
		}

		looking_definition = actor_look_get_looking_definition(actor_index);
		idle_facing = looking_definition->idle_facing_time_upper_bound>0.f;
		idle_aiming = looking_definition->idle_aim_time_upper_bound>0.f;
		idle_looking = looking_definition->idle_look_time_upper_bound>0.f;

		if (actor->orders.look.idle_look_type>_idle_look_none &&
			!aiming_locked &&
			(free_aiming || free_looking) &&
			(idle_facing || idle_aiming || idle_looking))
		{
			real_vector3d idle_major_vector;
			boolean major_valid = FALSE;
			boolean new_major_direction = FALSE;
			boolean minor_is_aiming = FALSE;
			boolean major_is_facing = idle_facing && free_facing && primary_priority==_primary_priority_face_360 && actor->control.idle_facing_timer==0;

			if (actor->control.idle_facing_timer>0)
			{
				actor->control.idle_facing_timer--;
			}

			if (actor->control.idle_major_active && actor->control.idle_major_is_aiming && !free_aiming)
			{
				actor->control.idle_major_active = TRUE;
				actor->control.idle_major_timer = actor_look_idle_timer(actor_index, looking_definition, _idle_timer_looking, TRUE);
				actor->control.idle_major_direction.type = _direction_specification_vector;
				actor->control.idle_major_direction.vector = actor->control.desired_aiming_vector;
			}

			if (!actor->control.idle_major_active || actor->control.idle_major_timer==0)
			{
				boolean major_is_aiming = TRUE;
				boolean major_enabled = free_aiming && idle_aiming;

				if (!major_enabled)
				{
					major_is_aiming = FALSE;
					major_enabled = free_looking && idle_looking;
				}

				if (major_enabled)
				{
					actor->control.idle_major_direction_is_interesting = actor_look_idle_new_major_direction(
						actor_index,
						looking_definition,
						major_is_aiming ? &actor->control.desired_facing_vector : &actor->control.desired_aiming_vector,
						major_is_facing,
						major_is_aiming,
						major_is_aiming && free_looking && idle_looking);
					match_assert("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1501, !actor->control.idle_major_active || (actor->control.idle_major_timer > 0));
					new_major_direction = TRUE;
				}
			}

			if (actor->control.idle_major_active)
			{
				actor->control.idle_major_timer--;

				if (actor_look_decode_direction(actor_index, &actor->control.idle_major_direction, &idle_major_vector))
				{
					match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1512, &idle_major_vector);

					if (free_aiming)
					{
						if (free_facing && idle_facing && actor->state.flying)
						{
							major_is_facing = TRUE;
							minor_is_aiming = TRUE;
						}

						if (major_is_facing)
						{
							major_valid = TRUE;
							facing_movement = FALSE;
							actor->control.desired_facing_vector = idle_major_vector;
							actor->control.desired_aiming_vector = idle_major_vector;
							actor->control.aiming_away_from_primary = TRUE;
						}
						else
						{
							major_valid = actor_look_valid_aim_vector(aiming_yaw_deviation, &actor->control.desired_facing_vector, &idle_major_vector);

							if (major_valid)
							{
								actor->control.desired_aiming_vector = idle_major_vector;
								actor->control.aiming_away_from_primary = TRUE;
							}
						}
					}
					else
					{
						major_valid = actor_look_valid_look_vector(
							looking_yaw_deviation,
							look_delta_deviations,
							&actor->control.desired_facing_vector,
							&actor->control.desired_aiming_vector,
							&idle_major_vector);

						if (major_valid)
						{
							actor->control.desired_looking_vector = idle_major_vector;
						}
					}
				}

				if (major_valid && new_major_direction && idle_looking)
				{
					actor->control.idle_minor_active = TRUE;
					actor->control.idle_minor_timer = actor_look_idle_timer(
						actor_index,
						looking_definition,
						_idle_timer_looking,
						actor->control.idle_major_direction_is_interesting);
					actor->control.idle_minor_direction = actor->control.idle_major_direction;

					if (major_is_facing)
					{
						actor->control.idle_facing_timer = actor_look_idle_timer(
							actor_index,
							looking_definition,
							_idle_timer_facing,
							actor->control.idle_major_direction_is_interesting);
					}
				}
			}

			if (!major_valid)
			{
				actor->control.idle_major_active = FALSE;
				idle_major_vector = actor->control.desired_aiming_vector;
			}

			if (free_aiming && ((free_looking && idle_looking) || (minor_is_aiming && idle_aiming)))
			{
				if (actor->control.idle_minor_timer==0)
				{
					actor_look_idle_new_minor_direction(actor_index, looking_definition, &idle_major_vector);
					match_assert("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1581, actor->control.idle_minor_timer > 0);
				}

				actor->control.idle_minor_timer--;

				if (actor->control.idle_minor_active)
				{
					real_vector3d idle_minor_vector;
					boolean minor_valid = FALSE;

					if (actor_look_decode_direction(actor_index, &actor->control.idle_minor_direction, &idle_minor_vector))
					{
						if (minor_is_aiming)
						{
							minor_valid = actor_look_valid_aim_vector(aiming_yaw_deviation, &actor->control.desired_facing_vector, &idle_minor_vector);
						}
						else
						{
							minor_valid = actor_look_valid_look_vector(
								looking_yaw_deviation,
								look_delta_deviations,
								&actor->control.desired_facing_vector,
								&actor->control.desired_aiming_vector,
								&idle_minor_vector);
						}
					}

					if (minor_valid)
					{
						match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1605, &idle_minor_vector);

						if (minor_is_aiming)
						{
							actor->control.desired_aiming_vector = idle_minor_vector;
						}

						actor->control.desired_looking_vector = idle_minor_vector;
					}
					else
					{
						actor->control.idle_minor_active = FALSE;
					}
				}
			}
			else
			{
				actor->control.idle_minor_active = FALSE;
			}
		}
		else
		{
			actor->control.idle_major_active = FALSE;
			actor->control.idle_major_direction_is_interesting = FALSE;
			actor->control.idle_minor_active = FALSE;
		}

		if (!actor->control.moving &&
			!actor->control.moving_forced_by_aiming &&
			!unit_is_busy(actor->meta.unit_index) &&
			actor->input.vehicle_index==NONE)
		{
			if (actor_look_valid_aim_vector(aiming_yaw_deviation, &actor->control.desired_facing_vector, &actor->control.desired_aiming_vector) &&
				!actor_look_valid_aim_vector(aiming_yaw_deviation, &actor->input.facing_vector, &actor->control.desired_aiming_vector))
			{
				actor->control.face_exactly = TRUE;
			}
			else if (looking_vector_available &&
				actor_look_valid_look_vector(
					looking_yaw_deviation,
					look_delta_deviations,
					&actor->control.desired_facing_vector,
					&actor->control.desired_aiming_vector,
					&actor->control.desired_looking_vector) &&
				!actor_look_valid_look_vector(
					looking_yaw_deviation,
					look_delta_deviations,
					&actor->input.facing_vector,
					&actor->control.desired_aiming_vector,
					&actor->control.desired_looking_vector))
			{
				actor->control.face_exactly = TRUE;
			}
		}

		if (!looking_vector_available)
		{
			actor->control.desired_looking_vector = actor->control.desired_aiming_vector;
		}
	}

	if (!actor->state.flying)
	{
		if (!realcmp(actor->control.desired_facing_vector.k, 0.f))
		{
			actor->control.desired_facing_vector.k = 0.f;

			if (normalize2d((real_vector2d *)&actor->control.desired_facing_vector)==0.f)
			{
				actor->control.desired_facing_vector = actor->input.facing_vector;
			}
		}

		match_assert_valid_real_normal2d("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1673, (real_vector2d *) &actor->output.facing_vector);
	}

	if (actor->control.desire_stationary_facing)
	{
		if (!actor->control.fixed_stationary_facing)
		{
			if (!actor->control.moving && dot_product3d(&actor->input.aiming_vector, &actor->control.desired_aiming_vector)>0.9f)
			{
				actor->control.fixed_stationary_facing = TRUE;
				actor->control.fixed_stationary_facing_vector = actor->control.desired_facing_vector;
			}
		}
		else if (actor_definition->defensive.stationary_facing_angle>0.f)
		{
			boolean stationary_facing_valid = TRUE;
			real stationary_facing_cosine = cosine(actor_definition->defensive.stationary_facing_angle);

			if (actor->state.flying)
			{
				stationary_facing_valid =
					dot_product3d(&actor->control.desired_facing_vector, &actor->control.fixed_stationary_facing_vector)>stationary_facing_cosine &&
					dot_product3d(&actor->control.desired_aiming_vector, &actor->control.fixed_stationary_facing_vector)>stationary_facing_cosine;
			}
			else
			{
				real_vector2d desired_facing = *(real_vector2d *)&actor->control.desired_facing_vector;
				real_vector2d desired_aiming = *(real_vector2d *)&actor->control.desired_aiming_vector;
				real_vector2d stationary_facing = *(real_vector2d *)&actor->control.fixed_stationary_facing_vector;

				if (normalize2d(&desired_facing)==0.f || normalize2d(&desired_aiming)==0.f || normalize2d(&stationary_facing)==0.f)
				{
					stationary_facing_valid = FALSE;
				}
				else
				{
					stationary_facing_valid =
						dot_product2d(&desired_facing, &stationary_facing)>stationary_facing_cosine &&
						dot_product2d(&desired_aiming, &stationary_facing)>stationary_facing_cosine;
				}
			}

			if (!stationary_facing_valid)
			{
				actor->control.fixed_stationary_facing = FALSE;
				actor_stimulus_abandon_stationary_facing(actor_index);
			}
		}
	}
	else
	{
		actor->control.fixed_stationary_facing = FALSE;
	}

	actor->output.facing_vector = actor->control.desired_facing_vector;
	actor->output.aiming_vector = actor->control.desired_aiming_vector;
	actor->output.looking_vector = actor->control.desired_looking_vector;
	actor_unit_control_exact_facing(actor_index, actor->control.face_exactly);

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1736, &actor->output.facing_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1737, &actor->output.aiming_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actor_looking.c", 1738, &actor->output.looking_vector);

	if (desire_aiming_at_target)
	{
		actor->output.aiming_speed = _unit_aiming_speed_alert;
	}
	else if (actor->orders.look.idle_look_type==_idle_look_combat)
	{
		actor->output.aiming_speed = _unit_aiming_speed_alert;
	}
	else
	{
		switch (actor->control.secondary_look_type)
		{
			case _secondary_look_weapon_impact:
			case _secondary_look_detonation:
			case _secondary_look_combat_stimulus_prop:
			case _secondary_look_damage:
			case _secondary_look_dangerous_object:
				actor->output.aiming_speed = _unit_aiming_speed_alert;
				break;

			default:
				actor->output.aiming_speed = _unit_aiming_speed_casual;
				break;
		}
	}

	return;
}
