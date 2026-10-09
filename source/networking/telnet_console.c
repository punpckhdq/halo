/*
TELNET_CONSOLE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "telnet_console.h"
#include "bungie_net/network/transport.h"
#include "hs.h"

/* ---------- constants */

enum
{
	MAXIMUM_TELNET_CLIENTS = 1, /* fake name */
	TELNET_CLIENT_BUFFER_SIZE = 128, /* fake name */
	TELNET_PORT = 23, /* fake name */
	TELNET_END_OF_TRANSMISSION = 4, /* fake name */
};

/* ---------- structures */

struct telnet_client /* fake name */
{
	struct transport_endpoint *endpoint;
	char buffer[TELNET_CLIENT_BUFFER_SIZE];
};

struct telnet_console_globals /* fake name */
{
	struct transport_endpoint *listening_endpoint;
	struct telnet_client clients[MAXIMUM_TELNET_CLIENTS];
	boolean initialized;
};

/* ---------- prototypes */

static boolean process_telnet_client_buffer(char *buffer, long length, struct telnet_client *client);

/* ---------- globals */

static struct telnet_console_globals telnet_console_globals; /* fake name */

/* ---------- public code */

void telnet_console_initialize(
	void)
{
	memset(&telnet_console_globals, 0, sizeof(telnet_console_globals));
	telnet_console_globals.listening_endpoint = create_transport_endpoint(_transport_type_tcp);

	if (telnet_console_globals.listening_endpoint)
	{
		struct transport_address address = {0};

		address.address_length = IPV4_ADDRESS_LENGTH;
		address.port = TELNET_PORT;

		if (bind_endpoint(telnet_console_globals.listening_endpoint, &address) == _transport_error_none)
		{
			if (listen_endpoint(telnet_console_globals.listening_endpoint) == _transport_error_none)
			{
				telnet_console_globals.initialized = TRUE;
			}
			else
			{
				error(_error_silent, "listen_endpoint() failed on telnet console endpoint");
				delete_transport_endpoint(telnet_console_globals.listening_endpoint);
				telnet_console_globals.listening_endpoint = NULL;
			}
		}
		else
		{
			error(_error_silent, "bind_endpoint() failed on telnet console endpoint");
			delete_transport_endpoint(telnet_console_globals.listening_endpoint);
			telnet_console_globals.listening_endpoint = NULL;
		}
	}
	else
	{
		error(_error_silent, "create_transport_endpoint() failed on telnet console endpoint");
	}

	return;
}

void telnet_console_dispose(
	void)
{
	if (telnet_console_globals.initialized)
	{
		if (telnet_console_globals.listening_endpoint)
		{
			delete_transport_endpoint(telnet_console_globals.listening_endpoint);
		}

		if (telnet_console_globals.clients[0].endpoint)
		{
			delete_transport_endpoint(telnet_console_globals.clients[0].endpoint);
		}
	}

	memset(&telnet_console_globals, 0, sizeof(telnet_console_globals));

	return;
}

void telnet_console_print(
	char *string)
{
	if (telnet_console_globals.initialized && string && string[0])
	{
		long length = strlen(string);
		struct telnet_client *client = &telnet_console_globals.clients[0];

		if (client->endpoint)
		{
			long result = write_endpoint(client->endpoint, "\r\n", 2);

			if (result > 0)
			{
				result = write_endpoint(client->endpoint, string, length);

				if (result > 0 && client->buffer[0])
				{
					result = write_endpoint(client->endpoint, client->buffer, strlen(client->buffer));
				}
			}

			if (result <= 0)
			{
				error(_error_silent, "connection lost to telnet client");
				delete_transport_endpoint(client->endpoint);
				client->endpoint = NULL;
			}
		}
	}

	return;
}

void telnet_console_process(
	void)
{
	if (telnet_console_globals.initialized)
	{
		char buffer[32];

		if (endpoint_readable(telnet_console_globals.listening_endpoint, 0))
		{
			struct transport_endpoint *endpoint = accept_endpoint(telnet_console_globals.listening_endpoint);

			if (endpoint)
			{
				long client_index;

				for (client_index = 0; client_index < MAXIMUM_TELNET_CLIENTS; client_index++)
				{
					if (!telnet_console_globals.clients[client_index].endpoint)
					{
						if (write_endpoint(
							endpoint,
							"Would you like to play a game?\r\n",
							strlen("Would you like to play a game?\r\n")) <= 0)
						{
							delete_transport_endpoint(endpoint);
						}
						else
						{
							telnet_console_globals.clients[client_index].endpoint = endpoint;
							telnet_console_globals.clients[client_index].buffer[0] = 0;
						}

						break;
					}
				}

				if (client_index == MAXIMUM_TELNET_CLIENTS)
				{
					write_endpoint(
						endpoint,
						"sorry - the maximum number of clients are already connected. goodbye!\r\n",
						strlen("sorry - the maximum number of clients are already connected. goodbye!\r\n"));
					delete_transport_endpoint(endpoint);
				}
			}
		}

		if (telnet_console_globals.clients[0].endpoint &&
			endpoint_readable(telnet_console_globals.clients[0].endpoint, 0))
		{
			long length = read_endpoint(telnet_console_globals.clients[0].endpoint, buffer, sizeof(buffer));
			boolean disconnect = TRUE;

			if (length > 0)
			{
				if (process_telnet_client_buffer(buffer, length, &telnet_console_globals.clients[0]))
				{
					disconnect = FALSE;
				}
				else
				{
					error(_error_silent, "error processing telnet client");
				}
			}
			else
			{
				error(_error_silent, "connection lost to telnet client ('%s')", transport_error_to_string((short)length));
			}

			if (disconnect && telnet_console_globals.clients[0].endpoint)
			{
				delete_transport_endpoint(telnet_console_globals.clients[0].endpoint);
				telnet_console_globals.clients[0].endpoint = NULL;
			}
		}
	}

	return;
}

/* ---------- private code */

static boolean process_telnet_client_buffer(
	char *buffer,
	long length,
	struct telnet_client *client)
{
	long index;
	boolean result = TRUE;

	for (index = 0; result && index < length; index++)
	{
		char *character = &buffer[index];

		if (*character <= 127)
		{
			if (isalnum(*character) || ispunct(*character) || *character == ' ')
			{
				long buffer_length = strlen(client->buffer) + 1;

				if (buffer_length >= TELNET_CLIENT_BUFFER_SIZE)
				{
					long written;

					client->buffer[0] = 0;
					written = write_endpoint(
						client->endpoint,
						"\r\noverflowed client buffer; resetting buffer\r\n",
						strlen("\r\noverflowed client buffer; resetting buffer\r\n"));

					if (written <= 0)
					{
						error(_error_silent, "failed to write to telnet client ('%s')", transport_error_to_string((short)written));
						result = FALSE;
					}

					break;
				}

				client->buffer[buffer_length - 1] = *character;
				client->buffer[buffer_length] = 0;
			}
			else
			{
				switch (*character)
				{
				case '\n':
				case '\r':
					if (client->buffer[0])
					{
						char expression[TELNET_CLIENT_BUFFER_SIZE];

						strncpy(expression, client->buffer, TELNET_CLIENT_BUFFER_SIZE - 1);
						expression[TELNET_CLIENT_BUFFER_SIZE - 1] = 0;
						client->buffer[0] = 0;

						if (hs_compile_and_evaluate(expression) && write_endpoint(client->endpoint, "\r\n", 2) <= 0)
						{
							result = FALSE;
						}
					}

					continue;

				case '\b':
					if (client->buffer[0])
					{
						long buffer_length = strlen(client->buffer);

						if (buffer_length > 0)
						{
							client->buffer[buffer_length - 1] = 0;
						}
					}
					break;

				case TELNET_END_OF_TRANSMISSION:
					write_endpoint(client->endpoint, "\r\ngoodbye!\r\n", strlen("\r\ngoodbye!\r\n"));
					delete_transport_endpoint(client->endpoint);
					client->endpoint = NULL;
					index = length;
					continue;

				default:
					continue;
				}
			}

			{
				long written = write_endpoint(client->endpoint, character, 1);

				if (written <= 0)
				{
					error(_error_silent, "failed to write to telnet client ('%s')", transport_error_to_string((short)written));
					result = FALSE;
				}
			}
		}
	}

	return result;
}
