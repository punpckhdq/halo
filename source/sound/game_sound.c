/*
GAME_SOUND.C
*/

/* ---------- headers */

#include "cseries.h"
#include "game_sound.h"
#include "sound_manager.h"
#include "sound_definitions.h"
#include "sound_classes.h"
#include "render.h"
#include "network_game_globals.h"
#include "units.h"
#include "players.h"
#include "game_state.h"
#include "collisions.h"
#include "director.h"
#include "structures.h"
#include "observer.h"
#include "object_types.h"
#include "cinematics.h"
#include "predicted_resources.h"
#include "sound_cache.h"

/* ---------- prototypes */

static void update_potentially_audible_looping_sound(long looping_sound_index, struct location const *location);
static void compute_combined_pas(void);
static boolean location_potentially_audible(struct location const *location);
static void scripted_looping_sound_stop_internal(long definition_index, boolean fixed_fadeout);
static boolean looping_sound_definition_is_music(long definition_index);
static void scripted_music_stop_all(void);

/* ---------- globals */

unsigned long combined_pas[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CLUSTERS_PER_STRUCTURE)];
struct game_sound_global_data *game_sound_globals;
struct data_array *game_looping_sound_data;

/* ---------- public code */

void game_sound_initialize(
	void)
{
	game_looping_sound_data = game_state_data_new("object looping sounds", MAXIMUM_GAME_LOOPING_SOUNDS_PER_MAP, sizeof(struct game_looping_sound_datum));
	game_sound_globals = game_state_malloc("game sound globals", NULL, sizeof(struct game_sound_global_data));

	return;
}

void game_sound_dispose(
	void)
{
	if (game_looping_sound_data)
	{
		game_looping_sound_data = NULL;
	}

	return;
}

void game_sound_initialize_for_new_map(
	void)
{
	if (game_looping_sound_data)
	{
		data_make_valid(game_looping_sound_data);
		game_sound_globals->background_loop_index = NONE;
		game_sound_globals->frame_index = 0;
	}

	return;
}

void game_sound_clear(
	void)
{
	long looping_sound_index;

	for (looping_sound_index = data_next_index(game_looping_sound_data, NONE);
		looping_sound_index != NONE;
		looping_sound_index = data_next_index(game_looping_sound_data, looping_sound_index))
	{
		struct game_looping_sound_datum *sound = game_looping_sound_get(looping_sound_index);
		struct looping_sound_definition *definition = looping_sound_definition_get(sound->definition_index);

		if (definition->runtime_scripting_sound_index == looping_sound_index)
		{
			match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 176, TEST_FLAG(sound->flags, _game_looping_sound_scripted_bit));
			definition->runtime_scripting_sound_index = NONE;
		}
		else if (definition->runtime_scripting_sound_index != NONE)
		{
			struct game_looping_sound_datum *sound = game_looping_sound_get(definition->runtime_scripting_sound_index);

			match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 184, sound->definition_index==sound->definition_index);
		}
	}

	return;
}

void game_sound_restore(
	void)
{
	struct tag_iterator the_devil;
	long looping_sound_index;
	long definition_index;

	for (looping_sound_index = data_next_index(game_looping_sound_data, NONE);
		looping_sound_index != NONE;
		looping_sound_index = data_next_index(game_looping_sound_data, looping_sound_index))
	{
		struct game_looping_sound_datum *sound = game_looping_sound_get(looping_sound_index);

		if (TEST_FLAG(sound->flags, _game_looping_sound_scripted_bit))
		{
			struct looping_sound_definition *definition = looping_sound_definition_get(sound->definition_index);

			if (!TEST_FLAG(definition->flags, _looping_sound_fake_impulse_sound_bit))
			{
				definition->runtime_scripting_sound_index = looping_sound_index;
			}
			else
			{
				datum_delete(game_looping_sound_data, looping_sound_index);
			}
		}
	}

	tag_iterator_new(&the_devil, SOUND_DEFINITION_TAG);
	while ((definition_index = tag_iterator_next(&the_devil)) != NONE)
	{
		sound_definition_get(definition_index)->runtime_scripting_time = NONE;
	}

	return;
}

long game_looping_sound_new(
	long object_index,
	long definition_index,
	char const *marker_name,
	short function_index)
{
	long looping_sound_index = NONE;

	match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 240, marker_name);

	if (definition_index != NONE)
	{
		struct object_marker marker;

		if (object_index == NONE || object_get_marker_by_name(object_index, marker_name, &marker, 1))
		{
			looping_sound_index = datum_new(game_looping_sound_data);
			if (looping_sound_index != NONE)
			{
				struct game_looping_sound_datum *sound = game_looping_sound_get(looping_sound_index);

				sound->object_index = object_index;
				sound->definition_index = definition_index;
				sound->state = _game_looping_sound_inactive;
				sound->flags = 0;
				sound->attachment.function_index = function_index;
				sound->last_audible_frame_index = NONE;
				if (object_index != NONE)
				{
					sound->attachment.node_index = marker.node_index;
					sound->attachment.position = marker.node_matrix.position;
					sound->attachment.forward = marker.node_matrix.forward;
				}
			}
		}
	}

	return looping_sound_index;
}

void game_looping_sound_delete(
	long looping_sound_index)
{
	struct game_looping_sound_datum *sound = game_looping_sound_get(looping_sound_index);
	struct looping_sound_definition *definition = looping_sound_definition_get(sound->definition_index);

	match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 281, definition->runtime_scripting_sound_index!=looping_sound_index);
	match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 282, definition->runtime_scripting_sound_index==NONE || game_looping_sound_get(definition->runtime_scripting_sound_index));

	datum_delete(game_looping_sound_data, looping_sound_index);

	return;
}

long unattached_impulse_sound_new(
	long definition_index,
	struct sound_location const *location,
	real scale)
{
	struct sound_source source;

	match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 329, location);
	match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 330, scale>=0.f && scale<=1.f);

	source.location = *location;
	source.scale = scale;
	source.spatialization_mode = _sound_spatialization_mode_absolute;
	source.gain = 1.f;

	return sound_new_impulse(definition_index, &source, NONE, NULL, NULL, 0);
}

long unspatialized_impulse_sound_new(
	long definition_index,
	real scale)
{
	struct sound_source source;

	match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 346, scale>=0.f && scale<=1.f);

	source.spatialization_mode = _sound_spatialization_mode_none;
	source.scale = scale;
	source.gain = 1.f;

	return sound_new_impulse(definition_index, &source, NONE, NULL, NULL, 0);
}

long scripted_sound_time(
	long definition_index)
{
	long time = 0;

	if (definition_index != NONE)
	{
		struct sound_definition *definition = sound_definition_get(definition_index);

		if (definition->runtime_scripting_time != NONE)
		{
			time = definition->runtime_scripting_time - game_time_get();
			time = MAX(time, 0);
		}
	}

	return time;
}

void scripted_sound_stop(
	long definition_index)
{
	if (definition_index != NONE)
	{
		struct sound_definition *definition = sound_definition_get(definition_index);

		if (definition->runtime_scripting_sound_index != NONE)
		{
			sound_stop_impulse(definition->runtime_scripting_sound_index);
			definition->runtime_scripting_sound_index = NONE;
			definition->runtime_scripting_time = NONE;
		}
	}

	return;
}

void scripted_foley_predict(
	long definition_index)
{
	if (definition_index != NONE)
	{
		short track_index;
		struct looping_sound_definition *definition = looping_sound_definition_get(definition_index);

		for (track_index = 0; track_index<definition->tracks.count; track_index++)
		{
			struct looping_sound_track *track = TAG_BLOCK_GET_ELEMENT(&definition->tracks, track_index, struct looping_sound_track);

			if (track->loop_sound.index != NONE)
			{
				struct sound_definition *sound = sound_definition_get(track->loop_sound.index);

				if (sound->pitch_ranges.count == 1)
				{
					struct sound_pitch_range *range = TAG_BLOCK_GET_ELEMENT(&sound->pitch_ranges, 0, struct sound_pitch_range);

					if (range->permutations.count)
					{
						struct sound_permutation *permutation = TAG_BLOCK_GET_ELEMENT(&range->permutations, 0, struct sound_permutation);

						_sound_cache_sound_request(permutation, FALSE, TRUE, FALSE);
					}
				}
			}
		}
	}

	return;
}

void scripted_looping_sound_set_scale(
	long definition_index,
	real scale)
{
	if (definition_index != NONE)
	{
		struct looping_sound_definition *definition = looping_sound_definition_get(definition_index);

		if (definition->runtime_scripting_sound_index != NONE)
		{
			struct game_looping_sound_datum *sound = game_looping_sound_get(definition->runtime_scripting_sound_index);

			sound->scale = PIN(scale, 0.f, 1.f);
		}
	}

	return;
}

void scripted_looping_sound_set_alternate(
	long definition_index,
	boolean alternate)
{
	if (definition_index != NONE)
	{
		struct looping_sound_definition *definition = looping_sound_definition_get(definition_index);

		if (definition->runtime_scripting_sound_index != NONE)
		{
			struct game_looping_sound_datum *sound = game_looping_sound_get(definition->runtime_scripting_sound_index);

			SET_FLAG(sound->flags, _game_looping_sound_alternate_bit, alternate);
		}
	}

	return;
}

long unattached_looping_sound_start(
	long definition_index,
	long source_object_index,
	real scale)
{
	struct looping_sound_definition *definition = looping_sound_definition_get(definition_index);
	long looping_sound_index = game_looping_sound_new(source_object_index, definition_index, "", NONE);

	if (looping_sound_index != NONE)
	{
		struct game_looping_sound_datum *sound = game_looping_sound_get(looping_sound_index);

		SET_FLAG(sound->flags, _game_looping_sound_unattached_bit, TRUE);
		sound->scale = scale;
	}

	return looping_sound_index;
}

void unattached_looping_sound_stop(
	long looping_sound_index)
{
	struct game_looping_sound_datum *sound = game_looping_sound_get(looping_sound_index);

	SET_FLAG(sound->flags, _game_looping_sound_unattached_stop_bit, TRUE);

	return;
}

static void update_potentially_audible_looping_sound(
	long looping_sound_index,
	struct location const *location)
{
	struct sound_source source;
	boolean playing;
	struct game_looping_sound_datum *sound = game_looping_sound_get(looping_sound_index);
	struct looping_sound_definition *definition = looping_sound_definition_get(sound->definition_index);
	boolean audible_last_frame = sound->last_audible_frame_index==NONE || sound->last_audible_frame_index==game_sound_globals->frame_index-1;

	if (!TEST_FLAG(sound->flags, _game_looping_sound_unattached_bit))
	{
		playing = object_get_function_value(sound->object_index, sound->attachment.function_index, &source.scale);
	}
	else
	{
		playing = !TEST_FLAG(sound->flags, _game_looping_sound_unattached_stop_bit);
		source.scale = sound->scale;
	}

	if (playing || (sound->state != _game_looping_sound_inactive && audible_last_frame))
	{
		if (sound->object_index != NONE)
		{
			real_vector3d unused_velocity;
			real_matrix4x3 *matrix = object_get_node_matrix(sound->object_index, sound->attachment.node_index);

			match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 619, location);
			matrix4x3_transform_point(matrix, &sound->attachment.position, &source.location.position);
			matrix4x3_transform_normal(matrix, &sound->attachment.forward, &source.location.forward);
			object_get_velocities(sound->object_index, &source.location.translational_velocity, &unused_velocity);
			source.location.game_location = *location;
			source.spatialization_mode = _sound_spatialization_mode_absolute;
		}
		else
		{
			source.spatialization_mode = _sound_spatialization_mode_none;
		}
		source.gain = 1.f;

		if (playing)
		{
			short refresh_state;

			if (sound->state == _game_looping_sound_active || !audible_last_frame)
			{
				refresh_state = _looping_sound_refresh_loop;
				sound->state = _game_looping_sound_active;
			}
			else
			{
				refresh_state = _looping_sound_refresh_start;
				sound->state = _game_looping_sound_active;
			}

			if (sound_refresh_looping(sound->definition_index, looping_sound_index, &source, refresh_state, TEST_FLAG(sound->flags, _game_looping_sound_alternate_bit), 0.f))
			{
				match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 657, TEST_FLAG(definition->flags, _looping_sound_fake_impulse_sound_bit));
				if (TEST_FLAG(sound->flags, _game_looping_sound_unattached_bit))
				{
					if (definition->runtime_scripting_sound_index == looping_sound_index)
					{
						definition->runtime_scripting_sound_index = NONE;
					}
					game_looping_sound_delete(looping_sound_index);
				}
				else
				{
					sound->state = _game_looping_sound_inactive;
				}
			}
		}
		else if (!audible_last_frame ||
			sound_refresh_looping(sound->definition_index, looping_sound_index, &source, _looping_sound_refresh_stop, TEST_FLAG(sound->flags, _game_looping_sound_alternate_bit), TEST_FLAG(sound->flags, _game_looping_sound_unattached_stop_fixed_fadeout_bit) ? 4.f : 0.f))
		{
			if (TEST_FLAG(sound->flags, _game_looping_sound_unattached_bit))
			{
				game_looping_sound_delete(looping_sound_index);
			}
			else
			{
				sound->state = _game_looping_sound_inactive;
			}
		}
		else
		{
			sound->state = _game_looping_sound_deactivating;
		}
	}
	else if (sound->state != _game_looping_sound_inactive)
	{
		sound->state = _game_looping_sound_inactive;
	}

	sound->last_audible_frame_index = game_sound_globals->frame_index;

	return;
}

boolean track_object_impulse_sound(
	long object_index,
	struct sound_attachment_data const *attachment_data,
	struct sound_source *source)
{
	struct object_datum *object = object_try_and_get(object_index);

	match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 794, attachment_data);
	match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 795, source);

	if (object)
	{
		struct location location;

		object_get_location(object_index, &location);
		if (location.cluster_index != NONE)
		{
			real_matrix4x3 *matrix = object_get_node_matrix(object_index, attachment_data->node_index==NONE ? 0 : attachment_data->node_index);

			source->location.game_location = location;
			matrix4x3_transform_point(matrix, &attachment_data->position, &source->location.position);
			matrix4x3_transform_normal(matrix, &attachment_data->forward, &source->location.forward);
			object_get_velocities(object_index, &source->location.translational_velocity, NULL);

			return TRUE;
		}
	}

	return FALSE;
}

void game_sound_set_mouth_aperture(
	long object_index,
	real mouth_aperture)
{
	if (game_looping_sound_data->valid && unit_try_and_get(object_index))
	{
		unit_set_mouth_aperture(object_index, mouth_aperture);
	}

	return;
}

static void compute_combined_pas(
	void)
{
	short local_player_index;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	memset(combined_pas, 0, BIT_VECTOR_SIZE_IN_BYTES(structure_bsp->clusters.count));
	for (local_player_index = 0; local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		if (local_player_get_player_index(local_player_index) != NONE)
		{
			struct observer_result const *camera = observer_get_camera(local_player_index);

			if (camera->location.cluster_index != NONE)
			{
				short cluster_index;

				for (cluster_index = 0; cluster_index<structure_bsp->clusters.count; cluster_index++)
				{
					real distance = (structure_bsp_get_cluster_encoded_sound_distance(structure_bsp, cluster_index, camera->location.cluster_index) & ~0x80) * 256.f / 127.f;

					if (distance<256.f)
					{
						BIT_VECTOR_SET_FLAG(combined_pas, cluster_index, TRUE);
					}
				}
			}
		}
	}

	return;
}

static boolean location_potentially_audible(
	struct location const *location)
{
	match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 870, location->cluster_index>=NONE && location->cluster_index<global_structure_bsp_get()->clusters.count);

	return location->cluster_index != NONE && BIT_VECTOR_TEST_FLAG(combined_pas, location->cluster_index);
}

static void scripted_looping_sound_stop_internal(
	long definition_index,
	boolean fixed_fadeout)
{
	if (definition_index != NONE)
	{
		struct looping_sound_definition *definition = looping_sound_definition_get(definition_index);

		if (definition->runtime_scripting_sound_index != NONE)
		{
			struct game_looping_sound_datum *sound = game_looping_sound_get(definition->runtime_scripting_sound_index);

			SET_FLAG(sound->flags, _game_looping_sound_scripted_bit, FALSE);
			unattached_looping_sound_stop(definition->runtime_scripting_sound_index);
			definition->runtime_scripting_sound_index = NONE;
			if (fixed_fadeout)
			{
				SET_FLAG(sound->flags, _game_looping_sound_unattached_stop_fixed_fadeout_bit, TRUE);
			}
		}
	}

	return;
}

static boolean looping_sound_definition_is_music(
	long definition_index)
{
	short track_index;
	struct looping_sound_definition *definition = looping_sound_definition_get(definition_index);

	for (track_index = 0; track_index<definition->tracks.count; track_index++)
	{
		struct looping_sound_track *track = TAG_BLOCK_GET_ELEMENT(&definition->tracks, track_index, struct looping_sound_track);

		if (track->loop_sound.index != NONE && sound_definition_get(track->loop_sound.index)->class_index == _sound_class_music)
		{
			return TRUE;
		}
	}

	return FALSE;
}

static void scripted_music_stop_all(
	void)
{
	long looping_sound_index;

	for (looping_sound_index = data_next_index(game_looping_sound_data, NONE);
		looping_sound_index != NONE;
		looping_sound_index = data_next_index(game_looping_sound_data, looping_sound_index))
	{
		struct game_looping_sound_datum *sound = game_looping_sound_get(looping_sound_index);

		if (sound->object_index == NONE && looping_sound_definition_is_music(sound->definition_index))
		{
			scripted_looping_sound_stop_internal(sound->definition_index, TRUE);
		}
	}

	return;
}

void game_sound_dispose_from_old_map(
	void)
{
	if (game_looping_sound_data && game_looping_sound_data->valid)
	{
		game_sound_clear();
		data_make_invalid(game_looping_sound_data);
	}

	return;
}

long object_impulse_sound_new(
	long object_index,
	long definition_index,
	short node_index,
	real_point3d const *position,
	real_vector3d const *forward,
	real scale)
{
	struct sound_source source;
	struct sound_attachment_data attachment_data;
	long sound_index = NONE;

	match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 301, position && forward);
	match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 302, scale>=0.f && scale<=1.f);

	source.spatialization_mode = _sound_spatialization_mode_absolute;
	source.gain = 1.f;
	attachment_data.position = *position;
	attachment_data.forward = *forward;
	attachment_data.node_index = node_index;
	source.location.game_location.cluster_index = NONE;
	if (track_object_impulse_sound(object_index, &attachment_data, &source))
	{
		source.scale = scale;
		sound_index = sound_new_impulse(definition_index, &source, object_index, track_object_impulse_sound, &attachment_data, sizeof(attachment_data));
	}

	return sound_index;
}

void scripted_sound_new(
	long definition_index,
	long source_object_index,
	real scale)
{
	if (definition_index != NONE)
	{
		long sound_index;
		struct sound_definition *definition = sound_definition_get(definition_index);

		sound_stop_impulse(definition->runtime_scripting_sound_index);
		definition->runtime_scripting_time = game_time_get() + definition->runtime_maximum_play_time*TICKS_PER_SECOND/1000;
		scale = PIN(scale, 0.f, 1.f);

		if (source_object_index != NONE)
		{
			struct object_marker head_marker;
			real_point3d node_position;
			real_vector3d node_forward;
			short node_index;

			if (object_get_marker_by_name(source_object_index, "head", &head_marker, 1))
			{
				node_index = head_marker.node_index;
				node_position = head_marker.node_matrix.position;
				node_forward = head_marker.node_matrix.forward;
			}
			else
			{
				node_index = 0;
				node_position = *global_origin3d;
				node_forward = *global_forward3d;
			}

			sound_index = object_impulse_sound_new(source_object_index, definition_index, node_index, &node_position, &node_forward, scale);
			if (sound_index != NONE)
			{
				object_type_notify_impulse_sound(source_object_index, definition_index, sound_index);
			}
		}
		else
		{
			sound_index = unspatialized_impulse_sound_new(definition_index, scale);
		}

		definition->runtime_scripting_sound_index = sound_index;
	}

	return;
}

void scripted_looping_sound_stop(
	long definition_index)
{
	scripted_looping_sound_stop_internal(definition_index, FALSE);

	return;
}

void game_sound_update(
	real dt)
{
	boolean crossed_water_boundary;
	long desired_background_definition_index;
	struct sound_environment *sound_environment;
	struct location location;
	long looping_sound_index;

	scenario_get_sound_environment(&desired_background_definition_index, &sound_environment, &crossed_water_boundary);
	sound_manager_set_sound_environment(sound_environment);
	compute_combined_pas();

	if (desired_background_definition_index == NONE)
	{
		if (game_sound_globals->background_loop_index != NONE)
		{
			unattached_looping_sound_stop(game_sound_globals->background_loop_index);
			game_sound_globals->background_loop_index = NONE;
		}
	}
	else if (game_sound_globals->background_loop_index != NONE)
	{
		if (game_looping_sound_get(game_sound_globals->background_loop_index)->definition_index != desired_background_definition_index)
		{
			unattached_looping_sound_stop(game_sound_globals->background_loop_index);
			game_sound_globals->background_loop_index = unattached_looping_sound_start(desired_background_definition_index, NONE, 1.f);
		}
	}
	else
	{
		game_sound_globals->background_loop_index = unattached_looping_sound_start(desired_background_definition_index, NONE, 1.f);
	}

	for (looping_sound_index = data_next_index(game_looping_sound_data, NONE);
		looping_sound_index != NONE;
		looping_sound_index = data_next_index(game_looping_sound_data, looping_sound_index))
	{
		struct game_looping_sound_datum *sound = game_looping_sound_get(looping_sound_index);

		if (sound->object_index == NONE)
		{
			update_potentially_audible_looping_sound(looping_sound_index, NULL);
		}
		else if (TEST_FLAG(sound->flags, _game_looping_sound_unattached_bit) && !object_try_and_get(sound->object_index))
		{
			struct looping_sound_definition *definition = looping_sound_definition_get(sound->definition_index);

			if (definition->runtime_scripting_sound_index == looping_sound_index)
			{
				definition->runtime_scripting_sound_index = NONE;
			}
			game_looping_sound_delete(looping_sound_index);
		}
		else if (TEST_FLAG(object_get(sound->object_index)->object.flags, _object_connected_to_map_bit))
		{
			object_get_location(sound->object_index, &location);
			if (location_potentially_audible(&location))
			{
				update_potentially_audible_looping_sound(looping_sound_index, &location);
			}
		}
	}

	game_sound_globals->frame_index++;

	return;
}

void compute_sound_obstruction(
	short local_player_index,
	struct sound_source *source,
	real distance)
{
	struct observer_result const *camera = observer_get_camera(local_player_index);

	match_collision_log_begin_user("c:\\halo\\SOURCE\\sound\\game_sound.c", 882, _collision_user_sounds);

	source->obstruction = 0.6f;
	source->occlusion = 1.f;
	match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 887, source->spatialization_mode==_sound_spatialization_mode_absolute);

	if (source->location.game_location.cluster_index != NONE && camera->location.cluster_index != NONE)
	{
		struct structure_bsp *structure_bsp = global_structure_bsp_get();
		byte encoded_distance = structure_bsp_get_cluster_encoded_sound_distance(structure_bsp, camera->location.cluster_index, source->location.game_location.cluster_index);
		real cluster_distance = (encoded_distance & ~0x80) * 256.f / 127.f;

		if (cluster_distance<256.f)
		{
			if (BIT_VECTOR_TEST_FLAG(structure_bsp_get_cluster_pvs(global_structure_bsp_get(), camera->location.cluster_index), source->location.game_location.cluster_index))
			{
				real_vector3d listener_to_source;
				struct collision_result collision;

				source->obstruction = 0.45f;
				vector_from_points3d(&camera->position, &source->location.position, &listener_to_source);
				if (!collision_test_vector(_collision_test_environment_flags | FLAG(_collision_test_objects_bit) | FLAG(_collision_test_objects_scenery_bit) | FLAG(_collision_test_objects_machines_bit),
					&camera->position, &listener_to_source, NONE, &collision))
				{
					source->obstruction = 0.f;
					source->occlusion = 0.f;
				}
			}

			if (source->obstruction != 0.f)
			{
				source->occlusion = 1.f - distance / (cluster_distance + distance);
				source->occlusion = PIN(source->occlusion * 1.4f, 0.f, 1.f);
			}
		}
	}

	match_collision_log_end_user("c:\\halo\\SOURCE\\sound\\game_sound.c", 926);

	return;
}

void scripted_looping_sound_start(
	long definition_index,
	long source_object_index,
	real scale)
{
	if (definition_index != NONE)
	{
		struct looping_sound_definition *definition = looping_sound_definition_get(definition_index);

		scripted_looping_sound_stop(definition_index);
		match_assert("c:\\halo\\SOURCE\\sound\\game_sound.c", 495, definition->runtime_scripting_sound_index==NONE);

		if (TEST_FLAG(definition->flags, _looping_sound_stops_music_bit))
		{
			scripted_music_stop_all();
		}

		definition->runtime_scripting_sound_index = unattached_looping_sound_start(definition_index, source_object_index, scale);
		if (definition->runtime_scripting_sound_index != NONE)
		{
			struct game_looping_sound_datum *sound = game_looping_sound_get(definition->runtime_scripting_sound_index);

			SET_FLAG(sound->flags, _game_looping_sound_scripted_bit, TRUE);
		}
	}

	return;
}
