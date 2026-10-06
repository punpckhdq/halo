/*
PREDICTED_RESOURCES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "predicted_resources.h"
#include "texture_cache.h"
#include "sound_cache.h"
#include "objects.h"
#include "unit_definitions.h"
#include "actor_definitions.h"
#include "meter_definitions.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"
#include "sound_definitions.h"
#include "effect_definitions.h"
#include "particle_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void predicted_resources_sound_precache(long sound_definition_index);

/* ---------- globals */

/* ---------- public code */

void predicted_resources_precache(
	struct tag_block *predicted_resources)
{
	short resource_index;

	for (resource_index = 0; resource_index < predicted_resources->count; resource_index++)
	{
		struct predicted_resource const *resource = TAG_BLOCK_GET_ELEMENT(predicted_resources, resource_index, struct predicted_resource);

		switch (resource->type)
		{
		case _predicted_resource_bitmap:
			_texture_cache_bitmap_get_hardware_format(
				TAG_BLOCK_GET_ELEMENT(&bitmap_group_get(resource->tag_index)->bitmaps, resource->resource_index, struct bitmap_data),
				FALSE,
				TRUE);
			break;
		case _predicted_resource_sound:
			predicted_resources_sound_precache(resource->tag_index);
			break;
		}
	}
}

/* ---------- private code */

static void predicted_resources_sound_precache(
	long sound_definition_index)
{
	short pitch_range_index;
	struct sound_definition *sound = sound_definition_get(sound_definition_index);

	for (pitch_range_index = 0; pitch_range_index < sound->pitch_ranges.count; pitch_range_index++)
	{
		short permutation_index;
		struct sound_pitch_range *pitch_range = TAG_BLOCK_GET_ELEMENT(&sound->pitch_ranges, pitch_range_index, struct sound_pitch_range);

		for (permutation_index = 0; permutation_index < pitch_range->actual_permutation_count; permutation_index++)
		{
			_sound_cache_sound_request(
				TAG_BLOCK_GET_ELEMENT(&pitch_range->permutations, permutation_index, struct sound_permutation),
				FALSE,
				TRUE,
				FALSE);
		}
	}
}
