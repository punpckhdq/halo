/*
SOUND_DEFINITIONS.H

file has inline function assertions.
*/

#ifndef __SOUND_DEFINITIONS_H
#define __SOUND_DEFINITIONS_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	SOUND_DEFINITION_TAG = 'snd!',
	SOUND_DEFINITION_VERSION = 4,
	MAXIMUM_PROMOTION_RULES_PER_SOUND = 4,
	MAXIMUM_PITCH_RANGES_PER_SOUND = 8,
	MAXIMUM_PERMUTATIONS_PER_PITCH_RANGE = 256,
	MAXIMUM_PERMUTATIONS_PER_RANDOM_PITCH_RANGE = 32,
	MAXIMUM_SOUND_DATA_SIZE = 0x400000,
	MAXIMUM_SOUND_MOUTH_DATA_SIZE = 8192,
	SOUND_MOUTH_SAMPLES_PER_SECOND = 30,
	MAXIMUM_SOUND_SUBTITLE_DATA_SIZE = 512,
	SOUND_COMPRESSION_BLOCK_SIZE = 64,
};

enum
{
	LOOPING_SOUND_DEFINITION_TAG = 'lsnd',
	LOOPING_SOUND_DEFINITION_VERSION = 3,
	CUSTOM_MUSIC_PLAY_ID = 'mply',
	MAXIMUM_TRACKS_PER_LOOPING_SOUND = 4,
	MAXIMUM_DETAIL_SOUNDS_PER_LOOPING_SOUND = 32,
};

enum
{
	_looping_sound_deafening_bit = 0,
	_looping_sound_fake_impulse_sound_bit,
	_looping_sound_stops_music_bit,
	NUMBER_OF_LOOPING_SOUND_FLAGS
};

/* ---------- macros */

#define sound_definition_get(index) ((struct sound_definition *)tag_get(SOUND_DEFINITION_TAG, (index)))

/* ---------- structures */

struct sound_scale_modifiers
{
	real skip_fraction;
	real gain;
	real pitch;
	long unused0[3];
};

struct sound_definition
{
	long flags;
	short class_index;
	short sample_rate;
	real minimum_distance;
	real maximum_distance;
	real skip_fraction;
	real pitch_lower_bound;
	real pitch_upper_bound;
	real inner_cone_angle;
	real outer_cone_angle;
	real outer_cone_gain;
	real gain;
	real maximum_bend;
	long unused[3];
	struct sound_scale_modifiers scale_lower_bound;
	struct sound_scale_modifiers scale_upper_bound;
	short encoding;
	short compression;
	struct tag_reference promotion_sound;	// sound_definition
	short promotion_count;
	word pad2;
	long runtime_maximum_play_time;
	long runtime_promotion_counter;
	long runtime_promotion_time;
	long runtime_scripting_time;
	long runtime_scripting_sound_index;
	struct tag_block pitch_ranges;
};

struct looping_sound_scale_modifiers
{
	real detail_period;
	long unused0[2];
};

struct looping_sound_definition
{
	unsigned long flags;
	struct looping_sound_scale_modifiers scale_lower_bound;
	struct looping_sound_scale_modifiers scale_upper_bound;
	long runtime_scripting_sound_index;
	real runtime_maximum_distance;
	long unused[2];
	struct tag_reference continuous_damage_effect;	// continuous_damage_effect_definition
	struct tag_block tracks;
	struct tag_block details;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __SOUND_DEFINITIONS_H
