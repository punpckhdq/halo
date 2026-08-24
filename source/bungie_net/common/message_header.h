/*
MESSAGE_HEADER.H

header included in hcex build.
*/

#ifndef __MESSAGE_HEADER_H
#define __MESSAGE_HEADER_H
#pragma once

/* ---------- constants */

enum
{
	_byte_order_host= 0,
	_byte_order_network= 1, // pushed by network_game_server_send_rejection_message
	MESSAGE_MAXIMUM_SIZE= 2048 // network_game_server_handle_client_machines / network_game_client_process_incoming_messages buffers
};

// limits the build_message_header asserts quote by name; _message_type_packet is what
// key_agreement.c's assert compares GET_MESSAGE_TYPE against, and _message_type_data is what the
// client/server message handlers' "received a bad message type (_message_type_data)" events name
// (types 0 and 1 are unnamed; there are NUMBER_OF_MESSAGE_TYPES of them)
enum
{
	MAXIMUM_MESSAGE_SIZE= 0xFFF,
	NUMBER_OF_MESSAGE_TYPES= 4,
	MESSAGE_FLAG_BITS_MASK= 3,

	_message_type_data= 2,
	_message_type_packet= 3
};

// the two flag bits message_encryption.c and key_agreement.c test
enum
{
	_message_flag_encrypted_bit= 0,
	_message_flag_key_bit
};

/* ---------- macros */

#define GET_MESSAGE_SIZE(header) ((header).packed_size>>4)

// the bungie_net layer addresses the header as a word
#define MESSAGE_GET_SIZE(msg) (*(msg)>>4)
#define MESSAGE_GET_FLAGS(msg) (*(msg)&MESSAGE_FLAG_BITS_MASK)
// key_agreement.c's assert text names GET_MESSAGE_TYPE and passes it the header word itself
#define GET_MESSAGE_TYPE(header) ((byte)(((header)>>2)&3))
#define GET_MESSAGE_FLAGS(header) ((header)&MESSAGE_FLAG_BITS_MASK)
#define MATCH_SET_MESSAGE_SIZE(file, line, msg, length) { match_assert(file, line, (0<=(length)) && ((length)<=MAXIMUM_MESSAGE_SIZE)); *(msg)= ((length)<<4) | (*(msg)&0xF); }
#define MATCH_SET_MESSAGE_TYPE(file, line, msg, type) { match_assert(file, line, (0<(type)) && ((type)<NUMBER_OF_MESSAGE_TYPES)); *(msg)= (*(msg)&0xFFF3) | (((word)((type)&3))<<2); }
#define MATCH_SET_MESSAGE_FLAGS(file, line, msg, flags) { match_assert(file, line, (0<=flags) && ((flags)<=MESSAGE_FLAG_BITS_MASK)); *(msg)= (*(msg)&0xFFFC) | (word)(flags); }

#define SET_MESSAGE_SIZE(msg, length) MATCH_SET_MESSAGE_SIZE(__FILE__, __LINE__, msg, length)
#define SET_MESSAGE_TYPE(msg, type) MATCH_SET_MESSAGE_TYPE(__FILE__, __LINE__, msg, type)
#define SET_MESSAGE_FLAGS(msg, flags) MATCH_SET_MESSAGE_FLAGS(__FILE__, __LINE__, msg, flags)

/* ---------- structures */

// the wire header is the leading word alone: create_message() lays the body down at message+sizeof(word)
typedef struct message_header
{
	word packed_size; // size<<4 in the upper 12 bits (GET_MESSAGE_SIZE)
	word pad_02; // 0x02, never read and never written; only sizeof(message_header) sees it
} message_header;

/* ---------- prototypes/MESSAGE_HEADER.C */

void build_message_header(word *msg, word length, byte type, byte flags);
void byte_swap_message_header(message_header *header, long byte_order); // swaps packed_size; both byte orders swap it the same way
void *create_message(long type, void const *data, word data_size, void *buffer, word buffer_size);

/* ---------- globals */

/* ---------- public code */

#endif // __MESSAGE_HEADER_H
