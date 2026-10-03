/*
SOUND_PREFERENCES.H

header included in hcex build.
*/

#ifndef __SOUND_PREFERENCES_H
#define __SOUND_PREFERENCES_H
#pragma once

/* ---------- constants */

enum
{
	_sound_channel_type_mono_compressed = 0,
	_sound_channel_type_mono_compressed_3d,
	_sound_channel_type_stereo_compressed,
	_sound_channel_type_stereo_compressed_44k,
	NUMBER_OF_SOUND_CHANNEL_TYPES,
};

enum
{
	_sound_channel_3d_bit = 0,
	_sound_channel_stereo_bit,
	_sound_channel_44k_bit,
	_sound_channel_compressed_bit,
	NUMBER_OF_SOUND_CHANNEL_TYPE_FLAGS,
};

/* ---------- macros */

/* ---------- structures */

struct sound_preferences
{
	short platform_code;
	short actual_channel_counts[NUMBER_OF_SOUND_CHANNEL_TYPES];
	short virtual_channel_counts[NUMBER_OF_SOUND_CHANNEL_TYPES];
};

/* ---------- prototypes/SOUND_PREFERENCES.C */

void read_sound_preferences(struct sound_preferences **preferences);
void write_sound_preferences(void);

/* ---------- globals */

extern short sound_channel_type_flags[NUMBER_OF_SOUND_CHANNEL_TYPES];

/* ---------- public code */

#endif // __SOUND_PREFERENCES_H
