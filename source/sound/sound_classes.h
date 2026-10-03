/*
SOUND_CLASSES.H

file has inline function assertions.
*/

#ifndef __SOUND_CLASSES_H
#define __SOUND_CLASSES_H
#pragma once

/* ---------- constants */

enum
{
	_sound_class_projectile_impact = 0,
	_sound_class_projectile_detonation,
	_sound_class_projectile_unused0,
	_sound_class_projectile_unused1,
	_sound_class_weapon_fire,
	_sound_class_weapon_ready,
	_sound_class_weapon_reload,
	_sound_class_weapon_empty,
	_sound_class_weapon_charge,
	_sound_class_weapon_overheat,
	_sound_class_weapon_idle,
	_sound_class_weapon_unused0,
	_sound_class_weapon_unused1,
	_sound_class_object_impacts,
	_sound_class_particle_impacts,
	_sound_class_slow_impacts,
	_sound_class_effect_unused2,
	_sound_class_effect_unused3,
	_sound_class_footstep,
	_sound_class_unit_dialog,
	_sound_class_unit_unused0,
	_sound_class_unit_unused1,
	_sound_class_vehicle_impact,
	_sound_class_vehicle_engine,
	_sound_class_vehicle_unused0,
	_sound_class_vehicle_unused1,
	_sound_class_device_door,
	_sound_class_device_force_field,
	_sound_class_device_machinery,
	_sound_class_device_nature,
	_sound_class_device_computers,
	_sound_class_device_unused1,
	_sound_class_music,
	_sound_class_ambient_nature,
	_sound_class_ambient_machinery,
	_sound_class_ambient_computers,
	_sound_class_marty_unused1,
	_sound_class_marty_unused2,
	_sound_class_marty_unused3,
	_sound_class_player_hurt,
	_sound_class_player_unused0,
	_sound_class_player_unused1,
	_sound_class_player_unused2,
	_sound_class_player_unused3,
	_sound_class_scripted_dialog_to_player,
	_sound_class_scripted_other,
	_sound_class_scripted_dialog_to_other,
	_sound_class_scripted_dialog_force_unspatialized,
	_sound_class_scripted_unused2,
	_sound_class_scripted_unused3,
	_sound_class_game_event,
	NUMBER_OF_SOUND_CLASSES,
};

enum
{
	_sound_cache_miss_mode_discard = 0,
	_sound_cache_miss_mode_postpone,
	NUMBER_OF_SOUND_CACHE_MISS_MODES,
};

enum
{
	MAXIMUM_SOUND_INSTANCES_PER_DEFINITION = 16,
	MAXIMUM_SOUND_INSTANCES_PER_OBJECT_PER_DEFINITION = 16,
};

/* ---------- macros */

/* ---------- structures */

struct sound_class_datum
{
	real desired_gain;
	real gain;
	short ticks;
};

struct sound_class_definition
{
	short maximum_number_per_definition;
	short maximum_number_per_object;
	long preemption_time;
	boolean speech;
	short priority;
	short cache_miss_mode;
	real reverb_damping_factor;
	real effect_damping_factor;
	real minimum_distance;
	real maximum_distance;
	real gain_lower_bound;
	real gain_upper_bound;
	boolean disabled;
};

/* ---------- prototypes/SOUND_CLASSES.C */

struct sound_class_definition *sound_class_get(short class_index);

void sound_classes_initialize(void);
void sound_classes_dispose(void);
void sound_classes_initialize_for_new_map(void);
void sound_classes_dispose_from_old_map(void);
void sound_classes_update(long ticks_elapsed);

real sound_class_get_gain(short class_index);
void sound_class_set_gain(char const *substring, real gain, short ticks);

void debug_sound_classes_enable(char const *substring, boolean enabled);
void debug_sound_classes_set_distances(char const *substring, real minimum_distance, real maximum_distance);
void debug_sound_classes_set_wet(char const *substring, real wet);

/* ---------- globals */

extern struct sound_class_definition sound_classes[NUMBER_OF_SOUND_CLASSES];
extern char const *sound_class_names[NUMBER_OF_SOUND_CLASSES];

/* ---------- public code */

#endif // __SOUND_CLASSES_H
