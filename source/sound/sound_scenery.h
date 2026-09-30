/*
SOUND_SCENERY.H

header included in hcex build.
*/

#ifndef __SOUND_SCENERY_H
#define __SOUND_SCENERY_H
#pragma once

/* ---------- headers */

#include "objects.h"

/* ---------- constants */

/* ---------- macros */

#define sound_scenery_get(index)	((struct sound_scenery_datum*)object_get_and_verify_type(index, _object_mask_sound_scenery))
#define sound_scenery_try_and_get(index)	((struct sound_scenery_datum*)object_try_and_get_and_verify_type(index, _object_mask_sound_scenery))

/* ---------- structures */

struct _sound_scenery_datum
{
	long unused;
};

struct sound_scenery_datum
{
	long definition_index;
	struct _object_datum object;
	struct _sound_scenery_datum scenery;
};

/* ---------- prototypes/SOUND_SCENERY.C */

boolean sound_scenery_new(long object_index);
void sound_scenery_delete(long sound_scenery_index);

/* ---------- globals */

/* ---------- public code */

#endif // __SOUND_SCENERY_H
