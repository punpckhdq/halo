/*
CIRCULAR_QUEUE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "circular_queue.h"

/* ---------- constants */

#define CIRCULAR_QUEUE_SIGNATURE 'circ'

/* ---------- structures */

struct circular_queue
{
	char *name; /* fake name */
	unsigned long signature; /* fake name */
	long read_offset;
	long write_offset;
	long buffer_size;
	byte *buffer; /* fake name */
};

/* ---------- prototypes */

static void circular_queue_verify(struct circular_queue *queue);

/* ---------- public code */

void circular_queue_reset(
	struct circular_queue *queue)
{
	queue->write_offset = 0;
	queue->read_offset = 0;

	return;
}

struct circular_queue *circular_queue_new(
	char *name,
	long buffer_size)
{
	struct circular_queue *queue = match_malloc("c:\\halo\\SOURCE\\memory\\circular_queue.c", 52, sizeof(struct circular_queue) + buffer_size + 1);

	if (queue)
	{
		memset(queue, 0, sizeof(struct circular_queue));
		queue->name = name;
		queue->signature = CIRCULAR_QUEUE_SIGNATURE;
		queue->buffer_size = buffer_size + 1;
		queue->buffer = (byte *)(queue + 1);
		circular_queue_verify(queue);
	}

	return queue;
}

void circular_queue_delete(
	struct circular_queue *queue)
{
	circular_queue_verify(queue);
	match_free("c:\\halo\\SOURCE\\memory\\circular_queue.c", 72, queue);

	return;
}

long circular_queue_size(
	struct circular_queue *queue)
{
	long size;

	circular_queue_verify(queue);
	size = queue->write_offset - queue->read_offset;
	if (size < 0)
	{
		size += queue->buffer_size;
	}

	return size;
}

long circular_queue_free_space(
	struct circular_queue *queue)
{
	long free_space = queue->buffer_size - circular_queue_size(queue) - 1;

	return free_space;
}

boolean circular_queue_queue_data(
	struct circular_queue *queue,
	void *data,
	long data_size)
{
	boolean success = FALSE;

	circular_queue_verify(queue);
	match_assert("c:\\halo\\SOURCE\\memory\\circular_queue.c", 116, data && data_size>0 && data_size<queue->buffer_size);

	if (circular_queue_size(queue) + data_size < queue->buffer_size)
	{
		long bytes_to_end = queue->buffer_size - queue->write_offset;

		if (data_size >= bytes_to_end)
		{
			memcpy(queue->buffer + queue->write_offset, data, bytes_to_end);
			queue->write_offset = 0;
			// TODO: this is supposed to be offset_pointer in cseries.h
			// indicated by unoptimized builds to be an inline
			data = (byte *)data + bytes_to_end;
			data_size -= bytes_to_end;
		}

		if (data_size > 0)
		{
			memcpy(queue->buffer + queue->write_offset, data, data_size);
			queue->write_offset += data_size;
		}

		match_assert("c:\\halo\\SOURCE\\memory\\circular_queue.c", 136, queue->write_offset>=0 && queue->write_offset<queue->buffer_size);
		success = TRUE;
	}

	return success;
}

boolean circular_queue_dequeue_data(
	struct circular_queue *queue,
	void *data,
	long data_size,
	boolean advance)
{
	boolean success = FALSE;

	circular_queue_verify(queue);
	match_assert("c:\\halo\\SOURCE\\memory\\circular_queue.c", 153, data && data_size>0 && data_size<queue->buffer_size);

	if (data_size <= circular_queue_size(queue))
	{
		long bytes_to_end = queue->buffer_size - queue->read_offset;
		long read_offset = queue->read_offset;

		if (data_size >= bytes_to_end)
		{
			memcpy(data, queue->buffer + read_offset, bytes_to_end);
			read_offset = 0;
			data = (byte *)data + bytes_to_end;
			data_size -= bytes_to_end;
		}

		if (data_size > 0)
		{
			memcpy(data, queue->buffer + read_offset, data_size);
			read_offset += data_size;
		}

		match_assert("c:\\halo\\SOURCE\\memory\\circular_queue.c", 174, read_offset>=0 && read_offset<queue->buffer_size);

		if (advance)
		{
			queue->read_offset = read_offset;
		}
		success = TRUE;
	}

	return success;
}

/* ---------- private code */

static void circular_queue_verify(
	struct circular_queue *queue)
{
	boolean valid = FALSE;

	if (queue &&
		queue->signature == CIRCULAR_QUEUE_SIGNATURE &&
		queue->buffer &&
		queue->buffer_size > 0 &&
		queue->read_offset >= 0 && queue->read_offset < queue->buffer_size &&
		queue->write_offset >= 0 && queue->write_offset < queue->buffer_size)
	{
		valid = TRUE;
	}
	match_vassert("c:\\halo\\SOURCE\\memory\\circular_queue.c", 204, valid, csprintf(temporary, "the circular queue @%p appears to be corrupt.", queue));

	return;
}
