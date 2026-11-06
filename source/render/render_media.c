/*
RENDER_MEDIA.C
*/

/* ---------- headers */

#include "cseries.h"
#include "render.h"

/* ---------- constants */

enum
{
	MAXIMUM_FRAME_LEVELS_OF_DETAIL = 5,			/* fake name */
	MAXIMUM_RENDER_TARGET_LEVELS_OF_DETAIL = 3,	/* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct frame_level_of_detail	/* fake name */
{
	long level_count;														/* fake name */
	real minimum_fraction;													/* fake name */
	real fractions[MAXIMUM_FRAME_LEVELS_OF_DETAIL];							/* fake name */
	real unused[3];															/* fake name */
	real render_target_fractions[MAXIMUM_RENDER_TARGET_LEVELS_OF_DETAIL];	/* fake name */
	real minimum_distance;													/* fake name */
	real distances[MAXIMUM_FRAME_LEVELS_OF_DETAIL];							/* fake name */
	real maximum_level;														/* fake name */
	real minimum_level;														/* fake name */
};

struct render_media_globals	/* fake name */
{
	boolean initialized;		/* fake name */
	long render_target_index;	/* fake name */
	long unused[9];				/* fake name */
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

/* ---------- private code */
