/*
PLAYER_PROFILE.H

header included in hcex build.
*/

#ifndef __PLAYER_PROFILE_H
#define __PLAYER_PROFILE_H
#pragma once

/* ---------- headers */

#include "input_abstraction.h"
#include "input_windows.h"
#include "event_manager.h"

/* ---------- constants */

enum
{
	_joystick_preset_standard = 0,
	_joystick_preset_south_paw,
	_joystick_preset_legacy,
	_joystick_preset_legacy_south_paw,
	NUMBER_OF_JOYSTICK_PRESETS,
};

/* ---------- macros */

/* ---------- structures */

struct player_profile
{
	wchar_t player_name[12];
	short primary_color_index;
	word flags;
	char single_player_map_flags[10];
	short last_single_player_map_played;
	struct
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

void player_profile_save_last_level_played(short local_player_index);
void player_profile_save_level_completed(short local_player_index);
long player_profile_get_random_color(void);
long player_profile_get_random_good_color(void);

/* ---------- globals */

/* ---------- public code */

#endif // __PLAYER_PROFILE_H
