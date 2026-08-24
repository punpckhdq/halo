/*
PLAYERS.H

header included in hcex build.
*/

#ifndef __PLAYERS_H
#define __PLAYERS_H
#pragma once

/* ---------- headers */

#include "game/game.h"

/* ---------- constants */

enum
{
	_player_powerup_active_camouflage = 0,
	_player_powerup_full_spectrum_vision,
	NUMBER_OF_PLAYER_POWERUPS,
};


/* ---------- macros */

#define player_get(index)			((struct player_datum*)datum_get(player_data, index))
#define player_try_and_get(index)	((struct player_datum*)datum_try_and_get(player_data, index))

/* ---------- structures */

struct player_action
{
	unsigned long control_flags;
	real_euler_angles2d desired_facing;
	real_vector2d throttle;
	real primary_trigger;
	short desired_weapon_index;
	short desired_grenade_index;
	short desired_zoom_level;
	short pad;
};

struct network_player
{
	wchar_t name[12];
	short primary_color_index;
	short icon_index;
	char machine_index;
	char controller_index;
	char team_index;
	char player_list_index;
};

struct player_datum
{
	short identifier;
	short local_player_index;
	wchar_t name[12];
	long squad_index;
	long team_index;
	long action_object_index;
	short action_result;
	short action_seat_index;
	long respawn_timer;
	long respawn_penalty;
	long unit_index;
	long dead_unit_index;
	short cluster_index;
	boolean swapped_weapons;
	byte pad0;
	long aim_assist_unit_index;
	long aim_assist_timestamp;
	struct network_player network_player_data;
	short powerup_durations[NUMBER_OF_PLAYER_POWERUPS];
	real speed_multiplier; // +0x6c, 1.0 at creation; slayer handicap
	long source_teleporter_netgame_index; // +0x70, NONE at creation
	long state_message_index; // +0x74, NONE at creation
	long state_message_player_index; // +0x78, NONE at creation
	long state_message_target_player; // +0x7c, NONE at creation
	long state_message_target_time; // +0x80
	long last_death_time; // +0x84, game_time_get() in game_engine_player_killed
	long target_player_index; // +0x88, slayer kill-in-order target ("next_target")
	struct game_statistics statistics; // +0x8c
	long telefrag_timeout; // +0xc8
	long quit_out_of_game_time; // +0xcc, NONE at creation
	boolean is_blocking_teleporter; // +0xd0
	boolean quit_out_of_game; // +0xd1
	byte pad1[2];
};

/* ---------- prototypes/PLAYER_CONTROL.C */

void player_control_unzoom(long unit_index);

/* ---------- prototypes/PLAYERS.C */

long player_index_from_unit_index(long unit_index);

unsigned long const *players_get_combined_pvs_local(void);
unsigned long const *players_get_combined_pvs(void);

void player_control_fix_for_loaded_game_state(void);

/* ---------- prototypes/PLAYER_QUEUES_NEW.C */

void update_queues_reset_and_fill_with_lies(void);

/* ---------- globals */

extern struct data_array *player_data;

/* ---------- public code */

#endif // __PLAYERS_H
