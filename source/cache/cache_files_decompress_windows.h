/*
CACHE_FILES_DECOMPRESS_WINDOWS.H

header included in hcex build.
*/

#ifndef __CACHE_FILES_DECOMPRESS_WINDOWS_H
#define __CACHE_FILES_DECOMPRESS_WINDOWS_H
#pragma once

/* ---------- constants */

enum
{
	_cache_copy_bad_file_failure = 0,
	_cache_copy_read_failure,
	_cache_copy_write_failure,
	_cache_copy_in_progress,
	_cache_copy_finised,
	NUMBER_OF_CACHE_COPY_STATES,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/CACHE_FILES_DECOMPRESS_WINDOWS.C */

long cache_copy_buffer_size(boolean should_block);
void cache_copy_set_priority(boolean blocking);
void cache_copy_initialize(void);
boolean cache_copy_compressed_file_complete(void);
void cache_copy_begin(void *buffer, long size, HANDLE destination_file, long destination_file_size, char const *source_file_name);
short cache_copy_get_status(real *progress);
void cache_copy_queue_end(void);
void cache_copy_end(void);
void __stdcall cache_copy_FileIOCompletionRoutine(unsigned long dwErrorCode, unsigned long dwNumberOfBytesTransfered, OVERLAPPED *lpOverlapped);

/* ---------- globals */

extern boolean decompressor_print_timing;

/* ---------- public code */

#endif // __CACHE_FILES_DECOMPRESS_WINDOWS_H
