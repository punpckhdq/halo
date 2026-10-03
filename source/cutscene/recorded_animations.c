/*
RECORDED_ANIMATIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "recorded_animations.h"
#include "recorded_animation_definitions.h"
#include "network_game_globals.h"
#include "players.h"
#include "game_state.h"
#include "vehicles.h"
#include "editor_stubs.h"
#include "hs_library_external.h"
#include "recorded_animation_playback.h"
#include "recorded_animation_playback_v1.h"
#include "draw_string.h"
#include "render_debug.h"

/* ---------- constants */

enum
{
	_recording_thread_finished_bit = 0,
	_recording_thread_killed_bit,
	_recording_thread_unit_was_controlled_bit,
	_recording_thread_delete_unit_on_complete_bit,
	_recording_thread_hover_vehicle_on_complete_bit,
	NUMBER_OF_RECORDING_THREAD_FLAGS
};

enum
{
	MAX_RECORDINGS_PLAYING = 64
};

enum
{
	DEBUG_RECORDING_STRING_SIZE = 10*1024, /* fake name */
	DEBUG_RECORDING_DISPLAY_LENGTH = 1024 /* fake name */
};

/* ---------- macros */

/* ---------- structures */

typedef void (*initialize_event_stream_proc)(struct animation_playback_controller *animation_state, struct unit_control_data *control, char const **playback_stream, byte unit_control_version); /* fake name */
typedef boolean (*apply_event_stream_proc)(struct animation_playback_controller *animation_state, struct unit_control_data *control, long *ticks, char const **playback_stream); /* fake name */

struct animation_playback
{
	initialize_event_stream_proc initialize_event_stream;
	apply_event_stream_proc apply_event_stream;
};

struct animation_thread
{
	short identifier;
	long unit_index;
	word ticks_left;
	word flags;
	long relative_ticks;
	char const *event_stream;
	struct unit_control_data controller;
	struct animation_playback_controller animation_state;
	short version;
};

struct animation_thread_debug /* fake name */
{
	boolean valid; /* fake name */
	char const *event_stream_start;
	long stream_length;
	short animation_index; /* fake name */
};

/* ---------- prototypes */

static boolean recorded_animation_play_internal(long unit_index, short animation_index, word extra_flags);
static struct animation_thread *get_controlling_thread(long unit_index, long *thread_index_reference);

/* ---------- globals */

static struct animation_playback current_playback =
{
	recorded_animation_initialize_event_stream,
	recorded_animation_apply_event_stream
};

static struct animation_playback v1_playback =
{
	recorded_animation_initialize_event_stream_v1,
	recorded_animation_apply_event_stream_v1
};

static struct animation_playback *playback_codec[RECORDED_ANIMATION_VERSION] =
{
	&v1_playback,
	&v1_playback,
	&v1_playback,
	&current_playback
};

static struct data_array *animation_threads = NULL;

boolean debug_recording = FALSE;
short debug_recording_newlines = 10;

static struct animation_thread_debug *animation_threads_debug = NULL;

/* ---------- public code */

void recorded_animations_initialize(
	void)
{
	animation_threads = game_state_data_new("recorded animations", MAX_RECORDINGS_PLAYING, sizeof(struct animation_thread));
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animations.c", 108, animation_threads);

	animation_threads_debug = match_malloc("c:\\halo\\SOURCE\\cutscene\\recorded_animations.c", 111, MAX_RECORDINGS_PLAYING*sizeof(struct animation_thread_debug));
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animations.c", 112, animation_threads_debug);

	return;
}

void recorded_animations_dispose(
	void)
{
	if (animation_threads_debug)
	{
		match_free("c:\\halo\\SOURCE\\cutscene\\recorded_animations.c", 123, animation_threads_debug);
		animation_threads_debug = NULL;
	}

	return;
}

void recorded_animations_initialize_for_new_map(
	void)
{
	data_make_valid(animation_threads);
	recorded_animations_clear_debug_storage();

	return;
}

void recorded_animations_dispose_from_old_map(
	void)
{
	data_make_invalid(animation_threads);

	return;
}

void recorded_animations_clear_debug_storage(
	void)
{
	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animations.c", 153, animation_threads_debug);
	memset(animation_threads_debug, 0, MAX_RECORDINGS_PLAYING*sizeof(struct animation_thread_debug));

	return;
}

void recorded_animation_kill(
	long unit_index)
{
	struct animation_thread *thread = get_controlling_thread(unit_index, NULL);

	if (thread)
	{
		thread->flags |= FLAG(_recording_thread_finished_bit) | FLAG(_recording_thread_killed_bit);
	}

	return;
}

static boolean recorded_animation_play_internal(
	long unit_index,
	short animation_index,
	word extra_flags)
{
	boolean result = FALSE;

	if (unit_index != NONE)
	{
		if (animation_index != NONE && animation_index < global_scenario_get()->recorded_animations.count)
		{
			struct animation_thread *thread;
			struct recorded_animation_definition *animation;
			long thread_index;

			unit_get(unit_index);
			player_index_from_unit_index(unit_index);
			thread = get_controlling_thread(unit_index, &thread_index);
			animation = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->recorded_animations, animation_index, struct recorded_animation_definition);

			if (!recorded_animation_controlling_unit(unit_index))
			{
				struct animation_thread_debug *thread_debug;

				if (!thread)
				{
					thread_index = datum_new(animation_threads);
					if (thread_index != NONE)
					{
						thread = datum_get(animation_threads, thread_index);
					}
				}

				if (thread)
				{
					match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animations.c", 233, animation->version>0&&animation->version<=RECORDED_ANIMATION_VERSION&&playback_codec[animation->version-1]);
					thread->unit_index = unit_index;
					thread->relative_ticks = 0;
					thread->ticks_left = animation->ticks;
					thread->event_stream = tag_data_get_pointer(&animation->animation_data, 0, animation->animation_data.size);

					thread_debug = &animation_threads_debug[DATUM_INDEX_TO_ABSOLUTE_INDEX(thread_index)];
					thread_debug->valid = TRUE;
					thread_debug->event_stream_start = thread->event_stream;
					thread_debug->stream_length = animation->animation_data.size;
					thread_debug->animation_index = animation_index;

					thread->version = animation->version - 1;
					thread->flags &= ~FLAG(_recording_thread_finished_bit);
					playback_codec[thread->version]->initialize_event_stream(&thread->animation_state, &thread->controller, &thread->event_stream, animation->unit_control_data_version);

					unit_set_actively_controlled(unit_index, TRUE);
					SET_FLAG(thread->flags, _recording_thread_unit_was_controlled_bit, unit_controllable(unit_index));
					unit_set_controllable(unit_index, FALSE);
					unit_set_possessed(unit_index, TRUE);
					object_set_automatic_deactivation(unit_index, FALSE);
					thread->flags |= extra_flags;
					result = TRUE;
				}
				else
				{
					error(_error_silent, "Could not allocate space for a new animation");
				}
			}
			else if (thread)
			{
				struct animation_thread_debug *thread_debug = &animation_threads_debug[DATUM_INDEX_TO_ABSOLUTE_INDEX(thread_index)];
				char const *playing_name = "<unknown>";

				if (thread_debug->valid)
				{
					playing_name = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->recorded_animations, thread_debug->animation_index, struct recorded_animation_definition)->name;
				}
				error(_error_silent, "trying to play %s while %s is playing", animation->name, playing_name);
			}
			else
			{
				error(_error_silent, "can't play animation on unit");
			}
		}
		else
		{
			error(_error_silent, "this animation doesn't exist");
		}
	}
	else
	{
		error(_error_silent, "unit doesn't exist");
	}

	return result;
}

boolean recorded_animation_play(
	long unit_index,
	short animation_index)
{
	return recorded_animation_play_internal(unit_index, animation_index, 0);
}

boolean recorded_animation_play_and_delete(
	long unit_index,
	short animation_index)
{
	return recorded_animation_play_internal(unit_index, animation_index, FLAG(_recording_thread_delete_unit_on_complete_bit));
}

boolean recorded_animation_play_and_hover(
	long vehicle_index,
	short animation_index)
{
	return recorded_animation_play_internal(vehicle_index, animation_index, FLAG(_recording_thread_hover_vehicle_on_complete_bit));
}

long recorded_animation_get_time_left(
	long unit_index)
{
	long ticks_left = 0;
	struct animation_thread *thread = get_controlling_thread(unit_index, NULL);

	match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animations.c", 312, !thread||thread->unit_index==unit_index);
	if (thread && thread->unit_index == unit_index)
	{
		ticks_left = thread->ticks_left;
	}

	return ticks_left;
}

void recorded_animations_update(
	void)
{
	struct data_iterator iterator;
	struct animation_thread *thread;

	data_iterator_new(&iterator, animation_threads);
	while (thread = data_iterator_next(&iterator))
	{
		if (unit_try_and_get(thread->unit_index))
		{
			struct animation_thread_debug *thread_debug;

			if (!TEST_FLAG(thread->flags, _recording_thread_finished_bit))
			{
				boolean finished;

				thread->ticks_left--;
				finished = !playback_codec[thread->version]->apply_event_stream(&thread->animation_state, &thread->controller, &thread->relative_ticks, &thread->event_stream);
				match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animations.c", 347, thread->relative_ticks>=0);
				thread_debug = &animation_threads_debug[DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index)];
				if (thread_debug->valid)
				{
					match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animations.c", 354, thread->event_stream-thread_debug->event_stream_start<thread_debug->stream_length||(thread->event_stream-thread_debug->event_stream_start==thread_debug->stream_length&&finished));
				}
				thread->relative_ticks++;
				unit_control(thread->unit_index, &thread->controller);
				SET_FLAG(thread->flags, _recording_thread_finished_bit, finished);
			}
			else
			{
				thread_debug = &animation_threads_debug[DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index)];
				match_vassert(
					"c:\\halo\\SOURCE\\cutscene\\recorded_animations.c",
					373,
					!thread_debug->valid || TEST_FLAG(thread->flags, _recording_thread_killed_bit) || thread->ticks_left==0,
					csprintf(
						temporary,
						"animation %s appears corrupt",
						TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->recorded_animations, thread_debug->animation_index, struct recorded_animation_definition)->name
					)
				);
				thread_debug->valid = FALSE;

				unit_set_controllable(thread->unit_index, TEST_FLAG(thread->flags, _recording_thread_unit_was_controlled_bit));
				unit_set_possessed(thread->unit_index, FALSE);
				unit_set_actively_controlled(thread->unit_index, FALSE);
				object_set_automatic_deactivation(thread->unit_index, TRUE);

				if (TEST_FLAG(thread->flags, _recording_thread_delete_unit_on_complete_bit))
				{
					hs_object_destroy(thread->unit_index);
				}

				if (TEST_FLAG(thread->flags, _recording_thread_hover_vehicle_on_complete_bit))
				{
					vehicle_hover(thread->unit_index, TRUE);
				}

				datum_delete(animation_threads, iterator.index);
			}
		}
		else
		{
			datum_delete(animation_threads, iterator.index);
		}
	}

	return;
}

void recorded_animation_verify(
	struct recorded_animation_definition const *recording)
{
	struct animation_playback_controller animation_state;
	struct unit_control_data controller;
	char const *stream = recording->animation_data.address;
	char const *playback_stream = stream;
	long size = recording->animation_data.size;
	long ticks_left = recording->ticks;
	long relative_ticks = 0;
	boolean finished;

	playback_codec[recording->version-1]->initialize_event_stream(&animation_state, &controller, &playback_stream, recording->unit_control_data_version);
	do
	{
		ticks_left--;
		finished = !playback_codec[recording->version-1]->apply_event_stream(&animation_state, &controller, &relative_ticks, &playback_stream);
		match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animations.c", 428, ticks_left>=0);
		match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animations.c", 429, relative_ticks>=0);
		match_assert("c:\\halo\\SOURCE\\cutscene\\recorded_animations.c", 430, playback_stream-stream<size||(playback_stream-stream==size&&finished));
		relative_ticks++;
	}
	while (!finished);

	return;
}

boolean recorded_animation_controlling_unit(
	long unit_index)
{
	struct data_iterator iterator;
	struct animation_thread *thread;
	boolean result = FALSE;

	data_iterator_new(&iterator, animation_threads);
	while (thread = data_iterator_next(&iterator))
	{
		if (thread->unit_index == unit_index && !TEST_FLAG(thread->flags, _recording_thread_finished_bit))
		{
			result = TRUE;
			break;
		}
	}

	return result;
}

void render_debug_recording(
	void)
{
	if (debug_recording)
	{
		char string[DEBUG_RECORDING_STRING_SIZE];
		short tab_stops[] = { 200, 300 };
		short newline_index;
		short length = 0;
		struct data_iterator iterator;
		struct animation_thread *thread;

		for (newline_index = 0; newline_index < debug_recording_newlines; newline_index++)
		{
			length += sprintf(&string[length], "|n");
		}
		length += sprintf(&string[length], "recording name|tticks left|tobject name");

		data_iterator_new(&iterator, animation_threads);
		while (thread = data_iterator_next(&iterator))
		{
			struct object_datum *object = object_try_and_get(thread->unit_index);
			struct animation_thread_debug *thread_debug = &animation_threads_debug[DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index)];

			if (!TEST_FLAG(thread->flags, _recording_thread_finished_bit) && object && object->object.name_index != NONE)
			{
				struct scenario_object_name *object_name = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->object_names, object->object.name_index, struct scenario_object_name);
				char const *recording_name = "<unknown>";

				if (thread_debug->valid)
				{
					recording_name = TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->recorded_animations, thread_debug->animation_index, struct recorded_animation_definition)->name;
				}
				length += sprintf(&string[length], "|n%s|t", recording_name);
				length += sprintf(&string[length], "%d|t", thread->ticks_left);
				length += sprintf(&string[length], "%s", object_name->name);
			}
		}

		string[DEBUG_RECORDING_DISPLAY_LENGTH] = '\0';
		draw_string_set_tab_stops(tab_stops, NUMBEROF(tab_stops));
		render_debug_string(TRUE, string);
		draw_string_set_tab_stops(tab_stops, 0);
	}

	return;
}

/* ---------- private code */

static struct animation_thread *get_controlling_thread(
	long unit_index,
	long *thread_index_reference)
{
	struct data_iterator iterator;
	struct animation_thread *thread;
	long thread_index = NONE;

	data_iterator_new(&iterator, animation_threads);
	while (thread = data_iterator_next(&iterator))
	{
		if (thread->unit_index == unit_index)
		{
			thread_index = iterator.index;
			break;
		}
	}

	if (thread_index_reference)
	{
		*thread_index_reference = thread_index;
	}

	return thread;
}
