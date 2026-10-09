/*
ACTOR_TYPES.H

header included in hcex build.
*/

#ifndef __ACTOR_TYPES_H
#define __ACTOR_TYPES_H
#pragma once

/* ---------- constants */

enum
{
	_actor_elite = 0,
	_actor_jackal,
	_actor_grunt,
	_actor_hunter,
	_actor_engineer,
	_actor_assassin,
	_actor_player,
	_actor_marine,
	_actor_crew,
	_actor_combat_form,
	_actor_infection_form,
	_actor_carrier_form,
	_actor_monitor,
	_actor_sentinel,
	_actor_none,
	_actor_mounted_weapon,
	NUMBER_OF_ACTOR_TYPES,
};

enum
{
	_race_player_bit = 0,
	_race_human_bit,
	_race_covenant_bit,
	_race_floodcombat_bit,
	_race_floodcarrier_bit,
	_race_floodinfection_bit,
	_race_sentinel_bit,
	NUMBER_OF_ACTOR_RACE_FLAGS,

	_race_none = 0,
	_race_player = FLAG(_race_player_bit),
	_race_human = FLAG(_race_human_bit),
	_race_covenant = FLAG(_race_covenant_bit),
	_race_floodcombat = FLAG(_race_floodcombat_bit),
	_race_floodcarrier = FLAG(_race_floodcarrier_bit),
	_race_floodinfection = FLAG(_race_floodinfection_bit),
	_race_flood = _race_floodcombat | _race_floodcarrier | _race_floodinfection,
	_race_sentinel = FLAG(_race_sentinel_bit),
	_race_all = MASK(NUMBER_OF_ACTOR_RACE_FLAGS),
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/ACTOR_TYPES.C */

char const *actor_type_get_name(short actor_type);
short actor_type_get_race(short actor_type);

/* ---------- globals */

/* ---------- public code */

#endif // __ACTOR_TYPES_H
