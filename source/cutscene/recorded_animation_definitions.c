/*
RECORDED_ANIMATION_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "recorded_animation_definitions.h"
#include "recorded_animation_playback.h"
#include "recorded_animation_playback_v1.h"
#include "scenario_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void byte_swap_recording(void *recording, void *data, long size);

/* ---------- globals */

static struct tag_field recorded_animation_block_fields[10];

struct tag_data_definition recorded_animation_event_stream_data =
{
	"recorded_animation_event_stream_data",
	0,
	MAXIMUM_RECORDED_ANIMATION_DATA_SIZE,
	byte_swap_recording
};

struct tag_block_definition recorded_animation_block =
{
	"recorded_animation_block",
	0,
	MAXIMUM_RECORDED_ANIMATIONS_PER_MAP,
	sizeof(struct recorded_animation_definition),
	NULL,
	recorded_animation_block_fields
};

static struct tag_field recorded_animation_block_fields[10] =
{
	{ _field_string, "name^" },
	{ _field_char_integer, "version*" },
	{ _field_char_integer, "raw animation data*" },
	{ _field_char_integer, "unit control data version*" },
	{ _field_pad, NULL, (void *)1 },
	{ _field_short_integer, "length of animation*:ticks" },
	{ _field_pad, NULL, (void *)2 },
	{ _field_pad, NULL, (void *)4 },
	{ _field_data, "recorded animation event stream*", &recorded_animation_event_stream_data },
	{ _field_terminator }
};

/* ---------- private code */

static void byte_swap_recording(
	void *recording,
	void *data,
	long size)
{
	struct recorded_animation_definition *animation = recording;

	switch (animation->version)
	{
	case 1:
	case 2:
	case 3:
		byte_swap_recording_stream_v1(data, size, animation->unit_control_data_version);
		break;
	case RECORDED_ANIMATION_VERSION:
		byte_swap_recording_stream(data, size, animation->unit_control_data_version);
		break;
	}

	return;
}

/* ---------- public code */

short scenario_get_animation_by_name(
	struct scenario *scenario,
	char const *animation_name)
{
	short result = NONE;
	short animation_index;

	for (animation_index = 0; animation_index < scenario->recorded_animations.count; animation_index++)
	{
		struct recorded_animation_definition *animation = TAG_BLOCK_GET_ELEMENT(&scenario->recorded_animations, animation_index, struct recorded_animation_definition);

		if (!_stricmp(animation->name, animation_name))
		{
			result = animation_index;
			break;
		}
	}

	return result;
}
