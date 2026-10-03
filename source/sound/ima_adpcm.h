/*
IMA_ADPCM.H

header included in hcex build.
*/

#ifndef __IMA_ADPCM_H
#define __IMA_ADPCM_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct bungie_ima_adpcm_header
{
	long sample_count; /* fake name */
	short predicted_sample; /* fake name */
	short step_index; /* fake name */
};

struct ima_adpcm_decompression_state /* fake name */
{
	long sample_count; /* fake name */
	long sample_index; /* fake name */
	short predicted_sample; /* fake name */
	short step_index; /* fake name */
};

/* ---------- prototypes/IMA_ADPCM.C */

long compress_ima_adpcm_audio_data(short const *samples, long sample_count, byte *buffer, long buffer_size);
long decompress_ima_adpcm_audio_data(void const *buffer, long buffer_size, short *samples, long maximum_sample_count, struct ima_adpcm_decompression_state *state);
void byte_swap_bungie_ima_adpcm_header(struct bungie_ima_adpcm_header *header);

/* ---------- globals */

/* ---------- public code */

#endif // __IMA_ADPCM_H
