/*
ACTIONS.H

header included in hcex build.
*/

#ifndef __ACTIONS_H
#define __ACTIONS_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"

/* ---------- constants */

enum
{
	_obey_metadata_targeting_bit = 0,
	_obey_metadata_commands_finished_bit,
	_obey_metadata_action_changed_bit,
	_obey_metadata_told_to_advance_bit,
	_obey_metadata_waiting_for_advance_notification_bit,
	NUMBER_OF_OBEY_METADATA_FLAGS,
};

enum
{
	_pursuit_location_target = 0,
	_pursuit_location_position,
	_pursuit_location_undirected,
};

enum
{
	_charge_goal_close_range = 0,
	_charge_goal_stalking,
	_charge_goal_melee,
	_charge_goal_melee_leaping,
	_charge_goal_vehicle_strafing,
	_charge_goal_vehicle_ramming,
	NUMBER_OF_CHARGE_GOALS,
};


#define ACTOR_MAXIMUM_AVOIDANCE_RAYS 9

/* ---------- macros */

/* ---------- structures */

struct action_specification
{
	long action;
	const char *name;
	real_argb_color const *const *color;
	unsigned long data_size;
	short action_class;
	void(*begin)(long);
	boolean(*perform)(long);
	void(*update)(long);
	void(*control)(long);
	void(*end)(long);
	void(*modify_color)(long, real_argb_color *);
	void(*replace_prop)(long, long, long);
	void(*flush_position_indices)(long);
	void(*flush_structure_indices)(long);
};

struct sleep_state_data
{
	word pad;
};

struct move_position_definition
{
	real_point3d position;
	real facing;
	real weight;
	real time_lower_bound;
	real time_upper_bound;
	short animation_reference_index;
	char sequence_id;
	byte pad2;
	long unused[2];
	short cluster_index;
	word pad;
	long unused2[8];
	long surface_index;
};

struct alert_state_data
{
	short move_position_order;
	short wait_ticks;
	boolean move_position_increasing;
	short target_move_position_index;
	short pending_move_position_index;
	boolean must_play_animation;
	struct move_position_definition target_position_definition;
};

struct fight_state_data
{
	short firing_position_timer;
	word pad;
};

struct __declspec(align(4)) guard_state_data
{
	short wait_ticks;
	short look_ticks;
	boolean path_begun;
	boolean post_combat;
	boolean post_combat_vocalized;
	boolean post_combat_shooting;
	boolean cower;
	boolean cower_panicked;
	boolean cower_from_retreat;
	short cower_ticks;
	boolean find_new_guard_position;
	boolean shout_about_dead_friend;
	long shout_dead_friend_prop_index;
	boolean has_guard_direction;
	boolean aim_in_guard_direction;
	real_vector3d guard_direction;
	short guard_location_type;

	union
	{
		short guard_firing_position_index;
		struct
		{
			real_point3d position;
			long surface_index;
			real radius;
		} guard_point;
	};

	long guard_look_prop_index;
	boolean guard_look_until_reached_point;
};

struct flee_state_data
{
	short forced_flee_ticks;
	short flee_stationary_ticks;
	boolean flee_from_last_visible_location;
	boolean allow_occluded_points;
	boolean find_new_flee_position;
	short flee_firing_position_index;
	boolean flee_firing_position_found_randomly;
	short panic_type;
	boolean unable_to_flee;
	boolean done_fleeing;
	boolean communicated_flee;
	long last_communication_time;
	long flee_timer;
	long flee_prop_index;
	boolean has_approach_point;
	real_point3d approach_point;
};

struct pursuit_location
{
	short type;
	short firing_position_index;
	short cluster_index;
	long surface_index;
	real_point3d position;
};

struct uncover_state_data
{
	boolean sneaking;
	boolean uncover_done;
	boolean uncover_exit_failure;
	boolean able_to_search;
	boolean no_target_sight_available;
	struct pursuit_location pursuit_location;
	boolean pursuit_location_inspected;
	long current_position_hold_timer;
	long uncover_desired_time;
	long uncover_remaining_time;
	long uncover_exit_persistent_timer;
};

struct search_state_data
{
	boolean search_done;
	boolean search_failed;
	boolean at_destination;
	boolean sneaking;
	boolean tenacious;
	boolean charging;
	struct pursuit_location pursuit_location;
	long search_desired_time;
	long search_remaining_time;
	long search_failure_timer;
};

struct wait_state_data
{
	boolean wait_done;
	boolean waiting_as_coordinator;
	boolean was_actively_searching;
	boolean desire_move;
	boolean move_failed;
	long entry_time;
	short look_timer;
	short exit_timer;
	short query_timer;
	long coordinator_prop_index;
};

struct vehicle_state_data
{
	long vehicle_index;
	short seat_index;
	boolean impromptu;
	boolean ignore_hint;
	boolean started_entry;
	boolean vehicle_entry_done;
	boolean vehicle_entry_failed;
	short pathfinding_failures;
	short stuck_detection_counter;
	long stuck_detection_time;
	real_point3d stuck_detection_point;
	real attempt_distance;
	real continue_distance;
	boolean currently_within_range;
	boolean currently_correct_facing;
	short fake_entry_potential_timer;
	boolean lock_facing;
	real_point3d destination_point;
	real_vector3d destination_facing;
	long destination_surface_index;
};

struct charge_state_data
{
	long charge_start_time;
	short goal;
	boolean launched_melee_attack;
	boolean finished_melee_attack;
	boolean aborted_melee_attack;
	boolean alignment_incorrect;
	boolean leap_possible_if_at_range;
	boolean launched_leap;
	boolean leap_pending;
	short leap_failure_timer;
	long leap_start_time;
	real_vector2d leap_alignment_vector;
	real leap_horizontal_velocity;
	real leap_vertical_velocity;
	boolean stalking_currently_exposed;
	boolean stalking_catch_target;
	short stalking_discovery_timer;
	boolean advancing;
	boolean unable_to_advance;
	real acceptable_target_range;
	boolean melee_suicide;
	short melee_ticks_until_dangerous;
	real melee_danger_range;
};

struct obey_individual_directmovement_control
{
	short facing;
	real_vector3d vector;
	real_point3d start_position;
};

struct obey_individual_jump_control
{
	short delay_ticks;
	real target_horizontal_vel;
	real target_vertical_vel;
};

struct obey_individual_simple_control
{
	byte current_command_index;
	byte loop_counter;
	short pause_timer;
	byte metadata_flags;
	byte simple_control_flags;
	
	union 
	{
		struct obey_individual_directmovement_control directmovement;
		struct obey_individual_jump_control jump;
	};
};

struct obey_individual_complex_control
{
	boolean override_crouch;
	short override_movement_type;
	boolean destination_valid;
	boolean destination_keep_moving;
	real_point3d destination_point;
	long destination_surface_index;
	boolean destination_facing;
	real_point3d destination_facing_point;
	boolean destination_radius_valid;
	real destination_radius;
	boolean play_action;
	short action_animation_impulse;
	short action_communication_type;
	boolean shoot_at_target;
	real_point3d shoot_target;
	real shoot_burst_length;
	boolean grenade_throw_depress_trigger;
	boolean grenade_throw_started;
	short grenade_throw_trajectory_type;
	real_point3d grenade_target;
};


struct obey_state_data
{
	short command_list_index;
	boolean initiative;
	boolean allow_looking;
	boolean allow_communication;
	boolean finished;
	struct obey_individual_simple_control simple_control;
	struct obey_individual_complex_control complex_control;
};

struct converse_state_data
{
	long conversation_index;
	boolean failed;
	boolean in_range;
	real run_to_distance;
	long run_to_unit_index;
	long run_to_prop_index;
};

struct avoid_state_data
{
	long pad;
};

struct action_state_data
{
	union
	{
		struct sleep_state_data sleep;
		struct alert_state_data alert;
		struct fight_state_data fight;
		struct guard_state_data guard;
		struct flee_state_data flee;
		struct uncover_state_data uncover;
		struct search_state_data search;
		struct wait_state_data wait;
		struct vehicle_state_data vehicle;
		struct charge_state_data charge;
		struct obey_state_data obey;
		struct converse_state_data converse;
		struct avoid_state_data avoid;
	};
};

/* ---------- prototypes/ACTIONS.C */

struct pursuit_location *actor_get_pursuit_location(long actor_index);

real_argb_color *actor_action_debug_color(long actor_index);

/* ---------- prototypes/ACTION_OBEY.C */

void action_obey_describe_command(struct scenario *scenario, struct ai_command_definition *command, char *string, long string_size);

/* ---------- globals */

/* ---------- public code */

#endif // __ACTIONS_H
