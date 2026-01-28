/*
GAME_SOUND.H

header included in hcex build.
*/

#ifndef __GAME_SOUND_H
#define __GAME_SOUND_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/GAME_SOUND.C */

void game_sound_clear(void);
void game_sound_restore(void);
long game_looping_sound_new(long object_index, long definition_index, char const *marker_name, short function_index);
void game_looping_sound_delete(long looping_sound_index);

/* ---------- globals */

/* ---------- public code */

#endif // __GAME_SOUND_H
