/*
KEY_AGREEMENT.C
*/

/* ---------- headers */

#include "cseries.h"
#include "key_agreement.h"
#include "message_header.h"
#include "public_key_crypt.h"
#include "random_numbers.h"
#include "bungie_net/network/transport.h"
#include "data_packet_groups.h"

/* ---------- constants */

enum
{
	_key_agreement_initiate = 0, /* fake name */
	_key_agreement_finalize, /* fake name */
	NUMBER_OF_KEY_AGREEMENT_PACKETS, /* fake name */
};

enum
{
	KEY_AGREEMENT_PACKET_VERSION = 1, /* fake name */
	NUMBER_OF_KEY_AGREEMENT_PACKET_CLASSES = 1, /* fake name */
	MAXIMUM_KEY_AGREEMENT_DECODED_PACKET_SIZE = 96, /* fake name */
	MAXIMUM_KEY_AGREEMENT_PACKET_SIZE = 128, /* fake name */
	KEY_AGREEMENT_BUFFER_SIZE = 512, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct message_initiate_key_agreement /* fake name */
{
	struct public_key prime;
	struct public_key g;
	struct public_key key;
};

struct message_finalize_key_agreement /* fake name */
{
	struct public_key key;
};

/* ---------- prototypes */

static char get_key_agreement_packet_type(message_header const *msgptr);
static boolean decode_key_agreement_packet(void *decoded_packet, void const *encoded_packet, short *encoded_packet_size, short *packet_type, short *packet_version, short expected_packet_class);
static boolean encode_key_agreement_packet(void const *decoded_packet, void *encoded_packet, short *encoded_packet_size, short packet_type, short packet_version);
static message_header *create_key_agreement_message(short packet_type, void const *packet, void *buffer, word buffer_size);
static message_header *create_message_initiate_key_agreement(struct public_key const *prime, struct public_key const *g, struct public_key const *key, void *buffer, word buffer_size);
static message_header *create_message_finalize_key_agreement(struct public_key const *key, void *buffer, word buffer_size);

/* ---------- globals */

static struct data_packet_field initiate_fields[] = /* fake name */
{
	{__pack_long, NUMBER_OF_PUBLIC_KEY_DWORDS, 0, 0, 0},
	{__pack_long, NUMBER_OF_PUBLIC_KEY_DWORDS, 0, 0, 0},
	{__pack_long, NUMBER_OF_PUBLIC_KEY_DWORDS, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition initiate_definition = /* fake name */
{
	"message_initiate_key_agreement_packet",
	0,
	sizeof(struct message_initiate_key_agreement),
	KEY_AGREEMENT_PACKET_VERSION,
	initiate_fields,
	FALSE,
};

static struct data_packet_field finalize_fields[] = /* fake name */
{
	{__pack_long, NUMBER_OF_PUBLIC_KEY_DWORDS, 0, 0, 0},
	{__pack_end, 0, 0, 0, 0},
};

static struct data_packet_definition finalize_definition = /* fake name */
{
	"message_finalize_key_agreement_packet",
	0,
	sizeof(struct message_finalize_key_agreement),
	KEY_AGREEMENT_PACKET_VERSION,
	finalize_fields,
	FALSE,
};

static struct data_packet_group_packet key_agreement_packets[NUMBER_OF_KEY_AGREEMENT_PACKETS] = /* fake name */
{
	{0, &initiate_definition},
	{0, &finalize_definition},
};

static struct data_packet_group_definition key_agreement_packets_group =
{
	"key_agreement_packets_group",
	NUMBER_OF_KEY_AGREEMENT_PACKETS,
	NUMBER_OF_KEY_AGREEMENT_PACKET_CLASSES,
	MAXIMUM_KEY_AGREEMENT_DECODED_PACKET_SIZE,
	MAXIMUM_KEY_AGREEMENT_PACKET_SIZE,
	key_agreement_packets,
};

static byte key_agreement_buffer[KEY_AGREEMENT_BUFFER_SIZE]; /* fake name */

/* ---------- public code */

long is_message_encryption_key_message(
	message_header const *msgptr,
	word message_size,
	byte *packet_type)
{
	long result;
	word flags;
	char encoded_packet_type;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\key_agreement.c", 196, msgptr && packet_type);

	flags = GET_MESSAGE_FLAGS(*msgptr);
	encoded_packet_type = ((char const *)msgptr)[message_size - 1];
	*packet_type = encoded_packet_type;

	if (TEST_FLAG(flags, _message_key_agreement_bit))
	{
		byte message_type = GET_MESSAGE_TYPE(*msgptr);

		if (message_type == _message_type_packet &&
			(encoded_packet_type == _key_agreement_initiate || encoded_packet_type == _key_agreement_finalize))
		{
			result = TRUE;
		}
		else
		{
			result = FALSE;
		}
	}
	else
	{
		result = FALSE;
	}

	return result;
}

boolean initiate_key_exchange(
	struct transport_endpoint *endpoint,
	struct public_key *key,
	struct public_key *prime,
	struct public_key *secret)
{
	struct public_key g;
	message_header *message;
	boolean success = TRUE;

	generate_key_parameters(prime, secret, &g);
	generate_public_key(prime, secret, &g, key);
	message = create_message_initiate_key_agreement(prime, &g, key, key_agreement_buffer, sizeof(key_agreement_buffer));

	if (message)
	{
		short length = GET_MESSAGE_SIZE(*message);

		byte_swap_message_header(message, _byte_order_network);

		if (write_endpoint(endpoint, message, length) != length)
		{
			success = FALSE;
		}
	}
	else
	{
		success = FALSE;
	}

	return success;
}

boolean complete_key_exchange(
	struct transport_endpoint *endpoint,
	message_header const *msgptr,
	struct public_key const *prime,
	struct public_key *secret,
	struct public_key *private_key)
{
	word length;
	short packet_size;
	short packet_type;
	byte message_type;
	struct message_initiate_key_agreement initiate_packet;
	struct message_finalize_key_agreement finalize_packet;
	struct public_key public_key;
	short packet_version = KEY_AGREEMENT_PACKET_VERSION;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\key_agreement.c", 261, msgptr && prime && secret && private_key);

	length = GET_MESSAGE_SIZE(*msgptr);
	message_type = GET_MESSAGE_TYPE(*msgptr);
	packet_size = length - sizeof(message_header);

	if (message_type == _message_type_packet)
	{
		packet_type = get_key_agreement_packet_type(msgptr);

		switch (packet_type)
		{
		case _key_agreement_initiate:
			if (decode_key_agreement_packet(&initiate_packet, msgptr + 1, &packet_size, &packet_type, &packet_version, 0))
			{
				message_header *message;

				secret->dwords[0] = randomrange(MINIMUM_KEY_SECRET, initiate_packet.prime.dwords[0] - 2);
				secret->dwords[1] = randomrange(MINIMUM_KEY_SECRET, initiate_packet.prime.dwords[1] - 2);

				generate_public_key(&initiate_packet.prime, secret, &initiate_packet.g, &public_key);
				message = create_message_finalize_key_agreement(&public_key, key_agreement_buffer, sizeof(key_agreement_buffer));

				if (message)
				{
					short message_length = GET_MESSAGE_SIZE(*message);

					byte_swap_message_header(message, _byte_order_network);

					if (write_endpoint(endpoint, message, message_length) == message_length)
					{
						generate_private_key(&initiate_packet.key, &initiate_packet.prime, secret, private_key);
						return TRUE;
					}
				}
			}
			break;
		case _key_agreement_finalize:
			if (decode_key_agreement_packet(&finalize_packet, msgptr + 1, &packet_size, &packet_type, &packet_version, 0))
			{
				generate_private_key(&finalize_packet.key, prime, secret, private_key);
				return TRUE;
			}
			break;
		}
	}

	return FALSE;
}

void initialize_key_agreement_packets(
	void)
{
	data_packet_group_initialize(&key_agreement_packets_group);

	return;
}

/* ---------- private code */

static char get_key_agreement_packet_type(
	message_header const *msgptr)
{
	byte message_type;
	word length = GET_MESSAGE_SIZE(*msgptr);

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\key_agreement.c", 77, (message_type= GET_MESSAGE_TYPE(*msgptr)) == _message_type_packet);

	return ((char const *)msgptr)[length - 1];
}

static boolean decode_key_agreement_packet(
	void *decoded_packet,
	void const *encoded_packet,
	short *encoded_packet_size,
	short *packet_type,
	short *packet_version,
	short expected_packet_class)
{
	return data_packet_group_decode_packet(
		&key_agreement_packets_group,
		decoded_packet,
		encoded_packet,
		encoded_packet_size,
		packet_type,
		packet_version,
		expected_packet_class);
}

static boolean encode_key_agreement_packet(
	void const *decoded_packet,
	void *encoded_packet,
	short *encoded_packet_size,
	short packet_type,
	short packet_version)
{
	return data_packet_group_encode_packet(
		&key_agreement_packets_group,
		decoded_packet,
		encoded_packet,
		encoded_packet_size,
		packet_type,
		packet_version);
}

static message_header *create_key_agreement_message(
	short packet_type,
	void const *packet,
	void *buffer,
	word buffer_size)
{
	byte encoded_packet[MAXIMUM_KEY_AGREEMENT_PACKET_SIZE] = {0};
	message_header *message = NULL;
	short packet_size = sizeof(encoded_packet);

	if (encode_key_agreement_packet(packet, encoded_packet, &packet_size, packet_type, KEY_AGREEMENT_PACKET_VERSION))
	{
		message = create_message(_message_type_packet, encoded_packet, packet_size, buffer, buffer_size);

		if (message)
		{
			SET_MESSAGE_FLAGS(*message, FLAG(_message_key_agreement_bit));
		}
	}

	return message;
}

static message_header *create_message_initiate_key_agreement(
	struct public_key const *prime,
	struct public_key const *g,
	struct public_key const *key,
	void *buffer,
	word buffer_size)
{
	struct message_initiate_key_agreement packet;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\key_agreement.c", 162, prime && g && key);

	packet.prime = *prime;
	packet.g = *g;
	packet.key = *key;

	return create_key_agreement_message(_key_agreement_initiate, &packet, buffer, buffer_size);
}

static message_header *create_message_finalize_key_agreement(
	struct public_key const *key,
	void *buffer,
	word buffer_size)
{
	struct message_finalize_key_agreement packet;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\key_agreement.c", 179, key);

	packet.key = *key;

	return create_key_agreement_message(_key_agreement_finalize, &packet, buffer, buffer_size);
}
