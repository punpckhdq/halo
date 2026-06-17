/*
GAME_GLOBALS.H

header included in hcex build.
*/

#ifndef __GAME_GLOBALS_H
#define __GAME_GLOBALS_H
#pragma once

/* ---------- constants */

/* referenced in game_globals.c? */
enum
{
	_material_dirt = 0,
	_material_sand = 1,
	_material_stone = 2,
	_material_snow = 3,
	_material_wood = 4,
	_material_hollow_metal = 5,
	_material_thin_metal = 6,
	_material_thick_metal = 7,
	_material_rubber = 8,
	_material_glass = 9,
	_material_force_field = 10,
	_material_grunt = 11,
	_material_hunter_armor = 12,
	_material_hunter_skin = 13,
	_material_elite = 14,
	_material_jackal = 15,
	_material_jackal_energy_shield = 16,
	_material_engineer = 17,
	_material_engineer_force_field = 18,
	_material_flood_combat_form = 19,
	_material_flood_carrier_form = 20,
	_material_cyborg = 21,
	_material_cyborg_energy_shield = 22,
	_material_armored_human = 23,
	_material_human = 24,
	_material_sentinel = 25,
	_material_monitor = 26,
	_material_plastic = 27,
	_material_water = 28,
	_material_leaves = 29,
	_material_elite_energy_shield = 30,
	_material_ice = 31,
	_material_hunter_shield = 32,
	NUMBER_OF_MATERIAL_TYPES = 33,
	MAXIMUM_NUMBER_OF_MATERIAL_TYPES = 40,
};

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
