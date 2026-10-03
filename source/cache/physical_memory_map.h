/*
PHYSICAL_MEMORY_MAP.H

header included in hcex build.
*/

#ifndef __PHYSICAL_MEMORY_MAP_H
#define __PHYSICAL_MEMORY_MAP_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/PHYSICAL_MEMORY_MAP.C */

void physical_memory_allocate(void);
void physical_memory_verify(void);
void *physical_memory_get_sound_cache_base_address(void);

/* ---------- globals */

/* ---------- public code */

#endif // __PHYSICAL_MEMORY_MAP_H
