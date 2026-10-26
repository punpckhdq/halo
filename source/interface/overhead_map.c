/*
OVERHEAD_MAP.C
*/

/* ---------- headers */

#include "cseries.h"
#include "game.h"

/* ---------- constants */

enum
{
	OVERHEAD_MAP_UPDATE_TICKS = 7,	/* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct overhead_map_globals	/* fake name */
{
	boolean active;
	rectangle2d bounds;
	boolean throttle_updates;
	boolean dirty;
	rectangle2d screen_bounds;
	real_point2d position;
	point2d screen_position;
	long last_update_time;
	long unknown24;
	struct bitmap_data *bitmap;
	long unknown2C;
};

/* ---------- prototypes */

/* ---------- globals */

static real const overhead_map_scale = 0.5f;	/* fake name */
static real const overhead_map_width = 84.f;	/* fake name */
static real const overhead_map_height = 80.f;	/* fake name */

static struct overhead_map_globals overhead_map_globals;	/* fake name */

/* ---------- public code */

void overhead_map_initialize(
	void)
{
	return;
}

void overhead_map_initialize_for_new_map(
	void)
{
	return;
}

void overhead_map_dispose_from_old_map(
	void)
{
	overhead_map_globals.active = FALSE;

	return;
}

void overhead_map_post_rasterize(
	void *unused0,
	void *unused1,
	real_point2d const *position)
{
	if (overhead_map_globals.active)
	{
		rectangle2d screen_bounds;

		overhead_map_globals.position = *position;
		screen_bounds.x0 = ((short)position->x >> 1) - overhead_map_globals.bitmap->registration_point.x;
		screen_bounds.y0 = ((short)position->y >> 1) - overhead_map_globals.bitmap->registration_point.y;
		screen_bounds.x1 = overhead_map_globals.bounds.x1 + screen_bounds.x0;
		screen_bounds.y1 = overhead_map_globals.bounds.y1 + screen_bounds.y0;
		overhead_map_globals.screen_position.x = screen_bounds.x0;
		overhead_map_globals.screen_position.y = screen_bounds.y0;
		screen_bounds.x0 += overhead_map_globals.bounds.x0;
		screen_bounds.y0 += overhead_map_globals.bounds.y0;

		if (!overhead_map_globals.throttle_updates ||
			local_time_get() - overhead_map_globals.last_update_time > OVERHEAD_MAP_UPDATE_TICKS)
		{
			overhead_map_globals.screen_bounds = screen_bounds;
			overhead_map_globals.last_update_time = local_time_get();
			overhead_map_globals.dirty = FALSE;
		}
	}

	return;
}

/* ---------- private code */
