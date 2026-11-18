/*
RASTERIZER_XBOX_PROFILE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "rasterizer_xbox.h"
#include "objects.h"
#include "render.h"
#include "bungie_net/common/message_header.h"
#include "unicode.h"
#include "rasterizer.h"
#include "input.h"
#include "bitmaps_inlines.h"
#include "rasterizer_console_vars.h"
#include "light_definitions.h"
#include "shaders.h"
#include "rasterizer/common/rasterizer_common.h"
#include "collisions.h"
#include "input_windows.h"
#include "texture_cache.h"
#include "strings/resource.h"
#include "shell.h"
#include "decals.h"
#include "fog_definitions.h"
#include "rasterizer_cinematics.h"
#include "shell_windows.h"

/* ---------- constants */

enum
{
	MAXIMUM_PROFILE_ERRORS = 3, /* fake name */
	NUMBER_OF_PROFILE_FRAMES = 16, /* fake name */
};

enum
{
	_profile_error_invalid_context_bit = 0, /* fake name */
	_profile_error_begin_out_of_synch_bit, /* fake name */
	_profile_error_end_out_of_synch_bit, /* fake name */
};

enum
{
	_profile_callback_begin_bit = 31, /* fake name */
};

/* ---------- prototypes */

static void profile_assert(short profile, char const *message, boolean condition);
static void __cdecl callback_function(DWORD context);
static void __cdecl frame_callback_function(DWORD context);
static boolean rasterizer_profile_enabled(void);

/* ---------- globals */

static LARGE_INTEGER profile_frequency = { 1 }; /* fake name */
static short current_profile = NONE; /* fake name */
static short profile_window_index = NONE; /* fake name */
static char *rasterizer_profile_names[NUMBER_OF_RASTERIZER_PROFILES] = /* fake name */
{
	"clear",
	"model sky",
	"models",
	"env lightmaps",
	"env shadows",
	"env lights",
	"env decals light",
	"env decals alpha-tested",
	"env textures",
	"env decals primary",
	"env decals secondary",
	"env lights specular",
	"env lightmaps specular",
	"env lightmaps ref.mask",
	"env reflection mirrors",
	"env reflections",
	"env transparent",
	"env fog",
	"env fog screen",
	"water",
	"env decals water",
	"detail objects",
	"queued transparents",
	"lens flare occl. submit",
	"lens flare occl. query",
	"lens flares",
	"screen effect",
	"HUD",
	"screen flash",
};

static __int64 frame_times[NUMBER_OF_PROFILE_FRAMES] = {0}; /* fake name */
static __int64 frame_end_times[NUMBER_OF_PROFILE_FRAMES] = {0}; /* fake name */
static __int64 frame_begin_times[NUMBER_OF_PROFILE_FRAMES] = {0}; /* fake name */
static __int64 profile_pushbuffer_sizes[NUMBER_OF_RASTERIZER_PROFILES] = {0}; /* fake name */
static volatile __int64 profile_times[NUMBER_OF_RASTERIZER_PROFILES] = {0}; /* fake name */
static volatile __int64 profile_begin_times[NUMBER_OF_RASTERIZER_PROFILES] = {0}; /* fake name */
static __int64 frame_pushbuffer_offset = 0; /* fake name */
static __int64 frame_pushbuffer_size = 0; /* fake name */
static short frame_index = 0; /* fake name */
static short last_frame_index = 0; /* fake name */
static short local_profile_enable = 0;
static unsigned long profile_mask = 0; /* fake name */
static volatile short profile_error_flags = 0; /* fake name */
static long profile_unused = 0; /* fake name */
static short profile_error_count = 0; /* fake name */

/* ---------- public code */

static void profile_assert(
	short profile,
	char const *message,
	boolean condition)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 60, message);

	if (!condition && profile_error_count<MAXIMUM_PROFILE_ERRORS)
	{
		if (profile!=NONE)
		{
			error(_error_silent, "### PROFILE (#%d): %s -- tell Bernie!", profile, message);
		}
		else
		{
			error(_error_silent, "### PROFILE: %s -- tell Bernie!", profile, message);
		}

		profile_error_count++;
	}

	return;
}

static void __cdecl callback_function(
	DWORD context)
{
	short profile = (short)context;
	boolean begin = TEST_FLAG(context, _profile_callback_begin_bit);

	if (profile>=0 && profile<NUMBER_OF_RASTERIZER_PROFILES)
	{
		LARGE_INTEGER time;

		QueryPerformanceCounter(&time);

		if (begin)
		{
			if (profile_begin_times[profile])
			{
				SET_FLAG(profile_error_flags, _profile_error_begin_out_of_synch_bit, TRUE);
			}

			profile_begin_times[profile] = time.QuadPart;
		}
		else
		{
			if (profile_begin_times[profile])
			{
				profile_times[profile] = time.QuadPart - profile_begin_times[profile];
				profile_begin_times[profile] = 0;
			}
			else
			{
				SET_FLAG(profile_error_flags, _profile_error_end_out_of_synch_bit, TRUE);
			}
		}
	}
	else
	{
		SET_FLAG(profile_error_flags, _profile_error_invalid_context_bit, TRUE);
	}

	return;
}

static void __cdecl frame_callback_function(
	DWORD context)
{
	short frame = (short)(context >> 1);

	if (frame>=0 && frame<NUMBER_OF_PROFILE_FRAMES)
	{
		LARGE_INTEGER time;

		QueryPerformanceCounter(&time);

		if (TEST_FLAG(context, 0))
		{
			frame_end_times[frame] = time.QuadPart;
			frame_times[frame] = time.QuadPart - frame_begin_times[frame];
			last_frame_index = frame;
		}
		else
		{
			frame_begin_times[frame] = time.QuadPart;
		}
	}

	return;
}

static boolean rasterizer_profile_enabled(
	void)
{
	return rasterizer_debug_options.statistics_mode==_rasterizer_statistics_mode_profile || rasterizer_debug_options.profile_log_enabled;
}

boolean rasterizer_profile_initialize(
	void)
{
	short profile;

	for (profile = 0; profile<NUMBER_OF_RASTERIZER_PROFILES; profile++)
	{
		profile_begin_times[profile] = 0;
		profile_times[profile] = 0;
	}

	memset(frame_begin_times, 0, sizeof(frame_begin_times));
	memset(frame_end_times, 0, sizeof(frame_end_times));
	memset(frame_times, 0, sizeof(frame_times));
	QueryPerformanceFrequency(&profile_frequency);

	return TRUE;
}

void rasterizer_profile_frame_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 194, global_d3d_device);

	profile_assert(NONE, "callback recieved invalid context", !TEST_FLAG(profile_error_flags, _profile_error_invalid_context_bit));
	profile_assert(NONE, "begin out-of-synch", !TEST_FLAG(profile_error_flags, _profile_error_begin_out_of_synch_bit));
	profile_assert(NONE, "end out-of-synch", !TEST_FLAG(profile_error_flags, _profile_error_end_out_of_synch_bit));
	profile_error_flags = 0;

	if (rasterizer_profile_enabled())
	{
		profile_mask = 0;
		profile_window_index = 0;
		current_profile = NONE;
		frame_pushbuffer_offset = 0;
		frame_index = (frame_index + 1) % NUMBER_OF_PROFILE_FRAMES;
		D3DDevice_InsertCallback(D3DCALLBACK_READ, frame_callback_function, frame_index * 2);
	}

	return;
}

void rasterizer_profile_window_begin(
	void)
{
	profile_window_index = global_window_parameters.window_index;
	current_profile = NONE;

	return;
}

void _rasterizer_profile_enable(
	boolean enable)
{
	if (enable)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 244, local_profile_enable>0);
		local_profile_enable--;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 249, local_profile_enable<100);
		local_profile_enable++;
	}

	return;
}

void rasterizer_profile_begin(
	short profile)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 259, global_d3d_device);

	if (rasterizer_profile_enabled() && !profile_window_index && !local_profile_enable)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 265, profile>=0 && profile<NUMBER_OF_RASTERIZER_PROFILES);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 266, global_d3d_device);

		profile_assert(profile, "profile duplication within frame (begin)", !TEST_FLAG(profile_mask, profile));
		profile_assert(profile, "profile begin/end pairing incorrect (begin)", current_profile==NONE);
		D3DDevice_InsertCallback(D3DCALLBACK_READ, callback_function, profile | FLAG(_profile_callback_begin_bit));
		current_profile = profile;
		profile_pushbuffer_sizes[profile] = 0;
	}

	return;
}

void rasterizer_profile_end(
	short profile)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 294, global_d3d_device);

	if (rasterizer_profile_enabled() && !profile_window_index && !local_profile_enable)
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 300, profile>=0 && profile<NUMBER_OF_RASTERIZER_PROFILES);
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 301, global_d3d_device);

		profile_assert(profile, "profile duplication within frame (end)", !TEST_FLAG(profile_mask, profile));
		profile_assert(profile, "profile begin/end pairing incorrect (end)", current_profile==profile);
		D3DDevice_InsertCallback(D3DCALLBACK_WRITE, callback_function, profile);
		profile_pushbuffer_sizes[profile] = 0;
		current_profile = NONE;
		SET_FLAG(profile_mask, profile, TRUE);
	}

	return;
}

char *rasterizer_profile_get_string(
	short profile)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 363, profile>=0 && profile<NUMBER_OF_RASTERIZER_PROFILES);

	return rasterizer_profile_names[profile];
}

real rasterizer_profile_query(
	short profile)
{
	real result = 0.f;

	if (rasterizer_profile_enabled())
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 375, global_d3d_device);

		if (profile==NUMBER_OF_RASTERIZER_PROFILES)
		{
			short profile_index;

			for (profile_index = 0; profile_index<NUMBER_OF_RASTERIZER_PROFILES; profile_index++)
			{
				result += (real)profile_times[profile_index];
			}

			result /= (real)profile_frequency.QuadPart;
		}
		else
		{
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 391, profile>=0 && profile<NUMBER_OF_RASTERIZER_PROFILES);

			profile_assert(profile, "profile not completed (query)", current_profile==NONE);

			if (TEST_FLAG(profile_mask, profile))
			{
				result = (real)profile_times[profile] / (real)profile_frequency.QuadPart;
			}
			else
			{
				result = -1.f;
			}
		}
	}

	return result;
}

long rasterizer_profile_query_pushbuffer(
	short profile)
{
	long result = 0;

	if (rasterizer_profile_enabled())
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 415, global_d3d_device);

		if (profile==NUMBER_OF_RASTERIZER_PROFILES)
		{
			result = 0;
		}
		else
		{
			match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 430, profile>=0 && profile<NUMBER_OF_RASTERIZER_PROFILES);

			profile_assert(profile, "profile not completed (query)", current_profile==NONE);

			if (TEST_FLAG(profile_mask, profile))
			{
				result = (long)MIN(profile_pushbuffer_sizes[profile], LONG_MAX);
			}
			else
			{
				result = NONE;
			}
		}
	}

	return result;
}

void rasterizer_profile_frame_end(
	void)
{
	if (rasterizer_profile_enabled())
	{
		match_assert("c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_profile.c", 455, global_d3d_device);

		frame_pushbuffer_size = (long)MIN(-frame_pushbuffer_offset, LONG_MAX);
		D3DDevice_InsertCallback(D3DCALLBACK_WRITE, frame_callback_function, frame_index * 2 + 1);
#ifdef DEBUG
		profile_rasterizer_stats((real)frame_times[last_frame_index] * 1000.f / (real)profile_frequency.QuadPart, frame_pushbuffer_size);
#endif
	}

	return;
}

void rasterizer_profile_window_end(
	void)
{
	return;
}

void rasterizer_profile_dispose(
	void)
{
	return;
}
