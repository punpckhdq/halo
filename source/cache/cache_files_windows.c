/*
CACHE_FILES_WINDOWS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "texture_cache.h"
#include "physical_memory_map.h"
#include "cache_files_decompress_windows.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "unicode.h"
#include "rasterizer.h"
#include "input.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "ui_widget.h"
#include "strings/resource.h"
#include "shell.h"
#include "build_number.h"
#include "shell_windows.h"
#include "rasterizer_hardware_format_utilities.h"
#include <io.h>

/* ---------- constants */

enum
{
	MAXIMUM_SIMULTANEOUS_CACHE_REQUESTS = 512,
};

enum
{
	_first_large_cached_map_file = 0,
	_last_large_cached_map_file = 1,
	_first_medium_cached_map_file = 2,
	_last_medium_cached_map_file = 2,
	_first_small_cached_map_file = 3,
	_last_small_cached_map_file = 5,
	NUMBER_OF_CACHED_MAP_FILES = 6,
	MAXIMUM_NUMBER_OF_CACHED_MAP_FILES = 20,
};

enum
{
	LARGE_CACHED_MAP_FILE_SIZE = 0x11600000, /* fake name */
	MEDIUM_CACHED_MAP_FILE_SIZE = 0x2300000, /* fake name */
	SMALL_CACHED_MAP_FILE_SIZE = 0x2F00000, /* fake name */
};

enum
{
	CACHE_FILE_READ_ALIGNMENT = 512, /* fake name */
	CACHE_FILE_WINDOWS_THREAD_STACK_SIZE = 16*1024, /* fake name */
	CACHED_MAP_FILE_BLOCK_TIMEOUT = 5000, /* fake name */
};

/* ---------- macros */

/* ---------- structures */

struct cached_map_file
{
	HANDLE handle;
	FILETIME last_modification_date;
	struct cache_file_header header;
};

struct cache_request
{
	OVERLAPPED overlapped;
	long size;
	void *buffer;
	boolean blocking;
	volatile boolean pending;
	boolean running;
	byte pad[1];
};

/* ---------- prototypes */

static void cached_map_issue_async_read(HANDLE file, OVERLAPPED *overlapped, void *buffer, long size, long offset, LPOVERLAPPED_COMPLETION_ROUTINE completion_routine, volatile boolean *completion_flag);
static void cached_map_issue_async_write(HANDLE file, OVERLAPPED *overlapped, void *buffer, long size, long offset, LPOVERLAPPED_COMPLETION_ROUTINE completion_routine, volatile boolean *completion_flag);
static void cached_map_issue_async_request(BOOL (__stdcall *async_request_function)(HANDLE, void *, unsigned long, OVERLAPPED *, LPOVERLAPPED_COMPLETION_ROUTINE), HANDLE file, OVERLAPPED *overlapped, void *buffer, long size, long offset, LPOVERLAPPED_COMPLETION_ROUTINE completion_routine, volatile boolean *completion_flag);
static boolean cached_map_block_on_async_request(volatile boolean *completion_flag);
static boolean cache_file_read_header_from_dvd(char const *name, struct cache_file_header *header);
static struct cache_request *cache_request_get(short request_index);
static short cache_request_next_free_index(void);
static void cache_requests_flush(void);
static void cached_map_files_delete(short map_file_index);
static void cache_files_zap_if_language_has_changed(void);
static void cached_map_files_open_all(void);
static void cached_map_file_set_modification_date(short map_file_index);
static void cached_map_file_read_header(short map_file_index);
static short cached_map_files_find_map(char const *name);
static short cached_map_files_find_free_map(long size, short scenario_type);
static struct cached_map_file *cached_map_file_get(short map_file_index);
static void cached_map_file_invalidate(short map_file_index);
static HANDLE cached_map_file_get_handle(short map_file_index);
static long cached_map_file_get_size(short map_file_index);
static void cached_map_file_get_path(short map_file_index, char *path);
static void cache_file_windows_thread_create(void);
static void cache_file_windows_thread_wake(void);
static unsigned long __stdcall cache_file_windows_thread_proc(void);
static void __stdcall cache_file_read_io_completion_routine(unsigned long error_code, unsigned long bytes_transferred, OVERLAPPED *overlapped);
static void __stdcall cache_file_blocking_io_completion_routine(unsigned long error_code, unsigned long bytes_transferred, OVERLAPPED *overlapped);
static void scenario_name_to_cache_file_path(char const *scenario_name, char *path);

/* ---------- globals */

static struct
{
	struct cached_map_file cached_map_files[NUMBER_OF_CACHED_MAP_FILES];
	boolean copy_in_progress;
	short copying_to_map_file_index;
	char copying_to_map_file_name[32];
	short open_map_file_index;
	short blocking_request_index;
	HANDLE sleep_event;
	HANDLE thread;
	struct cache_request *requests;
} cache_file_globals;

/* ---------- public code */

void cache_files_initialize(
	void)
{
	cache_file_globals.open_map_file_index = NONE;
	cache_file_globals.requests = match_malloc("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 187, MAXIMUM_SIMULTANEOUS_CACHE_REQUESTS * sizeof(struct cache_request));
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 188, cache_file_globals.requests);
	cache_file_windows_thread_create();
	cache_files_zap_if_language_has_changed();
	cached_map_files_open_all();
	cache_copy_initialize();
}

void cache_files_dispose(
	void)
{
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 207, cache_file_globals.open_map_file_index==NONE);
	match_free("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 209, cache_file_globals.requests);
}

boolean cache_file_open(
	char const *scenario_name,
	struct cache_file_header *header)
{
	short map_file_index = cached_map_files_find_map(scenario_name);

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 220, scenario_name);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 221, header);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 223, cache_file_globals.open_map_file_index==NONE);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 224, map_file_index!=NONE);
	memset(cache_file_globals.requests, 0, MAXIMUM_SIMULTANEOUS_CACHE_REQUESTS * sizeof(struct cache_request));
	cache_file_globals.open_map_file_index = map_file_index;
	memcpy(header, &cached_map_file_get(map_file_index)->header, sizeof(*header));

	return TRUE;
}

void cache_file_close(
	void)
{
	if (cache_file_globals.open_map_file_index != NONE)
	{
		cache_requests_flush();
		cache_file_globals.open_map_file_index = NONE;
	}
}

short cache_file_read(
	long tag_index,
	long offset,
	long size,
	void *buffer,
	boolean *completion_flag_reference,
	boolean blocking)
{
	short request_index = cache_request_next_free_index();
	struct cache_request *request = cache_request_get(request_index);

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 269, cache_file_globals.open_map_file_index!=NONE);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 272, buffer);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 273, completion_flag_reference);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 276, offset>=0);

	if (size & (CACHE_FILE_READ_ALIGNMENT - 1))
	{
		size = (size | (CACHE_FILE_READ_ALIGNMENT - 1)) + 1;
	}

	*completion_flag_reference = FALSE;
	memset(&request->overlapped, 0, sizeof(request->overlapped));
	request->overlapped.hEvent = (HANDLE)completion_flag_reference;
	request->size = size;
	request->overlapped.OffsetHigh = 0;
	request->overlapped.Offset = offset;
	request->buffer = buffer;
	request->pending = TRUE;
	request->blocking = blocking;
	request->running = FALSE;
	cache_file_windows_thread_wake();

	return request_index;
}

void cache_file_promote_read(
	short request_index)
{
	cache_request_get(request_index)->blocking = TRUE;
}

static boolean cache_file_read_header_from_dvd(
	char const *name,
	struct cache_file_header *header)
{
	char path[256];
	unsigned long bytes_read;
	HANDLE file;
	boolean success = FALSE;

	scenario_name_to_cache_file_path(name, path);
	file = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		if (ReadFile(file, header, sizeof(*header), &bytes_read, NULL) &&
			bytes_read == sizeof(*header) &&
			cache_file_header_verify(header, path, TRUE))
		{
			success = TRUE;
		}

		CloseHandle(file);
	}

	return success;
}

static void cached_map_issue_async_read(
	HANDLE file,
	OVERLAPPED *overlapped,
	void *buffer,
	long size,
	long offset,
	LPOVERLAPPED_COMPLETION_ROUTINE completion_routine,
	volatile boolean *completion_flag)
{
	cached_map_issue_async_request(
		ReadFileEx,
		file,
		overlapped,
		buffer,
		size,
		offset,
		completion_routine,
		completion_flag);
}

static void cached_map_issue_async_write(
	HANDLE file,
	OVERLAPPED *overlapped,
	void *buffer,
	long size,
	long offset,
	LPOVERLAPPED_COMPLETION_ROUTINE completion_routine,
	volatile boolean *completion_flag)
{
	cached_map_issue_async_request(
		WriteFileEx,
		file,
		overlapped,
		buffer,
		size,
		offset,
		completion_routine,
		completion_flag);
}

static void cached_map_issue_async_request(
	BOOL (__stdcall *async_request_function)(HANDLE, void *, unsigned long, OVERLAPPED *, LPOVERLAPPED_COMPLETION_ROUTINE),
	HANDLE file,
	OVERLAPPED *overlapped,
	void *buffer,
	long size,
	long offset,
	LPOVERLAPPED_COMPLETION_ROUTINE completion_routine,
	volatile boolean *completion_flag)
{
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 390, async_request_function);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 391, file!=INVALID_HANDLE_VALUE);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 392, overlapped);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 393, buffer);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 394, completion_flag);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 395, completion_routine);

	memset(overlapped, 0, sizeof(*overlapped));
	overlapped->Offset = offset;
	overlapped->OffsetHigh = 0;
	overlapped->hEvent = (HANDLE)completion_flag;
	SleepEx(0, TRUE);
	SetLastError(ERROR_SUCCESS);

	while (!async_request_function(file, buffer, size, overlapped, completion_routine))
	{
		unsigned long error_code = GetLastError();

		match_vassert(
			"c:\\halo\\SOURCE\\cache\\cache_files_windows.c",
			422,
			error_code == ERROR_NOT_ENOUGH_MEMORY || error_code == ERROR_NO_SYSTEM_RESOURCES || error_code == ERROR_INVALID_USER_BUFFER,
			csprintf(temporary, "Read/WriteFileEx() returned #%d", GetLastError()));
		SleepEx(0, TRUE);
		SetLastError(ERROR_SUCCESS);
	}
}

static boolean cached_map_block_on_async_request(
	volatile boolean *completion_flag)
{
	boolean result;

	if (!*completion_flag)
	{
		while (SleepEx(CACHED_MAP_FILE_BLOCK_TIMEOUT, TRUE) == WAIT_IO_COMPLETION && !*completion_flag);
	}

	result = *completion_flag;

	return result;
}

void cache_file_block_until_not_busy(
	void)
{
	boolean busy;

	do
	{
		short request_index;

		SleepEx(0, TRUE);
		busy = FALSE;

		for (request_index = 0; request_index < MAXIMUM_SIMULTANEOUS_CACHE_REQUESTS; request_index++)
		{
			if (cache_request_get(request_index)->pending)
			{
				busy = TRUE;
			}
		}
	}
	while (busy);
}

void tags_header_register_vertex_and_index_buffers(
	struct cache_file_tags_header *tags_header)
{
	short vertex_buffer_index;
	short index_buffer_index;

	for (vertex_buffer_index = 0; vertex_buffer_index < tags_header->vertex_buffer_count; vertex_buffer_index++)
	{
		IDirect3DVertexBuffer8 *vertex_buffer = &tags_header->vertex_buffers[vertex_buffer_index];

		vertex_buffer->Common = 1 | D3DCOMMON_TYPE_VERTEXBUFFER;
		IDirect3DVertexBuffer8_Register(vertex_buffer, NULL);
	}

	for (index_buffer_index = 0; index_buffer_index < tags_header->index_buffer_count; index_buffer_index++)
	{
		IDirect3DIndexBuffer8 *index_buffer = &tags_header->index_buffers[index_buffer_index];

		index_buffer->Common = 1 | D3DCOMMON_TYPE_INDEXBUFFER;
	}
}

void tags_header_deregister_vertex_and_index_buffers(
	struct cache_file_tags_header *tags_header)
{
	short vertex_buffer_index;
	short index_buffer_index;

	for (vertex_buffer_index = 0; vertex_buffer_index < tags_header->vertex_buffer_count; vertex_buffer_index++)
	{
		IDirect3DVertexBuffer8 *vertex_buffer = &tags_header->vertex_buffers[vertex_buffer_index];

		IDirect3DVertexBuffer8_BlockUntilNotBusy(vertex_buffer);
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 523, !IDirect3DVertexBuffer8_IsBusy(vertex_buffer));
	}

	for (index_buffer_index = 0; index_buffer_index < tags_header->index_buffer_count; index_buffer_index++)
	{
		IDirect3DIndexBuffer8 *index_buffer = &tags_header->index_buffers[index_buffer_index];

		IDirect3DIndexBuffer8_BlockUntilNotBusy(index_buffer);
		match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 536, !IDirect3DIndexBuffer8_IsBusy(index_buffer));
	}
}

void structure_bsp_header_register_vertex_buffers(
	struct cache_file_structure_bsp_header *structure_bsp_header)
{
	short vertex_buffer_index;

	for (vertex_buffer_index = 0; vertex_buffer_index < structure_bsp_header->vertex_buffer_count; vertex_buffer_index++)
	{
		IDirect3DVertexBuffer8 *vertex_buffer = &structure_bsp_header->vertex_buffers[vertex_buffer_index];

		vertex_buffer->Common = 1 | D3DCOMMON_TYPE_VERTEXBUFFER;
		IDirect3DVertexBuffer8_Register(vertex_buffer, NULL);
	}

	for (vertex_buffer_index = 0; vertex_buffer_index < structure_bsp_header->lightmap_vertex_buffer_count; vertex_buffer_index++)
	{
		IDirect3DVertexBuffer8 *vertex_buffer = &structure_bsp_header->lightmap_vertex_buffers[vertex_buffer_index];

		vertex_buffer->Common = 1 | D3DCOMMON_TYPE_VERTEXBUFFER;
		IDirect3DVertexBuffer8_Register(vertex_buffer, NULL);
	}
}

void structure_bsp_header_deregister_vertex_buffers(
	struct cache_file_structure_bsp_header *structure_bsp_header)
{
	short vertex_buffer_index;

	rasterizer_globals.current_lock_operation = _rasterizer_lock_bsp_switch;

	for (vertex_buffer_index = 0; vertex_buffer_index < structure_bsp_header->vertex_buffer_count; vertex_buffer_index++)
	{
		IDirect3DVertexBuffer8_BlockUntilNotBusy(&structure_bsp_header->vertex_buffers[vertex_buffer_index]);
	}

	for (vertex_buffer_index = 0; vertex_buffer_index < structure_bsp_header->lightmap_vertex_buffer_count; vertex_buffer_index++)
	{
		IDirect3DVertexBuffer8_BlockUntilNotBusy(&structure_bsp_header->lightmap_vertex_buffers[vertex_buffer_index]);
	}

	rasterizer_globals.current_lock_operation = _rasterizer_lock_none;
}

static struct cache_request *cache_request_get(
	short request_index)
{
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 614, request_index>=0 && request_index<MAXIMUM_SIMULTANEOUS_CACHE_REQUESTS);

	return &cache_file_globals.requests[request_index];
}

static short cache_request_next_free_index(
	void)
{
	short request_index;
	boolean warned = FALSE;

	for (;;)
	{
		for (request_index = 0; request_index < MAXIMUM_SIMULTANEOUS_CACHE_REQUESTS; request_index++)
		{
			if (!cache_request_get(request_index)->pending)
			{
				return request_index;
			}
		}

		if (!warned)
		{
			warned = TRUE;
		}
	}
}

static void cache_requests_flush(
	void)
{
	short request_index;

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 651, cache_file_globals.open_map_file_index!=NONE);

	for (request_index = 0; request_index < MAXIMUM_SIMULTANEOUS_CACHE_REQUESTS; request_index++)
	{
		struct cache_request *request = cache_request_get(request_index);

		while (request->pending);
	}
}

static void cached_map_files_delete(
	short map_file_index)
{
	char path[256];

	for (map_file_index++; map_file_index < MAXIMUM_NUMBER_OF_CACHED_MAP_FILES; map_file_index++)
	{
		cached_map_file_get_path(map_file_index, path);
		DeleteFile(path);
	}

	SetLastError(ERROR_SUCCESS);
}

static void cache_files_zap_if_language_has_changed(
	void)
{
	struct file_reference file;
	unsigned long last_language;
	unsigned long language = XGetLanguage();
	unsigned long previous_language = NONE;

	if (file_reference_create_from_path(&file, "z:\\last_language.dat", FALSE) && file_open(&file, FLAG(_permission_read_bit)))
	{
		if (file_read(&file, sizeof(last_language), &last_language))
		{
			previous_language = last_language;
		}

		file_close(&file);
	}

	if (previous_language != language)
	{
		short map_file_index;

		for (map_file_index = NONE; map_file_index < NUMBER_OF_CACHED_MAP_FILES; map_file_index++)
		{
			cached_map_files_delete(map_file_index);
		}
	}

	if (file_reference_create_from_path(&file, "z:\\last_language.dat", FALSE) &&
		file_create(&file) &&
		file_open(&file, FLAG(_permission_write_bit)))
	{
		file_write(&file, sizeof(language), &language);
		file_close(&file);
	}
}

static void cached_map_files_open_all(
	void)
{
	struct cache_file_header new_header;
	struct cache_file_header dvd_header;
	char path[256];
	OVERLAPPED overlapped;
	short map_file_index;
	boolean files_deleted = FALSE;

	cached_map_files_delete(NUMBER_OF_CACHED_MAP_FILES);

	for (map_file_index = 0; map_file_index < NUMBER_OF_CACHED_MAP_FILES; map_file_index++)
	{
		HANDLE handle;
		long size;
		struct cached_map_file *map_file = cached_map_file_get(map_file_index);
		boolean valid = FALSE;

		cached_map_file_get_path(map_file_index, path);
		size = cached_map_file_get_size(map_file_index);
		handle = CreateFile(
			path,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_ALWAYS,
			FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
			NULL);

		if (handle != INVALID_HANDLE_VALUE)
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS && GetFileSize(handle, NULL) == (unsigned long)size)
			{
				valid = TRUE;
			}
			else
			{
				volatile boolean write_complete;

				if (!files_deleted)
				{
					cached_map_files_delete(map_file_index);
					files_deleted = TRUE;
				}

				write_complete = FALSE;
				cached_map_issue_async_write(
					handle,
					&overlapped,
					&new_header,
					sizeof(new_header),
					0,
					cache_file_blocking_io_completion_routine,
					&write_complete);
				cached_map_block_on_async_request(&write_complete);
				valid = write_complete && SetFilePointer(handle, size, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER && SetEndOfFile(handle);
				match_vassert(
					"c:\\halo\\SOURCE\\cache\\cache_files_windows.c",
					811,
					valid,
					csprintf(temporary, "setup for new cache file failed (#%d)", GetLastError()));
			}
		}

		match_vassert(
			"c:\\halo\\SOURCE\\cache\\cache_files_windows.c",
			823,
			handle != INVALID_HANDLE_VALUE,
			csprintf(temporary, "couldn't open or create new cache file (#%d)", GetLastError()));

		if (!valid && handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(handle);
			handle = INVALID_HANDLE_VALUE;
		}

		map_file->handle = handle;

		if (valid)
		{
			struct cached_map_file *read_map_file = cached_map_file_get(map_file_index);

			cached_map_file_read_header(map_file_index);

			if (strcmp(read_map_file->header.build_number, BUILD_STRING) != 0)
			{
				valid = FALSE;
			}

			if (!cache_file_read_header_from_dvd(read_map_file->header.name, &dvd_header) ||
				read_map_file->header.checksum != dvd_header.checksum)
			{
				valid = FALSE;
			}
		}

		if (!valid)
		{
			memset(&map_file->header, 0, sizeof(map_file->header));
		}
	}
}

void cache_files_precache_set_priority(
	boolean blocking)
{
	cache_copy_set_priority(blocking);
}

boolean cache_files_precache_in_progress(
	void)
{
	return cache_file_globals.copy_in_progress;
}

boolean cache_files_precache_is_copying_map(
	char const *name)
{
	boolean result;

	if (cache_file_globals.copying_to_map_file_index != NONE &&
		strcmp(cache_file_globals.copying_to_map_file_name, tag_name_strip_path(name)) == 0)
	{
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}

	return result;
}

boolean cache_files_precache_map_loaded(
	char const *name)
{
	return cached_map_files_find_map(tag_name_strip_path(name)) != NONE;
}

boolean cache_files_precache_map_begin(
	char const *name,
	boolean blocking)
{
	struct cache_file_header header;
	char path[256];
	boolean result;
	char const *stripped_name = tag_name_strip_path(name);

	if (cached_map_files_find_map(tag_name_strip_path(name)) != NONE)
	{
		result = TRUE;
	}
	else if (cache_file_read_header_from_dvd(stripped_name, &header))
	{
		long buffer_size = cache_copy_buffer_size(blocking);
		void *buffer = texture_cache_steal_memory(buffer_size);
		short map_file_index = cached_map_files_find_free_map(header.size, header.scenario_type);
		struct cached_map_file *map_file = cached_map_file_get(map_file_index);

		memset(&map_file->header, 0, sizeof(map_file->header));
		cache_file_globals.copy_in_progress = TRUE;
		cache_file_globals.copying_to_map_file_index = map_file_index;
		strncpy(
			cache_file_globals.copying_to_map_file_name,
			stripped_name,
			sizeof(cache_file_globals.copying_to_map_file_name) - 1);
		cache_file_globals.copying_to_map_file_name[sizeof(cache_file_globals.copying_to_map_file_name) - 1] = 0;
		scenario_name_to_cache_file_path(stripped_name, path);
		error(_error_silent, "starting precaching of map '%s'", stripped_name);
		cache_copy_begin(
			buffer,
			buffer_size,
			cached_map_file_get_handle(map_file_index),
			cached_map_file_get_size(map_file_index),
			path);
		result = TRUE;
	}
	else
	{
		error(_error_silent, "couldn't find map '%s' on the DVD", stripped_name);
		error(_error_silent, "full path name '%s'", name);

		if (blocking)
		{
			display_error_damaged_media();
		}

		result = FALSE;
	}

	return result;
}

short cache_files_precache_map_status(
	real *progress)
{
	short status;

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 984, cache_file_globals.copy_in_progress);

	switch (cache_copy_get_status(progress))
	{
	case _cache_copy_bad_file_failure:
	case _cache_copy_read_failure:
		status = _cached_map_file_failed;
		break;
	case _cache_copy_write_failure:
		cached_map_file_invalidate(cache_file_globals.copying_to_map_file_index);
		status = _cached_map_file_failed;
		break;
	case _cache_copy_in_progress:
		status = _cached_map_file_in_progress;
		break;
	case _cache_copy_finised:
		status = _cached_map_file_success;
		break;
	default:
		match_halt("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1013);
	}

	return status;
}

void cache_files_precache_map_queue_end(
	void)
{
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1022, cache_file_globals.copy_in_progress);
	cache_copy_queue_end();
}

void cache_files_precache_map_end(
	void)
{
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1031, cache_file_globals.copy_in_progress);
	cache_copy_end();
	texture_cache_return_memory();
	cached_map_file_set_modification_date(cache_file_globals.copying_to_map_file_index);
	cached_map_file_read_header(cache_file_globals.copying_to_map_file_index);
	cache_file_globals.copy_in_progress = FALSE;
	cache_file_globals.copying_to_map_file_index = NONE;
}

static void cached_map_file_set_modification_date(
	short map_file_index)
{
	SYSTEMTIME system_time;
	struct cached_map_file *map_file = cached_map_file_get(map_file_index);

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &map_file->last_modification_date);
	SetFileTime(map_file->handle, &map_file->last_modification_date, NULL, NULL);
}

static void cached_map_file_read_header(
	short map_file_index)
{
	char path[256];
	OVERLAPPED overlapped;
	volatile boolean read_complete;
	struct cached_map_file *map_file = cached_map_file_get(map_file_index);

	cached_map_file_get_path(map_file_index, path);
	GetFileTime(map_file->handle, &map_file->last_modification_date, NULL, NULL);
	read_complete = FALSE;
	cached_map_issue_async_read(
		map_file->handle,
		&overlapped,
		&map_file->header,
		sizeof(map_file->header),
		0,
		cache_file_blocking_io_completion_routine,
		&read_complete);
	cached_map_block_on_async_request(&read_complete);

	if (read_complete)
	{
		if (!cache_file_header_verify(&map_file->header, path, FALSE))
		{
			memset(&map_file->header, 0, sizeof(map_file->header));
			memset(&map_file->last_modification_date, 0, sizeof(map_file->last_modification_date));
		}
	}
	else
	{
		match_vhalt(
			"c:\\halo\\SOURCE\\cache\\cache_files_windows.c",
			1121,
			csprintf(temporary, "couldn't read header from cache file (#%d)", GetLastError()));
		cached_map_file_invalidate(map_file_index);
	}
}

static short cached_map_files_find_map(
	char const *name)
{
	short map_file_index;

	for (map_file_index = 0; map_file_index < NUMBER_OF_CACHED_MAP_FILES; map_file_index++)
	{
		if (!_stricmp(name, cached_map_file_get(map_file_index)->header.name))
		{
			return map_file_index;
		}
	}

	return NONE;
}

static short cached_map_files_find_free_map(
	long size,
	short scenario_type)
{
	short first_map_file_index;
	short last_map_file_index;
	short map_file_index;
	struct cached_map_file *best_map_file;
	short best_map_file_index = NONE;

	switch (scenario_type)
	{
	case _scenario_type_solo:
		first_map_file_index = _first_large_cached_map_file;
		last_map_file_index = _last_large_cached_map_file;
		break;
	case _scenario_type_multiplayer:
		first_map_file_index = _first_small_cached_map_file;
		last_map_file_index = _last_small_cached_map_file;
		break;
	case _scenario_type_main_menu:
		first_map_file_index = _first_medium_cached_map_file;
		last_map_file_index = _last_medium_cached_map_file;
		break;
	default:
		match_halt("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1172);
	}

	for (map_file_index = first_map_file_index; map_file_index <= last_map_file_index; map_file_index++)
	{
		if (cache_file_globals.open_map_file_index != map_file_index)
		{
			struct cached_map_file *map_file = cached_map_file_get(map_file_index);

			if (cached_map_file_get_size(map_file_index) > size)
			{
				if (best_map_file_index == NONE ||
					cached_map_file_get_size(map_file_index) < cached_map_file_get_size(best_map_file_index) ||
					CompareFileTime(&best_map_file->last_modification_date, &map_file->last_modification_date) > 0)
				{
					best_map_file_index = map_file_index;
					best_map_file = map_file;
				}
			}
		}
	}

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1200, cache_file_globals.open_map_file_index!=best_map_file_index);

	return best_map_file_index;
}

static struct cached_map_file *cached_map_file_get(
	short map_file_index)
{
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1208, map_file_index>=0 && map_file_index<NUMBER_OF_CACHED_MAP_FILES);

	return &cache_file_globals.cached_map_files[map_file_index];
}

static void cached_map_file_invalidate(
	short map_file_index)
{
	cached_map_file_get(map_file_index)->handle = INVALID_HANDLE_VALUE;
}

static HANDLE cached_map_file_get_handle(
	short map_file_index)
{
	return cached_map_file_get(map_file_index)->handle;
}

static long cached_map_file_get_size(
	short map_file_index)
{
	long size;

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1232, map_file_index>=0 && map_file_index<NUMBER_OF_CACHED_MAP_FILES);

	if (map_file_index <= _last_large_cached_map_file)
	{
		size = LARGE_CACHED_MAP_FILE_SIZE;
	}
	else if (map_file_index <= _last_medium_cached_map_file)
	{
		size = MEDIUM_CACHED_MAP_FILE_SIZE;
	}
	else
	{
		size = SMALL_CACHED_MAP_FILE_SIZE;
	}

	return size;
}

static void cached_map_file_get_path(
	short map_file_index,
	char *path)
{
	sprintf(path, "z:\\cache%03d.map", map_file_index);
}

static void cache_file_windows_thread_create(
	void)
{
	cache_file_globals.sleep_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1253, cache_file_globals.sleep_event);
	cache_file_globals.thread = CreateThread(
		NULL,
		CACHE_FILE_WINDOWS_THREAD_STACK_SIZE,
		(LPTHREAD_START_ROUTINE)cache_file_windows_thread_proc,
		NULL,
		0,
		NULL);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1257, cache_file_globals.thread);
}

static void cache_file_windows_thread_wake(
	void)
{
	SetEvent(cache_file_globals.sleep_event);
}

static unsigned long __stdcall cache_file_windows_thread_proc(
	void)
{
	for (;;)
	{
		struct cache_request *best_request;

		while (WaitForSingleObjectEx(cache_file_globals.sleep_event, INFINITE, TRUE) == WAIT_IO_COMPLETION);

		do
		{
			short request_index;

			best_request = NULL;

			for (request_index = 0; request_index < MAXIMUM_SIMULTANEOUS_CACHE_REQUESTS; request_index++)
			{
				struct cache_request *request = cache_request_get(request_index);

				if (request->pending && !request->running &&
					(!best_request || (best_request->blocking > request->blocking && best_request->overlapped.Offset > request->overlapped.Offset)))
				{
					best_request = request;
				}
			}

			if (best_request)
			{
				HANDLE file = cached_map_file_get_handle(cache_file_globals.open_map_file_index);

				match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1327, !best_request->running);
				best_request->running = TRUE;
				cached_map_issue_async_read(
					file,
					&best_request->overlapped,
					best_request->buffer,
					best_request->size,
					best_request->overlapped.Offset,
					cache_file_read_io_completion_routine,
					(boolean *)best_request->overlapped.hEvent);
			}
		}
		while (best_request);
	}
}

static void __stdcall cache_file_read_io_completion_routine(
	unsigned long error_code,
	unsigned long bytes_transferred,
	OVERLAPPED *overlapped)
{
	struct cache_request *finished_request = (struct cache_request *)overlapped;

	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1387, error_code==ERROR_SUCCESS);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1388, bytes_transferred==finished_request->size);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1389, finished_request->overlapped.hEvent);
	*(boolean *)finished_request->overlapped.hEvent = TRUE;
	finished_request->pending = FALSE;
	finished_request->running = FALSE;
}

static void __stdcall cache_file_blocking_io_completion_routine(
	unsigned long error_code,
	unsigned long bytes_transferred,
	OVERLAPPED *overlapped)
{
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1405, error_code==ERROR_SUCCESS);
	match_assert("c:\\halo\\SOURCE\\cache\\cache_files_windows.c", 1406, overlapped->hEvent);
	*(boolean *)overlapped->hEvent = TRUE;
}

static void scenario_name_to_cache_file_path(
	char const *scenario_name,
	char *path)
{
	sprintf(path, "%s%s.map", cache_files_map_directory(), scenario_name);
}

/* ---------- private code */
