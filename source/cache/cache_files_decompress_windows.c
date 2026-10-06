/*
CACHE_FILES_DECOMPRESS_WINDOWS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "zlib.h"
#include "cache_files_decompress_windows.h"

/* ---------- constants */

enum
{
	_copy_write_failed_bit = 0,
	_copy_read_failed_bit,
	_copy_decompression_failed_bit,
	NUMBER_OF_COPY_FLAGS,

	_copy_error_first_bit = _copy_write_failed_bit,
	_copy_error_last_bit = _copy_decompression_failed_bit,
};

enum
{
	EVENT_TIMEOUT = 5000,
	BLOCKING_PROGRESS_CHECK_SLEEP_INTERVAL = 16,
};

enum
{
	MIN_ZLIB_BUFFER_SIZE = 0x12000,
	CACHE_HEADER_SIZE = 0x800,
};

enum
{
	NUMBER_OF_READ_BUFFERS = 8,
	NUMBER_OF_WRITE_BUFFERS = 1,
};

enum
{
	FILE_BLOCK_SIZE = 0x20000,
	WRITE_FILE_BLOCK_SIZE = 0x400000,
	TOTAL_READ_WRITE_BUFFER_SIZE = NUMBER_OF_READ_BUFFERS*FILE_BLOCK_SIZE + NUMBER_OF_WRITE_BUFFERS*WRITE_FILE_BLOCK_SIZE,
	TOTAL_BUFFER_SIZE = TOTAL_READ_WRITE_BUFFER_SIZE + MIN_ZLIB_BUFFER_SIZE,
};

enum
{
	_first_read_index = 0,
	_last_read_index = NUMBER_OF_READ_BUFFERS - 1,
	_raw_read_index,
	_first_write_index,
	_last_write_index = _first_write_index + NUMBER_OF_WRITE_BUFFERS - 1,
	_raw_write_index = NUMBER_OF_WRITE_BUFFERS,

	_read_buffer_base = _first_read_index,
	_write_buffer_base = _first_write_index,
	_raw_read_offset = _read_buffer_base + _raw_read_index,
	_raw_write_offset = _write_buffer_base + _raw_write_index,
	NUMBER_OF_OVERLAPPED_STRUCTURES,
};

enum
{
	_start_dynamic_data_offset = 0x990,
	_end_dynamic_data_offset = 0xB50,
};

enum
{
	_timing_file_data = 0, /* fake name */
	_timing_read_file, /* fake name */
	_timing_write_file, /* fake name */
	_timing_zlib, /* fake name */
	_timing_zlib_during_write_file, /* fake name */
	_timing_thread_blocked, /* fake name */
	_timing_thread_blocked_on_read, /* fake name */
	_timing_thread_blocked_on_write, /* fake name */
	_timing_copying, /* fake name */
	NUMBER_OF_TIMINGS, /* fake name */
};

enum
{
	COPY_THREAD_STACK_SIZE = 16*1024, /* fake name */
	GARBAGE_BUFFER_FILL = 0xFD, /* fake name */
	GARBAGE_DYNAMIC_DATA_FILL = 0xFA, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct cache_copy_read_request
{
	short read_sequence_index;
};

struct cache_copy_write_request
{
	short write_sequence_index;
};

struct timing_globals /* fake name */
{
	long totals[NUMBER_OF_TIMINGS]; /* fake name */
	LARGE_INTEGER start_times[NUMBER_OF_TIMINGS]; /* fake name */
};

struct simple_decompressor_definition
{
	char src_name[MAX_PATH];
	struct cache_file_header header;
	volatile unsigned long flags;
	z_stream zlib_stream;
	byte *zlib_buffer;
	long zlib_buffer_size;
	byte *next_allocation;
	HANDLE copy_start_event;
	HANDLE copy_stop_event;
	HANDLE copy_complete_event;
	HANDLE progress_update_event;
	HANDLE copy_thread;
	void *allocated_buffer;
	void *read_buffers[NUMBER_OF_READ_BUFFERS];
	void *write_buffers[NUMBER_OF_WRITE_BUFFERS];
	boolean blocking;
	byte pad0[3];
	HANDLE write_file_handle;
	HANDLE read_file_handle;
	unsigned long overlapped_in_use_flags[BIT_VECTOR_SIZE_IN_LONGS(NUMBER_OF_OVERLAPPED_STRUCTURES)];
	unsigned long overlapped_completed_flags[BIT_VECTOR_SIZE_IN_LONGS(NUMBER_OF_OVERLAPPED_STRUCTURES)];
	OVERLAPPED overlapped[NUMBER_OF_OVERLAPPED_STRUCTURES];
	struct cache_copy_read_request read_requests[NUMBER_OF_READ_BUFFERS];
	struct cache_copy_write_request write_requests[NUMBER_OF_WRITE_BUFFERS];
	long read_file_size;
	long async_read_bytes_left;
	long read_bytes_left;
	long async_write_bytes_left;
	long write_bytes_left;
	volatile real read_progress;
	long current_write_offset;
	long current_read_offset;
	struct cache_copy_read_request *current_request;
	struct cache_copy_write_request *current_write_request;
	long write_requests_pending;
	short current_read_sequence_index;
	short current_sequence_index;
	short current_write_buffer_index;
	short current_write_sequence_index;
	short next_write_sequence_index;
	short current_read_sequence_count;
	LARGE_INTEGER overlapped_issue_times[NUMBER_OF_OVERLAPPED_STRUCTURES]; /* fake name */
	long overlapped_elapsed_times[NUMBER_OF_OVERLAPPED_STRUCTURES]; /* fake name */
};

/* ---------- prototypes */

static boolean copy_should_stop(void);
static void cache_copy_initialize_zlib(struct simple_decompressor_definition *self);
static void cache_copy_dispose_zlib(struct simple_decompressor_definition *self);
static void *cache_copy_compressed_alloc(void *opaque, unsigned int items, unsigned int size);
static void cache_copy_compressed_free(void *opaque, void *address);
static unsigned long __stdcall simple_cache_copy_thread(void);
static void cache_copy_initialize_and_fill_with_garbage(struct simple_decompressor_definition *self);
static void cache_copy_initialize_read_data(struct simple_decompressor_definition *self);
static void cache_copy_initialize_file_data(struct simple_decompressor_definition *self);
static void cache_copy_initialize_read_buffers(struct simple_decompressor_definition *self);
static void cache_copy_update_write_buffers(struct simple_decompressor_definition *self);
static void cache_copy_run_decompression(struct simple_decompressor_definition *self);
static void cache_copy_issue_read_internal(struct simple_decompressor_definition *self, void *buffer, long size, long offset, short read_buffer_index);
static void cache_copy_issue_write_internal(struct simple_decompressor_definition *self, void *buffer, long size, long offset, short write_buffer_index);
static void cache_copy_issue_read_raw(struct simple_decompressor_definition *self, void *buffer, long size, long offset);
static void cache_copy_issue_write_raw(struct simple_decompressor_definition *self, void *buffer, long size, long offset);
static void wait_for_raw_read(struct simple_decompressor_definition *self);
static void wait_for_raw_write(struct simple_decompressor_definition *self);
static void cache_copy_issue_read_request_internal(struct simple_decompressor_definition *self, struct cache_copy_read_request *request, short read_buffer_index);
static void cache_copy_issue_read_request(struct simple_decompressor_definition *self, struct cache_copy_read_request *request);
static void cache_copy_issue_read(struct simple_decompressor_definition *self, short read_buffer_index);
struct cache_copy_read_request *acquire_read_request(struct simple_decompressor_definition *self, short read_sequence_index);
static long get_read_request_size(struct simple_decompressor_definition *self, struct cache_copy_read_request *request);
static void *get_read_request_buffer(struct simple_decompressor_definition *self, struct cache_copy_read_request *request);
static void release_read_request(struct simple_decompressor_definition *self, struct cache_copy_read_request *request);
static void *get_write_buffer(struct simple_decompressor_definition *self, short write_buffer_index);
static long get_write_buffer_size(struct simple_decompressor_definition *self, short write_buffer_index);
static void cache_copy_issue_write(struct simple_decompressor_definition *self, short write_buffer_index);
static boolean any_bit_vector_flag_set(unsigned long *bit_vector, int size);
static void wait_for_io_to_complete(struct simple_decompressor_definition *self);
static void set_copy_error(short flag);
static unsigned long get_copy_error_flags(void);
static void initialize_timing(void);
static void begin_timing(long timing_index);
static void end_timing(long timing_index);
static void print_timing(void);
static void give_up_time_if_necessary(void);

/* ---------- globals */

boolean decompressor_print_timing = FALSE;

static struct simple_decompressor_definition decompress_globals;
static struct timing_globals timing_globals; /* fake name */
static char decompressor_error_string[256]; /* fake name */

static struct simple_decompressor_definition *global_self = &decompress_globals;
static long timing_frequency = 1; /* fake name */

/* ---------- public code */

static boolean copy_should_stop(
	void)
{
	return WaitForSingleObject(global_self->copy_stop_event, 0) == WAIT_OBJECT_0;
}

long cache_copy_buffer_size(
	boolean should_block)
{
	global_self->blocking = should_block;

	if (should_block)
	{
		SetThreadPriority(global_self->copy_thread, THREAD_PRIORITY_ABOVE_NORMAL);
	}
	else
	{
		SetThreadPriority(global_self->copy_thread, THREAD_PRIORITY_NORMAL);
	}

	return TOTAL_BUFFER_SIZE;
}

void cache_copy_set_priority(
	boolean blocking)
{
	global_self->blocking = blocking;

	if (blocking)
	{
		SetThreadPriority(global_self->copy_thread, THREAD_PRIORITY_ABOVE_NORMAL);
	}
	else
	{
		SetThreadPriority(global_self->copy_thread, THREAD_PRIORITY_NORMAL);
	}
}

void cache_copy_initialize(
	void)
{
	LARGE_INTEGER freq;

	QueryPerformanceFrequency(&freq);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 487, freq.u.HighPart==0);
	timing_frequency = freq.u.LowPart;

	global_self->copy_complete_event = CreateEvent(NULL, TRUE, TRUE, NULL);
	global_self->copy_start_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	global_self->copy_stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	global_self->progress_update_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	global_self->zlib_stream.zalloc = cache_copy_compressed_alloc;
	global_self->zlib_stream.zfree = cache_copy_compressed_free;
	global_self->copy_thread = CreateThread(
		NULL,
		COPY_THREAD_STACK_SIZE,
		(LPTHREAD_START_ROUTINE)simple_cache_copy_thread,
		NULL,
		0,
		NULL);
}

boolean cache_copy_compressed_file_complete(
	void)
{
	return WaitForSingleObject(global_self->copy_complete_event, 0) == WAIT_OBJECT_0;
}

void cache_copy_begin(
	void *buffer,
	long size,
	HANDLE destination_file,
	long destination_file_size,
	char const *source_file_name)
{
	if (cache_copy_compressed_file_complete())
	{
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 524, source_file_name);
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 525, destination_file!=INVALID_HANDLE_VALUE);
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 526, buffer);
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 527, size>= TOTAL_BUFFER_SIZE);
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 528, destination_file_size==GetFileSize(destination_file, NULL));

		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 530, global_self->copy_complete_event);
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 531, global_self->copy_stop_event);
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 532, global_self->copy_start_event);
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 533, global_self->progress_update_event);

		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 535, global_self->copy_thread);

		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 537, global_self->zlib_stream.zalloc);
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 538, global_self->zlib_stream.zfree);

		global_self->flags = 0;
		global_self->allocated_buffer = buffer;
		strcpy(global_self->src_name, source_file_name);
		global_self->write_file_handle = destination_file;
		global_self->read_progress = 0.f;

		ResetEvent(global_self->copy_complete_event);
		ResetEvent(global_self->copy_stop_event);
		memset(&global_self->header, 0, sizeof(global_self->header));
		SetEvent(global_self->copy_start_event);
	}
	else
	{
		match_vhalt("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 564, "previous copy session did not complete");
	}
}

short cache_copy_get_status(
	real *progress)
{
	unsigned long error_flags = get_copy_error_flags();
	short status = _cache_copy_bad_file_failure;

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 574, progress);

	if (global_self->blocking)
	{
		Sleep(BLOCKING_PROGRESS_CHECK_SLEEP_INTERVAL);
	}

	if (!error_flags && global_self->copy_thread)
	{
		if (global_self->header.size > 0)
		{
			status = cache_copy_compressed_file_complete() ? _cache_copy_finised : _cache_copy_in_progress;

			if (WaitForSingleObject(global_self->progress_update_event, 0) == WAIT_OBJECT_0)
			{
				*progress = PIN(global_self->read_progress, 0.f, 1.f);
			}
		}
		else
		{
			*progress = 0.f;
			status = _cache_copy_in_progress;
		}
	}
	else
	{
		if (TEST_FLAG(error_flags, _copy_read_failed_bit))
		{
			status = _cache_copy_read_failure;
		}
		else if (TEST_FLAG(error_flags, _copy_decompression_failed_bit))
		{
			status = _cache_copy_bad_file_failure;
		}
		else if (TEST_FLAG(error_flags, _copy_write_failed_bit))
		{
			status = _cache_copy_write_failure;
		}
		else
		{
			match_unreachable("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 626);
		}

		*progress = 0.f;
	}

	return status;
}

void cache_copy_queue_end(
	void)
{
	if (!cache_copy_compressed_file_complete())
	{
		SetEvent(global_self->copy_stop_event);
	}
}

void cache_copy_end(
	void)
{
	if (!cache_copy_compressed_file_complete())
	{
		SetEvent(global_self->copy_stop_event);
		WaitForSingleObject(global_self->copy_complete_event, INFINITE);
	}

	if (decompressor_print_timing)
	{
		print_timing();
	}
}

static void cache_copy_initialize_zlib(
	struct simple_decompressor_definition *self)
{
	self->zlib_stream.next_in = NULL;
	self->zlib_stream.avail_in = 0;
	self->zlib_stream.next_out = NULL;
	self->zlib_stream.avail_out = 0;
	inflateInit(&self->zlib_stream);
}

static void cache_copy_dispose_zlib(
	struct simple_decompressor_definition *self)
{
	inflateEnd(&self->zlib_stream);
	self->zlib_stream.next_in = NULL;
	self->zlib_stream.avail_in = 0;
	self->zlib_stream.next_out = NULL;
	self->zlib_stream.avail_out = 0;
}

static void *cache_copy_compressed_alloc(
	void *opaque,
	unsigned int items,
	unsigned int size)
{
	void *result = global_self->next_allocation;

	global_self->next_allocation += items*size;
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 701, global_self->next_allocation-global_self->zlib_buffer<global_self->zlib_buffer_size);

	return result;
}

static void cache_copy_compressed_free(
	void *opaque,
	void *address)
{
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 710, (byte*)address<=global_self->next_allocation);
	global_self->next_allocation = address;
}

static unsigned long __stdcall simple_cache_copy_thread(
	void)
{
	struct simple_decompressor_definition *self = global_self;

	while (TRUE)
	{
		WaitForSingleObject(self->copy_start_event, INFINITE);

		initialize_timing();
		begin_timing(_timing_copying);

		cache_copy_initialize_and_fill_with_garbage(self);
		cache_copy_initialize_read_data(self);

		if (!copy_should_stop())
		{
			begin_timing(_timing_file_data);
			cache_copy_initialize_file_data(self);
			cache_copy_initialize_zlib(self);
			end_timing(_timing_file_data);

			if (cache_file_header_verify(&self->header, "cache decompressed", TRUE))
			{
				boolean success = TRUE;

				self->write_bytes_left = self->header.size - sizeof(self->header);
				self->async_write_bytes_left = self->write_bytes_left;
				cache_copy_initialize_read_buffers(self);

				while (!copy_should_stop() && self->write_bytes_left > 0 && success)
				{
					unsigned long wait_result;

					if (any_bit_vector_flag_set(self->overlapped_in_use_flags, BIT_VECTOR_SIZE_IN_LONGS(NUMBER_OF_OVERLAPPED_STRUCTURES)))
					{
						boolean blocked_on_read = !acquire_read_request(self, self->current_sequence_index);
						boolean blocked_on_write = self->write_requests_pending == NUMBER_OF_WRITE_BUFFERS && self->current_write_buffer_index == NONE;

						if (blocked_on_read || blocked_on_write)
						{
							if (blocked_on_read)
							{
								begin_timing(_timing_thread_blocked_on_read);
							}

							if (blocked_on_write)
							{
								begin_timing(_timing_thread_blocked_on_write);
							}

							begin_timing(_timing_thread_blocked);
							SetEvent(self->progress_update_event);
							wait_result = WaitForSingleObjectEx(self->copy_stop_event, EVENT_TIMEOUT, TRUE);
							end_timing(_timing_thread_blocked);

							if (blocked_on_write)
							{
								end_timing(_timing_thread_blocked_on_write);
							}

							if (blocked_on_read)
							{
								end_timing(_timing_thread_blocked_on_read);
							}
						}
						else
						{
							wait_result = WAIT_IO_COMPLETION;
						}
					}
					else
					{
						wait_result = WAIT_IO_COMPLETION;
					}

					success = FALSE;

					switch (wait_result)
					{
					case WAIT_OBJECT_0:
						break;
					case WAIT_IO_COMPLETION:
						match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 849, any_bit_vector_flag_set(self->overlapped_completed_flags, BIT_VECTOR_SIZE_IN_LONGS(NUMBER_OF_OVERLAPPED_STRUCTURES)));
						cache_copy_update_write_buffers(self);
						cache_copy_run_decompression(self);
						success = !(get_copy_error_flags() & MASK(NUMBER_OF_COPY_FLAGS));
						break;
					case WAIT_TIMEOUT:
						match_vhalt("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 866, "timeout for asynchronous i/o");
						set_copy_error(_copy_read_failed_bit);
						break;
					default:
						match_unreachable("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 875);
					}
				}

				if (!self->write_bytes_left)
				{
					wait_for_io_to_complete(self);
					cache_copy_issue_write_raw(self, &self->header, sizeof(self->header), 0);
				}
			}

			cache_copy_dispose_zlib(self);
		}

		wait_for_io_to_complete(self);
		CloseHandle(self->read_file_handle);
		self->read_file_handle = NULL;
		end_timing(_timing_copying);
		self->write_file_handle = NULL;
		SetEvent(self->copy_complete_event);
	}
}

static void cache_copy_initialize_and_fill_with_garbage(
	struct simple_decompressor_definition *self)
{
	short read_buffer_index;
	short write_buffer_index;
	byte *buffer = self->allocated_buffer;

	for (read_buffer_index = 0; read_buffer_index < NUMBER_OF_READ_BUFFERS; read_buffer_index++)
	{
		global_self->read_buffers[read_buffer_index] = buffer;
		buffer += FILE_BLOCK_SIZE;
	}

	for (write_buffer_index = 0; write_buffer_index < NUMBER_OF_WRITE_BUFFERS; write_buffer_index++)
	{
		global_self->write_buffers[write_buffer_index] = buffer;
		buffer += WRITE_FILE_BLOCK_SIZE;
	}

	XPhysicalProtect(self->allocated_buffer, TOTAL_BUFFER_SIZE, PAGE_READWRITE);
	memset(self->allocated_buffer, GARBAGE_BUFFER_FILL, TOTAL_READ_WRITE_BUFFER_SIZE);
	XPhysicalProtect(self->allocated_buffer, TOTAL_READ_WRITE_BUFFER_SIZE, PAGE_READONLY);

	self->zlib_buffer_size = MIN_ZLIB_BUFFER_SIZE;
	self->zlib_buffer = (byte *)self->allocated_buffer + TOTAL_READ_WRITE_BUFFER_SIZE;
	self->next_allocation = self->zlib_buffer;

	memset(
		(byte *)self + _start_dynamic_data_offset,
		GARBAGE_DYNAMIC_DATA_FILL,
		_end_dynamic_data_offset - _start_dynamic_data_offset);
}

static void cache_copy_initialize_read_data(
	struct simple_decompressor_definition *self)
{
	short read_buffer_index;
	short write_buffer_index;

	self->read_file_handle = CreateFile(
		self->src_name,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING,
		NULL);
	self->read_bytes_left = GetFileSize(self->read_file_handle, NULL);
	self->read_file_size = self->read_bytes_left;
	self->async_read_bytes_left = self->read_bytes_left;
	memset(self->overlapped, 0, sizeof(self->overlapped));
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 964, self->read_bytes_left>=sizeof(self->header));

	memset(self->overlapped_in_use_flags, 0, BIT_VECTOR_SIZE_IN_BYTES(NUMBER_OF_OVERLAPPED_STRUCTURES));
	memset(self->overlapped_completed_flags, 0, BIT_VECTOR_SIZE_IN_BYTES(NUMBER_OF_OVERLAPPED_STRUCTURES));

	for (read_buffer_index = 0; read_buffer_index < NUMBER_OF_READ_BUFFERS; read_buffer_index++)
	{
		self->read_requests[read_buffer_index].read_sequence_index = NONE;
	}

	for (write_buffer_index = 0; write_buffer_index < NUMBER_OF_WRITE_BUFFERS; write_buffer_index++)
	{
		self->write_requests[write_buffer_index].write_sequence_index = NONE;
	}

	self->current_read_sequence_index = 0;
	self->current_sequence_index = 0;
	self->current_write_sequence_index = 0;
	self->next_write_sequence_index = 0;
	self->write_requests_pending = 0;
	self->current_write_buffer_index = NONE;
}

static void cache_copy_initialize_file_data(
	struct simple_decompressor_definition *self)
{
	self->async_write_bytes_left = 0;
	memset(&self->header, 0, sizeof(self->header));
	cache_copy_issue_write_raw(self, &self->header, sizeof(self->header), 0);
	wait_for_raw_write(self);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1003, global_self->async_write_bytes_left==0);

	cache_copy_issue_read_raw(self, &self->header, sizeof(self->header), 0);
	wait_for_raw_read(self);
	cache_file_header_verify(&self->header, "blah", TRUE);

	self->read_bytes_left -= sizeof(self->header);
	self->current_read_offset = sizeof(self->header);
	self->current_write_offset = sizeof(self->header);
	self->read_progress = 0.f;
	self->current_request = NULL;
	self->current_read_sequence_count = 0;
	self->current_write_request = NULL;
}

static void cache_copy_initialize_read_buffers(
	struct simple_decompressor_definition *self)
{
	short read_buffer_index;

	for (read_buffer_index = 0; read_buffer_index < NUMBER_OF_READ_BUFFERS; read_buffer_index++)
	{
		long overlapped_index = _read_buffer_base + read_buffer_index;

		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1046, !BIT_VECTOR_TEST_FLAG(self->overlapped_in_use_flags, overlapped_index));
		cache_copy_issue_read(self, read_buffer_index);
		BIT_VECTOR_SET_FLAG(self->overlapped_in_use_flags, overlapped_index, TRUE);
	}
}

static void cache_copy_update_write_buffers(
	struct simple_decompressor_definition *self)
{
	short write_buffer_index;

	for (write_buffer_index = 0; write_buffer_index < NUMBER_OF_WRITE_BUFFERS; write_buffer_index++)
	{
		if (self->write_requests_pending > 0 && self->current_write_request && BIT_VECTOR_TEST_FLAG(self->overlapped_completed_flags, _write_buffer_base + write_buffer_index))
		{
			self->write_requests[write_buffer_index].write_sequence_index = NONE;
			BIT_VECTOR_SET_FLAG(self->overlapped_completed_flags, _write_buffer_base + write_buffer_index, FALSE);
			self->write_requests_pending--;
			self->current_write_request = NULL;
			match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1075, self->write_requests_pending>=0);
		}
	}

	if (self->write_bytes_left && !self->current_write_request && self->write_requests_pending > 0)
	{
		if (self->current_write_buffer_index == NONE || self->write_requests[self->current_write_buffer_index].write_sequence_index > self->current_write_sequence_index)
		{
			for (write_buffer_index = 0; write_buffer_index < NUMBER_OF_WRITE_BUFFERS; write_buffer_index++)
			{
				if (self->current_write_buffer_index != write_buffer_index && self->write_requests[write_buffer_index].write_sequence_index == self->current_write_sequence_index)
				{
					self->current_write_request = &self->write_requests[write_buffer_index];
					cache_copy_issue_write(self, write_buffer_index);
					self->current_write_sequence_index++;
					break;
				}
			}
		}
	}

	if (self->current_write_buffer_index == NONE && self->write_requests_pending < NUMBER_OF_WRITE_BUFFERS)
	{
		for (write_buffer_index = 0; write_buffer_index < NUMBER_OF_WRITE_BUFFERS; write_buffer_index++)
		{
			if (self->write_requests[write_buffer_index].write_sequence_index == NONE)
			{
				self->write_requests[write_buffer_index].write_sequence_index = self->next_write_sequence_index++;
				self->write_requests_pending++;
				self->current_write_buffer_index = write_buffer_index;
				XPhysicalProtect(self->write_buffers[write_buffer_index], WRITE_FILE_BLOCK_SIZE, PAGE_READWRITE);
				break;
			}
		}

		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1124, self->current_write_buffer_index!=NONE);
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1125, self->write_requests_pending<=NUMBER_OF_WRITE_BUFFERS);
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1126, self->write_requests[self->current_write_buffer_index].write_sequence_index+1==self->next_write_sequence_index);
	}
}

static void cache_copy_run_decompression(
	struct simple_decompressor_definition *self)
{
	z_stream *zlib_stream = &self->zlib_stream;

	while (TRUE)
	{
		SleepEx(0, TRUE);
		cache_copy_update_write_buffers(self);

		if (!zlib_stream->avail_in)
		{
			match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1149, !self->current_request);
			match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1150, !self->current_read_sequence_count);
			self->current_request = acquire_read_request(self, self->current_sequence_index);

			if (!self->current_request)
			{
				break;
			}

			zlib_stream->avail_in = get_read_request_size(self, self->current_request);
			zlib_stream->next_in = get_read_request_buffer(self, self->current_request);
			self->current_read_sequence_count = 1;
			match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1163, zlib_stream->avail_in==FILE_BLOCK_SIZE);
		}

		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1172, self->current_request);

		if (self->current_write_buffer_index == NONE)
		{
			break;
		}

		if (!zlib_stream->avail_out)
		{
			zlib_stream->next_out = get_write_buffer(self, self->current_write_buffer_index);
			zlib_stream->avail_out = get_write_buffer_size(self, self->current_write_buffer_index);
		}

		if (zlib_stream->avail_in && zlib_stream->avail_out)
		{
			int result;

			give_up_time_if_necessary();
			begin_timing(_timing_zlib);

			if (self->write_requests_pending > 1)
			{
				begin_timing(_timing_zlib_during_write_file);
			}

			result = inflate(zlib_stream, Z_NO_FLUSH);
			end_timing(_timing_zlib);

			if (self->write_requests_pending > 1)
			{
				end_timing(_timing_zlib_during_write_file);
			}

			if (result == Z_OK || result == Z_STREAM_END)
			{
				if (!zlib_stream->avail_in)
				{
					release_read_request(self, self->current_request);
					self->current_sequence_index++;
					self->current_read_sequence_count--;
					self->current_request = NULL;
				}

				if (!zlib_stream->avail_out || result == Z_STREAM_END)
				{
					self->current_write_buffer_index = NONE;
				}
			}
			else
			{
				if (!copy_should_stop())
				{
					match_vhalt(
						"c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c",
						1248,
						csprintf(
							decompressor_error_string,
							"decompression fucked up with error code (%d), msg '%s'",
							result,
							zlib_stream->msg ? zlib_stream->msg : ""));
					set_copy_error(_copy_decompression_failed_bit);
				}

				break;
			}
		}
	}
}

void __stdcall cache_copy_FileIOCompletionRoutine(
	unsigned long dwErrorCode,
	unsigned long dwNumberOfBytesTransfered,
	OVERLAPPED *lpOverlapped)
{
	unsigned long *overlapped_in_use_flags = global_self->overlapped_in_use_flags;
	unsigned long *overlapped_completed_flags = global_self->overlapped_completed_flags;
	long overlapped_index = lpOverlapped - global_self->overlapped;

	if (dwErrorCode == 0)
	{
		if (overlapped_index >= 0 && overlapped_index < NUMBER_OF_OVERLAPPED_STRUCTURES)
		{
			LARGE_INTEGER completion_time;

			QueryPerformanceCounter(&completion_time);
			BIT_VECTOR_SET_FLAG(overlapped_in_use_flags, overlapped_index, FALSE);
			BIT_VECTOR_SET_FLAG(overlapped_completed_flags, overlapped_index, TRUE);
		}

		if (overlapped_index >= _first_read_index && overlapped_index <= _last_read_index)
		{
			match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1291, global_self->async_read_bytes_left>0);
			global_self->async_read_bytes_left -= dwNumberOfBytesTransfered;
			ResetEvent(global_self->progress_update_event);
			global_self->read_progress = (real)(global_self->header.size - global_self->async_read_bytes_left) / global_self->header.size;
			SetEvent(global_self->progress_update_event);
		}
		else if (overlapped_index >= _first_write_index && overlapped_index <= _last_write_index)
		{
			match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1300, global_self->async_write_bytes_left>0);
			global_self->async_write_bytes_left -= dwNumberOfBytesTransfered;
		}
	}
	else
	{
		match_vhalt(
			"c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c",
			1306,
			csprintf(
				decompressor_error_string,
				"async i/o finished with error code %d",
				dwErrorCode));

		if (overlapped_index >= _first_write_index && overlapped_index <= _last_write_index)
		{
			set_copy_error(_copy_write_failed_bit);
		}
		else
		{
			set_copy_error(_copy_read_failed_bit);
		}
	}
}

static void cache_copy_issue_read_internal(
	struct simple_decompressor_definition *self,
	void *buffer,
	long size,
	long offset,
	short read_buffer_index)
{
	long overlapped_index;
	HANDLE file_handle;
	OVERLAPPED *overlapped;
	BOOL success;
	unsigned long last_error;

	begin_timing(_timing_read_file);

	overlapped_index = _read_buffer_base + read_buffer_index;
	file_handle = self->read_file_handle;
	overlapped = &self->overlapped[overlapped_index];
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1334, !BIT_VECTOR_TEST_FLAG(self->overlapped_in_use_flags, overlapped_index));
	BIT_VECTOR_SET_FLAG(self->overlapped_in_use_flags, overlapped_index, TRUE);

	overlapped->hEvent = (HANDLE)overlapped_index;
	overlapped->Offset = offset;
	overlapped->OffsetHigh = 0;
	QueryPerformanceCounter(&self->overlapped_issue_times[overlapped_index]);

	do
	{
		SleepEx(0, TRUE);
		SetLastError(NO_ERROR);
		success = ReadFileEx(file_handle, buffer, size, overlapped, cache_copy_FileIOCompletionRoutine);
		last_error = GetLastError();
	}
	while (!success && (last_error == ERROR_INVALID_USER_BUFFER || last_error == ERROR_NOT_ENOUGH_MEMORY || last_error == ERROR_NO_SYSTEM_RESOURCES));

	if (!success)
	{
		match_vhalt("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1369, "couldn't issue an asynchronous read");
		set_copy_error(_copy_read_failed_bit);
	}

	end_timing(_timing_read_file);
}

static void cache_copy_issue_write_internal(
	struct simple_decompressor_definition *self,
	void *buffer,
	long size,
	long offset,
	short write_buffer_index)
{
	long overlapped_index;
	HANDLE file_handle;
	OVERLAPPED *overlapped;
	BOOL success;
	unsigned long last_error;

	begin_timing(_timing_write_file);

	overlapped_index = _write_buffer_base + write_buffer_index;
	file_handle = self->write_file_handle;
	overlapped = &self->overlapped[overlapped_index];
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1411, !BIT_VECTOR_TEST_FLAG(self->overlapped_in_use_flags, overlapped_index));
	BIT_VECTOR_SET_FLAG(self->overlapped_in_use_flags, overlapped_index, TRUE);

	overlapped->hEvent = (HANDLE)overlapped_index;
	overlapped->Offset = offset;
	overlapped->OffsetHigh = 0;
	QueryPerformanceCounter(&self->overlapped_issue_times[overlapped_index]);

	do
	{
		SleepEx(0, TRUE);
		SetLastError(NO_ERROR);
		success = WriteFileEx(file_handle, buffer, size, overlapped, cache_copy_FileIOCompletionRoutine);
		last_error = GetLastError();
	}
	while (!success && (last_error == ERROR_INVALID_USER_BUFFER || last_error == ERROR_NOT_ENOUGH_MEMORY || last_error == ERROR_NO_SYSTEM_RESOURCES));

	if (!success)
	{
		match_vhalt("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1441, "couldn't issue an asynchronous write");
		set_copy_error(_copy_write_failed_bit);
	}

	end_timing(_timing_write_file);
}

static void cache_copy_issue_read_raw(
	struct simple_decompressor_definition *self,
	void *buffer,
	long size,
	long offset)
{
	cache_copy_issue_read_internal(self, buffer, size, offset, _raw_read_index);
}

static void cache_copy_issue_write_raw(
	struct simple_decompressor_definition *self,
	void *buffer,
	long size,
	long offset)
{
	cache_copy_issue_write_internal(self, buffer, size, offset, _raw_write_index);
}

static void wait_for_raw_read(
	struct simple_decompressor_definition *self)
{
	unsigned long wait_result = WaitForSingleObjectEx(self->copy_stop_event, EVENT_TIMEOUT, TRUE);

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1482, !BIT_VECTOR_TEST_FLAG(self->overlapped_in_use_flags, _raw_read_offset));
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1483, wait_result==WAIT_IO_COMPLETION);
	BIT_VECTOR_SET_FLAG(self->overlapped_in_use_flags, _raw_read_offset, FALSE);
}

static void wait_for_raw_write(
	struct simple_decompressor_definition *self)
{
	unsigned long wait_result;

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1494, BIT_VECTOR_TEST_FLAG(self->overlapped_in_use_flags, _raw_write_offset));
	wait_result = WaitForSingleObjectEx(self->copy_stop_event, EVENT_TIMEOUT, TRUE);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1498, !BIT_VECTOR_TEST_FLAG(self->overlapped_in_use_flags, _raw_write_offset));
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1499, wait_result==WAIT_IO_COMPLETION);
	BIT_VECTOR_SET_FLAG(self->overlapped_in_use_flags, _raw_write_offset, FALSE);
}

static void cache_copy_issue_read_request_internal(
	struct simple_decompressor_definition *self,
	struct cache_copy_read_request *request,
	short read_buffer_index)
{
	void *buffer = get_read_request_buffer(self, request);
	long size = MIN(FILE_BLOCK_SIZE, self->read_bytes_left);

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1516, read_buffer_index>=0 && read_buffer_index<NUMBER_OF_READ_BUFFERS);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1517, request->read_sequence_index==NONE);

	request->read_sequence_index = self->current_read_sequence_index;
	XPhysicalProtect(buffer, size, PAGE_READWRITE);
	cache_copy_issue_read_internal(self, buffer, size, self->current_read_offset, read_buffer_index);
	self->current_read_sequence_index++;
	self->read_bytes_left -= size;
	self->current_read_offset += size;
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1537, self->current_read_offset<=self->read_file_size);
}

static void cache_copy_issue_read_request(
	struct simple_decompressor_definition *self,
	struct cache_copy_read_request *request)
{
	short read_buffer_index = request - self->read_requests;

	cache_copy_issue_read_request_internal(self, request, read_buffer_index);
}

static void cache_copy_issue_read(
	struct simple_decompressor_definition *self,
	short read_buffer_index)
{
	struct cache_copy_read_request *request = &self->read_requests[read_buffer_index];

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1561, read_buffer_index>=0 && read_buffer_index<NUMBER_OF_READ_BUFFERS);
	cache_copy_issue_read_request_internal(self, request, read_buffer_index);
}

struct cache_copy_read_request *acquire_read_request(
	struct simple_decompressor_definition *self,
	short read_sequence_index)
{
	short read_buffer_index;
	struct cache_copy_read_request *result = NULL;

	for (read_buffer_index = 0; read_buffer_index < NUMBER_OF_READ_BUFFERS; read_buffer_index++)
	{
		if (self->read_requests[read_buffer_index].read_sequence_index == read_sequence_index &&
			BIT_VECTOR_TEST_FLAG(self->overlapped_completed_flags, _read_buffer_base + read_buffer_index))
		{
			result = &self->read_requests[read_buffer_index];
			XPhysicalProtect(self->read_buffers[read_buffer_index], FILE_BLOCK_SIZE, PAGE_READONLY);
			break;
		}
	}

	return result;
}

static long get_read_request_size(
	struct simple_decompressor_definition *self,
	struct cache_copy_read_request *request)
{
	return FILE_BLOCK_SIZE;
}

static void *get_read_request_buffer(
	struct simple_decompressor_definition *self,
	struct cache_copy_read_request *request)
{
	short read_buffer_index = request - self->read_requests;

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1606, read_buffer_index>=0 && read_buffer_index<NUMBER_OF_READ_BUFFERS);

	return self->read_buffers[read_buffer_index];
}

static void release_read_request(
	struct simple_decompressor_definition *self,
	struct cache_copy_read_request *request)
{
	short read_buffer_index = request - self->read_requests;

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1618, read_buffer_index>=0 && read_buffer_index<NUMBER_OF_READ_BUFFERS);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1619, BIT_VECTOR_TEST_FLAG(self->overlapped_completed_flags, _read_buffer_base+read_buffer_index));
	BIT_VECTOR_SET_FLAG(self->overlapped_completed_flags, _read_buffer_base + read_buffer_index, FALSE);
	request->read_sequence_index = NONE;
	cache_copy_issue_read_request(self, request);
}

static void *get_write_buffer(
	struct simple_decompressor_definition *self,
	short write_buffer_index)
{
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1633, write_buffer_index>=0 && write_buffer_index<NUMBER_OF_WRITE_BUFFERS);

	return self->write_buffers[write_buffer_index];
}

static long get_write_buffer_size(
	struct simple_decompressor_definition *self,
	short write_buffer_index)
{
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1642, write_buffer_index>=0 && write_buffer_index<NUMBER_OF_WRITE_BUFFERS);

	return WRITE_FILE_BLOCK_SIZE;
}

static void cache_copy_issue_write(
	struct simple_decompressor_definition *self,
	short write_buffer_index)
{
	void *buffer = get_write_buffer(self, write_buffer_index);
	long size = MIN(get_write_buffer_size(self, write_buffer_index), self->write_bytes_left);

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1654, write_buffer_index>=0 && write_buffer_index<NUMBER_OF_WRITE_BUFFERS);

	XPhysicalProtect(buffer, WRITE_FILE_BLOCK_SIZE, PAGE_READONLY);
	cache_copy_issue_write_internal(
		self,
		get_write_buffer(self, write_buffer_index),
		size,
		self->current_write_offset,
		write_buffer_index);
	self->current_write_offset += size;
	self->write_bytes_left -= size;
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1666, self->current_write_offset<=self->header.size);
}

static boolean any_bit_vector_flag_set(
	unsigned long *bit_vector,
	int size)
{
	short index;
	boolean result = FALSE;

	for (index = 0; index < size; index++)
	{
		result = result || bit_vector[index];
	}

	return result;
}

static void wait_for_io_to_complete(
	struct simple_decompressor_definition *self)
{
	short timeout_count = NUMBER_OF_OVERLAPPED_STRUCTURES;

	while (any_bit_vector_flag_set(self->overlapped_in_use_flags, BIT_VECTOR_SIZE_IN_LONGS(NUMBER_OF_OVERLAPPED_STRUCTURES)) && timeout_count--)
	{
		unsigned long wait_result = SleepEx(EVENT_TIMEOUT, TRUE);

		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1695, wait_result==WAIT_IO_COMPLETION);
	}

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_decompress_windows.c", 1699, !any_bit_vector_flag_set(self->overlapped_in_use_flags, BIT_VECTOR_SIZE_IN_LONGS(NUMBER_OF_OVERLAPPED_STRUCTURES)));
	memset(self->overlapped_completed_flags, 0, BIT_VECTOR_SIZE_IN_BYTES(NUMBER_OF_OVERLAPPED_STRUCTURES));
}

static void set_copy_error(
	short flag)
{
	SET_FLAG(global_self->flags, flag, TRUE);
}

static unsigned long get_copy_error_flags(
	void)
{
	return global_self->flags;
}

static void initialize_timing(
	void)
{
	memset(timing_globals.totals, 0, sizeof(timing_globals.totals));
}

static void begin_timing(
	long timing_index)
{
	QueryPerformanceCounter(&timing_globals.start_times[timing_index]);
}

static void end_timing(
	long timing_index)
{
	LARGE_INTEGER end_time;

	QueryPerformanceCounter(&end_time);
	timing_globals.totals[timing_index] += end_time.u.LowPart - timing_globals.start_times[timing_index].u.LowPart;
}

static void print_timing(
	void)
{
	error(_error_silent, "Timing for copying last cache file:");
	error(
		_error_silent,
		"    Total read file time: %.3f",
		(real)timing_globals.totals[_timing_read_file] / timing_frequency);
	error(
		_error_silent,
		"    Total write file time: %.3f",
		(real)timing_globals.totals[_timing_write_file] / timing_frequency);
	error(
		_error_silent,
		"    Total zlib time: %.3f",
		(real)timing_globals.totals[_timing_zlib] / timing_frequency);
	error(
		_error_silent,
		"    Total zlib during write file time: %.3f",
		(real)timing_globals.totals[_timing_zlib_during_write_file] / timing_frequency);
	error(
		_error_silent,
		"    Total thread blocked time: %.3f",
		(real)timing_globals.totals[_timing_thread_blocked] / timing_frequency);
	error(
		_error_silent,
		"    Total thread blocked on read time: %.3f",
		(real)timing_globals.totals[_timing_thread_blocked_on_read] / timing_frequency);
	error(
		_error_silent,
		"    Total thread blocked on write time: %.3f",
		(real)timing_globals.totals[_timing_thread_blocked_on_write] / timing_frequency);
	error(
		_error_silent,
		"    Total copying time: %.3f",
		(real)timing_globals.totals[_timing_copying] / timing_frequency);
}

static void give_up_time_if_necessary(
	void)
{
	if (!global_self->blocking)
	{
		SwitchToThread();
	}
}

/* ---------- private code */
