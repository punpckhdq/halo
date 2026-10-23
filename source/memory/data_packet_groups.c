/*
DATA_PACKET_GROUPS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "data_packet_groups.h"

/* ---------- structures */

struct packet_header
{
	char type;
};

/* ---------- globals */

static byte_swap_code packet_header_bs_codes[] = {_begin_bs_array, 1, sizeof(char), _end_bs_array};
static struct byte_swap_definition packet_header_bs_definition = {"packet_header", sizeof(struct packet_header), packet_header_bs_codes, BYTE_SWAP_DEFINITION_SIGNATURE};
static char *global_data_packet_groups_error_string;

/* ---------- public code */

void data_packet_group_initialize(
	struct data_packet_group_definition *group_definition)
{
	short packet_index;

	for (packet_index = 0; packet_index<group_definition->packet_type_count; packet_index++)
	{
		struct data_packet_group_packet *packet = &group_definition->packets[packet_index];

		if (packet->definition)
		{
			match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 40, packet->packet_class>=0 && packet->packet_class<group_definition->packet_class_count);
			match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 41, packet->definition->size<=group_definition->maximum_decoded_packet_size);
			match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 42, packet->definition->size + sizeof(struct packet_header)<=group_definition->maximum_encoded_packet_size);
			data_packet_verify(packet->definition);
		}
	}

	return;
}

char *data_packet_groups_get_error(
	void)
{
	char *result = global_data_packet_groups_error_string;

	global_data_packet_groups_error_string = NULL;
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 57, result);

	return result;
}

boolean data_packet_group_decode_packet(
	struct data_packet_group_definition *group_definition,
	void *decoded_packet,
	void *encoded_packet,
	short *encoded_packet_size,
	short *packet_type,
	short *packet_version,
	short expected_packet_class)
{
	char *error_string = NULL;

	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 73, decoded_packet);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 74, encoded_packet && encoded_packet_size);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 75, packet_type && packet_version);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 77, expected_packet_class>=0 && expected_packet_class<group_definition->packet_class_count);

	if (*encoded_packet_size>=sizeof(struct packet_header))
	{
		struct packet_header *header = (struct packet_header *)((byte *)encoded_packet + *encoded_packet_size - sizeof(struct packet_header));

		byte_swap_data(&packet_header_bs_definition, header, 1);

		if (header->type>=0 && header->type<group_definition->packet_type_count)
		{
			struct data_packet_group_packet *packet = &group_definition->packets[header->type];

			if (packet->packet_class==expected_packet_class)
			{
				*encoded_packet_size -= sizeof(struct packet_header);

				if (!packet->definition || data_packet_decode(packet->definition, encoded_packet, *encoded_packet_size, decoded_packet, packet_version, NULL))
				{
					*packet_type = header->type;
				}
				else
				{
					error_string = "got packet which wouldn't decode";
				}
			}
			else
			{
				error_string = "got packet with mismatched class";
			}
		}
		else
		{
			error_string = "got packet with bad type";
		}
	}
	else
	{
		error_string = "got packet with no header";
	}

	global_data_packet_groups_error_string = error_string;

	return !error_string;
}

boolean data_packet_group_encode_packet(
	struct data_packet_group_definition *group_definition,
	void *decoded_packet,
	void *encoded_packet,
	short *encoded_packet_size,
	short packet_type,
	short packet_version)
{
	char *error_string = NULL;
	struct data_packet_group_packet *packet;

	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 132, group_definition);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 133, packet_type>=0 && packet_type<group_definition->packet_type_count);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 134, encoded_packet && encoded_packet_size);

	packet = &group_definition->packets[packet_type];
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 139, packet->definition);

	if (data_packet_encode(packet->definition, packet_version, (void *)decoded_packet, encoded_packet, encoded_packet_size, (short)group_definition->maximum_encoded_packet_size))
	{
		if (!data_packet_group_append_packet_header(group_definition, encoded_packet, encoded_packet_size, packet_type))
		{
			error_string = global_data_packet_groups_error_string;
		}
	}
	else
	{
		error_string = "couldn't encode packet";
	}

	global_data_packet_groups_error_string = error_string;

	return !error_string;
}

boolean data_packet_group_append_packet_header(
	struct data_packet_group_definition *group_definition,
	void *encoded_packet,
	short *encoded_packet_size,
	short packet_type)
{
	struct packet_header *header = (struct packet_header *)((byte *)encoded_packet + *encoded_packet_size);
	char *error_string = NULL;

	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 172, encoded_packet && encoded_packet_size);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 173, *encoded_packet_size>=0);
	match_assert("c:\\halo\\SOURCE\\memory\\data_packet_groups.c", 174, packet_type>=0 && packet_type<group_definition->packet_type_count);

	if (*encoded_packet_size + sizeof(struct packet_header)<group_definition->maximum_encoded_packet_size)
	{
		header->type = (char)packet_type;
		byte_swap_data(&packet_header_bs_definition, header, 1);
		*encoded_packet_size += sizeof(struct packet_header);
	}
	else
	{
		error_string = "couldn't append header to encoded packet";
	}

	global_data_packet_groups_error_string = error_string;

	return !error_string;
}
