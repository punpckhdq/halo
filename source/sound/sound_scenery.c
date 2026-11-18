/*
SOUND_SCENERY.C
*/

/* ---------- headers */

#include "cseries.h"
#include "sound_scenery.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

boolean sound_scenery_new(
	long object_index)
{
	struct sound_scenery_datum *sound_scenery = sound_scenery_get(object_index);

	SET_FLAG(sound_scenery->object.flags, _object_shadowless_bit, TRUE);

	return TRUE;
}

void sound_scenery_delete(
	long sound_scenery_index)
{
	return;
}

/* ---------- private code */
