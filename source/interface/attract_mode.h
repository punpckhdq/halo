/*
ATTRACT_MODE.H

header included in hcex build.
*/

#ifndef __ATTRACT_MODE_H
#define __ATTRACT_MODE_H
#pragma once

/* ---------- constants */

enum
{
	_bink_attract1_movie= 0,
	_bink_attract2_movie,
	_bink_attract3_movie,
	NUMBER_OF_ATTRACT_MODE_MOVIES,

	_bink_intro_movie= 3,
	_bink_outro_movie,
	NUMBER_OF_BINK_MOVIES
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/ATTRACT_MODE.C */

boolean attract_mode_should_start(void);
void attract_mode_reset_timer(void);
const char *attract_mode_get_localized_movie_path(short movie);
void attract_mode_start(void);
void game_end_credits_start(void);

/* ---------- globals */

/* ---------- public code */

#endif // __ATTRACT_MODE_H
