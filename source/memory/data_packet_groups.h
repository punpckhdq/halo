/*
DATA_PACKET_GROUPS.H
*/

#ifndef __DATA_PACKET_GROUPS_H
#define __DATA_PACKET_GROUPS_H
#pragma once

/* ---------- headers */

#include "data_packets.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct data_packet_group_packet
{
	short packet_class;
	struct data_packet_definition *definition;
};

struct data_packet_group_definition
{
	char const *name;
	short packet_type_count;
	short packet_class_count;
	long maximum_decoded_packet_size;
	long maximum_encoded_packet_size;
	struct data_packet_group_packet *packets;
};

/* ---------- prototypes/DATA_PACKET_GROUPS.C */

void data_packet_group_initialize(struct data_packet_group_definition *group_definition);
boolean data_packet_group_decode_packet(struct data_packet_group_definition *group_definition, void *decoded_packet, void const *encoded_packet, short *encoded_packet_size, short *packet_type, short *packet_version, short expected_packet_class);
boolean data_packet_group_encode_packet(struct data_packet_group_definition *group_definition, void const *decoded_packet, void *encoded_packet, short *encoded_packet_size, short packet_type, short packet_version);

/* ---------- globals */

/* ---------- public code */

#endif // __DATA_PACKET_GROUPS_H
