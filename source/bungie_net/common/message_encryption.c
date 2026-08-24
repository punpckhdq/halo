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
	TEA_KEY_LONGS= 4,
	TEA_BLOCK_LONGS= 2
};

/* ---------- public code */

void message_encrypt(
	word *msgptr,
	unsigned long const key[2])
{
	word size, flags;
	word block_count;
	short remainder;
	unsigned long *block;
	long tea_key[TEA_KEY_LONGS];
	word i;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_encryption.c", 31, msgptr && key);

	flags= MESSAGE_GET_FLAGS(msgptr);
	size= MESSAGE_GET_SIZE(msgptr);
	if (!TEST_FLAG(flags, _message_flag_encrypted_bit))
	{
		block_count= (size-sizeof(word))/(TEA_BLOCK_LONGS*sizeof(unsigned long));
		remainder= (size-sizeof(word))%(TEA_BLOCK_LONGS*sizeof(unsigned long));
		block= (unsigned long *)(msgptr+1);
		tea_key[0]= tea_key[2]= key[0];
		tea_key[1]= tea_key[3]= key[1];
		for (i= 0; i!=block_count; i++)
		{
			tea_encipher(block, block, tea_key);
			block+= TEA_BLOCK_LONGS;
		}
		if (remainder)
		{
			reversible_crypt((byte *)block, remainder, (byte const *)key, 2*sizeof(unsigned long));
		}
		flags|= FLAG(_message_flag_encrypted_bit);
		MATCH_SET_MESSAGE_FLAGS("c:\\halo\\SOURCE\\bungie_net\\common\\message_encryption.c", 76, msgptr, flags);
	}
}

void message_decrypt(
	word *msgptr,
	unsigned long const key[2])
{
	word size, flags;
	word block_count;
	short remainder;
	unsigned long *block;
	long tea_key[TEA_KEY_LONGS];
	word i;

	match_assert("c:\\halo\\SOURCE\\bungie_net\\common\\message_encryption.c", 88, msgptr && key);

	flags= MESSAGE_GET_FLAGS(msgptr);
	size= MESSAGE_GET_SIZE(msgptr);
	if (TEST_FLAG(flags, _message_flag_encrypted_bit))
	{
		block_count= (size-sizeof(word))/(TEA_BLOCK_LONGS*sizeof(unsigned long));
		remainder= (size-sizeof(word))%(TEA_BLOCK_LONGS*sizeof(unsigned long));
		block= (unsigned long *)(msgptr+1);
		tea_key[0]= tea_key[2]= key[0];
		tea_key[1]= tea_key[3]= key[1];
		for (i= 0; i!=block_count; i++)
		{
			tea_decipher(block, block, tea_key);
			block+= TEA_BLOCK_LONGS;
		}
		if (remainder)
		{
			reversible_crypt((byte *)block, remainder, (byte const *)key, 2*sizeof(unsigned long));
		}
		flags&= ~FLAG(_message_flag_encrypted_bit);
		MATCH_SET_MESSAGE_FLAGS("c:\\halo\\SOURCE\\bungie_net\\common\\message_encryption.c", 131, msgptr, flags);
	}
}

void reversible_crypt(
	byte *data,
	long data_size,
	byte const *key,
	long key_size)
{
	long data_index= 0;
	long key_index= 0;
	long direction= 1;

	while (data_index<data_size)
	{
		data[data_index]= ~(data[data_index]^key[key_index]);
		key_index+= direction;
		data_index++;
		if (key_index==key_size || key_index<0)
		{
			direction= -direction;
			key_index+= direction;
		}
	}
}

void tea_encipher(
	unsigned long const input[2],
	unsigned long output[2],
	long const key[4])
{
	register unsigned long y= input[0], z= input[1], sum= 0, delta= 0x9E3779B9, a= key[0], b= key[1], c= key[2], d= key[3], n= 32;

	while (n-- > 0)
	{
		sum+= delta;
		y+= (z << 4)+a ^ z+sum ^ (z >> 5)+b;
		z+= (y << 4)+c ^ y+sum ^ (y >> 5)+d;
	}

	output[0]= y;
	output[1]= z;
}

void tea_decipher(
	unsigned long const input[2],
	unsigned long output[2],
	long const key[4])
{
	register unsigned long y= input[0], z= input[1], sum= 0xC6EF3720, delta= 0x9E3779B9, a= key[0], b= key[1], c= key[2], d= key[3], n= 32;

	while (n-- > 0)
	{
		z-= (y << 4)+c ^ y+sum ^ (y >> 5)+d;
		y-= (z << 4)+a ^ z+sum ^ (z >> 5)+b;
		sum-= delta;
	}

	output[0]= y;
	output[1]= z;
}
