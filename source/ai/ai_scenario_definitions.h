/*
AI_SCENARIO_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __AI_SCENARIO_DEFINITIONS_H
#define __AI_SCENARIO_DEFINITIONS_H
#pragma once

/* ---------- headers */


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
	_encounter_not_initially_placed_bit = 0,
	_encounter_respawn_enable_bit,
	_encounter_blind_bit,
	_encounter_deaf_bit,
	_encounter_braindead_bit,
	_encounter_3d_firing_positions_bit,
	_encounter_manual_structure_bsp_bit,
	NUMBER_OF_ENCOUNTER_FLAGS,
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

enum
{
	_ai_conversation_stop_if_anyone_dies_bit = 0,
	_ai_conversation_stop_if_damaged_bit,
	_ai_conversation_stop_if_visible_enemy_bit,
	_ai_conversation_stop_if_alerted_to_enemy_bit,
	_ai_conversation_player_must_be_visible_bit,
	_ai_conversation_stop_other_actions_bit,
	_ai_conversation_keep_trying_to_play_bit,
	_ai_conversation_player_must_be_looking_at_bit,
	NUMBER_OF_CONVERSATION_DEFINITION_FLAGS,
};

enum
{
	_ai_conversation_participant_optional_bit = 0,
	_ai_conversation_participant_has_alternate_bit,
	_ai_conversation_participant_is_alternate_bit,
	NUMBER_OF_CONVERSATION_PARTICIPANT_DEFINITION_FLAGS,
};

enum
{
	_ai_conversation_selection_friendly_actor = 0,
	_ai_conversation_selection_disembodied,
	_ai_conversation_selection_in_player_vehicle,
	_ai_conversation_selection_not_in_vehicle,
	_ai_conversation_selection_sargeant,
	_ai_conversation_selection_any_actor,
	_ai_conversation_selection_radio,
	_ai_conversation_selection_radio_sargeant,
	NUMBER_OF_CONVERSATION_SELECTION_TYPES,
};

enum
{
	_ai_conversation_line_addressee_look_back_bit = 0,
	_ai_conversation_line_everyone_look_at_speaker_bit,
	_ai_conversation_line_everyone_look_at_addressee_bit,
	_ai_conversation_line_wait_after_until_told_to_advance_bit,
	_ai_conversation_line_wait_until_speaker_nearby_bit,
	_ai_conversation_line_wait_until_everyone_nearby_bit,
	NUMBER_OF_CONVERSATION_LINE_FLAGS,
};

enum
{
	_ai_conversation_address_none = 0,
	_ai_conversation_address_player,
	_ai_conversation_address_participant,
	NUMBER_OF_CONVERSATION_ADDRESS_TYPES,
};

#define MAXIMUM_CONVERSATIONS_PER_MAP 128
#define MAXIMUM_PARTICIPANTS_PER_CONVERSATION 8
#define MAXIMUM_DIALOGUE_VARIANTS_PER_CONVERSATION_PARTICIPANT 6

/* ---------- macros */

#define ai_conversation_definition_get(index) TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->ai_conversations, (index), struct ai_conversation) /* fake name */

/* ---------- structures */

struct actor_starting_location_definition
{
	real_point3d position;
	real facing;
	short cluster_index;
	char sequence_id;
	byte flags;
	short default_state;
	short initial_state;
	short actor_palette_index;
	short command_list_index;
};

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

struct platoon_rule
{
	short rule_type;
	short platoon_index;
	long pad;
};

struct platoon_definition
{
	char name[TAG_STRING_LENGTH+1];
	unsigned long flags;
	unsigned long unused1[3];
	struct platoon_rule attacking_defending_rule;
	unsigned long unused2;
	struct platoon_rule maneuvering_rule;
	unsigned long unused3;
	unsigned long unused4[16];
	struct tag_block unused_blocks[3];
};

struct firing_position_definition
{
	real_point3d position;
	short group_index;
	short cluster_index;
	long pad;
	long surface_index;
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
	struct tag_block squads;					// struct squad_definition
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

struct ai_conversation_participant
{
	word pad;
	word flags;
	short selection_type;
	short actor_type;
	short preexisting_object_name_index;
	short new_attach_object_name_index;
	unsigned long unused[3];
	short dialogue_variants[MAXIMUM_DIALOGUE_VARIANTS_PER_CONVERSATION_PARTICIPANT];
	char ai_index_name[TAG_STRING_LENGTH+1];
	long runtime_ai_index;
	unsigned long unused2[3];
};

struct ai_conversation_line
{
	word flags;
	short participant_index;
	short address_type;
	short address_participant_index;
	unsigned long unused;
	real delay_time;
	unsigned long unused2[3];
	struct tag_reference dialogue[MAXIMUM_DIALOGUE_VARIANTS_PER_CONVERSATION_PARTICIPANT];
};

struct ai_conversation
{
	char name[TAG_STRING_LENGTH+1];
	word flags;
	word pad;
	real trigger_dist;
	real run_to_player_dist;
	unsigned long unused[9];
	struct tag_block participants;		// ai_conversation_participant
	struct tag_block lines;				// ai_conversation_line
	struct tag_block unused_block;
};

/* ---------- prototypes/AI_SCENARIO_DEFINITIONS.C */

/* ---------- globals */

/* ---------- public code */

#endif // __AI_SCENARIO_DEFINITIONS_H
