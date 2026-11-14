/*
ACTOR_TYPES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "actor_types.h"
#include "actors.h"
#include "actor_type_definitions.h"
#include "units.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

char const *global_actor_type_names[NUMBER_OF_ACTOR_TYPES] =
{
	"elite",
	"jackal",
	"grunt",
	"hunter",
	"engineer",
	"assassin",
	"player",
	"marine",
	"crew",
	"combat_form",
	"infection_form",
	"carrier_form",
	"monitor",
	"sentinel",
	"none",
	"mounted_weapon",
};

/* ---------- public code */

void actor_types_initialize(
	void)
{
	short actor_type;

	for (actor_type = 0; actor_type < NUMBER_OF_ACTOR_TYPES; actor_type++)
	{
		actor_type_definition_get(actor_type);
	}

	return;
}

char const *actor_type_get_name(
	short actor_type)
{
	struct actor_type_definition *actor_type_definition = actor_type_definition_get(actor_type);

	return actor_type_definition->name;
}

short actor_type_get_race(
	short actor_type)
{
	struct actor_type_definition *actor_type_definition = actor_type_definition_get(actor_type);

	return actor_type_definition->race;
}

short actor_type_get_when_to_search_at_target(
	short actor_type)
{
	struct actor_type_definition *actor_type_definition = actor_type_definition_get(actor_type);

	return actor_type_definition->when_to_search_at_target;
}

short actor_type_get_when_to_pursue(
	short actor_type)
{
	struct actor_type_definition *actor_type_definition = actor_type_definition_get(actor_type);

	return actor_type_definition->when_to_pursue;
}

short actor_type_get_when_to_search_pursuit(
	short actor_type)
{
	struct actor_type_definition *actor_type_definition = actor_type_definition_get(actor_type);

	return actor_type_definition->when_to_search_pursuit;
}

boolean actor_type_get_pursuit_controller(
	short actor_type)
{
	struct actor_type_definition *actor_type_definition = actor_type_definition_get(actor_type);

	return actor_type_definition->pursuit_controller;
}

boolean actor_type_get_swarm(
	short actor_type)
{
	struct actor_type_definition *actor_type_definition = actor_type_definition_get(actor_type);

	return actor_type_definition->swarm;
}

void actor_type_initialize(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_type_definition *actor_type_definition = actor_type_definition_get(actor->meta.type);

	if (actor_type_definition->initialize)
	{
		actor_type_definition->initialize(actor_index);
	}

	return;
}

void actor_type_decide_action(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_type_definition *actor_type_definition = actor_type_definition_get(actor->meta.type);

	match_assert("c:\\halo\\SOURCE\\ai\\actor_types.c", 129, actor_type_definition->decide_action);
	actor_type_definition->decide_action(actor_index);

	return;
}

void actor_type_swarm_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_type_definition *actor_type_definition = actor_type_definition_get(actor->meta.type);

	match_assert("c:\\halo\\SOURCE\\ai\\actor_types.c", 141, actor_type_definition->swarm);
	match_assert("c:\\halo\\SOURCE\\ai\\actor_types.c", 142, actor_type_definition->swarm_control);
	actor_type_definition->swarm_control(actor_index);

	return;
}

void actor_type_swarm_aim_jump(
	long actor_index,
	long unit_index,
	real jump_magnitude,
	real_vector3d *jump_velocity)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_type_definition *actor_type_definition = actor_type_definition_get(actor->meta.type);

	match_assert("c:\\halo\\SOURCE\\ai\\actor_types.c", 157, actor_type_definition->swarm);
	if (actor_type_definition->swarm_aim_jump)
	{
		actor_type_definition->swarm_aim_jump(actor_index, unit_index, jump_magnitude, jump_velocity);
	}

	return;
}

/* ---------- private code */
