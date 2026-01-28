/*
PLAYERS.H

header included in hcex build.
*/

#ifndef __PLAYERS_H
#define __PLAYERS_H
#pragma once

/* ---------- constants */

enum
{
	_player_powerup_active_camouflage= 0,
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
	struct game_statistics statistics;
	long telefrag_timeout;
	long quit_out_of_game_time;
	boolean is_blocking_teleporter;
	boolean quit_out_of_game;
	struct player_action action_input;
};

/* ---------- prototypes/PLAYERS.C */

unsigned long const *players_get_combined_pvs_local(void);
unsigned long const *players_get_combined_pvs(void);

void player_control_fix_for_loaded_game_state(void);

/* ---------- prototypes/PLAYER_QUEUES_NEW.C */

void update_queues_reset_and_fill_with_lies(void);

/* ---------- globals */

/* comm */
struct data_array* player_data;

/* ---------- public code */

#endif // __PLAYERS_H
