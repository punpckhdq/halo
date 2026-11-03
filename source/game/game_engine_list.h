/*
GAME_ENGINE_LIST.H

header included in hcex build.
*/

#ifndef __GAME_ENGINE_LIST_H
#define __GAME_ENGINE_LIST_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

extern struct game_engine *game_engines[];
extern struct game_engine ctf_engine;
extern struct game_engine slayer_engine;
extern struct game_engine oddball_engine;
extern struct game_engine king_engine;
extern struct game_engine race_engine;
extern struct game_engine stub_engine;

/* ---------- public code */

#endif // __GAME_ENGINE_LIST_H
