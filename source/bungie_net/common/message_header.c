/*
MESSAGE_HEADER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "message_header.h"
#include "memory_manager.h"
#include "64bit_math.h"
#include "random_numbers.h"
#include "bungie_net/network/transport.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

message_header *create_message(
	byte type,
	void const *data,
	word data_size,
	void *buffer,
	word buffer_size)
{
	short message_size = data_size + sizeof(message_header);

	if (buffer)
	{
		match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 41, buffer_size >= message_size);
	}
	else
	{
		buffer = match_malloc("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 46, message_size);
	}

	if (buffer)
	{
		build_message_header((message_header *)buffer, message_size, type, 0);

		if (data)
		{
			memcpy((byte *)buffer + sizeof(message_header), data, data_size);
		}
	}

	return (message_header *)buffer;
}

void build_message_header(
	message_header *msg,
	word length,
	byte type,
	byte flags)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 67, msg);
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 69, (0<=(length)) && ((length)<=MAXIMUM_MESSAGE_SIZE));

	SET_MESSAGE_SIZE(*msg, length);

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 70, (0<(type)) && ((type)<NUMBER_OF_MESSAGE_TYPES));

	SET_MESSAGE_TYPE(*msg, type);

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 71, (0<=flags) && ((flags)<=MESSAGE_FLAG_BITS_MASK));

	SET_MESSAGE_FLAGS(*msg, flags);

	return;
}

void byte_swap_message_header(
	message_header *header,
	long desired_order)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 80, header);

	if (desired_order == _byte_order_network)
	{
		*header = SWAP2(*header);
	}
	else if (desired_order == _byte_order_host)
	{
		*header = SWAP2(*header);
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 94, !"bad value for byte order");
	}

	return;
}
