/*
AI_DEBUG.H

header included in hcex build.
*/

#ifndef __AI_DEBUG_H
#define __AI_DEBUG_H
#pragma once

/* ---------- headers */



/* ---------- constants */

enum
{
	_firing_disabled = 0,
	_firing_busy,
	_firing_wrong_target,
	_firing_no_target,
	_firing_outside_active_area,
	_firing_not_visible,
	_firing_outside_range,
	_firing_blocked,
	_firing_holding_for_line,
	_firing_holding,
	_firing_pausing_for_line,
	_firing_pausing,
	_firing_wild,
	_firing_burst,
	_firing_not_in_midair,
	_firing_not_crouching,
	_firing_not_standing,
	_firing_not_stationary,
	_firing_underwater,
	_firing_min_range,
	NUMBER_OF_ACTOR_DEBUG_FIRING_DECISIONS,
};

enum
{
	_grenade_vehicle = 0,
	_grenade_unit_busy,
	_grenade_being_hurt,
	_grenade_no_grenades,
	_grenade_random_failed,
	_grenade_encounter_timeout,
	_grenade_target_failed,
	_grenade_not_enough_enemies,
	_grenade_collateral_damage,
	_grenade_trajectory_failed,
	_grenade_success,
	NUMBER_OF_ACTOR_DEBUG_GRENADE_DECISIONS,
};

enum
{
	_danger_avoidance_none = 0,
	_danger_avoidance_unnoticed,
	_danger_avoidance_animation_busy,
	_danger_avoidance_vehicle,
	_danger_avoidance_far_away,
	_danger_avoidance_outside_zone,
	_danger_avoidance_evasion_disallowed,
	_danger_avoidance_no_safe_direction,
	_danger_avoidance_no_desire,
	_danger_avoidance_can_avoid,
	_danger_avoidance_imminent_explosion,
	_danger_avoidance_imminent_impact,
	_danger_avoidance_no_animation,
	_danger_avoidance_attached_to_us,
	NUMBER_OF_ACTOR_DEBUG_DANGER_AVOIDANCE_DECISIONS,
};

enum
{
	_dive_not_attempted = 0,
	_dive_cannot_move,
	_dive_no_animation,
	_dive_animation_failure,
	_dive_success,
	NUMBER_OF_ACTOR_DEBUG_DIVE_DECISIONS
};

enum
{
	_charge_vehicle_success = 0,
	_charge_vehicle_not_driver,
	_charge_melee_swarm_cant,
	_charge_melee_inhibited,
	_charge_melee_notarget,
	_charge_melee_no_animation,
	_charge_melee_cannot_move,
	_charge_melee_success,
	_charge_stalking_success,
	_charge_close_success,
	NUMBER_OF_ACTOR_DEBUG_CHARGE_DECISIONS
};

/* ---------- macros */

/* ---------- structures */

struct ai_debug_unknown_state
{
	char __unknown0[1648];
};

struct ai_debug_firing_position
{
	boolean pursuit_position;
	boolean evaluated;
	char __unknown2[62];
};

#ifdef DEBUG
struct ai_debug_state
{
	boolean enter_debugger;
	boolean select_this_actor;
	boolean fix_defending_guard_firing_positions;
	boolean fix_actor_variants;
	boolean debug_fast_los;
	boolean debug_evaluate_all_positions;
	boolean debug_ignore_player;
	boolean debug_invisible_player;
	boolean debug_flee_always;
	boolean debug_force_all_active;
	boolean debug_disable_wounded_sounds;
	boolean debug_blind;
	boolean debug_deaf;
	boolean force_vocalizations;
	boolean force_crouch;
	boolean path_disable_obstacle_avoidance;
	boolean path_disable_smoothing;
	boolean field_11;
	char selected_squad_name[32];
	long selected_encounter_index;
	long selected_actor_index;
	boolean path_enable;
	boolean path_start_freeze;
	boolean path_end_freeze;
	boolean path_flood;
	real path_maximum_radius;
	boolean path_attractor;
	real path_attractor_radius;
	real path_attractor_weight;
	real path_accept_radius;
	char __unknown3C[81];
	boolean render;
	boolean render_all_actors;
	boolean render_inactive_actors;
	boolean render_lineoffire_crouching;
	boolean render_lineoffire;
	boolean render_lineofsight;
	boolean render_ballistic_lineoffire;
	boolean render_encounter_activeregion;
	boolean render_vision_cones;
	boolean render_current_state;
	boolean render_detailed_state;
	boolean render_props;
	boolean render_props_web;
	boolean render_props_no_friends;
	boolean render_props_unreachable;
	boolean render_props_unopposable;
	boolean render_props_target_weight;
	boolean render_idle_look;
	boolean render_targets;
	boolean render_targets_last_visible;
	boolean render_states;
	boolean render_support_surfaces;
	boolean render_recent_damage;
	boolean render_threats;
	boolean render_emotions;
	boolean render_audibility;
	boolean render_aiming_vectors;
	boolean render_secondary_looking;
	boolean render_vitality;
	boolean render_active_cover_seeking;
	boolean render_evaluations;
	boolean render_pursuit;
	boolean render_shooting;
	boolean render_trigger;
	boolean render_projectile_aiming;
	boolean render_aiming_validity;
	boolean render_speech;
	boolean render_teams;
	boolean render_player_ratings;
	boolean render_spatial_effects;
	boolean render_firing_positions;
	boolean render_gun_positions;
	boolean render_burst_geometry;
	boolean render_vehicle_avoidance;
	boolean render_vehicles_enterable;
	boolean render_melee_check;
	boolean render_dialogue_variants;
	boolean render_grenade_decisions;
	boolean render_danger_zones;
	boolean render_charge_decisions;
	boolean render_control;
	boolean render_activation;
	boolean render_paths;
	boolean render_paths_selected_only;
	boolean render_paths_failed;
	boolean render_paths_current;
	boolean render_paths_raw;
	boolean render_paths_smoothed;
	boolean render_paths_avoided;
	short render_paths_avoidance_segment;
	boolean render_paths_avoidance_obstacles;
	boolean render_paths_avoidance_search;
	boolean render_paths_destination;
	boolean render_paths_nodes;
	boolean render_paths_nodes_all;
	boolean render_paths_nodes_polygons;
	boolean render_paths_nodes_costs;
	boolean render_paths_nodes_closest;
	boolean render_player_aiming_blocked;
	boolean render_vector_avoidance;
	boolean render_vector_avoidance_rays;
	boolean render_vector_avoidance_sense_t;
	boolean render_vector_avoidance_avoid_t;
	boolean render_vector_avoidance_clear_time;
	boolean render_vector_avoidance_weights;
	boolean render_vector_avoidance_objects;
	boolean render_vector_avoidance_intermediate;
	boolean render_postcombat;
	long last_render_id;
	boolean lineoffire_valid;
	boolean lineoffire_success;
	real_point3d lineoffire_origin;
	real_vector3d lineoffire_vector;
	long lineoffire_numpills;
	boolean lineoffire_pillhit[16];
	real_point3d lineoffire_pillbase[16];
	real_vector3d lineoffire_pilldirectedheight[16];
	real lineoffire_pillwidth[16];
	boolean lineofsight_overflow;
	long lineofsight_numpoints;
	char __unknown2F0[262144];
	long field_42F0;
	char __unknown42F4[50472];
	struct path_state path_state;
	struct path_result path;
	struct path_debug_storage path_debug;
	boolean firing_position_context_valid;
	byte pad[3];
	struct ai_debug_unknown_state field_7D384;
	struct ai_debug_firing_position firing_positions[512];
	char __unknown859F4[32];
	boolean idle_look_valid;
	long prop_idle_actor_index;
	short prop_idle_look_count;
	long prop_idle_look_indicies[32];
	real prop_idle_look_distances[32];
	boolean field_85B20;
	boolean field_85B21;
	boolean field_85B22;
	long speaking_unit_index;
	short field_85B28;
	short vocalization_type;
};
#endif // DEBUG

/* ---------- prototypes/AI_DEBUG.C */

void ai_debug_initialize(void);
void ai_debug_dispose(void);
void ai_debug_dispose_from_old_map(void);
void ai_debug_clear_storage(void);
void ai_debug_actor_deleted(long actor_index);
struct path_debug_storage *ai_debug_get_last_path(long actor_index);
struct path_debug_storage *ai_debug_get_path_storage(long actor_index);
void ai_debug_select_encounter(long encounter_index);
void ai_debug_select_actor(long encounter_index, long actor_index);
void ai_debug_sound_point_set(void);
void ai_debug_lineoffire_new(real_point3d const *origin, real_vector3d const *vector);
void ai_debug_lineoffire_addpill(real_point3d const *base, real_vector3d const *directedheight, real width, boolean hit);
void ai_debug_lineoffire_success(boolean success);
boolean ai_debug_highlight_cluster(short index, real_argb_color const **highlight_color);
void ai_debug_lineofsight_reset(void);
char *ai_debug_describe_actor(long actor_index, long unit_index, boolean include_squad, char *buffer, long bufsize);
void ai_debug_vocalize(char const *speech_priority_name, char const *vocalization_type_name);
void ai_debug_speak(char const *vocalization_type_name);

void ai_debug_change_selected_encounter(boolean search_forwards);
void ai_debug_change_selected_actor(boolean search_forwards);

void ai_debug_initialize_for_new_map(void);


/* ---------- globals */

#ifdef DEBUG
extern struct ai_debug_state ai_debug;
#endif // DEBUG
extern struct actor_debug_info *actor_debug_array;
extern struct path_debug_storage *actor_path_debug_array;

extern real_point3d global_ai_debug_drawstack_next_position;
extern real_point3d global_ai_debug_drawstack_last_position;
extern real global_ai_debug_drawstack_height;
extern real_argb_color global_temporary_render_color;

/* ---------- public code */

#endif // __AI_DEBUG_H
