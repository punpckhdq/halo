/*
RENDER_MEDIA.C
*/

/* ---------- headers */

#include "cseries.h"
#include "render.h"

/* ---------- constants */

enum
{
	MAXIMUM_FRAME_LEVELS_OF_DETAIL = 5,	/* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct frame_level_of_detail	/* fake name */
{
	long level_count;
	real minimum_fraction;
	real fractions[MAXIMUM_FRAME_LEVELS_OF_DETAIL];
	real unused[3];
	real render_target_fractions[3];
	real minimum_distance;
	real distances[MAXIMUM_FRAME_LEVELS_OF_DETAIL];
	real maximum_level;
	real minimum_level;
};

struct render_media_globals	/* fake name */
{
	boolean initialized;
	long render_target_index;
	long unused[9];
};

/* ---------- prototypes */

/* ---------- globals */

real const media_wave_amplitude = 0.05f;

struct frame_level_of_detail standard_frame_level_of_detail =
{
	4,
	0.f,
	{ 0.33f, 0.66f, 1.f },
	{ 0.f },
	{ 0.f },
	0.f,
	{ 200.f, 400.f, 700.f },
	3.f,
	2.f
};

struct frame_level_of_detail render_target_frame_level_of_detail =
{
	6,
	0.f,
	{ 0.2f, 0.4f, 0.6f, 0.8f, 1.f },
	{ 0.f },
	{ 0.4f, 0.7f, 1.f },
	0.f,
	{ 50.f, 70.f, 125.f, 250.f, 500.f },
	6.f,
	4.f
};

static struct render_media_globals render_media_globals;

/* ---------- public code */

void render_media_initialize_for_new_map(
	void)
{
	render_media_globals.initialized = TRUE;
	render_media_globals.render_target_index = 0;

	return;
}

void render_media_dispose_from_old_map(
	void)
{
	render_media_globals.initialized = FALSE;

	return;
}

boolean render_media_build_render_target(
	void)
{
	return FALSE;
}

void render_media(
	void)
{
	return;
}
