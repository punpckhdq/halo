/*
GAME_SOUND.H

header included in hcex build.
*/

#ifndef __GAME_SOUND_H
#define __GAME_SOUND_H
#pragma once

/* ---------- constants */

enum
{
	_looping_sound_refresh_start = 0,
	_looping_sound_refresh_loop,
	_looping_sound_refresh_stop,
	NUMBER_OF_LOOPING_SOUND_REFRESH_STATES,
};

enum
{
	MAXIMUM_GAME_LOOPING_SOUNDS_PER_MAP = 1024,
};

enum
{
	_game_looping_sound_unattached_bit = 0,
	_game_looping_sound_unattached_stop_bit,
	_game_looping_sound_unattached_stop_fixed_fadeout_bit,
	_game_looping_sound_alternate_bit,
	_game_looping_sound_scripted_bit,
	NUMBER_OF_GAME_LOOPING_SOUND_FLAGS,
};

enum
{
	_game_looping_sound_active = 0,
	_game_looping_sound_deactivating,
	_game_looping_sound_inactive,
	NUMBER_OF_GAME_LOOPING_SOUND_STATES,
};

enum
{
	_sound_spatialization_mode_none = 0,
	_sound_spatialization_mode_absolute,
	_sound_spatialization_mode_relative,
	NUMBER_OF_SOUND_SPATIALIZATION_MODES,
};

/* ---------- macros */

#define game_looping_sound_get(index) ((struct game_looping_sound_datum *)datum_get(game_looping_sound_data, (index)))
#define game_looping_sound_try_and_get(index) ((struct game_looping_sound_datum *)datum_try_and_get(game_looping_sound_data, (index)))

/* ---------- structures */

struct sound_location
{
	real_point3d position;
	real_vector3d forward;
	real_vector3d translational_velocity;
	struct location game_location;
};

struct sound_source
{
	short spatialization_mode;
	real scale;
	real gain;
	struct sound_location location;
	real obstruction;
	real occlusion;
};

struct sound_attachment_data
{
	short function_index;
	short node_index;
	real_point3d position;
	real_vector3d forward;
};

struct game_looping_sound_datum
{
	short identifier;
	short state;
	long flags;
	real scale;
	long definition_index;
	long object_index;
	long last_audible_frame_index;
	struct sound_attachment_data attachment;
};

struct game_sound_global_data
{
	long frame_index;
	long background_loop_index;
};

/* ---------- prototypes/GAME_SOUND.C */

void game_sound_initialize(void);
void game_sound_dispose(void);
void game_sound_initialize_for_new_map(void);
void game_sound_dispose_from_old_map(void);
void game_sound_update(real dt);
void game_sound_clear(void);
void game_sound_restore(void);
long game_looping_sound_new(long object_index, long definition_index, char const *marker_name, short function_index);
void game_looping_sound_delete(long looping_sound_index);
long object_impulse_sound_new(
	long object_index,
	long definition_index,
	short node_index,
	real_point3d const *position,
	real_vector3d const *forward,
	real scale);
long unattached_impulse_sound_new(long definition_index, struct sound_location const *location, real scale);
long unspatialized_impulse_sound_new(long definition_index, real scale);
long unattached_looping_sound_start(long definition_index, long source_object_index, real scale);
void unattached_looping_sound_stop(long looping_sound_index);
void scripted_sound_new(long definition_index, long source_object_index, real scale);
long scripted_sound_time(long definition_index);
void scripted_sound_stop(long definition_index);
void scripted_foley_predict(long definition_index);
void scripted_looping_sound_start(long definition_index, long source_object_index, real scale);
void scripted_looping_sound_stop(long definition_index);
void scripted_looping_sound_set_scale(long definition_index, real scale);
void scripted_looping_sound_set_alternate(long definition_index, boolean alternate);
boolean track_object_impulse_sound(long object_index, struct sound_attachment_data const *attachment_data, struct sound_source *source);
void game_sound_set_mouth_aperture(long object_index, real mouth_aperture);
void compute_sound_obstruction(short local_player_index, struct sound_source *source, real distance);

/* ---------- globals */

extern unsigned long combined_pas[];
extern struct game_sound_global_data *game_sound_globals;
extern struct data_array *game_looping_sound_data;

/* ---------- public code */

#endif // __GAME_SOUND_H
