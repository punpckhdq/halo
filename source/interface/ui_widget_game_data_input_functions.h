/*
UI_WIDGET_GAME_DATA_INPUT_FUNCTIONS.H

header included in hcex build.
*/

#ifndef __UI_WIDGET_GAME_DATA_INPUT_FUNCTIONS_H
#define __UI_WIDGET_GAME_DATA_INPUT_FUNCTIONS_H
#pragma once

/* ---------- headers */

#include "player_profile.h"
#include "game_engine.h"

/* ---------- constants */

enum
{
	NUMBER_OF_CACHED_PROFILES = 3
};

/* ---------- macros */

/* ---------- structures */

struct _cached_player_profile
{
	long profile_index;
	struct player_profile profile;
};

struct _cached_variant_profile
{
	long profile_index;
	struct game_variant variant;
};

/* ---------- prototypes/UI_WIDGET_GAME_DATA_INPUT_FUNCTIONS.C */

void ui_widget_game_data_function_invoke(struct widget_instance *widget, short function);

/* ---------- globals */

extern struct _cached_player_profile cached_player_profile[NUMBER_OF_CACHED_PROFILES];
extern struct _cached_variant_profile cached_variant_profile[NUMBER_OF_CACHED_PROFILES];

/* ---------- public code */

#endif // __UI_WIDGET_GAME_DATA_INPUT_FUNCTIONS_H
