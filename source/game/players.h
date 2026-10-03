/*
PLAYERS.H

header included in hcex build.
*/

#ifndef __PLAYERS_H
#define __PLAYERS_H
#pragma once

/* ---------- headers */

#include "game.h"
#include "network_game_manager.h"

/* ---------- constants */

enum
{
	MAXIMUM_NUMBER_OF_LOCAL_PLAYERS = 4,
};

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

struct multiplayer_player_info
{
	real speed_multiplier;
	long teleporter_index;
	long state_message;
	long state_message_data;
	long player_display_index;
	long player_display_count;
	long time_of_death;
	long special;
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
	struct multiplayer_player_info multiplayer;
	struct game_statistics statistics;
	long telefrag_timeout;
	long quit_out_of_game_time;
	boolean is_blocking_teleporter;
	boolean quit_out_of_game;
};

struct unit_camera_info
{
	long unit_index;
	short seat_index;
	struct unit_camera *unit_camera;
	real_point3d unit_origin;
};

/* ---------- prototypes/PLAYER_CONTROL.C */

void player_control_update(real seconds_elapsed);

void player_control_unzoom(long unit_index);
long player_control_get_unit_index(short local_player_index);
real_vector3d *player_control_get_facing_direction(short local_player_index, real_vector3d *direction);
real player_control_get_field_of_view(short local_player_index);
real_euler_angles2d const *player_control_get_facing_angles(short local_player_index);
long player_control_get_aiming_unit_index(short local_player_index);
void player_control_get_unit_camera_info(short local_player_index, struct unit_camera_info *camera_info);

/* ---------- prototypes/PLAYERS.C */

long player_new(long machine_index, long player_index, short local_player_index, struct network_player *network_player_data);
void local_player_set_player_index(short local_player_index, long player_index);
boolean players_respawn_coop(void);

boolean local_player_exists(long local_player_index);
short local_player_count(void);
short local_player_get_next(short local_player_index);
long local_player_get_player_index(short local_player_index);

long player_index_from_unit_index(long unit_index);
long local_player_get_player_index(short local_player_index);
short local_player_get_next(short local_player_index);

unsigned long const *players_get_combined_pvs_local(void);
unsigned long const *players_get_combined_pvs(void);

void player_control_fix_for_loaded_game_state(void);

void player_input_enable(boolean enable);

/* ---------- prototypes/PLAYER_QUEUES_NEW.C */

void update_server_delete(void);
boolean update_server_new(void);
void update_server_start(void);

void update_queues_reset_and_fill_with_lies(void);

/* ---------- globals */

extern struct data_array *player_data;

extern real player_look_yaw_rate[];
extern real player_look_pitch_rate[];

/* ---------- public code */

#endif // __PLAYERS_H
