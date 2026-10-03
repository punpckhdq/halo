/*
RECORDED_ANIMATION_PLAYBACK.H

header included in hcex build.
*/

#ifndef __RECORDED_ANIMATION_PLAYBACK_H
#define __RECORDED_ANIMATION_PLAYBACK_H
#pragma once

/* ---------- constants */

enum
{
	_control_vector_facing_bit = 0,
	_control_vector_aiming_bit,
	_control_vector_looking_bit,
	NUMBER_OF_CONTROL_VECTORS
};

enum
{
	_playback_nothing = 0,
	_playback_end,
	_playback_animation_state_set,
	_playback_aiming_speed_set,
	_playback_control_flags_set,
	_playback_weapon_index_set,
	_playback_throttle_set,
	_playback_vector_char_difference_set,
	_playback_vector_short_difference_set = _playback_vector_char_difference_set + FLAG(NUMBER_OF_CONTROL_VECTORS),
	NUMBER_OF_PLAYBACK_EVENTS = _playback_vector_short_difference_set + FLAG(NUMBER_OF_CONTROL_VECTORS) + 1
};

enum
{
	_time_delta_zero = 0,
	_time_delta_one,
	_time_delta_byte,
	_time_delta_word,
	NUMBER_OF_TIME_DELTAS
};

/* ---------- macros */

/* ---------- structures */

struct animation_event_header
{
	byte event_time_type:2;
	byte event_type:6;
};

struct animation_state_event_data
{
	char animation_state;
};

struct aiming_speed_event_data
{
	char aiming_speed;
};

struct control_flags_event_data
{
	word control_flags;
};

struct weapon_index_event_data
{
	short weapon_index;
};

struct throttle_event_data
{
	real_vector2d throttle;
};

struct vector_char_difference_data
{
	char delta_yaw;
	char delta_pitch;
};

struct vector_short_difference_data
{
	short delta_yaw;
	short delta_pitch;
};

/* ---------- prototypes/RECORDED_ANIMATION_PLAYBACK.C */

void recorded_animation_initialize_event_stream(struct animation_playback_controller *animation_state, struct unit_control_data *control, char const **playback_stream, byte unit_control_version);
void recorded_animation_initialize_event_stream_with_size(struct animation_playback_controller *animation_state, struct unit_control_data *control, char const **playback_stream);
boolean recorded_animation_apply_event_stream(struct animation_playback_controller *animation_state, struct unit_control_data *control, long *ticks, char const **playback_stream);
void byte_swap_recording_stream(void *data, long size, byte unit_control_version);

/* ---------- globals */

/* ---------- public code */

#endif // __RECORDED_ANIMATION_PLAYBACK_H
