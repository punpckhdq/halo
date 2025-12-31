/*
GAME_STATE.H

header included in hcex build.
*/

#ifndef __GAME_STATE_H
#define __GAME_STATE_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/GAME_STATE_XBOX.C */

void game_state_initialize(void);

struct data_array *game_state_data_new(const char *name, short maximum_count, short size);

/* ---------- globals */

/* ---------- public code */

#endif // __GAME_STATE_H
