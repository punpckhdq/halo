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

enum
{
	_player_action_result_reload = 0,
	_player_action_result_pickup_powerup,
	_player_action_result_pickup_weapon,
	_player_action_result_exit_vehicle,
	_player_action_result_swap_for_grenades,
	_player_action_result_swap_for_powerup,
	_player_action_result_swap_for_weapon,
	_player_action_result_add_weapon_to_inventory,
	_player_action_result_enter_vehicle,
	_player_action_result_evict_from_vehicle,
	_player_action_result_touch_device,
	_player_action_result_flip_vehicle,
	NUMBER_OF_PLAYER_ACTION_RESULTS,
};

enum
{
	_player_respawn_failure_none = 0,
	_player_respawn_failure_combat,
	_player_respawn_failure_enemies,
	_player_respawn_failure_moving,
	_player_respawn_failure_vehicle,
	NUMBER_OF_PLAYER_RESPAWN_FAILURES,
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

struct player_action_collection
{
	struct player_action actions[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
};

struct game_update
{
	word number_of_actions;
	struct player_action actions[NETWORK_GAME_MAXIMUM_PLAYER_COUNT];
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

void player_control_initialize(void);
void player_control_dispose(void);
void player_control_initialize_for_new_map(void);
void player_control_new_unit(short local_player_index, long unit_index);
boolean player_control_update(real seconds_elapsed);

void player_control_unzoom(long unit_index);
long player_control_get_unit_index(short local_player_index);
real_vector3d *player_control_get_facing_direction(short local_player_index, real_vector3d *direction);
real player_control_get_field_of_view(short local_player_index);
real_euler_angles2d const *player_control_get_facing_angles(short local_player_index);
long player_control_get_aiming_unit_index(short local_player_index);
long player_control_get_target_object_index(short local_player_index);
void player_control_get_unit_camera_info(short local_player_index, struct unit_camera_info *camera_info);
real player_control_get_autoaim_level(short local_player_index);
short player_control_get_zoom_level(short local_player_index);
void player_control_permanent_impulse(short local_player_index, real_euler_angles2d const *impulse);
short unit_get_local_player_index(long unit_index);
void player_control_set_facing(short local_player_index, real_vector3d const *facing);

/* ---------- prototypes/PLAYERS.C */

long *machine_get_player_list(long machine_index);
void player_aiming_vector_from_facing(long player_index, real_vector3d *aiming_vector, real_euler_angles2d const *desired_facing);
void players_set_local_player_unit(short local_player_index, long unit_index);
void players_initialize(void);
void players_initialize_for_new_map(void);
void players_dispose_from_old_map(void);
void players_dispose(void);
void players_update_before_game(void);
void players_update_after_game(void);
boolean any_player_is_in_the_air(void);
boolean any_player_is_dead(void);

long player_new(long machine_index, long player_index, short local_player_index, struct network_player *network_player_data);
void local_player_set_player_index(short local_player_index, long player_index);
boolean players_respawn_coop(void);
void players_handle_deleted_object(long deleted_object_index);
short players_get_respawn_failure(void);
short player_get_starting_location_count(void);
struct scenario_player *player_get_starting_location(short starting_location_index);

boolean local_player_exists(long local_player_index);
short local_player_count(void);
short local_player_get_next(short local_player_index);
boolean players_are_all_dead(void);
long local_player_get_player_index(short local_player_index);

long player_index_from_unit_index(long unit_index);
long local_player_get_player_index(short local_player_index);
short local_player_get_next(short local_player_index);

unsigned long const *players_get_combined_pvs_local(void);
unsigned long const *players_get_combined_pvs(void);

void player_control_fix_for_loaded_game_state(void);
void players_reconnect_to_structure_bsp(void);

void player_input_enable(boolean enable);
boolean player_input_enabled(void);
long find_unused_local_player_index(void);
void player_delete(long player_index);
void player_died(long player_index);
void player_add_equipment(long unit_index, short profile_index, boolean reset);
void placement_data_set_change_color(struct object_placement_data *placement_data, real_rgb_color const *color);
long find_best_starting_location_index(long player_index);
boolean unit_should_autopick_weapon(long unit_index, long weapon_index);
boolean player_teleport(long player_index, long source_unit_index, real_point3d const *position);
boolean player_handle_powerup(long player_index, short powerup_type, short duration_in_ticks);
void player_handle_powerup_minor(long player_index, short powerup_type, short minor_duration);
void players_debug_render(void);
void debug_player_teleport(short source_local_player_index, short destination_local_player_index);

/* ---------- prototypes/PLAYER_RUMBLE.C */

void rumble_initialize(void);
void rumble_dispose(void);
void rumble_initialize_for_new_map(void);
void rumble_dispose_from_old_map(void);
void rumble_update(void);
void rumble_player_clear(short local_player_index);
void rumble_clear_all_now(void);

/* ---------- prototypes/PLAYER_QUEUES_NEW.C */

void update_server_add_player(long player_index);
void update_client_add_player(long player_index);
void update_client_handle_server_update(struct game_update *new_update, long update_number);
void update_server_next_update(void);
void update_server_build_server_update(long machine_index, struct game_update *update, long *update_number);
void update_server_handle_client_update(long machine_index, struct player_action *actions);
void update_client_build_client_update(struct player_action_collection *action_collection);
boolean update_client_dequeue(struct player_action *actions);
void update_client_queue(struct player_action *action);
void update_client_queue_push(void);

void update_server_delete(void);
boolean update_server_new(void);
void update_server_start(void);
void update_client_start(void);
void update_client_delete(void);
boolean update_client_new(void);
long update_client_get_maximum_actions(void);
long update_client_get_maximum_possible_server_time(void);
void update_client_local_ticks(short ticks);

void update_queues_reset_and_fill_with_lies(void);

/* ---------- prototypes/PLAYER_RUMBLE.C */

void rumble_player_set_scripted_values(real left, real right);
void rumble_player_set_scale(real scale);
void rumble_player_impulse(short local_player_index, struct rumble_definition const *rumble, real scale, real time_scale);
void rumble_player_clear(short local_player_index);
void rumble_player_continuous(short local_player_index, real left, real right);

/* ---------- globals */

extern struct data_array *player_data;

extern real player_look_yaw_rate[];
extern real player_look_pitch_rate[];

/* ---------- public code */

#endif // __PLAYERS_H
