/*
ATTRACT_MODE.C

symbols in this file:
000CB3F0 00a0:
	_attract_mode_should_start (0000)
000CB490 0010:
	_attract_mode_reset_timer (0000)
000CB4A0 01d0:
	_attract_mode_get_localized_movie_path (0000)
000CB670 0070:
	_attract_mode_start (0000)
000CB6E0 0020:
	_game_end_credits_start (0000)
002705A8 0058:
	??_C@_0FI@JAHFPCBF@unable?5to?5locate?5any?5movie?5for?5m@ (0000)
00270600 0017:
	??_C@_0BH@DKPBBHNJ@d?3?2bink?2attract3?$CFs?4bik?$AA@ (0000)
00270618 0017:
	??_C@_0BH@PGFLBHEH@d?3?2bink?2attract2?$CFs?4bik?$AA@ (0000)
00270630 0017:
	??_C@_0BH@HINEBAKE@d?3?2bink?2attract1?$CFs?4bik?$AA@ (0000)
00270648 0016:
	??_C@_0BG@COBLFOAN@d?3?2bink?2credits?$CFs?4bik?$AA@ (0000)
00270660 0014:
	??_C@_0BE@EMDLENBN@d?3?2bink?2intro?$CFs?4bik?$AA@ (0000)
00270674 0028:
	??_C@_0CI@DAMIEDGH@movie?$DO?$DN0?5?$CG?$CG?5movie?$DMNUMBER_OF_BINK@ (0000)
0027069C 0028:
	??_C@_0CI@HBODAJLB@c?3?2halo?2SOURCE?2interface?2attract@ (0000)
002706C4 0004:
	??_C@_03EBIAHDEK@_it?$AA@ (0000)
002706C8 0004:
	??_C@_03HNLBMOJ@_es?$AA@ (0000)
002706CC 0004:
	??_C@_03BMIGJDPB@_fr?$AA@ (0000)
002706D0 0004:
	??_C@_03BKIBMDAJ@_de?$AA@ (0000)
002E4C84 0002:
	_data_002e4c84 (0000)
00453AD8 0088:
	_bss_00453ad8 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries_windows.h"
#include "attract_mode.h"
#include "real_math.h"
#include "cache_files.h"
#include "bink_playback.h"
#include "ui_widget.h"
#include "sound_manager.h"
#include "international_strings.h"
#include "files.h"

/* ---------- constants */

enum
{
	ATTRACT_MODE_COUNTDOWN= 75000,
	MUSIC_FADE_TIME= 1500
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* random_math.c doesn't appear to have a header? */
extern unsigned long *get_global_local_random_seed_address(void);
extern short seed_random_range(unsigned long *seed, short lower_bound, short upper_bound);

extern boolean network_game_is_active();

extern unsigned long event_manager_time_of_last_event(void);

/* ---------- globals */

static unsigned long bss_00453ad8;

static char bss_00453ad8_8[128];

short data_002e4c84= NONE;

/* ---------- public code */

boolean attract_mode_should_start(
	void)
{
	boolean should_start= FALSE;
	real progress;

	if (cache_files_precache_in_progress() &&
		cache_files_precache_map_status(&progress)==1)
	{
		cache_files_precache_map_end();
	}

	if (main_menu_screen_is_active() &&
		!cache_files_precache_in_progress() &&
		!network_game_is_active() &&
		!bink_playback_active())
	{
		unsigned long time_elapsed;
		unsigned long current_time= system_milliseconds();
		unsigned long time_since_last_event= event_manager_time_of_last_event();
		
		time_since_last_event= MAX(bss_00453ad8, time_since_last_event);
		time_elapsed= current_time-time_since_last_event;
	
		if (time_elapsed>=ATTRACT_MODE_COUNTDOWN-MUSIC_FADE_TIME)
		{
			if (ui_main_menu_music_active())
			{
				ui_stop_main_menu_music();
			}
		}
		else
		{
			if (!ui_main_menu_music_active())
			{
				ui_start_main_menu_music();
			}
		}

		if (time_elapsed>=ATTRACT_MODE_COUNTDOWN)
		{
			should_start= TRUE;
		}
	}

	return should_start;
}

void attract_mode_reset_timer(
	void)
{
	bss_00453ad8= system_milliseconds();

	return;
}

const char *attract_mode_get_localized_movie_path(
	short movie)
{
	unsigned long language= 0;
	const char *movie_format= "";
	const char *language_suffixes[]=
	{
		"_de",
		"_fr",
		"_es",
		"_it",
		"",
		"",
		""
	};
	struct file_reference movie_file;

	match_assert("c:\\halo\\SOURCE\\interface\\attract_mode.c", 163, movie>=0 && movie<NUMBER_OF_BINK_MOVIES);

	switch (XGetLanguage())
	{
	case XC_LANGUAGE_ENGLISH:
		language= _english;
		break;
	case XC_LANGUAGE_JAPANESE:
		language= _japanese;
		break;
	case XC_LANGUAGE_GERMAN:
		language= _german;
		break;
	case XC_LANGUAGE_FRENCH:
		language= _french;
		break;
	case XC_LANGUAGE_SPANISH:
		language= _spanish;
		break;
	case XC_LANGUAGE_ITALIAN:
		language= _italian;
		break;
	default:
		language= _unknown;
		break;
	}

	switch (movie)
	{
	case _bink_attract1_movie:
		_snprintf(bss_00453ad8_8, NUMBEROF(bss_00453ad8_8), "d:\\bink\\attract1%s.bik", language_suffixes[language]);
		break;
	case _bink_attract2_movie:
		_snprintf(bss_00453ad8_8, NUMBEROF(bss_00453ad8_8), "d:\\bink\\attract2%s.bik", language_suffixes[language]);
		break;
	case _bink_attract3_movie:
		_snprintf(bss_00453ad8_8, NUMBEROF(bss_00453ad8_8), "d:\\bink\\attract3%s.bik", language_suffixes[language]);
		break;
	case _bink_intro_movie:
		_snprintf(bss_00453ad8_8, NUMBEROF(bss_00453ad8_8), "d:\\bink\\intro%s.bik", language_suffixes[language]);
		break;
	case _bink_outro_movie:
		_snprintf(bss_00453ad8_8, NUMBEROF(bss_00453ad8_8), "d:\\bink\\outro%s.bik", language_suffixes[language]);
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\interface\\attract_mode.c", 198, FALSE, "unreachable");
		break;
	}

	if (file_exists(file_reference_create_from_path(&movie_file, bss_00453ad8_8, FALSE)))
	{

	}

	return bss_00453ad8_8;
}

void attract_mode_start(
	void)
{
	short video_index;

	while (TRUE)
	{
		video_index= seed_random_range(get_global_local_random_seed_address(), 0, NUMBER_OF_ATTRACT_MODE_MOVIES);
		video_index= PIN(video_index, 0, _bink_attract3_movie);

		if (video_index!=data_002e4c84)
		{
			data_002e4c84= video_index;
			break;
		}
	}

	ui_stop_main_menu_music();

	bink_playback_start(attract_mode_get_localized_movie_path(video_index), 0x2e);

	if (!bink_playback_active())
	{
		bss_00453ad8= system_milliseconds();
	}

	return;
}

void game_end_credits_start(
	void)
{
	ui_stop_main_menu_music();
	sound_stop_all();

	bink_playback_start(attract_mode_get_localized_movie_path(_bink_outro_movie), 0x2e);

	return;
}

/* ---------- private code */
