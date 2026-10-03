/*
SOUND_MANAGER.H

header included in hcex build.
*/

#ifndef __SOUND_MANAGER_H
#define __SOUND_MANAGER_H
#pragma once

/* ---------- headers */

#include "sound_environment_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/SOUND_MANAGER.C */

void sound_render(void);

void sound_dispose(void);
void sound_reconnect_to_structure_bsp(void);
void sound_initialize(void);
void sound_stop_impulse(long sound_index);
void sound_stop_all(void);

/* ---------- globals */

/* ---------- public code */

#endif // __SOUND_MANAGER_H
