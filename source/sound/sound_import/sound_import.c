/*
SOUND_IMPORT.C
*/

/* ---------- headers */

#include "cseries.h"
#include "sound_import.h"
#include "files.h"

/* ---------- public code */

boolean sound_file_info_get(
	struct sound_file_info *info,
	struct file_reference *file)
{
	boolean success = TRUE;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_import\\sound_import.c", 18, info);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_import\\sound_import.c", 19, file);

	if (!(sound_file_is_aiff(file) && sound_file_aiff_info_get(file, info)) &&
		!(sound_file_is_wave(file) && sound_file_wave_info_get(file, info)))
	{
		success = FALSE;
	}

	return success;
}

boolean sound_raw_sample_data_get(
	struct file_reference *file,
	struct sound_file_info const *info,
	long *size,
	void *buffer)
{
	boolean success = TRUE;

	if (sound_file_is_aiff(file) && sound_file_aiff_raw_data_get(file, size, buffer))
	{
		sound_file_aiff_format(info, size, buffer);
	}
	else if (sound_file_is_wave(file) && sound_file_wave_raw_data_get(file, size, buffer))
	{
		sound_file_wave_format(info, size, buffer);
	}
	else
	{
		success = FALSE;
	}

	return success;
}
