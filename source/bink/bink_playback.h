/*
BINK_PLAYBACK.H

header included in hcex build.
*/

#ifndef __BINK_PLAYBACK_H
#define __BINK_PLAYBACK_H
#pragma once

/* ---------- constants */

enum
{
	_bink_playback_loop_bit= 0,
	_bink_playback_button_click_stops_movie_bit,
	_bink_playback_prevent_events_to_ui_bit,
	_bink_playback_dont_render_ui_bit,
	_bink_playback_full_screen_bit,
	_bink_playback_return_to_main_menu_when_finished_bit,
	_bink_playback_dont_allow_skipping_if_filesystem_check_thread_is_active_bit,
	_bink_playback_eat_up_memory_like_a_goddamn_beaver_bit,
	NUMBER_OF_BINK_PLAYBACK_FLAGS,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/BINK_PLAYBACK.C */

boolean bink_playback_active(void);
boolean bink_playback_ui_rendering_inhibited(void);
boolean bink_playback_in_progress(void);
void bink_playback_initialize(void);
void bink_playback_stop(void);
void bink_playback_dispose(void);
void bink_playback_render(void);
void bink_playback_start(const char *full_pathname, unsigned long flags);
void bink_playback_update(void);

/* ---------- globals */

/* ---------- public code */

#endif // __BINK_PLAYBACK_H
