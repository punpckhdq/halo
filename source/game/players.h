/*
PLAYERS.H

header included in hcex build.
*/

#ifndef __PLAYERS_H
#define __PLAYERS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/PLAYERS.C */

unsigned long const *players_get_combined_pvs_local(void);
unsigned long const *players_get_combined_pvs(void);

void player_control_fix_for_loaded_game_state(void);

/* ---------- prototypes/PLAYER_QUEUES_NEW.C */

void update_queues_reset_and_fill_with_lies(void);

/* ---------- globals */

/* ---------- public code */

#endif // __PLAYERS_H
