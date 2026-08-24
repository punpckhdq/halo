/*
KEY_AGREEMENT.C
*/

/* ---------- headers */

#include "cseries.h"

#include "public_key_crypt.h"
#include "memory/data_packets.h"
#include "message_header.h"
#include "random_numbers.h"
#include "bungie_net/network/transport.h"

/* ---------- constants */

enum
{
	_key_agreement_packet_initiate= 0,
	_key_agreement_packet_finalize,
	NUMBER_OF_KEY_AGREEMENT_PACKETS,

	KEY_AGREEMENT_PACKET_VERSION= 1,
	KEY_AGREEMENT_ENCODED_PACKET_SIZE= 128,
	KEY_AGREEMENT_MESSAGE_BUFFER_SIZE= 512
};

/* ---------- structures */

struct message_initiate_key_agreement_packet
{
	struct public_key prime;
	struct public_key g;
	struct public_key key;
};

struct message_finalize_key_agreement_packet
{
	struct public_key key;
};

/* ---------- prototypes */

static char get_key_agreement_packet_type(word *msgptr);
static boolean decode_key_agreement_packet(void *decoded_packet, void *encoded_packet, long *encoded_packet_size, short *packet_type, long *packet_version, short expected_packet_class);
static boolean encode_key_agreement_packet(void *decoded_packet, void *encoded_packet, long *encoded_packet_size, short packet_type, long packet_version);
static void *create_key_agreement_message(short packet_type, void *decoded_packet, void *buffer, word buffer_size);
static void *create_message_initiate_key_agreement(struct public_key *prime, struct public_key *g, struct public_key *key, void *buffer, word buffer_size);
static void *create_message_finalize_key_agreement(struct public_key *key, void *buffer, word buffer_size);

/* ---------- globals */

// the field tables, definitions and group below are one contiguous run of data
static struct data_packet_field message_initiate_key_agreement_packet_fields[]=
{
	{_data_packet_field_longs, 2, 0, 0, 0},
	{_data_packet_field_longs, 2, 0, 0, 0},
	{_data_packet_field_longs, 2, 0, 0, 0},
	{_data_packet_field_end, 0, 0, 0, 0}
};
static struct data_packet_definition message_initiate_key_agreement_packet=
{
	"message_initiate_key_agreement_packet", 0, sizeof(struct message_initiate_key_agreement_packet), KEY_AGREEMENT_PACKET_VERSION,
	message_initiate_key_agreement_packet_fields, FALSE
};
static struct data_packet_field message_finalize_key_agreement_packet_fields[]=
{
	{_data_packet_field_longs, 2, 0, 0, 0},
	{_data_packet_field_end, 0, 0, 0, 0}
};
static struct data_packet_definition message_finalize_key_agreement_packet=
{
	"message_finalize_key_agreement_packet", 0, sizeof(struct message_finalize_key_agreement_packet), KEY_AGREEMENT_PACKET_VERSION,
	message_finalize_key_agreement_packet_fields, FALSE
};
static struct data_packet_entry key_agreement_packets[NUMBER_OF_KEY_AGREEMENT_PACKETS]=
{
	{0, 0, &message_initiate_key_agreement_packet},
	{0, 0, &message_finalize_key_agreement_packet}
};
static struct data_packet_group_definition key_agreement_packets_group=
{
	"key_agreement_packets_group", NUMBER_OF_KEY_AGREEMENT_PACKETS, 1, 96, KEY_AGREEMENT_ENCODED_PACKET_SIZE, key_agreement_packets
};

static byte bss_0031c520[KEY_AGREEMENT_MESSAGE_BUFFER_SIZE]; // both exchanges build their message here

/* ---------- private code */

static char get_key_agreement_packet_type(
	word *msgptr)
{
	word message_size= MESSAGE_GET_SIZE(msgptr);
	byte message_type;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\key_agreement.c", 77, (message_type= GET_MESSAGE_TYPE(*msgptr)) == _message_type_packet);

	return ((char *)msgptr)[message_size-1];
}

static boolean decode_key_agreement_packet(
	void *decoded_packet,
	void *encoded_packet,
	long *encoded_packet_size,
	short *packet_type,
	long *packet_version,
	short expected_packet_class)
{
	return data_packet_group_decode_packet(&key_agreement_packets_group, decoded_packet, encoded_packet,
		encoded_packet_size, packet_type, packet_version, expected_packet_class);
}

static boolean encode_key_agreement_packet(
	void *decoded_packet,
	void *encoded_packet,
	long *encoded_packet_size,
	short packet_type,
	long packet_version)
{
	return data_packet_group_encode_packet(&key_agreement_packets_group, decoded_packet, encoded_packet,
		encoded_packet_size, packet_type, packet_version);
}

static void *create_key_agreement_message(
	short packet_type,
	void *decoded_packet,
	void *buffer,
	word buffer_size)
{
	word *message= NULL;
	byte encoded_packet[KEY_AGREEMENT_ENCODED_PACKET_SIZE]= {0};
	long encoded_packet_size= sizeof(encoded_packet);

	if (encode_key_agreement_packet(decoded_packet, encoded_packet, &encoded_packet_size, packet_type, KEY_AGREEMENT_PACKET_VERSION))
	{
		message= create_message(_message_type_packet, encoded_packet, encoded_packet_size, buffer, buffer_size);
		if (message)
		{
			*message= (*message&~FLAG(_message_flag_encrypted_bit))|FLAG(_message_flag_key_bit);
		}
	}

	return message;
}

static void *create_message_initiate_key_agreement(
	struct public_key *prime,
	struct public_key *g,
	struct public_key *key,
	void *buffer,
	word buffer_size)
{
	struct message_initiate_key_agreement_packet packet;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\key_agreement.c", 162, prime && g && key);

	packet.prime= *prime;
	packet.g= *g;
	packet.key= *key;

	return create_key_agreement_message(_key_agreement_packet_initiate, &packet, buffer, buffer_size);
}

static void *create_message_finalize_key_agreement(
	struct public_key *key,
	void *buffer,
	word buffer_size)
{
	struct message_finalize_key_agreement_packet packet;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\key_agreement.c", 179, key);

	packet.key= *key;

	return create_key_agreement_message(_key_agreement_packet_finalize, &packet, buffer, buffer_size);
}

/* ---------- public code */

long is_message_encryption_key_message(
	word *msgptr,
	word message_size,
	byte *packet_type)
{
	long flags;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\key_agreement.c", 196, msgptr && packet_type);

	flags= GET_MESSAGE_FLAGS(*msgptr);
	*packet_type= ((byte *)msgptr)[message_size-1];
	if (TEST_FLAG(flags, _message_flag_key_bit) && GET_MESSAGE_TYPE(*msgptr)==_message_type_packet &&
		(*packet_type==_key_agreement_packet_initiate || *packet_type==_key_agreement_packet_finalize))
	{
		return TRUE;
	}

	return FALSE;
}

boolean initiate_key_exchange(
	transport_endpoint_ref endpoint,
	struct public_key *public_key,
	struct public_key *p,
	struct public_key *x)
{
	boolean success= TRUE;
	struct public_key g;
	word *message;
	short message_size;

	generate_key_parameters(p, x, &g);
	generate_public_key(p, x, &g, public_key);
	message= create_message_initiate_key_agreement(p, &g, public_key, bss_0031c520, sizeof(bss_0031c520));
	if (message)
	{
		message_size= MESSAGE_GET_SIZE(message);
		byte_swap_message_header((struct message_header *)message, _byte_order_network);
		if (write_endpoint(endpoint, message, message_size)!=message_size)
		{
			success= FALSE;
		}
	}
	else
	{
		success= FALSE;
	}

	return success;
}

boolean complete_key_exchange(
	transport_endpoint_ref endpoint,
	word *msgptr,
	struct public_key *prime,
	struct public_key *secret,
	struct public_key *private_key)
{
	boolean success= FALSE;
	long packet_version= KEY_AGREEMENT_PACKET_VERSION;
	long encoded_packet_size;
	word message_size;
	byte message_type;
	short packet_type;
	struct message_finalize_key_agreement_packet finalize_packet;
	struct public_key public_key;
	struct message_initiate_key_agreement_packet initiate_packet;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\key_agreement.c", 261, msgptr && prime && secret && private_key);

	message_size= MESSAGE_GET_SIZE(msgptr);
	if ((message_type= GET_MESSAGE_TYPE(*msgptr))==_message_type_packet)
	{
		packet_type= get_key_agreement_packet_type(msgptr);
		encoded_packet_size= message_size-sizeof(word);
		switch (packet_type)
		{
		case _key_agreement_packet_finalize:
			if (decode_key_agreement_packet(&finalize_packet, msgptr+1, &encoded_packet_size, &packet_type, &packet_version, 0))
			{
				generate_private_key(&finalize_packet.key, prime, secret, private_key);
				success= TRUE;
			}
			break;
		case _key_agreement_packet_initiate:
			if (decode_key_agreement_packet(&initiate_packet, msgptr+1, &encoded_packet_size, &packet_type, &packet_version, 0))
			{
				word *message;
				short message_size;

				secret->dwords[0]= randomrange(255, initiate_packet.prime.dwords[0]-2);
				secret->dwords[1]= randomrange(255, initiate_packet.prime.dwords[1]-2);
				generate_public_key(&initiate_packet.prime, secret, &initiate_packet.g, &public_key);
				message= create_message_finalize_key_agreement(&public_key, bss_0031c520, sizeof(bss_0031c520));
				if (message)
				{
					message_size= MESSAGE_GET_SIZE(message);
					byte_swap_message_header((struct message_header *)message, _byte_order_network);
					if (write_endpoint(endpoint, message, message_size)==message_size)
					{
						generate_private_key(&initiate_packet.key, &initiate_packet.prime, secret, private_key);
						success= TRUE;
					}
				}
			}
			break;
		}
	}

	return success;
}

void initialize_key_agreement_packets(
	void)
{
	data_packet_group_initialize(&key_agreement_packets_group);

	return;
}
