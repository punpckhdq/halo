/*
GAME_GLOBALS.H

header included in hcex build.
*/

#ifndef __GAME_GLOBALS_H
#define __GAME_GLOBALS_H
#pragma once

/* ---------- headers */


/* ---------- constants */

enum
{
	GAME_GLOBALS_DEFINITION_TAG = 'matg',
	GAME_GLOBALS_DEFINITION_VERSION = 3
};

/* referenced in game_globals.c? */
enum
{
	_material_dirt = 0,
	_material_sand,
	_material_stone,
	_material_snow,
	_material_wood,
	_material_hollow_metal,
	_material_thin_metal,
	_material_thick_metal,
	_material_rubber,
	_material_glass,
	_material_force_field,
	_material_grunt,
	_material_hunter_armor,
	_material_hunter_skin,
	_material_elite,
	_material_jackal,
	_material_jackal_energy_shield,
	_material_engineer,
	_material_engineer_force_field,
	_material_flood_combat_form,
	_material_flood_carrier_form,
	_material_cyborg,
	_material_cyborg_energy_shield,
	_material_armored_human,
	_material_human,
	_material_sentinel,
	_material_monitor,
	_material_plastic,
	_material_water,
	_material_leaves,
	_material_elite_energy_shield,
	_material_ice,
	_material_hunter_shield,
	NUMBER_OF_MATERIAL_TYPES,
	MAXIMUM_NUMBER_OF_MATERIAL_TYPES = 40,
};

enum
{
	_game_difficulty_enemy_damage_scale = 0,
	_game_difficulty_enemy_vitality_scale,
	_game_difficulty_enemy_shield_scale,
	_game_difficulty_enemy_recharge_scale,
	_game_difficulty_friend_damage_scale,
	_game_difficulty_friend_vitality_scale,
	_game_difficulty_friend_shield_scale,
	_game_difficulty_friend_recharge_scale,
	_game_difficulty_infection_form_toughness,
	_game_difficulty_health_unused6,
	_game_difficulty_rate_of_fire_scale,
	_game_difficulty_fire_projectile_error_scale,
	_game_difficulty_burst_error_scale,
	_game_difficulty_new_target_delay_scale,
	_game_difficulty_burst_separation_delay_scale,
	_game_difficulty_target_tracking_bonus,
	_game_difficulty_target_leading_bonus,
	_game_difficulty_overcharge_chance_scale,
	_game_difficulty_special_fire_delay_scale,
	_game_difficulty_projectile_guidance_vs_player_scale,
	_game_difficulty_melee_delay_bonus,
	_game_difficulty_melee_delay_scale,
	_game_difficulty_fire_unused6,
	_game_difficulty_grenade_chance_scale,
	_game_difficulty_grenade_timer_scale,
	_game_difficulty_grenade_unused1,
	_game_difficulty_grenade_unused2,
	_game_difficulty_grenade_unused3,
	_game_difficulty_major_normal_placement,
	_game_difficulty_major_few_placement,
	_game_difficulty_major_many_placement,
	_game_difficulty_unused1,
	_game_difficulty_unused2,
	_game_difficulty_unused3,
	_game_difficulty_unused4,
};

enum
{
	_multiplayer_sound_oddball_spawn = 0,
	_multiplayer_sound_game_over,
	_multiplayer_sound_60_seconds,
	_multiplayer_sound_30_seconds,
	_multiplayer_sound_red_60_seconds,
	_multiplayer_sound_red_30_seconds,
	_multiplayer_sound_blue_60_seconds,
	_multiplayer_sound_blue_30_seconds,
	_multiplayer_sound_ctf_blue_took_flag,
	_multiplayer_sound_ctf_blue_returned_flag,
	_multiplayer_sound_ctf_blue_captured_flag,
	_multiplayer_sound_ctf_red_took_flag,
	_multiplayer_sound_ctf_red_returned_flag,
	_multiplayer_sound_ctf_red_captured_flag,
	_multiplayer_sound_double_kill,
	_multiplayer_sound_triple_kill,
	_multiplayer_sound_killtacular_kill,
	_multiplayer_sound_running_riot,
	_multiplayer_sound_killing_spree,
	_multiplayer_sound_oddball,
	_multiplayer_sound_race,
	_multiplayer_sound_slayer,
	_multiplayer_sound_ctf,
	_multiplayer_sound_warthog,
	_multiplayer_sound_ghost,
	_multiplayer_sound_scorpion,
	_multiplayer_sound_countdown_timer,
	_multiplayer_sound_teleporter_activate,
	_multiplayer_sound_flag_failure,
	_multiplayer_sound_countdown_for_respawn,
	_multiplayer_sound_hill_move,
	_multiplayer_sound_respawn,
	_multiplayer_sound_team_king,
	_multiplayer_sound_team_oddball,
	_multiplayer_sound_team_race,
	_multiplayer_sound_team_slayer,
	_multiplayer_sound_king,
	_multiplayer_sound_blue_team_ctf,
	_multiplayer_sound_red_team_ctf,
	_multiplayer_sound_hill_contested,
	_multiplayer_sound_hill_controlled,
	_multiplayer_sound_hill_occupied,
	_multiplayer_sound_countdown_timer_end,
	NUMBER_OF_MULTIPLAYER_SOUNDS,
};

/* ---------- macros */

#define game_globals_definition_get(index) ((struct game_globals *)tag_get(GAME_GLOBALS_DEFINITION_TAG, index)) /* fake name */

/* ---------- structures */

struct breakable_surface
{
	real maximum_vitality; 
	long unused1[2]; 
	unsigned long flags; 
	struct tag_reference effect; 
	struct tag_reference sound; 
	long unused2[6]; 
	struct tag_block particle_effects; 
};

struct material_definition
{
	unsigned long flags;
	long modifiers_unused[24];
	unsigned long biped_flags;
	real biped_maximum_acceleration;
	real biped_slip_angle;
	real biped_slow_angle;
	long biped_unused[8];
	real physics_ground_friction_scale;
	real physics_ground_friction_normal_k1_scale;
	real physics_ground_friction_normal_k0_scale;
	real physics_ground_depth_scale;
	real physics_ground_damp_fraction_scale;
	real physics_unused[19];
	long unused[120];
	struct breakable_surface breakable_surface;
	long unused2[15];
	struct tag_reference melee_hit_sound;
};

struct game_globals_grenade
{
	short maximum_count;
	short mp_spawn_default;
	struct tag_reference throwing_effect;
	struct tag_reference hud_interface;
	struct tag_reference item;
	struct tag_reference projectile;
};

struct game_globals_camera
{
	struct tag_reference default_unit_camera_track;
};

struct game_globals_player_control
{
	real magnetism_friction;
	real magnetism_adhesion;
	real magnetism_inconsequential_target_scale;
	real magnetism_unused[13];
	real look_acceleration_time;
	real look_acceleration_scale;
	real look_pegging_threshold;
	real look_default_pitch_rate;
	real look_default_yaw_rate;
	real look_autolevel_scale;
	real look_unused[5];
	short minimum_weapon_swap_ticks;
	short minimum_autolevel_enabled_ticks;
	real minimum_vehicle_flipping_angle;
	struct tag_block look_function;
};

struct game_globals_rasterizer_data
{
	struct tag_reference distance_attenuation;
	struct tag_reference vector_normalization;
	struct tag_reference atmospheric_fog_density;
	struct tag_reference planar_fog_density;
	struct tag_reference linear_corner_fade;
	struct tag_reference active_camouflage_distortion;
	struct tag_reference glow;
	long unused1[15];
	struct tag_reference default_textures[3];
	struct tag_reference test[4];
	struct tag_reference screen_effect_video_scanline_map;
	struct tag_reference screen_effect_video_noise_map;
	long unused2[13];
	word active_camouflage_flags;
	word pad;
	real active_camouflage_refraction_amount;
	real active_camouflage_distance_falloff;
	real_rgb_color active_camouflage_tint_color;
	real active_camouflage_hyper_stealth_refraction_amount;
	real active_camouflage_hyper_stealth_distance_falloff;
	real_rgb_color active_camouflage_hyper_stealth_tint_color;
	struct tag_reference distance_attenuation_2d_for_the_pc;
};

struct game_globals_player_information
{
	struct tag_reference player_unit;
	long unused1[7];
	real walking_speed;
	real double_speed_multiplier;
	real run_forward_speed;
	real run_backward_speed;
	real run_sideways_speed;
	real run_acceleration;
	real sneak_forward_speed;
	real sneak_backward_speed;
	real sneak_sideways_speed;
	real sneak_acceleration;
	real airborne_acceleration;
	real multiplayer_only_speed_muliplier;
	long movement_unused[3];
	real_vector3d grenade_origin;
	real grenade_unused[3];
	real stun_movement_penalty;
	real stun_turning_penalty;
	real stun_jumping_penalty;
	real minimum_stun_time;
	real maximum_stun_time;
	real stun_unused[2];
	real first_person_idle_time_lower_bound;
	real first_person_idle_time_upper_bound;
	real first_person_idle_skip_fraction;
	long unused_first_person_unused[4];
	struct tag_reference coop_respawn_effect;
	long unused2[11];
};

struct game_globals_first_person_interface
{
	struct tag_reference hands;
	struct tag_reference hud_base;
	struct tag_reference hud_shield_meter;
	point2d hud_shield_meter_origin;
	struct tag_reference hud_body_meter;
	point2d hud_body_meter_origin;
	struct tag_reference night_vision_off_on_effect;
	struct tag_reference night_vision_on_off_effect;
	unsigned long unused[22];
};

struct game_globals_multiplayer_information
{
	struct tag_reference flag;
	struct tag_reference player_unit;
	struct tag_block vehicles;
	struct tag_reference hill_shader;
	struct tag_reference flag_shader;
	struct tag_reference ball;
	struct tag_block sounds;
	long unused[14];
};

struct game_globals_falling_damage
{
	long falling_unused[2];
	real falling_distance_lower_bound;
	real falling_distance_upper_bound;
	struct tag_reference falling_damage;
	long terminal_velocity_unused[2];
	real maximum_distance;
	struct tag_reference maximum_distance_damage;
	struct tag_reference vehicle_hit_environment_damage_effect;
	struct tag_reference vehicle_killed_unit_damage_effect;
	struct tag_reference vehicle_collision_damage;
	struct tag_reference flaming_death_damage;
	long unused2[4];
	real runtime_maximum_falling_velocity;
	real runtime_minimum_damage_velocity;
	real runtime_maximum_damage_velocity;
};

struct game_globals
{
	unsigned long flags;
	long unused0[61];
	struct tag_block sounds;
	struct tag_block camera;
	struct tag_block player_control;			// game_globals_player_control
	struct tag_block difficulty_information;
	struct tag_block grenades;					// game_globals_grenade
	struct tag_block rasterizer_data;			// game_globals_rasterizer_data
	struct tag_block interface_tag_references;
	struct tag_block weapon_list;
	struct tag_block cheat_powerups;
	struct tag_block multiplayer_information;
	struct tag_block player_information;		// game_globals_player_information
	struct tag_block first_person_interface;	// game_globals_first_person_interface
	struct tag_block falling_damage;
	struct tag_block materials;
	struct tag_block playlist;
};

/* ---------- prototypes/EXAMPLE.C */

real game_difficulty_get_team_value(short value_type, short team_index);
char const *material_get_name(short material_type);
real game_difficulty_get_value(short value_type);

/* ---------- globals */

/* ---------- public code */

#endif // __GAME_GLOBALS_H
