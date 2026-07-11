/*
PROPS.H

header included in hcex build.
*/

#ifndef __PROPS_H
#define __PROPS_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"
#include "objects/objects.h"

/* ---------- constants */

enum
{
	_prop_state_unacknowledged = 0,
	_prop_state_becoming_acknowledged,
	_prop_state_becoming_unacknowledged,
	_prop_state_acknowledged,
	_prop_state_uninspected_orphan,
	_prop_state_inspected_orphan,
	NUMBER_OF_PROP_STATES,
};

enum
{
	_prop_lighting_darkness = 0,
	_prop_lighting_dim,
	_prop_lighting_bright,
	NUMBER_OF_PROP_LIGHTING_STATES,
};


/* ---------- macros */

#define prop_get(index)			((struct prop_datum *)datum_get(prop_data, (index)))
#define prop_try_and_get(index)	((struct prop_datum *)datum_try_and_get(prop_data, (index)))

/* ---------- structures */

struct prop_datum
{
	short identifier;
	long owner_actor_index;
	long next_prop_index;

	union
	{
		long orphan_prop_index;
		long parent_prop_index;
	};

	short type;
	short team_index;
	boolean swarm;
	long unit_index;
	long actor_index;
	real suicide_radius;
	short state;
	short timer;
	long swarm_unit_selected_time;
	real awareness;
	short perception;
	short visibility;
	short audibility;
	short ineffability;
	short line_of_sight;
	short orphan_lifespan_ticks;
	short orphan_inspection_ticks;
	real_vector3d orphan_hint_vector;
	short ticks_until_orphan;
	boolean orphan_corpse_cheated;
	real target_weight;
	real look_interest;
	real last_idle_look_interest;
	long last_idle_look_time;
	boolean enemy;
	boolean ally;
	boolean ally_status_changed;
	boolean in_use;
	boolean refresh_stimuli;
	short unit_effect;
	short unit_effect_decay_ticks;
	short required_ticks;
	short ticks_since_damage;
	real damage_inflicted_on_me;
	boolean currently_damaging_me;
	short dead_ticks;
	short visible_ticks;
	long last_perceived_time;
	real_point3d last_perceived_body_position;
	long last_visible_time;
	real_point3d last_visible_head_position;
	short unreachable_ticks;
	long last_unreachable_time;
	boolean unopposable_enemy;
	short unopposable_casualties_inflicted;
	short unopposable_casualty_decay_timer;
	short unopposable_trigger_hysteresis;
	short unopposable_trigger_timer;
	short unopposable_trigger_threshold;
	short ticks_since_definitely_located;
	long definite_knowledge_source_actor;
	boolean definitely_located;
	boolean tried_to_uncover;
	boolean tried_to_search;
	boolean abandoned_search;
	real_point3d body_position;
	real_point3d center_of_mass;
	real_vector3d velocity;
	real_vector3d actor_to_prop;
	long pathfinding_surface_index;
	real_point3d pathfinding_point;
	struct location body_location;
	real_point3d head_position;
	long vehicle_index;
	long attached_to_unit_index;
	boolean underwater;
	real distance;
	char lighting;
	char quantized_distance;
	char quantized_facing;
	char quantized_speed;
	char quantized_closing_speed;
	char child_units_attached;
	boolean delay_requirement_decision;
	boolean dead;
	boolean really_dead;
	boolean just_killed;
	boolean just_became_visible;
	boolean noncombat;
	boolean in_combat;
	boolean fighting;
	boolean player;
	boolean shooting;
	boolean flying;
	boolean active_camouflage;
	boolean flashlight;
	boolean ignore;
	boolean preferred_target;
	boolean vehicle_gunner;
	boolean dangerous_vehicle_driver;
};

struct prop_iterator
{
	long index;
	long next_index;
};

/* ---------- prototypes/PROPS.C */

void prop_iterator_new(struct prop_iterator *iterator, long actor_index);
struct prop_datum *prop_iterator_next(struct prop_iterator *iterator);

/* ---------- globals */

extern struct data_array *prop_data;

/* ---------- public code */

#endif // __PROPS_H
