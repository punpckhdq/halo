/*
SOUND_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "sound_definitions.h"
#include "sound_manager.h"
#include "sound_classes.h"
#include "ima_adpcm.h"
#include "ai_scenario_definitions.h"
#include "interface.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "network_game_globals.h"
#include "text_group.h"
#include "damage_effect_definitions.h"
#include "predicted_resources.h"
#include "sound_cache.h"

/* ---------- globals */

long const sound_sample_rate_samples_per_second[NUMBER_OF_SOUND_SAMPLE_RATES] = { 22050, 44100 };
real const oo_unsigned_char_max = 1.f/UNSIGNED_CHAR_MAX;

/* ---------- public code */

real sound_definition_get_maximum_distance(
	long sound_definition_index)
{
	struct sound_definition *sound = sound_definition_get(sound_definition_index);
	real maximum_distance = sound->maximum_distance;

	if (maximum_distance == 0.f)
	{
		maximum_distance = sound_class_get(sound->class_index)->maximum_distance;
	}

	return maximum_distance;
}

real sound_definition_get_minimum_distance(
	long sound_definition_index)
{
	struct sound_definition *sound = sound_definition_get(sound_definition_index);
	real minimum_distance = sound->minimum_distance;

	if (minimum_distance == 0.f)
	{
		minimum_distance = sound_class_get(sound->class_index)->minimum_distance;
	}

	return minimum_distance;
}

byte *sound_permutation_get_mouth_aperture(
	struct sound_permutation const *permutation,
	short tick_index)
{
	match_assert("c:\\halo\\SOURCE\\sound\\sound_definitions.c", 800, tick_index>=0 && tick_index<permutation->mouth_data.size);

	return (byte *)permutation->mouth_data.address + tick_index;
}

short sound_definition_find_pitch_range_by_pitch(
	struct sound_definition *sound,
	real pitch,
	short old_range_index)
{
	short range_index = NONE;

	if (old_range_index != NONE && old_range_index<sound->pitch_ranges.count)
	{
		struct sound_pitch_range *range = TAG_BLOCK_GET_ELEMENT(&sound->pitch_ranges, old_range_index, struct sound_pitch_range);

		if (range->bend_lower_bound<=pitch && pitch<=range->bend_upper_bound && range->permutations.count)
		{
			range_index = old_range_index;
		}
	}

	if (range_index == NONE)
	{
		real best_ratio = REAL_MAX;
		short pitch_range_index;

		for (pitch_range_index = 0; pitch_range_index<sound->pitch_ranges.count; pitch_range_index++)
		{
			struct sound_pitch_range *range = TAG_BLOCK_GET_ELEMENT(&sound->pitch_ranges, pitch_range_index, struct sound_pitch_range);

			if (range->permutations.count)
			{
				if (range->bend_lower_bound<=pitch && pitch<=range->bend_upper_bound)
				{
					range_index = pitch_range_index;
					break;
				}
				else
				{
					real ratio = range->bend_upper_bound<pitch ? pitch / range->bend_upper_bound : range->bend_lower_bound / pitch;

					if (ratio<best_ratio)
					{
						range_index = pitch_range_index;
						best_ratio = ratio;
					}
				}
			}
		}
	}

	return range_index;
}

void try_to_reset_permutations(
	struct sound_pitch_range *range)
{
	if (!(~range->runtime_permutation_flags & MASK(range->actual_permutation_count)))
	{
		range->runtime_permutation_flags = 0;
		if (range->actual_permutation_count>1)
		{
			SET_FLAG(range->runtime_permutation_flags, range->runtime_last_permutation_index, TRUE);
		}
	}

	return;
}

real sound_permutation_get_real_mouth_aperture(
	struct sound_permutation const *permutation,
	short estimated_tick_index)
{
	if (permutation->mouth_data.size)
	{
		estimated_tick_index = PIN(estimated_tick_index, 0, permutation->mouth_data.size - 1);

		return *sound_permutation_get_mouth_aperture(permutation, estimated_tick_index) * oo_unsigned_char_max;
	}
	else
	{
		error(_error_silent, "but how can you speak if you have no mouth data? (permutation %s)", permutation->name);

		return 0.f;
	}
}

short sound_definition_next_permutation(
	struct sound_definition *sound,
	short pitch_range_index,
	short looping_last_permutation_index)
{
	struct sound_pitch_range *range = TAG_BLOCK_GET_ELEMENT(&sound->pitch_ranges, pitch_range_index, struct sound_pitch_range);
	short permutation_index;
	short attempts = 0;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_definitions.c", 892, range->permutations.count);

	if (range->runtime_discarded_permutation_index != NONE)
	{
		permutation_index = range->runtime_discarded_permutation_index;
		range->runtime_discarded_permutation_index = NONE;
		range->runtime_last_permutation_index = permutation_index;
	}
	else if (TEST_FLAG(sound->flags, _sound_definition_linked_permutations_bit) && looping_last_permutation_index != NONE)
	{
		permutation_index = TAG_BLOCK_GET_ELEMENT(&range->permutations, looping_last_permutation_index, struct sound_permutation)->next_permutation_index;
	}
	else
	{
		permutation_index = local_random_range(0, range->actual_permutation_count);
		while (TRUE)
		{
			try_to_reset_permutations(range);
			if (!TEST_FLAG(range->runtime_permutation_flags, permutation_index))
			{
				SET_FLAG(range->runtime_permutation_flags, permutation_index, TRUE);
				if (attempts++ == 16 ||
					real_local_random() >= TAG_BLOCK_GET_ELEMENT(&range->permutations, permutation_index, struct sound_permutation)->skip_fraction)
				{
					break;
				}
			}

			permutation_index++;
			if (permutation_index == range->actual_permutation_count)
			{
				permutation_index = 0;
			}
		}
		range->runtime_last_permutation_index = permutation_index;
	}

	return permutation_index;
}
