/*
CRC.C
*/

/* ---------- headers */

#include "cseries.h"

/* ---------- constants */

enum
{
	CRC_NEW = -1,
	CRC_TABLE_SIZE = 256,
	CRC32_POLYNOMIAL = 0xEDB88320
};

/* ---------- prototypes */

static void build_crc_table(unsigned long *crc_table);

/* ---------- public code */

void crc_new(
	unsigned long *crc_reference)
{
	*crc_reference = CRC_NEW;

	return;
}

void crc_checksum_buffer(
	unsigned long *crc_reference,
	void const *buffer,
	long buffer_size)
{
	static unsigned long crc_table[256];
	static boolean crc_table_built;

	unsigned long crc;
	unsigned long temp1;
	unsigned long temp2;

	match_assert("c:\\halo\\SOURCE\\memory\\crc.c", 42, buffer_size>=0);

	if (!crc_table_built)
	{
		build_crc_table(crc_table);
		crc_table_built = TRUE;
	}

	crc = *crc_reference;

	while (buffer_size-->0)
	{
		temp1 = (crc>>8) & 0x00FFFFFF;
		temp2 = crc_table[(crc ^ *((byte *)buffer)++) & 0xFF];
		crc = temp1 ^ temp2;
	}

	*crc_reference = crc;

	return;
}

/* ---------- private code */

static void build_crc_table(
	unsigned long *crc_table)
{
	short table_index;

	for (table_index = 0; table_index<CRC_TABLE_SIZE; ++table_index)
	{
		unsigned long crc = table_index;
		short round;

		for (round = 0; round<8; ++round)
		{
			if (crc&1)
			{
				crc = (crc>>1) ^ CRC32_POLYNOMIAL;
			}
			else
			{
				crc = crc>>1;
			}
		}

		crc_table[table_index] = crc;
	}
}
