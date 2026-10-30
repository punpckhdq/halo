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

void cheats_initialize(void);
void cheats_dispose(void);
void cheats_initialize_for_new_map(void);
void cheats_dispose_from_old_map(void);
void cheats_update(void);
void cheats_load(void);

/* ---------- globals */

extern struct cheat_globals cheat;

/* ---------- public code */

#endif // __CHEATS_H
