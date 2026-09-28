/*
DATA_COMPRESS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "zlib.h"

/* ---------- structures */

struct compressed_header
{
	unsigned long size;
};

/* ---------- public code */

boolean data_compress(
	void const *uncompressed_buffer,
	unsigned long uncompressed_size,
	void *compressed_buffer,
	unsigned long *compressed_size,
	unsigned long maximum_compressed_size)
{
	boolean success = FALSE;

	if (maximum_compressed_size>=4)
	{
		long compress_status;

		struct compressed_header *header = (struct compressed_header*)compressed_buffer;
		header->size = SWAP4(uncompressed_size);

		*compressed_size = maximum_compressed_size - 4;

		compress_status = compress2((byte*)compressed_buffer + sizeof(struct compressed_header), compressed_size, (byte*)uncompressed_buffer, uncompressed_size, 9);

		if (compress_status==Z_OK)
		{
			*compressed_size += 4;
			success = TRUE;
		}
	}

	return success;
}

unsigned long data_decompressed_size(
	void const *compressed_buffer,
	unsigned long compressed_size)
{
	struct compressed_header *header = (struct compressed_header*)compressed_buffer;
	unsigned long size = 0;

	if (compressed_size>=4)
	{
		size = SWAP4(header->size);
	}

	return size;
}

boolean data_decompress(
	void const *compressed_buffer,
	unsigned long compressed_size,
	void *decompressed_buffer,
	unsigned long *decompressed_size)
{
	boolean success = FALSE;
	long uncompress_status;

	*decompressed_size = data_decompressed_size(compressed_buffer, compressed_size);

	uncompress_status = uncompress((byte*)decompressed_buffer, decompressed_size, (byte*)compressed_buffer + sizeof(struct compressed_header), compressed_size);

	if (uncompress_status==Z_OK)
	{
		success = TRUE;
	}

	return success;
}
