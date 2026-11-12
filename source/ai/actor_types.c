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

static struct actor_type_definition *actor_type_definition_get(short actor_type);

/* ---------- globals */

static struct actor_type_definition *actor_type_definitions[NUMBER_OF_ACTOR_TYPES] =
{
	&actor_type_elite,
	&actor_type_jackal,
	&actor_type_grunt,
	&actor_type_hunter,
	&actor_type_engineer,
	&actor_type_elite,
	&actor_type_marine,
	&actor_type_marine,
	&actor_type_crew,
	&actor_type_flood,
	&actor_type_infection,
	&actor_type_carrier,
	&actor_type_sentinel,
	&actor_type_sentinel,
	&actor_type_grunt,
	&actor_type_mounted_weapon,
};

const char *global_actor_type_names[NUMBER_OF_ACTOR_TYPES] =
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

// this function lives in actor_type_definitions.h in the original (see the assert file name)
static struct actor_type_definition *actor_type_definition_get(
	short actor_type)
{
	match_assert("c:\\halo\\source\\ai\\actor_type_definitions.h", 46, actor_type>=0 && actor_type<NUMBER_OF_ACTOR_TYPES);
	match_assert("c:\\halo\\source\\ai\\actor_type_definitions.h", 47, actor_type_definitions[actor_type]);
	match_assert("c:\\halo\\source\\ai\\actor_type_definitions.h", 50, actor_type_definitions[actor_type]->name);
	match_assert("c:\\halo\\source\\ai\\actor_type_definitions.h", 51, actor_type_definitions[actor_type]->decide_action);
	match_assert("c:\\halo\\source\\ai\\actor_type_definitions.h", 53, actor_type_definitions[actor_type]->when_to_search_at_target < NUMBER_OF_ACTOR_PURSUIT_SETTINGS);
	match_assert("c:\\halo\\source\\ai\\actor_type_definitions.h", 54, actor_type_definitions[actor_type]->when_to_pursue < NUMBER_OF_ACTOR_PURSUIT_SETTINGS);
	match_assert("c:\\halo\\source\\ai\\actor_type_definitions.h", 55, actor_type_definitions[actor_type]->when_to_search_pursuit < NUMBER_OF_ACTOR_PURSUIT_SETTINGS);

	return actor_type_definitions[actor_type];
}

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

const char *actor_type_get_name(
	short actor_type)
{
	return actor_type_definition_get(actor_type)->name;
}

short actor_type_get_race(
	short actor_type)
{
	return actor_type_definition_get(actor_type)->race;
}

short actor_type_get_when_to_search_at_target(
	short actor_type)
{
	return actor_type_definition_get(actor_type)->when_to_search_at_target;
}

short actor_type_get_when_to_pursue(
	short actor_type)
{
	return actor_type_definition_get(actor_type)->when_to_pursue;
}

short actor_type_get_when_to_search_pursuit(
	short actor_type)
{
	return actor_type_definition_get(actor_type)->when_to_search_pursuit;
}

boolean actor_type_get_pursuit_controller(
	short actor_type)
{
	return actor_type_definition_get(actor_type)->pursuit_controller;
}

boolean actor_type_get_swarm(
	short actor_type)
{
	return actor_type_definition_get(actor_type)->swarm;
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
