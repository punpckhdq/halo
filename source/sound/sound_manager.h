/*
SOUND_MANAGER.H

header included in hcex build.
*/

#ifndef __SOUND_MANAGER_H
#define __SOUND_MANAGER_H
#pragma once

/* ---------- headers */

#include "sound_environment_definitions.h"

/* ---------- constants */

enum
{
	MAXIMUM_SOUND_CHANNELS = 256,
	MAXIMUM_LOOPING_SOUNDS_PER_MAP = 128,
	MAXIMUM_SOUNDS_PER_MAP = 512,
	MAXIMUM_SOUND_CALLBACK_DATA = 48,
};

enum
{
	_sound_impulse = 0,
	_sound_start_track,
	_sound_loop_track,
	_sound_stopping_track,
	_sound_stop_track,
	NUMBER_OF_SOUND_TYPES,
};

enum
{
	_sound_delayed_bit = 0,
	_sound_cached_bit,
	_sound_inaudible_bit,
	_sound_waiting_for_cache_bit,
	NUMBER_OF_SOUND_FLAGS,
};

enum
{
	_sound_fade_mode_linear = 0,
	_sound_fade_mode_crossfade,
	NUMBER_OF_SOUND_FADE_MODES,
};

enum
{
	_sound_promotion_dont = 0,
	_sound_promotion_do,
	_sound_promotion_dont_play,
	NUMBER_OF_SOUND_PROMOTION_RESULTS,
};

enum
{
	_global_sound_water_in = 0,
	_global_sound_water_out,
	NUMBER_OF_GLOBAL_SOUNDS,
};

enum
{
	_sound_channel_idle = 0,
	_sound_channel_playing,
	_sound_channel_full,
	NUMBER_OF_SOUND_CHANNEL_STATES,
};

/* ---------- macros */


/* ---------- structures */

struct sound_listener
{
	boolean valid;
	boolean underwater;
	real_matrix4x3 matrix;
	real_vector3d velocity;
};

struct platform_sound_listener_properties
{
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	real_vector3d translational_velocity;
	struct sound_environment const *sound_environment; /* fake name */
};

struct platform_sound_channel_properties
{
	real minimum_distance;
	real maximum_distance;
	real pitch;
	real gain;
	real inner_cone_angle;
	real outer_cone_angle;
	real outer_cone_gain;
	real reverb_damping_factor;
};

struct platform_sound_manager_definition
{
	short platform_code;
	boolean (*initialize)(struct sound_preferences *preferences);
	void (*dispose)(void);
	void (*set_listener_properties)(struct platform_sound_listener_properties const *properties);
	void (*begin_scene)(void);
	void (*end_scene)(void);
	void (*queue_sound_to_channel)(short channel_index, struct sound_permutation *permutation);
	void (*channel_update)(short channel_index);
	void (*stop_channel)(short channel_index);
	short (*get_channel_state)(short channel_index);
	void (*set_pause)(boolean paused);
	void (*flush)(void);
	void (*set_channel_location)(short channel_index, boolean spatialized, struct sound_location const *location, real obstruction, real occlusion, boolean underwater);
	void (*set_channel_properties)(short channel_index, struct platform_sound_channel_properties const *properties, boolean gain_only);
};

struct sound_channel_datum
{
	long sound_index;
	short type_flags;
	real estimated_tick_time;
	real pitch;
	struct sound_permutation *playing_permutation;
	struct sound_permutation *queued_permutation;
};


/* ---------- prototypes/SOUND_MANAGER.C */

boolean sound_valid_for_channel(short compression, short encoding, short sample_rate, short spatialization_mode, short channel_type_flags);
struct platform_sound_manager_definition *current_platform_definition(void);
void sound_initialize(void);
void sound_initialize_for_new_map(void);
void sound_dispose_from_old_map(void);
void sound_dispose(void);
boolean sound_is_active(void);
void sound_pause(boolean paused);
void sound_render(void);
void sound_idle(void);
long sound_render_time(void);
void sound_reconnect_to_structure_bsp(void);
boolean sound_try_and_get(long sound_index);
void sound_stop_impulse_by_source_and_definition(long source_identifier, long definition_index);
void sound_enable(boolean enabled);
boolean sound_scripted_dialog_is_playing(void);
void sound_stop_all(void);
long sound_new_impulse(long definition_index, struct sound_source *source, long source_identifier, boolean (*track_proc)(long, void const *, struct sound_source *), void const *track_data, short track_data_size);
void sound_stop_impulse(long sound_index);
boolean sound_refresh_looping(long definition_index, long identifier, struct sound_source *source, short refresh_state, boolean alternate, real force_stop_time);
void sound_manager_set_sound_environment(struct sound_environment const *environment);

/* ---------- globals */

extern real sound_gain_under_dialog;
extern struct platform_sound_manager_definition *platform_definitions[];
extern real sound_fade_exponent;
extern boolean debug_looping_sound;
extern boolean debug_sound;
extern boolean loud_dialog_hack;
extern struct sound_channel_datum sound_channels[MAXIMUM_SOUND_CHANNELS];
extern struct data_array *looping_sound_data;
extern struct data_array *sound_data;

/* ---------- public code */

#endif // __SOUND_MANAGER_H
