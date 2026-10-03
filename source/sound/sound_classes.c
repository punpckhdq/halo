/*
SOUND_CLASSES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "sound_classes.h"
#include "game_state.h"

/* ---------- prototypes */

static struct sound_class_datum *sound_class_datum_get(short index);

/* ---------- globals */

struct sound_class_definition sound_classes[NUMBER_OF_SOUND_CLASSES] =
{
	{ 6, 4, 100, FALSE, 4, _sound_cache_miss_mode_discard, 0.5f, 0.f, 1.4f, 8.f, 1.f, 1.f },
	{ 4, 1, 200, FALSE, 5, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 8.f, 120.f, 1.f, 1.f },
	{ 0 },
	{ 0 },
	{ 4, 1, 0, FALSE, 4, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 4.f, 70.f, 1.f, 1.f },
	{ 4, 1, 500, FALSE, 4, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 1.f, 9.f, 1.f, 1.f },
	{ 4, 1, 500, FALSE, 4, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 1.f, 9.f, 1.f, 1.f },
	{ 4, 1, 60, FALSE, 4, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 1.f, 9.f, 1.f, 1.f },
	{ 4, 1, 500, FALSE, 4, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 1.f, 9.f, 1.f, 1.f },
	{ 4, 1, 500, FALSE, 4, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 1.f, 9.f, 1.f, 1.f },
	{ 4, 1, 500, FALSE, 4, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 1.f, 9.f, 1.f, 1.f },
	{ 0 },
	{ 0 },
	{ 4, 1, 100, FALSE, 3, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 0.5f, 3.f, 0.f, 1.f },
	{ 4, 1, 100, FALSE, 3, _sound_cache_miss_mode_discard, 0.5f, 0.f, 0.5f, 3.f, 0.f, 1.f },
	{ 4, 1, 1000, FALSE, 3, _sound_cache_miss_mode_discard, 0.5f, 0.f, 0.5f, 3.f, 0.f, 1.f },
	{ 0 },
	{ 0 },
	{ 4, 1, 200, FALSE, 3, _sound_cache_miss_mode_discard, 0.5f, 0.f, 0.9f, 10.f, 1.f, 1.f },
	{ 4, 1, 100, TRUE, 3, _sound_cache_miss_mode_postpone, 0.8f, 0.f, 3.f, 20.f, 0.f, 1.f },
	{ 0 },
	{ 0 },
	{ 4, 2, 400, FALSE, 3, _sound_cache_miss_mode_discard, 0.5f, 0.f, 1.4f, 8.f, 1.f, 1.f },
	{ 4, 2, 100, FALSE, 3, _sound_cache_miss_mode_postpone, 0.9f, 0.f, 1.4f, 8.f, 1.f, 1.f },
	{ 0 },
	{ 0 },
	{ 4, 1, 100, FALSE, 2, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 0.9f, 5.f, 1.f, 1.f },
	{ 4, 1, 100, FALSE, 2, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 0.9f, 5.f, 1.f, 1.f },
	{ 4, 1, 100, FALSE, 1, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 0.9f, 5.f, 1.f, 1.f },
	{ 4, 1, 100, FALSE, 1, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 0.9f, 5.f, 1.f, 1.f },
	{ 4, 1, 100, FALSE, 1, _sound_cache_miss_mode_postpone, 0.5f, 0.f, 0.5f, 3.f, 1.f, 1.f },
	{ 0 },
	{ 4, 4, 100, FALSE, 2, _sound_cache_miss_mode_postpone, 1.f, 0.f, 0.9f, 5.f, 0.f, 1.f },
	{ 4, 1, 100, FALSE, 1, _sound_cache_miss_mode_postpone, 1.f, 0.f, 0.9f, 5.f, 0.f, 1.f },
	{ 4, 1, 100, FALSE, 1, _sound_cache_miss_mode_postpone, 1.f, 0.f, 0.9f, 5.f, 0.f, 1.f },
	{ 4, 1, 100, FALSE, 1, _sound_cache_miss_mode_postpone, 1.f, 0.f, 0.5f, 3.f, 0.f, 1.f },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 4, 1, 100, FALSE, 4, _sound_cache_miss_mode_postpone, 1.f, 0.f, 0.5f, 3.f, 1.f, 1.f },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 4, 4, 100, TRUE, 6, _sound_cache_miss_mode_postpone, 0.8f, 0.f, 3.f, 20.f, 0.f, 1.f },
	{ 4, 4, 100, FALSE, 3, _sound_cache_miss_mode_postpone, 0.8f, 0.f, 2.f, 5.f, 0.f, 1.f },
	{ 4, 4, 100, TRUE, 5, _sound_cache_miss_mode_postpone, 0.8f, 0.f, 3.f, 20.f, 0.f, 1.f },
	{ 4, 4, 100, TRUE, 6, _sound_cache_miss_mode_postpone, 0.8f, 0.f, 3.f, 20.f, 0.f, 1.f },
	{ 0 },
	{ 0 },
	{ 4, 1, 100, FALSE, 5, _sound_cache_miss_mode_postpone, 1.f, 0.f, 3.f, 20.f, 1.f, 1.f },
};

char const *sound_class_names[NUMBER_OF_SOUND_CLASSES] =
{
	"projectile_impact",
	"projectile_detonation",
	"",
	"",
	"weapon_fire",
	"weapon_ready",
	"weapon_reload",
	"weapon_empty",
	"weapon_charge",
	"weapon_overheat",
	"weapon_idle",
	"",
	"",
	"object_impacts",
	"particle_impacts",
	"slow_particle_impacts",
	"",
	"",
	"unit_footsteps",
	"unit_dialog",
	"",
	"",
	"vehicle_collision",
	"vehicle_engine",
	"",
	"",
	"device_door",
	"device_force_field",
	"device_machinery",
	"device_nature",
	"device_computers",
	"",
	"music",
	"ambient_nature",
	"ambient_machinery",
	"ambient_computers",
	"",
	"",
	"",
	"first_person_damage",
	"",
	"",
	"",
	"",
	"scripted_dialog_player",
	"scripted_effect",
	"scripted_dialog_other",
	"scripted_dialog_force_unspatialized",
	"",
	"",
	"game_event"
};

struct sound_class_datum *sound_class_data;

/* ---------- public code */

struct sound_class_definition *sound_class_get(
	short class_index)
{
	struct sound_class_definition const *definition = &sound_classes[class_index];

	match_assert("c:\\halo\\source\\sound\\sound_classes.h", 131, class_index>=0 && class_index<NUMBER_OF_SOUND_CLASSES);
	match_assert("c:\\halo\\source\\sound\\sound_classes.h", 132, sound_class_names[class_index][0]);
	match_assert("c:\\halo\\source\\sound\\sound_classes.h", 133, definition->maximum_number_per_definition<=MAXIMUM_SOUND_INSTANCES_PER_DEFINITION);
	match_assert("c:\\halo\\source\\sound\\sound_classes.h", 134, definition->maximum_number_per_object<=MAXIMUM_SOUND_INSTANCES_PER_OBJECT_PER_DEFINITION);

	return &sound_classes[class_index];
}

void sound_classes_initialize(
	void)
{
	sound_class_data = game_state_malloc("sound classes", NULL, NUMBER_OF_SOUND_CLASSES*sizeof(struct sound_class_datum));

	return;
}

void sound_classes_dispose_from_old_map(
	void)
{
	return;
}

void sound_classes_dispose(
	void)
{
	sound_class_data = NULL;

	return;
}

static struct sound_class_datum *sound_class_datum_get(
	short index)
{
	match_assert("c:\\halo\\SOURCE\\sound\\sound_classes.c", 288, index>=0 && index<NUMBER_OF_SOUND_CLASSES);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_classes.c", 289, sound_class_data);

	return &sound_class_data[index];
}

void debug_sound_classes_enable(
	char const *substring,
	boolean enabled)
{
	short class_index;

	for (class_index = 0; class_index<NUMBER_OF_SOUND_CLASSES; class_index++)
	{
		if (sound_class_names[class_index][0] && strstr(sound_class_names[class_index], substring))
		{
			sound_class_get(class_index)->disabled = !enabled;
		}
	}

	return;
}

void debug_sound_classes_set_distances(
	char const *substring,
	real minimum_distance,
	real maximum_distance)
{
	short class_index;

	for (class_index = 0; class_index<NUMBER_OF_SOUND_CLASSES; class_index++)
	{
		if (sound_class_names[class_index][0] && strstr(sound_class_names[class_index], substring))
		{
			sound_class_get(class_index)->minimum_distance = minimum_distance;
			sound_class_get(class_index)->maximum_distance = maximum_distance;
		}
	}

	return;
}

void debug_sound_classes_set_wet(
	char const *substring,
	real wet)
{
	short class_index;

	for (class_index = 0; class_index<NUMBER_OF_SOUND_CLASSES; class_index++)
	{
		if (sound_class_names[class_index][0] && strstr(sound_class_names[class_index], substring))
		{
			sound_class_get(class_index)->reverb_damping_factor = PIN(1.f - wet, 0.f, 1.f);
		}
	}

	return;
}

void sound_classes_initialize_for_new_map(
	void)
{
	short class_index;

	for (class_index = 0; class_index<NUMBER_OF_SOUND_CLASSES; class_index++)
	{
		struct sound_class_datum *datum = sound_class_datum_get(class_index);

		datum->desired_gain = datum->gain = 1.f;
		datum->ticks = 0;
	}

	return;
}

void sound_classes_update(
	long ticks_elapsed)
{
	if (ticks_elapsed > 0)
	{
		short class_index;

		for (class_index = 0; class_index<NUMBER_OF_SOUND_CLASSES; class_index++)
		{
			struct sound_class_datum *datum = sound_class_datum_get(class_index);

			if (datum->ticks > ticks_elapsed)
			{
				datum->gain += (datum->desired_gain - datum->gain) * ((real)ticks_elapsed / datum->ticks);
				datum->ticks -= (short)ticks_elapsed;
			}
			else
			{
				datum->gain = datum->desired_gain;
				datum->ticks = 0;
			}
		}
	}

	return;
}

real sound_class_get_gain(
	short class_index)
{
	return sound_class_datum_get(class_index)->gain;
}

void sound_class_set_gain(
	char const *substring,
	real gain,
	short ticks)
{
	short class_index;

	for (class_index = 0; class_index<NUMBER_OF_SOUND_CLASSES; class_index++)
	{
		if (sound_class_names[class_index][0] && strstr(sound_class_names[class_index], substring))
		{
			struct sound_class_datum *datum = sound_class_datum_get(class_index);

			datum->desired_gain = PIN(gain, 0.f, 1.f);
			datum->ticks = FLOOR(ticks, 0);
		}
	}

	return;
}
