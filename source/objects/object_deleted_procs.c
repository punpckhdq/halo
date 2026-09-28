/*
OBJECT_DELETED_PROCS.C

symbols in this file:
00128700 0030:
	_object_deleted_procs_call (0000)
0030B378 000c:
	_object_deleted_procs (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "game.h"
#include "scenario_definitions.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "object_definitions.h"
#include "rasterizer_geometry.h"
#include "model_animation_definitions.h"
#include "model_definitions.h"
#include "damage_resistances.h"
#include "shader_definitions.h"
#include "ai.h"
#include "ai_constants.h"
#include "unit_definitions.h"
#include "bungie_net/common/message_header.h"
#include "network_game_globals.h"
#include "ai_communication.h"
#include "unicode.h"
#include "index_resolution.h"
#include "game_engine.h"
#include "network_game_manager.h"
#include "players.h"
#include "object_types.h"
#include "objects.h"
#include "damage.h"
#include "models.h"
#include "units.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

object_deleted_proc object_deleted_procs[3];

/* ---------- public code */

void object_deleted_procs_call(
	long deleted_object_index)
{
	long i =0;

	do
	{
		(object_deleted_procs[i])(deleted_object_index);
		i++;
	}
	while (i<NUMBEROF(object_deleted_procs));

	return;
}

/* ---------- private code */
