/*
ACTORS.H

header included in hcex build.
*/

#ifndef __ACTORS_H
#define __ACTORS_H
#pragma once

/* ---------- headers */

#include "actions.h"
#include "path.h"

#include "math/real_math.h"
#include "objects/objects.h"

/* ---------- constants */

enum
{
	NUMBER_OF_DISCARDED_FIRING_POSITIONS_PER_ACTOR = 4,
};

enum
{
	_actor_target_none = 0,
	_actor_target_partial_enemy,
	_actor_target_dead_enemy,
	_actor_target_disregarded_orphan,
	_actor_target_inspected_orphan,
	_actor_target_uninspected_orphan,
	_actor_target_definite_orphan,
	_actor_target_acknowledged_enemy,
	_actor_target_clear_line_of_sight_enemy,
	_actor_target_potentially_dangerous_enemy,
	_actor_target_visible_enemy,
	_actor_target_damaging_enemy,
	NUMBER_OF_ACTOR_TARGET_TYPES,
};

enum
{
	_primary_priority_none = 0,
	_primary_priority_facing,
	_primary_priority_exact_facing,
	_primary_priority_aiming,
	_primary_priority_locked_aiming,
	NUMBER_OF_PRIMARY_LOOK_TYPES,
};

enum
{
	_secondary_look_none = 0,
	_secondary_look_environmental_noise,
	_secondary_look_started_moving_prop,
	_secondary_look_weapon_impact,
	_secondary_look_newly_acknowledged_prop,
	_secondary_look_bumped_prop,
	_secondary_look_detonation,
	_secondary_look_shooting_prop,
	_secondary_look_communicating_prop,
	_secondary_look_communicated_direction,
	_secondary_look_combat_stimulus_prop,
	_secondary_look_damage,
	_secondary_look_dangerous_object,
	_secondary_look_scripted,
	NUMBER_OF_SECONDARY_LOOK_TYPES,
};

enum
{
	_secondary_look_priority_none = 0,
	_secondary_look_priority_default,
	_secondary_look_priority_idle_look,
	_secondary_look_priority_idle_aim,
	_secondary_look_priority_aim,
	_secondary_look_priority_turn_and_aim,
	_secondary_look_priority_stop_and_aim,
	_secondary_look_priority_override,
	_secondary_look_priority_override_exact_facing,
	NUMBER_OF_SECONDARY_LOOK_PRIORITIES,
};

enum
{
	_actor_threat_none = 0,
	_actor_threat_visible,
	_actor_threat_visible_facing_me,
	_actor_threat_visible_aiming_at_me,
	_actor_threat_shooting,
	_actor_threat_shooting_near_me,
	_actor_threat_shooting_at_me,
	_actor_threat_extremely_close_to_me,
	_actor_threat_damaging_me,
	NUMBER_OF_ACTOR_THREAT_TYPES,
};

enum
{
	_actor_action_none = 0,
	_actor_action_sleep,
	_actor_action_alert,
	_actor_action_fight,
	_actor_action_flee,
	_actor_action_uncover,
	_actor_action_guard,
	_actor_action_search,
	_actor_action_wait,
	_actor_action_vehicle,
	_actor_action_charge,
	_actor_action_obey,
	_actor_action_converse,
	_actor_action_avoid,
	NUMBER_OF_ACTOR_ACTIONS,
};

enum
{
	_actor_postcombat_none = 0,
	_actor_postcombat_speak_alone,
	_actor_postcombat_speak_unscathed,
	_actor_postcombat_speak_wounded,
	_actor_postcombat_speak_massacre,
	_actor_postcombat_speak_triumph,
	_actor_postcombat_run_to,
	_actor_postcombat_check_enemy,
	_actor_postcombat_check_friend,
	_actor_postcombat_shoot_corpse,
	_actor_postcombat_celebrate,
	NUMBER_OF_ACTOR_POSTCOMBAT_ACTIONS,
};

enum
{
	_actor_fire_state_none = 0,
	_actor_fire_state_holding,
	_actor_fire_state_bursting,
	_actor_fire_state_pausing,
	_actor_fire_state_wild,
	NUMBER_OF_ACTOR_FIRE_STATES,
};

enum
{
	_actor_vector_avoidance_clear = 0,
	_actor_vector_avoidance_obstructed_object,
	_actor_vector_avoidance_obstructed_structure,
	NUMBER_OF_ACTOR_VEHICLE_AVOIDANCE_RESULTS,
};

enum
{
	_direction_specification_movement = 0,
	_direction_specification_prop,
	_direction_specification_target,
	_direction_specification_point,
	_direction_specification_vector,
	_direction_specification_danger,
	_direction_specification_object,
	NUMBER_OF_DIRECTION_SPECIFICATION_TYPES,
};

enum
{
	_actor_unopposable_danger_none = 0,
	_actor_unopposable_danger_visible,
	_actor_unopposable_danger_shooting,
	_actor_unopposable_danger_shooting_nearby,
	_actor_unopposable_danger_damaging,
	_actor_unopposable_danger_retreat,
	_actor_unopposable_danger_friends_retreating,
	_actor_unopposable_danger_time_triggered,
	_actor_unopposable_danger_immediately_triggered,
	_actor_unopposable_danger_friends_killed,
	NUMBER_OF_ACTOR_UNOPPOSABLE_DANGER_TYPES,
};

enum
{
	_actor_movement_type_alert = 0,
	_actor_movement_type_asleep,
	_actor_movement_type_combat,
	_actor_movement_type_panic,
	_actor_movement_type_flaming,
	NUMBER_OF_ACTOR_MOVEMENT_TYPES,
};


enum
{
	_actor_perception_none = 0,
	_actor_perception_partial,
	_actor_perception_full,
	_actor_perception_unmistakable,
	NUMBER_OF_ACTOR_PERCEPTION_TYPES,
};

#define MAXIMUM_NUMBER_OF_ACTORS 256
#define MAXIMUM_NUMBER_OF_ACTOR_PATHS 32

#define MAXIMUM_UNIT_INDICES_PER_SWARM 16
#define MAXIMUM_COMPONENT_INDICES_PER_SWARM 16

#define MAXIMUM_NUMBER_OF_AVOIDANCE_OBJECTS 1024

/* ---------- macros */

#define actor_get(index)			((struct actor_datum *)datum_get(actor_data, (index)))
#define actor_try_and_get(index)	((struct actor_datum *)datum_try_and_get(actor_data, (index)))

#define swarm_get(index)			((struct swarm_datum *)datum_get(swarm_data, (index)))
#define swarm_try_and_get(index)	((struct swarm_datum *)datum_try_and_get(swarm_data, (index)))

/* ---------- structures */

struct actor_meta_data
{
	short type;
	boolean swarm;
	boolean frozen;
	boolean active;
	boolean encounterless;
	boolean force_active;
	boolean erase_next_tick;
	long last_active_time;
	short encounterless_active_timer;
	boolean dormant_desire;
	boolean dormant;
	short become_dormant_timer;
	long unit_index;
	boolean unique_leader;
	short swarm_unit_count;
	short swarm_original_unit_count;
	long swarm_unit_index;
	long swarm_cache_index;
	long next_actor_index;
	long disconnected_encounter_index;
	long encounter_index;
	short disconnected_squad_index;
	short squad_index;
	short platoon_index;
	short team_index;
	boolean stored_prevehicle_encounter;
	long prevehicle_encounter_index;
	short prevehicle_squad_index;
	short service_timer;
	byte timeslice;
	short highest_prop_timer;
	long first_prop_index;
	long interesting_orphan_index;
	long definition_index;
	long variant_definition_index;
};

struct actor_state_data
{
	short initial_state;
	short default_state;
	long last_default_state_time;
	char noncombat_sequence_id;
	short mode;
	short action;
	short combat_status;
	boolean action_changed;
	short artificial_combat_status;
	short suspicion_combat_status;
	long suspicion_timer;
	long combat_mode_timer;
	long in_combat_timer;
	long certain_combat_timer;
	long uncertain_combat_timer;
	boolean had_visible_enemy;
	boolean been_in_combat;
	boolean command_list_immediate;
	short command_list_index;
	short command_list_delay_timer;
	long last_command_list_time;
	boolean searching;
	boolean flying;
	struct action_state_data action_data;
};

struct actor_position_data
{
	real_point3d head_position;
	real_point3d body_position;
	real_vector3d body_facing;
	struct location body_location;
	real_vector3d velocity;
};

struct actor_input_data
{
	struct actor_position_data position;
	long vehicle_index;
	boolean in_midair;
	boolean underwater;
	short vehicle_driver_type;
	boolean vehicle_passenger;
	boolean vehicle_gunner;
	boolean vehicle_gunner_bombardment;
	long pathfinding_surface_index;
	real_point3d pathfinding_point;
	real_vector3d facing_vector;
	real_vector3d aiming_vector;
	real_vector3d looking_vector;
	real_vector3d looking_left_vector;
	real_vector3d looking_up_vector;
	long delayed_attached_projectile_index;
	boolean melee_attacker_attached;
	boolean burning_to_death;
	real body_vitality;
	real shield_vitality;
	real recent_body_damage;
	real recent_shield_damage;
};

struct actor_external_orders
{
	boolean stand_down;
	boolean defending;
	boolean playfighting;
	boolean disable_charging;
	boolean pursuit_is_coordinator;
	long pursuit_group_prop_index;
	short desired_target_type;
	long desired_target_ai_index;
	long conversation_index;
	long conversation_attention_unit_index;
	short postcombat_type;
	long postcombat_prop_index;
};

struct actor_situation
{
	char known_enemies;
	char visible_reachable_enemies;
	char specific_threats[9];
	char cumulative_threats[9];
	char area_friends;
	char area_fighting_friends;
	char area_fire_support_friends;
	char area_friends_by_type[16];
	char area_fighting_friends_by_type[16];
	char visible_friends;
	char visible_fighting_friends;
	char visible_friends_by_type[16];
	char visible_fighting_friends_by_type[16];
	char close_friends;
	char close_fighting_friends;
	char close_friends_by_type[16];
	char close_fighting_friends_by_type[16];
};

struct actor_target_data
{
	short target_type;
	short artificial_target_knowledge;
	long target_last_visible_time;
	long target_prop_index;
	boolean any_target_ever;
	long since_any_target_visible_timer;
	boolean target_really_alive;
};

struct actor_danger_projectile
{
	short time_until_explosion;
};

struct actor_danger_vehicle
{
	word pad;
};

struct actor_danger_suicide
{
	short time_until_death;
};

struct actor_danger_zone
{
	short danger_type;
	short hostility;
	short acknowledgement_timer;
	boolean currently_perceived;
	boolean noticed_danger;
	boolean allow_dive_evasion;
	boolean communicated;
	boolean attached_to_us;
	long object_index;
	long owner_unit_index;
	real danger_radius;
	real_point3d initial_position;
	real_vector3d initial_velocity;
	real_point3d position;
	real_vector3d velocity;
	real_point3d predict_danger_position;
	real current_distance_from_actor;
	real bounding_sphere_radius;
	real_point3d bounding_sphere_center;

	union
	{
		struct actor_danger_projectile projectile;
		struct actor_danger_vehicle vehicle;
		struct actor_danger_suicide suicide;
	};
};

struct actor_stimulus_data
{
	boolean was_damaged;
	boolean vehicle_eviction;
	short surprise_level;
	boolean was_surprised;
	long surprise_prop_index;
	boolean surprise_has_vector;
	real_vector3d surprise_vector;
	short panic_type;
	long panic_prop_index;
	short berserk_type;
	short combat_transition;
	boolean combat_transition_guard_at_point;
	real_point3d combat_transition_guard_point;
	long combat_transition_guard_point_surface_index;
	real combat_transition_guard_point_distance;
	boolean combat_transition_has_vector;
	real_vector3d combat_transition_vector;
	long combat_transition_guard_timer;
	long combat_transition_prop_index;
	long combat_transition_prop_look_timer;
	boolean combat_transition_prop_look_while_moving;
	short suspicion_combat_status;
	long suspicion_timer;
};

struct actor_emotion_data
{
	real instantaneous_danger;
	real perceived_danger;
	boolean defensive_crouch;
	short defensive_crouch_timer;
	boolean crouch_blocking_player_line_of_fire;
	boolean crouch_blocking_line_of_fire;
	boolean crouch_friends_in_line_of_fire;
	boolean moving_into_player_line_of_fire;
	short moving_into_fire_timer;
	boolean crouch_switching_active;
	boolean crouch_switching_current_value;
	short crouch_switching_change_timer;
	short crouch_switching_stay_timer;
	short evasion_delay_timer;
	long last_defensive_cover_seeking_time;
	long last_active_cover_seeking_time;
	boolean currently_defending;
	boolean forced_to_charge;
	boolean ignorant_of_broken_surfaces;
	boolean sighted_friendly_player;
	boolean berserk;
	boolean played_berserk_sound;
	long last_melee_check_time;
	long last_melee_attack_time;
	long last_vehicle_check_time;
	long last_vehicle_charge_time;
	boolean last_vehicle_exit_forced;
	long vehicle_ignore_index;
	long vehicle_ignore_time;
	long last_flee_failed_time;
	long flee_with_friends_disable_time;
	long corpse_ignore_time;
	long unopposable_friend_ignore_time;
	short unopposable_retreat_timer;
	long unopposable_retreat_prop_index;
	long unopposable_retreat_start_time;
	real original_body_vitality;
};

struct actor_discarded_firing_position
{
	byte temporary;
	byte pad;
	short index;
};

struct actor_firing_position_data
{
	short current_position_index;
	boolean current_position_found_outside_range;
	boolean moved_away_from_firing_position;
	boolean pursuit_fired_at_orphan;
	boolean pursuit_communicated_lost_contact;
	long pursuit_prop_index;
	short pursuit_positions_count;
	short next_discarded_firing_positions_entry;
	struct actor_discarded_firing_position discarded_firing_positions[NUMBER_OF_DISCARDED_FIRING_POSITIONS_PER_ACTOR];
	boolean last_discarded_firing_position_valid;
	boolean last_discarded_firing_position_temporary;
	real_point3d last_discarded_firing_position;
};

struct direction_specification
{
	short type;
	union
	{
		int prop_index;
		int object_index;
		real_point3d point;
		real_vector3d vector;
	};
};

struct actor_look_orders
{
	short primary_priority;
	struct direction_specification primary_direction;
	short idle_look_type;
};

struct actor_destination_orders
{
	short destination_type;
	boolean keep_moving;
	byte pad;
	
	union
	{
		struct
		{
			real_point3d point;
			long surface_index;
		} raw;
		
		short firing_position_index;
		short move_position_index;
		
		struct
		{
			int prop_index;
			real accept_radius;
		} prop;
	};

	long ignore_target_object_index;
};

struct actor_animation_orders
{
	short impulse;
	real_vector2d alignment_vector;
};

struct actor_move_orders
{
	struct actor_destination_orders destination;
	struct actor_animation_orders animation;
	boolean dive_into_cover;
	boolean emerge_from_cover;
	boolean stationary_crouch;
	boolean moving_crouch;
	boolean panicked;
	boolean flaming;
	boolean move_face_exactly;
	short override_movement_type;
	short override_movement_facing;
	boolean override_movement_direction;
	real_vector3d override_movement_direction_vector;
	boolean jump;
	boolean jump_leap;
	boolean jump_targeted;
	real_vector2d jump_alignment_vector;
	real jump_target_horizontal_vel;
	real jump_target_vertical_vel;
};

struct actor_combat_orders
{
	boolean shoot_at_target;
	boolean bombard_target;
	boolean abort_burst;
	boolean override_firing_restrictions;
	real override_burst_duration;
	boolean throw_grenade;
	boolean use_manual_target_point;
	real_point3d target_point;
};

struct actor_orders
{
	struct actor_look_orders look;
	struct actor_move_orders move;
	struct actor_combat_orders combat;
};

struct actor_path_control_data
{
	struct actor_destination_orders destination_orders;
	boolean at_destination;
	struct path_destination destination;
	real destination_distance;
	real destination_original_distance;
	boolean refreshed_this_tick;
	struct path_result path;
};

struct actor_control_data
{
	struct actor_path_control_data path;
	boolean moving;
	boolean moving_forced_by_aiming;
	boolean movement_complete;
	boolean movement_thwarted;
	boolean crouching;
	short moving_facing_direction;
	real_point3d moving_towards_point;
	real_vector3d moving_towards_vector;
	real_vector3d moving_forced_aim_direction;
	boolean jumping_targeted;
	real_vector2d jump_alignment_vector;
	real jump_target_horizontal_vel;
	real jump_target_vertical_vel;
	short secondary_look_type;
	short secondary_look_priority;
	short secondary_look_timer;
	struct direction_specification secondary_look_direction;
	boolean idle_major_active;
	boolean idle_major_is_aiming;
	boolean idle_major_direction_is_interesting;
	boolean idle_minor_active;
	long idle_facing_timer;
	long idle_major_timer;
	long idle_minor_timer;
	struct direction_specification idle_major_direction;
	struct direction_specification idle_minor_direction;
	boolean aiming_away_from_primary;
	boolean free_facing_vector;
	boolean optional_facing_vector;
	boolean desire_stationary_facing;
	boolean fixed_stationary_facing;
	boolean face_exactly;
	real face_exactly_oversteer_angle;
	real_vector3d fixed_stationary_facing_vector;
	real_vector3d desired_facing_vector;
	real_vector3d desired_aiming_vector;
	real_vector3d desired_looking_vector;
	boolean vector_avoidance_clear_times[8][2];
	short vector_avoidance_current_direction;
	real_vector3d vector_avoidance_rotation;
	real vector_avoidance_rotation_emergency;
	real vector_avoidance_rotation_emergency_instantaneous;
	short vector_avoidance_sharp_turn_timer;
	short fire_state;
	short fire_state_timer;
	short burst_disable_timer;
	short trigger_delay_timer;
	short blocked_communication_timer;
	short special_fire_delay;
	short special_fire_deny_attempts;
	boolean firing_at_new_target;
	boolean firing_while_moving;
	boolean overcharging_weapon;
	boolean fire_burst_secondary;
	boolean next_burst_secondary;
	real weapon_maximum_range;
	short current_fire_target_type;
	
	union
	{
		real_point3d current_fire_target_manual_point;
		long current_fire_target_prop_index;
	};
	
	long current_fire_target_timer;
	boolean current_fire_target_visible;
	boolean current_fire_target_underwater;
	boolean current_fire_target_superballistic;
	boolean current_fire_target_bombardment;
	boolean current_fire_target_outside_active_area;
	short current_fire_target_line_of_sight;
	boolean aiming_at_fire_target;
	real_point3d current_fire_target_position;
	real current_fire_target_range;
	real_vector3d current_fire_target_aim_vector;
	real current_fire_target_distance;
	real_point3d burst_initial_position;
	real_point3d burst_origin;
	real_vector3d burst_relative_position;
	real_vector3d burst_adjustment;
	real_point3d burst_target;
	boolean burst_aim_by_vector;
	real_vector3d burst_aim_vector;
	real burst_error;
	real burst_damage_modifier;
	boolean grenade_trying_to_throw;
	boolean grenade_current_lob;
	long grenade_last_check_time;
	real_point3d grenade_current_target;
	long grenade_current_prop_index;
	long grenade_current_ignore_object_index;
	real_vector3d grenade_current_aim_vector;
	real grenade_current_aim_speed;
	boolean idle_vocalization_combat;
	short idle_vocalization_timer;
};

struct actor_output_data
{
	unsigned long control_flags;
	short persistent_control_ticks;
	unsigned long persistent_control_flags;
	short movement_type;
	real_vector3d throttle;
	struct actor_animation_orders animation;
	short aiming_speed;
	real_vector3d facing_vector;
	real_vector3d aiming_vector;
	real_vector3d looking_vector;
	real analog_primary_trigger;
};

struct actor_datum
{
	short identifier;
	struct actor_meta_data meta;
	struct actor_state_data state;
	struct actor_input_data input;
	struct actor_external_orders external_orders;
	struct actor_situation situation;
	struct actor_target_data target;
	struct actor_danger_zone danger_zone;
	struct actor_stimulus_data stimuli;
	struct actor_emotion_data emotions;
	struct actor_firing_position_data firing_positions;
	struct actor_orders orders;
	struct actor_control_data control;
	struct actor_output_data output;
};

struct swarm_datum
{
	short identifier;
	short unit_count;
	long actor_index;
	short swarm_intermittent_action_timer;
	real_point3d swarm_center;
	long unit_indices[MAXIMUM_UNIT_INDICES_PER_SWARM];
	long component_indices[MAXIMUM_COMPONENT_INDICES_PER_SWARM];
};

struct vehicle_avoidance_cylinder
{
	long object_index;
	real_point3d base;
	real height;
	real width;
};

struct vector_avoidance_data
{
	const struct structure_bsp *structure;
	const struct collision_bsp *bsp;
	int object_index;
	real_point3d origin;
	real_vector3d forward;
	real_vector3d left;
	real_vector3d up;
	short avoidance_object_count;
	struct vehicle_avoidance_cylinder avoidance_objects[MAXIMUM_NUMBER_OF_AVOIDANCE_OBJECTS];
	real avoid_width;
	real avoid_distance;
};

/* ---------- prototypes/ACTORS.C */

real_argb_color const *actor_activation_debug_color(long actor_index);

boolean actor_get_running_blind_vector(long actor_index, real_vector3d *run_vector);
boolean actor_has_unlimited_grenades(long actor_index);

/* ---------- prototypes/ACTOR_COMBAT.C */

long actor_aim_grenade(long actor_index, real_point3d const *origin, real_vector3d *vector);

/* ---------- prototypes/ACTOR_MOVING.C */

boolean actor_move_animation_busy(long actor_index);

void actor_move_transform_avoidance_vector(struct vector_avoidance_data *avoidance_data, real_vector3d const *avoidance_vector, real_vector3d *direction_vector);
void actor_move_get_avoidance_direction(struct vector_avoidance_data *avoidance_data, real direction, real_vector3d *direction_vector);

boolean actor_path_has_path(long actor_index);
boolean actor_path_at_destination(long actor_index);

/* ---------- prototypes/ACTOR_PERCEPTION.C */

void actor_get_vision_distances(long actor_index, real maximum_vision_distance, real perception_factor, real horizontal_angle, real *full_distance_reference, real *partial_distance_reference);

short actor_perception_aiming_vector_test_blockage(real_point3d const *source_position, real_vector3d const *source_vector, real_point3d const *friend_position, real_vector3d *friend_direction_to_aiming_vector);

/* ---------- globals */

extern struct data_array *swarm_data;
extern struct data_array *actor_data;

/* ---------- public code */

#endif // __ACTORS_H
