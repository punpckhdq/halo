/*
PLATFORM_SOUND.H

header included in hcex build.
*/

#ifndef __PLATFORM_SOUND_H
#define __PLATFORM_SOUND_H
#pragma once

/* ---------- constants */

enum
{
	_platform_sound_dsound = 0,
	_platform_sound_macintosh,
	NUMBER_OF_PLATFORM_SOUND_CODES,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

extern struct platform_sound_manager_definition platform_sound_dsound;

/* ---------- public code */

#endif // __PLATFORM_SOUND_H
