/*
MESSAGE_ENCRYPTION.C
*/

/* ---------- headers */

#include "cseries.h"
#include "message_encryption.h"
#include "message_header.h"

/* ---------- constants */

enum
{
	MESSAGE_KEY_LONGS = 2, /* fake name */
	MESSAGE_KEY_SIZE = MESSAGE_KEY_LONGS*sizeof(unsigned long), /* fake name */
	TEA_KEY_LONGS = 4, /* fake name */
	TEA_BLOCK_LONGS = 2, /* fake name */
	TEA_BLOCK_SIZE = TEA_BLOCK_LONGS*sizeof(unsigned long), /* fake name */
	TEA_ROUNDS = 32, /* fake name */
};

/* ---------- macros */

#define TEA_DELTA 0x9E3779B9 /* fake name */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void message_encrypt(
	message_header *msgptr,
	unsigned long const *key)
{
	word flags;
	word length;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_encryption.c", 31, msgptr && key);

	flags = GET_MESSAGE_FLAGS(*msgptr);
	length = GET_MESSAGE_SIZE(*msgptr);

	if (!TEST_FLAG(flags, _message_encrypted_bit))
	{
		long tea_key[TEA_KEY_LONGS];
		word i;
		word block_count = (length - sizeof(message_header)) / TEA_BLOCK_SIZE;
		short tail_size = (length - sizeof(message_header)) % TEA_BLOCK_SIZE;
		unsigned long *block = (unsigned long *)(msgptr + 1);

		tea_key[2] = key[0];
		tea_key[0] = key[0];
		tea_key[3] = key[1];
		tea_key[1] = key[1];

		for (i = 0; i != block_count; i++)
		{
			tea_encipher(block, block, tea_key);
			block += TEA_BLOCK_LONGS;
		}

		if (tail_size)
		{
			reversible_crypt((byte *)block, tail_size, (byte const *)key, MESSAGE_KEY_SIZE);
		}

		SET_FLAG(flags, _message_encrypted_bit, TRUE);
		match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_encryption.c", 76, (0<=flags) && ((flags)<=MESSAGE_FLAG_BITS_MASK));
		SET_MESSAGE_FLAGS(*msgptr, flags);
	}

	return;
}

void message_decrypt(
	message_header *msgptr,
	unsigned long const *key)
{
	word flags;
	word length;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_encryption.c", 88, msgptr && key);

	flags = GET_MESSAGE_FLAGS(*msgptr);
	length = GET_MESSAGE_SIZE(*msgptr);

	if (TEST_FLAG(flags, _message_encrypted_bit))
	{
		long tea_key[TEA_KEY_LONGS];
		word i;
		word block_count = (length - sizeof(message_header)) / TEA_BLOCK_SIZE;
		short tail_size = (length - sizeof(message_header)) % TEA_BLOCK_SIZE;
		unsigned long *block = (unsigned long *)(msgptr + 1);

		tea_key[2] = key[0];
		tea_key[0] = key[0];
		tea_key[3] = key[1];
		tea_key[1] = key[1];

		for (i = 0; i != block_count; i++)
		{
			tea_decipher(block, block, tea_key);
			block += TEA_BLOCK_LONGS;
		}

		if (tail_size)
		{
			reversible_crypt((byte *)block, tail_size, (byte const *)key, MESSAGE_KEY_SIZE);
		}

		SET_FLAG(flags, _message_encrypted_bit, FALSE);
		match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_encryption.c", 131, (0<=flags) && ((flags)<=MESSAGE_FLAG_BITS_MASK));
		SET_MESSAGE_FLAGS(*msgptr, flags);
	}

	return;
}

void reversible_crypt(
	byte *data,
	long data_size,
	byte const *key,
	long key_size)
{
	long i = 0;
	long key_index = 0;
	long direction = 1;

	while (i < data_size)
	{
		data[i] = ~(data[i] ^ key[key_index]);
		key_index += direction;
		i++;

		if (key_index == key_size || key_index < 0)
		{
			direction = -direction;
			key_index += direction;
		}
	}

	return;
}

void tea_encipher(
	unsigned long const *input,
	unsigned long *output,
	long const *key)
{
	long i;
	unsigned long y = input[0];
	unsigned long z = input[1];
	unsigned long sum = 0;
	unsigned long delta = TEA_DELTA;
	long a = key[0];
	long b = key[1];
	long c = key[2];
	long d = key[3];

	for (i = 0; i < TEA_ROUNDS; i++)
	{
		sum += delta;
		y += ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		z += ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
	}

	output[0] = y;
	output[1] = z;

	return;
}

void tea_decipher(
	unsigned long const *input,
	unsigned long *output,
	long const *key)
{
	long i;
	unsigned long y = input[0];
	unsigned long z = input[1];
	unsigned long sum = TEA_DELTA*TEA_ROUNDS;
	unsigned long delta = TEA_DELTA;
	long a = key[0];
	long b = key[1];
	long c = key[2];
	long d = key[3];

	for (i = 0; i < TEA_ROUNDS; i++)
	{
		z -= ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
		y -= ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		sum -= delta;
	}

	output[0] = y;
	output[1] = z;

	return;
}
