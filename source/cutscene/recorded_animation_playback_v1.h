/*
RECORDED_ANIMATION_PLAYBACK_V1.H

header included in hcex build.
*/

#ifndef __RECORDED_ANIMATION_PLAYBACK_V1_H
#define __RECORDED_ANIMATION_PLAYBACK_V1_H
#pragma once

/* ---------- constants */

enum
{
	_playback_v1_nothing = 0,
	_playback_v1_end,
	_playback_v1_animation_state_set,
	_playback_v1_aiming_speed_set,
	_playback_v1_control_flags_set,
	_playback_v1_weapon_index_set,
	_playback_v1_throttle_set,
	_playback_v1_vectors_synchronize,
	_playback_v1_vectors_desynchronize,
	_playback_v1_facing_vector_set,
	_playback_v1_aiming_vector_set,
	_playback_v1_looking_vector_set,
	_playback_v1_facing_aiming_vector_set,
	_playback_v1_facing_looking_vector_set,
	_playback_v1_aiming_looking_vector_set,
	_playback_v1_facing_aiming_looking_vector_set,
	_playback_v1_facing_angles_set,
	_playback_v1_aiming_angles_set,
	_playback_v1_looking_angles_set,
	_playback_v1_facing_aiming_angles_set,
	_playback_v1_facing_looking_angles_set,
	_playback_v1_aiming_looking_angles_set,
	_playback_v1_facing_aiming_looking_angles_set,
	NUMBER_OF_PLAYBACK_V1_EVENTS
};

/* ---------- macros */

/* ---------- structures */

struct animation_event_v1
{
	short type;
	word time_delta;
};

struct animation_state_set_event_v1
{
	short type;
	word time_delta;
	char animation_state;
};

struct aiming_speed_set_event_v1
{
	short type;
	word time_delta;
	char aiming_speed;
};

struct control_flags_set_event_v1
{
	short type;
	word time_delta;
	word control_flags;
};

struct weapon_index_set_event_v1
{
	short type;
	word time_delta;
	short weapon_index;
};

struct throttle_set_event_v1
{
	short type;
	word time_delta;
	real_vector2d throttle;
};

struct facing_vector_set_event_v1
{
	short type;
	word time_delta;
	real_vector3d facing_vector;
};

struct aiming_vector_set_event_v1
{
	short type;
	word time_delta;
	real_vector3d aiming_vector;
};

struct looking_vector_set_event_v1
{
	short type;
	word time_delta;
	real_vector3d looking_vector;
};

struct multi_vector_set_event_v1
{
	short type;
	word time_delta;
	real_vector3d vector;
};

struct angle_vector_set_event_v1
{
	short type;
	word time_delta;
	real_euler_angles2d angles;
};

/* ---------- prototypes/RECORDED_ANIMATION_PLAYBACK_V1.C */

void recorded_animation_initialize_event_stream_v1(struct animation_playback_controller *animation_state, struct unit_control_data *control, char const **playback_stream, byte unit_control_version);
boolean recorded_animation_apply_event_stream_v1(struct animation_playback_controller *animation_state, struct unit_control_data *control, long *ticks, char const **playback_stream);
void byte_swap_recording_stream_v1(void *data, long size, byte unit_control_version);

/* ---------- globals */

/* ---------- public code */

#endif // __RECORDED_ANIMATION_PLAYBACK_V1_H
