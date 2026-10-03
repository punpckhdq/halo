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

/* ---------- macros */

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

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __DATA_PACKETS_H
