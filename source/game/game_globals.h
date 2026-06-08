/*
GAME_GLOBALS.H

header included in hcex build.
*/

#ifndef __GAME_GLOBALS_H
#define __GAME_GLOBALS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct game_globals_grenade
{
	short maximum_count;
	short mp_spawn_default;
	struct tag_reference throwing_effect;
	struct tag_reference hud_interface;
	struct tag_reference item;
	struct tag_reference projectile;
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

struct game_globals
{
	unsigned long flags;
	long unused0[61];
	struct tag_block sounds;
	struct tag_block camera;
	struct tag_block player_control;
	struct tag_block difficulty_information;
	struct tag_block grenades;					// game_globals_grenade
	struct tag_block rasterizer_data;			// game_globals_rasterizer_data
	struct tag_block interface_tag_references;
	struct tag_block weapon_list;
	struct tag_block cheat_powerups;
	struct tag_block multiplayer_information;
	struct tag_block player_information;		// game_globals_player_information
	struct tag_block first_person_interface;
	struct tag_block falling_damage;
	struct tag_block materials;
	struct tag_block playlist;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __GAME_GLOBALS_H
