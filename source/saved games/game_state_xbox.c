/*
GAME_STATE_XBOX.C
*/

/* ---------- headers */

#include "cseries.h"
#include "game_state.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "network_game_globals.h"
#include "sound_manager.h"
#include "text_group.h"
#include "player_ui.h"
#include "physical_memory_map.h"

/* ---------- constants */

enum
{
	GAME_STATE_SIZE = 0x345000,			/* fake name */
	GAME_STATE_FILE_SIZE = 0x380000,	/* fake name */
};

/* ---------- macros */

#define GAME_STATE_FILE_NAME "savegame.bin"						/* fake name */
#define GAME_STATE_FILE_PATH "z:\\" GAME_STATE_FILE_NAME		/* fake name */
#define CORE_DIRECTORY "d:\\core"								/* fake name */
#define CORE_PATH_FORMAT CORE_DIRECTORY "\\%s"					/* fake name */

/* ---------- prototypes */

static boolean game_state_get_persistent_storage_path(char *path);
static void delete_persistent_storage(void);
static HANDLE game_state_open_persistent_storage(char const *directory);

/* ---------- globals */

static struct
{
	boolean buffer_allocated;
	void *buffer;
	long buffer_size;
	boolean file_open;
	boolean file_valid_for_read;
	HANDLE handle;
} xbox_game_state_globals;

/* ---------- public code */

void *game_state_allocate_buffer(
	unsigned long address,
	unsigned long cpu_size,
	unsigned long gpu_size)
{
	void *result;

	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 46, !xbox_game_state_globals.buffer_allocated);

	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 48, address);
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 49, cpu_size>0);
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 50, gpu_size>0);


	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 53, !(cpu_size&(CPU_PAGE_SIZE-1)));
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 54, !(gpu_size&(CPU_PAGE_SIZE-1)));

	result = physical_memory_get_game_state_base_address();

	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 58, result);
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 59, (unsigned long)result==address);

	XPhysicalProtect((void *)(address + cpu_size), gpu_size, PAGE_READWRITE | PAGE_WRITECOMBINE);

	xbox_game_state_globals.buffer_allocated = TRUE;
	xbox_game_state_globals.buffer = (void *)address;
	xbox_game_state_globals.buffer_size = cpu_size + gpu_size;

	return result;
}

void game_state_free_buffer(
	void)
{
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 75, xbox_game_state_globals.buffer_allocated);

	XPhysicalFree(xbox_game_state_globals.buffer);
	xbox_game_state_globals.buffer_allocated = FALSE;

	return;
}

void game_state_create_or_open_file(
	void)
{
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 86, xbox_game_state_globals.buffer_allocated);
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 87, !xbox_game_state_globals.file_open);

	xbox_game_state_globals.handle = CreateFile(
		GAME_STATE_FILE_PATH,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	if (xbox_game_state_globals.handle != INVALID_HANDLE_VALUE &&
		SetFilePointer(xbox_game_state_globals.handle, GAME_STATE_FILE_SIZE, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER &&
		SetEndOfFile(xbox_game_state_globals.handle))
	{
		xbox_game_state_globals.file_open = TRUE;
	}
	else
	{
		match_vhalt(
			"c:\\halo\\SOURCE\\saved games\\game_state_xbox.c",
			97,
			csprintf(
				temporary,
				"couldn't open or create saved game file (#%d)",
				GetLastError()));
	}

	return;
}

void game_state_close_file(
	void)
{
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 106, xbox_game_state_globals.file_open);

	CloseHandle(xbox_game_state_globals.handle);
	xbox_game_state_globals.file_open = FALSE;

	return;
}

boolean game_state_write_to_file(
	void)
{
	unsigned long bytes_written;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 120, xbox_game_state_globals.buffer_allocated);
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 121, xbox_game_state_globals.file_open);

	if (SetFilePointer(xbox_game_state_globals.handle, 0, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER &&
		WriteFile(xbox_game_state_globals.handle, xbox_game_state_globals.buffer, xbox_game_state_globals.buffer_size, &bytes_written, NULL) &&
		bytes_written == xbox_game_state_globals.buffer_size)
	{
		xbox_game_state_globals.file_valid_for_read = TRUE;
		success = TRUE;
	}
	else
	{
		match_vhalt(
			"c:\\halo\\SOURCE\\saved games\\game_state_xbox.c",
			132,
			csprintf(
				temporary,
				"couldn't write saved game file (#%d)",
				GetLastError()));
	}

	return success;
}

boolean game_state_read_from_file(
	void)
{
	unsigned long bytes_read;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 144, xbox_game_state_globals.buffer_allocated);
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 145, xbox_game_state_globals.file_open);
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 146, xbox_game_state_globals.file_valid_for_read || recover_saved_games_hack);

	if (SetFilePointer(xbox_game_state_globals.handle, 0, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER &&
		ReadFile(xbox_game_state_globals.handle, xbox_game_state_globals.buffer, xbox_game_state_globals.buffer_size, &bytes_read, NULL) &&
		bytes_read == xbox_game_state_globals.buffer_size)
	{
		success = TRUE;
	}
	else
	{
		match_vhalt(
			"c:\\halo\\SOURCE\\saved games\\game_state_xbox.c",
			156,
			csprintf(
				temporary,
				"couldn't read saved game file (#%d)",
				GetLastError()));
	}

	return success;
}

boolean game_state_write_core(
	char const *name,
	void *buffer,
	long buffer_size)
{
	char filename[1024];
	unsigned long bytes_written;
	HANDLE file;
	boolean success = FALSE;

	CreateDirectory(CORE_DIRECTORY, NULL);
	sprintf(filename, CORE_PATH_FORMAT, name);
	file = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		if (WriteFile(file, buffer, buffer_size, &bytes_written, NULL) &&
			bytes_written == buffer_size)
		{
			success = TRUE;
		}
	}

	CloseHandle(file);

	return success;
}

boolean game_state_read_core_header(
	char const *name,
	void *header,
	long header_size)
{
	char filename[1024];
	unsigned long bytes_read;
	HANDLE file;
	boolean success = FALSE;

	sprintf(filename, CORE_PATH_FORMAT, name);
	file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		if (ReadFile(file, header, header_size, &bytes_read, NULL) &&
			bytes_read == header_size)
		{
			success = TRUE;
		}
	}

	CloseHandle(file);

	return success;
}

void game_state_read_core(
	char const *name,
	void *buffer,
	long buffer_size)
{
	char filename[1024];
	unsigned long bytes_read;
	HANDLE file;

	sprintf(filename, CORE_PATH_FORMAT, name);
	file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file == INVALID_HANDLE_VALUE ||
		!ReadFile(file, buffer, buffer_size, &bytes_read, NULL) ||
		bytes_read != buffer_size)
	{
		match_vhalt(
			"c:\\halo\\SOURCE\\saved games\\game_state_xbox.c",
			226,
			"game state has been corrupted (thank you, come again)");
	}

	CloseHandle(file);

	return;
}

boolean game_state_read_header_from_persistent_storage(
	void *header,
	unsigned long *header_checksum,
	long header_size,
	long buffer_size,
	boolean *corrupted)
{
	byte checksum_buffer[0x20000];
	unsigned long bytes_read;
	unsigned long checksum;
	unsigned long header_checksum_value;
	long bytes_remaining;
	long read_size;
	HANDLE file = game_state_open_persistent_storage(NULL);
	boolean success = FALSE;

	if (corrupted)
	{
		*corrupted = FALSE;
	}

	if (file != INVALID_HANDLE_VALUE)
	{
		if (SetFilePointer(file, 0, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER &&
			ReadFile(file, header, header_size, &bytes_read, NULL) &&
			bytes_read == header_size)
		{
			header_checksum_value = *header_checksum;
			crc_new(&checksum);
			*header_checksum = 0;
			crc_checksum_buffer(&checksum, header, header_size);

			for (bytes_remaining = buffer_size - header_size; bytes_remaining > 0; bytes_remaining -= read_size)
			{
				read_size = MIN(sizeof(checksum_buffer), bytes_remaining);

				if (ReadFile(file, checksum_buffer, read_size, &bytes_read, NULL) &&
					bytes_read == read_size)
				{
					crc_checksum_buffer(&checksum, checksum_buffer, read_size);
				}

				sound_idle();
			}

			if (checksum == header_checksum_value)
			{
				success = TRUE;
			}
			else
			{
				if (corrupted && header_checksum_value)
				{
					*corrupted = TRUE;
				}

				error(_error_silent, "checksum failed on persistent storage");
			}
		}
		else
		{
			match_vhalt(
				"c:\\halo\\SOURCE\\saved games\\game_state_xbox.c",
				298,
				csprintf(
					temporary,
					"couldn't read header from persistent storage (#%d)",
					GetLastError()));
			delete_persistent_storage();
		}

		CloseHandle(file);
	}

	return success;
}

void game_state_write_to_persistent_storage(
	void *buffer,
	unsigned long *header_checksum,
	long header_size,
	long buffer_size)
{
	byte saved_header[0x800];
	unsigned long bytes_written;
	unsigned long checksum;
	HANDLE file = game_state_open_persistent_storage(NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		*header_checksum = 0;
		crc_new(&checksum);
		crc_checksum_buffer(&checksum, buffer, GAME_STATE_SIZE);
		*header_checksum = checksum;

		match_assert("c:\\halo\\SOURCE\\saved games\\game_state_xbox.c", 333, header_size<sizeof(saved_header));

		memcpy(saved_header, buffer, header_size);
		memset(buffer, 0, header_size);

		if (SetFilePointer(file, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER ||
			!WriteFile(file, buffer, buffer_size, &bytes_written, NULL) ||
			bytes_written != buffer_size ||
			SetFilePointer(file, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER ||
			!WriteFile(file, saved_header, header_size, &bytes_written, NULL) ||
			bytes_written != header_size)
		{
			match_vhalt(
				"c:\\halo\\SOURCE\\saved games\\game_state_xbox.c",
				351,
				csprintf(
					temporary,
					"failed to write to persistent storage (#%d)",
					GetLastError()));
			delete_persistent_storage();
		}

		memcpy(buffer, saved_header, header_size);
		CloseHandle(file);
	}

	return;
}

void game_state_read_from_persistent_storage(
	void *buffer,
	long buffer_size)
{
	unsigned long bytes_read;
	HANDLE file = game_state_open_persistent_storage(NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		if (SetFilePointer(file, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER ||
			!ReadFile(file, buffer, buffer_size, &bytes_read, NULL) ||
			bytes_read != buffer_size)
		{
			match_vhalt(
				"c:\\halo\\SOURCE\\saved games\\game_state_xbox.c",
				383,
				csprintf(
					temporary,
					"failed to read from persistent storage (#%d)",
					GetLastError()));
			delete_persistent_storage();
		}

		CloseHandle(file);
	}

	return;
}

char const *game_state_get_persistent_storage_filename(
	void)
{
	return GAME_STATE_FILE_NAME;
}

void game_state_create_persistent_storage(
	char const *path)
{
	HANDLE file = game_state_open_persistent_storage(path);

	if (file != INVALID_HANDLE_VALUE)
	{
		CloseHandle(file);
	}

	return;
}

/* ---------- private code */

static boolean game_state_get_persistent_storage_path(
	char *path)
{
	boolean success = player_ui_get_path_to_local_player_profile_directory(0, path) != FALSE;

	return success;
}

static void delete_persistent_storage(
	void)
{
	char path[MAXIMUM_FILENAME_LENGTH+1];

	if (game_state_get_persistent_storage_path(path))
	{
		DeleteFile(path);
	}

	return;
}

static HANDLE game_state_open_persistent_storage(
	char const *directory)
{
	char path[MAXIMUM_FILENAME_LENGTH+1];
	byte empty_header[0x4000];
	unsigned long bytes_written;
	HANDLE file = INVALID_HANDLE_VALUE;
	boolean success = FALSE;

	if (directory || game_state_get_persistent_storage_path(path))
	{
		if (directory)
		{
			strcpy(path, directory);
		}
		else
		{
			game_state_get_persistent_storage_path(path);
		}

		strcat(path, game_state_get_persistent_storage_filename());
		file = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);

		if (file != INVALID_HANDLE_VALUE)
		{
			if (GetFileSize(file, NULL) == GAME_STATE_FILE_SIZE)
			{
				success = TRUE;
			}
			else
			{
				memset(empty_header, 0, sizeof(empty_header));

				if (WriteFile(file, empty_header, sizeof(empty_header), &bytes_written, NULL) &&
					bytes_written == sizeof(empty_header) &&
					SetFilePointer(file, GAME_STATE_FILE_SIZE, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER &&
					SetEndOfFile(file))
				{
					success = TRUE;
				}
				else
				{
					match_vhalt(
						"c:\\halo\\SOURCE\\saved games\\game_state_xbox.c",
						491,
						csprintf(
							temporary,
							"couldn't resize persistent storage \"%s\"",
							path));
					delete_persistent_storage();
				}
			}
		}
		else
		{
			match_vhalt(
				"c:\\halo\\SOURCE\\saved games\\game_state_xbox.c",
				498,
				csprintf(
					temporary,
					"couldn't open or create persistent storage \"%s\"",
					path));
		}
	}

	if (!success)
	{
		if (file != INVALID_HANDLE_VALUE)
		{
			CloseHandle(file);
		}

		file = INVALID_HANDLE_VALUE;
	}

	return file;
}
