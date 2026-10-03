/*
SOUND_IMPORT.H

header included in hcex build.
*/

#ifndef __SOUND_IMPORT_H
#define __SOUND_IMPORT_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct sound_file_info
{
	long samples_per_second; /* fake name */
	short channel_count; /* fake name */
	short unused; /* fake name */
	short significant_bits_per_sample;
};

/* ---------- prototypes/SOUND_IMPORT.C */

boolean sound_file_info_get(struct sound_file_info *info, struct file_reference *file);
boolean sound_raw_sample_data_get(struct file_reference *file, struct sound_file_info const *info, long *size, void *buffer);

/* ---------- prototypes/SOUND_AIFF.C */

boolean sound_file_is_aiff(struct file_reference *file);
boolean sound_file_aiff_info_get(struct file_reference *file, struct sound_file_info *info);
boolean sound_file_aiff_raw_data_get(struct file_reference *file, long *size, void *buffer);
void sound_file_aiff_format(struct sound_file_info const *info, long *size, void *buffer);

/* ---------- prototypes/SOUND_WAVE.C */

boolean sound_file_is_wave(struct file_reference *file);
boolean sound_file_wave_info_get(struct file_reference *file, struct sound_file_info *info);
boolean sound_file_wave_raw_data_get(struct file_reference *file, long *size, void *buffer);
void sound_file_wave_format(struct sound_file_info const *info, long *size, void *buffer);

/* ---------- globals */

/* ---------- public code */

#endif // __SOUND_IMPORT_H
