/*
GAME_ENGINE.H

header included in hcex build.
*/

#ifndef __GAME_ENGINE_H
#define __GAME_ENGINE_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

enum
{
	_game_engine_ctf = 1,
	_game_engine_slayer,
	_game_engine_oddball,
	_game_engine_king,
	_game_engine_race,
	_game_engine_stub = 7,
};

struct game_variant
{
	wchar_t name[12]; // +0x00
	long engine_index; // +0x18, _game_engine_*
	// the engine-independent settings, common to every variant
	struct universal_variant
	{
		boolean teams; // +0x1c, game_engine_has_teams
		byte pad0[3]; // +0x1d, alignment; zero-filled and read nowhere
		unsigned long flags; // +0x20, bit 1 display_team_indicators, bit 2 infinite grenades, bit 3 no shields
		// how a player is shown where the goals are
		long goal_radar; // +0x24, 0 puts them on the motion sensor, 1 draws them as nav points, 2 does neither
		boolean odd_man_out; // +0x28, game_engine_player_is_odd_man_out
		byte pad1[3]; // +0x29, alignment; zero-filled and read nowhere
		long respawn_time_growth; // +0x2c, game_engine_player_killed
		long respawn_time; // +0x30, game_engine_player_killed
		long suicide_penalty; // +0x34, game_engine_player_killed
		long maximum_lives; // +0x38, game_engine_player_is_out_of_lives
		real health; // +0x3c, game_engine_get_damage_multiplier = 1/PIN(x, 0.25, 4)
		long score_to_win; // +0x40, slayer_player_update ends the game at team_score >= this
		// weapon set, pinned to 0..10 by game_engine_variant_cleanup: game_engine_remap_weapon rewrites
		// every weapon on the map through it, and game_engine_remap_equipment every grenade
		long weapon_set; // +0x44, game_engine_postspawn_player_update special-cases 3, 9, 10
		// vehicle set, pinned to 0..4 by game_engine_variant_cleanup: game_engine_remap_vehicle keeps
		// every multiplayer vehicle at 0, removes them all at 1, and keeps only multiplayer_information
		// vehicle 0, 1 or 2 at 2, 3 or 4
		long vehicle_set; // +0x48
	} universal_variant; // +0x1c..+0x4b
	// engine-specific settings; each build_game_variant_* writes only the fields its engine uses
	union
	{
		struct
		{
			boolean no_death_bonus; // +0x4c, set by build_game_variant_slayer_pro; clear: dead player gains speed
			boolean no_kill_penalty; // +0x4d, set by build_game_variant_slayer_pro; clear: killer loses speed
			boolean kill_in_order; // +0x4e, target logic in game_engine_slayer.c
		} slayer;
		struct
		{
			boolean assault; // +0x4c, exchanges the two flags between the bases
			boolean reset_on_capture; // +0x4d, every ctf variant clears it and only game_engine_variant_cleanup touches it
			boolean flag_must_reset; // +0x4e, set: touching your own flag away from home no longer returns it
			boolean flag_at_home_to_score; // +0x4f, set: a capture scores nothing while your own flag is away from home
			long single_flag_time; // +0x50, positive: one flag instead of two, changing teams on this interval
		} ctf;
		struct
		{
			boolean random_start; // +0x4c, set: the ball spawns at a random ball netgame flag rather than its own
			boolean ball_spawn_delay; // +0x4d, build_game_variant_oddball and build_game_variant_reverse_tag set it, nothing reads it
			byte pad2[2]; // +0x4e, zero-filled: no oddball variant writes it and the oddball engine never reads it
			long speed_with_ball; // +0x50, ball carrier speed: 1 for 1.0, 2 for 1.25, anything else 0.75
			// the trait a player is asked about: +0x54 while he carries a ball, +0x58 otherwise
			long trait_with_ball; // +0x54, 1 lets a carrier keep active camouflage, every other value depowers it
			long trait_without_ball; // +0x58, nothing but that test reads it
			long ball_type; // +0x5c, 0 scores by time held, 1 is reverse tag and passes the ball to the killer, 2 passes it and scores by kills
			long ball_spawn_count; // +0x60, the number of balls in play
		} oddball;
		struct
		{
			boolean moving_hill; // +0x4c, set: the hill moves to the next one every 1800 ticks
		} king;
		struct
		{
			long race_type; // +0x4c, 0 requires the flags in index order, 1 allows any order, 2 shares one flag between everyone
			long team_scoring; // +0x50, a team's lap count is its slowest member's at 0, its fastest at 1, the sum at 2
		} race;
		byte engine_specific[24]; // +0x4c..+0x63
	};
	short flags2; // +0x64, every build_game_variant_* writes 1 and no code reads it
	short pad3; // +0x66, zero-filled: never written and never read
};

/* ---------- prototypes/GAME_ENGINE.C */

boolean game_engine_running(void);

boolean game_engine_infinite_grenades(long player_index);

long game_engine_remap_object_definition(long definition_index);

long game_engine_remap_vehicle(long vehicle_definition_index);
long game_engine_remap_equipment(long equipment_definition_index);
long game_engine_remap_weapon(long weapon_definition_index);


boolean game_engine_allow_integrated_lights(long object_index);

/* ---------- globals */

extern struct game_engine *game_engine;

/* ---------- public code */

#endif // __GAME_ENGINE_H
