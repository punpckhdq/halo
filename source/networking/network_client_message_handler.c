/*
NETWORK_CLIENT_MESSAGE_HANDLER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "network_client_message_handler.h"
#include "network_client_manager.h"
#include "network_messages.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean network_game_client_handle_message_server_game_advertise(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_pong(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_machine_accepted(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_machine_rejected(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_game_settings_update(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_pregame_countdown(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_pregame_keep_alive(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_postgame_keep_alive(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_begin_game(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_graceful_game_exit_pregame(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_game_update(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_add_player_ingame(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_remove_player_ingame(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_game_over(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_switch_to_pregame(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);
static boolean network_game_client_handle_message_server_graceful_game_exit_postgame(struct network_game_client *client, message_header *message, short message_size, struct transport_address *source_address);

/* ---------- globals */

/* ---------- public code */

boolean network_game_client_handle_message(
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	word message_type;
	boolean success = TRUE;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_message_handler.c",
		47,
		client && message && (message_size == GET_MESSAGE_SIZE(*message)) && source_address);

	message_type = (byte)GET_MESSAGE_TYPE(*message);

	if (GET_MESSAGE_FLAGS(*message))
	{
		network_event("client received client message with invalid flags");
	}
	else
	{
		switch (message_type)
		{
		case _message_type_error:
			if (message_size >= sizeof(*message) + sizeof(struct message_error))
			{
				struct message_error *error = (struct message_error *)(message + 1);

				network_event(
					"client received low-level error message: error= #%d (%s)",
					error->error_code,
					error->error_string);
			}
			else
			{
				network_event("client received a malformed/damaged message from a server");
			}

			break;
		case _message_type_data:
			network_event("client received a bad message type (_message_type_data)");
			break;
		case _message_type_packet:
			switch (((byte *)message)[message_size - 1])
			{
			case _message_type_server_game_advertise:
				success = network_game_client_handle_message_server_game_advertise(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_game_advertise() failed");
				}

				break;
			case _message_type_server_pong:
				success = network_game_client_handle_message_server_pong(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_pong() failed");
				}

				break;
			case _message_type_server_machine_accepted:
				success = network_game_client_handle_message_server_machine_accepted(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_machine_accepted() failed");
				}

				break;
			case _message_type_server_machine_rejected:
				success = network_game_client_handle_message_server_machine_rejected(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_machine_rejected() failed");
				}

				break;
			case _message_type_server_game_settings_update:
				success = network_game_client_handle_message_server_game_settings_update(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_game_settings_update() failed");
				}

				break;
			case _message_type_server_pregame_countdown:
				success = network_game_client_handle_message_server_pregame_countdown(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_pregame_countdown() failed");
				}

				break;
			case _message_type_server_pregame_keep_alive:
				success = network_game_client_handle_message_server_pregame_keep_alive(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_pregame_keep_alive() failed");
				}

				break;
			case _message_type_server_postgame_keep_alive:
				success = network_game_client_handle_message_server_postgame_keep_alive(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_postgame_keep_alive() failed");
				}

				break;
			case _message_type_server_begin_game:
				success = network_game_client_handle_message_server_begin_game(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_begin_game() failed");
				}

				break;
			case _message_type_server_graceful_game_exit_pregame:
				success = network_game_client_handle_message_server_graceful_game_exit_pregame(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_graceful_game_exit_pregame() failed");
				}

				break;
			case _message_type_server_game_update:
				success = network_game_client_handle_message_server_game_update(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_game_update() failed");
				}

				break;
			case _message_type_server_add_player_ingame:
				success = network_game_client_handle_message_server_add_player_ingame(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_add_player_ingame() failed");
				}

				break;
			case _message_type_server_remove_player_ingame:
				success = network_game_client_handle_message_server_remove_player_ingame(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_remove_player_ingame() failed");
				}

				break;
			case _message_type_server_game_over:
				success = network_game_client_handle_message_server_game_over(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_game_over() failed");
				}

				break;
			case _message_type_server_switch_to_pregame:
				success = network_game_client_handle_message_server_switch_to_pregame(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_switch_to_pregame() failed");
				}

				break;
			case _message_type_server_graceful_game_exit_postgame:
				success = network_game_client_handle_message_server_graceful_game_exit_postgame(client, message, message_size, source_address);

				if (!success)
				{
					network_event("network_game_client_handle_message_server_graceful_game_exit_postgame() failed");
				}

				break;
			default:
				network_event(
					"unknown packet type received from system @ address: %s",
					transport_address_to_string(source_address));
				break;
			}

			break;
		default:
			network_event("client received a message with an unknown message type");
			break;
		}
	}

	return success;
}

/* ---------- private code */

static boolean network_game_client_handle_message_server_game_advertise(
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	if (network_game_client_get_state(client, NULL) == _network_game_client_state_searching)
	{
		struct message_server_game_advertise message_packet;
		short packet_type = _message_type_server_game_advertise;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(*message);

		if (decode_network_game_message(
			&message_packet,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_unconnected_server))
		{
			if (transport_is_nonce(message_packet.client_nonce, NETWORK_GAME_NONCE_BYTES))
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
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	if (network_game_client_get_state(client, NULL) == _network_game_client_state_searching)
	{
		struct message_server_pong message_packet;
		short packet_type = _message_type_server_pong;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(*message);

		if (decode_network_game_message(
			&message_packet,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_unconnected_server))
		{
			network_game_client_ponged(client, source_address, message_packet.timestamp);
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
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	boolean success = TRUE;

	if (network_game_client_address_matches_server(client, source_address) &&
		network_game_client_get_state(client, NULL) == _network_game_client_state_joining)
	{
		struct message_server_machine_accepted message_packet;
		short packet_type = _message_type_server_machine_accepted;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(*message);

		if (decode_network_game_message(
			&message_packet,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_server_pregame))
		{
			network_game_client_accepted_into_game(client, source_address, &message_packet);
		}
		else
		{
			network_event("failed to decode a message_server_machine_accepted packet");
			success = FALSE;
		}
	}
	else
	{
		network_event("ignoring a message_server_machine_accepted message; either a bad machine or we aren't joining");
		success = FALSE;
	}

	return success;
}

static boolean network_game_client_handle_message_server_machine_rejected(
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	boolean success = TRUE;

	if (network_game_client_address_matches_server(client, source_address) &&
		network_game_client_get_state(client, NULL) == _network_game_client_state_joining)
	{
		struct message_server_machine_rejected message_packet;
		short packet_type = _message_type_server_machine_rejected;
		short packet_version = NETWORK_GAME_MESSAGE_VERSION;

		message_size -= sizeof(*message);

		if (decode_network_game_message(
			&message_packet,
			message + 1,
			&message_size,
			&packet_type,
			&packet_version,
			_message_class_server_pregame))
		{
			network_game_client_rejected_by_game(client, source_address, message_packet.rejection_code);
		}
		else
		{
			network_event("failed to decode a message_server_machine_rejected packet");
			success = FALSE;
		}
	}
	else
	{
		network_event("ignoring a message_server_machine_rejected message; either a bad machine or we aren't joining");
		success = FALSE;
	}

	return success;
}

static boolean network_game_client_handle_message_server_game_settings_update(
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 361, client != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 362, source_address != NULL);

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL) == _network_game_client_state_pregame)
		{
			struct message_server_game_settings_update message_packet;
			short packet_type = _message_type_server_game_settings_update;
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;

			message_size -= sizeof(*message);

			if (decode_network_game_message(
				&message_packet,
				message + 1,
				&message_size,
				&packet_type,
				&packet_version,
				_message_class_server_pregame))
			{
				success = network_game_client_game_settings_updated(client, &message_packet);

				if (!success)
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
			success = TRUE;
		}
	}
	else
	{
		network_event("ignoring a message_server_game_settings_update; came from a bad machine");
		success = TRUE;
	}

	return success;
}

static boolean network_game_client_handle_message_server_pregame_countdown(
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 411, client != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 412, source_address != NULL);

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL) == _network_game_client_state_pregame)
		{
			struct message_server_pregame_countdown message_packet;
			short packet_type = _message_type_server_pregame_countdown;
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;

			message_size -= sizeof(*message);

			if (decode_network_game_message(
				&message_packet,
				message + 1,
				&message_size,
				&packet_type,
				&packet_version,
				_message_class_server_pregame))
			{
				network_game_client_countdown_timer_update(client, message_packet.seconds_to_game_start);
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
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 452, client != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 453, source_address != NULL);

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL) == _network_game_client_state_pregame)
		{
			struct message_server_pregame_keep_alive message_packet;
			short packet_type = _message_type_server_pregame_keep_alive;
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;

			message_size -= sizeof(*message);

			if (!decode_network_game_message(
				&message_packet,
				message + 1,
				&message_size,
				&packet_type,
				&packet_version,
				_message_class_server_pregame))
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
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 494, client != NULL);
	match_assert("c:\\halo\\SOURCE\\networking\\network_client_message_handler.c", 495, source_address != NULL);

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL) == _network_game_client_state_postgame)
		{
			struct message_server_postgame_keep_alive message_packet;
			short packet_type = _message_type_server_postgame_keep_alive;
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;

			message_size -= sizeof(*message);

			if (!decode_network_game_message(
				&message_packet,
				message + 1,
				&message_size,
				&packet_type,
				&packet_version,
				_message_class_server_postgame))
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
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	boolean success = FALSE;

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL) == _network_game_client_state_pregame)
		{
			struct message_server_begin_game message_packet;
			short packet_type = _message_type_server_begin_game;
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;

			message_size -= sizeof(*message);

			if (decode_network_game_message(
				&message_packet,
				message + 1,
				&message_size,
				&packet_type,
				&packet_version,
				_message_class_server_pregame))
			{
				success = network_game_client_game_has_started(client);

				if (!success)
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
		success = TRUE;
	}

	return success;
}

static boolean network_game_client_handle_message_server_graceful_game_exit_pregame(
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL) == _network_game_client_state_pregame)
		{
			struct message_server_graceful_game_exit_pregame message_packet;
			short packet_type = _message_type_server_graceful_game_exit_pregame;
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;

			message_size -= sizeof(*message);

			if (decode_network_game_message(
				&message_packet,
				message + 1,
				&message_size,
				&packet_type,
				&packet_version,
				_message_class_server_pregame))
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
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	boolean success = FALSE;

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL) == _network_game_client_state_ingame)
		{
			struct message_server_game_update message_packet;
			short packet_type = _message_type_server_game_update;
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;

			message_size -= sizeof(*message);

			if (decode_network_game_message(
				&message_packet,
				message + 1,
				&message_size,
				&packet_type,
				&packet_version,
				_message_class_server_ingame))
			{
				success = network_game_client_handle_game_update(client, &message_packet);

				if (!success)
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
		success = TRUE;
	}

	if (!success)
	{
		network_game_client_game_out_of_sync(client);
	}

	return success;
}

static boolean network_game_client_handle_message_server_add_player_ingame(
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	boolean success = FALSE;

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL) == _network_game_client_state_ingame)
		{
			struct message_server_add_player_ingame message_packet;
			short packet_type = _message_type_server_add_player_ingame;
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;

			message_size -= sizeof(*message);

			if (decode_network_game_message(
				&message_packet,
				message + 1,
				&message_size,
				&packet_type,
				&packet_version,
				_message_class_server_ingame))
			{
				success = network_game_client_add_player_to_game(client, &message_packet.player);

				if (!success)
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
		success = TRUE;
	}

	if (!success)
	{
		network_game_client_game_out_of_sync(client);
	}

	return success;
}

static boolean network_game_client_handle_message_server_remove_player_ingame(
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	boolean success = FALSE;

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL) == _network_game_client_state_ingame)
		{
			struct message_server_remove_player_ingame message_packet;
			short packet_type = _message_type_server_remove_player_ingame;
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;

			message_size -= sizeof(*message);

			if (decode_network_game_message(
				&message_packet,
				message + 1,
				&message_size,
				&packet_type,
				&packet_version,
				_message_class_server_ingame))
			{
				success = network_game_client_remove_player(
					client,
					&message_packet.player,
					message_packet.quit_out_of_game_time);

				if (!success)
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
		success = TRUE;
	}

	if (!success)
	{
		network_game_client_game_out_of_sync(client);
	}

	return success;
}

static boolean network_game_client_handle_message_server_game_over(
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL) == _network_game_client_state_ingame)
		{
			struct message_server_game_over message_packet;
			short packet_type = _message_type_server_game_over;
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;

			message_size -= sizeof(*message);

			if (!decode_network_game_message(
				&message_packet,
				message + 1,
				&message_size,
				&packet_type,
				&packet_version,
				_message_class_server_ingame))
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
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	boolean success = FALSE;

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL) == _network_game_client_state_postgame)
		{
			struct message_server_switch_to_pregame message_packet;
			short packet_type = _message_type_server_switch_to_pregame;
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;

			message_size -= sizeof(*message);

			if (!decode_network_game_message(
				&message_packet,
				message + 1,
				&message_size,
				&packet_type,
				&packet_version,
				_message_class_server_postgame))
			{
				network_event("failed to decode a message_server_switch_to_pregame packet");
			}

			success = network_game_client_switch_to_pregame(client);

			if (!success)
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
		success = TRUE;
	}

	return success;
}

static boolean network_game_client_handle_message_server_graceful_game_exit_postgame(
	struct network_game_client *client,
	message_header *message,
	short message_size,
	struct transport_address *source_address)
{
	boolean success = FALSE;

	if (network_game_client_address_matches_server(client, source_address))
	{
		if (network_game_client_get_state(client, NULL) == _network_game_client_state_postgame)
		{
			struct message_server_graceful_game_exit_postgame message_packet;
			short packet_type = _message_type_server_graceful_game_exit_postgame;
			short packet_version = NETWORK_GAME_MESSAGE_VERSION;

			message_size -= sizeof(*message);

			if (!decode_network_game_message(
				&message_packet,
				message + 1,
				&message_size,
				&packet_type,
				&packet_version,
				_message_class_server_postgame))
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
		success = TRUE;
	}

	return success;
}
