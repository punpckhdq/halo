/*
BINK_PLAYBACK.C
*/

/* ---------- headers */

#include "cseries.h"
#include "bink_playback.h"
#include "bink.h"
#include "bitmaps.h"
#include "bitmap_macros.h"
#include "cache_files.h"
#include "texture_cache.h"
#include "render.h"
#include "rasterizer.h"
#include "shader_definitions.h"
#include "draw_string.h"
#include "input.h"
#include "interface.h"
#include "event_manager.h"
#include "attract_mode.h"
#include "ui_widget.h"
#include "main.h"
#include "sound_dsound.h"

/* ---------- constants */

enum
{
	MAXIMUM_BINK_POINTER_BLOCKS = 16, /* fake name */
	BINK_MEMORY_POOL_SIZE = 0x400000, /* fake name */
	BINK_MEMORY_POOL_EXTRA_SIZE = 0xC00000, /* fake name */
	BINK_IO_BUFFER_SIZE = 0xC00000, /* fake name */
	BINK_MEMORY_GUARD_SIZE = 0x3000, /* fake name */
	BINK_FRAME_BUFFER_ALIGNMENT = 128, /* fake name */
	CPU_PAGE_SIZE = 4096,
};

/* ---------- macros */

/* ---------- structures */

struct bink_debug_info /* fake name, from halo 3 */
{
	long previous_skipped_frames;
	long previous_skipped_blits;
	long previous_desired_frame_index;
	long desired_frame_index;
	long display_skipped_frames;
	long display_skipped_blits;
	long display_frames;
};

struct bink_globals_definition /* fake name */
{
	boolean initialized;
	boolean time_to_decompress_next_frame;
	unsigned long playback_flags;
	HBINK bink;
	short width;
	short height;
	boolean unknown10;
	long unknown14;
	long surface_type;
	long bytes_per_pixel;
	IDirect3DTexture8 *texture;
	struct bink_debug_info debug_info;
	struct rasterizer_dynamic_screen_geometry_parameters screen_geometry_parameters;
	void *memory_pool_base;
	long memory_pool_offset;
	long memory_pool_size;
};

/* ---------- prototypes */

static void bink_get_memory_available(); /* K&R style: some callers pass a string, which blocks inlining */
static void *bink_alloc_permanent(unsigned long size_in_bytes, unsigned long alignment_in_bytes);
static boolean is_all_bink_memory_free(void);
static void *RADLINK bink_alloc(unsigned long size_in_bytes);
static void RADLINK bink_free(void *pointer);
static void bink_decompress_audio_frame(void);
static void bink_draw_frame(void);
static boolean bink_query_analog_controller_buttons(void);
static void bink_free_texture_cache(void);
static void bink_decompress_video_frame(void);
static void bink_playback_update__internal(void);

/* ---------- globals */

static long bink_memory_available = NONE; /* fake name */
static boolean bink_saved_frame_rate_throttle = TRUE; /* fake name */

static void *bink_pointer_list[MAXIMUM_BINK_POINTER_BLOCKS];
static struct bitmap_data bink_bitmap; /* fake name */
static IDirect3DBaseTexture8 bink_texture; /* fake name */
boolean debug_bink;
struct bink_globals_definition bink_globals;
static long bink_pointer_count;

/* ---------- public code */

static void bink_get_memory_available()
{
	MEMORYSTATUS status;

	memset(&status, 0, sizeof(status));
	status.dwLength = sizeof(status);
	GlobalMemoryStatus(&status);
	bink_memory_available = status.dwAvailPhys >> 10;

	return;
}

boolean bink_playback_active(
	void)
{
	return bink_globals.bink && bink_globals.initialized;
}

boolean bink_playback_ui_rendering_inhibited(
	void)
{
	return bink_globals.initialized && TEST_FLAG(bink_globals.playback_flags, _bink_playback_dont_render_ui_bit);
}

boolean bink_playback_in_progress(
	void)
{
	return bink_globals.bink != NULL;
}

static void *bink_alloc_permanent(
	unsigned long size_in_bytes,
	unsigned long alignment_in_bytes)
{
	char *pointer = (char *)bink_globals.memory_pool_base + bink_globals.memory_pool_size - size_in_bytes;

	if (alignment_in_bytes && ((unsigned long)pointer & (alignment_in_bytes - 1)))
	{
		match_assert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 691, alignment_in_bytes>0 && (alignment_in_bytes&(alignment_in_bytes-1))==0);
		size_in_bytes += (alignment_in_bytes - (unsigned long)pointer) & (alignment_in_bytes - 1);
		pointer -= (alignment_in_bytes - (unsigned long)pointer) & (alignment_in_bytes - 1);
	}

	match_assert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 697, size_in_bytes>0);
	match_assert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 698, bink_globals.memory_pool_size>=size_in_bytes);
	match_assert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 699, bink_globals.memory_pool_base);
	bink_globals.memory_pool_size -= size_in_bytes;

	return pointer;
}

static boolean is_all_bink_memory_free(
	void)
{
	boolean all_bink_memory_is_free = TRUE;
	short pointer_index;

	for (pointer_index = 0; pointer_index < bink_pointer_count; pointer_index++)
	{
		if (bink_pointer_list[pointer_index])
		{
			all_bink_memory_is_free = FALSE;
		}
	}

	return all_bink_memory_is_free;
}

static void *RADLINK bink_alloc(
	unsigned long size_in_bytes)
{
	void *pointer = NULL;
	boolean allocation_failed;

	bink_get_memory_available();

	if (bink_pointer_count > 0 && !bink_pointer_list[0])
	{
		match_vassert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 735, is_all_bink_memory_free(), "### FATAL_ERROR bink just confused the hell out of me (1)");
		if (is_all_bink_memory_free())
		{
			bink_pointer_count = 0;
			bink_globals.memory_pool_offset = 0;
		}
	}

	allocation_failed = bink_globals.memory_pool_offset + size_in_bytes > (unsigned long)bink_globals.memory_pool_size ||
		bink_pointer_count >= MAXIMUM_BINK_POINTER_BLOCKS ||
		!bink_globals.memory_pool_base;
	if (allocation_failed)
	{
		match_assert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 759, !"bink memory allocation should not fail");
		__asm int 3;
	}
	else
	{
		pointer = (char *)bink_globals.memory_pool_base + bink_globals.memory_pool_offset;
		bink_globals.memory_pool_offset += size_in_bytes;
		XPhysicalProtect(pointer, size_in_bytes, PAGE_READWRITE);
		match_vassert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 786, bink_globals.memory_pool_offset <= bink_globals.memory_pool_size,
			csprintf(temporary, "### FATAL_ERROR bink needs more memory (requested %d bytes over the %d-byte limit)",
				bink_globals.memory_pool_offset - bink_globals.memory_pool_size, bink_globals.memory_pool_size));
		match_vassert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 787, bink_pointer_count < MAXIMUM_BINK_POINTER_BLOCKS, "### FATAL_ERROR bink needs more pointer blocks");
		bink_pointer_list[bink_pointer_count++] = pointer;

		bink_get_memory_available();

		match_assert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 799, bink_globals.memory_pool_offset < bink_globals.memory_pool_size);
		bink_globals.memory_pool_offset += BINK_MEMORY_GUARD_SIZE;
		if (bink_globals.memory_pool_offset > bink_globals.memory_pool_size)
		{
			bink_globals.memory_pool_offset = bink_globals.memory_pool_size;
		}
	}

	return pointer;
}

static void RADLINK bink_free(
	void *pointer)
{
	long pointer_index;

	bink_get_memory_available();

	for (pointer_index = 0; pointer_index < bink_pointer_count; pointer_index++)
	{
		if (bink_pointer_list[pointer_index] == pointer)
		{
			bink_pointer_list[pointer_index] = NULL;
			break;
		}
	}
	match_vassert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 825, pointer_index < bink_pointer_count, "### FATAL_ERROR bink just confused the hell out of me (2)");

	bink_get_memory_available();

	return;
}

static void bink_decompress_audio_frame(
	void)
{
	return;
}

static void bink_draw_frame(
	void)
{
	rectangle2d bounds;
	struct dynamic_screen_vertex vertices[4];
	short vertex_index;

	if (TEST_FLAG(bink_globals.playback_flags, _bink_playback_full_screen_bit))
	{
		bounds = rasterizer_globals.screen_bounds;
	}
	else
	{
		short screen_width = rasterizer_globals.screen_bounds.x1 - rasterizer_globals.screen_bounds.x0;
		short screen_height = rasterizer_globals.screen_bounds.y1 - rasterizer_globals.screen_bounds.y0;

		bounds.x0 = (screen_width - bink_globals.width) / 2;
		bounds.x1 = (screen_width + bink_globals.width) / 2;
		bounds.y0 = (screen_height - bink_globals.height) / 2;
		bounds.y1 = (screen_height + bink_globals.height) / 2;
	}

	for (vertex_index = 0; vertex_index < 4; vertex_index++)
	{
		real u = ((vertex_index + 1) & 2) ? (real)bink_globals.width : 0.0f;
		real v = vertex_index > 1 ? (real)bink_globals.height : 0.0f;
		real x = ((vertex_index + 1) & 2) ? (real)bounds.x1 : (real)bounds.x0;
		real y = vertex_index > 1 ? (real)bounds.y1 : (real)bounds.y0;

		vertices[vertex_index].position.x = x;
		vertices[vertex_index].position.y = y;
		vertices[vertex_index].texcoord.x = u;
		vertices[vertex_index].texcoord.y = v;
		vertices[vertex_index].color = PIXEL32_FROM_ARGB(255, 255, 255, 255);
	}
	rasterizer_psuedo_dynamic_screen_quad_draw(&bink_globals.screen_geometry_parameters, vertices);

	if (debug_bink)
	{
		char string[4096];
		BINKSUMMARY summary;
		BINKREALTIME realtime;
		rectangle2d bounds;
		short tab_stops[6];
		point2d cursor;
		real one_over_frames;

		tab_stops[0] = 250;
		tab_stops[1] = 0;
		tab_stops[2] = 0;
		tab_stops[3] = 0;
		tab_stops[4] = 0;
		tab_stops[5] = 0;
		cursor.x = 0;
		cursor.y = 0;
		BinkGetRealtime(bink_globals.bink, &realtime, 0);
		one_over_frames = 1.0f / realtime.Frames;
		sprintf(string, "FramesTime=|t%.02f|nFrameVideoDecompTime=|t%.02f|nFrameAudioDecompTime=|t%.02f|nFrameReadTime=|t%.02f|nFrameIdleReadTime=|t%.02f|nFrameThreadReadTime=|t%.02f|nFramesBlitTime=|t%.02f|n|nFrames=|t%d",
			realtime.FramesTime * one_over_frames,
			realtime.FramesVideoDecompTime * one_over_frames,
			realtime.FramesAudioDecompTime * one_over_frames,
			realtime.FramesReadTime * one_over_frames,
			realtime.FramesIdleReadTime * one_over_frames,
			realtime.FramesThreadReadTime * one_over_frames,
			realtime.FramesBlitTime * one_over_frames,
			realtime.Frames);
		bounds = rasterizer_globals.frame_bounds;
		offset_rectangle2d(&bounds, 0, 32);
		interface_set_bitmap_text_draw_mode(1, NONE, 0, 0, 5, 0);
		draw_string_set_color(global_real_argb_green);
		draw_string_set_tab_stops(tab_stops, 1);
		rasterizer_draw_string(&bounds, NULL, &cursor, -4, string);
		bounds.y0 = cursor.y + 31;

		if (bink_globals.debug_info.desired_frame_index - bink_globals.debug_info.previous_desired_frame_index >= 29)
		{
			BinkGetSummary(bink_globals.bink, &summary);
			bink_globals.debug_info.display_skipped_frames = summary.SkippedFrames - bink_globals.debug_info.previous_skipped_frames;
			bink_globals.debug_info.display_skipped_blits = summary.SkippedBlits - bink_globals.debug_info.previous_skipped_blits;
			bink_globals.debug_info.display_frames = bink_globals.debug_info.desired_frame_index - bink_globals.debug_info.previous_desired_frame_index;
			bink_globals.debug_info.previous_skipped_frames = summary.SkippedFrames;
			bink_globals.debug_info.previous_skipped_blits = summary.SkippedBlits;
			bink_globals.debug_info.previous_desired_frame_index = bink_globals.debug_info.desired_frame_index;
		}
		sprintf(string, "SkippedFrames=|t%d (%d)|nSkippedBlits=|t%d|n",
			bink_globals.debug_info.display_skipped_frames,
			bink_globals.debug_info.display_frames,
			bink_globals.debug_info.display_skipped_blits);
		draw_string_set_color(global_real_argb_red);
		rasterizer_draw_string(&bounds, NULL, &cursor, -4, string);
	}

	return;
}

static boolean bink_query_analog_controller_buttons(
	void)
{
	boolean button_down = FALSE;
	short gamepad_index;

	for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS && !button_down; gamepad_index++)
	{
		struct gamepad_state const *state = input_get_gamepad_state(gamepad_index);

		if (state)
		{
			short button_index;

			button_down = state->buttons[_gamepad_binary_button_start] == 1 || state->buttons[_gamepad_binary_button_back] == 1;
			if (!button_down)
			{
				for (button_index = 0; button_index < NUMBER_OF_GAMEPAD_ANALOG_BUTTONS; button_index++)
				{
					if (state->buttons[button_index] == 1)
					{
						button_down = TRUE;
						break;
					}
				}
			}
		}
	}

	return button_down;
}

void bink_playback_initialize(
	void)
{
	memset(&bink_globals, 0, sizeof(bink_globals));
	RADSetMemory(bink_alloc, bink_free);
	bink_globals.initialized = TRUE;

	return;
}

static void bink_free_texture_cache(
	void)
{
	if (bink_globals.initialized && bink_globals.memory_pool_base)
	{
		match_vassert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 665, is_all_bink_memory_free(), "we released the texture cache but we still had memory allocated");
		texture_cache_return_memory();
		bink_globals.memory_pool_base = NULL;
		bink_globals.texture = NULL;
		bink_globals.memory_pool_size = 0;
	}

	return;
}

static void bink_decompress_video_frame(
	void)
{
	D3DLOCKED_RECT locked_rect;

	BinkDoFrame(bink_globals.bink);
	BinkNextFrame(bink_globals.bink);
	rasterizer_globals.current_lock_operation = _rasterizer_lock_bink;
	IDirect3DTexture8_LockRect(bink_globals.texture, 0, &locked_rect, NULL, 0);
	BinkCopyToBuffer(bink_globals.bink, locked_rect.pBits, locked_rect.Pitch, bink_globals.bink->Height, 0, 0, bink_globals.surface_type | BINKCOPYALL);
	IDirect3DTexture8_UnlockRect(bink_globals.texture, 0);
	rasterizer_globals.current_lock_operation = _rasterizer_lock_none;

	return;
}

void bink_playback_stop(
	void)
{
	if (bink_globals.initialized)
	{
		if (TEST_FLAG(bink_globals.playback_flags, _bink_playback_prevent_events_to_ui_bit))
		{
			event_manager_suppress(FALSE);
		}

		if (bink_globals.bink)
		{
			BinkClose(bink_globals.bink);
			bink_globals.bink = NULL;
		}

		bink_free_texture_cache();

		if (TEST_FLAG(bink_globals.playback_flags, _bink_playback_return_to_main_menu_when_finished_bit))
		{
			main_menu_load();
		}

		bink_globals.playback_flags = 0;
		global_frame_rate_throttle = bink_saved_frame_rate_throttle;
		attract_mode_reset_timer();
	}

	return;
}

void bink_playback_dispose(
	void)
{
	if (bink_globals.initialized)
	{
		bink_playback_stop();
		memset(&bink_globals, 0, sizeof(bink_globals));
	}

	return;
}

static void bink_playback_update__internal(
	void)
{
	boolean stop_playback = FALSE;

	if (bink_globals.initialized && bink_globals.bink)
	{
		if (global_frame_rate_throttle)
		{
			bink_globals.time_to_decompress_next_frame = !BinkWait(bink_globals.bink);
		}
		else
		{
			while (BinkWait(bink_globals.bink));
			bink_globals.time_to_decompress_next_frame = TRUE;
		}

		if (TEST_FLAG(bink_globals.playback_flags, _bink_playback_button_click_stops_movie_bit))
		{
			boolean skipping_allowed = TEST_FLAG(bink_globals.playback_flags, _bink_playback_dont_allow_skipping_if_filesystem_check_thread_is_active_bit) ?
				!filesystem_check_thread_is_active() :
				TRUE;

			if (skipping_allowed && bink_query_analog_controller_buttons())
			{
				stop_playback = TRUE;
			}
		}

		if (!stop_playback &&
			(!bink_globals.bink || bink_globals.bink->FrameNum == bink_globals.bink->Frames - 1) &&
			!TEST_FLAG(bink_globals.playback_flags, _bink_playback_loop_bit))
		{
			stop_playback = TRUE;
		}

		if (stop_playback)
		{
			bink_playback_stop();
		}
	}

	return;
}

void bink_playback_render(
	void)
{
	if (bink_globals.initialized && bink_globals.bink)
	{
		if (global_frame_rate_throttle)
		{
			bink_globals.time_to_decompress_next_frame = TRUE;
		}

		if (bink_globals.time_to_decompress_next_frame)
		{
			bink_decompress_audio_frame();
			bink_decompress_video_frame();
			bink_globals.time_to_decompress_next_frame = FALSE;
		}

		bink_draw_frame();
		bink_globals.debug_info.desired_frame_index++;

		if (!global_frame_rate_throttle)
		{
			bink_playback_update__internal();
		}
	}

	return;
}

void bink_playback_start(
	const char *full_pathname,
	unsigned long flags)
{
	bink_get_memory_available();

	if (bink_globals.initialized && !cache_files_precache_in_progress())
	{
		match_vassert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 414, !bink_globals.bink, "there is already a bink movie being played");

		bink_globals.memory_pool_size = BINK_MEMORY_POOL_SIZE + (TEST_FLAG(flags, _bink_playback_eat_up_memory_like_a_goddamn_beaver_bit) ? BINK_MEMORY_POOL_EXTRA_SIZE : 0);
		bink_globals.memory_pool_offset = 0;
		bink_globals.memory_pool_base = texture_cache_steal_memory(bink_globals.memory_pool_size);
		match_assert("c:\\halo\\SOURCE\\bink\\bink_playback.c", 424, 0 == (bink_globals.memory_pool_size % CPU_PAGE_SIZE));
		XPhysicalProtect(bink_globals.memory_pool_base, bink_globals.memory_pool_size, PAGE_READONLY);

		if (bink_globals.memory_pool_base)
		{
			LPDIRECTSOUND dsound;
			boolean success;

			bink_get_memory_available("begin BinkOpen");
			dsound = dsound_get();
			success = FALSE;
			if (dsound)
			{
				bink_get_memory_available("begin BinkSoundUseDirectSound");
				if (BinkSoundUseDirectSound(dsound))
				{
					success = TRUE;
				}
				bink_get_memory_available("end BinkSoundUseDirectSound");
			}
			if (!success)
			{
				error(_error_silent, "### ERROR no DirectSound for bink");
			}

			if (TEST_FLAG(flags, _bink_playback_eat_up_memory_like_a_goddamn_beaver_bit))
			{
				BinkSetIOSize(BINK_IO_BUFFER_SIZE);
				bink_globals.bink = BinkOpen(full_pathname, BINKIOSIZE);
			}
			else
			{
				bink_globals.bink = BinkOpen(full_pathname, 0);
			}
			bink_get_memory_available("end BinkOpen");

			if (bink_globals.bink)
			{
				long frame_buffer_size = (bink_globals.bink->Width * bink_globals.bink->Height * 4 + (CPU_PAGE_SIZE - 1)) & ~(CPU_PAGE_SIZE - 1);
				char *frame_buffer = bink_alloc_permanent(frame_buffer_size, BINK_FRAME_BUFFER_ALIGNMENT);
				long byte_index;

				bink_get_memory_available("begin XPhysicalProtect");
				XPhysicalProtect(frame_buffer, frame_buffer_size, PAGE_READWRITE | PAGE_WRITECOMBINE);
				bink_get_memory_available("end XPhysicalProtect");

				for (byte_index = 0; byte_index < frame_buffer_size; byte_index++)
				{
					frame_buffer[byte_index] = rand() >> 8;
				}

				bink_globals.width = (short)bink_globals.bink->Width;
				bink_globals.height = (short)bink_globals.bink->Height;
				bink_globals.unknown14 = 0;
				bink_globals.surface_type = BINKSURFACE32;
				bink_globals.bytes_per_pixel = 4;
				bink_globals.unknown10 = FALSE;
				bink_globals.texture = (IDirect3DTexture8 *)&bink_texture;

				bink_texture.Common = D3DCOMMON_TYPE_TEXTURE | 1;
				bink_texture.Data = 0;
				bink_texture.Lock = 0;
				bink_texture.Size = ((((unsigned long)(bink_globals.width * bink_globals.bytes_per_pixel) / D3DTEXTURE_PITCH_ALIGNMENT - 1) << D3DSIZE_PITCH_SHIFT) |
					((bink_globals.height - 1) << D3DSIZE_HEIGHT_SHIFT) |
					(bink_globals.width - 1));
				bink_texture.Format = (D3DFMT_LIN_X8R8G8B8 << D3DFORMAT_FORMAT_SHIFT) |
					(1 << D3DFORMAT_MIPMAP_SHIFT) |
					(2 << D3DFORMAT_DIMENSION_SHIFT) |
					D3DFORMAT_BORDERSOURCE_COLOR |
					D3DFORMAT_DMACHANNEL_A;
				IDirect3DBaseTexture8_Register(&bink_texture, frame_buffer);

				memset(&bink_globals.screen_geometry_parameters, 0, sizeof(bink_globals.screen_geometry_parameters));
				bink_globals.screen_geometry_parameters.map_texture_scale[0].i = 1.0f;
				bink_globals.screen_geometry_parameters.map_texture_scale[0].j = 1.0f;
				bink_globals.screen_geometry_parameters.map_scale[0].i = 1.0f;
				bink_globals.screen_geometry_parameters.map_scale[0].j = 1.0f;
				bink_globals.screen_geometry_parameters.meter_parameters = NULL;
				bink_globals.screen_geometry_parameters.point_sampled = FALSE;
				bink_globals.screen_geometry_parameters.framebuffer_blend_function = _shader_framebuffer_blend_function_alpha_multiply_add;

				bink_bitmap.signature = BITMAP_SIGNATURE;
				bink_bitmap.width = bink_globals.width;
				bink_bitmap.height = bink_globals.height;
				bink_bitmap.depth = 1;
				bink_bitmap.type = _bitmap_type_2d;
				bink_bitmap.format = _bitmap_format_x8r8g8b8;
				bink_bitmap.flags = FLAG(_bitmap_linear_bit);
				bink_bitmap.pixels_size = bink_bitmap.width * bink_bitmap.height * bitmap_format_get_bits_per_pixel(bink_bitmap.format) / 8;
				bink_bitmap.tag_index = NONE;
				bink_bitmap.cache_block_index = NONE;
				bink_bitmap.base_address = (void *)NONE;
				bink_bitmap.hardware_format = bink_globals.texture;
				bink_globals.screen_geometry_parameters.map[0] = &bink_bitmap;

				bink_globals.playback_flags = flags;
				if (TEST_FLAG(flags, _bink_playback_prevent_events_to_ui_bit))
				{
					bink_get_memory_available("begin event_manager");
					event_manager_flush();
					event_manager_suppress(TRUE);
					bink_get_memory_available("end event_manager");
				}

				bink_decompress_video_frame();
				bink_globals.debug_info.previous_skipped_frames = 0;
				bink_globals.debug_info.previous_skipped_blits = 0;
				bink_globals.debug_info.previous_desired_frame_index = 0;
				bink_globals.debug_info.desired_frame_index = 0;
				bink_globals.debug_info.display_skipped_frames = 0;
				bink_globals.debug_info.display_skipped_blits = 0;
				bink_globals.debug_info.display_frames = 0;
				bink_saved_frame_rate_throttle = global_frame_rate_throttle;
				global_frame_rate_throttle = FALSE;
			}
			else
			{
				error(_error_silent, "### ERROR failed to open bink file '%s'", full_pathname);
				bink_free_texture_cache();
			}
		}
		else
		{
			error(_error_immediate, "### ERROR bink failed to fucking steal some fucking memory from the fucking texture cache");
			bink_free_texture_cache();
		}
	}

	return;
}

void bink_playback_update(
	void)
{
	if (global_frame_rate_throttle)
	{
		bink_playback_update__internal();
	}

	return;
}

/* ---------- private code */
