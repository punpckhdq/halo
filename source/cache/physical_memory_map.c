/*
PHYSICAL_MEMORY_MAP.C
*/

/* ---------- headers */

#include "cseries.h"
#include "physical_memory_map.h"
#include "sound_cache.h"

/* ---------- constants */

#define PHYSICAL_MEMORY_BASE_ADDRESS 0x80000000 /* fake name */

enum
{
	GAME_STATE_SIZE = 0x345000, /* fake name */
	GAME_STATE_CPU_SIZE = 0x305000,
	TEXTURE_CACHE_SIZE = 0x1600000, /* fake name */
	SOUND_CACHE_SIZE = 0x400000, /* fake name */

	PHYSICAL_MEMORY_PAGE_SIZE = 0x1000, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct physical_memory_map_globals
{
	void *game_state_base_address;
	void *tag_cache_base_address;
	void *texture_cache_base_address;
	void *sound_cache_base_address;
};

/* ---------- prototypes */

/* ---------- globals */

static struct physical_memory_map_globals physical_memory_map_globals;

/* ---------- public code */

void physical_memory_allocate(
	void)
{
	physical_memory_map_globals.game_state_base_address = XPhysicalAlloc(
		GAME_STATE_SIZE,
		GAME_STATE_BASE_ADDRESS - PHYSICAL_MEMORY_BASE_ADDRESS,
		0,
		PAGE_READWRITE);
	match_assert("c:\\halo\\SOURCE\\cache\\physical_memory_map.c", 46, (unsigned long)physical_memory_map_globals.game_state_base_address==GAME_STATE_BASE_ADDRESS);

	physical_memory_map_globals.tag_cache_base_address = XPhysicalAlloc(
		TAG_CACHE_SIZE,
		TAG_CACHE_BASE_ADDRESS - PHYSICAL_MEMORY_BASE_ADDRESS,
		0,
		PAGE_READWRITE);
	match_assert("c:\\halo\\SOURCE\\cache\\physical_memory_map.c", 50, (unsigned long)physical_memory_map_globals.tag_cache_base_address==TAG_CACHE_BASE_ADDRESS);

	physical_memory_map_globals.texture_cache_base_address = XPhysicalAlloc(
		TEXTURE_CACHE_SIZE,
		MAXULONG_PTR,
		0,
		PAGE_READWRITE | PAGE_WRITECOMBINE);
	match_assert("c:\\halo\\SOURCE\\cache\\physical_memory_map.c", 55, physical_memory_map_globals.texture_cache_base_address);

	physical_memory_map_globals.sound_cache_base_address = XPhysicalAlloc(
		SOUND_CACHE_SIZE,
		MAXULONG_PTR,
		0,
		PAGE_READWRITE);
	match_assert("c:\\halo\\SOURCE\\cache\\physical_memory_map.c", 58, physical_memory_map_globals.sound_cache_base_address);
}

void physical_memory_verify(
	void)
{
	byte *address;

	for (address = (byte *)physical_memory_map_globals.tag_cache_base_address;
		address < (byte *)physical_memory_map_globals.tag_cache_base_address + TAG_CACHE_SIZE;
		address += PHYSICAL_MEMORY_PAGE_SIZE)
	{
		unsigned long page_status = XQueryMemoryProtect(address);

		match_assert("c:\\halo\\SOURCE\\cache\\physical_memory_map.c", 77, page_status == PAGE_READWRITE);
	}

	for (address = (byte *)physical_memory_map_globals.game_state_base_address;
		address < (byte *)physical_memory_map_globals.game_state_base_address + GAME_STATE_CPU_SIZE;
		address += PHYSICAL_MEMORY_PAGE_SIZE)
	{
		unsigned long page_status = XQueryMemoryProtect(address);

		match_assert("c:\\halo\\SOURCE\\cache\\physical_memory_map.c", 86, page_status == PAGE_READWRITE);
	}
}

void physical_memory_free(
	void)
{
	if (physical_memory_map_globals.game_state_base_address)
	{
		XPhysicalFree(physical_memory_map_globals.game_state_base_address);
	}

	if (physical_memory_map_globals.tag_cache_base_address)
	{
		XPhysicalFree(physical_memory_map_globals.tag_cache_base_address);
	}

	if (physical_memory_map_globals.texture_cache_base_address)
	{
		XPhysicalFree(physical_memory_map_globals.texture_cache_base_address);
	}

	if (physical_memory_map_globals.sound_cache_base_address)
	{
		XPhysicalFree(physical_memory_map_globals.sound_cache_base_address);
	}
}

void *physical_memory_get_game_state_base_address(
	void)
{
	return physical_memory_map_globals.game_state_base_address;
}

void *physical_memory_get_tag_cache_base_address(
	void)
{
	return physical_memory_map_globals.tag_cache_base_address;
}

void *physical_memory_get_texture_cache_base_address(
	void)
{
	return physical_memory_map_globals.texture_cache_base_address;
}

void *physical_memory_get_sound_cache_base_address(
	void)
{
	return physical_memory_map_globals.sound_cache_base_address;
}

/* ---------- private code */
