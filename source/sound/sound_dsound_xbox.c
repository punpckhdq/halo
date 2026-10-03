/*
SOUND_DSOUND_XBOX.C
*/

/* ---------- headers */

#include "cseries.h"
#include "sound_manager.h"
#include "sound_definitions.h"
#include "platform_sound.h"
#include "sound_preferences.h"
#include "ai_scenario_definitions.h"
#include "game_globals.h"
#include "interface.h"
#include "damage.h"
#include "breakable_surfaces.h"
#include "scenario.h"
#include "network_game_globals.h"
#include "sound_cache.h"
#include "game_sound.h"
#include "sound_dsound.h"
#include "physical_memory_map.h"
#include "draw_string.h"
#include "render_debug.h"
#include <dsstdfx.h>

#include "sound_dsound_xbox_effects_image.h"

/* ---------- constants */

enum
{
	MAXIMUM_QUEUED_PACKETS_PER_CHANNEL = 4, /* fake name */
	SOUND_CACHE_SIZE = 0x400000, /* fake name */
	MAXIMUM_INTERRUPT_ERROR_LENGTH = 256, /* fake name */
	INANITY_BUFFER_SIZE = 32, /* fake name */
};

/* ---------- macros */

#define PACKET_SIZE(type_flags) /* fake name */ \
	((TEST_FLAG(type_flags, _sound_channel_compressed_bit) ? 2304 : 8192)* \
	(TEST_FLAG(type_flags, _sound_channel_44k_bit) ? 2.f : 1.f)* \
	(TEST_FLAG(type_flags, _sound_channel_stereo_bit) ? 2.f : 1.f))

#define REAL_CMP_EPSILON(new_value, old_value, epsilon) /* fake name */ \
	(!(fabs((new_value)-(old_value))<(epsilon)))

/* ---------- structures */

struct dsound_virtual_channel /* fake name */
{
	short channel_index;
	short type_index;
};

struct dsound_channel /* fake name */
{
	short state;
	short virtual_channel_index;
	boolean spatialized; /* fake name */
	boolean underwater; /* fake name */
	boolean stopping;
	short queued_packet_count; /* fake name */
	struct sound_location location; /* fake name */
	short type_flags;
	real gain; /* fake name */
	real pitch; /* fake name */
	real obstruction; /* fake name */
	real occlusion; /* fake name */
	real minimum_distance; /* fake name */
	real maximum_distance; /* fake name */
	real outer_cone_gain; /* fake name */
	real inner_cone_angle; /* fake name */
	real outer_cone_angle; /* fake name */
	real reverb_damping_factor; /* fake name */
	long sample_offset; /* fake name */
	struct sound_permutation *playing_permutation; /* fake name */
	struct sound_permutation *queued_permutation; /* fake name */
	LPDIRECTSOUNDSTREAM stream;
};

struct dsound_globals_definition
{
	boolean initialized; /* fake name */
	short virtual_channel_count;
	struct dsound_virtual_channel virtual_channels[MAXIMUM_SOUND_CHANNELS]; /* fake name */
	short actual_channel_count;
	struct dsound_channel actual_channels[MAXIMUM_SOUND_CHANNELS]; /* fake name */
	short first_channel_index[NUMBER_OF_SOUND_CHANNEL_TYPES]; /* fake name */
	struct platform_sound_listener_properties listener; /* fake name */
	struct sound_environment sound_environment; /* fake name */
	DSCAPS caps; /* fake name */
	LPDIRECTSOUND dsound_object;
	LPDIRECTSOUNDBUFFER inanity_buffer; /* fake name */
	byte inanity_buffer_data[INANITY_BUFFER_SIZE]; /* fake name */
	boolean paused;
	real pause_gain;
};

/* ---------- prototypes */

extern void WINAPI DirectSoundStopStream(LPDIRECTSOUNDSTREAM stream);
extern BOOL WINAPI DirectSoundGetStreamVoiceStatus(LPDIRECTSOUNDSTREAM stream);

static struct dsound_channel *channel_get(short index);
static struct dsound_virtual_channel *vchannel_get(short index); /* fake name */
static void interrupt_time_error(HRESULT const *result, char const *string);
static boolean vchannel_new(short virtual_channel_index, short type_index); /* fake name */
static boolean dsound_initialize(struct sound_preferences *preferences);
static void dsound_dispose(void);
static void set_listener_properties_dsound(struct platform_sound_listener_properties const *properties);
static void begin_scene_dsound(void);
static void end_scene_dsound(void);
static void dsound_virtual_queue(short virtual_channel_index, struct sound_permutation *sound);
static void dsound_virtual_update(short virtual_channel_index);
static void dsound_virtual_stop(short virtual_channel_index);
static short dsound_virtual_get_state(short virtual_channel_index);
static void pause_dsound(boolean paused);
static void flush_dsound(void);
static void dsound_virtual_set_location(short virtual_channel_index, boolean spatialize, struct sound_location const *location, real obstruction, real occlusion, boolean underwater);
static void dsound_virtual_set_properties(short virtual_channel_index, struct platform_sound_channel_properties const *properties, boolean gain_only);
static void channel_update_i3dl2_source(short channel_index); /* fake name */
static void channel_stop(short channel_index); /* fake name */
static boolean channel_stop_finished(short channel_index); /* fake name */
static short channel_get_state(short channel_index); /* fake name */
static boolean channel_queue_packet(short channel_index); /* fake name */
static void dsound_error(HRESULT result, char const *format, ...); /* fake name */
static void vchannel_find_channel(short virtual_channel_index); /* fake name */
static short dsound_virtual_touch(short virtual_channel_index);
static boolean create_inanity_channel(void); /* fake name */
static void channel_set_properties(short channel_index, struct platform_sound_channel_properties const *properties, boolean gain_only); /* fake name */
static void channel_queue_packets(short channel_index); /* fake name */
static void CALLBACK channel_packet_completion_callback(LPVOID stream_context, LPVOID packet_context, DWORD status); /* fake name */
static void channel_set_location(short channel_index, struct sound_location const *location, boolean spatialized, real obstruction, real occlusion, boolean underwater); /* fake name */
static void channel_queue_sound(short channel_index, struct sound_permutation *permutation); /* fake name */
static boolean channel_new(short channel_index, short type_flags); /* fake name */

/* ---------- globals */

struct platform_sound_manager_definition platform_sound_dsound=
{
	_platform_sound_dsound,
	dsound_initialize,
	dsound_dispose,
	set_listener_properties_dsound,
	begin_scene_dsound,
	end_scene_dsound,
	dsound_virtual_queue,
	dsound_virtual_update,
	dsound_virtual_stop,
	dsound_virtual_get_state,
	pause_dsound,
	flush_dsound,
	dsound_virtual_set_location,
	dsound_virtual_set_properties
};

static real underwater_direct_gain = 0.25f; /* fake name */

static char interrupt_error_string[MAXIMUM_INTERRUPT_ERROR_LENGTH+1]; /* fake name */

boolean debug_sound_channels;
HRESULT interrupt_result;
struct dsound_globals_definition dsound_globals;

/* ---------- public code */

LPDIRECTSOUND dsound_get(
	void)
{
	return dsound_globals.initialized ? dsound_globals.dsound_object : NULL;
}

/* ---------- private code */

static struct dsound_channel *channel_get(
	short index)
{
	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 105, index>=0 && index<dsound_globals.actual_channel_count);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 106, index<MAXIMUM_SOUND_CHANNELS);

	return &dsound_globals.actual_channels[index];
}

static struct dsound_virtual_channel *vchannel_get(
	short index)
{
	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 114, index>=0 && index<dsound_globals.virtual_channel_count);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 115, index<MAXIMUM_SOUND_CHANNELS);

	return &dsound_globals.virtual_channels[index];
}

static void interrupt_time_error(
	HRESULT const *result,
	char const *string)
{
	if (result)
	{
		interrupt_result = *result;
	}

	if (strlen(interrupt_error_string) + strlen(string)<MAXIMUM_INTERRUPT_ERROR_LENGTH)
	{
		sprintf(interrupt_error_string + strlen(interrupt_error_string), string);
	}

	return;
}

static boolean dsound_initialize(
	struct sound_preferences *preferences)
{
	boolean success = FALSE;
	HRESULT result;

	dsound_globals.initialized = FALSE;
	dsound_globals.paused = FALSE;
	dsound_globals.pause_gain = 1.f;
	dsound_globals.inanity_buffer = NULL;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 234, preferences);

	result = DirectSoundCreate(NULL, &dsound_globals.dsound_object, NULL);
	if (SUCCEEDED(result))
	{
		DSCAPS caps;

		result = IDirectSound_GetCaps(dsound_globals.dsound_object, &caps);
		if (SUCCEEDED(result))
		{
			dsound_globals.caps = caps;

			result = IDirectSound_SetDistanceFactor(dsound_globals.dsound_object, METERS_PER_UNIT, DS3D_IMMEDIATE);
			if (SUCCEEDED(result))
			{
				result = IDirectSound_SetRolloffFactor(dsound_globals.dsound_object, 1.f, DS3D_IMMEDIATE);
				if (SUCCEEDED(result))
				{
					struct platform_sound_listener_properties listener;
					short type_index;
					short virtual_channel_index;
					short actual_channel_index;

					memset(&listener, 0, sizeof(listener));
					listener.forward = *global_forward3d;
					listener.up = *global_up3d;
					listener.sound_environment = &default_sound_environment;

					{
						DSEFFECTIMAGELOC image_location;
						LPDSEFFECTIMAGEDESC image_description;

						image_location.dwI3DL2ReverbIndex = I3DL2_CHAIN_I3DL2_REVERB;
						image_location.dwCrosstalkIndex = I3DL2_CHAIN_XTALK;
						result = IDirectSound_DownloadEffectsImage(dsound_globals.dsound_object, dsound_effects_image, sizeof(dsound_effects_image), &image_location, &image_description);
						if (FAILED(result))
						{
							dsound_error(result, "could not download effects image.");
						}
					}

					IDirectSound_SetMixBinHeadroom(dsound_globals.dsound_object, 0x7fffffff, 0);
					DirectSoundUseFullHRTF();
					set_listener_properties_dsound(&listener);

					success = TRUE;

					virtual_channel_index = 0;
					for (type_index = 0; type_index<NUMBER_OF_SOUND_CHANNEL_TYPES; type_index++)
					{
						short channel_index;

						for (channel_index = 0; channel_index<preferences->virtual_channel_counts[type_index]; channel_index++)
						{
							dsound_globals.virtual_channel_count++;
							success = success && vchannel_new(virtual_channel_index++, type_index);
						}
					}

					actual_channel_index = 0;
					for (type_index = 0; type_index<NUMBER_OF_SOUND_CHANNEL_TYPES; type_index++)
					{
						short channel_index;

						dsound_globals.first_channel_index[type_index] = actual_channel_index;
						for (channel_index = 0; channel_index<preferences->actual_channel_counts[type_index]; channel_index++)
						{
							dsound_globals.actual_channel_count++;
							success = success && channel_new(actual_channel_index++, sound_channel_type_flags[type_index]);
						}
					}

					success = success && create_inanity_channel();
				}
				else
				{
					dsound_error(result, "could not adjust rolloff factor");
				}
			}
			else
			{
				dsound_error(result, "could not adjust distance factor");
			}
		}
		else
		{
			dsound_error(result, "could not get caps for sound card?");
		}
	}
	else
	{
		dsound_error(result, "could not create direct sound object");
	}

	if (success)
	{
		dsound_globals.initialized = TRUE;
	}
	else
	{
		dsound_dispose();
	}

	return success;
}

static boolean vchannel_new(
	short virtual_channel_index,
	short type_index)
{
	struct dsound_virtual_channel *vchannel = vchannel_get(virtual_channel_index);

	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 422, type_index>=0 && type_index<NUMBER_OF_SOUND_CHANNEL_TYPES);

	vchannel->type_index = type_index;
	vchannel->channel_index = NONE;

	return TRUE;
}

static void dsound_dispose(
	void)
{
	long channel_index;

	for (channel_index = 0; channel_index<dsound_globals.actual_channel_count; channel_index++)
	{
		struct dsound_channel *channel = channel_get((short)channel_index);

		if (channel->stream)
		{
			IDirectSoundStream_Release(channel->stream);
		}
	}

	if (dsound_globals.inanity_buffer)
	{
		IDirectSoundBuffer_Release(dsound_globals.inanity_buffer);
	}

	dsound_globals.actual_channel_count = 0;
	dsound_globals.virtual_channel_count = 0;

	if (dsound_globals.dsound_object)
	{
		IDirectSound_Release(dsound_globals.dsound_object);
		dsound_globals.dsound_object = NULL;
	}

	dsound_globals.initialized = FALSE;

	return;
}

static void set_listener_properties_dsound(
	struct platform_sound_listener_properties const *properties)
{
	HRESULT result;

	if (REAL_CMP_EPSILON(properties->position.x, dsound_globals.listener.position.x, 0.05f) ||
		REAL_CMP_EPSILON(properties->position.y, dsound_globals.listener.position.y, 0.05f) ||
		REAL_CMP_EPSILON(properties->position.z, dsound_globals.listener.position.z, 0.05f) ||
		!dsound_globals.initialized)
	{
		result = IDirectSound_SetPosition(dsound_globals.dsound_object, properties->position.x, (properties->position.z), (properties->position.y), DS3D_DEFERRED);
		if (FAILED(result))
		{
			dsound_error(result, "couldn't set listener position.");
		}
		dsound_globals.listener.position = properties->position;
	}

	if (REAL_CMP_EPSILON(properties->forward.i, dsound_globals.listener.forward.i, 0.05f) ||
		REAL_CMP_EPSILON(properties->forward.j, dsound_globals.listener.forward.j, 0.05f) ||
		REAL_CMP_EPSILON(properties->forward.k, dsound_globals.listener.forward.k, 0.05f) ||
		REAL_CMP_EPSILON(properties->up.i, dsound_globals.listener.up.i, 0.05f) ||
		REAL_CMP_EPSILON(properties->up.j, dsound_globals.listener.up.j, 0.05f) ||
		REAL_CMP_EPSILON(properties->up.k, dsound_globals.listener.up.k, 0.05f) ||
		!dsound_globals.initialized)
	{
		result = IDirectSound_SetOrientation(dsound_globals.dsound_object,
			properties->forward.i, (properties->forward.k), (properties->forward.j),
			properties->up.i, (properties->up.k), (properties->up.j),
			DS3D_DEFERRED);
		if (FAILED(result))
		{
			dsound_error(result, "couldn't set listener orientation.");
		}
		dsound_globals.listener.forward = properties->forward;
		dsound_globals.listener.up = properties->up;
	}

	if (REAL_CMP_EPSILON(properties->translational_velocity.i, dsound_globals.listener.translational_velocity.i, 0.01f) ||
		REAL_CMP_EPSILON(properties->translational_velocity.j, dsound_globals.listener.translational_velocity.j, 0.01f) ||
		REAL_CMP_EPSILON(properties->translational_velocity.k, dsound_globals.listener.translational_velocity.k, 0.01f) ||
		!dsound_globals.initialized)
	{
		result = IDirectSound_SetVelocity(dsound_globals.dsound_object, properties->translational_velocity.i, (properties->translational_velocity.k), (properties->translational_velocity.j), DS3D_DEFERRED);
		if (FAILED(result))
		{
			dsound_error(result, "couldn't set listener velocity.");
		}
		dsound_globals.listener.translational_velocity = properties->translational_velocity;
	}

	if (memcmp(properties->sound_environment, &dsound_globals.sound_environment, sizeof(struct sound_environment)) || !dsound_globals.initialized)
	{
		struct sound_environment const *environment = properties->sound_environment;
		DSI3DL2LISTENER listener;

		dsound_globals.sound_environment = *environment;

		listener.lRoom = dsound_volume_from_gain(environment->room_intensity, 0);
		listener.lRoomHF = dsound_volume_from_gain(environment->room_intensity_hf, 0);
		listener.flRoomRolloffFactor = environment->room_rolloff_factor;
		listener.flDecayTime = environment->decay_time;
		listener.flDecayHFRatio = environment->decay_hf_ratio;
		listener.lReflections = dsound_volume_from_gain(environment->reflections_intensity, 1000);
		listener.flReflectionsDelay = environment->reflections_delay;
		listener.lReverb = dsound_volume_from_gain(environment->reverb_intensity, 2000);
		listener.flReverbDelay = environment->reverb_delay;
		listener.flDiffusion = environment->diffusion*100.f;
		listener.flDensity = environment->density*100.f;
		listener.flHFReference = environment->hf_reference;
		IDirectSound_SetI3DL2Listener(dsound_globals.dsound_object, &listener, DS3D_DEFERRED);
	}

	return;
}

static boolean create_inanity_channel(
	void)
{
	boolean success = FALSE;
	WAVEFORMATEX wfm;
	DSBUFFERDESC desc;
	HRESULT result;

	wfm.wFormatTag = WAVE_FORMAT_PCM;
	wfm.wBitsPerSample = 16;
	wfm.nChannels = 1;
	wfm.nBlockAlign = 2;
	wfm.nSamplesPerSec = 22050;
	wfm.nAvgBytesPerSec = 44100;

	memset(&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = 0;
	desc.dwBufferBytes = INANITY_BUFFER_SIZE;
	desc.lpwfxFormat = &wfm;
	desc.dwMixBinMask = DSMIXBIN_XTLK_BACK_LEFT | DSMIXBIN_XTLK_BACK_RIGHT | DSMIXBIN_I3DL2 | DSMIXBIN_FXSEND_0 | DSMIXBIN_FXSEND_1;

	result = IDirectSound_CreateSoundBuffer(dsound_globals.dsound_object, &desc, &dsound_globals.inanity_buffer, NULL);
	if (SUCCEEDED(result))
	{
		memset(dsound_globals.inanity_buffer_data, 0, INANITY_BUFFER_SIZE);
		IDirectSoundBuffer_SetBufferData(dsound_globals.inanity_buffer, dsound_globals.inanity_buffer_data, INANITY_BUFFER_SIZE);

		result = IDirectSoundBuffer_Play(dsound_globals.inanity_buffer, 0, 0, DSBPLAY_LOOPING);
		if (SUCCEEDED(result))
		{
			success = TRUE;
		}
		else
		{
			dsound_error(result, "failed to start the inanity channel.");
		}
	}
	else
	{
		dsound_error(result, "failed to create the inanity channel.");
	}

	return success;
}

static void begin_scene_dsound(
	void)
{
	DirectSoundDoWork();

	if (strlen(interrupt_error_string))
	{
		dsound_error(interrupt_result, interrupt_error_string);
	}
	interrupt_error_string[0] = 0;
	interrupt_result = S_OK;

	return;
}

static void end_scene_dsound(
	void)
{
	HRESULT result;

	result = IDirectSound_CommitDeferredSettings(dsound_globals.dsound_object);
	if (FAILED(result))
	{
		dsound_error(result, "couldn't commit deferred settings.");
	}

	if ((dsound_globals.paused && dsound_globals.pause_gain!=0.f) || (!dsound_globals.paused && dsound_globals.pause_gain!=1.f))
	{
		match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 634, dsound_globals.pause_gain>=0 && dsound_globals.pause_gain<=1.f);

		if (dsound_globals.paused)
		{
			dsound_globals.pause_gain = MAX(0.0, dsound_globals.pause_gain - 0.15);
		}
		else
		{
			dsound_globals.pause_gain = MIN(1.0, dsound_globals.pause_gain + 0.15);
		}

		match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 645, dsound_globals.pause_gain>=0 && dsound_globals.pause_gain<=1.f);

		if (dsound_globals.paused)
		{
			short channel_index;

			for (channel_index = 0; channel_index<dsound_globals.actual_channel_count; channel_index++)
			{
				struct dsound_channel *channel = channel_get(channel_index);

				if (channel->state!=_sound_channel_idle)
				{
					IDirectSoundStream_SetVolume(channel->stream, dsound_volume_from_gain(dsound_globals.pause_gain*channel->gain, 0));
				}
			}
		}
	}

	if (debug_sound_channels)
	{
		short tab_stops[3] = {280};
		char buffer[8192];
		short channel_index;

		draw_string_set_tab_stops(tab_stops, 1);
		buffer[0] = 0;
		for (channel_index = 0; channel_index<dsound_globals.actual_channel_count; channel_index++)
		{
			struct dsound_channel *channel = channel_get(channel_index);

			if (channel_index<16 || channel_index>48)
			{
				if (channel->state!=_sound_channel_idle)
				{
					sprintf(buffer + strlen(buffer), "%d %1.2f %1.2f %s(%s)", channel->queued_packet_count, channel->gain, channel->pitch,
						channel->playing_permutation ? channel->playing_permutation->name : "",
						channel->queued_permutation ? channel->queued_permutation->name : "");
				}
				sprintf(buffer + strlen(buffer), "|t");
				if (channel_index & 1)
				{
					sprintf(buffer + strlen(buffer), "|n");
				}
			}
		}
		render_debug_string(FALSE, buffer);
	}

	return;
}

static void flush_dsound(
	void)
{
	short channel_index;

	for (channel_index = 0; channel_index<dsound_globals.actual_channel_count; channel_index++)
	{
		struct dsound_channel *channel = channel_get(channel_index);

		match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 706, channel->stopping || channel->state==_sound_channel_idle);
		if (channel->stopping)
		{
			while (!channel_stop_finished(channel_index));
		}

		if (channel->queued_packet_count)
		{
			error(_error_silent, "DirectSound: you're screwed if you try to save and quit -- the devil.");
			channel->queued_packet_count = 0;
		}
	}

	return;
}

static void pause_dsound(
	boolean paused)
{
	short channel_index;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 728, paused!=dsound_globals.paused);

	if (paused)
	{
		for (channel_index = 0; channel_index<dsound_globals.actual_channel_count; channel_index++)
		{
			struct dsound_channel *channel = channel_get(channel_index);

			if (channel->stopping)
			{
				while (!channel_stop_finished(channel_index));
			}
		}
	}
	else
	{
		for (channel_index = 0; channel_index<dsound_globals.actual_channel_count; channel_index++)
		{
			struct dsound_channel *channel = channel_get(channel_index);

			if (channel->state!=_sound_channel_idle)
			{
				IDirectSoundStream_Flush(channel->stream);
				channel->sample_offset-= PACKET_SIZE(channel->type_flags)*MAXIMUM_QUEUED_PACKETS_PER_CHANNEL;
				channel->sample_offset = FLOOR(channel->sample_offset, 0);
				channel->gain = 0.f;
			}
		}

		DirectSoundDoWork();

		for (channel_index = 0; channel_index<dsound_globals.actual_channel_count; channel_index++)
		{
			struct dsound_channel *channel = channel_get(channel_index);

			if (channel->queued_packet_count)
			{
				error(_error_silent, "DirectSound: you're screwed if you try to save and quit -- the devil.");
				channel->queued_packet_count = 0;
			}

			if (channel->state!=_sound_channel_idle)
			{
				channel_queue_packets(channel_index);
			}
		}
	}

	dsound_globals.paused = paused;

	return;
}

static void dsound_virtual_set_location(
	short virtual_channel_index,
	boolean spatialize,
	struct sound_location const *location,
	real obstruction,
	real occlusion,
	boolean underwater)
{
	short channel_index = dsound_virtual_touch(virtual_channel_index);

	if (channel_index!=NONE)
	{
		channel_set_location(channel_index, location, spatialize, obstruction, occlusion, underwater);
	}

	return;
}

static void channel_set_location(
	short channel_index,
	struct sound_location const *location,
	boolean spatialized,
	real obstruction,
	real occlusion,
	boolean underwater)
{
	struct dsound_channel *channel = channel_get(channel_index);
	boolean mode_changed = FALSE;
	HRESULT result;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 906, TEST_FLAG(channel->type_flags, _sound_channel_3d_bit));
	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 907, channel->stream);

	if (channel->spatialized!=spatialized || !dsound_globals.initialized)
	{
		result = IDirectSoundStream_SetMode(channel->stream, spatialized ? DS3DMODE_NORMAL : DS3DMODE_DISABLE, DS3D_DEFERRED);
		channel->spatialized = spatialized;
		mode_changed = TRUE;
		if (FAILED(result))
		{
			dsound_error(result, "couldn't set channel spatialization.");
		}
	}

	if (REAL_CMP_EPSILON(location->position.x, channel->location.position.x, 0.05f) ||
		REAL_CMP_EPSILON(location->position.y, channel->location.position.y, 0.05f) ||
		REAL_CMP_EPSILON(location->position.z, channel->location.position.z, 0.05f) ||
		!dsound_globals.initialized)
	{
		result = IDirectSoundStream_SetPosition(channel->stream, location->position.x, (location->position.z), (location->position.y), DS3D_DEFERRED);
		if (FAILED(result))
		{
			dsound_error(result, "couldn't set channel position.");
		}
		channel->location.position = location->position;
	}

	if (REAL_CMP_EPSILON(location->forward.i, channel->location.forward.i, 0.05f) ||
		REAL_CMP_EPSILON(location->forward.j, channel->location.forward.j, 0.05f) ||
		REAL_CMP_EPSILON(location->forward.k, channel->location.forward.k, 0.05f) ||
		!dsound_globals.initialized)
	{
		match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 933, valid_real_normal3d(&location->forward));

		result = IDirectSoundStream_SetConeOrientation(channel->stream, location->forward.i, (location->forward.k), (location->forward.j), DS3D_DEFERRED);
		if (FAILED(result))
		{
			dsound_error(result, "couldn't set channel orientation.");
		}
		channel->location.forward = location->forward;
	}

	if (REAL_CMP_EPSILON(location->translational_velocity.i, channel->location.translational_velocity.i, 0.01f) ||
		REAL_CMP_EPSILON(location->translational_velocity.j, channel->location.translational_velocity.j, 0.01f) ||
		REAL_CMP_EPSILON(location->translational_velocity.k, channel->location.translational_velocity.k, 0.01f) ||
		!dsound_globals.initialized)
	{
		result = IDirectSoundStream_SetVelocity(channel->stream, location->translational_velocity.i, (location->translational_velocity.k), (location->translational_velocity.j), DS3D_DEFERRED);
		if (FAILED(result))
		{
			dsound_error(result, "couldn't set channel velocity.");
		}
		channel->location.translational_velocity = location->translational_velocity;
	}

	if (REAL_CMP_EPSILON(obstruction, channel->obstruction, 0.001f) ||
		REAL_CMP_EPSILON(occlusion, channel->occlusion, 0.001f) ||
		channel->underwater!=underwater ||
		mode_changed ||
		!dsound_globals.initialized)
	{
		channel->obstruction = obstruction;
		channel->occlusion = occlusion;
		channel->underwater = underwater;
		channel_update_i3dl2_source(channel_index);
	}

	return;
}

static void dsound_virtual_set_properties(
	short virtual_channel_index,
	struct platform_sound_channel_properties const *properties,
	boolean gain_only)
{
	short channel_index = dsound_virtual_touch(virtual_channel_index);

	if (channel_index!=NONE)
	{
		channel_set_properties(channel_index, properties, gain_only);
	}

	return;
}

static void channel_set_properties(
	short channel_index,
	struct platform_sound_channel_properties const *properties,
	boolean gain_only)
{
	struct dsound_channel *channel = channel_get(channel_index);
	real gain = dsound_globals.pause_gain*properties->gain;
	HRESULT result;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 980, properties->gain>=0.f && properties->gain<=1.f);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 981, dsound_globals.pause_gain>=0 && dsound_globals.pause_gain<=1.f);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 982, channel->stream);

	if (REAL_CMP_EPSILON(gain, channel->gain, 0.001f) || !dsound_globals.initialized)
	{
		result = IDirectSoundStream_SetVolume(channel->stream, dsound_volume_from_gain(gain, 0));
		if (FAILED(result))
		{
			dsound_error(result, "couldn't set channel volume.");
		}
		channel->gain = gain;
	}

	if (!gain_only)
	{
		if (REAL_CMP_EPSILON(properties->pitch, channel->pitch, 0.001f) || !dsound_globals.initialized)
		{
			long samples_per_second = sound_samples_per_second(TEST_FLAG(channel->type_flags, _sound_channel_44k_bit));

			result = IDirectSoundStream_SetFrequency(channel->stream, dsound_frequency_from_pitch(samples_per_second, properties->pitch));
			if (FAILED(result))
			{
				dsound_error(result, "couldn't set channel pitch.");
			}
			channel->pitch = properties->pitch;
		}

		if (TEST_FLAG(channel->type_flags, _sound_channel_3d_bit))
		{
			if (REAL_CMP_EPSILON(properties->maximum_distance, channel->maximum_distance, 0.05f) || !dsound_globals.initialized)
			{
				result = IDirectSoundStream_SetMaxDistance(channel->stream, properties->maximum_distance, DS3D_DEFERRED);
				if (FAILED(result))
				{
					dsound_error(result, "couldn't set channel max distance.");
				}
				channel->maximum_distance = properties->maximum_distance;
			}

			if (REAL_CMP_EPSILON(properties->minimum_distance, channel->minimum_distance, 0.05f) || !dsound_globals.initialized)
			{
				result = IDirectSoundStream_SetMinDistance(channel->stream, properties->minimum_distance, DS3D_DEFERRED);
				if (FAILED(result))
				{
					dsound_error(result, "couldn't set channel min distance.");
				}
				channel->minimum_distance = properties->minimum_distance;
			}

			if (REAL_CMP_EPSILON(properties->inner_cone_angle, channel->inner_cone_angle, 0.034906585f) ||
				REAL_CMP_EPSILON(properties->outer_cone_angle, channel->outer_cone_angle, 0.034906585f) ||
				!dsound_globals.initialized)
			{
				result = IDirectSoundStream_SetConeAngles(channel->stream, dsound_angle_from_angle(properties->inner_cone_angle), dsound_angle_from_angle(properties->outer_cone_angle), DS3D_IMMEDIATE);
				if (FAILED(result))
				{
					dsound_error(result, "couldn't set channel cone angles.");
				}
				channel->inner_cone_angle = properties->inner_cone_angle;
				channel->outer_cone_angle = properties->outer_cone_angle;
			}

			if (REAL_CMP_EPSILON(properties->outer_cone_gain, channel->outer_cone_gain, 0.001f) || !dsound_globals.initialized)
			{
				result = IDirectSoundStream_SetConeOutsideVolume(channel->stream, dsound_volume_from_gain(properties->outer_cone_gain, 0), DS3D_DEFERRED);
				if (FAILED(result))
				{
					dsound_error(result, "couldn't set channel cone volume.");
				}
				channel->outer_cone_gain = properties->outer_cone_gain;
			}

			if (REAL_CMP_EPSILON(properties->reverb_damping_factor, channel->reverb_damping_factor, 0.001f) || !dsound_globals.initialized)
			{
				channel->reverb_damping_factor = properties->reverb_damping_factor;
				channel_update_i3dl2_source(channel_index);
			}
		}
	}

	return;
}

static void channel_update_i3dl2_source(
	short channel_index)
{
	struct dsound_channel *channel = channel_get(channel_index);
	DSI3DL2BUFFER source;

	memset(&source, 0, sizeof(source));
	source.lDirect = 0;
	source.lDirectHF = 0;
	source.flRoomRolloffFactor = 0.f;
	source.Obstruction.flLFRatio = 0.f;
	source.Occlusion.flLFRatio = 0.2f;

	if (channel->spatialized)
	{
		real room_gain = 1.f - channel->reverb_damping_factor;

		if (channel->underwater)
		{
			source.lDirectHF = dsound_volume_from_gain(underwater_direct_gain, 0);
			source.lDirect = dsound_volume_from_gain(underwater_direct_gain, 0);
		}
		else
		{
			room_gain*= 0.5f;
		}

		source.lRoomHF = source.lRoom = dsound_volume_from_gain(room_gain, 0);
		source.Obstruction.lHFLevel = dsound_obstruction_from_obstruction(channel->occlusion);
		source.Occlusion.lHFLevel = dsound_occlusion_from_occlusion(channel->obstruction);
	}
	else
	{
		source.lRoomHF = source.lRoom = DSBVOLUME_MIN;
		source.Obstruction.lHFLevel = 0;
		source.Occlusion.lHFLevel = 0;
	}

	IDirectSoundStream_SetI3DL2Source(channel->stream, &source, DS3D_DEFERRED);

	return;
}

static void dsound_virtual_queue(
	short virtual_channel_index,
	struct sound_permutation *sound)
{
	short channel_index = dsound_virtual_touch(virtual_channel_index);

	if (channel_index!=NONE)
	{
		channel_queue_sound(channel_index, sound);
	}

	return;
}

static void channel_queue_sound(
	short channel_index,
	struct sound_permutation *sound)
{
	struct dsound_channel *channel = channel_get(channel_index);
	HRESULT result;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 1111, !dsound_globals.paused);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 1112, sound);

	switch (channel->state)
	{
	case _sound_channel_idle:
		channel->state = _sound_channel_playing;
		channel->playing_permutation = sound;
		channel->sample_offset = 0;
		channel->queued_packet_count = 0;

		result = IDirectSound_CommitDeferredSettings(dsound_globals.dsound_object);
		if (FAILED(result))
		{
			dsound_error(result, "couldn't commit deferred settings.");
		}

		channel_queue_packets(channel_index);
		break;

	case _sound_channel_playing:
		channel->state = _sound_channel_full;
	case _sound_channel_full:
		channel->queued_permutation = sound;
		break;

	default:
		match_vhalt("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 1144, "bad DirectSound channel state.");
	}

	return;
}

static void channel_queue_packets(
	short channel_index)
{
	struct dsound_channel *channel = channel_get(channel_index);

	while (channel->queued_packet_count<MAXIMUM_QUEUED_PACKETS_PER_CHANNEL && channel->playing_permutation)
	{
		DWORD status;
		HRESULT result = IDirectSoundStream_GetStatus(channel->stream, &status);

		if (FAILED(result))
		{
			interrupt_time_error(&result, "couldn't get channel status.");
			break;
		}

		if (!(status & XMO_STATUSF_ACCEPT_INPUT_DATA) || !channel_queue_packet(channel_index))
		{
			break;
		}
	}

	return;
}

static boolean channel_queue_packet(
	short channel_index)
{
	struct dsound_channel *channel = channel_get(channel_index);
	boolean success = FALSE;

	if (channel->playing_permutation)
	{
		if (channel->playing_permutation->cache_base_address)
		{
			if ((byte *)channel->playing_permutation->cache_base_address>=(byte *)physical_memory_get_sound_cache_base_address() &&
				(byte *)channel->playing_permutation->cache_base_address + channel->playing_permutation->samples.size<=(byte *)physical_memory_get_sound_cache_base_address() + SOUND_CACHE_SIZE)
			{
				struct sound_permutation *permutation = channel->playing_permutation;
				long sample_offset = channel->sample_offset;
				long remaining_size = permutation->samples.size - sample_offset;
				XMEDIAPACKET packet;
				HRESULT result;

				channel->queued_packet_count++;

				packet.pvBuffer = (byte *)permutation->cache_base_address + sample_offset;
				packet.pdwCompletedSize = NULL;
				packet.pdwStatus = NULL;
				packet.pContext = permutation;
				packet.prtTimestamp = NULL;

				if (remaining_size<=PACKET_SIZE(channel->type_flags))
				{
					if (sample_offset)
					{
						packet.dwMaxSize = remaining_size;
						channel->playing_permutation = channel->queued_permutation;
						channel->queued_permutation = NULL;
						channel->sample_offset = 0;
						if (channel->state==_sound_channel_full)
						{
							channel->state = _sound_channel_playing;
						}
					}
					else
					{
						long block_size = 36*(TEST_FLAG(channel->type_flags, _sound_channel_stereo_bit) ? 2 : 1);

						channel->sample_offset = MAX(remaining_size/block_size/2, 1)*block_size;
						packet.dwMaxSize = channel->sample_offset;
					}
				}
				else
				{
					packet.dwMaxSize = (long)PACKET_SIZE(channel->type_flags);
					channel->sample_offset+= packet.dwMaxSize;
				}

				if (packet.dwMaxSize)
				{
					result = IDirectSoundStream_Process(channel->stream, &packet, NULL);
					if (SUCCEEDED(result))
					{
						sound_cache_sound_hardware_lock(permutation);
						success = TRUE;
					}
					else
					{
						interrupt_time_error(&result, "couldn't queue sound packet.");
					}
				}
			}
			else
			{
				sprintf(temporary, "trying to queue sound %s but it's outside the valid range. (%ld)", channel->playing_permutation->name, channel->playing_permutation->cache_base_address);
				interrupt_time_error(NULL, temporary);
			}
		}
		else
		{
			interrupt_time_error(NULL, "trying to queue sound but samples is null.");
		}
	}
	else
	{
		interrupt_time_error(NULL, "trying to queue sound but sound is null.");
	}

	return success;
}

static void CALLBACK channel_packet_completion_callback(
	LPVOID stream_context,
	LPVOID packet_context,
	DWORD status)
{
	short channel_index = (short)stream_context;

	if (channel_index>=0 && channel_index<dsound_globals.actual_channel_count)
	{
		struct dsound_channel *channel = channel_get(channel_index);

		if (status==XMEDIAPACKET_STATUS_SUCCESS || status==XMEDIAPACKET_STATUS_FLUSHED)
		{
			sound_cache_sound_hardware_unlock((struct sound_permutation *)packet_context);
			channel->queued_packet_count--;

			if (!dsound_globals.paused)
			{
				if (!channel->queued_packet_count)
				{
					channel->state = _sound_channel_idle;
				}
				else if (status!=XMEDIAPACKET_STATUS_FLUSHED)
				{
					channel_queue_packets(channel_index);
				}
			}
		}
		else if (status==XMEDIAPACKET_STATUS_FAILURE)
		{
			interrupt_time_error(NULL, "status is failure.");
		}
		else if (status==XMEDIAPACKET_STATUS_PENDING)
		{
			interrupt_time_error(NULL, "status is pending.");
		}
		else
		{
			interrupt_time_error(NULL, "status is undefined.");
		}
	}
	else
	{
		interrupt_time_error(NULL, "trying to queue sound to invalid channel.");
	}

	return;
}

static void channel_stop(
	short channel_index)
{
	struct dsound_channel *channel = channel_get(channel_index);

	if (channel->state!=_sound_channel_idle)
	{
		DirectSoundStopStream(channel->stream);
		channel->stopping = TRUE;
		channel->state = _sound_channel_idle;
	}
	channel->playing_permutation = NULL;
	channel->queued_permutation = NULL;

	return;
}

static boolean channel_stop_finished(
	short channel_index)
{
	struct dsound_channel *channel = channel_get(channel_index);
	boolean finished;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 1212, channel->stopping);

	finished = !DirectSoundGetStreamVoiceStatus(channel->stream);
	if (finished)
	{
		IDirectSoundStream_Flush(channel->stream);
		channel->stopping = FALSE;
	}

	return finished;
}

static short channel_get_state(
	short channel_index)
{
	return channel_get(channel_index)->state;
}

static void dsound_error(
	HRESULT result,
	char const *format,
	...)
{
	char temporary[4096];
	char const *error_string = "<unknown error>";
	va_list arguments;

	va_start(arguments, format);
	vsprintf(temporary, format, arguments);
	va_end(arguments);

	switch (result)
	{
	case DSERR_CONTROLUNAVAIL:
		error_string = "DSERR_CONTROLUNAVAIL";
		break;
	case DSERR_INVALIDCALL:
		error_string = "DSERR_INVALIDCALL";
		break;
	case DSERR_NODRIVER:
		error_string = "DSERR_NODRIVER";
		break;
	case DSERR_OUTOFMEMORY:
		error_string = "DSERR_OUTOFMEMORY";
		break;
	case DSERR_UNSUPPORTED:
		error_string = "DSERR_UNSUPPORTED";
		break;
	case DSERR_GENERIC:
		error_string = "DSERR_GENERIC";
		break;
	case DSERR_NOAGGREGATION:
		error_string = "DSERR_NOAGGREGATION";
		break;
	}

	error(_error_silent, "DirectSound:  '%s' (%s#%d)", temporary, error_string);

	return;
}

static void vchannel_find_channel(
	short virtual_channel_index)
{
	struct dsound_virtual_channel *vchannel = vchannel_get(virtual_channel_index);
	short channel_index;

	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 1420, vchannel->channel_index==NONE);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 1421, vchannel->type_index>=0 && vchannel->type_index<NUMBER_OF_SOUND_CHANNEL_TYPES);

	for (channel_index = dsound_globals.first_channel_index[vchannel->type_index];
		vchannel->channel_index==NONE && channel_index<dsound_globals.actual_channel_count;
		channel_index++)
	{
		struct dsound_channel *channel = channel_get(channel_index);

		if (channel->type_flags!=sound_channel_type_flags[vchannel->type_index])
		{
			break;
		}

		if (channel->virtual_channel_index==NONE && (!channel->stopping || channel_stop_finished(channel_index)))
		{
			vchannel->channel_index = channel_index;
		}
	}

	if (vchannel->channel_index!=NONE)
	{
		channel_get(vchannel->channel_index)->virtual_channel_index = virtual_channel_index;
	}
	else
	{
		error(_error_silent, "WARNING: ran out of actual sound channels of type %d", vchannel->type_index);
	}

	return;
}

static short dsound_virtual_touch(
	short virtual_channel_index)
{
	struct dsound_virtual_channel *vchannel = vchannel_get(virtual_channel_index);

	if (vchannel->channel_index==NONE)
	{
		vchannel_find_channel(virtual_channel_index);
	}

	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 1468, vchannel->channel_index==NONE || channel_get(vchannel->channel_index)->type_flags==sound_channel_type_flags[vchannel->type_index]);
	match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 1469, vchannel->channel_index==NONE || channel_get(vchannel->channel_index)->virtual_channel_index==virtual_channel_index);

	return vchannel->channel_index;
}

static void dsound_virtual_update(
	short virtual_channel_index)
{
	return;
}

static void dsound_virtual_stop(
	short virtual_channel_index)
{
	struct dsound_virtual_channel *vchannel = vchannel_get(virtual_channel_index);

	if (vchannel->channel_index!=NONE)
	{
		match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 1505, channel_get(vchannel->channel_index)->virtual_channel_index==virtual_channel_index);

		channel_stop(vchannel->channel_index);
		channel_get(vchannel->channel_index)->virtual_channel_index = NONE;
		vchannel->channel_index = NONE;
	}

	return;
}

static short dsound_virtual_get_state(
	short virtual_channel_index)
{
	struct dsound_virtual_channel *vchannel = vchannel_get(virtual_channel_index);
	short state;

	if (vchannel->channel_index!=NONE)
	{
		match_assert("c:\\halo\\SOURCE\\sound\\sound_dsound_xbox.c", 1527, channel_get(vchannel->channel_index)->virtual_channel_index==virtual_channel_index);

		state = channel_get_state(vchannel->channel_index);
	}
	else
	{
		state = _sound_channel_idle;
	}

	return state;
}

static boolean channel_new(
	short channel_index,
	short type_flags)
{
	struct dsound_channel *channel = channel_get(channel_index);
	boolean success = FALSE;
	XBOXADPCMWAVEFORMAT wfm;
	DSSTREAMDESC desc;
	HRESULT result;

	channel->type_flags = type_flags;
	channel->virtual_channel_index = NONE;
	channel->stopping = FALSE;
	channel->playing_permutation = NULL;
	channel->queued_permutation = NULL;

	if (!TEST_FLAG(type_flags, _sound_channel_compressed_bit))
	{
		wfm.wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfm.wfx.wBitsPerSample = 16;
		wfm.wfx.nChannels = 2;
		wfm.wfx.nBlockAlign = 4;
		wfm.wfx.nSamplesPerSec = sound_sample_rate_samples_per_second[_sound_sample_rate_44k];
		wfm.wfx.nAvgBytesPerSec = wfm.wfx.nSamplesPerSec*wfm.wfx.nBlockAlign;
	}
	else
	{
		wfm.wfx.wFormatTag = WAVE_FORMAT_XBOX_ADPCM;
		wfm.wfx.wBitsPerSample = 4;
		wfm.wfx.nChannels = TEST_FLAG(type_flags, _sound_channel_stereo_bit) ? 2 : 1;
		wfm.wfx.nBlockAlign = 36*wfm.wfx.nChannels;
		wfm.wfx.nSamplesPerSec = sound_samples_per_second(TEST_FLAG(type_flags, _sound_channel_44k_bit));
		wfm.wfx.nAvgBytesPerSec = wfm.wfx.nSamplesPerSec/64*wfm.wfx.nBlockAlign;
		wfm.wfx.cbSize = 2;
		wfm.wSamplesPerBlock = 64;
	}

	memset(&desc, 0, sizeof(desc));
	desc.dwMaxAttachedPackets = MAXIMUM_QUEUED_PACKETS_PER_CHANNEL;
	desc.lpwfxFormat = (LPWAVEFORMATEX)&wfm;
	desc.dwFlags = 0;
	desc.lpfnCallback = channel_packet_completion_callback;
	desc.lpvContext = (LPVOID)channel_index;
	if (TEST_FLAG(type_flags, _sound_channel_3d_bit))
	{
		desc.dwFlags = DSSTREAMCAPS_CTRL3D;
	}

	result = IDirectSound_CreateSoundStream(dsound_globals.dsound_object, &desc, &channel->stream, NULL);
	if (SUCCEEDED(result))
	{
		struct platform_sound_channel_properties properties;

		if (TEST_FLAG(type_flags, _sound_channel_3d_bit))
		{
			struct sound_location location;

			memset(&location, 0, sizeof(location));
			location.forward = *global_forward3d;
			channel_set_location(channel_index, &location, FALSE, 0.f, 0.f, FALSE);
		}
		else
		{
			DWORD speaker_configuration;
			LONG mix_bin_volumes[8];
			DWORD mix_bins = 0;

			IDirectSound_GetSpeakerConfig(dsound_globals.dsound_object, &speaker_configuration);
			if (speaker_configuration & DSSPEAKER_ENABLE_AC3)
			{
				if (!TEST_FLAG(type_flags, _sound_channel_stereo_bit))
				{
					mix_bins = DSMIXBIN_FRONT_LEFT | DSMIXBIN_FRONT_RIGHT | DSMIXBIN_FRONT_CENTER;
					mix_bin_volumes[0] = dsound_volume_from_gain(0.5f, 0);
					mix_bin_volumes[1] = dsound_volume_from_gain(0.5f, 0);
					mix_bin_volumes[2] = dsound_volume_from_gain(0.5f, 0);
				}
				else
				{
					mix_bins = DSMIXBIN_FRONT_LEFT | DSMIXBIN_FRONT_RIGHT | DSMIXBIN_BACK_LEFT | DSMIXBIN_BACK_RIGHT | DSMIXBIN_FXSEND_0 | DSMIXBIN_FXSEND_1;
					mix_bin_volumes[0] = dsound_volume_from_gain(1.f, 0);
					mix_bin_volumes[1] = dsound_volume_from_gain(1.f, 0);
					mix_bin_volumes[2] = dsound_volume_from_gain(0.5f, 0);
					mix_bin_volumes[3] = dsound_volume_from_gain(0.5f, 0);
					mix_bin_volumes[4] = dsound_volume_from_gain(0.5f, 0);
					mix_bin_volumes[5] = dsound_volume_from_gain(0.5f, 0);
				}
			}
			else if (!TEST_FLAG(type_flags, _sound_channel_stereo_bit))
			{
				mix_bins = DSMIXBIN_FRONT_LEFT | DSMIXBIN_FRONT_RIGHT;
				mix_bin_volumes[0] = dsound_volume_from_gain(0.5f, 0);
				mix_bin_volumes[1] = dsound_volume_from_gain(0.5f, 0);
			}

			if (mix_bins)
			{
				IDirectSoundStream_SetMixBins(channel->stream, mix_bins);
				IDirectSoundStream_SetMixBinVolumes(channel->stream, mix_bins, mix_bin_volumes);
			}
		}

		success = TRUE;

		memset(&properties, 0, sizeof(properties));
		properties.minimum_distance = 1.f;
		properties.maximum_distance = 1.f;
		channel_set_properties(channel_index, &properties, FALSE);
	}
	else
	{
		dsound_error(result, "couldn't create sound stream.");
	}

	return success;
}
