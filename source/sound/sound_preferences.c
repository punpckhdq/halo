/*
SOUND_PREFERENCES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "sound_preferences.h"
#include "sound_manager.h"
#include "sound_definitions.h"
#include "platform_sound.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "network_game_globals.h"

/* ---------- globals */

static struct sound_preferences default_sound_preferences =
{
	_platform_sound_dsound,
	{ 10, 51, 10, 10 },
	{ 9, 46, 9, 9 }
};

short sound_channel_type_flags[NUMBER_OF_SOUND_CHANNEL_TYPES] =
{
	FLAG(_sound_channel_compressed_bit),
	FLAG(_sound_channel_compressed_bit) | FLAG(_sound_channel_3d_bit),
	FLAG(_sound_channel_compressed_bit) | FLAG(_sound_channel_stereo_bit),
	FLAG(_sound_channel_compressed_bit) | FLAG(_sound_channel_stereo_bit) | FLAG(_sound_channel_44k_bit)
};

/* ---------- public code */

void read_sound_preferences(
	struct sound_preferences **preferences)
{
	*preferences = &default_sound_preferences;

	return;
}

void write_sound_preferences(
	void)
{
	return;
}
