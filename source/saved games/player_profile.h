/*
PLAYER_PROFILE.H

header included in hcex build.
*/

#ifndef __PLAYER_PROFILE_H
#define __PLAYER_PROFILE_H
#pragma once

/* ---------- headers */

#include "main.h"
#include "input_abstraction.h"
#include "input_windows.h"
#include "event_manager.h"

/* ---------- constants */

enum
{
	MAXIMUM_PLAYER_PROFILE_NAME_LENGTH = 11, /* fake name */
};

enum
{
	_button_preset_standard = 0,
	_button_preset_southpaw,
	_button_preset_jumpy,
	_button_preset_bumperjumper,
	_button_preset_boxer,
};

enum
{
	_joystick_preset_standard = 0,
	_joystick_preset_south_paw,
	_joystick_preset_legacy,
	_joystick_preset_legacy_south_paw,
	NUMBER_OF_JOYSTICK_PRESETS,
};

enum
{
	NUMBER_OF_DEFAULT_PROFILES = 2,
};

enum
{
	_profile_is_default_bit = 0,
	_profile_is_builtin_bit,
	_profile_finished_game_bit,
	NUMBER_OF_PROFILE_FLAGS,

	_default_profile_index_mask = ~MASK(CHAR_BITS),
};

/* ---------- macros */

/* ---------- structures */

struct player_profile
{
	wchar_t player_name[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH+1];
	short primary_color_index;
	word flags;
	char single_player_map_flags[NUMBER_OF_SINGLE_PLAYER_LEVELS];
	short last_single_player_map_played;

	struct _controller_settings
	{
		byte button_preset;
		byte joystick_preset;
		byte look_sensitivity;
		boolean invert_look;
		boolean vibration_disabled;
		boolean flight_stick_aircraft_controls;
		boolean autocenter;
		boolean ingame_help_disabled;
	} controller_settings;
};

/* ---------- prototypes/PLAYER_PROFILE.C */

void player_profiles_initialize(void);
void player_profiles_dispose(void);
void player_profiles_enumerate_available_to_local_player_index(short local_player_index, word *number_of_profiles, long *player_profile_indices, boolean include_default_profiles);
void player_profile_delete(long player_profile_index);
boolean player_profile_get(long player_profile_index, struct player_profile *profile);
boolean player_profile_get_from_path(char *full_path, struct player_profile *profile);
void player_profile_save(long player_profile_index, struct player_profile *profile);
real_argb_color player_profile_get_argb_color(long index);
short player_profile_number_of_available_primary_colors(void);
real_rgb_color player_profile_get_rgb_color(long index);
long player_profile_get_random_good_color(void);
long player_profile_get_random_color(void);
void player_profile_save_last_level_played(short local_player_index);
void player_profile_save_level_completed(short local_player_index);
void player_profile_get_highest_completed_solo_level(struct player_profile const *profile, short *level, short *difficulty);
boolean player_profile_get_enclosing_directory_path(long player_profile_index, char *full_path);
long player_profile_new(short local_player_index, wchar_t *name);

/* ---------- globals */

/* ---------- public code */

#endif // __PLAYER_PROFILE_H
