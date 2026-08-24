/*
DATA_PACKET_GROUPS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "data_packet_groups.h"
#include "data_packets.h"

#include "byte_swapping.h"

/* ---------- structures */

// the only thing a group adds to an encoded packet: the type, appended after the payload
struct packet_header
{
	char packet_type;
};

/* ---------- prototypes */

boolean data_packet_group_append_packet_header(struct data_packet_group_definition *group_definition, void *encoded_packet, short *encoded_packet_size, short packet_type);

/* ---------- globals */

// the swap codes and the definition naming them are one contiguous run of data
static struct
{
	byte_swap_code codes[4];
	struct byte_swap_definition definition;
} data_00309e38=
{
	{_begin_bs_array, 1, _1byte, _end_bs_array},
	{"packet_header", sizeof(struct packet_header), data_00309e38.codes, BYTE_SWAP_DEFINITION_SIGNATURE, FALSE}
};

// why the last encode or decode failed, handed out once by data_packet_groups_get_error
static char const *bss_00456624;

/* ---------- public code */

void data_packet_group_initialize(
	struct data_packet_group_definition *group_definition)
{
	short packet_type;

	for (packet_type= 0; packet_type<group_definition->packet_type_count; packet_type++)
	{
		struct data_packet_entry *packet= &group_definition->packets[packet_type];

		if (packet->definition)
		{
			match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 40, packet->packet_class>=0 && packet->packet_class<group_definition->packet_class_count);
			match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 41, packet->definition->size<=group_definition->maximum_decoded_packet_size);
			match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 42, packet->definition->size + sizeof(struct packet_header)<=group_definition->maximum_encoded_packet_size);

			data_packet_verify(packet->definition);
		}
	}
}

char const *data_packet_groups_get_error(
	void)
{
	char const *result= bss_00456624;

	bss_00456624= NULL;

	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 57, result);

	return result;
}

boolean data_packet_group_decode_packet(
	struct data_packet_group_definition *group_definition,
	void *decoded_packet,
	void const *encoded_packet,
	short *encoded_packet_size,
	short *packet_type,
	short *packet_version,
	short expected_packet_class)
{
	char const *error= NULL;

	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 73, decoded_packet);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 74, encoded_packet && encoded_packet_size);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 75, packet_type && packet_version);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 77, expected_packet_class>=0 && expected_packet_class<group_definition->packet_class_count);

	if (*encoded_packet_size>=sizeof(struct packet_header))
	{
		struct packet_header *packet_header= (struct packet_header *)((char *)encoded_packet + *encoded_packet_size - sizeof(struct packet_header));

		byte_swap_data(&data_00309e38.definition, packet_header, 1);

		if (packet_header->packet_type>=0 && packet_header->packet_type<group_definition->packet_type_count)
		{
			struct data_packet_entry *packet= &group_definition->packets[packet_header->packet_type];

			if (packet->packet_class==expected_packet_class)
			{
				*encoded_packet_size -= sizeof(struct packet_header);

				if (!packet->definition ||
					data_packet_decode(packet->definition, encoded_packet, *encoded_packet_size, decoded_packet, packet_version, NULL))
				{
					*packet_type= packet_header->packet_type;
				}
				else
				{
					error= "got packet which wouldn't decode";
				}
			}
			else
			{
				error= "got packet with mismatched class";
			}
		}
		else
		{
			error= "got packet with bad type";
		}
	}
	else
	{
		error= "got packet with no header";
	}

	bss_00456624= error;

	return error==NULL;
}

boolean data_packet_group_encode_packet(
	struct data_packet_group_definition *group_definition,
	void const *decoded_packet,
	void *encoded_packet,
	short *encoded_packet_size,
	short packet_type,
	long packet_version)
{
	char const *error= NULL;
	struct data_packet_entry *packet;

	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 132, group_definition);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 133, packet_type>=0 && packet_type<group_definition->packet_type_count);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 134, encoded_packet && encoded_packet_size);

	packet= &group_definition->packets[packet_type];

	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 139, packet->definition);

	if (data_packet_encode(packet->definition, packet_version, decoded_packet, encoded_packet, encoded_packet_size,
		group_definition->maximum_encoded_packet_size))
	{
		if (!data_packet_group_append_packet_header(group_definition, encoded_packet, encoded_packet_size, packet_type))
		{
			error= bss_00456624;
		}
	}
	else
	{
		error= "couldn't encode packet";
	}

	bss_00456624= error;

	return error==NULL;
}

/* ---------- private code */

boolean data_packet_group_append_packet_header(
	struct data_packet_group_definition *group_definition,
	void *encoded_packet,
	short *encoded_packet_size,
	short packet_type)
{
	struct packet_header *packet_header= (struct packet_header *)((char *)encoded_packet + *encoded_packet_size);
	char const *error= NULL;

	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 172, encoded_packet && encoded_packet_size);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 173, *encoded_packet_size>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 174, packet_type>=0 && packet_type<group_definition->packet_type_count);

	if (*encoded_packet_size + sizeof(struct packet_header)<group_definition->maximum_encoded_packet_size)
	{
		packet_header->packet_type= (char)packet_type;

		byte_swap_data(&data_00309e38.definition, packet_header, 1);

		*encoded_packet_size += sizeof(struct packet_header);
	}
	else
	{
		error= "couldn't append header to encoded packet";
	}

	bss_00456624= error;

	return error==NULL;
}
