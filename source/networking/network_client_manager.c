/*
NETWORK_CLIENT_MANAGER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries_windows.h"
#include "network_client_manager.h"
#include "network_client_message_handler.h"
#include "network_connection.h"
#include "network_game_globals.h"
#include "network_game_manager.h"
#include "network_messages.h"
#include "game.h"
#include "players.h"
#include "interface.h"
#include "ui_widget.h"

/* ---------- constants */

enum
{
	_network_game_client_connected_to_server_bit= 0,
	_network_game_client_sent_join_request_to_server_bit,
	NUMBER_OF_NETWORK_GAME_CLIENT_FLAGS,

	ADVERTISED_GAME_TIMEOUT= 6000, // milliseconds a game stays valid after its last advertisement
	PRECACHE_POLL_INTERVAL= 1000, // milliseconds between precache progress reports
	BROADCAST_SEARCH_INTERVAL= 2000, // milliseconds between broadcast game searches
	PING_INTERVAL= 1000, // milliseconds between pings to the server we are watching
	JOIN_TIMEOUT= 120000, // milliseconds a join attempt is given before it is abandoned
	MAXIMUM_INCOMING_MESSAGE_SIZE= 2048
};

/* ---------- macros */

/* ---------- structures */

struct network_game_client
{
	word machine_index; // 0x000, NONE until the server accepts us
	byte pad_002[2]; // 0x002, aligns the advertised game array
	advertised_game_data advertised_games[MAXIMUM_ADVERTISED_GAMES]; // 0x004
	transport_address server_address; // 0x808, pongs from any other address are discarded
	long last_ping_time; // 0x820, system_milliseconds() of the last ping we sent
	byte unknown_824[2]; // 0x824
	word ping_sample_count; // 0x826, divisor of the running average below
	word average_ping; // 0x828, milliseconds
	boolean accepting_pongs; // 0x82A, clear: network_game_client_ponged discards the sample
	byte pad_82B; // 0x82B
	struct network_connection *connection; // 0x82C
	transport_connect_process_ref connect_process; // 0x830, live only while a join is in flight
	unsigned long join_start_time; // 0x834, system_milliseconds() when the join began
	network_game_join_parameters join_parameters; // 0x838, copied whole from the caller
	byte pad_85A[2]; // 0x85A
	network_game_data game; // 0x85C
	long unknown_C90; // 0x0C90, set when a join begins and before a teardown
	long last_broadcast_time; // 0x0C94, system_milliseconds() of the last broadcast game search
	unsigned long next_update_number; // 0x0C98, also the test for whether the server has started
	long last_update_time; // 0x0C9C, system_milliseconds() of the last server update we handled
	long precache_poll_time; // 0x0CA0, system_milliseconds() of the last precache progress report
	short seconds_to_game_start; // 0x0CA4, NONE outside the pregame countdown
	word state; // 0x0CA6, one of the _network_game_client_state_* values
	word error; // 0x0CA8, one of the _network_game_client_error_* values
	word flags; // 0x0CAA
	boolean out_of_sync; // 0x0CAC
	boolean connection_going_stale; // 0x0CAD, set from network_connection_going_stale() by the ingame handler
	byte pad_CAE[2]; // 0x0CAE, rounds the record out to 0xCB0
}; // 0xCB0

/* ---------- prototypes */

static boolean check_networking_and_generate_error(void);
static boolean add_advertised_game(advertised_game_data *list, message_server_game_advertise *message);
static boolean add_advertised_game(
	advertised_game_data *list,
	message_server_game_advertise *message)
{
	boolean success = FALSE;
	boolean game_to_add_is_open = TEST_FLAG(message->flags, _game_advertise_open_bit) && (message->current_number_of_machines < MAXIMUM_NETWORK_MACHINE_COUNT);
	long itr;
	advertised_game_data *item = NULL;

	for(itr = 0; itr < MAXIMUM_ADVERTISED_GAMES; itr++)
	{
		advertised_game_data *current= list + itr;

		if (!network_game_client_advertised_game_is_valid(current))
		{
			csmemset(current, 0, sizeof(*current));
		}
	}

	for(itr = 0; itr < MAXIMUM_ADVERTISED_GAMES; itr++)
	{
		advertised_game_data *current= list + itr;

		if (transport_nonce_is_equal(current->server_nonce, message->server_nonce))
		{
			item = current;
			break;
		}
	}

	if (NULL == item)
	{
		for(itr = 0; itr < MAXIMUM_ADVERTISED_GAMES; itr++)
		{
			advertised_game_data *current= list + itr;

			if (!(current->valid))
			{
				item = current;
				break;
			}
		}

		if ((NULL == item) && (game_to_add_is_open))
		{
			for(itr = 0; itr < MAXIMUM_ADVERTISED_GAMES; itr++)
			{
				advertised_game_data *current = list + itr;

				match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1567, current->valid);

				if (!(current->open))
				{
					item = current;
					csmemset(item, 0, sizeof(*item));

					break;
				}
			}
		}
	}

	if (item)
	{
		item->valid = TRUE;

		item->key= message->key;
		item->key_id= message->key_id;
		item->xnet_address= message->host_address;
		csmemcpy(item->server_nonce, message->server_nonce, sizeof(message->server_nonce));
		item->time_in_milliseconds_of_last_news = system_milliseconds();

		item->platform= message->platform;
		if (message->name[0] != L'\0')
		{
			ustrncpy(item->name, message->name, MAXIMUM_NETWORK_GAME_NAME_LENGTH-1);
		}
		else
		{
			ustrncpy(item->name, L"???", MAXIMUM_NETWORK_GAME_NAME_LENGTH-1);
		}
		item->name[MAXIMUM_NETWORK_GAME_NAME_LENGTH-1]= L'\0';
		item->game_engine= message->game_engine;
		csmemcpy(&item->map, &message->map, sizeof(message->map));
		item->current_number_of_machines= message->current_number_of_machines;
		item->current_number_of_players= message->current_number_of_players;
		item->maximum_number_of_players= message->maximum_number_of_players;
		item->score_to_win= message->score_to_win;
		item->open= game_to_add_is_open;
		item->teams_enabled= TEST_FLAG(message->flags, _game_advertise_teams_enabled_bit);
		item->terminator= (item->game_engine == _game_engine_oddball) && TEST_FLAG(message->flags, _game_advertise_oddball_terminator_bit);

		success= TRUE;

		network_event("there is %s %s net game with %d players and %d machines",
			(item->open ? "an open" : "a closed"),
			(item->platform == _game_platform_xbox) ? "XBox" :
				((item->platform == _game_platform_mswindows) ? "PC" : "<unknown platform>"),
			item->current_number_of_players, item->current_number_of_machines);
	}
	else
	{
		error(_error_silent, "not fatal, but we have to many active network games cannot add more to the list");
	}

	return success;
}

static void network_game_client_set_error(network_game_client *client, word error);
static void network_game_client_update_precache_status(network_game_client *client);
static boolean network_game_client_process_incoming_messages(network_game_client *client);
static boolean network_game_client_idle_searching(
	network_game_client *client)
{
	boolean result= TRUE;
	unsigned long now= system_milliseconds();

	network_game_client_keep_alive(client);

	result = check_networking_and_generate_error();

	if (result == TRUE)
	{
		if (global_network_game_server_get())
		{
			advertised_game_data game= {0};
			network_game_join_parameters join_parameters;
			transport_address address;

			address.address.ipv4_address= IPV4_LOOPBACK_ADDRESS;
			address.port= NETWORK_SERVER_PORT;
			address.address_length = IPV4_ADDRESS_LENGTH;

			game.platform= network_game_get_local_platform();
			transport_get_nonce(game.server_nonce, sizeof(game.server_nonce));

			join_parameters.password[0]= L'\0';
			network_game_generate_join_game_token(join_parameters.join_token);
			if (!network_game_client_initiate_join_game(client, &game, &join_parameters, &address))
			{
				result = FALSE;
				display_error_when_main_menu_loaded(_error_network_failed_to_join_game);
				network_event("network_game_client_initiate_join_game() failed");
			}
		}
		else if ((result= network_connection_idle(client->connection, _connection_timeout_fast, NULL)) == FALSE)
		{
			display_error_when_main_menu_loaded(_error_network_failed_to_join_game);
			network_event("network_connection_idle() failed in network_game_client_idle_searching()");
		}
		else if ((result= network_game_client_process_incoming_messages(client)) == FALSE)
		{
			network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_searching()");
		}
		else if ((now - client->last_broadcast_time) > BROADCAST_SEARCH_INTERVAL)
		{
			if (global_network_game_server_get() == NULL)
			{
				message_client_broadcast_game_search message_packet;
				transport_address broadcast_address;
				message_header *message;

				message_packet.port= NETWORK_CLIENT_PORT;
				message_packet.version= NETWORK_GAME_MESSAGE_VERSION;
				transport_get_nonce(message_packet.nonce, sizeof(message_packet.nonce));

				broadcast_address.address_length= IPV4_ADDRESS_LENGTH;
				broadcast_address.address.ipv4_address= IPV4_BROADCAST_ADDRESS;
				broadcast_address.port= NETWORK_SERVER_PORT;

				if ((message= create_network_game_message(_message_type_client_broadcast_game_search, &message_packet, sizeof(message_packet))) != NULL)
				{
					if ((result= network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), &broadcast_address, FALSE)) == TRUE)
					{
						network_event("sent out a broadcast game search packet");
						client->last_broadcast_time= now;
					}
					else
					{
						network_event("network_game_client_write() failed while sending a message_client_broadcast_game_search message");
					}
				}
				else
				{
					network_event("failed to create a message_client_broadcast_game_search message");
				}
			}
		}
		else if ((client->accepting_pongs == TRUE) &&
			((now - client->last_ping_time) > PING_INTERVAL))
		{
			message_client_ping ping_packet;
			message_header *message;

			ping_packet.system_milliseconds= now;
			ping_packet.reply_to_port= NETWORK_CLIENT_PORT;

			if ((message= create_network_game_message(_message_type_client_ping, &ping_packet, sizeof(ping_packet))) != NULL)
			{
				if (network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), &client->server_address, FALSE))
				{
					client->last_ping_time= now;
				}
				else
				{
					network_event("network_game_client_write() failed while sending a message_client_ping message");
				}
			}
			else
			{
				network_event("failed to create a message_client_ping message");
			}
		}
	}

	return result;
}

static boolean network_game_client_idle_joining(
	network_game_client *client)
{
	boolean success= TRUE;

	success = check_networking_and_generate_error();

	if (success == TRUE)
	{
		if (network_connection_connected(client->connection))
		{
			if (!(client->flags & FLAG(_network_game_client_sent_join_request_to_server_bit)))
			{
				message_client_join_game_request message_packet;
				message_header *message;

				csmemset(&message_packet, 0, sizeof(message_packet));

				network_game_generate_local_machine_name(message_packet.machine_name);
				csmemcpy(message_packet.join_token, client->join_parameters.join_token, sizeof(message_packet.join_token));
				
				if ((message= create_network_game_message(_message_type_client_join_game_request, &message_packet, sizeof(message_packet))) != NULL)
				{
					if (network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
					{
						client->flags|= FLAG(_network_game_client_sent_join_request_to_server_bit);
					}
					else
					{
						network_event("network_game_client_write() failed to send a message_client_join_game_request message");
					}
				}
				else
				{
					network_event("failed to create a message_client_join_game_request message");
				}
			}
			client->connect_process= NULL;
		}
		else if (client->connect_process)
		{
			if ((system_milliseconds() - client->join_start_time) > JOIN_TIMEOUT)
			{
				network_event("client connection process has timed out; aborting connection attempt");
				cancel_connect_process(client->connect_process);
				client->connect_process= NULL;
				success= FALSE;
			}
		}
	}

	if (success == TRUE)
	{
		if (success= network_connection_idle(client->connection, _connection_timeout_fast, NULL))
		{
			if ((success= network_game_client_process_incoming_messages(client)) == FALSE)
			{
				network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_joining()");
			}
		}
		else
		{
			network_event("network_connection_idle() failed in network_game_client_idle_joining()");
		}
	}

	return success;
}

static boolean network_game_client_idle_pregame(
	network_game_client *client)
{
	boolean result= TRUE;

	result = check_networking_and_generate_error();
	if (!result)
	{
		goto exit;
	}

	if (!network_connection_active(client->connection) || !network_connection_connected(client->connection))
	{
		result= FALSE;
	}
	else if (result)
	{
		network_game_client_update_precache_status(client);

		result= network_connection_idle(client->connection, _connection_timeout, NULL);

		if (!result)
		{
			network_event("network_connection_idle() failed in network_game_client_idle_pregame()");
			goto exit;
		}

		result= network_game_client_process_incoming_messages(client);

		if (!result)
		{
			network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_pregame()");
			goto exit;
		}

		if (client->state == _network_game_client_state_pregame)
		{
		}
	}

exit:
	if ((!result) && (!network_connection_active(client->connection)))
	{
		display_error_when_main_menu_loaded(_error_network_server_shut_down);
		result= FALSE;
	}

	return result;
}

static boolean network_game_client_idle_searching(network_game_client *client);
static boolean network_game_client_idle_joining(network_game_client *client);
static boolean network_game_client_idle_pregame(network_game_client *client);
static boolean network_game_client_idle_ingame(network_game_client *client);
static boolean network_game_client_idle_ingame(
	network_game_client *client)
{
	boolean success= TRUE;
	
	if (!network_connection_active(client->connection) || !network_connection_connected(client->connection))
	{
		error(_error_silent, "new idle in game abort hit");
		display_error_when_main_menu_loaded(_error_network_server_shut_down);
		success= FALSE;
	}
	else if (!network_game_is_splitscreen_local())
	{
		boolean connection_stale= network_connection_going_stale(client->connection);

		if (!transport_network_available())
		{
			display_error_when_main_menu_loaded(_error_network_connection_lost);
			network_event("network connection went down (idle in game)!");
			success= FALSE;
		}
		else if (connection_stale && !client->connection_going_stale)
		{
			short player_index;

			for (player_index= local_player_get_next(NONE); player_index != NONE; player_index= local_player_get_next(player_index))
			{
				display_error(_error_network_trouble_is_brewing, player_index, FALSE, FALSE);
			}
			network_event("network client connection has been silent for a dangerously long amount of time");
		}
		client->connection_going_stale= connection_stale;
	}
	if (success == TRUE)
	{
		success= network_connection_idle(client->connection, _connection_timeout, NULL);

		if (success)
		{
			success= network_game_client_process_incoming_messages(client);

			if (success)
			{
			}
			else
			{
				network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_ingame()");
			}
		}
		else
		{
			if (!network_connection_active(client->connection) || !network_connection_connected(client->connection))
			{
				error(_error_silent, "new2 idle in game abort hit");
				display_error_when_main_menu_loaded(_error_network_server_shut_down);
				success= FALSE;
			}

			network_event("network_connection_idle() failed in network_game_client_idle_ingame()");
		}
	}

	return success;
}

static boolean network_game_client_idle_postgame(network_game_client *client);

/* ---------- globals */

boolean allow_out_of_sync;
static boolean network_game_client_dont_use_directly_in_use;
static struct network_game_client network_game_client_dont_use_directly;

/* ---------- private code */

static boolean check_networking_and_generate_error(
	void)
{
	boolean result= TRUE;

	if (!network_game_is_splitscreen_local())
	{
		result= transport_network_available();
		if (!result)
		{
			error(_error_silent, "network connection went down!");
			display_error_when_main_menu_loaded(_error_network_connection_lost);
		}
	}

	return result;
}

/* ---------- public code */

void network_game_client_dispose(
	network_game_client *client)
{
	if (client)
	{
		if (client->connection)
		{
			network_connection_delete(client->connection);
		}

		match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 178, network_game_client_dont_use_directly_in_use);

		network_game_client_dont_use_directly_in_use= FALSE;
	}

	network_event("network client disposed");
}

void network_game_client_keep_alive(
	struct network_game_client *client)
{
	network_connection_keep_alive(client->connection);
}

word network_game_client_get_state(
	network_game_client *client,
	word *progress)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 249, client);

	if (progress)
	{
		*progress= 0;
		if (client->state==_network_game_client_state_joining)
		{
			*progress= (word)((100*system_milliseconds() - 100*client->join_start_time)/JOIN_TIMEOUT);
		}
	}

	return client->state;
}

boolean network_game_client_initiate_join_game(
	network_game_client *client,
	struct advertised_game_data *game,
	network_game_join_parameters *join_parameters,
	transport_address *address)
{
	boolean result;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 343, client && (client->state == _network_game_client_state_searching) && game && join_parameters && client->connection && !network_connection_connected(client->connection) && (game->platform == network_game_get_local_platform()));

	client->unknown_C90= 1;
	client->connect_process= NULL;
	client->join_start_time= system_milliseconds();
	csmemcpy(&client->join_parameters, join_parameters, sizeof(network_game_join_parameters));

	result= network_connection_connect(client->connection, address, NULL);
	if (result==TRUE)
	{
		client->state= _network_game_client_state_joining;
		network_event("attempting to connect to game @ %s", transport_address_to_string(address));

		return TRUE;
	}

	display_error_when_main_menu_loaded(_network_game_client_error_host_closed_down-1);
	network_event("failed attempt to initiate a connection to game @ %s", transport_address_to_string(address));

	return result;
}

boolean network_game_client_set_machine(
	network_game_client *client,
	network_machine *machine)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 481, client && (client->machine_index<MAXIMUM_NETWORK_MACHINE_COUNT) && network_machine_is_valid(machine));

	csmemcpy(&client->game.machines[client->machine_index], machine, sizeof(network_machine));

	return TRUE;
}

network_machine *network_game_client_get_machine(
	network_game_client *client)
{
	if (client && client->machine_index<MAXIMUM_NETWORK_MACHINE_COUNT)
	{
		return &client->game.machines[client->machine_index];
	}

	return NULL;
}

short network_game_client_get_machine_index(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 509, client);

	return client->machine_index;
}

advertised_game_data *network_game_client_get_available_games(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 684, client);

	return client->advertised_games;
}

word network_game_client_get_error(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 692, client);

	return client->error;
}

short network_game_client_get_seconds_to_game_start(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 700, client);

	return client->seconds_to_game_start;
}

boolean network_game_client_write(
	struct network_connection *connection,
	message_header *message,
	unsigned short message_size,
	transport_address *dest_address,
	boolean reliable)
{
	return network_connection_write(connection, message, message_size, dest_address, reliable);
}

boolean network_game_client_address_matches_server(
	network_game_client *client,
	transport_address *address)
{
	transport_address remote_address;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 722, client != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 723, client->connection);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 724, address != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 725, address->address.ipv4_address);

	network_connection_get_address(client->connection, &remote_address, NULL);

	return (boolean)(remote_address.address.ipv4_address==address->address.ipv4_address);
}

void network_game_client_game_out_of_sync(
	network_game_client *client)
{
	if (!allow_out_of_sync)
	{
		network_event("local machine is out of sync with the server");
		if (client->out_of_sync == FALSE)
		{
			short player_index;

			for (player_index= local_player_get_next(NONE); player_index != NONE; player_index= local_player_get_next(player_index))
			{
				display_error(_error_network_out_of_sync_alert, player_index, TRUE, FALSE);
			}
		}
		client->out_of_sync= TRUE;
	}

	return;
}

void network_game_client_ponged(
	network_game_client *client,
	transport_address *source_address,
	unsigned long sent_time_system_milliseconds)
{
	unsigned long now;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 775, client && source_address);

	if (client->accepting_pongs && client->server_address.address.ipv4_address==source_address->address.ipv4_address)
	{
		now= system_milliseconds();
		if (sent_time_system_milliseconds<=now)
		{
			client->average_ping= (word)((now + client->ping_sample_count*client->average_ping - sent_time_system_milliseconds)/(client->ping_sample_count + 1));
			client->ping_sample_count++;
		}
		else
		{
			network_event("received a pong from the future");
		}
	}
	else
	{
		network_event("received a pong from a system we aren't interested in");
	}
}

void network_game_client_accepted_into_game(
	network_game_client *client,
	transport_address *source_address,
	message_server_machine_accepted *message_packet)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 807, client && source_address && message_packet && (client->state == _network_game_client_state_joining));

	if ((message_packet->machine_index>=0) && (message_packet->machine_index<MAXIMUM_NETWORK_MACHINE_COUNT))
	{
		message_client_settings_request settings_request;
		message_header *message;

		client->machine_index= message_packet->machine_index;
		client->game.machines[message_packet->machine_index].machine_index= message_packet->machine_index;
		client->state= _network_game_client_state_pregame;

		network_game_set_random_seed(message_packet->server_random_seed);

		network_event("successfully joined a net game; our machine is #%d", message_packet->machine_index);

		network_game_generate_local_machine_name(settings_request.machine.name);
		settings_request.machine.machine_index= message_packet->machine_index;

		message= create_network_game_message(_message_type_client_settings_request, &settings_request, sizeof(settings_request));
		if (message)
		{
			if (!network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
			{
				network_event("network_game_client_write() failed while sending a message_client_settings_request message");
			}
		}
		else
		{
			network_event("failed to create a message_client_settings_request message");
		}
	}
	else
	{
		network_event("received a message_server_machine_accepted message with a bad machine_index");
	}
}

boolean network_game_client_game_settings_updated(
	network_game_client *client,
	message_server_game_settings_update *message_packet)
{
	boolean success;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 885, client && message_packet);

	if ((message_packet->game.machine_count>=0) && (message_packet->game.machine_count<=MAXIMUM_NETWORK_MACHINE_COUNT) &&
		(message_packet->game.player_count>=0) && (message_packet->game.player_count<=NETWORK_GAME_MAXIMUM_PLAYER_COUNT))
	{
		network_game_data game;

		if (csstrcmp(message_packet->game.map.name, client->game.map.name) != 0)
		{
			network_event("precaching map '%s'...", message_packet->game.map.name);
			main_set_multiplayer_map_name(message_packet->game.map.name);
		}

		csmemcpy(&game, &client->game, sizeof(network_game_data));
		csmemcpy(&client->game, &message_packet->game, sizeof(network_game_data));
		csmemcpy(&client->game.local_data, &game.local_data, sizeof(game.local_data));

		network_event("received updated game settings from the server; there are %d players on %d machines in the game",
			message_packet->game.player_count, message_packet->game.machine_count);
		network_event("player count %d machine count %d", message_packet->game.player_count, message_packet->game.machine_count);

		success= TRUE;
	}
	else
	{
		network_event("invalid message_server_game_settings_update message received player count %d machine count %d", message_packet->game.player_count, message_packet->game.machine_count);
		success= FALSE;
	}

	return success;
}

boolean network_game_client_game_has_started(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 944, client && (client->state == _network_game_client_state_pregame));

	client->seconds_to_game_start = NONE;

	network_game_client_keep_alive(client);

	if (network_game_create_game_objects(&client->game))
	{
		int i, local_player_index;

		for (i= local_player_index= 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i++)
		{
			if (client->game.players[i].machine_index == client->machine_index)
			{
				while ((client->game.players[i].machine_index == client->machine_index) &&
					network_player_is_valid(&client->game.players[i]))
				{
					local_player_set_player_index(client->game.players[i].controller_index, unstrip_player_index(client->game.players[i].player_list_index));
					i+= 1;
				}
				break;
			}
		}

		network_game_client_keep_alive(client);

		{
			message_client_loaded message_packet= {0};
			message_header *message= create_network_game_message(_message_type_client_loaded, &message_packet, sizeof(message_packet));

			if (message != NULL)
			{
				if (network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
				{
					network_event("local machine is loaded & ready to play");
					client->state= _network_game_client_state_ingame;
					client->next_update_number= 0;
					client->last_update_time= 0;
					client->connection_going_stale= FALSE;

					ui_widgets_close_all();
					game_time_start();
					game_initial_pulse();
				}
				else
				{
					network_event("network_game_client_write() failed while sending a message_client_loaded message");
				}
			}
			else
			{
				network_event("failed to create a message_client_loaded message");
			}
		}
	}
	else
	{
		network_event("failed to load the necessary game data");
	}

	return (client->state == _network_game_client_state_ingame) ? TRUE : FALSE;
}

long unstrip_player_index(
	long stripped_player_index)
{
	long result= NONE;
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);

	while (player= data_iterator_next(&iterator))
	{
		if ((iterator.index & 0xffff) == (stripped_player_index & 0xffff))
		{
			result= iterator.index;

			break;
		}
	}

	return result;
}

boolean network_game_client_handle_game_update(
	network_game_client *client,
	message_server_game_update *message_packet)
{
	boolean success= FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1037, client && message_packet);

	if (message_packet->player_count < client->game.player_count)
	{
		csmemset(&message_packet->action_update[message_packet->player_count], 0,
			(client->game.player_count-message_packet->player_count)*sizeof(struct player_action));
		message_packet->player_count= client->game.player_count;
	}
	{
		struct game_update update;

		if (message_packet->update_number != client->next_update_number)
		{
			network_event("out of sync: missed a server update (expected #%ld, got #%ld)", client->next_update_number, message_packet->update_number);
			network_game_client_game_out_of_sync(client);
		}
		else if (!global_network_game_server_get())
		{
			if (game_time_get() == (long)message_packet->update_number)
			{
				if (message_packet->debug_game_time != game_time_get())
				{
					network_event("not a bug, but update %d time %d our time %d", message_packet->update_number, message_packet->debug_game_time, game_time_get());
				}
			}

			if (game_time_get() == message_packet->debug_game_time)
			{
				if (message_packet->debug_random_seed != get_random_seed())
				{
					network_event("out of sync: client/server random seed mismatch, update= #%ld, game time= #%ld (%ld) (#%lx/#%lx)",
						message_packet->update_number, game_time_get(), message_packet->debug_game_time, get_random_seed(), message_packet->debug_random_seed);
					network_game_client_game_out_of_sync(client);
				}
			}

			if ((message_packet->update_number % TICKS_PER_SECOND) == 0)
			{
				network_event("client is lagging behind the server by #%d game ticks", message_packet->update_number - game_time_get());
			}
		}

		update.number_of_actions= message_packet->player_count;
		csmemcpy(update.actions, message_packet->action_update, update.number_of_actions*sizeof(struct player_action));

		update_client_handle_server_update(&update, message_packet->update_number);

		client->next_update_number+= 1;
		client->last_update_time= system_milliseconds();

		success= TRUE;
	}

	return success;
}

boolean network_game_client_add_player_to_game(
	network_game_client *client,
	struct network_player *player)
{
	boolean result= FALSE;
	long player_index;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1122, client && player);

	if (network_player_is_valid(player))
	{
		result= network_game_add_player(&client->game, player);
		if (result)
		{
			if (client->state==_network_game_client_state_ingame)
			{
				player= &client->game.players[client->game.player_count - 1];

				result= network_game_spawn_player(player);
				if (result)
				{
					player_index= unstrip_player_index(player->player_list_index);

					if (player->machine_index==client->machine_index)
					{
						local_player_set_player_index(player->controller_index, player_index);
					}

					update_client_add_player(player_index);

					if (global_network_game_server_get())
					{
						update_server_add_player(player_index);
					}
				}
				else
				{
					return result;
				}
			}

			network_event("added new player to the game (machine #%d / controller #%d)", player->machine_index, player->controller_index);
		}
	}

	return result;
}

void network_game_client_switch_to_postgame(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1164, client);

	game_engine_switch_to_postgame();
	client->state= _network_game_client_state_postgame;

	network_event("switching to postgame");
}

boolean network_game_client_switch_to_pregame(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1177, client);

	if (client->state!=_network_game_client_state_pregame)
	{
		network_game_reset_for_next_round(&client->game, TRUE);
		network_connection_keep_alive(client->connection);

		client->next_update_number= 0;
		client->unknown_C90= 1;
		client->last_update_time= 0;
		client->connection_going_stale= 0;
		client->state= _network_game_client_state_pregame;
		client->out_of_sync= FALSE;

		network_event("switching to pregame");
		network_game_reset_to_pregame_ui();
		network_connection_keep_alive(client->connection);
	}

	return TRUE;
}

struct network_connection *network_game_client_get_connection(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1203, client);

	return client->connection;
}

void network_game_client_get_remote_server_address(
	network_game_client *client,
	transport_address *remote_address)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1212, client);

	network_connection_get_address(client->connection, remote_address, NULL);
}

network_game_data *network_game_client_get_game(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1229, client);

	return &client->game;
}

boolean network_game_client_server_has_started_game(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1237, client);

	return (client->next_update_number > 0) ? TRUE : FALSE;
}

long network_game_client_get_next_update_number(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1245, client);

	return client->next_update_number;
}

boolean network_client_get_oos(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1253, client);

	return client->out_of_sync;
}

boolean network_game_client_add_player(
	network_game_client *client,
	short local_player_index)
{
	boolean result= TRUE;
	struct player_profile profile;
	message_client_add_player_request_pregame add_player_request;
	struct network_player player;
	message_header *message;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1328, client && (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	player_ui_get_active_player_profile(local_player_index, &profile);

	player.controller_index= (char)local_player_index;
	player.machine_index= (char)client->machine_index;
	ustrncpy(player.name, profile.player_name, 11);
	player.name[11]= 0;
	player.primary_color_index= profile.primary_color_index;
	player.icon_index= NONE;
	player.team_index= NONE;
	player.player_list_index= NONE;

	network_event("requesting a player addition (controller index #%d)", player.controller_index);

	switch (client->state)
	{
		case _network_game_client_state_searching:
		case _network_game_client_state_joining:
			network_event("can't add players to a game until after a game is joined");
			result= FALSE;
			break;

		case _network_game_client_state_pregame:
			csmemcpy(&add_player_request, &player, sizeof(struct network_player));
			message= create_network_game_message(_message_type_client_add_player_request_pregame, &add_player_request, sizeof(message_client_add_player_request_pregame));
			if (message)
			{
				result= network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE);
				if (!result)
				{
					network_event("network_game_client_write() failed while sending a message_client_add_player_request_pregame message");
				}
			}
			else
			{
				network_event("failed to create a message_client_add_player_request_pregame message");
			}
			break;

		case _network_game_client_state_ingame:
			csmemcpy(&add_player_request, &player, sizeof(struct network_player));
			message= create_network_game_message(_message_type_client_add_player_request_ingame, &add_player_request, sizeof(message_client_add_player_request_pregame));
			if (message)
			{
				result= network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE);
				if (!result)
				{
					network_event("network_game_client_write() failed while sending a message_client_add_player_request_ingame message");
				}
			}
			else
			{
				network_event("failed to create a message_client_add_player_request_ingame message");
			}
			break;

		case _network_game_client_state_postgame:
			network_event("client tried to add a new player in post-game");
			result= FALSE;
			break;

		default:
			network_event("client is in an unknown state");
			break;
	}

	return result;
}

boolean network_game_client_update_local_player_data(
	network_game_client *client,
	struct network_player *player)
{
	boolean result= FALSE;
	message_client_player_settings_request settings_request;
	message_header *message;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1415, client && player);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1416, player->machine_index==client->machine_index);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1417, network_player_is_valid(player));

	csmemcpy(&settings_request, player, sizeof(struct network_player));

	if (settings_request.player.team_index==NONE)
	{
		settings_request.player.team_index= 0;
	}

	message= create_network_game_message(_message_type_client_player_settings_request, &settings_request, sizeof(message_client_player_settings_request));
	if (message)
	{
		if (network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
		{
			result= TRUE;
		}
		else
		{
			network_event("network_game_client_update_local_player_data() failed while sending a message_client_player_settings_request message");
		}
	}

	return result;
}

boolean network_game_client_request_start_time_change(
	network_game_client *client,
	short request_type)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1445, client);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1446, (request_type>=0) && (request_type<NUMBER_OF_GAME_START_REQUESTS));

	if (client->state == _network_game_client_state_pregame)
	{
		message_client_game_start_request message_packet;
		message_header *message;

		message_packet.request= request_type;
		message= create_network_game_message(_message_type_client_game_start_request, &message_packet, sizeof(message_packet));
		if (message != NULL)
		{
			if (!network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
			{
				network_event("network_game_client_request_start_time_change() failed to send a message_client_game_start_request message");
			}
		}
	}
	else
	{
		network_event("failed to send a message_client_game_start_request because we are not in the pregame state");
	}

	return TRUE;
}

void network_game_client_countdown_timer_update(
	network_game_client *client,
	short seconds_to_start)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1475, client);

	client->seconds_to_game_start= seconds_to_start;
}

boolean network_game_client_advertised_game_is_valid(
	struct advertised_game_data *advertised_game)
{
	boolean result= TRUE;

	if (!advertised_game->valid ||
		(long)(system_milliseconds() - advertised_game->time_in_milliseconds_of_last_news)>ADVERTISED_GAME_TIMEOUT)
	{
		result= FALSE;
	}

	return result;
}

static void network_game_client_set_error(
	network_game_client *client,
	word error)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1634, client);

	if (error >= NUMBER_OF_NETWORK_GAME_CLIENT_ERROR_CODES) error= _network_game_client_error_unknown;
	if (client->error == _network_game_client_error_none) client->error= error;
}

static void network_game_client_update_precache_status(
	network_game_client *client)
{
	long current_time = system_milliseconds();
	long time_between_precache_updates = MILLISECONDS_PER_SECOND;
	long time_to_send_next_precache_update = client->precache_poll_time + time_between_precache_updates;

	if (current_time > time_to_send_next_precache_update)
	{
		boolean map_is_precached;
		char *multiplayer_map_name = main_get_multiplayer_map_name();

		client->precache_poll_time = current_time;

		map_is_precached = cache_files_give_time_to_precache(multiplayer_map_name);

		if (map_is_precached)
		{
			message_client_map_is_precached_pregame precache_status= {0};
			message_header *message;

			csstrncpy(precache_status.map_name, multiplayer_map_name, sizeof(precache_status.map_name));

			message= create_network_game_message(_message_type_client_map_is_precached_pregame, &precache_status, sizeof(precache_status));

			if (message != NULL)
			{
				if (!network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
				{
					network_event("network_game_client_write() failed while sending a message_client_graceful_game_exit_pregame message");
				}
			}
		}
	}
}

static boolean network_game_client_process_incoming_messages(
	network_game_client *client)
{
	boolean result= TRUE;
	byte buffer[MAXIMUM_INCOMING_MESSAGE_SIZE];
	transport_address source_address;
	long buffer_size= MAXIMUM_INCOMING_MESSAGE_SIZE;

	do
	{
		if (!network_connection_read(client->connection, (message_header *)buffer, &buffer_size, &source_address))
		{
			break;
		}

		result= network_game_client_handle_message(client, (message_header *)buffer, (short)buffer_size, &source_address);
		if (!result)
		{
			network_event("network_game_client_handle_message() failed in network_game_client_process_incoming_messages()");
		}

		buffer_size= MAXIMUM_INCOMING_MESSAGE_SIZE;
	}
	while (result);

	return result;
}

boolean network_game_client_leave_game(
	network_game_client *client)
{
	boolean result= TRUE;
	long message_struct;
	message_header *message;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 377, client && client->connection);

	network_event("leaving network game");

	switch (client->state)
	{
		case _network_game_client_state_searching:
			match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 384, !network_connection_connected(client->connection));
			break;

		case _network_game_client_state_joining:
			if (client->connect_process)
			{
				cancel_connect_process(client->connect_process);
				client->connect_process= NULL;
			}

			if (network_connection_connected(client->connection))
			{
				result= network_connection_disconnect(client->connection);
				if (!result)
				{
					network_event("network_connection_disconnect() failed _network_game_client_state_joining");
				}
			}
			break;

		case _network_game_client_state_pregame:
			message_struct= 0;
			if (network_connection_connected(client->connection))
			{
				message= create_network_game_message(_message_type_client_graceful_game_exit_pregame, &message_struct, sizeof(long));
				if (message)
				{
					if (!network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
					{
						network_event("network_game_client_write() failed while sending a message_client_graceful_game_exit_pregame message");
					}
				}
				else
				{
					network_event("failed to create a message_client_graceful_game_exit_pregame message");
				}

				result= network_connection_disconnect(client->connection);
				if (!result)
				{
					network_event("network_connection_disconnect() failed _network_game_client_state_pregame");
				}
			}
			break;

		case _network_game_client_state_ingame:
			if (network_connection_connected(client->connection))
			{
				result= network_connection_disconnect(client->connection);
				if (!result)
				{
					network_event("network_connection_disconnect() failed _network_game_client_state_ingame");
				}
			}
			break;

		case _network_game_client_state_postgame:
			message_struct= 0;
			if (network_connection_connected(client->connection))
			{
				message= create_network_game_message(_message_type_client_graceful_game_exit_postgame, &message_struct, sizeof(long));
				if (message && !network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE))
				{
					network_event("network_game_client_write() failed while sending a message_client_graceful_game_exit_postgame message");
				}

				result= network_connection_disconnect(client->connection);
				if (!result)
				{
					network_event("network_connection_disconnect() failed _network_game_client_state_postgame");
				}
			}
			break;

		default:
			network_event("client is in an unknown state");
			break;
	}

	network_game_invalidate(&client->game);
	client->state= _network_game_client_state_searching;

	return result;
}

boolean network_game_client_request_remove_player(
	network_game_client *client,
	struct network_player *player)
{
	boolean result= TRUE;
	struct network_player player_record;
	message_header *message;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 520, client && network_player_is_valid(player));
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 521, client->game.machines[client->machine_index].machine_index==player->machine_index);

	network_event("requesting a player removal (controller index #%d)", player->controller_index);

	switch (client->state)
	{
		case _network_game_client_state_searching:
		case _network_game_client_state_joining:
			network_event("can't remove players from a game until after a game is joined");
			result= FALSE;
			break;

		case _network_game_client_state_pregame:
			csmemcpy(&player_record, player, sizeof(struct network_player));
			message= create_network_game_message(_message_type_client_remove_player_request_pregame, &player_record, sizeof(struct network_player));
			if (!message)
			{
				network_event("failed to create a message_client_remove_player_request_pregame mesage");
				result= FALSE;
				break;
			}
			result= network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE);
			break;

		case _network_game_client_state_ingame:
			csmemcpy(&player_record, player, sizeof(struct network_player));
			message= create_network_game_message(_message_type_client_remove_player_request_ingame, &player_record, sizeof(struct network_player));
			if (message)
			{
				result= network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE);
			}
			else
			{
				network_event("failed to create a message_client_remove_player_request_ingame message");
				result= FALSE;
			}
			break;

		case _network_game_client_state_postgame:
			csmemcpy(&player_record, player, sizeof(struct network_player));
			message= create_network_game_message(_message_type_client_remove_player_request_postgame, &player_record, sizeof(struct network_player));
			if (message)
			{
				result= network_game_client_write(client->connection, message, GET_MESSAGE_SIZE(*message), NULL, TRUE);
				if (!result)
				{
					network_event("network_game_client_write() failed while sending a message_client_remove_player_request_postgame message");
					result= FALSE;
				}
			}
			else
			{
				network_event("failed to create a message_client_remove_player_request_postgame message");
				result= FALSE;
			}
			break;

		default:
			network_event("client is in an unknown state");
			break;
	}

	return result;
}

boolean network_game_client_remove_player(
	network_game_client *client,
	struct network_player *player,
	long time_of_death)
{
	boolean success= FALSE;
	long game_player_index= NONE;
	int i;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 627, client && player);

	for (i= 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i++)
	{
		if (network_player_is_valid(&client->game.players[i]) &&
			(client->game.players[i].machine_index == player->machine_index) &&
			(client->game.players[i].controller_index == player->controller_index))
		{
			game_player_index= unstrip_player_index(client->game.players[i].player_list_index);
			success= network_game_remove_player(&client->game, player);
			break;
		}
	}
	if (success)
	{
		if (client->game.local_data.game_objects_loaded)
		{
			if (game_player_index && (game_player_index != NONE))
			{
				struct player_datum *player= player_get(game_player_index);

				if (NONE != time_of_death)
				{
					error(_error_silent, "%x quit of of game at tick %d (now %d)", game_player_index, time_of_death, game_time_get());
					player->quit_out_of_game_time = time_of_death;
				}

				for (i= 0; i < NETWORK_GAME_MAXIMUM_PLAYER_COUNT; i++)
				{
					if (network_player_is_valid(&client->game.players[i]) &&
						(client->game.players[i].machine_index == client->machine_index))
					{
						break;
					}
				}
				if (i == NETWORK_GAME_MAXIMUM_PLAYER_COUNT)
				{
					network_game_client_all_local_players_have_quit();
					network_event("no local players remain in the game, exiting the game now");
				}
			}
			else
			{
				error(_error_silent, "network game tried to delete a player with a phony player index (#0x%08lX)", game_player_index);
				success= FALSE;
			}
		}
	}

	return success;
}

void network_game_client_new_advertised_game(
	network_game_client *client,
	message_server_game_advertise *message_packet)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 764, client && message_packet);

	add_advertised_game(client->advertised_games, message_packet);
}

void network_game_client_game_shutdown(
	network_game_client *client)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1020, client);

	network_game_client_set_error(client, _network_game_client_error_host_closed_down);

	network_event("the game host is shutting down");
	network_game_client_all_local_players_have_quit();
}

void network_game_client_reset(
	network_game_client *client,
	boolean teardown_connection)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 1262, client);

	network_game_invalidate(&client->game);
	client->machine_index= NONE;
	client->state= _network_game_client_state_searching;
	
	if (teardown_connection && client->connection && network_connection_connected(client->connection))
	{
		client->unknown_C90= 1;
		if (network_connection_disconnect(client->connection))
		{
			client->flags&= ~FLAG(_network_game_client_connected_to_server_bit);
		}
		else
		{
			network_game_client_set_error(client, _network_game_client_error_unknown);
			network_event("failed to reinitialize network game client");
		}

	}
	
	client->error= _network_game_client_error_none;
	client->flags&= ~FLAG(_network_game_client_sent_join_request_to_server_bit);
	client->last_broadcast_time= 0;
	client->next_update_number= 0;
	client->last_update_time= 0;
	client->connection_going_stale= FALSE;
	client->seconds_to_game_start= NONE;
	client->out_of_sync= FALSE;

	return;
}

static boolean network_game_client_idle_postgame(
	network_game_client *client)
{
	boolean success;

	success = check_networking_and_generate_error();

	if (!success)
	{
		goto exit;
	}

	success= network_connection_idle(client->connection, _connection_timeout, NULL);

	if (!success)
	{
		network_event("network_connection_idle() failed in network_game_client_idle_postgame()");
		goto exit;
	}

	success= network_game_client_process_incoming_messages(client);

	if (!success)
	{
		network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_postgame()");
		goto exit;
	}

exit:
	if ((!success) && (!network_connection_active(client->connection)))
	{
		display_error_when_main_menu_loaded(_error_network_server_shut_down);
		success= FALSE;
	}

	return success;
}

network_game_client *network_game_client_create(
	void)
{
	network_game_client *client = &network_game_client_dont_use_directly;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 148, !network_game_client_dont_use_directly_in_use);
	network_game_client_dont_use_directly_in_use = TRUE;

	csmemset(client, 0, sizeof(*client));

	if (client != NULL)
	{
		if ((client->connection= network_connection_new(FLAG(_connection_create_clientside_client_bit),
			NETWORK_CLIENT_PORT)) != NULL)
		{
			network_game_client_reset(client, FALSE);
		}
		else
		{
			network_event("network_game_create_client() failed; could not create network connection");
			network_game_client_dispose(client);
			client= NULL;
		}
	}

	return client;
}

boolean network_game_client_idle(
	network_game_client *client)
{
	boolean result= FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 198, client);

	switch (client->state)
	{
		case _network_game_client_state_searching:
			result= network_game_client_idle_searching(client);
			if (!result)
			{
				network_event("network_game_client_idle_searching() failed");
			}
			break;

		case _network_game_client_state_joining:
			result= network_game_client_idle_joining(client);
			if (!result)
			{
				network_event("network_game_client_idle_joining() failed");
			}
			break;

		case _network_game_client_state_pregame:
			result= network_game_client_idle_pregame(client);
			if (!result)
			{
				network_event("network_game_client_idle_pregame() failed");
			}
			break;

		case _network_game_client_state_ingame:
			result= network_game_client_idle_ingame(client);
			if (!result)
			{
				network_event("network_game_client_idle_ingame() failed");
			}
			break;

		case _network_game_client_state_postgame:
			result= network_game_client_idle_postgame(client);
			if (!result)
			{
				network_event("network_game_client_idle_postgame() failed");
			}
			break;

		default:
			match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 238, !"unknown client state");
			break;
	}

	return result;
}

void network_game_client_rejected_by_game(
	network_game_client *client,
	transport_address *source_address,
	word reason)
{
	char *reason_string= "<unknown>";

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_manager.c", 858, client && source_address);

	client->state= _network_game_client_state_searching;

	switch (reason)
	{
		case _rejection_code_version_too_old: reason_string= "_rejection_code_version_too_old"; break;
		case _rejection_code_version_too_new: reason_string= "_rejection_code_version_too_new"; break;
		case _rejection_code_bad_join_token: reason_string= "_rejection_code_bad_join_token"; break;
		case _rejection_code_bad_password: reason_string= "_rejection_code_bad_password"; break;
		case _rejection_code_game_is_full: reason_string= "_rejection_code_game_is_full"; break;
		case _rejection_code_game_is_closed: reason_string= "_rejection_code_game_is_closed"; break;
		case _rejection_code_blacklisted_machine: reason_string= "_rejection_code_blacklisted_machine"; break;
	}

	network_event("unable to join game: reason= #%d/%s", reason, reason_string);
	network_game_client_reset(client, TRUE);
}
