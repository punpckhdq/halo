/*
ACTOR_TYPE_MOUNTED_WEAPON.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "ai_debug.h"
#include "encounters.h"
#include "props.h"
#include "actor_types.h"
#include "units.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

void mounted_weapon_decide_action(long actor_index);

/* ---------- globals */

struct actor_type_definition actor_type_mounted_weapon =
{
	"mounted_weapon",
	_race_none,
	2,
	2,
	2,
	FALSE,
	FALSE,
	NULL,
	mounted_weapon_decide_action,
	NULL,
	NULL
};

/* ---------- public code */

void mounted_weapon_decide_action(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor_action_handle_initial_action(actor_index);
	actor_action_handle_pending_command_list(actor_index);
	if (!actor_action_deny_transition(actor_index))
	{
		actor_action_handle_combat_transition(actor_index);
	}

	switch (actor->state.action)
	{
	case _actor_action_fight:
	case _actor_action_flee:
	case _actor_action_guard:
	case _actor_action_charge:
		if (!actor_action_handle_combat_status(actor_index, TRUE, FALSE))
		{
			actor_action_handle_combat_failure(actor_index);
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
