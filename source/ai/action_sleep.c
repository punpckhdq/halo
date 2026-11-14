/*
ACTION_SLEEP.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actors.h"
#include "encounters.h"
#include "props.h"
#include "units.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void action_sleep_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->orders.look.idle_look_type = _idle_look_none;

	return;
}

/* ---------- private code */
