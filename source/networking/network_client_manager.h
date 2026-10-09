/*
NETWORK_CLIENT_MANAGER.H

header included in hcex build.
*/

#ifndef __NETWORK_CLIENT_MANAGER_H
#define __NETWORK_CLIENT_MANAGER_H
#pragma once

/* ---------- headers */

#include "cluster_partitions.h"
#include "network_messages.h"

/* ---------- constants */

enum
{
	_network_game_client_state_searching = 0,
	_network_game_client_state_joining,
	_network_game_client_state_pregame,
	_network_game_client_state_ingame,
	_network_game_client_state_postgame,
	NUMBER_OF_NETWORK_GAME_CLIENT_STATES
};

enum
{
	_network_game_client_error_none = 0,
	_network_game_client_error_unknown,
	_network_game_client_error_bad_password,
	_network_game_client_error_bad_version,
	_network_game_client_error_bad_join_token,
	_network_game_client_error_game_full,
	_network_game_client_error_game_closed,
	_network_game_client_error_connection_lost,
	_network_game_client_error_host_closed_down,
	NUMBER_OF_NETWORK_GAME_CLIENT_ERROR_CODES
};

enum
{
	_network_game_client_connected_to_server_bit = 0,
	_network_game_client_sent_join_request_to_server_bit,
	NUMBER_OF_NETWORK_GAME_CLIENT_FLAGS
};

enum
{
	MAXIMUM_NETWORK_ADVERTISED_GAMES = 9, /* fake name */
	NETWORK_GAME_CLIENT_CONNECTION_PROCESS_TIMEOUT_MILLISECONDS_DEFAULT = 120000,
	NETWORK_GAME_CLIENT_GAME_SEARCH_INTERVAL_MILLISECONDS = 2000,
	NETWORK_GAME_CLIENT_GAME_ADVERTISED_GAME_TIMEOUT = 6000,
	NETWORK_GAME_CLIENT_GAME_PING_INTERVAL_MILLISECONDS = 1000,
	NETWORK_GAME_CLIENT_INITIAL_PACKET_SEQUENCE_NUMBER = 1,
	NETWORK_GAME_CLIENT_MESSAGE_BUFFER_SIZE = 2048, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct network_game_join_parameters
{
	word version;
	wchar_t password[MAXIMUM_NETWORK_GAME_SERVER_PASSWORD_LENGTH];
	byte join_token[NETWORK_JOIN_GAME_TOKEN_SIZE];
};

struct advertised_game_data
{
	XNKID key_id;
	XNKEY key;
	XNADDR host_address;
	char server_nonce[NETWORK_GAME_NONCE_BYTES];
	long time_in_milliseconds_of_last_news;
	wchar_t name[MAXIMUM_NETWORK_GAME_NAME_LENGTH];
	struct network_map map;
	short game_engine;
	word current_number_of_machines;
	word current_number_of_players;
	word maximum_number_of_players;
	short score_to_win;
	word platform;
	boolean open;
	boolean valid;
	boolean teams_enabled;
	boolean terminator;
};

struct network_game_client
{
	word machine_index;
	struct advertised_game_data available_games[MAXIMUM_NETWORK_ADVERTISED_GAMES];
	struct transport_address ping_address; /* fake name */
	unsigned long last_ping_time; /* fake name */
	word unused; /* fake name */
	word ping_count; /* fake name */
	word average_ping; /* fake name */
	boolean pinging; /* fake name */
	struct network_connection *connection;
	struct transport_connect_process *connect_process; /* fake name */
	unsigned long connection_start_time; /* fake name */
	struct network_game_join_parameters join_parameters;
	struct network_game_data game;
	long packet_sequence_number; /* fake name */
	unsigned long last_broadcast_search_time; /* fake name */
	unsigned long next_update_number;
	unsigned long last_update_time; /* fake name */
	long last_precache_time; /* fake name */
	short seconds_to_game_start;
	word state;
	word error;
	word flags;
	boolean out_of_sync; /* fake name */
	boolean connection_going_stale; /* fake name */
};

/* ---------- prototypes/NETWORK_CLIENT_MANAGER.C */

struct network_game_client *network_game_client_create(void);
void network_game_client_dispose(struct network_game_client *client);
boolean network_game_client_idle(struct network_game_client *client);
void network_game_client_keep_alive(struct network_game_client *client);
word network_game_client_get_state(struct network_game_client *client, word *progress);
boolean network_game_client_initiate_join_game(struct network_game_client *client, struct advertised_game_data *game, struct network_game_join_parameters *join_parameters, struct transport_address *address);
boolean network_game_client_leave_game(struct network_game_client *client);
boolean network_game_client_set_machine(struct network_game_client *client, struct network_machine *machine);
struct network_machine *network_game_client_get_machine(struct network_game_client *client);
short network_game_client_get_machine_index(struct network_game_client *client);
boolean network_game_client_request_remove_player(struct network_game_client *client, struct network_player *player);
boolean network_game_client_remove_player(struct network_game_client *client, struct network_player *player, long quit_out_of_game_time);
struct advertised_game_data *network_game_client_get_available_games(struct network_game_client *client);
word network_game_client_get_error(struct network_game_client *client);
short network_game_client_get_seconds_to_game_start(struct network_game_client *client);
boolean network_game_client_write(struct network_connection *connection, message_header *message, word message_size, struct transport_address *address, boolean reliable);
boolean network_game_client_address_matches_server(struct network_game_client *client, struct transport_address *address);
void network_game_client_game_out_of_sync(struct network_game_client *client);
void network_game_client_new_advertised_game(struct network_game_client *client, struct message_server_game_advertise *message_packet);
void network_game_client_ponged(struct network_game_client *client, struct transport_address *source_address, unsigned long timestamp);
void network_game_client_accepted_into_game(struct network_game_client *client, struct transport_address *source_address, struct message_server_machine_accepted *message_packet);
void network_game_client_rejected_by_game(struct network_game_client *client, struct transport_address *source_address, word rejection_code);
boolean network_game_client_game_settings_updated(struct network_game_client *client, struct message_server_game_settings_update *message_packet);
long unstrip_player_index(long player_index);
boolean network_game_client_game_has_started(struct network_game_client *client);
void network_game_client_game_shutdown(struct network_game_client *client);
boolean network_game_client_handle_game_update(struct network_game_client *client, struct message_server_game_update *message_packet);
boolean network_game_client_add_player_to_game(struct network_game_client *client, struct network_player *player);
void network_game_client_switch_to_postgame(struct network_game_client *client);
boolean network_game_client_switch_to_pregame(struct network_game_client *client);
struct network_connection *network_game_client_get_connection(struct network_game_client *client);
void network_game_client_get_remote_server_address(struct network_game_client *client, struct transport_address *address);
struct network_game_data *network_game_client_get_game(struct network_game_client *client);
boolean network_game_client_server_has_started_game(struct network_game_client *client);
long network_game_client_get_next_update_number(struct network_game_client *client);
boolean network_client_get_oos(struct network_game_client *client);
void network_game_client_reset(struct network_game_client *client, boolean teardown_connection);
boolean network_game_client_add_player(struct network_game_client *client, short local_player_index);
boolean network_game_client_update_local_player_data(struct network_game_client *client, struct network_player *player);
boolean network_game_client_request_start_time_change(struct network_game_client *client, short request_type);
void network_game_client_countdown_timer_update(struct network_game_client *client, short seconds_to_game_start);
boolean network_game_client_advertised_game_is_valid(struct advertised_game_data *advertised_game);

/* ---------- globals */

extern boolean allow_out_of_sync;

/* ---------- public code */

#endif // __NETWORK_CLIENT_MANAGER_H
