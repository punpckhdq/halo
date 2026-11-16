/*
ACTIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actions.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "ai_profile.h"
#include "ai_script.h"
#include "ai_globals.h"
#include "vehicles.h"

/* ---------- constants */

enum
{
	_active_cover_seeking_wrong_action = 0, /* fake name */
	_active_cover_seeking_visible_target, /* fake name */
	_active_cover_seeking_repeat_timer, /* fake name */
	_active_cover_seeking_visible_timer, /* fake name */
	_active_cover_seeking_shielded, /* fake name */
	_active_cover_seeking_unavailable, /* fake name */
	_active_cover_seeking_success, /* fake name */
	_active_cover_seeking_panic, /* fake name */
};

/* ---------- structures */

struct dive_animation_possibility /* fake name */
{
	short animation_impulse; /* fake name */
	short animation_direction;
	real animation_desire; /* fake name */
};

/* ---------- prototypes */

static boolean actor_action_allowed_to_enter_vehicle(long actor_index, long vehicle_index);
static boolean actor_action_find_escape_from_danger(long actor_index, short *escape_direction_reference, real *escape_distance_reference, real_vector2d *alignment_vector_reference, boolean *escape_is_ledge_reference);
static void actor_action_determine_pursuit_options(long actor_index, short desire_target_search, short desire_pursuit, short desire_pursuit_search, boolean controlling_group_pursuit, boolean controlled_by_group_pursuit, boolean stand_down, boolean must_charge, boolean *allow_target_uncover, boolean *allow_indefinite_target_uncover, boolean *allow_target_search, boolean *allow_pursuit, boolean *allow_pursuit_search, boolean *allow_waiting);
static boolean actor_pursuit_consider_nearby_actor(long actor_index, boolean pursuit_coordinator, long friend_actor_index);

/* ---------- globals */

struct action_specification const global_action_functions[NUMBER_OF_ACTOR_ACTIONS] =
{
	{ _actor_action_none, "none", &global_real_argb_black, 0, _action_class_noncombat, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
	{ _actor_action_sleep, "sleep", &global_real_argb_lightblue, 0, _action_class_noncombat, NULL, NULL, NULL, action_sleep_control, NULL, NULL, NULL, NULL, NULL },
	{ _actor_action_alert, "alert", &global_real_argb_cyan, sizeof(struct alert_state_data), _action_class_noncombat, action_alert_begin, action_alert_perform, action_alert_update, action_alert_control, NULL, NULL, NULL, action_alert_flush_position_indices, action_alert_flush_structure_indices },
	{ _actor_action_fight, "fight", &global_real_argb_white, sizeof(struct fight_state_data), _action_class_active, action_fight_begin, action_fight_perform, action_fight_update, action_fight_control, action_fight_end, NULL, NULL, NULL, NULL },
	{ _actor_action_flee, "flee", &global_real_argb_yellow, sizeof(struct flee_state_data), _action_class_transitory, action_flee_begin, action_flee_perform, action_flee_update, action_flee_control, action_flee_end, action_flee_modify_color, action_flee_replace_prop, action_flee_flush_position_indices, NULL },
	{ _actor_action_uncover, "uncover", &global_real_argb_blue, sizeof(struct uncover_state_data), _action_class_pursuit, action_uncover_begin, action_uncover_perform, action_uncover_update, action_uncover_control, NULL, action_uncover_modify_color, NULL, action_uncover_flush_position_indices, NULL },
	{ _actor_action_guard, "guard", &global_real_argb_salmon, sizeof(struct guard_state_data), _action_class_passive, action_guard_begin, action_guard_perform, action_guard_update, action_guard_control, action_guard_end, action_guard_modify_color, action_guard_replace_prop, action_guard_flush_position_indices, action_guard_flush_structure_indices },
	{ _actor_action_search, "search", &global_real_argb_aqua, sizeof(struct search_state_data), _action_class_pursuit, action_search_begin, action_search_perform, action_search_update, action_search_control, NULL, NULL, NULL, action_search_flush_position_indices, NULL },
	{ _actor_action_wait, "wait", &global_real_argb_magenta, sizeof(struct wait_state_data), _action_class_pursuit, action_wait_begin, action_wait_perform, action_wait_update, action_wait_control, NULL, NULL, NULL, NULL, NULL },
	{ _actor_action_vehicle, "vehicle", &global_real_argb_darkgreen, sizeof(struct vehicle_state_data), _action_class_transitory, action_vehicle_begin, action_vehicle_perform, action_vehicle_update, action_vehicle_control, NULL, NULL, NULL, NULL, NULL },
	{ _actor_action_charge, "charge", &global_real_argb_red, sizeof(struct charge_state_data), _action_class_active, action_charge_begin, action_charge_perform, action_charge_update, action_charge_control, NULL, NULL, NULL, NULL, NULL },
	{ _actor_action_obey, "obey", &global_real_argb_purple, sizeof(struct obey_state_data), _action_class_transitory, action_obey_begin, action_obey_perform, action_obey_update, action_obey_control, action_obey_end, NULL, NULL, NULL, NULL },
	{ _actor_action_converse, "converse", &global_real_argb_orange, sizeof(struct converse_state_data), _action_class_transitory, action_converse_begin, action_converse_perform, action_converse_update, action_converse_control, action_converse_end, NULL, action_converse_replace_prop, NULL, NULL },
	{ _actor_action_avoid, "avoid", &global_real_argb_grey, sizeof(struct avoid_state_data), _action_class_transitory, action_avoid_begin, action_avoid_perform, action_avoid_update, action_avoid_control, action_avoid_end, NULL, NULL, NULL, NULL }
};

struct dive_animation_possibility const global_dive_animation_table[] =
{
	{ _unit_animation_impulse_dive_left, _actor_evade_left, 1.5f },
	{ _unit_animation_impulse_evade_left, _actor_evade_left, 0.f },
	{ _unit_animation_impulse_dive_right, _actor_evade_right, 1.5f },
	{ _unit_animation_impulse_evade_right, _actor_evade_right, 0.f },
	{ _unit_animation_impulse_dive_front, _actor_evade_forward, 1.5f },
	{ _unit_animation_impulse_dive_back, _actor_evade_back, 1.5f },
	{ NONE, NONE, 0.f }
};

const short global_state_move_position_orders[NUMBER_OF_ACTOR_DEFAULT_STATES] =
{
	_move_position_order_none,						// _actor_default_state_none
	_move_position_order_none,						// _actor_default_state_asleep
	_move_position_order_none,						// _actor_default_state_alert
	_move_position_order_repeat,					// _actor_default_state_moving_repeat_position
	_move_position_order_loop,						// _actor_default_state_moving_loop
	_move_position_order_loop_back_and_forth,		// _actor_default_state_moving_loop_back_and_forth
	_move_position_order_loop_randomly,				// _actor_default_state_moving_loop_randomly
	_move_position_order_random,					// _actor_default_state_moving_randomly
	_move_position_order_none,						// _actor_default_state_guarding
	_move_position_order_none,						// _actor_default_state_guarding_at_guard_point
	_move_position_order_none,						// _actor_default_state_searching
	_move_position_order_none,						// _actor_default_state_fleeing
};

const short global_default_states[NUMBER_OF_ACTOR_DEFAULT_STATES] =
{
	_actor_default_state_none,						// _actor_default_state_none
	_actor_default_state_alert,						// _actor_default_state_asleep
	_actor_default_state_alert,						// _actor_default_state_alert
	_actor_default_state_moving_repeat_position,	// _actor_default_state_moving_repeat_position
	_actor_default_state_moving_loop,				// _actor_default_state_moving_loop
	_actor_default_state_moving_loop_back_and_forth,// _actor_default_state_moving_loop_back_and_forth
	_actor_default_state_moving_loop_randomly,		// _actor_default_state_moving_loop_randomly
	_actor_default_state_moving_randomly,			// _actor_default_state_moving_randomly
	_actor_default_state_guarding,					// _actor_default_state_guarding
	_actor_default_state_guarding_at_guard_point,	// _actor_default_state_guarding_at_guard_point
	_actor_default_state_guarding_at_guard_point,	// _actor_default_state_searching
	_actor_default_state_guarding,					// _actor_default_state_fleeing
};

char const *global_actor_mode_names[NUMBER_OF_ACTOR_MODES] =
{
	"asleep",
	"alert",
	"combat",
};

/* ---------- public code */

boolean actor_action_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 143, (actor->state.action >= 0) && (actor->state.action < NUMBER_OF_ACTOR_ACTIONS));

	if (global_action_functions[actor->state.action].perform)
	{
		result = global_action_functions[actor->state.action].perform(actor_index);
	}

	return result;
}

void actor_action_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 158, (actor->state.action >= 0) && (actor->state.action < NUMBER_OF_ACTOR_ACTIONS));

	if (global_action_functions[actor->state.action].update)
	{
		global_action_functions[actor->state.action].update(actor_index);
	}

	return;
}

void actor_action_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 173, (actor->state.action >= 0) && (actor->state.action < NUMBER_OF_ACTOR_ACTIONS));

	if (global_action_functions[actor->state.action].control)
	{
		global_action_functions[actor->state.action].control(actor_index);
	}

	return;
}

void actor_action_replace_prop(
	long actor_index,
	long invalid_prop_index,
	long replacement_prop_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 190, (actor->state.action >= 0) && (actor->state.action < NUMBER_OF_ACTOR_ACTIONS));

	if (global_action_functions[actor->state.action].replace_prop)
	{
		global_action_functions[actor->state.action].replace_prop(actor_index, invalid_prop_index, replacement_prop_index);
	}

	return;
}

void actor_action_flush_position_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 205, (actor->state.action >= 0) && (actor->state.action < NUMBER_OF_ACTOR_ACTIONS));

	if (global_action_functions[actor->state.action].flush_position_indices)
	{
		global_action_functions[actor->state.action].flush_position_indices(actor_index);
	}

	return;
}

void actor_action_flush_structure_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 220, (actor->state.action >= 0) && (actor->state.action < NUMBER_OF_ACTOR_ACTIONS));

	if (global_action_functions[actor->state.action].flush_structure_indices)
	{
		global_action_functions[actor->state.action].flush_structure_indices(actor_index);
	}

	return;
}

boolean actor_action_set_default_state(
	long actor_index,
	short override_state)
{
	struct actor_datum *actor = actor_get(actor_index);
	long current_time = game_time_get();
	boolean result = FALSE;

	if (override_state!=NONE ||
		actor->state.last_default_state_time==NONE ||
		actor->state.last_default_state_time + 45<current_time)
	{
		short default_state;
		struct action_state_data new_state_data;

		actor->state.last_default_state_time = current_time;

		if (override_state!=NONE)
		{
			default_state = override_state;
		}
		else if (actor->state.initial_state!=NONE)
		{
			default_state = actor->state.initial_state;
			actor->state.initial_state = NONE;
		}
		else if (actor->state.default_state!=NONE)
		{
			default_state = actor->state.default_state;
		}
		else
		{
			default_state = _actor_default_state_none;
		}

		switch (default_state)
		{
			case _actor_default_state_asleep:
				if (actor->state.mode!=_actor_mode_asleep)
				{
					actor->state.mode = _actor_mode_asleep;
					actor_action_change(actor_index, _actor_action_sleep, NULL);
					result = TRUE;
				}
				break;

			case _actor_default_state_guarding:
				if ((actor->state.action!=_actor_action_guard ||
					actor->state.action_data.guard.guard_location_type!=_actor_guard_location_current) &&
					action_guard_setup_current_position(actor_index, &new_state_data.guard))
				{
					actor_action_change(actor_index, _actor_action_guard, &new_state_data);
					result = TRUE;
				}
				break;

			case _actor_default_state_guarding_at_guard_point:
				if (actor->state.action==_actor_action_guard)
				{
					if (actor->state.action_data.guard.guard_location_type!=_actor_guard_location_firing_position)
					{
						actor->state.action_data.guard.find_new_guard_position = TRUE;
					}
				}
				else if (action_guard_setup_find_position(actor_index, 0, &new_state_data.guard))
				{
					actor_action_change(actor_index, _actor_action_guard, &new_state_data);
					result = TRUE;
				}
				break;

			case _actor_default_state_searching:
				if (actor_action_class(actor_index)!=_action_class_pursuit)
				{
					actor->state.mode = _actor_mode_combat;
					actor->state.artificial_combat_status = _actor_combat_status_investigate;
					actor->state.combat_status = _actor_combat_status_investigate;

					if (!actor_action_handle_lost_contact(actor_index) &&
						action_guard_setup_current_position(actor_index, &new_state_data.guard))
					{
						actor_action_change(actor_index, _actor_action_guard, &new_state_data);
						result = TRUE;
					}
				}
				break;

			case _actor_default_state_fleeing:
				if (actor->state.action!=_actor_action_flee)
				{
					if (action_flee_setup(actor_index, _actor_panic_scripted, NONE, TRUE, FALSE, FALSE, &new_state_data.flee))
					{
						actor_action_change(actor_index, _actor_action_flee, &new_state_data);
						result = TRUE;
					}
					else if (actor->state.action!=_actor_action_guard &&
						action_guard_setup_current_position(actor_index, &new_state_data.guard))
					{
						actor_action_change(actor_index, _actor_action_guard, &new_state_data);
						result = TRUE;
					}
				}
				break;

			case _actor_default_state_none:
			case _actor_default_state_alert:
			case _actor_default_state_moving_repeat_position:
			case _actor_default_state_moving_loop:
			case _actor_default_state_moving_loop_back_and_forth:
			case _actor_default_state_moving_loop_randomly:
			case _actor_default_state_moving_randomly:
				if ((actor->state.action!=_actor_action_alert ||
					actor->state.action_data.alert.move_position_order!=global_state_move_position_orders[default_state]) &&
					action_alert_setup(actor_index, global_state_move_position_orders[default_state], NONE, &new_state_data.alert))
				{
					actor_action_change(actor_index, _actor_action_alert, &new_state_data);
					result = TRUE;
				}
				break;
		}

		if (!result &&
			actor->state.action==_actor_action_none &&
			action_alert_setup(actor_index, global_state_move_position_orders[_actor_default_state_alert], NONE, &new_state_data.alert))
		{
			actor_action_change(actor_index, _actor_action_alert, &new_state_data);
			result = TRUE;
		}
	}

	return result;
}

boolean actor_action_handle_initial_action(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->state.action==_actor_action_none && actor->state.mode!=_actor_mode_braindead)
	{
		result = actor_action_set_default_state(actor_index, NONE);
	}

	return result;
}

boolean actor_action_handle_pending_command_list(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->state.command_list_index!=NONE &&
		(actor->state.command_list_immediate ||
		(actor->state.mode!=_actor_mode_braindead && !actor_action_deny_transition(actor_index))))
	{
		struct action_state_data new_state_data;

		if (action_obey_command_list_setup(actor_index, actor->state.command_list_index, &new_state_data.obey))
		{
			actor_action_change(actor_index, _actor_action_obey, &new_state_data);
			result = TRUE;
		}

		actor->state.command_list_immediate = FALSE;
		actor->state.command_list_index = NONE;
	}

	return result;
}

boolean actor_action_handle_surprise(
	long actor_index,
	short minimum_surprise_level)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	boolean result = FALSE;

	if (!actor->input.vehicle_passenger && actor->stimuli.surprise_level>=minimum_surprise_level)
	{
		real_vector2d alignment_vector;
		long cause_unit_index;
		long hostility;
		short animation_impulse;

		if (actor->stimuli.surprise_has_vector)
		{
			alignment_vector = *(real_vector2d *)&actor->stimuli.surprise_vector;
			normalize2d(&alignment_vector);

			if (dot_product2d(&alignment_vector, (real_vector2d *)&actor->control.desired_facing_vector)>=0.f)
			{
				animation_impulse = _unit_animation_impulse_surprise_front;
			}
			else
			{
				animation_impulse = _unit_animation_impulse_surprise_back;
				negate_vector2d(&alignment_vector, &alignment_vector);
			}
		}
		else
		{
			alignment_vector = *(real_vector2d *)&actor->input.facing_vector;
			normalize2d(&alignment_vector);
			animation_impulse = _unit_animation_impulse_surprise_front;
		}

		actor_move_animation_impulse(actor_index, animation_impulse, &alignment_vector);
		cause_unit_index = NONE;
		hostility = _comm_hostility_none;

		if (actor->stimuli.surprise_prop_index!=NONE)
		{
			struct prop_datum *prop = prop_get(actor->stimuli.surprise_prop_index);

			cause_unit_index = prop->unit_index;
			hostility = prop->enemy ? _comm_hostility_enemy : _comm_hostility_friend;
		}

		ai_communication_event(_ai_communication_surprise, actor->meta.unit_index, cause_unit_index, hostility, NONE, NONE, NULL);

		if (actor_variant_definition->ranged_combat.surprise_wildfire_time>0.f)
		{
			actor_combat_fire_wildly(actor_index, (short)(actor_variant_definition->ranged_combat.surprise_wildfire_time * TICKS_PER_SECOND));
		}

		if (actor_variant_definition->ranged_combat.surprise_delay_time>0.f)
		{
			actor_combat_disable_bursts(actor_index, (short)(actor_variant_definition->ranged_combat.surprise_delay_time * TICKS_PER_SECOND));
		}

		actor_stimulus_was_surprised(actor_index);

		if (actor->stimuli.surprise_prop_index!=NONE)
		{
			actor_situation_try_new_target(actor_index, actor->stimuli.surprise_prop_index);
		}

		result = TRUE;
	}

	actor->stimuli.surprise_level = _actor_surprise_none;

	return result;
}

boolean actor_action_handle_panic_from_surprise(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean result = FALSE;

	if (actor->stimuli.was_surprised && TEST_FLAG(actor_definition->flags, _actor_definition_panic_on_surprise_bit))
	{
		if (actor->stimuli.panic_type==_actor_panic_none || actor->stimuli.panic_prop_index==NONE)
		{
			actor->stimuli.panic_prop_index = actor->stimuli.surprise_prop_index;
		}

		actor->stimuli.panic_type = MAX(actor->stimuli.panic_type, _actor_panic_surprise);
		actor->stimuli.was_surprised = FALSE;
		result = TRUE;
	}

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 528, (actor->stimuli.panic_type == 0) || (actor->stimuli.panic_prop_index != 0x00000000));

	return result;
}

boolean actor_action_handle_panic_from_damage(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean result = FALSE;

	if (actor->stimuli.was_damaged &&
		((game_connection()==_game_connection_local && ai_debug.flee_always) ||
		actor->input.recent_body_damage>actor_definition->panic.panic_damage_threshold))
	{
		if (actor->stimuli.panic_type==_actor_panic_none || actor->stimuli.panic_prop_index==NONE)
		{
			actor->stimuli.panic_prop_index = actor_perception_find_recent_damaging_prop_index(actor_index, TRUE);
		}

		actor->stimuli.panic_type = MAX(actor->stimuli.panic_type, _actor_panic_damage);
		actor->stimuli.was_damaged = FALSE;
		result = TRUE;
	}

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 552, (actor->stimuli.panic_type == 0) || (actor->stimuli.panic_prop_index != 0x00000000));

	return result;
}

boolean actor_action_handle_panic_from_burning_to_death(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->input.burning_to_death)
	{
		long panic_prop_index = NONE;

		if (actor->meta.unit_index!=NONE)
		{
			struct unit_datum *unit = unit_get(actor->meta.unit_index);
			long responsible_unit_index = ai_get_responsible_unit(unit->unit.flaming_death_attacker_object_index, TRUE);

			if (responsible_unit_index!=NONE)
			{
				panic_prop_index = prop_get_active_by_unit_index(actor_index, responsible_unit_index);
			}
		}

		if (actor->stimuli.panic_type==_actor_panic_none || actor->stimuli.panic_prop_index==NONE)
		{
			actor->stimuli.panic_prop_index = panic_prop_index;
		}

		actor->stimuli.panic_type = MAX(actor->stimuli.panic_type, _actor_panic_burning_to_death);
		result = TRUE;
	}

	return result;
}

boolean actor_action_handle_panic_from_attached_projectiles(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->input.delayed_attached_projectile_index!=NONE)
	{
		struct object_datum *projectile = object_try_and_get(actor->input.delayed_attached_projectile_index);
		long panic_prop_index = NONE;

		if (projectile)
		{
			long responsible_unit_index = ai_get_responsible_unit(projectile->object.owner_object_index, TRUE);

			if (responsible_unit_index!=NONE)
			{
				panic_prop_index = prop_get_active_by_unit_index(actor_index, responsible_unit_index);
			}
		}

		if (actor->stimuli.panic_type==_actor_panic_none || actor->stimuli.panic_prop_index==NONE)
		{
			actor->stimuli.panic_prop_index = panic_prop_index;
		}

		actor->stimuli.panic_type = MAX(actor->stimuli.panic_type, _actor_panic_delayed_projectile_attached_to_us);
		result = TRUE;
	}

	return result;
}

boolean actor_action_handle_panic_from_attached_melee_attackers(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->input.melee_attacker_attached)
	{
		actor->stimuli.panic_type = MAX(actor->stimuli.panic_type, _actor_panic_melee_attached_to_us);
		actor->stimuli.panic_prop_index = NONE;
		result = TRUE;
	}

	return result;
}

boolean actor_action_handle_panic_transition(
	long actor_index,
	short minimum_panic_level,
	boolean enforced_calm,
	short force_panic_level)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->stimuli.panic_type>=minimum_panic_level && !actor->input.vehicle_passenger)
	{
		if (actor->state.action==_actor_action_flee && actor->state.action_data.flee.panic_type>_actor_panic_none)
		{
			actor->state.action_data.flee.panic_type = MAX(actor->state.action_data.flee.panic_type, actor->stimuli.panic_type);
		}
		else if (actor->emotions.last_flee_failed_time==NONE || actor->emotions.last_flee_failed_time + 7<game_time_get())
		{
			boolean force_flee_transition = actor->stimuli.panic_type>=force_panic_level;

			match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 661, actor->stimuli.panic_prop_index != 0x00000000);

			if (enforced_calm && !force_flee_transition)
			{
				ai_communication_event(_ai_communication_attempted_flee, actor->meta.unit_index, NONE, NONE, NONE, NONE, NULL);
			}
			else
			{
				result = actor_action_try_to_panic(
					actor_index,
					actor->stimuli.panic_type,
					actor->stimuli.panic_prop_index,
					force_flee_transition);
			}
		}
	}

	actor->stimuli.panic_type = _actor_panic_none;

	return result;
}

boolean actor_action_handle_berserking_from_attacking_mode(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean result = FALSE;

	if (TEST_FLAG(actor_definition->flags, _actor_definition_always_berserk_in_attacking_mode_bit) &&
		!actor->external_orders.defending &&
		actor->state.combat_status>=_actor_combat_status_clear_los)
	{
		actor->stimuli.berserk_type = MAX(actor->stimuli.berserk_type, _actor_berserk_attacking);
		result = TRUE;
	}

	return result;
}

boolean actor_action_handle_berserking_from_attached_projectiles(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct actor_variant_definition *actor_variant_definition = actor_combat_get_firing_variant_definition(actor_index);
	boolean result = FALSE;

	if (actor->input.delayed_attached_projectile_index!=NONE && actor->state.combat_status>=_actor_combat_status_clear_los)
	{
		struct prop_datum *prop = prop_get(actor->target.target_prop_index);

		match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 712, actor->target.target_prop_index != NONE);

		if (prop->distance<actor_variant_definition->ranged_combat.berserk_firing_range_upper_bound &&
			real_random()<actor_definition->berserk.grenade_attached_chance)
		{
			actor->stimuli.berserk_type = MAX(actor->stimuli.berserk_type, _actor_berserk_attached_projectiles);
			result = TRUE;
		}
	}

	return result;
}

boolean actor_action_handle_berserking_from_proximity(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean result = FALSE;

	if (actor->state.combat_status>=_actor_combat_status_clear_los)
	{
		struct prop_datum *prop = prop_get(actor->target.target_prop_index);

		match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 737, actor->target.target_prop_index != NONE);

		if (prop->distance<actor_definition->berserk.proximity_berserk_distance)
		{
			actor->stimuli.berserk_type = MAX(actor->stimuli.berserk_type, _actor_berserk_proximity);
			result = TRUE;
		}
	}

	return result;
}

boolean actor_action_handle_berserking_from_damage(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean result = FALSE;

	if (actor->stimuli.was_damaged &&
		actor->input.recent_body_damage>actor_definition->berserk.damage_berserk_amount &&
		actor->input.body_vitality<actor_definition->berserk.damage_berserk_threshold)
	{
		actor->stimuli.berserk_type = MAX(actor->stimuli.berserk_type, _actor_berserk_damage);
		actor->stimuli.was_damaged = FALSE;
		result = TRUE;
	}

	return result;
}

boolean actor_action_handle_berserk_transition(
	long actor_index,
	short berserk_level)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->stimuli.berserk_type>=berserk_level && !actor->emotions.berserk)
	{
		actor_berserk(actor_index, TRUE);

		if (actor->state.combat_status>=_actor_combat_status_certain)
		{
			result = actor_action_handle_combat_selection(actor_index);
		}
	}

	actor->stimuli.berserk_type = _actor_berserk_none;

	return result;
}

boolean actor_action_handle_combat_transition(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->state.mode<_actor_mode_combat && actor->stimuli.combat_transition!=_actor_combat_status_none)
	{
		struct action_state_data new_state_data;

		actor->state.mode = _actor_mode_combat;

		if (action_guard_setup_from_combat_transition(actor_index, &new_state_data.guard))
		{
			actor_action_change(actor_index, _actor_action_guard, &new_state_data);
		}
		else
		{
			actor_action_handle_combat_selection(actor_index);
		}

		actor->stimuli.combat_transition = _actor_combat_status_none;
		result = TRUE;
	}
	else if (actor->state.mode==_actor_mode_combat &&
		actor->state.combat_status==_actor_combat_status_none &&
		actor_action_class(actor_index)==_action_class_active)
	{
	}

	return result;
}

boolean actor_action_deny_transition(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->state.command_list_index!=NONE && actor->state.command_list_delay_timer>0)
	{
		result = TRUE;
	}

	if (actor->meta.encounter_index!=NONE)
	{
		struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);
		struct squad_datum *squad = encounter_get_squad(encounter, actor->meta.squad_index);

		if (squad->delay_timer>0)
		{
			if (actor->state.combat_status>=_actor_combat_status_clear_los)
			{
				encounter_squad_timer_expire(actor->meta.encounter_index, actor->meta.squad_index);
			}
			else
			{
				result = TRUE;
			}
		}
	}

	if (actor->state.action==_actor_action_obey &&
		!actor->state.action_data.obey.initiative &&
		!actor->state.action_data.obey.finished)
	{
		result = TRUE;
	}

	return result;
}

static boolean actor_action_allowed_to_enter_vehicle(
	long actor_index,
	long vehicle_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = TRUE;

	if (vehicle_index==actor->emotions.vehicle_ignore_index && game_time_get()<actor->emotions.vehicle_ignore_time)
	{
		result = FALSE;
	}

	return result;
}

boolean actor_action_handle_vehicle_entry(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	long current_time = game_time_get();
	boolean result = FALSE;

	if ((actor->state.action==_actor_action_flee && actor->state.action_data.flee.panic_type>_actor_panic_none) ||
		actor->state.action==_actor_action_obey)
	{
	}
	else if (actor->emotions.last_vehicle_check_time==NONE || actor->emotions.last_vehicle_check_time + 45<current_time)
	{
		real best_distance_squared = REAL_MAX;
		real attempt_distance = REAL_MAX;
		real continue_distance = REAL_MAX;
		long vehicle_index = NONE;

		actor->emotions.last_vehicle_check_time = current_time;

		if (TEST_FLAG(actor_definition->flags, _actor_definition_helps_players_in_vehicles_bit))
		{
			struct prop_iterator iterator;
			struct prop_datum *prop;

			prop_iterator_new(&iterator, actor_index);

			while (prop = prop_iterator_next(&iterator))
			{
				if (prop->state>=_prop_state_becoming_unacknowledged &&
					prop->state<=_prop_state_acknowledged &&
					prop->player &&
					!prop->enemy &&
					prop->vehicle_index!=NONE &&
					actor_action_allowed_to_enter_vehicle(actor_index, prop->vehicle_index))
				{
					struct vehicle_datum *vehicle = vehicle_try_and_get(prop->vehicle_index);

					if (vehicle && vehicle->unit.driver_object_index==prop->unit_index)
					{
						real_point3d vehicle_origin;
						real distance_squared;

						object_get_origin(prop->vehicle_index, &vehicle_origin);
						distance_squared = distance_squared3d(&actor->input.position.body_position, &vehicle_origin);

						if (distance_squared<100.f && distance_squared<best_distance_squared)
						{
							vehicle_index = prop->vehicle_index;
							best_distance_squared = (prop->distance) * (prop->distance);
							attempt_distance = 8.f;
							continue_distance = 10.f;
						}
					}
				}
			}
		}

		if (vehicle_index==NONE && actor->state.certain_combat_timer>=60)
		{
			short enterable_index;

			for (enterable_index = 0; enterable_index<ai_globals->enterable_vehicle_count; enterable_index++)
			{
				struct ai_vehicle_enterable *enterable = &ai_globals->enterable_vehicles[enterable_index];

				if (vehicle_try_and_get(enterable->vehicle_index) &&
					actor_action_allowed_to_enter_vehicle(actor_index, enterable->vehicle_index))
				{
					real_point3d position;
					real distance_squared;

					object_get_origin(enterable->vehicle_index, &position);
					distance_squared = distance_squared3d(&position, &actor->input.position.body_position);

					if (distance_squared<best_distance_squared)
					{
						if (enterable->radius!=REAL_MAX && distance_squared>enterable->radius * enterable->radius)
						{
							continue;
						}

						if (enterable->team_bitmask>0 &&
							(actor->meta.team_index==NONE || !TEST_FLAG(enterable->team_bitmask, actor->meta.team_index)))
						{
							continue;
						}

						if (enterable->actor_type_bitmask>0 && !TEST_FLAG(enterable->actor_type_bitmask, actor->meta.type))
						{
							continue;
						}

						if (enterable->ai_indices_count>0)
						{
							boolean valid = FALSE;
							short ai_index_index;

							for (ai_index_index = 0; ai_index_index<enterable->ai_indices_count; ai_index_index++)
							{
								long ai_index = enterable->ai_indices[ai_index_index];

								if (ai_index!=NONE)
								{
									valid = DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index)==AI_INDEX_GET_ENCOUNTER_INDEX(ai_index);

									if (valid)
									{
										switch (AI_INDEX_GET_TYPE(ai_index))
										{
											case _ai_index_platoon:
												valid = actor->meta.platoon_index==AI_INDEX_GET_SUBINDEX(ai_index);
												break;

											case _ai_index_squad:
												valid = actor->meta.squad_index==AI_INDEX_GET_SUBINDEX(ai_index);
												break;
										}
									}

									if (valid)
									{
										break;
									}
								}
							}

							if (!valid)
							{
								continue;
							}
						}

						vehicle_index = enterable->vehicle_index;
						best_distance_squared = distance_squared;
						attempt_distance = enterable->radius + 3.f;
						continue_distance = enterable->radius + 6.f;
					}
				}
			}
		}

		if (vehicle_index!=NONE)
		{
			struct action_state_data new_state_data;

			if (action_vehicle_setup_impromptu(actor_index, vehicle_index, attempt_distance, continue_distance, &new_state_data.vehicle))
			{
				actor_action_change(actor_index, _actor_action_vehicle, &new_state_data);
				result = TRUE;
			}
		}
	}

	return result;
}

boolean actor_action_handle_vehicle_exit(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->input.vehicle_index!=NONE)
	{
		struct prop_iterator iterator;
		struct prop_datum *prop;
		boolean exit_vehicle = FALSE;
		boolean exit_forced = FALSE;

		prop_iterator_new(&iterator, actor_index);

		while (prop = prop_iterator_next(&iterator))
		{
			if (prop->state>=_prop_state_becoming_unacknowledged &&
				prop->state<=_prop_state_acknowledged &&
				prop->player &&
				prop->enemy &&
				prop->vehicle_index==actor->input.vehicle_index)
			{
				exit_vehicle = TRUE;
				exit_forced = TRUE;
				break;
			}
		}

		if (actor->stimuli.vehicle_eviction)
		{
			exit_vehicle = TRUE;
		}

		if (actor->input.vehicle_passenger &&
			(actor->input.delayed_attached_projectile_index!=NONE ||
			(actor->danger_zone.danger_type==_actor_danger_zone_projectile && actor->danger_zone.attached_to_us)))
		{
			exit_vehicle = TRUE;
			exit_forced = TRUE;
		}

		if (exit_vehicle)
		{
			actor->emotions.last_vehicle_exit_forced = exit_forced;

			if (unit_try_and_exit_seat(actor->meta.unit_index))
			{
				actor->emotions.vehicle_ignore_index = actor->input.vehicle_index;
				actor->emotions.vehicle_ignore_time = game_time_get() + 180;
				result = TRUE;
			}

			actor->emotions.last_vehicle_exit_forced = FALSE;
		}
	}

	actor->stimuli.vehicle_eviction = FALSE;

	return result;
}

boolean actor_action_handle_grenade_throwing(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	boolean result = FALSE;

	if (actor->target.target_type>=_actor_target_uninspected_orphan &&
		(actor->state.action!=_actor_action_flee || actor->state.action_data.flee.panic_type<=_actor_panic_none))
	{
		struct prop_datum *prop = prop_get(actor->target.target_prop_index);

		switch (actor_variant_definition->grenade_combat.stimulus_type)
		{
			case _actor_grenade_stimulus_visibletarget:
				if (actor->state.combat_status>=_actor_combat_status_clear_los)
				{
					result = actor_action_consider_grenade(actor_index);
				}
				break;

			case _actor_grenade_stimulus_seekcover:
				if (prop->swarm ||
					(actor->state.action==_actor_action_flee && actor->state.action_data.flee.panic_type==_actor_panic_none))
				{
					result = actor_action_consider_grenade(actor_index);
				}
				break;
		}

		if (actor->control.grenade_trying_to_throw)
		{
			actor_action_try_to_throw_grenade(actor_index, FALSE);
		}
	}
	else
	{
		actor->control.grenade_trying_to_throw = FALSE;
	}

	return result;
}

boolean actor_action_allow_cover_seeking(
	long actor_index,
	boolean unopposable)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean result = TRUE;

	if (!unopposable && !actor->external_orders.playfighting)
	{
		if (actor_definition->defensive.hide_target_not_visible_time>0.f)
		{
			if (actor->state.combat_status>=_actor_combat_status_visible)
			{
				result = FALSE;
			}
			else
			{
				short hide_ticks = (short)(actor_definition->defensive.hide_target_not_visible_time * TICKS_PER_SECOND);

				if (actor->target.target_last_visible_time!=NONE &&
					game_time_get()<actor->target.target_last_visible_time + hide_ticks)
				{
					result = FALSE;
				}
			}
		}

		if (actor_definition->defensive.cover_damage_threshold>0.f &&
			actor->input.recent_body_damage<actor_definition->defensive.cover_damage_threshold)
		{
			result = FALSE;
		}
	}

	if (actor->emotions.berserk)
	{
		result = FALSE;
	}

	if (actor->input.vehicle_passenger)
	{
		result = FALSE;
	}

	return result;
}

boolean actor_action_handle_evasion(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	long current_time = game_time_get();
	boolean result = FALSE;

	if (actor->emotions.unopposable_retreat_timer>0 && actor->input.vehicle_index==NONE)
	{
		struct prop_datum *prop = prop_get(actor->emotions.unopposable_retreat_prop_index);

		if (prop->unopposable_enemy &&
			(prop->line_of_sight==_ai_line_of_sight_clear || prop->line_of_sight==_ai_line_of_sight_occluded) &&
			(actor->emotions.last_defensive_cover_seeking_time==NONE || actor->emotions.last_defensive_cover_seeking_time + 30<=current_time))
		{
			actor->emotions.last_defensive_cover_seeking_time = current_time;

			if (actor_action_allow_cover_seeking(actor_index, TRUE))
			{
				if (actor_action_try_to_seek_cover(actor_index, FALSE, TRUE))
				{
					result = TRUE;
				}
				else if (TEST_FLAG(actor_definition->flags, _actor_definition_panicked_by_unopposable_enemy_bit) &&
					actor_action_try_to_panic(actor_index, _actor_panic_unopposable_enemy, actor->emotions.unopposable_retreat_prop_index, FALSE))
				{
					result = TRUE;
				}
			}
		}
	}

	if (!result)
	{
		real danger_threshold;

		if (actor->emotions.currently_defending && !actor->emotions.berserk)
		{
			danger_threshold = actor_definition->defensive.evasion_danger_threshold_defending;
		}
		else
		{
			danger_threshold = actor_definition->defensive.evasion_danger_threshold_attacking;
		}

		if (actor->external_orders.playfighting && actor_definition->defensive.evasion_seek_cover_chance>0.f)
		{
			danger_threshold = MIN(danger_threshold, 1.1f);
		}

		if (actor->emotions.perceived_danger>danger_threshold)
		{
			boolean allow_cover_seeking;
			boolean allow_evasion;

			if (!actor->control.moving)
			{
				real_random();
			}

			if (actor_variant_definition->grenade_combat.stimulus_type==_actor_grenade_stimulus_seekcover &&
				actor_action_consider_grenade(actor_index))
			{
				actor->emotions.perceived_danger = 0.f;
				result = TRUE;
			}

			allow_cover_seeking = TRUE;
			allow_evasion = TRUE;

			if (actor->emotions.defensive_crouch && TEST_FLAG(actor_definition->flags, _actor_definition_crouch_try_not_to_move_bit))
			{
				allow_cover_seeking = FALSE;

				if (actor->target.target_prop_index!=NONE)
				{
					struct prop_datum *prop = prop_get(actor->target.target_prop_index);

					if (prop->quantized_facing<=_prop_facing_central && prop->quantized_distance<=_prop_distance_near)
					{
						allow_cover_seeking = TRUE;
					}
				}
			}

			if (actor->state.action==_actor_action_charge &&
				(actor->state.action_data.charge.goal==_charge_goal_melee ||
				actor->state.action_data.charge.goal==_charge_goal_melee_leaping))
			{
				allow_evasion = FALSE;
			}

			if (!result &&
				allow_cover_seeking &&
				(actor->emotions.last_defensive_cover_seeking_time==NONE || actor->emotions.last_defensive_cover_seeking_time + 30<=current_time))
			{
				actor->emotions.last_defensive_cover_seeking_time = current_time;

				if (actor_action_allow_cover_seeking(actor_index, FALSE) &&
					real_random()<actor_definition->defensive.evasion_seek_cover_chance &&
					actor_action_try_to_seek_cover(actor_index, FALSE, TRUE))
				{
					ai_communication_event(_ai_communication_cover, actor->meta.unit_index, actor_target_unit_index(actor_index), NONE, NONE, NONE, NULL);
					actor->emotions.perceived_danger = 0.f;
					result = TRUE;
				}
			}

			if (!result &&
				allow_evasion &&
				actor->emotions.evasion_delay_timer==0 &&
				actor_action_try_to_evade(actor_index))
			{
				actor->emotions.perceived_danger = 0.f;
				actor->emotions.evasion_delay_timer = (short)(actor_definition->defensive.evasion_delay_timer * TICKS_PER_SECOND);
				actor->firing_positions.moved_away_from_firing_position = TRUE;
				result = TRUE;
			}
		}
	}

	return result;
}

static boolean actor_action_find_escape_from_danger(
	long actor_index,
	short *escape_direction_reference,
	real *escape_distance_reference,
	real_vector2d *alignment_vector_reference,
	boolean *escape_is_ledge_reference)
{
	real_vector2d alignment_vector;
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *unit = unit_get(actor->meta.unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	boolean result = FALSE;
	boolean escape_is_ledge = FALSE;
	short escape_direction = NONE;
	real escape_distance = unit_definition->unit.dive_distance;

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 1393, actor->danger_zone.danger_type > _actor_danger_zone_none);

	if (escape_distance>0.f)
	{
		struct actor_definition *actor_definition;
		real_vector3d danger_vector;
		real_point3d left_point;
		real_point3d right_point;
		real_vector3d left_vector;
		real_vector3d right_vector;
		real left_distance;
		real right_distance;
		boolean left_available;
		boolean left_safe;
		boolean left_is_ledge;
		boolean right_available;
		boolean right_safe;
		boolean right_is_ledge;
		struct path_collision_result collision_result;
		real maximum_ledge_height;

		set_real_vector2d(&alignment_vector, -actor->danger_zone.velocity.i, -actor->danger_zone.velocity.j);

		if (normalize2d(&alignment_vector)<1.f / TICKS_PER_SECOND)
		{
			vector_from_points2d(
				(real_point2d *)&actor->input.position.body_position,
				(real_point2d *)&actor->danger_zone.position,
				&alignment_vector);

			if (normalize2d(&alignment_vector)==0.f)
			{
				alignment_vector = *(real_vector2d *)&actor->input.facing_vector;

				if (normalize2d(&alignment_vector)==0.f)
				{
					alignment_vector = *global_forward2d;
				}
			}
		}

		actor_definition = actor_definition_get(actor->meta.definition_index);
		maximum_ledge_height = TEST_FLAG(actor_definition->flags, _actor_definition_dive_off_ledges_bit) ? 8.f : 0.f;
		vector_from_points3d(&actor->danger_zone.position, &actor->danger_zone.predict_danger_position, &danger_vector);
		set_real_vector3d(&left_vector, -alignment_vector.j, alignment_vector.i, 0.f);
		set_real_vector3d(&right_vector, alignment_vector.j, -alignment_vector.i, 0.f);
		point_from_line3d(&actor->input.position.body_position, &left_vector, escape_distance, &left_point);
		point_from_line3d(&actor->input.position.body_position, &right_vector, escape_distance, &right_point);

		left_available = actor_move_try_evasion_vector(
			actor_index,
			(real_vector2d *)&left_vector,
			escape_distance,
			maximum_ledge_height,
			&left_is_ledge,
			&collision_result);
		left_distance = point_to_line_distance3d(&left_point, &actor->danger_zone.position, &danger_vector);
		left_safe = left_available && left_distance>actor->danger_zone.danger_radius;

		right_available = actor_move_try_evasion_vector(
			actor_index,
			(real_vector2d *)&right_vector,
			escape_distance,
			maximum_ledge_height,
			&right_is_ledge,
			&collision_result);
		right_distance = point_to_line_distance3d(&right_point, &actor->danger_zone.position, &danger_vector);
		right_safe = right_available && right_distance>actor->danger_zone.danger_radius;

		if (left_available)
		{
			if (right_available)
			{
				real distance_difference = left_distance - right_distance;

				if (left_is_ledge>right_is_ledge || left_safe>right_safe || distance_difference>0.3f)
				{
					escape_direction = _actor_evade_left;
					escape_is_ledge = left_is_ledge;
					result = TRUE;
				}
				else if (right_is_ledge>left_is_ledge || right_safe>left_safe || distance_difference<-0.3f)
				{
					escape_direction = _actor_evade_right;
					escape_is_ledge = right_is_ledge;
					result = TRUE;
				}
				else
				{
					escape_direction = _actor_evade_random_side;
					escape_is_ledge = left_is_ledge;
					result = left_safe;
				}
			}
			else
			{
				escape_direction = _actor_evade_left;
				escape_is_ledge = left_is_ledge;
				result = left_safe;
			}
		}
		else if (right_available)
		{
			escape_direction = _actor_evade_right;
			escape_is_ledge = right_is_ledge;
			result = right_safe;
		}
	}

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 1484, escape_direction_reference && escape_distance_reference && alignment_vector_reference);

	*escape_direction_reference = escape_direction;
	*escape_distance_reference = escape_distance;
	*escape_is_ledge_reference = escape_is_ledge;
	*alignment_vector_reference = alignment_vector;

	return result;
}

boolean actor_action_handle_danger_avoidance(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_debug_info *actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
	boolean result = FALSE;
	boolean near_danger = FALSE;
	boolean movement_into_danger = FALSE;

	actor_debug_info->danger_avoidance_time = game_time_get();
	actor_debug_info->danger_decision = _danger_avoidance_none;
	actor_debug_info->dive_decision = _dive_not_attempted;
	actor_debug_info->danger_abandoned_path = FALSE;

	if (actor->danger_zone.danger_type==_actor_danger_zone_none)
	{
		actor_debug_info->danger_decision = _danger_avoidance_none;
	}
	else if (!actor->danger_zone.noticed_danger)
	{
		actor_debug_info->danger_decision = _danger_avoidance_unnoticed;
	}
	else if (actor->danger_zone.attached_to_us)
	{
		actor_debug_info->danger_decision = _danger_avoidance_attached_to_us;
	}
	else if (distance_squared3d(&actor->input.position.body_position, &actor->danger_zone.bounding_sphere_center)>
		(actor->danger_zone.bounding_sphere_radius + 3.f) * (actor->danger_zone.bounding_sphere_radius + 3.f))
	{
		actor_debug_info->danger_decision = _danger_avoidance_far_away;
		actor_debug_info->danger_far_dist = distance3d(&actor->input.position.body_position, &actor->danger_zone.bounding_sphere_center);
		actor_debug_info->danger_far_radius = actor->danger_zone.bounding_sphere_radius + 3.f;
	}
	else if (actor_move_animation_busy(actor_index))
	{
		actor_debug_info->danger_decision = _danger_avoidance_animation_busy;
	}
	else if (actor->input.vehicle_passenger)
	{
		actor_debug_info->danger_decision = _danger_avoidance_vehicle;
	}
	else
	{
		real_vector3d danger_vector;
		real distance_squared;
		boolean inside_danger_zone;
		boolean destination_inside_danger_zone;

		vector_from_points3d(&actor->danger_zone.position, &actor->danger_zone.predict_danger_position, &danger_vector);
		distance_squared = point_to_line_distance_squared3d(&actor->input.position.body_position, &actor->danger_zone.position, &danger_vector);
		inside_danger_zone = distance_squared<actor->danger_zone.danger_radius * actor->danger_zone.danger_radius;
		near_danger = distance_squared<(actor->danger_zone.danger_radius + 3.5f) * (actor->danger_zone.danger_radius + 3.5f);

		if (actor_path_has_path(actor_index))
		{
			real destination_distance_squared = point_to_line_distance_squared3d(
				&actor->control.path.path.endpoint.point,
				&actor->danger_zone.position,
				&danger_vector);

			destination_inside_danger_zone = destination_distance_squared<actor->danger_zone.danger_radius * actor->danger_zone.danger_radius;

			if (actor->control.moving && !inside_danger_zone && !destination_inside_danger_zone)
			{
				real_vector3d predicted_motion;
				real motion_distance_squared;

				scale_vector3d(&actor->control.moving_towards_vector, 3.f, &predicted_motion);
				motion_distance_squared = vector_to_line_distance_squared3d(
					&actor->input.position.body_position,
					&predicted_motion,
					&actor->danger_zone.position,
					&danger_vector);
				movement_into_danger = motion_distance_squared<actor->danger_zone.danger_radius * actor->danger_zone.danger_radius;
			}
		}
		else
		{
			destination_inside_danger_zone = inside_danger_zone;
			movement_into_danger = FALSE;
		}

		if ((destination_inside_danger_zone || (movement_into_danger && !inside_danger_zone)) &&
			actor->firing_positions.current_position_index!=NONE)
		{
			actor_discard_firing_position(actor_index, actor->firing_positions.current_position_index, TRUE);
			actor_debug_info->danger_abandoned_path = TRUE;
		}

		if (inside_danger_zone)
		{
			short escape_direction;
			real dive_distance;
			real_vector2d alignment_vector;
			boolean escape_is_ledge;
			boolean escape_safe;
			boolean imminent = FALSE;
			real intersect_time = fast_vector_intersection_with_sphere(
				&actor->danger_zone.position,
				&danger_vector,
				&actor->input.position.body_position,
				actor->danger_zone.danger_radius);

			if (intersect_time<REAL_MAX)
			{
				intersect_time *= 45.f;
			}

			switch (actor->danger_zone.danger_type)
			{
				case _actor_danger_zone_suicide:
					if (intersect_time==0.f &&
						actor->danger_zone.suicide.time_until_death!=NONE &&
						actor->danger_zone.suicide.time_until_death<30)
					{
						imminent = TRUE;
					}
					break;

				case _actor_danger_zone_projectile:
					if (intersect_time==0.f &&
						actor->danger_zone.projectile.time_until_explosion!=NONE &&
						actor->danger_zone.projectile.time_until_explosion<20)
					{
						imminent = TRUE;
					}
					break;

				case _actor_danger_zone_vehicle:
					if (intersect_time<30.f)
					{
						imminent = TRUE;
					}
					break;
			}

			if ((actor->danger_zone.hostility==_actor_danger_hostility_enemy || imminent) && !actor->danger_zone.communicated)
			{
				short hostility = NONE;

				switch (actor->danger_zone.hostility)
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

				if (actor->danger_zone.danger_type==_actor_danger_zone_projectile)
				{
					ai_communication_event(_ai_communication_grenade_danger, actor->meta.unit_index, NONE, hostility, NONE, NONE, NULL);
				}

				actor->danger_zone.communicated = TRUE;
			}

			escape_safe = actor_action_find_escape_from_danger(actor_index, &escape_direction, &dive_distance, &alignment_vector, &escape_is_ledge);

			if (escape_direction==NONE)
			{
				actor_debug_info->danger_decision = _danger_avoidance_no_safe_direction;
			}
			else if (actor->danger_zone.allow_dive_evasion && actor->input.vehicle_index==NONE)
			{
				boolean dive = FALSE;

				switch (actor->danger_zone.danger_type)
				{
					case _actor_danger_zone_suicide:
						if ((escape_safe || escape_is_ledge) && intersect_time==0.f)
						{
							dive = TRUE;
							actor_debug_info->danger_decision = _danger_avoidance_can_avoid;
						}
						else if (imminent)
						{
							dive = TRUE;
							actor_debug_info->danger_decision = _danger_avoidance_imminent_explosion;
						}
						break;

					case _actor_danger_zone_projectile:
						if ((escape_safe || escape_is_ledge) && intersect_time<7.f)
						{
							dive = TRUE;
							actor_debug_info->danger_decision = _danger_avoidance_can_avoid;
						}
						else if (imminent)
						{
							dive = TRUE;
							actor_debug_info->danger_decision = _danger_avoidance_imminent_explosion;
						}
						break;

					case _actor_danger_zone_vehicle:
						if (imminent)
						{
							dive = TRUE;
							actor_debug_info->danger_decision = _danger_avoidance_imminent_impact;
						}
						break;
				}

				if (dive)
				{
					struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

					result = actor_action_try_to_dive(
						actor_index,
						escape_direction,
						dive_distance,
						&alignment_vector,
						TEST_FLAG(actor_definition->flags, _actor_definition_dive_off_ledges_bit) ? 8.f : 0.f);
				}
				else
				{
					actor_debug_info->danger_decision = _danger_avoidance_no_desire;
					actor_debug_info->danger_intersect_time = intersect_time;
				}
			}
			else
			{
				actor_debug_info->danger_decision = _danger_avoidance_evasion_disallowed;
			}
		}
		else
		{
			actor_debug_info->danger_decision = _danger_avoidance_outside_zone;
			actor_debug_info->danger_zone_dist = square_root(distance_squared);
			actor_debug_info->danger_zone_radius = actor->danger_zone.danger_radius;
		}
	}

	if (!result &&
		near_danger &&
		(actor_path_at_destination(actor_index) || !actor->control.moving || movement_into_danger) &&
		!actor->emotions.forced_to_charge)
	{
		short action_class = actor_action_class(actor_index);

		if (action_class==_action_class_passive || action_class==_action_class_pursuit)
		{
			struct action_state_data new_state_data;

			if (action_avoid_setup(actor_index, &new_state_data.avoid))
			{
				actor_action_change(actor_index, _actor_action_avoid, &new_state_data);
				result = TRUE;
			}
		}
	}

	return result;
}

boolean actor_action_handle_active_cover_seeking(
	long actor_index,
	boolean allow_panicking,
	boolean force_panicking)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_debug_info *actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	boolean result = FALSE;

	if (actor->meta.timeslice)
	{
		actor_debug_info->field_B8 = TRUE;
		actor_debug_info->field_BA = _active_cover_seeking_shielded;
		actor_debug_info->field_BC = actor->target.target_last_visible_time==NONE ? 1000 : game_time_get() - actor->target.target_last_visible_time;
		actor_debug_info->field_C0 = actor->input.shield_vitality;

		if (actor->input.shield_vitality<=actor_definition->defensive.shield_fraction_hide)
		{
			short action_class = actor_action_class(actor_index);

			actor_debug_info->field_BA = _active_cover_seeking_wrong_action;

			if (!actor->emotions.berserk && (action_class==_action_class_active || action_class==_action_class_pursuit))
			{
				actor_debug_info->field_BA = _active_cover_seeking_visible_target;

				if (actor->state.combat_status>=_actor_combat_status_investigate)
				{
					long current_time = game_time_get();

					actor_debug_info->field_BA = _active_cover_seeking_repeat_timer;

					if (actor->emotions.last_active_cover_seeking_time==NONE ||
						current_time>=actor->emotions.last_active_cover_seeking_time + 30)
					{
						actor_debug_info->field_BA = _active_cover_seeking_visible_timer;
						actor->emotions.last_active_cover_seeking_time = current_time;

						if (actor_action_allow_cover_seeking(actor_index, FALSE))
						{
							actor_debug_info->field_BA = _active_cover_seeking_unavailable;

							if (actor_action_try_to_seek_cover(actor_index, TRUE, FALSE))
							{
								actor_debug_info->field_BA = _active_cover_seeking_success;
								result = TRUE;
							}
							else if (allow_panicking &&
								actor_action_try_to_panic(actor_index, _actor_panic_no_cover_available, actor->target.target_prop_index, force_panicking))
							{
								actor_debug_info->field_BA = _active_cover_seeking_panic;
								result = TRUE;
							}
						}
					}
				}
			}
		}
	}

	return result;
}

boolean actor_action_handle_combat_selection(
	long actor_index)
{
	struct action_state_data new_state_data;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	struct actor_variant_definition *firing_variant_definition = actor_combat_get_firing_variant_definition(actor_index);
	boolean result = FALSE;
	struct prop_datum *prop = NULL;
	real target_distance = REAL_MAX;

	if (actor->target.target_prop_index!=NONE)
	{
		prop = prop_get(actor->target.target_prop_index);
		target_distance = prop->distance;
	}

	if (prop &&
		actor->state.action==_actor_action_charge &&
		actor->state.action_data.charge.goal==_charge_goal_stalking)
	{
		boolean discovered = FALSE;

		if (prop->currently_damaging_me)
		{
			discovered = TRUE;
		}
		else if (prop->shooting && prop->quantized_distance<=_prop_distance_near)
		{
			discovered = TRUE;
		}
		else if (actor_definition->defensive.stalking_discovery_time>0.f &&
			actor->state.action_data.charge.stalking_discovery_timer>=(short)(actor_definition->defensive.stalking_discovery_time * TICKS_PER_SECOND))
		{
			discovered = TRUE;
		}

		if (discovered)
		{
			if (target_distance>firing_variant_definition->ranged_combat.combat_range_upper_bound)
			{
				result = actor_action_try_to_seek_cover(actor_index, FALSE, FALSE);
			}

			if (!result)
			{
				actor_berserk(actor_index, TRUE);
			}
		}
	}

	if (prop)
	{
		if ((!actor_has_ranged_weapon(actor_index) || (prop->vehicle_index==NONE && !prop->swarm)) &&
			(actor->state.action!=_actor_action_charge ||
			(actor->state.action_data.charge.goal!=_charge_goal_melee &&
			actor->state.action_data.charge.goal!=_charge_goal_melee_leaping)) &&
			!result &&
			!actor->meta.swarm &&
			actor->input.vehicle_index==NONE &&
			actor->control.fire_state!=_actor_fire_state_bursting)
		{
			real melee_delay;
			real melee_range;
			boolean melee;
			struct unit_datum *unit = unit_get(actor->meta.unit_index);
			long current_time = game_time_get();
			boolean berserk_melee = actor->emotions.berserk;

			if (!actor_has_ranged_weapon(actor_index) && !TEST_FLAG(actor_definition->flags, _actor_definition_stalking_behavior_bit))
			{
				berserk_melee = TRUE;
			}

			melee_delay = actor->emotions.berserk ? 0.f : actor_definition->berserk.melee_attack_delay_timer;
			melee_delay = game_difficulty_get_value(_game_difficulty_melee_delay_bonus) +
				game_difficulty_get_value(_game_difficulty_melee_delay_scale) * melee_delay;
			melee_range = berserk_melee ?
				actor_variant_definition->ranged_combat.berserk_melee_range :
				actor_variant_definition->ranged_combat.melee_range;

			if (actor->emotions.last_melee_check_time!=NONE && actor->emotions.last_melee_check_time + 10>=current_time)
			{
				melee = FALSE;
			}
			else if (target_distance>melee_range)
			{
				melee = FALSE;
			}
			else if (actor->external_orders.disable_charging &&
				target_distance>MAX(0.f, actor_definition->berserk.melee_attack_range) + 0.8f)
			{
				melee = FALSE;
			}
			else if (actor->emotions.last_melee_attack_time!=NONE &&
				(real)actor->emotions.last_melee_attack_time + melee_delay * TICKS_PER_SECOND>=(real)current_time)
			{
				melee = FALSE;
			}
			else if (actor_has_ranged_weapon(actor_index) && !actor->input.underwater)
			{
				melee = TRUE;
			}
			else
			{
				melee = TRUE;
			}

			if (melee)
			{
				actor->emotions.last_melee_check_time = current_time;

				if (action_charge_setup(actor_index, _charge_goal_melee, &new_state_data.charge))
				{
					actor_action_change(actor_index, _actor_action_charge, &new_state_data);
					result = TRUE;
				}
			}
		}

		if (actor->state.action!=_actor_action_charge &&
			!actor->external_orders.disable_charging &&
			!result &&
			actor->input.vehicle_driver_type>_actor_vehicle_driver_none)
		{
			struct vehicle_datum *vehicle = vehicle_get(actor->input.vehicle_index);
			struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

			if (actor->emotions.last_vehicle_charge_time==NONE ||
				(real)actor->emotions.last_vehicle_charge_time + vehicle_definition->vehicle.ai_charge_repeat_time * TICKS_PER_SECOND<(real)game_time_get())
			{
				boolean vehicle_charge = FALSE;

				if (actor->input.vehicle_driver_type==_actor_vehicle_driver_directional_flying)
				{
					vehicle_charge = target_distance>firing_variant_definition->ranged_combat.melee_range &&
						prop->line_of_sight==_ai_line_of_sight_clear;
				}

				if (vehicle_charge && action_charge_setup(actor_index, _charge_goal_vehicle_strafing, &new_state_data.charge))
				{
					actor_action_change(actor_index, _actor_action_charge, &new_state_data);
					result = TRUE;
				}
			}
		}
	}

	if (!result)
	{
		boolean charge = actor->emotions.forced_to_charge && !actor->external_orders.disable_charging;
		boolean restart_charge = FALSE;

		if (!actor->external_orders.disable_charging &&
			!actor_has_ranged_weapon(actor_index) &&
			TEST_FLAG(actor_definition->flags, _actor_definition_charge_in_attacking_mode_bit))
		{
			charge = TRUE;
		}

		if (actor->state.action==_actor_action_charge)
		{
			if (actor->state.action_data.charge.goal==_charge_goal_melee ||
				actor->state.action_data.charge.goal==_charge_goal_melee_leaping)
			{
				if (actor->state.action_data.charge.finished_melee_attack ||
					actor->state.action_data.charge.aborted_melee_attack ||
					actor->state.action_data.charge.unable_to_advance)
				{
					restart_charge = TRUE;
				}
				else
				{
					charge = TRUE;
				}
			}
			else if (actor->external_orders.disable_charging)
			{
				charge = FALSE;
				restart_charge = TRUE;
			}
			else if (actor->state.action_data.charge.goal==_charge_goal_vehicle_strafing ||
				actor->state.action_data.charge.goal==_charge_goal_vehicle_ramming)
			{
				if (actor->state.action_data.charge.unable_to_advance ||
					actor->input.vehicle_driver_type<=_actor_vehicle_driver_unknown)
				{
					restart_charge = TRUE;
				}
				else
				{
					charge = TRUE;

					if (actor->state.action_data.charge.goal==_charge_goal_vehicle_strafing)
					{
						struct vehicle_datum *vehicle = vehicle_get(actor->input.vehicle_index);
						struct vehicle_definition *vehicle_definition = vehicle_definition_get(vehicle->definition_index);

						if (actor->control.path.at_destination &&
							actor->control.path.destination_orders.destination_type==_destination_prop &&
							actor->control.path.destination_orders.prop.prop_index==actor->target.target_prop_index)
						{
							charge = FALSE;
						}
						else if (target_distance<vehicle_definition->vehicle.ai_strafing_stop_range)
						{
							charge = FALSE;
						}
						else if (target_distance<2.f * vehicle_definition->vehicle.ai_strafing_stop_range &&
							dot_product3d(&actor->input.facing_vector, &prop->actor_to_prop)<0.5f)
						{
							charge = FALSE;
						}
					}
				}
			}
		}

		if (charge && (restart_charge || actor->state.action!=_actor_action_charge))
		{
			if (action_charge_setup(actor_index, _charge_goal_close_range, &new_state_data.charge))
			{
				actor_action_change(actor_index, _actor_action_charge, &new_state_data);
				result = TRUE;
			}
			else
			{
				charge = FALSE;
			}
		}

		if (!charge && !result && actor->state.action!=_actor_action_fight)
		{
			boolean success = action_fight_setup(actor_index, &new_state_data.fight);

			match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2171, success);
			actor_action_change(actor_index, _actor_action_fight, &new_state_data);
			result = TRUE;
		}

		if (!result)
		{
			if (charge)
			{
				match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2182, actor->state.action == _actor_action_charge);

				if (actor->state.action_data.charge.goal==_charge_goal_melee ||
					actor->state.action_data.charge.goal==_charge_goal_melee_leaping)
				{
					match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2185, !actor->state.action_data.charge.finished_melee_attack && !actor->state.action_data.charge.aborted_melee_attack && !actor->state.action_data.charge.unable_to_advance);
				}
				else if (actor->state.action_data.charge.goal==_charge_goal_vehicle_strafing ||
					actor->state.action_data.charge.goal==_charge_goal_vehicle_ramming)
				{
					match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2189, !actor->state.action_data.charge.unable_to_advance);
				}
			}
			else
			{
				match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2194, actor->state.action == _actor_action_fight);
			}
		}
	}

	return result;
}

static void actor_action_determine_pursuit_options(
	long actor_index,
	short desire_target_search,
	short desire_pursuit,
	short desire_pursuit_search,
	boolean controlling_group_pursuit,
	boolean controlled_by_group_pursuit,
	boolean stand_down,
	boolean must_charge,
	boolean *allow_target_uncover,
	boolean *allow_indefinite_target_uncover,
	boolean *allow_target_search,
	boolean *allow_pursuit,
	boolean *allow_pursuit_search,
	boolean *allow_waiting)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (desire_target_search==_actor_pursuit_never ||
		(desire_target_search==_actor_pursuit_not_when_group && controlling_group_pursuit))
	{
		*allow_target_search = FALSE;
	}

	if (desire_pursuit==_actor_pursuit_never ||
		(desire_pursuit==_actor_pursuit_not_when_group && controlling_group_pursuit))
	{
		*allow_pursuit = FALSE;
		*allow_pursuit_search = FALSE;
	}
	else if (desire_pursuit_search==_actor_pursuit_never ||
		(desire_pursuit_search==_actor_pursuit_not_when_group && controlling_group_pursuit))
	{
		*allow_pursuit_search = FALSE;
	}

	if (stand_down)
	{
		*allow_pursuit = FALSE;
		*allow_pursuit_search = FALSE;
		*allow_waiting = FALSE;
	}

	if (actor->target.target_prop_index==NONE)
	{
		*allow_target_uncover = FALSE;
		*allow_target_search = FALSE;
	}
	else
	{
		struct prop_datum *prop = prop_get(actor->target.target_prop_index);

		*allow_target_uncover = !prop->tried_to_uncover;
		*allow_target_search = !prop->tried_to_search;

		if (must_charge)
		{
			*allow_target_uncover = FALSE;
		}
		else if (*allow_target_search)
		{
			if (prop->state>=_prop_state_becoming_unacknowledged && prop->state<=_prop_state_acknowledged)
			{
				actor_perception_find_prop_pathfinding_location(actor_index, actor->target.target_prop_index);
			}

			*allow_target_search = actor_nearby_firing_positions(actor_index, &prop->pathfinding_point, prop->pathfinding_surface_index, 1);
		}

		if (!*allow_indefinite_target_uncover && actor->state.combat_status<_actor_combat_status_definite)
		{
			*allow_target_uncover = FALSE;
		}

		if (actor->input.vehicle_driver_type>_actor_vehicle_driver_none)
		{
			*allow_waiting = FALSE;
		}

		if (actor->input.vehicle_driver_type==_actor_vehicle_driver_directional_flying)
		{
			*allow_target_search = FALSE;
			*allow_pursuit_search = FALSE;
		}
	}

	return;
}

boolean actor_action_handle_lost_contact(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
	struct encounter_datum *encounter = actor->meta.encounter_index==NONE ? NULL : encounter_get(actor->meta.encounter_index);
	boolean handled = FALSE;
	boolean stand_down = FALSE;
	boolean postcombat = FALSE;

	if (encounter &&
		encounter->stand_down &&
		actor->state.combat_status<=_actor_combat_status_investigate &&
		actor->state.artificial_combat_status==_actor_combat_status_none &&
		actor->state.suspicion_combat_status==_actor_combat_status_none)
	{
		stand_down = TRUE;
	}

	if (actor->external_orders.postcombat_type>_actor_postcombat_none &&
		actor->state.combat_status<=_actor_combat_status_investigate &&
		actor->state.suspicion_combat_status==_actor_combat_status_none)
	{
		postcombat = TRUE;
	}

	if (actor->state.mode<_actor_mode_combat && actor_action_class(actor_index)==_action_class_noncombat)
	{
		handled = TRUE;
	}

	if (!handled &&
		!actor->input.vehicle_passenger &&
		!postcombat &&
		!stand_down &&
		actor->state.combat_status>=_actor_combat_status_investigate)
	{
		struct prop_datum *prop = actor->target.target_prop_index==NONE ? NULL : prop_get(actor->target.target_prop_index);
		boolean pursue_tenacious = FALSE;
		boolean allow_target_uncover = FALSE;
		boolean allow_indefinite_target_uncover = FALSE;
		boolean allow_target_search = FALSE;
		boolean allow_pursuit = FALSE;
		boolean allow_pursuit_search = FALSE;
		boolean wait_after_pursuit = FALSE;

		if (!prop || !prop->abandoned_search)
		{
			short when_to_search_at_target = actor_type_get_when_to_search_at_target(actor->meta.type);
			short when_to_pursue = actor_type_get_when_to_pursue(actor->meta.type);
			short when_to_search_pursuit = actor_type_get_when_to_search_pursuit(actor->meta.type);
			boolean is_pursuit_controller = actor_type_get_pursuit_controller(actor->meta.type);
			short group_pursuit_restriction = 0;
			boolean controlling_group_pursuit = FALSE;
			boolean controlled_by_group_pursuit = FALSE;

			if (prop)
			{
				allow_indefinite_target_uncover = TRUE;
				allow_target_uncover = TRUE;
			}

			allow_pursuit = TRUE;
			allow_target_search = TRUE;
			allow_pursuit_search = TRUE;

			if (actor->meta.encounter_index!=NONE)
			{
				encounter_modify_pursuit_desires(
					actor->meta.encounter_index,
					actor->meta.squad_index,
					&pursue_tenacious,
					&group_pursuit_restriction,
					&is_pursuit_controller,
					&when_to_search_at_target,
					&when_to_pursue,
					&when_to_search_pursuit);

				if (actor->meta.swarm)
				{
					allow_target_search = TRUE;
					allow_indefinite_target_uncover = TRUE;
					allow_target_uncover = TRUE;
					allow_pursuit_search = TRUE;
					allow_pursuit = TRUE;
				}
				else
				{
					encounter_determine_pursuit_availability(
						actor->meta.encounter_index,
						actor_index,
						group_pursuit_restriction,
						is_pursuit_controller,
						&allow_target_uncover,
						&allow_indefinite_target_uncover,
						&allow_target_search,
						&allow_pursuit,
						&allow_pursuit_search,
						&controlling_group_pursuit,
						&controlled_by_group_pursuit,
						&wait_after_pursuit);
				}
			}

			actor_action_determine_pursuit_options(
				actor_index,
				when_to_search_at_target,
				when_to_pursue,
				when_to_search_pursuit,
				controlling_group_pursuit,
				controlled_by_group_pursuit,
				stand_down,
				actor->emotions.forced_to_charge,
				&allow_target_uncover,
				&allow_indefinite_target_uncover,
				&allow_target_search,
				&allow_pursuit,
				&allow_pursuit_search,
				&wait_after_pursuit);
		}

		if (actor->firing_positions.pursuit_prop_index!=actor->target.target_prop_index)
		{
			actor->firing_positions.pursuit_positions_count = 0;
			actor->firing_positions.pursuit_prop_index = actor->target.target_prop_index;
			actor->firing_positions.pursuit_fired_at_orphan = FALSE;
			actor->firing_positions.pursuit_communicated_lost_contact = FALSE;
		}

		if (!handled)
		{
			if (allow_target_uncover)
			{
				struct action_state_data new_state_data;

				if (action_uncover_setup_target(actor_index, allow_target_search, &new_state_data.uncover))
				{
					actor_action_change(actor_index, _actor_action_uncover, &new_state_data);
					handled = TRUE;
				}
			}

			if (!handled)
			{
				actor_perception_tried_to_uncover(actor_index, actor->target.target_prop_index);
			}
		}

		if (!handled)
		{
			if (allow_target_search)
			{
				struct action_state_data new_state_data;

				if (action_search_setup_target(actor_index, actor->emotions.forced_to_charge, &new_state_data.search))
				{
					actor_action_change(actor_index, _actor_action_search, &new_state_data);
					handled = TRUE;
				}
			}

			if (!handled)
			{
				actor_perception_tried_to_search(actor_index, actor->target.target_prop_index);
			}
		}

		if (!handled &&
			actor->firing_positions.pursuit_fired_at_orphan &&
			!actor->firing_positions.pursuit_communicated_lost_contact)
		{
			ai_communication_event(_ai_communication_lost_contact, actor->meta.unit_index, actor_target_unit_index(actor_index), NONE, NONE, NONE, NULL);
			actor->firing_positions.pursuit_communicated_lost_contact = TRUE;
		}

		if (!handled && allow_pursuit)
		{
			boolean already_uncovered = FALSE;
			short firing_position_index = NONE;

			actor->state.searching = TRUE;
			match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2463, !stand_down);

			if (actor->meta.swarm)
			{
				if (!handled && allow_pursuit_search)
				{
					struct action_state_data new_state_data;

					if (action_search_setup_undirected(actor_index, pursue_tenacious, &new_state_data.search))
					{
						actor_action_change(actor_index, _actor_action_search, &new_state_data);
						handled = TRUE;
					}
				}
			}
			else
			{
				if (actor->state.action==_actor_action_uncover &&
					allow_pursuit_search &&
					actor->state.action_data.uncover.pursuit_location.type==_pursuit_location_position)
				{
					already_uncovered = TRUE;
					firing_position_index = actor->state.action_data.uncover.pursuit_location.firing_position_index;
				}

				if (firing_position_index==NONE)
				{
					short maximum_pursuit_positions = actor->external_orders.pursuit_group_prop_index==NONE ?
						actor_definition->pursuit.uncoordinated_pursuit_positions :
						actor_definition->pursuit.coordinated_pursuit_positions;

					if (pursue_tenacious ||
						actor->firing_positions.pursuit_prop_index!=actor->target.target_prop_index ||
						actor->firing_positions.pursuit_positions_count<maximum_pursuit_positions)
					{
						struct firing_position_evaluation_context evaluation_context;
						struct firing_position new_firing_position;
						long previous_owner;
						boolean path_state_valid;
						struct path_state path_state;

						memset(&evaluation_context, 0, sizeof(evaluation_context));
						evaluation_context.evaluation_mode = _firing_point_evaluation_mode_pursue;
						evaluation_context.evaluation_data.pursue.orphan_prop_index = actor->target.target_prop_index;
						evaluation_context.evaluation_data.pursue.last_perceived_time = !prop ? NONE : prop->last_perceived_time;
						evaluation_context.evaluation_data.pursue.tenacious = pursue_tenacious;
						evaluation_context.find_path_direction_from_target = actor->target.target_prop_index!=NONE;
						evaluation_context.allowed_position_mask = actor_get_firing_position_group(actor_index, evaluation_context.evaluation_mode, 0);
						evaluation_context.maximum_search_range = 20.f;
						firing_position_index = actor_select_firing_position(
							actor_index,
							&evaluation_context,
							&new_firing_position,
							&previous_owner,
							&path_state,
							&path_state_valid);
					}
				}

				if (firing_position_index!=NONE)
				{
					if (!handled && !already_uncovered)
					{
						struct action_state_data new_state_data;

						if (action_uncover_setup_pursuit(actor_index, firing_position_index, &new_state_data.uncover))
						{
							actor_action_change(actor_index, _actor_action_uncover, &new_state_data);
							handled = TRUE;
						}
					}

					if (!handled && allow_pursuit_search)
					{
						struct action_state_data new_state_data;

						if (action_search_setup_pursuit(actor_index, firing_position_index, pursue_tenacious, &new_state_data.search))
						{
							actor_action_change(actor_index, _actor_action_search, &new_state_data);
							handled = TRUE;
						}
					}
				}
			}

			if (handled)
			{
				long history_start_time = !prop ? NONE : prop->last_perceived_time;

				if (actor->meta.encounter_index!=NONE &&
					encounter_mark_examined_pursuit_position(actor->meta.encounter_index, actor_index, firing_position_index, history_start_time))
				{
					if (actor->firing_positions.pursuit_positions_count==0)
					{
						ai_communication_event(_ai_communication_search_start, actor->meta.unit_index, NONE, NONE, NONE, NONE, NULL);
					}

					actor->firing_positions.pursuit_positions_count++;
				}
			}
		}

		if (!handled && actor->firing_positions.pursuit_positions_count>0 && actor->meta.unit_index!=NONE)
		{
			ai_communication_event(_ai_communication_search_abandon, actor->meta.unit_index, NONE, NONE, NONE, NONE, NULL);
		}

		if (!handled && !actor->meta.swarm && wait_after_pursuit)
		{
			struct action_state_data new_state_data;

			match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2595, !stand_down);

			if (action_wait_setup(actor_index, allow_pursuit, &new_state_data.wait))
			{
				actor_action_change(actor_index, _actor_action_wait, &new_state_data);
				handled = TRUE;
			}
		}
	}

	if (!handled && postcombat)
	{
		struct action_state_data new_state_data;

		if (actor->state.action==_actor_action_guard && actor->state.action_data.guard.post_combat)
		{
			handled = TRUE;
		}
		else if (action_guard_setup_postcombat(actor_index, &new_state_data.guard))
		{
			actor_action_change(actor_index, _actor_action_guard, &new_state_data);
			handled = TRUE;
		}
	}

	if (!handled && stand_down)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2629, encounter);
		handled = actor_action_set_default_state(actor_index, NONE);
	}

	if (!handled && actor_action_class(actor_index)!=_action_class_passive)
	{
		struct action_state_data new_state_data;
		boolean success;
		long delay_timer = 0;

		if ((actor->state.action!=_actor_action_search || actor->state.action_data.search.search_failed) &&
			actor->state.action!=_actor_action_wait)
		{
			delay_timer = 3 * TICKS_PER_SECOND;
		}

		actor_perception_abandoned_search(actor_index, actor->target.target_prop_index);
		success = action_guard_setup_find_position(actor_index, delay_timer, &new_state_data.guard);
		match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2658, success);
		actor_action_change(actor_index, _actor_action_guard, &new_state_data);
		handled = TRUE;
	}

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2663, handled || (actor_action_class(actor_index) == _action_class_passive));

	return handled;
}

boolean actor_action_handle_done_fleeing(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->state.action==_actor_action_flee && actor->state.action_data.flee.done_fleeing)
	{
		struct action_state_data new_state_data;
		boolean success = action_guard_setup_from_fleeing(actor_index, &actor->state.action_data.flee, &new_state_data.guard);

		match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2681, success);
		actor_action_change(actor_index, _actor_action_guard, &new_state_data);
		result = TRUE;
	}

	return result;
}

boolean actor_action_can_stop_guarding(
	long actor_index,
	short guard_investigate_threshold,
	short cower_investigate_threshold)
{
	boolean result;
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2697, actor->state.action == _actor_action_guard);

	if (actor->state.action_data.guard.cower)
	{
		result = actor->state.combat_status>=cower_investigate_threshold;
	}
	else if (actor->state.action_data.guard.wait_ticks<=0)
	{
		result = TRUE;
	}
	else
	{
		result = actor->state.combat_status>=guard_investigate_threshold ||
			(actor->external_orders.postcombat_type>_actor_postcombat_none && !actor->state.action_data.guard.post_combat);
	}

	return result;
}

boolean actor_action_can_stop_conversing(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = TRUE;

	if (actor->external_orders.conversation_index!=NONE)
	{
		struct conversation_datum *conversation = conversation_get(actor->external_orders.conversation_index);
		struct ai_conversation *conversation_definition = ai_conversation_definition_get(conversation->conversation_definition_index);

		if (TEST_FLAG(conversation_definition->flags, _ai_conversation_stop_if_damaged_bit) &&
			actor->situation.specific_threats[_actor_threat_damaging_me])
		{
			result = TRUE;
		}
		else if (TEST_FLAG(conversation_definition->flags, _ai_conversation_stop_if_visible_enemy_bit) &&
			actor->target.target_type>=_actor_target_potentially_dangerous_enemy)
		{
			result = TRUE;
		}
		else if (TEST_FLAG(conversation_definition->flags, _ai_conversation_stop_if_alerted_to_enemy_bit) &&
			actor->target.target_type>=_actor_target_definite_orphan)
		{
			result = TRUE;
		}
		else
		{
			result = FALSE;
		}
	}

	return result;
}

boolean actor_action_handle_combat_status(
	long actor_index,
	boolean allow_initiative,
	boolean force_decision)
{
	struct actor_datum *actor = actor_get(actor_index);
	short action_class = actor_action_class(actor_index);
	boolean result = FALSE;

	if (force_decision)
	{
		allow_initiative = TRUE;
	}

	switch (action_class)
	{
		case _action_class_noncombat:
			break;

		case _action_class_passive:
		case _action_class_transitory:
			if (allow_initiative)
			{
				if (actor->state.combat_status>=_actor_combat_status_clear_los)
				{
					result = actor_action_handle_combat_selection(actor_index);
				}
				else if (actor->state.combat_status>=_actor_combat_status_investigate || actor->state.action==_actor_action_alert)
				{
					result = actor_action_handle_lost_contact(actor_index);
				}
				else if (actor->state.combat_status==_actor_combat_status_none &&
					(actor->external_orders.stand_down || actor->external_orders.postcombat_type>_actor_postcombat_none))
				{
					result = actor_action_handle_lost_contact(actor_index);
				}
			}
			break;

		case _action_class_active:
			if (actor->state.combat_status>=_actor_combat_status_certain)
			{
				result = actor_action_handle_combat_selection(actor_index);
			}
			else
			{
				result = actor_action_handle_lost_contact(actor_index);
			}
			break;

		case _action_class_pursuit:
			if (allow_initiative && actor->state.combat_status>=_actor_combat_status_certain)
			{
				result = actor_action_handle_combat_selection(actor_index);
			}
			else if (actor->state.combat_status<_actor_combat_status_investigate)
			{
				result = actor_action_handle_lost_contact(actor_index);
			}
			else if (actor->target.target_prop_index!=NONE)
			{
				struct prop_datum *prop = prop_get(actor->target.target_prop_index);
				boolean lost_contact = FALSE;

				if (actor->target.target_prop_index!=actor->firing_positions.pursuit_prop_index)
				{
					lost_contact = TRUE;
				}
				else if (!prop->tried_to_uncover &&
					(actor->state.action!=_actor_action_uncover ||
					actor->state.action_data.uncover.pursuit_location.type!=_pursuit_location_target))
				{
					lost_contact = TRUE;
				}
				else if (!prop->tried_to_search &&
					(actor->state.action!=_actor_action_uncover ||
					actor->state.action_data.uncover.pursuit_location.type!=_pursuit_location_target) &&
					(actor->state.action!=_actor_action_search ||
					actor->state.action_data.search.pursuit_location.type!=_pursuit_location_target))
				{
					lost_contact = TRUE;
				}

				if (lost_contact)
				{
					result = actor_action_handle_lost_contact(actor_index);
				}
			}
			break;

		default:
			match_halt("c:\\halo\\SOURCE\\ai\\actions.c", 2841);
			break;
	}

	if (!result && force_decision)
	{
		result = actor_action_handle_lost_contact(actor_index);
	}

	return result;
}

boolean actor_action_handle_combat_failure(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->state.action==_actor_action_charge)
	{
		boolean failure = FALSE;

		if (actor->state.action_data.charge.goal==_charge_goal_melee ||
			actor->state.action_data.charge.goal==_charge_goal_melee_leaping)
		{
			if (actor->state.action_data.charge.finished_melee_attack ||
				actor->state.action_data.charge.aborted_melee_attack ||
				actor->state.action_data.charge.unable_to_advance)
			{
				failure = TRUE;
			}
		}
		else if (actor->state.action_data.charge.goal==_charge_goal_vehicle_strafing ||
			actor->state.action_data.charge.goal==_charge_goal_vehicle_ramming)
		{
			failure = actor->state.action_data.charge.unable_to_advance;
		}

		if (failure)
		{
			result = actor_action_handle_combat_selection(actor_index);
		}
	}

	return result;
}

boolean actor_action_handle_exit_pursuit(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;
	boolean exit_pursuit = FALSE;

	switch (actor->state.action)
	{
		case _actor_action_uncover:
			exit_pursuit = actor->state.action_data.uncover.uncover_done;

			if (exit_pursuit && actor->state.action_data.uncover.pursuit_location.type==_pursuit_location_target)
			{
				actor_perception_tried_to_uncover(actor_index, actor->target.target_prop_index);
			}
			break;

		case _actor_action_search:
			exit_pursuit = actor->state.action_data.search.search_done;

			if (exit_pursuit && actor->state.action_data.search.pursuit_location.type==_pursuit_location_target)
			{
				actor_perception_tried_to_search(actor_index, actor->target.target_prop_index);
			}
			break;

		case _actor_action_wait:
			exit_pursuit = actor->state.action_data.wait.wait_done;

			if (exit_pursuit)
			{
				actor_perception_abandoned_search(actor_index, actor->target.target_prop_index);
			}
			break;
	}

	if (exit_pursuit)
	{
		result = actor_action_handle_lost_contact(actor_index);
	}

	return result;
}

void actor_action_change(
	long actor_index,
	long new_action_type,
	struct action_state_data *new_action_data)
{
	struct actor_datum *actor = actor_get(actor_index);

	ai_profile.meters[_ai_meter_action_change].current_count++;

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2947, (new_action_type >= 0) && (new_action_type < NUMBER_OF_ACTOR_ACTIONS));
	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2948, global_action_functions[new_action_type].action == new_action_type);

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 2951, (actor->state.action >= 0) && (actor->state.action < NUMBER_OF_ACTOR_ACTIONS));

	if (global_action_functions[actor->state.action].end)
	{
		global_action_functions[actor->state.action].end(actor_index);
	}

	if (global_action_functions[new_action_type].action_class==_action_class_noncombat)
	{
		if (actor->state.mode>=_actor_mode_combat)
		{
			actor->state.mode = _actor_mode_alert;
		}
	}
	else if (actor->state.mode<_actor_mode_combat)
	{
		actor->state.mode = _actor_mode_combat;
	}

	actor_clear_discarded_firing_positions(actor_index, FALSE);

	if (global_action_functions[new_action_type].data_size>0 && new_action_data)
	{
		memcpy(&actor->state.action_data, new_action_data, global_action_functions[new_action_type].data_size);
	}

	actor->state.action = (short)new_action_type;
	actor->state.action_changed = TRUE;

	if (global_action_functions[actor->state.action].begin)
	{
		global_action_functions[actor->state.action].begin(actor_index);
	}

	return;
}

boolean actor_action_test_grenade(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	boolean result = FALSE;
	long current_time = game_time_get();
	struct actor_debug_info *actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];

	actor_debug_info->grenade_eval_time = current_time;

	if (actor->input.vehicle_index!=NONE)
	{
		actor_debug_info->grenade_decision = _grenade_vehicle;
	}
	else
	{
		boolean encounter_timeout = FALSE;

		if (actor->meta.encounter_index!=NONE)
		{
			short timeout_ticks;
			struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);
			real timeout = actor_variant_definition->grenade_combat.encounter_grenade_timeout;

			timeout = game_difficulty_get_team_value(_game_difficulty_grenade_timer_scale, encounter->team_index) * timeout;

			if (actor->external_orders.playfighting)
			{
				timeout *= 2.f;
			}

			timeout_ticks = (short)(timeout * TICKS_PER_SECOND);
			encounter_timeout = encounter->last_grenade_throw_time!=NONE && encounter->last_grenade_throw_time + timeout_ticks>current_time;

			if (encounter_timeout)
			{
				actor_debug_info->grenade_decision = _grenade_encounter_timeout;
				actor_debug_info->grenade_encounter_timeout_ticks = (short)(timeout_ticks + encounter->last_grenade_throw_time - current_time);
			}
		}

		if (!encounter_timeout)
		{
			real_point3d grenade_target;
			long grenade_target_prop_index;
			long grenade_ignore_object_index;
			short grenade_enemy_count;

			if (actor_combat_find_grenade_target(actor_index, &grenade_target, &grenade_target_prop_index, &grenade_ignore_object_index))
			{
				if (actor_combat_check_collateral_damage(
					actor_index,
					actor_variant_definition->grenade_combat.enemy_radius,
					actor_variant_definition->grenade_combat.collateral_damage_radius,
					&grenade_target,
					&grenade_enemy_count))
				{
					if (grenade_enemy_count>=actor_variant_definition->grenade_combat.minimum_enemy_count)
					{
						if (actor_combat_plan_grenade_trajectory(
							actor_index,
							actor_variant_definition->grenade_combat.trajectory_type,
							&grenade_target,
							grenade_target_prop_index,
							grenade_ignore_object_index))
						{
							result = TRUE;
							actor_debug_info->grenade_decision = _grenade_success;
						}
						else
						{
							actor_debug_info->grenade_decision = _grenade_trajectory_failed;
						}
					}
					else
					{
						actor_debug_info->grenade_decision = _grenade_not_enough_enemies;
						actor_debug_info->grenade_enemy_count = grenade_enemy_count;
						actor_debug_info->grenade_required_enemy_count = actor_variant_definition->grenade_combat.minimum_enemy_count;
					}
				}
				else
				{
					actor_debug_info->grenade_decision = _grenade_collateral_damage;
				}
			}
			else
			{
				actor_debug_info->grenade_decision = _grenade_target_failed;
			}
		}
	}

	return result;
}

boolean actor_action_try_to_throw_grenade(
	long actor_index,
	boolean known_trajectory)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *unit = unit_get(actor->meta.unit_index);
	boolean result = FALSE;

	if (unit_is_busy(actor->meta.unit_index) || unit->object.current_body_damage>0.f)
	{
	}
	else
	{
		if (!known_trajectory && !actor_action_test_grenade(actor_index))
		{
			actor->control.grenade_trying_to_throw = FALSE;
		}

		if (actor->control.grenade_trying_to_throw)
		{
			real_vector2d grenade_target_direction;

			vector_from_points2d(
				(real_point2d *)&actor->input.position.body_position,
				(real_point2d *)&actor->control.grenade_current_target,
				&grenade_target_direction);

			if (normalize2d(&grenade_target_direction)>0.f &&
				!(dot_product2d(&grenade_target_direction, (real_vector2d *)&actor->input.facing_vector)<_cosine30))
			{
				actor->orders.combat.throw_grenade = TRUE;
				actor->control.grenade_trying_to_throw = FALSE;

				if (actor->meta.encounter_index!=NONE)
				{
					struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

					encounter->last_grenade_throw_time = game_time_get();
				}

				result = TRUE;
			}
		}
	}

	return result;
}

boolean actor_action_consider_grenade(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *actor_variant_definition = actor_variant_definition_get(actor->meta.variant_definition_index);
	boolean result = FALSE;

	if (actor->control.grenade_trying_to_throw)
	{
		result = TRUE;
	}
	else if (ai_globals->grenades_enabled &&
		actor_variant_definition->grenade_combat.grenade_type!=NONE &&
		actor_variant_definition->grenade_combat.trajectory_type!=NONE)
	{
		long current_time = game_time_get();

		if (actor->control.grenade_last_check_time==NONE ||
			(real)current_time>=(real)actor->control.grenade_last_check_time + actor_variant_definition->grenade_combat.throw_grenade_delay * TICKS_PER_SECOND)
		{
			real chance = actor_variant_definition->grenade_combat.throw_grenade_chance;

			chance = game_difficulty_get_team_value(_game_difficulty_grenade_chance_scale, actor->meta.team_index) * chance;
			actor->control.grenade_last_check_time = current_time;

			if (real_random()<chance && actor_action_test_grenade(actor_index))
			{
				actor->control.grenade_trying_to_throw = TRUE;
				actor_action_try_to_throw_grenade(actor_index, TRUE);
				result = TRUE;
			}
		}
	}

	return result;
}

boolean actor_action_try_to_seek_cover(
	long actor_index,
	boolean cover_from_last_visible_location,
	boolean allow_occluded_points)
{
	struct action_state_data new_state_data;
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (action_flee_setup(
		actor_index,
		_actor_panic_none,
		actor->target.target_prop_index,
		FALSE,
		cover_from_last_visible_location,
		allow_occluded_points,
		&new_state_data.flee))
	{
		actor_action_change(actor_index, _actor_action_flee, &new_state_data);
		result = TRUE;
	}

	return result;
}

boolean actor_action_try_to_panic(
	long actor_index,
	short panic_type,
	long panic_prop_index,
	boolean force_flee_transition)
{
	struct action_state_data new_state_data;
	boolean result = FALSE;

	if (action_flee_setup(actor_index, panic_type, panic_prop_index, force_flee_transition, FALSE, FALSE, &new_state_data.flee))
	{
		actor_action_change(actor_index, _actor_action_flee, &new_state_data);
		result = TRUE;
	}

	return result;
}

boolean actor_action_try_to_evade(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->input.vehicle_index==NONE &&
		!actor_move_animation_busy(actor_index) &&
		!actor->control.moving &&
		actor->target.target_prop_index!=NONE)
	{
		struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);
		struct unit_datum *unit = unit_get(actor->meta.unit_index);
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
		struct prop_datum *prop = prop_get(actor->target.target_prop_index);

		if (unit_definition->unit.evade_distance>0.f)
		{
			real facing_dot_product;

			if (TEST_FLAG(actor_definition->flags, _actor_definition_flying_bit))
			{
				facing_dot_product = dot_product3d(&prop->actor_to_prop, &actor->input.facing_vector);
			}
			else
			{
				real_vector2d target_direction = *(real_vector2d *)&prop->actor_to_prop;

				facing_dot_product = normalize2d(&target_direction)>0.f ?
					dot_product2d(&target_direction, (real_vector2d *)&actor->input.facing_vector) :
					1.f;
			}

			if (facing_dot_product>0.4f)
			{
				struct path_collision_result collision_result;
				real_vector2d alignment_vector;
				short evade_direction = _actor_evade_random_side;
				boolean evasion_is_ledge;

				alignment_vector = *(real_vector2d *)&prop->actor_to_prop;
				normalize2d(&alignment_vector);

				if (actor_move_try_evasion_direction(
					actor_index,
					&alignment_vector,
					unit_definition->unit.evade_distance,
					&evade_direction,
					0.f,
					&evasion_is_ledge,
					&collision_result))
				{
					short animation_impulse;

					if (evade_direction==_actor_evade_right)
					{
						animation_impulse = _unit_animation_impulse_evade_right;
					}
					else
					{
						match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 3299, evade_direction == _actor_evade_left);
						animation_impulse = _unit_animation_impulse_evade_left;
					}

					if (unit_test_animation_impulse(actor->meta.unit_index, animation_impulse))
					{
						result = actor_move_animation_impulse(actor_index, animation_impulse, &alignment_vector);
					}
				}
			}
		}
	}

	return result;
}

boolean actor_action_try_to_dive(
	long actor_index,
	short escape_direction,
	real dive_distance,
	real_vector2d *alignment_vector,
	real maximum_ledge_height)
{
	struct path_collision_result collision_result;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_debug_info *actor_debug_info = &actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
	boolean result = FALSE;
	boolean dive_off_ledge = FALSE;

	actor_debug_info->dive_decision_time = game_time_get();

	if (actor->input.vehicle_index==NONE &&
		actor_move_try_evasion_direction(
			actor_index,
			alignment_vector,
			dive_distance,
			&escape_direction,
			maximum_ledge_height,
			&dive_off_ledge,
			&collision_result))
	{
		real_vector2d evade_vector;
		real animation_desire[4];
		real_vector2d left_animation_alignment;
		short best_animation_impulse;
		short best_animation_direction;
		real best_animation_desire;
		struct dive_animation_possibility const *possibility;

		switch (escape_direction)
		{
			case _actor_evade_left:
				set_real_vector2d(&evade_vector, -alignment_vector->j, alignment_vector->i);
				break;

			case _actor_evade_right:
				set_real_vector2d(&evade_vector, alignment_vector->j, -alignment_vector->i);
				break;

			case _actor_evade_forward:
				set_real_vector2d(&evade_vector, alignment_vector->i, alignment_vector->j);
				break;

			case _actor_evade_back:
				set_real_vector2d(&evade_vector, alignment_vector->i, alignment_vector->j);
				break;

			default:
				match_halt("c:\\halo\\SOURCE\\ai\\actions.c", 3364);
				break;
		}

		set_real_vector2d(&left_animation_alignment, -actor->input.facing_vector.j, actor->input.facing_vector.i);
		animation_desire[_actor_evade_forward] = dot_product2d((real_vector2d *)&actor->input.facing_vector, &evade_vector);
		animation_desire[_actor_evade_left] = dot_product2d(&left_animation_alignment, &evade_vector);
		animation_desire[_actor_evade_back] = -animation_desire[_actor_evade_forward];
		animation_desire[_actor_evade_right] = -animation_desire[_actor_evade_left];
		best_animation_impulse = NONE;
		best_animation_direction = NONE;
		best_animation_desire = -0.5f;

		for (possibility = global_dive_animation_table; possibility->animation_impulse!=NONE; possibility++)
		{
			match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 3390, (possibility->animation_direction >= 0) && (possibility->animation_direction < 4));

			if (animation_desire[possibility->animation_direction] + possibility->animation_desire>best_animation_desire &&
				unit_test_animation_impulse(actor->meta.unit_index, possibility->animation_impulse))
			{
				best_animation_impulse = possibility->animation_impulse;
				best_animation_direction = possibility->animation_direction;
				best_animation_desire = animation_desire[possibility->animation_direction] + possibility->animation_desire;
			}
		}

		if (best_animation_impulse==NONE)
		{
			actor_debug_info->dive_decision = _dive_no_animation;
		}
		else
		{
			real_vector2d animation_alignment_vector;

			switch (best_animation_direction)
			{
				case _actor_evade_left:
					set_real_vector2d(&animation_alignment_vector, evade_vector.j, -evade_vector.i);
					break;

				case _actor_evade_right:
					set_real_vector2d(&animation_alignment_vector, -evade_vector.j, evade_vector.i);
					break;

				case _actor_evade_forward:
					set_real_vector2d(&animation_alignment_vector, evade_vector.i, evade_vector.j);
					break;

				case _actor_evade_back:
					set_real_vector2d(&animation_alignment_vector, evade_vector.i, evade_vector.j);
					break;

				default:
					match_halt("c:\\halo\\SOURCE\\ai\\actions.c", 3433);
					break;
			}

			result = actor_move_animation_impulse(actor_index, best_animation_impulse, &animation_alignment_vector);

			if (result)
			{
				ai_communication_event(_ai_communication_dive, actor->meta.unit_index, NONE, NONE, NONE, NONE, NULL);
				actor_debug_info->dive_decision = _dive_success;
			}
			else
			{
				actor_debug_info->dive_decision = _dive_animation_failure;
			}
		}
	}
	else
	{
		actor_debug_info->dive_decision = _dive_cannot_move;
	}

	return result;
}

boolean actor_action_try_to_enter_vehicle(
	long actor_index,
	long vehicle_index,
	char const *seat_substring_name,
	short seat_desire_type,
	short precomputed_seat_count,
	short *precomputed_seat_indices)
{
	short *seat_indices;
	short seat_indices_new[16];
	short seat_index_index;
	struct action_state_data new_state_data;
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;
	short seat_count = 0;

	if (!precomputed_seat_indices)
	{
		seat_indices = seat_indices_new;
		seat_count = vehicle_scripting_find_available_seats(
			vehicle_index,
			seat_substring_name,
			seat_desire_type,
			seat_indices_new,
			NUMBEROF(seat_indices_new));
	}
	else
	{
		seat_count = precomputed_seat_count;
		seat_indices = precomputed_seat_indices;
	}

	for (seat_index_index = 0; seat_index_index<seat_count; seat_index_index++)
	{
		short seat_index = seat_indices[seat_index_index];

		if (seat_index!=NONE &&
			unit_has_animation_to_enter_seat(actor->meta.unit_index, vehicle_index, seat_index) &&
			action_vehicle_setup_specific(actor_index, vehicle_index, seat_index, &new_state_data.vehicle))
		{
			actor_action_change(actor_index, _actor_action_vehicle, &new_state_data);
			seat_indices[seat_index_index] = NONE;
			result = TRUE;
			break;
		}
	}

	return result;
}

struct pursuit_location *actor_get_pursuit_location(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct pursuit_location *pursuit_location = NULL;

	if (actor->state.action==_actor_action_search)
	{
		pursuit_location = &actor->state.action_data.search.pursuit_location;
	}
	else if (actor->state.action==_actor_action_uncover)
	{
		pursuit_location = &actor->state.action_data.uncover.pursuit_location;
	}

	return pursuit_location;
}

boolean actors_searching_same_position(
	long actor_index,
	long friend_actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_datum *friend_actor = actor_get(friend_actor_index);
	struct pursuit_location *pursuit_location = actor_get_pursuit_location(actor_index);
	struct pursuit_location *friend_pursuit_location = actor_get_pursuit_location(friend_actor_index);
	boolean result = FALSE;

	if (pursuit_location && friend_pursuit_location)
	{
		if (pursuit_location->type==_pursuit_location_target && friend_pursuit_location->type==_pursuit_location_target)
		{
			struct prop_datum *prop = prop_try_and_get(actor->target.target_prop_index);
			struct prop_datum *friend_prop = prop_try_and_get(friend_actor->target.target_prop_index);

			if (prop && friend_prop)
			{
				result = distance_squared3d(&prop->body_position, &friend_prop->body_position)<0.7f * 0.7f;
			}
		}
		else if (pursuit_location->type==_pursuit_location_position && friend_pursuit_location->type==_pursuit_location_position)
		{
			result = pursuit_location->firing_position_index==friend_pursuit_location->firing_position_index;
		}
		else if (pursuit_location->type==_pursuit_location_undirected && friend_pursuit_location->type==_pursuit_location_undirected)
		{
			result = TRUE;
		}
	}

	return result;
}

static boolean actor_pursuit_consider_nearby_actor(
	long actor_index,
	boolean pursuit_coordinator,
	long friend_actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_datum *friend_actor = actor_try_and_get(friend_actor_index);
	boolean result = FALSE;

	if (friend_actor &&
		friend_actor->state.combat_status>=_actor_combat_status_investigate &&
		friend_actor->state.combat_status<_actor_combat_status_certain &&
		(friend_actor->state.action==_actor_action_search ||
		friend_actor->state.action==_actor_action_uncover ||
		(!pursuit_coordinator && friend_actor->state.action==_actor_action_wait) ||
		(friend_actor->state.action==_actor_action_guard &&
		!friend_actor->state.action_data.guard.cower &&
		friend_actor->state.action_data.guard.wait_ticks>0)) &&
		actor_type_get_pursuit_controller(friend_actor->meta.type)!=pursuit_coordinator)
	{
		result = TRUE;
	}

	return result;
}

long actor_pursuit_find_nearby_actors(
	long actor_index,
	boolean pursuit_coordinator)
{
	struct actor_datum *actor = actor_get(actor_index);
	short required_actor_count = pursuit_coordinator ? 2 : 1;
	long actor_count = 0;
	long best_prop_index = NONE;
	real best_distance = REAL_MAX;

	{
		struct prop_iterator iterator;
		struct prop_datum *prop;

		prop_iterator_new(&iterator, actor_index);

		while (prop = prop_iterator_next(&iterator))
		{
			if (!prop->enemy &&
				!prop->dead &&
				prop->actor_index!=NONE &&
				(!pursuit_coordinator ||
				(prop->state>=_prop_state_becoming_unacknowledged && prop->state<=_prop_state_acknowledged)))
			{
				if (actor_pursuit_consider_nearby_actor(actor_index, pursuit_coordinator, prop->actor_index))
				{
					actor_count++;

					if (prop->distance<best_distance)
					{
						best_prop_index = iterator.index;
						best_distance = prop->distance;
					}
				}
			}
		}
	}

	if (actor_count<required_actor_count && actor->meta.encounter_index!=NONE)
	{
		struct encounter_actor_iterator iterator;
		struct actor_datum *friend_actor;

		encounter_actor_iterator_new(&iterator, actor->meta.encounter_index);

		while (friend_actor = encounter_actor_iterator_next(&iterator))
		{
			if (friend_actor->meta.unit_index!=NONE &&
				actor_pursuit_consider_nearby_actor(actor_index, pursuit_coordinator, iterator.index))
			{
				long prop_index = prop_get_active_by_unit_index(actor_index, friend_actor->meta.unit_index);

				if (prop_index==NONE)
				{
					prop_index = prop_get_base_by_unit_index(actor_index, friend_actor->meta.unit_index, TRUE, FALSE);
				}

				if (prop_index!=NONE)
				{
					real distance = distance3d(&actor->input.position.body_position, &friend_actor->input.position.body_position);

					actor_count++;

					if (distance<best_distance)
					{
						best_prop_index = prop_index;
						best_distance = distance;
					}

					if (actor_count>=required_actor_count)
					{
						break;
					}
				}
			}
		}
	}

	actor->external_orders.pursuit_group_prop_index = best_prop_index;

	return actor_count;
}

char const *actor_action_name(
	short actor_action_index)
{
	char const *name = "unknown";

	if (actor_action_index>=0 && actor_action_index<NUMBER_OF_ACTOR_ACTIONS)
	{
		name = global_action_functions[actor_action_index].name;
	}

	return name;
}

char const *actor_mode_name(
	short actor_mode_index)
{
	char const *name = "unknown";

	if (actor_mode_index>=0 && actor_mode_index<NUMBER_OF_ACTOR_MODES)
	{
		name = global_actor_mode_names[actor_mode_index];
	}

	return name;
}

real_argb_color *actor_action_debug_color(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	global_temporary_render_color = *global_real_argb_black;

	if (actor->state.action>=0 && actor->state.action<NUMBER_OF_ACTOR_ACTIONS)
	{
		global_temporary_render_color = **global_action_functions[actor->state.action].color;

		if (global_action_functions[actor->state.action].modify_color)
		{
			global_action_functions[actor->state.action].modify_color(actor_index, &global_temporary_render_color);
		}
	}

	return &global_temporary_render_color;
}

short actor_action_class(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actions.c", 3736, (actor->state.action >= 0) && (actor->state.action < NUMBER_OF_ACTOR_ACTIONS));

	return global_action_functions[actor->state.action].action_class;
}

short actor_action_get_default_state(
	short initial_state)
{
	short default_state = _actor_default_state_none;

	if (initial_state>=0 && initial_state<NUMBER_OF_ACTOR_DEFAULT_STATES)
	{
		default_state = global_default_states[initial_state];
	}

	return default_state;
}
