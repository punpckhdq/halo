/*
SOUND_MANAGER.C
*/

/* ---------- headers */

#include "cseries.h"
#include "sound_manager.h"
#include "game_sound.h"
#include "sound_definitions.h"
#include "sound_classes.h"
#include "platform_sound.h"
#include "sound_preferences.h"
#include "ima_adpcm.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "network_game_globals.h"
#include "players.h"
#include "console.h"
#include "render_debug.h"
#include "structures.h"
#include "observer.h"
#include "physics_constants.h"
#include "terminal.h"
#include "cinematics.h"
#include "player_effects.h"
#include "sound_cache.h"

/* ---------- macros */

#define sound_get(index) ((struct sound_datum *)datum_get(sound_data, (index)))
#define looping_sound_get(index) ((struct looping_sound_datum *)datum_get(looping_sound_data, (index)))
#define looping_sound_try_and_get(index) ((struct looping_sound_datum *)datum_try_and_get(looping_sound_data, (index)))

/* ---------- structures */

struct sound_datum
{
	short identifier;
	short type;
	word flags;
	short listener_index;
	long definition_index;
	long source_identifier;
	boolean (*track_proc)(long, void const *, struct sound_source *);
	struct sound_source source;
	byte track_data[MAXIMUM_SOUND_CALLBACK_DATA];
	long start_time;
	real pitch;
	short playing_channel_index;
	short pitch_range_index;
	short permutation_index;
	short fade_mode;
	short loop_track_index;
	long next_definition_index;
	real fade_interpolation_start;
	real fade_interpolation_end;
	long fade_start_time;
	long fade_stop_time;
};

struct looping_sound_detail_datum
{
	long next_play_time;
};

struct looping_sound_track_datum
{
	long primary_sound_index;
};

struct looping_sound_datum
{
	short identifier;
	word pad;
	long definition_index;
	long loop_identifier;
	struct sound_source source;
	boolean flip_flop;
	boolean alternate;
	boolean ordered_permutations_finished;
	short component_sound_count;
	short state;
	struct looping_sound_detail_datum details[MAXIMUM_DETAIL_SOUNDS_PER_LOOPING_SOUND];
	struct looping_sound_track_datum tracks[MAXIMUM_TRACKS_PER_LOOPING_SOUND];
};

struct sound_channel_summary
{
	short like_definition_count;
	short like_definition_channels[MAXIMUM_SOUND_INSTANCES_PER_DEFINITION];
	short maximum_instance_count;
	short like_source_count;
	short like_source_channels[MAXIMUM_SOUND_INSTANCES_PER_OBJECT_PER_DEFINITION];
	short maximum_source_instance_count;
};

struct loop_impulse_sound_tracking_data
{
	real_vector3d position_offset;
};

struct sound_manager_globals_t
{
	boolean initialized;
	boolean active;
	boolean paused;
	boolean idling;
	long game_time_when_no_scripted_dialog_will_be_playing;
	struct platform_sound_manager_definition *platform;
	long render_time;
	real ticks_elapsed;
	boolean flip_flop;
	struct sound_listener listeners[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	struct sound_environment sound_environment;
	real nondialog_gain;
	short channel_count;
};

/* ---------- prototypes */

static __forceinline void sound_update_time(void);
static real sound_scale_value(real base, real lower_bound_modifier, real upper_bound_modifier, real scale);
static boolean sound_definition_is_playable(long definition_index);
static struct sound_channel_datum *channel_get(short index);
static struct sound_listener *listener_get(short index);
static short sound_definition_promote(long definition_index);
static real sound_manager_master_gain(short class_index);
static void sound_delete(long sound_index);
static long sound_travel_milliseconds(real distance);
static boolean refresh_sound(long sound_index);
static void sound_channel_summary_build(struct sound_channel_summary *summary, long sound_index);
static void channel_queue_sound(short channel_index, struct sound_permutation *permutation);
static void channel_set_properties(short channel_index, struct platform_sound_channel_properties *properties, boolean gain_only);
static short channel_get_state(short channel_index);
static void channel_stop(short channel_index);
static boolean track_loop_track_sound(long looping_sound_index, void const *unused, struct sound_source *source);
static boolean track_loop_impulse_sound(long looping_sound_index, struct loop_impulse_sound_tracking_data const *track_data, struct sound_source *source);
static void sound_set_definition_begin(long sound_index, long definition_index);
static real sound_calculate_fade(long sound_index);
static long looping_sound_find(long identifier);
static real limit_pitch(real desired_pitch, real old_pitch, real maximum_bend);
static void render_debug_sound(long sound_index);
static void render_debug_looping_sound(long definition_index, struct sound_source const *source);
static real sound_scale_random_value(real base_lower_bound, real base_upper_bound, real lower_bound_modifier, real upper_bound_modifier, real scale);
static void sound_start_fade(short mode, real seconds, long fade_in_sound_index, long fade_out_sound_index);
static void sound_stop(long sound_index);
static real source_distance_squared(short listener_index, struct sound_source const *source);
static real source_distance(short listener_index, struct sound_source const *source);
static short sound_find_like_channel(long sound_index, short *channel_indices, short channel_count);
static boolean sound_preempts_sound(long challenger_sound_index, long champion_sound_index, real challenger_distance_squared);
static void update_channel_for_impulse_sound(short channel_index, real fade);
static long looping_sound_new(long definition_index, long identifier, struct sound_source const *source);
static void sound_set_definition_end(long sound_index);
static void detail_sound_random_offset(struct looping_sound_detail *detail_definition, real_vector3d *offset);
static short source_audible(struct sound_source *source, real maximum_distance);
static void refresh_sounds(void);
static short sound_find_best_channel(long sound_index);
static long looping_sound_new_sound(long looping_sound_index, long definition_index, short track_index, short type);
static void update_channel_for_looping_sound(short channel_index, real fade);
static void refresh_listener(void);
static short sound_find_channel(long sound_index);
static void update_channels(void);
static void process_looping_sounds(void);
static void prioritize_sounds(void);

/* ---------- globals */

static real const sound_pitch_range_fade_time = 0.5f;
static real const sound_inaudible_fade_out_time = 2.f;
static real const sound_inaudible_fade_back_in_time = 0.5f;
static real const sound_player_fade_out_time = 0.3f;
static real const oo_speed_of_sound = 1000.f/(340.f/METERS_PER_UNIT);
static long const speed_of_sound_threshold = 250;
static real const sound_priority_epsilon = 0.1f;

real sound_gain_under_dialog = 0.7f;
struct platform_sound_manager_definition *platform_definitions[NUMBER_OF_PLATFORM_SOUND_CODES] =
{
	&platform_sound_dsound,
	NULL
};
static struct profile_section sound_render_section = {"sound_render", NONE, TRUE};
real sound_fade_exponent = 2.5f;

static struct sound_manager_globals_t sound_manager_globals;

boolean debug_looping_sound;
boolean debug_sound;
boolean loud_dialog_hack;
struct sound_channel_datum sound_channels[MAXIMUM_SOUND_CHANNELS];
struct data_array *looping_sound_data;
struct data_array *sound_data;

/* ---------- public code */

boolean sound_valid_for_channel(
	short compression,
	short encoding,
	short sample_rate,
	short spatialization_mode,
	short channel_type_flags)
{
	boolean valid = TRUE;

	if (!TEST_FLAG(channel_type_flags, _sound_channel_compressed_bit) != !compression)
	{
		valid = FALSE;
	}

	if (!TEST_FLAG(channel_type_flags, _sound_channel_stereo_bit) != !encoding)
	{
		valid = FALSE;
	}

	if (TEST_FLAG(channel_type_flags, _sound_channel_44k_bit) != sample_rate)
	{
		valid = FALSE;
	}

	if (!TEST_FLAG(channel_type_flags, _sound_channel_stereo_bit) &&
		!TEST_FLAG(channel_type_flags, _sound_channel_3d_bit) != !spatialization_mode)
	{
		valid = FALSE;
	}

	return valid;
}

struct platform_sound_manager_definition *current_platform_definition(
	void)
{
	return sound_manager_globals.platform;
}

void sound_initialize_for_new_map(
	void)
{
	return;
}

void sound_dispose(
	void)
{
	if (sound_manager_globals.initialized)
	{
		sound_manager_globals.platform->dispose();
		data_make_invalid(sound_data);
		data_make_invalid(looping_sound_data);
		sound_manager_globals.initialized = FALSE;
	}

	if (sound_data)
	{
		data_dispose(sound_data);
	}

	if (looping_sound_data)
	{
		data_dispose(looping_sound_data);
	}

	sound_cache_delete();

	return;
}

boolean sound_is_active(
	void)
{
	return sound_manager_globals.initialized && sound_manager_globals.active;
}

void sound_pause(
	boolean paused)
{
	if (paused != sound_manager_globals.paused)
	{
		sound_manager_globals.paused = paused;
		sound_manager_globals.platform->set_pause(paused);

		if (!paused)
		{
			sound_manager_globals.render_time = system_milliseconds();
		}
	}

	return;
}

long sound_render_time(
	void)
{
	return sound_manager_globals.render_time;
}

void sound_reconnect_to_structure_bsp(
	void)
{
	if (sound_is_active())
	{
		long sound_index;

		for (sound_index = data_next_index(sound_data, NONE); sound_index != NONE; sound_index = data_next_index(sound_data, sound_index))
		{
			struct sound_datum *sound = sound_get(sound_index);

			if (sound->source.spatialization_mode == _sound_spatialization_mode_absolute)
			{
				scenario_location_from_point(&sound->source.location.game_location, &sound->source.location.position);
			}
		}
	}

	return;
}

boolean sound_try_and_get(
	long sound_index)
{
	return datum_try_and_get(sound_data, sound_index) != NULL;
}

void sound_enable(
	boolean enabled)
{
	sound_manager_globals.active = enabled;

	return;
}

boolean sound_scripted_dialog_is_playing(
	void)
{
	return game_time_get() < sound_manager_globals.game_time_when_no_scripted_dialog_will_be_playing;
}

void sound_manager_set_sound_environment(
	struct sound_environment const *environment)
{
	sound_manager_globals.sound_environment = *environment;

	return;
}

static __forceinline void sound_update_time(
	void)
{
	long time = system_milliseconds();

	sound_manager_globals.ticks_elapsed = ((real)time - sound_manager_globals.render_time) * 0.03f;
	sound_manager_globals.render_time = time;

	return;
}

static real sound_scale_value(
	real base,
	real lower_bound_modifier,
	real upper_bound_modifier,
	real scale)
{
	return base * ((upper_bound_modifier - lower_bound_modifier) * scale + lower_bound_modifier);
}

static boolean sound_definition_is_playable(
	long definition_index)
{
	struct sound_definition *definition = sound_definition_get(definition_index);

	return definition->pitch_ranges.count &&
		TAG_BLOCK_GET_ELEMENT(&definition->pitch_ranges, 0, struct sound_pitch_range)->permutations.count &&
		!sound_class_get(definition->class_index)->disabled;
}

static struct sound_channel_datum *channel_get(
	short index)
{
	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1064, index>=0 && index<sound_manager_globals.channel_count);

	return &sound_channels[index];
}

static struct sound_listener *listener_get(
	short index)
{
	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1072, index>=0 && index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);

	return &sound_manager_globals.listeners[index];
}

static short sound_definition_promote(
	long definition_index)
{
	short result = _sound_promotion_dont;
	struct sound_definition *definition = sound_definition_get(definition_index);

	if (definition->promotion_count)
	{
		definition->runtime_promotion_counter += definition->runtime_promotion_time - sound_manager_globals.render_time;
		definition->runtime_promotion_counter = FLOOR(definition->runtime_promotion_counter, 0);
		definition->runtime_promotion_time = sound_manager_globals.render_time;
		definition->runtime_promotion_counter += definition->runtime_maximum_play_time;

		if (definition->runtime_promotion_counter > definition->promotion_count*definition->runtime_maximum_play_time)
		{
			if (definition->promotion_sound.index != NONE)
			{
				definition->runtime_promotion_counter = 0;
				result = _sound_promotion_do;
			}
			else
			{
				definition->runtime_promotion_counter -= definition->runtime_maximum_play_time;
				result = _sound_promotion_dont_play;
			}
		}
	}

	return result;
}

static real sound_manager_master_gain(
	short class_index)
{
	real gain = sound_class_get_gain(class_index);

	if (class_index != _sound_class_scripted_dialog_to_player &&
		class_index != _sound_class_scripted_dialog_to_other &&
		class_index != _sound_class_scripted_dialog_force_unspatialized)
	{
		gain *= sound_manager_globals.nondialog_gain;
	}

	return gain;
}

static void sound_delete(
	long sound_index)
{
	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1231, sound_get(sound_index)->playing_channel_index==NONE);

	datum_delete(sound_data, sound_index);

	return;
}

static long sound_travel_milliseconds(
	real distance)
{
	return (long)(oo_speed_of_sound * distance);
}

static boolean refresh_sound(
	long sound_index)
{
	struct sound_datum *sound = sound_get(sound_index);
	boolean result = TRUE;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1439, sound->playing_channel_index==NONE || channel_get(sound->playing_channel_index)->sound_index==sound_index);

	if (!TEST_FLAG(sound->flags, _sound_delayed_bit) &&
		sound->track_proc &&
		sound->start_time<sound_manager_globals.render_time &&
		!sound->track_proc(sound->source_identifier, sound->track_data, &sound->source))
	{
		if (sound->type == _sound_impulse && !sound_class_get(sound_definition_get(sound->definition_index)->class_index)->speech)
		{
			sound->track_proc = NULL;
		}
		else
		{
			result = FALSE;
		}
	}

	return result;
}

static void sound_channel_summary_build(
	struct sound_channel_summary *summary,
	long sound_index)
{
	short channel_index;
	struct sound_datum *sound = sound_get(sound_index);
	struct sound_definition *definition = sound_definition_get(sound->definition_index);

	summary->like_definition_count = 0;
	summary->like_source_count = 0;
	summary->maximum_instance_count = sound_class_get(definition->class_index)->maximum_number_per_definition;
	summary->maximum_source_instance_count = sound_class_get(definition->class_index)->maximum_number_per_object;
	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1700, summary->maximum_source_instance_count<=MAXIMUM_SOUND_INSTANCES_PER_DEFINITION);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1701, summary->maximum_instance_count<=MAXIMUM_SOUND_INSTANCES_PER_OBJECT_PER_DEFINITION);

	for (channel_index = 0; channel_index<sound_manager_globals.channel_count; channel_index++)
	{
		struct sound_channel_datum *channel = channel_get(channel_index);

		if (channel->sound_index != NONE && channel->sound_index != sound_index)
		{
			struct sound_datum *channel_sound = sound_get(channel->sound_index);

			if (sound_valid_for_channel(definition->compression, definition->encoding, definition->sample_rate, sound->source.spatialization_mode, channel->type_flags) &&
				sound->definition_index == channel_sound->definition_index)
			{
				match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1717, summary->like_definition_count<summary->maximum_instance_count);
				summary->like_definition_channels[summary->like_definition_count++] = channel_index;

				if (sound->source_identifier != NONE && sound->source_identifier == channel_sound->source_identifier)
				{
					match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1723, summary->like_source_count<summary->maximum_source_instance_count);
					summary->like_source_channels[summary->like_source_count++] = channel_index;
				}
			}
		}
	}

	return;
}

static void channel_queue_sound(
	short channel_index,
	struct sound_permutation *permutation)
{
	struct sound_channel_datum *channel = channel_get(channel_index);

	if (channel->queued_permutation)
	{
		sound_cache_sound_finished(channel->queued_permutation);
	}

	sound_manager_globals.platform->queue_sound_to_channel(channel_index, permutation);

	if (channel->playing_permutation)
	{
		channel->queued_permutation = permutation;
	}
	else
	{
		channel->playing_permutation = permutation;
		channel->estimated_tick_time = 0.f;
	}

	return;
}

static void channel_set_properties(
	short channel_index,
	struct platform_sound_channel_properties *properties,
	boolean gain_only)
{
	struct sound_channel_datum *channel = channel_get(channel_index);

	if (!gain_only)
	{
		match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 2120, properties->pitch>0.f);
		channel->pitch = properties->pitch;
	}

	sound_manager_globals.platform->set_channel_properties(channel_index, properties, gain_only);

	return;
}

static short channel_get_state(
	short channel_index)
{
	struct sound_channel_datum *channel = channel_get(channel_index);
	short state = sound_manager_globals.platform->get_channel_state(channel_index);

	if (channel->queued_permutation && state<_sound_channel_full)
	{
		sound_cache_sound_finished(channel->playing_permutation);
		channel->playing_permutation = channel->queued_permutation;
		channel->queued_permutation = NULL;
		channel->estimated_tick_time = 0.f;

		if (!sound_cache_sound_loaded(channel->playing_permutation))
		{
			state = _sound_channel_idle;
		}
	}

	if (channel->playing_permutation && state<_sound_channel_playing)
	{
		match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 2152, !channel->queued_permutation);
		sound_cache_sound_finished(channel->playing_permutation);
		channel->playing_permutation = NULL;
	}

	channel->estimated_tick_time += sound_manager_globals.ticks_elapsed * channel->pitch;

	return state;
}

static void channel_stop(
	short channel_index)
{
	struct sound_channel_datum *channel = channel_get(channel_index);

	if (channel->queued_permutation)
	{
		sound_cache_sound_finished(channel->queued_permutation);
		channel->queued_permutation = NULL;
	}

	if (channel->playing_permutation)
	{
		sound_cache_sound_finished(channel->playing_permutation);
		channel->playing_permutation = NULL;
	}

	sound_manager_globals.platform->stop_channel(channel_index);

	return;
}

static boolean track_loop_track_sound(
	long looping_sound_index,
	void const *unused,
	struct sound_source *source)
{
	struct looping_sound_datum *loop = looping_sound_try_and_get(looping_sound_index);

	if (loop)
	{
		*source = loop->source;

		return TRUE;
	}

	return FALSE;
}

static boolean track_loop_impulse_sound(
	long looping_sound_index,
	struct loop_impulse_sound_tracking_data const *track_data,
	struct sound_source *source)
{
	struct looping_sound_datum *loop = looping_sound_try_and_get(looping_sound_index);
	boolean result = FALSE;

	if (loop)
	{
		source->obstruction = loop->source.obstruction;
		source->occlusion = loop->source.occlusion;

		if (loop->source.spatialization_mode != _sound_spatialization_mode_none)
		{
			source->location.translational_velocity = loop->source.location.translational_velocity;
			source->location.forward = loop->source.location.forward;
			source->location.game_location = loop->source.location.game_location;
		}
		else
		{
			source->location.forward = *global_forward3d;
			source->location.translational_velocity = *global_zero_vector3d;
		}

		*(real_vector3d *)&source->location.position = track_data->position_offset;

		if (source->spatialization_mode == _sound_spatialization_mode_absolute)
		{
			source->location.position.x += loop->source.location.position.x;
			source->location.position.y += loop->source.location.position.y;
			source->location.position.z += loop->source.location.position.z;
		}

		result = TRUE;
	}

	return result;
}

static void sound_set_definition_begin(
	long sound_index,
	long definition_index)
{
	struct sound_datum *sound = sound_get(sound_index);

	if (sound->definition_index != definition_index)
	{
		sound->next_definition_index = definition_index;
	}

	return;
}

static real sound_calculate_fade(
	long sound_index)
{
	struct sound_datum *sound = sound_get(sound_index);
	real fade = 1.f;

	if (sound->fade_start_time != sound->fade_stop_time)
	{
		real t = ((real)sound_manager_globals.render_time - sound->fade_start_time) / (sound->fade_stop_time - sound->fade_start_time);

		t = PIN(t, 0.f, 1.f);

		switch (sound->fade_mode)
		{
		case _sound_fade_mode_linear:
			break;
		case _sound_fade_mode_crossfade:
			if (sound->fade_interpolation_end > sound->fade_interpolation_start)
			{
				t = (real)pow(t, 1.f/sound_fade_exponent);
			}
			else
			{
				t = (real)(1.f - pow(1.f - t, 1.f/sound_fade_exponent));
			}
			break;
		default:
			match_halt("c:\\halo\\SOURCE\\sound\\sound_manager.c", 2678);
		}

		if (t == 1.f)
		{
			sound->fade_stop_time = 0;
			sound->fade_start_time = 0;
		}

		fade = (sound->fade_interpolation_end - sound->fade_interpolation_start) * t + sound->fade_interpolation_start;
	}

	return fade;
}

static long looping_sound_find(
	long identifier)
{
	long looping_sound_index;

	for (looping_sound_index = data_next_index(looping_sound_data, NONE);
		looping_sound_index != NONE;
		looping_sound_index = data_next_index(looping_sound_data, looping_sound_index))
	{
		if (looping_sound_get(looping_sound_index)->loop_identifier == identifier)
		{
			return looping_sound_index;
		}
	}

	return NONE;
}

static real limit_pitch(
	real desired_pitch,
	real old_pitch,
	real maximum_bend)
{
	real pitch;

	if (maximum_bend != 0.f && desired_pitch != old_pitch)
	{
		if (desired_pitch > old_pitch)
		{
			pitch = old_pitch*maximum_bend;
			pitch = MIN(desired_pitch, pitch);
		}
		else
		{
			pitch = old_pitch/maximum_bend;
			pitch = MAX(desired_pitch, pitch);
		}
	}
	else
	{
		pitch = desired_pitch;
	}

	return pitch;
}

static void render_debug_sound(
	long sound_index)
{
	if (debug_sound)
	{
		char printbuffer[512];
		struct sound_datum *sound = sound_get(sound_index);
		struct sound_definition *definition = sound_definition_get(sound->definition_index);

		render_debug_sphere(FALSE, &sound->source.location.position, sound_definition_get_maximum_distance(sound->definition_index), global_real_argb_yellow);
		render_debug_sphere(FALSE, &sound->source.location.position, sound_definition_get_minimum_distance(sound->definition_index), global_real_argb_red);
		sprintf(printbuffer, "%s|n%f %f", tag_get_name(sound->definition_index), sound->source.obstruction, sound->source.occlusion);
		render_debug_string_at_point(FALSE, &sound->source.location.position, printbuffer, global_real_argb_white);
	}

	return;
}

static void render_debug_looping_sound(
	long definition_index,
	struct sound_source const *source)
{
	if (debug_looping_sound && source->spatialization_mode == _sound_spatialization_mode_absolute)
	{
		short track_index;
		short detail_index;
		struct looping_sound_definition *definition = looping_sound_definition_get(definition_index);
		real minimum_distance = 0.f;
		real maximum_distance = 0.f;

		for (track_index = 0; track_index<definition->tracks.count; track_index++)
		{
			struct looping_sound_track *track = TAG_BLOCK_GET_ELEMENT(&definition->tracks, 0, struct looping_sound_track);

			if (track->loop_sound.index != NONE)
			{
				struct sound_definition *sound = sound_definition_get(track->loop_sound.index);

				minimum_distance = sound_definition_get_minimum_distance(track->loop_sound.index);
				maximum_distance = sound_definition_get_maximum_distance(track->loop_sound.index);
				break;
			}
		}

		if (minimum_distance == 0.f)
		{
			for (detail_index = 0; detail_index<definition->details.count; detail_index++)
			{
				struct looping_sound_detail *detail = TAG_BLOCK_GET_ELEMENT(&definition->details, 0, struct looping_sound_detail);

				if (detail->sound.index != NONE)
				{
					struct sound_definition *sound = sound_definition_get(detail->sound.index);

					minimum_distance = sound_definition_get_minimum_distance(detail->sound.index);
					maximum_distance = sound_definition_get_maximum_distance(detail->sound.index);
					break;
				}
			}
		}

		render_debug_string_at_point(FALSE, &source->location.position, tag_get_name(definition_index), global_real_argb_white);
		render_debug_sphere(FALSE, &source->location.position, maximum_distance, global_real_argb_cyan);
		render_debug_sphere(FALSE, &source->location.position, minimum_distance, global_real_argb_blue);
	}

	return;
}

void sound_initialize(
	void)
{
	struct sound_preferences *preferences;

	sound_manager_globals.initialized = FALSE;
	sound_manager_globals.active = TRUE;
	read_sound_preferences(&preferences);
	sound_cache_new();
	sound_manager_globals.sound_environment = default_sound_environment;
	sound_manager_globals.nondialog_gain = 1.f;

	if (preferences->platform_code>=0 && preferences->platform_code<NUMBER_OF_PLATFORM_SOUND_CODES &&
		platform_definitions[preferences->platform_code] &&
		platform_definitions[preferences->platform_code]->platform_code == preferences->platform_code)
	{
		sound_manager_globals.platform = platform_definitions[preferences->platform_code];
		sound_data = data_new("sounds", MAXIMUM_SOUNDS_PER_MAP, sizeof(struct sound_datum));

		if (sound_data)
		{
			looping_sound_data = data_new("looping sounds", MAXIMUM_LOOPING_SOUNDS_PER_MAP, sizeof(struct looping_sound_datum));

			if (looping_sound_data && sound_manager_globals.platform->initialize(preferences))
			{
				short channel_type;
				short channel_index = 0;

				data_make_valid(sound_data);
				data_make_valid(looping_sound_data);

				for (channel_type = 0; channel_type<NUMBER_OF_SOUND_CHANNEL_TYPES; channel_type++)
				{
					short type_channel_index;

					sound_manager_globals.channel_count += preferences->virtual_channel_counts[channel_type];
					match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 360, sound_manager_globals.channel_count<=MAXIMUM_SOUND_CHANNELS);

					for (type_channel_index = 0; type_channel_index<preferences->virtual_channel_counts[channel_type]; type_channel_index++)
					{
						struct sound_channel_datum *channel = channel_get(channel_index++);

						channel->sound_index = NONE;
						channel->type_flags = sound_channel_type_flags[channel_type];
						channel->playing_permutation = NULL;
						channel->queued_permutation = NULL;
					}
				}

				sound_manager_globals.initialized = TRUE;
			}
		}
	}

	return;
}

static real sound_scale_random_value(
	real base_lower_bound,
	real base_upper_bound,
	real lower_bound_modifier,
	real upper_bound_modifier,
	real scale)
{
	return sound_scale_value(real_local_random_range(base_lower_bound, base_upper_bound), lower_bound_modifier, upper_bound_modifier, scale);
}

static void sound_start_fade(
	short mode,
	real seconds,
	long fade_in_sound_index,
	long fade_out_sound_index)
{
	long start_time;
	long stop_time;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1087, mode==_sound_fade_mode_linear || mode==_sound_fade_mode_crossfade);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1088, seconds>=0.f);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1089, fade_in_sound_index!=NONE || fade_out_sound_index!=NONE);

	start_time = sound_manager_globals.render_time - 1;
	stop_time = (long)(seconds*1000.f + start_time);
	stop_time = MAX(stop_time, sound_manager_globals.render_time);

	if (fade_in_sound_index != NONE)
	{
		struct sound_datum *sound = sound_get(fade_in_sound_index);

		if (sound->fade_start_time != sound->fade_stop_time)
		{
			sound->fade_interpolation_start = sound_calculate_fade(fade_in_sound_index);
		}
		else
		{
			sound->fade_interpolation_start = 0.f;
		}

		sound->fade_interpolation_end = 1.f;
		sound->fade_mode = mode;
		sound->fade_start_time = start_time;
		sound->fade_stop_time = stop_time;
	}

	if (fade_out_sound_index != NONE)
	{
		struct sound_datum *sound = sound_get(fade_out_sound_index);

		sound->fade_interpolation_start = sound_calculate_fade(fade_out_sound_index);
		sound->fade_interpolation_end = 0.f;
		sound->fade_mode = mode;
		sound->fade_start_time = start_time;
		sound->fade_stop_time = stop_time;
	}

	return;
}

static void sound_stop(
	long sound_index)
{
	struct sound_datum *sound = sound_get(sound_index);
	struct sound_definition *definition = sound_definition_get(sound->definition_index);

	if (sound->playing_channel_index != NONE)
	{
		channel_get(sound->playing_channel_index)->sound_index = NONE;
		channel_stop(sound->playing_channel_index);
		sound->playing_channel_index = NONE;
	}
	else if (TEST_FLAG(sound->flags, _sound_cached_bit))
	{
		sound_cache_sound_finished(TAG_BLOCK_GET_ELEMENT(&TAG_BLOCK_GET_ELEMENT(&sound_definition_get(sound->definition_index)->pitch_ranges, sound->pitch_range_index, struct sound_pitch_range)->permutations, sound->permutation_index, struct sound_permutation));
	}

	if (sound->type != _sound_impulse)
	{
		struct looping_sound_datum *loop = looping_sound_try_and_get(sound->source_identifier);

		if (loop)
		{
			loop->component_sound_count--;

			if (loop->tracks[sound->loop_track_index].primary_sound_index == sound_index)
			{
				loop->tracks[sound->loop_track_index].primary_sound_index = NONE;
			}
		}
	}

	if (definition->runtime_scripting_sound_index == sound_index)
	{
		definition->runtime_scripting_sound_index = NONE;
	}

	sound_delete(sound_index);

	return;
}

static real source_distance_squared(
	short listener_index,
	struct sound_source const *source)
{
	real distance_squared;

	switch (source->spatialization_mode)
	{
	case _sound_spatialization_mode_none:
		distance_squared = 0.f;
		break;
	case _sound_spatialization_mode_absolute:
		distance_squared = distance_squared3d(&source->location.position, &listener_get(listener_index)->matrix.position);
		match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1396, listener_get(listener_index)->valid);
		break;
	case _sound_spatialization_mode_relative:
		distance_squared = magnitude_squared3d((real_vector3d const *)&source->location.position);
		break;
	default:
		match_halt("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1402);
	}

	return distance_squared;
}

static real source_distance(
	short listener_index,
	struct sound_source const *source)
{
	real distance;

	switch (source->spatialization_mode)
	{
	case _sound_spatialization_mode_none:
		distance = 0.f;
		break;
	case _sound_spatialization_mode_absolute:
		distance = distance3d(&source->location.position, &listener_get(listener_index)->matrix.position);
		match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1421, listener_get(listener_index)->valid);
		break;
	case _sound_spatialization_mode_relative:
		distance = magnitude3d((real_vector3d const *)&source->location.position);
		break;
	default:
		match_halt("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1427);
	}

	return distance;
}

static short sound_find_like_channel(
	long sound_index,
	short *channel_indices,
	short channel_count)
{
	short index;
	struct sound_datum *sound = sound_get(sound_index);
	struct sound_definition *definition = sound_definition_get(sound->definition_index);
	real distance_squared = source_distance_squared(sound->listener_index, &sound->source);

	for (index = 0; index<channel_count; index++)
	{
		short channel_index = channel_indices[index];
		struct sound_datum *channel_sound = sound_get(channel_get(channel_index)->sound_index);

		if (sound_manager_globals.render_time - channel_sound->start_time >= sound_class_get(definition->class_index)->preemption_time &&
			distance_squared - source_distance_squared(channel_sound->listener_index, &channel_sound->source) < 1.f)
		{
			return channel_index;
		}
	}

	return NONE;
}

static boolean sound_preempts_sound(
	long challenger_sound_index,
	long champion_sound_index,
	real challenger_distance_squared)
{
	struct sound_definition *challenger_definition = sound_definition_get(sound_get(challenger_sound_index)->definition_index);
	struct sound_datum *champion = sound_get(champion_sound_index);
	struct sound_definition *champion_definition = sound_definition_get(champion->definition_index);

	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1891, challenger_sound_index!=champion_sound_index);

	return sound_class_get(challenger_definition->class_index)->priority > sound_class_get(champion_definition->class_index)->priority ||
		(sound_class_get(challenger_definition->class_index)->priority == sound_class_get(champion_definition->class_index)->priority &&
		source_distance_squared(champion->listener_index, &champion->source) > challenger_distance_squared);
}

static void update_channel_for_impulse_sound(
	short channel_index,
	real fade)
{
	struct sound_channel_datum *channel = channel_get(channel_index);
	struct sound_datum *sound = sound_get(channel->sound_index);
	struct sound_definition *definition = sound_definition_get(sound->definition_index);
	real scale = sound->source.scale;
	real gain = sound_scale_value(fade * sound->source.gain * sound_manager_master_gain(definition->class_index), definition->scale_lower_bound.gain, definition->scale_upper_bound.gain, scale);

	if (sound->playing_channel_index == NONE)
	{
		struct platform_sound_channel_properties properties;
		struct sound_pitch_range *range = TAG_BLOCK_GET_ELEMENT(&definition->pitch_ranges, sound->pitch_range_index, struct sound_pitch_range);
		struct sound_permutation *permutation = TAG_BLOCK_GET_ELEMENT(&range->permutations, sound->permutation_index, struct sound_permutation);

		properties.gain = permutation->gain * definition->gain * gain;
		properties.pitch = sound->pitch * range->runtime_oo_natural_pitch;
		properties.minimum_distance = sound_definition_get_minimum_distance(sound->definition_index);
		properties.maximum_distance = REAL_MAX;
		properties.inner_cone_angle = definition->inner_cone_angle;
		properties.outer_cone_angle = definition->outer_cone_angle;
		properties.outer_cone_gain = definition->outer_cone_gain;
		properties.reverb_damping_factor = sound_class_get(definition->class_index)->reverb_damping_factor;
		match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1946, sound_cache_sound_loaded(permutation));

		channel_set_properties(channel_index, &properties, FALSE);
		channel_queue_sound(channel_index, permutation);
		sound->playing_channel_index = channel_index;
	}
	else
	{
		struct platform_sound_channel_properties properties;

		properties.gain = channel->playing_permutation->gain * definition->gain * gain;
		channel_set_properties(channel_index, &properties, TRUE);
	}

	sound_manager_globals.platform->channel_update(channel_index);

	return;
}

static long looping_sound_new(
	long definition_index,
	long identifier,
	struct sound_source const *source)
{
	long looping_sound_index = NONE;

	if (sound_is_active())
	{
		looping_sound_index = datum_new(looping_sound_data);

		if (looping_sound_index != NONE)
		{
			short detail_index;
			struct looping_sound_datum *loop = looping_sound_get(looping_sound_index);
			struct looping_sound_definition *definition = looping_sound_definition_get(definition_index);

			loop->definition_index = definition_index;
			loop->loop_identifier = identifier;
			loop->component_sound_count = 0;
			loop->ordered_permutations_finished = FALSE;

			for (detail_index = 0; detail_index<definition->details.count; detail_index++)
			{
				struct looping_sound_detail *detail = TAG_BLOCK_GET_ELEMENT(&definition->details, detail_index, struct looping_sound_detail);
				struct sound_definition *sound = sound_definition_get(detail->sound.index);

				loop->details[detail_index].next_play_time = (long)(sound_scale_random_value(detail->period_lower_bound, detail->period_upper_bound, definition->scale_lower_bound.detail_period, definition->scale_upper_bound.detail_period, source->scale) * 1000.f + sound_manager_globals.render_time);
			}
		}
	}

	return looping_sound_index;
}

static void sound_set_definition_end(
	long sound_index)
{
	struct sound_channel_summary summary;
	struct sound_datum *sound = sound_get(sound_index);
	struct sound_definition *definition = sound_definition_get(sound->next_definition_index);

	SET_FLAG(sound->flags, _sound_waiting_for_cache_bit, TRUE);
	sound->definition_index = sound->next_definition_index;
	sound->next_definition_index = NONE;
	sound->pitch_range_index = sound_definition_find_pitch_range_by_pitch(definition, sound->pitch, sound->pitch_range_index);
	sound->permutation_index = sound_definition_next_permutation(definition, sound->pitch_range_index, NONE);

	if (sound->playing_channel_index != NONE)
	{
		short channel_index;

		sound_channel_summary_build(&summary, sound_index);

		if (summary.like_source_count>=summary.maximum_source_instance_count)
		{
			channel_index = sound_find_like_channel(sound_index, summary.like_source_channels, summary.like_source_count);
			if (channel_index != NONE)
			{
				sound_stop(channel_get(channel_index)->sound_index);
			}
			else
			{
				sound_stop(sound_index);
			}
		}
		else if (summary.like_definition_count>=summary.maximum_instance_count)
		{
			channel_index = sound_find_like_channel(sound_index, summary.like_definition_channels, summary.like_definition_count);
			if (channel_index != NONE)
			{
				sound_stop(channel_get(channel_index)->sound_index);
			}
			else
			{
				sound_stop(sound_index);
			}
		}
	}

	return;
}

static void detail_sound_random_offset(
	struct looping_sound_detail *detail_definition,
	real_vector3d *offset)
{
	real distance = real_local_random_range(detail_definition->distance_lower_bound, detail_definition->distance_upper_bound);

	if (distance != 0.f)
	{
		real_euler_angles2d random_angles;

		random_angles.pitch = real_local_random_range(detail_definition->phi_lower_bound, detail_definition->phi_upper_bound);
		random_angles.yaw = real_local_random_range(detail_definition->theta_lower_bound, detail_definition->theta_upper_bound);
		vector3d_from_euler_angles2d(offset, &random_angles);
		scale_vector3d(offset, distance, offset);
	}
	else
	{
		*offset = *global_zero_vector3d;
	}

	return;
}

void sound_stop_impulse(
	long sound_index)
{
	if (sound_try_and_get(sound_index))
	{
		match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 705, sound_get(sound_index)->type==_sound_impulse);

		if (sound_get(sound_index)->type == _sound_impulse)
		{
			sound_start_fade(_sound_fade_mode_linear, 0.3f, NONE, sound_index);
		}
	}

	return;
}

void sound_stop_impulse_by_source_and_definition(
	long source_identifier,
	long definition_index)
{
	long sound_index;

	for (sound_index = data_next_index(sound_data, NONE); sound_index != NONE; sound_index = data_next_index(sound_data, sound_index))
	{
		struct sound_datum *sound = sound_get(sound_index);

		if (sound->type == _sound_impulse && sound->source_identifier == source_identifier && sound->definition_index == definition_index)
		{
			sound_stop_impulse(sound_index);
			break;
		}
	}

	return;
}

void sound_stop_all(
	void)
{
	if (sound_manager_globals.initialized)
	{
		long sound_index;

		for (sound_index = data_next_index(sound_data, NONE); sound_index != NONE; sound_index = data_next_index(sound_data, sound_index))
		{
			sound_stop(sound_index);
		}

		data_delete_all(looping_sound_data);
		sound_manager_globals.platform->flush();
	}

	sound_manager_globals.game_time_when_no_scripted_dialog_will_be_playing = 0;

	return;
}

static short source_audible(
	struct sound_source *source,
	real maximum_distance)
{
	short listener_index = NONE;

	if (source->spatialization_mode == _sound_spatialization_mode_none)
	{
		listener_index = 0;
	}
	else if (source->spatialization_mode == _sound_spatialization_mode_relative)
	{
		if (source_distance_squared(NONE, source) < maximum_distance)
		{
			listener_index = 0;
		}
	}
	else
	{
		short index;
		real best_distance_squared = REAL_MAX;

		for (index = 0; index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; index++)
		{
			if (listener_get(index)->valid)
			{
				real distance_squared = source_distance_squared(index, source);

				if (distance_squared < best_distance_squared)
				{
					best_distance_squared = distance_squared;
					listener_index = index;
				}
			}
		}

		if (listener_index != NONE)
		{
			compute_sound_obstruction(listener_index, source, square_root(best_distance_squared));
		}

		if (maximum_distance*maximum_distance < best_distance_squared || source->occlusion == 1.f)
		{
			listener_index = NONE;
		}
	}

	return listener_index;
}

static void refresh_sounds(
	void)
{
	long sound_index;
	boolean players_dead = players_are_all_dead();
	boolean scripted_dialog_playing = FALSE;

	for (sound_index = data_next_index(sound_data, NONE); sound_index != NONE; sound_index = data_next_index(sound_data, sound_index))
	{
		struct sound_datum *sound = sound_get(sound_index);
		struct sound_definition *definition = sound_definition_get(sound->definition_index);

		if ((sound->playing_channel_index == NONE ||
			channel_get_state(sound->playing_channel_index) != _sound_channel_idle ||
			sound->type == _sound_loop_track ||
			sound->type == _sound_stopping_track) &&
			refresh_sound(sound_index))
		{
			short listener_index = source_audible(&sound->source, sound_definition_get_maximum_distance(sound->definition_index));

			render_debug_sound(sound_index);

			if (definition->class_index == _sound_class_scripted_dialog_to_player ||
				definition->class_index == _sound_class_scripted_dialog_to_other ||
				definition->class_index == _sound_class_scripted_dialog_force_unspatialized)
			{
				scripted_dialog_playing = TRUE;
			}

			if (listener_index == NONE)
			{
				if (!TEST_FLAG(sound->flags, _sound_inaudible_bit))
				{
					sound_start_fade(_sound_fade_mode_linear, sound_inaudible_fade_out_time, NONE, sound_index);
					SET_FLAG(sound->flags, _sound_inaudible_bit, TRUE);
				}
			}
			else
			{
				sound->listener_index = listener_index;

				if (TEST_FLAG(sound->flags, _sound_inaudible_bit))
				{
					sound_start_fade(_sound_fade_mode_linear, sound_inaudible_fade_back_in_time, sound_index, NONE);
					SET_FLAG(sound->flags, _sound_inaudible_bit, FALSE);
				}
			}

			if (players_dead)
			{
				if (definition->class_index == _sound_class_scripted_dialog_to_player)
				{
					if (sound->playing_channel_index != NONE)
					{
						sound_start_fade(_sound_fade_mode_linear, sound_player_fade_out_time, NONE, sound_index);
					}
					else
					{
						sound_stop(sound_index);
					}
				}
				else if (definition->class_index == _sound_class_scripted_dialog_to_other && sound->playing_channel_index == NONE)
				{
					sound_stop(sound_index);
				}
			}
		}
		else
		{
			sound_stop(sound_index);
		}
	}

	if (scripted_dialog_playing)
	{
		real maximum_change = sound_manager_globals.ticks_elapsed * 0.03f;
		real change = sound_gain_under_dialog - sound_manager_globals.nondialog_gain;

		sound_manager_globals.nondialog_gain += PIN(change, -maximum_change, maximum_change);
	}
	else
	{
		real maximum_change = sound_manager_globals.ticks_elapsed * 0.007f;
		real change = 1.f - sound_manager_globals.nondialog_gain;

		sound_manager_globals.nondialog_gain += PIN(change, -maximum_change, maximum_change);
	}

	return;
}

static short sound_find_best_channel(
	long sound_index)
{
	real best_distance_squared;
	short channel_index;
	short best_channel_index = NONE;
	struct sound_datum *sound = sound_get(sound_index);
	struct sound_definition *definition = sound_definition_get(sound->definition_index);
	long best_sound_index = NONE;
	real distance_squared = source_distance_squared(sound->listener_index, &sound->source);

	for (channel_index = 0; channel_index<sound_manager_globals.channel_count; channel_index++)
	{
		struct sound_channel_datum *channel = channel_get(channel_index);

		if (sound_valid_for_channel(definition->compression, definition->encoding, definition->sample_rate, sound->source.spatialization_mode, channel->type_flags))
		{
			if (channel->sound_index == NONE)
			{
				return channel_index;
			}

			if (sound_preempts_sound(sound_index, channel->sound_index, distance_squared) &&
				(best_channel_index == NONE || sound_preempts_sound(best_sound_index, channel->sound_index, best_distance_squared)))
			{
				struct sound_datum *channel_sound = sound_get(channel->sound_index);

				best_channel_index = channel_index;
				best_sound_index = channel->sound_index;
				best_distance_squared = source_distance_squared(channel_sound->listener_index, &channel_sound->source);
			}
		}
	}

	return best_channel_index;
}

static long looping_sound_new_sound(
	long looping_sound_index,
	long definition_index,
	short track_index,
	short type)
{
	long sound_index = NONE;
	struct looping_sound_datum *loop = looping_sound_get(looping_sound_index);
	real scale = loop->source.scale;

	if (sound_definition_is_playable(definition_index))
	{
		struct sound_definition *definition = sound_definition_get(definition_index);
		real maximum_distance = sound_definition_get_maximum_distance(definition_index);
		short listener_index = source_audible(&loop->source, maximum_distance);

		if (listener_index != NONE)
		{
			sound_index = datum_new(sound_data);

			if (sound_index != NONE)
			{
				struct sound_datum *sound = sound_get(sound_index);

				sound->listener_index = listener_index;
				sound->definition_index = definition_index;
				sound->playing_channel_index = NONE;
				sound->flags = 0;
				sound->pitch = real_local_random_range(definition->pitch_lower_bound, definition->pitch_upper_bound);
				sound->source_identifier = looping_sound_index;
				sound->source = loop->source;
				sound->type = type;
				sound->start_time = sound_manager_globals.render_time;
				sound->loop_track_index = track_index;
				sound->track_proc = track_loop_track_sound;
				sound->fade_stop_time = 0;
				sound->fade_start_time = 0;
				sound->next_definition_index = NONE;
				sound->pitch_range_index = sound_definition_find_pitch_range_by_pitch(definition, sound_scale_value(sound->pitch, definition->scale_lower_bound.pitch, definition->scale_upper_bound.pitch, scale), NONE);
				sound->permutation_index = sound_definition_next_permutation(definition, sound->pitch_range_index, NONE);
				_sound_cache_sound_request(TAG_BLOCK_GET_ELEMENT(&TAG_BLOCK_GET_ELEMENT(&sound_definition_get(sound->definition_index)->pitch_ranges, sound->pitch_range_index, struct sound_pitch_range)->permutations, sound->permutation_index, struct sound_permutation), FALSE, TRUE, FALSE);
				loop->component_sound_count++;
			}
		}
	}

	return sound_index;
}

static void update_channel_for_looping_sound(
	short channel_index,
	real fade)
{
	struct platform_sound_channel_properties properties;
	struct sound_pitch_range *range;
	struct sound_channel_datum *channel = channel_get(channel_index);
	struct sound_datum *sound = sound_get(channel->sound_index);
	struct sound_definition *definition = sound_definition_get(sound->definition_index);
	struct looping_sound_datum *loop = looping_sound_get(sound->source_identifier);
	struct looping_sound_track_datum *track_datum = &loop->tracks[sound->loop_track_index];
	struct looping_sound_definition *loop_definition = looping_sound_definition_get(loop->definition_index);
	struct looping_sound_track *track = TAG_BLOCK_GET_ELEMENT(&loop_definition->tracks, sound->loop_track_index, struct looping_sound_track);
	real scale = sound->source.scale;
	real pitch = sound_scale_value(sound->pitch, definition->scale_lower_bound.pitch, definition->scale_upper_bound.pitch, scale);

	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 2500, sound->type!=_sound_impulse);

	properties.minimum_distance = sound_definition_get_minimum_distance(sound->definition_index);
	properties.maximum_distance = REAL_MAX;
	properties.inner_cone_angle = definition->inner_cone_angle;
	properties.outer_cone_angle = definition->outer_cone_angle;
	properties.outer_cone_gain = definition->outer_cone_gain;
	properties.reverb_damping_factor = sound_class_get(definition->class_index)->reverb_damping_factor;
	properties.gain = sound_scale_value(fade * sound->source.gain * definition->gain * track->gain * sound_manager_master_gain(definition->class_index), definition->scale_lower_bound.gain, definition->scale_upper_bound.gain, scale);

	if (sound->playing_channel_index == NONE)
	{
		struct sound_permutation *permutation;

		range = TAG_BLOCK_GET_ELEMENT(&definition->pitch_ranges, sound->pitch_range_index, struct sound_pitch_range);
		permutation = TAG_BLOCK_GET_ELEMENT(&range->permutations, sound->permutation_index, struct sound_permutation);
		properties.gain *= permutation->gain;
		properties.pitch = pitch * range->runtime_oo_natural_pitch;
		match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 2522, sound_cache_sound_loaded(permutation));

		channel_set_properties(channel_index, &properties, FALSE);
		channel_queue_sound(channel_index, permutation);
		sound->playing_channel_index = channel_index;
	}
	else
	{
		range = TAG_BLOCK_GET_ELEMENT(&definition->pitch_ranges, sound->pitch_range_index, struct sound_pitch_range);
		pitch = limit_pitch(pitch, channel_get(sound->playing_channel_index)->pitch * range->natural_pitch, definition->maximum_bend);
		properties.pitch = pitch * range->runtime_oo_natural_pitch;

		if (sound->type == _sound_loop_track &&
			(sound->fade_start_time == sound->fade_stop_time || sound->fade_interpolation_end != 0.f) &&
			sound_definition_find_pitch_range_by_pitch(definition, pitch, sound->pitch_range_index) != sound->pitch_range_index &&
			channel->sound_index == track_datum->primary_sound_index &&
			!sound_manager_globals.idling)
		{
			long new_sound_index = looping_sound_new_sound(sound->source_identifier, sound->definition_index, sound->loop_track_index, _sound_loop_track);

			if (new_sound_index != NONE)
			{
				sound_start_fade(_sound_fade_mode_crossfade, sound_pitch_range_fade_time, new_sound_index, channel->sound_index);
				track_datum->primary_sound_index = new_sound_index;
			}
		}

		if (sound->type != _sound_stop_track && (sound->type != _sound_start_track || !TEST_FLAG(track->flags, _fade_in_at_start_bit)))
		{
			short state = channel_get_state(sound->playing_channel_index);

			if (state != _sound_channel_full ||
				TEST_FLAG(sound->flags, _sound_waiting_for_cache_bit) ||
				(state == _sound_channel_full && channel->playing_permutation->next_permutation_index == NONE && sound->next_definition_index != NONE))
			{
				struct sound_permutation *permutation;

				if (sound->next_definition_index != NONE &&
					(!channel->playing_permutation || channel->playing_permutation->next_permutation_index == NONE))
				{
					sound_set_definition_end(channel->sound_index);
					definition = sound_definition_get(sound->definition_index);
					range = TAG_BLOCK_GET_ELEMENT(&definition->pitch_ranges, sound->pitch_range_index, struct sound_pitch_range);
				}
				else if (!TEST_FLAG(sound->flags, _sound_waiting_for_cache_bit))
				{
					short permutation_index = sound_definition_next_permutation(definition, sound->pitch_range_index, sound->permutation_index);

					if (permutation_index == NONE)
					{
						match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 2588, TEST_FLAG(definition->flags, _sound_definition_linked_permutations_bit));

						if (!TEST_FLAG(loop_definition->flags, _looping_sound_fake_impulse_sound_bit))
						{
							permutation_index = sound_definition_next_permutation(definition, sound->pitch_range_index, NONE);
						}
						else
						{
							sound->type = _sound_stop_track;
							loop->ordered_permutations_finished = TRUE;
						}
					}

					if (permutation_index != NONE)
					{
						SET_FLAG(sound->flags, _sound_waiting_for_cache_bit, TRUE);
						sound->permutation_index = permutation_index;
					}
				}

				permutation = TAG_BLOCK_GET_ELEMENT(&range->permutations, sound->permutation_index, struct sound_permutation);

				if (sound->type != _sound_stop_track && _sound_cache_sound_request(permutation, FALSE, TRUE, TRUE))
				{
					SET_FLAG(sound->flags, _sound_waiting_for_cache_bit, FALSE);
					channel_queue_sound(channel_index, permutation);

					if (sound->next_definition_index == NONE && permutation->next_permutation_index == NONE)
					{
						if (sound->type == _sound_start_track)
						{
							sound->type = _sound_loop_track;
						}
						else if (sound->type == _sound_stopping_track)
						{
							sound->type = _sound_stop_track;
						}
					}
				}
			}
		}

		properties.gain *= TAG_BLOCK_GET_ELEMENT(&range->permutations, sound->permutation_index, struct sound_permutation)->gain;
		channel_set_properties(channel_index, &properties, FALSE);
	}

	sound_manager_globals.platform->channel_update(channel_index);

	return;
}

long sound_new_impulse(
	long definition_index,
	struct sound_source *source,
	long source_identifier,
	boolean (*track_proc)(long, void const *, struct sound_source *),
	void const *track_data,
	short track_data_size)
{
	long sound_index = NONE;
	struct sound_definition *definition = sound_definition_get(definition_index);
	real scale = source->scale;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 576, track_data_size<=MAXIMUM_SOUND_CALLBACK_DATA);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 578, source->spatialization_mode==_sound_spatialization_mode_none || valid_real_normal3d(&source->location.forward));

	if (definition->class_index == _sound_class_scripted_dialog_to_player ||
		definition->class_index == _sound_class_scripted_dialog_to_other ||
		definition->class_index == _sound_class_scripted_dialog_force_unspatialized)
	{
		long time = game_time_get() + definition->runtime_maximum_play_time*TICKS_PER_SECOND/1000 + 10;

		if (time > sound_manager_globals.game_time_when_no_scripted_dialog_will_be_playing)
		{
			sound_manager_globals.game_time_when_no_scripted_dialog_will_be_playing = time;
		}

		if (loud_dialog_hack)
		{
			source->spatialization_mode = _sound_spatialization_mode_none;
		}
	}

	if (definition->class_index == _sound_class_scripted_dialog_force_unspatialized)
	{
		source->spatialization_mode = _sound_spatialization_mode_none;
	}

	if (sound_is_active())
	{
		if (definition->compression == _sound_compression_xbox_adpcm &&
			((definition->encoding == _sound_encoding_mono && definition->sample_rate == _sound_sample_rate_22k) || definition->encoding == _sound_encoding_stereo))
		{
			if (source->scale != 0.f || definition->scale_lower_bound.gain != 0.f)
			{
				real random = real_local_random();

				if (random > sound_scale_value(definition->skip_fraction, definition->scale_lower_bound.skip_fraction, definition->scale_upper_bound.skip_fraction, scale))
				{
					real maximum_distance = sound_definition_get_maximum_distance(definition_index);

					if (sound_definition_is_playable(definition_index))
					{
						short listener_index = source_audible(source, maximum_distance);

						if (listener_index != NONE)
						{
							short promotion = sound_definition_promote(definition_index);

							if (promotion == _sound_promotion_dont)
							{
								sound_index = datum_new(sound_data);

								if (sound_index != NONE)
								{
									struct sound_datum *sound = sound_get(sound_index);
									long delay = sound_travel_milliseconds(source_distance(listener_index, source));

									sound->definition_index = definition_index;
									sound->playing_channel_index = NONE;
									sound->listener_index = listener_index;
									sound->type = _sound_impulse;
									sound->pitch = sound_scale_random_value(definition->pitch_lower_bound, definition->pitch_upper_bound, definition->scale_lower_bound.pitch, definition->scale_upper_bound.pitch, source->scale);
									sound->flags = 0;
									sound->source_identifier = source_identifier;
									sound->source = *source;
									sound->track_proc = track_proc;

									if (track_proc)
									{
										match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 654, sound->track_data);
										memcpy(sound->track_data, track_data, track_data_size);
									}

									sound->pitch_range_index = sound_definition_find_pitch_range_by_pitch(definition, sound->pitch, NONE);
									sound->permutation_index = sound_definition_next_permutation(definition, sound->pitch_range_index, NONE);
									sound->fade_stop_time = 0;
									sound->fade_start_time = 0;
									sound->loop_track_index = NONE;
									_sound_cache_sound_request(TAG_BLOCK_GET_ELEMENT(&TAG_BLOCK_GET_ELEMENT(&sound_definition_get(sound->definition_index)->pitch_ranges, sound->pitch_range_index, struct sound_pitch_range)->permutations, sound->permutation_index, struct sound_permutation), FALSE, TRUE, FALSE);

									if (delay > speed_of_sound_threshold)
									{
										sound->start_time = sound_manager_globals.render_time + delay;
										SET_FLAG(sound->flags, _sound_delayed_bit, TRUE);
									}
									else
									{
										sound->start_time = sound_manager_globals.render_time;
									}
								}
							}
							else if (promotion == _sound_promotion_do)
							{
								sound_index = sound_new_impulse(definition->promotion_sound.index, source, source_identifier, track_proc, track_data, track_data_size);
							}
							else
							{
								sound_index = NONE;
							}
						}
					}
				}
			}
		}
		else
		{
			error(_error_silent, "attempt to play a sound that was not a mono 22k compressed sound or a stereo 22k or 44k compressed sound.");
		}
	}

	return sound_index;
}

boolean sound_refresh_looping(
	long definition_index,
	long identifier,
	struct sound_source *source,
	short refresh_state,
	boolean alternate,
	real force_stop_time)
{
	boolean result = refresh_state == _looping_sound_refresh_stop;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 756, source->spatialization_mode==_sound_spatialization_mode_none || valid_real_normal3d(&source->location.forward));

	render_debug_looping_sound(definition_index, source);

	if (sound_is_active())
	{
		long looping_sound_index = looping_sound_find(identifier);
		boolean new_loop = FALSE;

		result = TRUE;

		if (looping_sound_index == NONE && refresh_state != _looping_sound_refresh_stop)
		{
			looping_sound_index = looping_sound_new(definition_index, identifier, source);
			new_loop = TRUE;
		}

		if (looping_sound_index != NONE)
		{
			struct looping_sound_datum *loop = looping_sound_get(looping_sound_index);
			struct looping_sound_definition *definition = looping_sound_definition_get(definition_index);

			match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 782, loop->definition_index==definition_index);

			loop->source = *source;
			loop->flip_flop = sound_manager_globals.flip_flop;

			if ((refresh_state == _looping_sound_refresh_stop || loop->ordered_permutations_finished) && loop->component_sound_count == 0)
			{
				datum_delete(looping_sound_data, looping_sound_index);
			}
			else
			{
				short track_index;

				result = FALSE;

				if (definition->continuous_damage_effect.index != NONE)
				{
					player_effect_continuous_refresh(definition->continuous_damage_effect.index, &source->location.position);
				}

				for (track_index = 0; track_index<definition->tracks.count; track_index++)
				{
					struct looping_sound_track *track = TAG_BLOCK_GET_ELEMENT(&definition->tracks, track_index, struct looping_sound_track);
					struct looping_sound_track_datum *track_datum = &loop->tracks[track_index];

					if (new_loop)
					{
						track_datum->primary_sound_index = NONE;
					}

					if (refresh_state == _looping_sound_refresh_start && track->start_sound.index != NONE)
					{
						track_datum->primary_sound_index = looping_sound_new_sound(looping_sound_index, track->start_sound.index, track_index, _sound_start_track);
					}

					if (refresh_state != _looping_sound_refresh_stop && !loop->ordered_permutations_finished)
					{
						long loop_definition_index = track->loop_sound.index;

						if (alternate && track->alternate_loop_sound.index != NONE)
						{
							loop_definition_index = track->alternate_loop_sound.index;
						}

						if (loop_definition_index != NONE)
						{
							if (track_datum->primary_sound_index != NONE &&
								(refresh_state != _looping_sound_refresh_start || !TEST_FLAG(track->flags, _fade_in_at_start_bit)))
							{
								struct sound_datum *primary_sound = sound_get(track_datum->primary_sound_index);

								if (alternate != loop->alternate && TEST_FLAG(track->flags, _fade_in_alternate_bit))
								{
									long sound_index = looping_sound_new_sound(looping_sound_index, loop_definition_index, track_index, _sound_loop_track);

									if (sound_index != NONE)
									{
										sound_start_fade(_sound_fade_mode_linear, track->fade_out_duration, sound_index, track_datum->primary_sound_index);
										track_datum->primary_sound_index = sound_index;
									}
								}
								else if (!new_loop)
								{
									sound_set_definition_begin(track_datum->primary_sound_index, loop_definition_index);
								}
							}
							else
							{
								long sound_index = looping_sound_new_sound(looping_sound_index, loop_definition_index, track_index, _sound_loop_track);

								if (sound_index != NONE)
								{
									struct sound_datum *sound = sound_get(sound_index);

									if (refresh_state == _looping_sound_refresh_start)
									{
										if (TEST_FLAG(track->flags, _fade_in_at_start_bit))
										{
											sound_start_fade(_sound_fade_mode_linear, track->fade_in_duration, sound_index, NONE);
										}
									}
									else
									{
										sound_start_fade(_sound_fade_mode_linear, sound_inaudible_fade_out_time, sound_index, NONE);
									}

									track_datum->primary_sound_index = sound_index;
								}
							}
						}
					}
					else if (loop->state != _looping_sound_refresh_stop)
					{
						if (force_stop_time != 0.f)
						{
							sound_start_fade(_sound_fade_mode_linear, force_stop_time, NONE, track_datum->primary_sound_index);
						}
						else
						{
							if (track_datum->primary_sound_index != NONE &&
								(TEST_FLAG(track->flags, _fade_out_at_stop_bit) ||
								(track->stop_sound.index == NONE && !TEST_FLAG(definition->flags, _looping_sound_fake_impulse_sound_bit))))
							{
								sound_start_fade(_sound_fade_mode_linear, track->fade_out_duration, NONE, track_datum->primary_sound_index);
							}

							if (track->stop_sound.index != NONE)
							{
								long stop_definition_index = track->stop_sound.index;

								if (alternate && track->alternate_stop_sound.index != NONE)
								{
									stop_definition_index = track->alternate_stop_sound.index;
								}

								if (TEST_FLAG(track->flags, _fade_out_at_stop_bit))
								{
									looping_sound_new_sound(looping_sound_index, stop_definition_index, track_index, _sound_stop_track);
								}
								else if (track_datum->primary_sound_index != NONE)
								{
									struct sound_datum *sound = sound_get(track_datum->primary_sound_index);

									if (sound->playing_channel_index != NONE)
									{
										sound_set_definition_begin(track_datum->primary_sound_index, stop_definition_index);
										sound->type = _sound_stopping_track;
									}
								}
							}
						}
					}
				}

				if (loop->component_sound_count == 0 && source_audible(source, definition->runtime_maximum_distance) == NONE)
				{
					datum_delete(looping_sound_data, looping_sound_index);
				}

				loop->alternate = alternate;
				loop->state = refresh_state;
			}
		}
		else if (new_loop)
		{
			result = FALSE;
		}
	}

	return result;
}

static void refresh_listener(
	void)
{
	if (game_in_progress())
	{
		struct platform_sound_listener_properties default_listener;
		struct sound_source source;
		short local_player_index;

		for (local_player_index = 0; local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
		{
			struct sound_listener *listener = listener_get(local_player_index);

			if (local_player_get_player_index(local_player_index) != NONE)
			{
				boolean underwater;
				struct observer_result const *camera = observer_get_camera(local_player_index);

				match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1263, camera);

				listener->valid = TRUE;
				underwater = scenario_location_underwater(&camera->location, &camera->position, NULL);

				if (listener->underwater != underwater)
				{
					struct game_globals *game_globals = scenario_get_game_globals();

					source.spatialization_mode = _sound_spatialization_mode_none;
					source.scale = 1.f;
					source.gain = 1.f;

					if (underwater)
					{
						if (game_globals->sounds.count>_global_sound_water_in)
						{
							struct tag_reference *reference = TAG_BLOCK_GET_ELEMENT(&game_globals->sounds, _global_sound_water_in, struct tag_reference);

							if (reference->index != NONE)
							{
								sound_new_impulse(reference->index, &source, NONE, NULL, NULL, 0);
							}
						}
					}
					else
					{
						if (game_globals->sounds.count>_global_sound_water_out)
						{
							struct tag_reference *reference = TAG_BLOCK_GET_ELEMENT(&game_globals->sounds, _global_sound_water_out, struct tag_reference);

							if (reference->index != NONE)
							{
								sound_new_impulse(reference->index, &source, NONE, NULL, NULL, 0);
							}
						}
					}
				}

				listener->underwater = underwater;
				matrix4x3_from_point_and_vectors(&listener->matrix, &camera->position, &camera->forward, &camera->up);
				matrix4x3_inverse_transform_vector(&listener->matrix, &camera->velocity, &listener->velocity);
			}
			else
			{
				listener->valid = FALSE;
			}
		}

		default_listener.forward = *global_forward3d;
		default_listener.up = *global_up3d;
		default_listener.position = *global_origin3d;
		default_listener.translational_velocity = *global_zero_vector3d;
		default_listener.sound_environment = &sound_manager_globals.sound_environment;
		sound_manager_globals.platform->set_listener_properties(&default_listener);
	}

	return;
}

static short sound_find_channel(
	long sound_index)
{
	struct sound_channel_summary summary;
	struct sound_datum *sound = sound_get(sound_index);

	if (sound->playing_channel_index != NONE)
	{
		match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1744, channel_get(sound->playing_channel_index)->sound_index==sound_index);

		return sound->playing_channel_index;
	}

	if (sound_class_get(sound_definition_get(sound->definition_index)->class_index)->speech && sound->source_identifier != NONE)
	{
		short channel_index;

		for (channel_index = 0; channel_index<sound_manager_globals.channel_count; channel_index++)
		{
			struct sound_channel_datum *channel = channel_get(channel_index);

			if (channel->sound_index != NONE)
			{
				struct sound_datum *channel_sound = sound_get(channel->sound_index);

				if (channel_sound->source_identifier == sound->source_identifier &&
					sound_class_get(sound_definition_get(channel_sound->definition_index)->class_index)->speech)
				{
					sound->source.spatialization_mode = channel_sound->source.spatialization_mode;

					return channel_index;
				}
			}
		}

		return sound_find_best_channel(sound_index);
	}

	sound_channel_summary_build(&summary, sound_index);

	if (summary.like_source_count>=summary.maximum_source_instance_count)
	{
		return sound_find_like_channel(sound_index, summary.like_source_channels, summary.like_source_count);
	}

	if (summary.like_definition_count>=summary.maximum_instance_count)
	{
		return sound_find_like_channel(sound_index, summary.like_definition_channels, summary.like_definition_count);
	}

	return sound_find_best_channel(sound_index);
}

static void update_channels(
	void)
{
	short channel_index;

	for (channel_index = 0; channel_index<sound_manager_globals.channel_count; channel_index++)
	{
		struct sound_channel_datum *channel = channel_get(channel_index);

		if (channel->sound_index != NONE)
		{
			struct sound_datum *sound = sound_get(channel->sound_index);
			struct sound_definition *definition = sound_definition_get(sound->definition_index);
			real fade = sound_calculate_fade(channel->sound_index);

			if (fade == 0.f && sound->fade_interpolation_end == 0.f)
			{
				sound_stop(channel->sound_index);
				channel->sound_index = NONE;
			}
			else
			{
				if (TEST_FLAG(channel->type_flags, _sound_channel_3d_bit))
				{
					switch (sound->source.spatialization_mode)
					{
					case _sound_spatialization_mode_none:
						match_halt("c:\\halo\\SOURCE\\sound\\sound_manager.c", 2004);
						break;
					case _sound_spatialization_mode_absolute:
					{
						struct sound_location transformed_location;
						struct sound_listener *listener = listener_get(sound->listener_index);

						match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 2011, listener->valid);
						matrix4x3_inverse_transform_point(&listener->matrix, &sound->source.location.position, &transformed_location.position);
						matrix4x3_inverse_transform_normal(&listener->matrix, &sound->source.location.forward, &transformed_location.forward);
						matrix4x3_inverse_transform_vector(&listener->matrix, &sound->source.location.translational_velocity, &transformed_location.translational_velocity);
						transformed_location.translational_velocity.i = transformed_location.translational_velocity.i*TICKS_PER_SECOND - listener->velocity.i;
						transformed_location.translational_velocity.j = transformed_location.translational_velocity.j*TICKS_PER_SECOND - listener->velocity.j;
						transformed_location.translational_velocity.k = transformed_location.translational_velocity.k*TICKS_PER_SECOND - listener->velocity.k;
						sound_manager_globals.platform->set_channel_location(channel_index, TRUE, &transformed_location, sound->source.obstruction, sound->source.occlusion, listener->underwater);
						break;
					}
					case _sound_spatialization_mode_relative:
						sound_manager_globals.platform->set_channel_location(channel_index, TRUE, &sound->source.location, 0.f, 0.f, FALSE);
						break;
					default:
						match_halt("c:\\halo\\SOURCE\\sound\\sound_manager.c", 2028);
					}
				}
				else
				{
					real_point3d position = sound->source.location.position;

					switch (sound->source.spatialization_mode)
					{
					case _sound_spatialization_mode_none:
						break;
					case _sound_spatialization_mode_absolute:
					{
						struct sound_listener *listener = listener_get(sound->listener_index);

						match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 2042, listener->valid);
						matrix4x3_inverse_transform_point(&listener->matrix, &sound->source.location.position, &position);
					}
					case _sound_spatialization_mode_relative:
					{
						real minimum_distance = sound_definition_get_minimum_distance(sound->definition_index);
						real maximum_distance = sound_definition_get_maximum_distance(sound->definition_index);

						fade *= PIN(1.f - (magnitude3d((real_vector3d *)&position) - minimum_distance) / (maximum_distance - minimum_distance), 0.f, 1.f);
						break;
					}
					default:
						match_halt("c:\\halo\\SOURCE\\sound\\sound_manager.c", 2058);
					}
				}

				if (sound->type == _sound_impulse)
				{
					update_channel_for_impulse_sound(channel_index, fade);
				}
				else
				{
					update_channel_for_looping_sound(channel_index, fade);
				}

				if (sound_class_get(definition->class_index)->speech && sound->track_proc == track_object_impulse_sound)
				{
					game_sound_set_mouth_aperture(sound->source_identifier, sound_permutation_get_real_mouth_aperture(channel->playing_permutation, (short)channel->estimated_tick_time));
				}
			}
		}
	}

	return;
}

static void process_looping_sounds(
	void)
{
	long looping_sound_index;

	for (looping_sound_index = data_next_index(looping_sound_data, NONE);
		looping_sound_index != NONE;
		looping_sound_index = data_next_index(looping_sound_data, looping_sound_index))
	{
		struct looping_sound_datum *loop = looping_sound_get(looping_sound_index);
		struct looping_sound_definition *definition = looping_sound_definition_get(loop->definition_index);

		if (loop->flip_flop != sound_manager_globals.flip_flop)
		{
			datum_delete(looping_sound_data, looping_sound_index);
		}
		else if (loop->state != _looping_sound_refresh_stop)
		{
			short detail_index;

			for (detail_index = 0; detail_index<definition->details.count; detail_index++)
			{
				struct looping_sound_detail *detail = TAG_BLOCK_GET_ELEMENT(&definition->details, detail_index, struct looping_sound_detail);

				if (loop->details[detail_index].next_play_time<sound_manager_globals.render_time && detail->sound.index != NONE)
				{
					struct sound_definition *sound_definition = sound_definition_get(detail->sound.index);
					real scale = loop->source.scale;

					if (!(TEST_FLAG(detail->flags, _detail_dont_play_with_alternate_bit) && loop->alternate) &&
						!(TEST_FLAG(detail->flags, _detail_dont_play_without_alternate_bit) && !loop->alternate))
					{
						struct loop_impulse_sound_tracking_data track_data;
						struct sound_source detail_source;

						detail_source.spatialization_mode = loop->source.spatialization_mode==_sound_spatialization_mode_none ? _sound_spatialization_mode_relative : _sound_spatialization_mode_absolute;
						detail_source.gain = detail->gain;
						detail_source.scale = scale;
						detail_sound_random_offset(detail, &track_data.position_offset);
						track_loop_impulse_sound(looping_sound_index, &track_data, &detail_source);
						sound_new_impulse(detail->sound.index, &detail_source, looping_sound_index, track_loop_impulse_sound, &track_data, sizeof(track_data));
					}

					loop->details[detail_index].next_play_time = (long)(sound_scale_random_value(detail->period_lower_bound, detail->period_upper_bound, definition->scale_lower_bound.detail_period, definition->scale_upper_bound.detail_period, scale) * 1000.f + sound_definition->runtime_maximum_play_time + sound_manager_globals.render_time);
				}
			}
		}
	}

	return;
}

void sound_idle(
	void)
{
	sound_manager_globals.idling = TRUE;

	if (sound_is_active())
	{
		sound_manager_globals.platform->begin_scene();

		if (!sound_manager_globals.paused)
		{
			sound_update_time();
			update_channels();
		}

		sound_manager_globals.platform->end_scene();
	}

	sound_cache_idle();
	sound_manager_globals.idling = FALSE;

	return;
}

static void prioritize_sounds(
	void)
{
	long sound_index;

	for (sound_index = data_next_index(sound_data, NONE); sound_index != NONE; sound_index = data_next_index(sound_data, sound_index))
	{
		struct sound_datum *sound = sound_get(sound_index);

		if (sound->start_time<=sound_manager_globals.render_time)
		{
			if (sound->playing_channel_index != NONE ||
				_sound_cache_sound_request(TAG_BLOCK_GET_ELEMENT(&TAG_BLOCK_GET_ELEMENT(&sound_definition_get(sound->definition_index)->pitch_ranges, sound->pitch_range_index, struct sound_pitch_range)->permutations, sound->permutation_index, struct sound_permutation), FALSE, TRUE, TRUE))
			{
				short channel_index;

				SET_FLAG(sound->flags, _sound_cached_bit, TRUE);
				channel_index = sound_find_channel(sound_index);

				if (channel_index != NONE)
				{
					struct sound_channel_datum *channel = channel_get(channel_index);

					if (channel->sound_index != sound_index)
					{
						struct sound_definition *definition = sound_definition_get(sound->definition_index);

						match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1602, sound_valid_for_channel(definition->compression, definition->encoding, definition->sample_rate, sound->source.spatialization_mode, channel->type_flags));

						if (channel->sound_index != NONE)
						{
							sound_stop(channel->sound_index);
						}

						channel->sound_index = sound_index;
						sound->start_time = sound_manager_globals.render_time;
					}
					else
					{
						match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1620, sound->playing_channel_index==channel_index);
					}
				}
				else
				{
					sound_stop(sound_index);
				}
			}
			else if (sound->playing_channel_index == NONE && sound->track_proc != track_loop_impulse_sound)
			{
				struct sound_definition *definition = sound_definition_get(sound->definition_index);

				switch (sound_class_get(definition->class_index)->cache_miss_mode)
				{
				case _sound_cache_miss_mode_discard:
				{
					struct sound_pitch_range *range = TAG_BLOCK_GET_ELEMENT(&definition->pitch_ranges, sound->pitch_range_index, struct sound_pitch_range);

					if (range->runtime_discarded_permutation_index == NONE)
					{
						range->runtime_discarded_permutation_index = sound->permutation_index;
					}

					sound_stop(sound_index);
					break;
				}
				case _sound_cache_miss_mode_postpone:
					break;
				default:
					match_halt("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1657);
				}
			}
		}
		else
		{
			match_assert("c:\\halo\\SOURCE\\sound\\sound_manager.c", 1667, TEST_FLAG(sound->flags, _sound_delayed_bit) || sound_class_get(sound_definition_get(sound->definition_index)->class_index)->cache_miss_mode==_sound_cache_miss_mode_postpone);
		}
	}

	return;
}

void sound_dispose_from_old_map(
	void)
{
	if (!sound_manager_globals.paused && sound_manager_globals.initialized && sound_manager_globals.active)
	{
		long sound_index;
		long start_time = system_milliseconds();
		boolean sounds_fading = FALSE;

		for (sound_index = data_next_index(sound_data, NONE); sound_index != NONE; sound_index = data_next_index(sound_data, sound_index))
		{
			sound_start_fade(_sound_fade_mode_linear, 0.3f, NONE, sound_index);
			sounds_fading = TRUE;
		}

		while (sounds_fading && system_milliseconds() < start_time + 300.f)
		{
			sound_idle();
		}
	}

	sound_pause(FALSE);
	sound_stop_all();

	if (looping_sound_data)
	{
		data_delete_all(looping_sound_data);
	}

	return;
}

void sound_render(
	void)
{
	profile_enter(sound_render_section);

	if (sound_is_active())
	{
		sound_manager_globals.platform->begin_scene();

		if (!sound_manager_globals.paused)
		{
			sound_update_time();
			sound_classes_update((long)sound_manager_globals.ticks_elapsed);
			refresh_listener();
			process_looping_sounds();
			refresh_sounds();
			prioritize_sounds();
			update_channels();
			sound_manager_globals.flip_flop = !sound_manager_globals.flip_flop;
		}

		sound_manager_globals.platform->end_scene();
	}

	if (!sound_manager_globals.paused)
	{
		sound_cache_idle();
	}

	profile_exit(sound_render_section);

	return;
}
