/*
RECORDED_ANIMATION_PLAYBACK.C
*/

/* ---------- headers */

#include "cseries.h"
#include "recorded_animation_playback.h"
#include "recorded_animations.h"
#include "recorded_animation_definitions.h"

/* ---------- constants */

#define CONTROLLER_ANGLE_STEPS 1000 /* fake name */

/* ---------- macros */

// each simple event copies one field of the same name into the unit control data
#define APPLY_EVENT(line, field) /* fake name */ \
static void apply_##field(struct animation_playback_controller *animation_state, struct unit_control_data *control, struct animation_event_header const *header, char const **playback_stream) \
{ \
	struct field##_event_data const *event_data = (struct field##_event_data const *)*playback_stream; \
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", line, control); \
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", line, header); \
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", line, header->event_type==_playback_##field##_set); \
	control->field = event_data->field; \
	*playback_stream += sizeof(struct field##_event_data); \
}

/* ---------- private code */

APPLY_EVENT(25, animation_state)
APPLY_EVENT(26, aiming_speed)
APPLY_EVENT(27, control_flags)
APPLY_EVENT(28, weapon_index)

static void apply_throttle(
	struct animation_playback_controller *animation_state,
	struct unit_control_data *control,
	struct animation_event_header const *header,
	char const **playback_stream)
{
	struct throttle_event_data const *event_data = (struct throttle_event_data const *)*playback_stream;

	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 33, control);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 35, header);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 36, header->event_type==_playback_throttle_set);

	control->throttle.i = event_data->throttle.i;
	control->throttle.j = event_data->throttle.j;
	control->throttle.k = 0.f;
	*playback_stream += sizeof(struct throttle_event_data);

	return;
}

static void update_controller_char(
	struct vector_char_difference_data const *event_data,
	struct direction_playback_controller *control)
{
	control->yaw += event_data->delta_yaw;
	if (control->yaw > CONTROLLER_ANGLE_STEPS)
	{
		control->yaw -= CONTROLLER_ANGLE_STEPS;
	}
	else if (control->yaw < -CONTROLLER_ANGLE_STEPS)
	{
		control->yaw += CONTROLLER_ANGLE_STEPS;
	}
	control->pitch += event_data->delta_pitch;

	return;
}

static void update_controller_short(
	struct vector_short_difference_data const *event_data,
	struct direction_playback_controller *control)
{
	control->yaw += event_data->delta_yaw;
	if (control->yaw > CONTROLLER_ANGLE_STEPS)
	{
		control->yaw -= CONTROLLER_ANGLE_STEPS;
	}
	else if (control->yaw < -CONTROLLER_ANGLE_STEPS)
	{
		control->yaw += CONTROLLER_ANGLE_STEPS;
	}
	control->pitch += event_data->delta_pitch;

	return;
}

static void uncompress_vector_from_controller(
	real_vector3d *vector,
	struct direction_playback_controller const *controller)
{
	real_euler_angles2d t;

	t.yaw = controller->yaw * (_pi / CONTROLLER_ANGLE_STEPS);
	t.pitch = controller->pitch * (_pi / CONTROLLER_ANGLE_STEPS);
	vector3d_from_euler_angles2d(vector, &t);

	return;
}

static void apply_vector_char_difference(
	struct animation_playback_controller *animation_state,
	struct unit_control_data *control,
	struct animation_event_header const *header,
	char const **playback_stream)
{
	struct vector_char_difference_data const *event_data = (struct vector_char_difference_data const *)*playback_stream;
	short vector_flags;

	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 100, control);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 102, header);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 103, header->event_type>=_playback_vector_char_difference_set&&header->event_type-_playback_vector_char_difference_set<FLAG(NUMBER_OF_CONTROL_VECTORS));

	vector_flags = header->event_type - _playback_vector_char_difference_set;
	if (vector_flags & FLAG(_control_vector_facing_bit))
	{
		update_controller_char(event_data, &animation_state->facing_control);
		uncompress_vector_from_controller(&control->facing_vector, &animation_state->facing_control);
	}
	if (vector_flags & FLAG(_control_vector_aiming_bit))
	{
		if (vector_flags & FLAG(_control_vector_facing_bit))
		{
			animation_state->aiming_control = animation_state->facing_control;
			control->aiming_vector = control->facing_vector;
		}
		else
		{
			update_controller_char(event_data, &animation_state->aiming_control);
			uncompress_vector_from_controller(&control->aiming_vector, &animation_state->aiming_control);
		}
	}
	if (vector_flags & FLAG(_control_vector_looking_bit))
	{
		if (vector_flags & FLAG(_control_vector_facing_bit))
		{
			animation_state->looking_control = animation_state->facing_control;
			control->looking_vector = control->facing_vector;
		}
		else if (vector_flags & FLAG(_control_vector_aiming_bit))
		{
			animation_state->looking_control = animation_state->aiming_control;
			control->looking_vector = control->aiming_vector;
		}
		else
		{
			update_controller_char(event_data, &animation_state->looking_control);
			uncompress_vector_from_controller(&control->looking_vector, &animation_state->looking_control);
		}
	}
	*playback_stream += sizeof(struct vector_char_difference_data);

	return;
}

static void apply_vector_short_difference(
	struct animation_playback_controller *animation_state,
	struct unit_control_data *control,
	struct animation_event_header const *header,
	char const **playback_stream)
{
	struct vector_short_difference_data const *event_data = (struct vector_short_difference_data const *)*playback_stream;
	short vector_flags;

	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 160, control);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 162, header);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 163, header->event_type>=_playback_vector_short_difference_set&&header->event_type-_playback_vector_short_difference_set<FLAG(NUMBER_OF_CONTROL_VECTORS));

	vector_flags = header->event_type - _playback_vector_short_difference_set;
	if (vector_flags & FLAG(_control_vector_facing_bit))
	{
		update_controller_short(event_data, &animation_state->facing_control);
		uncompress_vector_from_controller(&control->facing_vector, &animation_state->facing_control);
	}
	if (vector_flags & FLAG(_control_vector_aiming_bit))
	{
		if (vector_flags & FLAG(_control_vector_facing_bit))
		{
			animation_state->aiming_control = animation_state->facing_control;
			control->aiming_vector = control->facing_vector;
		}
		else
		{
			update_controller_short(event_data, &animation_state->aiming_control);
			uncompress_vector_from_controller(&control->aiming_vector, &animation_state->aiming_control);
		}
	}
	if (vector_flags & FLAG(_control_vector_looking_bit))
	{
		if (vector_flags & FLAG(_control_vector_facing_bit))
		{
			animation_state->looking_control = animation_state->facing_control;
			control->looking_vector = control->facing_vector;
		}
		else if (vector_flags & FLAG(_control_vector_aiming_bit))
		{
			animation_state->looking_control = animation_state->aiming_control;
			control->looking_vector = control->aiming_vector;
		}
		else
		{
			update_controller_short(event_data, &animation_state->looking_control);
			uncompress_vector_from_controller(&control->looking_vector, &animation_state->looking_control);
		}
	}
	*playback_stream += sizeof(struct vector_short_difference_data);

	return;
}

/* ---------- globals */

static void (*apply_funcs[])(struct animation_playback_controller *animation_state, struct unit_control_data *control, struct animation_event_header const *header, char const **playback_stream) =
{
	NULL, // _playback_nothing
	NULL, // _playback_end
	apply_animation_state,
	apply_aiming_speed,
	apply_control_flags,
	apply_weapon_index,
	apply_throttle,
	apply_vector_char_difference,
	apply_vector_char_difference,
	apply_vector_char_difference,
	apply_vector_char_difference,
	apply_vector_char_difference,
	apply_vector_char_difference,
	apply_vector_char_difference,
	apply_vector_char_difference,
	apply_vector_short_difference,
	apply_vector_short_difference,
	apply_vector_short_difference,
	apply_vector_short_difference,
	apply_vector_short_difference,
	apply_vector_short_difference,
	apply_vector_short_difference,
	apply_vector_short_difference
};

static byte_swap_code animation_state_event_data_bs_codes[] =
{
	_1byte
};

static struct byte_swap_definition animation_state_event_data_bs_definition =
{
	"animation_state_event_data",
	sizeof(struct animation_state_event_data),
	animation_state_event_data_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code aiming_speed_event_data_bs_codes[] =
{
	_1byte
};

static struct byte_swap_definition aiming_speed_event_data_bs_definition =
{
	"aiming_speed_event_data",
	sizeof(struct aiming_speed_event_data),
	aiming_speed_event_data_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code control_flags_event_data_bs_codes[] =
{
	_2byte
};

static struct byte_swap_definition control_flags_event_data_bs_definition =
{
	"control_flags_event_data",
	sizeof(struct control_flags_event_data),
	control_flags_event_data_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code weapon_index_event_data_bs_codes[] =
{
	_2byte
};

static struct byte_swap_definition weapon_index_event_data_bs_definition =
{
	"weapon_index_event_data",
	sizeof(struct weapon_index_event_data),
	weapon_index_event_data_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code throttle_event_data_bs_codes[] =
{
	_4byte,
	_4byte
};

static struct byte_swap_definition throttle_event_data_bs_definition =
{
	"throttle_event_data",
	sizeof(struct throttle_event_data),
	throttle_event_data_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code vector_char_difference_data_bs_codes[] =
{
	_1byte,
	_1byte
};

static struct byte_swap_definition vector_char_difference_data_bs_definition =
{
	"vector_char_difference_data",
	sizeof(struct vector_char_difference_data),
	vector_char_difference_data_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

static byte_swap_code vector_short_difference_data_bs_codes[] =
{
	_2byte,
	_2byte
};

static struct byte_swap_definition vector_short_difference_data_bs_definition =
{
	"vector_short_difference_data",
	sizeof(struct vector_short_difference_data),
	vector_short_difference_data_bs_codes,
	BYTE_SWAP_DEFINITION_SIGNATURE,
	FALSE
};

/* ---------- public code */

void recorded_animation_initialize_event_stream(
	struct animation_playback_controller *animation_state,
	struct unit_control_data *control,
	char const **playback_stream,
	byte unit_control_version)
{
	recorded_animation_initialize_unit_control(control, playback_stream, unit_control_version);
	*animation_state = *(struct animation_playback_controller const *)*playback_stream;
	*playback_stream += sizeof(struct animation_playback_controller);

	return;
}

void recorded_animation_initialize_event_stream_with_size(
	struct animation_playback_controller *animation_state,
	struct unit_control_data *control,
	char const **playback_stream)
{
	*control = *(struct unit_control_data const *)*playback_stream;
	*playback_stream += sizeof(struct unit_control_data);
	*animation_state = *(struct animation_playback_controller const *)*playback_stream;
	*playback_stream += sizeof(struct animation_playback_controller);

	return;
}

boolean recorded_animation_apply_event_stream(
	struct animation_playback_controller *animation_state,
	struct unit_control_data *control,
	long *ticks,
	char const **playback_stream)
{
	struct animation_event_header const *header;
	word time_delta;
	boolean result = TRUE;

	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 275, control);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 276, ticks);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 277, playback_stream);
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 278, *playback_stream);

	while (TRUE)
	{
		word header_size = 0;

		header = (struct animation_event_header const *)*playback_stream;
		switch (header->event_time_type)
		{
		case _time_delta_zero:
			time_delta = 0;
			header_size = sizeof(struct animation_event_header);
			break;
		case _time_delta_one:
			time_delta = 1;
			header_size = sizeof(struct animation_event_header);
			break;
		case _time_delta_byte:
			time_delta = *(byte const *)(header + 1);
			header_size = sizeof(struct animation_event_header) + sizeof(byte);
			match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 301, time_delta>1&&time_delta<=UNSIGNED_CHAR_MAX);
			break;
		case _time_delta_word:
			time_delta = *(word const *)(header + 1);
			header_size = sizeof(struct animation_event_header) + sizeof(word);
			match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 306, time_delta>UNSIGNED_CHAR_MAX);
			break;
		default:
			match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 309, !"unreachable");
		}

		if (*ticks < time_delta || header->event_type == _playback_end)
		{
			break;
		}

		*playback_stream += header_size;
		match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animation_playback.c", 315, header->event_type<NUMBEROF(apply_funcs));
		if (apply_funcs[header->event_type])
		{
			apply_funcs[header->event_type](animation_state, control, header, playback_stream);
		}
		*ticks -= time_delta;
	}

	if (header->event_type == _playback_end && *ticks == time_delta)
	{
		result = FALSE;
	}

	return result;
}

void byte_swap_recording_stream(
	void *data,
	long size,
	byte unit_control_version)
{
	return;
}
