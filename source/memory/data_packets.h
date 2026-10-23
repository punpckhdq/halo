/*
DATA_PACKETS.H

header included in hcex build.
*/

#ifndef __DATA_PACKETS_H
#define __DATA_PACKETS_H
#pragma once

/* ---------- constants */

enum
{
	__pack_pad = 0,
	__pack_char,
	__pack_short,
	__pack_long,
	__pack_int64,
	__pack_string,
	__pack_data,
	__pack_array,
	__pack_fixed_data,
	__pack_end,
	NUMBER_OF_PACKET_FIELD_TYPES,
};

/* ---------- structures */

struct data_packet_field
{
	short type;
	short count;
	short first_version;
	short last_version;
	short size;
};

struct data_packet_definition
{
	char const *name;
	unsigned long flags;
	short size;
	short version;
	struct data_packet_field *fields;
	boolean initialized_flag;
};

/* ---------- prototypes/DATA_PACKETS.C */

void data_packet_verify(struct data_packet_definition *packet_definition);
boolean data_packet_encode(struct data_packet_definition *packet_definition, short version, void *packet, void *buffer, short *buffer_size, short maximum_buffer_size);
boolean data_packet_decode(struct data_packet_definition *packet_definition, void *encoded_packet, short encoded_packet_size, void *decoded_packet, short *version_reference, short *actual_encoded_packet_size_reference);

#endif // __DATA_PACKETS_H
