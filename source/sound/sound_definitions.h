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
	_sound_sample_rate_22k = 0,
	_sound_sample_rate_44k,
	NUMBER_OF_SOUND_SAMPLE_RATES,
};

enum
{
	_sound_encoding_mono = 0,
	_sound_encoding_stereo,
	NUMBER_OF_SOUND_ENCODINGS,
};

enum
{
	_sound_compression_none = 0,
	_sound_compression_xbox_adpcm,
	_sound_compression_ima_adpcm,
	_sound_compression_ogg,
	NUMBER_OF_SOUND_COMPRESSION_TYPES,
};

enum
{
	_looping_sound_deafening_bit = 0,
	_looping_sound_fake_impulse_sound_bit,
	_looping_sound_stops_music_bit,
	NUMBER_OF_LOOPING_SOUND_FLAGS,
};

enum
{
	_fade_in_at_start_bit = 0,
	_fade_out_at_stop_bit,
	_fade_in_alternate_bit,
	NUMBER_OF_LOOPING_SOUND_TRACK_FLAGS,
};

enum
{
	_detail_dont_play_with_alternate_bit = 0,
	_detail_dont_play_without_alternate_bit,
	NUMBER_OF_LOOPING_SOUND_DETAIL_FLAGS,
};

enum
{
	_sound_definition_fit_to_compression_block_size_bit = 0,
	_sound_definition_linked_permutations_bit,
	NUMBER_OF_SOUND_DEFINITION_FLAGS,
};

/* ---------- macros */

#define sound_definition_get(index) ((struct sound_definition *)tag_get(SOUND_DEFINITION_TAG, (index)))
#define looping_sound_definition_get(index) ((struct looping_sound_definition *)tag_get(LOOPING_SOUND_DEFINITION_TAG, (index)))

/* ---------- structures */

struct sound_scale_modifiers
{
	real skip_fraction;
	real gain;
	real pitch;
	long unused0[3];
};

struct sound_permutation
{
	char name[32];
	real skip_fraction;
	real gain;
	short duplicate_compression;
	short next_permutation_index;
	long cache_block_index;
	void *cache_base_address;
	long cache_tag_index;
	long unused0[1];
	long runtime_tag_index;
	struct tag_data samples;
	struct tag_data mouth_data;
	struct tag_data subtitle_data;
};

struct sound_pitch_range
{
	char name[32];
	real natural_pitch;
	real bend_lower_bound;
	real bend_upper_bound;
	short actual_permutation_count;
	word plenty_of_unused_space_here;
	real runtime_oo_natural_pitch;
	unsigned long runtime_permutation_flags;
	short runtime_last_permutation_index;
	short runtime_discarded_permutation_index;
	struct tag_block permutations;
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
	struct tag_reference promotion_sound;
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
	struct tag_reference continuous_damage_effect;
	struct tag_block tracks;
	struct tag_block details;
};

struct looping_sound_track
{
	unsigned long flags;
	real gain;
	real fade_in_duration;
	real fade_out_duration;
	long unused[8];
	struct tag_reference start_sound;
	struct tag_reference loop_sound;
	struct tag_reference stop_sound;
	long unused2[8];
	struct tag_reference alternate_loop_sound;
	struct tag_reference alternate_stop_sound;
};

struct looping_sound_detail
{
	struct tag_reference sound;
	real period_lower_bound;
	real period_upper_bound;
	real gain;
	long flags;
	long unused0[12];
	real theta_lower_bound;
	real theta_upper_bound;
	real phi_lower_bound;
	real phi_upper_bound;
	real distance_lower_bound;
	real distance_upper_bound;
};

/* ---------- prototypes/SOUND_DEFINITIONS.C */

real sound_definition_get_maximum_distance(long sound_definition_index);
real sound_definition_get_minimum_distance(long sound_definition_index);
byte *sound_permutation_get_mouth_aperture(struct sound_permutation const *permutation, short tick_index);
short sound_definition_find_pitch_range_by_pitch(struct sound_definition *sound, real pitch, short old_range_index);
void try_to_reset_permutations(struct sound_pitch_range *range);
real sound_permutation_get_real_mouth_aperture(struct sound_permutation const *permutation, short estimated_tick_index);
short sound_definition_next_permutation(struct sound_definition *sound, short pitch_range_index, short looping_last_permutation_index);

/* ---------- globals */

extern long const sound_sample_rate_samples_per_second[NUMBER_OF_SOUND_SAMPLE_RATES];
extern real const oo_unsigned_char_max;

/* ---------- public code */

__inline long sound_samples_per_second(
	short sample_rate)
{
	match_assert("c:\\halo\\source\\sound\\sound_definitions.h", 309, sample_rate>=0 && sample_rate<NUMBER_OF_SOUND_SAMPLE_RATES);

	return sound_sample_rate_samples_per_second[sample_rate];
}

#endif // __SOUND_DEFINITIONS_H
