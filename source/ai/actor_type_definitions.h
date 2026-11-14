/*
ACTOR_TYPE_DEFINITIONS.H

file has inline function assertions.
*/

#ifndef __ACTOR_TYPE_DEFINITIONS_H
#define __ACTOR_TYPE_DEFINITIONS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

extern struct actor_type_definition actor_type_elite;
extern struct actor_type_definition actor_type_jackal;
extern struct actor_type_definition actor_type_grunt;
extern struct actor_type_definition actor_type_hunter;
extern struct actor_type_definition actor_type_engineer;
extern struct actor_type_definition actor_type_marine;
extern struct actor_type_definition actor_type_crew;
extern struct actor_type_definition actor_type_flood;
extern struct actor_type_definition actor_type_infection;
extern struct actor_type_definition actor_type_carrier;
extern struct actor_type_definition actor_type_sentinel;
extern struct actor_type_definition actor_type_mounted_weapon;

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

/* ---------- public code */

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

#endif // __ACTOR_TYPE_DEFINITIONS_H
