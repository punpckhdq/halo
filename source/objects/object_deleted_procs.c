/*
OBJECT_DELETED_PROCS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "object_types.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "objects.h"
#include "ai.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

object_deleted_proc object_deleted_procs[] =
{
	objects_fix_for_deleted_object,
	ai_handle_deleted_object,
	players_handle_deleted_object
};

/* ---------- public code */

void object_deleted_procs_call(
	long deleted_object_index)
{
	short proc_index;

	for (proc_index = 0; proc_index < NUMBEROF(object_deleted_procs); proc_index++)
	{
		object_deleted_procs[proc_index](deleted_object_index);
	}

	return;
}

/* ---------- private code */
