/*
ATTRACT_MODE.C
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
#include "errors.h"
#include "event_manager.h"
#include "network_game_globals.h"

/* ---------- constants */

enum
{
	ATTRACT_MODE_COUNTDOWN = 75000,
	MUSIC_FADE_TIME = 1500
};

/* ---------- prototypes */

/* random_math.c doesn't appear to have a header? */
extern unsigned long *get_global_local_random_seed_address(void);
extern short seed_random_range(unsigned long *seed, short lower_bound, short upper_bound);

/* ---------- globals */

static unsigned long bss_00453ad8;
static char bss_00453ae8[128];

short data_002e4c84 = NONE;

/* ---------- public code */

boolean attract_mode_should_start(
	void)
{
	boolean should_start = FALSE;
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
		unsigned long current_time = system_milliseconds();
		unsigned long time_since_last_event = event_manager_time_of_last_event();
		
		time_since_last_event = MAX(bss_00453ad8, time_since_last_event);
		time_elapsed = current_time-time_since_last_event;
	
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
			should_start = TRUE;
		}
	}

	return should_start;
}

void attract_mode_reset_timer(
	void)
{
	bss_00453ad8 = system_milliseconds();

	return;
}

const char *attract_mode_get_localized_movie_path(
	short movie)
{
	short attempted_languages = 0;
	short language;

	const char *language_suffixes[] =
	{
		"_de",
		"_fr",
		"_es",
		"_it",
		"",
		"",
		""
	};

	match_assert("c:\\halo\\SOURCE\\interface\\attract_mode.c", 163, movie>=0 && movie<NUMBER_OF_BINK_MOVIES);

	switch (XGetLanguage())
	{
	case XC_LANGUAGE_GERMAN:
		language = _german;
		break;
	case XC_LANGUAGE_FRENCH:
		language = _french;
		break;
	case XC_LANGUAGE_SPANISH:
		language = _spanish;
		break;
	case XC_LANGUAGE_ITALIAN:
		language = _italian;
		break;
	case XC_LANGUAGE_ENGLISH:
		language = _english;
		break;
	case XC_LANGUAGE_JAPANESE:
		language = _japanese;
		break;
	default:
		language = _unknown;
		break;
	}

	while (TRUE)
	{
		struct file_reference movie_file;

		switch (movie)
		{
		case _bink_intro_movie:
			_snprintf(bss_00453ae8, NUMBEROF(bss_00453ae8), "d:\\bink\\intro%s.bik", language_suffixes[language]);
			break;
		case _bink_outro_movie:
			_snprintf(bss_00453ae8, NUMBEROF(bss_00453ae8), "d:\\bink\\credits%s.bik", language_suffixes[language]);
			break;
		case _bink_attract1_movie:
			_snprintf(bss_00453ae8, NUMBEROF(bss_00453ae8), "d:\\bink\\attract1%s.bik", language_suffixes[language]);
			break;
		case _bink_attract2_movie:
			_snprintf(bss_00453ae8, NUMBEROF(bss_00453ae8), "d:\\bink\\attract2%s.bik", language_suffixes[language]);
			break;
		case _bink_attract3_movie:
			_snprintf(bss_00453ae8, NUMBEROF(bss_00453ae8), "d:\\bink\\attract3%s.bik", language_suffixes[language]);
			break;
		default:
			match_assert("c:\\halo\\SOURCE\\interface\\attract_mode.c", 198, !"unreachable");
			break;
		}

		if (file_exists(file_reference_create_from_path(&movie_file, bss_00453ae8, FALSE)))
		{
			break;
		}

		attempted_languages|= FLAG(language);

		for (language = 0; language<NUMBER_OF_SUPPORTED_LANGUAGES; language++)
		{
			if (!TEST_FLAG(attempted_languages, language))
			{
				break;
			}
		}

		if (language==NUMBER_OF_SUPPORTED_LANGUAGES)
		{
			error(_error_silent, "unable to locate any movie for movie #%d (checked for all possible language variations)", movie);
			bss_00453ae8[0] = '\0';

			break;
		}
	}

	return bss_00453ae8;
}

void attract_mode_start(
	void)
{
	short video_index;

	while (TRUE)
	{
		video_index = seed_random_range(get_global_local_random_seed_address(), 0, NUMBER_OF_ATTRACT_MODE_MOVIES);
		video_index = PIN(video_index, 0, _bink_attract3_movie);

		if (video_index!=data_002e4c84)
		{
			data_002e4c84 = video_index;
			break;
		}
	}

	ui_stop_main_menu_music();

	bink_playback_start(attract_mode_get_localized_movie_path(video_index), 0x2e);

	if (!bink_playback_active())
	{
		bss_00453ad8 = system_milliseconds();
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
