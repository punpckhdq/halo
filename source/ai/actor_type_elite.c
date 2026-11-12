/*
ACTOR_TYPE_ELITE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "props.h"
#include "actor_types.h"
#include "actor_type_definitions.h"
#include "units.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

void elite_decide_action(long actor_index);

/* ---------- globals */

struct actor_type_definition actor_type_elite =
{
	"elite",
	_race_covenant,
	0,
	1,
	1,
	TRUE,
	FALSE,
	NULL,
	elite_decide_action,
	NULL,
	NULL
};

/* ---------- public code */

void elite_decide_action(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	actor_action_handle_initial_action(actor_index);
	actor_action_handle_pending_command_list(actor_index);
	actor_action_handle_surprise(actor_index, _actor_surprise_unprepared_grenade);
	if (!actor_action_deny_transition(actor_index))
	{
		actor_action_handle_berserking_from_attacking_mode(actor_index);
		actor_action_handle_berserking_from_damage(actor_index);
		actor_action_handle_berserking_from_proximity(actor_index);
		actor_action_handle_berserking_from_attached_projectiles(actor_index);
		actor_action_handle_berserk_transition(actor_index, _actor_berserk_attacking);
		if (!actor->emotions.berserk)
		{
			actor_action_handle_panic_from_attached_projectiles(actor_index);
			actor_action_handle_panic_from_attached_melee_attackers(actor_index);
			actor_action_handle_panic_transition(actor_index, _actor_panic_grenade_attached_to_us, FALSE, _actor_panic_melee_attached_to_us);
		}
		actor_action_handle_combat_transition(actor_index);
		actor_action_handle_active_cover_seeking(actor_index, FALSE, FALSE);
		actor_action_handle_vehicle_entry(actor_index);
		actor_action_handle_vehicle_exit(actor_index);
		actor_action_handle_grenade_throwing(actor_index);
		actor_action_handle_danger_avoidance(actor_index);
	}

	switch (actor->state.action)
	{
		case _actor_action_fight:
		case _actor_action_charge:
			if (!actor_action_handle_combat_status(actor_index, TRUE, FALSE))
			{
				if (!actor_action_handle_combat_failure(actor_index))
				{
					actor_action_handle_evasion(actor_index);
				}
			}
			break;
		case _actor_action_guard:
			if (actor->state.action_data.guard.cower && !actor->state.action_data.guard.cower_panicked && !actor->state.action_data.guard.cower_from_retreat)
			{
				real shield_fraction = actor->state.combat_status >= _actor_combat_status_certain ?
					actor_definition->defensive.shield_fraction_emerge_attack :
					actor_definition->defensive.shield_fraction_emerge_pursue;

				if (shield_fraction > actor->input.shield_vitality)
				{
					actor->state.action_data.guard.cower = TRUE;
					actor->state.action_data.guard.cower_ticks = 30;
				}
				else
				{
					actor->state.action_data.guard.cower = FALSE;
					actor->state.action_data.guard.cower_ticks = 0;
				}
			}
			actor_action_handle_combat_status(actor_index, actor_action_can_stop_guarding(actor_index, _actor_combat_status_definite, _actor_combat_status_dangerous), FALSE);
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
		case _actor_action_uncover:
		case _actor_action_search:
		case _actor_action_wait:
			if (!actor_action_handle_combat_status(actor_index, TRUE, FALSE))
			{
				actor_action_handle_exit_pursuit(actor_index);
			}
			break;
		case _actor_action_vehicle:
			if (actor->state.action_data.vehicle.vehicle_entry_done || actor->state.action_data.vehicle.vehicle_entry_failed)
			{
				actor_action_handle_combat_status(actor_index, TRUE, TRUE);
			}
			break;
		case _actor_action_obey:
			actor_action_handle_combat_status(actor_index, actor->state.action_data.obey.initiative, actor->state.action_data.obey.finished);
			break;
		case _actor_action_converse:
			actor_action_handle_combat_status(actor_index, actor_action_can_stop_conversing(actor_index),
				actor->state.action_data.converse.failed || actor->external_orders.conversation_index == NONE);
			break;
		case _actor_action_avoid:
			if (actor->danger_zone.danger_type == _actor_danger_zone_none)
			{
				actor_action_handle_combat_status(actor_index, TRUE, TRUE);
			}
			break;
	}

	return;
}

/* ---------- private code */
