/*
SOUND_ENVIRONMENT_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __SOUND_ENVIRONMENT_DEFINITIONS_H
#define __SOUND_ENVIRONMENT_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	SOUND_ENVIRONMENT_DEFINITION_TAG = 'snde' /* fake name */
};

/* ---------- macros */

#define sound_environment_get(index) ((index) == NONE ? &default_sound_environment : (struct sound_environment *)tag_get(SOUND_ENVIRONMENT_DEFINITION_TAG, (index))) /* fake name */
#define sound_environment_try_and_get(index) ((index) == NONE ? (struct sound_environment *)NULL : sound_environment_get(index)) /* fake name */

/* ---------- structures */

struct sound_environment
{
	long pad1;
	short priority;
	unsigned short pad2;
	real room_intensity;
	real room_intensity_hf;
	real room_rolloff_factor;
	real decay_time;
	real decay_hf_ratio;
	real reflections_intensity;
	real reflections_delay;
	real reverb_intensity;
	real reverb_delay;
	real diffusion;
	real density;
	real hf_reference;
	long unused[4];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

extern struct sound_environment default_sound_environment;

/* ---------- public code */

#endif // __SOUND_ENVIRONMENT_DEFINITIONS_H
