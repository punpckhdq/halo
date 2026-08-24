/*
NETWORK_CLIENT_MESSAGE_HANDLER.C

- the packet body starts at sizeof(word), not sizeof(message_header); only the leading word is read here
*/

/* ---------- headers */

#include "cseries.h"
#include "network_client_message_handler.h"
#include "network_client_manager.h"
#include "network_messages.h"
#include "transport.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean network_game_client_handle_message_server_game_advertise(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_pong(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_machine_accepted(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_machine_rejected(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_game_settings_update(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_pregame_countdown(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_pregame_keep_alive(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_postgame_keep_alive(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_begin_game(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_graceful_game_exit_pregame(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_game_update(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_add_player_ingame(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_remove_player_ingame(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_game_over(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_switch_to_pregame(network_game_client *client, message_header *message, short message_size, transport_address *source_address);
static boolean network_game_client_handle_message_server_graceful_game_exit_postgame(network_game_client *client, message_header *message, short message_size, transport_address *source_address);

/* ---------- globals */

/* ---------- public code */

boolean network_game_client_handle_message(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	boolean result= TRUE;
	word header;
	word message_type;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 47, client && message && (message_size == GET_MESSAGE_SIZE(*message)) && source_address);

	header= message->packed_size;
	message_type= GET_MESSAGE_TYPE(message->packed_size);

	if (GET_MESSAGE_FLAGS(header)!=0)
	{
		network_event("client received client message with invalid flags");
	}
	else
	{
		switch (message_type)
		{
			case 1: // low-level error message from the server
			{
				// the payload is a 0x80 byte string followed by the one byte error code
				if (message_size>=sizeof(word)+0x80+1)
				{
					byte *error_string= (byte *)message + sizeof(word);

					network_event("client received low-level error message: error= #%d (%s)", error_string[0x80], error_string);
				}
				else
				{
					network_event("client received a malformed/damaged message from a server");
				}
				break;
			}

			case _message_type_data:
			{
				network_event("client received a bad message type (_message_type_data)");
				break;
			}

			case _message_type_packet:
			{
				switch (((byte *)message)[message_size-1])
				{
					case _message_type_server_game_advertise:
					{
						result= network_game_client_handle_message_server_game_advertise(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_game_advertise() failed");
						}
						break;
					}

					case _message_type_server_pong:
					{
						result= network_game_client_handle_message_server_pong(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_pong() failed");
						}
						break;
					}

					case _message_type_server_machine_accepted:
					{
						result= network_game_client_handle_message_server_machine_accepted(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_machine_accepted() failed");
						}
						break;
					}

					case _message_type_server_machine_rejected:
					{
						result= network_game_client_handle_message_server_machine_rejected(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_machine_rejected() failed");
						}
						break;
					}

					case _message_type_server_game_settings_update:
					{
						result= network_game_client_handle_message_server_game_settings_update(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_game_settings_update() failed");
						}
						break;
					}

					case _message_type_server_pregame_countdown:
					{
						result= network_game_client_handle_message_server_pregame_countdown(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_pregame_countdown() failed");
						}
						break;
					}

					case _message_type_server_pregame_keep_alive:
					{
						result= network_game_client_handle_message_server_pregame_keep_alive(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_pregame_keep_alive() failed");
						}
						break;
					}

					case _message_type_server_postgame_keep_alive:
					{
						result= network_game_client_handle_message_server_postgame_keep_alive(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_postgame_keep_alive() failed");
						}
						break;
					}

					case _message_type_server_begin_game:
					{
						result= network_game_client_handle_message_server_begin_game(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_begin_game() failed");
						}
						break;
					}

					case _message_type_server_graceful_game_exit_pregame:
					{
						result= network_game_client_handle_message_server_graceful_game_exit_pregame(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_graceful_game_exit_pregame() failed");
						}
						break;
					}

					case _message_type_server_game_update:
					{
						result= network_game_client_handle_message_server_game_update(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_game_update() failed");
						}
						break;
					}

					case _message_type_server_add_player_ingame:
					{
						result= network_game_client_handle_message_server_add_player_ingame(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_add_player_ingame() failed");
						}
						break;
					}

					case _message_type_server_remove_player_ingame:
					{
						result= network_game_client_handle_message_server_remove_player_ingame(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_remove_player_ingame() failed");
						}
						break;
					}

					case _message_type_server_game_over:
					{
						result= network_game_client_handle_message_server_game_over(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_game_over() failed");
						}
						break;
					}

					case _message_type_server_switch_to_pregame:
					{
						result= network_game_client_handle_message_server_switch_to_pregame(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_switch_to_pregame() failed");
						}
						break;
					}

					case _message_type_server_graceful_game_exit_postgame:
					{
						result= network_game_client_handle_message_server_graceful_game_exit_postgame(client, message, message_size, source_address);
						if (!result)
						{
							network_event("network_game_client_handle_message_server_graceful_game_exit_postgame() failed");
						}
						break;
					}

					default:
					{
						network_event("unknown packet type received from system @ address: %s", transport_address_to_string(source_address));
						break;
					}
				}
				break;
			}

			default:
			{
				network_event("client received a message with an unknown message type");
				break;
			}
		}
	}

	return result;
}

/* ---------- private code */

static boolean network_game_client_handle_message_server_game_advertise(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	if (network_game_client_get_state(client, NULL)==_network_game_client_state_searching)
	{
		message_server_game_advertise message_packet;
		short packet_type= _message_type_server_game_advertise;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size-= sizeof(word);
		if (decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 1))
		{
			// the advertisement echoes the nonce we broadcast in our game search
			if (transport_is_nonce(message_packet.client_nonce, sizeof(message_packet.client_nonce)))
			{
				network_game_client_new_advertised_game(client, &message_packet);
			}
		}
		else
		{
			network_event("failed to decode a message_server_game_advertise packet");
		}
	}
	else
	{
		network_event("ignoring an advertised game because we are not looking for new games");
	}

	return TRUE;
}

static boolean network_game_client_handle_message_server_pong(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	if (network_game_client_get_state(client, NULL)==_network_game_client_state_searching)
	{
		message_server_pong message_packet;
		short packet_type= _message_type_server_pong;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size-= sizeof(word);
		if (decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 1))
		{
			network_game_client_ponged(client, source_address, message_packet.system_milliseconds);
		}
		else
		{
			network_event("failed to decode a message_server_pong packet");
		}
	}
	else
	{
		network_event("ignoring a pong message because we are not listening for them");
	}

	return TRUE;
}

static boolean network_game_client_handle_message_server_machine_accepted(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	boolean result= TRUE;

	if (network_game_client_address_matches_server(client, source_address) &&
		(network_game_client_get_state(client, NULL)==_network_game_client_state_joining))
	{
		message_server_machine_accepted message_packet;
		short packet_type= _message_type_server_machine_accepted;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size-= sizeof(word);
		if (decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 2))
		{
			network_game_client_accepted_into_game(client, source_address, &message_packet);
		}
		else
		{
			network_event("failed to decode a message_server_machine_accepted packet");
			result= FALSE;
		}
	}
	else
	{
		network_event("ignoring a message_server_machine_accepted message; either a bad machine or we aren't joining");
		result= FALSE;
	}

	return result;
}

static boolean network_game_client_handle_message_server_machine_rejected(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	boolean result= TRUE;

	if (network_game_client_address_matches_server(client, source_address) &&
		(network_game_client_get_state(client, NULL)==_network_game_client_state_joining))
	{
		message_server_machine_rejected message_packet;
		short packet_type= _message_type_server_machine_rejected;
		short packet_version= NETWORK_GAME_MESSAGE_VERSION;

		message_size-= sizeof(word);
		if (decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 2))
		{
			network_game_client_rejected_by_game(client, source_address, message_packet.reason);
		}
		else
		{
			network_event("failed to decode a message_server_machine_rejected packet");
			result= FALSE;
		}
	}
	else
	{
		network_event("ignoring a message_server_machine_rejected message; either a bad machine or we aren't joining");
		result= FALSE;
	}

	return result;
}

static boolean network_game_client_handle_message_server_game_settings_update(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	boolean result= FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 361, client != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 362, source_address != NULL);

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL)==_network_game_client_state_pregame)
		{
			message_server_game_settings_update message_packet;
			short packet_type= _message_type_server_game_settings_update;
			short packet_version= NETWORK_GAME_MESSAGE_VERSION;

			message_size-= sizeof(word);
			if (decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 2))
			{
				result= network_game_client_game_settings_updated(client, &message_packet);
				if (!result)
				{
					network_event("network_game_client_game_settings_updated() failed");
				}
			}
			else
			{
				network_event("failed to decode a message_server_game_settings_update packet");
			}
		}
		else
		{
			network_event("failed to handle a message_server_game_settings_update message; not in pregame state");
			result= TRUE;
		}
	}
	else
	{
		network_event("ignoring a message_server_game_settings_update; came from a bad machine");
		result= TRUE;
	}

	return result;
}

static boolean network_game_client_handle_message_server_pregame_countdown(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 411, client != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 412, source_address != NULL);

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL)==_network_game_client_state_pregame)
		{
			message_server_pregame_countdown message_packet;
			short packet_type= _message_type_server_pregame_countdown;
			short packet_version= NETWORK_GAME_MESSAGE_VERSION;

			message_size-= sizeof(word);
			if (decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 2))
			{
				network_game_client_countdown_timer_update(client, message_packet.seconds_to_start);
			}
			else
			{
				network_event("failed to decode a message_server_pregame_countdown packet");
			}
		}
		else
		{
			network_event("failed to handle a message_server_pregame_countdown message; not in pregame state");
		}
	}
	else
	{
		network_event("ignoring a message_server_pregame_countdown; came from a bad machine");
	}

	return TRUE;
}

static boolean network_game_client_handle_message_server_pregame_keep_alive(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 452, client != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 453, source_address != NULL);

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL)==_network_game_client_state_pregame)
		{
			message_server_pregame_keep_alive message_packet;
			short packet_type= _message_type_server_pregame_keep_alive;
			short packet_version= NETWORK_GAME_MESSAGE_VERSION;

			message_size-= sizeof(word);
			if (!decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 2))
			{
				network_event("failed to decode a message_server_pregame_keep_alive packet");
			}
		}
		else
		{
			network_event("failed to handle a message_server_pregame_keep_alive message; not in pregame state");
		}
	}
	else
	{
		network_event("ignoring a message_server_pregame_keep_alive; came from a bad machine");
	}

	return TRUE;
}

static boolean network_game_client_handle_message_server_postgame_keep_alive(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 494, client != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 495, source_address != NULL);

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL)==_network_game_client_state_postgame)
		{
			message_server_postgame_keep_alive message_packet;
			short packet_type= _message_type_server_postgame_keep_alive;
			short packet_version= NETWORK_GAME_MESSAGE_VERSION;

			message_size-= sizeof(word);
			if (!decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 6))
			{
				network_event("failed to decode a message_server_postgame_keep_alive packet");
			}
		}
		else
		{
			network_event("failed to handle a message_server_postgame_keep_alive message; not in postgame state");
		}
	}
	else
	{
		network_event("ignoring a message_server_postgame_keep_alive; came from a bad machine");
	}

	return TRUE;
}

static boolean network_game_client_handle_message_server_begin_game(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	boolean result= FALSE;

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL)==_network_game_client_state_pregame)
		{
			message_server_begin_game message_packet;
			short packet_type= _message_type_server_begin_game;
			short packet_version= NETWORK_GAME_MESSAGE_VERSION;

			message_size-= sizeof(word);
			if (decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 2))
			{
				result= network_game_client_game_has_started(client);
				if (!result)
				{
					network_event("network_game_client_game_has_started() failed");
				}
			}
			else
			{
				network_event("failed to decode a message_server_begin_game packet");
			}
		}
		else
		{
			network_event("failed to handle a message_server_begin_game message; we are not in pregame");
		}
	}
	else
	{
		network_event("ignoring a message_server_begin_game message; came from a bad machine");
		result= TRUE;
	}

	return result;
}

static boolean network_game_client_handle_message_server_graceful_game_exit_pregame(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL)==_network_game_client_state_pregame)
		{
			message_server_graceful_game_exit_pregame message_packet;
			short packet_type= _message_type_server_graceful_game_exit_pregame;
			short packet_version= NETWORK_GAME_MESSAGE_VERSION;

			message_size-= sizeof(word);
			if (decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 2))
			{
				network_game_client_game_shutdown(client);
			}
			else
			{
				network_event("failed to decode a message_server_graceful_game_exit_pregame packet");
			}
		}
		else
		{
			network_event("failed to handle a message_server_graceful_game_exit_pregame message; we are not in pregame");
		}
	}
	else
	{
		network_event("ignoring a message_server_graceful_game_exit_pregame message; came from a bad machine");
	}

	return TRUE;
}

static boolean network_game_client_handle_message_server_game_update(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	boolean result= FALSE;

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL)==_network_game_client_state_ingame)
		{
			message_server_game_update message_packet;
			short packet_type= _message_type_server_game_update;
			short packet_version= NETWORK_GAME_MESSAGE_VERSION;

			message_size-= sizeof(word);
			if (decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 4))
			{
				result= network_game_client_handle_game_update(client, &message_packet);
				if (!result)
				{
					network_event("network_game_client_handle_game_update() failed");
				}
			}
			else
			{
				network_event("failed to decode a message_server_game_update packet");
			}
		}
		else
		{
			network_event("failed to handle a message_server_game_update message; we are not in game");
		}
	}
	else
	{
		network_event("ignoring a message_server_game_update message; came from a bad machine");
		result= TRUE;
	}

	if (!result)
	{
		network_game_client_game_out_of_sync(client);
	}

	return result;
}

static boolean network_game_client_handle_message_server_add_player_ingame(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	boolean result= FALSE;

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL)==_network_game_client_state_ingame)
		{
			message_server_add_player_ingame message_packet;
			short packet_type= _message_type_server_add_player_ingame;
			short packet_version= NETWORK_GAME_MESSAGE_VERSION;

			message_size-= sizeof(word);
			if (decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 4))
			{
				result= network_game_client_add_player_to_game(client, &message_packet.player);
				if (!result)
				{
					network_event("network_game_client_add_player_to_game() failed");
				}
			}
			else
			{
				network_event("failed to decode a message_server_add_player_ingame packet");
			}
		}
		else
		{
			network_event("failed to handle a message_server_add_player_ingame message; we are not in game");
		}
	}
	else
	{
		network_event("ignoring a message_server_add_player_ingame message; came from a bad machine");
		result= TRUE;
	}

	if (!result)
	{
		network_game_client_game_out_of_sync(client);
	}

	return result;
}

static boolean network_game_client_handle_message_server_remove_player_ingame(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	boolean result= FALSE;

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL)==_network_game_client_state_ingame)
		{
			message_server_remove_player_ingame message_packet;
			short packet_type= _message_type_server_remove_player_ingame;
			short packet_version= NETWORK_GAME_MESSAGE_VERSION;

			message_size-= sizeof(word);
			if (decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 4))
			{
				result= network_game_client_remove_player(client, &message_packet.player, message_packet.time_of_quit_out_of_game);
				if (!result)
				{
					network_event("network_game_client_remove_player() failed");
				}
			}
			else
			{
				network_event("failed to decode a message_server_remove_player_ingame packet");
			}
		}
		else
		{
			network_event("failed to handle a message_server_remove_player_ingame message; we are not in game");
		}
	}
	else
	{
		network_event("ignoring a message_server_remove_player_ingame message; came from a bad machine");
		result= TRUE;
	}

	if (!result)
	{
		network_game_client_game_out_of_sync(client);
	}

	return result;
}

static boolean network_game_client_handle_message_server_game_over(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL)==_network_game_client_state_ingame)
		{
			message_server_game_over message_packet;
			short packet_type= _message_type_server_game_over;
			short packet_version= NETWORK_GAME_MESSAGE_VERSION;

			message_size-= sizeof(word);
			if (!decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 4))
			{
				network_event("failed to decode a message_server_game_over message (not critical)");
			}

			network_game_client_switch_to_postgame(client);
		}
		else
		{
			network_event("failed to handle a message_server_game_over message; we are not in game");
		}
	}
	else
	{
		network_event("ignoring a message_server_game_over message; came from a bad machine");
	}

	return TRUE;
}

static boolean network_game_client_handle_message_server_switch_to_pregame(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	boolean result= FALSE;

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL)==_network_game_client_state_postgame)
		{
			message_server_switch_to_pregame message_packet;
			short packet_type= _message_type_server_switch_to_pregame;
			short packet_version= NETWORK_GAME_MESSAGE_VERSION;

			message_size-= sizeof(word);
			if (!decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 6))
			{
				network_event("failed to decode a message_server_switch_to_pregame packet");
			}

			result= network_game_client_switch_to_pregame(client);
			if (!result)
			{
				network_event("network_game_client_switch_to_pregame() failed");
			}
		}
		else
		{
			network_event("failed to handle a message_server_switch_to_pregame message; we are not in post-game");
		}
	}
	else
	{
		network_event("ignoring a message_server_switch_to_pregame message; came from a bad machine");
		result= TRUE;
	}

	return result;
}

static boolean network_game_client_handle_message_server_graceful_game_exit_postgame(
	network_game_client *client,
	message_header *message,
	short message_size,
	transport_address *source_address)
{
	boolean result= FALSE;

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL)==_network_game_client_state_postgame)
		{
			message_server_graceful_game_exit_postgame message_packet;
			short packet_type= _message_type_server_graceful_game_exit_postgame;
			short packet_version= NETWORK_GAME_MESSAGE_VERSION;

			message_size-= sizeof(word);
			if (!decode_network_game_message(&message_packet, (byte *)message + sizeof(word), &message_size, &packet_type, &packet_version, 6))
			{
				network_event("failed to decode a message_server_graceful_game_exit_postgame packet (not critical)");
			}

			network_game_client_game_shutdown(client);
		}
		else
		{
			network_event("failed to handle a message_server_graceful_game_exit_postgame message; we are not in post-game");
		}
	}
	else
	{
		network_event("ignoring a message_server_graceful_game_exit_postgame message; came from a bad machine");
		result= TRUE;
	}

	return result;
}
