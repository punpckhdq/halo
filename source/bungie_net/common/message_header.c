/*
MESSAGE_HEADER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "message_header.h"

/* ---------- macros */

#define SWAP_HEADER(header) (*(header)= (word)((*(header)<<8)|(*(header)>>8)))

/* ---------- public code */

void *create_message(
	long type,
	void const *data,
	word data_size,
	void *buffer,
	word buffer_size)
{
	word *message= buffer;
	short message_size= data_size+sizeof(word);

	if (buffer)
	{
		match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 41, buffer_size >= message_size);
	}
	else
	{
		message= match_malloc("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 46, message_size);
	}

	if (message)
	{
		build_message_header(message, message_size, (byte)type, 0);
		if (data) csmemcpy(message+1, data, data_size);
	}

	return message;
}

void build_message_header(
	word *msg,
	word length,
	byte type,
	byte flags)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 67, msg);

	MATCH_SET_MESSAGE_SIZE("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 69, msg, length);
	MATCH_SET_MESSAGE_TYPE("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 70, msg, type);
	MATCH_SET_MESSAGE_FLAGS("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 71, msg, flags);

	return;
}

void byte_swap_message_header(
	struct message_header *header,
	long byte_order)
{
	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 80, header);

	if (byte_order==_byte_order_network)
	{

		SWAP_HEADER(&header->packed_size);
	}
	else if (byte_order==_byte_order_host)
	{

		SWAP_HEADER(&header->packed_size);
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_header.c", 94, !"bad value for byte order");
	}

	return;
}
