/*
ACTOR_TYPE_DEFINITIONS.H

file has inline function assertions.
*/

#ifndef __ACTOR_TYPE_DEFINITIONS_H
#define __ACTOR_TYPE_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	_actor_pursuit_always = 0,
	_actor_pursuit_not_when_group,
	_actor_pursuit_never,
	NUMBER_OF_ACTOR_PURSUIT_SETTINGS,
};

/* ---------- macros */

/* ---------- structures */

struct actor_type_definition
{
	const char *name;
	short race;
	short when_to_search_at_target;
	short when_to_pursue;
	short when_to_search_pursuit;
	boolean pursuit_controller;
	boolean swarm;
	void (*initialize)(long actor_index);
	void (*decide_action)(long actor_index);
	void (*swarm_control)(long actor_index);
	void (*swarm_aim_jump)(long actor_index, long unit_index, real jump_magnitude, real_vector3d *jump_velocity);
};

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

/* ---------- public code */

#endif // __ACTOR_TYPE_DEFINITIONS_H
