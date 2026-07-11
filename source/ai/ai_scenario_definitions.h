/*
AI_SCENARIO_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __AI_SCENARIO_DEFINITIONS_H
#define __AI_SCENARIO_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"
#include "tag_files/tag_files.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

enum
{
	_squad_unused_bit = 0,
	_squad_never_search_bit,
	_squad_timer_starts_immediately_bit,
	_squad_delay_forever_bit,
	_squad_magic_sight_after_timer_bit,
	_squad_automatic_migration_bit,
	NUMBER_OF_SQUAD_FLAGS,
};

enum
{
	_firing_position_group_attacking = 0,
	_firing_position_group_attacking_search,
	_firing_position_group_attacking_guard,
	_firing_position_group_defending,
	_firing_position_group_defending_search,
	_firing_position_group_defending_guard,
	_firing_position_group_pursuing,
	NUMBER_OF_FIRING_POSITION_GROUPS,

	MAXIMUM_NUMBER_OF_FIRING_POSITION_GROUPS = 8,
};

enum
{
	_ai_atom_pause = 0,
	_ai_atom_go_to,
	_ai_atom_go_to_and_face,
	_ai_atom_move_direction,
	_ai_atom_look,
	_ai_atom_animation_mode,
	_ai_atom_crouch,
	_ai_atom_shoot,
	_ai_atom_grenade,
	_ai_atom_vehicle,
	_ai_atom_running_jump,
	_ai_atom_targeted_jump,
	_ai_atom_script,
	_ai_atom_animate,
	_ai_atom_recording,
	_ai_atom_action,
	_ai_atom_vocalize,
	_ai_atom_targeting,
	_ai_atom_initiative,
	_ai_atom_wait,
	_ai_atom_loop,
	_ai_atom_die,
	_ai_atom_move_immediate,
	_ai_atom_look_random,
	_ai_atom_look_player,
	_ai_atom_look_object,
	_ai_atom_set_radius,
	_ai_atom_teleport,
	NUMBER_OF_AI_ATOM_TYPES,
};


/* ---------- macros */

/* ---------- structures */

struct squad_definition
{
	char name[TAG_STRING_LENGTH+1];
	short actor_palette_index;
	short platoon_index;
	short initial_state;
	short default_state;
	unsigned long flags;
	short unique_leader_type;
	word pad;
	unsigned long unused1[7];
	word pad5;
	short maneuver_squad_index;
	real squad_delay_timer;
	unsigned long firing_position_groups[MAXIMUM_NUMBER_OF_FIRING_POSITION_GROUPS];
	unsigned long pad2[2];
	short min_count;
	short max_count;
	short major_upgrade;
	word pad3;
	short respawn_min_actors;
	short respawn_max_actors;
	short respawn_total_count;
	word pad4;
	real respawn_time_lower_bound;
	real respawn_time_upper_bound;
	unsigned long unused3[12];
	struct tag_block move_positions;
	struct tag_block starting_locations;
	struct tag_block unused_block;
};

struct encounter_definition
{
	char name[TAG_STRING_LENGTH+1];
	unsigned long flags;
	short team_index;
	short version;
	short searching;
	short manual_structure_bsp_reference_index;
	real respawn_time_lower_bound;
	real respawn_time_upper_bound;
	unsigned long unused[18];
	word pad2;
	short runtime_structure_bsp_reference_index;
	struct tag_block squads;
	struct tag_block platoons;
	struct tag_block firing_positions;
	struct tag_block player_starting_locations;
};

struct ai_command_definition
{
	short atom_type;
	short atom_modifier;
	real parameter1;
	real parameter2;
	short point1_index;
	short point2_index;
	short animation_reference_index;
	short script_reference_index;
	short recording_reference_index;
	short command_index;
	short object_name_index;
	word pad;
	unsigned long unused;
};

struct ai_command_point_definition
{
	real_point3d position;
	long surface_index;
	unsigned long unused;
};

struct ai_command_list_definition
{
	char name[TAG_STRING_LENGTH+1];
	unsigned long flags;
	unsigned long unused[2];
	short manual_structure_bsp_reference_index;
	short runtime_structure_bsp_reference_index;
	struct tag_block commands;			// ai_command_definition
	struct tag_block points;			// ai_command_point_definition
	struct tag_block unused_blocks[2];
};

/* ---------- prototypes/AI_SCENARIO_DEFINITIONS.C */

/* ---------- globals */

/* ---------- public code */

#endif // __AI_SCENARIO_DEFINITIONS_H
