/*
FILES.H

header included in hcex build.
*/

#ifndef __FILES_H
#define __FILES_H
#pragma once

/* ---------- constants */

enum
{
	FILE_REFERENCE_SIZE= 268,
	MAXIMUM_FILENAME_LENGTH= 255
};

enum
{
	_permission_read_bit= 0,
	_permission_write_bit,
	_permission_append_bit,
	NUMBER_OF_PERMISSION_FLAGS
};

enum
{
	_file_reference_application_relative= 0,
#ifndef xbox
	_file_reference_cd_relative,
#endif
	_file_reference_absolute,

	NUMBER_OF_FILE_REFERENCE_LOCATIONS
};

enum
{
	FILE_REFERENCE_SIGNATURE= 'filo'
};

enum
{
	_has_filename_bit= 0,

	NUMBER_OF_REFERENCE_INFO_FLAGS
};

enum
{
	//NUMBER_OF_DATASTORE_ENTRIES
	DATASTORE_MAX_DATA_SIZE= 255,
	DATASTORE_MAX_FIELD_NAME_SIZE= 255
};

struct datastore_entry
{
	char name[DATASTORE_MAX_FIELD_NAME_SIZE];
	char data[DATASTORE_MAX_DATA_SIZE];
};

struct datastore
{
	struct datastore_entry entry[200];
};

//enum
//{
//    _find_files_recursive_bit= 0,
//    _find_files_enumerate_directories_bit,
//    NUMBER_OF_FIND_FILES_FLAGS,
//};
//windows version of the header below?
//enum
//{
//    FIRST_DRIVE_LETTER= 65, // 0x0041 'A'
//    LAST_DRIVE_LETTER= 90, // 0x005A 'Z'
//    DRIVE_NAME_LENGTH= 4, // 0x0004
//    DIRECTORY_SEPARATOR= 92, // 0x005C '\'
//    EXTENSION_SEPARATOR= 46, // 0x002E '.'
//    BAD_FILE= -1, // 0xFF
//    MAXIMUM_SEARCH_DEPTH= 8, // 0x0008
//};
//
//enum
//{
//    has_filename_bit= 0,
//    NUMBER_OF_REFERENCE_INFO_FLAGS,
//};
//
//struct find_files_state
//{
//    unsigned long flags; // 0x0
//    short depth; // 0x4
//    short location; // 0x6
//    char path[256]; // 0x8
//    void *handles[8]; // 0x108
//    _WIN32_FIND_DATAA data; // 0x128
//};
//static_assert(sizeof(find_files_state) == 616, "Invalid find_files_state size");

struct file_reference_info
{
	unsigned long signature; // 0x0
	unsigned short flags; // 0x4
	short location; // 0x6
	char path[256]; // 0x8
#ifdef BUILDING_FILES_WINDOWS
	void *file_handle;
#endif
};

enum
{
	_name_directory_bit= 0,
	_name_parent_directory_bit,
	_name_filename_bit,
	_name_extension_bit,
	NUMBER_OF_NAME_FLAGS,
};

/* ---------- macros */

/* ---------- structures */

struct file_reference
{
	char data[FILE_REFERENCE_SIZE];
};

struct file_last_modification_date
{
	char data[8];
};

/* ---------- prototypes/FILES.C */

void file_location_set_volume(short location, const char *volume_name);
struct file_reference *file_reference_create(struct file_reference *reference, short location);
long find_files(unsigned long flags, const struct file_reference *directory, long maximum_count, struct file_reference *references);
void *file_read_into_memory(struct file_reference *reference, unsigned long *size);
void file_printf(struct file_reference *file, char *format, ...);
struct file_reference_info *file_reference_get_info(struct file_reference *reference);
struct file_reference *file_reference_copy(struct file_reference *destination, const struct file_reference *source);
struct file_reference *file_reference_add_directory(struct file_reference *reference, const char *directory);
struct file_reference *file_reference_set_name(struct file_reference *reference, const char *name);
short file_reference_get_location(const struct file_reference *reference);
char *file_reference_get_name(const struct file_reference *reference, unsigned long flags, char *name);
boolean file_references_equal(const struct file_reference *reference0, const struct file_reference *reference1);
struct file_reference *file_reference_create_from_path(struct file_reference *reference, const char *path, boolean is_directory);
void directory_create_or_delete_contents(const char *directory_name);
boolean datastore_read(const char *file_name, const char *field_name, long length, void *data);
boolean datastore_write(const char *file_name, const char *field_name, long length, const void *data);

/* ---------- prototypes/FILES_WINDOWS.C */

// file_location_is_valid
// file_compare_last_modification_dates
void find_files_start(unsigned long flags, const struct file_reference *directory);
void file_path_add_name(char *path, const char *name);
void file_path_add_extension(char *path, const char *extension);
void file_path_remove_name(char *path);
void file_path_split(char *path, char **directory, char **parent_directory, char **filename, char **extension, boolean has_filename);
void file_location_get_full_path(short location, const char *path, char *full_path);
// file_read_only
boolean file_create(struct file_reference *file);
boolean file_delete(struct file_reference *file);
boolean file_exists(const struct file_reference *file);
// file_rename
boolean file_open(struct file_reference *file, unsigned long flags);
boolean file_close(struct file_reference *file);
unsigned long file_get_position(const struct file_reference *file);
boolean file_set_position(const struct file_reference *file, unsigned long position);
unsigned long file_get_eof(const struct file_reference *file);
boolean file_set_eof(const struct file_reference *file, unsigned long position);
boolean file_read(const struct file_reference *file, unsigned long count, void *buffer);
boolean file_write(const struct file_reference *file, unsigned long count, const void *buffer);
// file_read_from_position
// file_write_to_position
// file_get_last_modification_date
// file_get_size
boolean find_files_next(struct file_reference *file, struct file_last_modification_date *date);

/* ---------- globals */

/* comm */
char file_location_volume_names[NUMBER_OF_FILE_REFERENCE_LOCATIONS-1][256];

/* ---------- public code */

#endif // __FILES_H
