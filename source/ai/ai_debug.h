/*
AI_DEBUG.H

header included in hcex build.
*/

#ifndef __AI_DEBUG_H
#define __AI_DEBUG_H
#pragma once

/* ---------- headers */

#include "actors.h"

#include "math/real_math.h"

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

struct ai_debug_state
{
	boolean enter_debugger;
	boolean select_this_actor;
	char __unknown02[16];
	char selected_squad_name[32];
	int selected_squad_index;
	int selected_actor_index;
	char __unknown3C[105];
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
	char __unknownF8[547380];
};

struct actor_debug_info
{
	long last_render_id;
	long last_path_refresh;
	short firing_decision;
	real shooting_rof;
	real_point3d burst_last_known_position;
	real_point3d burst_tracked_position;
	real_vector3d burst_lead_vector;
	char __unknown1C[40];
	long last_projectile_aiming_time;
	boolean field_60;
	real_point3d field_64;
	real_vector3d field_70;
	real_point3d field_7C;
	boolean field_88;
	real_vector3d field_8C;
	real_vector3d field_98;
	boolean field_A4;
	short field_A6;
	real field_A8;
	real field_AC;
	real field_B0;
	real field_B4;
	boolean field_B8;
	short field_BA;
	short field_BC;
	real field_C0;
	long last_vehicle_avoidance_time;
	real_point3d field_C8;
	real_point3d field_D4;
	real field_E0;
	real_point3d field_E4;
	real field_F0;
	boolean field_F4;
	real_point3d field_F8;
	long last_melee_time;
	real_point3d field_108;
	real_vector3d field_114;
	real_point3d field_120;
	real_vector3d field_12C;
	boolean field_138;
	boolean field_139;
	real_point3d field_13C;
	real field_148;
	real field_14C;
	long grenade_eval_time;
	short grenade_decision;
	short grenade_encounter_timeout_ticks;
	short grenade_enemy_count;
	short grenade_required_enemy_count;
	real grenade_current_damage;
	real grenade_random_value;
	real grenade_random_chance;
	long danger_avoidance_time;
	short danger_decision;
	boolean danger_abandoned_path;
	real danger_far_dist;
	real danger_far_radius;
	real danger_zone_dist;
	real danger_zone_radius;
	real danger_intersect_time;
	long dive_decision_time;
	short dive_decision;
	long charge_last_time;
	short charge_decision;
	real field_194;
	boolean field_198;
	short flying_error_ticks;
	long field_19C;
	struct vector_avoidance_data avoidance_data;
	short avoidance_type[ACTOR_MAXIMUM_AVOIDANCE_RAYS];
	real collision_t[ACTOR_MAXIMUM_AVOIDANCE_RAYS];
	real_point3d ray_origin[ACTOR_MAXIMUM_AVOIDANCE_RAYS];
	real_vector3d ray_direction[ACTOR_MAXIMUM_AVOIDANCE_RAYS];
	short field_62F8[8][2];
	real avoid_t[8][2];
	real_point3d field_6358[8][2];
	real_vector3d field_6418[8][2];
	real field_64D8[8];
	long field_64F8;
	real field_64FC;
	short field_6500;
	real field_6504;
	real field_6508;
	real sign_no_danger;
	real field_6510;
	real sign_too_far_cosangle;
	real sign_rotated;
	real field_651C;
	real field_6520;
	real_vector3d field_6524;
	real_vector3d field_6530;
	short field_653C;
	real_vector3d field_6540;
	real field_654C;
	boolean field_6550;
	boolean field_6551;
	real field_6554;
	real field_6558;
	real_vector3d avoidance_vector;
	real field_6568;
	long vision_last_time;
	real vision_last_maximum_distance;
	real vision_last_perception_factor;
	short num_debug_evaluations;
	short evaluation_mode;
};

/* ---------- prototypes/AI_DEBUG.C */

void ai_debug_initialize(void);

void ai_debug_initialize_for_new_map(void);

void ai_debug_change_selected_encounter(boolean a1);

void ai_debug_change_selected_actor(boolean a1);

char *ai_debug_describe_actor(long actor_index, long unit_index, boolean include_squad, char *buffer, long bufsize);

struct path_debug_storage *ai_debug_get_last_path(long);

boolean ai_debug_highlight_cluster(short index, real_argb_color const **highlight_color);

/* ---------- globals */

extern struct ai_debug_state ai_debug;
extern struct actor_debug_info *actor_debug_array;
extern struct path_debug_storage *actor_path_debug_array;

extern real_point3d global_ai_debug_drawstack_next_position;
extern real_point3d global_ai_debug_drawstack_last_position;
extern real global_ai_debug_drawstack_height;
extern real_argb_color global_temporary_render_color;

/* ---------- public code */

#endif // __AI_DEBUG_H
