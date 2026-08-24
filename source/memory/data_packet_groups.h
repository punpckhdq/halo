/*
DATA_PACKET_GROUPS.H

header included in hcex build.
*/

#ifndef __DATA_PACKET_GROUPS_H
#define __DATA_PACKET_GROUPS_H
#pragma once

/* ---------- constants */

// the two field types the packet tables spell out by name; the rest go in as their type codes
enum
{
	_data_packet_field_longs= 3,
	_data_packet_field_end= 9
};

/* ---------- macros */

/* ---------- structures */

struct data_packet_field
{
	short type;
	short count;
	short minimum_version;
	short maximum_version;
	short size;
};

struct data_packet_definition
{
	char const *name;
	long flags;
	short size;
	short version;
	struct data_packet_field *fields;
	boolean initialized;
};

struct data_packet_entry
{
	short packet_class;
	short flags;
	struct data_packet_definition *definition;
};

struct data_packet_group_definition
{
	char const *name;
	short packet_type_count;
	short packet_class_count;
	long maximum_decoded_packet_size;
	long maximum_encoded_packet_size;
	struct data_packet_entry *packets;
};

/* ---------- prototypes/DATA_PACKET_GROUPS.C */

void data_packet_group_initialize(struct data_packet_group_definition *group_definition);
char const *data_packet_groups_get_error(void);
boolean data_packet_group_encode_packet(struct data_packet_group_definition *group_definition, void const *decoded_packet, void *encoded_packet, short *encoded_packet_size, short packet_type, long packet_version);
boolean data_packet_group_decode_packet(struct data_packet_group_definition *group_definition, void *decoded_packet, void const *encoded_packet, short *encoded_packet_size, short *packet_type, short *packet_version, short expected_packet_class);

/* ---------- globals */

/* ---------- public code */

#endif // __DATA_PACKET_GROUPS_H
