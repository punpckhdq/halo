/*
FILES.C
*/

/* ---------- headers */

#include "cseries.h"

/* ---------- constants */

enum
{
	FILE_REFERENCE_SIGNATURE = 'filo'
};

enum
{
	_has_filename_bit = 0,
	NUMBER_OF_REFERENCE_INFO_FLAGS
};

/* ---------- macros */

#define NUMBER_OF_DATASTORE_ENTRIES 200 /* fake name */
#define DATASTORE_MAX_DATA_SIZE 255
#define DATASTORE_MAX_FIELD_NAME_SIZE 255

/* ---------- structures */

struct datastore_entry
{
	char name[DATASTORE_MAX_FIELD_NAME_SIZE];
	char data[DATASTORE_MAX_DATA_SIZE];
};

struct datastore
{
	struct datastore_entry entry[NUMBER_OF_DATASTORE_ENTRIES];
};

struct file_reference_info
{
	unsigned long signature;
	word flags;
	short location;
	char path[MAXIMUM_FILENAME_LENGTH+1];
};

/* ---------- globals */

char file_location_volume_names[NUMBER_OF_FILE_REFERENCE_LOCATIONS-1][MAXIMUM_FILENAME_LENGTH+1];

/* ---------- public code */

void file_location_set_volume(
	short location,
	char const *volume_name)
{
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 75, location>0 && location<NUMBER_OF_FILE_REFERENCE_LOCATIONS);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 76, strlen(file_location_volume_names[location])==0);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 77, strlen(volume_name)<=MAXIMUM_FILENAME_LENGTH);

	strncpy(file_location_volume_names[location], volume_name, NUMBEROF(*file_location_volume_names)-1);
	file_location_volume_names[location][NUMBEROF(*file_location_volume_names)-1] = '\0';

	return;
}

struct file_reference *file_reference_create(
	struct file_reference *reference,
	short location)
{
	struct file_reference_info *info = (struct file_reference_info *)reference;

	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 91, info);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 92, location>=NONE && location<NUMBER_OF_FILE_REFERENCE_LOCATIONS);

	memset(info, 0, sizeof(*reference));
	info->signature = FILE_REFERENCE_SIGNATURE;
	info->location = location;

	return reference;
}

struct file_reference *file_reference_create_from_path(
	struct file_reference *reference,
	char const *path,
	boolean directory)
{
	file_reference_create(reference, NONE);

	if (directory)
	{
		file_reference_add_directory(reference, path);
	}
	else
	{
		file_reference_set_name(reference, path);
	}

	return reference;
}

struct file_reference *file_reference_copy(
	struct file_reference *destination,
	struct file_reference const *source)
{
	struct file_reference_info const *info = file_reference_get_info((struct file_reference *)source);

	memcpy(destination, source, sizeof(*info));

	return destination;
}

struct file_reference *file_reference_add_directory(
	struct file_reference *reference,
	char const *directory)
{
	struct file_reference_info *info = file_reference_get_info(reference);

	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 137, directory);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 138, !TEST_FLAG(info->flags, _has_filename_bit));

	file_path_add_name(info->path, directory);

	return reference;
}

struct file_reference *file_reference_set_name(
	struct file_reference *reference,
	char const *name)
{
	struct file_reference_info *info = file_reference_get_info(reference);

	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 151, name);

	if (TEST_FLAG(info->flags, _has_filename_bit))
	{
		file_path_remove_name(info->path);
	}

	file_path_add_name(info->path, name);
	SET_FLAG(info->flags, _has_filename_bit, TRUE);

	return reference;
}

short file_reference_get_location(
	struct file_reference const *reference)
{
	struct file_reference_info const *info = file_reference_get_info((struct file_reference *)reference);

	return info->location;
}

char *file_reference_get_name(
	struct file_reference const *reference,
	unsigned long flags,
	char *name)
{
	struct file_reference_info const *info = file_reference_get_info((struct file_reference *)reference);
	char full_path[MAXIMUM_FILENAME_LENGTH+1] = {0};
	char *directory;
	char *parent_directory;
	char *filename;
	char *extension;

	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 185, name);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 186, VALID_FLAGS(info->flags, NUMBER_OF_NAME_FLAGS));
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 187, flags);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 188, flags!=(FLAG(_name_directory_bit)|FLAG(_name_extension_bit)));
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 189, !TEST_FLAG(flags, _name_directory_bit) || !TEST_FLAG(flags, _name_parent_directory_bit));

	file_location_get_full_path(info->location, info->path, full_path);
	file_path_split(
		full_path,
		&directory,
		&parent_directory,
		&filename,
		&extension,
		TEST_FLAG(info->flags, _has_filename_bit));

	name[0] = '\0';

	if (TEST_FLAG(flags, _name_directory_bit))
	{
		file_path_add_name(name, directory);
	}

	if (TEST_FLAG(flags, _name_parent_directory_bit))
	{
		file_path_add_name(name, parent_directory);
	}

	if (TEST_FLAG(flags, _name_filename_bit))
	{
		file_path_add_name(name, filename);
	}

	if (TEST_FLAG(flags, _name_extension_bit))
	{
		file_path_add_extension(name, extension);
	}

	return name;
}

boolean file_references_equal(
	struct file_reference const *reference0,
	struct file_reference const *reference1)
{
	struct file_reference_info const *info0 = file_reference_get_info((struct file_reference *)reference0);
	struct file_reference_info const *info1 = file_reference_get_info((struct file_reference *)reference1);
	boolean equal = FALSE;

	if (info0->location==info1->location && !strcmp(info0->path, info1->path))
	{
		equal = TRUE;
	}

	return equal;
}

long find_files(
	unsigned long flags,
	struct file_reference const *directory,
	long maximum_count,
	struct file_reference *references)
{
	long count = 0;

	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 257, maximum_count>0);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 258, references);

	find_files_start(flags, directory);

	while (count<maximum_count && find_files_next(&references[count], NULL))
	{
		count++;
	}

	return count;
}

void *file_read_into_memory(
	struct file_reference *reference,
	unsigned long *size)
{
	void *buffer = NULL;

	if (file_open(reference, FLAG(_permission_read_bit)))
	{
		*size = file_get_eof(reference);
		buffer = match_malloc("c:\\halo\\SOURCE\\tag_files\\files.c", 280, *size);

		if (buffer)
		{
			if (!file_read(reference, *size, buffer))
			{
				match_free("c:\\halo\\SOURCE\\tag_files\\files.c", 286, buffer);
				buffer = NULL;
			}
		}

		file_close(reference);
	}

	return buffer;
}

void file_printf(
	struct file_reference *file,
	char *format,
	...)
{
	char buffer[1024];
	va_list arglist;

	if (format)
	{
		va_start(arglist, format);
		vsprintf(buffer, format, arglist);
		va_end(arglist);

		file_write(file, strlen(buffer), buffer);
		file_set_eof(file, file_get_position(file));
	}

	return;
}

void directory_create_or_delete_contents(
	char const *directory_name)
{
	struct file_reference directory;

	file_reference_create_from_path(&directory, directory_name, TRUE);

	if (file_exists(&directory))
	{
		struct file_reference file;

		find_files_start(0, &directory);

		while (find_files_next(&file, NULL))
		{
			file_delete(&file);
		}
	}
	else
	{
		file_create(&directory);
	}

	return;
}

boolean datastore_read(
	char const *file_name,
	char const *field_name,
	long length,
	void *data)
{
	boolean success;
	struct file_reference file_ref;
	struct datastore *datastore = NULL;
	unsigned long datastore_size = 0;

	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 369, NULL != file_name);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 370, NULL != field_name);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 371, '\0' != file_name[0]);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 372, '\0' != field_name[0]);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 373, length < DATASTORE_MAX_DATA_SIZE);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 374, strlen(field_name) < DATASTORE_MAX_FIELD_NAME_SIZE);

	file_reference_create_from_path(&file_ref, file_name, FALSE);

	if (file_exists(&file_ref))
	{
		datastore = file_read_into_memory(&file_ref, &datastore_size);

		if (!datastore)
		{
			file_delete(&file_ref);
		}

		if (datastore_size!=sizeof(*datastore))
		{
			match_free("c:\\halo\\SOURCE\\tag_files\\files.c", 389, datastore);
			datastore = NULL;
			datastore_size = 0;
			file_delete(&file_ref);
		}

		if (datastore)
		{
			long itr;

			for (itr = 0; itr<NUMBER_OF_DATASTORE_ENTRIES; itr++)
			{
				if (!strcmp(datastore->entry[itr].name, field_name))
				{
					memcpy(data, datastore->entry[itr].data, length);
					success = TRUE;
					break;
				}

				if (datastore->entry[itr].name[0]=='\0')
				{
					break;
				}
			}

			match_free("c:\\halo\\SOURCE\\tag_files\\files.c", 416, datastore);
		}
	}

	return success;
}

boolean datastore_write(
	char const *file_name,
	char const *field_name,
	long length,
	void const *data)
{
	boolean success = FALSE;
	struct file_reference file_ref;
	struct datastore *datastore = NULL;
	unsigned long datastore_size = 0;

	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 430, NULL != file_name);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 431, NULL != field_name);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 432, '\0' != file_name[0]);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 433, '\0' != field_name[0]);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 434, length < DATASTORE_MAX_DATA_SIZE);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 435, strlen(field_name) < DATASTORE_MAX_FIELD_NAME_SIZE);

	file_reference_create_from_path(&file_ref, file_name, FALSE);

	if (file_exists(&file_ref))
	{
		datastore = file_read_into_memory(&file_ref, &datastore_size);

		if (!datastore)
		{
			file_delete(&file_ref);
		}

		if (datastore_size!=sizeof(*datastore))
		{
			match_free("c:\\halo\\SOURCE\\tag_files\\files.c", 450, datastore);
			datastore = NULL;
			datastore_size = 0;
			file_delete(&file_ref);
		}
	}

	if (!datastore)
	{
		datastore = match_malloc("c:\\halo\\SOURCE\\tag_files\\files.c", 461, sizeof(*datastore));

		if (datastore)
		{
			memset(datastore, 0, sizeof(*datastore));
		}
	}

	if (datastore)
	{
		long itr;

		for (itr = 0; itr<NUMBER_OF_DATASTORE_ENTRIES; itr++)
		{
			if (datastore->entry[itr].name[0]=='\0' || !strcmp(datastore->entry[itr].name, field_name))
			{
				strcpy(datastore->entry[itr].name, field_name);
				memcpy(datastore->entry[itr].data, data, length);
				success = TRUE;
				break;
			}
		}

		if (!file_exists(&file_ref))
		{
			file_create(&file_ref);
		}

		if (file_open(&file_ref, FLAG(_permission_write_bit)))
		{
			file_write(&file_ref, sizeof(*datastore), datastore);
			file_close(&file_ref);
		}

		match_free("c:\\halo\\SOURCE\\tag_files\\files.c", 492, datastore);
	}

	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 495, success);

	return success;
}

#ifdef DEBUG
struct file_reference_info *file_reference_get_info(
	struct file_reference *reference)
{
	struct file_reference_info *info = (struct file_reference_info *)reference;

	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 508, info);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 509, info->signature==FILE_REFERENCE_SIGNATURE);
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 510, VALID_FLAGS(info->flags, NUMBER_OF_REFERENCE_INFO_FLAGS));
	match_assert("c:\\halo\\SOURCE\\tag_files\\files.c", 511, info->location>=NONE && info->location<NUMBER_OF_FILE_REFERENCE_LOCATIONS);

	return info;
}
#endif
