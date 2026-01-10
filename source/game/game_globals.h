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
	unsigned short active_camouflage_flags;
	unsigned short pad;
	float active_camouflage_refraction_amount;
	float active_camouflage_distance_falloff;
	real_rgb_color active_camouflage_tint_color;
	float active_camouflage_hyper_stealth_refraction_amount;
	float active_camouflage_hyper_stealth_distance_falloff;
	real_rgb_color active_camouflage_hyper_stealth_tint_color;
	struct tag_reference distance_attenuation_2d_for_the_pc;
};

struct game_globals
{
	unsigned long flags;
	long unused0[61];
	struct tag_block sounds;
	struct tag_block camera;
	struct tag_block player_control;
	struct tag_block difficulty_information;
	struct tag_block grenades;
	struct tag_block rasterizer_data;
	struct tag_block interface_tag_references;
	struct tag_block weapon_list;
	struct tag_block cheat_powerups;
	struct tag_block multiplayer_information;
	struct tag_block player_information;
	struct tag_block first_person_interface;
	struct tag_block falling_damage;
	struct tag_block materials;
	struct tag_block playlist;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __GAME_GLOBALS_H
