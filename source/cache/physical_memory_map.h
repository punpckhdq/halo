/*
PHYSICAL_MEMORY_MAP.H

header included in hcex build.
*/

#ifndef __PHYSICAL_MEMORY_MAP_H
#define __PHYSICAL_MEMORY_MAP_H
#pragma once

/* ---------- constants */

#define GAME_STATE_BASE_ADDRESS 0x80061000
#define TAG_CACHE_BASE_ADDRESS 0x803a6000

enum
{
	TAG_CACHE_SIZE = 0x1600000, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/PHYSICAL_MEMORY_MAP.C */

void physical_memory_allocate(void);
void physical_memory_verify(void);
void physical_memory_free(void);
void *physical_memory_get_game_state_base_address(void);
void *physical_memory_get_tag_cache_base_address(void);
void *physical_memory_get_texture_cache_base_address(void);
void *physical_memory_get_sound_cache_base_address(void);

/* ---------- globals */

/* ---------- public code */

#endif // __PHYSICAL_MEMORY_MAP_H
