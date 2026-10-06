/*
XBOX_SOUND_CACHE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "texture_cache.h"
#include "sound_cache.h"
#include "physical_memory_map.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "rasterizer.h"
#include "console.h"
#include "lruv_cache.h"
#include "sound_manager.h"
#include "render_debug.h"
#include "sound_definitions.h"
#include "draw_string.h"
#include "terminal.h"
#include "platform_sound.h"
#include "sound_preferences.h"
#include "sound/sound_import/sound_import.h"

/* ---------- constants */

enum
{
	MAXIMUM_CACHE_SOUNDS = 512, /* fake name */
	SOUND_CACHE_PAGE_COUNT = 1024, /* fake name */
	SOUND_CACHE_PAGE_SIZE_BITS = 12, /* fake name */
	SOUND_CACHE_BLOWN_REPORT_INTERVAL_MILLISECONDS = 10000, /* fake name */
	SOUND_CACHE_DEBUG_ROW_HEIGHT = 10, /* fake name */
};

/* ---------- macros */

#define cache_sound_get(index) ((struct cache_sound_datum *)datum_get(xbox_sound_cache_globals.cache_sounds, (index))) /* fake name */

/* ---------- structures */

struct cache_sound_datum
{
	short identifier;
	boolean available;
	boolean postprocessed;
	byte software_reference_count;
	byte hardware_reference_count;
	struct sound_permutation *sound;
};

/* ---------- prototypes */

static boolean sound_cache_locked_block_proc(long block_index);
static void sound_cache_delete_block_proc(long block_index);
static char const *cache_block_get_sound_permutation_name(long block_index);
static void sound_cache_start_loading_sound(struct sound_permutation *sound);
static void render_inverse_transform_screen_point(real_point2d const *screen_pos, real_point3d *world_pos, real_vector3d *world_vec);

/* ---------- globals */

boolean debug_sound_cache;
boolean debug_sound_reference_counts;
short assertion_count;

static unsigned long sound_cache_last_blown_time; /* fake name */

static struct
{
	struct data_array *cache_sounds;
	byte *base_address;
	struct lruv_cache *cache;
} xbox_sound_cache_globals;

/* ---------- public code */

void sound_cache_new(
	void)
{
	xbox_sound_cache_globals.cache_sounds = data_new(
		"xbox sound",
		MAXIMUM_CACHE_SOUNDS,
		sizeof(struct cache_sound_datum));
	match_assert("c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c", 69, xbox_sound_cache_globals.cache_sounds);

	xbox_sound_cache_globals.cache = lruv_new(
		"xbox sound cache",
		SOUND_CACHE_PAGE_COUNT,
		SOUND_CACHE_PAGE_SIZE_BITS,
		MAXIMUM_CACHE_SOUNDS,
		sound_cache_delete_block_proc,
		sound_cache_locked_block_proc);
	match_assert("c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c", 73, xbox_sound_cache_globals.cache);

	xbox_sound_cache_globals.base_address = physical_memory_get_sound_cache_base_address();
	match_assert("c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c", 76, xbox_sound_cache_globals.base_address);
}

void sound_cache_delete(
	void)
{
	data_dispose(xbox_sound_cache_globals.cache_sounds);
	lruv_delete(xbox_sound_cache_globals.cache);
	xbox_sound_cache_globals.base_address = NULL;
}

void sound_cache_open(
	void)
{
	data_make_valid(xbox_sound_cache_globals.cache_sounds);
}

void sound_cache_flush(
	void)
{
	struct data_iterator iterator;
	struct cache_sound_datum *cache_sound;

	data_iterator_new(&iterator, xbox_sound_cache_globals.cache_sounds);

	while (cache_sound = data_iterator_next(&iterator))
	{
		if (!cache_sound->software_reference_count && !cache_sound->hardware_reference_count)
		{
			sound_cache_sound_delete(cache_sound->sound);
		}
	}
}

void sound_cache_close(
	void)
{
	struct data_iterator iterator;
	struct cache_sound_datum *cache_sound;

	data_iterator_new(&iterator, xbox_sound_cache_globals.cache_sounds);

	while (cache_sound = data_iterator_next(&iterator))
	{
		sound_cache_sound_delete(cache_sound->sound);
	}

	data_make_invalid(xbox_sound_cache_globals.cache_sounds);
}

void sound_cache_idle(
	void)
{
	lruv_idle(xbox_sound_cache_globals.cache);
	match_vassert("c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c", 148, !assertion_count, "hardware sound reference count failure.");
}

void sound_cache_sound_new(
	long tag_index,
	struct sound_permutation *sound)
{
	match_assert("c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c", 158, sound->cache_base_address==NULL);

	sound->cache_block_index = NONE;
	sound->cache_base_address = NULL;
	sound->cache_tag_index = tag_index;
}

void sound_cache_sound_delete(
	struct sound_permutation *sound)
{
	if (sound->cache_block_index != NONE)
	{
		match_vassert(
			"c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c",
			173,
			!cache_sound_get(sound->cache_block_index)->software_reference_count,
			csprintf(
				temporary,
				"tried to delete sound %s(%s) from the cache while it was playing (soft).",
				tag_get_name(cache_sound_get(sound->cache_block_index)->sound->runtime_tag_index),
				cache_sound_get(sound->cache_block_index)->sound->name));
		match_vassert(
			"c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c",
			174,
			!cache_sound_get(sound->cache_block_index)->hardware_reference_count,
			csprintf(
				temporary,
				"tried to delete sound %s(%s) from the cache while it was playing (hard).",
				tag_get_name(cache_sound_get(sound->cache_block_index)->sound->runtime_tag_index),
				cache_sound_get(sound->cache_block_index)->sound->name));

		lruv_block_delete(xbox_sound_cache_globals.cache, sound->cache_block_index);
	}

	sound->cache_block_index = NONE;
	sound->cache_base_address = NULL;
}

boolean _sound_cache_sound_request(
	struct sound_permutation *sound,
	boolean block,
	boolean load,
	boolean reference)
{
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c", 194, load || !block);
	match_assert("c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c", 196, load || !reference);
	match_assert("c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c", 198, sound->cache_tag_index!=0);

	if (sound->cache_block_index == NONE && load)
	{
		sound_cache_start_loading_sound(sound);
	}

	if (sound->cache_block_index != NONE)
	{
		struct cache_sound_datum *cache_sound;

		lruv_block_touch(xbox_sound_cache_globals.cache, sound->cache_block_index);

		do
		{
			cache_sound = cache_sound_get(sound->cache_block_index);

			if (cache_sound->available)
			{
				if (!cache_sound->postprocessed)
				{
					cache_sound->postprocessed = TRUE;
					cache_sound->software_reference_count = 0;
					cache_sound->hardware_reference_count = 0;
				}

				if (reference)
				{
					if (debug_sound_reference_counts)
					{
						error(
							_error_silent,
							"--- request %d %s",
							cache_sound->software_reference_count,
							cache_sound->sound->name);
					}

					match_assert("c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c", 236, cache_sound->software_reference_count<UNSIGNED_CHAR_MAX);
					cache_sound->software_reference_count++;
				}

				result = TRUE;
			}
			else
			{
				SwitchToThread();
			}
		}
		while (!result && block);
	}

	return result;
}

void sound_cache_sound_finished(
	struct sound_permutation *sound)
{
	struct cache_sound_datum *cache_sound = cache_sound_get(sound->cache_block_index);

	if (debug_sound_reference_counts)
	{
		error(
			_error_silent,
			"--- finish %d %s",
			cache_sound->software_reference_count,
			cache_sound->sound->name);
	}

	match_assert("c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c", 263, cache_sound->software_reference_count);
	cache_sound->software_reference_count--;
}

void sound_cache_sound_hardware_lock(
	struct sound_permutation *sound)
{
	struct cache_sound_datum *cache_sound = cache_sound_get(sound->cache_block_index);

	if (cache_sound->hardware_reference_count < UNSIGNED_CHAR_MAX)
	{
		cache_sound->hardware_reference_count++;
	}
	else
	{
		assertion_count++;
	}
}

void sound_cache_sound_hardware_unlock(
	struct sound_permutation *sound)
{
	struct cache_sound_datum *cache_sound = cache_sound_get(sound->cache_block_index);

	if (cache_sound->hardware_reference_count)
	{
		cache_sound->hardware_reference_count--;
	}
	else
	{
		assertion_count++;
	}
}

static boolean sound_cache_locked_block_proc(
	long block_index)
{
	struct cache_sound_datum *cache_sound = cache_sound_get(block_index);

	return !cache_sound->available || cache_sound->software_reference_count || cache_sound->hardware_reference_count;
}

static void sound_cache_delete_block_proc(
	long block_index)
{
	struct cache_sound_datum *cache_sound = cache_sound_get(block_index);

	match_vassert(
		"c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c",
		321,
		!cache_sound->software_reference_count && !cache_sound->hardware_reference_count,
		csprintf(
			temporary,
			"tried to delete sound %s(%s) from the cache while it was playing.",
			tag_get_name(cache_sound->sound->runtime_tag_index),
			cache_sound->sound->name));
	match_assert("c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c", 324, cache_sound->sound->cache_block_index==block_index);

	cache_sound->sound->cache_block_index = NONE;
	cache_sound->sound->cache_base_address = NULL;
	datum_delete(xbox_sound_cache_globals.cache_sounds, block_index);
}

static char const *cache_block_get_sound_permutation_name(
	long block_index)
{
	static char name[256];
	struct cache_sound_datum *cache_sound = cache_sound_get(block_index);

	sprintf(
		name,
		"%s (%s)",
		tag_get_name(cache_sound->sound->runtime_tag_index),
		cache_sound->sound->name);

	return name;
}

static void sound_cache_start_loading_sound(
	struct sound_permutation *sound)
{
	long cache_block_index = lruv_block_new(xbox_sound_cache_globals.cache, sound->samples.size);

	if (cache_block_index != NONE)
	{
		byte *address = xbox_sound_cache_globals.base_address + lruv_block_get_address(xbox_sound_cache_globals.cache, cache_block_index);
		long new_cache_sound_index = datum_new_at_index(xbox_sound_cache_globals.cache_sounds, cache_block_index);
		struct cache_sound_datum *cache_sound = cache_sound_get(cache_block_index);

		match_assert("c:\\halo\\SOURCE\\cache\\xbox_sound_cache.c", 368, new_cache_sound_index==cache_block_index);

		sound->cache_block_index = cache_block_index;
		sound->cache_base_address = address;
		cache_sound->sound = sound;
		cache_file_read(
			sound->cache_tag_index,
			sound->samples.file_offset,
			sound->samples.size,
			address,
			&cache_sound->available,
			FALSE);
	}
	else if (system_milliseconds() - sound_cache_last_blown_time > SOUND_CACHE_BLOWN_REPORT_INTERVAL_MILLISECONDS)
	{
		terminal_printf(global_real_argb_purple, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		error(_error_silent, "SOUND CACHE BLOWN!!!! double-click \"GETSTABBED.BAT\" on your PC now!!!");
		terminal_printf(global_real_argb_purple, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		lruv_debug_to_file(
			"d:\\stabbed.txt",
			sound->name,
			sound->samples.size,
			xbox_sound_cache_globals.cache,
			scenario_debug_to_file,
			cache_block_get_sound_permutation_name);
		sound_cache_last_blown_time = system_milliseconds();
	}
}

void sound_cache_debug_render(
	void)
{
	if (debug_sound_cache)
	{
		byte page_usage[SOUND_CACHE_PAGE_COUNT];
		long page_index;
		real_argb_color const *colors[NUMBER_OF_LRUV_CACHE_PAGE_USAGE_FLAGS] =
		{
			global_real_argb_red,
			global_real_argb_green,
			global_real_argb_blue,
			global_real_argb_yellow
		};

		lruv_cache_get_page_usage(xbox_sound_cache_globals.cache, page_usage);

		for (page_index = 0; page_index < RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH; page_index++)
		{
			long flag_index;

			for (flag_index = 0; flag_index < NUMBER_OF_LRUV_CACHE_PAGE_USAGE_FLAGS; flag_index++)
			{
				if (TEST_FLAG(page_usage[(short)page_index], flag_index))
				{
					real_point2d screen_points[2];
					real_point3d points[2];
					real near_distance;
					long point_index;

					screen_points[0].x = (real)(page_index % RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH);
					screen_points[0].y = (real)(SOUND_CACHE_DEBUG_ROW_HEIGHT * (flag_index + NUMBER_OF_LRUV_CACHE_PAGE_USAGE_FLAGS * (page_index / RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH)));
					screen_points[1].x = screen_points[0].x;
					screen_points[1].y = (real)(SOUND_CACHE_DEBUG_ROW_HEIGHT * (flag_index + NUMBER_OF_LRUV_CACHE_PAGE_USAGE_FLAGS * (page_index / RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH)) + SOUND_CACHE_DEBUG_ROW_HEIGHT);

					near_distance = render.camera.z_near + 0.001f;

					for (point_index = 0; point_index < 2; point_index++)
					{
						real_vector3d vector;
						real distance;
						real_point2d const *screen_pos = &screen_points[point_index];
						real_point3d *world_pos = &points[point_index];
						real_vector3d *world_vec = &vector;

						render_inverse_transform_screen_point(screen_pos, world_pos, world_vec);
						distance = near_distance / dot_product3d(&vector, &render.camera.forward);
						point_from_line3d(&points[point_index], &vector, distance, &points[point_index]);
					}

					render_debug_line(TRUE, &points[0], &points[1], colors[flag_index]);
				}
			}
		}
	}
}

/* ---------- private code */

static void render_inverse_transform_screen_point(
	real_point2d const *screen_pos,
	real_point3d *world_pos,
	real_vector3d *world_vec)
{
	enum
	{
		_upper_left = 0,
		_upper_right,
		_lower_left,
		_lower_right,
		_camera_position
	};
	real_vector3d across;
	real_vector3d down;
	real_point3d point;
	real u = screen_pos->x / RASTERIZER_TARGET_RENDER_PRIMARY_WIDTH;
	real v = 1.f - screen_pos->y / RASTERIZER_TARGET_RENDER_PRIMARY_HEIGHT;

	add_vectors3d(
		(real_vector3d const *)&render.frustum.world_vertices[_camera_position],
		global_zero_vector3d,
		(real_vector3d *)world_pos);
	vector_from_points3d(
		&render.frustum.world_vertices[_upper_left],
		&render.frustum.world_vertices[_upper_right],
		&across);
	vector_from_points3d(
		&render.frustum.world_vertices[_upper_left],
		&render.frustum.world_vertices[_lower_left],
		&down);
	point_from_line3d(&render.frustum.world_vertices[_upper_left], &across, u, &point);
	point_from_line3d(&point, &down, v, &point);
	vector_from_points3d(world_pos, &point, world_vec);
}
