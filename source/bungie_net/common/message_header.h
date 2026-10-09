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
	MAXIMUM_MESSAGE_SIZE = 0xFFF,
	NUMBER_OF_MESSAGE_TYPES = 4,
	MESSAGE_FLAG_BITS_MASK = 3,
	MESSAGE_TYPE_BITS_MASK = 3, /* fake name */
	MESSAGE_TYPE_SHIFT = 2, /* fake name */
	MESSAGE_SIZE_SHIFT = 4, /* fake name */
};

enum
{
	_message_type_error = 1,
	_message_type_data,
	_message_type_packet,
};

enum
{
	MAXIMUM_ERROR_STRING_LENGTH = 128,
};

enum
{
	_message_flag_encrypted_bit = 0,
	_message_flag_new_encryption_key_bit,
	NUMBER_OF_MESSAGE_FLAGS,
};

enum
{
	_byte_order_host = 0,
	_byte_order_network,
};

/* ---------- macros */

#define GET_MESSAGE_FLAGS(message) ((message)&MESSAGE_FLAG_BITS_MASK) /* fake name */
#define GET_MESSAGE_TYPE(message) (((message)>>MESSAGE_TYPE_SHIFT)&MESSAGE_TYPE_BITS_MASK)
#define GET_MESSAGE_SIZE(message) ((message)>>MESSAGE_SIZE_SHIFT)
#define SET_MESSAGE_FLAGS(message, flags) ((message) = ((message)&~MESSAGE_FLAG_BITS_MASK)|(flags)) /* fake name */
#define SET_MESSAGE_TYPE(message, type) ((message) = ((message)&(word)~(MESSAGE_TYPE_BITS_MASK<<MESSAGE_TYPE_SHIFT))|(((type)&MESSAGE_TYPE_BITS_MASK)<<MESSAGE_TYPE_SHIFT)) /* fake name */
#define SET_MESSAGE_SIZE(message, size) ((message) = ((message)&(FLAG(MESSAGE_SIZE_SHIFT)-1))|((size)<<MESSAGE_SIZE_SHIFT)) /* fake name */

/* ---------- structures */

typedef word message_header;

struct message_error /* fake name */
{
	char error_string[MAXIMUM_ERROR_STRING_LENGTH]; /* fake name */
	byte error_code; /* fake name */
};

/* ---------- prototypes/MESSAGE_HEADER.C */

message_header *create_message(byte type, void const *data, word data_size, void *buffer, word buffer_size);
void build_message_header(message_header *msg, word length, byte type, byte flags);
void byte_swap_message_header(message_header *header, long desired_order);

/* ---------- globals */

/* ---------- public code */

#endif // __MESSAGE_HEADER_H
