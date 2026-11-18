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
	FILE_REFERENCE_SIZE = 268,
	MAXIMUM_FILENAME_LENGTH = 255
};

enum
{
	_permission_read_bit = 0,
	_permission_write_bit,
	_permission_append_bit,
	NUMBER_OF_PERMISSION_FLAGS
};

enum
{
	_file_reference_application_relative = 0,
#ifndef xbox
	_file_reference_cd_relative,
#endif
	_file_reference_absolute,

	NUMBER_OF_FILE_REFERENCE_LOCATIONS
};

enum
{
	_name_directory_bit = 0,
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

void file_location_set_volume(short location, char const *volume_name);
struct file_reference *file_reference_create(struct file_reference *reference, short location);
struct file_reference *file_reference_create_from_path(struct file_reference *reference, char const *path, boolean directory);
struct file_reference *file_reference_copy(struct file_reference *destination, struct file_reference const *source);
struct file_reference *file_reference_add_directory(struct file_reference *reference, char const *directory);
struct file_reference *file_reference_set_name(struct file_reference *reference, char const *name);
short file_reference_get_location(struct file_reference const *reference);
char *file_reference_get_name(struct file_reference const *reference, unsigned long flags, char *name);
boolean file_references_equal(struct file_reference const *reference0, struct file_reference const *reference1);
long find_files(unsigned long flags, struct file_reference const *directory, long maximum_count, struct file_reference *references);
void *file_read_into_memory(struct file_reference *reference, unsigned long *size);
void file_printf(struct file_reference *file, char *format, ...);
void directory_create_or_delete_contents(char const *directory_name);
boolean datastore_read(char const *file_name, char const *field_name, long length, void *data);
boolean datastore_write(char const *file_name, char const *field_name, long length, void const *data);
#ifdef DEBUG
struct file_reference_info *file_reference_get_info(struct file_reference *reference);
#else
#define file_reference_get_info(reference) ((struct file_reference_info *)(reference))
#endif

/* ---------- prototypes/FILES_WINDOWS.C */

boolean file_location_is_valid(short location);
boolean file_create(struct file_reference *file);
boolean file_delete(struct file_reference *file);
boolean file_exists(struct file_reference const *file);
boolean file_rename(struct file_reference *file, char const *name);
boolean file_open(struct file_reference *file, unsigned long flags);
boolean file_close(struct file_reference *file);
unsigned long file_get_position(struct file_reference const *file);
boolean file_set_position(struct file_reference const *file, unsigned long position);
unsigned long file_get_eof(struct file_reference const *file);
boolean file_set_eof(struct file_reference const *file, unsigned long position);
boolean file_read(struct file_reference const *file, unsigned long count, void *buffer);
boolean file_write(struct file_reference const *file, unsigned long count, void const *buffer);
boolean file_read_from_position(struct file_reference const *file, unsigned long position, unsigned long count, void *buffer);
boolean file_write_to_position(struct file_reference const *file, unsigned long position, unsigned long count, void const *buffer);
boolean file_get_last_modification_date(struct file_reference const *file, struct file_last_modification_date *date);
long file_compare_last_modification_dates(struct file_last_modification_date *date1, struct file_last_modification_date *date2);
boolean file_get_size(struct file_reference const *file, unsigned long *size);
void find_files_start(unsigned long flags, struct file_reference const *directory);
boolean find_files_next(struct file_reference *file, struct file_last_modification_date *date);
void file_path_add_name(char *path, char const *name);
void file_path_add_extension(char *path, char const *extension);
void file_path_remove_name(char *path);
void file_path_split(char *path, char **directory, char **parent_directory, char **filename, char **extension, boolean has_filename);
void file_location_get_full_path(short location, char const *path, char *full_path);
boolean file_read_only(struct file_reference *file);

/* ---------- globals */

extern char file_location_volume_names[NUMBER_OF_FILE_REFERENCE_LOCATIONS-1][MAXIMUM_FILENAME_LENGTH+1];

/* ---------- public code */

#endif // __FILES_H
