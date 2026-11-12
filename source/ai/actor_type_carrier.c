/*
ACTOR_TYPE_CARRIER.C
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

void carrier_decide_action(long actor_index);

/* ---------- globals */

struct actor_type_definition actor_type_carrier =
{
	"flood carrier",
	_race_floodcarrier,
	0,
	2,
	2,
	TRUE,
	FALSE,
	NULL,
	carrier_decide_action,
	NULL,
	NULL
};

/* ---------- public code */

void carrier_decide_action(
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
		actor_action_handle_berserk_transition(actor_index, _actor_berserk_attacking);
		actor_action_handle_combat_transition(actor_index);
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
		case _actor_action_obey:
			actor_action_handle_combat_status(actor_index, actor->state.action_data.obey.initiative, actor->state.action_data.obey.finished);
			break;
	}

	return;
}

/* ---------- private code */
