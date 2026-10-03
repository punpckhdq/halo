/*
RECORDED_ANIMATION_PLAYBACK_V1.C
*/

/* ---------- headers */

#include "cseries.h"
#include "recorded_animation_playback_v1.h"
#include "recorded_animations.h"
#include "recorded_animation_definitions.h"

/* ---------- constants */

/* ---------- macros */

// each simple event copies one field of the same name into the unit control data
#define APPLY_EVENT(line, field) /* fake name */ \
static void apply_##field(struct unit_control_data *control, struct animation_event_v1 const *anim_event_v1, char const **playback_stream) \
{ \
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", line, control); \
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", line, anim_event_v1); \
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", line, anim_event_v1->type==_playback_v1_##field##_set); \
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", line, playback_stream); \
	control->field = ((struct field##_set_event_v1 const *)anim_event_v1)->field; \
	*playback_stream += sizeof(struct field##_set_event_v1); \
}

/* ---------- prototypes */

/* ---------- private code */

APPLY_EVENT(25, animation_state)
APPLY_EVENT(26, aiming_speed)
APPLY_EVENT(27, control_flags)
APPLY_EVENT(28, weapon_index)

static void apply_throttle(
	struct unit_control_data *control,
	struct animation_event_v1 const *anim_event_v1,
	char const **playback_stream)
{
	struct throttle_set_event_v1 const *event = (struct throttle_set_event_v1 const *)anim_event_v1;

	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 33, control);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 34, anim_event_v1);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 35, anim_event_v1->type==_playback_v1_throttle_set);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 36, playback_stream);

	control->throttle.i = event->throttle.i;
	control->throttle.j = event->throttle.j;
	control->throttle.k = 0.f;
	*playback_stream += sizeof(struct throttle_set_event_v1);

	return;
}

APPLY_EVENT(44, facing_vector)
APPLY_EVENT(45, aiming_vector)
APPLY_EVENT(46, looking_vector)

static void apply_angle_vector(
	struct unit_control_data *control,
	struct animation_event_v1 const *anim_event_v1,
	char const **playback_stream)
{
	struct angle_vector_set_event_v1 const *event = (struct angle_vector_set_event_v1 const *)anim_event_v1;
	real_vector3d vector;

	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 56, control);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 57, anim_event_v1);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 58, anim_event_v1->type>=_playback_v1_facing_angles_set && anim_event_v1->type<=_playback_v1_facing_aiming_looking_angles_set);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 59, playback_stream);

	vector3d_from_euler_angles2d(&vector, &event->angles);
	if (anim_event_v1->type != _playback_v1_aiming_looking_angles_set)
	{
		control->facing_vector = vector;
	}
	if (anim_event_v1->type != _playback_v1_facing_looking_angles_set)
	{
		control->aiming_vector = vector;
	}
	if (anim_event_v1->type != _playback_v1_facing_aiming_angles_set)
	{
		control->looking_vector = vector;
	}
	*playback_stream += sizeof(struct angle_vector_set_event_v1);

	return;
}

static void apply_multi_vector(
	struct unit_control_data *control,
	struct animation_event_v1 const *anim_event_v1,
	char const **playback_stream)
{
	struct multi_vector_set_event_v1 const *event = (struct multi_vector_set_event_v1 const *)anim_event_v1;

	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 86, control);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 87, anim_event_v1);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 88, anim_event_v1->type>=_playback_v1_facing_aiming_vector_set&&anim_event_v1->type<=_playback_v1_facing_aiming_looking_vector_set);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 89, playback_stream);

	if (anim_event_v1->type != _playback_v1_aiming_looking_vector_set)
	{
		control->facing_vector = event->vector;
	}
	if (anim_event_v1->type != _playback_v1_facing_looking_vector_set)
	{
		control->aiming_vector = event->vector;
	}
	if (anim_event_v1->type != _playback_v1_facing_aiming_vector_set)
	{
		control->looking_vector = event->vector;
	}
	*playback_stream += sizeof(struct multi_vector_set_event_v1);

	return;
}

/* ---------- globals */

static void (*apply_funcs[NUMBER_OF_PLAYBACK_V1_EVENTS])(struct unit_control_data *control, struct animation_event_v1 const *anim_event_v1, char const **playback_stream) =
{
	NULL, // _playback_v1_nothing
	NULL, // _playback_v1_end
	apply_animation_state,
	apply_aiming_speed,
	apply_control_flags,
	apply_weapon_index,
	apply_throttle,
	NULL, // _playback_v1_vectors_synchronize
	NULL, // _playback_v1_vectors_desynchronize
	apply_facing_vector,
	apply_aiming_vector,
	apply_looking_vector,
	apply_multi_vector, // _playback_v1_facing_aiming_vector_set
	apply_multi_vector, // _playback_v1_facing_looking_vector_set
	apply_multi_vector, // _playback_v1_aiming_looking_vector_set
	apply_multi_vector, // _playback_v1_facing_aiming_looking_vector_set
	apply_angle_vector, // _playback_v1_facing_angles_set
	apply_angle_vector, // _playback_v1_aiming_angles_set
	apply_angle_vector, // _playback_v1_looking_angles_set
	apply_angle_vector, // _playback_v1_facing_aiming_angles_set
	apply_angle_vector, // _playback_v1_facing_looking_angles_set
	apply_angle_vector, // _playback_v1_aiming_looking_angles_set
	apply_angle_vector // _playback_v1_facing_aiming_looking_angles_set
};

static byte_swap_code animation_event_v1_bs_codes[] =
{
	_2byte,
	_2byte
};

static struct byte_swap_definition animation_event_v1_bs_definition =
{
	"animation_event_v1",
	sizeof(struct animation_event_v1),
	animation_event_v1_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code animation_state_set_event_v1_bs_codes[] =
{
	_1byte
};

static struct byte_swap_definition animation_state_set_event_v1_bs_definition =
{
	"animation_state_set_event_v1",
	sizeof(struct animation_state_set_event_v1),
	animation_state_set_event_v1_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code aiming_speed_set_event_v1_bs_codes[] =
{
	_1byte
};

static struct byte_swap_definition aiming_speed_set_event_v1_bs_definition =
{
	"aiming_speed_set_event_v1",
	sizeof(struct aiming_speed_set_event_v1),
	aiming_speed_set_event_v1_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code control_flags_set_event_v1_bs_codes[] =
{
	_2byte
};

static struct byte_swap_definition control_flags_set_event_v1_bs_definition =
{
	"control_flags_set_event_v1",
	sizeof(struct control_flags_set_event_v1),
	control_flags_set_event_v1_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code weapon_index_set_event_v1_bs_codes[] =
{
	_2byte
};

static struct byte_swap_definition weapon_index_set_event_v1_bs_definition =
{
	"weapon_index_set_event_v1",
	sizeof(struct weapon_index_set_event_v1),
	weapon_index_set_event_v1_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code throttle_set_event_v1_bs_codes[] =
{
	_4byte,
	_4byte
};

static struct byte_swap_definition throttle_set_event_v1_bs_definition =
{
	"throttle_set_event_v1",
	sizeof(struct throttle_set_event_v1),
	throttle_set_event_v1_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code multi_vector_set_event_v1_bs_codes[] =
{
	_4byte,
	_4byte,
	_4byte
};

static struct byte_swap_definition multi_vector_set_event_v1_bs_definition =
{
	"multi_vector_set_event_v1",
	sizeof(struct multi_vector_set_event_v1),
	multi_vector_set_event_v1_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code angle_vector_set_event_v1_bs_codes[] =
{
	_4byte,
	_4byte
};

static struct byte_swap_definition angle_vector_set_event_v1_bs_definition =
{
	"angle_vector_set_event_v1",
	sizeof(struct angle_vector_set_event_v1),
	angle_vector_set_event_v1_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

/* ---------- public code */

void recorded_animation_initialize_event_stream_v1(
	struct animation_playback_controller *animation_state,
	struct unit_control_data *control,
	char const **playback_stream,
	byte unit_control_version)
{
	recorded_animation_initialize_unit_control(control, playback_stream, unit_control_version);

	return;
}

boolean recorded_animation_apply_event_stream_v1(
	struct animation_playback_controller *animation_state,
	struct unit_control_data *control,
	long *ticks,
	char const **playback_stream)
{
	struct animation_event_v1 const *anim_event_v1;
	boolean result = TRUE;

	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 162, control);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 163, ticks);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 164, playback_stream);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback_v1.c", 165, *playback_stream);

	anim_event_v1 = (struct animation_event_v1 const *)*playback_stream;
	while (*ticks >= anim_event_v1->time_delta && anim_event_v1->type != _playback_v1_end)
	{
		void (*apply_func)(struct unit_control_data *control, struct animation_event_v1 const *anim_event_v1, char const **playback_stream) = apply_funcs[anim_event_v1->type];

		if (apply_func)
		{
			apply_func(control, anim_event_v1, playback_stream);
		}
		else
		{
			*playback_stream += sizeof(struct animation_event_v1);
		}
		*ticks -= anim_event_v1->time_delta;
		anim_event_v1 = (struct animation_event_v1 const *)*playback_stream;
	}

	if (anim_event_v1->type == _playback_v1_end && *ticks == anim_event_v1->time_delta)
	{
		result = FALSE;
	}

	return result;
}

void byte_swap_recording_stream_v1(
	void *data,
	long size,
	byte unit_control_version)
{
	return;
}
