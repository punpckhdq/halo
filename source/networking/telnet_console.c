/*
TELNET_CONSOLE.C

- a tcp listener that hands whatever a single telnet client types to hs_compile_and_evaluate(),
and echoes error() and terminal output back to it
*/

/* ---------- headers */

#include "cseries.h"
#include "errors.h"
#include "telnet_console.h"
#include "network_connection.h"
#include "transport.h"
#include "hs.h"

/* ---------- constants */

enum
{
	TELNET_CONSOLE_PORT= 23,

	MAXIMUM_TELNET_CLIENTS= 1,
	MAXIMUM_TELNET_BUFFER_SIZE= 128,
	TELNET_READ_BUFFER_SIZE= 32
};

/* ---------- structures */

struct telnet_client
{
	transport_endpoint_ref endpoint; // 0x00
	char buffer[MAXIMUM_TELNET_BUFFER_SIZE]; // 0x04
}; // 0x84

struct telnet_console_globals
{
	transport_endpoint_ref endpoint; // 0x00
	struct telnet_client clients[MAXIMUM_TELNET_CLIENTS]; // 0x04
	boolean initialized; // 0x88
}; // 0x8c

/* ---------- prototypes */

// call the CRT entry points, not <ctype.h>'s macros
#undef isalnum
#undef ispunct
int __cdecl isalnum(int c);
int __cdecl ispunct(int c);

static boolean process_telnet_client_buffer(struct telnet_client *client, char *buffer, long buffer_size);

/* ---------- globals */

static struct telnet_console_globals telnet_console_globals;

/* ---------- public code */

void telnet_console_initialize(void)
{
	csmemset(&telnet_console_globals, 0, sizeof(telnet_console_globals));

	telnet_console_globals.endpoint= create_transport_endpoint(_transport_type_tcp);

	if (telnet_console_globals.endpoint)
	{
		transport_address address= {0};

		address.address_length= IPV4_ADDRESS_LENGTH;
		address.port= TELNET_CONSOLE_PORT;

		if (!bind_endpoint(telnet_console_globals.endpoint, &address))
		{
			if (!listen_endpoint(telnet_console_globals.endpoint))
			{
				telnet_console_globals.initialized= TRUE;
			}
			else
			{
				error(_error_silent, "listen_endpoint() failed on telnet console endpoint");
				delete_transport_endpoint(telnet_console_globals.endpoint);
				telnet_console_globals.endpoint= NULL;
			}
		}
		else
		{
			error(_error_silent, "bind_endpoint() failed on telnet console endpoint");
			delete_transport_endpoint(telnet_console_globals.endpoint);
			telnet_console_globals.endpoint= NULL;
		}
	}
	else
	{
		error(_error_silent, "create_transport_endpoint() failed on telnet console endpoint");
	}

	return;
}

void telnet_console_dispose(void)
{
	if (telnet_console_globals.initialized)
	{
		if (telnet_console_globals.endpoint)
		{
			delete_transport_endpoint(telnet_console_globals.endpoint);
		}

		if (telnet_console_globals.clients[0].endpoint)
		{
			delete_transport_endpoint(telnet_console_globals.clients[0].endpoint);
		}
	}

	csmemset(&telnet_console_globals, 0, sizeof(telnet_console_globals));

	return;
}

void telnet_console_print(
	char *string)
{
	if (telnet_console_globals.initialized)
	{
		if (string && string[0])
		{
			long length= csstrlen(string);

			if (telnet_console_globals.clients[0].endpoint)
			{
				long result= write_endpoint(telnet_console_globals.clients[0].endpoint, "\r\n", 2);

				if (result>0)
				{
					result= write_endpoint(telnet_console_globals.clients[0].endpoint, string, length);

					if (result>0 && telnet_console_globals.clients[0].buffer[0])
					{
						result= write_endpoint(telnet_console_globals.clients[0].endpoint, telnet_console_globals.clients[0].buffer, csstrlen(telnet_console_globals.clients[0].buffer));
					}
				}

				if (result<=0)
				{
					error(_error_silent, "connection lost to telnet client");
					delete_transport_endpoint(telnet_console_globals.clients[0].endpoint);
					telnet_console_globals.clients[0].endpoint= NULL;
				}
			}
		}
	}

	return;
}

void telnet_console_process(void)
{
	if (telnet_console_globals.initialized)
	{
		if (endpoint_readable(telnet_console_globals.endpoint, 0))
		{
			transport_endpoint_ref new_endpoint= accept_endpoint(telnet_console_globals.endpoint);

			if (new_endpoint)
			{
				long client_index;

				for (client_index= 0; client_index<MAXIMUM_TELNET_CLIENTS; client_index++)
				{
					if (!telnet_console_globals.clients[client_index].endpoint)
					{
						char *message= "Would you like to play a game?\r\n";

						if (write_endpoint(new_endpoint, message, csstrlen(message))<=0)
						{
							delete_transport_endpoint(new_endpoint);
						}
						else
						{
							telnet_console_globals.clients[client_index].endpoint= new_endpoint;
							telnet_console_globals.clients[client_index].buffer[0]= '\0';
						}

						break;
					}
				}

				if (client_index==MAXIMUM_TELNET_CLIENTS)
				{
					char *message= "sorry - the maximum number of clients are already connected. goodbye!\r\n";

					write_endpoint(new_endpoint, message, csstrlen(message));
					delete_transport_endpoint(new_endpoint);
				}
			}
		}

		if (telnet_console_globals.clients[0].endpoint && endpoint_readable(telnet_console_globals.clients[0].endpoint, 0))
		{
			char buffer[TELNET_READ_BUFFER_SIZE];
			long size= read_endpoint(telnet_console_globals.clients[0].endpoint, buffer, sizeof(buffer));

			if (size>0)
			{
				if (process_telnet_client_buffer(&telnet_console_globals.clients[0], buffer, size))
				{
					return;
				}

				error(_error_silent, "error processing telnet client");
			}
			else
			{
				error(_error_silent, "connection lost to telnet client ('%s')", transport_error_to_string(size));
			}

			if (telnet_console_globals.clients[0].endpoint)
			{
				delete_transport_endpoint(telnet_console_globals.clients[0].endpoint);
				telnet_console_globals.clients[0].endpoint= NULL;
			}
		}
	}

	return;
}

/* ---------- private code */

static boolean process_telnet_client_buffer(
	struct telnet_client *client,
	char *buffer,
	long buffer_size)
{
	boolean result= TRUE;
	long index;

	for (index= 0; result && index<buffer_size; index++)
	{
		long echo_result;

		if (buffer[index]>0x7f)
		{
			continue;
		}

		if (isalnum(buffer[index]) || ispunct(buffer[index]) || buffer[index]==' ')
		{
			long length= csstrlen(client->buffer)+1;

			if (length>=MAXIMUM_TELNET_BUFFER_SIZE)
			{
				char *message= "\r\noverflowed client buffer; resetting buffer\r\n";
				long overflow_result;

				client->buffer[0]= '\0';

				overflow_result= write_endpoint(client->endpoint, message, csstrlen(message));
				if (overflow_result<=0)
				{
					error(_error_silent, "failed to write to telnet client ('%s')", transport_error_to_string(overflow_result));

					return FALSE;
				}

				return result;
			}

			client->buffer[length-1]= buffer[index];
			client->buffer[length]= '\0';
		}
		else
		{
			switch (buffer[index])
			{
				case '\n':
				case '\r':
				{
					if (client->buffer[0])
					{
						char command[MAXIMUM_TELNET_BUFFER_SIZE];

						csstrncpy(command, client->buffer, sizeof(command)-1);
						command[sizeof(command)-1]= '\0';
						client->buffer[0]= '\0';

						if (hs_compile_and_evaluate(command))
						{
							if (write_endpoint(client->endpoint, "\r\n", 2)<=0)
							{
								result= FALSE;
							}
						}
					}
					continue;
				}

				case '\b':
				{
					if (client->buffer[0])
					{
						long length= csstrlen(client->buffer);

						if (length>0)
						{
							client->buffer[length-1]= '\0';
						}
					}
					break;
				}

				case '\x04':
				{
					char *message= "\r\ngoodbye!\r\n";

					write_endpoint(client->endpoint, message, csstrlen(message));
					delete_transport_endpoint(client->endpoint);
					client->endpoint= NULL;
					index= buffer_size;
					continue;
				}

				default:
					continue;
			}
		}

		echo_result= write_endpoint(client->endpoint, &buffer[index], 1);
		if (echo_result<=0)
		{
			error(_error_silent, "failed to write to telnet client ('%s')", transport_error_to_string(echo_result));
			result= FALSE;
		}
	}

	return result;
}
