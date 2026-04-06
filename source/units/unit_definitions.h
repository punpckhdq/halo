/*
UNIT_DEFINITIONS.H
*/

#ifndef __UNIT_DEFINITIONS_H
#define __UNIT_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "objects/object_definitions.h"

/* ---------- constants */

enum
{
	UNIT_DEFINITION_TAG= 'unit',
	UNIT_DEFINITION_VERSION= 2,
};

enum
{
	_unit_has_circular_aiming_angles_bit= 0,
	_unit_is_destroyed_after_dying_bit,
	_unit_has_slow_animation_interpolation_bit,
	_unit_fires_from_camera_bit,
	_unit_has_entrance_points_inside_bounding_sphere_bit,
	_unit_unused_bit,
	_unit_causes_passenger_dialogue_bit,
	_unit_resists_pings_bit,
	_unit_melee_attack_is_fatal_bit,
	_unit_does_not_reorient_during_pings_bit,
	_unit_has_no_aiming_bit,
	_unit_simple_creature_bit,
	_unit_impact_melee_attaches_bit,
	_unit_impact_melee_die_on_shield_bit,
	_unit_cannot_open_doors_automatically_bit,
	_unit_melee_attackers_cannot_attach_bit,
	_unit_never_unsuspecting_bit,
	_unit_shield_sapping_bit,
	_unit_runs_around_flaming_bit,
	_unit_is_inconsequential_bit,
	_unit_is_special_bit,
	_unit_ignored_by_autoaiming_bit,
	_unit_shields_fry_infection_forms_bit,
	_unit_integrated_light_controls_weapon_directly_bit,
	_unit_integrated_light_fucking_lasts_forever_bit,
	NUMBER_OF_UNIT_DEFINITION_FLAGS,
};

/* ---------- macros */

#define unit_definition_get(index) ((struct unit_definition *)tag_get(UNIT_DEFINITION_TAG, index))

/* ---------- structures */

struct unit_camera
{
	char marker_name[TAG_STRING_LENGTH+1];
	char submerged_marker_name[TAG_STRING_LENGTH+1];
	real pitch_autolevel;
	real pitch_minimum;
	real pitch_maximum;
	struct tag_block unit_camera_tracks;
};

struct _unit_definition
{
	unsigned long flags;
	short default_team;
	short constant_sound;
	real child_damage_fraction;
	struct tag_reference integrated_light_toggle_effect;
	short function_modes[4];
	real camera_field_of_view;
	real camera_impulse_stiffness;
	struct unit_camera camera;
	real_vector3d acceleration_scale;
	real integrated_light_poweron_time;
	real integrated_light_poweroff_time;
	real integrated_light_lifespan;
	real soft_ping_threshold;
	real soft_ping_minimum_interrupt_time;
	real hard_ping_threshold;
	real hard_ping_minimum_interrupt_time;
	real hard_death_threshold;
	real feign_death_threshold;
	real feign_death_time;
	real evade_distance;
	real dive_distance;
	long movement_unused;
	real stunned_movement_threshold;
	real feign_death_chance;
	real feign_death_repeat_chance;
	struct tag_reference spawned_actor_variant;
	short spawn_actor_lower_bound;
	short spawn_actor_upper_bound;
	real spawn_throw_velocity;
	real aiming_velocity_maximum;
	real aiming_acceleration_maximum;
	real casual_aiming_modifier;
	real looking_velocity_maximum;
	real looking_acceleration_maximum;
	long unused4[2];
	real ai_vehicle_avoidance_radius;
	real ai_danger_radius;
	struct tag_reference melee_damage;
	short blip_type;
	short padxxx;
	long unused[3];
	struct tag_block huds;
	struct tag_block dialogue_variants;
	real grenade_velocity;
	short grenade_type;
	short grenade_count;
	short runtime_soft_ping_minimum_interrupt_ticks;
	short runtime_hard_ping_minimum_interrupt_ticks;
	struct tag_block powered_seats;
	struct tag_block initial_weapons;
	struct tag_block seats;
};

struct unit_definition
{
	struct _object_definition object;
	struct _unit_definition unit;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __UNIT_DEFINITIONS_H
