/*
CHEATS.H

header included in hcex build.
*/

#ifndef __CHEATS_H
#define __CHEATS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct cheat_globals
{
	boolean deathless_player;
	boolean jetpack;
	boolean infinite_ammo;
	boolean bump_possession;
	boolean super_jump;
	boolean reflexive_damage_effects;
	boolean medusa;
	boolean omnipotent;
	boolean controller_enabled;
	boolean bottomless_clip;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* comm */
struct cheat_globals cheat;

/* ---------- public code */

#endif // __CHEATS_H
